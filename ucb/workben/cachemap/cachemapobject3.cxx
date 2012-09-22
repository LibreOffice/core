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

#include "cachemapobject3.hxx"
#include "osl/diagnose.h"
#include "osl/interlck.h"
#include "osl/mutex.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"

#include <memory>

using ucb::cachemap::Object3;
using ucb::cachemap::ObjectContainer3;

inline
Object3::Object3(rtl::Reference< ObjectContainer3 > const & rContainer):
    m_xContainer(rContainer),
    m_nRefCount(0)
{
    OSL_ASSERT(m_xContainer.is());
}

inline Object3::~Object3() SAL_THROW(())
{}

void Object3::release() SAL_THROW(())
{
    if (osl_atomic_decrement(&m_nRefCount) == 0)
    {
        m_xContainer->releaseElement(this);
        delete this;
    }
}

void ObjectContainer3::releaseElement(Object3 * pElement) SAL_THROW(())
{
    OSL_ASSERT(pElement);
    osl::MutexGuard aGuard(m_aMutex);
    if (pElement->m_aContainerIt != m_aMap.end())
        m_aMap.erase(pElement->m_aContainerIt);
}

ObjectContainer3::ObjectContainer3()
{}

ObjectContainer3::~ObjectContainer3() SAL_THROW(())
{}

rtl::Reference< Object3 > ObjectContainer3::get(rtl::OUString const & rKey)
{
    osl::MutexGuard aGuard(m_aMutex);
    Map::iterator aIt(m_aMap.find(rKey));
    if (aIt == m_aMap.end())
    {
        std::auto_ptr< Object3 > xElement(new Object3(this));
        aIt = m_aMap.insert(Map::value_type(rKey, xElement.get())).first;
        aIt->second->m_aContainerIt = aIt;
        xElement.release();
        return aIt->second;
    }
    else if (osl_atomic_increment(&aIt->second->m_nRefCount) > 1)
    {
        rtl::Reference< Object3 > xElement(aIt->second);
        osl_atomic_decrement(&aIt->second->m_nRefCount);
        return xElement;
    }
    else
    {
        osl_atomic_decrement(&aIt->second->m_nRefCount);
        aIt->second->m_aContainerIt = m_aMap.end();
        aIt->second = new Object3(this);
        aIt->second->m_aContainerIt = aIt;
        return aIt->second;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
