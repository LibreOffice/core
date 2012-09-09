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


#ifndef ACCESSIBILITY_EXT_ACCESSIBLEBROWSEBOX_HXX
#define ACCESSIBILITY_EXT_ACCESSIBLEBROWSEBOX_HXX

#include <accessibility/extended/AccessibleBrowseBoxBase.hxx>
#include <cppuhelper/weakref.hxx>
#include <svtools/accessibletableprovider.hxx>


#include <memory>

// ============================================================================

namespace accessibility {

    class AccessibleBrowseBoxImpl;
    class AccessibleBrowseBoxTable;

// ============================================================================

/** This class represents the complete accessible BrowseBox object. */
class AccessibleBrowseBox : public AccessibleBrowseBoxBase
{
    friend class AccessibleBrowseBoxAccess;

protected:
    AccessibleBrowseBox(
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxParent,
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxCreator,
        ::svt::IAccessibleTableProvider& _rBrowseBox
    );

    virtual ~AccessibleBrowseBox();

    /** sets the XAccessible which created the context

        <p>To be called only once, and only if in the ctor NULL was passed.</p>
    */
    void    setCreator(
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxCreator
    );

    /** Cleans up members. */
    using AccessibleBrowseBoxBase::disposing;
    virtual void SAL_CALL disposing();

protected:
    // XAccessibleContext -----------------------------------------------------

    /** @return  The count of visible children. */
    virtual sal_Int32 SAL_CALL getAccessibleChildCount()
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return  The XAccessible interface of the specified child. */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
    getAccessibleChild( sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );

    /** @return  The role of this object (a table). */
//    virtual sal_Int16 SAL_CALL getAccessibleRole()
//        throw ( ::com::sun::star::uno::RuntimeException );

    // XAccessibleComponent ---------------------------------------------------

    /** @return
            The accessible child rendered under the given point.
    */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
    getAccessibleAtPoint( const ::com::sun::star::awt::Point& rPoint )
        throw ( ::com::sun::star::uno::RuntimeException );

    /** Grabs the focus to the BrowseBox. */
    virtual void SAL_CALL grabFocus()
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return  The key bindings associated with this object. */
    virtual ::com::sun::star::uno::Any SAL_CALL getAccessibleKeyBinding()
        throw ( ::com::sun::star::uno::RuntimeException );

    // XServiceInfo -----------------------------------------------------------

    /** @return
            The name of this class.
    */
    virtual OUString SAL_CALL getImplementationName()
        throw ( ::com::sun::star::uno::RuntimeException );

public:
    // helper functions
    /** commitHeaderBarEvent commit the event at all listeners of the column/row header bar
        @param nEventId
            the event id
        @param rNewValue
            the new value
        @param rOldValue
            the old value
    */
    void commitHeaderBarEvent(sal_Int16 nEventId,
            const ::com::sun::star::uno::Any& rNewValue,
            const ::com::sun::star::uno::Any& rOldValue,sal_Bool _bColumnHeaderBar = sal_True);

    // helper functions
    /** commitTableEvent commit the event at all listeners of the table
        @param nEventId
            the event id
        @param rNewValue
            the new value
        @param rOldValue
            the old value
    */
    void commitTableEvent(sal_Int16 nEventId,
            const ::com::sun::star::uno::Any& rNewValue,
            const ::com::sun::star::uno::Any& rOldValue);

    /** returns the accessible object for the row or the column header bar
    */
    inline ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible >
        getHeaderBar( ::svt::AccessibleBrowseBoxObjType _eObjType )
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
    virtual Rectangle implGetBoundingBox();
    /** @attention  This method requires locked mutex's and a living object.
        @return  The bounding box (VCL rect.) in screen coordinates. */
    virtual Rectangle implGetBoundingBoxOnScreen();

    // internal helper methods ------------------------------------------------

    /** This method creates (once) and returns the accessible data table child.
        @attention  This method requires locked mutex's and a living object.
        @return  The XAccessible interface of the data table. */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > implGetTable();

    /** This method creates (once) and returns the specified header bar.
        @attention  This method requires locked mutex's and a living object.
        @return  The XAccessible interface of the header bar. */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible >
        implGetHeaderBar( ::svt::AccessibleBrowseBoxObjType eObjType );

