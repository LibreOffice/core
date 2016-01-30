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

#include <ctype.h>
#include "flat/ETable.hxx"
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <svl/converter.hxx>
#include "flat/EConnection.hxx"
#include "flat/EColumns.hxx"
#include <osl/thread.h>
#include <svl/zforlist.hxx>
#include <rtl/math.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/numbers.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/string.hxx>
#include <comphelper/types.hxx>
#include "flat/EDriver.hxx"
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/NumberFormatter.hpp>
#include <com/sun/star/util/NumberFormatsSupplier.hpp>
#include <unotools/configmgr.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <connectivity/dbconversion.hxx>
#include "file/quotedstring.hxx"
#include <unotools/syslocale.hxx>

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::flat;
using namespace connectivity::file;
using namespace ::cppu;
using namespace utl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using std::vector;
using std::lower_bound;


void OFlatTable::fillColumns(const ::com::sun::star::lang::Locale& _aLocale)
{
    m_bNeedToReadLine = true; // we overwrite m_aCurrentLine, seek the stream, ...
    m_pFileStream->Seek(0);
    m_aCurrentLine = QuotedTokenizedString();
    bool bRead = true;

    const OFlatConnection* const pConnection = getFlatConnection();
    const bool bHasHeaderLine = pConnection->isHeaderLine();

    QuotedTokenizedString aHeaderLine;
    TRowPositionInFile rowPos(0, 0);
    sal_Int32 rowNum(0);
    if ( bHasHeaderLine )
    {
        bRead = readLine(&rowPos.second, &rowPos.first, true);
        if(bRead)
            aHeaderLine = m_aCurrentLine;
    }
    setRowPos(rowNum++, rowPos);

    // read first row
    QuotedTokenizedString aFirstLine;
    if(bRead)
    {
        bRead = readLine(&rowPos.second, &rowPos.first);
        if(bRead)
            setRowPos(rowNum++, rowPos);
    }

    if ( !bHasHeaderLine || !aHeaderLine.Len())
    {
        // use first non-empty row as headerline because we need the number of columns
        while(bRead && m_aCurrentLine.Len() == 0)
        {
            bRead = readLine(&rowPos.second, &rowPos.first);
            if(bRead)
                setRowPos(rowNum++, rowPos);
        }
        aHeaderLine = m_aCurrentLine;
    }
    // column count
    const sal_Int32 nFieldCount = aHeaderLine.GetTokenCount(m_cFieldDelimiter,m_cStringDelimiter);

    if(!m_aColumns.is())
        m_aColumns = new OSQLColumns();
    else
        m_aColumns->get().clear();

    m_aTypes.clear();
    m_aPrecisions.clear();
    m_aScales.clear();
    // reserve some space
    m_aColumns->get().reserve(nFieldCount+1);
    m_aTypes.assign(nFieldCount+1,DataType::SQLNULL);
    m_aPrecisions.assign(nFieldCount+1,-1);
    m_aScales.assign(nFieldCount+1,-1);

    const bool bCase = m_pConnection->getMetaData()->supportsMixedCaseQuotedIdentifiers();
    CharClass aCharClass( pConnection->getDriver()->getComponentContext(), LanguageTag( _aLocale));
    // read description
    const sal_Unicode cDecimalDelimiter  = pConnection->getDecimalDelimiter();
    const sal_Unicode cThousandDelimiter = pConnection->getThousandDelimiter();
    ::comphelper::UStringMixEqual aCase(bCase);
    vector<OUString> aColumnNames;
    vector<OUString> m_aTypeNames;
    m_aTypeNames.resize(nFieldCount);
    const sal_Int32 nMaxRowsToScan = pConnection->getMaxRowsToScan();
    sal_Int32 nRowCount = 0;

    do
    {
        sal_Int32 nStartPosHeaderLine = 0; // use for efficient way to get the tokens
        sal_Int32 nStartPosFirstLine = 0; // use for efficient way to get the tokens
        sal_Int32 nStartPosFirstLine2 = 0;
        for( sal_Int32 i = 0; i < nFieldCount; i++ )
        {
            if ( nRowCount == 0)
            {
                OUString aColumnName;
                if ( bHasHeaderLine )
                {
                    aColumnName = aHeaderLine.GetTokenSpecial(nStartPosHeaderLine,m_cFieldDelimiter,m_cStringDelimiter);
                }
                if ( aColumnName.isEmpty() )
                {
                    aColumnName = "C" + OUString::number(i+1);
                }
                aColumnNames.push_back(aColumnName);
            }
            if(bRead)
            {
                impl_fillColumnInfo_nothrow(m_aCurrentLine, nStartPosFirstLine, nStartPosFirstLine2,
                                            m_aTypes[i], m_aPrecisions[i], m_aScales[i], m_aTypeNames[i],
                                            cDecimalDelimiter, cThousandDelimiter, aCharClass);
            }
        }
        ++nRowCount;
        bRead = readLine(&rowPos.second, &rowPos.first);
        if(bRead)
            setRowPos(rowNum++, rowPos);
    }
    while(nRowCount < nMaxRowsToScan && bRead);

    for( sal_Int32 i = 0; i < nFieldCount; i++ )
    {
        // check if the columname already exists
        OUString aAlias(aColumnNames[i]);
        OSQLColumns::Vector::const_iterator aFind = connectivity::find(m_aColumns->get().begin(),m_aColumns->get().end(),aAlias,aCase);
        sal_Int32 nExprCnt = 0;
        while(aFind != m_aColumns->get().end())
        {
            aAlias = aColumnNames[i] + OUString::number(++nExprCnt);
            aFind = connectivity::find(m_aColumns->get().begin(),m_aColumns->get().end(),aAlias,aCase);
        }

        sdbcx::OColumn* pColumn = new sdbcx::OColumn(aAlias,m_aTypeNames[i],OUString(),OUString(),
                                                ColumnValue::NULLABLE,
                                                m_aPrecisions[i],
                                                m_aScales[i],
                                                m_aTypes[i],
                                                false,
                                                false,
                                                false,
                                                bCase,
                                                m_CatalogName, getSchema(), getName());
        Reference< XPropertySet> xCol = pColumn;
        m_aColumns->get().push_back(xCol);
    }

    m_pFileStream->Seek(m_aRowPosToFilePos[0].second);
}

