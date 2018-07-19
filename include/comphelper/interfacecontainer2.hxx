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
#ifndef INCLUDED_COMPHELPER_INTERFACECONTAINER2_H
#define INCLUDED_COMPHELPER_INTERFACECONTAINER2_H

#include <sal/config.h>

#include <cstddef>
#include <functional>
#include <vector>
#include <utility>

#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <rtl/alloc.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/EventObject.hpp>

#include <com/sun/star/lang/DisposedException.hpp>
#include <comphelper/comphelperdllapi.h>

/** */ //for docpp
namespace comphelper
{

namespace detail {

    /**
      This is here to optimise space in the common case that there are zero or one
      listeners.
    */
    union element_alias2
    {
        std::vector< css::uno::Reference< css::uno::XInterface > > *pAsVector;
        css::uno::XInterface * pAsInterface;
        element_alias2() : pAsInterface(nullptr) {}
    };

}


class OInterfaceContainerHelper2;
/**
  This is the iterator of a InterfaceContainerHelper. Typically
  one constructs an instance on the stack for one firing session.
  It is not allowed to assign or copy an instance of this class.

  @see OInterfaceContainerHelper
 */
class COMPHELPER_DLLPUBLIC OInterfaceIteratorHelper2
{
public:
    /**
       Create an iterator over the elements of the container. The iterator
       copies the elements of the container. A change to the container
       during the lifetime of an iterator is allowed and does not
       affect the iterator-instance. The iterator and the container take cares
       themself for concurrent access, no additional guarding is necessary.

       Remark: The copy is on demand. The iterator copy the elements only if the container
       change the contents. It is not allowed to destroy the container as long
       as an iterator exist.

       @param rCont the container of the elements.
     */
    OInterfaceIteratorHelper2( OInterfaceContainerHelper2 & rCont );

    /**
      Releases the connection to the container.
     */
    ~OInterfaceIteratorHelper2();

    /** Return true, if there are more elements in the iterator. */
    bool hasMoreElements() const
        { return nRemain != 0; }
    /** Return the next element of the iterator. Calling this method if
        hasMoreElements() has returned false, is an error. Cast the
        returned pointer to the
     */
    css::uno::XInterface * next();

    /** Removes the current element (the last one returned by next())
        from the underlying container. Calling this method before
        next() has been called or calling it twice with no next()
        inbetween is an error.
    */
    void remove();

private:
    OInterfaceContainerHelper2 & rCont;
    bool const                   bIsList;
    detail::element_alias2       aData;
    sal_Int32                    nRemain;

    OInterfaceIteratorHelper2( const OInterfaceIteratorHelper2 & )
        SAL_DELETED_FUNCTION;
    OInterfaceIteratorHelper2 &  operator = ( const OInterfaceIteratorHelper2 & )
        SAL_DELETED_FUNCTION;
};


/**
  A container of interfaces. To access the elements use an iterator.
  This implementation is thread save.

  @see OInterfaceIteratorHelper
 */
class COMPHELPER_DLLPUBLIC OInterfaceContainerHelper2
{
public:
    // these are here to force memory de/allocation to sal lib.
    static void * operator new( size_t nSize )
        { return ::rtl_allocateMemory( nSize ); }
    static void operator delete( void * pMem )
        { ::rtl_freeMemory( pMem ); }
    static void * operator new( size_t, void * pMem )
        { return pMem; }
    static void operator delete( void *, void * )
        {}

    /**
       Create an interface container.

       @param rMutex    the mutex to protect multi thread access.
       The lifetime must be longer than the lifetime
       of this object.
     */
    OInterfaceContainerHelper2( ::osl::Mutex & rMutex );
    /**
      Release all interfaces. All iterators must be destroyed before
      the container is destructed.
     */
    ~OInterfaceContainerHelper2();
    /**
      Return the number of Elements in the container. Only useful if you have acquired
      the mutex.
     */
    sal_Int32 getLength() const;

    /**
      Return all interfaces added to this container.
     **/
    std::vector< css::uno::Reference< css::uno::XInterface > > getElements() const;

