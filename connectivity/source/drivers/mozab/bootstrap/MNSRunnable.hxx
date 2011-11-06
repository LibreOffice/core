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



#ifndef _CONNECTIVITY_MAB_MOZAB_RUNNABLE_HXX_
#define _CONNECTIVITY_MAB_MOZAB_RUNNABLE_HXX_
#include "MNSDeclares.hxx"
#include <osl/thread.hxx>

#include <MNSInclude.hxx>
#include <com/sun/star/uno/Reference.hxx>
#ifndef _COM_SUN_STAR_MOZILLA_XCODEPROXY_HDL_
#include <com/sun/star/mozilla/XCodeProxy.hpp>
#endif


using namespace com::sun::star::uno;
using namespace com::sun::star::mozilla;

namespace connectivity
{
    namespace mozab
    {
    class MNSRunnable : public nsIRunnable
        {
        public:
            nsIRunnable * ProxiedObject();
            MNSRunnable();
            virtual ~MNSRunnable();
            NS_DECL_ISUPPORTS
            NS_DECL_NSIRUNNABLE

        public:
            sal_Int32 StartProxy(const ::com::sun::star::uno::Reference< ::com::sun::star::mozilla::XCodeProxy >& aCode); //Call this to start proxy

        private:
            nsIRunnable* _ProxiedObject;
            Reference<XCodeProxy>  xCode;
#if OSL_DEBUG_LEVEL > 0
            oslThreadIdentifier m_oThreadID;
#endif
        };
    }
}
#endif //_CONNECTIVITY_MAB_MOZABHELPER_HXX_
