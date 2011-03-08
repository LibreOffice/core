/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include "CFStringUtilities.hxx"
#include "resourceprovider.hxx"
#include "NSString_OOoAdditions.hxx"

#include "ControlHelper.hxx"

#pragma mark DEFINES
#define CLASS_NAME "ControlHelper"
#define POPUP_WIDTH_MIN 200
#define POPUP_WIDTH_MAX 350

using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::ui::dialogs::TemplateDescription;
using namespace ::com::sun::star::ui::dialogs::ExtendedFilePickerElementIds;
using namespace ::com::sun::star::ui::dialogs::CommonFilePickerElementIds;
using namespace ::rtl;

#pragma mark Constructor / Destructor
//------------------------------------------------------------------------------------
// Constructor / Destructor
//------------------------------------------------------------------------------------
ControlHelper::ControlHelper()
: m_pUserPane(NULL)
, m_pFilterControl(nil)
, m_bUserPaneNeeded( false )
, m_bIsUserPaneLaidOut(false)
, m_bIsFilterControlNeeded(false)
, m_pFilterHelper(NULL)
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    int i;

    for( i = 0; i < TOGGLE_LAST; i++ ) {
        m_bToggleVisibility[i] = false;
    }

    for( i = 0; i < LIST_LAST; i++ ) {
        m_bListVisibility[i] = false;
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

ControlHelper::~ControlHelper()
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    NSAutoreleasePool *pool = [NSAutoreleasePool new];

    if (NULL != m_pUserPane) {
        [m_pUserPane release];
    }

    for(std::list<NSControl *>::iterator control = m_aActiveControls.begin(); control != m_aActiveControls.end(); ++control) {
        NSControl* pControl = (*control);
        NSString* sLabelName = m_aMapListLabels[pControl];
        if (sLabelName != nil) {
            [sLabelName release];
        }
        if ([pControl class] == [NSPopUpButton class]) {
            NSTextField* pField = m_aMapListLabelFields[(NSPopUpButton*)pControl];
            if (pField != nil) {
                [pField release];
            }
        }
        [pControl release];
    }

    if (m_pFilterControl != NULL) {
        [m_pFilterControl setTarget:nil];
    }

    [pool release];

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

#pragma mark XInitialization delegate
//------------------------------------------------
// XInitialization delegate
//------------------------------------------------
void ControlHelper::initialize( sal_Int16 nTemplateId )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "templateId", nTemplateId);

    switch( nTemplateId )
    {
        case FILESAVE_AUTOEXTENSION_PASSWORD:
            m_bToggleVisibility[AUTOEXTENSION] = true;
            m_bToggleVisibility[PASSWORD] = true;
            break;
        case FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS:
            m_bToggleVisibility[AUTOEXTENSION] = true;
            m_bToggleVisibility[PASSWORD] = true;
            m_bToggleVisibility[FILTEROPTIONS] = true;
            break;
        case FILESAVE_AUTOEXTENSION_SELECTION:
            m_bToggleVisibility[AUTOEXTENSION] = true;
            m_bToggleVisibility[SELECTION] = true;
            break;
        case FILESAVE_AUTOEXTENSION_TEMPLATE:
            m_bToggleVisibility[AUTOEXTENSION] = true;
            m_bListVisibility[TEMPLATE] = true;
            break;
        case FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE:
            m_bToggleVisibility[LINK] = true;
            m_bToggleVisibility[PREVIEW] = true;
            m_bListVisibility[IMAGE_TEMPLATE] = true;
            break;
        case FILEOPEN_READONLY_VERSION:
            m_bToggleVisibility[READONLY] = true;
            m_bListVisibility[VERSION] = true;
            break;
        case FILEOPEN_LINK_PREVIEW:
            m_bToggleVisibility[LINK] = true;
            m_bToggleVisibility[PREVIEW] = true;
            break;
        case FILESAVE_AUTOEXTENSION:
            m_bToggleVisibility[AUTOEXTENSION] = true;
            break;
    }

    createControls();

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

#pragma mark XFilePickerControlAccess delegates
//------------------------------------------------------------------------------------
// XFilePickerControlAccess functions
//------------------------------------------------------------------------------------

