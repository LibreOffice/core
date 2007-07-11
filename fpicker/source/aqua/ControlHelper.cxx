/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ControlHelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-07-11 10:57:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef  _COM_SUN_STAR_UI_DIALOGS_EXTENDEDFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_CONTROLACTIONS_HPP_
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_TEMPLATEDESCRIPTION_HPP_
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CONTROLHELPER_HXX_
#include "ControlHelper.hxx"
#endif
#ifndef _RESOURCEPROVIDER_HXX_
#include "resourceprovider.hxx"
#endif

#pragma mark DEFINES
#define CLASS_NAME "ControlHelper"

using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::ui::dialogs::TemplateDescription;
using namespace ::com::sun::star::ui::dialogs::ExtendedFilePickerElementIds;
using namespace ::rtl;

#pragma mark Constructor / Destructor
//------------------------------------------------------------------------------------
// Constructor / Destructor
//------------------------------------------------------------------------------------
ControlHelper::ControlHelper()
: m_nLastCustomizeTryWidth(0)
, m_nLastCustomizeTryHeight(0)
, m_bUserPaneNeeded( false )
, m_bIsFilterPopupPresent(false)
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    int i;

    for( i = 0; i < TOGGLE_LAST; i++ ) {
        m_bToggleVisibility[i] = false;
    }

    //  for( i = 0; i < BUTTON_LAST; i++ )
    //      m_pButtons[i] = NULL;

    for( i = 0; i < LIST_LAST; i++ ) {
        m_bListVisibility[i] = false;
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

ControlHelper::~ControlHelper()
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    if (NULL != m_pUserPane) {
        DisposeControl(m_pUserPane);
    }

    for(std::list<ControlRef>::iterator control = m_aActiveControls.begin(); control != m_aActiveControls.end(); control++) {
        ControlRef pControl = (*control);
        ControlActionUPP pAction = GetControlAction(pControl);
        if (NULL != pAction) {
            DisposeControlActionUPP(pAction);
        }
        DisposeControl(*control);
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

#pragma mark XInitialization delegate
//------------------------------------------------
// XInitialization delegate
//------------------------------------------------
void ControlHelper::initialize(sal_Int16 nTemplateId)
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

void ControlHelper::enableControl(sal_Int16 nControlId, sal_Bool bEnable)
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "controlId", nControlId, "enable", bEnable);

    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    ControlRef pControl;

    if( ( pControl = getControl(nControlId)) ) {
        if( bEnable ) {
            OSL_TRACE( "enable" );
        } else {
            OSL_TRACE( "disable" );
        }
        HIViewSetEnabled(pControl, bEnable);
    } else {
        OSL_TRACE("enable unknown control %d", nControlId );
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

OUString ControlHelper::getLabel(sal_Int16 nControlId)
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "controlId", nControlId);

    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    ControlRef pControl;

    if( !( pControl = getControl( nControlId ) ) ) {
        OSL_TRACE("Get label on unknown control %d", nControlId);
        return OUString();
    }

    CFStringRef sLabel;
    CopyControlTitleAsCFString(pControl, &sLabel);
    rtl::OUString retVal = CFStringToOUString(sLabel);
    CFRelease(sLabel);

    DBG_PRINT_EXIT(CLASS_NAME, __func__, retVal);

    return retVal;
}

