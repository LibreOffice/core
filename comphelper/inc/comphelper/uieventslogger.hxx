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



#ifndef _COMPHELPER_UIEVENTSLOGGER_HXX
#define _COMPHELPER_UIEVENTSLOGGER_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/util/URL.hpp>
#include <comphelper/comphelperdllapi.h>
#include <rtl/ustring.hxx>


namespace comphelper
{
    // this class is part of the OOo Improvement Program Core
    class COMPHELPER_DLLPUBLIC UiEventsLogger
    {
        public:
            static sal_Bool isEnabled();
            static sal_Int32 getSessionLogEventCount();
            static void appendDispatchOrigin( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& args, const ::rtl::OUString& originapp, const ::rtl::OUString& originwidget);
            static ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue> purgeDispatchOrigin(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& args);
            static void logDispatch(const ::com::sun::star::util::URL& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& args);
            static void logVcl(const ::rtl::OUString& parent_id, sal_Int32 window_type, const ::rtl::OUString& id, const ::rtl::OUString& method, const ::rtl::OUString& param);
            static void logVcl(const ::rtl::OUString& parent_id, sal_Int32 window_type, const ::rtl::OUString& id, const ::rtl::OUString& method, sal_Int32 param);
            static void logVcl(const ::rtl::OUString& parent_id, sal_Int32 window_type, const ::rtl::OUString& id, const ::rtl::OUString& method);
            static void disposing();
            static void reinit();
            virtual ~UiEventsLogger() {}
    };
}
#endif