void ControlHelper::enableControl( const sal_Int16 nControlId, const sal_Bool bEnable ) const
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "controlId", nControlId, "enable", bEnable);

    SolarMutexGuard aGuard;

    if (nControlId == ExtendedFilePickerElementIds::CHECKBOX_PREVIEW) {
        OSL_TRACE(" preview checkbox cannot be changed");
        DBG_PRINT_EXIT(CLASS_NAME, __func__);
        return;
    }

    NSControl* pControl = getControl(nControlId);

    if( pControl != nil ) {
        if( bEnable ) {
            OSL_TRACE( "enable" );
        } else {
            OSL_TRACE( "disable" );
        }
        [pControl setEnabled:bEnable];
    } else {
        OSL_TRACE("enable unknown control %d", nControlId );
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

OUString ControlHelper::getLabel( sal_Int16 nControlId )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "controlId", nControlId);

    SolarMutexGuard aGuard;

    NSControl* pControl = getControl( nControlId );

    if( pControl == nil ) {
        OSL_TRACE("Get label for unknown control %d", nControlId);
        return OUString();
    }

    rtl::OUString retVal;
    if ([pControl class] == [NSPopUpButton class]) {
        NSString *temp = m_aMapListLabels[pControl];
        if (temp != nil)
            retVal = [temp OUString];
    }
    else {
        NSString* sLabel = [[pControl cell] title];
        retVal = [sLabel OUString];
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__, retVal);

    return retVal;
}

void ControlHelper::setLabel( sal_Int16 nControlId, const NSString* aLabel )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "controlId", nControlId, "label", aLabel);

    SolarMutexGuard aGuard;

    NSAutoreleasePool *pool = [NSAutoreleasePool new];

    NSControl* pControl = getControl(nControlId);

    if (nil != pControl) {
        if ([pControl class] == [NSPopUpButton class]) {
            NSString *sOldName = m_aMapListLabels[pControl];
            if (sOldName != NULL && sOldName != aLabel) {
                [sOldName release];
            }

            m_aMapListLabels[pControl] = [aLabel retain];
        } else if ([pControl class] == [NSButton class]) {
            [[pControl cell] setTitle:aLabel];
        }
    } else {
        OSL_TRACE("Control not found to set label for");
    }

    layoutControls();

    [pool release];

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void ControlHelper::setValue( sal_Int16 nControlId, sal_Int16 nControlAction, const uno::Any& rValue )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "controlId", nControlId, "controlAction", nControlAction);

    SolarMutexGuard aGuard;

    if (nControlId == ExtendedFilePickerElementIds::CHECKBOX_PREVIEW) {
        OSL_TRACE(" value for preview is unchangeable");
    }
    else {
        NSControl* pControl = getControl( nControlId );

        if( pControl == nil ) {
            OSL_TRACE("enable unknown control %d", nControlId);
        } else {
            if( [pControl class] == [NSPopUpButton class] ) {
                HandleSetListValue(pControl, nControlAction, rValue);
            } else if( [pControl class] == [NSButton class] ) {
                sal_Bool bChecked = false;
                rValue >>= bChecked;
                OSL_TRACE(" value is a bool: %d", bChecked);
                [(NSButton*)pControl setState:(bChecked ? NSOnState : NSOffState)];
            } else
            {
                OSL_TRACE("Can't set value on button / list %d %d",
                          nControlId, nControlAction);
            }
        }
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

uno::Any ControlHelper::getValue( sal_Int16 nControlId, sal_Int16 nControlAction ) const
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "controlId", nControlId, "controlAction", nControlAction);

    SolarMutexGuard aGuard;
    uno::Any aRetval;

    NSControl* pControl = getControl( nControlId );

    if( pControl == nil ) {
        OSL_TRACE("get value for unknown control %d", nControlId);
        aRetval <<= sal_True;
    } else {
        if( [pControl class] == [NSPopUpButton class] ) {
            aRetval = HandleGetListValue(pControl, nControlAction);
        } else if( [pControl class] == [NSButton class] ) {
            //NSLog(@"control: %@", [[pControl cell] title]);
            sal_Bool bValue = [(NSButton*)pControl state] == NSOnState ? sal_True : sal_False;
            aRetval <<= bValue;
            OSL_TRACE("value is a bool (checkbox): %d", bValue);
        }
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);

    return aRetval;
}

