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

#ifndef INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLEGRIDCONTROL_HXX
#define INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLEGRIDCONTROL_HXX

#include <accessibility/extended/AccessibleGridControlBase.hxx>
#include <accessibility/extended/AccessibleGridControlTable.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>
#include <svtools/accessibletable.hxx>
#include <memory>

namespace accessibility {

    class AccessibleGridControl_Impl;



/** This class represents the complete accessible Grid Control object. */
class AccessibleGridControl : public AccessibleGridControlBase
{
    friend class AccessibleGridControlAccess;

protected:
    AccessibleGridControl(
        const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
        const css::uno::Reference< css::accessibility::XAccessible >& _rxCreator,
    ::svt::table::IAccessibleTable& _rTable
    );

    virtual ~AccessibleGridControl();

    /** Cleans up members. */
    using AccessibleGridControlBase::disposing;
    virtual void SAL_CALL disposing() override;

protected:
    // XAccessibleContext -----------------------------------------------------

    /** @return  The count of visible children. */
    virtual sal_Int32 SAL_CALL getAccessibleChildCount()
        throw ( css::uno::RuntimeException, std::exception ) override;

    /** @return  The XAccessible interface of the specified child. */
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
    getAccessibleChild( sal_Int32 nChildIndex )
        throw ( css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException, std::exception ) override;

    /** @return  The role of this object (a table). */
    virtual sal_Int16 SAL_CALL getAccessibleRole()
        throw ( css::uno::RuntimeException, std::exception ) override;

    // XAccessibleComponent ---------------------------------------------------

    /** @return
            The accessible child rendered under the given point.
    */
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
    getAccessibleAtPoint( const css::awt::Point& rPoint )
        throw ( css::uno::RuntimeException, std::exception ) override;

    /** Grabs the focus to the Grid Control. */
    virtual void SAL_CALL grabFocus()
        throw ( css::uno::RuntimeException, std::exception ) override;

    // XServiceInfo -----------------------------------------------------------

    /** @return
            The name of this class.
    */
    virtual OUString SAL_CALL getImplementationName()
        throw ( css::uno::RuntimeException, std::exception ) override;

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

protected:
    // internal virtual methods -----------------------------------------------

    /** @attention  This method requires locked mutex's and a living object.
        @return  The bounding box (VCL rect.) relative to the parent window. */
    virtual Rectangle implGetBoundingBox() override;
    /** @attention  This method requires locked mutex's and a living object.
        @return  The bounding box (VCL rect.) in screen coordinates. */
    virtual Rectangle implGetBoundingBoxOnScreen() override;

    // internal helper methods ------------------------------------------------

    /** This method creates (once) and returns the accessible data table child.
        @attention  This method requires locked mutex's and a living object.
        @return  The XAccessible interface of the data table. */
    css::uno::Reference< css::accessibility::XAccessible > implGetTable();

    /** This method creates (once) and returns the specified header bar.
        @attention  This method requires locked mutex's and a living object.
        @return  The XAccessible interface of the header bar. */
    css::uno::Reference< css::accessibility::XAccessible >
        implGetHeaderBar( ::svt::table::AccessibleTableControlObjType eObjType );

    /** This method returns one of the children that are always present:
        Data table, row and column header bar or corner control.
        @attention  This method requires locked mutex's and a living object.
        @return  The XAccessible interface of the specified child. */
    css::uno::Reference< css::accessibility::XAccessible >
        implGetFixedChild( sal_Int32 nChildIndex );

    /** This method creates and returns an accessible table.
        @return  An AccessibleGridControlTable. */
    AccessibleGridControlTable* createAccessibleTable();

private:
    // members ----------------------------------------------------------------
    std::unique_ptr< AccessibleGridControl_Impl > m_xImpl;
};


/** the XAccessible which creates/returns an AccessibleGridControl

    <p>The instance holds its XAccessibleContext with a hard reference, while
    the context holds this instance weak.</p>
*/

class AccessibleGridControlAccess :
     public ::cppu::WeakImplHelper< css::accessibility::XAccessible >
    ,public ::svt::table::IAccessibleTableControl
{
private:
    css::uno::Reference< css::accessibility::XAccessible >
                                        m_xParent;
    ::svt::table::IAccessibleTable *    m_pTable;

    css::uno::Reference< css::accessibility::XAccessibleContext >
                                m_xContext;
    AccessibleGridControl*      m_pContext;
                                    // note that this pointer is valid as long as m_xContext is valid!

public:
    AccessibleGridControlAccess(
        const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
        ::svt::table::IAccessibleTable& _rTable
    );

    /// checks whether the accessible context is still alive
    bool                            isContextAlive() const;

    /// returns the AccessibleContext belonging to this Accessible
    inline AccessibleGridControl*            getContext()         { return m_pContext; }
    inline const AccessibleGridControl*      getContext() const   { return m_pContext; }

protected:
    virtual ~AccessibleGridControlAccess();

    // XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext >
        SAL_CALL getAccessibleContext() throw ( css::uno::RuntimeException, std::exception ) override;

    // IAccessibleTable
    virtual css::uno::Reference< css::accessibility::XAccessible >
        getMyself() override
    {
        return this;
    }
    void DisposeAccessImpl() override;
    virtual bool isAlive() const override
    {
        return isContextAlive();
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
        const css::uno::Any& rNewValue, const css::uno::Any& rOldValue ) override
    {
        AccessibleGridControl* pContext( getContext() );
        if ( pContext )
            pContext->commitEvent( nEventId, rNewValue, rOldValue );
    }

private:
    AccessibleGridControlAccess( const AccessibleGridControlAccess& ) = delete;
    AccessibleGridControlAccess& operator=( const AccessibleGridControlAccess& ) = delete;
};


} // namespace accessibility



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
