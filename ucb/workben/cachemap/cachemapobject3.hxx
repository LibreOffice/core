/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cachemapobject3.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 16:20:26 $
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

#ifndef INCLUDED_UCB_CACHEMAPOBJECT3_HXX
#define INCLUDED_UCB_CACHEMAPOBJECT3_HXX

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
namespace ucb { namespace cachemap { class Object3; } }

namespace ucb { namespace cachemap {

class ObjectContainer3: public salhelper::SimpleReferenceObject
{
public:
    ObjectContainer3();

    virtual ~ObjectContainer3() SAL_THROW(());

    rtl::Reference< Object3 > get(rtl::OUString const & rKey);

private:
    typedef std::map< rtl::OUString, Object3 * > Map;

    Map m_aMap;
    osl::Mutex m_aMutex;

    void releaseElement(Object3 * pElement) SAL_THROW(());

    friend class Object3; // to access Map, releaseElement()
};

class Object3
{
public:
    inline void acquire() SAL_THROW(())
    { osl_incrementInterlockedCount(&m_nRefCount); }

    void release() SAL_THROW(());

private:
    rtl::Reference< ObjectContainer3 > m_xContainer;
    ObjectContainer3::Map::iterator m_aContainerIt;
    oslInterlockedCount m_nRefCount;

    inline Object3(rtl::Reference< ObjectContainer3 > const & rContainer);

    inline ~Object3() SAL_THROW(());

    Object3(Object3 &); // not implemented
    void operator =(Object3); // not implemented

    friend class ObjectContainer3;
        // to access m_aContainerIt, m_nRefCount, Object3(), ~Object3()
#if defined WNT
    friend struct std::auto_ptr< Object3 >; // to access ~Object3()
        // work around compiler bug...
#else // WNT
    friend class std::auto_ptr< Object3 >; // to access ~Object3()
#endif // WNT
};

} }

#endif // INCLUDED_UCB_CACHEMAPOBJECT3_HXX
