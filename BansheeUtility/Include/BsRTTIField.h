#pragma once

#include <string>

#include <type_traits>

#include "BsPrerequisitesUtil.h"
#include "BsIReflectable.h"
#include "BsManagedDataBlock.h"
#include "BsException.h"
#include "BsAny.h"

namespace BansheeEngine
{
	class RTTITypeBase;
	struct RTTIField;

	/**
	 * @brief	Types of fields we can serialize:
	 * 			
	 * - Plain - Native data types, POD (Plain old data) structures, or in general types we don't want to (or can't) inherit from IReflectable.   
	 *			 Type must be copyable by memcpy.
	 *			  
	 * - DataBlock - Array of bytes of a certain size. When returning a data block you may specify if its managed or unmanaged.  
	 *				 Managed data blocks have their buffers deleted after they go out of scope. This is useful if you need to return some
	 *				 temporary data. On the other hand if the data in the block belongs to your class, and isn't temporary, keep the data unmanaged.
	 *				 
	 * - Reflectable - Field that is of IReflectable type. Cannot be a pointer to IReflectable and must be actual value type.   
	 *				   Type and its fields are serialized recursively. Supports versioning so you may add/remove fields from the type
	 *				   without breaking previously serialized data.
	 * 
	 * - ReflectablePtr - A pointer to IReflectable. Same as "Reflectable" except that data isn't serialized as a value type,  
	 *					  but as a pointer, which may be referenced by multiple other instances. All references are saved upon
	 *					  serialization and restored upon deserialization.
	 */
	enum SerializableFieldType
	{
		SerializableFT_Plain,
		SerializableFT_DataBlock,
		SerializableFT_Reflectable,
		SerializableFT_ReflectablePtr
	};

	/**
	 * @brief	Various flags you can assign to RTTI fields.
	 */
	enum RTTIFieldFlag
	{
		// This flag is only used on field types of ReflectablePtr type, and it is used
		// to solve circular references. Circular references cause an issue when deserializing, 
		// as the algorithm doesn't know which object to deserialize first. By making one of 
		// the references weak, you tell the algorithm that it doesn't have to guarantee 
		// the object will be fully deserialized before being assigned to the field.
		//
		// In short: If you make a reference weak, when "set" method of that field is called,
		// it is not guaranteed the value provided is fully initialized, so you should not access any of its
		// data until deserialization is fully complete. You only need to use this flag if the RTTI system
		// complains that is has found a circular reference.
		RTTI_Flag_WeakRef = 0x01
	};

	/**
	 * @brief	Structure that keeps meta-data concerning a single class field. You can use
	 * 			this data for setting and getting values for that field on a specific class instance.
	 * 			
	 *			Class also contains an unique field name, and an unique field ID. Fields may contain
	 *			single types or an array of types. See "SerializableFieldType" for information about 
	 *			specific field types.
	 * 			
	 * @note	Most of the methods for retrieving and setting data accept "void *" for both
	 * 			the data and the owning class instance. It is up to the caller to ensure that
	 * 			pointer is of proper type.
	 */
	struct BS_UTILITY_EXPORT RTTIField
	{
		Any valueGetter;
		Any valueSetter;

		Any arraySizeGetter;
		Any arraySizeSetter;

		String mName;
		UINT16 mUniqueId;
		bool mIsVectorType;
		SerializableFieldType mType;
		UINT64 mFlags;

		bool isPlainType() { return mType == SerializableFT_Plain; }
		bool isDataBlockType() { return mType == SerializableFT_DataBlock; }
		bool isReflectableType() { return mType == SerializableFT_Reflectable; }
		bool isReflectablePtrType() { return mType == SerializableFT_ReflectablePtr; }
		
		/**
		 * @brief	Returns flags that were set in the field meta-data.
		 */
		UINT64 getFlags() const { return mFlags; }

		/**
		 * @brief	Gets the size of an array contained by the field, if the field
		 * 			represents an array. Throws exception if field is not an array.
		 */
		virtual UINT32 getArraySize(void* object) = 0;

		/**
		 * @brief	Changes the size of an array contained by the field, if the field
		 * 			represents an array. Throws exception if field is not an array.
		 */
		virtual void setArraySize(void* object, UINT32 size) = 0;

		/**
		 * @brief	Returns the type id for the type used in this field.
		 */
		virtual UINT32 getTypeSize() = 0;

		/**
		 * @brief	Query if the field has dynamic size. 
		 *
		 * @note	Field should have dynamic size if:
		 * 			 - The field can have varying size  
		 * 			 - The field size is over 255  
		 * 			 
		 * 			Types like integers, floats, bools, POD structs dont have dynamic size.
		 * 			Types like strings, vectors, maps do.
		 * 			
		 *			If your type has a static size but that size exceeds 255 bytes you also need to
		 *			use dynamic field size. (You will be warned during compilation if you don't follow this rule)
		 */
		virtual bool hasDynamicSize() = 0;

		/**
		 * @brief	Throws an exception if this field doesn't contain a plain value.
		 *
		 * @param	array	If true then the field must support plain array type.
		 */
		void checkIsPlain(bool array);

		/**
		 * @brief	Throws an exception if this field doesn't contain a complex value.
		 *
		 * @param	array	If true then the field must support complex array type.
		 */
		void checkIsComplex(bool array);

		/**
		 * @brief	Throws an exception if this field doesn't contain a complex pointer value.
		 *
		 * @param	array	If true then the field must support complex pointer array type.
		 */
		void checkIsComplexPtr(bool array);

		/**
		 * @brief	Throws an exception depending if the field is or isn't an array.
		 *
		 * @param	array	If true, then exception will be thrown if field is not an array.
		 * 					If false, then it will be thrown if field is an array.
		 */
		void checkIsArray(bool array);

		/**
		 * @brief	Throws an exception if this field doesn't contain a data block value.
		 */
		void checkIsDataBlock();

	protected:
		void initAll(Any valueGetter, Any valueSetter, Any arraySizeGetter, Any arraySizeSetter,
			String mName, UINT16 mUniqueId, bool mIsVectorType, SerializableFieldType type, UINT64 flags)
		{
			this->valueGetter = valueGetter;
			this->valueSetter = valueSetter;
			this->arraySizeGetter = arraySizeGetter;
			this->arraySizeSetter = arraySizeSetter;
			this->mName = mName;
			this->mUniqueId = mUniqueId;
			this->mIsVectorType = mIsVectorType;
			this->mType = type;
			this->mFlags = flags;
		}
	};
}