void ControlHelper::createUserPane()
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    if (m_bUserPaneNeeded == false) {
        OSL_TRACE("no user pane needed");
        DBG_PRINT_EXIT(CLASS_NAME, __func__);
        return;
    }

    if (nil != m_pUserPane) {
        OSL_TRACE("user pane already exists");
        DBG_PRINT_EXIT(CLASS_NAME, __func__);
        return;
    }

    if (m_bIsFilterControlNeeded == true && m_pFilterControl == nil) {
        createFilterControl();
    }

    NSRect minRect = NSMakeRect(0,0,300,33);
    m_pUserPane = [[NSView alloc] initWithFrame:minRect];

    int currentHeight = kAquaSpaceBoxFrameViewDiffTop + kAquaSpaceBoxFrameViewDiffBottom;
    int currentWidth = 300;

    BOOL bPopupControlPresent = NO;
    BOOL bButtonControlPresent = NO;

    int nCheckboxMaxWidth = 0;
    int nPopupMaxWidth = 0;
    int nPopupLabelMaxWidth = 0;

    for (::std::list<NSControl*>::iterator child = m_aActiveControls.begin(); child != m_aActiveControls.end(); child++) {
        OSL_TRACE("currentHeight: %d", currentHeight);

        NSControl* pControl = *child;

        //let the control calculate its size
        [pControl sizeToFit];

        NSRect frame = [pControl frame];
        OSL_TRACE("frame for control %s is {%f, %f, %f, %f}", [[pControl description] UTF8String], frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);

        int nControlHeight = frame.size.height;
        int nControlWidth = frame.size.width;

        // Note: controls are grouped by kind, first all popup menus, then checkboxes
        if ([pControl class] == [NSPopUpButton class]) {
            if (bPopupControlPresent == YES) {
                //this is not the first popup
                currentHeight += kAquaSpaceBetweenPopupMenus;
            }
            else if (child != m_aActiveControls.begin()){
                currentHeight += kAquaSpaceBetweenControls;
            }

            bPopupControlPresent = YES;

            // we have to add the label text width
            NSString *label = m_aMapListLabels[pControl];

            NSTextField *textField = createLabelWithString(label);
            [textField sizeToFit];
            m_aMapListLabelFields[(NSPopUpButton*)pControl] = textField;
            [m_pUserPane addSubview:textField];

            NSRect tfRect = [textField frame];
            OSL_TRACE("frame for textfield %s is {%f, %f, %f, %f}", [[textField description] UTF8String], tfRect.origin.x, tfRect.origin.y, tfRect.size.width, tfRect.size.height);

            int tfWidth = tfRect.size.width;

            if (nPopupLabelMaxWidth < tfWidth) {
                nPopupLabelMaxWidth = tfWidth;
            }

            frame.origin.x += (kAquaSpaceBetweenControls - kAquaSpaceLabelFrameBoundsDiffH - kAquaSpacePopupMenuFrameBoundsDiffLeft) + tfWidth;

            if (nControlWidth < POPUP_WIDTH_MIN) {
                nControlWidth = POPUP_WIDTH_MIN;
                frame.size.width = nControlWidth;
                [pControl setFrame:frame];
            }

            if (nControlWidth > POPUP_WIDTH_MAX) {
                nControlWidth = POPUP_WIDTH_MAX;
                frame.size.width = nControlWidth;
                [pControl setFrame:frame];
            }

            //set the max size
            if (nPopupMaxWidth < nControlWidth) {
                nPopupMaxWidth = nControlWidth;
            }

            nControlWidth += tfWidth + kAquaSpaceBetweenControls - kAquaSpaceLabelFrameBoundsDiffH - kAquaSpacePopupMenuFrameBoundsDiffLeft;
            if (nControlHeight < kAquaPopupButtonDefaultHeight) {
                //maybe the popup has no menu item yet, so set a default height
                nControlHeight = kAquaPopupButtonDefaultHeight;
            }

            nControlHeight -= kAquaSpacePopupMenuFrameBoundsDiffV;
        }
        else if ([pControl class] == [NSButton class]) {
            if (child != m_aActiveControls.begin()){
                currentHeight += kAquaSpaceBetweenControls;
            }

            if (nCheckboxMaxWidth < nControlWidth) {
                nCheckboxMaxWidth = nControlWidth;
            }

            bButtonControlPresent = YES;
            nControlWidth -= 2 * kAquaSpaceSwitchButtonFrameBoundsDiff;
            nControlHeight -= 2 * kAquaSpaceSwitchButtonFrameBoundsDiff;
        }

        // if ((nControlWidth + 2 * kAquaSpaceInsideGroupH) > currentWidth) {
        //     currentWidth = nControlWidth + 2 * kAquaSpaceInsideGroupH;
        // }

        currentHeight += nControlHeight;

        [m_pUserPane addSubview:pControl];
    }

    OSL_TRACE("height after adding all controls: %d", currentHeight);

    if (bPopupControlPresent && bButtonControlPresent)
    {
        //after a popup button (array) and before a different kind of control we need some extra space instead of the standard
        currentHeight -= kAquaSpaceBetweenControls;
        currentHeight += kAquaSpaceAfterPopupButtonsV;
        OSL_TRACE("popup extra space added, currentHeight: %d", currentHeight);
    }

    int nLongestPopupWidth = nPopupMaxWidth + nPopupLabelMaxWidth + kAquaSpaceBetweenControls - kAquaSpacePopupMenuFrameBoundsDiffLeft - kAquaSpaceLabelFrameBoundsDiffH;

    currentWidth = nLongestPopupWidth > nCheckboxMaxWidth ? nLongestPopupWidth : nCheckboxMaxWidth;
    OSL_TRACE("longest control width: %d", currentWidth);

    currentWidth += 2* kAquaSpaceInsideGroupH;

    if (currentWidth < minRect.size.width)
        currentWidth = minRect.size.width;

    if (currentHeight < minRect.size.height)
        currentHeight = minRect.size.height;

    NSRect upRect = NSMakeRect(0, 0, currentWidth, currentHeight );
    OSL_TRACE("setting user pane rect to {%f, %f, %f, %f}",upRect.origin.x, upRect.origin.y, upRect.size.width, upRect.size.height);

    [m_pUserPane setFrame:upRect];

    layoutControls();

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

