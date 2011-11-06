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



#include "precompiled_sd.hxx"

#include "UpdateRequest.hxx"

#include "framework/FrameworkHelper.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

using ::rtl::OUString;

namespace sd { namespace framework {

UpdateRequest::UpdateRequest (void)
    throw()
    : UpdateRequestInterfaceBase(MutexOwner::maMutex)
{
}




UpdateRequest::~UpdateRequest (void) throw()
{
}




void SAL_CALL UpdateRequest::execute (const Reference<XConfiguration>& rxConfiguration)
    throw (RuntimeException)
{
    (void)rxConfiguration;
    // Do nothing here.  The configuration is updated when the request queue
    // becomes empty.
}




OUString SAL_CALL UpdateRequest::getName (void)
    throw (RuntimeException)
{
    return OUString::createFromAscii("UpdateRequest");
}




void SAL_CALL UpdateRequest::setName (const OUString& rsName)
    throw (RuntimeException)
{
    (void)rsName;
    // Ignored.
}

} } // end of namespace sd::framework

