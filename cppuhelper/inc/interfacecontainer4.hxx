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
#include <o3tl/cow_wrapper.hxx>
#include <cassert>
#include <mutex>
#include <vector>

namespace com::sun::star::uno
{
class XInterface;
}

/**
This is a straight copy of the include/comphelper/interfacecontainer4.hxx file, copied here
because it is nigh impossible to move shared code down into the URE layer.
*/

namespace cppuhelper
{
template <class ListenerT> class OInterfaceContainerHelper4;
/**
  This is the iterator of an OInterfaceContainerHelper4. Typically
  one constructs an instance on the stack for one firing session.
  It is not allowed to assign or copy an instance of this class.

  @tparam ListenerT UNO event listener type
  @see OInterfaceContainerHelper4
 */
template <class ListenerT> class OInterfaceIteratorHelper4
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
       @param rGuard
            this parameter only here to make that this container is accessed while locked
     */
    OInterfaceIteratorHelper4(std::unique_lock<std::mutex>& rGuard,
                              OInterfaceContainerHelper4<ListenerT>& rCont_)
        : m_rCont(rCont_)
        , m_aData(m_rCont.maData)
        // const_cast so we don't trigger make_unique via o3tl::cow_wrapper::operator->
        , m_nRemain(std::as_const(m_aData)->size())
    {
        assert(rGuard.owns_lock());
        (void)rGuard;
    }

    /** Return true, if there are more elements in the iterator. */
    bool hasMoreElements() const { return m_nRemain != 0; }
    /** Return the next element of the iterator. Calling this method if
        hasMoreElements() has returned false, is an error.
     */
    css::uno::Reference<ListenerT> const& next();

    /** Removes the current element (the last one returned by next())
        from the underlying container. Calling this method before
        next() has been called or calling it twice with no next()
        in between is an error.
        @param rGuard
            this parameter only here to make that this container is accessed while locked
    */
    void remove(::std::unique_lock<::std::mutex>& rGuard);

private:
    OInterfaceContainerHelper4<ListenerT>& m_rCont;
    o3tl::cow_wrapper<std::vector<css::uno::Reference<ListenerT>>,
                      o3tl::ThreadSafeRefCountingPolicy>
        m_aData;
    sal_Int32 m_nRemain;

    OInterfaceIteratorHelper4(const OInterfaceIteratorHelper4&) = delete;
    OInterfaceIteratorHelper4& operator=(const OInterfaceIteratorHelper4&) = delete;
};

template <class ListenerT>
const css::uno::Reference<ListenerT>& OInterfaceIteratorHelper4<ListenerT>::next()
{
    m_nRemain--;
    return (*std::as_const(m_aData))[m_nRemain];
}

template <class ListenerT>
void OInterfaceIteratorHelper4<ListenerT>::remove(::std::unique_lock<::std::mutex>& rGuard)
{
    m_rCont.removeInterface(rGuard, (*std::as_const(m_aData))[m_nRemain]);
}

/**
  A container of interfaces. To access the elements use an iterator.
  This implementation is thread-safe.

  This is a copy of the code at include/comphelper/interfacecontainer3.hxx,
  except that it (a) uses std::mutex instead of osl::Mutex and (b) does not
  store a reference to the mutex, but relies on the calling class to take
  a lock around using it.

  @tparam ListenerT UNO event listener type
  @see OInterfaceIteratorHelper
 */
template <class ListenerT> class OInterfaceContainerHelper4
{
public:
    OInterfaceContainerHelper4();

    /**
      Return the number of Elements in the container. Only useful if you have acquired
      the mutex.
      @param rGuard
        this parameter only here to make that this container is accessed while locked
     */
    sal_Int32 getLength(std::unique_lock<std::mutex>& rGuard) const;

    /**
      Return all interfaces added to this container.
      @param rGuard
          this parameter only here to make that this container is accessed while locked
     **/
    std::vector<css::uno::Reference<ListenerT>>
    getElements(std::unique_lock<std::mutex>& rGuard) const;

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
        @param rGuard
            this parameter only here to make that this container is accessed while locked
        @return
                the new count of elements in the container
    */
    sal_Int32 addInterface(std::unique_lock<std::mutex>& rGuard,
                           const css::uno::Reference<ListenerT>& rxIFace);
    /** Removes an element from the container.  It uses interface equality to remove the interface.

        @param rxIFace
               interface to be removed
        @param rGuard
               this parameter only here to make that this container is accessed while locked
        @return
                the new count of elements in the container
    */
    sal_Int32 removeInterface(std::unique_lock<std::mutex>& rGuard,
                              const css::uno::Reference<ListenerT>& rxIFace);
    /**
      Call disposing on all object in the container that
      support XEventListener. Then clear the container.
      The guard is unlock()'ed before calling the listeners.
     */
    void disposeAndClear(::std::unique_lock<::std::mutex>& rGuard,
                         const css::lang::EventObject& rEvt);
    /**
      Clears the container without calling disposing().
        @param rGuard
            this parameter only here to make that this container is accessed while locked
     */
    void clear(::std::unique_lock<::std::mutex>& rGuard);

    /** Executes a functor for each contained listener of specified type, e.g.
        <code>forEach<awt::XPaintListener>(...</code>.

        If a css::lang::DisposedException occurs which relates to
        the called listener, then that listener is removed from the container.

        @tparam FuncT unary functor type, let your compiler deduce this for you
        @param func unary functor object expecting an argument of type
                    css::uno::Reference<ListenerT>
        @param rGuard
            this parameter only here to make that this container is accessed while locked
    */
    template <typename FuncT>
    inline void forEach(std::unique_lock<std::mutex>& rGuard, FuncT const& func) const;

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
        @param rGuard
            this parameter only here to make that this container is accessed while locked

        Example:
@code
    awt::PaintEvent aEvent( static_cast< cppu::OWeakObject* >( this ), ... );
    listeners.notifyEach( &XPaintListener::windowPaint, aEvent );
@endcode
    */
    template <typename EventT>
    inline void notifyEach(std::unique_lock<std::mutex>& rGuard,
                           void (SAL_CALL ListenerT::*NotificationMethod)(const EventT&),
                           const EventT& Event) const;

    // this is moveable, but not copyable
    OInterfaceContainerHelper4(OInterfaceContainerHelper4&&) = default;
    OInterfaceContainerHelper4& operator=(OInterfaceContainerHelper4&&) = default;

private:
    friend class OInterfaceIteratorHelper4<ListenerT>;
    o3tl::cow_wrapper<std::vector<css::uno::Reference<ListenerT>>,
                      o3tl::ThreadSafeRefCountingPolicy>
        maData;
    OInterfaceContainerHelper4(const OInterfaceContainerHelper4&) = delete;
    OInterfaceContainerHelper4& operator=(const OInterfaceContainerHelper4&) = delete;

    static o3tl::cow_wrapper<std::vector<css::uno::Reference<ListenerT>>,
                             o3tl::ThreadSafeRefCountingPolicy>&
    DEFAULT()
    {
        static o3tl::cow_wrapper<std::vector<css::uno::Reference<ListenerT>>,
                                 o3tl::ThreadSafeRefCountingPolicy>
            SINGLETON;
        return SINGLETON;
    }

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
            assert(m_pMethod);
        }

        void operator()(const css::uno::Reference<ListenerT>& listener) const
        {
            (listener.get()->*m_pMethod)(m_rEvent);
        }
    };
};

