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
#include "LTable.hxx"
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#ifndef _COM_SUN_STAR_SQLC_XROW_HPP_
//#include <com/sun/star/sdbc/XRow.hpp>
#endif
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
#include <unotools/syslocale.hxx>
#ifndef CONNECTIVITY_EVOAB_DEBUG_HELPER_HXX
#include "LDebug.hxx"
#endif
#include <map>

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::evoab;
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
void OEvoabTable::fillColumns(const ::com::sun::star::lang::Locale& _aLocale)
{
    sal_Bool bRead = sal_True;

    QuotedTokenizedString aHeaderLine;
    OEvoabConnection* pConnection = (OEvoabConnection*)m_pConnection;
    if (pConnection->isHeaderLine())
    {
        while(bRead && !aHeaderLine.Len())
        {
            bRead = m_pFileStream->ReadByteStringLine(aHeaderLine,pConnection->getTextEncoding());
        }
    }

    // read first row
    QuotedTokenizedString aFirstLine;
    bRead = m_pFileStream->ReadByteStringLine(aFirstLine,pConnection->getTextEncoding());

    if (!pConnection->isHeaderLine() || !aHeaderLine.Len())
    {
        while(bRead && !aFirstLine.Len())
        {
            bRead = m_pFileStream->ReadByteStringLine(aFirstLine,pConnection->getTextEncoding());
        }
        // use first row as headerline because we need the number of columns
        aHeaderLine = aFirstLine;
    }
    // column count
    xub_StrLen nFieldCount = aHeaderLine.GetTokenCount(pConnection->getFieldDelimiter(),pConnection->getStringDelimiter());

    if(!m_aColumns.isValid())
        m_aColumns = new OSQLColumns();
    else
        m_aColumns->get().clear();

    m_aTypes.clear();
    m_aPrecisions.clear();
    m_aScales.clear();
    // reserve some space
    m_aColumnRawNames.reserve(nFieldCount);
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
    xub_StrLen nStartPosHeaderLine = 0; // use for eficient way to get the tokens
    xub_StrLen nStartPosFirstLine = 0; // use for eficient way to get the tokens
    xub_StrLen nStartPosFirstLine2 = 0;
    for (xub_StrLen i = 0; i < nFieldCount; i++)
    {
        if (pConnection->isHeaderLine())
        {
            aHeaderLine.GetTokenSpecial(aColumnName,nStartPosHeaderLine,pConnection->getFieldDelimiter(),pConnection->getStringDelimiter());
        }
        else
        {
            // no column name so ...
            aColumnName = 'C';
            aColumnName += String::CreateFromInt32(i+1);
        }
        //OSL_TRACE("OEvoabTable::aColumnName = %s\n", ((OUtoCStr(::rtl::OUString(aColumnName))) ? (OUtoCStr(::rtl::OUString(aColumnName))):("NULL")) );

        sal_Int32 eType;
        sal_uInt16 nPrecision = 0;
        sal_uInt16 nScale = 0;

        sal_Bool bNumeric = sal_False;
        sal_uIntPtr  nIndex = 0;

        // first without fielddelimiter
        String aField;
        aFirstLine.GetTokenSpecial(aField,nStartPosFirstLine,pConnection->getFieldDelimiter(),'\0');
        //OSL_TRACE("OEvoabTable::aField = %s\n", ((OUtoCStr(::rtl::OUString(aField))) ? (OUtoCStr(::rtl::OUString(aField))):("NULL")) );

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

            //OSL_TRACE("OEvoabTable::aField2 = %s\n", ((OUtoCStr(::rtl::OUString(aField2))) ? (OUtoCStr(::rtl::OUString(aField2))):("NULL")) );

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
                    // nur Ziffern und Dezimalpunkt und Tausender-Trennzeichen?
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
                    // Ist der Trenner richtig angegeben?
                    String aValue = aField2.GetToken(0,cDecimalDelimiter);
                    for (sal_Int32 j = aValue.Len() - 4; j >= 0; j -= 4)
                    {
                        sal_Unicode c = aValue.GetChar(j);
                        // nur Ziffern und Dezimalpunkt und Tausender-Trennzeichen?
                        if (c == cThousandDelimiter && j)
                            continue;
                        else
                        {
                            bNumeric = sal_False;
                            break;
                        }
                    }
                }

                // jetzt koennte es noch ein Datumsfeld sein
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
                    aTypeName = ::rtl::OUString::createFromAscii("DECIMAL");
                }
                else
                {
                    eType = DataType::DOUBLE;
                    aTypeName = ::rtl::OUString::createFromAscii("DOUBLE");
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
                    aTypeName = ::rtl::OUString::createFromAscii("DATE");
                    break;
                case NUMBERFORMAT_DATETIME:
                    eType = DataType::TIMESTAMP;
                    aTypeName = ::rtl::OUString::createFromAscii("TIMESTAMP");
                    break;
                case NUMBERFORMAT_TIME:
                    eType = DataType::TIME;
                    aTypeName = ::rtl::OUString::createFromAscii("TIME");
                    break;
                default:
                    eType = DataType::VARCHAR;
                    nPrecision = 0; // nyi: Daten koennen aber laenger sein!
                    nScale = 0;
                    aTypeName = ::rtl::OUString::createFromAscii("VARCHAR");
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

        m_aColumnRawNames.push_back(::rtl::OUString(aAlias));

        //~ sdbcx::OColumn* pColumn = new sdbcx::OColumn(aAlias,aTypeName,::rtl::OUString(),
                                                //~ ColumnValue::NULLABLE,
                                                //~ nPrecision,
                                                //~ nScale,
                                                //~ eType,
                                                //~ sal_False,
                                                //~ sal_False,
                                                //~ sal_False,
                                                //~ bCase);
        //~ Reference< XPropertySet> xCol = pColumn;
        //~ m_aColumns->push_back(xCol);
        m_aTypes.push_back(eType);
        m_aPrecisions.push_back(nPrecision);
        m_aScales.push_back(nScale);
    }
    setColumnAliases();
    m_pFileStream->Seek(STREAM_SEEK_TO_BEGIN);
}
// -------------------------------------------------------------------------
OEvoabTable::OEvoabTable(sdbcx::OCollection* _pTables,OEvoabConnection* _pConnection) : OEvoabTable_BASE(_pTables,_pConnection)
{

}
// -------------------------------------------------------------------------
OEvoabTable::OEvoabTable(sdbcx::OCollection* _pTables,OEvoabConnection* _pConnection,
                    const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _Type,
                    const ::rtl::OUString& _Description ,
                    const ::rtl::OUString& _SchemaName,
                    const ::rtl::OUString& _CatalogName
                ) : OEvoabTable_BASE(_pTables,_pConnection,_Name,
                                  _Type,
                                  _Description,
                                  _SchemaName,
                                  _CatalogName)
    ,m_nRowPos(0)
    ,m_nMaxRowCount(0)
{
    //~ OSL_TRACE("OEvoabTable::(in constructor)_Name = %s\n", ((OUtoCStr(_Name)) ? (OUtoCStr(_Name)):("NULL")) );
    //~ OSL_TRACE("OEvoabTable::(in constructor)_Type = %s\n", ((OUtoCStr(_Type)) ? (OUtoCStr(_Type)):("NULL")) );
    //~ OSL_TRACE("OEvoabTable::(in constructor)_Description = %s\n", ((OUtoCStr(_Description)) ? (OUtoCStr(_Description)):("NULL")) );
    //~ OSL_TRACE("OEvoabTable::(in constructor)_SchemaName = %s\n", ((OUtoCStr(_SchemaName)) ? (OUtoCStr(_SchemaName)):("NULL")) );
    //~ OSL_TRACE("OEvoabTable::(in constructor)_CatalogName = %s\n", ((OUtoCStr(_CatalogName)) ? (OUtoCStr(_CatalogName)):("NULL")) );
}
// -----------------------------------------------------------------------------
void OEvoabTable::construct()
{
    SvtSysLocale aLocale;
    ::com::sun::star::lang::Locale aAppLocale(aLocale.GetLocaleDataPtr()->getLocale());
    Sequence< ::com::sun::star::uno::Any > aArg(1);
    aArg[0] <<= aAppLocale;

    Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xSupplier(m_pConnection->getDriver()->getFactory()->createInstanceWithArguments(::rtl::OUString::createFromAscii("com.sun.star.util.NumberFormatsSupplier"),aArg),UNO_QUERY);
    m_xNumberFormatter = Reference< ::com::sun::star::util::XNumberFormatter >(m_pConnection->getDriver()->getFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.util.NumberFormatter")),UNO_QUERY);
    m_xNumberFormatter->attachNumberFormatsSupplier(xSupplier);

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
        OSL_TRACE("OEvoabTable::construct()::m_pFileStream created\n" );
        m_pFileStream->Seek(STREAM_SEEK_TO_END);
        sal_Int32 nSize = m_pFileStream->Tell();
        m_pFileStream->Seek(STREAM_SEEK_TO_BEGIN);

        // Buffersize abhaengig von der Filegroesse
        m_pFileStream->SetBufferSize(nSize > 1000000 ? 32768 :
                                    nSize > 100000  ? 16384 :
                                    nSize > 10000   ? 4096  : 1024);
        OSL_TRACE("OEvoabTable::construct()::m_pFileStream->Tell() = %d\n", nSize );

        fillColumns(aAppLocale);

        refreshColumns();
    }
}
// -------------------------------------------------------------------------
String OEvoabTable::getEntry()
{
    ::rtl::OUString sURL;
    try
    {
        Reference< XResultSet > xDir = m_pConnection->getDir()->getStaticResultSet();
        Reference< XRow> xRow(xDir,UNO_QUERY);
        ::rtl::OUString sName;
        ::rtl::OUString sExt;

        INetURLObject aURL;
        xDir->beforeFirst();
        static const ::rtl::OUString s_sSeparator(RTL_CONSTASCII_USTRINGPARAM("/"));
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
    catch(Exception&)
    {
        OSL_ASSERT(0);
    }
    return sURL.getStr();
}
// -------------------------------------------------------------------------
void OEvoabTable::refreshColumns()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    TStringVector aVector;
    aVector.reserve(m_aColumns->get().size());

    for(OSQLColumns::Vector::const_iterator aIter = m_aColumns->get().begin();aIter != m_aColumns->get().end();++aIter)
        aVector.push_back(Reference< XNamed>(*aIter,UNO_QUERY)->getName());

    if(m_pColumns)
        m_pColumns->reFill(aVector);
    else
        m_pColumns  = new OEvoabColumns(this,m_aMutex,aVector);

    OSL_TRACE("OEvoabTable::refreshColumns()::end\n" );
}

