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
#ifndef CHART2_WEAKLISTENERADAPTER_HXX
#define CHART2_WEAKLISTENERADAPTER_HXX

#include <com/sun/star/uno/XWeak.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/implbase1.hxx>

namespace chart
{
// --------------------------------------------------------------------------------

/** Adapter that enables adding listeners as weak UNO references.  Thus, adding
    an object as listener to a broadcaster does not increase its reference
    count.

    <p>The helper class, of course, is held as hard reference at the
    broadcaster, but this should never be a problem as the adapter's life time
    depends on no other object.</p>

    <p>Note that in order to remove an object as listener, you have to remove
    the same wrapper that you added, i.e., you should store the adapter as a
    member in the adaptee class for later use.</p>
 */
template< class Listener >
    class WeakListenerAdapter : public
    ::cppu::WeakImplHelper1< Listener >
{
public:
    explicit WeakListenerAdapter( const ::com::sun::star::uno::Reference< Listener > & xListener ) :
            m_xListener( xListener )
    {}
    explicit WeakListenerAdapter( const ::com::sun::star::uno::WeakReference< Listener > & xListener ) :
            m_xListener( xListener )
    {}
    virtual ~WeakListenerAdapter()
    {}

protected:
    // ____ XEventListener (base of all listeners) ____
    virtual void SAL_CALL disposing(
        const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException)
    {
        ::com::sun::star::uno::Reference<
              ::com::sun::star::lang::XEventListener > xEventListener =
          ::com::sun::star::uno::Reference<
              ::com::sun::star::lang::XEventListener >(
                  ::com::sun::star::uno::Reference< Listener >( m_xListener), ::com::sun::star::uno::UNO_QUERY );
        if( xEventListener.is())
            xEventListener->disposing( Source );
    }

    ::com::sun::star::uno::Reference< Listener > getListener() const
    {
        return m_xListener;
    }

private:
    ::com::sun::star::uno::WeakReference< Listener > m_xListener;
};

// --------------------------------------------------------------------------------

class WeakModifyListenerAdapter :
        public WeakListenerAdapter< ::com::sun::star::util::XModifyListener >
{
public:
    explicit WeakModifyListenerAdapter(
        const ::com::sun::star::uno::WeakReference< ::com::sun::star::util::XModifyListener > & xListener );
    virtual ~WeakModifyListenerAdapter();

protected:
    // ____ XModifyListener ____
    virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException);
};

// --------------------------------------------------------------------------------

class WeakSelectionChangeListenerAdapter :
        public WeakListenerAdapter< ::com::sun::star::view::XSelectionChangeListener >
{
public:
    explicit WeakSelectionChangeListenerAdapter(
        const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener > & xListener );
    virtual ~WeakSelectionChangeListenerAdapter();

protected:
    // ____ XSelectionChangeListener ____
    virtual void SAL_CALL selectionChanged(
        const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException);
};

} //  namespace chart

// CHART2_WEAKLISTENERADAPTER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