void OFlatTable::impl_fillColumnInfo_nothrow(QuotedTokenizedString& aFirstLine, sal_Int32& nStartPosFirstLine, sal_Int32& nStartPosFirstLine2,
                                             sal_Int32& io_nType, sal_Int32& io_nPrecisions, sal_Int32& io_nScales, OUString& o_sTypeName,
                                             const sal_Unicode cDecimalDelimiter, const sal_Unicode cThousandDelimiter, const CharClass&  aCharClass)
{
    if ( io_nType != DataType::VARCHAR )
    {
        bool bNumeric = io_nType == DataType::SQLNULL || io_nType == DataType::DOUBLE || io_nType == DataType::DECIMAL || io_nType == DataType::INTEGER;
        sal_uLong  nIndex = 0;

        if ( bNumeric )
        {
            // first without fielddelimiter
            OUString aField = aFirstLine.GetTokenSpecial(nStartPosFirstLine,m_cFieldDelimiter);
            if (aField.isEmpty() ||
                (m_cStringDelimiter && m_cStringDelimiter == aField[0]))
            {
                bNumeric = false;
                if ( m_cStringDelimiter != '\0' )
                    aField = aFirstLine.GetTokenSpecial(nStartPosFirstLine2,m_cFieldDelimiter,m_cStringDelimiter);
                else
                    nStartPosFirstLine2 = nStartPosFirstLine;
            }
            else
            {
                OUString aField2;
                if ( m_cStringDelimiter != '\0' )
                    aField2 = aFirstLine.GetTokenSpecial(nStartPosFirstLine2,m_cFieldDelimiter,m_cStringDelimiter);
                else
                    aField2 = aField;

                if (aField2.isEmpty())
                {
                    bNumeric = false;
                }
                else
                {
                    bNumeric = true;
                    sal_Int32 nDot = 0;
                    sal_Int32 nDecimalDelCount = 0;
                    sal_Int32 nSpaceCount = 0;
                    for( sal_Int32 j = 0; j < aField2.getLength(); j++ )
                    {
                        const sal_Unicode c = aField2[j];
                        if ( j == nSpaceCount && m_cFieldDelimiter != 32 && c == 32 )
                        {
                            ++nSpaceCount;
                            continue;
                        }
                        // just digits, decimal- and thousands-delimiter?
                        if ( ( !cDecimalDelimiter  || c != cDecimalDelimiter )  &&
                             ( !cThousandDelimiter || c != cThousandDelimiter ) &&
                            !aCharClass.isDigit(aField2,j)                      &&
                            ( j != 0 || (c != '+' && c != '-' ) ) )
                        {
                            bNumeric = false;
                            break;
                        }
                        if (cDecimalDelimiter && c == cDecimalDelimiter)
                        {
                            io_nPrecisions = 15; // we have an decimal value
                            io_nScales = 2;
                            ++nDecimalDelCount;
                        } // if (cDecimalDelimiter && c == cDecimalDelimiter)
                        if ( c == '.' )
                            ++nDot;
                    }

                    if (nDecimalDelCount > 1 || nDot > 1 ) // if there is more than one dot it isn't a number
                        bNumeric = false;
                    if (bNumeric && cThousandDelimiter)
                    {
                        // Is the delimiter correct?
                        const OUString aValue = aField2.getToken(0,cDecimalDelimiter);
                        for( sal_Int32 j = aValue.getLength() - 4; j >= 0; j -= 4)
                        {
                            const sal_Unicode c = aValue[j];
                            // just digits, decimal- and thousands-delimiter?
                            if (c == cThousandDelimiter && j)
                                continue;
                            else
                            {
                                bNumeric = false;
                                break;
                            }
                        }
                    }

                    // now also check for a date field
                    if (!bNumeric)
                    {
                        try
                        {
                            nIndex = m_xNumberFormatter->detectNumberFormat(::com::sun::star::util::NumberFormat::ALL,aField2);
                        }
                        catch(Exception&)
                        {
                        }
                    }
                }
            }
        }
        else if ( io_nType == DataType::DATE || io_nType == DataType::TIMESTAMP || io_nType == DataType::TIME)
        {
            OUString aField = aFirstLine.GetTokenSpecial(nStartPosFirstLine,m_cFieldDelimiter);
            if (aField.isEmpty() ||
                (m_cStringDelimiter && m_cStringDelimiter == aField[0]))
            {
            }
            else
            {
                OUString aField2;
                if ( m_cStringDelimiter != '\0' )
                    aField2 = aFirstLine.GetTokenSpecial(nStartPosFirstLine2,m_cFieldDelimiter,m_cStringDelimiter);
                else
                    aField2 = aField;
                if (!aField2.isEmpty() )
                {
                    try
                    {
                        nIndex = m_xNumberFormatter->detectNumberFormat(::com::sun::star::util::NumberFormat::ALL,aField2);
                    }
                    catch(Exception&)
                    {
                    }
                }
            }
        }

        if (bNumeric)
        {
            if (cDecimalDelimiter)
            {
                if(io_nPrecisions)
                {
                    io_nType = DataType::DECIMAL;
                    o_sTypeName = "DECIMAL";
                }
                else
                {
                    io_nType = DataType::DOUBLE;
                    o_sTypeName = "DOUBLE";
                }
            }
            else
            {
                io_nType = DataType::INTEGER;
                io_nPrecisions = 0;
                io_nScales = 0;
            }
        }
        else
        {
            switch (comphelper::getNumberFormatType(m_xNumberFormatter,nIndex))
            {
                case css::util::NumberFormat::DATE:
                    io_nType = DataType::DATE;
                    o_sTypeName = "DATE";
                    break;
                case css::util::NumberFormat::DATETIME:
                    io_nType = DataType::TIMESTAMP;
                    o_sTypeName = "TIMESTAMP";
                    break;
                case css::util::NumberFormat::TIME:
                    io_nType = DataType::TIME;
                    o_sTypeName = "TIME";
                    break;
                default:
                    io_nType = DataType::VARCHAR;
                    io_nPrecisions = 0; // nyi: Data can be longer!
                    io_nScales = 0;
                    o_sTypeName = "VARCHAR";
            };
        }
    }
    else
    {
        OUString aField = aFirstLine.GetTokenSpecial(nStartPosFirstLine,m_cFieldDelimiter);
        if (aField.isEmpty() ||
                (m_cStringDelimiter && m_cStringDelimiter == aField[0]))
        {
            if ( m_cStringDelimiter != '\0' )
                aField = aFirstLine.GetTokenSpecial(nStartPosFirstLine2, m_cFieldDelimiter, m_cStringDelimiter);
            else
                nStartPosFirstLine2 = nStartPosFirstLine;
        }
        else
        {
            if ( m_cStringDelimiter != '\0' )
                aFirstLine.GetTokenSpecial(nStartPosFirstLine2, m_cFieldDelimiter, m_cStringDelimiter);
        }
    }
}

