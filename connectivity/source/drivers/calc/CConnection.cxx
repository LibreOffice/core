/*************************************************************************
 *
 *  $RCSfile: CConnection.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2001-03-06 17:56:55 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
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

#ifndef _CONNECTIVITY_CALC_CONNECTION_HXX_
#include "calc/CConnection.hxx"
#endif
#ifndef _CONNECTIVITY_CALC_DATABASEMETADATA_HXX_
#include "calc/CDatabaseMetaData.hxx"
#endif
#ifndef _CONNECTIVITY_CALC_CATALOG_HXX_
#include "calc/CCatalog.hxx"
#endif
#ifndef _CONNECTIVITY_RESOURCE_HRC_
#include "Resource.hrc"
#endif
#ifndef _CONNECTIVITY_MODULECONTEXT_HXX_
#include "ModuleContext.hxx"
#endif
#ifndef _CONNECTIVITY_CALC_ODRIVER_HXX_
#include "calc/CDriver.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEETDOCUMENT_HPP_
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#endif
#ifndef _URLOBJ_HXX //autogen wg. INetURLObject
#include <tools/urlobj.hxx>
#endif
#ifndef _CONNECTIVITY_CALC_PREPAREDSTATEMENT_HXX_
#include "calc/CPreparedStatement.hxx"
#endif
#ifndef _CONNECTIVITY_CALC_STATEMENT_HXX_
#include "calc/CStatement.hxx"
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif

using namespace connectivity::calc;
using namespace connectivity::file;

typedef connectivity::file::OConnection OConnection_B;

//------------------------------------------------------------------------------

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::sheet;

// --------------------------------------------------------------------------------

OCalcConnection::OCalcConnection(ODriver* _pDriver) : OConnection(_pDriver)
{
    // m_aFilenameExtension is not used
}

OCalcConnection::~OCalcConnection()
{
}

void OCalcConnection::construct(const ::rtl::OUString& url,const Sequence< PropertyValue >& info)
    throw(SQLException)
{
    //  open file

    sal_Int32 nLen = url.indexOf(':');
    nLen = url.indexOf(':',nLen+1);
    ::rtl::OUString aDSN(url.copy(nLen+1));

    String aFileName = aDSN;
    INetURLObject aURL;
    aURL.SetSmartProtocol(INET_PROT_FILE);
    {
        SvtPathOptions aPathOptions;
        aFileName = aPathOptions.SubstituteVariable(aFileName);
    }
    aURL.SetSmartURL(aFileName);
    if ( aURL.GetProtocol() == INET_PROT_NOT_VALID )
    {
        //  don't pass invalid URL to loadComponentFromURL
        throw SQLException();
    }
    aFileName = aURL.GetMainURL();

    Reference< XComponentLoader > xDesktop( getDriver()->getFactory()->createInstance(
                    ::rtl::OUString::createFromAscii("com.sun.star.frame.Desktop")), UNO_QUERY );
    if (!xDesktop.is())
    {
        OSL_ASSERT("no desktop");
        throw SQLException();
    }

    //  open read-only as long as updating isn't implemented

    Sequence<PropertyValue> aArgs(2);
    aArgs[0].Name = ::rtl::OUString::createFromAscii("Hidden");
    aArgs[0].Value <<= (sal_Bool) sal_True;
    aArgs[1].Name = ::rtl::OUString::createFromAscii("ReadOnly");
    aArgs[1].Value <<= (sal_Bool) sal_True;

    Reference<XComponent> xComponent = xDesktop->loadComponentFromURL(
                            aFileName, ::rtl::OUString::createFromAscii("_blank"), 0, aArgs );
    m_xDoc = Reference<XSpreadsheetDocument>( xComponent, UNO_QUERY );

    //  if the URL is not a spreadsheet document, throw the exception here
    //  instead of at the first access to it
    if ( !m_xDoc.is() )
        throw SQLException();

    // file::OConnection::construct (reads the directory) is not called
}

void OCalcConnection::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    ::comphelper::disposeComponent( m_xDoc );

    OConnection::disposing();
}

// XServiceInfo
// --------------------------------------------------------------------------------

IMPLEMENT_SERVICE_INFO(OCalcConnection, "com.sun.star.sdbc.drivers.calc.Connection", "com.sun.star.sdbc.Connection")

// --------------------------------------------------------------------------------

Reference< XDatabaseMetaData > SAL_CALL OCalcConnection::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

    Reference< XDatabaseMetaData > xMetaData = m_xMetaData;
    if(!xMetaData.is())
    {
        xMetaData = new OCalcDatabaseMetaData(this);
        m_xMetaData = xMetaData;
    }

    return xMetaData;
}

//------------------------------------------------------------------------------

::com::sun::star::uno::Reference< XTablesSupplier > OCalcConnection::createCatalog()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    Reference< XTablesSupplier > xTab = m_xCatalog;
    if(!m_xCatalog.get().is())
    {
        OCalcCatalog *pCat = new OCalcCatalog(this);
        xTab = pCat;
        m_xCatalog = xTab;
    }
    return xTab;
}

// --------------------------------------------------------------------------------

Reference< XStatement > SAL_CALL OCalcConnection::createStatement(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

    Reference< XStatement > xReturn = new OCalcStatement(this);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}

// --------------------------------------------------------------------------------

Reference< XPreparedStatement > SAL_CALL OCalcConnection::prepareStatement( const ::rtl::OUString& sql )
    throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

    OCalcPreparedStatement* pStmt = new OCalcPreparedStatement(this,m_aTypeInfo);
    Reference< XPreparedStatement > xHoldAlive = pStmt;
    pStmt->construct(sql);
    m_aStatements.push_back(WeakReferenceHelper(*pStmt));
    return pStmt;
}

// --------------------------------------------------------------------------------

Reference< XPreparedStatement > SAL_CALL OCalcConnection::prepareCall( const ::rtl::OUString& sql )
    throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();
    return NULL;
}


