/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <ctype.h>
#include <algorithm>
#include "LFolderList.hxx"
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <svl/converter.hxx>
#include "LConnection.hxx"
#include "LColumns.hxx"
#include <osl/thread.h>
#include <tools/config.hxx>
#include <comphelper/sequence.hxx>
#include <svl/zforlist.hxx>
#include <rtl/math.hxx>
#include <stdio.h>      //sprintf
#include <comphelper/extract.hxx>
#include <comphelper/numbers.hxx>
#include "LDriver.hxx"
#include <com/sun/star/util/NumberFormat.hpp>
#include <unotools/configmgr.hxx>
#include <i18npool/mslangid.hxx>
#include "connectivity/dbconversion.hxx"
#include <comphelper/types.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <tools/debug.hxx>
#include "connectivity/dbexception.hxx"
#include "LDebug.hxx"
#include <unotools/syslocale.hxx>

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::evoab;
using namespace connectivity::file;
using namespace ::cppu;
using namespace utl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

// -------------------------------------------------------------------------
void OEvoabFolderList::fillColumns(const ::com::sun::star::lang::Locale& _aLocale)
{
    sal_Bool bRead = sal_True;

    QuotedTokenizedString aHeaderLine;
    OEvoabConnection* pConnection = (OEvoabConnection*)m_pConnection;

    // read first row
    QuotedTokenizedString aFirstLine;
    bRead = m_pFileStream->ReadByteStringLine(aFirstLine,pConnection->getTextEncoding());

    while(bRead && !aFirstLine.Len())
    {
        bRead = m_pFileStream->ReadByteStringLine(aFirstLine,pConnection->getTextEncoding());
    }
    // use first row as headerline because we need the number of columns
    aHeaderLine = aFirstLine;

    // column count
    xub_StrLen nFieldCount = aHeaderLine.GetTokenCount(pConnection->getFieldDelimiter(),pConnection->getStringDelimiter());

    if(!m_aColumns.is())
        m_aColumns = new OSQLColumns();
    else
        m_aColumns->get().clear();

    m_aTypes.clear();
    m_aPrecisions.clear();
    m_aScales.clear();
    // reserve some space
    m_aColumns->get().reserve(nFieldCount);
    m_aTypes.reserve(nFieldCount);
    m_aPrecisions.reserve(nFieldCount);
    m_aScales.reserve(nFieldCount);

    sal_Bool bCase = getConnection()->getMetaData()->supportsMixedCaseQuotedIdentifiers();
    CharClass aCharClass(pConnection->getDriver()->getFactory(),_aLocale);
    // read description
    sal_Unicode cDecimalDelimiter  = pConnection->getDecimalDelimiter();
    sal_Unicode cThousandDelimiter = pConnection->getThousandDelimiter();
    String aColumnName;
    ::rtl::OUString aTypeName;
    ::comphelper::UStringMixEqual aCase(bCase);
    xub_StrLen nStartPosFirstLine = 0; // use for eficient way to get the tokens
    xub_StrLen nStartPosFirstLine2 = 0;
    for (xub_StrLen i = 0; i < nFieldCount; i++)
    {

        // no column name so ...
        aColumnName = 'C';
        aColumnName += String::CreateFromInt32(i+1);

        sal_Int32 eType;
        sal_uInt16 nPrecision = 0;
        sal_uInt16 nScale = 0;

        sal_Bool bNumeric = sal_False;
        sal_uIntPtr  nIndex = 0;

        // first without fielddelimiter
        String aField;
        aFirstLine.GetTokenSpecial(aField,nStartPosFirstLine,pConnection->getFieldDelimiter(),'\0');
        if (aField.Len() == 0 ||
            (pConnection->getStringDelimiter() && pConnection->getStringDelimiter() == aField.GetChar(0)))
        {
            bNumeric = sal_False;
        }
        else
        {
            String aField2;
            if ( pConnection->getStringDelimiter() != '\0' )
                aFirstLine.GetTokenSpecial(aField2,nStartPosFirstLine2,pConnection->getFieldDelimiter(),pConnection->getStringDelimiter());
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
                for (xub_StrLen j = 0; j < aField2.Len(); j++)
                {
                    sal_Unicode c = aField2.GetChar(j);
                    // Only digits and decimalpoint and thousands delimiter?
                    if ((!cDecimalDelimiter || c != cDecimalDelimiter) &&
                        (!cThousandDelimiter || c != cThousandDelimiter) &&
                        !aCharClass.isDigit(aField2,j))
                    {
                        bNumeric = sal_False;
                        break;
                    }
                    if (cDecimalDelimiter && c == cDecimalDelimiter)
                    {
                        nPrecision = 15; // we have an decimal value
                        nScale = 2;
                        nDot++;
                    }
                }

                if (nDot > 1) // if there is more than one dot it isn't a number
                    bNumeric = sal_False;
                if (bNumeric && cThousandDelimiter)
                {
                    // Is the delimiter given correctly?
                    String aValue = aField2.GetToken(0,cDecimalDelimiter);
                    for (sal_Int32 j = aValue.Len() - 4; j >= 0; j -= 4)
                    {
                        sal_Unicode c = aValue.GetChar(j);
                        // Only digits, decimalpoint and thousands delimiter?
                        if (c == cThousandDelimiter && j)
                            continue;
                        else
                        {
                            bNumeric = sal_False;
                            break;
                        }
                    }
                }

                // now it might still be a date-field
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

        sal_Int32 nFlags = 0;
        if (bNumeric)
        {
            if (cDecimalDelimiter)
            {
                if(nPrecision)
                {
                    eType = DataType::DECIMAL;
                    aTypeName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DECIMAL"));
                }
                else
                {
                    eType = DataType::DOUBLE;
                    aTypeName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DOUBLE"));
                }
            }
            else
                eType = DataType::INTEGER;
            nFlags = ColumnSearch::BASIC;
        }
        else
        {

            switch (comphelper::getNumberFormatType(m_xNumberFormatter,nIndex))
            {
                case NUMBERFORMAT_DATE:
                    eType = DataType::DATE;
                    aTypeName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DATE"));
                    break;
                case NUMBERFORMAT_DATETIME:
                    eType = DataType::TIMESTAMP;
                    aTypeName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TIMESTAMP"));
                    break;
                case NUMBERFORMAT_TIME:
                    eType = DataType::TIME;
                    aTypeName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TIME"));
                    break;
                default:
                    eType = DataType::VARCHAR;
                    nPrecision = 0; // nyi: Data can be longer!
                    nScale = 0;
                    aTypeName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VARCHAR"));
            };
            nFlags |= ColumnSearch::CHAR;
        }

        // check if the columname already exists
        String aAlias(aColumnName);
        OSQLColumns::Vector::const_iterator aFind = connectivity::find(m_aColumns->get().begin(),m_aColumns->get().end(),aAlias,aCase);
        sal_Int32 nExprCnt = 0;
        while(aFind != m_aColumns->get().end())
        {
            (aAlias = aColumnName) += String::CreateFromInt32(++nExprCnt);
            aFind = connectivity::find(m_aColumns->get().begin(),m_aColumns->get().end(),aAlias,aCase);
        }

        sdbcx::OColumn* pColumn = new sdbcx::OColumn(aAlias,aTypeName,::rtl::OUString(),
                                                ColumnValue::NULLABLE,
                                                nPrecision,
                                                nScale,
                                                eType,
                                                sal_False,
                                                sal_False,
                                                sal_False,
                                                bCase);
        Reference< XPropertySet> xCol = pColumn;
        m_aColumns->get().push_back(xCol);
        m_aTypes.push_back(eType);
        m_aPrecisions.push_back(nPrecision);
        m_aScales.push_back(nScale);
    }
    m_pFileStream->Seek(STREAM_SEEK_TO_BEGIN);
}
// -------------------------------------------------------------------------
DBG_NAME( OEvoabFolderList );
OEvoabFolderList::OEvoabFolderList(OEvoabConnection* _pConnection)
    :m_nFilePos(0)
    ,m_pFileStream(NULL)
    ,m_pConnection(_pConnection)
    ,m_bIsNull(sal_False)
{
    DBG_CTOR( OEvoabFolderList, NULL );
    m_aColumns = new OSQLColumns();

    construct();
}
// -----------------------------------------------------------------------------
void OEvoabFolderList::construct()
{
    SvtSysLocale aLocale;
    ::com::sun::star::lang::Locale aAppLocale(aLocale.GetLocaleDataPtr()->getLocale());
    Sequence< ::com::sun::star::uno::Any > aArg(1);
    aArg[0] <<= aAppLocale;

    Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xSupplier(m_pConnection->getDriver()->getFactory()->createInstanceWithArguments(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.NumberFormatsSupplier")),aArg),UNO_QUERY);
    m_xNumberFormatter = Reference< ::com::sun::star::util::XNumberFormatter >(m_pConnection->getDriver()->getFactory()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.NumberFormatter"))),UNO_QUERY);
    m_xNumberFormatter->attachNumberFormatsSupplier(xSupplier);

    INetURLObject aURL;
    aURL.SetURL(m_pConnection->getDriver()->getEvoFolderListFileURL());

    String aFileName = aURL.GetMainURL(INetURLObject::NO_DECODE);

    EVO_TRACE_STRING("OJ::construct()::aFileName = %s\n", aFileName );
    m_pFileStream = createStream_simpleError( aFileName,STREAM_READWRITE | STREAM_NOCREATE | STREAM_SHARE_DENYWRITE);

    if(!m_pFileStream)
        m_pFileStream = createStream_simpleError( aFileName,STREAM_READ | STREAM_NOCREATE | STREAM_SHARE_DENYNONE);

    if(m_pFileStream)
    {
        m_pFileStream->Seek(STREAM_SEEK_TO_END);
        sal_Int32 nSize = m_pFileStream->Tell();
        m_pFileStream->Seek(STREAM_SEEK_TO_BEGIN);

        // Buffersize dependent on the file-size
        m_pFileStream->SetBufferSize(nSize > 1000000 ? 32768 :
                                    nSize > 100000  ? 16384 :
                                    nSize > 10000   ? 4096  : 1024);
        OSL_TRACE("OEvoabFolderList::construct()::m_pFileStream->Tell() = %d\n", nSize );

        fillColumns(aAppLocale);
    }
}


//------------------------------------------------------------------
sal_Bool OEvoabFolderList::fetchRow(OValueRow _rRow,const OSQLColumns & _rCols)
{
    (_rRow->get())[0] = m_nFilePos; // the "bookmark"

    OEvoabConnection* pConnection = (OEvoabConnection*)m_pConnection;
    // Fields:
    xub_StrLen nStartPos = 0;
    String aStr;
    OSQLColumns::Vector::const_iterator aIter = _rCols.get().begin();
    for (sal_Int32 i = 0; aIter != _rCols.get().end();++aIter, ++i)
    {
        m_aCurrentLine.GetTokenSpecial(aStr,nStartPos,pConnection->getFieldDelimiter(),pConnection->getStringDelimiter());
        //OSL_TRACE("OEvoabFolderList::fetchRow()::aStr = %s\n", ((OUtoCStr(::rtl::OUString(aStr))) ? (OUtoCStr(::rtl::OUString(aStr))):("NULL")) );

        if (aStr.Len() == 0)
            (_rRow->get())[i+1].setNull();
        else
        {
            // length depending on the data type
            sal_Int32   nType   = m_aTypes[i];
            switch(nType)
            {
                case DataType::TIMESTAMP:
                case DataType::DATE:
                case DataType::TIME:
                {
                    double nRes = 0.0;
                    try
                    {
                        nRes = m_xNumberFormatter->convertStringToNumber(::com::sun::star::util::NumberFormat::ALL,aStr);
                        Reference<XPropertySet> xProp(m_xNumberFormatter->getNumberFormatsSupplier()->getNumberFormatSettings(),UNO_QUERY);
                        com::sun::star::util::Date aDate;
                        xProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NullDate"))) >>= aDate;

                        switch(nType)
                        {
                            case DataType::DATE:
                                (_rRow->get())[i+1] = ::dbtools::DBTypeConversion::toDouble(::dbtools::DBTypeConversion::toDate(nRes,aDate));
                                break;
                            case DataType::TIMESTAMP:
                                (_rRow->get())[i+1] = ::dbtools::DBTypeConversion::toDouble(::dbtools::DBTypeConversion::toDateTime(nRes,aDate));
                                break;
                            default:
                                (_rRow->get())[i+1] = ::dbtools::DBTypeConversion::toDouble(::dbtools::DBTypeConversion::toTime(nRes));
                        }
                    }
                    catch(Exception&)
                    {
                        (_rRow->get())[i+1].setNull();
                    }
                }   break;
                case DataType::DOUBLE:
                case DataType::INTEGER:
                case DataType::DECIMAL:
                case DataType::NUMERIC:
                {
                    sal_Unicode cDecimalDelimiter = pConnection->getDecimalDelimiter();
                    sal_Unicode cThousandDelimiter = pConnection->getThousandDelimiter();
                    String aStrConverted;

                    OSL_ENSURE(cDecimalDelimiter && nType != DataType::INTEGER ||
                               !cDecimalDelimiter && nType == DataType::INTEGER,
                               "FalscherTyp");

                    // Convert to standard notation (DecimalPOINT without Thousands-comma):
                    for (xub_StrLen j = 0; j < aStr.Len(); ++j)
                    {
                        if (cDecimalDelimiter && aStr.GetChar(j) == cDecimalDelimiter)
                            aStrConverted += '.';
                        else if ( aStr.GetChar(j) == '.' ) // special case, if decimal seperator isn't '.' we have to vut the string after it
                            break;
                        else if (cThousandDelimiter && aStr.GetChar(j) == cThousandDelimiter)
                        {
                            // leave out
                        }
                        else
                            aStrConverted += aStr.GetChar(j) ;
                    }
                    double nVal = ::rtl::math::stringToDouble(aStrConverted.GetBuffer(),',','.',NULL,NULL);

                    if ( DataType::DECIMAL == nType || DataType::NUMERIC == nType )
                        (_rRow->get())[i+1] = String::CreateFromDouble(nVal);
                    else
                        (_rRow->get())[i+1] = nVal;
                } break;

                default:
                {
                    // put value as string in Row variable
                    (_rRow->get())[i+1] = aStr;
                }
                break;
            }
        }
    }
    return sal_True;
}


//------------------------------------------------------------------
sal_Bool OEvoabFolderList::seekRow(IResultSetHelper::Movement eCursorPosition)
{
    //OSL_TRACE("OEvoabFolderList::seekRow()::(before SeekRow)m_aCurrentLine = %d\n", ((OUtoCStr(::rtl::OUString(m_aCurrentLine))) ? (OUtoCStr(::rtl::OUString(m_aCurrentLine))):("NULL")) );

    if ( !m_pFileStream )
        return sal_False;
    OEvoabConnection* pConnection = (OEvoabConnection*)m_pConnection;
    // ----------------------------------------------------------
    // Prepare positioning:
    //OSL_TRACE("OEvoabFolderList::seekRow()::(before SeekRow,m_pFileStriam Exist)m_aCurrentLine = %d\n", ((OUtoCStr(::rtl::OUString(m_aCurrentLine))) ? (OUtoCStr(::rtl::OUString(m_aCurrentLine))):("NULL")) );

    switch(eCursorPosition)
    {
        case IResultSetHelper::FIRST:
            m_nFilePos = 0;
            // run through

        case IResultSetHelper::NEXT:
            m_pFileStream->Seek(m_nFilePos);

            if (m_pFileStream->IsEof())
            {
                OSL_TRACE( "OEvoabFolderList::seekRow: EOF /before/ reading the line." );
                return sal_False;
            }

            m_pFileStream->ReadByteStringLine(m_aCurrentLine,pConnection->getTextEncoding());
            if (m_pFileStream->IsEof())
            {
                OSL_TRACE( "OEvoabFolderList::seekRow: EOF /after/ reading the line." );
                if ( !m_aCurrentLine.Len() )
                {
                    OSL_TRACE( "OEvoabFolderList::seekRow: empty line read." );
                    return sal_False;
                }
            }
            m_nFilePos = m_pFileStream->Tell();
            break;

        default:
            OSL_ENSURE( sal_False, "OEvoabFolderList::seekRow: unsupported positioning!" );
            break;
    }

    //OSL_TRACE("OEvoabFolderList::seekRow()::(after SeekRow)m_aCurrentLine = %d\n", ((OUtoCStr(::rtl::OUString(m_aCurrentLine))) ? (OUtoCStr(::rtl::OUString(m_aCurrentLine))):("NULL")) );

    return sal_True;
}
// -----------------------------------------------------------------------------
SvStream* OEvoabFolderList::createStream_simpleError( const String& _rFileName, StreamMode _eOpenMode)
{
    utl::UcbLockBytesHandler* p_null_dummy=NULL;
    SvStream* pReturn = ::utl::UcbStreamHelper::CreateStream( _rFileName, _eOpenMode, p_null_dummy);
    if (pReturn && (ERRCODE_NONE != pReturn->GetErrorCode()))
    {
        delete pReturn;
        pReturn = NULL;
    }
    return pReturn;
}
// -----------------------------------------------------------------------------
const ORowSetValue& OEvoabFolderList::getValue(sal_Int32 _nColumnIndex ) throw(::com::sun::star::sdbc::SQLException)
{
    checkIndex( _nColumnIndex );

    m_bIsNull = (m_aRow->get())[_nColumnIndex].isNull();
    return (m_aRow->get())[_nColumnIndex];
}
// -----------------------------------------------------------------------------
void OEvoabFolderList::checkIndex(sal_Int32 _nColumnIndex ) throw(::com::sun::star::sdbc::SQLException)
{
    if (   _nColumnIndex <= 0 || _nColumnIndex >= (sal_Int32)m_aRow->get().size() ) {
//        ::dbtools::throwInvalidIndexException();
            ;
    }
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabFolderList::getString( sal_Int32 _nColumnIndex ) throw(SQLException, RuntimeException)
{
    return getValue(_nColumnIndex);
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OEvoabFolderList::getInt( sal_Int32 _nColumnIndex ) throw(SQLException, RuntimeException)
{
    return getValue( _nColumnIndex );
}
// -----------------------------------------------------------------------------
void OEvoabFolderList::initializeRow(sal_Int32 _nColumnCount)
{
    if(!m_aRow.is())
    {
        m_aRow  = new OValueVector(_nColumnCount);
        (m_aRow->get())[0].setBound(sal_True);
        ::std::for_each(m_aRow->get().begin()+1,m_aRow->get().end(),TSetBound(sal_False));
    }
    //OSL_TRACE("OEvoabFolderList::initializeRow()::_nColumnCount = %d\n", _nColumnCount);

}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OEvoabFolderList::first(  ) throw(SQLException, RuntimeException)
{
    sal_Bool bSuccess = seekRow(IResultSetHelper::FIRST);

    EVO_TRACE_STRING("OEvoabFolderList::first(): returning %s\n", ::rtl::OUString::valueOf(bSuccess) );
    return bSuccess;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OEvoabFolderList::next(  ) throw(SQLException, RuntimeException)
{
    sal_Bool bSuccess = seekRow(IResultSetHelper::NEXT);

    EVO_TRACE_STRING("OEvoabFolderList::next(): returning %s\n", ::rtl::OUString::valueOf(bSuccess) );
    return bSuccess;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OEvoabFolderList::getRow(  ) throw(SQLException, RuntimeException)
{
    sal_Bool bRet = fetchRow(m_aRow,*(getTableColumns()));
    EVO_TRACE_STRING("OEvoabFolderList::getRow()::fetchRow() = %s\n", ::rtl::OUString::valueOf(bRet) );

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
