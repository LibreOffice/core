/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <dbase/DIndex.hxx>
#include <dbase/DIndexColumns.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <connectivity/sdbcx/VColumn.hxx>
#include <dbase/DTable.hxx>
#include <dbase/DIndexIter.hxx>
#include <osl/file.hxx>
#include <sal/log.hxx>
#include <tools/config.hxx>
#include <connectivity/CommonTools.hxx>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <unotools/ucbhelper.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <connectivity/dbexception.hxx>
#include <dbase/DResultSet.hxx>
#include <strings.hrc>
#include <unotools/sharedunocomponent.hxx>

using namespace ::comphelper;

using namespace connectivity;
using namespace utl;
using namespace ::cppu;
using namespace connectivity::file;
using namespace connectivity::sdbcx;
using namespace connectivity::dbase;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;

IMPLEMENT_SERVICE_INFO(ODbaseIndex,"com.sun.star.sdbcx.driver.dbase.Index","com.sun.star.sdbcx.Index");

ODbaseIndex::ODbaseIndex(ODbaseTable* _pTable)
    : OIndex(true/*_pTable->getConnection()->getMetaData()->supportsMixedCaseQuotedIdentifiers()*/)
    , m_nCurNode(NODE_NOTFOUND)
    , m_nPageCount(0)
    , m_nRootPage(0)
    , m_pTable(_pTable)
    , m_bUseCollector(false)
{
    memset(&m_aHeader, 0, sizeof(m_aHeader));
    construct();
}

ODbaseIndex::ODbaseIndex(   ODbaseTable* _pTable,
                            const NDXHeader& _rHeader,
                            const OUString& _rName)
    : OIndex(_rName, OUString(), _rHeader.db_unique, false, false, true)
    , m_aHeader(_rHeader)
    , m_nCurNode(NODE_NOTFOUND)
    , m_nPageCount(0)
    , m_nRootPage(0)
    , m_pTable(_pTable)
    , m_bUseCollector(false)
{
    construct();
}

ODbaseIndex::~ODbaseIndex()
{
    closeImpl();
}

void ODbaseIndex::refreshColumns()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ::std::vector< OUString> aVector;
    if(!isNew())
    {
        OSL_ENSURE(m_pFileStream,"FileStream is not opened!");
        OSL_ENSURE(m_aHeader.db_name[0] != '\0',"Invalid name for the column!");
        aVector.push_back(OUString::createFromAscii(m_aHeader.db_name));
    }

    if(m_pColumns)
        m_pColumns->reFill(aVector);
    else
        m_pColumns = new ODbaseIndexColumns(this,m_aMutex,aVector);
}

Sequence< sal_Int8 > ODbaseIndex::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId implId;

    return implId.getImplementationId();
}

// XUnoTunnel

