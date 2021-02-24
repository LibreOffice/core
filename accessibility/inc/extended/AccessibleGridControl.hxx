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

#include <extended/AccessibleGridControlBase.hxx>
#include <extended/AccessibleGridControlTable.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>
#include <rtl/ref.hxx>
#include <vcl/accessibletable.hxx>

namespace accessibility {

    class AccessibleGridControlHeader;


/** This class represents the complete accessible Grid Control object. */
class AccessibleGridControl final : public AccessibleGridControlBase
{
    friend class AccessibleGridControlAccess;

    AccessibleGridControl(
        const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
        const css::uno::Reference< css::accessibility::XAccessible >& _rxCreator,
    ::vcl::table::IAccessibleTable& _rTable
    );

    virtual ~AccessibleGridControl() override = default;

    /** Cleans up members. */
    using AccessibleGridControlBase::disposing;
    virtual void SAL_CALL disposing() override;

    // XAccessibleContext -----------------------------------------------------

    /** @return  The count of visible children. */
    virtual sal_Int32 SAL_CALL getAccessibleChildCount() override;

    /** @return  The XAccessible interface of the specified child. */
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
    getAccessibleChild( sal_Int32 nChildIndex ) override;

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
        @return  The bounding box (VCL rect.) relative to the parent window. */
    virtual tools::Rectangle implGetBoundingBox() override;
    /** @attention  This method requires locked mutex's and a living object.
        @return  The bounding box (VCL rect.) in screen coordinates. */
    virtual tools::Rectangle implGetBoundingBoxOnScreen() override;

    // internal helper methods ------------------------------------------------

    /** This method creates (once) and returns the accessible data table child.
        @attention  This method requires locked mutex's and a living object.
        @return  The XAccessible interface of the data table. */
    css::uno::Reference< css::accessibility::XAccessible > implGetTable();

    /** This method creates (once) and returns the specified header bar.
        @attention  This method requires locked mutex's and a living object.
        @return  The XAccessible interface of the header bar. */
    css::uno::Reference< css::accessibility::XAccessible >
        implGetHeaderBar( ::vcl::table::AccessibleTableControlObjType eObjType );

    /** This method returns one of the children that are always present:
        Data table, row and column header bar or corner control.
        @attention  This method requires locked mutex's and a living object.
        @return  The XAccessible interface of the specified child. */
    css::uno::Reference< css::accessibility::XAccessible >
        implGetFixedChild( sal_Int32 nChildIndex );

    /** This method creates and returns an accessible table.
        @return  An AccessibleGridControlTable. */
    rtl::Reference<AccessibleGridControlTable> createAccessibleTable();

    /// the css::accessibility::XAccessible which created the AccessibleGridControl
    css::uno::WeakReference< css::accessibility::XAccessible >                    m_aCreator;

    /** The data table child. */
    rtl::Reference<AccessibleGridControlTable>                m_xTable;

    /** The header bar for rows. */
    rtl::Reference<AccessibleGridControlHeader>               m_xRowHeaderBar;

    /** The header bar for columns (first row of the table). */
    rtl::Reference<AccessibleGridControlHeader>               m_xColumnHeaderBar;

    /** The table cell child. */
    rtl::Reference<AccessibleGridControlTableCell>            m_xCell;

    /** @return  The count of visible children. */
    inline sal_Int32 implGetAccessibleChildCount();
};


/** the XAccessible which creates/returns an AccessibleGridControl

    <p>The instance holds its XAccessibleContext with a hard reference, while
    the context holds this instance weak.</p>
*/

class AccessibleGridControlAccess :
     public ::cppu::WeakImplHelper< css::accessibility::XAccessible >
    ,public ::vcl::table::IAccessibleTableControl
{
private:
    css::uno::Reference< css::accessibility::XAccessible > m_xParent;
    ::vcl::table::IAccessibleTable *                       m_pTable;
    rtl::Reference<AccessibleGridControl>                  m_xContext;

public:
    AccessibleGridControlAccess(
        const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
        ::vcl::table::IAccessibleTable& _rTable
    );

    /// returns the AccessibleContext belonging to this Accessible
    AccessibleGridControl*            getContext()  { return m_xContext.get(); }

protected:
    virtual ~AccessibleGridControlAccess() override;

    // XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext >
        SAL_CALL getAccessibleContext() override;

    // IAccessibleTable
    virtual css::uno::Reference< css::accessibility::XAccessible >
        getMyself() override
    {
        return this;
    }
    void DisposeAccessImpl() override;
    virtual bool isAlive() const override
    {
        return m_xContext.is() && m_xContext->isAlive();
    }
    virtual void commitCellEvent( sal_Int16 nEventId,
         const css::uno::Any& rNewValue, const css::uno::Any& rOldValue ) override
    {
         AccessibleGridControl* pContext( getContext() );
         if ( pContext )
            pContext->commitCellEvent( nEventId, rNewValue, rOldValue );
    }
    virtual void commitTableEvent( sal_Int16 nEventId,
         const css::uno::Any& rNewValue, const css::uno::Any& rOldValue ) override
    {
         AccessibleGridControl* pContext( getContext() );
         if ( pContext )
            pContext->commitTableEvent( nEventId, rNewValue, rOldValue );
    }
    virtual void commitEvent( sal_Int16 nEventId,
        const css::uno::Any& rNewValue ) override
    {
        AccessibleGridControl* pContext( getContext() );
        if ( pContext )
            pContext->commitEvent( nEventId, rNewValue, css::uno::Any() );
    }

private:
    AccessibleGridControlAccess( const AccessibleGridControlAccess& ) = delete;
    AccessibleGridControlAccess& operator=( const AccessibleGridControlAccess& ) = delete;
};


} // namespace accessibility



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
