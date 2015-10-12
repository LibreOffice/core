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


#ifndef INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLEBROWSEBOX_HXX
#define INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLEBROWSEBOX_HXX

#include <accessibility/extended/AccessibleBrowseBoxBase.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>
#include <svtools/accessibletableprovider.hxx>
#include <memory>

namespace accessibility {

    class AccessibleBrowseBoxImpl;
    class AccessibleBrowseBoxTable;



/** This class represents the complete accessible BrowseBox object. */
class AccessibleBrowseBox : public AccessibleBrowseBoxBase
{
    friend class AccessibleBrowseBoxAccess;

protected:
    AccessibleBrowseBox(
        const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
        const css::uno::Reference< css::accessibility::XAccessible >& _rxCreator,
        ::svt::IAccessibleTableProvider& _rBrowseBox
    );

    virtual ~AccessibleBrowseBox();

    /** sets the XAccessible which created the context

        <p>To be called only once, and only if in the ctor NULL was passed.</p>
    */
    void    setCreator(
        const css::uno::Reference< css::accessibility::XAccessible >& _rxCreator
    );

    /** Cleans up members. */
    using AccessibleBrowseBoxBase::disposing;
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

protected:
    // XAccessibleContext -----------------------------------------------------

    /** @return  The count of visible children. */
    virtual sal_Int32 SAL_CALL getAccessibleChildCount()
        throw ( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    /** @return  The XAccessible interface of the specified child. */
    virtual css::uno::Reference<
        css::accessibility::XAccessible > SAL_CALL
    getAccessibleChild( sal_Int32 nChildIndex )
        throw ( css::lang::IndexOutOfBoundsException,
                css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XAccessibleComponent ---------------------------------------------------

    /** @return
            The accessible child rendered under the given point.
    */
    virtual css::uno::Reference<
        css::accessibility::XAccessible > SAL_CALL
    getAccessibleAtPoint( const css::awt::Point& rPoint )
        throw ( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    /** Grabs the focus to the BrowseBox. */
    virtual void SAL_CALL grabFocus()
        throw ( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XServiceInfo -----------------------------------------------------------

    /** @return
            The name of this class.
    */
    virtual OUString SAL_CALL getImplementationName()
        throw ( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

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
            const css::uno::Any& rNewValue,
            const css::uno::Any& rOldValue,bool _bColumnHeaderBar = true);

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
            const css::uno::Any& rNewValue,
            const css::uno::Any& rOldValue);

    /** returns the accessible object for the row or the column header bar
    */
    inline css::uno::Reference<
        css::accessibility::XAccessible >
        getHeaderBar( ::svt::AccessibleBrowseBoxObjType _eObjType )
        {
            return implGetHeaderBar(_eObjType);
        }

    /** returns the accessible object for the table representation
    */
    inline css::uno::Reference<
        css::accessibility::XAccessible >
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
    css::uno::Reference<
        css::accessibility::XAccessible > implGetTable();

    /** This method creates (once) and returns the specified header bar.
        @attention  This method requires locked mutex's and a living object.
        @return  The XAccessible interface of the header bar. */
    css::uno::Reference<
        css::accessibility::XAccessible >
        implGetHeaderBar( ::svt::AccessibleBrowseBoxObjType eObjType );

    /** This method returns one of the children that are always present:
        Data table, row and column header bar or corner control.
        @attention  This method requires locked mutex's and a living object.
        @return  The XAccessible interface of the specified child. */
    css::uno::Reference<
        css::accessibility::XAccessible >
    implGetFixedChild( sal_Int32 nChildIndex );

    /** This method creates and returns an accessible table.
        @return  An AccessibleBrowseBoxTable. */
    virtual AccessibleBrowseBoxTable*   createAccessibleTable();

private:
    // members ----------------------------------------------------------------
    std::unique_ptr< AccessibleBrowseBoxImpl > m_xImpl;
};


/** the XAccessible which creates/returns an AccessibleBrowseBox

    <p>The instance holds its XAccessibleContext with a hard reference, while
    the context holds this instance weak.</p>
*/
typedef ::cppu::WeakImplHelper<   css::accessibility::XAccessible
                              >   AccessibleBrowseBoxAccess_Base;

class AccessibleBrowseBoxAccess :public AccessibleBrowseBoxAccess_Base
                                ,public ::svt::IAccessibleBrowseBox
{
private:
    ::osl::Mutex                m_aMutex;
    css::uno::Reference< css::accessibility::XAccessible >
                                        m_xParent;
    ::svt::IAccessibleTableProvider&    m_rBrowseBox;

    css::uno::Reference< css::accessibility::XAccessibleContext >
                                m_xContext;
    AccessibleBrowseBox*        m_pContext;
                                    // note that this pointer is valid as long as m_xContext is valid!

public:
    AccessibleBrowseBoxAccess(
        const css::uno::Reference< css::accessibility::XAccessible >& _rxParent,
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
    virtual css::uno::Reference< css::accessibility::XAccessibleContext >
        SAL_CALL getAccessibleContext() throw ( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // IAccessibleBrowseBox
    virtual css::uno::Reference< css::accessibility::XAccessible >
        getMyself() SAL_OVERRIDE
    {
        return this;
    }
    void dispose() SAL_OVERRIDE;
    virtual bool isAlive() const SAL_OVERRIDE
    {
        return isContextAlive();
    }
    virtual css::uno::Reference< css::accessibility::XAccessible >
        getHeaderBar( ::svt::AccessibleBrowseBoxObjType _eObjType ) SAL_OVERRIDE
    {
        css::uno::Reference< css::accessibility::XAccessible > xAccessible;
        AccessibleBrowseBox* pContext( getContext() );
        if ( pContext )
            xAccessible = pContext->getHeaderBar( _eObjType );
        return xAccessible;
    }
    virtual css::uno::Reference< css::accessibility::XAccessible >
        getTable() SAL_OVERRIDE
    {
        css::uno::Reference< css::accessibility::XAccessible > xAccessible;
        AccessibleBrowseBox* pContext( getContext() );
        if ( pContext )
            xAccessible = pContext->getTable();
        return xAccessible;
    }
    virtual void commitHeaderBarEvent( sal_Int16 nEventId, const css::uno::Any& rNewValue,
        const css::uno::Any& rOldValue, bool _bColumnHeaderBar ) SAL_OVERRIDE
    {
        AccessibleBrowseBox* pContext( getContext() );
        if ( pContext )
            pContext->commitHeaderBarEvent( nEventId, rNewValue, rOldValue, _bColumnHeaderBar );
    }
    virtual void commitTableEvent( sal_Int16 nEventId,
        const css::uno::Any& rNewValue, const css::uno::Any& rOldValue ) SAL_OVERRIDE
    {
        AccessibleBrowseBox* pContext( getContext() );
        if ( pContext )
            pContext->commitTableEvent( nEventId, rNewValue, rOldValue );
    }
    virtual void commitEvent( sal_Int16 nEventId,
        const css::uno::Any& rNewValue, const css::uno::Any& rOldValue ) SAL_OVERRIDE
    {
        AccessibleBrowseBox* pContext( getContext() );
        if ( pContext )
            pContext->commitEvent( nEventId, rNewValue, rOldValue );
    }

private:
    AccessibleBrowseBoxAccess( const AccessibleBrowseBoxAccess& ) = delete;
    AccessibleBrowseBoxAccess& operator=( const AccessibleBrowseBoxAccess& ) = delete;
};


} // namespace accessibility



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
