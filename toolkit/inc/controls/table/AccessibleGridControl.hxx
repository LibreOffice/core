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

#include "AccessibleGridControlBase.hxx"
#include "AccessibleGridControlTable.hxx"
#include "tablecontrol.hxx"

#include <cppuhelper/weakref.hxx>
#include <rtl/ref.hxx>

namespace accessibility {

    class AccessibleGridControlHeader;


/** This class represents the complete accessible Grid Control object. */
    class AccessibleGridControl final
        : public cppu::ImplInheritanceHelper<AccessibleGridControlBase,
                                             css::accessibility::XAccessible>
{
    virtual ~AccessibleGridControl() override = default;

    /** Cleans up members. */
    using AccessibleGridControlBase::disposing;
    virtual void SAL_CALL disposing() override;

    // XAccessible
    virtual css::uno::Reference<css::accessibility::XAccessibleContext>
        SAL_CALL getAccessibleContext() override;

    // XAccessibleContext -----------------------------------------------------

    /** @return  The count of visible children. */
    virtual sal_Int64 SAL_CALL getAccessibleChildCount() override;

    /** @return  The XAccessible interface of the specified child. */
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
    getAccessibleChild( sal_Int64 nChildIndex ) override;

    /** @return  The role of this object (a table). */
    virtual sal_Int16 SAL_CALL getAccessibleRole() override;

    // XAccessibleComponent ---------------------------------------------------

    /** @return
            The accessible child rendered under the given point.
    */
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
    getAccessibleAtPoint( const css::awt::Point& rPoint ) override;

    /** Grabs the focus to the Grid Control. */
    virtual void SAL_CALL grabFocus() override;

    // XServiceInfo -----------------------------------------------------------

    /** @return
            The name of this class.
    */
    virtual OUString SAL_CALL getImplementationName() override;

public:
    AccessibleGridControl(
        const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
        svt::table::TableControl& _rTable);

    // helper functions

     /** commitCellEvent commit the event at all listeners of the table
         @param nEventId
             the event id
         @param rNewValue
             the new value
         @param rOldValue
             the old value
     */
     void commitCellEvent(sal_Int16 nEventId, const css::uno::Any& rNewValue,
             const css::uno::Any& rOldValue);

     /** commitTableEvent commit the event at all listeners of the table
         @param nEventId
             the event id
         @param rNewValue
             the new value
         @param rOldValue
             the old value
     */
     void commitTableEvent(sal_Int16 nEventId, const css::uno::Any& rNewValue,
             const css::uno::Any& rOldValue);

private:
    // internal virtual methods -----------------------------------------------

    /** @attention  This method requires locked mutex's and a living object.
        @return  The bounding box (VCL rect.) in screen coordinates. */
    virtual AbsoluteScreenPixelRectangle implGetBoundingBoxOnScreen() override;

    // internal helper methods ------------------------------------------------

    /** The data table child. */
    rtl::Reference<AccessibleGridControlTable>                m_xTable;

    /** The header bar for rows. */
    rtl::Reference<AccessibleGridControlHeader>               m_xRowHeaderBar;

    /** The header bar for columns (first row of the table). */
    rtl::Reference<AccessibleGridControlHeader>               m_xColumnHeaderBar;

    /** @return  The count of visible children. */
    inline sal_Int64 implGetAccessibleChildCount();
};

} // namespace accessibility



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
