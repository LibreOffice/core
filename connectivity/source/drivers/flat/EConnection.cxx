/*************************************************************************
 *
 *  $RCSfile: EConnection.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-17 09:14:16 $
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

#ifndef _CONNECTIVITY_FLAT_ECONNECTION_HXX_
#include "flat/EConnection.hxx"
#endif
#ifndef _CONNECTIVITY_FLAT_EDATABASEMETADATA_HXX_
#include "flat/EDatabaseMetaData.hxx"
#endif
#ifndef _CONNECTIVITY_FLAT_CATALOG_HXX_
#include "flat/ECatalog.hxx"
#endif
#ifndef _CONNECTIVITY_RESOURCE_HRC_
#include "Resource.hrc"
#endif
#ifndef _CONNECTIVITY_MODULECONTEXT_HXX_
#include "ModuleContext.hxx"
#endif
#ifndef _CONNECTIVITY_FLAT_ODRIVER_HXX_
#include "flat/EDriver.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _URLOBJ_HXX //autogen wg. INetURLObject
#include <tools/urlobj.hxx>
#endif
#ifndef _CONNECTIVITY_FLAT_DPREPAREDSTATEMENT_HXX_
#include "flat/EPreparedStatement.hxx"
#endif
#ifndef _CONNECTIVITY_FLAT_DSTATEMENT_HXX_
#include "flat/EStatement.hxx"
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif

using namespace connectivity::flat;
using namespace connectivity::file;

typedef connectivity::file::OConnection  OConnection_B;

//------------------------------------------------------------------------------
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;

// --------------------------------------------------------------------------------
OFlatConnection::OFlatConnection(ODriver*   _pDriver) : OConnection(_pDriver)
    ,m_bFixedLength(sal_False)
    ,m_bHeaderLine(sal_True)
    ,m_cFieldDelimiter(';')
    ,m_cStringDelimiter('"')
    ,m_cDecimalDelimiter(',')
    ,m_cThousandDelimiter('.')
{
}
//-----------------------------------------------------------------------------
OFlatConnection::~OFlatConnection()
{
}

// XServiceInfo
// --------------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO(OFlatConnection, "com.sun.star.sdbc.drivers.flat.Connection", "com.sun.star.sdbc.Connection")

//-----------------------------------------------------------------------------
void OFlatConnection::construct(const ::rtl::OUString& url,const Sequence< PropertyValue >& info)  throw(SQLException)
{
    osl_incrementInterlockedCount( &m_refCount );

    ::rtl::OUString aExt;
    const PropertyValue *pBegin  = info.getConstArray();
    const PropertyValue *pEnd    = pBegin + info.getLength();
    for(;pBegin != pEnd;++pBegin)
    {
        if(!pBegin->Name.compareToAscii("FixedLength"))
            m_bFixedLength = cppu::any2bool(pBegin->Value);
        else if(!pBegin->Name.compareToAscii("HeaderLine"))
            m_bHeaderLine = cppu::any2bool(pBegin->Value);
        else if(!pBegin->Name.compareToAscii("FieldDelimiter"))
            pBegin->Value >>= m_cFieldDelimiter;
        else if(!pBegin->Name.compareToAscii("StringDelimiter"))
            pBegin->Value >>= m_cStringDelimiter;
        else if(!pBegin->Name.compareToAscii("DecimalDelimiter"))
            pBegin->Value >>= m_cDecimalDelimiter;
        else if(!pBegin->Name.compareToAscii("ThousandDelimiter"))
            pBegin->Value >>= m_cThousandDelimiter;
    }

    osl_decrementInterlockedCount( &m_refCount );
    OConnection::construct(url,info);
}
// --------------------------------------------------------------------------------
Reference< XDatabaseMetaData > SAL_CALL OFlatConnection::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_B::rBHelper.bDisposed)
        throw DisposedException();

    if(!m_xMetaData.is())
        m_xMetaData = new OFlatDatabaseMetaData(this);

    return m_xMetaData;
}
//------------------------------------------------------------------------------
::com::sun::star::uno::Reference< XTablesSupplier > OFlatConnection::createCatalog()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    Reference< XTablesSupplier > xTab = m_xCatalog;
    if(!m_xCatalog.get().is())
    {
        OFlatCatalog *pCat = new OFlatCatalog(this);
        xTab = pCat;
        m_xCatalog = xTab;
    }
    return xTab;
}
// --------------------------------------------------------------------------------
Reference< XStatement > SAL_CALL OFlatConnection::createStatement(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();
    OFlatStatement* pStmt = new OFlatStatement(this);

    Reference< XStatement > xStmt = pStmt;
    m_aStatements.push_back(WeakReferenceHelper(*pStmt));
    return xStmt;
}
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL OFlatConnection::prepareStatement( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();

    OFlatPreparedStatement* pStmt = new OFlatPreparedStatement(this,m_aTypeInfo);
    Reference< XPreparedStatement > xStmt = pStmt;
    pStmt->construct(sql);

    m_aStatements.push_back(WeakReferenceHelper(*pStmt));
    return xStmt;
}
// --------------------------------------------------------------------------------
Reference< XPreparedStatement > SAL_CALL OFlatConnection::prepareCall( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OConnection_BASE::rBHelper.bDisposed)
        throw DisposedException();
    return NULL;
}


