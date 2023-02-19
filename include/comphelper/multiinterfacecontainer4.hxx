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
#include <comphelper/interfacecontainer4.hxx>
#include <memory>
#include <mutex>
#include <vector>
/** */ //for docpp
namespace comphelper
{
/**
  A helper class to store interface references of different types.
  This is a copy of the code at include/comphelper/multiinterfacecontainer3.hxx,
  except that it (a) uses std::mutex instead of osl::Mutex and (b) does not
  store a reference to the mutex, but relies on the calling class to take
  a lock around using it.
  @see OInterfaceIteratorHelper3
  @see OInterfaceContainerHelper3
 */
template <class key, class listener, class equalImpl = std::equal_to<key>>
class OMultiTypeInterfaceContainerHelperVar4
{
public:
    OMultiTypeInterfaceContainerHelperVar4() {}
    /**
      Return all id's under which at least one interface is added.
     */
    inline std::vector<key> getContainedTypes(std::unique_lock<std::mutex>& rGuard) const
    {
        assert(rGuard.owns_lock());
        std::vector<key> aInterfaceTypes;
        aInterfaceTypes.reserve(m_aMap.size());
        for (const auto& rPair : m_aMap)
            // are interfaces added to this container?
            if (rPair.second->getLength(rGuard))
                // yes, put the type in the array
                aInterfaceTypes.push_back(rPair.first);
        return aInterfaceTypes;
    }
    inline bool hasContainedTypes(std::unique_lock<std::mutex>& rGuard) const
    {
        assert(rGuard.owns_lock());
        for (const auto& rPair : m_aMap)
            // are interfaces added to this container?
            if (rPair.second->getLength(rGuard))
                return true;
        return false;
    }
    /**
      Return the container created under this key.
      The InterfaceContainerHelper exists until the whole MultiTypeContainer is destroyed.
      @return the container created under this key. If the container
                 was not created, null was returned.
     */
    inline OInterfaceContainerHelper4<listener>* getContainer(std::unique_lock<std::mutex>& rGuard,
                                                              const key& rKey) const
    {
        auto iter = find(rGuard, rKey);
        if (iter != m_aMap.end())
            return (*iter).second.get();
        return nullptr;
    }
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
    inline sal_Int32 addInterface(::std::unique_lock<::std::mutex>& rGuard, const key& rKey,
                                  const css::uno::Reference<listener>& rListener)
    {
        auto iter = find(rGuard, rKey);
        if (iter == m_aMap.end())
        {
            auto pLC = new OInterfaceContainerHelper4<listener>();
            m_aMap.emplace_back(rKey, pLC);
            return pLC->addInterface(rGuard, rListener);
        }
        else
            return (*iter).second->addInterface(rGuard, rListener);
    }
    /** Removes an element from the container with the specified key.
        It uses interface equality to remove the interface.
        @param rKey
               the id of the container
        @param rxIFace
               interface to be removed
        @return
                the new count of elements in the container
    */
    inline sal_Int32 removeInterface(::std::unique_lock<::std::mutex>& rGuard, const key& rKey,
                                     const css::uno::Reference<listener>& rListener)
    {
        // search container with id nUik
        auto iter = find(rGuard, rKey);
        // container found?
        if (iter != m_aMap.end())
            return (*iter).second->removeInterface(rGuard, rListener);
        // no container with this id. Always return 0
        return 0;
    }
    /**
      Call disposing on all references in the container, that
      support XEventListener. Then clears the container.
      @param rEvt the event object which is passed during disposing() call
     */
    inline void disposeAndClear(std::unique_lock<std::mutex>& rGuard,
                                const css::lang::EventObject& rEvt)
    {
        assert(rGuard.owns_lock());
        // create a copy, because do not fire event in a guarded section
        InterfaceMap tempMap;
        {
            tempMap = std::move(m_aMap);
        }
        rGuard.unlock();
        // So... we don't want to hold the normal mutex while we fire
        // the events, but the calling convention here wants a mutex, so
        // just create a temporary/fake one. Since the listeners we
        // are working with are now function-local, we don't really need
        // a mutex at all, but it's easier to create a fake one than
        // create a bunch of special-case code for this situation.
        std::mutex tempMutex;
        std::unique_lock tempGuard(tempMutex);
        for (auto& rPair : tempMap)
        {
            OInterfaceIteratorHelper4<listener> aIt(tempGuard, *rPair.second);
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
        rGuard.lock(); // return with lock in same state as entry
    }
    /**
      Remove all elements of all containers. Does not delete the container.
     */
    inline void clear(std::unique_lock<std::mutex>& rGuard)
    {
        assert(rGuard.owns_lock());
        (void)rGuard;
        for (const auto& rPair : m_aMap)
            rPair.second->clear();
    }
    typedef key keyType;

private:
    typedef ::std::vector<std::pair<key, std::unique_ptr<OInterfaceContainerHelper4<listener>>>>
        InterfaceMap;
    InterfaceMap m_aMap;
    typename InterfaceMap::const_iterator find(std::unique_lock<std::mutex>& rGuard,
                                               const key& rKey) const
    {
        assert(rGuard.owns_lock());
        (void)rGuard;
        auto iter = m_aMap.begin();
        auto end = m_aMap.end();
        while (iter != end)
        {
            equalImpl equal;
            if (equal(iter->first, rKey))
                break;
            ++iter;
        }
        return iter;
    }
    OMultiTypeInterfaceContainerHelperVar4(const OMultiTypeInterfaceContainerHelperVar4&) = delete;
    OMultiTypeInterfaceContainerHelperVar4& operator=(const OMultiTypeInterfaceContainerHelperVar4&)
        = delete;
};
} // namespace comphelper
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
