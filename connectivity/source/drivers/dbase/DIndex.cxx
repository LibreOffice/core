/*************************************************************************
 *
 *  $RCSfile: DIndex.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: oj $ $Date: 2000-11-03 14:17:57 $
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

#ifndef _CONNECTIVITY_DBASE_INDEX_HXX_
#include "dbase/DIndex.hxx"
#endif
#ifndef _CONNECTIVITY_DBASE_INDEXCOLUMNS_HXX_
#include "dbase/DIndexColumns.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#define CONNECTIVITY_PROPERTY_NAME_SPACE dbase
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include "connectivity/sdbcx/VColumn.hxx"
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _CONNECTIVITY_DBASE_TABLE_HXX_
#include "dbase/DTable.hxx"
#endif
#ifndef _CONNECTIVITY_DBASE_INDEXITER_HXX_
#include "dbase/DIndexIter.hxx"
#endif
#ifndef _CONFIG_HXX //autogen
#include <vcl/config.hxx>
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "connectivity/CommonTools.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATA_HPP_
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif
// define the properties of this lib
// this file includes the properties for this dll
namespace connectivity
{
    namespace dbase
    {
#ifndef CONNECTIVITY_USTRINGDESCRIPTION_IMPL_HXX
#include "UStringDescription_Impl.hxx"
#endif
    }
}
// -------------------------------------------------------------------------
using namespace connectivity;
using namespace ucb;
using namespace cppu;
using namespace connectivity::file;
using namespace connectivity::sdbcx;
using namespace connectivity::dbase;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::ucb;


IMPLEMENT_SERVICE_INFO(ODbaseIndex,"com.sun.star.sdbcx.driver.dbase.Index","com.sun.star.sdbcx.Index");
// -------------------------------------------------------------------------
ODbaseIndex::ODbaseIndex(ODbaseTable* _pTable) : OIndex(_pTable->getConnection()->getMetaData()->storesMixedCaseQuotedIdentifiers())
    , m_pTable(_pTable)
{
    construct();
}
// -------------------------------------------------------------------------
ODbaseIndex::ODbaseIndex(   ODbaseTable* _pTable,
                            const NDXHeader& _rHeader,
                            const ::rtl::OUString& _rName)
    : OIndex(_rName,::rtl::OUString(),_rHeader.db_unique,sal_False,sal_False,_pTable->getConnection()->getMetaData()->storesMixedCaseQuotedIdentifiers())
    , m_aHeader(_rHeader)
    , m_pTable(_pTable)
{
    construct();
}
// -------------------------------------------------------------------------
void ODbaseIndex::refreshColumns()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ::std::vector< ::rtl::OUString> aVector;
    aVector.push_back(::rtl::OUString::createFromAscii(m_aHeader.db_name));

    if(m_pColumns)
        delete m_pColumns;
    m_pColumns = new ODbaseIndexColumns(this,m_aMutex,aVector);
}
//--------------------------------------------------------------------------
Sequence< sal_Int8 > ODbaseIndex::getUnoTunnelImplementationId()
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
sal_Int64 ODbaseIndex::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    if (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return (sal_Int64)this;

    return ODbaseIndex_BASE::getSomething(rId);
}
//------------------------------------------------------------------
ONDXPagePtr ODbaseIndex::getRoot()
{
    openIndexFile();
    if (!m_aRoot.Is())
    {
        m_nRootPage = m_aHeader.db_rootpage;
        m_nPageCount = m_aHeader.db_pagecount;
        m_aRoot = CreatePage(m_nRootPage,NULL,TRUE);
    }
    return m_aRoot;
}
//------------------------------------------------------------------
sal_Bool ODbaseIndex::openIndexFile()
{
    if(!m_aFileStream.IsOpen())
    {
        INetURLObject aURL;

        aURL.SetSmartProtocol(INET_PROT_FILE);
        aURL.SetSmartURL(m_pTable->getEntry(), INetURLObject::ENCODE_ALL);

        aURL.setName(m_Name);
        aURL.setExtension(String::CreateFromAscii("ndx"));

        //  Dir* pDir = m_pTable->getConnection()->getDir();
        //  String aPath = pDir->GetName();
        //  aPath += m_Name.getStr();
        //  DirEntry aEntry(aPath);
        //  aEntry.setExtension(String::CreateFromAscii("ndx"));
        m_aFileStream.Open(aURL.getFSysPath(INetURLObject::FSYS_DETECT), STREAM_READWRITE | STREAM_NOCREATE | STREAM_SHARE_DENYWRITE);

        m_aFileStream.SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
        m_aFileStream.SetBufferSize(512);
    }

    return m_aFileStream.IsOpen();
}
//------------------------------------------------------------------
OIndexIterator* ODbaseIndex::createIterator(OBoolOperator* pOp,
                                                  const OOperand* pOperand)
{
    openIndexFile();
    return new OIndexIterator(this, pOp, pOperand);
}
//------------------------------------------------------------------
BOOL ODbaseIndex::ConvertToKey(ONDXKey* rKey, sal_uInt32 nRec, const ORowSetValue& rValue)
{
    OSL_ENSHURE(m_aFileStream.IsOpen(),"FileStream is not opened!");
    // Sucht ein bestimmten Wert im Index
    // Wenn der Index Unique ist, interssiert der Key nicht, sonst ja
    try
    {
        if (m_aHeader.db_keytype == 0)
        {
            *rKey = ONDXKey(rValue.getString(), nRec );
        }
        else
        {
            if (rValue.isNull())
                *rKey = ONDXKey(rValue.getDouble(), DataType::DOUBLE, nRec );
            else
                *rKey = ONDXKey(rValue.getDouble(), nRec );
        }
    }
    catch (...)
    {
        return FALSE;
    }
    return TRUE;
}

//------------------------------------------------------------------
BOOL ODbaseIndex::Find(sal_uInt32 nRec, const ORowSetValue& rValue)
{
    openIndexFile();
    OSL_ENSHURE(m_aFileStream.IsOpen(),"FileStream is not opened!");
    // Sucht ein bestimmten Wert im Index
    // Wenn der Index Unique ist, interssiert der Key nicht, sonst ja
    ONDXKey aKey;
    return ConvertToKey(&aKey, nRec, rValue) && getRoot()->Find(aKey);
}

//------------------------------------------------------------------
BOOL ODbaseIndex::Insert(sal_uInt32 nRec, const ORowSetValue& rValue)
{
    openIndexFile();
    OSL_ENSHURE(m_aFileStream.IsOpen(),"FileStream is not opened!");
    ONDXKey aKey;

    // Existiert der Wert bereits
    // Find immer verwenden um das aktuelle Blatt zu bestimmen
    if (!ConvertToKey(&aKey, nRec, rValue) || (getRoot()->Find(aKey) && isUnique()))
        return FALSE;

    ONDXNode aNewNode(aKey);

    // einfuegen in das aktuelle Blatt
    if (!m_aCurLeaf.Is())
        return FALSE;

    BOOL bResult = m_aCurLeaf->Insert(aNewNode);
    Release(bResult);

    return bResult;
}

//------------------------------------------------------------------
BOOL ODbaseIndex::Update(sal_uInt32 nRec, const ORowSetValue& rOldValue,
                         const ORowSetValue& rNewValue)
{
    openIndexFile();
    OSL_ENSHURE(m_aFileStream.IsOpen(),"FileStream is not opened!");
    ONDXKey aKey;
    if (!ConvertToKey(&aKey, nRec, rNewValue) || (isUnique() && getRoot()->Find(aKey)))
        return FALSE;
    else
        return Delete(nRec, rOldValue) && Insert(nRec,rNewValue);
}

//------------------------------------------------------------------
BOOL ODbaseIndex::Delete(sal_uInt32 nRec, const ORowSetValue& rValue)
{
    openIndexFile();
    OSL_ENSHURE(m_aFileStream.IsOpen(),"FileStream is not opened!");
    // Existiert der Wert bereits
    // Find immer verwenden um das aktuelle Blatt zu bestimmen
    ONDXKey aKey;
    if (!ConvertToKey(&aKey, nRec, rValue) || !getRoot()->Find(aKey))
        return FALSE;

    ONDXNode aNewNode(aKey);

    // einfuegen in das aktuelle Blatt
    if (!m_aCurLeaf.Is())
        return FALSE;
#if DEBUG
    m_aRoot->PrintPage();
#endif

    return m_aCurLeaf->Delete(m_nCurNode);
}
//------------------------------------------------------------------
void ODbaseIndex::Collect(ONDXPage* pPage)
{
    OSL_ENSHURE(m_aFileStream.IsOpen(),"FileStream is not opened!");
    if (pPage)
        m_aCollector.push_back(pPage);
}
//------------------------------------------------------------------
void ODbaseIndex::Release(BOOL bSave)
{
    // Freigeben der Indexressourcen
    m_bUseCollector = FALSE;

    if (m_aCurLeaf.Is())
    {
        m_aCurLeaf->Release(bSave);
        m_aCurLeaf.Clear();
    }

    // Wurzel freigeben
    if (m_aRoot.Is())
    {
        m_aRoot->Release(bSave);
        m_aRoot.Clear();
    }
    // alle Referenzen freigeben, bevor der FileStream geschlossen wird
    for (ULONG i = 0; i < m_aCollector.size(); i++)
        m_aCollector[i]->QueryDelete();

    m_aCollector.clear();

    // Header modifiziert ?
    if (bSave && (m_aHeader.db_rootpage != m_nRootPage ||
        m_aHeader.db_pagecount != m_nPageCount))
    {
        m_aHeader.db_rootpage = m_nRootPage;
        m_aHeader.db_pagecount = m_nPageCount;
        m_aFileStream << *this;
    }
    m_nRootPage = m_nPageCount = 0;
    m_nCurNode = NODE_NOTFOUND;
}
//------------------------------------------------------------------
ONDXPage* ODbaseIndex::CreatePage(sal_uInt32 nPagePos, ONDXPage* pParent, BOOL bLoad)
{
    OSL_ENSHURE(m_aFileStream.IsOpen(),"FileStream is not opened!");

    ONDXPage* pPage;
    if (m_aCollector.size())
    {
        pPage = *(m_aCollector.end() - 1);
        m_aCollector.pop_back();
        pPage->SetPagePos(nPagePos);
        pPage->SetParent(pParent);
    }
    else
        pPage = new ONDXPage(*this, nPagePos, pParent);

    if (bLoad)
        m_aFileStream >> *pPage;

    return pPage;
}

//------------------------------------------------------------------
SvStream& connectivity::dbase::operator >> (SvStream &rStream, ODbaseIndex& rIndex)
{
    rStream.Seek(0);
    rStream.Read(&rIndex.m_aHeader,512);

    // Text convertierung
    ByteString aText(rIndex.m_aHeader.db_name);
    //  aText.Convert(rIndex.GetDBFConnection()->GetCharacterSet(), gsl_getSystemTextEncoding());
    //  aText.Convert(rIndex.GetDBFConnection()->GetCharacterSet(), gsl_getSystemTextEncoding());
    strcpy(rIndex.m_aHeader.db_name,aText.GetBuffer());

    rIndex.m_nRootPage = rIndex.m_aHeader.db_rootpage;
    rIndex.m_nPageCount = rIndex.m_aHeader.db_pagecount;
    return rStream;
}
//------------------------------------------------------------------
SvStream& connectivity::dbase::operator << (SvStream &rStream, ODbaseIndex& rIndex)
{
    rStream.Seek(0);
    ByteString aText(rIndex.m_aHeader.db_name);
    //  aText.Convert(gsl_getSystemTextEncoding(), rIndex.GetDBFConnection()->GetCharacterSet());
    strcpy(rIndex.m_aHeader.db_name,aText.GetBuffer());
    sal_Int32 nWrites = rStream.Write(&rIndex.m_aHeader,512);
    OSL_ENSHURE(nWrites == 512,"Write not successful: Wrong header size for dbase index!");
    return rStream;
}
// -------------------------------------------------------------------------
INetURLObject ODbaseIndex::getEntry()
{
    INetURLObject aDir = m_pTable->getEntry();
    aDir.setName(m_Name);
    return aDir;
}
//------------------------------------------------------------------
void ODbaseIndex::createINFEntry()
{
    // inf Datei abgleichen
    String aNDX;
    //  Dir* pDir = m_pTable->getConnection()->getDir();
    //  String aPath = pDir->GetName();
    //  aPath += m_Name.getStr();
    INetURLObject aEntry(getEntry());
    aEntry.setExtension(String::CreateFromAscii("ndx"));

    INetURLObject aInfEntry(m_pTable->getEntry());
    aInfEntry.setExtension(String::CreateFromAscii("inf"));

    Config aInfFile(aInfEntry.getFSysPath(INetURLObject::FSYS_DETECT));
    aInfFile.SetGroup(dBASE_III_GROUP);

    USHORT nSuffix = aInfFile.GetKeyCount();
    ByteString aNewEntry,aKeyName;
    BOOL bCase = isCaseSensitive();
    while (!aNewEntry.Len())
    {
        aNewEntry = "NDX";
        aNewEntry += ByteString::CreateFromInt32(++nSuffix);
        for (USHORT i = 0; i < aInfFile.GetKeyCount(); i++)
        {
            aKeyName = aInfFile.GetKeyName(i);
            if (bCase ? aKeyName == aNewEntry : aKeyName.EqualsIgnoreCaseAscii(aNewEntry))
            {
                aNewEntry.Erase();
                break;
            }
        }
    }
    aInfFile.WriteKey(aNewEntry,ByteString(aEntry.GetName(),gsl_getSystemTextEncoding()));
}
// -------------------------------------------------------------------------
BOOL ODbaseIndex::DropImpl()
{
    if (m_aFileStream.IsOpen())
        m_aFileStream.Close();

    INetURLObject aIndexEntry(getEntry());
    aIndexEntry.setExtension(String::CreateFromAscii("ndx"));

    Content aContent(aIndexEntry.GetMainURL(),Reference<XCommandEnvironment>());
    aContent.executeCommand( rtl::OUString::createFromAscii( "delete" ),bool2any( sal_True ) );

//  ULONG nErrorCode = aIndexEntry.Kill();
//  if (nErrorCode != SVSTREAM_OK && nErrorCode != SVSTREAM_FILE_NOT_FOUND)
//  {
//      //  aStatus.SetError(nErrorCode,INDEX,aName);
//      return FALSE;
//  }

    // InfDatei abgleichen
    String aNDX;
    INetURLObject aEntry( m_pTable->getEntry());
    aEntry.setExtension(String::CreateFromAscii("inf"));

    Config aInfFile(aEntry.getFSysPath(INetURLObject::FSYS_DETECT));
    aInfFile.SetGroup(dBASE_III_GROUP);
    USHORT nKeyCnt = aInfFile.GetKeyCount();
    ByteString aKeyName;

    INetURLObject aEntryToComp(getEntry());
    aEntryToComp.setExtension(String::CreateFromAscii("ndx"));

    for (USHORT nKey = 0; nKey < nKeyCnt; nKey++)
    {
        // Verweist der Key auf ein Indexfile?...
        aKeyName = aInfFile.GetKeyName( nKey );
        //...wenn ja, Indexliste der Tabelle hinzufuegen
        if (aEntry.IsCaseSensitive() ? aKeyName.Copy(0,3) == "NDX" : aKeyName.Copy(0,3).EqualsIgnoreCaseAscii("NDX"))
        {
            aEntryToComp.setName(String(aInfFile.ReadKey(aKeyName),gsl_getSystemTextEncoding()));
            aEntryToComp.setExtension(String::CreateFromAscii("ndx"));
            if (aEntryToComp == aIndexEntry)
            {
                aInfFile.DeleteKey(aKeyName);
                break;
            }
        }
    }
    return TRUE;
}
// -------------------------------------------------------------------------
//------------------------------------------------------------------
BOOL ODbaseIndex::CreateImpl()
{
    // Anlegen des Index
    INetURLObject aEntry(getEntry());
    aEntry.setExtension(String::CreateFromAscii("ndx"));

    Content aContent(aEntry.GetMainURL(),Reference<XCommandEnvironment>());
    if (aContent.isDocument())
    {
        //  aStatus.SetError(ERRCODE_IO_ALREADYEXISTS,INDEX,aEntry.GetFull());
        return FALSE;
    }

    // Index ist nur einstufig
    if (m_pColumns->getCount() != 2)
    {
        //  aStatus.SetDriverNotCapableError();
        return FALSE;
    }

    Reference<XFastPropertySet> xCol;
    m_pColumns->getByIndex(1) >>= xCol;

    // ist die Spalte schon indiziert ?
    if (!xCol.is())
    {
//      String aText = String(OResId(STR_STAT_INDEX_COLUMN_NOT_FOUND));
//      aText.SearchAndReplace(String::CreateFromAscii("#"),pColumn->GetName());
//      aText.SearchAndReplace(String::CreateFromAscii("%"),GetTable()->Name());
//      aStatus.Set(SDB_STAT_ERROR,
//              String::CreateFromAscii("01000"),
//              aStatus.CreateErrorMessage(aText),
//              0, String() );
        return FALSE;
    }
//  else if (pColumn && pColumn->IsIndexed())
//  {
//      String aText = String(OResId(STR_STAT_INDEX_COLUMN_ALREADY_INDEXED));
//      aText.SearchAndReplace(String::CreateFromAscii("#"),pColumn->GetName());
//      aStatus.Set(SDB_STAT_ERROR,
//              String::CreateFromAscii("01000"),
//              aStatus.CreateErrorMessage(aText),
//              0, String() );
//      return FALSE;
//  }

    // Anlegen des Indexfiles
    m_aFileStream.Open(aEntry.getFSysPath(INetURLObject::FSYS_DETECT), STREAM_READWRITE | STREAM_SHARE_DENYWRITE | STREAM_TRUNC);
    if (!m_aFileStream.IsOpen())
        return FALSE;

    m_aFileStream.SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
    m_aFileStream.SetBufferSize(512);

    // Zun‰chst muﬂ das Ergebnis sortiert sein
    Reference<XStatement> xStmt = m_pTable->getConnection()->createStatement();

    String aName(getString(xCol->getFastPropertyValue(PROPERTY_ID_NAME)));

    String aQuote(m_pTable->getConnection()->getMetaData()->getIdentifierQuoteString());
    String aStatement;
    aStatement.AssignAscii("SELECT ");
    aStatement += aQuote;
    aStatement += aName;
    aStatement += aQuote;
    aStatement.AppendAscii(" FROM ");
    aStatement += aQuote;
    aStatement += m_pTable->getName().getStr();
    aStatement += aQuote;
    aStatement.AppendAscii(" ORDER BY ");
    aStatement += aQuote;
    aStatement += aName;
    aStatement += aQuote;

    if (!m_IsUnique) // zusaetzlich sortierung mit der bookmarkspalte
    {
        aStatement.AppendAscii(" ,");
        aStatement += aQuote;
        aStatement.AppendAscii("[BOOKMARK]"); // this is a special column
        aStatement += aQuote;
    }

    Reference<XResultSet> xSet = xStmt->executeQuery(aStatement);

    if (!xSet.is())
    {
        m_aFileStream.Close();
        //  aEntry.Kill();
        aContent.executeCommand( rtl::OUString::createFromAscii( "delete" ),bool2any( sal_True ) );
        return FALSE;
    }

    // Setzen der Headerinfo
    memset(&m_aHeader,0,sizeof(m_aHeader));
    m_aFileStream.SetStreamSize(512);

    sal_Int32 nType = 0;
    xCol->getFastPropertyValue(PROPERTY_ID_TYPE) >>= nType;

    m_aHeader.db_keytype = (nType == DataType::VARCHAR || nType == DataType::CHAR) ? 0 : 1;
    m_aHeader.db_keylen  = (m_aHeader.db_keytype) ? 8 : (USHORT)getINT32(xCol->getFastPropertyValue(PROPERTY_ID_PRECISION));
    m_aHeader.db_maxkeys = (512 - 8) / (8 + m_aHeader.db_keylen);
    ByteString aCol(aName,gsl_getSystemTextEncoding());
    strcpy(m_aHeader.db_name,aCol.GetBuffer());
    m_aHeader.db_unique  = m_IsUnique ? 1: 0;
    m_aHeader.db_keyrec  = m_aHeader.db_keylen + 8;

    // modifizierung am Header werden ueber Unterschiede zw. HeaderInfo und nRootPage
    // bzw. nPageCout erkannt

    m_nRootPage = 1;
    m_nPageCount = 2;

    //  ODatabaseType eType = m_aHeader.db_keytype == 0 ? DataType::VARCHAR : DataType::DOUBLE;
    m_aCurLeaf = m_aRoot = CreatePage(m_nRootPage);
    m_aRoot->SetModified(TRUE);

    m_bUseCollector = TRUE;

    //  ULONG nRowsLeft = pCursor->RowCount();
    Reference<XRow> xRow(xSet,UNO_QUERY);

    xSet->last();
    sal_Int32 nRowsLeft = xSet->getRow();
    xSet->beforeFirst();

    // Erzeugen der Indexstruktur
    while (xSet->next())
    {
        //  ODbRow& rRow = *pCursor->GetRow();
        // ueberpruefen auf doppelten eintrag
        if (m_IsUnique && m_nCurNode != NODE_NOTFOUND)
        {
            ONDXKey aKey(m_aHeader.db_keytype ? ORowSetValue(xRow->getDouble(1)) : ORowSetValue(xRow->getString(1)), nType, 0);
            if (aKey == (*m_aCurLeaf)[m_nCurNode].GetKey())
            {
//              String aText = String(OResId(STR_STAT_INDEX_NOT_UNIQUE));
//              aText.SearchAndReplace(String::CreateFromAscii("#"),aName);
//              aStatus.Set(SDB_STAT_ERROR,
//                      String::CreateFromAscii("01000"),
//                      aStatus.CreateErrorMessage(aText),
//                      0, String() );
                break;
            }
        }
        ONDXKey aKey(m_aHeader.db_keytype ? ORowSetValue(xRow->getDouble(1)) : ORowSetValue(xRow->getString(1)), nType, xSet->getRow());
        ONDXNode aNewNode(aKey);
        if (!m_aCurLeaf->Insert(aNewNode, --nRowsLeft))
            break;

#ifdef DEBUG
        //DBG_TRACE1("SDB: %s", (const char*)pCursor->Variable(1)->GetString());
        //      PrintTree();
#endif
    }

//  BOOL bResult = !pCursor->IsInRange();
//  if (!bResult)
//  {
//      m_aFileStream.Close();
//      aEntry.Kill();
//      Release(FALSE);
//  }
//  else
//  {

        Release();
//      m_aFileStream.Close();
        // den FielStream NICHT schliessen, da per definitionem ein OObject nach dem Kreieren offen ist

        // inf Datei abgleichen
        createINFEntry();
//  }
//
    //  pCursor->ReleaseRef();
    return sal_True;
}



