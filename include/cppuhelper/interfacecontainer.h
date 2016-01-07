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
#ifndef INCLUDED_CPPUHELPER_INTERFACECONTAINER_H
#define INCLUDED_CPPUHELPER_INTERFACECONTAINER_H

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
#include <cppuhelper/cppuhelperdllapi.h>

/** */ //for docpp
namespace cppu
{

namespace detail {

    /**
      This is here to optimise space in the common case that there are zero or one
      listeners.
    */
    union element_alias
    {
        css::uno::Sequence< css::uno::Reference< css::uno::XInterface > > *pAsSequence;
        css::uno::XInterface * pAsInterface;
        element_alias() : pAsInterface(NULL) {}
    };

}


class OInterfaceContainerHelper;
/**
  This is the iterator of a InterfaceContainerHelper. Typically
  one constructs an instance on the stack for one firing session.
  It is not allowed to assign or copy an instance of this class.

  @see OInterfaceContainerHelper
 */
class CPPUHELPER_DLLPUBLIC OInterfaceIteratorHelper
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
    OInterfaceIteratorHelper( OInterfaceContainerHelper & rCont );

    /**
      Releases the connection to the container.
     */
    ~OInterfaceIteratorHelper();

    /** Return true, if there are more elements in the iterator. */
    bool SAL_CALL hasMoreElements() const
        { return nRemain != 0; }
    /** Return the next element of the iterator. Calling this method if
        hasMoreElements() has returned false, is an error. Cast the
        returned pointer to the
     */
    css::uno::XInterface * SAL_CALL next();

    /** Removes the current element (the last one returned by next())
        from the underlying container. Calling this method before
        next() has been called or calling it twice with no next()
        inbetween is an error.
    */
    void SAL_CALL remove();

private:
    OInterfaceContainerHelper & rCont;
    sal_Bool                    bIsList;

    detail::element_alias aData;

    sal_Int32                   nRemain;

    OInterfaceIteratorHelper( const OInterfaceIteratorHelper & )
        SAL_DELETED_FUNCTION;
    OInterfaceIteratorHelper &  operator = ( const OInterfaceIteratorHelper & )
        SAL_DELETED_FUNCTION;
};


/**
  A container of interfaces. To access the elements use an iterator.
  This implementation is thread save.

  @see OInterfaceIteratorHelper
 */
class CPPUHELPER_DLLPUBLIC OInterfaceContainerHelper
{
public:
    // these are here to force memory de/allocation to sal lib.
    inline static void * SAL_CALL operator new( size_t nSize )
        { return ::rtl_allocateMemory( nSize ); }
    inline static void SAL_CALL operator delete( void * pMem )
        { ::rtl_freeMemory( pMem ); }
    inline static void * SAL_CALL operator new( size_t, void * pMem )
        { return pMem; }
    inline static void SAL_CALL operator delete( void *, void * )
        {}

    /**
       Create an interface container.

       @param rMutex    the mutex to protect multi thread access.
       The lifetime must be longer than the lifetime
       of this object.
     */
    OInterfaceContainerHelper( ::osl::Mutex & rMutex );
    /**
      Release all interfaces. All iterators must be destroyed before
      the container is destructed.
     */
    ~OInterfaceContainerHelper();
    /**
      Return the number of Elements in the container. Only useful if you have acquired
      the mutex.
     */
    sal_Int32 SAL_CALL getLength() const;

