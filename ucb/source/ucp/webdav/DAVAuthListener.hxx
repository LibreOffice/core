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


#ifndef _DAVAUTHLISTENER_HXX_
#define _DAVAUTHLISTENER_HXX_

#include <salhelper/simplereferenceobject.hxx>
#include <rtl/ustring.hxx>

#include <com/sun/star/uno/XReference.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>

namespace webdav_ucp
{

class DAVAuthListener : public salhelper::SimpleReferenceObject
{
    public:
        virtual int authenticate(
            const ::rtl::OUString & inRealm,
            const ::rtl::OUString & inHostName,
            ::rtl::OUString & inoutUserName,
            ::rtl::OUString & outPassWord,
            sal_Bool bCanUseSystemCredentials ) = 0;
};

} // namespace webdav_ucp

#endif // _DAVAUTHLISTENER_HXX_