template <class T>
inline OInterfaceContainerHelper4<T>::OInterfaceContainerHelper4()
    : maData(OInterfaceContainerHelper4<T>::DEFAULT())
{
}

template <class T>
template <typename FuncT>
inline void OInterfaceContainerHelper4<T>::forEach(std::unique_lock<std::mutex>& rGuard,
                                                   FuncT const& func) const
{
    assert(rGuard.owns_lock());
    if (std::as_const(maData)->size() == 0)
    {
        return;
    }
    const_cast<OInterfaceContainerHelper4&>(*this)
        .maData.make_unique(); // so we can iterate over the data without holding the lock
    OInterfaceIteratorHelper4<T> iter(rGuard, const_cast<OInterfaceContainerHelper4&>(*this));
    rGuard.unlock();
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
            {
                rGuard.lock();
                iter.remove(rGuard);
                rGuard.unlock();
            }
        }
    }
    rGuard.lock();
}

template <class ListenerT>
template <typename EventT>
inline void OInterfaceContainerHelper4<ListenerT>::notifyEach(
    std::unique_lock<std::mutex>& rGuard,
    void (SAL_CALL ListenerT::*NotificationMethod)(const EventT&), const EventT& Event) const
{
    forEach<NotifySingleListener<EventT>>(rGuard,
                                          NotifySingleListener<EventT>(NotificationMethod, Event));
}

template <class ListenerT>
sal_Int32
OInterfaceContainerHelper4<ListenerT>::getLength(std::unique_lock<std::mutex>& rGuard) const
{
    assert(rGuard.owns_lock());
    (void)rGuard;
    return maData->size();
}

template <class ListenerT>
std::vector<css::uno::Reference<ListenerT>>
OInterfaceContainerHelper4<ListenerT>::getElements(std::unique_lock<std::mutex>& rGuard) const
{
    assert(rGuard.owns_lock());
    (void)rGuard;
    return *maData;
}

template <class ListenerT>
sal_Int32
OInterfaceContainerHelper4<ListenerT>::addInterface(std::unique_lock<std::mutex>& rGuard,
                                                    const css::uno::Reference<ListenerT>& rListener)
{
    assert(rGuard.owns_lock());
    (void)rGuard;
    assert(rListener.is());
    maData->push_back(rListener);
    return std::as_const(maData)->size();
}

template <class ListenerT>
sal_Int32 OInterfaceContainerHelper4<ListenerT>::removeInterface(
    std::unique_lock<std::mutex>& rGuard, const css::uno::Reference<ListenerT>& rListener)
{
    assert(rGuard.owns_lock());
    (void)rGuard;
    assert(rListener.is());

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

    return std::as_const(maData)->size();
}

template <class ListenerT>
void OInterfaceContainerHelper4<ListenerT>::disposeAndClear(std::unique_lock<std::mutex>& rGuard,
                                                            const css::lang::EventObject& rEvt)
{
    {
        OInterfaceIteratorHelper4<ListenerT> aIt(rGuard, *this);
        maData
            = DEFAULT(); // cheaper than calling maData->clear() because it doesn't allocate a new vector
        rGuard.unlock();
        // unlock followed by iterating is only safe because we are not going to call remove() on the iterator
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
    // tdf#152077 need to destruct the OInterfaceIteratorHelper4 before we take the lock again
    // because there is a vague chance that destructing it will trigger a call back into something
    // that wants to take the lock.
    rGuard.lock();
}

template <class ListenerT>
void OInterfaceContainerHelper4<ListenerT>::clear(::std::unique_lock<::std::mutex>& rGuard)
{
    assert(rGuard.owns_lock());
    (void)rGuard;
    maData->clear();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
