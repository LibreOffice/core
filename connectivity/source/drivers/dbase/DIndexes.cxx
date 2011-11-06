/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#include "dbase/DIndexes.hxx"
#include "dbase/DIndex.hxx"
#include <connectivity/dbexception.hxx>
#include <unotools/ucbhelper.hxx>
#include <comphelper/types.hxx>
#include "resource/dbase_res.hrc"

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
    if ( !UCBContentHelper::Exists(sFile) )
    {
        const ::rtl::OUString sError( m_pTable->getConnection()->getResources().getResourceStringWithSubstitution(
                STR_COULD_NOT_LOAD_FILE,
                "$filename$", sFile
            ) );
        ::dbtools::throwGenericSQLException( sError, *m_pTable );
    }

    sdbcx::ObjectType xRet;
    SvStream* pFileStream = ::connectivity::file::OFileTable::createStream_simpleError(sFile,STREAM_READ | STREAM_NOCREATE| STREAM_SHARE_DENYWRITE);
    if(pFileStream)
    {
        pFileStream->SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
        pFileStream->SetBufferSize(PAGE_SIZE);
        ODbaseIndex::NDXHeader aHeader;

        pFileStream->Seek(0);
        pFileStream->Read(&aHeader,PAGE_SIZE);
        delete pFileStream;

        ODbaseIndex* pIndex = new ODbaseIndex(m_pTable,aHeader,_rName);
        xRet = pIndex;
        pIndex->openIndexFile();
    }
    else
    {
        const ::rtl::OUString sError( m_pTable->getConnection()->getResources().getResourceStringWithSubstitution(
                STR_COULD_NOT_LOAD_FILE,
                "$filename$", sFile
             ) );
        ::dbtools::throwGenericSQLException( sError, *m_pTable );
    }
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