    /**
      Return all interfaces added to this container.
     **/
    css::uno::Sequence< css::uno::Reference< css::uno::XInterface > > SAL_CALL getElements() const;

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
    sal_Int32 SAL_CALL addInterface( const css::uno::Reference< css::uno::XInterface > & rxIFace );
    /** Removes an element from the container.  It uses interface equality to remove the interface.

        @param rxIFace
               interface to be removed
        @return
                the new count of elements in the container
    */
    sal_Int32 SAL_CALL removeInterface( const css::uno::Reference< css::uno::XInterface > & rxIFace );
    /**
      Call disposing on all object in the container that
      support XEventListener. Than clear the container.
     */
    void SAL_CALL disposeAndClear( const css::lang::EventObject & rEvt );
    /**
      Clears the container without calling disposing().
     */
    void SAL_CALL clear();

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
friend class OInterfaceIteratorHelper;
    /**
      bIsList == TRUE -> aData.pAsSequence of type Sequence< XInterfaceSequence >,
      otherwise aData.pAsInterface == of type (XEventListener *)
     */
    detail::element_alias   aData;
    ::osl::Mutex &          rMutex;
    /** TRUE -> used by an iterator. */
    sal_Bool                bInUse;
    /** TRUE -> aData.pAsSequence is of type Sequence< XInterfaceSequence >. */
    sal_Bool                bIsList;

    OInterfaceContainerHelper( const OInterfaceContainerHelper & )
        SAL_DELETED_FUNCTION;
    OInterfaceContainerHelper & operator = ( const OInterfaceContainerHelper & )
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
        NotificationMethod  m_pMethod;
        const EventT&       m_rEvent;
    public:
        NotifySingleListener( NotificationMethod method, const EventT& event ) : m_pMethod( method ), m_rEvent( event ) { }

        void operator()( const css::uno::Reference<ListenerT>& listener ) const
        {
            (listener.get()->*m_pMethod)( m_rEvent );
        }
    };
};