OFlatTable::OFlatTable(sdbcx::OCollection* _pTables,OFlatConnection* _pConnection,
                    const OUString& _Name,
                    const OUString& _Type,
                    const OUString& _Description ,
                    const OUString& _SchemaName,
                    const OUString& _CatalogName
                ) : OFlatTable_BASE(_pTables,_pConnection,_Name,
                                  _Type,
                                  _Description,
                                  _SchemaName,
                                  _CatalogName)
    ,m_nRowPos(0)
    ,m_nMaxRowCount(0)
    ,m_cStringDelimiter(_pConnection->getStringDelimiter())
    ,m_cFieldDelimiter(_pConnection->getFieldDelimiter())
    ,m_bNeedToReadLine(false)
{

}

void OFlatTable::construct()
{
    SvtSysLocale aLocale;
    ::com::sun::star::lang::Locale aAppLocale(aLocale.GetLanguageTag().getLocale());

    Reference< XNumberFormatsSupplier > xSupplier = NumberFormatsSupplier::createWithLocale( m_pConnection->getDriver()->getComponentContext(), aAppLocale );
    m_xNumberFormatter.set( NumberFormatter::create( m_pConnection->getDriver()->getComponentContext()), UNO_QUERY_THROW);
    m_xNumberFormatter->attachNumberFormatsSupplier(xSupplier);
    Reference<XPropertySet> xProp(xSupplier->getNumberFormatSettings(),UNO_QUERY);
    xProp->getPropertyValue("NullDate") >>= m_aNullDate;

    INetURLObject aURL;
    aURL.SetURL(getEntry());

    if(aURL.getExtension() != m_pConnection->getExtension())
        aURL.setExtension(m_pConnection->getExtension());

    OUString aFileName = aURL.GetMainURL(INetURLObject::NO_DECODE);

    m_pFileStream = createStream_simpleError( aFileName, STREAM_READWRITE | StreamMode::NOCREATE | StreamMode::SHARE_DENYWRITE);

    if(!m_pFileStream)
        m_pFileStream = createStream_simpleError( aFileName, StreamMode::READ | StreamMode::NOCREATE | StreamMode::SHARE_DENYNONE);

    if(m_pFileStream)
    {
        sal_uInt64 const nSize = m_pFileStream->remainingSize();

        // Buffersize is dependent on the file-size
        m_pFileStream->SetBufferSize(nSize > 1000000 ? 32768 :
                                    nSize > 100000  ? 16384 :
                                    nSize > 10000   ? 4096  : 1024);

        fillColumns(aAppLocale);

        refreshColumns();
    }
}

