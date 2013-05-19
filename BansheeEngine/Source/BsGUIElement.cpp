#include "BsGUIElement.h"
#include "BsGUIWidget.h"
#include "BsGUISkin.h"
#include "BsGUILayout.h"
#include "CmException.h"

using namespace CamelotFramework;

namespace BansheeEngine
{
	GUIElement::GUIElement(GUIWidget& parent, const GUI_LAYOUT_OPTIONS& layoutOptions)
		:mParent(parent), mIsDirty(true), mParentLayout(nullptr), mLayoutOptions(layoutOptions), mWidth(0), mHeight(0), mDepth(0)
	{
		mParent.registerElement(this);
	}

	GUIElement::~GUIElement()
	{
		if(mParentLayout != nullptr)
			mParentLayout->removeElement(this);
	}

	void GUIElement::updateRenderElements()
	{
		updateRenderElementsInternal();
		markAsClean();
	}

	void GUIElement::setLayoutOptions(const GUI_LAYOUT_OPTIONS& layoutOptions) 
	{
		if(layoutOptions.maxWidth < layoutOptions.minWidth)
		{
			CM_EXCEPT(InvalidParametersException, "Maximum width is less than minimum width! Max width: " + 
			toString(layoutOptions.maxWidth) + ". Min width: " + toString(layoutOptions.minWidth));
		}

		if(layoutOptions.maxHeight < layoutOptions.minHeight)
		{
			CM_EXCEPT(InvalidParametersException, "Maximum height is less than minimum height! Max height: " + 
			toString(layoutOptions.maxHeight) + ". Min height: " + toString(layoutOptions.minHeight));
		}

		mLayoutOptions = layoutOptions; 
	}


	bool GUIElement::mouseEvent(const GUIMouseEvent& ev)
	{
		return false;
	}

	void GUIElement::_destroyInternal(GUIElement* element)
	{
		CM_DELETE(element, GUIElement, PoolAlloc);
	}

	void GUIElement::destroy(GUIElement* element)
	{
		element->mParent.unregisterElement(element);

		CM_DELETE(element, GUIElement, PoolAlloc);
	}
}