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



#ifndef __FRAMEWORK_CONFIG_CONFIGACCESS_HXX_
#define __FRAMEWORK_CONFIG_CONFIGACCESS_HXX_

//_______________________________________
// my own includes

#include <threadhelp/threadhelpbase.hxx>
#include <macros/debug.hxx>
#include <general.h>

//_______________________________________
// interface includes
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

//_______________________________________
// other includes
#include <rtl/ustring.hxx>
#include <fwidllapi.h>

//_______________________________________
// namespace

namespace framework{

//_______________________________________
// public const

//_______________________________________
/**
    @short  implements a simple configuration access
    @descr  Sometimes it's better to have direct config access
            instead of using soecialize config items of the svtools
            project. This class can wrapp such configuration access.
 */
class FWI_DLLPUBLIC ConfigAccess : public ThreadHelpBase
{
    //___________________________________
    // const

    public:

        /** represent the possible modes of the internal wrapped configuration access */
        enum EOpenMode
        {
            /// config isn't used yet
            E_CLOSED,
            /// config access is open for reading only
            E_READONLY,
            /// config access is open for reading/writing data
            E_READWRITE
        };

    //___________________________________
    // member

    private:

        /**
            reference to the uno service manager
            It's necessary to instanciate own needed services.
         */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        /** hold an opened configuration alive */
        css::uno::Reference< css::uno::XInterface > m_xConfig;

        /** knows the root of the opened config access point */
        ::rtl::OUString m_sRoot;

        /** represent the current open mode */
        EOpenMode m_eMode;

    //___________________________________
    // native interface methods

    public:

                 ConfigAccess( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR ,
                               const ::rtl::OUString&                                        sRoot );
        virtual ~ConfigAccess();

        virtual void      open   ( EOpenMode eMode );
        virtual void      close  (                 );
        virtual EOpenMode getMode(                 ) const;

        virtual const css::uno::Reference< css::uno::XInterface >& cfg();
};

} // namespace framework

#endif // __FRAMEWORK_CONFIG_CONFIGACCESS_HXX_
