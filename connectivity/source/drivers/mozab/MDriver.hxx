/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MDriver.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 06:18:42 $
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
#ifndef CONNECTIVITY_SDRIVER_HXX
#define CONNECTIVITY_SDRIVER_HXX

#ifndef _COM_SUN_STAR_SDBC_XDRIVER_HPP_
#include <com/sun/star/sdbc/XDriver.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "connectivity/CommonTools.hxx"
#endif
#ifndef _OSL_MODULE_H_
#include <osl/module.h>
#endif


#define MOZAB_MOZILLA_SCHEMA    "mozilla"
#define MOZAB_THUNDERBIRD_SCHEMA "thunderbird"
#define MOZAB_LDAP_SCHEMA       "ldap"
#define MOZAB_OUTLOOK_SCHEMA    "outlook"
#define MOZAB_OUTLOOKEXP_SCHEMA "outlookexp"
#define MOZAB_DRIVER_IMPL_NAME "com.sun.star.comp.sdbc.MozabDriver"

namespace connectivity
{
    namespace mozab
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL MozabDriver_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception );

        typedef void* (SAL_CALL * OMozabConnection_CreateInstanceFunction)(void* _pDriver );
        typedef void  (SAL_CALL * OSetMozabServiceFactory)( void* _pFactory );


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

            ::osl::Mutex                m_aMutex;       // mutex is need to control member access
            connectivity::OWeakRefArray m_xConnections; //  vector containing a list
                                                        //  of all the Connection objects
                                                        //  for this Driver
            oslModule                   s_hModule;
            OMozabConnection_CreateInstanceFunction s_pCreationFunc;
            void registerClient();
            virtual ~MozabDriver();
        public:

            MozabDriver(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory);

            // OComponentHelper
            virtual void SAL_CALL disposing(void);
            // XInterface
            static ::rtl::OUString getImplementationName_Static(  ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(  ) throw (::com::sun::star::uno::RuntimeException);

            // XServiceInfo
            virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

            // XDriver
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL connect( const ::rtl::OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL acceptsURL( const ::rtl::OUString& url ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sdbc::DriverPropertyInfo > SAL_CALL getPropertyInfo( const ::rtl::OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getMajorVersion(  ) throw(::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getMinorVersion(  ) throw(::com::sun::star::uno::RuntimeException);

            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        & getMSFactory(void) const { return m_xMSFactory; }

            static EDriverType acceptsURL_Stat( const ::rtl::OUString& url );
            // static methods to return the names of the uri
            static const sal_Char*    getSDBC_SCHEME_MOZILLA();
            static const sal_Char*    getSDBC_SCHEME_THUNDERBIRD();
            static const sal_Char*    getSDBC_SCHEME_LDAP();
            static const sal_Char*    getSDBC_SCHEME_OUTLOOK_MAPI();
            static const sal_Char*    getSDBC_SCHEME_OUTLOOK_EXPRESS();
        };
    }

}

#endif // CONNECTIVITY_SDRIVER_HXX
