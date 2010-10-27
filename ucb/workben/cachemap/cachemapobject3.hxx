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

#ifndef INCLUDED_UCB_CACHEMAPOBJECT3_HXX
#define INCLUDED_UCB_CACHEMAPOBJECT3_HXX

#include "osl/interlck.h"
#include "osl/mutex.hxx"
#include "rtl/ref.hxx"
#include "sal/types.h"
#include "salhelper/simplereferenceobject.hxx"

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
