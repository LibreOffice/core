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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"
#include "cachemapobjectcontainer2.hxx"
#include "cachemapobject2.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/XWeak.hpp"
#include "cppuhelper/weakref.hxx"
#include "osl/mutex.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"

using ucb::cachemap::Object2;
using ucb::cachemap::ObjectContainer2;
using namespace com::sun::star;

ObjectContainer2::ObjectContainer2()
{}

ObjectContainer2::~ObjectContainer2() SAL_THROW(())
{}

rtl::Reference< Object2 > ObjectContainer2::get(rtl::OUString const & rKey)
{
    rtl::Reference< Object2 > xElement;
    {
        osl::MutexGuard aGuard(m_aMutex);
        Map::iterator aIt(m_aMap.find(rKey));
        if (aIt != m_aMap.end())
            xElement = static_cast< Object2 * >(
                           uno::Reference< uno::XWeak >(
                                   aIt->second.get(), uno::UNO_QUERY).
                               get());
        if (!xElement.is())
        {
            xElement = new Object2;
            m_aMap[rKey]
                = uno::WeakReference< Object2 >(xElement.get());
        }
    }
    return xElement;
}
