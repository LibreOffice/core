/*************************************************************************
 *
 *  $RCSfile: YDriver.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2002-11-11 08:30:12 $
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

#ifndef CONNECTIVITY_MYSQL_DRIVER_HXX
#include "mysql/YDriver.hxx"
#endif
#ifndef CONNECTIVITY_MYSQL_CATALOG_HXX
#include "mysql/YCatalog.hxx"
#endif
#ifndef CONNECTIVITY_MYSQL_CONNECTION_HXX
#include "mysql/YConnection.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include "connectivity/dbexception.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XDRIVERACCESS_HPP_
#include <com/sun/star/sdbc/XDriverAccess.hpp>
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif

//........................................................................
namespace connectivity
{
//........................................................................
    using namespace mysql;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;

    namespace mysql
    {
        Reference< XInterface >  SAL_CALL ODriverDelegator_CreateInstance(const Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFac) throw( Exception )
        {
            return *(new ODriverDelegator(_rxFac));
        }
    }


    //====================================================================
    //= ODriverDelegator
    //====================================================================
    //--------------------------------------------------------------------
    ODriverDelegator::ODriverDelegator(const Reference< XMultiServiceFactory >& _rxFactory)
        : ODriverDelegator_BASE(m_aMutex)
        ,m_xFactory(_rxFactory)
        ,m_bUseOdbc(sal_True)
    {
    }

    //--------------------------------------------------------------------
    ODriverDelegator::~ODriverDelegator()
    {
        ::comphelper::disposeComponent(m_xODBCDriver);
        ::comphelper::disposeComponent(m_xJDBCDriver);
    }

    // --------------------------------------------------------------------------------
    void ODriverDelegator::disposing()
    {
        ::osl::MutexGuard aGuard(m_aMutex);


        for (TWeakPairVector::iterator i = m_aConnections.begin(); m_aConnections.end() != i; ++i)
        {
            ::comphelper::disposeComponent(i->first.get());
        }
        m_aConnections.clear();
        TWeakPairVector().swap(m_aConnections);

        ODriverDelegator_BASE::disposing();
    }

    namespace
    {
        sal_Bool isOdbcPropertySet(const Sequence< PropertyValue >& info)
        {
            const static ::rtl::OUString sPropName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsMyODBCDriver"));
            const PropertyValue* pSupported = info.getConstArray();
            const PropertyValue* pEnd = pSupported + info.getLength();
            for (;pSupported != pEnd && !pSupported->Name.equals(sPropName); ++pSupported)
                ;
            return pSupported != pEnd && ::comphelper::getBOOL(pSupported->Value);
        }
        //--------------------------------------------------------------------
        Reference< XDriver > lcl_loadDriver(const Reference< XMultiServiceFactory >& _rxFactory,const ::rtl::OUString& _sUrl)
        {
            Reference<XDriverAccess> xDriverAccess(_rxFactory->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbc.DriverManager")) ),UNO_QUERY);
            OSL_ENSURE(xDriverAccess.is(),"Could not load driver manager!");
            Reference< XDriver > xDriver;
            if ( xDriverAccess.is() )
                xDriver = xDriverAccess->getDriverByURL(_sUrl);
            return xDriver;
        }
        //--------------------------------------------------------------------
        Sequence< PropertyValue > lcl_convertProperties(const Sequence< PropertyValue >& info)
        {
            const static ::rtl::OUString sPropName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsMyODBCDriver"));

            ::std::vector<PropertyValue> aProps;
            const PropertyValue* pSupported = info.getConstArray();
            const PropertyValue* pEnd = pSupported + info.getLength();
            for (;pSupported != pEnd; ++pSupported)
            {
                if ( pSupported->Name.equals(sPropName) && ::comphelper::getBOOL(pSupported->Value) )
                {
                    // here we know that we will connect to MyODBC,
                    // so we have to add the fetch auto increments things
                    aProps.push_back( PropertyValue(
                                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsAutoRetrievingEnabled"))
                                        ,0
                                        ,makeAny(sal_True)
                                        ,PropertyState_DIRECT_VALUE) );
                    aProps.push_back( PropertyValue(
                                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AutoRetrievingStatement"))
                                        ,0
                                        ,makeAny(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("select last_id()")))
                                        ,PropertyState_DIRECT_VALUE) );
                    // and some more info
                    aProps.push_back( PropertyValue(
                                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Silent"))
                                        ,0
                                        ,makeAny(sal_True)
                                        ,PropertyState_DIRECT_VALUE) );
                    aProps.push_back( PropertyValue(
                                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParameterNameSubstitution"))
                                        ,0
                                        ,makeAny(sal_True)
                                        ,PropertyState_DIRECT_VALUE) );
                }
                else
                    aProps.push_back( *pSupported );
            }
            return Sequence< PropertyValue >(aProps.begin(),aProps.size());
        }
    }
    //--------------------------------------------------------------------
    Reference< XDriver > ODriverDelegator::loadDriver( const ::rtl::OUString& url, const Sequence< PropertyValue >& info )
    {
        Reference< XDriver > xDriver;
        ::rtl::OUString sCuttedUrl = url.copy(11);
        if ( isOdbcPropertySet(info ) )
        {
            if ( !m_xODBCDriver.is() )
            {
                sCuttedUrl = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdbc:odbc:")) + sCuttedUrl;
                m_xODBCDriver = lcl_loadDriver(m_xFactory,sCuttedUrl);
            }
            xDriver = m_xODBCDriver;
        }
        else
        {
            if ( !m_xJDBCDriver.is() )
            {
                sCuttedUrl = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("jdbc:")) + sCuttedUrl;
                m_xJDBCDriver = lcl_loadDriver(m_xFactory,sCuttedUrl);
            }
            xDriver = m_xJDBCDriver;
        }

        return xDriver;
    }

    //--------------------------------------------------------------------
    Reference< XConnection > SAL_CALL ODriverDelegator::connect( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw (SQLException, RuntimeException)
    {
        Reference< XConnection > xConnection;
        if ( acceptsURL(url) )
        {
            Reference< XDriver > xDriver;
            xDriver = loadDriver(url,info);
            if ( xDriver.is() )
            {
                ::rtl::OUString sCuttedUrl = url.copy(11);
                if ( isOdbcPropertySet(info ) )
                    sCuttedUrl = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdbc:odbc:")) + sCuttedUrl;
                else
                    sCuttedUrl = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("jdbc:")) + sCuttedUrl;

                Sequence< PropertyValue > aConvertedProperties = lcl_convertProperties(info);

                xConnection = xDriver->connect( sCuttedUrl, aConvertedProperties );
                if ( xConnection.is() )
                    m_aConnections.push_back(TWeakPair(WeakReferenceHelper(xConnection),WeakReferenceHelper()));

                // now we have to set the URL to get the correct answer for metadata()->getURL()
                Reference< XUnoTunnel> xTunnel(xConnection,UNO_QUERY);
                if ( xTunnel.is() )
                {
                    OMetaConnection* pConnection = reinterpret_cast<OMetaConnection*>(xTunnel->getSomething( OMetaConnection::getUnoTunnelImplementationId() ));
                    if ( pConnection )
                        pConnection->setURL(url);
                }
            }
        }
        return xConnection;
    }

    //--------------------------------------------------------------------
    sal_Bool SAL_CALL ODriverDelegator::acceptsURL( const ::rtl::OUString& url ) throw (SQLException, RuntimeException)
    {
        return (!url.compareTo(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdbc:mysql:")),11));
    }

    //--------------------------------------------------------------------
    Sequence< DriverPropertyInfo > SAL_CALL ODriverDelegator::getPropertyInfo( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw (SQLException, RuntimeException)
    {
        Sequence< DriverPropertyInfo > aInfo;

        if ( acceptsURL(url) )
        {
//          loadDriver(url,info);
//          if ( m_xDriver.is() )
//              aInfo = m_xDriver->getPropertyInfo(url, info);
        }
        return aInfo;
    }

    //--------------------------------------------------------------------
    sal_Int32 SAL_CALL ODriverDelegator::getMajorVersion(  ) throw (RuntimeException)
    {
        return 1;
    }

    //--------------------------------------------------------------------
    sal_Int32 SAL_CALL ODriverDelegator::getMinorVersion(  ) throw (RuntimeException)
    {
        return 0;
    }

    //--------------------------------------------------------------------
    Reference< XTablesSupplier > SAL_CALL ODriverDelegator::getDataDefinitionByConnection( const Reference< XConnection >& connection ) throw (SQLException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(ODriverDelegator_BASE::rBHelper.bDisposed);

        Reference< XTablesSupplier > xTab;
        TWeakPairVector::iterator aEnd = m_aConnections.end();
        for (TWeakPairVector::iterator i = m_aConnections.begin(); aEnd != i; ++i)
        {
            if ( Reference< XConnection >::query(i->first.get().get()) == connection )
            {
                xTab = Reference< XTablesSupplier >(i->second.get().get(),UNO_QUERY);
                if ( !xTab.is() )
                    xTab = new OMySQLCatalog(connection);
                break;
            }
        }
        return xTab;
    }

    //--------------------------------------------------------------------
    Reference< XTablesSupplier > SAL_CALL ODriverDelegator::getDataDefinitionByURL( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw (SQLException, RuntimeException)
    {
        if ( ! acceptsURL(url) )
            ::dbtools::throwGenericSQLException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Invalid URL!")) ,*this);

        return getDataDefinitionByConnection(connect(url,info));
    }

    // XServiceInfo
    // --------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
    rtl::OUString ODriverDelegator::getImplementationName_Static(  ) throw(RuntimeException)
    {
        return rtl::OUString::createFromAscii("org.openoffice.comp.drivers.MySQL.Driver");
    }
    //------------------------------------------------------------------------------
    Sequence< ::rtl::OUString > ODriverDelegator::getSupportedServiceNames_Static(  ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aSNS( 2 );
        aSNS[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbc.Driver");
        aSNS[1] = ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.Driver");
        return aSNS;
    }
    //------------------------------------------------------------------
    ::rtl::OUString SAL_CALL ODriverDelegator::getImplementationName(  ) throw(RuntimeException)
    {
        return getImplementationName_Static();
    }

    //------------------------------------------------------------------
    sal_Bool SAL_CALL ODriverDelegator::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
    {
        Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
        const ::rtl::OUString* pSupported = aSupported.getConstArray();
        const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
        for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
            ;

        return pSupported != pEnd;
    }
    //------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL ODriverDelegator::getSupportedServiceNames(  ) throw(RuntimeException)
    {
        return getSupportedServiceNames_Static();
    }
    //------------------------------------------------------------------
    void SAL_CALL ODriverDelegator::createCatalog( const Sequence< PropertyValue >& info ) throw (SQLException, ::com::sun::star::container::ElementExistException, RuntimeException)
    {
    }
    //------------------------------------------------------------------
//........................................................................
}   // namespace connectivity
//........................................................................
