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



#ifndef INCLUDED_UCB_CACHEMAPOBJECTCONTAINER2_HXX
#define INCLUDED_UCB_CACHEMAPOBJECTCONTAINER2_HXX

#include "cppuhelper/weakref.hxx"
#include "osl/mutex.hxx"
#include "rtl/ref.hxx"
#include "sal/types.h"

#ifndef INCLUDED_HASH_MAP
#include <hash_map>
#define INCLUDED_HASH_MAP
#endif

namespace rtl {
    class OUString;
    struct OUStringHash;
}
namespace ucb { namespace cachemap { class Object2; } }

namespace ucb { namespace cachemap {

class ObjectContainer2
{
public:
    ObjectContainer2();

    ~ObjectContainer2() SAL_THROW(());

    rtl::Reference< Object2 > get(rtl::OUString const & rKey);

private:
    typedef std::hash_map< rtl::OUString,
                           com::sun::star::uno::WeakReference< Object2 >,
                           rtl::OUStringHash >
    Map;

    ObjectContainer2(ObjectContainer2 &); // not implemented
    void operator =(ObjectContainer2); // not implemented

    Map m_aMap;
    osl::Mutex m_aMutex;
};

} }

#endif // INCLUDED_UCB_CACHEMAPOBJECTCONTAINER2_HXX