// -------------------------------------------------------------------------
void SAL_CALL OEvoabTable::disposing(void)
{
    OFileTable::disposing();
    ::osl::MutexGuard aGuard(m_aMutex);
    m_aColumns = NULL;
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OEvoabTable::getTypes(  ) throw(RuntimeException)
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
Any SAL_CALL OEvoabTable::queryInterface( const Type & rType ) throw(RuntimeException)
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
Sequence< sal_Int8 > OEvoabTable::getUnoTunnelImplementationId()
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
sal_Int64 OEvoabTable::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    return (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
                ? reinterpret_cast< sal_Int64 >( this )
                : OEvoabTable_BASE::getSomething(rId);
}
//------------------------------------------------------------------
sal_Bool OEvoabTable::fetchRow(OValueRefRow& _rRow,const OSQLColumns & _rCols,sal_Bool bIsTable,sal_Bool bRetrieveData)
{
    *(_rRow->get())[0] = m_nFilePos;

    if (!bRetrieveData)
        return sal_True;

    OEvoabConnection* pConnection = (OEvoabConnection*)m_pConnection;
    // Felder:
    xub_StrLen nStartPos = 0;
    String aStr;
    OSQLColumns::Vector::const_iterator aIter = _rCols.get().begin();
    for (sal_Int32 i = 0; aIter != _rCols.get().end();++aIter, ++i)
    {
        m_aCurrentLine.GetTokenSpecial(aStr,nStartPos,pConnection->getFieldDelimiter(),pConnection->getStringDelimiter());
        //OSL_TRACE("OEvoabTable::fetchRow()::aStr = %s\n", ((OUtoCStr(::rtl::OUString(aStr))) ? (OUtoCStr(::rtl::OUString(aStr))):("NULL")) );

        if (aStr.Len() == 0)
            (_rRow->get())[i+1]->setNull();
        else
        {
            // Laengen je nach Datentyp:
            sal_Int32   nLen,
                        nType = 0;
            if(bIsTable)
            {
                nLen    = m_aPrecisions[i];
                nType   = m_aTypes[i];
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
                    double nRes = 0.0;
                    try
                    {
                        nRes = m_xNumberFormatter->convertStringToNumber(::com::sun::star::util::NumberFormat::ALL,aStr);
                        Reference<XPropertySet> xProp(m_xNumberFormatter->getNumberFormatsSupplier()->getNumberFormatSettings(),UNO_QUERY);
                        com::sun::star::util::Date aDate;
                        xProp->getPropertyValue(::rtl::OUString::createFromAscii("NullDate")) >>= aDate;

                        switch(nType)
                        {
                            case DataType::DATE:
                                *(_rRow->get())[i+1] = ::dbtools::DBTypeConversion::toDouble(::dbtools::DBTypeConversion::toDate(nRes,aDate));
                                break;
                            case DataType::TIMESTAMP:
                                *(_rRow->get())[i+1] = ::dbtools::DBTypeConversion::toDouble(::dbtools::DBTypeConversion::toDateTime(nRes,aDate));
                                break;
                            default:
                                *(_rRow->get())[i+1] = ::dbtools::DBTypeConversion::toDouble(::dbtools::DBTypeConversion::toTime(nRes));
                        }
                    }
                    catch(Exception&)
                    {
                        (_rRow->get())[i+1]->setNull();
                    }
                }   break;
                case DataType::DOUBLE:
                case DataType::INTEGER:
                case DataType::DECIMAL:             // #99178# OJ
                case DataType::NUMERIC:
                {
                    sal_Unicode cDecimalDelimiter = pConnection->getDecimalDelimiter();
                    sal_Unicode cThousandDelimiter = pConnection->getThousandDelimiter();
                    String aStrConverted;

                    OSL_ENSURE(cDecimalDelimiter && nType != DataType::INTEGER ||
                               !cDecimalDelimiter && nType == DataType::INTEGER,
                               "FalscherTyp");

                    // In Standard-Notation (DezimalPUNKT ohne Tausender-Komma) umwandeln:
                    for (xub_StrLen j = 0; j < aStr.Len(); ++j)
                    {
                        if (cDecimalDelimiter && aStr.GetChar(j) == cDecimalDelimiter)
                            aStrConverted += '.';
                        else if ( aStr.GetChar(j) == '.' ) // special case, if decimal seperator isn't '.' we have to vut the string after it
                            break; // #99189# OJ
                        else if (cThousandDelimiter && aStr.GetChar(j) == cThousandDelimiter)
                        {
                            // weglassen
                        }
                        else
                            aStrConverted += aStr.GetChar(j) ;
                    }
                    double nVal = ::rtl::math::stringToDouble(aStrConverted.GetBuffer(),',','.',NULL,NULL);

                    // #99178# OJ
                    if ( DataType::DECIMAL == nType || DataType::NUMERIC == nType )
                        *(_rRow->get())[i+1] = ORowSetValue(String::CreateFromDouble(nVal));
                    else
                        *(_rRow->get())[i+1] = nVal;
                } break;

                default:
                {
                    // Wert als String in Variable der Row uebernehmen
                    *(_rRow->get())[i+1] = ORowSetValue(aStr);
                }
                break;
            }
        }
    }
    return sal_True;
}
// -----------------------------------------------------------------------------
sal_Bool OEvoabTable::setColumnAliases()
{

    size_t nSize = m_aColumnRawNames.size();
    if(nSize == 0 || m_aPrecisions.size() != nSize || m_aScales.size() != nSize || m_aTypes.size() != nSize)
        return sal_False;
    m_aColumns->get().clear();
    m_aColumns->get().reserve(nSize);
    ::rtl::OUString  aColumnReadName;
    ::rtl::OUString  aColumnHeadlineName;
    ::rtl::OUString  aColumnDisplayName;
    ::rtl::OUString  aColumnFinalName;
    String           sColumnFinalName;

    const TStringVector& colAliasNames = ((OEvoabConnection*)m_pConnection)->getColumnAlias().getAlias();
    const ::std::map< ::rtl::OUString, ::rtl::OUString > & colMap = ((OEvoabConnection*)m_pConnection)->getColumnAlias().getAliasMap();
    ::osl::MutexGuard aGuard( m_aMutex );

    for(size_t i = 0;i < nSize; ++i)
    {
        aColumnReadName = m_aColumnRawNames[i];
        //OSL_TRACE("OEvoabTable::getColumnRows()::aColumnReadName = %s\n", ((OUtoCStr(aColumnReadName)) ? (OUtoCStr(aColumnReadName)):("NULL")) );
        sal_Bool bFound = sal_False;
        for ( size_t j = 0; j < colAliasNames.size(); ++j )
        {
            aColumnDisplayName = colAliasNames[j];

            ::std::map< ::rtl::OUString, ::rtl::OUString >::const_iterator aPos = colMap.find( aColumnDisplayName );
            if ( colMap.end() != aPos )
            {
                aColumnHeadlineName = aPos->second;
                EVO_TRACE_STRING( "OEvoabTable::getColumnRows()::aColumnDisplayName = %s\n", aColumnDisplayName );
                EVO_TRACE_STRING( "OEvoabTable::getColumnRows()::aColumnHeadlineName= %s\n", aColumnHeadlineName );
                if(aColumnReadName == aColumnHeadlineName)
                {
                    //OSL_TRACE("OEvoabTable::getColumnRows()::aColumnHeadlineName = %s\n", ((OUtoCStr(aColumnHeadlineName)) ? (OUtoCStr(aColumnHeadlineName)):("NULL")) );
                    //OSL_TRACE("OEvoabTable::getColumnRows()::aColumnDisplayName = %s\n", ((OUtoCStr(aColumnDisplayName)) ? (OUtoCStr(aColumnDisplayName)):("NULL")) );
                    aColumnFinalName = aColumnDisplayName;
                    bFound = sal_True;
                    //OSL_TRACE("OEvoabTable::getColumnRows()::j = %d\n", j );

                    break;
                }
            }
            else
                OSL_ENSURE( sal_False, "OEvoabTable::setColumnAliases: did not find one of the aliases!" );
        }
        if(!bFound)
            aColumnFinalName = aColumnReadName;
        sColumnFinalName = aColumnFinalName;

        sal_Bool bCase = getConnection()->getMetaData()->supportsMixedCaseQuotedIdentifiers();
        ::rtl::OUString aTypeName;
        aTypeName = ::rtl::OUString::createFromAscii("VARCHAR");
        sdbcx::OColumn* pColumn = new sdbcx::OColumn(sColumnFinalName,aTypeName,::rtl::OUString(),
                                                ColumnValue::NULLABLE,
                                                m_aPrecisions[i],
                                                m_aScales[i],
                                                m_aTypes[i],
                                                sal_False,
                                                sal_False,
                                                sal_False,
                                                bCase);
        Reference< XPropertySet> xCol = pColumn;
        m_aColumns->get().push_back(xCol);
    }
    return sal_True;
}
// -----------------------------------------------------------------------------
void OEvoabTable::refreshIndexes()
{
}
// -----------------------------------------------------------------------------
sal_Bool OEvoabTable::checkHeaderLine()
{
    if (m_nFilePos == 0 && ((OEvoabConnection*)m_pConnection)->isHeaderLine())
    {
        sal_Bool bRead2;
        do
        {
            bRead2 = m_pFileStream->ReadByteStringLine(m_aCurrentLine,m_pConnection->getTextEncoding());
        }
        while(bRead2 && !m_aCurrentLine.Len());

        m_nFilePos = m_pFileStream->Tell();
        if (m_pFileStream->IsEof())
            return sal_False;
    }
    return sal_True;
}
//------------------------------------------------------------------
sal_Bool OEvoabTable::seekRow(IResultSetHelper::Movement eCursorPosition, sal_Int32 nOffset, sal_Int32& nCurPos)
{
    //OSL_TRACE("OEvoabTable::(before SeekRow)m_aCurrentLine = %d\n", ((OUtoCStr(::rtl::OUString(m_aCurrentLine))) ? (OUtoCStr(::rtl::OUString(m_aCurrentLine))):("NULL")) );

    if ( !m_pFileStream )
        return sal_False;
    OEvoabConnection* pConnection = (OEvoabConnection*)m_pConnection;
    // ----------------------------------------------------------
    // Positionierung vorbereiten:
    //OSL_TRACE("OEvoabTable::(before SeekRow,m_pFileStriam Exist)m_aCurrentLine = %d\n", ((OUtoCStr(::rtl::OUString(m_aCurrentLine))) ? (OUtoCStr(::rtl::OUString(m_aCurrentLine))):("NULL")) );

    m_nFilePos = nCurPos;

    switch(eCursorPosition)
    {
        case IResultSetHelper::FIRST:
            m_nFilePos = 0;
            m_nRowPos = 1;
            // run through
        case IResultSetHelper::NEXT:
            if(eCursorPosition != IResultSetHelper::FIRST)
                ++m_nRowPos;
            m_pFileStream->Seek(m_nFilePos);
            if (m_pFileStream->IsEof() || !checkHeaderLine())
            {
                m_nMaxRowCount = m_nRowPos;
                return sal_False;
            }

            m_aRowToFilePos.insert(::std::map<sal_Int32,sal_Int32>::value_type(m_nRowPos,m_nFilePos));

            m_pFileStream->ReadByteStringLine(m_aCurrentLine,pConnection->getTextEncoding());
            if (m_pFileStream->IsEof())
            {
                m_nMaxRowCount = m_nRowPos;
                return sal_False;
            }
            nCurPos = m_pFileStream->Tell();
            break;
        case IResultSetHelper::PRIOR:
            --m_nRowPos;
            if(m_nRowPos > 0)
            {
                m_nFilePos = m_aRowToFilePos.find(m_nRowPos)->second;
                m_pFileStream->Seek(m_nFilePos);
                if (m_pFileStream->IsEof() || !checkHeaderLine())
                    return sal_False;
                m_pFileStream->ReadByteStringLine(m_aCurrentLine,pConnection->getTextEncoding());
                if (m_pFileStream->IsEof())
                    return sal_False;
                nCurPos = m_pFileStream->Tell();
            }
            else
                m_nRowPos = 0;

            break;
        case IResultSetHelper::LAST:
            if(m_nMaxRowCount)
            {
                m_nFilePos = m_aRowToFilePos.rbegin()->second;
                m_nRowPos  = m_aRowToFilePos.rbegin()->first;
                m_pFileStream->Seek(m_nFilePos);
                if (m_pFileStream->IsEof() || !checkHeaderLine())
                    return sal_False;
                m_pFileStream->ReadByteStringLine(m_aCurrentLine,pConnection->getTextEncoding());
                if (m_pFileStream->IsEof())
                    return sal_False;
                nCurPos = m_pFileStream->Tell();
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
                ::std::map<sal_Int32,sal_Int32>::const_iterator aIter = m_aRowToFilePos.find(nOffset);
                if(aIter != m_aRowToFilePos.end())
                {
                    m_nFilePos = aIter->second;
                    m_pFileStream->Seek(m_nFilePos);
                    if (m_pFileStream->IsEof() || !checkHeaderLine())
                        return sal_False;
                    m_pFileStream->ReadByteStringLine(m_aCurrentLine,pConnection->getTextEncoding());
                    if (m_pFileStream->IsEof())
                        return sal_False;
                    nCurPos = m_pFileStream->Tell();
                }
                else if(m_nMaxRowCount && nOffset > m_nMaxRowCount) // offset is outside the table
                {
                    m_nRowPos = m_nMaxRowCount;
                    return sal_False;
                }
                else
                {
                    aIter = m_aRowToFilePos.upper_bound(nOffset);
                    if(aIter == m_aRowToFilePos.end())
                    {
                        m_nRowPos   = m_aRowToFilePos.rbegin()->first;
                        nCurPos = m_nFilePos = m_aRowToFilePos.rbegin()->second;
                        while(m_nRowPos != nOffset)
                            seekRow(IResultSetHelper::NEXT,1,nCurPos);
                    }
                    else
                    {
                        --aIter;
                        m_nRowPos   = aIter->first;
                        m_nFilePos  = aIter->second;
                        m_pFileStream->Seek(m_nFilePos);
                        if (m_pFileStream->IsEof() || !checkHeaderLine())
                            return sal_False;
                        m_pFileStream->ReadByteStringLine(m_aCurrentLine,pConnection->getTextEncoding());
                        if (m_pFileStream->IsEof())
                            return sal_False;
                        nCurPos = m_pFileStream->Tell();
                    }
                }
            }

            break;
        case IResultSetHelper::BOOKMARK:
            m_pFileStream->Seek(nOffset);
            if (m_pFileStream->IsEof())
                return sal_False;

            m_nFilePos = m_pFileStream->Tell(); // Byte-Position in der Datei merken (am ZeilenANFANG)
            m_pFileStream->ReadByteStringLine(m_aCurrentLine,pConnection->getTextEncoding());
            if (m_pFileStream->IsEof())
                return sal_False;
            nCurPos  = m_pFileStream->Tell();
            break;
    }

    //OSL_TRACE("OEvoabTable::(after SeekRow)m_aCurrentLine = %d\n", ((OUtoCStr(::rtl::OUString(m_aCurrentLine))) ? (OUtoCStr(::rtl::OUString(m_aCurrentLine))):("NULL")) );

    return sal_True;
}
// -----------------------------------------------------------------------------
