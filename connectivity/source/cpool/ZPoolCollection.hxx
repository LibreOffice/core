/*************************************************************************
 *
 *  $RCSfile: ZPoolCollection.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:38:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef CONNECTIVITY_POOLCOLLECTION_HXX
#define CONNECTIVITY_POOLCOLLECTION_HXX

#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase5.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDRIVERMANAGER_HPP_
#include <com/sun/star/sdbc/XDriverManager.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDRIVER_HPP_
#include <com/sun/star/sdbc/XDriver.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDRIVERACCESS_HPP_
#include <com/sun/star/sdbc/XDriverAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XPOOLEDCONNECTION_HPP_
#include <com/sun/star/sdbc/XPooledConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XTERMINATELISTENER_HPP_
#include <com/sun/star/frame/XTerminateListener.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_XPROXYFACTORY_HPP_
#include <com/sun/star/reflection/XProxyFactory.hpp>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

namespace connectivity
{
    class OConnectionPool;
    //==========================================================================
    //= OPoolCollection - the one-instance service for PooledConnections
    //= manages the active connections and the connections in the pool
    //==========================================================================
    typedef ::cppu::WeakImplHelper5<    ::com::sun::star::sdbc::XDriverManager,
                                        ::com::sun::star::sdbc::XDriverAccess,
                                        ::com::sun::star::lang::XServiceInfo,
                                        ::com::sun::star::frame::XTerminateListener,
                                        ::com::sun::star::beans::XPropertyChangeListener
                                        >   OPoolCollection_Base;

    /// OPoolCollection: controll the whole connection pooling for oo
    class OPoolCollection : public OPoolCollection_Base
    {

        //==========================================================================
        typedef ::comphelper::OInterfaceCompare< ::com::sun::star::sdbc::XDriver >  ODriverCompare;
        DECLARE_STL_USTRINGACCESS_MAP(OConnectionPool*, OConnectionPools);

        DECLARE_STL_MAP(
                ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver >,
                ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XDriver >,
                ODriverCompare,
                MapDriver2DriverRef );

        MapDriver2DriverRef                                                                 m_aDriverProxies;
        ::osl::Mutex                                                                        m_aMutex;
        OConnectionPools                                                                    m_aPools;          // the driver pools
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xServiceFactory;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriverManager >          m_xManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriverAccess >           m_xDriverAccess;
        ::com::sun::star::uno::Reference< ::com::sun::star::reflection::XProxyFactory >     m_xProxyFactory;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >               m_xConfigNode;      // config node for generel connection pooling
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDesktop>                m_xDesktop;

    private:
        OPoolCollection(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory);

        // some configuration helper methods
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createWithServiceFactory(const ::rtl::OUString& _rPath) const;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getConfigPoolRoot();
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createWithProvider(   const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxConfProvider,
                                                                                                    const ::rtl::OUString& _rPath) const;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > openNode( const ::rtl::OUString& _rPath,
                                                                                        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xTreeNode) const throw();
        sal_Bool isPoolingEnabled();
        sal_Bool isDriverPoolingEnabled(const ::rtl::OUString& _sDriverImplName,
                                        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxDriverNode);
        sal_Bool isPoolingEnabledByUrl( const ::rtl::OUString& _sUrl,
                                        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver >& _rxDriver,
                                        ::rtl::OUString& _rsImplName,
                                        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxDriverNode);

        OConnectionPool* getConnectionPool( const ::rtl::OUString& _sImplName,
                                            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver >& _xDriver,
                                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxDriverNode);
        void clearConnectionPools(sal_Bool _bDispose);
    protected:
        virtual ~OPoolCollection();
    public:

        static ::com::sun::star::uno::Any getNodeValue( const ::rtl::OUString& _rPath,
                                                        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& _xTreeNode)throw();

    // XDriverManager
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnection( const ::rtl::OUString& url ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnectionWithInfo( const ::rtl::OUString& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setLoginTimeout( sal_Int32 seconds ) throw(::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getLoginTimeout(  ) throw(::com::sun::star::uno::RuntimeException);

    //XDriverAccess
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver > SAL_CALL getDriverByURL( const ::rtl::OUString& url ) throw (::com::sun::star::uno::RuntimeException);
    // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo - static methods
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL CreateInstance(const::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);
        static ::rtl::OUString SAL_CALL getImplementationName_Static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_Static(  ) throw(::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw (::com::sun::star::uno::RuntimeException);

        // XTerminateListener
        virtual void SAL_CALL queryTermination( const ::com::sun::star::lang::EventObject& Event ) throw (::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL notifyTermination( const ::com::sun::star::lang::EventObject& Event ) throw (::com::sun::star::uno::RuntimeException);
    };
}
#endif // CONNECTIVITY_POOLCOLLECTION_HXX