template <typename ListenerT, typename FuncT>
inline void OInterfaceContainerHelper::forEach( FuncT const& func )
{
    OInterfaceIteratorHelper iter( *this );
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
inline void OInterfaceContainerHelper::notifyEach( void ( SAL_CALL ListenerT::*NotificationMethod )( const EventT& ), const EventT& Event )
{
    forEach< ListenerT, NotifySingleListener< ListenerT, EventT > >( NotifySingleListener< ListenerT, EventT >( NotificationMethod, Event ) );
}


/**
  A helper class to store interface references of different types.

  @see OInterfaceIteratorHelper
  @see OInterfaceContainerHelper
 */
template< class key, class hashImpl = void, class equalImpl = std::equal_to<key> >
class OMultiTypeInterfaceContainerHelperVar
{
public:
    // these are here to force memory de/allocation to sal lib.
    inline static void * SAL_CALL operator new( size_t nSize )
        { return ::rtl_allocateMemory( nSize ); }
    inline static void SAL_CALL operator delete( void * pMem )
        { ::rtl_freeMemory( pMem ); }
    inline static void * SAL_CALL operator new( size_t, void * pMem )
        { return pMem; }
    inline static void SAL_CALL operator delete( void *, void * )
        {}

    /**
      Create a container of interface containers.

      @param rMutex the mutex to protect multi thread access.
                         The lifetime must be longer than the lifetime
                         of this object.
     */
    inline OMultiTypeInterfaceContainerHelperVar( ::osl::Mutex & rMutex );
    /**
      Deletes all containers.
     */
    inline ~OMultiTypeInterfaceContainerHelperVar();

    /**
      Return all id's under which at least one interface is added.
     */
    inline css::uno::Sequence< key > SAL_CALL getContainedTypes() const;

    /**
      Return the container created under this key.
      The InterfaceContainerHelper exists until the whole MultiTypeContainer is destroyed.
      @return the container created under this key. If the container
                 was not created, null was returned.
     */
    inline OInterfaceContainerHelper * SAL_CALL getContainer( const key & ) const;

    /** Inserts an element into the container with the specified key.
        The position is not specified, thus it is not specified in which order events are fired.

        @attention
        If you add the same interface more than once, then it will be added to the elements list
        more than once and thus if you want to remove that interface from the list, you have to call
        removeInterface() the same number of times.
        In the latter case, you will also get events fired more than once (if the interface is a
        listener interface).

        @param rKey
               the id of the container
        @param r
               interface to be added; it is allowed, to insert null or
               the same interface more than once
        @return
                the new count of elements in the container
    */
    inline sal_Int32 SAL_CALL addInterface(
        const key & rKey,
        const css::uno::Reference< css::uno::XInterface > & r );

    /** Removes an element from the container with the specified key.
        It uses interface equality to remove the interface.

        @param rKey
               the id of the container
        @param rxIFace
               interface to be removed
        @return
                the new count of elements in the container
    */
    inline sal_Int32 SAL_CALL removeInterface(
        const key & rKey,
        const css::uno::Reference< css::uno::XInterface > & rxIFace );

    /**
      Call disposing on all references in the container, that
      support XEventListener. Then clears the container.
      @param rEvt the event object which is passed during disposing() call
     */
    inline void SAL_CALL disposeAndClear( const css::lang::EventObject & rEvt );
    /**
      Remove all elements of all containers. Does not delete the container.
     */
    inline void SAL_CALL clear();

    typedef key keyType;
private:
    typedef ::std::vector< std::pair < key , void* > > InterfaceMap;
    InterfaceMap *m_pMap;
    ::osl::Mutex &  rMutex;

    inline typename InterfaceMap::iterator find(const key &rKey) const
    {
        typename InterfaceMap::iterator iter = m_pMap->begin();
        typename InterfaceMap::iterator end = m_pMap->end();

        while( iter != end )
        {
            equalImpl equal;
            if( equal( iter->first, rKey ) )
                break;
            iter++;
        }
        return iter;
    }

    OMultiTypeInterfaceContainerHelperVar( const OMultiTypeInterfaceContainerHelperVar & ) SAL_DELETED_FUNCTION;
    OMultiTypeInterfaceContainerHelperVar & operator = ( const OMultiTypeInterfaceContainerHelperVar & ) SAL_DELETED_FUNCTION;
};




/**
  This struct contains the standard variables of a broadcaster. Helper
  classes only know a reference to this struct instead of references
  to the four members. The access to the members must be guarded with
  rMutex.

  The additional template parameter keyType has been added, because gcc
  can't compile addListener( const container::keyType &key ).
 */
template < class container , class keyType >
struct OBroadcastHelperVar
{
    /** The shared mutex. */
    ::osl::Mutex &                      rMutex;
    /** ListenerContainer class is thread safe. */
    container   aLC;
    /** Dispose call ready. */
    sal_Bool                            bDisposed;
    /** In dispose call. */
    sal_Bool                            bInDispose;

    /**
      Initialize the structure. bDispose and bInDispose are set to false.
      @param rMutex_ the mutex reference.
     */
    OBroadcastHelperVar( ::osl::Mutex & rMutex_ )
        : rMutex( rMutex_ )
        , aLC( rMutex_ )
        , bDisposed( sal_False )
        , bInDispose( sal_False )
    {}

    /**
      adds a listener threadsafe.
     **/
    inline void addListener(
        const keyType &key,
        const css::uno::Reference < css::uno::XInterface > &r )
    {
        ::osl::MutexGuard guard( rMutex );
        OSL_ENSURE( !bInDispose, "do not add listeners in the dispose call" );
        OSL_ENSURE( !bDisposed, "object is disposed" );
        if( ! bInDispose && ! bDisposed  )
            aLC.addInterface( key , r );
    }

    /**
      removes a listener threadsafe
     **/
    inline void removeListener(
        const keyType &key,
        const css::uno::Reference < css::uno::XInterface > & r )
    {
        ::osl::MutexGuard guard( rMutex );
        OSL_ENSURE( !bDisposed, "object is disposed" );
        if( ! bInDispose && ! bDisposed  )
            aLC.removeInterface( key , r );
    }

    /**
      Return the container created under this key.
      @return the container created under this key. If the container
                was not created, null was returned. This can be used to optimize
              performance ( construction of an event object can be avoided ).
     ***/
    inline OInterfaceContainerHelper * SAL_CALL getContainer( const keyType &key ) const
        { return aLC.getContainer( key ); }
};

/*------------------------------------------
*
* In general, the above templates are used with a Type as key.
* Therefore a default declaration is given ( OMultiTypeInterfaceContainerHelper and OBroadcastHelper )
*
*------------------------------------------*/

// helper function call class
struct hashType_Impl
{
    size_t operator()(const css::uno::Type & s) const
    { return (size_t) s.getTypeName().hashCode(); }
};


/** Specialized class for key type css::uno::Type,
    without explicit usage of STL symbols.
*/
class CPPUHELPER_DLLPUBLIC OMultiTypeInterfaceContainerHelper
{
public:
    // these are here to force memory de/allocation to sal lib.
    inline static void * SAL_CALL operator new( size_t nSize )
        { return ::rtl_allocateMemory( nSize ); }
    inline static void SAL_CALL operator delete( void * pMem )
        { ::rtl_freeMemory( pMem ); }
    inline static void * SAL_CALL operator new( size_t, void * pMem )
        { return pMem; }
    inline static void SAL_CALL operator delete( void *, void * )
        {}

    /**
      Create a container of interface containers.

      @param rMutex the mutex to protect multi thread access.
                         The lifetime must be longer than the lifetime
                         of this object.
     */
    OMultiTypeInterfaceContainerHelper( ::osl::Mutex & rMutex );
    /**
      Delete all containers.
     */
    ~OMultiTypeInterfaceContainerHelper();

    /**
      Return all id's under which at least one interface is added.
     */
    css::uno::Sequence< css::uno::Type > SAL_CALL getContainedTypes() const;

    /**
      Return the container created under this key.
      @return the container created under this key. If the container
                 was not created, null was returned.
     */
    OInterfaceContainerHelper * SAL_CALL getContainer( const css::uno::Type & rKey ) const;

    /** Inserts an element into the container with the specified key.
        The position is not specified, thus it is not specified in which order events are fired.

        @attention
        If you add the same interface more than once, then it will be added to the elements list
        more than once and thus if you want to remove that interface from the list, you have to call
        removeInterface() the same number of times.
        In the latter case, you will also get events fired more than once (if the interface is a
        listener interface).

        @param rKey
               the id of the container
        @param r
               interface to be added; it is allowed, to insert null or
               the same interface more than once
        @return
                the new count of elements in the container
    */
    sal_Int32 SAL_CALL addInterface(
        const css::uno::Type & rKey,
        const css::uno::Reference< css::uno::XInterface > & r );

    /** Removes an element from the container with the specified key.
        It uses interface equality to remove the interface.

        @param rKey
               the id of the container
        @param rxIFace
               interface to be removed
        @return
                the new count of elements in the container
    */
    sal_Int32 SAL_CALL removeInterface(
        const css::uno::Type & rKey,
        const css::uno::Reference< css::uno::XInterface > & rxIFace );

    /**
      Call disposing on all object in the container that
      support XEventListener. Than clear the container.
     */
    void SAL_CALL disposeAndClear( const css::lang::EventObject & rEvt );
    /**
      Remove all elements of all containers. Does not delete the container.
     */
    void SAL_CALL clear();

    typedef css::uno::Type keyType;
private:
    void *          m_pMap;
    ::osl::Mutex &  rMutex;

    OMultiTypeInterfaceContainerHelper( const OMultiTypeInterfaceContainerHelper & ) SAL_DELETED_FUNCTION;
    OMultiTypeInterfaceContainerHelper & operator = ( const OMultiTypeInterfaceContainerHelper & ) SAL_DELETED_FUNCTION;
};

typedef OBroadcastHelperVar< OMultiTypeInterfaceContainerHelper , OMultiTypeInterfaceContainerHelper::keyType > OBroadcastHelper;

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
