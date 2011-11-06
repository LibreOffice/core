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



#ifndef __FRAMEWORK_HELPER_CONFIGIMPORTER_HXX_
#define __FRAMEWORK_HELPER_CONFIGIMPORTER_HXX_

#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <rtl/ustring.hxx>
#include <framework/fwedllapi.h>

namespace framework
{
    class FWE_DLLPUBLIC UIConfigurationImporterOOo1x
    {
        public:
            static sal_Bool ImportCustomToolbars(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIConfigurationManager >& rContainerFactory,
                        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > >& rSeqContainer,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rServiceManager,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& rToolbarStorage );
    };

} // namespace framework

#endif // __FRAMEWORK_HELPER_CONFIGIMPORTER_HXX_