OUString OFlatTable::getEntry()
{
    OUString sURL;
    try
    {
        Reference< XResultSet > xDir = m_pConnection->getDir()->getStaticResultSet();
        Reference< XRow> xRow(xDir,UNO_QUERY);
        OUString sName;
        OUString sExt;

        INetURLObject aURL;
        xDir->beforeFirst();
        static const char s_sSeparator[] = "/";
        while(xDir->next())
        {
            sName = xRow->getString(1);
            aURL.SetSmartProtocol(INetProtocol::File);
            OUString sUrl = m_pConnection->getURL() +  s_sSeparator + sName;
            aURL.SetSmartURL( sUrl );

            // cut the extension
            sExt = aURL.getExtension();

            // name and extension have to coincide
            if ( m_pConnection->matchesExtension( sExt ) )
            {
                if ( !sExt.isEmpty() )
                    sName = sName.replaceAt(sName.getLength() - (sExt.getLength() + 1), sExt.getLength()+1, OUString());
                if ( sName == m_Name )
                {
                    Reference< XContentAccess > xContentAccess( xDir, UNO_QUERY );
                    sURL = xContentAccess->queryContentIdentifierString();
                    break;
                }
            }
        }
        xDir->beforeFirst(); // move back to before first record
    }
    catch(const Exception&)
    {
        OSL_ASSERT(false);
    }
    return sURL;
}

