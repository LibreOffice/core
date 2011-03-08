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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"
#include "cachemapobject1.hxx"
#include "osl/diagnose.h"
#include "osl/interlck.h"
#include "osl/mutex.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif

using ucb::cachemap::Object1;
using ucb::cachemap::ObjectContainer1;

inline
Object1::Object1(rtl::Reference< ObjectContainer1 > const & rContainer):
    m_xContainer(rContainer),
    m_nRefCount(0)
{
    OSL_ASSERT(m_xContainer.is());
}

inline Object1::~Object1() SAL_THROW(())
{}

void ObjectContainer1::releaseElement(Object1 * pElement) SAL_THROW(())
{
    OSL_ASSERT(pElement);
    bool bDelete = false;
    {
        osl::MutexGuard aGuard(m_aMutex);
        if (osl_decrementInterlockedCount(&pElement->m_nRefCount) == 0)
        {
            m_aMap.erase(pElement->m_aContainerIt);
            bDelete = true;
        }
    }
    if (bDelete)
        delete pElement;
}

ObjectContainer1::ObjectContainer1()
{}

ObjectContainer1::~ObjectContainer1() SAL_THROW(())
{}

rtl::Reference< Object1 > ObjectContainer1::get(rtl::OUString const & rKey)
{
    osl::MutexGuard aGuard(m_aMutex);
    Map::iterator aIt(m_aMap.find(rKey));
    if (aIt == m_aMap.end())
    {
        std::auto_ptr< Object1 > xElement(new Object1(this));
        aIt = m_aMap.insert(Map::value_type(rKey, xElement.get())).first;
        aIt->second->m_aContainerIt = aIt;
        xElement.release();
    }
    return aIt->second;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
