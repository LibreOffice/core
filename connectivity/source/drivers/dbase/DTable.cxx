/*************************************************************************
 *
 *  $RCSfile: DTable.cxx,v $
 *
 *  $Revision: 1.35 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-28 11:31:44 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
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

#ifndef _CONNECTIVITY_DBASE_TABLE_HXX_
#include "dbase/DTable.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTACCESS_HPP_
#include <com/sun/star/ucb/XContentAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SQLC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _SV_CONVERTER_HXX_
#include <svtools/converter.hxx>
#endif
#ifndef _CONNECTIVITY_DBASE_DCONNECTION_HXX_
#include "dbase/DConnection.hxx"
#endif
#ifndef _CONNECTIVITY_DBASE_COLUMNS_HXX_
#include "dbase/DColumns.hxx"
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif
#ifndef _CONFIG_HXX //autogen
#include <vcl/config.hxx>
#endif
#ifndef _CONNECTIVITY_DBASE_INDEX_HXX_
#include "dbase/DIndex.hxx"
#endif
#ifndef _CONNECTIVITY_DBASE_INDEXES_HXX_
#include "dbase/DIndexes.hxx"
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _CONNECTIVITY_DATECONVERSION_HXX_
#include "connectivity/DateConversion.hxx"
#endif
#ifndef _INTN_HXX //autogen
#include <tools/intn.hxx>
#endif
#ifndef _ZFORLIST_HXX //autogen
#include <svtools/zforlist.hxx>
#endif
#ifndef _SOLMATH_HXX //autogen wg. SolarMath
#include <tools/solmath.hxx>
#endif
#include <stdio.h>      //sprintf
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#define CONNECTIVITY_PROPERTY_NAME_SPACE dbase
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _DBHELPER_DBCONVERSION_HXX_
#include <connectivity/dbconversion.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _UNOTOOLS_TEMPFILE_HXX
#include <unotools/tempfile.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

using namespace connectivity;
using namespace connectivity::dbase;
using namespace connectivity::file;
using namespace ::ucb;
using namespace ::utl;
using namespace ::cppu;
using namespace ::dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

// -------------------------------------------------------------------------
void ODbaseTable::readHeader()
{
    m_pFileStream->RefreshBuffer(); // sicherstellen, dass die Kopfinformationen tatsaechlich neu gelesen werden
    m_pFileStream->Seek(STREAM_SEEK_TO_BEGIN);

    BYTE nType;
    (*m_pFileStream) >> nType;
    m_pFileStream->Read((char*)(&m_aHeader.db_aedat), 3*sizeof(BYTE));
    (*m_pFileStream) >> m_aHeader.db_anz;
    (*m_pFileStream) >> m_aHeader.db_kopf;
    (*m_pFileStream) >> m_aHeader.db_slng;
    m_pFileStream->Read((char*)(&m_aHeader.db_frei), 20*sizeof(BYTE));

    if (m_aHeader.db_anz  < 0 ||
        m_aHeader.db_kopf <= 0 ||
        m_aHeader.db_slng <= 0 ||
        ((m_aHeader.db_kopf - 1) / 32 - 1) <= 0) // anzahl felder
    {
        // no dbase file
        ::rtl::OUString sMessage = ::rtl::OUString::createFromAscii("[StarOffice Base dbase] The file '");
        sMessage += getEntry();
        sMessage += ::rtl::OUString::createFromAscii(" is an invalid (or unrecognized) dBase file.");
        throwGenericSQLException(sMessage, static_cast<XNamed*>(this));
    }
    else
    {
        // Konsistenzpruefung des Header:
        m_aHeader.db_typ = (DBFType)nType;
        switch (m_aHeader.db_typ)
        {
            case dBaseIII:
            case dBaseIV:
            case dBaseV:
            case dBaseFS:
            case dBaseFSMemo:
            case dBaseIVMemoSQL:
            case dBaseIIIMemo:
            case dBaseIVMemo:
            case FoxProMemo:
                m_pFileStream->SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
                break;
            default:
            {
                // no dbase file
                ::rtl::OUString sMessage = ::rtl::OUString::createFromAscii("[StarOffice Base dbase] The file ");
                sMessage += getEntry();
                sMessage += ::rtl::OUString::createFromAscii(" is an invalid (or unrecognized) dBase file.");
                throwGenericSQLException(sMessage, static_cast<XNamed*>(this));
            }
        }
    }
}
// -------------------------------------------------------------------------
void ODbaseTable::fillColumns()
{
    m_pFileStream->Seek(STREAM_SEEK_TO_BEGIN);
    m_pFileStream->Seek(32L);

    m_aColumns->clear();
    m_aTypes.clear();
    m_aPrecisions.clear();
    m_aScales.clear();

    // Anzahl Felder:
    sal_Int32 nFieldCount = (m_aHeader.db_kopf - 1) / 32 - 1;

    String aStrFieldName;aStrFieldName.AssignAscii("Column");
    sal_Int32 nFieldCnt = 0;
    ::rtl::OUString aTypeName;

    for (sal_Int32 i = 0; i < nFieldCount; i++)
    {
        DBFColumn aDBFColumn;
        m_pFileStream->Read((char*)&aDBFColumn, sizeof(aDBFColumn));

        String aColumnName((const char *)aDBFColumn.db_fnm,getConnection()->getTextEncoding());

        sal_Int32 nPrecision = aDBFColumn.db_flng;
        sal_Int32 eType;

        switch (aDBFColumn.db_typ)
        {
            case 'C':
                eType = DataType::VARCHAR;
                aTypeName = ::rtl::OUString::createFromAscii("VARCHAR");
                break;
            case 'F':
            case 'N':
                eType = DataType::DECIMAL;
                aTypeName = ::rtl::OUString::createFromAscii("DECIMAL");

                // Bei numerischen Feldern werden zwei Zeichen mehr geschrieben, als die Precision der Spaltenbeschreibung eigentlich
                // angibt, um Platz fuer das eventuelle Vorzeichen und das Komma zu haben. Das muss ich jetzt aber wieder rausrechnen.
                nPrecision = SvDbaseConverter::ConvertPrecisionToOdbc(nPrecision,aDBFColumn.db_dez);
                    // leider gilt das eben Gesagte nicht fuer aeltere Versionen ....
                    ;
                break;
            case 'L':
                eType = DataType::BIT;
                aTypeName = ::rtl::OUString::createFromAscii("BIT");
                break;
            case 'D':
                eType = DataType::DATE;
                aTypeName = ::rtl::OUString::createFromAscii("DATE");
                break;
            case 'M':
                eType = DataType::LONGVARCHAR;
                aTypeName = ::rtl::OUString::createFromAscii("LONGVARCHAR");
                nPrecision = 0;
                break;
            default:
                aTypeName = ::rtl::OUString::createFromAscii("OTHER");
                eType = DataType::OTHER;

        }

        sal_Int32 nFlags = 0;
        switch (aDBFColumn.db_typ)
        {
            case 'C':
            case 'D':
            case 'L':   nFlags = ColumnSearch::FULL; break;
            case 'F':
            case 'N':   nFlags = ColumnSearch::BASIC; break;
            case 'M':   nFlags = ColumnSearch::CHAR; break;
            default:
                        nFlags = ColumnSearch::NONE;

        }

        sdbcx::OColumn* pColumn = new sdbcx::OColumn(aColumnName,aTypeName,::rtl::OUString(),
                                                ColumnValue::NULLABLE,nPrecision,aDBFColumn.db_dez,eType,sal_False,sal_False,sal_False,
                                                getConnection()->getMetaData()->storesMixedCaseQuotedIdentifiers());
        Reference< XPropertySet> xCol = pColumn;
        m_aColumns->push_back(xCol);
        m_aTypes.push_back(eType);
        m_aPrecisions.push_back(nPrecision);
        m_aScales.push_back(aDBFColumn.db_dez);
    }
}
// -------------------------------------------------------------------------
ODbaseTable::ODbaseTable(ODbaseConnection* _pConnection)
        :ODbaseTable_BASE(_pConnection)
        ,m_pMemoStream(NULL)
        ,m_bWriteableMemo(sal_False)
{
    // initialize the header
    m_aHeader.db_typ    = dBaseIII;
    m_aHeader.db_anz    = 0;
    m_aHeader.db_kopf   = 0;
    m_aHeader.db_slng   = 0;
}
// -------------------------------------------------------------------------
ODbaseTable::ODbaseTable(ODbaseConnection* _pConnection,
                    const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _Type,
                    const ::rtl::OUString& _Description ,
                    const ::rtl::OUString& _SchemaName,
                    const ::rtl::OUString& _CatalogName
                ) : ODbaseTable_BASE(_pConnection,_Name,
                                  _Type,
                                  _Description,
                                  _SchemaName,
                                  _CatalogName)
                ,m_pMemoStream(NULL)
                ,m_bWriteableMemo(sal_False)
{
}
// -----------------------------------------------------------------------------
void ODbaseTable::construct()
{
    // initialize the header
    m_aHeader.db_typ    = dBaseIII;
    m_aHeader.db_anz    = 0;
    m_aHeader.db_kopf   = 0;
    m_aHeader.db_slng   = 0;

    String sFileName(getEntry());

    INetURLObject aURL;
    aURL.SetURL(sFileName);

    OSL_ENSURE(aURL.getExtension() == m_pConnection->getExtension(),
        "ODbaseTable::ODbaseTable: invalid extension!");
        // getEntry is expected to ensure the corect file name

    m_pFileStream = ::utl::UcbStreamHelper::CreateStream( sFileName,STREAM_READWRITE | STREAM_NOCREATE | STREAM_SHARE_DENYWRITE);

    if (!(m_bWriteable = (NULL != m_pFileStream)))
        m_pFileStream = ::utl::UcbStreamHelper::CreateStream( sFileName,STREAM_READ | STREAM_NOCREATE | STREAM_SHARE_DENYNONE );

    if(m_pFileStream)
    {
        readHeader();
        if (HasMemoFields())
        {
            // Memo-Dateinamen bilden (.DBT):
            // nyi: Unschoen fuer Unix und Mac!

            if (m_aHeader.db_typ == FoxProMemo) // foxpro uses another extension
                aURL.SetExtension(String::CreateFromAscii("fpt"));
            else
                aURL.SetExtension(String::CreateFromAscii("dbt"));

            // Wenn die Memodatei nicht gefunden wird, werden die Daten trotzdem angezeigt
            // allerdings koennen keine Updates durchgefuehrt werden
            // jedoch die Operation wird ausgefuehrt
            m_pMemoStream = ::utl::UcbStreamHelper::CreateStream( aURL.GetURLNoPass(), STREAM_READWRITE | STREAM_NOCREATE | STREAM_SHARE_DENYWRITE);
            if (!(m_bWriteableMemo = (NULL != m_pMemoStream)))
                m_pMemoStream = ::utl::UcbStreamHelper::CreateStream( aURL.GetURLNoPass(), STREAM_READ | STREAM_NOCREATE | STREAM_SHARE_DENYNONE );
            if (m_pMemoStream)
                ReadMemoHeader();
        }
        fillColumns();

        m_pFileStream->Seek(STREAM_SEEK_TO_END);
        UINT32 nFileSize = m_pFileStream->Tell();
        m_pFileStream->Seek(STREAM_SEEK_TO_BEGIN);

        // Buffersize abhaengig von der Filegroesse
        m_pFileStream->SetBufferSize(nFileSize > 1000000 ? 32768 :
                                  nFileSize > 100000 ? 16384 :
                                  nFileSize > 10000 ? 4096 : 1024);

        if (m_pMemoStream)
        {
            // Puffer genau auf Laenge eines Satzes stellen
            m_pMemoStream->Seek(STREAM_SEEK_TO_END);
            nFileSize = m_pMemoStream->Tell();
            m_pMemoStream->Seek(STREAM_SEEK_TO_BEGIN);

            // Buffersize abhaengig von der Filegroesse
            m_pMemoStream->SetBufferSize(nFileSize > 1000000 ? 32768 :
                                          nFileSize > 100000 ? 16384 :
                                          nFileSize > 10000 ? 4096 :
                                          m_aMemoHeader.db_size);
        }

        AllocBuffer();
    }
}
//------------------------------------------------------------------
BOOL ODbaseTable::ReadMemoHeader()
{
    m_pMemoStream->SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
    m_pMemoStream->RefreshBuffer();         // sicherstellen das die Kopfinformationen tatsaechlich neu gelesen werden
    m_pMemoStream->Seek(0L);

    (*m_pMemoStream) >> m_aMemoHeader.db_next;
    switch (m_aHeader.db_typ)
    {
        case dBaseIIIMemo:  // dBase III: feste Blockgröße
        case dBaseIVMemo:
            // manchmal wird aber auch dBase3 dBase4 Memo zugeordnet
            m_pMemoStream->Seek(20L);
            (*m_pMemoStream) >> m_aMemoHeader.db_size;
            if (m_aMemoHeader.db_size > 1 && m_aMemoHeader.db_size != 512)  // 1 steht auch fuer dBase 3
                m_aMemoHeader.db_typ  = MemodBaseIV;
            else if (m_aMemoHeader.db_size > 1 && m_aMemoHeader.db_size == 512)
            {
                // nun gibt es noch manche Dateien, die verwenden eine Gößenangabe,
                // sind aber dennoch dBase Dateien
                char sHeader[4];
                m_pMemoStream->Seek(m_aMemoHeader.db_size);
                m_pMemoStream->Read(sHeader,4);

                if ((m_pMemoStream->GetErrorCode() != ERRCODE_NONE) || ((BYTE)sHeader[0]) != 0xFF || ((BYTE)sHeader[1]) != 0xFF || ((BYTE)sHeader[2]) != 0x08)
                    m_aMemoHeader.db_typ  = MemodBaseIII;
                else
                    m_aMemoHeader.db_typ  = MemodBaseIV;
            }
            else
            {
                m_aMemoHeader.db_typ  = MemodBaseIII;
                m_aMemoHeader.db_size = 512;
            }
            break;
        case FoxProMemo:
            m_aMemoHeader.db_typ    = MemoFoxPro;
            m_pMemoStream->Seek(6L);
            m_pMemoStream->SetNumberFormatInt(NUMBERFORMAT_INT_BIGENDIAN);
            (*m_pMemoStream) >> m_aMemoHeader.db_size;
    }
    return TRUE;
}
// -------------------------------------------------------------------------
String ODbaseTable::getEntry()
{
    ::rtl::OUString aURL;
    Reference< XResultSet > xDir = m_pConnection->getDir()->getStaticResultSet();
    Reference< XRow> xRow(xDir,UNO_QUERY);
    ::rtl::OUString sName;
    ::rtl::OUString sExt;
    ::rtl::OUString sNeededExt(m_pConnection->getExtension());
    sal_Int32 nExtLen = sNeededExt.getLength();
    sal_Int32 nExtLenWithSep = nExtLen + 1;
    xDir->beforeFirst();
    while(xDir->next())
    {
        sName = xRow->getString(1);

        // cut the extension
        sExt = sName.copy(sName.getLength() - nExtLen);
        sName = sName.copy(0, sName.getLength() - nExtLenWithSep);

        // name and extension have to coincide
        if ((sName == m_Name) && (sExt == sNeededExt))
        {
            Reference< XContentAccess > xContentAccess( xDir, UNO_QUERY );
            aURL = xContentAccess->queryContentIdentifierString();
            break;
        }
    }
    xDir->beforeFirst(); // move back to before first record
    return aURL.getStr();
}
// -------------------------------------------------------------------------
void ODbaseTable::refreshColumns()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ::std::vector< ::rtl::OUString> aVector;

    for(OSQLColumns::const_iterator aIter = m_aColumns->begin();aIter != m_aColumns->end();++aIter)
        aVector.push_back(Reference< XNamed>(*aIter,UNO_QUERY)->getName());

    if(m_pColumns)
        delete m_pColumns;
    m_pColumns  = new ODbaseColumns(this,m_aMutex,aVector);
}
// -------------------------------------------------------------------------
void ODbaseTable::refreshIndexes()
{
    ::std::vector< ::rtl::OUString> aVector;
    if(m_pFileStream)
    {
        INetURLObject aURL;
        aURL.SetURL(getEntry());

        aURL.setExtension(String::CreateFromAscii("inf"));
        Config aInfFile(aURL.getFSysPath(INetURLObject::FSYS_DETECT));
        aInfFile.SetGroup(dBASE_III_GROUP);
        USHORT nKeyCnt = aInfFile.GetKeyCount();
        ByteString aKeyName;
        ByteString aIndexName;

        for (USHORT nKey = 0,nPos=0; nKey < nKeyCnt; nKey++)
        {
            // Verweist der Key auf ein Indexfile?...
            aKeyName = aInfFile.GetKeyName( nKey );
            //...wenn ja, Indexliste der Tabelle hinzufuegen
            if (aKeyName.Copy(0,3) == ByteString("NDX") )
            {
                aIndexName = aInfFile.ReadKey(aKeyName);
                aURL.setName(String(aIndexName,getConnection()->getTextEncoding()));
                try
                {
                    Content aCnt(aURL.GetURLNoPass(),Reference<XCommandEnvironment>());
                    if (aCnt.isDocument())
                    {
                        aVector.push_back(aURL.getBase());
                    }
                }
                catch(Exception&) // a execption is thrown when no file exists
                {
                }
            }
        }
    }
    if(m_pIndexes)
        delete m_pIndexes;
    m_pIndexes  = new ODbaseIndexes(this,m_aMutex,aVector);
}

// -------------------------------------------------------------------------
void SAL_CALL ODbaseTable::disposing(void)
{
    OFileTable::disposing();
    ::osl::MutexGuard aGuard(m_aMutex);
#ifdef DEBUG
    for(OSQLColumns::const_iterator aIter = m_aColumns->begin();aIter != m_aColumns->end();++aIter)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xProp = *aIter;
        xProp = NULL;
    }
#endif
    m_aColumns->clear();
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL ODbaseTable::getTypes(  ) throw(RuntimeException)
{
    Sequence< Type > aTypes = OTable_TYPEDEF::getTypes();
    Sequence< Type > aRet(aTypes.getLength()-2);
    const Type* pBegin = aTypes.getConstArray();
    const Type* pEnd = pBegin + aTypes.getLength();
    sal_Int32 i=0;
    for(;pBegin != pEnd;++pBegin,++i)
    {
        if(!(*pBegin == ::getCppuType((const Reference<XKeysSupplier>*)0)   ||
            *pBegin == ::getCppuType((const Reference<XAlterTable>*)0)  ||
            *pBegin == ::getCppuType((const Reference<XDataDescriptorFactory>*)0)))
        {
            aRet.getArray()[i] = *pBegin;
        }
    }
    aRet.getArray()[i] = ::getCppuType( (const Reference< ::com::sun::star::lang::XUnoTunnel > *)0 );

    return aRet;
}

// -------------------------------------------------------------------------
Any SAL_CALL ODbaseTable::queryInterface( const Type & rType ) throw(RuntimeException)
{
    if( rType == ::getCppuType((const Reference<XKeysSupplier>*)0)  ||
        rType == ::getCppuType((const Reference<XAlterTable>*)0)    ||
        rType == ::getCppuType((const Reference<XDataDescriptorFactory>*)0))
        return Any();

    Any aRet = ::cppu::queryInterface(rType,static_cast< ::com::sun::star::lang::XUnoTunnel*> (this));
    if(aRet.hasValue())
        return aRet;

    return OTable_TYPEDEF::queryInterface(rType);
}

//--------------------------------------------------------------------------
Sequence< sal_Int8 > ODbaseTable::getUnoTunnelImplementationId()
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
sal_Int64 ODbaseTable::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    if (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return (sal_Int64)this;

    return ODbaseTable_BASE::getSomething(rId);
}
//------------------------------------------------------------------
sal_Bool ODbaseTable::seekRow(FilePosition eCursorPosition, sal_Int32 nOffset, sal_Int32& nCurPos)
{
    // ----------------------------------------------------------
    // Positionierung vorbereiten:

    sal_uInt32  nNumberOfRecords = (sal_uInt32)m_aHeader.db_anz;
    sal_uInt32 nTempPos = m_nFilePos;
    m_nFilePos = nCurPos;

    switch(eCursorPosition)
    {
        case FILE_NEXT:
            m_nFilePos++;
            break;
        case FILE_PRIOR:
            if (m_nFilePos > 0)
                m_nFilePos--;
            break;
        case FILE_FIRST:
            m_nFilePos = 1;
            break;
        case FILE_LAST:
            m_nFilePos = nNumberOfRecords;
            break;
        case FILE_RELATIVE:
            m_nFilePos = (((sal_Int32)m_nFilePos) + nOffset < 0) ? 0L
                            : (sal_uInt32)(((sal_Int32)m_nFilePos) + nOffset);
            break;
        case FILE_ABSOLUTE:
        case FILE_BOOKMARK:
            m_nFilePos = (sal_uInt32)nOffset;
            break;
    }

    if (m_nFilePos > (sal_Int32)nNumberOfRecords)
        m_nFilePos = (sal_Int32)nNumberOfRecords + 1;

    if (m_nFilePos == 0 || m_nFilePos == (sal_Int32)nNumberOfRecords + 1)
        goto Error;
    else
    {
        sal_uInt16 nEntryLen = m_aHeader.db_slng;

        OSL_ENSURE(m_nFilePos >= 1,"SdbDBFCursor::FileFetchRow: ungueltige Record-Position");
        sal_Int32 nPos = m_aHeader.db_kopf + (sal_Int32)(m_nFilePos-1) * nEntryLen;

        ULONG nLen = m_pFileStream->Seek(nPos);
        if (m_pFileStream->GetError() != ERRCODE_NONE)
            goto Error;

        nLen = m_pFileStream->Read((char*)m_pBuffer, nEntryLen);
        if (m_pFileStream->GetError() != ERRCODE_NONE)
            goto Error;
    }
    goto End;

Error:
    switch(eCursorPosition)
    {
        case FILE_PRIOR:
        case FILE_FIRST:
            m_nFilePos = 0;
            break;
        case FILE_LAST:
        case FILE_NEXT:
        case FILE_ABSOLUTE:
        case FILE_RELATIVE:
            if (nOffset > 0)
                m_nFilePos = nNumberOfRecords + 1;
            else if (nOffset < 0)
                m_nFilePos = 0;
            break;
        case FILE_BOOKMARK:
            m_nFilePos = nTempPos;   // vorherige Position
    }
    //  aStatus.Set(SDB_STAT_NO_DATA_FOUND);
    return sal_False;

End:
    nCurPos = m_nFilePos;
    return sal_True;
}
//------------------------------------------------------------------
sal_Bool ODbaseTable::fetchRow(OValueRow _rRow,const OSQLColumns & _rCols, sal_Bool _bUseTableDefs,sal_Bool bRetrieveData)
{
    // Einlesen der Daten
    BOOL bIsCurRecordDeleted = ((char)m_pBuffer[0] == '*') ? TRUE : sal_False;

    // only read the bookmark

    // Satz als geloescht markieren
    //  rRow.setState(bIsCurRecordDeleted ? ROW_DELETED : ROW_CLEAN );
    _rRow->setDeleted(bIsCurRecordDeleted);
    (*_rRow)[0] = m_nFilePos;

    if (!bRetrieveData)
        return TRUE;

    sal_Int32 nByteOffset = 1;
    // Felder:
    OSQLColumns::const_iterator aIter = _rCols.begin();
    for (sal_Int32 i = 1; aIter != _rCols.end();++aIter, i++)
    {
        //  pVal = (*_rRow)[i].getBodyPtr();
        Reference< XPropertySet> xColumn = *aIter;

        ::rtl::OUString aName;
        xColumn->getPropertyValue(PROPERTY_NAME) >>= aName;
        // Laengen je nach Datentyp:
        // nyi: eine zentrale Funktion, die die Laenge liefert!
        sal_Int32 nLen;
        sal_Int32 nType;
        if(_bUseTableDefs)
        {
            nLen    = m_aPrecisions[i-1];
            nType   = m_aTypes[i-1];
        }
        else
        {
            xColumn->getPropertyValue(PROPERTY_PRECISION)   >>= nLen;
            xColumn->getPropertyValue(PROPERTY_TYPE)        >>= nType;
        }
        switch(nType)
        {
            case DataType::DATE:        nLen = 8; break;
            case DataType::DECIMAL:
                if(_bUseTableDefs)
                    nLen = SvDbaseConverter::ConvertPrecisionToDbase(nLen,m_aScales[i-1]);
                else
                    nLen = SvDbaseConverter::ConvertPrecisionToDbase(nLen,getINT32(xColumn->getPropertyValue(PROPERTY_SCALE)));
                break;  // das Vorzeichen und das Komma
            case DataType::BIT:         nLen = 1; break;
            case DataType::LONGVARCHAR: nLen = 10; break;
            case DataType::OTHER:
                nByteOffset += nLen;
                continue;
            default:                    break;
        }

        // Ist die Variable ueberhaupt gebunden?
        if (!(*_rRow)[i].isBound())
        {
            // Nein - naechstes Feld.
            nByteOffset += nLen;
            continue;
        }

        char *pData = (char *) (m_pBuffer + nByteOffset);

        //  (*_rRow)[i].setType(nType);

        if (nType == DataType::CHAR || nType == DataType::VARCHAR)
        {
            char cLast = pData[nLen];
            pData[nLen] = 0;
            String aStr(pData,(xub_StrLen)nLen,getConnection()->getTextEncoding());
            aStr.EraseTrailingChars();

            if (!aStr.Len())                // keine StringLaenge, dann NULL
                (*_rRow)[i].setNull();
            else
            {
                ::rtl::OUString aStr2(aStr);
                (*_rRow)[i] = aStr2;
            }
            pData[nLen] = cLast;
        }
        else
        {
            // Falls Nul-Zeichen im String enthalten sind, in Blanks umwandeln!
            for (sal_Int32 k = 0; k < nLen; k++)
            {
                if (pData[k] == '\0')
                    pData[k] = ' ';
            }

            String aStr(pData, (xub_StrLen)nLen,getConnection()->getTextEncoding());        // Spaces am Anfang und am Ende entfernen:
            aStr.EraseLeadingChars();
            aStr.EraseTrailingChars();

            if (!aStr.Len())
            {
                nByteOffset += nLen;
                (*_rRow)[i].setNull();  // keine Werte -> fertig
                continue;
            }

            switch (nType)
            {
                case DataType::DATE:
                {
                    if (aStr.Len() != nLen)
                    {
                        (*_rRow)[i].setNull();
                        break;
                    }
                    sal_uInt16  nYear   = (sal_uInt16)aStr.Copy( 0, 4 ).ToInt32();
                    sal_uInt16  nMonth  = (sal_uInt16)aStr.Copy( 4, 2 ).ToInt32();
                    sal_uInt16  nDay    = (sal_uInt16)aStr.Copy( 6, 2 ).ToInt32();

                    ::com::sun::star::util::Date aDate(nDay,nMonth,nYear);
                    (*_rRow)[i] = aDate;
                }
                break;
                case DataType::DECIMAL:
                    (*_rRow)[i] = aStr.ToDouble();
                    //  pVal->setDouble(SdbTools::ToDouble(aStr));
                break;
                case DataType::BIT:
                {
                    BOOL b;
                    switch (* ((const char *)pData))
                    {
                        case 'T':
                        case 'Y':
                        case 'J':   b = TRUE; break;
                        default:    b = sal_False; break;
                    }
                    (*_rRow)[i] = b;
                    //  pVal->setDouble(b);
                }
                break;
                case DataType::LONGVARCHAR:
                {
                    long nBlockNo = aStr.ToInt32(); // Blocknummer lesen
                    if (nBlockNo > 0 && m_pMemoStream) // Daten aus Memo-Datei lesen, nur wenn
                    {
                        if (!ReadMemo(nBlockNo, (*_rRow)[i]))
                            break;
                    }
                    else
                        (*_rRow)[i].setNull();
                }   break;
                default:
                    OSL_ASSERT("Falscher Type");
            }
            (*_rRow)[i].setTypeKind(nType);
        }

//      if (aStatus.IsError())
//          break;
        // Und weiter ...
        nByteOffset += nLen;
    }
    return sal_True;
}
//------------------------------------------------------------------
BOOL ODbaseTable::ReadMemo(ULONG nBlockNo, ORowSetValue& aVariable)
{
    BOOL bIsText = TRUE;
    //  SdbConnection* pConnection = GetConnection();

    m_pMemoStream->Seek(nBlockNo * m_aMemoHeader.db_size);
    switch (m_aMemoHeader.db_typ)
    {
        case MemodBaseIII: // dBase III-Memofeld, endet mit Ctrl-Z
        {
            const char cEOF = (char) 0x1a;
            ByteString aBStr;
            static char aBuf[514];
            aBuf[512] = 0;          // sonst kann der Zufall uebel mitspielen
            BOOL bReady = sal_False;

            do
            {
                m_pMemoStream->Read(&aBuf,512);

                USHORT i = 0;
                while (aBuf[i] != cEOF && ++i < 512)
                    ;
                bReady = aBuf[i] == cEOF;

                aBuf[i] = 0;
                aBStr += aBuf;

            } while (!bReady && !m_pMemoStream->IsEof() && aBStr.Len() < STRING_MAXLEN);

            ::rtl::OUString aStr(aBStr.GetBuffer(), aBStr.Len(),getConnection()->getTextEncoding());
            aVariable = Sequence<sal_Int8>(reinterpret_cast<const sal_Int8*>(aStr.getStr()),sizeof(sal_Unicode)*aStr.getLength());

        } break;
        case MemoFoxPro:
        case MemodBaseIV: // dBase IV-Memofeld mit Laengenangabe
        {
            char sHeader[4];
            m_pMemoStream->Read(sHeader,4);
            // Foxpro stores text and binary data
            if (m_aMemoHeader.db_typ == MemoFoxPro)
            {
                if (((BYTE)sHeader[0]) != 0 || ((BYTE)sHeader[1]) != 0 || ((BYTE)sHeader[2]) != 0)
                {
//                  String aText = String(SdbResId(STR_STAT_FILE_INVALID));
//                  aText.SearchAndReplace(String::CreateFromAscii("%%d"),m_pMemoStream->GetFileName());
//                  aText.SearchAndReplace(String::CreateFromAscii("%%t"),aStatus.TypeToString(MEMO));
//                  aStatus.Set(SDB_STAT_ERROR,
//                          String::CreateFromAscii("01000"),
//                          aStatus.CreateErrorMessage(aText),
//                          0, String() );
                    return sal_False;
                }

                bIsText = sHeader[3] != 0;
            }
            else if (((BYTE)sHeader[0]) != 0xFF || ((BYTE)sHeader[1]) != 0xFF || ((BYTE)sHeader[2]) != 0x08)
            {
//              String aText = String(SdbResId(STR_STAT_FILE_INVALID));
//              aText.SearchAndReplace(String::CreateFromAscii("%%d"),m_pMemoStream->GetFileName());
//              aText.SearchAndReplace(String::CreateFromAscii("%%t"),aStatus.TypeToString(MEMO));
//              aStatus.Set(SDB_STAT_ERROR,
//                      String::CreateFromAscii("01000"),
//                      aStatus.CreateErrorMessage(aText),
//                      0, String() );
                return sal_False;
            }

            ULONG nLength;
            (*m_pMemoStream) >> nLength;

            if (m_aMemoHeader.db_typ == MemodBaseIV)
                nLength -= 8;

            //  char cChar;
            if (nLength < STRING_MAXLEN && bIsText)
            {
                ByteString aBStr;
                aBStr.Expand(USHORT (nLength));
                m_pMemoStream->Read(aBStr.AllocBuffer((USHORT)nLength),nLength);
                aBStr.ReleaseBufferAccess();
                ::rtl::OUString aStr(aBStr.GetBuffer(),aBStr.Len(), getConnection()->getTextEncoding());
                aVariable = Sequence<sal_Int8>(reinterpret_cast<const sal_Int8*>(aStr.getStr()),sizeof(sal_Unicode)*aStr.getLength());
            }
            else
            {
                Sequence<sal_Int8> aText(nLength);
                sal_Int8* pData = aText.getArray();
                sal_Char cChar;
                for (ULONG i = 0; i < nLength; i++)
                {
                    m_pMemoStream->Read(&cChar,1);
                    (*pData++) = cChar;
                }
                aVariable = aText;
                //  return sal_False;
            }
        }
    }
    return sal_True;
}
// -------------------------------------------------------------------------
void ODbaseTable::FileClose()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    // falls noch nicht alles geschrieben wurde
    if (m_pMemoStream && m_pMemoStream->IsWritable())
        m_pMemoStream->Flush();

    delete m_pMemoStream;
    m_pMemoStream = NULL;

    ODbaseTable_BASE::FileClose();
}
// -------------------------------------------------------------------------
BOOL ODbaseTable::CreateImpl()
{
    OSL_ENSURE(!m_pFileStream, "SequenceError");

    INetURLObject aURL;
    aURL.SetSmartProtocol(INET_PROT_FILE);
    String aName = getEntry();
    if(!aName.Len())
    {
        ::rtl::OUString aIdent = m_pConnection->getContent()->getIdentifier()->getContentIdentifier();
        aIdent += ::rtl::OUString::createFromAscii("/");
        aIdent += m_Name;
        aName = aIdent.getStr();
    }
    aURL.SetURL(aName);

    if(aURL.getExtension() != m_pConnection->getExtension())
        aURL.setExtension(m_pConnection->getExtension());

    Content aContent(aURL.GetURLNoPass(),Reference<XCommandEnvironment>());
    try
    {
        if (aContent.isDocument())
        {
            // Hack fuer Bug #30609 , nur wenn das File existiert und die Laenge > 0 gibt es einen Fehler
            SvStream* pFileStream = ::utl::UcbStreamHelper::CreateStream( aURL.GetURLNoPass(),STREAM_READ);

            if (pFileStream && pFileStream->Seek(STREAM_SEEK_TO_END))
            {
                //  aStatus.SetError(ERRCODE_IO_ALREADYEXISTS,TABLE,aFile.GetFull());
                return sal_False;
            }
            delete pFileStream;
        }
    }
    catch(Exception&) // a execption is thrown when no file exists
    {
    }

    BOOL bMemoFile = sal_False;

    sal_Bool bOk = CreateFile(aURL, bMemoFile);

    FileClose();

    if (!bOk)
    {
        aContent.executeCommand( rtl::OUString::createFromAscii( "delete" ),bool2any( sal_True ) );
        return sal_False;
    }

    if (bMemoFile)
    {
        String aExt = aURL.getExtension();
        aURL.setExtension(String::CreateFromAscii("dbt"));                      // extension for memo file
        Content aMemo1Content(aURL.GetURLNoPass(),Reference<XCommandEnvironment>());

        sal_Bool bMemoAlreadyExists = sal_False;
        try
        {
            bMemoAlreadyExists = aMemo1Content.isDocument();
        }
        catch(Exception&) // a execption is thrown when no file exists
        {
        }
        if (bMemoAlreadyExists)
        {
            //  aStatus.SetError(ERRCODE_IO_ALREADYEXISTS,MEMO,aFile.GetFull());
            aURL.setExtension(aExt);      // kill dbf file
            Content aMemoContent(aURL.GetURLNoPass(),Reference<XCommandEnvironment>());
            aMemoContent.executeCommand( rtl::OUString::createFromAscii( "delete" ),bool2any( sal_True ) );

            ::rtl::OUString sMessage = ::rtl::OUString::createFromAscii("[StarOffice Base dbase] The memo file '");
            sMessage += aName;
            sMessage += ::rtl::OUString::createFromAscii(" already exists.");
            throwGenericSQLException(sMessage, static_cast<XNamed*>(this));
        }
        if (!CreateMemoFile(aURL))
        {
            aURL.setExtension(aExt);      // kill dbf file
            Content aMemoContent(aURL.GetURLNoPass(),Reference<XCommandEnvironment>());
            aMemoContent.executeCommand( rtl::OUString::createFromAscii( "delete" ),bool2any( sal_True ) );
            return sal_False;
        }
        m_aHeader.db_typ = dBaseIIIMemo;
    }
    else
        m_aHeader.db_typ = dBaseIII;

//  if (GetDBFConnection()->GetShowDeleted())
//      nPrivileges = SDB_PR_READ | SDB_PR_INSERT | SDB_PR_UPDATE |
//          SDB_PR_ALTER | SDB_PR_DROP;
//  else
        //  nPrivileges = SDB_PR_READ | SDB_PR_INSERT | SDB_PR_UPDATE |
            //  SDB_PR_DELETE | SDB_PR_ALTER | SDB_PR_DROP;

    return TRUE;
}

//------------------------------------------------------------------
// erzeugt grundsätzlich dBase IV Datei Format
BOOL ODbaseTable::CreateFile(const INetURLObject& aFile, BOOL& bCreateMemo)
{
    bCreateMemo = sal_False;
    Date aDate;                                     // aktuelles Datum

    m_pFileStream = ::utl::UcbStreamHelper::CreateStream( aFile.GetURLNoPass(),STREAM_READWRITE | STREAM_SHARE_DENYWRITE | STREAM_TRUNC);

    if (!m_pFileStream)
        return sal_False;

    char aBuffer[21];               // write buffer
    memset(aBuffer,0,sizeof(aBuffer));

    m_pFileStream->Seek(0L);
    (*m_pFileStream) << (BYTE) dBaseIII;                                                                // dBase format
    (*m_pFileStream) << (BYTE) (aDate.GetYear() % 100);                 // aktuelles Datum


    (*m_pFileStream) << (BYTE) aDate.GetMonth();
    (*m_pFileStream) << (BYTE) aDate.GetDay();
    (*m_pFileStream) << 0L;                                                                                                     // Anzahl der Datensätze
    (*m_pFileStream) << (USHORT)((m_pColumns->getCount()+1) * 32 + 1);                // Kopfinformationen,
                                                                        // pColumns erhält immer eine Spalte mehr
    (*m_pFileStream) << (USHORT) 0;                                                                                     // Satzlänge wird später bestimmt
    m_pFileStream->Write(aBuffer, 20);

    USHORT nRecLength = 1;                                                                                          // Länge 1 für deleted flag
    sal_Int32  nMaxFieldLength = m_pConnection->getMetaData()->getMaxColumnNameLength();
    Reference<XIndexAccess> xColumns(getColumns(),UNO_QUERY);

    ::rtl::OUString aName;
    Reference<XPropertySet> xCol;
    for(sal_Int32 i=0;i<xColumns->getCount();++i)
    {
        ::cppu::extractInterface(xCol,xColumns->getByIndex(i));
        OSL_ENSURE(xCol.is(),"This should be a column!");

        char  cTyp;

        xCol->getPropertyValue(PROPERTY_NAME) >>= aName;

        if (aName.getLength() > nMaxFieldLength)
        {
            ::rtl::OUString sMsg = ::rtl::OUString::createFromAscii("Invalid column name length for column: ");
            sMsg += aName;
            sMsg += ::rtl::OUString::createFromAscii("!");
            throw SQLException(sMsg,*this,::rtl::OUString::createFromAscii("HY0000"),1000,Any());
        }

        ByteString aCol(aName.getStr(), getConnection()->getTextEncoding());
        (*m_pFileStream) << aCol.GetBuffer();
        m_pFileStream->Write(aBuffer, 11 - aCol.Len());

        switch (getINT32(xCol->getPropertyValue(PROPERTY_TYPE)))
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
                cTyp = 'C';
                break;
            case DataType::TINYINT:
            case DataType::SMALLINT:
            case DataType::INTEGER:
            case DataType::BIGINT:
            case DataType::DECIMAL:
            case DataType::NUMERIC:
            case DataType::REAL:
            case DataType::DOUBLE:
                cTyp = 'N';                             // nur dBase 3 format
                break;
            case DataType::DATE:
                cTyp = 'D';
                break;
            case DataType::BIT:
                cTyp = 'L';
                break;
            case DataType::LONGVARBINARY:
            case DataType::LONGVARCHAR:
                cTyp = 'M';
                break;
            default:
                {
                    ::rtl::OUString sMsg = ::rtl::OUString::createFromAscii("Invalid column type for column: ");
                    sMsg += aName;
                    sMsg += ::rtl::OUString::createFromAscii("!");
                    throw SQLException(sMsg,*this,::rtl::OUString::createFromAscii("HY0000"),1000,Any());
                }
        }

        (*m_pFileStream) << cTyp;
        m_pFileStream->Write(aBuffer, 4);

        sal_Int32 nPrecision = 0;
        xCol->getPropertyValue(PROPERTY_PRECISION) >>= nPrecision;
        sal_Int32 nScale = 0;
        xCol->getPropertyValue(PROPERTY_SCALE) >>= nScale;

        switch(cTyp)
        {
            case 'C':
                OSL_ENSURE(nPrecision < 255, "ODbaseTable::Create: Column zu lang!");
                if (nPrecision > 254)
                {
                    ::rtl::OUString sMsg = ::rtl::OUString::createFromAscii("Invalid precision for column: ");
                    sMsg += aName;
                    sMsg += ::rtl::OUString::createFromAscii("!");
                    throw SQLException(sMsg,*this,::rtl::OUString::createFromAscii("HY0000"),1000,Any());
                }
                (*m_pFileStream) << (BYTE) Min((ULONG)nPrecision, 255UL);      //Feldlänge
                nRecLength += (USHORT)Min((ULONG)nPrecision, 255UL);
                (*m_pFileStream) << (BYTE)0;                                                                //Nachkommastellen
                break;
            case 'F':
            case 'N':
                OSL_ENSURE(nPrecision >=  nScale,
                           "ODbaseTable::Create: Feldlänge muß größer Nachkommastellen sein!");
                if (nPrecision <  nScale)
                {
                    ::rtl::OUString sMsg = ::rtl::OUString::createFromAscii("Precision is less than scale for column: ");
                    sMsg += aName;
                    sMsg += ::rtl::OUString::createFromAscii("!");
                    throw SQLException(sMsg,*this,::rtl::OUString::createFromAscii("HY0000"),1000,Any());
                    break;
                }
                if (getBOOL(xCol->getPropertyValue(PROPERTY_ISCURRENCY))) // Currency wird gesondert behandelt
                {
                    (*m_pFileStream) << (BYTE)10;          // Standard Laenge
                    (*m_pFileStream) << (BYTE)4;
                    nRecLength += 10;
                }
                else
                {
                    sal_Int32 nPrec = SvDbaseConverter::ConvertPrecisionToDbase(nPrecision,nScale);

                    (*m_pFileStream) << (BYTE)( nPrec);
                    (*m_pFileStream) << (BYTE)nScale;
                    nRecLength += (USHORT)nPrec;
                }
                break;
            case 'L':
                (*m_pFileStream) << (BYTE)1;
                (*m_pFileStream) << (BYTE)0;
                nRecLength++;
                break;
            case 'D':
                (*m_pFileStream) << (BYTE)8;
                (*m_pFileStream) << (BYTE)0;
                nRecLength += 8;
                break;
            case 'M':
                bCreateMemo = TRUE;
                (*m_pFileStream) << (BYTE)10;
                (*m_pFileStream) << (BYTE)0;
                nRecLength += 10;
                break;
            default:
                {
                    ::rtl::OUString sMsg = ::rtl::OUString::createFromAscii("Invalid column type for column: ");
                    sMsg += aName;
                    sMsg += ::rtl::OUString::createFromAscii("!");
                    throw SQLException(sMsg,*this,::rtl::OUString::createFromAscii("HY0000"),1000,Any());
                }
        }
        m_pFileStream->Write(aBuffer, 14);
    }

    (*m_pFileStream) << (BYTE)0x0d;                                     // kopf ende
    m_pFileStream->Seek(10L);
    (*m_pFileStream) << nRecLength;                                     // satzlänge nachträglich eintragen

    if (bCreateMemo)
    {
        m_pFileStream->Seek(0L);
        (*m_pFileStream) << (BYTE) dBaseIIIMemo;
    }
    return TRUE;
}

//------------------------------------------------------------------
// erzeugt grundsätzlich dBase III Datei Format
BOOL ODbaseTable::CreateMemoFile(const INetURLObject& aFile)
{
    // Makro zum Filehandling fürs Erzeugen von Tabellen
    m_pMemoStream = ::utl::UcbStreamHelper::CreateStream( aFile.GetURLNoPass(),STREAM_READWRITE | STREAM_SHARE_DENYWRITE);

    if (!m_pMemoStream)
        return sal_False;

    char aBuffer[512];              // write buffer
    memset(aBuffer,0,sizeof(aBuffer));

#ifdef WIN
    m_pMemoStream->Seek(0L);
    for (UINT16 i = 0; i < 512; i++)
    {
        (*m_pMemoStream) << BYTE(0);
    }
#else
    m_pMemoStream->SetFiller('\0');
    m_pMemoStream->SetStreamSize(512);
#endif

    m_pMemoStream->Seek(0L);
    (*m_pMemoStream) << long(1);                  // Zeiger auf ersten freien Block

    m_pMemoStream->Flush();
    delete m_pMemoStream;
    m_pMemoStream = NULL;
    return TRUE;
}
//------------------------------------------------------------------
BOOL ODbaseTable::DropImpl()
{
//  NAMESPACE_VOS(OGuard) aGuard(m_pLock);
//
//  if (InUse())
//  {
//      aStatus.SetError(ERRCODE_IO_LOCKVIOLATION,TABLE,aName);
//      return sal_False;
//  }

    FileClose();

    INetURLObject aURL;
    aURL.SetURL(getEntry());

    Content aContent(aURL.GetURLNoPass(),Reference<XCommandEnvironment>());
    aContent.executeCommand( rtl::OUString::createFromAscii( "delete" ),
                                 makeAny( sal_Bool( sal_True ) ) );

    if (HasMemoFields())
    {
        aURL.setExtension(String::CreateFromAscii("dbt"));
        Content aMemoContent(aURL.GetURLNoPass(),Reference<XCommandEnvironment>());
        aMemoContent.executeCommand( rtl::OUString::createFromAscii( "delete" ),bool2any( sal_True ) );
    }

    // jetzt noch die Indices loeschen
    String aIndexName;
    //  aFile.SetExtension(String::CreateFromAscii("ndx"));
    refreshIndexes(); // look for indexes which must be deleted as well
    if(m_pIndexes)
    {
        sal_Int32 nCount = m_pIndexes->getCount(),
               i      = 0;
        while (i < nCount)
        {
            m_pIndexes->dropByIndex(i);
        }
    }
    //  aFile.SetBase(m_Name);
    aURL.setExtension(String::CreateFromAscii("inf"));
    Content aInfContent(aURL.GetURLNoPass(),Reference<XCommandEnvironment>());
    aInfContent.executeCommand( rtl::OUString::createFromAscii( "delete" ),bool2any( sal_True ) );
    return TRUE;
}
//------------------------------------------------------------------
BOOL ODbaseTable::InsertRow(OValueVector& rRow, BOOL bFlush,const Reference<XIndexAccess>& _xCols)
{
    // Buffer mit Leerzeichen füllen
    AllocBuffer();
    memset(m_pBuffer, ' ', m_aHeader.db_slng);

    // Gesamte neue Row uebernehmen:
    // ... und am Ende als neuen Record hinzufuegen:
    UINT32 nTempPos = m_nFilePos,
           nFileSize,
           nMemoFileSize;

    m_nFilePos = (ULONG)m_aHeader.db_anz + 1;
    if (!UpdateBuffer(rRow,NULL,_xCols))
    {
        m_nFilePos = nTempPos;
        return sal_False;
    }

    m_pFileStream->Seek(STREAM_SEEK_TO_END);
    nFileSize = m_pFileStream->Tell();

    if (HasMemoFields() && m_pMemoStream)
    {
        m_pMemoStream->Seek(STREAM_SEEK_TO_END);
        nMemoFileSize = m_pMemoStream->Tell();
    }

    if (!WriteBuffer())
    {
        m_pFileStream->SetStreamSize(nFileSize);                // alte Größe restaurieren

        if (HasMemoFields() && m_pMemoStream)
            m_pMemoStream->SetStreamSize(nMemoFileSize);    // alte Größe restaurieren
        m_nFilePos = nTempPos;                              // Fileposition restaurieren
    }
    else
    {
        // Anzahl Datensaetze im Header erhoehen:
        m_pFileStream->Seek( 4L );
        (*m_pFileStream) << (m_aHeader.db_anz + 1);

        // beim AppendOnly kein Flush!
        if (bFlush)
            m_pFileStream->Flush();

        // bei Erfolg # erhöhen
        m_aHeader.db_anz++;
        rRow[0] = m_nFilePos;                               // BOOKmark setzen
        m_nFilePos = nTempPos;
    }

    return sal_True;;
}

//------------------------------------------------------------------
BOOL ODbaseTable::UpdateRow(OValueVector& rRow, OValueRow pOrgRow,const Reference<XIndexAccess>& _xCols)
{
    // Buffer mit Leerzeichen füllen
    AllocBuffer();

    // Auf gewuenschten Record positionieren:
    long nPos = m_aHeader.db_kopf + (long)(m_nFilePos-1) * m_aHeader.db_slng;
    m_pFileStream->Seek(nPos);
    m_pFileStream->Read((char*)m_pBuffer, m_aHeader.db_slng);

    UINT32 nMemoFileSize;
    if (HasMemoFields() && m_pMemoStream)
    {
        m_pMemoStream->Seek(STREAM_SEEK_TO_END);
        nMemoFileSize = m_pMemoStream->Tell();
    }
    if (!UpdateBuffer(rRow, pOrgRow,_xCols) || !WriteBuffer())
    {
        if (HasMemoFields() && m_pMemoStream)
            m_pMemoStream->SetStreamSize(nMemoFileSize);    // alte Größe restaurieren
    }
    else
    {
        m_pFileStream->Flush();
    }
    return sal_True;
}

//------------------------------------------------------------------
BOOL ODbaseTable::DeleteRow(const OSQLColumns& _rCols)
{
    // Einfach das Loesch-Flag setzen (egal, ob es schon gesetzt war
    // oder nicht):
    // Auf gewuenschten Record positionieren:
    long nPos = m_aHeader.db_kopf + (long)(m_nFilePos-1) * m_aHeader.db_slng;
    m_pFileStream->Seek(nPos);

    OValueRow aRow = new OValueVector(_rCols.size());

    if (!fetchRow(aRow,_rCols,TRUE,TRUE))
        return sal_False;

    Reference<XPropertySet> xCol;
    ::rtl::OUString aColName;
    ::comphelper::UStringMixEqual aCase(isCaseSensitive());
    for (USHORT i = 0; i < m_pColumns->getCount(); i++)
    {
        ::cppu::extractInterface(xCol,m_pColumns->getByIndex(i));
        //  const SdbFILEColumn *pColumn = (const SdbFILEColumn *)(*aOriginalColumns)[i];

        xCol->getPropertyValue(PROPERTY_NAME) >>= aColName;
        Reference<XPropertySet> xIndex = isUniqueByColumnName(aColName);
        if (xIndex.is())
        {
            Reference<XUnoTunnel> xTunnel(xIndex,UNO_QUERY);
            OSL_ENSURE(xTunnel.is(),"No TunnelImplementation!");
            ODbaseIndex* pIndex = (ODbaseIndex*)xTunnel->getSomething(ODbaseIndex::getUnoTunnelImplementationId());
            OSL_ENSURE(pIndex,"ODbaseTable::UpdateBuffer: No Index returned!");

            OSQLColumns::const_iterator aIter = _rCols.begin();
            //  sal_Int32 nPos = 0;
            for(;aIter != _rCols.end();++aIter,++nPos)
            {
//              Reference<XPropertySet> xFindCol;
//              _xCols->getByIndex(nPos) >>= xFindCol;
                if(aCase(getString((*aIter)->getPropertyValue(PROPERTY_REALNAME)),aColName))
                    break;
            }
            if (aIter == _rCols.end())
                continue;

            pIndex->Delete(m_nFilePos,(*aRow)[nPos]);
        }
    }

    m_pFileStream->Seek(nPos);
    (*m_pFileStream) << (BYTE)'*';
    m_pFileStream->Flush();
    return sal_True;;
}

//------------------------------------------------------------------
BOOL ODbaseTable::WriteMemo(ORowSetValue& aVariable, ULONG& rBlockNr)
{
    // wird die BlockNr 0 vorgegeben, wird der block ans Ende gehaengt
    char cChar = 0;
    BOOL bIsText = TRUE;
    //  SdbConnection* pConnection = GetConnection();

    ULONG nSize = 0;
    ULONG nStreamSize;
    BYTE nHeader[4];

    ByteString aStr;
        //      ::Sequence<sal_Int8>* pData = NULL;
//  if (aVariable.getValueType() == ::getCppuType((const ::com::sun::star::uno::Sequence< sal_Int8 > *)0))
//  {
//              pData = (::Sequence<sal_Int8>*)aVariable.get();
//      nSize = pData->getLength();
//  }
//  else
//  {
        aStr = ByteString(String(aVariable.getString()), getConnection()->getTextEncoding());
        nSize = aStr.Len();
    //  }

    // Anhaengen oder ueberschreiben
    BOOL bAppend = rBlockNr == 0;

    if (!bAppend)
    {
        switch (m_aMemoHeader.db_typ)
        {
            case MemodBaseIII: // dBase III-Memofeld, endet mit 2 * Ctrl-Z
                bAppend = nSize > (512 - 2);
                break;
            case MemoFoxPro:
            case MemodBaseIV: // dBase IV-Memofeld mit Laengenangabe
            {
                char sHeader[4];
                m_pMemoStream->Seek(rBlockNr * m_aMemoHeader.db_size);
                m_pMemoStream->SeekRel(4L);
                m_pMemoStream->Read(sHeader,4);

                ULONG nOldSize;
                if (m_aMemoHeader.db_typ == MemoFoxPro)
                    nOldSize = ((((unsigned char)sHeader[0]) * 256 +
                                 (unsigned char)sHeader[1]) * 256 +
                                 (unsigned char)sHeader[2]) * 256 +
                                 (unsigned char)sHeader[3];
                else
                    nOldSize = ((((unsigned char)sHeader[3]) * 256 +
                                 (unsigned char)sHeader[2]) * 256 +
                                 (unsigned char)sHeader[1]) * 256 +
                                 (unsigned char)sHeader[0]  - 8;

                // passt die neue Laenge in die belegten Bloecke
                ULONG nUsedBlocks = ((nSize + 8) / m_aMemoHeader.db_size) + (((nSize + 8) % m_aMemoHeader.db_size > 0) ? 1 : 0),
                      nOldUsedBlocks = ((nOldSize + 8) / m_aMemoHeader.db_size) + (((nOldSize + 8) % m_aMemoHeader.db_size > 0) ? 1 : 0);
                bAppend = nUsedBlocks > nOldUsedBlocks;
            }
        }
    }

    if (bAppend)
    {
        ULONG nStreamSize;
        nStreamSize = m_pMemoStream->Seek(STREAM_SEEK_TO_END);
        // letzten block auffuellen
        rBlockNr = (nStreamSize / m_aMemoHeader.db_size) + ((nStreamSize % m_aMemoHeader.db_size) > 0 ? 1 : 0);

        m_pMemoStream->SetStreamSize(rBlockNr * m_aMemoHeader.db_size);
        m_pMemoStream->Seek(STREAM_SEEK_TO_END);
    }
    else
    {
        m_pMemoStream->Seek(rBlockNr * m_aMemoHeader.db_size);
    }

    switch (m_aMemoHeader.db_typ)
    {
        case MemodBaseIII: // dBase III-Memofeld, endet mit Ctrl-Z
        {
            const char cEOF = (char) 0x1a;
            nSize++;

//          if (pData)
//          {
//              m_pMemoStream->Write((const char*) pData->getConstArray(), pData->getLength());
//          }
//          else
//          {
                m_pMemoStream->Write(aStr.GetBuffer(), aStr.Len());
            //  }

            (*m_pMemoStream) << cEOF << cEOF;
        } break;
        case MemoFoxPro:
        case MemodBaseIV: // dBase IV-Memofeld mit Laengenangabe
        {
            (*m_pMemoStream) << (BYTE)0xFF
                                         << (BYTE)0xFF
                                         << (BYTE)0x08;

            UINT32 nWriteSize = nSize;
            if (m_aMemoHeader.db_typ == MemoFoxPro)
            {
                (*m_pMemoStream) << (BYTE) 0x01; // ((pData = NULL) ? 0x01 : 0x00);
                for (int i = 4; i > 0; nWriteSize >>= 8)
                    nHeader[--i] = (BYTE) (nWriteSize % 256);
            }
            else
            {
                (*m_pMemoStream) << (BYTE) 0x00;
                nWriteSize += 8;
                for (int i = 0; i < 4; nWriteSize >>= 8)
                    nHeader[i++] = (BYTE) (nWriteSize % 256);
            }

            m_pMemoStream->Write(nHeader,4);
//          if (pData)
//          {
//              m_pMemoStream->Write((const char*) pData->getConstArray(), pData->getLength());
//          }
//          else
//          {
                m_pMemoStream->Write(aStr.GetBuffer(), aStr.Len());
            //  }
            m_pMemoStream->Flush();
        }
    }


    // Schreiben der neuen Blocknummer
    if (bAppend)
    {
        nStreamSize = m_pMemoStream->Seek(STREAM_SEEK_TO_END);
        m_aMemoHeader.db_next = (nStreamSize / m_aMemoHeader.db_size) + ((nStreamSize % m_aMemoHeader.db_size) > 0 ? 1 : 0);

        // Schreiben der neuen Blocknummer
        m_pMemoStream->Seek(0L);
        (*m_pMemoStream) << m_aMemoHeader.db_next;
        m_pMemoStream->Flush();
    }
    return sal_True;
}
//------------------------------------------------------------------
void ODbaseTable::AllocBuffer()
{
    UINT16 nSize = m_aHeader.db_slng;
    OSL_ENSURE(nSize > 0, "Size too small");

    if (m_nBufferSize != nSize)
    {
        delete m_pBuffer;
        m_pBuffer = NULL;
    }

    // Falls noch kein Puffer vorhanden: allozieren:
    if (m_pBuffer == NULL && nSize)
    {
        m_nBufferSize = nSize;
        m_pBuffer       = new BYTE[m_nBufferSize+1];
    }
}
// -------------------------------------------------------------------------
Reference<XPropertySet> ODbaseTable::isUniqueByColumnName(const ::rtl::OUString& _rColName)
{
    if(!m_pIndexes)
        refreshIndexes();
    Reference<XPropertySet> xIndex;
    for(sal_Int32 i=0;i<m_pIndexes->getCount();++i)
    {
        ::cppu::extractInterface(xIndex,m_pIndexes->getByIndex(i));
        if(xIndex.is() && getBOOL(xIndex->getPropertyValue(PROPERTY_ISUNIQUE)))
        {
            Reference<XNameAccess> xCols(Reference<XColumnsSupplier>(xIndex,UNO_QUERY)->getColumns());
            if(xCols->hasByName(_rColName))
                return xIndex;

        }
    }
    return Reference<XPropertySet>();
}
//------------------------------------------------------------------
double toDouble(const ByteString& rString)
{
    static International aInter(LANGUAGE_ENGLISH);
    static int nErrno=0;
    BOOL bInitialized = sal_False;
    if (!bInitialized)
    {   // ensure that the two members we're interested in are really set
        // (if the system doesn't know the locale en_US aIntl would be initialized with the
        // system language which may be anything - which we don't want ...)
        // 74342 - 21.03.00 - FS
        aInter.SetNumThousandSep(',');
        aInter.SetNumDecimalSep('.');
        bInitialized = TRUE;
    }
    return SolarMath::StringToDouble(UniString(rString,gsl_getSystemTextEncoding()).GetBuffer(),aInter,nErrno);
}

//------------------------------------------------------------------
BOOL ODbaseTable::UpdateBuffer(OValueVector& rRow, OValueRow pOrgRow,const Reference<XIndexAccess>& _xCols)
{
    sal_Int32 nByteOffset  = 1;

    // Felder aktualisieren:
    Reference<XPropertySet> xCol;
    Reference<XPropertySet> xIndex;
    USHORT i;
    ::rtl::OUString aColName;
    ::std::vector< Reference<XPropertySet> > aIndexedCols(m_pColumns->getCount());

    ::comphelper::UStringMixEqual aCase(isCaseSensitive());

    // first search a key that exist already in the table
    for (i = 0; i < m_pColumns->getCount(); i++)
    {
        ::cppu::extractInterface(xCol,m_pColumns->getByIndex(i));
        xCol->getPropertyValue(PROPERTY_NAME) >>= aColName;

        //  const SdbFILEColumn *pColumn = (const SdbFILEColumn *)(*aOriginalColumns)[i];
        sal_Int32 nPos = 0;
        for(;nPos<_xCols->getCount();++nPos)
        {
            Reference<XPropertySet> xFindCol;
            ::cppu::extractInterface(xFindCol,_xCols->getByIndex(nPos));
            if(aCase(getString(xFindCol->getPropertyValue(PROPERTY_NAME)),aColName))
                break;
        }
        if (nPos >= _xCols->getCount())
            continue;

        ++nPos;
        xIndex = isUniqueByColumnName(aColName);
        aIndexedCols[i] = xIndex;
        if (xIndex.is())
        {
            // first check if the value is different to the old one and when if it conform to the index
            if(pOrgRow.isValid() && (rRow[nPos].isNull() || rRow[nPos] == (*pOrgRow)[nPos]))
                continue;
            else
            {
                //  ODbVariantRef xVar = (pVal == NULL) ? new ODbVariant() : pVal;
                Reference<XUnoTunnel> xTunnel(xIndex,UNO_QUERY);
                OSL_ENSURE(xTunnel.is(),"No TunnelImplementation!");
                ODbaseIndex* pIndex = (ODbaseIndex*)xTunnel->getSomething(ODbaseIndex::getUnoTunnelImplementationId());
                OSL_ENSURE(pIndex,"ODbaseTable::UpdateBuffer: No Index returned!");

                if (pIndex->Find(0,rRow[nPos]))
                {
                    // es existiert kein eindeutiger Wert
                    ::rtl::OUString sMsg = ::rtl::OUString::createFromAscii("Dupilcate value found!");
                    throw SQLException(sMsg,*this,::rtl::OUString::createFromAscii("HY0000"),1000,Any());
                }
            }
        }
    }

    // when we are here there is no double key in the table

    for (i = 0; i < m_pColumns->getCount(); i++)
    {
        ::cppu::extractInterface(xCol,m_pColumns->getByIndex(i));
        xCol->getPropertyValue(PROPERTY_NAME) >>= aColName;

        // Laengen je nach Datentyp:
        // nyi: eine zentrale Funktion, die die Laenge liefert!
        sal_Int32 nLen = getINT32(xCol->getPropertyValue(PROPERTY_PRECISION));
        sal_Int32 nType = getINT32(xCol->getPropertyValue(PROPERTY_TYPE));
        switch (nType)
        {
            case DataType::DATE:        nLen = 8; break;
            case DataType::DECIMAL:
                nLen = SvDbaseConverter::ConvertPrecisionToDbase(nLen,getINT32(xCol->getPropertyValue(PROPERTY_SCALE)));
                break;  // das Vorzeichen und das Komma
            case DataType::BIT: nLen = 1; break;
            case DataType::LONGVARCHAR:nLen = 10; break;
            default:                    break;

        }

        sal_Int32 nPos = 0;
        for(;nPos<_xCols->getCount();++nPos)
        {
            Reference<XPropertySet> xFindCol;
            ::cppu::extractInterface(xFindCol,_xCols->getByIndex(nPos));
            if(aCase(getString(xFindCol->getPropertyValue(PROPERTY_NAME)),aColName))
                break;
        }

        if (nPos >= _xCols->getCount())
        {
            nByteOffset += nLen;
            continue;
        }

        ++nPos; // the row values start at 1
        if (aIndexedCols[i].is())
        {
            Reference<XUnoTunnel> xTunnel(aIndexedCols[i],UNO_QUERY);
            OSL_ENSURE(xTunnel.is(),"No TunnelImplementation!");
            ODbaseIndex* pIndex = (ODbaseIndex*)xTunnel->getSomething(ODbaseIndex::getUnoTunnelImplementationId());
            OSL_ENSURE(pIndex,"ODbaseTable::UpdateBuffer: No Index returned!");
            // Update !!
            if (pOrgRow.isValid() && !rRow[nPos].isNull() )//&& pVal->isModified())
                pIndex->Update(m_nFilePos,(*pOrgRow)[nPos],rRow[nPos]);
            else
            {
                //  ODbVariantRef xVar = (pVal == NULL) ? new ODbVariant() : pVal;
                pIndex->Insert(m_nFilePos,rRow[nPos]);
            }
        }

        // Ist die Variable ueberhaupt gebunden?
        if (!rRow[nPos].isBound() )
        {
            // Nein - naechstes Feld.
            nByteOffset += nLen;
            continue;
        }

        char* pData = (char *)(m_pBuffer + nByteOffset);
        if (rRow[nPos].isNull())
        {
            memset(pData,' ',nLen); // Zuruecksetzen auf NULL
            nByteOffset += nLen;
            continue;
        }
        try
        {
            switch (nType)
            {
                case DataType::DATE:
                {
                    ::com::sun::star::util::Date aDate;
                    if(rRow[nPos].getTypeKind() == DataType::DOUBLE)
                        aDate = ::dbtools::DBTypeConversion::toDate(rRow[nPos].getDouble());
                    else
                        aDate = rRow[nPos];
                    char s[9];
                    sprintf(s,"%04d%02d%02d",
                        (int)aDate.Year,
                        (int)aDate.Month,
                        (int)aDate.Day);

                    // Genau 8 Byte kopieren:
                    strncpy(pData,s,sizeof s - 1);
                } break;
                case DataType::DECIMAL:
                {
                    memset(pData,' ',nLen); // Zuruecksetzen auf NULL

                    double n = rRow[nPos];

                    int nPrecision      = (int)getINT32(xCol->getPropertyValue(PROPERTY_PRECISION));
                    int nScale          = (int)getINT32(xCol->getPropertyValue(PROPERTY_SCALE));
                    // ein const_cast, da GetFormatPrecision am SvNumberFormat nicht const ist, obwohl es das eigentlich
                    // sein koennte und muesste

                    String aString;
                    SolarMath::DoubleToString(aString,n,'F',nScale,'.');
                    ByteString aDefaultValue(aString, getConnection()->getTextEncoding());
                    BOOL bValidLength   = sal_False;
                    if (aDefaultValue.Len() <= nLen)
                    {
                        strncpy(pData,aDefaultValue.GetBuffer(),nLen);
                        // write the resulting double back
                        rRow[nPos] = toDouble(aDefaultValue);
                        bValidLength = TRUE;
                    }
                    if (!bValidLength)
                    {
                        String sError;
                        sError.AppendAscii("The ");
                        sError += aColName.getStr();
                        sError.AppendAscii(" column has been defined as a \"Decimal\" type, the max. length is ");
                        sError += String::CreateFromInt32(nPrecision);
                        sError.AppendAscii(" characters (with ");
                        sError += String::CreateFromInt32(nScale);
                        sError.AppendAscii(" decimal places).\n\nThe specified value is longer than the number of digits allowed.");
                        throwGenericSQLException(sError, static_cast<XNamed*>(this));
                    }
                } break;
                case DataType::BIT:
                    *pData = rRow[nPos].getBool() ? 'T' : 'F';
                    break;
                case DataType::LONGVARCHAR:
                {
                    char cNext = pData[nLen]; // merken und temporaer durch 0 ersetzen
                    pData[nLen] = '\0';       // das geht, da der Puffer immer ein Zeichen groesser ist ...

                    ULONG nBlockNo = strtol((const char *)pData,NULL,10);   // Blocknummer lesen

                    // Naechstes Anfangszeichen wieder restaurieren:
                    pData[nLen] = cNext;
                    if (!m_pMemoStream || !WriteMemo(rRow[nPos], nBlockNo))
                        break;

                    ByteString aStr;
                    ByteString aBlock(ByteString::CreateFromInt32(nBlockNo));
                    aStr.Expand(nLen - aBlock.Len(), '0');
                    aStr += aBlock;
                    aStr.Convert(gsl_getSystemTextEncoding(),getConnection()->getTextEncoding());
                    // Zeichen kopieren:
                    memset(pData,' ',nLen); // Zuruecksetzen auf NULL
                    memcpy(pData, aStr.GetBuffer(), nLen);
                }   break;
                default:
                {
                    memset(pData,' ',nLen); // Zuruecksetzen auf NULL
                    ByteString aStr(rRow[nPos].getString().getStr(),getConnection()->getTextEncoding());
                    // Zeichen kopieren:
                    memcpy(pData, aStr.GetBuffer(), min(nLen,(sal_Int32)aStr.Len()));
                }   break;
            }
        }
        catch ( Exception& )
        {
            ::rtl::OUString sMsg = ::rtl::OUString::createFromAscii("Invalid value for column: ");
            sMsg += aColName;
            sMsg += ::rtl::OUString::createFromAscii("!");
            throw SQLException(sMsg,*this,::rtl::OUString::createFromAscii("HY0000"),1000,Any());
        }
        // Und weiter ...
        nByteOffset += nLen;
    }
    return sal_True;
}


//------------------------------------------------------------------
BOOL ODbaseTable::WriteBuffer()
{
    OSL_ENSURE(m_nFilePos >= 1,"SdbDBFCursor::FileFetchRow: ungueltige Record-Position");

    // Auf gewuenschten Record positionieren:
    long nPos = m_aHeader.db_kopf + (long)(m_nFilePos-1) * m_aHeader.db_slng;
    m_pFileStream->Seek(nPos);
    return m_pFileStream->Write((char*) m_pBuffer, m_aHeader.db_slng) > 0;
}
// -----------------------------------------------------------------------------
// XAlterTable
void SAL_CALL ODbaseTable::alterColumnByName( const ::rtl::OUString& colName, const Reference< XPropertySet >& descriptor ) throw(SQLException, NoSuchElementException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if (ODbaseTable_BASE::rBHelper.bDisposed)
        throw DisposedException();

    Reference<XDataDescriptorFactory> xOldColumn;
    m_pColumns->getByName(colName) >>= xOldColumn;

    alterColumn(m_pColumns->findColumn(colName),descriptor,xOldColumn);
}
// -------------------------------------------------------------------------
void SAL_CALL ODbaseTable::alterColumnByIndex( sal_Int32 index, const Reference< XPropertySet >& descriptor ) throw(SQLException, ::com::sun::star::lang::IndexOutOfBoundsException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if (ODbaseTable_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(index < 0 || index >= m_pColumns->getCount())
        throw IndexOutOfBoundsException();

    Reference<XDataDescriptorFactory> xOldColumn;
    m_pColumns->getByIndex(index) >>= xOldColumn;
    alterColumn(index,descriptor,xOldColumn);
}
// -----------------------------------------------------------------------------
void ODbaseTable::alterColumn(sal_Int32 index,
                              const Reference< XPropertySet >& descriptor ,
                              const Reference< XDataDescriptorFactory >& xOldColumn )
{
    if(index < 0 || index >= m_pColumns->getCount())
        throw IndexOutOfBoundsException();

    OSL_ENSURE(descriptor.is(),"descriptor can not be null!");
    OSL_ENSURE(xOldColumn.is(),"xOldColumn can not be null!");
    // creates a copy of the the original column and copy all properties from descriptor in xCopyColumn
    Reference<XPropertySet> xCopyColumn = xOldColumn->createDataDescriptor();
    ::comphelper::copyProperties(descriptor,xCopyColumn);


//  // get the name
//  ::rtl::OUString sOldName,sNewName;
//  xOldColumn->getPropertyByName(PROPERTY_NAME) >>= sOldName;
//  descriptor->getPropertyByName(PROPERTY_NAME) >>= sNewName;
//
//  // get the type
//  sal_Int32 nOldType,nNewType;
//  xOldColumn->getPropertyByName(PROPERTY_TYPE) >>= nOldType;
//  descriptor->getPropertyByName(PROPERTY_TYPE) >>= nNewType;
//
//  // get the precision
//  sal_Int32 nOldPrec,nNewPrec;
//  xOldColumn->getPropertyByName(PROPERTY_PRECISION) >>= nOldPrec;
//  descriptor->getPropertyByName(PROPERTY_PRECISION) >>= nNewPrec;
//
//  // get the scale
//  sal_Int32 nOldScale,nNewScale;
//  xOldColumn->getPropertyByName(PROPERTY_SCALE) >>= nOldScale;
//  descriptor->getPropertyByName(PROPERTY_SCALE) >>= nNewScale;
//
//  // check if currency changed
//  sal_Bool bOldCur,bNewCur;
//  bOldCur = ::cppu::any2bool(xOldColumn->getPropertyByName(PROPERTY_ISCURRENCY));
//  bNewCur = ::cppu::any2bool(descriptor->getPropertyByName(PROPERTY_ISCURRENCY));

    // creates a temp file

    String sTempName = createTempFile();

    ODbaseTable* pNewTable = new ODbaseTable(static_cast<ODbaseConnection*>(m_pConnection));
    Reference<XPropertySet> xHoldTable = pNewTable;
    pNewTable->setPropertyValue(PROPERTY_NAME,makeAny(::rtl::OUString(sTempName)));
    Reference<XAppend> xAppend(pNewTable->getColumns(),UNO_QUERY);

    // copy the structure
    sal_Int32 i=0;
    for(;i < index;++i)
    {
        Reference<XDataDescriptorFactory> xColumn;
        m_pColumns->getByIndex(index) >>= xColumn;
        Reference<XPropertySet> xCpy = xColumn->createDataDescriptor();
        xAppend->appendByDescriptor(xCpy);
    }
    ++i; // now insert our new column
    xAppend->appendByDescriptor(xCopyColumn);

    for(;i < m_pColumns->getCount();++i)
    {
        Reference<XDataDescriptorFactory> xColumn;
        m_pColumns->getByIndex(index) >>= xColumn;
        Reference<XPropertySet> xCpy = xColumn->createDataDescriptor();
        xAppend->appendByDescriptor(xCpy);
    }

    // construct the new table
    if(!pNewTable->CreateImpl())
    {
        delete pNewTable;
        return;
    }

    // copy the data
    copyData(pNewTable);


    DropImpl();
    pNewTable->rename(m_Name);
    // release the temp file
    pNewTable = NULL;
    ::comphelper::disposeComponent(xHoldTable);

    FileClose();
    construct();
}
// -------------------------------------------------------------------------
void SAL_CALL ODbaseTable::rename( const ::rtl::OUString& newName ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if (ODbaseTable_BASE::rBHelper.bDisposed)
        throw DisposedException();

    FileClose();
    String aName = getEntry();
    if(!aName.Len())
    {
        ::rtl::OUString aIdent = m_pConnection->getContent()->getIdentifier()->getContentIdentifier();
        aIdent += ::rtl::OUString::createFromAscii("/");
        aIdent += m_Name;
        aName = aIdent;
    }
    INetURLObject aURL;
    aURL.SetURL(aName);

    if(aURL.getExtension() != m_pConnection->getExtension())
        aURL.setExtension(m_pConnection->getExtension());
    String sNewName(newName);
    sNewName.AppendAscii(".");
    sNewName += m_pConnection->getExtension();

    try
    {
        Content aContent(aURL.GetURLNoPass(),Reference<XCommandEnvironment>());
        aContent.setPropertyValue( rtl::OUString::createFromAscii( "Title" ),makeAny( ::rtl::OUString(sNewName) ) );
    }
    catch(Exception&)
    {
        throw ElementExistException();
    }
    m_Name = newName;
    construct();
}
// -----------------------------------------------------------------------------
void ODbaseTable::addColumn(const Reference< XPropertySet >& _xNewColumn)
{
    String sTempName = createTempFile();

    ODbaseTable* pNewTable = new ODbaseTable(static_cast<ODbaseConnection*>(m_pConnection));
    Reference<XPropertySet> xHoldTable = pNewTable;
    pNewTable->setPropertyValue(PROPERTY_NAME,makeAny(::rtl::OUString(sTempName)));
    {
        Reference<XAppend> xAppend(pNewTable->getColumns(),UNO_QUERY);

        // copy the structure
        for(sal_Int32 i=0;i < m_pColumns->getCount();++i)
        {
            Reference<XDataDescriptorFactory> xColumn;
            m_pColumns->getByIndex(i) >>= xColumn;
            Reference<XPropertySet> xCpy = xColumn->createDataDescriptor();
            xAppend->appendByDescriptor(xCpy);
        }
        xAppend->appendByDescriptor(_xNewColumn);
    }

    // construct the new table
    if(!pNewTable->CreateImpl())
    {
        return;
    }
    // copy the data
    copyData(pNewTable);
    // drop the old table
    DropImpl();
    pNewTable->rename(m_Name);
    // release the temp file
    pNewTable = NULL;
    ::comphelper::disposeComponent(xHoldTable);
    FileClose();
    construct();
}
// -----------------------------------------------------------------------------
String ODbaseTable::createTempFile()
{
    ::rtl::OUString aIdent = m_pConnection->getContent()->getIdentifier()->getContentIdentifier();
    aIdent += ::rtl::OUString::createFromAscii("/");
    String sTempName(aIdent);
    String sExt;
    sExt.AssignAscii(".");
    sExt += m_pConnection->getExtension();

    TempFile aTempFile(String(m_Name),&sExt,&sTempName);
    if(!aTempFile.IsValid())
        throw SQLException(::rtl::OUString::createFromAscii("Error while alter table!"),NULL,::rtl::OUString::createFromAscii("HY0000"),1000,Any());

    INetURLObject aURL;
    aURL.SetSmartProtocol(INET_PROT_FILE);
    aURL.SetURL(aTempFile.GetURL());

    Content aContent(aURL.GetURLNoPass(),Reference<XCommandEnvironment>());
    try
    {
        if (aContent.isDocument())
        {
            // Hack fuer Bug #30609 , nur wenn das File existiert und die Laenge > 0 gibt es einen Fehler
            SvStream* pFileStream = ::utl::UcbStreamHelper::CreateStream( aURL.GetURLNoPass(),STREAM_READ);

            if (pFileStream && pFileStream->Seek(STREAM_SEEK_TO_END))
            {
                //  aStatus.SetError(ERRCODE_IO_ALREADYEXISTS,TABLE,aFile.GetFull());
                return String();
            }
            delete pFileStream;
        }
    }
    catch(Exception&) // a exception is thrown when no file exists
    {
    }
    String sNewName(aURL.getName());
    sNewName.Erase(sNewName.Len() - sExt.Len());
    return sNewName;
}
// -----------------------------------------------------------------------------
void ODbaseTable::copyData(ODbaseTable* _pNewTable)
{
    OValueRow aRow = new OValueVector(m_pColumns->getCount());
    for(OValueVector::iterator aIter = aRow->begin(); aIter != aRow->end();++aIter)
        aIter->setBound(sal_True);

    sal_Bool bOk = sal_True;
    sal_Int32 nCurPos;
    for(sal_uInt32 nRowPos = 0; nRowPos < m_aHeader.db_anz;++nRowPos)
    {
        if(bOk = seekRow(FILE_BOOKMARK,nRowPos,nCurPos))
        {
            if(bOk = fetchRow(aRow,m_aColumns.getBody(),sal_True,sal_True))
            {
                bOk = _pNewTable->InsertRow(*aRow,sal_True,_pNewTable->m_pColumns);
                OSL_ENSURE(bOk,"Row could not be inserted!");
            }
            else
                OSL_ENSURE(bOk,"Row could not be fetched!");
        }
        else
        {
            OSL_ASSERT(0);
        }
    }
}
// -----------------------------------------------------------------------------