void ControlHelper::setLabel( sal_Int16 nControlId, const CFStringRef aLabel )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "controlId", nControlId, "label", aLabel);

    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    ControlRef pControl = getControl(nControlId);
    if (NULL != pControl) {
        ControlType aSubType;
        GetControlProperty(pControl,kAppFourCharCode,kControlPropertySubType,sizeof(aSubType),NULL,&aSubType);
        if (aSubType == POPUPMENU) {
            MenuRef rMenu = GetControlPopupMenuHandle(pControl);
            //for the sizes to be calculated correctly, a menu item must be present in the menu
            CFStringRef dummy = CFSTR("hi");
            AppendMenuItemTextWithCFString(rMenu, dummy, 0, 0, 0);
            //dummy is no longer needed
            CFRelease(dummy);

            SetControlTitleWithCFString(pControl, NULL);

            Rect sizeWoTitle;
            GetBestControlRect(pControl,&sizeWoTitle,NULL);
            //...and set the title here
            SetControlTitleWithCFString(pControl,aLabel);
            Rect sizeWithTitle;
            GetBestControlRect(pControl,&sizeWithTitle,NULL);

            //now we no longer need the dummy menu item
            DeleteMenuItem(rMenu, 1);

            //get the title's size
            int nTitleWidth = sizeWithTitle.right - sizeWoTitle.right;
            SetControlProperty(pControl,kAppFourCharCode,kPopupControlPropertyTitleWidth,sizeof(nTitleWidth),&nTitleWidth);

        } else if (aSubType == CHECKBOX) {
            SetControlTitleWithCFString(pControl,aLabel);
        }
    } else {
        OSL_TRACE("Control not found to set label for");
    }

    layoutControls();

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void ControlHelper::setValue( sal_Int16 nControlId, sal_Int16 nControlAction, const uno::Any& rValue )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "controlId", nControlId, "controlAction", nControlAction);

    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    ControlRef pControl;

    if( !( pControl = getControl( nControlId ) ) ) {
        OSL_TRACE("enable unknown control %d", nControlId);
    } else {
        ControlType aSubType;
        GetControlProperty(pControl,kAppFourCharCode,kControlPropertySubType,sizeof(aSubType),NULL,&aSubType);
        if( aSubType == CHECKBOX ) {
            sal_Bool bChecked = false;
            rValue >>= bChecked;
            OSL_TRACE(" value is a bool: %d", bChecked);
            SetControlValue(pControl, bChecked);
        } else if( aSubType == POPUPMENU ) {
            HandleSetListValue(pControl, nControlAction, rValue);
        } else
        {
            OSL_TRACE("Can't set value on button / list %d %d",
                      nControlId, nControlAction);
        }
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

uno::Any ControlHelper::getValue(sal_Int16 nControlId, sal_Int16 nControlAction)
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "controlId", nControlId, "controlAction", nControlAction);

    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    uno::Any aRetval;

    ControlRef pControl;

    if( !( pControl = getControl( nControlId ) ) ) {
        OSL_TRACE("get value for unknown control %d", nControlId);
    } else {
        ControlType aSubType;
        GetControlProperty(pControl,kAppFourCharCode,kControlPropertySubType,sizeof(aSubType),NULL,&aSubType);
        if( aSubType == CHECKBOX ) {
            sal_Bool bValue = (sal_Bool)GetControlValue(pControl);
            aRetval <<= bValue;
            OSL_TRACE("value is a bool: %d", bValue);
        } else if( aSubType == POPUPMENU ) {
            aRetval = HandleGetListValue(pControl, nControlAction);
        }
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);

    return aRetval;
}

