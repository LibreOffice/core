/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cachemapobject1.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 16:19:30 $
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

#ifndef INCLUDED_UCB_CACHEMAPOBJECT1_HXX
#define INCLUDED_UCB_CACHEMAPOBJECT1_HXX

#ifndef _OSL_INTERLOCK_H_
#include "osl/interlck.h"
#endif
#ifndef _OSL_MUTEX_HXX_
#include "osl/mutex.hxx"
#endif
#ifndef _RTL_REF_HXX_
#include "rtl/ref.hxx"
#endif
#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#ifndef _SALHELPER_SIMPLEREFERENCEOBJECT_HXX_
#include "salhelper/simplereferenceobject.hxx"
#endif

#ifndef INCLUDED_MAP
#include <map>
#define INCLUDED_MAP
#endif
#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif

namespace rtl { class OUString; }
namespace ucb { namespace cachemap { class Object1; } }

namespace ucb { namespace cachemap {

class ObjectContainer1: public salhelper::SimpleReferenceObject
{
public:
    ObjectContainer1();

    virtual ~ObjectContainer1() SAL_THROW(());

    rtl::Reference< Object1 > get(rtl::OUString const & rKey);

private:
    typedef std::map< rtl::OUString, Object1 * > Map;

    Map m_aMap;
    osl::Mutex m_aMutex;

    void releaseElement(Object1 * pElement) SAL_THROW(());

    friend class Object1; // to access Map, releaseElement()
};

class Object1
{
public:
    inline void acquire() SAL_THROW(())
    { osl_incrementInterlockedCount(&m_nRefCount); }

    inline void release() SAL_THROW(())
    { m_xContainer->releaseElement(this); }

private:
    rtl::Reference< ObjectContainer1 > m_xContainer;
    ObjectContainer1::Map::iterator m_aContainerIt;
    oslInterlockedCount m_nRefCount;

    inline Object1(rtl::Reference< ObjectContainer1 > const & rContainer);

    inline ~Object1() SAL_THROW(());

    Object1(Object1 &); // not implemented
    void operator =(Object1); // not implemented

    friend class ObjectContainer1;
        // to access m_aContainerIt, m_nRefCount, Object1(), ~Object1()
#if defined WNT
    friend struct std::auto_ptr< Object1 >; // to access ~Object1()
        // work around compiler bug...
#else // WNT
    friend class std::auto_ptr< Object1 >; // to access ~Object1()
#endif // WNT
};

} }

#endif // INCLUDED_UCB_CACHEMAPOBJECT1_HXX
