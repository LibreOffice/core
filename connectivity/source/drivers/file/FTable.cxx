/*************************************************************************
 *
 *  $RCSfile: FTable.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 08:26:02 $
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

#include <stdio.h>

#ifndef _CONNECTIVITY_FILE_TABLE_HXX_
#include "file/FTable.hxx"
#endif
#ifndef _CONNECTIVITY_FILE_COLUMNS_HXX_
#include "file/FColumns.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

using namespace connectivity;
using namespace connectivity::file;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;

DBG_NAME( file_OFileTable );
OFileTable::OFileTable(sdbcx::OCollection* _pTables,OConnection* _pConnection)
: OTable_TYPEDEF(_pTables,_pConnection->getMetaData()->storesMixedCaseQuotedIdentifiers())
                ,m_pConnection(_pConnection)
                ,m_nFilePos(0)
                ,m_nBufferSize(0)
                ,m_pBuffer(NULL)
                ,m_pFileStream(NULL)
                ,m_bWriteable(sal_False)
{
    DBG_CTOR( file_OFileTable, NULL );
    construct();
    TStringVector aVector;
    //  m_pColumns  = new OColumns(this,m_aMutex,aVector);
    m_aColumns = new OSQLColumns();
}
// -------------------------------------------------------------------------
OFileTable::OFileTable( sdbcx::OCollection* _pTables,OConnection* _pConnection,
                    const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _Type,
                    const ::rtl::OUString& _Description ,
                    const ::rtl::OUString& _SchemaName,
                    const ::rtl::OUString& _CatalogName
                ) : OTable_TYPEDEF(_pTables,_pConnection->getMetaData()->storesMixedCaseQuotedIdentifiers(),
                                  _Name,
                                  _Type,
                                  _Description,
                                  _SchemaName,
                                  _CatalogName)
                ,m_pConnection(_pConnection)
                ,m_nFilePos(0)
                ,m_nBufferSize(0)
                ,m_pBuffer(NULL)
                ,m_pFileStream(NULL)
                ,m_bWriteable(sal_False)
{
    DBG_CTOR( file_OFileTable, NULL );
    m_aColumns = new OSQLColumns();
    construct();
    //  refreshColumns();
}
// -------------------------------------------------------------------------
OFileTable::~OFileTable( )
{
    DBG_DTOR( file_OFileTable, NULL );
}
// -------------------------------------------------------------------------
void OFileTable::refreshColumns()
{
    TStringVector aVector;
        Reference< XResultSet > xResult = m_pConnection->getMetaData()->getColumns(Any(),
                                                    m_SchemaName,m_Name,::rtl::OUString::createFromAscii("%"));

    if(xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        while(xResult->next())
            aVector.push_back(xRow->getString(4));
    }

    if(m_pColumns)
        m_pColumns->reFill(aVector);
    else
        m_pColumns  = new OColumns(this,m_aMutex,aVector);
}
// -------------------------------------------------------------------------
void OFileTable::refreshKeys()
{
}
// -------------------------------------------------------------------------
void OFileTable::refreshIndexes()
{
}
// -------------------------------------------------------------------------
Any SAL_CALL OFileTable::queryInterface( const Type & rType ) throw(RuntimeException)
{
    if( rType == ::getCppuType((const Reference<XKeysSupplier>*)0) ||
        rType == ::getCppuType((const Reference<XRename>*)0) ||
        rType == ::getCppuType((const Reference<XAlterTable>*)0) ||
        rType == ::getCppuType((const Reference<XIndexesSupplier>*)0) ||
        rType == ::getCppuType((const Reference<XDataDescriptorFactory>*)0))
        return Any();

    return OTable_TYPEDEF::queryInterface(rType);
}
// -------------------------------------------------------------------------
void SAL_CALL OFileTable::disposing(void)
{
    OTable::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);

    FileClose();
    m_xMetaData = NULL;
}
//--------------------------------------------------------------------------
Sequence< sal_Int8 > OFileTable::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId * pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

// com::sun::star::lang::XUnoTunnel
//------------------------------------------------------------------
sal_Int64 OFileTable::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    return (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
                ?
            (sal_Int64)this
                :
            OTable_TYPEDEF::getSomething(rId);
}
// -----------------------------------------------------------------------------
void OFileTable::FileClose()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    if (m_pFileStream && m_pFileStream->IsWritable())
        m_pFileStream->Flush();

    delete m_pFileStream;
    m_pFileStream = NULL;

    if (m_pBuffer)
    {
        delete[] m_pBuffer;
        m_pBuffer = NULL;
    }
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void SAL_CALL OFileTable::acquire() throw()
{
    OTable_TYPEDEF::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OFileTable::release() throw()
{
    OTable_TYPEDEF::release();
}
// -----------------------------------------------------------------------------
BOOL OFileTable::InsertRow(OValueRefVector& rRow, BOOL bFlush,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& _xCols)
{
    return sal_False;
}
// -----------------------------------------------------------------------------
BOOL OFileTable::DeleteRow(const OSQLColumns& _rCols)
{
    return sal_False;
}
// -----------------------------------------------------------------------------
BOOL OFileTable::UpdateRow(OValueRefVector& rRow, OValueRefRow& pOrgRow,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& _xCols)
{
    return sal_False;
}
// -----------------------------------------------------------------------------
void OFileTable::addColumn(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& descriptor)
{
}
// -----------------------------------------------------------------------------
void OFileTable::dropColumn(sal_Int32 _nPos)
{
}

// -----------------------------------------------------------------------------
SvStream* OFileTable::createStream_simpleError( const String& _rFileName, StreamMode _eOpenMode)
{
    SvStream* pReturn = ::utl::UcbStreamHelper::CreateStream( _rFileName, _eOpenMode, NULL);
    if (pReturn && (ERRCODE_NONE != pReturn->GetErrorCode()))
    {
        delete pReturn;
        pReturn = NULL;
    }
    return pReturn;
}

// -----------------------------------------------------------------------------
void OFileTable::refreshHeader()
{
}
// -----------------------------------------------------------------------------

