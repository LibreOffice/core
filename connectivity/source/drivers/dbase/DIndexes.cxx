/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DIndexes.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:20:35 $
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
#ifndef _CONNECTIVITY_DBASE_INDEXES_HXX_
#include "dbase/DIndexes.hxx"
#endif
#ifndef _CONNECTIVITY_DBASE_INDEX_HXX_
#include "dbase/DIndex.hxx"
#endif
#ifndef _UNOTOOLS_UCBHELPER_HXX
#include <unotools/ucbhelper.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _CONNECTIVITY_FILE_TABLE_HXX_
#include "FTable.hxx"
#endif

using namespace ::comphelper;

using namespace utl;
using namespace connectivity;
using namespace connectivity::dbase;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

namespace starutil      = ::com::sun::star::util;

sdbcx::ObjectType ODbaseIndexes::createObject(const ::rtl::OUString& _rName)
{
    //  Dir* pDir = m_pTable->getConnection()->getDir();
    //  String aPath = pDir->GetName();
    //  aPath += _rName.getStr();
    ::rtl::OUString sFile = m_pTable->getConnection()->getURL();
    sFile += OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DELIMITER);
    sFile += _rName;
    sFile += ::rtl::OUString::createFromAscii(".ndx");
    if(!UCBContentHelper::Exists(sFile))
        throw SQLException(::rtl::OUString::createFromAscii("Index file doesn't exists!"),*m_pTable,OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_HY0000),1000,Any());

    sdbcx::ObjectType xRet;
    SvStream* pFileStream = ::connectivity::file::OFileTable::createStream_simpleError(sFile,STREAM_READ | STREAM_NOCREATE| STREAM_SHARE_DENYWRITE);
    if(pFileStream)
    {
        pFileStream->SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
        pFileStream->SetBufferSize(512);
        ODbaseIndex::NDXHeader aHeader;

        pFileStream->Seek(0);
        pFileStream->Read(&aHeader,512);
        delete pFileStream;

        ODbaseIndex* pIndex = new ODbaseIndex(m_pTable,aHeader,_rName);
        xRet = pIndex;
        pIndex->openIndexFile();
    }
    else
        throw SQLException(::rtl::OUString::createFromAscii("Could not open index file"),*m_pTable,OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_HY0000),1000,Any());
    return xRet;
}
// -------------------------------------------------------------------------
void ODbaseIndexes::impl_refresh(  ) throw(RuntimeException)
{
    if(m_pTable)
        m_pTable->refreshIndexes();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > ODbaseIndexes::createEmptyObject()
{
    return new ODbaseIndex(m_pTable);
}
typedef connectivity::sdbcx::OCollection ODbaseTables_BASE_BASE;
// -------------------------------------------------------------------------
// XAppend
void ODbaseIndexes::appendObject( const Reference< XPropertySet >& descriptor )
{
    Reference<XUnoTunnel> xTunnel(descriptor,UNO_QUERY);
    if(xTunnel.is())
    {
        ODbaseIndex* pIndex = reinterpret_cast< ODbaseIndex* >( xTunnel->getSomething(ODbaseIndex::getUnoTunnelImplementationId()) );
        if(!pIndex || !pIndex->CreateImpl())
            throw SQLException();
    }
}
// -------------------------------------------------------------------------
// XDrop
void ODbaseIndexes::dropObject(sal_Int32 _nPos,const ::rtl::OUString /*_sElementName*/)
{
    Reference< XUnoTunnel> xTunnel(getObject(_nPos),UNO_QUERY);
    if ( xTunnel.is() )
    {
        ODbaseIndex* pIndex = reinterpret_cast< ODbaseIndex* >( xTunnel->getSomething(ODbaseIndex::getUnoTunnelImplementationId()) );
        if ( pIndex )
            pIndex->DropImpl();
    }

}
// -------------------------------------------------------------------------


