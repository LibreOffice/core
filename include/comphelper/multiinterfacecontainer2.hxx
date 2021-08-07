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
#include <comphelper/interfacecontainer2.hxx>
#include <memory>
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
/**
  A helper class to store interface references of different types.
  This is a copy of the similiar class at include/cppuhelper/interfacecontainer.h,
  but now uses the improved comphelper::InterfaceContainer2.

  @see OInterfaceIteratorHelper2
  @see OInterfaceContainerHelper2
 */
template <class key, class hashImpl = void, class equalImpl = std::equal_to<key>>
class OMultiTypeInterfaceContainerHelperVar2
{
public:
    /**
      Create a container of interface containers.

      @param rMutex the mutex to protect multi thread access.
                         The lifetime must be longer than the lifetime
                         of this object.
     */
    inline OMultiTypeInterfaceContainerHelperVar2(::osl::Mutex& rMutex)
        : rMutex(rMutex)
    {
    }

    /**
      Return all id's under which at least one interface is added.
     */
    inline std::vector<key> getContainedTypes() const
    {
        ::osl::MutexGuard aGuard(rMutex);
        std::vector<key> aInterfaceTypes;
        aInterfaceTypes.reserve(m_aMap.size());
        for (const auto& rPair : m_aMap)
            // are interfaces added to this container?
            if (rPair.second->getLength())
                // yes, put the type in the array
                aInterfaceTypes.push_back(rPair.first);
        return aInterfaceTypes;
    }

    inline bool hasContainedTypes() const
    {
        ::osl::MutexGuard aGuard(rMutex);
        for (const auto& rPair : m_aMap)
            // are interfaces added to this container?
            if (rPair.second->getLength())
                return true;
        return false;
    }

    /**
      Return the container created under this key.
      The InterfaceContainerHelper exists until the whole MultiTypeContainer is destroyed.
      @return the container created under this key. If the container
                 was not created, null was returned.
     */
    inline OInterfaceContainerHelper2* getContainer(const key& rKey) const
    {
        ::osl::MutexGuard aGuard(rMutex);

        auto iter = find(rKey);
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
    inline sal_Int32 addInterface(const key& rKey,
                                  const css::uno::Reference<css::uno::XInterface>& rListener)
    {
        ::osl::MutexGuard aGuard(rMutex);
        auto iter = find(rKey);
        if (iter == m_aMap.end())
        {
            OInterfaceContainerHelper2* pLC = new OInterfaceContainerHelper2(rMutex);
            m_aMap.emplace_back(rKey, pLC);
            return pLC->addInterface(rListener);
        }
        else
            return (*iter).second->addInterface(rListener);
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
    inline sal_Int32 removeInterface(const key& rKey,
                                     const css::uno::Reference<css::uno::XInterface>& rListener)
    {
        ::osl::MutexGuard aGuard(rMutex);

        // search container with id nUik
        auto iter = find(rKey);
        // container found?
        if (iter != m_aMap.end())
            return (*iter).second->removeInterface(rListener);

        // no container with this id. Always return 0
        return 0;
    }

    /**
      Call disposing on all references in the container, that
      support XEventListener. Then clears the container.
      @param rEvt the event object which is passed during disposing() call
     */
    inline void disposeAndClear(const css::lang::EventObject& rEvt)
    {
        typename InterfaceMap::size_type nSize = 0;
        OInterfaceContainerHelper2** ppListenerContainers = nullptr;
        {
            ::osl::MutexGuard aGuard(rMutex);
            nSize = m_aMap.size();
            if (nSize)
            {
                typedef OInterfaceContainerHelper2* ppp;
                ppListenerContainers = new ppp[nSize];

                typename InterfaceMap::iterator iter = m_aMap.begin();
                typename InterfaceMap::iterator end = m_aMap.end();

                typename InterfaceMap::size_type i = 0;
                while (iter != end)
                {
                    ppListenerContainers[i++] = (*iter).second.get();
                    ++iter;
                }
            }
        }

        // create a copy, because do not fire event in a guarded section
        for (typename InterfaceMap::size_type i = 0; i < nSize; i++)
        {
            if (ppListenerContainers[i])
                ppListenerContainers[i]->disposeAndClear(rEvt);
        }

        delete[] ppListenerContainers;
    }

    /**
      Remove all elements of all containers. Does not delete the container.
     */
    inline void clear()
    {
        ::osl::MutexGuard aGuard(rMutex);

        for (const auto& rPair : m_aMap)
            rPair.second->clear();
    }

    typedef key keyType;

private:
    typedef ::std::vector<std::pair<key, std::unique_ptr<OInterfaceContainerHelper2>>> InterfaceMap;
    InterfaceMap m_aMap;
    ::osl::Mutex& rMutex;

    typename InterfaceMap::const_iterator find(const key& rKey) const
    {
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

    OMultiTypeInterfaceContainerHelperVar2(const OMultiTypeInterfaceContainerHelperVar2&) = delete;
    OMultiTypeInterfaceContainerHelperVar2& operator=(const OMultiTypeInterfaceContainerHelperVar2&)
        = delete;
};

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