#pragma mark NavigationServices callback delegates
//------------------------------------------------------------------------------------
// NavigationServices callback delegates
//------------------------------------------------------------------------------------
void ControlHelper::handleStart(NavCBRecPtr callBackParms)
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    ::vos::OGuard aGuard(Application::GetSolarMutex());

    OSStatus status = noErr;

    if (NULL != m_pUserPane) {
        layoutControls();
        for (::std::list<ControlRef>::iterator child = m_aActiveControls.begin(); child != m_aActiveControls.end(); child++) {
            ControlRef pControl = *child;
            EmbedControl(pControl, m_pUserPane);
        }

        status = NavCustomControl(callBackParms->context,kNavCtlAddControl,&m_pUserPane);
        if (status != noErr) {
            OSL_TRACE("NavigationServices refused to add the user pane");
        }
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void ControlHelper::handleCustomize(NavCBRecPtr callBackParms)
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    ::vos::OGuard aGuard(Application::GetSolarMutex());

    if (true == m_bUserPaneNeeded && NULL == m_pUserPane) {
        createUserPane(callBackParms->window);
    }

    if (NULL == m_pUserPane) {
        //nothing to do
        return;
    }

    if (callBackParms->customRect.right == 0 && callBackParms->customRect.bottom == 0) {
        //this is the first try, let's try normal sized controls
        Rect normalSize;// = m_pUserPane->getSize(SalAquaControlRefWrapper::NORMAL_SIZE);
        GetBestControlRect(m_pUserPane,&normalSize,NULL);
        m_nLastCustomizeTryWidth = normalSize.right - normalSize.left;
        m_nLastCustomizeTryHeight = normalSize.bottom - normalSize.top;
        callBackParms->customRect.right = callBackParms->customRect.left + m_nLastCustomizeTryWidth;
        callBackParms->customRect.bottom = callBackParms->customRect.top + m_nLastCustomizeTryHeight;
    } else {
        int nSuggestedWidth = callBackParms->customRect.right - callBackParms->customRect.left;
        int nSuggestedHeight = callBackParms->customRect.bottom - callBackParms->customRect.top;
        if (nSuggestedWidth >= m_nLastCustomizeTryWidth && nSuggestedHeight >= m_nLastCustomizeTryHeight) {
            //everything was accepted
            return;
        } else {
            //TODO implement small and mini behavior
            //this can be done in a separate child workspace sometime
        }
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void ControlHelper::handleEvent(NavCBRecPtr callBackParms, NavEventCallbackMessage aLatestEvent)
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "latestEvent", aLatestEvent);

    ::vos::OGuard aGuard(Application::GetSolarMutex());

    NavEventData data = callBackParms->eventData;
    NavEventDataInfo info = data.eventDataParms;
    EventRecord *event = info.event;
    EventKind aEventKind = event->what;
    EventModifiers modifiers = event->modifiers;

    if (aEventKind == updateEvt) {
        OSL_TRACE("UPDATE event");
        // the following is a little hacky because normally Apple says in her
        // docs that we don't have to care about drawing our custom controls
        // once they are added to the dialog. But this is definitely not true
        // because without this hack controls would suddenly disappear if one
        // control in the user pane area gets clicked.
        bool display;
        ControlRef pControl = NULL;

        for (::std::list<ControlRef>::iterator child = m_aActiveControls.begin(); child != m_aActiveControls.end(); child++) {
            pControl = *child;
            if (NULL == pControl)
                continue;
            display = HIViewGetNeedsDisplay(pControl) || (aLatestEvent == kNavCBAdjustRect);
            if (!display) {
                OSL_TRACE("REFRESHING CONTROL MYSELF");
                Draw1Control(pControl);
            }
        }

        return;
    }

    if (aEventKind != mouseDown) {
        return;
    }

    Rect globWinBounds;
    GetWindowBounds(callBackParms->window,kWindowContentRgn,&globWinBounds);

    MacOSPoint mousePt;
    GetMouse(&mousePt);
    SInt16 partResult;
    ControlRef pControl = FindControlUnderMouse(mousePt,callBackParms->window,&partResult);

    if (!IsControlEnabled(pControl))
        return;
    //    OSL_TRACE("partResult: %d", partResult);
    //
    ControlType nSubType = NONE;
    GetControlProperty(pControl,kAppFourCharCode,kControlPropertySubType,sizeof(ControlType),NULL,&nSubType);

    if (NONE == nSubType) {
        //do nothing
        return;
    }

    if (nSubType == CHECKBOX) {
        SInt16 oldValue = GetControlValue(pControl);
        bool tracking = true;
        SetControlProperty(pControl,kAppFourCharCode,kControlPropertyTracking, sizeof(bool),&tracking);
        ControlPartCode lastCode = -1;
        SetControlProperty(pControl, kAppFourCharCode,kControlPropertyLastPartCode, sizeof(ControlPartCode), &lastCode);
        //ControlPartCode part = TrackControl(pControl,mousePt,(ControlActionUPP)-1);
        ControlPartCode part = HandleControlClick(pControl,mousePt,modifiers,(ControlActionUPP)-1);
        SetControlProperty(pControl, kAppFourCharCode,kControlPropertyLastPartCode, sizeof(ControlPartCode), &lastCode);
        tracking = false;
        SetControlProperty(pControl,kAppFourCharCode,kControlPropertyTracking, sizeof(bool),&tracking);
        if (part == 0) {
            SInt16 newValue = GetControlValue(pControl);
            if (newValue == oldValue) {
                //hit control handler was not called
                MacOSPoint mouseUpPt;
                GetMouse(&mouseUpPt);
                if (mouseUpPt.h == mousePt.h && mouseUpPt.v == mousePt.v) {
                    SetControlValue(pControl,!newValue);
                }
            }
        } else {
            SetControlValue(pControl,1 - GetControlValue(pControl));
        }
    } else if (nSubType == POPUPMENU) {
        Rect localBounds;
        GetControlBounds(pControl,&localBounds);
        Rect upBounds;
        GetControlBounds(m_pUserPane,&upBounds);
        localBounds.left += upBounds.left;
        localBounds.top += upBounds.top;
        OSL_TRACE("  localBounds t:%d l:%d b:%d r:%d", localBounds.top, localBounds.left, localBounds.bottom, localBounds.right);

        int nTitleWidth;
        GetControlProperty(pControl, kAppFourCharCode, kPopupControlPropertyTitleWidth,sizeof(nTitleWidth),NULL,&nTitleWidth);
        //if we are hit in the popup's title, then do nothing
        if (localBounds.left + nTitleWidth > mousePt.h) {
            return;
        }

        //we are hit in the menu, so display the menu
        MenuRef pMenu = GetControlPopupMenuHandle(pControl);
        SInt16 nSelectedItem = GetControlValue(pControl);
        CheckMenuItem(pMenu,nSelectedItem,TRUE);

        int menuLeftPoint = globWinBounds.left + localBounds.left + nTitleWidth - 22 + 9;
        int menuTopPoint = localBounds.top + globWinBounds.top + 1;

        OSL_TRACE("  menuLeft: %d", menuLeftPoint);

        long index = PopUpMenuSelect(pMenu, menuTopPoint, menuLeftPoint, GetControlValue(pControl));
        short high = (index & 0xFF00) >> 16;
        short low = (index & 0x00FF);
        OSL_TRACE(" item selected: %d %d", high, low);
        if (low != 0) {
            if (low != nSelectedItem)
                CheckMenuItem(pMenu,nSelectedItem,FALSE);
            SetControlValue(pControl,low);
            OSL_TRACE(" popup menu value: %d", GetControlValue(pControl));
        }
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void ControlHelper::handleAdjustRect(NavCBRecPtr callBackParms)
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    if (NULL != m_pUserPane) {
        SetControlBounds(m_pUserPane,&(callBackParms->customRect));
    }
    layoutControls();

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void ControlHelper::createUserPane(WindowRef parent)
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    if (NULL != m_pUserPane) {
        return;
    }

    OSStatus status = noErr;

    int currenttop = 0;
    //NavServices will never be smaller
    int maxWidth = kAquaNavigationServicesMinWidth;

    for (::std::list<ControlRef>::iterator child = m_aActiveControls.begin(); child != m_aActiveControls.end(); child++) {
        ControlRef pControl = *child;

        Rect bestSize;
        status = GetBestControlRect(pControl,&bestSize,NULL);
        int nControlHeight = bestSize.bottom - bestSize.top;
        int nControlWidth = bestSize.right - bestSize.left;
        if ((nControlWidth + 2 * kAquaSpaceInsideGroup) > maxWidth) {
            maxWidth = nControlWidth;
        }

        if (pControl == *(m_aActiveControls.begin())) {
            //first control
            if (true == m_bIsFilterPopupPresent) {
                //in this case we already have a control in the user pane: the file formats popup control
                // and the user pane is already spaced correctly
                //currenttop += kAquaSpaceBetweenControls;
            } else {
                currenttop += kAquaSpaceInsideGroup;//from top
            }
        }
        else {
            //we already have a control
            currenttop += kAquaSpaceBetweenControls;//in-between space before
        }

        //set the top value
        bestSize.top = currenttop;
        bestSize.bottom = currenttop + nControlHeight;
        bestSize.left = kAquaSpaceInsideGroup;
        bestSize.right = kAquaSpaceInsideGroup + nControlWidth;

        currenttop += nControlHeight;
    }
    //from bottom
    currenttop += kAquaSpaceInsideGroup;

    //ok, we have all controls
    Rect upRect = { 0, 0, currenttop, maxWidth };
    status = CreateUserPaneControl(parent,&upRect,0x16,&m_pUserPane);

    layoutControls();

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

#pragma mark Private / Misc
//------------------------------------------------------------------------------------
// Private / Misc
//------------------------------------------------------------------------------------
void SalAquaCheckboxAction(ControlRef pControl, ControlPartCode partCode)
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "partCode", partCode);

    ControlPartCode lastCode;
    GetControlProperty(pControl,kAppFourCharCode,kControlPropertyLastPartCode,sizeof(ControlPartCode),NULL,&lastCode);

    bool tracking;
    GetControlProperty(pControl,kAppFourCharCode,kControlPropertyTracking,sizeof(bool),NULL, &tracking);
    if (true == tracking) {
        if (lastCode == -1) {
            //first run
            HiliteControl(pControl, kControlCheckBoxPart);
        }
        lastCode = partCode;
        SetControlProperty(pControl,kAppFourCharCode,kControlPropertyLastPartCode,sizeof(ControlPartCode),&lastCode);
    } else {
        //keep initial state
        if (partCode != 0)
            SetControlValue(pControl, !GetControlValue(pControl));
        lastCode = -1;
        SetControlProperty(pControl, kAppFourCharCode, kControlPropertyLastPartCode, sizeof(ControlPartCode), &lastCode);
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void ControlHelper::createControls()
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    OSStatus status = noErr;
    CResourceProvider aResProvider;
    Rect dummyRect = { 0, 0, 0, 0 };

#define MENUOFFSET 1024
    for (int i = 0; i < LIST_LAST; i++) {
        if (true == m_bListVisibility[i]) {
            m_bUserPaneNeeded = true;

            int elementName = getControlElementName(POPUPMENU, i);
            CFStringRef sLabel = aResProvider.getResString(elementName);

            MenuRef rMenu;
            int menuId = MENUOFFSET + i;
            status = CreateNewMenu(menuId, 0, &rMenu);
            InsertMenu(rMenu, -1);

            //we create the popup menu control without title
            status = CreatePopupButtonControl(NULL,&dummyRect,NULL, menuId, TRUE, -1,teFlushDefault,0,&m_pListControls[i]);

            ControlType aSubType = POPUPMENU;
            SetControlProperty(m_pListControls[i],kAppFourCharCode,kControlPropertySubType,sizeof(aSubType),&aSubType);

#define MAP_LIST_( elem ) \
case elem: \
    setLabel(ExtendedFilePickerElementIds::LISTBOX_##elem, sLabel); \
    break

            switch(i) {
                MAP_LIST_(VERSION);
                MAP_LIST_(TEMPLATE);
                MAP_LIST_(IMAGE_TEMPLATE);
            }
            CFRelease(sLabel);

            m_aActiveControls.push_back(m_pListControls[i]);

            Rect size;
            GetBestControlRect(m_pListControls[i],&size,NULL);
            if (size.right < kAquaNavigationServicesPopupControlWidth) {
                size.right = kAquaNavigationServicesPopupControlWidth;
                SetControlBounds(m_pListControls[i], &size);
            }
            //we don't have any menu items in the beginning, so let's disable the control
            //HIViewSetEnabled(m_pListControls[i],false);
        } else {
            m_pListControls[i] = NULL;
        }
    }

    for (int i = 1; i < TOGGLE_LAST; i++) {
        if (true == m_bToggleVisibility[i]) {
            m_bUserPaneNeeded = true;

            int elementName = getControlElementName(CHECKBOX, i);
            CFStringRef sLabel = aResProvider.getResString(elementName);

            status = CreateCheckBoxControl(NULL, &dummyRect, sLabel, FALSE, FALSE, &m_pToggles[i]);
            CFRelease(sLabel);
            m_aActiveControls.push_back(m_pToggles[i]);

            ControlType aSubType = CHECKBOX;
            SetControlProperty(m_pToggles[i],kAppFourCharCode,kControlPropertySubType,sizeof(aSubType),&aSubType);

            ControlActionUPP pControlAction = NewControlActionUPP(SalAquaCheckboxAction);
            SetControlAction(m_pToggles[i], pControlAction);
        } else {
            m_pToggles[i] = NULL;
        }
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

int ControlHelper::getControlElementName(ControlType type, int nControlId)
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "type", type, "controlId", nControlId);

    int nReturn = -1;
    switch (type) {
        case CHECKBOX:
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
            break;
        }
        case POPUPMENU:
        {
            switch (nControlId) {
                LIST_ELEMENT( VERSION );
                LIST_ELEMENT( TEMPLATE );
                LIST_ELEMENT( IMAGE_TEMPLATE );
            }
            break;
        }
        case NONE:
            break;
        default:
            break;
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__, nReturn);

    return nReturn;
}

void ControlHelper::HandleSetListValue(const ControlRef pControl, const sal_Int16 nControlAction, const uno::Any& rValue)
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "controlAction", nControlAction);

    MenuRef rMenu = GetControlPopupMenuHandle(pControl);
    if (NULL == rMenu) {
        return;
    }

    switch (nControlAction)
    {
        case ControlActions::ADD_ITEM:
        {
            OSL_TRACE("ADD_ITEMS");
            OUString sItem;
            rValue >>= sItem;

            int nMenuItems = CountMenuItems(rMenu);
            if (nMenuItems == 0) {
                //HIViewSetEnabled(pControl, true);
            }

            CFStringRef sCFItem = CFStringCreateWithOUString(sItem);
            OSL_TRACE("Adding menu item: %s", OUStringToOString(sItem, RTL_TEXTENCODING_UTF8).getStr());
            AppendMenuItemTextWithCFString(rMenu,sCFItem,0,0,NULL);
            SetControlMaximum(pControl,CountMenuItems(rMenu));
            CFRelease(sCFItem);
            //gtk_combo_box_append_text(pWidget, rtl::OUStringToOString(sItem, RTL_TEXTENCODING_UTF8).getStr());
            //                 if (!bVersionWidthUnset)
            //                 {
            //                     //                    HackWidthToFirst(pWidget);
            //                     bVersionWidthUnset = true;
            //                 }
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
                int nMenuItems = CountMenuItems(rMenu);
                if (nMenuItems == 0) {
                    //HIViewSetEnabled(pControl, true);
                }
                CFStringRef sCFItem = CFStringCreateWithOUString(aStringList[i]);
                OSL_TRACE("Adding menu item: %s", OUStringToOString(aStringList[i], RTL_TEXTENCODING_UTF8).getStr());
                AppendMenuItemTextWithCFString(rMenu,sCFItem,0,0,NULL);
                CFRelease(sCFItem);
            }
            SetControlMaximum(pControl,CountMenuItems(rMenu));
        }
            break;
        case ControlActions::DELETE_ITEM:
        {
            OSL_TRACE("DELETE_ITEM");
            sal_Int32 nPos = -1;
            rValue >>= nPos;
            //in a MenuRef the first element has position 1
            OSL_TRACE("Deleteing item at position %d", (nPos + 1));
            DeleteMenuItem(rMenu,nPos + 1);
            int nMenuItems = CountMenuItems(rMenu);
            SetControlMaximum(pControl, nMenuItems);
            if (nMenuItems == 0) {
                //HIViewSetEnabled(pControl, false);
            }
        }
            break;
        case ControlActions::DELETE_ITEMS:
        {
            OSL_TRACE("DELETE_ITEMS");
            int nItems = CountMenuItems(rMenu);
            if (nItems == 0)
                return;
            DeleteMenuItems(rMenu,1, nItems);
            int nMenuItems = CountMenuItems(rMenu);
            SetControlMaximum(pControl, nMenuItems);
            if (nMenuItems == 0) {
                //HIViewSetEnabled(pControl, false);
            }
        }
            break;
        case ControlActions::SET_SELECT_ITEM:
        {
            sal_Int32 nPos = -1;
            rValue >>= nPos;
            OSL_TRACE("Selecting item at position %d", (nPos + 1));
            //in a MenuRef the first element has position 1
            SetControlValue(pControl, nPos + 1);
        }
            break;
        default:
            OSL_TRACE("undocumented/unimplemented ControlAction for a list");
            break;
    }

    layoutControls();

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}


