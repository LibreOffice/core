/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include "CFStringUtilities.hxx"
#include "resourceprovider.hxx"
#include "NSString_OOoAdditions.hxx"
#include "sal/log.hxx"

#include "ControlHelper.hxx"

#pragma mark DEFINES
#define POPUP_WIDTH_MIN 200
#define POPUP_WIDTH_MAX 350

using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::ui::dialogs::TemplateDescription;
using namespace ::com::sun::star::ui::dialogs::ExtendedFilePickerElementIds;
using namespace ::com::sun::star::ui::dialogs::CommonFilePickerElementIds;

namespace {

uno::Any HandleGetListValue(const NSControl* pControl, const sal_Int16 nControlAction)
{
    uno::Any aAny;

    if ([pControl class] != [NSPopUpButton class]) {
        SAL_INFO("fpicker.aqua","not a popup button");
        return aAny;
    }

    NSPopUpButton *pButton = (NSPopUpButton*)pControl;
    NSMenu *rMenu = [pButton menu];
    if (nil == rMenu) {
        SAL_INFO("fpicker.aqua","button has no menu");
        return aAny;
    }

    switch (nControlAction)
    {
        case ControlActions::GET_ITEMS:
        {
            SAL_INFO("fpicker.aqua","GET_ITEMS");
            uno::Sequence< OUString > aItemList;

            int nItems = [rMenu numberOfItems];
            if (nItems > 0) {
                aItemList.realloc(nItems);
            }
            for (int i = 0; i < nItems; i++) {
                NSString* sCFItem = [pButton itemTitleAtIndex:i];
                if (nil != sCFItem) {
                    aItemList[i] = [sCFItem OUString];
                    SAL_INFO("fpicker.aqua","Return value[" << (i - 1) << "]: " << aItemList[i - 1]);
                }
            }

            aAny <<= aItemList;
        }
            break;
        case ControlActions::GET_SELECTED_ITEM:
        {
            SAL_INFO("fpicker.aqua","GET_SELECTED_ITEM");
            NSString* sCFItem = [pButton titleOfSelectedItem];
            if (nil != sCFItem) {
                OUString sString = [sCFItem OUString];
                SAL_INFO("fpicker.aqua","Return value: " << sString);
                aAny <<= sString;
            }
        }
            break;
        case ControlActions::GET_SELECTED_ITEM_INDEX:
        {
            SAL_INFO("fpicker.aqua","GET_SELECTED_ITEM_INDEX");
            sal_Int32 nActive = [pButton indexOfSelectedItem];
            SAL_INFO("fpicker.aqua","Return value: " << nActive);
            aAny <<= nActive;
        }
            break;
        default:
            SAL_INFO("fpicker.aqua","undocumented/unimplemented ControlAction for a list");
            break;
    }

    return aAny;
}

NSTextField* createLabelWithString(NSString* labelString)
{
    NSTextField *textField = [NSTextField new];
    [textField setEditable:NO];
    [textField setSelectable:NO];
    [textField setDrawsBackground:NO];
    [textField setBordered:NO];
    SAL_WNODEPRECATED_DECLARATIONS_PUSH //TODO: 10.9 setTitle
    [[textField cell] setTitle:labelString];
    SAL_WNODEPRECATED_DECLARATIONS_POP

    return textField;
}

}

#pragma mark Constructor / Destructor

// Constructor / Destructor

ControlHelper::ControlHelper()
: m_pUserPane(nullptr)
, m_pFilterControl(nil)
, m_bUserPaneNeeded( false )
, m_bIsUserPaneLaidOut(false)
, m_bIsFilterControlNeeded(false)
, m_pFilterHelper(nullptr)
{
    int i;

    for( i = 0; i < TOGGLE_LAST; i++ ) {
        m_bToggleVisibility[i] = false;
    }

    for( i = 0; i < LIST_LAST; i++ ) {
        m_bListVisibility[i] = false;
    }
}

ControlHelper::~ControlHelper()
{
    NSAutoreleasePool *pool = [NSAutoreleasePool new];

    if (nullptr != m_pUserPane) {
        [m_pUserPane release];
    }

    if (m_pFilterControl != nullptr) {
        [m_pFilterControl setTarget:nil];
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

    [pool release];
}

#pragma mark XInitialization delegate

// XInitialization delegate

void ControlHelper::initialize( sal_Int16 nTemplateId )
{
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
        case FILEOPEN_PREVIEW:
            m_bToggleVisibility[PREVIEW] = true;
            break;
        case FILEOPEN_LINK_PLAY:
            m_bToggleVisibility[LINK] = true;
    }

    createControls();
}

