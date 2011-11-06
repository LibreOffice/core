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



#ifndef __FRAMEWORK_DISPATCH_UIEVENTLOGHELPER_HXX_
#define __FRAMEWORK_DISPATCH_UIEVENTLOGHELPER_HXX_

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/util/URL.hpp>
#include <comphelper/uieventslogger.hxx>
#include <rtl/ustring.hxx>
#include <services.h>

namespace framework
{
    class UiEventLogHelper
    {
        public:
            UiEventLogHelper(const ::rtl::OUString& aWidgetname)
                : m_aWidgetName(aWidgetname)
                , m_hasAppName(false)
            { }

            void log(const ::com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServiceManager,
                const ::com::sun::star::uno::Reference< com::sun::star::uno::XInterface >& rModel,
                const ::com::sun::star::util::URL& rUrl,
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& rArgs)
            {

                if(!m_hasAppName && rServiceManager.is() && rModel.is())
                {
                    try
                    {
                        static ::rtl::OUString our_aModuleManagerName = SERVICENAME_MODULEMANAGER;
                        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModuleManager > xModuleManager(
                            rServiceManager->createInstance(our_aModuleManagerName)
                            , ::com::sun::star::uno::UNO_QUERY_THROW);
                        m_aAppName = xModuleManager->identify(rModel);
                        m_hasAppName = true;
                    } catch(::com::sun::star::uno::Exception&) {}
                }
                ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue> aArgsWithOrigin(rArgs);
                ::comphelper::UiEventsLogger::appendDispatchOrigin(aArgsWithOrigin, m_aAppName, m_aWidgetName);
                ::comphelper::UiEventsLogger::logDispatch(rUrl, aArgsWithOrigin);
            }

        private:
            const ::rtl::OUString m_aWidgetName;
            bool m_hasAppName;
            ::rtl::OUString m_aAppName;
    };
}

#endif // __FRAMEWORK_DISPATCH_UIEVENTLOGHELPER_HXX_
