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



#ifndef __FRAMEWORK_ACCELERATORS_MODULEACCELERATORCONFIGURATION_HXX_
#define __FRAMEWORK_ACCELERATORS_MODULEACCELERATORCONFIGURATION_HXX_

//__________________________________________
// own includes

#include <accelerators/acceleratorconfiguration.hxx>
#include <accelerators/presethandler.hxx>

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/interface.hxx>
#endif
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>

//__________________________________________
// interface includes
#include <com/sun/star/lang/XInitialization.hpp>

//__________________________________________
// other includes

//__________________________________________
// definition

namespace framework
{

//__________________________________________
/**
    implements a read/write access to a module
    dependend accelerator configuration.
 */
class ModuleAcceleratorConfiguration : public XCUBasedAcceleratorConfiguration
                                     , public css::lang::XServiceInfo
                                     , public css::lang::XInitialization
{
    //______________________________________
    // member

    private:

        //----------------------------------
        /** identify the application module, where this accelerator
            configuration cache should work on. */
        ::rtl::OUString m_sModule;
        ::rtl::OUString m_sLocale;

    //______________________________________
    // interface

    public:

        //----------------------------------
        /** initialize this instance and fill the internal cache.

            @param  xSMGR
                    reference to an uno service manager, which is used internaly.
         */
        ModuleAcceleratorConfiguration(const css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR);

        //----------------------------------
        /** TODO */
        virtual ~ModuleAcceleratorConfiguration();

        // XInterface, XTypeProvider, XServiceInfo
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        // XInitialization
        virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any >& lArguments)
            throw(css::uno::Exception       ,
                  css::uno::RuntimeException);

    //______________________________________
    // helper

    private:

        //----------------------------------
        /** read all data into the cache. */
        void impl_ts_fillCache();
};

} // namespace framework

#endif // __FRAMEWORK_ACCELERATORS_MODULEACCELERATORCONFIGURATION_HXX_