sal_Int64 ODbaseIndex::getSomething( const Sequence< sal_Int8 > & rId )
{
    return (rId.getLength() == 16 && 0 == memcmp(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
                ? reinterpret_cast< sal_Int64 >( this )
                : ODbaseIndex_BASE::getSomething(rId);
}

ONDXPagePtr const & ODbaseIndex::getRoot()
{
    openIndexFile();
    if (!m_aRoot.Is())
    {
        m_nRootPage = m_aHeader.db_rootpage;
        m_nPageCount = m_aHeader.db_pagecount;
        m_aRoot = CreatePage(m_nRootPage,nullptr,true);
    }
    return m_aRoot;
}

void ODbaseIndex::openIndexFile()
{
    if(!m_pFileStream)
    {
        OUString sFile = getCompletePath();
        if(UCBContentHelper::Exists(sFile))
        {
            m_pFileStream = OFileTable::createStream_simpleError(sFile, StreamMode::READWRITE | StreamMode::NOCREATE | StreamMode::SHARE_DENYWRITE);
            if (!m_pFileStream)
                m_pFileStream = OFileTable::createStream_simpleError(sFile, StreamMode::READ | StreamMode::NOCREATE | StreamMode::SHARE_DENYNONE);
            if(m_pFileStream)
            {
                m_pFileStream->SetEndian(SvStreamEndian::LITTLE);
                m_pFileStream->SetBufferSize(DINDEX_PAGE_SIZE);
                (*m_pFileStream) >> *this;
            }
        }
        if(!m_pFileStream)
        {
            const OUString sError( m_pTable->getConnection()->getResources().getResourceStringWithSubstitution(
                STR_COULD_NOT_LOAD_FILE,
                "$filename$", sFile
             ) );
            ::dbtools::throwGenericSQLException( sError, *this );
        }
    }
}

std::unique_ptr<OIndexIterator> ODbaseIndex::createIterator()
{
    openIndexFile();
    return std::make_unique<OIndexIterator>(this);
}

bool ODbaseIndex::ConvertToKey(ONDXKey* rKey, sal_uInt32 nRec, const ORowSetValue& rValue)
{
    OSL_ENSURE(m_pFileStream,"FileStream is not opened!");
    // Search a specific value in Index
    // If the Index is unique, the key doesn't matter
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
        OSL_ASSERT(false);
        return false;
    }
    return true;
}


bool ODbaseIndex::Find(sal_uInt32 nRec, const ORowSetValue& rValue)
{
    openIndexFile();
    OSL_ENSURE(m_pFileStream,"FileStream is not opened!");
    // Search a specific value in Index
    // If the Index is unique, the key doesn't matter
    ONDXKey aKey;
    return ConvertToKey(&aKey, nRec, rValue) && getRoot()->Find(aKey);
}


bool ODbaseIndex::Insert(sal_uInt32 nRec, const ORowSetValue& rValue)
{
    openIndexFile();
    OSL_ENSURE(m_pFileStream,"FileStream is not opened!");
    ONDXKey aKey;

    // Does the value already exist
    // Use Find() always to determine the actual leaf
    if (!ConvertToKey(&aKey, nRec, rValue) || (getRoot()->Find(aKey) && isUnique()))
        return false;

    ONDXNode aNewNode(aKey);

    // insert in the current leaf
    if (!m_aCurLeaf.Is())
        return false;

    bool bResult = m_aCurLeaf->Insert(aNewNode);
    Release(bResult);

    return bResult;
}


bool ODbaseIndex::Update(sal_uInt32 nRec, const ORowSetValue& rOldValue,
                         const ORowSetValue& rNewValue)
{
    openIndexFile();
    OSL_ENSURE(m_pFileStream,"FileStream is not opened!");
    ONDXKey aKey;
    if (!ConvertToKey(&aKey, nRec, rNewValue) || (isUnique() && getRoot()->Find(aKey)))
        return false;
    else
        return Delete(nRec, rOldValue) && Insert(nRec,rNewValue);
}


bool ODbaseIndex::Delete(sal_uInt32 nRec, const ORowSetValue& rValue)
{
    openIndexFile();
    OSL_ENSURE(m_pFileStream,"FileStream is not opened!");
    // Does the value already exist
    // Always use Find() to determine the actual leaf
    ONDXKey aKey;
    if (!ConvertToKey(&aKey, nRec, rValue) || !getRoot()->Find(aKey))
        return false;

    // insert in the current leaf
    if (!m_aCurLeaf.Is())
        return false;
#if OSL_DEBUG_LEVEL > 1
    m_aRoot->PrintPage();
#endif

    m_aCurLeaf->Delete(m_nCurNode);
    return true;
}

void ODbaseIndex::Collect(ONDXPage* pPage)
{
    if (pPage)
        m_aCollector.push_back(pPage);
}

void ODbaseIndex::Release(bool bSave)
{
    // Release the Index-resources
    m_bUseCollector = false;

    if (m_aCurLeaf.Is())
    {
        m_aCurLeaf->Release(bSave);
        m_aCurLeaf.Clear();
    }

    // Release the root
    if (m_aRoot.Is())
    {
        m_aRoot->Release(bSave);
        m_aRoot.Clear();
    }
    // Release all references, before the FileStream will be closed
    for (auto& i : m_aCollector)
        i->QueryDelete();

    m_aCollector.clear();

    // Header modified?
    if (bSave && (m_aHeader.db_rootpage != m_nRootPage ||
        m_aHeader.db_pagecount != m_nPageCount))
    {
        m_aHeader.db_rootpage = m_nRootPage;
        m_aHeader.db_pagecount = m_nPageCount;
        WriteODbaseIndex( *m_pFileStream, *this );
    }
    m_nRootPage = m_nPageCount = 0;
    m_nCurNode = NODE_NOTFOUND;

    closeImpl();
}

void ODbaseIndex::closeImpl()
{
    m_pFileStream.reset();
}

ONDXPage* ODbaseIndex::CreatePage(sal_uInt32 nPagePos, ONDXPage* pParent, bool bLoad)
{
    OSL_ENSURE(m_pFileStream,"FileStream is not opened!");

    ONDXPage* pPage;
    if ( !m_aCollector.empty() )
    {
        pPage = *(m_aCollector.rbegin());
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

void connectivity::dbase::ReadHeader(
        SvStream & rStream, ODbaseIndex::NDXHeader & rHeader)
{
#if !defined(NDEBUG)
    sal_uInt64 const nOldPos(rStream.Tell());
#endif
    rStream.ReadUInt32(rHeader.db_rootpage);
    rStream.ReadUInt32(rHeader.db_pagecount);
    rStream.ReadBytes(&rHeader.db_free, 4);
    rStream.ReadUInt16(rHeader.db_keylen);
    rStream.ReadUInt16(rHeader.db_maxkeys);
    rStream.ReadUInt16(rHeader.db_keytype);
    rStream.ReadUInt16(rHeader.db_keyrec);
    rStream.ReadBytes(&rHeader.db_free1, 3);
    rStream.ReadUChar(rHeader.db_unique);
    rStream.ReadBytes(&rHeader.db_name, 488);
    assert(rStream.GetError() || rStream.Tell() == nOldPos + DINDEX_PAGE_SIZE);
}

SvStream& connectivity::dbase::operator >> (SvStream &rStream, ODbaseIndex& rIndex)
{
    rStream.Seek(0);
    ReadHeader(rStream, rIndex.m_aHeader);

    rIndex.m_nRootPage = rIndex.m_aHeader.db_rootpage;
    rIndex.m_nPageCount = rIndex.m_aHeader.db_pagecount;
    return rStream;
}

SvStream& connectivity::dbase::WriteODbaseIndex(SvStream &rStream, ODbaseIndex& rIndex)
{
    rStream.Seek(0);
    rStream.WriteUInt32(rIndex.m_aHeader.db_rootpage);
    rStream.WriteUInt32(rIndex.m_aHeader.db_pagecount);
    rStream.WriteBytes(&rIndex.m_aHeader.db_free, 4);
    rStream.WriteUInt16(rIndex.m_aHeader.db_keylen);
    rStream.WriteUInt16(rIndex.m_aHeader.db_maxkeys);
    rStream.WriteUInt16(rIndex.m_aHeader.db_keytype);
    rStream.WriteUInt16(rIndex.m_aHeader.db_keyrec);
    rStream.WriteBytes(&rIndex.m_aHeader.db_free1, 3);
    rStream.WriteUChar(rIndex.m_aHeader.db_unique);
    rStream.WriteBytes(&rIndex.m_aHeader.db_name, 488);
    assert(rStream.GetError() || rStream.Tell() == DINDEX_PAGE_SIZE);
    SAL_WARN_IF(rStream.GetError(), "connectivity.dbase", "write error");
    return rStream;
}

OUString ODbaseIndex::getCompletePath()
{
    OUString sDir = m_pTable->getConnection()->getURL() +
        OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DELIMITER) +
        m_Name + ".ndx";
    return sDir;
}

void ODbaseIndex::createINFEntry()
{
    // synchronize inf-file
    const OUString sEntry(m_Name + ".ndx");

    OUString sCfgFile(m_pTable->getConnection()->getURL() +
                      OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DELIMITER) +
                      m_pTable->getName() +
                      ".inf");

    OUString sPhysicalPath;
    osl::FileBase::getSystemPathFromFileURL(sCfgFile, sPhysicalPath);

    Config aInfFile(sPhysicalPath);
    aInfFile.SetGroup(dBASE_III_GROUP);

    sal_uInt16 nSuffix = aInfFile.GetKeyCount();
    OString aNewEntry,aKeyName;
    bool bCase = isCaseSensitive();
    while (aNewEntry.isEmpty())
    {
        aNewEntry = OString("NDX");
        aNewEntry += OString::number(++nSuffix);
        for (sal_uInt16 i = 0; i < aInfFile.GetKeyCount(); i++)
        {
            aKeyName = aInfFile.GetKeyName(i);
            if (bCase ? aKeyName == aNewEntry : aKeyName.equalsIgnoreAsciiCase(aNewEntry))
            {
                aNewEntry.clear();
                break;
            }
        }
    }
    aInfFile.WriteKey(aNewEntry, OUStringToOString(sEntry, m_pTable->getConnection()->getTextEncoding()));
}

void ODbaseIndex::DropImpl()
{
    closeImpl();

    OUString sPath = getCompletePath();
    if(UCBContentHelper::Exists(sPath))
    {
        if(!UCBContentHelper::Kill(sPath))
            m_pTable->getConnection()->throwGenericSQLException(STR_COULD_NOT_DELETE_INDEX,*m_pTable);
    }

    // synchronize inf-file
    OUString sCfgFile = m_pTable->getConnection()->getURL() +
        OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DELIMITER) +
        m_pTable->getName() + ".inf";

    OUString sPhysicalPath;
    OSL_VERIFY( osl::FileBase::getSystemPathFromFileURL(sCfgFile, sPhysicalPath)
                    == osl::FileBase::E_None );

    Config aInfFile(sPhysicalPath);
    aInfFile.SetGroup(dBASE_III_GROUP);
    sal_uInt16 nKeyCnt = aInfFile.GetKeyCount();
    OString aKeyName;
    OUString sEntry = m_Name + ".ndx";

    // delete entries from the inf file
    for (sal_uInt16 nKey = 0; nKey < nKeyCnt; nKey++)
    {
        // References the Key to an Index-file?
        aKeyName = aInfFile.GetKeyName( nKey );
        if (aKeyName.startsWith("NDX"))
        {
            if(sEntry == OStringToOUString(aInfFile.ReadKey(aKeyName),m_pTable->getConnection()->getTextEncoding()))
            {
                aInfFile.DeleteKey(aKeyName);
                break;
            }
        }
    }
}

