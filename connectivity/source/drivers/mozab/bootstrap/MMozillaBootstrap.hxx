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
#ifndef CONNECTIVITY_SMOZILLABOOTSTRAP_HXX
#define CONNECTIVITY_SMOZILLABOOTSTRAP_HXX

#include <com/sun/star/mozilla/XMozillaBootstrap.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/compbase2.hxx>
#include <osl/module.h>


#define MOZAB_MozillaBootstrap_IMPL_NAME "com.sun.star.comp.mozilla.MozillaBootstrap"

namespace connectivity
{
    namespace mozab
    {
        typedef ::cppu::WeakComponentImplHelper2<   ::com::sun::star::mozilla::XMozillaBootstrap,
                                                    ::com::sun::star::lang::XServiceInfo > OMozillaBootstrap_BASE;
        class ProfileAccess;
        class ProfileManager;
        class MozillaBootstrap : public OMozillaBootstrap_BASE
        {
        protected:
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xMSFactory;

            ::osl::Mutex                m_aMutex;       // mutex is need to control member access
            virtual ~MozillaBootstrap();
        protected:
            ProfileAccess * m_ProfileAccess;
            ProfileManager  * m_ProfileManager;
        public:

            void Init();
            MozillaBootstrap(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory);

            // OComponentHelper
            virtual void SAL_CALL disposing(void) SAL_OVERRIDE;
            // XInterface
            static OUString getImplementationName_Static(  ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_Static(  ) throw (::com::sun::star::uno::RuntimeException);

            // XServiceInfo
            virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

            // XMozillaBootstrap

            // XProfileDiscover
            virtual ::sal_Int32 SAL_CALL getProfileCount( ::com::sun::star::mozilla::MozillaProductType product) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            virtual ::sal_Int32 SAL_CALL getProfileList( ::com::sun::star::mozilla::MozillaProductType product, ::com::sun::star::uno::Sequence< OUString >& list ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            virtual OUString SAL_CALL getDefaultProfile( ::com::sun::star::mozilla::MozillaProductType product ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            virtual OUString SAL_CALL getProfilePath( ::com::sun::star::mozilla::MozillaProductType product, const OUString& profileName ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            virtual sal_Bool SAL_CALL isProfileLocked( ::com::sun::star::mozilla::MozillaProductType product, const OUString& profileName ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            virtual sal_Bool SAL_CALL getProfileExists( ::com::sun::star::mozilla::MozillaProductType product, const OUString& profileName ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

            // XProfileManager
            virtual ::sal_Int32 SAL_CALL bootupProfile( ::com::sun::star::mozilla::MozillaProductType product, const OUString& profileName ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            virtual ::sal_Int32 SAL_CALL shutdownProfile(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            virtual ::com::sun::star::mozilla::MozillaProductType SAL_CALL getCurrentProduct(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            virtual OUString SAL_CALL getCurrentProfile(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            virtual sal_Bool SAL_CALL isCurrentProfileLocked(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            virtual OUString SAL_CALL setCurrentProfile( ::com::sun::star::mozilla::MozillaProductType product, const OUString& profileName ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

            // XProxyRunner
            virtual ::sal_Int32 SAL_CALL Run( const ::com::sun::star::uno::Reference< ::com::sun::star::mozilla::XCodeProxy >& aCode ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        & getMSFactory(void) const { return m_xMSFactory; }

        };
    }

}

#endif // CONNECTIVITY_SMozillaBootstrap_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