void OFlatTable::refreshColumns()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    TStringVector aVector;
    aVector.reserve(m_aColumns->get().size());

    for(OSQLColumns::Vector::const_iterator aIter = m_aColumns->get().begin();aIter != m_aColumns->get().end();++aIter)
        aVector.push_back(Reference< XNamed>(*aIter,UNO_QUERY)->getName());

    if(m_pColumns)
        m_pColumns->reFill(aVector);
    else
        m_pColumns  = new OFlatColumns(this,m_aMutex,aVector);
}


void SAL_CALL OFlatTable::disposing()
{
    OFileTable::disposing();
    ::osl::MutexGuard aGuard(m_aMutex);
    m_aColumns = nullptr;
}

Sequence< Type > SAL_CALL OFlatTable::getTypes(  ) throw(RuntimeException, std::exception)
{
    Sequence< Type > aTypes = OTable_TYPEDEF::getTypes();
    vector<Type> aOwnTypes;
    aOwnTypes.reserve(aTypes.getLength());
    const Type* pBegin = aTypes.getConstArray();
    const Type* pEnd = pBegin + aTypes.getLength();
    for(;pBegin != pEnd;++pBegin)
    {
        if(!(*pBegin == cppu::UnoType<XKeysSupplier>::get()||
            *pBegin == cppu::UnoType<XRename>::get()||
            *pBegin == cppu::UnoType<XIndexesSupplier>::get()||
            *pBegin == cppu::UnoType<XAlterTable>::get()||
            *pBegin == cppu::UnoType<XDataDescriptorFactory>::get()))
        {
            aOwnTypes.push_back(*pBegin);
        }
    }
    return Sequence< Type >(aOwnTypes.data(), aOwnTypes.size());
}


Any SAL_CALL OFlatTable::queryInterface( const Type & rType ) throw(RuntimeException, std::exception)
{
    if( rType == cppu::UnoType<XKeysSupplier>::get()||
        rType == cppu::UnoType<XIndexesSupplier>::get()||
        rType == cppu::UnoType<XRename>::get()||
        rType == cppu::UnoType<XAlterTable>::get()||
        rType == cppu::UnoType<XDataDescriptorFactory>::get())
        return Any();

    Any aRet = OTable_TYPEDEF::queryInterface(rType);
    return aRet.hasValue() ? aRet : ::cppu::queryInterface(rType,static_cast< ::com::sun::star::lang::XUnoTunnel*> (this));
}


Sequence< sal_Int8 > OFlatTable::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId * pId = nullptr;
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

