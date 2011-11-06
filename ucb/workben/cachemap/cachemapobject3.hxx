/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