    /** This method returns one of the children that are always present:
        Data table, row and column header bar or corner control.
        @attention  This method requires locked mutex's and a living object.
        @return  The XAccessible interface of the specified child. */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible >
    implGetFixedChild( sal_Int32 nChildIndex );

    /** This method creates and returns an accessible table.
        @return  An AccessibleBrowseBoxTable. */
    virtual AccessibleBrowseBoxTable*   createAccessibleTable();

private:
    // members ----------------------------------------------------------------
    ::std::auto_ptr< AccessibleBrowseBoxImpl > m_pImpl;
};

// ============================================================================
/** the XAccessible which creates/returns an AccessibleBrowseBox

    <p>The instance holds it's XAccessibleContext with a hard reference, while
    the contxt holds this instance weak.</p>
*/
typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::accessibility::XAccessible
                                >   AccessibleBrowseBoxAccess_Base;

class AccessibleBrowseBoxAccess :public AccessibleBrowseBoxAccess_Base
                                ,public ::svt::IAccessibleBrowseBox
{
private:
    ::osl::Mutex                m_aMutex;
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                                        m_xParent;
    ::svt::IAccessibleTableProvider&    m_rBrowseBox;

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
                                m_xContext;
    AccessibleBrowseBox*        m_pContext;
                                    // note that this pointer is valid as long as m_xContext is valid!

public:
    AccessibleBrowseBoxAccess(
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _rxParent,
        ::svt::IAccessibleTableProvider& _rBrowseBox
    );

    /// checks whether the accessible context is still alive
    bool                            isContextAlive() const;

    /// returns the AccessibleContext belonging to this Accessible
    inline AccessibleBrowseBox*            getContext()         { return m_pContext; }
    inline const AccessibleBrowseBox*      getContext() const   { return m_pContext; }

protected:
    virtual ~AccessibleBrowseBoxAccess();

    // XAccessible
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
        SAL_CALL getAccessibleContext() throw ( ::com::sun::star::uno::RuntimeException );

    // IAccessibleBrowseBox
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        getMyself()
    {
        return this;
    }
    void dispose();
    virtual sal_Bool isAlive() const
    {
        return isContextAlive();
    }
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        getHeaderBar( ::svt::AccessibleBrowseBoxObjType _eObjType )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > xAccessible;
        AccessibleBrowseBox* pContext( getContext() );
        if ( pContext )
            xAccessible = pContext->getHeaderBar( _eObjType );
        return xAccessible;
    }
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        getTable()
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > xAccessible;
        AccessibleBrowseBox* pContext( getContext() );
        if ( pContext )
            xAccessible = pContext->getTable();
        return xAccessible;
    }
    virtual void commitHeaderBarEvent( sal_Int16 nEventId, const ::com::sun::star::uno::Any& rNewValue,
        const ::com::sun::star::uno::Any& rOldValue, sal_Bool _bColumnHeaderBar )
    {
        AccessibleBrowseBox* pContext( getContext() );
        if ( pContext )
            pContext->commitHeaderBarEvent( nEventId, rNewValue, rOldValue, _bColumnHeaderBar );
    }
    virtual void commitTableEvent( sal_Int16 nEventId,
        const ::com::sun::star::uno::Any& rNewValue, const ::com::sun::star::uno::Any& rOldValue )
    {
        AccessibleBrowseBox* pContext( getContext() );
        if ( pContext )
            pContext->commitTableEvent( nEventId, rNewValue, rOldValue );
    }
    virtual void commitEvent( sal_Int16 nEventId,
        const ::com::sun::star::uno::Any& rNewValue, const ::com::sun::star::uno::Any& rOldValue )
    {
        AccessibleBrowseBox* pContext( getContext() );
        if ( pContext )
            pContext->commitEvent( nEventId, rNewValue, rOldValue );
    }

private:
    AccessibleBrowseBoxAccess();                                                // never implemented
    AccessibleBrowseBoxAccess( const AccessibleBrowseBoxAccess& );              // never implemented
    AccessibleBrowseBoxAccess& operator=( const AccessibleBrowseBoxAccess& );   // never implemented
};

// ============================================================================
} // namespace accessibility

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
