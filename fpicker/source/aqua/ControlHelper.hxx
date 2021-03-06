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

#pragma once

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


    // Constructor / Destructor

    ControlHelper();
    virtual ~ControlHelper();


    // XInitialization delegate

    void       initialize( sal_Int16 templateId );


    // XFilePickerControlAccess function delegates

    void       setValue( sal_Int16 nControlId, sal_Int16 nControlAction, const uno::Any& rValue );
    uno::Any   getValue( sal_Int16 nControlId, sal_Int16 nControlAction ) const;
    void       enableControl( sal_Int16 nControlId, bool bEnable ) const;
    OUString   getLabel( sal_Int16 nControlId );
    void       setLabel( sal_Int16 nControlId, NSString* aLabel );


    // other stuff

    void       updateFilterUI();


    // Type definitions

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
        IMAGE_ANCHOR,
        LIST_LAST
    };


    // inline functions

    NSView* getUserPane() {
        if (!m_bIsUserPaneLaidOut) {
            createUserPane();
        }
        return m_pUserPane;
    }

    bool    getVisibility(ToggleType tToggle) {
        return m_bToggleVisibility[tToggle];
    }

    void    setFilterControlNeeded(bool bNeeded) {
        m_bIsFilterControlNeeded = bNeeded;
        if (bNeeded) {
            m_bUserPaneNeeded = true;
        }
    }

    void    setFilterHelper(FilterHelper* pFilterHelper) {
        m_pFilterHelper = pFilterHelper;
    }

    void    setFilePickerDelegate(AquaFilePickerDelegate* pDelegate) {
        m_pDelegate = pDelegate;
    }

    bool    isAutoExtensionEnabled() {
        return ([static_cast<NSButton*>(m_pToggles[AUTOEXTENSION]) state] == NSControlStateValueOn);
    }

private:

    // private member variables


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


    // private methods

    void HandleSetListValue(const NSControl* pControl, const sal_Int16 nControlAction, const uno::Any& rValue);

    void         createControls();
    void         createFilterControl();
    void         createUserPane();

    static int   getControlElementName(const Class clazz, const int nControlId);
    NSControl*   getControl( const sal_Int16 nControlId ) const;
    static int   getVerticalDistance(const NSControl* first, const NSControl* second);

    void         layoutControls();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