void ODbaseIndex::impl_killFileAndthrowError_throw(const char* pErrorId, const OUString& _sFile)
{
    closeImpl();
    if(UCBContentHelper::Exists(_sFile))
        UCBContentHelper::Kill(_sFile);
    m_pTable->getConnection()->throwGenericSQLException(pErrorId, *this);
}

void ODbaseIndex::CreateImpl()
{
    // Create the Index
    const OUString sFile = getCompletePath();
    if(UCBContentHelper::Exists(sFile))
    {
        const OUString sError( m_pTable->getConnection()->getResources().getResourceStringWithSubstitution(
            STR_COULD_NOT_CREATE_INDEX_NAME,
            "$filename$", sFile
         ) );
        ::dbtools::throwGenericSQLException( sError, *this );
    }
    // Index comprises only one column
    if (m_pColumns->getCount() > 1)
        m_pTable->getConnection()->throwGenericSQLException(STR_ONL_ONE_COLUMN_PER_INDEX,*this);

    Reference<XFastPropertySet> xCol(m_pColumns->getByIndex(0),UNO_QUERY);

    // Is the column already indexed?
    if ( !xCol.is() )
        ::dbtools::throwFunctionSequenceException(*this);

    // create the index file
    m_pFileStream = OFileTable::createStream_simpleError(sFile,StreamMode::READWRITE | StreamMode::SHARE_DENYWRITE | StreamMode::TRUNC);
    if (!m_pFileStream)
    {
        const OUString sError( m_pTable->getConnection()->getResources().getResourceStringWithSubstitution(
            STR_COULD_NOT_LOAD_FILE,
            "$filename$", sFile
         ) );
        ::dbtools::throwGenericSQLException( sError, *this );
    }

    m_pFileStream->SetEndian(SvStreamEndian::LITTLE);
    m_pFileStream->SetBufferSize(DINDEX_PAGE_SIZE);

    // firstly the result must be sorted
    utl::SharedUNOComponent<XStatement> xStmt;
    utl::SharedUNOComponent<XResultSet> xSet;
    OUString aName;
    try
    {
        xStmt.set( m_pTable->getConnection()->createStatement(), UNO_SET_THROW);

        aName = getString(xCol->getFastPropertyValue(PROPERTY_ID_NAME));

        const OUString aQuote(m_pTable->getConnection()->getMetaData()->getIdentifierQuoteString());
        OUString aStatement( "SELECT " + aQuote + aName + aQuote +" FROM " + aQuote + m_pTable->getName() + aQuote + " ORDER BY " + aQuote + aName + aQuote);

        xSet.set( xStmt->executeQuery(aStatement),UNO_SET_THROW );
    }
    catch(const Exception& )
    {
        impl_killFileAndthrowError_throw(STR_COULD_NOT_CREATE_INDEX,sFile);
    }
    if (!xSet.is())
    {
        impl_killFileAndthrowError_throw(STR_COULD_NOT_CREATE_INDEX,sFile);
    }

    // Set the header info
    memset(&m_aHeader,0,sizeof(m_aHeader));
    sal_Int32 nType = 0;
    ::rtl::Reference<OSQLColumns> aCols = m_pTable->getTableColumns();
    const Reference< XPropertySet > xTableCol(*find(aCols->get().begin(),aCols->get().end(),aName,::comphelper::UStringMixEqual(isCaseSensitive())));

    xTableCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE)) >>= nType;

    m_aHeader.db_keytype = (nType == DataType::VARCHAR || nType == DataType::CHAR) ? 0 : 1;
    m_aHeader.db_keylen  = (m_aHeader.db_keytype) ? 8 : static_cast<sal_uInt16>(getINT32(xTableCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION))));
    m_aHeader.db_keylen = (( m_aHeader.db_keylen - 1) / 4 + 1) * 4;
    m_aHeader.db_maxkeys = (DINDEX_PAGE_SIZE - 4) / (8 + m_aHeader.db_keylen);
    if ( m_aHeader.db_maxkeys < 3 )
    {
        impl_killFileAndthrowError_throw(STR_COULD_NOT_CREATE_INDEX_KEYSIZE,sFile);
    }

    m_pFileStream->SetStreamSize(DINDEX_PAGE_SIZE);

    OString aCol(OUStringToOString(aName, m_pTable->getConnection()->getTextEncoding()));
    strncpy(m_aHeader.db_name, aCol.getStr(), std::min<size_t>(sizeof(m_aHeader.db_name), aCol.getLength()));
    m_aHeader.db_unique  = m_IsUnique ? 1: 0;
    m_aHeader.db_keyrec  = m_aHeader.db_keylen + 8;

    // modifications of the header are detected by differences between
    // the HeaderInfo and nRootPage or nPageCount respectively
    m_nRootPage = 1;
    m_nPageCount = 2;

    m_aCurLeaf = m_aRoot = CreatePage(m_nRootPage);
    m_aRoot->SetModified(true);

    m_bUseCollector = true;

    sal_Int32 nRowsLeft = 0;
    Reference<XRow> xRow(xSet,UNO_QUERY);

    if(xSet->last())
    {
        Reference< XUnoTunnel> xTunnel(xSet, UNO_QUERY_THROW);
        ODbaseResultSet* pDbaseRes = reinterpret_cast< ODbaseResultSet* >( xTunnel->getSomething(ODbaseResultSet::getUnoTunnelImplementationId()) );
        assert(pDbaseRes); //"No dbase resultset found? What's going on here!
        nRowsLeft = xSet->getRow();

        xSet->beforeFirst();
        ORowSetValue    atmpValue=ORowSetValue();
        ONDXKey aKey(atmpValue, nType, 0);
        ONDXKey aInsertKey(atmpValue, nType, 0);
        // Create the index structure
        while (xSet->next())
        {
            ORowSetValue aValue(m_aHeader.db_keytype ? ORowSetValue(xRow->getDouble(1)) : ORowSetValue(xRow->getString(1)));
            // checking for duplicate entries
            if (m_IsUnique && m_nCurNode != NODE_NOTFOUND)
            {
                aKey.setValue(aValue);
                if (aKey == (*m_aCurLeaf)[m_nCurNode].GetKey())
                {
                    impl_killFileAndthrowError_throw(STR_COULD_NOT_CREATE_INDEX_NOT_UNIQUE,sFile);
                }
            }
            aInsertKey.setValue(aValue);
            aInsertKey.setRecord(pDbaseRes->getCurrentFilePos());

            ONDXNode aNewNode(aInsertKey);
            if (!m_aCurLeaf->Insert(aNewNode, --nRowsLeft))
                break;
        }
    }

    if(nRowsLeft)
    {
        impl_killFileAndthrowError_throw(STR_COULD_NOT_CREATE_INDEX,sFile);
    }
    Release();
    createINFEntry();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