#pragma mark XFilePickerControlAccess delegates

// XFilePickerControlAccess functions


void ControlHelper::enableControl( const sal_Int16 nControlId, const bool bEnable ) const
{
    SolarMutexGuard aGuard;

    if (nControlId == ExtendedFilePickerElementIds::CHECKBOX_PREVIEW) {
        SAL_INFO("fpicker.aqua"," preview checkbox cannot be changed");
        return;
    }

    NSControl* pControl = getControl(nControlId);

    if( pControl != nil ) {
        if( bEnable ) {
            SAL_INFO("fpicker.aqua", "enable" );
        } else {
            SAL_INFO("fpicker.aqua", "disable" );
        }
        [pControl setEnabled:bEnable];
    } else {
        SAL_INFO("fpicker.aqua","enable unknown control " << nControlId );
    }
}

OUString ControlHelper::getLabel( sal_Int16 nControlId )
{
    SolarMutexGuard aGuard;

    NSControl* pControl = getControl( nControlId );

    if( pControl == nil ) {
        SAL_INFO("fpicker.aqua","Get label for unknown control " << nControlId);
        return OUString();
    }

    rtl::OUString retVal;
    if ([pControl class] == [NSPopUpButton class]) {
        NSString *temp = m_aMapListLabels[pControl];
        if (temp != nil)
            retVal = [temp OUString];
    }
    else {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH //TODO: 10.9 title
        NSString* sLabel = [[pControl cell] title];
        SAL_WNODEPRECATED_DECLARATIONS_POP
        retVal = [sLabel OUString];
    }

    return retVal;
}

void ControlHelper::setLabel( sal_Int16 nControlId, NSString* aLabel )
{
    SolarMutexGuard aGuard;

    NSAutoreleasePool *pool = [NSAutoreleasePool new];

    NSControl* pControl = getControl(nControlId);

    if (nil != pControl) {
        if ([pControl class] == [NSPopUpButton class]) {
            NSString *sOldName = m_aMapListLabels[pControl];
            if (sOldName != nullptr && sOldName != aLabel) {
                [sOldName release];
            }

            m_aMapListLabels[pControl] = [aLabel retain];
        } else if ([pControl class] == [NSButton class]) {
            SAL_WNODEPRECATED_DECLARATIONS_PUSH //TODO: 10.9 setTitle
            [[pControl cell] setTitle:aLabel];
            SAL_WNODEPRECATED_DECLARATIONS_POP
        }
    } else {
        SAL_INFO("fpicker.aqua","Control not found to set label for");
    }

    layoutControls();

    [pool release];
}

void ControlHelper::setValue( sal_Int16 nControlId, sal_Int16 nControlAction, const uno::Any& rValue )
{
    SolarMutexGuard aGuard;

    if (nControlId == ExtendedFilePickerElementIds::CHECKBOX_PREVIEW) {
        SAL_INFO("fpicker.aqua"," value for preview is unchangeable");
    }
    else {
        NSControl* pControl = getControl( nControlId );

        if( pControl == nil ) {
            SAL_INFO("fpicker.aqua","enable unknown control " << nControlId);
        } else {
            if( [pControl class] == [NSPopUpButton class] ) {
                HandleSetListValue(pControl, nControlAction, rValue);
            } else if( [pControl class] == [NSButton class] ) {
                bool bChecked = false;
                rValue >>= bChecked;
                SAL_INFO("fpicker.aqua"," value is a bool: " << bChecked);
                [(NSButton*)pControl setState:(bChecked ? NSOnState : NSOffState)];
            } else
            {
                SAL_INFO("fpicker.aqua","Can't set value on button / list " << nControlId << " " << nControlAction);
            }
        }
    }
}

