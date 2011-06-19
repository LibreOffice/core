/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#include "file/FDriver.hxx"
#include "file/FConnection.hxx"
#include "file/fcode.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include <comphelper/types.hxx>
#include "connectivity/dbexception.hxx"
#include "resource/common_res.hrc"
#include "resource/sharedresources.hxx"
#include <rtl/logfile.hxx>


using namespace connectivity::file;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
// --------------------------------------------------------------------------------
OFileDriver::OFileDriver(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory)
    : ODriver_BASE(m_aMutex)
    ,m_xFactory(_rxFactory)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileDriver::OFileDriver" );
}
// --------------------------------------------------------------------------------
void OFileDriver::disposing()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileDriver::disposing" );
    ::osl::MutexGuard aGuard(m_aMutex);


    for (OWeakRefArray::iterator i = m_xConnections.begin(); m_xConnections.end() != i; ++i)
    {
        Reference< XComponent > xComp(i->get(), UNO_QUERY);
        if (xComp.is())
            xComp->dispose();
    }
    m_xConnections.clear();

    ODriver_BASE::disposing();
}

// static ServiceInfo
//------------------------------------------------------------------------------
rtl::OUString OFileDriver::getImplementationName_Static(  ) throw(RuntimeException)
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbc.driver.file.Driver"));
}
//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > OFileDriver::getSupportedServiceNames_Static(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 2 );
    aSNS[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbc.Driver"));
    aSNS[1] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbcx.Driver"));
    return aSNS;
}

//------------------------------------------------------------------
::rtl::OUString SAL_CALL OFileDriver::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------
sal_Bool SAL_CALL OFileDriver::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}

//------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL OFileDriver::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

// --------------------------------------------------------------------------------
Reference< XConnection > SAL_CALL OFileDriver::connect( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileDriver::connect" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODriver_BASE::rBHelper.bDisposed);

    OConnection* pCon = new OConnection(this);
    Reference< XConnection > xCon = pCon;
    pCon->construct(url,info);
    m_xConnections.push_back(WeakReferenceHelper(*pCon));

    return xCon;
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL OFileDriver::acceptsURL( const ::rtl::OUString& url )
                throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileDriver::acceptsURL" );
    return (!url.compareTo(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdbc:file:")),10));
}
// --------------------------------------------------------------------------------
Sequence< DriverPropertyInfo > SAL_CALL OFileDriver::getPropertyInfo( const ::rtl::OUString& url, const Sequence< PropertyValue >& /*info*/ ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileDriver::getPropertyInfo" );
    if ( acceptsURL(url) )
    {
        ::std::vector< DriverPropertyInfo > aDriverInfo;

        Sequence< ::rtl::OUString > aBoolean(2);
        aBoolean[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("0"));
        aBoolean[1] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("1"));

        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharSet"))
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharSet of the database."))
                ,sal_False
                ,::rtl::OUString()
                ,Sequence< ::rtl::OUString >())
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Extension"))
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Extension of the file format."))
                ,sal_False
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".*"))
                ,Sequence< ::rtl::OUString >())
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowDeleted"))
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Display inactive records."))
                ,sal_False
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("0"))
                ,aBoolean)
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("EnableSQL92Check"))
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Use SQL92 naming constraints."))
                ,sal_False
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("0"))
                ,aBoolean)
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UseRelativePath"))
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Handle the connection url as relative path."))
                ,sal_False
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("0"))
                ,aBoolean)
                );
        aDriverInfo.push_back(DriverPropertyInfo(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("URL"))
                ,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("The URL of the database document which is used to create an absolute path."))
                ,sal_False
                ,::rtl::OUString()
                ,Sequence< ::rtl::OUString >())
                );
        return Sequence< DriverPropertyInfo >(&(aDriverInfo[0]),aDriverInfo.size());
    } // if ( acceptsURL(url) )
    {
        ::connectivity::SharedResources aResources;
        const ::rtl::OUString sMessage = aResources.getResourceString(STR_URI_SYNTAX_ERROR);
        ::dbtools::throwGenericSQLException(sMessage ,*this);
    } // if ( ! acceptsURL(url) )
    return Sequence< DriverPropertyInfo >();
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL OFileDriver::getMajorVersion(  ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileDriver::getMajorVersion" );
    return 1;
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL OFileDriver::getMinorVersion(  ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileDriver::getMinorVersion" );
    return 0;
}
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
// XDataDefinitionSupplier
Reference< XTablesSupplier > SAL_CALL OFileDriver::getDataDefinitionByConnection( const Reference< ::com::sun::star::sdbc::XConnection >& connection ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileDriver::getDataDefinitionByConnection" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(ODriver_BASE::rBHelper.bDisposed);

    Reference< XTablesSupplier > xTab = NULL;
    Reference< ::com::sun::star::lang::XUnoTunnel> xTunnel(connection,UNO_QUERY);
    if(xTunnel.is())
    {
        OConnection* pSearchConnection = reinterpret_cast< OConnection* >( xTunnel->getSomething(OConnection::getUnoTunnelImplementationId()) );
        OConnection* pConnection = NULL;
        for (OWeakRefArray::iterator i = m_xConnections.begin(); m_xConnections.end() != i; ++i)
        {
            if ((OConnection*) Reference< XConnection >::query(i->get().get()).get() == pSearchConnection)
            {
                pConnection = pSearchConnection;
                break;
            }
        }

        if(pConnection)
            xTab = pConnection->createCatalog();
    }
    return xTab;
}

// --------------------------------------------------------------------------------
Reference< XTablesSupplier > SAL_CALL OFileDriver::getDataDefinitionByURL( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OFileDriver::getDataDefinitionByURL" );
    if ( ! acceptsURL(url) )
    {
        ::connectivity::SharedResources aResources;
        const ::rtl::OUString sMessage = aResources.getResourceString(STR_URI_SYNTAX_ERROR);
        ::dbtools::throwGenericSQLException(sMessage ,*this);
    }
    return getDataDefinitionByConnection(connect(url,info));
}
// -----------------------------------------------------------------------------
void OOperandParam::describe(const Reference< XPropertySet>& rColumn, ::rtl::Reference<connectivity::OSQLColumns> rParameterColumns)
{
    // den alten namen beibehalten

    OSL_ENSURE(getRowPos() < rParameterColumns->get().size(),"Invalid index for orderkey values!");

    Reference< XPropertySet> xColumn = (rParameterColumns->get())[getRowPos()];

    try
    {
        xColumn->setPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME),rColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME)));
        xColumn->setPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DEFAULTVALUE),rColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DEFAULTVALUE)));
        xColumn->setPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION),rColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION)));
        xColumn->setPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE),rColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE)));
        xColumn->setPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE),rColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE)));
        xColumn->setPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISNULLABLE),rColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISNULLABLE)));
        xColumn->setPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISAUTOINCREMENT),rColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISAUTOINCREMENT)));
    }
    catch(const Exception&)
    {
    }

    m_eDBType = ::comphelper::getINT32(rColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE)));
}
// -----------------------------------------------------------------------------
OOperandAttr::OOperandAttr(sal_uInt16 _nPos,const Reference< XPropertySet>& _xColumn)
    : OOperandRow(_nPos,::comphelper::getINT32(_xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE))))
    , m_xColumn(_xColumn)
{
}
// -----------------------------------------------------------------------------




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
