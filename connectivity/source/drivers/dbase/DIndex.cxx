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

#include "dbase/DIndex.hxx"
#include "dbase/DIndexColumns.hxx"
#include <com/sun/star/lang/DisposedException.hpp>
#include "connectivity/sdbcx/VColumn.hxx"
#include <comphelper/sequence.hxx>
#include "dbase/DTable.hxx"
#include "dbase/DIndexIter.hxx"
#include <tools/config.hxx>
#include "connectivity/CommonTools.hxx"
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <comphelper/extract.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbhelper.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbexception.hxx>
#include "dbase/DResultSet.hxx"
#include "diagnose_ex.h"
#include "resource/dbase_res.hrc"
#include <unotools/sharedunocomponent.hxx>

using namespace ::comphelper;
// -------------------------------------------------------------------------
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
// -------------------------------------------------------------------------
ODbaseIndex::ODbaseIndex(ODbaseTable* _pTable) : OIndex(sal_True/*_pTable->getConnection()->getMetaData()->supportsMixedCaseQuotedIdentifiers()*/)
    ,m_pFileStream(NULL)
    ,m_nCurNode(NODE_NOTFOUND)
    ,m_pTable(_pTable)
{
    m_aHeader.db_pagecount = m_aHeader.db_rootpage = m_aHeader.db_keytype = m_aHeader.db_maxkeys = m_aHeader.db_keylen = 0;
    m_aHeader.db_name[0] = '\0';
    construct();
}
// -------------------------------------------------------------------------
ODbaseIndex::ODbaseIndex(   ODbaseTable* _pTable,
                            const NDXHeader& _rHeader,
                            const ::rtl::OUString& _rName)
    :OIndex(_rName,::rtl::OUString(),_rHeader.db_unique,sal_False,sal_False,sal_True)
    ,m_pFileStream(NULL)
    ,m_aHeader(_rHeader)
    ,m_nCurNode(NODE_NOTFOUND)
    ,m_pTable(_pTable)
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

    TStringVector aVector;
    if(!isNew())
    {
        OSL_ENSURE(m_pFileStream,"FileStream is not opened!");
        OSL_ENSURE(m_aHeader.db_name[0] != '\0',"Invalid name for the column!");
        aVector.push_back(::rtl::OUString::createFromAscii(m_aHeader.db_name));
    }

    if(m_pColumns)
        m_pColumns->reFill(aVector);
    else
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