sal_Int64 OFlatTable::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException, std::exception)
{
    return (rId.getLength() == 16 && 0 == memcmp(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
                ? reinterpret_cast< sal_Int64 >( this )
                : OFlatTable_BASE::getSomething(rId);
}

bool OFlatTable::fetchRow(OValueRefRow& _rRow, const OSQLColumns & _rCols, bool bIsTable, bool bRetrieveData)
{
    *(_rRow->get())[0] = m_nFilePos;

    if (!bRetrieveData)
        return true;

    bool result = false;
    if ( m_bNeedToReadLine )
    {
        m_pFileStream->Seek(m_nFilePos);
        TRowPositionInFile rowPos(0, 0);
        if(readLine(&rowPos.second, &rowPos.first))
        {
            setRowPos(m_nRowPos, rowPos);
            m_bNeedToReadLine = false;
            result = true;
        }
        // else let run through so that we set _rRow to all NULL
    }

    const OFlatConnection * const pConnection = getFlatConnection();
    const sal_Unicode cDecimalDelimiter = pConnection->getDecimalDelimiter();
    const sal_Unicode cThousandDelimiter = pConnection->getThousandDelimiter();
    // Fields:
    sal_Int32 nStartPos = 0;
    OSQLColumns::Vector::const_iterator aIter = _rCols.get().begin();
    OSQLColumns::Vector::const_iterator aEnd = _rCols.get().end();
    const OValueRefVector::Vector::size_type nCount = _rRow->get().size();
    for (OValueRefVector::Vector::size_type i = 1;
         aIter != aEnd && i < nCount;
         ++aIter, i++)
    {
        OUString aStr = m_aCurrentLine.GetTokenSpecial(nStartPos,m_cFieldDelimiter,m_cStringDelimiter);

        if (aStr.isEmpty())
        {
            (_rRow->get())[i]->setNull();
        }
        else
        {
            sal_Int32 nType = 0;
            if(bIsTable)
            {
                nType   = m_aTypes[i-1];
            }
            else
            {
                Reference< XPropertySet> xColumn = *aIter;
                xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE))       >>= nType;
            }
            switch(nType)
            {
                case DataType::TIMESTAMP:
                case DataType::DATE:
                case DataType::TIME:
                {
                    try
                    {
                        double nRes = m_xNumberFormatter->convertStringToNumber(::com::sun::star::util::NumberFormat::ALL,aStr);

                        switch(nType)
                        {
                            case DataType::DATE:
                                *(_rRow->get())[i] = ::dbtools::DBTypeConversion::toDouble(::dbtools::DBTypeConversion::toDate(nRes,m_aNullDate));
                                break;
                            case DataType::TIMESTAMP:
                                *(_rRow->get())[i] = ::dbtools::DBTypeConversion::toDouble(::dbtools::DBTypeConversion::toDateTime(nRes,m_aNullDate));
                                break;
                            default:
                                *(_rRow->get())[i] = ::dbtools::DBTypeConversion::toDouble(::dbtools::DBTypeConversion::toTime(nRes));
                        }
                    }
                    catch(Exception&)
                    {
                        (_rRow->get())[i]->setNull();
                    }
                }   break;
                case DataType::DOUBLE:
                case DataType::INTEGER:
                case DataType::DECIMAL:
                case DataType::NUMERIC:
                {

                    OUString aStrConverted;
                    if ( DataType::INTEGER != nType )
                    {
                        OSL_ENSURE((cDecimalDelimiter && nType != DataType::INTEGER) ||
                                   (!cDecimalDelimiter && nType == DataType::INTEGER),
                                   "FalscherTyp");

                        OUStringBuffer aBuf(aStr.getLength());
                        // convert to Standard-Notation (DecimalPOINT without thousands-comma):
                        for (sal_Int32 j = 0; j < aStr.getLength(); ++j)
                        {
                            const sal_Unicode cChar = aStr[j];
                            if (cDecimalDelimiter && cChar == cDecimalDelimiter)
                                aBuf.append('.');
                            else if ( cChar == '.' ) // special case, if decimal separator isn't '.' we have to put the string after it
                                continue;
                            else if (cThousandDelimiter && cChar == cThousandDelimiter)
                            {
                                // leave out
                            }
                            else
                                aBuf.append(cChar);
                        } // for (j = 0; j < aStr.getLength(); ++j)
                        aStrConverted = aBuf.makeStringAndClear();
                    } // if ( DataType::INTEGER != nType )
                    else
                    {
                        if ( cThousandDelimiter )
                            aStrConverted = comphelper::string::remove(aStr, cThousandDelimiter);
                        else
                            aStrConverted = aStr;
                    }
                    const double nVal = ::rtl::math::stringToDouble(aStrConverted,'.',',');

                    // #99178# OJ
                    if ( DataType::DECIMAL == nType || DataType::NUMERIC == nType )
                        *(_rRow->get())[i] = OUString::number(nVal);
                    else
                        *(_rRow->get())[i] = nVal;
                } break;

                default:
                {
                    // Copy Value as String in Row-Variable
                    *(_rRow->get())[i] = ORowSetValue(aStr);
                }
                break;
            } // switch(nType)
            (_rRow->get())[i]->setTypeKind(nType);
        }
    }
    return result;
}


