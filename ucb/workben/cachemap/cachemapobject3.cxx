/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cachemapobject3.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 14:09:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

#ifndef INCLUDED_UCB_CACHEMAPOBJECT3_HXX
#include "cachemapobject3.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include "osl/diagnose.h"
#endif
#ifndef _OSL_INTERLOCK_H_
#include "osl/interlck.h"
#endif
#ifndef _OSL_MUTEX_HXX_
#include "osl/mutex.hxx"
#endif
#ifndef _RTL_REF_HXX_
#include "rtl/ref.hxx"
#endif
#ifndef _RTL_USTRING_HXX_
#include "rtl/ustring.hxx"
#endif

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif

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
    if (osl_decrementInterlockedCount(&m_nRefCount) == 0)
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
    else if (osl_incrementInterlockedCount(&aIt->second->m_nRefCount) > 1)
    {
        rtl::Reference< Object3 > xElement(aIt->second);
        osl_decrementInterlockedCount(&aIt->second->m_nRefCount);
        return xElement;
    }
    else
    {
        osl_decrementInterlockedCount(&aIt->second->m_nRefCount);
        aIt->second->m_aContainerIt = m_aMap.end();
        aIt->second = new Object3(this);
        aIt->second->m_aContainerIt = aIt;
        return aIt->second;
    }
}
