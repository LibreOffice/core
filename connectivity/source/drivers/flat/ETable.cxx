/*************************************************************************
 *
 *  $RCSfile: ETable.cxx,v $
 *
 *  $Revision: 1.45 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:04:35 $
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

#include <ctype.h>

#ifndef _CONNECTIVITY_FLAT_TABLE_HXX_
#include "flat/ETable.hxx"
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
#ifndef _CONNECTIVITY_FLAT_ECONNECTION_HXX_
#include "flat/EConnection.hxx"
#endif
#ifndef _CONNECTIVITY_FLAT_COLUMNS_HXX_
#include "flat/EColumns.hxx"
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif
#ifndef _CONFIG_HXX
#include <tools/config.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _INTN_HXX //autogen
#include <tools/intn.hxx>
#endif
#ifndef _ZFORLIST_HXX //autogen
#include <svtools/zforlist.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif
#include <stdio.h>      //sprintf
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COMPHELPER_NUMBERS_HXX_
#include <comphelper/numbers.hxx>
#endif
#ifndef _CONNECTIVITY_FLAT_EDRIVER_HXX_
#include "flat/EDriver.hxx"
#endif
#ifndef _COM_SUN_STAR_UTIL_NUMBERFORMAT_HPP_
#include <com/sun/star/util/NumberFormat.hpp>
#endif
#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#ifndef _DBHELPER_DBCONVERSION_HXX_
#include "connectivity/dbconversion.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::flat;
using namespace connectivity::file;
using namespace ucb;
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
void OFlatTable::fillColumns()
{
    BOOL bRead = TRUE;

    OFlatString aHeaderLine;
    OFlatConnection* pConnection = (OFlatConnection*)m_pConnection;
    if (pConnection->isHeaderLine())
    {
        while(bRead && !aHeaderLine.Len())
        {
            bRead = m_pFileStream->ReadByteStringLine(aHeaderLine,pConnection->getTextEncoding());
        }
    }

    // read first row
    OFlatString aFirstLine;

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
        m_aColumns->clear();

    m_aTypes.clear();
    m_aPrecisions.clear();
    m_aScales.clear();
    // reserve some space
    m_aColumns->reserve(nFieldCount);
    m_aTypes.reserve(nFieldCount);
    m_aPrecisions.reserve(nFieldCount);
    m_aScales.reserve(nFieldCount);

    sal_Bool bCase = getConnection()->getMetaData()->storesMixedCaseQuotedIdentifiers();
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
        sal_Int32 eType;
        UINT16 nPrecision = 0;
        UINT16 nScale = 0;

        BOOL bNumeric = FALSE;
        double nRes=0.0;
        ULONG  nIndex = 0;

        // first without fielddelimiter
        String aField;
        aFirstLine.GetTokenSpecial(aField,nStartPosFirstLine,pConnection->getFieldDelimiter(),'\0');
        if (aField.Len() == 0 ||
            (pConnection->getStringDelimiter() && pConnection->getStringDelimiter() == aField.GetChar(0)))
        {
            bNumeric = FALSE;
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
                bNumeric = FALSE;
            }
            else
            {
                bNumeric = TRUE;
                xub_StrLen nDot = 0;
                for (xub_StrLen j = 0; j < aField2.Len(); j++)
                {
                    sal_Unicode c = aField2.GetChar(j);
                    // nur Ziffern und Dezimalpunkt und Tausender-Trennzeichen?
                    if ((!cDecimalDelimiter || c != cDecimalDelimiter) &&
                        (!cThousandDelimiter || c != cThousandDelimiter) &&
                        !isdigit(c))
                    {
                        bNumeric = FALSE;
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
                    bNumeric = FALSE;
                if (bNumeric && cThousandDelimiter)
                {
                    // Ist der Trenner richtig angegeben?
                    String aValue = aField2.GetToken(0,cDecimalDelimiter);
                    for (sal_Int32 j = aValue.Len() - 4; j >= 0; j -= 4)
                    {
                        sal_Unicode c = aValue.GetChar(static_cast<sal_uInt16>(j));
                        // nur Ziffern und Dezimalpunkt und Tausender-Trennzeichen?
                        if (c == cThousandDelimiter && j)
                            continue;
                        else
                        {
                            bNumeric = FALSE;
                            break;
                        }
                    }
                }

                // jetzt könnte es noch ein Datumsfeld sein
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
        OSQLColumns::const_iterator aFind = connectivity::find(m_aColumns->begin(),m_aColumns->end(),aAlias,aCase);
        sal_Int32 nExprCnt = 0;
        while(aFind != m_aColumns->end())
        {
            (aAlias = aColumnName) += String::CreateFromInt32(++nExprCnt);
            aFind = connectivity::find(m_aColumns->begin(),m_aColumns->end(),aAlias,aCase);
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
        m_aColumns->push_back(xCol);
        m_aTypes.push_back(eType);
        m_aPrecisions.push_back(nPrecision);
        m_aScales.push_back(nScale);
    }
    m_pFileStream->Seek(STREAM_SEEK_TO_BEGIN);
}
// -------------------------------------------------------------------------
OFlatTable::OFlatTable(sdbcx::OCollection* _pTables,OFlatConnection* _pConnection) : OFlatTable_BASE(_pTables,_pConnection)
{

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
    ,m_nRowPos(0)
    ,m_nMaxRowCount(0)
{

}
// -----------------------------------------------------------------------------
void OFlatTable::construct()
{
    Any aValue = ConfigManager::GetDirectConfigProperty(ConfigManager::LOCALE);
    LanguageType eLanguage = ConvertIsoStringToLanguage(comphelper::getString(aValue),'-');

    String sLanguage, sCountry;
    ConvertLanguageToIsoNames(eLanguage, sLanguage, sCountry);
    ::com::sun::star::lang::Locale aAppLocale(sLanguage,sCountry,rtl::OUString());
    Sequence< ::com::sun::star::uno::Any > aArg(1);
    aArg[0] <<= aAppLocale;

    Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xSupplier(m_pConnection->getDriver()->getFactory()->createInstanceWithArguments(::rtl::OUString::createFromAscii("com.sun.star.util.NumberFormatsSupplier"),aArg),UNO_QUERY);
    m_xNumberFormatter = Reference< ::com::sun::star::util::XNumberFormatter >(m_pConnection->getDriver()->getFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.util.NumberFormatter")),UNO_QUERY);
    m_xNumberFormatter->attachNumberFormatsSupplier(xSupplier);

    INetURLObject aURL;
    aURL.SetURL(getEntry());

    if(aURL.getExtension() != m_pConnection->getExtension())
        aURL.setExtension(m_pConnection->getExtension());

    //  Content aContent(aURL.GetMainURL());

    String aFileName = aURL.GetMainURL(INetURLObject::NO_DECODE);

    m_pFileStream = createStream_simpleError( aFileName,STREAM_READWRITE | STREAM_NOCREATE | STREAM_SHARE_DENYWRITE);

    if(!m_pFileStream)
        m_pFileStream = createStream_simpleError( aFileName,STREAM_READ | STREAM_NOCREATE | STREAM_SHARE_DENYNONE);

    if(m_pFileStream)
    {
        m_pFileStream->Seek(STREAM_SEEK_TO_END);
        sal_Int32 nSize = m_pFileStream->Tell();
        m_pFileStream->Seek(STREAM_SEEK_TO_BEGIN);

        // Buffersize abhaengig von der Filegroesse
        m_pFileStream->SetBufferSize(nSize > 1000000 ? 32768 :
                                    nSize > 100000  ? 16384 :
                                    nSize > 10000   ? 4096  : 1024);

        fillColumns();

        refreshColumns();
    }
}
// -------------------------------------------------------------------------
String OFlatTable::getEntry()
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
void OFlatTable::refreshColumns()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    TStringVector aVector;
    aVector.reserve(m_aColumns->size());

    for(OSQLColumns::const_iterator aIter = m_aColumns->begin();aIter != m_aColumns->end();++aIter)
        aVector.push_back(Reference< XNamed>(*aIter,UNO_QUERY)->getName());

    if(m_pColumns)
        m_pColumns->reFill(aVector);
    else
        m_pColumns  = new OFlatColumns(this,m_aMutex,aVector);
}

// -------------------------------------------------------------------------
void SAL_CALL OFlatTable::disposing(void)
{
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
    return Sequence< Type >(aOwnTypes.begin(),aOwnTypes.size());
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
    return (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
                ?
            (sal_Int64)this
                :
            OFlatTable_BASE::getSomething(rId);
}
//------------------------------------------------------------------
sal_Bool OFlatTable::fetchRow(OValueRefRow& _rRow,const OSQLColumns & _rCols,sal_Bool bIsTable,sal_Bool bRetrieveData)
{
    *(*_rRow)[0] = m_nFilePos;

    if (!bRetrieveData)
        return TRUE;

    OFlatConnection* pConnection = (OFlatConnection*)m_pConnection;
    sal_Int32 nByteOffset = 1;
    // Felder:
    xub_StrLen nStartPos = 0;
    String aStr;
    OSQLColumns::const_iterator aIter = _rCols.begin();
    for (sal_Int32 i = 0; aIter != _rCols.end();++aIter, ++i)
    {
        m_aCurrentLine.GetTokenSpecial(aStr,nStartPos,pConnection->getFieldDelimiter(),pConnection->getStringDelimiter());

        if (aStr.Len() == 0)
            (*_rRow)[i+1]->setNull();
        else
        {
            // Laengen je nach Datentyp:
            sal_Int32   nLen,
                        nType;
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
                                *(*_rRow)[i+1] = ::dbtools::DBTypeConversion::toDouble(::dbtools::DBTypeConversion::toDate(nRes,aDate));
                                break;
                            case DataType::TIMESTAMP:
                                *(*_rRow)[i+1] = ::dbtools::DBTypeConversion::toDouble(::dbtools::DBTypeConversion::toDateTime(nRes,aDate));
                                break;
                            default:
                                *(*_rRow)[i+1] = ::dbtools::DBTypeConversion::toDouble(::dbtools::DBTypeConversion::toTime(nRes));
                        }
                    }
                    catch(Exception&)
                    {
                        (*_rRow)[i+1]->setNull();
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
                            continue; // #99189# OJ
                        else if (cThousandDelimiter && aStr.GetChar(j) == cThousandDelimiter)
                        {
                            // weglassen
                        }
                        else
                            aStrConverted += aStr.GetChar(j) ;
                    }
                    double nVal = ::rtl::math::stringToDouble(aStrConverted,'.',',',NULL,NULL);

                    // #99178# OJ
                    if ( DataType::DECIMAL == nType || DataType::NUMERIC == nType )
                        *(*_rRow)[i+1] = ORowSetValue(String::CreateFromDouble(nVal));
                    else
                        *(*_rRow)[i+1] = nVal;
                } break;

                default:
                {
                    // Wert als String in Variable der Row uebernehmen
                    *(*_rRow)[i+1] = ORowSetValue(aStr);
                }
                break;
            }
        }
    }
    return sal_True;
}

// -----------------------------------------------------------------------------



