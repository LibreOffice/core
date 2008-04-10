/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DIndexes.cxx,v $
 * $Revision: 1.18 $
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
#include "dbase/DIndexes.hxx"
#include "dbase/DIndex.hxx"
#include <connectivity/dbexception.hxx>
#include <unotools/ucbhelper.hxx>
#include <comphelper/types.hxx>

using namespace ::comphelper;

using namespace utl;
using namespace ::connectivity;
using namespace ::dbtools;
using namespace ::connectivity::dbase;
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
        ::dbtools::throwGenericSQLException(
            ::rtl::OUString::createFromAscii( "Index file doesn't exists." ),
            // TODO: resource
            *m_pTable );

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
        ::dbtools::throwGenericSQLException(
            ::rtl::OUString::createFromAscii( "Could not open index file."),
            // TODO: resource
            *m_pTable );
    return xRet;
}
// -------------------------------------------------------------------------
void ODbaseIndexes::impl_refresh(  ) throw(RuntimeException)
{
    if(m_pTable)
        m_pTable->refreshIndexes();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > ODbaseIndexes::createDescriptor()
{
    return new ODbaseIndex(m_pTable);
}
typedef connectivity::sdbcx::OCollection ODbaseTables_BASE_BASE;
// -------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType ODbaseIndexes::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    Reference<XUnoTunnel> xTunnel(descriptor,UNO_QUERY);
    if(xTunnel.is())
    {
        ODbaseIndex* pIndex = reinterpret_cast< ODbaseIndex* >( xTunnel->getSomething(ODbaseIndex::getUnoTunnelImplementationId()) );
        if(!pIndex || !pIndex->CreateImpl())
            throw SQLException();
    }

    return createObject( _rForName );
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


