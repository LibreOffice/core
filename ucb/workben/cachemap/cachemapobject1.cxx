/*************************************************************************
 *
 *  $RCSfile: cachemapobject1.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sb $ $Date: 2001-06-11 13:03:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef INCLUDED_UCB_CACHEMAPOBJECT1_HXX
#include "cachemapobject1.hxx"
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