void OFlatTable::refreshHeader()
{
    SAL_INFO( "connectivity.drivers", "flat lionel@mamane.lu OFlatTable::refreshHeader" );
}


namespace
{
    template< typename Tp, typename Te> struct RangeBefore
    {
        bool operator() (const Tp &p, const Te &e)
        {
            assert(p.first <= p.second);
            return p.second <= e;
        }
    };
}

bool OFlatTable::seekRow(IResultSetHelper::Movement eCursorPosition, sal_Int32 nOffset, sal_Int32& nCurPos)
{
    OSL_ENSURE(m_pFileStream,"OFlatTable::seekRow: FileStream is NULL!");


    switch(eCursorPosition)
    {
        case IResultSetHelper::FIRST:
            m_nRowPos = 0;
            // run through
        case IResultSetHelper::NEXT:
            {
                assert(m_nRowPos >= 0);
                if(m_nMaxRowCount != 0 && m_nRowPos > m_nMaxRowCount)
                    return false;
                ++m_nRowPos;
                if(m_aRowPosToFilePos.size() > static_cast< vector< TRowPositionInFile >::size_type >(m_nRowPos))
                {
                    m_bNeedToReadLine = true;
                    m_nFilePos  = m_aRowPosToFilePos[m_nRowPos].first;
                    nCurPos     = m_aRowPosToFilePos[m_nRowPos].second;
                }
                else
                {
                    assert(m_aRowPosToFilePos.size() == static_cast< vector< TRowPositionInFile >::size_type >(m_nRowPos));
                    const TRowPositionInFile &lastRowPos(m_aRowPosToFilePos.back());
                    // Our ResultSet is allowed to disagree with us only
                    // on the position of the first line
                    // (because of the special case of the header...)
                    assert(m_nRowPos == 1 || nCurPos == lastRowPos.second);

                    m_nFilePos = lastRowPos.second;
                    m_pFileStream->Seek(m_nFilePos);

                    TRowPositionInFile newRowPos;
                    if(!readLine(&newRowPos.second, &newRowPos.first))
                    {
                        m_nMaxRowCount = m_nRowPos - 1;
                        return false;
                    }

                    nCurPos = newRowPos.second;
                    setRowPos(m_nRowPos, newRowPos);
                }
            }

            break;
        case IResultSetHelper::PRIOR:
            assert(m_nRowPos >= 0);

            if(m_nRowPos == 0)
                return false;

            --m_nRowPos;
            {
                assert (m_nRowPos >= 0);
                assert(m_aRowPosToFilePos.size() >= static_cast< vector< TRowPositionInFile >::size_type >(m_nRowPos));
                const TRowPositionInFile &aPositions(m_aRowPosToFilePos[m_nRowPos]);
                m_nFilePos = aPositions.first;
                nCurPos = aPositions.second;
                m_bNeedToReadLine = true;
            }

            break;
        case IResultSetHelper::LAST:
            if (m_nMaxRowCount == 0)
            {
                while(seekRow(IResultSetHelper::NEXT, 1, nCurPos)) ; // run through after last row
            }
            // m_nMaxRowCount can still be zero, but now it means there a genuinely zero rows in the table
            return seekRow(IResultSetHelper::ABSOLUTE1, m_nMaxRowCount, nCurPos);
            break;
        case IResultSetHelper::RELATIVE1:
            {
                const sal_Int32 nNewRowPos = m_nRowPos + nOffset;
                if (nNewRowPos < 0)
                    return false;
                // ABSOLUTE will take care of case nNewRowPos > nMaxRowCount
                return seekRow(IResultSetHelper::ABSOLUTE1, nNewRowPos, nCurPos);
            }
        case IResultSetHelper::ABSOLUTE1:
            {
                if(nOffset < 0)
                {
                    if (m_nMaxRowCount == 0)
                    {
                        if (!seekRow(IResultSetHelper::LAST, 0, nCurPos))
                            return false;
                    }
                    // m_nMaxRowCount can still be zero, but now it means there a genuinely zero rows in the table
                    nOffset = m_nMaxRowCount + nOffset;
                }
                if(nOffset < 0)
                {
                    seekRow(IResultSetHelper::ABSOLUTE1, 0, nCurPos);
                    return false;
                }
                if(m_nMaxRowCount && nOffset > m_nMaxRowCount)
                {
                    m_nRowPos = m_nMaxRowCount + 1;
                    const TRowPositionInFile &lastRowPos(m_aRowPosToFilePos.back());
                    m_nFilePos = lastRowPos.second;
                    nCurPos = lastRowPos.second;
                    return false;
                }

                assert(m_nRowPos >=0);
                assert(m_aRowPosToFilePos.size() > static_cast< vector< TRowPositionInFile >::size_type >(m_nRowPos));
                assert(nOffset >= 0);
                if(m_aRowPosToFilePos.size() > static_cast< vector< TRowPositionInFile >::size_type >(nOffset))
                {
                    m_nFilePos  = m_aRowPosToFilePos[nOffset].first;
                    nCurPos     = m_aRowPosToFilePos[nOffset].second;
                    m_nRowPos   = nOffset;
                    m_bNeedToReadLine = true;
                }
                else
                {
                    assert(m_nRowPos < nOffset);
                    while(m_nRowPos < nOffset)
                    {
                        if(!seekRow(IResultSetHelper::NEXT, 1, nCurPos))
                            return false;
                    }
                    assert(m_nRowPos == nOffset);
                }
            }

            break;
        case IResultSetHelper::BOOKMARK:
            {
                vector< TRowPositionInFile >::const_iterator aFind = lower_bound(m_aRowPosToFilePos.begin(),
                                                                                 m_aRowPosToFilePos.end(),
                                                                                 nOffset,
                                                                                 RangeBefore< TRowPositionInFile, sal_Int32 >());

                if(aFind == m_aRowPosToFilePos.end() || aFind->first != nOffset)
                    //invalid bookmark
                    return false;

                m_bNeedToReadLine = true;
                m_nFilePos  = aFind->first;
                nCurPos     = aFind->second;
                m_nRowPos = aFind - m_aRowPosToFilePos.begin();
                break;
            }
    }

    return true;
}


