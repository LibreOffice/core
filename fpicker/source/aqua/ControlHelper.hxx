/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ControlHelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-07-11 10:57:54 $
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

#ifndef _CONTROLHELPER_HXX_
#define _CONTROLHELPER_HXX_

#ifndef _RTL_USTRING_H_
#include <rtl/ustring.hxx>
#endif
#ifndef  _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#include <list>

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

#ifndef _SALAQUACONSTANTS_H_
#include "SalAquaConstants.h"
#endif

using namespace com::sun::star;
using namespace rtl;

class ControlHelper {

public:

    //------------------------------------------------------------------------------------
    // Constructor / Destructor
    //------------------------------------------------------------------------------------
    ControlHelper();
    virtual ~ControlHelper();

    //------------------------------------------------
    // XInitialization delegate
    //------------------------------------------------
    void       initialize(sal_Int16 templateId);

    //------------------------------------------------------------------------------------
    // XFilePickerControlAccess function delegates
    //------------------------------------------------------------------------------------
    void       setValue( sal_Int16 nControlId, sal_Int16 nControlAction, const uno::Any& rValue );
    uno::Any   getValue(sal_Int16 nControlId, sal_Int16 nControlAction);
    void       enableControl(sal_Int16 nControlId, sal_Bool bEnable);
    OUString   getLabel(sal_Int16 nControlId);
    void       setLabel( sal_Int16 nControlId, const CFStringRef aLabel );

    //------------------------------------------------------------------------------------
    // NavigationServices callback delegates
    //------------------------------------------------------------------------------------
    void       handleStart(NavCBRecPtr callBackParms);
    void       handleCustomize(NavCBRecPtr callBackParms);
    void       handleEvent(NavCBRecPtr callBackParms, NavEventCallbackMessage aLatestEvent);
    void       handleAdjustRect(NavCBRecPtr callBackParms);

    //------------------------------------------------------------------------------------
    // Type definitions
    //------------------------------------------------------------------------------------
    enum ControlType {
        NONE
        , CHECKBOX
        , POPUPMENU
        , USERPANE
    };

    enum ToggleType {
        AUTOEXTENSION, //but autoextension is handled differently on MacOSX
        PASSWORD,
        FILTEROPTIONS,
        READONLY,
        LINK,
        PREVIEW,
        SELECTION,
        TOGGLE_LAST
    };

    enum ListType {
        VERSION,
        TEMPLATE,
        IMAGE_TEMPLATE,
        LIST_LAST
    };

    //------------------------------------------------------------------------------------
    // inline functions
    //------------------------------------------------------------------------------------
    inline ControlRef getUserPane() {
        return m_pUserPane;
    }

    inline bool       getVisibility(ToggleType tToggle) {
        return m_bToggleVisibility[tToggle];
    }

    inline void       setPopupPresent(bool bPresent) {
        m_bIsFilterPopupPresent = bPresent;
    }

private:
    //------------------------------------------------------------------------------------
    // private member variables
    //------------------------------------------------------------------------------------
    ControlRef    m_pUserPane;

    int m_nLastCustomizeTryWidth;
    int m_nLastCustomizeTryHeight;

    ControlRef m_pToggles[ TOGGLE_LAST ];
    bool m_bToggleVisibility[TOGGLE_LAST];

//    enum tButtonType {
//        PLAY,
//        BUTTON_LAST
//    };
//
//    ControlRef m_pButtons[ BUTTON_LAST ];

    ControlRef m_pListControls[ LIST_LAST ];
    bool m_bListVisibility[ LIST_LAST ];

    bool m_bUserPaneNeeded;

    ::std::list<ControlRef> m_aActiveControls;

    bool m_bIsFilterPopupPresent;

    //------------------------------------------------------------------------------------
    // private methods
    //------------------------------------------------------------------------------------
    void HandleSetListValue(const ControlRef pControl, const sal_Int16 nControlAction, const uno::Any& rValue);
    uno::Any HandleGetListValue(const ControlRef pControl, const sal_Int16 nControlAction) const;

    void       createControls();
    void       createUserPane(WindowRef parent);
    int        getControlElementName(ControlType type, int nControlId);
    ControlRef getControl( const sal_Int16 nControlId );

    void layoutControls();
};

#endif //_CONTROLHELPER_HXX_