// XUnoTunnel
//------------------------------------------------------------------
sal_Int64 ODbaseIndex::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    return (rId.getLength() == 16 && 0 == memcmp(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
                ? reinterpret_cast< sal_Int64 >( this )
                : ODbaseIndex_BASE::getSomething(rId);
}
//------------------------------------------------------------------
ONDXPagePtr ODbaseIndex::getRoot()
{
    openIndexFile();
    if (!m_aRoot.Is())
    {
        m_nRootPage = m_aHeader.db_rootpage;
        m_nPageCount = m_aHeader.db_pagecount;
        m_aRoot = CreatePage(m_nRootPage,NULL,sal_True);
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
            m_pFileStream = OFileTable::createStream_simpleError(sFile, STREAM_READWRITE | STREAM_NOCREATE | STREAM_SHARE_DENYWRITE);
            if (!m_pFileStream)
                m_pFileStream = OFileTable::createStream_simpleError(sFile,STREAM_READ | STREAM_NOCREATE | STREAM_SHARE_DENYNONE);
            if(m_pFileStream)
            {
                m_pFileStream->SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
                m_pFileStream->SetBufferSize(PAGE_SIZE);
                (*m_pFileStream) >> *this;
            }
        }
        if(!m_pFileStream)
        {
            const ::rtl::OUString sError( m_pTable->getConnection()->getResources().getResourceStringWithSubstitution(
                STR_COULD_NOT_LOAD_FILE,
                "$filename$", sFile
             ) );
            ::dbtools::throwGenericSQLException( sError, *this );
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
sal_Bool ODbaseIndex::ConvertToKey(ONDXKey* rKey, sal_uInt32 nRec, const ORowSetValue& rValue)
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
        OSL_ASSERT(0);
        return sal_False;
    }
    return sal_True;
}

//------------------------------------------------------------------
sal_Bool ODbaseIndex::Find(sal_uInt32 nRec, const ORowSetValue& rValue)
{
    openIndexFile();
    OSL_ENSURE(m_pFileStream,"FileStream is not opened!");
    // Search a specific value in Index
    // If the Index is unique, the key doesn't matter
    ONDXKey aKey;
    return ConvertToKey(&aKey, nRec, rValue) && getRoot()->Find(aKey);
}

//------------------------------------------------------------------
sal_Bool ODbaseIndex::Insert(sal_uInt32 nRec, const ORowSetValue& rValue)
{
    openIndexFile();
    OSL_ENSURE(m_pFileStream,"FileStream is not opened!");
    ONDXKey aKey;

    // Does the value already exist
    // Use Find() always to determine the actual leaf
    if (!ConvertToKey(&aKey, nRec, rValue) || (getRoot()->Find(aKey) && isUnique()))
        return sal_False;

    ONDXNode aNewNode(aKey);

    // insert in the current leaf
    if (!m_aCurLeaf.Is())
        return sal_False;

    sal_Bool bResult = m_aCurLeaf->Insert(aNewNode);
    Release(bResult);

    return bResult;
}

//------------------------------------------------------------------
sal_Bool ODbaseIndex::Update(sal_uInt32 nRec, const ORowSetValue& rOldValue,
                         const ORowSetValue& rNewValue)
{
    openIndexFile();
    OSL_ENSURE(m_pFileStream,"FileStream is not opened!");
    ONDXKey aKey;
    if (!ConvertToKey(&aKey, nRec, rNewValue) || (isUnique() && getRoot()->Find(aKey)))
        return sal_False;
    else
        return Delete(nRec, rOldValue) && Insert(nRec,rNewValue);
}

//------------------------------------------------------------------
sal_Bool ODbaseIndex::Delete(sal_uInt32 nRec, const ORowSetValue& rValue)
{
    openIndexFile();
    OSL_ENSURE(m_pFileStream,"FileStream is not opened!");
    // Does the value already exist
    // Always use Find() to determine the actual leaf
    ONDXKey aKey;
    if (!ConvertToKey(&aKey, nRec, rValue) || !getRoot()->Find(aKey))
        return sal_False;

    ONDXNode aNewNode(aKey);

    // insert in the current leaf
    if (!m_aCurLeaf.Is())
        return sal_False;
#if OSL_DEBUG_LEVEL > 1
    m_aRoot->PrintPage();
#endif

    return m_aCurLeaf->Delete(m_nCurNode);
}
//------------------------------------------------------------------
void ODbaseIndex::Collect(ONDXPage* pPage)
{
    if (pPage)
        m_aCollector.push_back(pPage);
}
//------------------------------------------------------------------
void ODbaseIndex::Release(sal_Bool bSave)
{
    // Release the Index-recources
    m_bUseCollector = sal_False;

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
    for (sal_uIntPtr i = 0; i < m_aCollector.size(); i++)
        m_aCollector[i]->QueryDelete();

    m_aCollector.clear();

    // Header modified?
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
ONDXPage* ODbaseIndex::CreatePage(sal_uInt32 nPagePos, ONDXPage* pParent, sal_Bool bLoad)
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

//------------------------------------------------------------------
SvStream& connectivity::dbase::operator >> (SvStream &rStream, ODbaseIndex& rIndex)
{
    rStream.Seek(0);
    rStream.Read(&rIndex.m_aHeader,PAGE_SIZE);

    rIndex.m_nRootPage = rIndex.m_aHeader.db_rootpage;
    rIndex.m_nPageCount = rIndex.m_aHeader.db_pagecount;
    return rStream;
}
//------------------------------------------------------------------
SvStream& connectivity::dbase::operator << (SvStream &rStream, ODbaseIndex& rIndex)
{
    rStream.Seek(0);
    OSL_VERIFY_EQUALS( rStream.Write(&rIndex.m_aHeader,PAGE_SIZE), PAGE_SIZE, "Write not successful: Wrong header size for dbase index!");
    return rStream;
}
// -------------------------------------------------------------------------
::rtl::OUString ODbaseIndex::getCompletePath()
{
    ::rtl::OUString sDir = m_pTable->getConnection()->getURL();
    sDir += OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DELIMITER);
    sDir += m_Name;
    sDir += ::rtl::OUString(".ndx");
    return sDir;
}
//------------------------------------------------------------------
void ODbaseIndex::createINFEntry()
{
    // synchronize inf-file
    String sEntry = m_Name;
    sEntry += rtl::OUString(".ndx");

    ::rtl::OUString sCfgFile(m_pTable->getConnection()->getURL());
    sCfgFile += OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DELIMITER);
    sCfgFile += m_pTable->getName();
    sCfgFile += ::rtl::OUString(".inf");

    rtl::OUString sPhysicalPath;
    LocalFileHelper::ConvertURLToPhysicalName(sCfgFile,sPhysicalPath);

    Config aInfFile(sPhysicalPath);
    aInfFile.SetGroup(dBASE_III_GROUP);

    sal_uInt16 nSuffix = aInfFile.GetKeyCount();
    rtl::OString aNewEntry,aKeyName;
    sal_Bool bCase = isCaseSensitive();
    while (aNewEntry.isEmpty())
    {
        aNewEntry = rtl::OString(RTL_CONSTASCII_STRINGPARAM("NDX"));
        aNewEntry += rtl::OString::valueOf(static_cast<sal_Int32>(++nSuffix));
        for (sal_uInt16 i = 0; i < aInfFile.GetKeyCount(); i++)
        {
            aKeyName = aInfFile.GetKeyName(i);
            if (bCase ? aKeyName.equals(aNewEntry) : aKeyName.equalsIgnoreAsciiCase(aNewEntry))
            {
                aNewEntry = rtl::OString();
                break;
            }
        }
    }
    aInfFile.WriteKey(aNewEntry, rtl::OUStringToOString(sEntry, m_pTable->getConnection()->getTextEncoding()));
}
// -------------------------------------------------------------------------
sal_Bool ODbaseIndex::DropImpl()
{
    closeImpl();

    ::rtl::OUString sPath = getCompletePath();
    if(UCBContentHelper::Exists(sPath))
    {
        if(!UCBContentHelper::Kill(sPath))
            m_pTable->getConnection()->throwGenericSQLException(STR_COULD_NOT_DELETE_INDEX,*m_pTable);
    }

    // synchronize inf-file
    ::rtl::OUString sCfgFile(m_pTable->getConnection()->getURL());
    sCfgFile += OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DELIMITER);
    sCfgFile += m_pTable->getName();
    sCfgFile += ::rtl::OUString(".inf");

    rtl::OUString sPhysicalPath;
    OSL_VERIFY_RES( LocalFileHelper::ConvertURLToPhysicalName(sCfgFile, sPhysicalPath),
        "Can not convert Config Filename into Physical Name!");

    Config aInfFile(sPhysicalPath);
    aInfFile.SetGroup(dBASE_III_GROUP);
    sal_uInt16 nKeyCnt = aInfFile.GetKeyCount();
    rtl::OString aKeyName;
    String sEntry = m_Name;
    sEntry += rtl::OUString(".ndx");

    // delete entries from the inf file
    for (sal_uInt16 nKey = 0; nKey < nKeyCnt; nKey++)
    {
        // References the Key to an Index-file?
        aKeyName = aInfFile.GetKeyName( nKey );
        if (aKeyName.copy(0,3).equalsL(RTL_CONSTASCII_STRINGPARAM("NDX")))
        {
            if(sEntry == String(rtl::OStringToOUString(aInfFile.ReadKey(aKeyName),m_pTable->getConnection()->getTextEncoding())))
            {
                aInfFile.DeleteKey(aKeyName);
                break;
            }
        }
    }
    return sal_True;
}
// -------------------------------------------------------------------------
void ODbaseIndex::impl_killFileAndthrowError_throw(sal_uInt16 _nErrorId,const ::rtl::OUString& _sFile)
{
    closeImpl();
    if(UCBContentHelper::Exists(_sFile))
        UCBContentHelper::Kill(_sFile);
    m_pTable->getConnection()->throwGenericSQLException(_nErrorId,*this);
}
//------------------------------------------------------------------
sal_Bool ODbaseIndex::CreateImpl()
{
    // Create the Index
    const ::rtl::OUString sFile = getCompletePath();
    if(UCBContentHelper::Exists(sFile))
    {
        const ::rtl::OUString sError( m_pTable->getConnection()->getResources().getResourceStringWithSubstitution(
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
    m_pFileStream = OFileTable::createStream_simpleError(sFile,STREAM_READWRITE | STREAM_SHARE_DENYWRITE | STREAM_TRUNC);
    if (!m_pFileStream)
    {
        const ::rtl::OUString sError( m_pTable->getConnection()->getResources().getResourceStringWithSubstitution(
            STR_COULD_NOT_LOAD_FILE,
            "$filename$", sFile
         ) );
        ::dbtools::throwGenericSQLException( sError, *this );
    }

    m_pFileStream->SetNumberFormatInt(NUMBERFORMAT_INT_LITTLEENDIAN);
    m_pFileStream->SetBufferSize(PAGE_SIZE);

    // firstly the result must be sorted
    utl::SharedUNOComponent<XStatement> xStmt;
    utl::SharedUNOComponent<XResultSet> xSet;
    String aName;
    try
    {
        xStmt.set( m_pTable->getConnection()->createStatement(), UNO_SET_THROW);

        aName = getString(xCol->getFastPropertyValue(PROPERTY_ID_NAME));

        const String aQuote(m_pTable->getConnection()->getMetaData()->getIdentifierQuoteString());
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
    m_aHeader.db_keylen  = (m_aHeader.db_keytype) ? 8 : (sal_uInt16)getINT32(xTableCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION)));
    m_aHeader.db_keylen = (( m_aHeader.db_keylen - 1) / 4 + 1) * 4;
    m_aHeader.db_maxkeys = (PAGE_SIZE - 4) / (8 + m_aHeader.db_keylen);
    if ( m_aHeader.db_maxkeys < 3 )
    {
        impl_killFileAndthrowError_throw(STR_COULD_NOT_CREATE_INDEX_KEYSIZE,sFile);
    }

    m_pFileStream->SetStreamSize(PAGE_SIZE);

    rtl::OString aCol(rtl::OUStringToOString(aName, m_pTable->getConnection()->getTextEncoding()));
    strncpy(m_aHeader.db_name, aCol.getStr(), std::min<size_t>(sizeof(m_aHeader.db_name), aCol.getLength()));
    m_aHeader.db_unique  = m_IsUnique ? 1: 0;
    m_aHeader.db_keyrec  = m_aHeader.db_keylen + 8;

    // modifications of the header are detected by differences between
    // the HeaderInfo and nRootPage or nPageCount respectively
    m_nRootPage = 1;
    m_nPageCount = 2;

    m_aCurLeaf = m_aRoot = CreatePage(m_nRootPage);
    m_aRoot->SetModified(sal_True);

    m_bUseCollector = sal_True;

    sal_Int32 nRowsLeft = 0;
    Reference<XRow> xRow(xSet,UNO_QUERY);

    if(xSet->last())
    {
        Reference< XUnoTunnel> xTunnel(xSet,UNO_QUERY);
        ODbaseResultSet* pDbaseRes = NULL;
        if(xTunnel.is())
            pDbaseRes = reinterpret_cast< ODbaseResultSet* >( xTunnel->getSomething(ODbaseResultSet::getUnoTunnelImplementationId()) );
        OSL_ENSURE(pDbaseRes,"No dbase resultset found? What's going on here!");
        Reference<XRowLocate> xRowLocate(xSet,UNO_QUERY);
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
    return sal_True;
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void SAL_CALL ODbaseIndex::acquire() throw()
{
    ODbaseIndex_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL ODbaseIndex::release() throw()
{
    ODbaseIndex_BASE::release();
}
// -----------------------------------------------------------------------------



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
