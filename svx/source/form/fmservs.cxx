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
#include "precompiled_svx.hxx"
#include <com/sun/star/container/XSet.hpp>
#include <cppuhelper/factory.hxx>
#include <comphelper/processfactory.hxx>
#include "fmservs.hxx"

// ------------------------------------------------------------------------
#define DECL_SERVICE(ImplName)                      \
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ImplName##_NewInstance_Impl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > &) throw( ::com::sun::star::uno::Exception );

#define REGISTER_SERVICE(ImplName, ServiceName)                         \
    sString = (ServiceName);                                        \
    xSingleFactory = ::cppu::createSingleFactory(xServiceFactory,               \
                        ::rtl::OUString(), ImplName##_NewInstance_Impl,             \
                        ::com::sun::star::uno::Sequence< ::rtl::OUString>(&sString, 1));    \
    if (xSingleFactory.is())                                            \
        xSet->insert(::com::sun::star::uno::makeAny(xSingleFactory));


    DECL_SERVICE( FmXGridControl )
    DECL_SERVICE( FormController )
    DECL_SERVICE( LegacyFormController )


// ------------------------------------------------------------------------
namespace svxform
{

#define DECL_SELFAWARE_SERVICE( ClassName )                     \
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ClassName##_Create(      \
                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );  \
    ::rtl::OUString SAL_CALL ClassName##_GetImplementationName();                                           \
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL ClassName##_GetSupportedServiceNames();     \


#define REGISTER_SELFAWARE_SERVICE( ClassName )                     \
    xSingleFactory = ::cppu::createSingleFactory( xServiceFactory,  \
                        ClassName##_GetImplementationName(),        \
                        ClassName##_Create,                         \
                        ClassName##_GetSupportedServiceNames()      \
                     );                                             \
    if ( xSingleFactory.is() )                                      \
        xSet->insert( ::com::sun::star::uno::makeAny( xSingleFactory ) );


    // ------------------------------------------------------------------------
    DECL_SELFAWARE_SERVICE( OAddConditionDialog )

    // ------------------------------------------------------------------------
    void ImplSmartRegisterUnoServices()
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  xServiceFactory(::comphelper::getProcessServiceFactory(), ::com::sun::star::uno::UNO_QUERY);
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XSet >  xSet(xServiceFactory, ::com::sun::star::uno::UNO_QUERY);
        if (!xSet.is())
            return;

        ::com::sun::star::uno::Sequence< ::rtl::OUString> aServices;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory >  xSingleFactory;

        ::rtl::OUString sString;

        // ------------------------------------------------------------------------
        // FormController
        REGISTER_SERVICE( FormController, FM_FORM_CONTROLLER );
        REGISTER_SERVICE( LegacyFormController, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.FormController" ) ) );

        // ------------------------------------------------------------------------
        // FormController
        REGISTER_SELFAWARE_SERVICE( OAddConditionDialog );

        // ------------------------------------------------------------------------
        // DBGridControl
        REGISTER_SERVICE(FmXGridControl, FM_CONTROL_GRID);  // compatibility
        REGISTER_SERVICE(FmXGridControl, FM_CONTROL_GRIDCONTROL);
        REGISTER_SERVICE(FmXGridControl, FM_SUN_CONTROL_GRIDCONTROL);
    };

}   // namespace svxform
