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
    void       setLabel( sal_Int16 nControlId, NSString* aLabel );

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
    NSTextField* createLabelWithString(NSString* label);

    int          getControlElementName(const Class clazz, const int nControlId) const;
    NSControl*   getControl( const sal_Int16 nControlId ) const;
    static int   getVerticalDistance(const NSControl* first, const NSControl* second);

    void         layoutControls();
};

#endif //_CONTROLHELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
