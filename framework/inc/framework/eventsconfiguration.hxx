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



#ifndef __FRAMEWORK_XML_EVENTSCONFIGURATION_HXX_
#define __FRAMEWORK_XML_EVENTSCONFIGURATION_HXX_

#include <framework/fwedllapi.h>
#include <svl/svarray.hxx>
#include <tools/string.hxx>
#include <tools/stream.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

// #110897#
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

namespace framework
{

struct FWE_DLLPUBLIC EventsConfig
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString >              aEventNames;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >   aEventsProperties;
};

class FWE_DLLPUBLIC EventsConfiguration
{
    public:
        // #110897#
        static sal_Bool LoadEventsConfig(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
            SvStream& rInStream, EventsConfig& aItems );

        // #110897#
        static sal_Bool StoreEventsConfig(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
            SvStream& rOutStream, const EventsConfig& aItems );
};

} // namespace framework

#endif // __FRAMEWORK_XML_EVENTSCONFIGURATION_HXX_