#pragma mark Private / Misc
//------------------------------------------------------------------------------------
// Private / Misc
//------------------------------------------------------------------------------------
void ControlHelper::createControls()
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    CResourceProvider aResProvider;
    for (int i = 0; i < LIST_LAST; i++) {
        if (true == m_bListVisibility[i]) {
            m_bUserPaneNeeded = true;

            int elementName = getControlElementName([NSPopUpButton class], i);
            NSString* sLabel = aResProvider.getResString(elementName);

            m_pListControls[i] = [NSPopUpButton new];

#define MAP_LIST_( elem ) \
 case elem: \
     setLabel(ExtendedFilePickerElementIds::LISTBOX_##elem, sLabel); \
     break

            switch(i) {
                MAP_LIST_(VERSION);
                MAP_LIST_(TEMPLATE);
                MAP_LIST_(IMAGE_TEMPLATE);
            }

            m_aActiveControls.push_back(m_pListControls[i]);
        } else {
            m_pListControls[i] = nil;
        }
    }

    for (int i = 0/*#i102102*/; i < TOGGLE_LAST; i++) {
        if (true == m_bToggleVisibility[i]) {
            m_bUserPaneNeeded = true;

            int elementName = getControlElementName([NSButton class], i);
            NSString* sLabel = aResProvider.getResString(elementName);

            NSButton *button = [NSButton new];
            [button setTitle:sLabel];

            [button setButtonType:NSSwitchButton];

            [button setState:NSOffState];

            if (i == AUTOEXTENSION) {
                [button setTarget:m_pDelegate];
                [button setAction:@selector(autoextensionChanged:)];
            }

            m_pToggles[i] = button;

            m_aActiveControls.push_back(m_pToggles[i]);
        } else {
            m_pToggles[i] = nil;
        }
    }

    //preview is always on with Mac OS X
    NSControl *pPreviewBox = m_pToggles[PREVIEW];
    if (pPreviewBox != nil) {
        [pPreviewBox setEnabled:NO];
        [(NSButton*)pPreviewBox setState:NSOnState];
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

#define TOGGLE_ELEMENT( elem ) \
case elem: \
    nReturn = CHECKBOX_##elem; \
    DBG_PRINT_EXIT(CLASS_NAME, __func__, nReturn); \
    return nReturn
#define LIST_ELEMENT( elem ) \
case elem: \
    nReturn = LISTBOX_##elem##_LABEL; \
    DBG_PRINT_EXIT(CLASS_NAME, __func__, nReturn); \
    return nReturn

int ControlHelper::getControlElementName(const Class aClazz, const int nControlId) const
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "aClazz", [[aClazz description] UTF8String], "controlId", nControlId);

    int nReturn = -1;
    if (aClazz == [NSButton class])
    {
        switch (nControlId) {
            TOGGLE_ELEMENT( AUTOEXTENSION );
            TOGGLE_ELEMENT( PASSWORD );
            TOGGLE_ELEMENT( FILTEROPTIONS );
            TOGGLE_ELEMENT( READONLY );
            TOGGLE_ELEMENT( LINK );
            TOGGLE_ELEMENT( PREVIEW );
            TOGGLE_ELEMENT( SELECTION );
        }
    }
    else if (aClazz == [NSPopUpButton class])
    {
        switch (nControlId) {
            LIST_ELEMENT( VERSION );
            LIST_ELEMENT( TEMPLATE );
            LIST_ELEMENT( IMAGE_TEMPLATE );
        }
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__, nReturn);

    return nReturn;
}

