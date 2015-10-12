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

#ifndef INCLUDED_SC_INC_ACCESSIBLEFILTERTOPWINDOW_HXX
#define INCLUDED_SC_INC_ACCESSIBLEFILTERTOPWINDOW_HXX

#include "AccessibleFilterMenu.hxx"

class ScCheckListMenuWindow;

class ScAccessibleFilterTopWindow : public ScAccessibleFilterMenu
{
public:
    ScAccessibleFilterTopWindow(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>& rxParent,
            ScCheckListMenuWindow* pWin, const OUString& rName);
    virtual ~ScAccessibleFilterTopWindow();

    // XAccessibleContext

    virtual sal_Int32 SAL_CALL getAccessibleChildCount()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild(sal_Int32 nIndex)
            throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException, std::exception) override;

    virtual OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // Non-UNO Methods

    enum ChildControlType {
        EDIT_SEARCH_BOX, LISTBOX, TOGGLE_ALL, SINGLE_ON_BTN, SINGLE_OFF_BTN, OK_BTN, CANCEL_BTN
    };
    void setAccessibleChild(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible >& rAccessible,
        ChildControlType eType);

private:
    /** The top menu part */
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        mxAccMenu;

    /** Edit search box for searching field members */
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        mxAccEditSearchBox;
    /** check list box for field member visibility */
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        mxAccListBox;

    /** check box for toggling all field member's visibility. */
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        mxAccToggleAll;

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        mxAccSingleOnBtn;

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        mxAccSingleOffBtn;

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        mxAccOkBtn;

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        mxAccCancelBtn;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