uno::Any ControlHelper::HandleGetListValue(const ControlRef pControl, const sal_Int16 nControlAction) const
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "controlAction", nControlAction);

    uno::Any aAny;

    MenuRef rMenu = GetControlPopupMenuHandle(pControl);
    if (NULL == rMenu) {
        return aAny;
    }

    switch (nControlAction)
    {
        case ControlActions::GET_ITEMS:
        {
            OSL_TRACE("GET_ITEMS");
            uno::Sequence< OUString > aItemList;

            int nItems = CountMenuItems(rMenu);
            if (nItems > 0) {
                aItemList.realloc(nItems);
            }
            for (int i = 1; i <= nItems; i++) {
                CFStringRef sCFItem;
                CopyMenuItemTextAsCFString(rMenu, i, &sCFItem);
                if (NULL != sCFItem) {
                    aItemList[i - 1] = CFStringToOUString(sCFItem);
                    OSL_TRACE("Return value[%d]: %s", (i - 1), OUStringToOString(aItemList[i - 1], RTL_TEXTENCODING_UTF8).getStr());
                    CFRelease(sCFItem);
                }
            }

            aAny <<= aItemList;
        }
            break;
        case ControlActions::GET_SELECTED_ITEM:
        {
            OSL_TRACE("GET_SELECTED_ITEM");
            int nIndex = GetControlValue(pControl);
            CFStringRef sCFItem;
            CopyMenuItemTextAsCFString(rMenu, nIndex, &sCFItem);
            if (NULL != sCFItem) {
                OUString sString = CFStringToOUString(sCFItem);
                OSL_TRACE("Return value: %s", OUStringToOString(sString, RTL_TEXTENCODING_UTF8).getStr());
                aAny <<= sString;
                CFRelease(sCFItem);
            }
        }
            break;
        case ControlActions::GET_SELECTED_ITEM_INDEX:
        {
            OSL_TRACE("GET_SELECTED_ITEM_INDEX");
            //menu indexes start at 1, so we have to subtract 1
            int nActive = GetControlValue(pControl) - 1;
            OSL_TRACE("Return value: %d", nActive);
            aAny <<= static_cast< sal_Int32 >(nActive);
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
ControlRef ControlHelper::getControl( const sal_Int16 nControlId)
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "controlId", nControlId);

    ControlRef pWidget = NULL;

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
        case AUTOEXTENSION:
        {
            //handle differently on MacOSX
            break;
        }
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

    if (NULL == m_pUserPane) {
        DBG_PRINT_EXIT(CLASS_NAME, __func__);
        return;
    }

    OSStatus status = noErr;
    Rect userPaneRect;
    GetControlBounds(m_pUserPane,&userPaneRect);
    int nUsableWidth = userPaneRect.right - userPaneRect.left - 2 * kAquaSpaceInsideGroup;

    int currenttop = 0;
    int nCheckboxMaxWidth = 0;

    for (::std::list<ControlRef>::iterator child = m_aActiveControls.begin(); child != m_aActiveControls.end(); child++) {
        ControlRef pControl = *child;

        Rect bestSize;
        status = GetBestControlRect(pControl,&bestSize,NULL);
        int nControlHeight = bestSize.bottom - bestSize.top;
        int nControlWidth = bestSize.right - bestSize.left;

        if (pControl == *(m_aActiveControls.begin())) {
            //first control
            if (true == m_bIsFilterPopupPresent) {
                //in this case we already have a control in the user pane: the file formats popup control
                // and the user pane is already spaced correctly
                //currenttop += kAquaSpaceBetweenControls;
            } else {
                currenttop += kAquaSpaceInsideGroup;//from top
            }
        }
        else {
            //we already have a control
            currenttop += kAquaSpaceBetweenControls;//in-between space before
        }

        ControlType aSubType;
        GetControlProperty(pControl,kAppFourCharCode,kControlPropertySubType,sizeof(aSubType),NULL,&aSubType);
        if (aSubType == POPUPMENU) {
            int nTitleWidth;
            GetControlProperty(pControl,kAppFourCharCode,kPopupControlPropertyTitleWidth,sizeof(nTitleWidth),NULL,&nTitleWidth);
            if (nControlWidth < kAquaNavigationServicesPopupControlWidth + nTitleWidth)
                nControlWidth = kAquaNavigationServicesPopupControlWidth + nTitleWidth;
            //let's center popups
            bestSize.left = kAquaSpaceInsideGroup + (nUsableWidth - nControlWidth) / 2;
            bestSize.right = bestSize.left + nControlWidth;
        } else {
            //for checkboxes first determine max. width
            if (nCheckboxMaxWidth < nControlWidth) {
                nCheckboxMaxWidth = nControlWidth;
            }
        }

        //set the top value
        bestSize.top = currenttop;
        bestSize.bottom = currenttop + nControlHeight;
        SetControlBounds(pControl, &bestSize);

        currenttop += nControlHeight;
    }

    int nCheckboxLeft = kAquaSpaceInsideGroup + (nUsableWidth - nCheckboxMaxWidth) / 2;
    //second run to set checkbox dimensions
    for (::std::list<ControlRef>::iterator child = m_aActiveControls.begin(); child != m_aActiveControls.end(); child++) {
        ControlRef pControl = *child;

        ControlType aSubType;
        GetControlProperty(pControl,kAppFourCharCode,kControlPropertySubType,sizeof(aSubType),NULL,&aSubType);
        if (aSubType != CHECKBOX) {
            //nothing to do for other types
            continue;
        }

        Rect bestSize;
        status = GetBestControlRect(pControl,&bestSize,NULL);
        int nControlWidth = bestSize.right - bestSize.left;

        bestSize.left = nCheckboxLeft;
        bestSize.right = bestSize.left + nControlWidth;
        SetControlBounds(pControl, &bestSize);
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}
