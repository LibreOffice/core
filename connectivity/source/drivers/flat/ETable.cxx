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
#include <stdio.h>      //sprintf
#include <comphelper/extract.hxx>
#include <comphelper/numbers.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/string.hxx>
#include <comphelper/types.hxx>
#include "flat/EDriver.hxx"
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/NumberFormatter.hpp>
#include <unotools/configmgr.hxx>
#include <i18npool/mslangid.hxx>
#include "connectivity/dbconversion.hxx"
#include "file/quotedstring.hxx"
#include <unotools/syslocale.hxx>
#include <rtl/logfile.hxx>

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

// -------------------------------------------------------------------------
void OFlatTable::fillColumns(const ::com::sun::star::lang::Locale& _aLocale)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "flat", "Ocke.Janssen@sun.com", "OFlatTable::fillColumns" );
    sal_Bool bRead = sal_True;

    QuotedTokenizedString aHeaderLine;
    OFlatConnection* pConnection = (OFlatConnection*)m_pConnection;
    const rtl_TextEncoding nEncoding = m_pConnection->getTextEncoding();
    const sal_Bool bHasHeaderLine = pConnection->isHeaderLine();
    if ( bHasHeaderLine )
    {
        while(bRead && !aHeaderLine.Len())
        {
            bRead = m_pFileStream->ReadByteStringLine(aHeaderLine,nEncoding);
        }
        m_nStartRowFilePos = m_pFileStream->Tell();
    }

    // read first row
    QuotedTokenizedString aFirstLine;
    bRead = m_pFileStream->ReadByteStringLine(aFirstLine,nEncoding);

    if ( !bHasHeaderLine || !aHeaderLine.Len())
    {
        while(bRead && !aFirstLine.Len())
        {
            bRead = m_pFileStream->ReadByteStringLine(aFirstLine,nEncoding);
        }
        // use first row as headerline because we need the number of columns
        aHeaderLine = aFirstLine;
    }
    // column count
    const xub_StrLen nFieldCount = aHeaderLine.GetTokenCount(m_cFieldDelimiter,m_cStringDelimiter);

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

    const sal_Bool bCase = m_pConnection->getMetaData()->supportsMixedCaseQuotedIdentifiers();
    CharClass aCharClass(pConnection->getDriver()->getFactory(),_aLocale);
    // read description
    const sal_Unicode cDecimalDelimiter  = pConnection->getDecimalDelimiter();
    const sal_Unicode cThousandDelimiter = pConnection->getThousandDelimiter();
    String aColumnName;
    ::comphelper::UStringMixEqual aCase(bCase);
    ::std::vector<String> aColumnNames,m_aTypeNames;
    m_aTypeNames.resize(nFieldCount);
    const sal_Int32 nMaxRowsToScan = pConnection->getMaxRowsToScan();
    sal_Int32 nRowCount = 0;
    do
    {
        xub_StrLen nStartPosHeaderLine = 0; // use for eficient way to get the tokens
        xub_StrLen nStartPosFirstLine = 0; // use for eficient way to get the tokens
        xub_StrLen nStartPosFirstLine2 = 0;
        for (xub_StrLen i = 0; i < nFieldCount; i++)
        {
            if ( nRowCount == 0)
            {
                if ( bHasHeaderLine )
                {
                    aHeaderLine.GetTokenSpecial(aColumnName,nStartPosHeaderLine,m_cFieldDelimiter,m_cStringDelimiter);
                    if ( !aColumnName.Len() )
                    {
                        aColumnName = 'C';
                        aColumnName += String::CreateFromInt32(i+1);
                    }
                }
                else
                {
                    // no column name so ...
                    aColumnName = 'C';
                    aColumnName += String::CreateFromInt32(i+1);
                }
                aColumnNames.push_back(aColumnName);
            }
            impl_fillColumnInfo_nothrow(aFirstLine,nStartPosFirstLine,nStartPosFirstLine2,m_aTypes[i],m_aPrecisions[i],m_aScales[i],m_aTypeNames[i],cDecimalDelimiter,cThousandDelimiter,aCharClass);
        }
        ++nRowCount;
    }
    while(nRowCount < nMaxRowsToScan && m_pFileStream->ReadByteStringLine(aFirstLine,nEncoding) && !m_pFileStream->IsEof());

    for (xub_StrLen i = 0; i < nFieldCount; i++)
    {
        // check if the columname already exists
        String aAlias(aColumnNames[i]);
        OSQLColumns::Vector::const_iterator aFind = connectivity::find(m_aColumns->get().begin(),m_aColumns->get().end(),aAlias,aCase);
        sal_Int32 nExprCnt = 0;
        while(aFind != m_aColumns->get().end())
        {
            (aAlias = aColumnNames[i]) += String::CreateFromInt32(++nExprCnt);
            aFind = connectivity::find(m_aColumns->get().begin(),m_aColumns->get().end(),aAlias,aCase);
        }

        sdbcx::OColumn* pColumn = new sdbcx::OColumn(aAlias,m_aTypeNames[i],::rtl::OUString(),::rtl::OUString(),
                                                ColumnValue::NULLABLE,
                                                m_aPrecisions[i],
                                                m_aScales[i],
                                                m_aTypes[i],
                                                sal_False,
                                                sal_False,
                                                sal_False,
                                                bCase,
                                                m_CatalogName, getSchema(), getName());
        Reference< XPropertySet> xCol = pColumn;
        m_aColumns->get().push_back(xCol);
    }
    m_pFileStream->Seek(m_nStartRowFilePos);
}
void OFlatTable::impl_fillColumnInfo_nothrow(QuotedTokenizedString& aFirstLine,xub_StrLen& nStartPosFirstLine,xub_StrLen& nStartPosFirstLine2
                                             ,sal_Int32& io_nType,sal_Int32& io_nPrecisions,sal_Int32& io_nScales,String& o_sTypeName
                                             ,const sal_Unicode cDecimalDelimiter,const sal_Unicode cThousandDelimiter,const CharClass&  aCharClass)
{
    if ( io_nType != DataType::VARCHAR )
    {
        sal_Bool bNumeric = io_nType == DataType::SQLNULL || io_nType == DataType::DOUBLE || io_nType == DataType::DECIMAL || io_nType == DataType::INTEGER;
        sal_uLong  nIndex = 0;

        if ( bNumeric )
        {
            // first without fielddelimiter
            String aField;
            aFirstLine.GetTokenSpecial(aField,nStartPosFirstLine,m_cFieldDelimiter,'\0');
            if (aField.Len() == 0 ||
                (m_cStringDelimiter && m_cStringDelimiter == aField.GetChar(0)))
            {
                bNumeric = sal_False;
                if ( m_cStringDelimiter != '\0' )
                    aFirstLine.GetTokenSpecial(aField,nStartPosFirstLine2,m_cFieldDelimiter,m_cStringDelimiter);
                else
                    nStartPosFirstLine2 = nStartPosFirstLine;
            }
            else
            {
                String aField2;
                if ( m_cStringDelimiter != '\0' )
                    aFirstLine.GetTokenSpecial(aField2,nStartPosFirstLine2,m_cFieldDelimiter,m_cStringDelimiter);
                else
                    aField2 = aField;

                if (aField2.Len() == 0)
                {
                    bNumeric = sal_False;
                }
                else
                {
                    bNumeric = sal_True;
                    xub_StrLen nDot = 0;
                    xub_StrLen nDecimalDelCount = 0;
                    xub_StrLen nSpaceCount = 0;
                    for (xub_StrLen j = 0; j < aField2.Len(); j++)
                    {
                        const sal_Unicode c = aField2.GetChar(j);
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
                            bNumeric = sal_False;
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
                        bNumeric = sal_False;
                    if (bNumeric && cThousandDelimiter)
                    {
                        // Is the delimiter correct?
                        const String aValue = aField2.GetToken(0,cDecimalDelimiter);
                        for (sal_Int32 j = aValue.Len() - 4; j >= 0; j -= 4)
                        {
                            const sal_Unicode c = aValue.GetChar(static_cast<sal_uInt16>(j));
                            // just digits, decimal- and thousands-delimiter?
                            if (c == cThousandDelimiter && j)
                                continue;
                            else
                            {
                                bNumeric = sal_False;
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
            String aField;
            aFirstLine.GetTokenSpecial(aField,nStartPosFirstLine,m_cFieldDelimiter,'\0');
            if (aField.Len() == 0 ||
                (m_cStringDelimiter && m_cStringDelimiter == aField.GetChar(0)))
            {
            }
            else
            {
                String aField2;
                if ( m_cStringDelimiter != '\0' )
                    aFirstLine.GetTokenSpecial(aField2,nStartPosFirstLine2,m_cFieldDelimiter,m_cStringDelimiter);
                else
                    aField2 = aField;
                if (aField2.Len() )
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

        sal_Int32 nFlags = 0;
        if (bNumeric)
        {
            if (cDecimalDelimiter)
            {
                if(io_nPrecisions)
                {
                    io_nType = DataType::DECIMAL;
                    static const ::rtl::OUString s_sDECIMAL("DECIMAL");
                    o_sTypeName = s_sDECIMAL;
                }
                else
                {
                    io_nType = DataType::DOUBLE;
                    static const ::rtl::OUString s_sDOUBLE("DOUBLE");
                    o_sTypeName = s_sDOUBLE;
                }
            }
            else
            {
                io_nType = DataType::INTEGER;
                io_nPrecisions = 0;
                io_nScales = 0;
            }
            nFlags = ColumnSearch::BASIC;
        }
        else
        {
            switch (comphelper::getNumberFormatType(m_xNumberFormatter,nIndex))
            {
                case NUMBERFORMAT_DATE:
                    io_nType = DataType::DATE;
                    {
                        static const ::rtl::OUString s_sDATE("DATE");
                        o_sTypeName = s_sDATE;
                    }
                    break;
                case NUMBERFORMAT_DATETIME:
                    io_nType = DataType::TIMESTAMP;
                    {
                        static const ::rtl::OUString s_sTIMESTAMP("TIMESTAMP");
                        o_sTypeName = s_sTIMESTAMP;
                    }
                    break;
                case NUMBERFORMAT_TIME:
                    io_nType = DataType::TIME;
                    {
                        static const ::rtl::OUString s_sTIME("TIME");
                        o_sTypeName = s_sTIME;
                    }
                    break;
                default:
                    io_nType = DataType::VARCHAR;
                    io_nPrecisions = 0; // nyi: Data can be longer!
                    io_nScales = 0;
                    {
                        static const ::rtl::OUString s_sVARCHAR("VARCHAR");
                        o_sTypeName = s_sVARCHAR;
                    }
            };
            nFlags |= ColumnSearch::CHAR;
        }
    }
    else
    {
        String aField;
        aFirstLine.GetTokenSpecial(aField,nStartPosFirstLine,m_cFieldDelimiter,'\0');
        if (aField.Len() == 0 ||
                (m_cStringDelimiter && m_cStringDelimiter == aField.GetChar(0)))
        {
            if ( m_cStringDelimiter != '\0' )
                aFirstLine.GetTokenSpecial(aField,nStartPosFirstLine2,m_cFieldDelimiter,m_cStringDelimiter);
            else
                nStartPosFirstLine2 = nStartPosFirstLine;
        }
        else
        {
            String aField2;
            if ( m_cStringDelimiter != '\0' )
                aFirstLine.GetTokenSpecial(aField2,nStartPosFirstLine2,m_cFieldDelimiter,m_cStringDelimiter);
        }
    }
}
// -------------------------------------------------------------------------
OFlatTable::OFlatTable(sdbcx::OCollection* _pTables,OFlatConnection* _pConnection,
                    const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _Type,
                    const ::rtl::OUString& _Description ,
                    const ::rtl::OUString& _SchemaName,
                    const ::rtl::OUString& _CatalogName
                ) : OFlatTable_BASE(_pTables,_pConnection,_Name,
                                  _Type,
                                  _Description,
                                  _SchemaName,
                                  _CatalogName)
    ,m_nStartRowFilePos(0)
    ,m_nRowPos(0)
    ,m_nMaxRowCount(0)
    ,m_cStringDelimiter(_pConnection->getStringDelimiter())
    ,m_cFieldDelimiter(_pConnection->getFieldDelimiter())
    ,m_bNeedToReadLine(false)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "flat", "Ocke.Janssen@sun.com", "OFlatTable::OFlatTable" );

}
// -----------------------------------------------------------------------------
void OFlatTable::construct()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "flat", "Ocke.Janssen@sun.com", "OFlatTable::construct" );
    SvtSysLocale aLocale;
    ::com::sun::star::lang::Locale aAppLocale(aLocale.GetLocaleDataPtr()->getLocale());
    Sequence< ::com::sun::star::uno::Any > aArg(1);
    aArg[0] <<= aAppLocale;

    Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xSupplier(m_pConnection->getDriver()->getFactory()->createInstanceWithArguments(::rtl::OUString("com.sun.star.util.NumberFormatsSupplier"),aArg),UNO_QUERY);
    m_xNumberFormatter = Reference< ::com::sun::star::util::XNumberFormatter >(
          ::com::sun::star::util::NumberFormatter::create(
             comphelper::getComponentContext(m_pConnection->getDriver()->getFactory())),
          UNO_QUERY_THROW);
    m_xNumberFormatter->attachNumberFormatsSupplier(xSupplier);
    Reference<XPropertySet> xProp(xSupplier->getNumberFormatSettings(),UNO_QUERY);
    xProp->getPropertyValue(::rtl::OUString("NullDate")) >>= m_aNullDate;

    INetURLObject aURL;
    aURL.SetURL(getEntry());

    if(aURL.getExtension() != rtl::OUString(m_pConnection->getExtension()))
        aURL.setExtension(m_pConnection->getExtension());

    String aFileName = aURL.GetMainURL(INetURLObject::NO_DECODE);

    m_pFileStream = createStream_simpleError( aFileName,STREAM_READWRITE | STREAM_NOCREATE | STREAM_SHARE_DENYWRITE);

    if(!m_pFileStream)
        m_pFileStream = createStream_simpleError( aFileName,STREAM_READ | STREAM_NOCREATE | STREAM_SHARE_DENYNONE);

    if(m_pFileStream)
    {
        m_pFileStream->Seek(STREAM_SEEK_TO_END);
        sal_Int32 nSize = m_pFileStream->Tell();
        m_pFileStream->Seek(STREAM_SEEK_TO_BEGIN);

        // Buffersize is dependent on the file-size
        m_pFileStream->SetBufferSize(nSize > 1000000 ? 32768 :
                                    nSize > 100000  ? 16384 :
                                    nSize > 10000   ? 4096  : 1024);

        fillColumns(aAppLocale);

        refreshColumns();
    }
}
// -------------------------------------------------------------------------
String OFlatTable::getEntry()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "flat", "Ocke.Janssen@sun.com", "OFlatTable::getEntry" );
    ::rtl::OUString sURL;
    try
    {
        Reference< XResultSet > xDir = m_pConnection->getDir()->getStaticResultSet();
        Reference< XRow> xRow(xDir,UNO_QUERY);
        ::rtl::OUString sName;
        ::rtl::OUString sExt;

        INetURLObject aURL;
        xDir->beforeFirst();
        static const ::rtl::OUString s_sSeparator("/");
        while(xDir->next())
        {
            sName = xRow->getString(1);
            aURL.SetSmartProtocol(INET_PROT_FILE);
            String sUrl = m_pConnection->getURL() +  s_sSeparator + sName;
            aURL.SetSmartURL( sUrl );

            // cut the extension
            sExt = aURL.getExtension();

            // name and extension have to coincide
            if ( m_pConnection->matchesExtension( sExt ) )
            {
                if ( !sExt.isEmpty() )
                    sName = sName.replaceAt(sName.getLength()-(sExt.getLength()+1),sExt.getLength()+1,::rtl::OUString());
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
        OSL_ASSERT(0);
    }
    return sURL;
}
// -------------------------------------------------------------------------
void OFlatTable::refreshColumns()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "flat", "Ocke.Janssen@sun.com", "OFlatTable::refreshColumns" );
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

// -------------------------------------------------------------------------
void SAL_CALL OFlatTable::disposing(void)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "flat", "Ocke.Janssen@sun.com", "OFlatTable::disposing" );
    OFileTable::disposing();
    ::osl::MutexGuard aGuard(m_aMutex);
    m_aColumns = NULL;
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OFlatTable::getTypes(  ) throw(RuntimeException)
{
    Sequence< Type > aTypes = OTable_TYPEDEF::getTypes();
    ::std::vector<Type> aOwnTypes;
    aOwnTypes.reserve(aTypes.getLength());
    const Type* pBegin = aTypes.getConstArray();
    const Type* pEnd = pBegin + aTypes.getLength();
    for(;pBegin != pEnd;++pBegin)
    {
        if(!(*pBegin == ::getCppuType((const Reference<XKeysSupplier>*)0)   ||
            *pBegin == ::getCppuType((const Reference<XRename>*)0)          ||
            *pBegin == ::getCppuType((const Reference<XIndexesSupplier>*)0) ||
            *pBegin == ::getCppuType((const Reference<XAlterTable>*)0)      ||
            *pBegin == ::getCppuType((const Reference<XDataDescriptorFactory>*)0)))
        {
            aOwnTypes.push_back(*pBegin);
        }
    }
    Type *pTypes = aOwnTypes.empty() ? 0 : &aOwnTypes[0];
    return Sequence< Type >(pTypes, aOwnTypes.size());
}

// -------------------------------------------------------------------------
Any SAL_CALL OFlatTable::queryInterface( const Type & rType ) throw(RuntimeException)
{
    if( rType == ::getCppuType((const Reference<XKeysSupplier>*)0)      ||
        rType == ::getCppuType((const Reference<XIndexesSupplier>*)0)   ||
        rType == ::getCppuType((const Reference<XRename>*)0)            ||
        rType == ::getCppuType((const Reference<XAlterTable>*)0)        ||
        rType == ::getCppuType((const Reference<XDataDescriptorFactory>*)0))
        return Any();

    Any aRet = OTable_TYPEDEF::queryInterface(rType);
    return aRet.hasValue() ? aRet : ::cppu::queryInterface(rType,static_cast< ::com::sun::star::lang::XUnoTunnel*> (this));
}

//--------------------------------------------------------------------------
Sequence< sal_Int8 > OFlatTable::getUnoTunnelImplementationId()
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
sal_Int64 OFlatTable::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "flat", "Ocke.Janssen@sun.com", "OFlatTable::getSomething" );
    return (rId.getLength() == 16 && 0 == memcmp(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
                ? reinterpret_cast< sal_Int64 >( this )
                : OFlatTable_BASE::getSomething(rId);
}
//------------------------------------------------------------------
sal_Bool OFlatTable::fetchRow(OValueRefRow& _rRow,const OSQLColumns & _rCols,sal_Bool bIsTable,sal_Bool bRetrieveData)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "flat", "Ocke.Janssen@sun.com", "OFlatTable::fetchRow" );
    *(_rRow->get())[0] = m_nFilePos;

    if (!bRetrieveData)
        return sal_True;
    if ( m_bNeedToReadLine )
    {
        sal_Int32 nCurrentPos = 0;
        m_pFileStream->Seek(m_nFilePos);
        readLine(nCurrentPos);
        m_bNeedToReadLine = false;
    }

    OFlatConnection* pConnection = (OFlatConnection*)m_pConnection;
    const sal_Unicode cDecimalDelimiter = pConnection->getDecimalDelimiter();
    const sal_Unicode cThousandDelimiter = pConnection->getThousandDelimiter();
    // Fields:
    xub_StrLen nStartPos = 0;
    String aStr;
    OSQLColumns::Vector::const_iterator aIter = _rCols.get().begin();
    OSQLColumns::Vector::const_iterator aEnd = _rCols.get().end();
    const OValueRefVector::Vector::size_type nCount = _rRow->get().size();
    for (OValueRefVector::Vector::size_type i = 1; aIter != aEnd && i < nCount;
         ++aIter, i++)
    {
        m_aCurrentLine.GetTokenSpecial(aStr,nStartPos,m_cFieldDelimiter,m_cStringDelimiter);

        if (aStr.Len() == 0)
            (_rRow->get())[i]->setNull();
        else
        {
            // lengths depending on data-type:
            sal_Int32   nLen,
                        nType = 0;
            if(bIsTable)
            {
                nLen    = m_aPrecisions[i-1];
                nType   = m_aTypes[i-1];
            }
            else
            {
                Reference< XPropertySet> xColumn = *aIter;
                xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION))  >>= nLen;
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

                    String aStrConverted;
                    if ( DataType::INTEGER != nType )
                    {
                        sal_Unicode* pData = aStrConverted.AllocBuffer(aStr.Len());
                        const sal_Unicode* pStart = pData;

                        OSL_ENSURE((cDecimalDelimiter && nType != DataType::INTEGER) ||
                                   (!cDecimalDelimiter && nType == DataType::INTEGER),
                                   "FalscherTyp");

                        // convert to Standard-Notation (DecimalPOINT without thousands-comma):
                        for (xub_StrLen j = 0; j < aStr.Len(); ++j)
                        {
                            const sal_Unicode cChar = aStr.GetChar(j);
                            if (cDecimalDelimiter && cChar == cDecimalDelimiter)
                                *pData++ = '.';
                                //aStrConverted.Append( '.' );
                            else if ( cChar == '.' ) // special case, if decimal seperator isn't '.' we have to put the string after it
                                continue;
                            else if (cThousandDelimiter && cChar == cThousandDelimiter)
                            {
                                // leave out
                            }
                            else
                                *pData++ = cChar;
                                //aStrConverted.Append(cChar);
                        } // for (xub_StrLen j = 0; j < aStr.Len(); ++j)
                        aStrConverted.ReleaseBufferAccess(xub_StrLen(pData - pStart));
                    } // if ( DataType::INTEGER != nType )
                    else
                    {
                        if ( cThousandDelimiter )
                            aStrConverted = comphelper::string::remove(aStr, cThousandDelimiter);
                        else
                            aStrConverted = aStr;
                    }
                    const double nVal = ::rtl::math::stringToDouble(aStrConverted,'.',',',NULL,NULL);

                    // #99178# OJ
                    if ( DataType::DECIMAL == nType || DataType::NUMERIC == nType )
                        *(_rRow->get())[i] = ::rtl::OUString::valueOf(nVal);
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
    return sal_True;
}
void OFlatTable::refreshHeader()
{
    m_nRowPos = 0;
}
// -----------------------------------------------------------------------------
sal_Bool OFlatTable::seekRow(IResultSetHelper::Movement eCursorPosition, sal_Int32 nOffset, sal_Int32& nCurPos)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "flat", "Ocke.Janssen@sun.com", "OFlatTable::seekRow" );
    OSL_ENSURE(m_pFileStream,"OFlatTable::seekRow: FileStream is NULL!");
    // ----------------------------------------------------------
    // Prepare positioning:
    m_nFilePos = nCurPos;

    switch(eCursorPosition)
    {
        case IResultSetHelper::FIRST:
            m_nRowPos = 0;
            // run through
        case IResultSetHelper::NEXT:
            {
                ++m_nRowPos;
                ::std::map<sal_Int32,TRowPositionsInFile::iterator>::const_iterator aFind = m_aRowPosToFilePos.find(m_nRowPos);
                m_bNeedToReadLine = aFind != m_aRowPosToFilePos.end();
                if ( m_bNeedToReadLine )
                {
                    m_nFilePos  = aFind->second->first;
                    nCurPos     = aFind->second->second;
                } // if ( m_bNeedToReadLine )
                else
                {
                    if ( m_nRowPos == 1 )
                        m_nFilePos = m_nStartRowFilePos;
                    m_pFileStream->Seek(m_nFilePos);
                    if ( m_pFileStream->IsEof() || !readLine(nCurPos) /*|| !checkHeaderLine()*/)
                    {
                        m_nMaxRowCount = m_nRowPos -1;
                        return sal_False;
                    } // if ( m_pFileStream->IsEof() || !readLine(nCurPos) /*|| !checkHeaderLine()*/)

                    TRowPositionsInFile::iterator aPos = m_aFilePosToEndLinePos.insert(TRowPositionsInFile::value_type(m_nFilePos,nCurPos)).first;
                    m_aRowPosToFilePos.insert(::std::map<sal_Int32,TRowPositionsInFile::iterator>::value_type(m_nRowPos,aPos));
                }
            }

            break;
        case IResultSetHelper::PRIOR:
            --m_nRowPos;
            if(m_nRowPos > 0)
            {
                TRowPositionsInFile::iterator aPositions = m_aRowPosToFilePos[m_nRowPos];
                m_nFilePos = aPositions->first;
                nCurPos = aPositions->second;
                m_bNeedToReadLine = true;
            }
            else
                m_nRowPos = 0;

            break;
        case IResultSetHelper::LAST:
            if ( m_nMaxRowCount )
            {
                ::std::map<sal_Int32,TRowPositionsInFile::iterator>::reverse_iterator aLastPos = m_aRowPosToFilePos.rbegin();
                m_nRowPos  = aLastPos->first;
                m_nFilePos = aLastPos->second->first;
                nCurPos    = aLastPos->second->second;

                //m_pFileStream->Seek(m_nFilePos);
                m_bNeedToReadLine = true;
                //if ( m_pFileStream->IsEof() /*|| !checkHeaderLine()*/ || !readLine(nCurPos) )
                //  return sal_False;
            }
            else
            {
                while(seekRow(IResultSetHelper::NEXT,1,nCurPos)) ; // run through after last row
                // now I know all
                seekRow(IResultSetHelper::PRIOR,1,nCurPos);
            }
            break;
        case IResultSetHelper::RELATIVE:
            if(nOffset > 0)
            {
                for(sal_Int32 i = 0;i<nOffset;++i)
                    seekRow(IResultSetHelper::NEXT,1,nCurPos);
            }
            else if(nOffset < 0)
            {
                for(sal_Int32 i = nOffset;i;++i)
                    seekRow(IResultSetHelper::PRIOR,1,nCurPos);
            }
            break;
        case IResultSetHelper::ABSOLUTE:
            {
                if(nOffset < 0)
                    nOffset = m_nRowPos + nOffset;
                ::std::map<sal_Int32,TRowPositionsInFile::iterator>::const_iterator aIter = m_aRowPosToFilePos.find(nOffset);
                if(aIter != m_aRowPosToFilePos.end())
                {
                    m_nFilePos  = aIter->second->first;
                    nCurPos     = aIter->second->second;
                    //m_pFileStream->Seek(m_nFilePos);
                    m_bNeedToReadLine = true;
                    //if ( m_pFileStream->IsEof() /*|| !checkHeaderLine()*/ || !readLine(nCurPos) )
                    //  return sal_False;
                }
                else if(m_nMaxRowCount && nOffset > m_nMaxRowCount) // offset is outside the table
                {
                    m_nRowPos = m_nMaxRowCount;
                    return sal_False;
                }
                else
                {
                    aIter = m_aRowPosToFilePos.upper_bound(nOffset);
                    if(aIter == m_aRowPosToFilePos.end())
                    {
                        ::std::map<sal_Int32,TRowPositionsInFile::iterator>::reverse_iterator aLastPos = m_aRowPosToFilePos.rbegin();
                        m_nRowPos   = aLastPos->first;
                        nCurPos = m_nFilePos = aLastPos->second->first;
                        while(m_nRowPos != nOffset)
                            seekRow(IResultSetHelper::NEXT,1,nCurPos);
                    }
                    else
                    {
                        --aIter;
                        m_nRowPos   = aIter->first;
                        m_nFilePos  = aIter->second->first;
                        nCurPos     = aIter->second->second;
                        //m_pFileStream->Seek(m_nFilePos);
                        m_bNeedToReadLine = true;
                        //if ( m_pFileStream->IsEof() /*|| !checkHeaderLine()*/ || !readLine(nCurPos) )
                        //  return sal_False;
                    }
                }
            }

            break;
        case IResultSetHelper::BOOKMARK:
            {
                TRowPositionsInFile::const_iterator aFind = m_aFilePosToEndLinePos.find(nOffset);
                m_bNeedToReadLine = aFind != m_aFilePosToEndLinePos.end();
                if ( m_bNeedToReadLine )
                {
                    m_nFilePos  = aFind->first;
                    nCurPos = aFind->second;
                }
                else
                {
                    m_nFilePos = nOffset;
                    m_pFileStream->Seek(nOffset);
                    if (m_pFileStream->IsEof() || !readLine(nCurPos) )
                        return sal_False;
                }
                break;
            }
    }

    //nCurPos = m_nFilePos;

    return sal_True;
}
// -----------------------------------------------------------------------------
sal_Bool OFlatTable::readLine(sal_Int32& _rnCurrentPos)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "flat", "Ocke.Janssen@sun.com", "OFlatTable::readLine" );
    const rtl_TextEncoding nEncoding = m_pConnection->getTextEncoding();
    m_pFileStream->ReadByteStringLine(m_aCurrentLine,nEncoding);
    if (m_pFileStream->IsEof())
        return sal_False;

    QuotedTokenizedString sLine = m_aCurrentLine; // check if the string continues on next line
    while( (comphelper::string::getTokenCount(sLine.GetString(), m_cStringDelimiter) % 2) != 1 )
    {
        m_pFileStream->ReadByteStringLine(sLine,nEncoding);
        if ( !m_pFileStream->IsEof() )
        {
            m_aCurrentLine.GetString().Append('\n');
            m_aCurrentLine.GetString() += sLine.GetString();
            sLine = m_aCurrentLine;
        }
        else
            break;
    }
    _rnCurrentPos = m_pFileStream->Tell();
    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
