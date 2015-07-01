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
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxParent,
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxCreator,
    ::svt::table::IAccessibleTable& _rTable
    );

    virtual ~AccessibleGridControl();

    /** Cleans up members. */
    using AccessibleGridControlBase::disposing;
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

protected:
    // XAccessibleContext -----------------------------------------------------

    /** @return  The count of visible children. */
    virtual sal_Int32 SAL_CALL getAccessibleChildCount()
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    /** @return  The XAccessible interface of the specified child. */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
    getAccessibleChild( sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    /** @return  The role of this object (a table). */
    virtual sal_Int16 SAL_CALL getAccessibleRole()
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XAccessibleComponent ---------------------------------------------------

    /** @return
            The accessible child rendered under the given point.
    */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
    getAccessibleAtPoint( const ::com::sun::star::awt::Point& rPoint )
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    /** Grabs the focus to the Grid Control. */
    virtual void SAL_CALL grabFocus()
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XServiceInfo -----------------------------------------------------------

    /** @return
            The name of this class.
    */
    virtual OUString SAL_CALL getImplementationName()
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

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
     void commitCellEvent(sal_Int16 nEventId, const ::com::sun::star::uno::Any& rNewValue,
             const ::com::sun::star::uno::Any& rOldValue);

     /** commitTableEvent commit the event at all listeners of the table
         @param nEventId
             the event id
         @param rNewValue
             the new value
         @param rOldValue
             the old value
     */
     void commitTableEvent(sal_Int16 nEventId, const ::com::sun::star::uno::Any& rNewValue,
             const ::com::sun::star::uno::Any& rOldValue);
    /** returns the accessible object for the row or the column header bar
    */
    inline ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible >
        getHeaderBar( ::svt::table::AccessibleTableControlObjType _eObjType )
        {
            return implGetHeaderBar(_eObjType);
        }

    /** returns the accessible object for the table representation
    */
    inline ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible >
        getTable( )
        {
            return implGetTable();
        }

protected:
    // internal virtual methods -----------------------------------------------

    /** @attention  This method requires locked mutex's and a living object.
        @return  The bounding box (VCL rect.) relative to the parent window. */
    virtual Rectangle implGetBoundingBox() SAL_OVERRIDE;
    /** @attention  This method requires locked mutex's and a living object.
        @return  The bounding box (VCL rect.) in screen coordinates. */
    virtual Rectangle implGetBoundingBoxOnScreen() SAL_OVERRIDE;

    // internal helper methods ------------------------------------------------

    /** This method creates (once) and returns the accessible data table child.
        @attention  This method requires locked mutex's and a living object.
        @return  The XAccessible interface of the data table. */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > implGetTable();

    /** This method creates (once) and returns the specified header bar.
        @attention  This method requires locked mutex's and a living object.
        @return  The XAccessible interface of the header bar. */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible >
        implGetHeaderBar( ::svt::table::AccessibleTableControlObjType eObjType );

    /** This method returns one of the children that are always present:
        Data table, row and column header bar or corner control.
        @attention  This method requires locked mutex's and a living object.
        @return  The XAccessible interface of the specified child. */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible >
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
    the contxt holds this instance weak.</p>
*/
typedef ::cppu::WeakImplHelper< ::com::sun::star::accessibility::XAccessible > AccessibleGridControlAccess_Base;

class AccessibleGridControlAccess :public AccessibleGridControlAccess_Base
    ,public ::svt::table::IAccessibleTableControl
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                                        m_xParent;
    ::svt::table::IAccessibleTable *    m_pTable;

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
                                m_xContext;
    AccessibleGridControl*      m_pContext;
                                    // note that this pointer is valid as long as m_xContext is valid!

public:
    AccessibleGridControlAccess(
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxParent,
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
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
        SAL_CALL getAccessibleContext() throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // IAccessibleTable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        getMyself() SAL_OVERRIDE
    {
        return this;
    }
    void DisposeAccessImpl() SAL_OVERRIDE;
    virtual bool isAlive() const SAL_OVERRIDE
    {
        return isContextAlive();
    }
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        getTableHeader( ::svt::table::AccessibleTableControlObjType _eObjType ) SAL_OVERRIDE
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > xAccessible;
        AccessibleGridControl* pContext( getContext() );
        if ( pContext )
            xAccessible = pContext->getHeaderBar( _eObjType );
        return xAccessible;
    }
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        getTable() SAL_OVERRIDE
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > xAccessible;
        AccessibleGridControl* pContext( getContext() );
        if ( pContext )
            xAccessible = pContext->getTable();
        return xAccessible;
    }
    virtual void commitCellEvent( sal_Int16 nEventId,
         const ::com::sun::star::uno::Any& rNewValue, const ::com::sun::star::uno::Any& rOldValue ) SAL_OVERRIDE
    {
         AccessibleGridControl* pContext( getContext() );
         if ( pContext )
            pContext->commitCellEvent( nEventId, rNewValue, rOldValue );
    }
    virtual void commitTableEvent( sal_Int16 nEventId,
         const ::com::sun::star::uno::Any& rNewValue, const ::com::sun::star::uno::Any& rOldValue ) SAL_OVERRIDE
    {
         AccessibleGridControl* pContext( getContext() );
         if ( pContext )
            pContext->commitTableEvent( nEventId, rNewValue, rOldValue );
    }
    virtual void commitEvent( sal_Int16 nEventId,
        const ::com::sun::star::uno::Any& rNewValue, const ::com::sun::star::uno::Any& rOldValue ) SAL_OVERRIDE
    {
        AccessibleGridControl* pContext( getContext() );
        if ( pContext )
            pContext->commitEvent( nEventId, rNewValue, rOldValue );
    }

private:
    AccessibleGridControlAccess( const AccessibleGridControlAccess& ) SAL_DELETED_FUNCTION;
    AccessibleGridControlAccess& operator=( const AccessibleGridControlAccess& ) SAL_DELETED_FUNCTION;
};


} // namespace accessibility



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
