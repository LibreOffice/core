/*************************************************************************
 *
 *  $RCSfile: DConnection.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-17 09:15:19 $
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

#ifndef _CONNECTIVITY_DBASE_OCONNECTION_HXX_
#include "dbase/DConnection.hxx"
#endif
#ifndef _CONNECTIVITY_DBASE_ODATABASEMETADATA_HXX_
#include "dbase/DDatabaseMetaData.hxx"
#endif
#ifndef _CONNECTIVITY_DBASE_CATALOG_HXX_
#include "dbase/DCatalog.hxx"
#endif
#ifndef _CONNECTIVITY_RESOURCE_HRC_
#include "Resource.hrc"
#endif
#ifndef _CONNECTIVITY_MODULECONTEXT_HXX_
#include "ModuleContext.hxx"
#endif
#ifndef _CONNECTIVITY_DBASE_ODRIVER_HXX_
#include "dbase/DDriver.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _URLOBJ_HXX //autogen wg. INetURLObject
#include <tools/urlobj.hxx>
#endif
#ifndef _CONNECTIVITY_DBASE_DPREPAREDSTATEMENT_HXX_
#include "dbase/DPreparedStatement.hxx"
#endif
#ifndef _CONNECTIVITY_DBASE_DSTATEMENT_HXX_
#include "dbase/DStatement.hxx"
#endif
using namespace connectivity::dbase;
using namespace connectivity::file;

typedef connectivity::file::OConnection  OConnection_B;

//------------------------------------------------------------------------------
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;

// --------------------------------------------------------------------------------
ODbaseConnection::ODbaseConnection(ODriver* _pDriver) : OConnection(_pDriver)
{
    m_aFilenameExtension = String::CreateFromAscii("dbf");
}
//-----------------------------------------------------------------------------
ODbaseConnection::~ODbaseConnection()
{
}

// XServiceInfo
// --------------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO(ODbaseConnection, "com.sun.star.sdbc.drivers.dbase.Connection", "com.sun.star.sdbc.Connection")

// --------------------------------------------------------------------------------
Reference< XDatabaseMetaData > SAL_CALL ODbaseConnection::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_B::rBHelper.bDisposed)
        throw DisposedException();

    if(!m_xMetaData.is())
        m_xMetaData = new ODbaseDatabaseMetaData(this);

    return m_xMetaData;
}
//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< XTablesSupplier > ODbaseConnection::createCatalog()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    Reference< XTablesSupplier > xTab = m_xCatalog;
    if(!m_xCatalog.get().is())
    {
        ODbaseCatalog *pCat = new ODbaseCatalog(this);
        xTab = pCat;
        m_xCatalog = xTab;
    }
    return xTab;
}
// --------------------------------------------------------------------------------
Reference< XStatement > SAL_CALL ODbaseConnection::createStatement(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

    Reference< XStatement > xReturn = new ODbaseStatement(this);
    m_aStatements.push_back(WeakReferenceHelper(xReturn));
    return xReturn;
}
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL ODbaseConnection::prepareStatement( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

    ODbasePreparedStatement* pStmt = new ODbasePreparedStatement(this,m_aTypeInfo);
    Reference< XPreparedStatement > xHoldAlive = pStmt;
    pStmt->construct(sql);
    m_aStatements.push_back(WeakReferenceHelper(*pStmt));
    return pStmt;
}
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL ODbaseConnection::prepareCall( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();
    return NULL;
}


