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



#ifndef SD_DRAW_SUB_CONTROLLER_HXX
#define SD_DRAW_SUB_CONTROLLER_HXX

#include <com/sun/star/drawing/XDrawSubController.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/compbase2.hxx>

namespace sd {

    class DrawSubControllerInterfaceBase : public ::cppu::WeakComponentImplHelper2<
                    ::com::sun::star::drawing::XDrawSubController,
                    ::com::sun::star::lang::XServiceInfo >
    {
    public:
        DrawSubControllerInterfaceBase( ::osl::Mutex& aMutex )
            :  ::cppu::WeakComponentImplHelper2<
                    ::com::sun::star::drawing::XDrawSubController,
                    ::com::sun::star::lang::XServiceInfo >( aMutex ) {}

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException) = 0;
        virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException) = 0;
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException) = 0;
    };

}


#endif

