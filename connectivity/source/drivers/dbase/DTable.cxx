/*************************************************************************
 *
 *  $RCSfile: DTable.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-09 12:31:04 $
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
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif

using namespace connectivity;
using namespace connectivity::dbase;
using namespace connectivity::file;
using namespace ucb;
using namespace cppu;
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
    m_aFileStream.RefreshBuffer(); // sicherstellen, dass die Kopfinformationen tatsaechlich neu gelesen werden
    m_aFileStream.Seek(STREAM_SEEK_TO_BEGIN);

    BYTE aTyp;
    m_aFileStream >> aTyp;
    m_aFileStream.Read((char*)m_aHeader.db_aedat, 3*sizeof(BYTE));
    m_aFileStream >> m_aHeader.db_anz;
    m_aFileStream >> m_aHeader.db_kopf;
    m_aFileStream >> m_aHeader.db_slng;
    m_aFileStream.Read((char*)m_aHeader.db_frei, 20*sizeof(BYTE));

    if (m_aHeader.db_anz  < 0 ||
        m_aHeader.db_kopf <= 0 ||
        m_aHeader.db_slng <= 0 ||
        ((m_aHeader.db_kopf - 1) / 32 - 1) <= 0) // anzahl felder
    {
        // Dies ist keine DBase Datei
    }
    else
    {
        // Konsistenzpruefung des Header:
        m_aHeader.db_typ = (DBFType)aTyp;
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
                m_aFileStream.SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
                break;
            default:
            {   // Dies ist keine DBase Datei
            }
        }
    }
}
// -------------------------------------------------------------------------
void ODbaseTable::fillColumns()
{
    m_aFileStream.Seek(STREAM_SEEK_TO_BEGIN);
    m_aFileStream.Seek(32L);

    // Anzahl Felder:
    sal_uInt32 nFieldCount = (m_aHeader.db_kopf - 1) / 32 - 1;

    String aStrFieldName;aStrFieldName.AssignAscii("Column");
    sal_Int32 nFieldCnt = 0;
    ::rtl::OUString aTypeName;

    for (sal_uInt32 i = 0; i < nFieldCount; i++)
    {
        DBFColumn aDBFColumn;
        m_aFileStream.Read((char*)&aDBFColumn, sizeof(aDBFColumn));

        // Info auslesen und in SdbColumn packen:
        String aColumnName((const char *)aDBFColumn.db_fnm,getConnection()->getTextEncoding());

//      while (aOriginalColumns->ColumnNumber(aColumnName) != SDB_COLUMN_NOTFOUND)
//          (aColumnName = aStrFieldName) += String::CreateFromsal_Int32(++nFieldCnt);

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
        Reference< XFastPropertySet> xCol = pColumn;
        m_aColumns->push_back(xCol);
    }
}
// -------------------------------------------------------------------------
//void ODbaseTable::fillIndexes()
//{
//  Dir* pDir = m_pConnection->getDir();
//  String aPath = pDir->GetName();
//  aPath += _Name.getStr();
//  DirEntry aEntry(aPath);
//  aEntry.SetExtension(String::CreateFromAscii("inf"));
//
//  Config aInfFile(aEntry.GetFull());
//  aInfFile.SetGroup(dBASE_III_GROUP);
//  sal_Int32 nKeyCnt = aInfFile.GetKeyCount();
//  ByteString aKeyName;
//  ByteString aIndexName;
//
//  for (sal_Int32 nKey = 0,nPos=0; nKey < nKeyCnt; nKey++)
//  {
//      // Verweist der Key auf ein Indexfile?...
//      aKeyName = aInfFile.GetKeyName( nKey );
//      //...wenn ja, Indexliste der Tabelle hinzufuegen
//      if (aKeyName.Copy(0,3) == ByteString("NDX") )
//      {
//          aIndexName = aInfFile.ReadKey(aKeyName);
//          aEntry.SetName(String(aIndexName,getConnection()->getTextEncoding()));
//          if (aEntry.Exists())
//          {
//              readIndex(aEntry.GetBase());
//          }
//              //  _rList.Insert(new String(aEntry.GetBase()), nPos++);
//      }
//  }
//}
// -------------------------------------------------------------------------
//void ODbaseTable::readIndex(const String& _rName)
//{
//}
// -------------------------------------------------------------------------
ODbaseTable::ODbaseTable(ODbaseConnection* _pConnection) : ODbaseTable_BASE(_pConnection)
    //  , m_aColumns(_pConnection->getMetaData()->storesMixedCaseQuotedIdentifiers())
{

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
                //  , m_aColumns(_pConnection->getMetaData()->storesMixedCaseQuotedIdentifiers())
{
    INetURLObject aURL;
    aURL.SetSmartProtocol(INET_PROT_FILE);
    aURL.SetSmartURL(getEntry(), INetURLObject::ENCODE_ALL);

    if(aURL.getExtension() != m_pConnection->getExtension())
        aURL.setExtension(m_pConnection->getExtension());

    //  Content aContent(aURL.GetMainURL());

    m_aFileStream.Open(aURL.getFSysPath(INetURLObject::FSYS_DETECT), STREAM_READWRITE | STREAM_NOCREATE | STREAM_SHARE_DENYWRITE);
    if(!m_aFileStream.IsOpen())
        m_aFileStream.Open(aURL.getFSysPath(INetURLObject::FSYS_DETECT), STREAM_READ | STREAM_NOCREATE | STREAM_SHARE_DENYNONE );

    if(m_aFileStream.IsOpen())
    {
        readHeader();

        if (HasMemoFields())
        {
            // Memo-Dateinamen bilden (.DBT):
            // nyi: Unschoen fuer Unix und Mac!

            if (m_aHeader.db_typ == FoxProMemo) // foxpro verwendet andere extension
                aURL.SetExtension(String::CreateFromAscii("fpt"));  // nyi: Gross-/Kleinschreibung bei Unix? Klein ist sicherlich schoener.
            else
                aURL.SetExtension(String::CreateFromAscii("dbt"));  // nyi: Gross-/Kleinschreibung bei Unix? Klein ist sicherlich schoener.

            // Wenn die Memodatei nicht gefunden wird, werden die Daten trotzdem angezeigt
            // allerdings koennen keine Updates durchgefuehrt werden
            // jedoch die Operation wird ausgefuehrt
            m_aMemoStream.Open(aURL.getFSysPath(INetURLObject::FSYS_DETECT), STREAM_READWRITE | STREAM_NOCREATE | STREAM_SHARE_DENYWRITE);
            if (m_aMemoStream.IsOpen())
                ReadMemoHeader();

    //      if (aStatus.IsError())
    //      {
    //          String aText = String(SdbResId(STR_STAT_FILE_NOT_FOUND));
    //          aText.SearchAndReplace(String::CreateFromAscii("%%d"),aFileEntry.GetName());
    //          aText.SearchAndReplace(String::CreateFromAscii("%%t"),aStatus.TypeToString(MEMO));
    //          aStatus.Set(SDB_STAT_SUCCESS_WITH_INFO,
    //                      String::CreateFromAscii("01000"),
    //                      aStatus.CreateErrorMessage(aText),
    //                      0, String() );
    //          m_aMemoStream.Close();
    //      }
        }
    //  if (aStatus.IsError())
    //      FileClose();
    //
        fillColumns();

        m_aFileStream.Seek(STREAM_SEEK_TO_END);
        UINT32 nFileSize = m_aFileStream.Tell();
        m_aFileStream.Seek(STREAM_SEEK_TO_BEGIN);

        // Buffersize abhaengig von der Filegroesse
        m_aFileStream.SetBufferSize(nFileSize > 1000000 ? 32768 :
                                  nFileSize > 100000 ? 16384 :
                                  nFileSize > 10000 ? 4096 : 1024);

        if (m_aMemoStream.IsOpen())
        {
            // Puffer genau auf Laenge eines Satzes stellen
            m_aMemoStream.Seek(STREAM_SEEK_TO_END);
            nFileSize = m_aMemoStream.Tell();
            m_aMemoStream.Seek(STREAM_SEEK_TO_BEGIN);

            // Buffersize abhaengig von der Filegroesse
            m_aMemoStream.SetBufferSize(nFileSize > 1000000 ? 32768 :
                                          nFileSize > 100000 ? 16384 :
                                          nFileSize > 10000 ? 4096 :
                                          m_aMemoHeader.db_size);
        }

        AllocBuffer();

        refreshColumns();
        refreshIndexes();
    }
}
//------------------------------------------------------------------
BOOL ODbaseTable::ReadMemoHeader()
{
    m_aMemoStream.SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
    m_aMemoStream.RefreshBuffer();          // sicherstellen das die Kopfinformationen tatsaechlich neu gelesen werden
    m_aMemoStream.Seek(0L);

    m_aMemoStream >> m_aMemoHeader.db_next;
    switch (m_aHeader.db_typ)
    {
        case dBaseIIIMemo:  // dBase III: feste Blockgröße
        case dBaseIVMemo:
            // manchmal wird aber auch dBase3 dBase4 Memo zugeordnet
            m_aMemoStream.Seek(20L);
            m_aMemoStream >> m_aMemoHeader.db_size;
            if (m_aMemoHeader.db_size > 1 && m_aMemoHeader.db_size != 512)  // 1 steht auch fuer dBase 3
                m_aMemoHeader.db_typ  = MemodBaseIV;
            else if (m_aMemoHeader.db_size > 1 && m_aMemoHeader.db_size == 512)
            {
                // nun gibt es noch manche Dateien, die verwenden eine Gößenangabe,
                // sind aber dennoch dBase Dateien
                char sHeader[4];
                m_aMemoStream.Seek(m_aMemoHeader.db_size);
                m_aMemoStream.Read(sHeader,4);

                if ((m_aMemoStream.GetErrorCode() != ERRCODE_NONE) || ((BYTE)sHeader[0]) != 0xFF || ((BYTE)sHeader[1]) != 0xFF || ((BYTE)sHeader[2]) != 0x08)
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
            m_aMemoStream.Seek(6L);
            m_aMemoStream.SetNumberFormatInt(NUMBERFORMAT_INT_BIGENDIAN);
            m_aMemoStream >> m_aMemoHeader.db_size;
    }
    return TRUE;
}
// -------------------------------------------------------------------------
String ODbaseTable::getEntry()
{
    ::rtl::OUString aURL;
    Reference< XResultSet > xDir = m_pConnection->getDir()->getStaticResultSet();
    Reference< XRow> xRow(xDir,UNO_QUERY);
    ::rtl::OUString aName;
    sal_Int32 nLen = m_pConnection->getExtension().Len()+1;
    xDir->beforeFirst();
    while(xDir->next())
    {
        aName = xRow->getString(1);
        aName = aName.replaceAt(aName.getLength()-nLen,nLen,::rtl::OUString());
        if(aName == m_Name)
        {
            Reference< XContentAccess > xContentAccess( xDir, UNO_QUERY );
            aURL = xContentAccess->queryContentIdentfierString();
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
//  Dir* pDir = m_pConnection->getDir();
//  String aPath = pDir->GetName();
//  aPath += m_Name.getStr();
    INetURLObject aURL;
    aURL.SetSmartProtocol(INET_PROT_FILE);
    aURL.SetSmartURL(getEntry(), INetURLObject::ENCODE_ALL);

    aURL.setExtension(String::CreateFromAscii("inf"));

    Config aInfFile(aURL.getFSysPath(INetURLObject::FSYS_DETECT));
    aInfFile.SetGroup(dBASE_III_GROUP);
    sal_Int32 nKeyCnt = aInfFile.GetKeyCount();
    ByteString aKeyName;
    ByteString aIndexName;

    ::std::vector< ::rtl::OUString> aVector;

    for (sal_Int32 nKey = 0,nPos=0; nKey < nKeyCnt; nKey++)
    {
        // Verweist der Key auf ein Indexfile?...
        aKeyName = aInfFile.GetKeyName( nKey );
        //...wenn ja, Indexliste der Tabelle hinzufuegen
        if (aKeyName.Copy(0,3) == ByteString("NDX") )
        {
            aIndexName = aInfFile.ReadKey(aKeyName);
            aURL.setName(String(aIndexName,getConnection()->getTextEncoding()));
            Content aCnt(aURL.GetMainURL(),Reference<XCommandEnvironment>());
            if (aCnt.isDocument())
            {
                aVector.push_back(aURL.getBase());
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
    m_aColumns->clear();
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL ODbaseTable::getTypes(  ) throw(RuntimeException)
{
    Sequence< Type > aTypes = OTable_TYPEDEF::getTypes();
    Sequence< Type > aRet(aTypes.getLength()-3);
    const Type* pBegin = aTypes.getConstArray();
    const Type* pEnd = pBegin + aTypes.getLength();
    sal_Int32 i=0;
    for(;pBegin != pEnd;++pBegin,++i)
    {
        if(!(*pBegin == ::getCppuType((const Reference<XKeysSupplier>*)0) ||
            *pBegin == ::getCppuType((const Reference<XRename>*)0) ||
            *pBegin == ::getCppuType((const Reference<XAlterTable>*)0) ||
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
    if( rType == ::getCppuType((const Reference<XKeysSupplier>*)0) ||
        rType == ::getCppuType((const Reference<XRename>*)0) ||
        rType == ::getCppuType((const Reference<XAlterTable>*)0) ||
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

        OSL_ENSHURE(m_nFilePos >= 1,"SdbDBFCursor::FileFetchRow: ungueltige Record-Position");
        sal_Int32 nPos = m_aHeader.db_kopf + (sal_Int32)(m_nFilePos-1) * nEntryLen;

        ULONG nLen = m_aFileStream.Seek(nPos);
        if (m_aFileStream.GetError() != ERRCODE_NONE)
            goto Error;

        nLen = m_aFileStream.Read((char*)m_pBuffer, nEntryLen);
        if (m_aFileStream.GetError() != ERRCODE_NONE)
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
sal_Bool ODbaseTable::fetchRow(file::OValueRow _rRow,const OSQLColumns & _rCols, sal_Bool bRetrieveData)
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
        Reference< XFastPropertySet> xColumn = *aIter;

        // Laengen je nach Datentyp:
        // nyi: eine zentrale Funktion, die die Laenge liefert!
        sal_Int32 nLen;
        xColumn->getFastPropertyValue(PROPERTY_ID_PRECISION) >>= nLen;
        sal_Int32 nType = getINT32(xColumn->getFastPropertyValue(PROPERTY_ID_TYPE));
        switch(nType)
        {
            case DataType::DATE:        nLen = 8; break;
            case DataType::DECIMAL:
                nLen = SvDbaseConverter::ConvertPrecisionToDbase(nLen,getINT32(xColumn->getFastPropertyValue(PROPERTY_ID_SCALE)));
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
            String aStr(pData,getConnection()->getTextEncoding());
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

            String aStr(pData, nLen);       // Spaces am Anfang und am Ende entfernen:
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
                    sal_Int32  nYear   = aStr.Copy( 0, 4 ).ToInt32();
                    sal_Int32  nMonth  = aStr.Copy( 4, 2 ).ToInt32();
                    sal_Int32  nDay    = aStr.Copy( 6, 2 ).ToInt32();

                    ::com::sun::star::util::Date aDate(nDay,nMonth,nYear);
                    (*_rRow)[i] = DateConversion::toDouble(aDate);
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
                    if (nBlockNo > 0 && m_aMemoStream.IsOpen()) // Daten aus Memo-Datei lesen, nur wenn
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
            (*_rRow)[i].setType(nType);
        }

//      if (aStatus.IsError())
//          break;
        // Und weiter ...
        nByteOffset += nLen;
    }
    return sal_True;
}
//------------------------------------------------------------------
BOOL ODbaseTable::ReadMemo(ULONG nBlockNo, OFileValue& aVariable)
{
    BOOL bIsText = TRUE;
    //  SdbConnection* pConnection = GetConnection();

    m_aMemoStream.Seek(nBlockNo * m_aMemoHeader.db_size);
    switch (m_aMemoHeader.db_typ)
    {
        case MemodBaseIII: // dBase III-Memofeld, endet mit Ctrl-Z
        {
            const char cEOF = (char) 0x1a;
            ByteString aStr;
            static char aBuf[514];
            aBuf[512] = 0;          // sonst kann der Zufall uebel mitspielen
            BOOL bReady = sal_False;

            do
            {
                m_aMemoStream.Read(&aBuf,512);

                USHORT i = 0;
                while (aBuf[i] != cEOF && ++i < 512)
                    ;
                bReady = aBuf[i] == cEOF;

                aBuf[i] = 0;
                aStr += aBuf;

            } while (!bReady && !m_aMemoStream.IsEof() && aStr.Len() < STRING_MAXLEN);

            aVariable = ::rtl::OUString(aStr.GetBuffer(), aStr.Len(),getConnection()->getTextEncoding());
        } break;
        case MemoFoxPro:
        case MemodBaseIV: // dBase IV-Memofeld mit Laengenangabe
        {
            char sHeader[4];
            m_aMemoStream.Read(sHeader,4);
            // Foxpro stores text and binary data
            if (m_aMemoHeader.db_typ == MemoFoxPro)
            {
                if (((BYTE)sHeader[0]) != 0 || ((BYTE)sHeader[1]) != 0 || ((BYTE)sHeader[2]) != 0)
                {
//                  String aText = String(SdbResId(STR_STAT_FILE_INVALID));
//                  aText.SearchAndReplace(String::CreateFromAscii("%%d"),m_aMemoStream.GetFileName());
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
//              aText.SearchAndReplace(String::CreateFromAscii("%%d"),m_aMemoStream.GetFileName());
//              aText.SearchAndReplace(String::CreateFromAscii("%%t"),aStatus.TypeToString(MEMO));
//              aStatus.Set(SDB_STAT_ERROR,
//                      String::CreateFromAscii("01000"),
//                      aStatus.CreateErrorMessage(aText),
//                      0, String() );
                return sal_False;
            }

            ULONG nLength;
            m_aMemoStream >> nLength;

            if (m_aMemoHeader.db_typ == MemodBaseIV)
                nLength -= 8;

            //  char cChar;
            if (nLength < STRING_MAXLEN && bIsText)
            {
                ByteString aStr;
                aStr.Expand(USHORT (nLength));
                m_aMemoStream.Read(aStr.AllocBuffer((USHORT)nLength),nLength);
                aStr.ReleaseBufferAccess();
                aVariable = ::rtl::OUString(aStr.GetBuffer(),aStr.Len(), getConnection()->getTextEncoding());
            }
            else
            {
//                              ::Sequence<sal_Int8> aText(nLength);
//              sal_Int8* pData = aText.getArray();
//              for (ULONG i = 0; i < nLength; i++)
//              {
//                  m_aMemoStream.Read(&cChar,1);
//                  (*pData++) = cChar;
//              }
//              aVariable.setBytes(aText);
                return sal_False;
            }
        }
    }
    return sal_True;
}
// -------------------------------------------------------------------------
void ODbaseTable::FileClose()
{
    // falls noch nicht alles geschrieben wurde
    if (m_aMemoStream.IsOpen() && m_aMemoStream.IsWritable())
        m_aMemoStream.Flush();

    m_aMemoStream.Close();

    if (m_aFileStream.IsOpen() && m_aFileStream.IsWritable())
        m_aFileStream.Flush();

    m_aFileStream.Close();

    if (m_pBuffer != NULL)
    {
        delete m_pBuffer;
        m_pBuffer = NULL;
    }
}
// -------------------------------------------------------------------------
BOOL ODbaseTable::CreateImpl()
{
    OSL_ENSHURE(!m_aFileStream.IsOpen(), "SequenceError");

    INetURLObject aURL;
    aURL.SetSmartProtocol(INET_PROT_FILE);
    aURL.SetSmartURL(getEntry(), INetURLObject::ENCODE_ALL);

    if(aURL.getExtension() != m_pConnection->getExtension())
        aURL.setExtension(m_pConnection->getExtension());

    Content aContent(aURL.GetMainURL(),Reference<XCommandEnvironment>());

    if (aContent.isDocument())
    {
        // Hack fuer Bug #30609 , nur wenn das File existiert und die Laenge > 0 gibt es einen Fehler
        SvFileStream m_aFileStream;
        m_aFileStream.Open(aURL.getFSysPath(INetURLObject::FSYS_DETECT),STREAM_STD_READ);

        if (m_aFileStream.IsOpen() && m_aFileStream.Seek(STREAM_SEEK_TO_END))
        {
            //  aStatus.SetError(ERRCODE_IO_ALREADYEXISTS,TABLE,aFile.GetFull());
            return sal_False;
        }
        m_aFileStream.Close();
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
        Content aMemo1Content(aURL.GetMainURL(),Reference<XCommandEnvironment>());

        if (aMemo1Content.isDocument())
        {
            //  aStatus.SetError(ERRCODE_IO_ALREADYEXISTS,MEMO,aFile.GetFull());
            aURL.setExtension(aExt);      // kill dbf file
            Content aMemoContent(aURL.GetMainURL(),Reference<XCommandEnvironment>());
            aMemoContent.executeCommand( rtl::OUString::createFromAscii( "delete" ),bool2any( sal_True ) );
            return sal_False;
        }
        if (!CreateMemoFile(aURL))
        {
            aURL.setExtension(aExt);      // kill dbf file
            Content aMemoContent(aURL.GetMainURL(),Reference<XCommandEnvironment>());
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

    m_aFileStream.Open(aFile.getFSysPath(INetURLObject::FSYS_DETECT), STREAM_READWRITE | STREAM_SHARE_DENYWRITE | STREAM_TRUNC);
    if (!m_aFileStream.IsOpen())
        return sal_False;

    char aBuffer[21];               // write buffer
    memset(aBuffer,0,sizeof(aBuffer));

    m_aFileStream.Seek(0L);
    m_aFileStream << (BYTE) dBaseIII;                                                                // dBase format
    m_aFileStream << (BYTE) (aDate.GetYear() % 100);                 // aktuelles Datum


    m_aFileStream << (BYTE) aDate.GetMonth();
    m_aFileStream << (BYTE) aDate.GetDay();
    m_aFileStream << 0L;                                                                                                     // Anzahl der Datensätze
    m_aFileStream << (USHORT)(m_pColumns->getCount() * 32 + 1);                // Kopfinformationen,
                                                                        // pColumns erhält immer eine Spalte mehr
    m_aFileStream << (USHORT) 0;                                                                                     // Satzlänge wird später bestimmt
    m_aFileStream.Write(aBuffer, 20);

    USHORT nRecLength = 1;                                                                                          // Länge 1 für deleted flag
    ULONG  nMaxFieldLength = m_pConnection->getMetaData()->getMaxColumnNameLength();
    Reference<XIndexAccess> xColumns(getColumns(),UNO_QUERY);

    ::rtl::OUString aName;
    Reference<XFastPropertySet> xCol;
    for(sal_Int32 i=0;i<xColumns->getCount();++i)
    {
        xColumns->getByIndex(i) >>= xCol;
        OSL_ENSHURE(xCol.is(),"This should be a column!");

        char  cTyp;

        xCol->getFastPropertyValue(PROPERTY_ID_NAME) >>= aName;

        if (aName.getLength() > nMaxFieldLength)
        {
//          String aText = String(SdbResId(STR_DBF_INVALIDFIELDNAMELENGTH));
//          aText.SearchAndReplace(String::CreateFromAscii("#"),rColumn.GetName());
//          aStatus.Set(SDB_STAT_ERROR,
//                      String::CreateFromAscii("01000"),
//                      aStatus.CreateErrorMessage(aText),
//                      0, String() );
            break;
        }

        ByteString aCol(aName.getStr(),gsl_getSystemTextEncoding());
        m_aFileStream << aCol.GetBuffer();
        m_aFileStream.Write(aBuffer, 11 - aCol.Len());

        switch (getINT32(xCol->getFastPropertyValue(PROPERTY_ID_TYPE)))
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
//                  aStatus.Set(SDB_STAT_ERROR,
//                      String::CreateFromAscii("01000"),
//                      aStatus.CreateErrorMessage(String(SdbResId(STR_DBF_INVALIDFORMAT))),
//                      0, String() );
                    break;
                }
        }

        m_aFileStream << cTyp;
        m_aFileStream.Write(aBuffer, 4);

        sal_Int32 nPrecision = 0;
        xCol->getFastPropertyValue(PROPERTY_ID_PRECISION) >>= nPrecision;
        sal_Int32 nScale = 0;
        xCol->getFastPropertyValue(PROPERTY_ID_SCALE) >>= nScale;

        switch(cTyp)
        {
            case 'C':
                OSL_ENSHURE(nPrecision < 255, "ODbaseTable::Create: Column zu lang!");
                if (nPrecision > 254)
                {
//                  String aText = String(SdbResId(STR_DBF_INVALIDFIELDLENGTH));
//                  aText.SearchAndReplace(String::CreateFromAscii("#"),rColumn.GetName());
//                  aStatus.Set(SDB_STAT_ERROR,
//                              String::CreateFromAscii("01000"),
//                              aStatus.CreateErrorMessage(aText),
//                              0, String() );
                    //  break;
                }
                m_aFileStream << (BYTE) min(nPrecision, 255UL);      //Feldlänge
                nRecLength += (USHORT)min(nPrecision, 255UL);
                m_aFileStream << (BYTE)0;                                                                //Nachkommastellen
                break;
            case 'F':
            case 'N':
                OSL_ENSHURE(nPrecision >=  nScale,
                           "ODbaseTable::Create: Feldlänge muß größer Nachkommastellen sein!");
                if (nPrecision <  nScale)
                {
//                  aStatus.Set(SDB_STAT_ERROR,
//                      String::CreateFromAscii("01000"),
//                      aStatus.CreateErrorMessage(String(SdbResId(STR_DBF_INVALIDFORMAT))),
//                      0, String() );
                    break;
                }
                if (getBOOL(xCol->getFastPropertyValue(PROPERTY_ID_ISCURRENCY))) // Currency wird gesondert behandelt
                {
                    m_aFileStream << (BYTE)10;          // Standard Laenge
                    m_aFileStream << (BYTE)4;
                    nRecLength += 10;
                }
                else
                {
                    UINT16 nPrec = SvDbaseConverter::ConvertPrecisionToDbase(nPrecision,nScale);

                    m_aFileStream << (BYTE)( nPrec);
                    m_aFileStream << (BYTE)nScale;
                    nRecLength += (USHORT)nPrec;
                }
                break;
            case 'L':
                m_aFileStream << (BYTE)1;
                m_aFileStream << (BYTE)0;
                nRecLength++;
                break;
            case 'D':
                m_aFileStream << (BYTE)8;
                m_aFileStream << (BYTE)0;
                nRecLength += 8;
                break;
            case 'M':
                bCreateMemo = TRUE;
                m_aFileStream << (BYTE)10;
                m_aFileStream << (BYTE)0;
                nRecLength += 10;
                break;
            default:
//              aStatus.Set(SDB_STAT_ERROR,
//                      String::CreateFromAscii("01000"),
//                      aStatus.CreateErrorMessage(String(SdbResId(STR_DBF_INVALIDFORMAT))),
//                      0, String() );
                break;
        }
        m_aFileStream.Write(aBuffer, 14);
    }

//  if (aStatus.IsError())
//      return sal_False;

    m_aFileStream << (BYTE)0x0d;                                     // kopf ende
    m_aFileStream.Seek(10L);
    m_aFileStream << nRecLength;                                     // satzlänge nachträglich eintragen

    if (bCreateMemo)
    {
        m_aFileStream.Seek(0L);
        m_aFileStream << (BYTE) dBaseIIIMemo;
    }
    return TRUE;
}

//------------------------------------------------------------------
// erzeugt grundsätzlich dBase III Datei Format
BOOL ODbaseTable::CreateMemoFile(const INetURLObject& aFile)
{
    // Makro zum Filehandling fürs Erzeugen von Tabellen
    m_aMemoStream.Open(aFile.getFSysPath(INetURLObject::FSYS_DETECT), STREAM_READWRITE | STREAM_SHARE_DENYWRITE);
    if (!m_aMemoStream.IsOpen())
        return sal_False;

    char aBuffer[512];              // write buffer
    memset(aBuffer,0,sizeof(aBuffer));

#ifdef WIN
    m_aMemoStream.Seek(0L);
    for (UINT16 i = 0; i < 512; i++)
    {
        m_aMemoStream << BYTE(0);
    }
#else
    m_aMemoStream.SetFiller('\0');
    m_aMemoStream.SetStreamSize(512);
#endif

    m_aMemoStream.Seek(0L);
    m_aMemoStream << long(1);                  // Zeiger auf ersten freien Block

    m_aMemoStream.Close();
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
    aURL.SetSmartProtocol(INET_PROT_FILE);
    aURL.SetSmartURL(getEntry(), INetURLObject::ENCODE_ALL);

    Content aContent(aURL.GetMainURL(),Reference<XCommandEnvironment>());
    aContent.executeCommand( rtl::OUString::createFromAscii( "delete" ),
                                 makeAny( sal_Bool( sal_True ) ) );

    if (HasMemoFields())
    {
        aURL.setExtension(String::CreateFromAscii("dbt"));
        Content aMemoContent(aURL.GetMainURL(),Reference<XCommandEnvironment>());
        aMemoContent.executeCommand( rtl::OUString::createFromAscii( "delete" ),bool2any( sal_True ) );
    }

    // jetzt noch die Indices loeschen
    String aIndexName;
    //  aFile.SetExtension(String::CreateFromAscii("ndx"));
    USHORT nCount = m_pIndexes->getCount(),
           i      = 0;
    while (i < nCount)
    {
        m_pIndexes->dropByIndex(i);
    }
    //  aFile.SetBase(m_Name);
    aURL.setExtension(String::CreateFromAscii("inf"));
    Content aInfContent(aURL.GetMainURL(),Reference<XCommandEnvironment>());
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

    String aName = m_aFileStream.GetFileName();

    m_aFileStream.Seek(STREAM_SEEK_TO_END);
    nFileSize = m_aFileStream.Tell();

    if (HasMemoFields() && m_aMemoStream.IsOpen())
    {
        m_aMemoStream.Seek(STREAM_SEEK_TO_END);
        nMemoFileSize = m_aMemoStream.Tell();
    }

    if (!WriteBuffer())
    {
        m_aFileStream.SetStreamSize(nFileSize);             // alte Größe restaurieren

        if (HasMemoFields() && m_aMemoStream.IsOpen())
            m_aMemoStream.SetStreamSize(nMemoFileSize); // alte Größe restaurieren
        m_nFilePos = nTempPos;                              // Fileposition restaurieren
    }
    else
    {
        // Anzahl Datensaetze im Header erhoehen:
        m_aFileStream.Seek( 4L );
        m_aFileStream << (m_aHeader.db_anz + 1);

        // beim AppendOnly kein Flush!
        if (bFlush)
            m_aFileStream.Flush();

        // bei Erfolg # erhöhen
        m_aHeader.db_anz++;
        rRow[0] = m_nFilePos;                               // BOOKmark setzen
        m_nFilePos = nTempPos;
    }

    return sal_True;;
}

//------------------------------------------------------------------
BOOL ODbaseTable::UpdateRow(file::OValueVector& rRow, OValueRow pOrgRow,const Reference<XIndexAccess>& _xCols)
{
    // Buffer mit Leerzeichen füllen
    AllocBuffer();

    // Auf gewuenschten Record positionieren:
    long nPos = m_aHeader.db_kopf + (long)(m_nFilePos-1) * m_aHeader.db_slng;
    m_aFileStream.Seek(nPos);
    m_aFileStream.Read((char*)m_pBuffer, m_aHeader.db_slng);

    UINT32 nMemoFileSize;
    if (HasMemoFields() && m_aMemoStream.IsOpen())
    {
        m_aMemoStream.Seek(STREAM_SEEK_TO_END);
        nMemoFileSize = m_aMemoStream.Tell();
    }
    if (!UpdateBuffer(rRow, pOrgRow,_xCols) || !WriteBuffer())
    {
        if (HasMemoFields() && m_aMemoStream.IsOpen())
            m_aMemoStream.SetStreamSize(nMemoFileSize); // alte Größe restaurieren
    }
    else
    {
        m_aFileStream.Flush();
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
    m_aFileStream.Seek(nPos);

    OValueRow aRow = new OValueVector(_rCols.size());

    if (!fetchRow(aRow,_rCols,TRUE))
        return sal_False;

    Reference<XFastPropertySet> xCol;
    ::rtl::OUString aColName;
    ::utl::UStringMixEqual aCase(isCaseSensitive());
    for (USHORT i = 0; i < m_pColumns->getCount(); i++)
    {
        m_pColumns->getByIndex(i) >>= xCol;
        //  const SdbFILEColumn *pColumn = (const SdbFILEColumn *)(*aOriginalColumns)[i];

        xCol->getFastPropertyValue(PROPERTY_ID_NAME) >>= aColName;
        Reference<XFastPropertySet> xIndex = isUniqueByColumnName(aColName);
        if (xIndex.is())
        {
            Reference<XUnoTunnel> xTunnel(xIndex,UNO_QUERY);
            OSL_ENSHURE(xTunnel.is(),"No TunnelImplementation!");
            ODbaseIndex* pIndex = (ODbaseIndex*)xTunnel->getSomething(ODbaseIndex::getUnoTunnelImplementationId());
            OSL_ENSHURE(pIndex,"ODbaseTable::UpdateBuffer: No Index returned!");

            OSQLColumns::const_iterator aIter = _rCols.begin();
            //  sal_Int32 nPos = 0;
            for(;aIter != _rCols.end();++aIter,++nPos)
            {
//              Reference<XFastPropertySet> xFindCol;
//              _xCols->getByIndex(nPos) >>= xFindCol;
                if(aCase(getString((*aIter)->getFastPropertyValue(PROPERTY_ID_REALNAME)),aColName))
                    break;
            }
            if (aIter == _rCols.end())
                continue;

            pIndex->Delete(m_nFilePos,(*aRow)[nPos]);
        }
    }

    m_aFileStream.Seek(nPos);
    m_aFileStream << (BYTE)'*';
    m_aFileStream.Flush();
    return sal_True;;
}

//------------------------------------------------------------------
BOOL ODbaseTable::WriteMemo(OFileValue& aVariable, ULONG& rBlockNr)
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
        aStr = ByteString(aVariable.getString().getStr(), getConnection()->getTextEncoding());
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
                m_aMemoStream.Seek(rBlockNr * m_aMemoHeader.db_size);
                m_aMemoStream.SeekRel(4L);
                m_aMemoStream.Read(sHeader,4);

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
        nStreamSize = m_aMemoStream.Seek(STREAM_SEEK_TO_END);
        // letzten block auffuellen
        rBlockNr = (nStreamSize / m_aMemoHeader.db_size) + ((nStreamSize % m_aMemoHeader.db_size) > 0 ? 1 : 0);

        m_aMemoStream.SetStreamSize(rBlockNr * m_aMemoHeader.db_size);
        m_aMemoStream.Seek(STREAM_SEEK_TO_END);
    }
    else
    {
        m_aMemoStream.Seek(rBlockNr * m_aMemoHeader.db_size);
    }

    switch (m_aMemoHeader.db_typ)
    {
        case MemodBaseIII: // dBase III-Memofeld, endet mit Ctrl-Z
        {
            const char cEOF = (char) 0x1a;
            nSize++;

//          if (pData)
//          {
//              m_aMemoStream.Write((const char*) pData->getConstArray(), pData->getLength());
//          }
//          else
//          {
                m_aMemoStream.Write(aStr.GetBuffer(), aStr.Len());
            //  }

            m_aMemoStream << cEOF << cEOF;
        } break;
        case MemoFoxPro:
        case MemodBaseIV: // dBase IV-Memofeld mit Laengenangabe
        {
            m_aMemoStream << (BYTE)0xFF
                                         << (BYTE)0xFF
                                         << (BYTE)0x08;

            UINT32 nWriteSize = nSize;
            if (m_aMemoHeader.db_typ == MemoFoxPro)
            {
                m_aMemoStream << (BYTE) 0x01; // ((pData = NULL) ? 0x01 : 0x00);
                for (int i = 4; i > 0; nWriteSize >>= 8)
                    nHeader[--i] = (BYTE) (nWriteSize % 256);
            }
            else
            {
                m_aMemoStream << (BYTE) 0x00;
                nWriteSize += 8;
                for (int i = 0; i < 4; nWriteSize >>= 8)
                    nHeader[i++] = (BYTE) (nWriteSize % 256);
            }

            m_aMemoStream.Write(nHeader,4);
//          if (pData)
//          {
//              m_aMemoStream.Write((const char*) pData->getConstArray(), pData->getLength());
//          }
//          else
//          {
                m_aMemoStream.Write(aStr.GetBuffer(), aStr.Len());
            //  }
            m_aMemoStream.Flush();
        }
    }


    // Schreiben der neuen Blocknummer
    if (bAppend)
    {
        nStreamSize = m_aMemoStream.Seek(STREAM_SEEK_TO_END);
        m_aMemoHeader.db_next = (nStreamSize / m_aMemoHeader.db_size) + ((nStreamSize % m_aMemoHeader.db_size) > 0 ? 1 : 0);

        // Schreiben der neuen Blocknummer
        m_aMemoStream.Seek(0L);
        m_aMemoStream << m_aMemoHeader.db_next;
        m_aMemoStream.Flush();
    }
    return sal_True;
}
//------------------------------------------------------------------
void ODbaseTable::AllocBuffer()
{
    UINT16 nSize = m_aHeader.db_slng;
    OSL_ENSHURE(nSize > 0, "Size too small");

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
Reference<XFastPropertySet> ODbaseTable::isUniqueByColumnName(const ::rtl::OUString& _rColName)
{
    if(!m_pIndexes)
        refreshIndexes();
    Reference<XFastPropertySet> xIndex;
    for(sal_Int32 i=0;i<m_pIndexes->getCount();++i)
    {
        m_pIndexes->getByIndex(i) >>= xIndex;
        if(getBOOL(xIndex->getFastPropertyValue(PROPERTY_ID_ISUNIQUE)))
        {
            Reference<XNameAccess> xCols(Reference<XColumnsSupplier>(xIndex,UNO_QUERY)->getColumns());
            if(xCols->hasByName(_rColName))
                return xIndex;

        }
    }
    return Reference<XFastPropertySet>();
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
    USHORT nByteOffset  = 1;

    // Felder aktualisieren:
    Reference<XFastPropertySet> xCol;
    Reference<XFastPropertySet> xIndex;
    USHORT i;
    ::rtl::OUString aColName;
    ::std::vector< Reference<XFastPropertySet> > aIndexedCols(m_pColumns->getCount());

    ::utl::UStringMixEqual aCase(isCaseSensitive());

    // first search a key that exist already in the table
    for (i = 0; i < m_pColumns->getCount(); i++)
    {
        m_pColumns->getByIndex(i) >>= xCol;
        xCol->getFastPropertyValue(PROPERTY_ID_NAME) >>= aColName;

        //  const SdbFILEColumn *pColumn = (const SdbFILEColumn *)(*aOriginalColumns)[i];
        sal_Int32 nPos = 0;
        for(;nPos<_xCols->getCount();++nPos)
        {
            Reference<XFastPropertySet> xFindCol;
            _xCols->getByIndex(nPos) >>= xFindCol;
            if(aCase(getString(xFindCol->getFastPropertyValue(PROPERTY_ID_NAME)),aColName))
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
                OSL_ENSHURE(xTunnel.is(),"No TunnelImplementation!");
                ODbaseIndex* pIndex = (ODbaseIndex*)xTunnel->getSomething(ODbaseIndex::getUnoTunnelImplementationId());
                OSL_ENSHURE(pIndex,"ODbaseTable::UpdateBuffer: No Index returned!");

                if (pIndex->Find(0,rRow[nPos]))
                {
                    // es existiert kein eindeutiger Wert
//                  String aText = String(SdbResId(STR_VALUE_NOTUNIQUE));
//                  aText.SearchAndReplace(String::CreateFromAscii("#"),pColumn->GetName());
//                  String strDetailed = String(SdbResId(STR_DBF_DUPL_VALUE_INFO));
//                  strDetailed.SearchAndReplace(String::CreateFromAscii("$col$"),pColumn->GetName());
//                  aStatus.Set(SDB_STAT_ERROR,
//                      String::CreateFromAscii("01000"),
//                      aStatus.CreateErrorMessage(aText),
//                      0, strDetailed );
                    return sal_False;
                }
            }
        }
    }

    // when we are here there is no double key in the table

    for (i = 0; i < m_pColumns->getCount(); i++)
    {
        m_pColumns->getByIndex(i) >>= xCol;
        xCol->getFastPropertyValue(PROPERTY_ID_NAME) >>= aColName;

        // Laengen je nach Datentyp:
        // nyi: eine zentrale Funktion, die die Laenge liefert!
        USHORT nLen = (USHORT)getINT32(xCol->getFastPropertyValue(PROPERTY_ID_PRECISION));
        sal_Int32 nType = getINT32(xCol->getFastPropertyValue(PROPERTY_ID_TYPE));
        switch (nType)
        {
            case DataType::DATE:        nLen = 8; break;
            case DataType::DECIMAL:
                nLen = SvDbaseConverter::ConvertPrecisionToDbase(nLen,getINT32(xCol->getFastPropertyValue(PROPERTY_ID_SCALE)));
                break;  // das Vorzeichen und das Komma
            case DataType::BIT: nLen = 1; break;
            case DataType::LONGVARCHAR:nLen = 10; break;
            default:                    break;

        }

        sal_Int32 nPos = 0;
        for(;nPos<_xCols->getCount();++nPos)
        {
            Reference<XFastPropertySet> xFindCol;
            _xCols->getByIndex(nPos) >>= xFindCol;
            if(aCase(getString(xFindCol->getFastPropertyValue(PROPERTY_ID_NAME)),aColName))
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
            OSL_ENSHURE(xTunnel.is(),"No TunnelImplementation!");
            ODbaseIndex* pIndex = (ODbaseIndex*)xTunnel->getSomething(ODbaseIndex::getUnoTunnelImplementationId());
            OSL_ENSHURE(pIndex,"ODbaseTable::UpdateBuffer: No Index returned!");
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
                    ::com::sun::star::util::Date aDate = DateConversion::toDate(rRow[nPos]);
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

                    int nPrecision      = (int)getINT32(xCol->getFastPropertyValue(PROPERTY_ID_PRECISION));
                    int nScale          = (int)getINT32(xCol->getFastPropertyValue(PROPERTY_ID_SCALE));
                    // ein const_cast, da GetFormatPrecision am SvNumberFormat nicht const ist, obwohl es das eigentlich
                    // sein koennte und muesste

                    String aString;
                    SolarMath::DoubleToString(aString,n,'F',nScale,'.');
                    ByteString aDefaultValue(aString,gsl_getSystemTextEncoding());
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
//                      String strError(SdbResId(STR_DBF_INVALID_FIELD_VALUE));
//                      strError.SearchAndReplace(String::CreateFromAscii("$name$"), pColumn->GetName());
//
//                      String strDetailedInformation(SdbResId(STR_DBF_INVALID_FIELD_VALUE_DECIMAL));
//                      strDetailedInformation.SearchAndReplace(String::CreateFromAscii("$name$"), pColumn->GetName());
//                      strDetailedInformation.SearchAndReplace(String::CreateFromAscii("#length#"), nPrecision);
//                      strDetailedInformation.SearchAndReplace(String::CreateFromAscii("#scale#"), nScale);
//                      aStatus.Set(SDB_STAT_ERROR, String::CreateFromAscii("S1000"), aStatus.CreateErrorMessage(strError), 0, strDetailedInformation);
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
                    if (!m_aMemoStream.IsOpen() || !WriteMemo(rRow[nPos], nBlockNo))
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
                    memcpy(pData, aStr.GetBuffer(), min(nLen,aStr.Len()));
                }   break;
            }
        }
        catch ( ... )
        {
//          String strError(SdbResId(STR_DBF_INVALID_FIELD_VALUE));
//          strError.SearchAndReplace(String::CreateFromAscii("$name$"), pColumn->GetName());
//          aStatus.Set(SDB_STAT_ERROR, String::CreateFromAscii("S1000"), aStatus.CreateErrorMessage(strError), 0, String());
        }
//      if (aStatus.IsError())
//          break;
        // Und weiter ...
        nByteOffset += nLen;
    }
    return sal_True;
}


//------------------------------------------------------------------
BOOL ODbaseTable::WriteBuffer()
{
    OSL_ENSHURE(m_nFilePos >= 1,"SdbDBFCursor::FileFetchRow: ungueltige Record-Position");

    // Auf gewuenschten Record positionieren:
    long nPos = m_aHeader.db_kopf + (long)(m_nFilePos-1) * m_aHeader.db_slng;
    m_aFileStream.Seek(nPos);
    return m_aFileStream.Write((char*) m_pBuffer, m_aHeader.db_slng) > 0;
}