void ControlHelper::HandleSetListValue(const NSControl* pControl, const sal_Int16 nControlAction, const uno::Any& rValue)
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "controlAction", nControlAction);

    if ([pControl class] != [NSPopUpButton class]) {
        OSL_TRACE("not a popup menu");
        DBG_PRINT_EXIT(CLASS_NAME, __func__);
        return;
    }

    NSPopUpButton *pButton = (NSPopUpButton*)pControl;
    NSMenu *rMenu = [pButton menu];
    if (nil == rMenu) {
        OSL_TRACE("button has no menu");
        DBG_PRINT_EXIT(CLASS_NAME, __func__);
        return;
    }

    switch (nControlAction)
    {
        case ControlActions::ADD_ITEM:
        {
            OSL_TRACE("ADD_ITEMS");
            OUString sItem;
            rValue >>= sItem;

            NSString* sCFItem = [NSString stringWithOUString:sItem];
            OSL_TRACE("Adding menu item: %s", OUStringToOString(sItem, RTL_TEXTENCODING_UTF8).getStr());
            [pButton addItemWithTitle:sCFItem];
        }
            break;
        case ControlActions::ADD_ITEMS:
        {
            OSL_TRACE("ADD_ITEMS");
            uno::Sequence< OUString > aStringList;
            rValue >>= aStringList;
            sal_Int32 nItemCount = aStringList.getLength();
            for (sal_Int32 i = 0; i < nItemCount; ++i)
            {
                NSString* sCFItem = [NSString stringWithOUString:aStringList[i]];
                OSL_TRACE("Adding menu item: %s", OUStringToOString(aStringList[i], RTL_TEXTENCODING_UTF8).getStr());
                [pButton addItemWithTitle:sCFItem];
            }
        }
            break;
        case ControlActions::DELETE_ITEM:
        {
            OSL_TRACE("DELETE_ITEM");
            sal_Int32 nPos = -1;
            rValue >>= nPos;
            OSL_TRACE("Deleting item at position %d", (nPos));
            [rMenu removeItemAtIndex:nPos];
        }
            break;
        case ControlActions::DELETE_ITEMS:
        {
            OSL_TRACE("DELETE_ITEMS");
            int nItems = [rMenu numberOfItems];
            if (nItems == 0) {
                OSL_TRACE("no menu items to delete");
                DBG_PRINT_EXIT(CLASS_NAME, __func__);
                return;
            }
            for(sal_Int32 i = 0; i < nItems; i++) {
                [rMenu removeItemAtIndex:i];
            }
        }
            break;
        case ControlActions::SET_SELECT_ITEM:
        {
            sal_Int32 nPos = -1;
            rValue >>= nPos;
            OSL_TRACE("Selecting item at position %d", nPos);
            [pButton selectItemAtIndex:nPos];
        }
            break;
        default:
            OSL_TRACE("undocumented/unimplemented ControlAction for a list");
            break;
    }

    layoutControls();

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}