bool OFlatTable::readLine(sal_Int32 * const pEndPos, sal_Int32 * const pStartPos, const bool nonEmpty)
{
    const rtl_TextEncoding nEncoding = m_pConnection->getTextEncoding();
    m_aCurrentLine = QuotedTokenizedString();
    do
    {
        if (pStartPos)
            *pStartPos = (sal_Int32)m_pFileStream->Tell();
        m_pFileStream->ReadByteStringLine(m_aCurrentLine, nEncoding);
        if (m_pFileStream->IsEof())
            return false;

        QuotedTokenizedString sLine = m_aCurrentLine; // check if the string continues on next line
        while( (comphelper::string::getTokenCount(sLine.GetString(), m_cStringDelimiter) % 2) != 1 )
        {
            m_pFileStream->ReadByteStringLine(sLine,nEncoding);
            if ( !m_pFileStream->IsEof() )
            {
                OUString aStr = m_aCurrentLine.GetString() + "\n" + sLine.GetString();
                m_aCurrentLine.SetString(aStr);
                sLine = m_aCurrentLine;
            }
            else
                break;
        }
    }
    while(nonEmpty && m_aCurrentLine.Len() == 0);

    if(pEndPos)
        *pEndPos = (sal_Int32)m_pFileStream->Tell();
    return true;
}


void OFlatTable::setRowPos(const vector<TRowPositionInFile>::size_type rowNum, const TRowPositionInFile &rowPos)
{
    assert(m_aRowPosToFilePos.size() >= rowNum);
    if(m_aRowPosToFilePos.size() == rowNum)
        m_aRowPosToFilePos.push_back(rowPos);
    else
    {
        SAL_WARN_IF(m_aRowPosToFilePos[rowNum] != rowPos,
                    "connectivity.flat",
                    "Setting position for row " << rowNum << " to (" << rowPos.first << ", " << rowPos.second << "), "
                    "but already had different position (" << m_aRowPosToFilePos[rowNum].first << ", " << m_aRowPosToFilePos[rowNum].second << ")");
        m_aRowPosToFilePos[rowNum] = rowPos;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
