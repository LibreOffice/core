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

#include <sal/config.h>

#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <comphelper/comphelperdllapi.h>
#include <o3tl/cow_wrapper.hxx>
#include <vector>

namespace com::sun::star::uno
{
class XInterface;
}
namespace osl
{
class Mutex;
}

/** */ //for docpp
namespace comphelper
{
template <class ListenerT> class OInterfaceContainerHelper3;
/**
  This is the iterator of an OInterfaceContainerHelper3. Typically
  one constructs an instance on the stack for one firing session.
  It is not allowed to assign or copy an instance of this class.

  @tparam ListenerT UNO event listener type
  @see OInterfaceContainerHelper3
 */
template <class ListenerT> class OInterfaceIteratorHelper3
{
public:
    /**
       Create an iterator over the elements of the container. The iterator
       copies the elements of the container. A change to the container
       during the lifetime of an iterator is allowed and does not
       affect the iterator-instance. The iterator and the container take cares
       themself for concurrent access, no additional guarding is necessary.

       Remark: The copy is on demand. The iterator copy the elements only if the container
       change the contents...

       @param rCont the container of the elements.
     */
    OInterfaceIteratorHelper3(OInterfaceContainerHelper3<ListenerT>& rCont_)
        : rCont(rCont_)
        , maData(rCont.maData)
        , nRemain(maData->size())
    {
    }

    /** Return true, if there are more elements in the iterator. */
    bool hasMoreElements() const { return nRemain != 0; }
    /** Return the next element of the iterator. Calling this method if
        hasMoreElements() has returned false, is an error.
     */
    css::uno::Reference<ListenerT> const& next();

    /** Removes the current element (the last one returned by next())
        from the underlying container. Calling this method before
        next() has been called or calling it twice with no next()
        in between is an error.
    */
    void remove();

private:
    OInterfaceContainerHelper3<ListenerT>& rCont;
    o3tl::cow_wrapper<std::vector<css::uno::Reference<ListenerT>>,
                      o3tl::ThreadSafeRefCountingPolicy>
        maData;
    sal_Int32 nRemain;

    OInterfaceIteratorHelper3(const OInterfaceIteratorHelper3&) = delete;
    OInterfaceIteratorHelper3& operator=(const OInterfaceIteratorHelper3&) = delete;
};

template <class ListenerT>
const css::uno::Reference<ListenerT>& OInterfaceIteratorHelper3<ListenerT>::next()
{
    nRemain--;
    return (*maData)[nRemain];
}

template <class ListenerT> void OInterfaceIteratorHelper3<ListenerT>::remove()
{
    rCont.removeInterface((*maData)[nRemain]);
}

/**
  A container of interfaces. To access the elements use an iterator.
  This implementation is thread-safe.

  This is a copy of the code at include/comphelper/interfacecontainer2.hxx,
  except that it is templatized on the type of the listener, which allows
  some parts of the code to avoid doing an UNO_QUERY that can be expensive
  in bulk.

  @tparam ListenerT UNO event listener type
  @see OInterfaceIteratorHelper
 */
template <class ListenerT> class SAL_DLLPUBLIC_TEMPLATE OInterfaceContainerHelper3
{
public:
    /**
       Create an interface container.

       @param rMutex    the mutex to protect multi thread access.
       The lifetime must be longer than the lifetime
       of this object.
     */
    OInterfaceContainerHelper3(::osl::Mutex& rMutex_)
        : mrMutex(rMutex_)
    {
    }
    /**
      Return the number of Elements in the container. Only useful if you have acquired
      the mutex.
     */
    sal_Int32 getLength() const
    {
        osl::MutexGuard aGuard(mrMutex);
        return maData->size();
    }

    /**
      Return all interfaces added to this container.
     **/
    std::vector<css::uno::Reference<ListenerT>> getElements() const;

    /** Inserts an element into the container.  The position is not specified, thus it is not
        specified in which order events are fired.

        @attention
        If you add the same interface more than once, then it will be added to the elements list
        more than once and thus if you want to remove that interface from the list, you have to call
        removeInterface() the same number of times.
        In the latter case, you will also get events fired more than once (if the interface is a
        listener interface).

        @param rxIFace
               interface to be added; it is allowed to insert
               the same interface more than once
        @return
                the new count of elements in the container
    */
    sal_Int32 addInterface(const css::uno::Reference<ListenerT>& rxIFace);
    /** Removes an element from the container.  It uses interface equality to remove the interface.

        @param rxIFace
               interface to be removed
        @return
                the new count of elements in the container
    */
    sal_Int32 removeInterface(const css::uno::Reference<ListenerT>& rxIFace);
    /** Return an interface by index
    */
    const css::uno::Reference<ListenerT>& getInterface(sal_Int32 nIndex) const;
    /**
      Call disposing on all object in the container that
      support XEventListener. Then clear the container.
     */
    void disposeAndClear(const css::lang::EventObject& rEvt);
    /**
      Clears the container without calling disposing().
     */
    void clear();

    /** Executes a functor for each contained listener of specified type, e.g.
        <code>forEach<awt::XPaintListener>(...</code>.

        If a css::lang::DisposedException occurs which relates to
        the called listener, then that listener is removed from the container.

        @tparam FuncT unary functor type, let your compiler deduce this for you
        @param func unary functor object expecting an argument of type
                    css::uno::Reference<ListenerT>
    */
    template <typename FuncT> inline void forEach(FuncT const& func);

    /** Calls a UNO listener method for each contained listener.

        The listener method must take a single argument of type EventT,
        and return <code>void</code>.

        If a css::lang::DisposedException occurs which relates to
        the called listener, then that listener is removed from the container.

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
    template <typename EventT>
    inline void notifyEach(void (SAL_CALL ListenerT::*NotificationMethod)(const EventT&),
                           const EventT& Event);

private:
    friend class OInterfaceIteratorHelper3<ListenerT>;
    o3tl::cow_wrapper<std::vector<css::uno::Reference<ListenerT>>,
                      o3tl::ThreadSafeRefCountingPolicy>
        maData;
    ::osl::Mutex& mrMutex;
    OInterfaceContainerHelper3(const OInterfaceContainerHelper3&) = delete;
    OInterfaceContainerHelper3& operator=(const OInterfaceContainerHelper3&) = delete;

private:
    template <typename EventT> class NotifySingleListener
    {
    private:
        typedef void (SAL_CALL ListenerT::*NotificationMethod)(const EventT&);
        NotificationMethod const m_pMethod;
        const EventT& m_rEvent;

    public:
        NotifySingleListener(NotificationMethod method, const EventT& event)
            : m_pMethod(method)
            , m_rEvent(event)
        {
        }

        void operator()(const css::uno::Reference<ListenerT>& listener) const
        {
            (listener.get()->*m_pMethod)(m_rEvent);
        }
    };
};

template <class T>
template <typename FuncT>
inline void OInterfaceContainerHelper3<T>::forEach(FuncT const& func)
{
    OInterfaceIteratorHelper3<T> iter(*this);
    while (iter.hasMoreElements())
    {
        auto xListener = iter.next();
        try
        {
            func(xListener);
        }
        catch (css::lang::DisposedException const& exc)
        {
            if (exc.Context == xListener)
                iter.remove();
        }
    }
}

template <class ListenerT>
template <typename EventT>
inline void OInterfaceContainerHelper3<ListenerT>::notifyEach(
    void (SAL_CALL ListenerT::*NotificationMethod)(const EventT&), const EventT& Event)
{
    forEach<NotifySingleListener<EventT>>(NotifySingleListener<EventT>(NotificationMethod, Event));
}

template <class ListenerT>
std::vector<css::uno::Reference<ListenerT>>
OInterfaceContainerHelper3<ListenerT>::getElements() const
{
    std::vector<css::uno::Reference<ListenerT>> rVec;
    osl::MutexGuard aGuard(mrMutex);
    rVec = *maData;
    return rVec;
}

template <class ListenerT>
sal_Int32
OInterfaceContainerHelper3<ListenerT>::addInterface(const css::uno::Reference<ListenerT>& rListener)
{
    assert(rListener.is());
    osl::MutexGuard aGuard(mrMutex);

    maData->push_back(rListener);
    return maData->size();
}

template <class ListenerT>
sal_Int32 OInterfaceContainerHelper3<ListenerT>::removeInterface(
    const css::uno::Reference<ListenerT>& rListener)
{
    assert(rListener.is());
    osl::MutexGuard aGuard(mrMutex);

    // It is not valid to compare the pointer directly, but it's faster.
    auto it = std::find_if(maData->begin(), maData->end(),
                           [&rListener](const css::uno::Reference<css::uno::XInterface>& rItem) {
                               return rItem.get() == rListener.get();
                           });

    // interface not found, use the correct compare method
    if (it == maData->end())
        it = std::find(maData->begin(), maData->end(), rListener);

    if (it != maData->end())
        maData->erase(it);

    return maData->size();
}

template <class ListenerT>
const css::uno::Reference<ListenerT>&
OInterfaceContainerHelper3<ListenerT>::getInterface(sal_Int32 nIndex) const
{
    osl::MutexGuard aGuard(mrMutex);

    return (*maData)[nIndex];
}

template <class ListenerT>
void OInterfaceContainerHelper3<ListenerT>::disposeAndClear(const css::lang::EventObject& rEvt)
{
    osl::ClearableMutexGuard aGuard(mrMutex);
    OInterfaceIteratorHelper3<ListenerT> aIt(*this);
    maData->clear();
    aGuard.clear();
    while (aIt.hasMoreElements())
    {
        try
        {
            aIt.next()->disposing(rEvt);
        }
        catch (css::uno::RuntimeException&)
        {
            // be robust, if e.g. a remote bridge has disposed already.
            // there is no way to delegate the error to the caller :o(.
        }
    }
}

template <class ListenerT> void OInterfaceContainerHelper3<ListenerT>::clear()
{
    osl::MutexGuard aGuard(mrMutex);
    maData->clear();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