uno::Any ControlHelper::HandleGetListValue(const NSControl* pControl, const sal_Int16 nControlAction) const
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "controlAction", nControlAction);

    uno::Any aAny;

    if ([pControl class] != [NSPopUpButton class]) {
        OSL_TRACE("not a popup button");
        DBG_PRINT_EXIT(CLASS_NAME, __func__);
        return aAny;
    }

    NSPopUpButton *pButton = (NSPopUpButton*)pControl;
    NSMenu *rMenu = [pButton menu];
    if (nil == rMenu) {
        OSL_TRACE("button has no menu");
        DBG_PRINT_EXIT(CLASS_NAME, __func__);
        return aAny;
    }

    switch (nControlAction)
    {
        case ControlActions::GET_ITEMS:
        {
            OSL_TRACE("GET_ITEMS");
            uno::Sequence< OUString > aItemList;

            int nItems = [rMenu numberOfItems];
            if (nItems > 0) {
                aItemList.realloc(nItems);
            }
            for (int i = 0; i < nItems; i++) {
                NSString* sCFItem = [pButton itemTitleAtIndex:i];
                if (nil != sCFItem) {
                    aItemList[i] = [sCFItem OUString];
                    OSL_TRACE("Return value[%d]: %s", (i - 1), OUStringToOString(aItemList[i - 1], RTL_TEXTENCODING_UTF8).getStr());
                }
            }

            aAny <<= aItemList;
        }
            break;
        case ControlActions::GET_SELECTED_ITEM:
        {
            OSL_TRACE("GET_SELECTED_ITEM");
            NSString* sCFItem = [pButton titleOfSelectedItem];
            if (nil != sCFItem) {
                OUString sString = [sCFItem OUString];
                OSL_TRACE("Return value: %s", OUStringToOString(sString, RTL_TEXTENCODING_UTF8).getStr());
                aAny <<= sString;
            }
        }
            break;
        case ControlActions::GET_SELECTED_ITEM_INDEX:
        {
            OSL_TRACE("GET_SELECTED_ITEM_INDEX");
            sal_Int32 nActive = [pButton indexOfSelectedItem];
            OSL_TRACE("Return value: %d", nActive);
            aAny <<= nActive;
        }
            break;
        default:
            OSL_TRACE("undocumented/unimplemented ControlAction for a list");
            break;
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);

    return aAny;
}


// cf. offapi/com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.idl
NSControl* ControlHelper::getControl( const sal_Int16 nControlId ) const
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "controlId", nControlId);

    NSControl* pWidget = nil;

#define MAP_TOGGLE( elem ) \
case ExtendedFilePickerElementIds::CHECKBOX_##elem: \
    pWidget = m_pToggles[elem]; \
    break

#define MAP_BUTTON( elem ) \
case ExtendedFilePickerElementIds::PUSHBUTTON_##elem: \
    pWidget = m_pButtons[elem]; \
    break

#define MAP_LIST( elem ) \
case ExtendedFilePickerElementIds::LISTBOX_##elem: \
    pWidget = m_pListControls[elem]; \
    break

#define MAP_LIST_LABEL( elem ) \
case ExtendedFilePickerElementIds::LISTBOX_##elem##_LABEL: \
    pWidget = m_pListControls[elem]; \
    break

    switch( nControlId )
    {
            MAP_TOGGLE( AUTOEXTENSION );
            MAP_TOGGLE( PASSWORD );
            MAP_TOGGLE( FILTEROPTIONS );
            MAP_TOGGLE( READONLY );
            MAP_TOGGLE( LINK );
            MAP_TOGGLE( PREVIEW );
            MAP_TOGGLE( SELECTION );
            //MAP_BUTTON( PLAY );
            MAP_LIST( VERSION );
            MAP_LIST( TEMPLATE );
            MAP_LIST( IMAGE_TEMPLATE );
            MAP_LIST_LABEL( VERSION );
            MAP_LIST_LABEL( TEMPLATE );
            MAP_LIST_LABEL( IMAGE_TEMPLATE );
        default:
            OSL_TRACE("Handle unknown control %d", nControlId);
            break;
    }
#undef MAP

    DBG_PRINT_EXIT(CLASS_NAME, __func__);

    return pWidget;
}