    /** Inserts an element into the container.  The position is not specified, thus it is not
        specified in which order events are fired.

        @attention
        If you add the same interface more than once, then it will be added to the elements list
        more than once and thus if you want to remove that interface from the list, you have to call
        removeInterface() the same number of times.
        In the latter case, you will also get events fired more than once (if the interface is a
        listener interface).

        @param rxIFace
               interface to be added; it is allowed to insert null or
               the same interface more than once
        @return
                the new count of elements in the container
    */
    sal_Int32 addInterface( const css::uno::Reference< css::uno::XInterface > & rxIFace );
    /** Removes an element from the container.  It uses interface equality to remove the interface.

        @param rxIFace
               interface to be removed
        @return
                the new count of elements in the container
    */
    sal_Int32 removeInterface( const css::uno::Reference< css::uno::XInterface > & rxIFace );
    /**
      Call disposing on all object in the container that
      support XEventListener. Than clear the container.
     */
    void disposeAndClear( const css::lang::EventObject & rEvt );
    /**
      Clears the container without calling disposing().
     */
    void clear();

    /** Executes a functor for each contained listener of specified type, e.g.
        <code>forEach<awt::XPaintListener>(...</code>.

        If a css::lang::DisposedException occurs which relates to
        the called listener, then that listener is removed from the container.

        @tparam ListenerT listener type
        @tparam FuncT unary functor type, let your compiler deduce this for you
        @param func unary functor object expecting an argument of type
                    css::uno::Reference<ListenerT>
    */
    template <typename ListenerT, typename FuncT>
    inline void forEach( FuncT const& func );

    /** Calls a UNO listener method for each contained listener.

        The listener method must take a single argument of type EventT,
        and return <code>void</code>.

        If a css::lang::DisposedException occurs which relates to
        the called listener, then that listener is removed from the container.

        @tparam ListenerT UNO event listener type, let your compiler deduce this for you
        @tparam EventT event type, let your compiler deduce this for you
        @param NotificationMethod
            Pointer to a method of a ListenerT interface.
        @param Event
            Event to notify to all contained listeners

        Example:
@code
    awt::PaintEvent aEvent( static_cast< cppu::OWeakObject* >( this ), ... );
    listeners.notifyEach( &XPaintListener::windowPaint, aEvent );
@endcode
    */
    template< typename ListenerT, typename EventT >
    inline void notifyEach( void ( SAL_CALL ListenerT::*NotificationMethod )( const EventT& ), const EventT& Event );

private:
friend class OInterfaceIteratorHelper2;
    /**
      bIsList == TRUE -> aData.pAsVector of type vector< XInterfaceSequence >,
      otherwise aData.pAsInterface == of type (XEventListener *)
     */
    detail::element_alias2  aData;
    ::osl::Mutex &          rMutex;
    /** TRUE -> used by an iterator. */
    bool                    bInUse;
    /** TRUE -> aData.pAsVector is of type Sequence< XInterfaceSequence >. */
    bool                    bIsList;

    OInterfaceContainerHelper2( const OInterfaceContainerHelper2 & )
        SAL_DELETED_FUNCTION;
    OInterfaceContainerHelper2 & operator = ( const OInterfaceContainerHelper2 & )
        SAL_DELETED_FUNCTION;

    /*
      Duplicate content of the container and release the old one without destroying.
      The mutex must be locked and the memberbInUse must be true.
     */
    void copyAndResetInUse();

private:
    template< typename ListenerT, typename EventT >
    class NotifySingleListener
    {
    private:
        typedef void ( SAL_CALL ListenerT::*NotificationMethod )( const EventT& );
        NotificationMethod const m_pMethod;
        const EventT&            m_rEvent;
    public:
        NotifySingleListener( NotificationMethod method, const EventT& event ) : m_pMethod( method ), m_rEvent( event ) { }

        void operator()( const css::uno::Reference<ListenerT>& listener ) const
        {
            (listener.get()->*m_pMethod)( m_rEvent );
        }
    };
};

template <typename ListenerT, typename FuncT>
inline void OInterfaceContainerHelper2::forEach( FuncT const& func )
{
    OInterfaceIteratorHelper2 iter( *this );
    while (iter.hasMoreElements()) {
        css::uno::Reference<ListenerT> const xListener( iter.next(), css::uno::UNO_QUERY );
        if (xListener.is()) {
            try {
                func( xListener );
            }
            catch (css::lang::DisposedException const& exc) {
                if (exc.Context == xListener)
                    iter.remove();
            }
        }
    }
}

template< typename ListenerT, typename EventT >
inline void OInterfaceContainerHelper2::notifyEach( void ( SAL_CALL ListenerT::*NotificationMethod )( const EventT& ), const EventT& Event )
{
    forEach< ListenerT, NotifySingleListener< ListenerT, EventT > >( NotifySingleListener< ListenerT, EventT >( NotificationMethod, Event ) );
}

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
