/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mdrivermanager.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 14:41:36 $
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

#ifndef _CONNECTIVITY_DRIVERMANAGER_HXX_
#define _CONNECTIVITY_DRIVERMANAGER_HXX_

#ifndef _COM_SUN_STAR_SDBC_XDRIVERMANAGER_HPP_
#include <com/sun/star/sdbc/XDriverManager.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XNAMINGSERVICE_HPP_
#include <com/sun/star/uno/XNamingService.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDRIVERACCESS_HPP_
#include <com/sun/star/sdbc/XDriverAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#include <cppuhelper/implbase5.hxx>
#include <comphelper/stl_types.hxx>
#include <comphelper/logging.hxx>
#include <comphelper/componentcontext.hxx>
#include <osl/mutex.hxx>

namespace drivermanager
{

    //======================================================================
    //= various
    //======================================================================
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver >                     SdbcDriver;
    DECLARE_STL_USTRINGACCESS_MAP( SdbcDriver, DriverCollection );

    typedef ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory >       DriverFactory;
    struct DriverAccess
    {
        ::rtl::OUString     sImplementationName;        /// the implementation name of the driver
        DriverFactory       xComponentFactory;          /// the factory to create the driver component (if not already done so)
        SdbcDriver          xDriver;                    /// the driver itself
    };

    //==========================================================================
    //= OSDBCDriverManager - the one-instance service for managing SDBC drivers
    //==========================================================================
    typedef ::cppu::WeakImplHelper5 <   ::com::sun::star::sdbc::XDriverManager
                                    ,   ::com::sun::star::sdbc::XDriverAccess
                                    ,   ::com::sun::star::container::XEnumerationAccess
                                    ,   ::com::sun::star::lang::XServiceInfo
                                    ,   ::com::sun::star::uno::XNamingService
                                    >   OSDBCDriverManager_Base;

    class OSDBCDriverManager : public OSDBCDriverManager_Base
    {
        friend class ODriverEnumeration;

        ::osl::Mutex                    m_aMutex;
        ::comphelper::ComponentContext  m_aContext;
        ::comphelper::EventLogger       m_aEventLogger;

        DECLARE_STL_VECTOR(DriverAccess, DriverAccessArray);
        DriverAccessArray       m_aDriversBS;

        // for drivers registered at runtime (not bootstrapped) we don't require an XServiceInfo interface,
        // so we have to remember their impl-name in another way
        DECLARE_STL_USTRINGACCESS_MAP(SdbcDriver, DriverCollection);
        DriverCollection                m_aDriversRT;

        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                                m_xServiceFactory;
        sal_Int32               m_nLoginTimeout;

    private:
        OSDBCDriverManager(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext );
        ~OSDBCDriverManager();

    public:

    // XDriverManager
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnection( const ::rtl::OUString& url ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnectionWithInfo( const ::rtl::OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setLoginTimeout( sal_Int32 seconds ) throw(::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getLoginTimeout(  ) throw(::com::sun::star::uno::RuntimeException);

    // XDriverAccess
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver > SAL_CALL getDriverByURL( const ::rtl::OUString& url ) throw(::com::sun::star::uno::RuntimeException);

    // XEnumerationAccess
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createEnumeration(  ) throw(::com::sun::star::uno::RuntimeException);

    // XElementAccess
        virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo - static methods
        static ::rtl::OUString SAL_CALL getImplementationName_static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::rtl::OUString SAL_CALL getSingletonName_static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL Create( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxContext );

    // XNamingService
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getRegisteredObject( const ::rtl::OUString& Name ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL registerObject( const ::rtl::OUString& Name, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Object ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL revokeObject( const ::rtl::OUString& Name ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver > implGetDriverForURL(const ::rtl::OUString& _rURL);

        /** retrieve the driver order preferences from the configuration and
            sort m_aDriversBS accordingly.
        */
        void initializeDriverPrecedence();

        void bootstrapDrivers();
    };

}   // namespace drivermanager

#endif // _CONNECTIVITY_DRIVERMANAGER_HXX_