void ControlHelper::layoutControls()
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    SolarMutexGuard aGuard;

    if (nil == m_pUserPane) {
        OSL_TRACE("no user pane to layout");
        DBG_PRINT_EXIT(CLASS_NAME, __func__);
        return;
    }

    if (m_bIsUserPaneLaidOut == true) {
        OSL_TRACE("user pane already laid out");
        DBG_PRINT_EXIT(CLASS_NAME, __func__);
        return;
    }

    NSRect userPaneRect = [m_pUserPane frame];
    OSL_TRACE("userPane frame: {%f, %f, %f, %f}",userPaneRect.origin.x, userPaneRect.origin.y, userPaneRect.size.width, userPaneRect.size.height);

    int nUsableWidth = userPaneRect.size.width;

    //NOTE: NSView's coordinate system starts in the lower left hand corner but we start adding controls from the top,
    // so we subtract from the vertical position as we make our way down the pane.
    int currenttop = userPaneRect.size.height;
    int nCheckboxMaxWidth = 0;
    int nPopupMaxWidth = 0;
    int nPopupLabelMaxWidth = 0;

    //first loop to determine max sizes
    for (::std::list<NSControl*>::iterator child = m_aActiveControls.begin(); child != m_aActiveControls.end(); child++) {
        NSControl* pControl = *child;

        NSRect controlRect = [pControl frame];
        int nControlWidth = controlRect.size.width;

        Class aSubType = [pControl class];
        if (aSubType == [NSPopUpButton class]) {
            if (nPopupMaxWidth < nControlWidth) {
                nPopupMaxWidth = nControlWidth;
            }
            NSTextField *label = m_aMapListLabelFields[(NSPopUpButton*)pControl];
            NSRect labelFrame = [label frame];
            int nLabelWidth = labelFrame.size.width;
            if (nPopupLabelMaxWidth < nLabelWidth) {
                nPopupLabelMaxWidth = nLabelWidth;
            }
        } else {
            if (nCheckboxMaxWidth < nControlWidth) {
                nCheckboxMaxWidth = nControlWidth;
            }
        }
    }

    int nLongestPopupWidth = nPopupMaxWidth + nPopupLabelMaxWidth + kAquaSpaceBetweenControls - kAquaSpacePopupMenuFrameBoundsDiffLeft - kAquaSpaceLabelFrameBoundsDiffH;
    OSL_TRACE("longest popup width: %d", nLongestPopupWidth);

    NSControl* previousControl = nil;

    int nDistBetweenControls = 0;

    for (::std::list<NSControl*>::iterator child = m_aActiveControls.begin(); child != m_aActiveControls.end(); child++) {
        NSControl* pControl = *child;

        //get the control's bounds
        NSRect controlRect = [pControl frame];
        int nControlHeight = controlRect.size.height;
        int nControlWidth = controlRect.size.width;

        //subtract the height from the current vertical position, because the control's bounds origin rect will be its lower left hand corner
        currenttop -= nControlHeight;

        Class aSubType = [pControl class];

        //add space between the previous control and this control according to Apple's HIG
        nDistBetweenControls = getVerticalDistance(previousControl, pControl);
        OSL_TRACE("vertical distance: %d", nDistBetweenControls);
        currenttop -= nDistBetweenControls;

        previousControl = pControl;

        if (aSubType == [NSPopUpButton class]) {
            //move vertically up some pixels to space the controls between their real (visual) bounds
            currenttop += kAquaSpacePopupMenuFrameBoundsDiffTop;//from top

            //get the corresponding popup label
            NSTextField *label = m_aMapListLabelFields[(NSPopUpButton*)pControl];
            NSRect labelFrame = [label frame];
            int totalWidth = nPopupMaxWidth + labelFrame.size.width + kAquaSpaceBetweenControls - kAquaSpacePopupMenuFrameBoundsDiffLeft - kAquaSpaceLabelFrameBoundsDiffH;
            OSL_TRACE("totalWidth: %d", totalWidth);
            //let's center popups
            int left = (nUsableWidth + nLongestPopupWidth) / 2 - totalWidth;
            OSL_TRACE("left: %d", left);
            labelFrame.origin.x = left;
            labelFrame.origin.y = currenttop + kAquaSpaceLabelPopupDiffV;
            OSL_TRACE("setting label at: {%f, %f, %f, %f}",labelFrame.origin.x, labelFrame.origin.y, labelFrame.size.width, labelFrame.size.height);
            [label setFrame:labelFrame];

            controlRect.origin.x = left + labelFrame.size.width + kAquaSpaceBetweenControls - kAquaSpaceLabelFrameBoundsDiffH - kAquaSpacePopupMenuFrameBoundsDiffLeft;
            controlRect.origin.y = currenttop;
            controlRect.size.width = nPopupMaxWidth;
            OSL_TRACE("setting popup at: {%f, %f, %f, %f}",controlRect.origin.x, controlRect.origin.y, controlRect.size.width, controlRect.size.height);
            [pControl setFrame:controlRect];

            //add some space to place the vertical position right below the popup's visual bounds
            currenttop += kAquaSpacePopupMenuFrameBoundsDiffBottom;
        } else {
            currenttop += kAquaSpaceSwitchButtonFrameBoundsDiff;//from top

            nControlWidth = nCheckboxMaxWidth;
            int left = (nUsableWidth - nCheckboxMaxWidth) / 2;
            controlRect.origin.x = left;
            controlRect.origin.y = currenttop;
            controlRect.size.width = nPopupMaxWidth;
            [pControl setFrame:controlRect];
            OSL_TRACE("setting checkbox at: {%f, %f, %f, %f}",controlRect.origin.x, controlRect.origin.y, controlRect.size.width, controlRect.size.height);

            currenttop += kAquaSpaceSwitchButtonFrameBoundsDiff;
        }
    }

    m_bIsUserPaneLaidOut = true;

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void ControlHelper::createFilterControl() {
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    CResourceProvider aResProvider;
    NSString* sLabel = aResProvider.getResString(CommonFilePickerElementIds::LISTBOX_FILTER_LABEL);

    m_pFilterControl = [NSPopUpButton new];

    [m_pFilterControl setAction:@selector(filterSelectedAtIndex:)];
    [m_pFilterControl setTarget:m_pDelegate];

    NSMenu *menu = [m_pFilterControl menu];

    for (NSStringList::iterator iter = m_pFilterHelper->getFilterNames()->begin(); iter != m_pFilterHelper->getFilterNames()->end(); iter++) {
        NSString *filterName = *iter;
        OSL_TRACE("adding filter name: %s", [filterName UTF8String]);
        if ([filterName isEqualToString:@"-"]) {
            [menu addItem:[NSMenuItem separatorItem]];
        }
        else {
            [m_pFilterControl addItemWithTitle:filterName];
        }
    }

    // always add the filter as first item
    m_aActiveControls.push_front(m_pFilterControl);
    m_aMapListLabels[m_pFilterControl] = [sLabel retain];

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

NSTextField* ControlHelper::createLabelWithString(const NSString* labelString) {
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "label", labelString);

    NSTextField *textField = [NSTextField new];
    [textField setEditable:NO];
    [textField setSelectable:NO];
    [textField setDrawsBackground:NO];
    [textField setBordered:NO];
    [[textField cell] setTitle:labelString];

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
    return textField;
}

int ControlHelper::getVerticalDistance(const NSControl* first, const NSControl* second)
{
    if (first == nil) {
        return kAquaSpaceBoxFrameViewDiffTop;
    }
    else if (second == nil) {
        return kAquaSpaceBoxFrameViewDiffBottom;
    }
    else {
        Class firstClass = [first class];
        Class secondClass = [second class];

        if (firstClass == [NSPopUpButton class]) {
            if (secondClass == [NSPopUpButton class]) {
                return kAquaSpaceBetweenPopupMenus;
            }
            else {
                return kAquaSpaceAfterPopupButtonsV;
            }
        }

        return kAquaSpaceBetweenControls;
    }
}

void ControlHelper::updateFilterUI()
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    if (m_bIsFilterControlNeeded == false || m_pFilterHelper == NULL) {
        OSL_TRACE("no filter control needed or no filter helper present");
        DBG_PRINT_EXIT(CLASS_NAME, __func__);
        return;
    }

    int index = m_pFilterHelper->getCurrentFilterIndex();

    if (m_pFilterControl == nil) {
        createFilterControl();
    }

    [m_pFilterControl selectItemAtIndex:index];

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