uno::Any ControlHelper::getValue( sal_Int16 nControlId, sal_Int16 nControlAction ) const
{
    SolarMutexGuard aGuard;
    uno::Any aRetval;

    NSControl* pControl = getControl( nControlId );

    if( pControl == nil ) {
        SAL_INFO("fpicker.aqua","get value for unknown control " << nControlId);
    } else {
        if( [pControl class] == [NSPopUpButton class] ) {
            aRetval = HandleGetListValue(pControl, nControlAction);
        } else if( [pControl class] == [NSButton class] ) {
            //NSLog(@"control: %@", [[pControl cell] title]);
            bool bValue = [(NSButton*)pControl state] == NSOnState;
            aRetval <<= bValue;
            SAL_INFO("fpicker.aqua","value is a bool (checkbox): " << bValue);
        }
    }

    return aRetval;
}

void ControlHelper::createUserPane()
{
    if (!m_bUserPaneNeeded) {
        SAL_INFO("fpicker.aqua","no user pane needed");
        return;
    }

    if (nil != m_pUserPane) {
        SAL_INFO("fpicker.aqua","user pane already exists");
        return;
    }

    if (m_bIsFilterControlNeeded && m_pFilterControl == nil) {
        createFilterControl();
    }

    NSRect minRect = NSMakeRect(0,0,300,33);
    m_pUserPane = [[NSView alloc] initWithFrame:minRect];

    int currentHeight = kAquaSpaceBoxFrameViewDiffTop + kAquaSpaceBoxFrameViewDiffBottom;
    int currentWidth = 300;

    bool bPopupControlPresent = false;
    bool bButtonControlPresent = false;

    int nCheckboxMaxWidth = 0;
    int nPopupMaxWidth = 0;
    int nPopupLabelMaxWidth = 0;

    for (::std::list<NSControl*>::iterator child = m_aActiveControls.begin(); child != m_aActiveControls.end(); child++) {
        SAL_INFO("fpicker.aqua","currentHeight: " << currentHeight);

        NSControl* pControl = *child;

        //let the control calculate its size
        [pControl sizeToFit];

        NSRect frame = [pControl frame];
        SAL_INFO("fpicker.aqua","frame for control " << [[pControl description] UTF8String] << " is {" << frame.origin.x << ", " << frame.origin.y << ", " << frame.size.width << ", " << frame.size.height << "}");

        int nControlHeight = frame.size.height;
        int nControlWidth = frame.size.width;

        // Note: controls are grouped by kind, first all popup menus, then checkboxes
        if ([pControl class] == [NSPopUpButton class]) {
            if (bPopupControlPresent) {
                //this is not the first popup
                currentHeight += kAquaSpaceBetweenPopupMenus;
            }
            else if (child != m_aActiveControls.begin()){
                currentHeight += kAquaSpaceBetweenControls;
            }

            bPopupControlPresent = true;

            // we have to add the label text width
            NSString *label = m_aMapListLabels[pControl];

            NSTextField *textField = createLabelWithString(label);
            [textField sizeToFit];
            m_aMapListLabelFields[(NSPopUpButton*)pControl] = textField;
            [m_pUserPane addSubview:textField];

            NSRect tfRect = [textField frame];
            SAL_INFO("fpicker.aqua","frame for textfield " << [[textField description] UTF8String] << " is {" << tfRect.origin.x << ", " << tfRect.origin.y << ", " << tfRect.size.width << ", " << tfRect.size.height << "}");

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

    SAL_INFO("fpicker.aqua","height after adding all controls: " << currentHeight);

    if (bPopupControlPresent && bButtonControlPresent)
    {
        //after a popup button (array) and before a different kind of control we need some extra space instead of the standard
        currentHeight -= kAquaSpaceBetweenControls;
        currentHeight += kAquaSpaceAfterPopupButtonsV;
        SAL_INFO("fpicker.aqua","popup extra space added, currentHeight: " << currentHeight);
    }

    int nLongestPopupWidth = nPopupMaxWidth + nPopupLabelMaxWidth + kAquaSpaceBetweenControls - kAquaSpacePopupMenuFrameBoundsDiffLeft - kAquaSpaceLabelFrameBoundsDiffH;

    currentWidth = nLongestPopupWidth > nCheckboxMaxWidth ? nLongestPopupWidth : nCheckboxMaxWidth;
    SAL_INFO("fpicker.aqua","longest control width: " << currentWidth);

    currentWidth += 2* kAquaSpaceInsideGroupH;

    if (currentWidth < minRect.size.width)
        currentWidth = minRect.size.width;

    if (currentHeight < minRect.size.height)
        currentHeight = minRect.size.height;

    NSRect upRect = NSMakeRect(0, 0, currentWidth, currentHeight );
    SAL_INFO("fpicker.aqua","setting user pane rect to {" << upRect.origin.x << ", " << upRect.origin.y << ", " << upRect.size.width << ", " << upRect.size.height << "}");

    [m_pUserPane setFrame:upRect];

    layoutControls();
}

#pragma mark Private / Misc

// Private / Misc

void ControlHelper::createControls()
{
    for (int i = 0; i < LIST_LAST; i++) {
        if (m_bListVisibility[i]) {
            m_bUserPaneNeeded = true;

            int elementName = getControlElementName([NSPopUpButton class], i);
            NSString* sLabel = CResourceProvider::getResString(elementName);

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
        if (m_bToggleVisibility[i]) {
            m_bUserPaneNeeded = true;

            int elementName = getControlElementName([NSButton class], i);
            NSString* sLabel = CResourceProvider::getResString(elementName);

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
}

#define TOGGLE_ELEMENT( elem ) \
case elem: \
    nReturn = CHECKBOX_##elem; \
    return nReturn
#define LIST_ELEMENT( elem ) \
case elem: \
    nReturn = LISTBOX_##elem##_LABEL; \
    return nReturn

int ControlHelper::getControlElementName(const Class aClazz, const int nControlId)
{
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

    return nReturn;
}

void ControlHelper::HandleSetListValue(const NSControl* pControl, const sal_Int16 nControlAction, const uno::Any& rValue)
{
    if ([pControl class] != [NSPopUpButton class]) {
        SAL_INFO("fpicker.aqua","not a popup menu");
        return;
    }

    NSPopUpButton *pButton = (NSPopUpButton*)pControl;
    NSMenu *rMenu = [pButton menu];
    if (nil == rMenu) {
        SAL_INFO("fpicker.aqua","button has no menu");
        return;
    }

    switch (nControlAction)
    {
        case ControlActions::ADD_ITEM:
        {
            SAL_INFO("fpicker.aqua","ADD_ITEMS");
            OUString sItem;
            rValue >>= sItem;

            NSString* sCFItem = [NSString stringWithOUString:sItem];
            SAL_INFO("fpicker.aqua","Adding menu item: " << sItem);
            [pButton addItemWithTitle:sCFItem];
        }
            break;
        case ControlActions::ADD_ITEMS:
        {
            SAL_INFO("fpicker.aqua","ADD_ITEMS");
            uno::Sequence< OUString > aStringList;
            rValue >>= aStringList;
            sal_Int32 nItemCount = aStringList.getLength();
            for (sal_Int32 i = 0; i < nItemCount; ++i)
            {
                NSString* sCFItem = [NSString stringWithOUString:aStringList[i]];
                SAL_INFO("fpicker.aqua","Adding menu item: " << aStringList[i]);
                [pButton addItemWithTitle:sCFItem];
            }
        }
            break;
        case ControlActions::DELETE_ITEM:
        {
            SAL_INFO("fpicker.aqua","DELETE_ITEM");
            sal_Int32 nPos = -1;
            rValue >>= nPos;
            SAL_INFO("fpicker.aqua","Deleting item at position " << (nPos));
            [rMenu removeItemAtIndex:nPos];
        }
            break;
        case ControlActions::DELETE_ITEMS:
        {
            SAL_INFO("fpicker.aqua","DELETE_ITEMS");
            int nItems = [rMenu numberOfItems];
            if (nItems == 0) {
                SAL_INFO("fpicker.aqua","no menu items to delete");
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
            SAL_INFO("fpicker.aqua","Selecting item at position " << nPos);
            [pButton selectItemAtIndex:nPos];
        }
            break;
        default:
            SAL_INFO("fpicker.aqua","undocumented/unimplemented ControlAction for a list");
            break;
    }

    layoutControls();
}

// cf. offapi/com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.idl
NSControl* ControlHelper::getControl( const sal_Int16 nControlId ) const
{
    NSControl* pWidget = nil;

#define MAP_TOGGLE( elem ) \
case ExtendedFilePickerElementIds::CHECKBOX_##elem: \
    pWidget = m_pToggles[elem]; \
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
            SAL_INFO("fpicker.aqua","Handle unknown control " << nControlId);
            break;
    }
#undef MAP

    return pWidget;
}

void ControlHelper::layoutControls()
{
    SolarMutexGuard aGuard;

    if (nil == m_pUserPane) {
        SAL_INFO("fpicker.aqua","no user pane to layout");
        return;
    }

    if (m_bIsUserPaneLaidOut) {
        SAL_INFO("fpicker.aqua","user pane already laid out");
        return;
    }

    NSRect userPaneRect = [m_pUserPane frame];
    SAL_INFO("fpicker.aqua","userPane frame: {" << userPaneRect.origin.x << ", " << userPaneRect.origin.y << ", " << userPaneRect.size.width << ", " << userPaneRect.size.height << "}");

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
    SAL_INFO("fpicker.aqua","longest popup width: " << nLongestPopupWidth);

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
        SAL_INFO("fpicker.aqua","vertical distance: " << nDistBetweenControls);
        currenttop -= nDistBetweenControls;

        previousControl = pControl;

        if (aSubType == [NSPopUpButton class]) {
            //move vertically up some pixels to space the controls between their real (visual) bounds
            currenttop += kAquaSpacePopupMenuFrameBoundsDiffTop;//from top

            //get the corresponding popup label
            NSTextField *label = m_aMapListLabelFields[(NSPopUpButton*)pControl];
            NSRect labelFrame = [label frame];
            int totalWidth = nPopupMaxWidth + labelFrame.size.width + kAquaSpaceBetweenControls - kAquaSpacePopupMenuFrameBoundsDiffLeft - kAquaSpaceLabelFrameBoundsDiffH;
            SAL_INFO("fpicker.aqua","totalWidth: " << totalWidth);
            //let's center popups
            int left = (nUsableWidth + nLongestPopupWidth) / 2 - totalWidth;
            SAL_INFO("fpicker.aqua","left: " << left);
            labelFrame.origin.x = left;
            labelFrame.origin.y = currenttop + kAquaSpaceLabelPopupDiffV;
            SAL_INFO("fpicker.aqua","setting label at: {" << labelFrame.origin.x << ", " << labelFrame.origin.y << ", " << labelFrame.size.width << ", " << labelFrame.size.height << "}");
            [label setFrame:labelFrame];

            controlRect.origin.x = left + labelFrame.size.width + kAquaSpaceBetweenControls - kAquaSpaceLabelFrameBoundsDiffH - kAquaSpacePopupMenuFrameBoundsDiffLeft;
            controlRect.origin.y = currenttop;
            controlRect.size.width = nPopupMaxWidth;
            SAL_INFO("fpicker.aqua","setting popup at: {" << controlRect.origin.x << ", " << controlRect.origin.y << ", " << controlRect.size.width << ", " << controlRect.size.height << "}");
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
            SAL_INFO("fpicker.aqua","setting checkbox at: {" << controlRect.origin.x << ", " << controlRect.origin.y << ", " << controlRect.size.width << ", " << controlRect.size.height << "}");

            currenttop += kAquaSpaceSwitchButtonFrameBoundsDiff;
        }
    }

    m_bIsUserPaneLaidOut = true;
}

void ControlHelper::createFilterControl()
{
    NSString* sLabel = CResourceProvider::getResString(CommonFilePickerElementIds::LISTBOX_FILTER_LABEL);

    m_pFilterControl = [NSPopUpButton new];

    [m_pFilterControl setAction:@selector(filterSelectedAtIndex:)];
    [m_pFilterControl setTarget:m_pDelegate];

    NSMenu *menu = [m_pFilterControl menu];

    for (NSStringList::iterator iter = m_pFilterHelper->getFilterNames()->begin(); iter != m_pFilterHelper->getFilterNames()->end(); ++iter) {
        NSString *filterName = *iter;
        SAL_INFO("fpicker.aqua","adding filter name: " << [filterName UTF8String]);
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
    if (!m_bIsFilterControlNeeded || m_pFilterHelper == nullptr) {
        SAL_INFO("fpicker.aqua","no filter control needed or no filter helper present");
        return;
    }

    int index = m_pFilterHelper->getCurrentFilterIndex();

    if (m_pFilterControl == nil) {
        createFilterControl();
    }

    [m_pFilterControl selectItemAtIndex:index];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
