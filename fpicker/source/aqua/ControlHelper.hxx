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

#ifndef _CONTROLHELPER_HXX_
#define _CONTROLHELPER_HXX_

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Any.hxx>

#include <list>
#include <map>

#include <premac.h>
#include <Cocoa/Cocoa.h>
#include <postmac.h>
#include "SalAquaConstants.h"
#include "FilterHelper.hxx"
#include "AquaFilePickerDelegate.hxx"

using namespace com::sun::star;

using ::rtl::OUString;

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
    void       initialize( sal_Int16 templateId );

    //------------------------------------------------------------------------------------
    // XFilePickerControlAccess function delegates
    //------------------------------------------------------------------------------------
    void       setValue( sal_Int16 nControlId, sal_Int16 nControlAction, const uno::Any& rValue );
    uno::Any   getValue( sal_Int16 nControlId, sal_Int16 nControlAction ) const;
    void       enableControl( sal_Int16 nControlId, sal_Bool bEnable ) const;
    OUString   getLabel( sal_Int16 nControlId );
    void       setLabel( sal_Int16 nControlId, const NSString* aLabel );

    //------------------------------------------------------------------------------------
    // other stuff
    //------------------------------------------------------------------------------------
    void       updateFilterUI();

    //------------------------------------------------------------------------------------
    // Type definitions
    //------------------------------------------------------------------------------------
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
    inline NSView* getUserPane() {
        if (m_bIsUserPaneLaidOut == false) {
            createUserPane();
        }
        return m_pUserPane;
    }

    inline bool    getVisibility(ToggleType tToggle) {
        return m_bToggleVisibility[tToggle];
    }

    inline void    setFilterControlNeeded(bool bNeeded) {
        m_bIsFilterControlNeeded = bNeeded;
        if (bNeeded == true) {
            m_bUserPaneNeeded = true;
        }
    }

    inline void    setFilterHelper(FilterHelper* pFilterHelper) {
        m_pFilterHelper = pFilterHelper;
    }

    inline void    setFilePickerDelegate(AquaFilePickerDelegate* pDelegate) {
        m_pDelegate = pDelegate;
    }

    inline bool    isAutoExtensionEnabled() {
        return ([((NSButton*) m_pToggles[AUTOEXTENSION]) state] == NSOnState);
    }

private:
    //------------------------------------------------------------------------------------
    // private member variables
    //------------------------------------------------------------------------------------

    /** the native view object */
    NSView*    m_pUserPane;

    /** the checkbox controls */
    NSControl* m_pToggles[ TOGGLE_LAST ];

    /** the visibility flags for the checkboxes */
    bool m_bToggleVisibility[TOGGLE_LAST];

    /** the special filter control */
    NSPopUpButton *m_pFilterControl;

    /** the popup menu controls (except for the filter control) */
    NSControl* m_pListControls[ LIST_LAST ];

    /** a map to store a control's label text */
    ::std::map<NSControl *, NSString *> m_aMapListLabels;

    /** a map to store a popup menu's label text field */
    ::std::map<NSPopUpButton *, NSTextField *> m_aMapListLabelFields;

    /** the visibility flags for the popup menus */
    bool m_bListVisibility[ LIST_LAST ];

    /** indicates if a user pane is needed */
    bool m_bUserPaneNeeded;

    /** indicates if the user pane was laid out already */
    bool m_bIsUserPaneLaidOut;

    /** indicates if a filter control is needed */
    bool m_bIsFilterControlNeeded;

    /** a list with all actively used controls */
    ::std::list<NSControl*> m_aActiveControls;

    /** the filter helper */
    FilterHelper *m_pFilterHelper;

    /** the save or open panel's delegate */
    AquaFilePickerDelegate *m_pDelegate;

    //------------------------------------------------------------------------------------
    // private methods
    //------------------------------------------------------------------------------------
    void HandleSetListValue(const NSControl* pControl, const sal_Int16 nControlAction, const uno::Any& rValue);
    uno::Any HandleGetListValue(const NSControl* pControl, const sal_Int16 nControlAction) const;

    void         createControls();
    void         createFilterControl();
    void         createUserPane();
    NSTextField* createLabelWithString(const NSString* label);

    int          getControlElementName(const Class clazz, const int nControlId) const;
    NSControl*   getControl( const sal_Int16 nControlId ) const;
    static int   getVerticalDistance(const NSControl* first, const NSControl* second);

    void         layoutControls();
};

#endif //_CONTROLHELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
