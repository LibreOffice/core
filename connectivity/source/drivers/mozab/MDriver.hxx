/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef CONNECTIVITY_SDRIVER_HXX
#define CONNECTIVITY_SDRIVER_HXX

#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/compbase2.hxx>
#include "connectivity/CommonTools.hxx"
#include <osl/module.h>


#define MOZAB_DRIVER_IMPL_NAME "com.sun.star.comp.sdbc.MozabDriver"

namespace connectivity
{
    namespace mozab
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL MozabDriver_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception );

        typedef       void* (SAL_CALL * OMozabConnection_CreateInstanceFunction)(void* _pDriver );
        typedef       void  (SAL_CALL * OSetMozabServiceFactory)( void* _pFactory );
        typedef const void* (SAL_CALL * OGetSdbcScheme_Function)( short );


        typedef ::cppu::WeakComponentImplHelper2<   ::com::sun::star::sdbc::XDriver,
                                                    ::com::sun::star::lang::XServiceInfo > ODriver_BASE;

        enum EDriverType
        {
            Mozilla,
            ThunderBird,
            LDAP,
            Outlook,
            OutlookExpress,
            Unknown
        };

        class MozabDriver : public ODriver_BASE
        {
        protected:
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xMSFactory;

            ::osl::Mutex                            m_aMutex;       // mutex is need to control member access
            connectivity::OWeakRefArray             m_xConnections; //  vector containing a list
                                                                    //  of all the Connection objects
                                                                    //  for this Driver
            oslModule                               m_hModule;
            OMozabConnection_CreateInstanceFunction m_pCreationFunc;
            OGetSdbcScheme_Function                 m_pSchemeFunction;

            bool ensureInit();
            virtual ~MozabDriver();
        public:

            MozabDriver(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory);

            // OComponentHelper
            virtual void SAL_CALL disposing(void);
            // XInterface
            static OUString getImplementationName_Static(  ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_Static(  ) throw (::com::sun::star::uno::RuntimeException);

            // XServiceInfo
            virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

            // XDriver
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL connect( const OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL acceptsURL( const OUString& url ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sdbc::DriverPropertyInfo > SAL_CALL getPropertyInfo( const OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getMajorVersion(  ) throw(::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getMinorVersion(  ) throw(::com::sun::star::uno::RuntimeException);

            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        & getMSFactory(void) const { return m_xMSFactory; }

        private:
            EDriverType impl_classifyURL( const OUString& url );
        };
    }

}

#endif // CONNECTIVITY_SDRIVER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
