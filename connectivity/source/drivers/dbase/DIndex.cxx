/*************************************************************************
 *
 *  $RCSfile: DIndex.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-30 13:57:18 $
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
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#ifndef _UNOTOOLS_UCBHELPER_HXX
#include <unotools/ucbhelper.hxx>
#endif
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
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
using namespace utl;
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
    ,m_pFileStream(NULL)
{
    m_aHeader.db_maxkeys = m_aHeader.db_maxkeys = m_aHeader.db_keylen = m_aHeader.db_pagecount = m_aHeader.db_rootpage = 0;
    m_aHeader.db_name[0] = '\0';
    construct();
}
// -------------------------------------------------------------------------
ODbaseIndex::ODbaseIndex(   ODbaseTable* _pTable,
                            const NDXHeader& _rHeader,
                            const ::rtl::OUString& _rName)
    : OIndex(_rName,::rtl::OUString(),_rHeader.db_unique,sal_False,sal_False,_pTable->getConnection()->getMetaData()->storesMixedCaseQuotedIdentifiers())
    , m_aHeader(_rHeader)
    , m_pTable(_pTable)
    ,m_pFileStream(NULL)
{
    construct();
}
// -----------------------------------------------------------------------------
ODbaseIndex::~ODbaseIndex()
{
    closeImpl();
}
// -------------------------------------------------------------------------
void ODbaseIndex::refreshColumns()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ::std::vector< ::rtl::OUString> aVector;
    if(!isNew())
    {
        OSL_ENSURE(m_pFileStream,"FileStream is not opened!");
        OSL_ENSURE(m_aHeader.db_name[0] != '\0',"Invalid name for the column!");
        aVector.push_back(::rtl::OUString::createFromAscii(m_aHeader.db_name));
    }

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
    if(!m_pFileStream)
    {
        ::rtl::OUString sFile = getCompletePath();
        if(UCBContentHelper::Exists(sFile))
        {
            m_pFileStream = UcbStreamHelper::CreateStream(sFile,STREAM_READWRITE | STREAM_NOCREATE | STREAM_SHARE_DENYWRITE);
            if(!m_pFileStream)
                m_pFileStream = UcbStreamHelper::CreateStream(sFile,STREAM_READ | STREAM_NOCREATE | STREAM_SHARE_DENYNONE );
            if(m_pFileStream)
            {
                m_pFileStream->SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
                m_pFileStream->SetBufferSize(512);
                (*m_pFileStream) >> *this;
            }
        }
        if(!m_pFileStream)
        {
            ::rtl::OUString sErrMsg = ::rtl::OUString::createFromAscii("Could not open index: ");
            sErrMsg += sFile;
            throw SQLException(sErrMsg,*this,SQLSTATE_GENERAL,1000,Any());
        }
    }

    return m_pFileStream != NULL;
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
    OSL_ENSURE(m_pFileStream,"FileStream is not opened!");
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
    catch (Exception&)
    {
        OSL_ASSERT(0);
        return FALSE;
    }
    return TRUE;
}

//------------------------------------------------------------------
BOOL ODbaseIndex::Find(sal_uInt32 nRec, const ORowSetValue& rValue)
{
    openIndexFile();
    OSL_ENSURE(m_pFileStream,"FileStream is not opened!");
    // Sucht ein bestimmten Wert im Index
    // Wenn der Index Unique ist, interssiert der Key nicht, sonst ja
    ONDXKey aKey;
    return ConvertToKey(&aKey, nRec, rValue) && getRoot()->Find(aKey);
}

//------------------------------------------------------------------
BOOL ODbaseIndex::Insert(sal_uInt32 nRec, const ORowSetValue& rValue)
{
    openIndexFile();
    OSL_ENSURE(m_pFileStream,"FileStream is not opened!");
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
    OSL_ENSURE(m_pFileStream,"FileStream is not opened!");
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
    OSL_ENSURE(m_pFileStream,"FileStream is not opened!");
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
    OSL_ENSURE(m_pFileStream,"FileStream is not opened!");
    if (pPage)
    {
        pPage->acquire();
        m_aCollector.push_back(pPage);
    }
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
        (*m_pFileStream) << *this;
    }
    m_nRootPage = m_nPageCount = 0;
    m_nCurNode = NODE_NOTFOUND;

    closeImpl();
}
// -----------------------------------------------------------------------------
void ODbaseIndex::closeImpl()
{
    if(m_pFileStream)
    {
        delete m_pFileStream;
        m_pFileStream = NULL;
    }
}
//------------------------------------------------------------------
ONDXPage* ODbaseIndex::CreatePage(sal_uInt32 nPagePos, ONDXPage* pParent, BOOL bLoad)
{
    OSL_ENSURE(m_pFileStream,"FileStream is not opened!");

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
        (*m_pFileStream) >> *pPage;

    return pPage;
}

//------------------------------------------------------------------
SvStream& connectivity::dbase::operator >> (SvStream &rStream, ODbaseIndex& rIndex)
{
    rStream.Seek(0);
    rStream.Read(&rIndex.m_aHeader,512);

    // Text convertierung
    ByteString aText(rIndex.m_aHeader.db_name);
    //  aText.Convert(rIndex.m_pTable->getConnection()->GetCharacterSet(), m_pTable->getConnection()->getTextEncoding());
    //  aText.Convert(rIndex.m_pTable->getConnection()->GetCharacterSet(), m_pTable->getConnection()->getTextEncoding());
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
    //  aText.Convert(m_pTable->getConnection()->getTextEncoding(), rIndex.m_pTable->getConnection()->GetCharacterSet());
    strcpy(rIndex.m_aHeader.db_name,aText.GetBuffer());
    sal_Int32 nWrites = rStream.Write(&rIndex.m_aHeader,512);
    OSL_ENSURE(nWrites == 512,"Write not successful: Wrong header size for dbase index!");
    return rStream;
}
// -------------------------------------------------------------------------
::rtl::OUString ODbaseIndex::getCompletePath()
{
    ::rtl::OUString sDir = m_pTable->getConnection()->getURL();
    sDir += STR_DELIMITER;
    sDir += m_Name;
    sDir += ::rtl::OUString::createFromAscii(".ndx");
    return sDir;
}
//------------------------------------------------------------------
void ODbaseIndex::createINFEntry()
{
    // inf Datei abgleichen
    String sEntry = m_Name;
    sEntry += String::CreateFromAscii(".ndx");

    ::rtl::OUString sCfgFile(m_pTable->getConnection()->getURL());
    sCfgFile += STR_DELIMITER;
    sCfgFile += m_pTable->getName();
    sCfgFile += ::rtl::OUString::createFromAscii(".inf");

    String sPhysicalPath;
    LocalFileHelper::ConvertURLToPhysicalName(sCfgFile,sPhysicalPath);

    Config aInfFile(sPhysicalPath);
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
    aInfFile.WriteKey(aNewEntry,ByteString(sEntry,m_pTable->getConnection()->getTextEncoding()));
}
// -------------------------------------------------------------------------
BOOL ODbaseIndex::DropImpl()
{
    closeImpl();

    ::rtl::OUString sPath = getCompletePath();
    if(UCBContentHelper::Exists(sPath))
    {
        if(!UCBContentHelper::Kill(sPath))
            throw SQLException(::rtl::OUString::createFromAscii("Could not delete index!"),*m_pTable,SQLSTATE_GENERAL,1000,Any());
    }

    // InfDatei abgleichen

    ::rtl::OUString sCfgFile(m_pTable->getConnection()->getURL());
    sCfgFile += STR_DELIMITER;
    sCfgFile += m_pTable->getName();
    sCfgFile += ::rtl::OUString::createFromAscii(".inf");

    String sPhysicalPath;
    String sNDX(sCfgFile);
    sal_Bool bOk = LocalFileHelper::ConvertURLToPhysicalName(sNDX,sPhysicalPath);
    OSL_ENSURE(bOk,"Can not convert Config Filename into Physical Name!");

    Config aInfFile(sPhysicalPath);
    aInfFile.SetGroup(dBASE_III_GROUP);
    USHORT nKeyCnt = aInfFile.GetKeyCount();
    ByteString aKeyName;
    String sEntry = m_Name;
    sEntry += String::CreateFromAscii(".ndx");

    // delete entries from the inf file
    for (USHORT nKey = 0; nKey < nKeyCnt; nKey++)
    {
        // Verweist der Key auf ein Indexfile?...
        aKeyName = aInfFile.GetKeyName( nKey );
        if (aKeyName.Copy(0,3) == "ndx")
        {
            if(sEntry == String(aInfFile.ReadKey(aKeyName),m_pTable->getConnection()->getTextEncoding()))
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
    ::rtl::OUString sFile = getCompletePath();
    if(UCBContentHelper::Exists(sFile))
        throw SQLException(::rtl::OUString::createFromAscii("Object already exists!"),*this,SQLSTATE_SEQUENCE,1000,Any());

    // Index ist nur einstufig
    if (m_pColumns->getCount() > 1)
        throw SQLException(::rtl::OUString::createFromAscii("Not capable! Only one column per index."),*this,SQLSTATE_SEQUENCE,1000,Any());

    Reference<XFastPropertySet> xCol;
    ::cppu::extractInterface(xCol,m_pColumns->getByIndex(0));

    // ist die Spalte schon indiziert ?
    if (!xCol.is())
        throw ::dbtools::FunctionSequenceException(*this);
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

    // create the index file
    m_pFileStream = UcbStreamHelper::CreateStream(sFile,STREAM_READWRITE | STREAM_SHARE_DENYWRITE | STREAM_TRUNC);
    if (!m_pFileStream)
        throw SQLException(::rtl::OUString::createFromAscii("Could not access index file!"),*this,SQLSTATE_SEQUENCE,1000,Any());

    m_pFileStream->SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
    m_pFileStream->SetBufferSize(512);

    // Zun‰chst muﬂ das Ergebnis sortiert sein
    Reference<XStatement> xStmt;
    Reference<XResultSet> xSet;
    String aName;
    try
    {
        xStmt = m_pTable->getConnection()->createStatement();

        aName = getString(xCol->getFastPropertyValue(PROPERTY_ID_NAME));

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

//      if (!m_IsUnique) // zusaetzlich sortierung mit der bookmarkspalte
//      {
//          aStatement.AppendAscii(" ,");
//          aStatement += aQuote;
//          aStatement.AppendAscii("[BOOKMARK]"); // this is a special column
//          aStatement += aQuote;
//      }

        xSet = xStmt->executeQuery(aStatement);
    }
    catch(Exception& e)
    {

        closeImpl();
        if(UCBContentHelper::Exists(sFile))
            UCBContentHelper::Kill(sFile);
        throw SQLException(::rtl::OUString::createFromAscii("Could not create index!"),*this,SQLSTATE_SEQUENCE,1000,makeAny(e));
    }
    if (!xSet.is())
    {

        closeImpl();
        if(UCBContentHelper::Exists(sFile))
            UCBContentHelper::Kill(sFile);
        throw SQLException(::rtl::OUString::createFromAscii("Could not create index!"),*this,SQLSTATE_SEQUENCE,1000,Any());
    }

    // Setzen der Headerinfo
    memset(&m_aHeader,0,sizeof(m_aHeader));
    m_pFileStream->SetStreamSize(512);

    sal_Int32 nType = 0;
    ::vos::ORef<OSQLColumns> aCols = m_pTable->getTableColumns();

    Reference< XPropertySet > xTableCol(*find(aCols->begin(),aCols->end(),aName,::comphelper::UStringMixEqual(isCaseSensitive())));

    xTableCol->getPropertyValue(PROPERTY_TYPE) >>= nType;

    m_aHeader.db_keytype = (nType == DataType::VARCHAR || nType == DataType::CHAR) ? 0 : 1;
    m_aHeader.db_keylen  = (m_aHeader.db_keytype) ? 8 : (USHORT)getINT32(xTableCol->getPropertyValue(PROPERTY_PRECISION));
    m_aHeader.db_maxkeys = (512 - 8) / (8 + m_aHeader.db_keylen);

    ByteString aCol(aName,m_pTable->getConnection()->getTextEncoding());
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

    if(xSet->last())
    {
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

                    closeImpl();
                    if(UCBContentHelper::Exists(sFile))
                        UCBContentHelper::Kill(sFile);
                    throw SQLException(::rtl::OUString::createFromAscii("Can not create index values are not unique!"),*this,SQLSTATE_GENERAL,1000,Any());
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
    }
    xRow = NULL;
    ::comphelper::disposeComponent(xSet);
    ::comphelper::disposeComponent(xStmt);

    Release();
    createINFEntry();
    return sal_True;
}
// -----------------------------------------------------------------------------



