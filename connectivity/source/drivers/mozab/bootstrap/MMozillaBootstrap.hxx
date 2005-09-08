/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MMozillaBootstrap.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 06:22:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef CONNECTIVITY_SMOZILLABOOTSTRAP_HXX
#define CONNECTIVITY_SMOZILLABOOTSTRAP_HXX

#ifndef _COM_SUN_STAR_MOZILLA_XMOZILLABOOTSTRAP_HPP_
#include <com/sun/star/mozilla/XMozillaBootstrap.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif
#ifndef _OSL_MODULE_H_
#include <osl/module.h>
#endif


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
            virtual void SAL_CALL disposing(void);
            // XInterface
            static ::rtl::OUString getImplementationName_Static(  ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(  ) throw (::com::sun::star::uno::RuntimeException);

            // XServiceInfo
            virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

            // XMozillaBootstrap

            // XProfileDiscover
            virtual ::sal_Int32 SAL_CALL getProfileCount( ::com::sun::star::mozilla::MozillaProductType product) throw (::com::sun::star::uno::RuntimeException);
            virtual ::sal_Int32 SAL_CALL getProfileList( ::com::sun::star::mozilla::MozillaProductType product, ::com::sun::star::uno::Sequence< ::rtl::OUString >& list ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::rtl::OUString SAL_CALL getDefaultProfile( ::com::sun::star::mozilla::MozillaProductType product ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::rtl::OUString SAL_CALL getProfilePath( ::com::sun::star::mozilla::MozillaProductType product, const ::rtl::OUString& profileName ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::sal_Bool SAL_CALL isProfileLocked( ::com::sun::star::mozilla::MozillaProductType product, const ::rtl::OUString& profileName ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::sal_Bool SAL_CALL getProfileExists( ::com::sun::star::mozilla::MozillaProductType product, const ::rtl::OUString& profileName ) throw (::com::sun::star::uno::RuntimeException);

            // XProfileManager
            virtual ::sal_Int32 SAL_CALL bootupProfile( ::com::sun::star::mozilla::MozillaProductType product, const ::rtl::OUString& profileName ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::sal_Int32 SAL_CALL shutdownProfile(  ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::mozilla::MozillaProductType SAL_CALL getCurrentProduct(  ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::rtl::OUString SAL_CALL getCurrentProfile(  ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::sal_Bool SAL_CALL isCurrentProfileLocked(  ) throw (::com::sun::star::uno::RuntimeException);
            virtual ::rtl::OUString SAL_CALL setCurrentProfile( ::com::sun::star::mozilla::MozillaProductType product, const ::rtl::OUString& profileName ) throw (::com::sun::star::uno::RuntimeException);

            // XProxyRunner
            virtual ::sal_Int32 SAL_CALL Run( const ::com::sun::star::uno::Reference< ::com::sun::star::mozilla::XCodeProxy >& aCode ) throw (::com::sun::star::uno::RuntimeException);

            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        & getMSFactory(void) const { return m_xMSFactory; }

        };
    }

}

#endif // CONNECTIVITY_SMozillaBootstrap_HXX
