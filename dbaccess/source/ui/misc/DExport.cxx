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

#include "DExport.hxx"
#include "moduledbu.hxx"

#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XAppend.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include "dbustrings.hrc"
#include "dbu_misc.hrc"
#include <connectivity/dbconversion.hxx>
#include <sfx2/sfxhtml.hxx>
#include <svl/numuno.hxx>
#include <connectivity/dbtools.hxx>
#include <comphelper/extract.hxx>
#include "TypeInfo.hxx"
#include "FieldDescriptions.hxx"
#include "UITools.hxx"
#include <unotools/configmgr.hxx>
#include <memory>
#include <o3tl/compat_functional.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <tools/contnr.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include "WCopyTable.hxx"
#include "WExtendPages.hxx"
#include "WCPage.hxx"
#include <unotools/syslocale.hxx>
#include <svl/zforlist.hxx>
#include <connectivity/dbexception.hxx>
#include <connectivity/FValue.hxx>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdb/application/CopyTableOperation.hpp>
#include "sqlmessage.hxx"
#include "UpdateHelperImpl.hxx"
#include <vcl/msgbox.hxx>
#include <cppuhelper/exc_hlp.hxx>

using namespace dbaui;
using namespace utl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::awt;

namespace CopyTableOperation = ::com::sun::star::sdb::application::CopyTableOperation;

// ODatabaseExport
DBG_NAME(ODatabaseExport)
ODatabaseExport::ODatabaseExport(sal_Int32 nRows,
                                 const TPositions &_rColumnPositions,
                                 const Reference< XNumberFormatter >& _rxNumberF,
                                 const Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
                                 const TColumnVector* pList,
                                 const OTypeInfoMap* _pInfoMap,
                                 sal_Bool _bAutoIncrementEnabled,
                                 SvStream& _rInputStream)
    :m_vColumns(_rColumnPositions)
    ,m_aDestColumns(sal_True)
    ,m_xFormatter(_rxNumberF)
    ,m_xContext(_rxContext)
    ,m_pFormatter(NULL)
    ,m_rInputStream( _rInputStream )
    ,m_pTypeInfo()
    ,m_pColumnList(pList)
    ,m_pInfoMap(_pInfoMap)
    ,m_nColumnPos(0)
    ,m_nRows(1)
    ,m_nRowCount(0)
    ,m_nDefToken( osl_getThreadTextEncoding() )
    ,m_bError(sal_False)
    ,m_bInTbl(sal_False)
    ,m_bHead(sal_True)
    ,m_bDontAskAgain(sal_False)
    ,m_bIsAutoIncrement(_bAutoIncrementEnabled)
    ,m_bFoundTable(sal_False)
    ,m_bCheckOnly(sal_False)
    ,m_bAppendFirstLine(false)
{
    SAL_INFO("dbaccess.ui", "ODatabaseExport::ODatabaseExport" );
    DBG_CTOR(ODatabaseExport,NULL);

    m_nRows += nRows;
    sal_Int32 nCount = 0;
    for(sal_Int32 j=0;j < (sal_Int32)m_vColumns.size();++j)
        if ( m_vColumns[j].first != COLUMN_POSITION_NOT_FOUND )
            ++nCount;

    m_vColumnSize.resize(nCount);
    m_vNumberFormat.resize(nCount);
    for(sal_Int32 i=0;i<nCount;++i)
    {
        m_vColumnSize[i] = 0;
        m_vNumberFormat[i] = 0;
    }

    try
    {
        SvtSysLocale aSysLocale;
        m_aLocale = aSysLocale.GetLanguageTag().getLocale();
    }
    catch(Exception&)
    {
    }

    SetColumnTypes(pList,_pInfoMap);
}

ODatabaseExport::ODatabaseExport(const SharedConnection& _rxConnection,
                                 const Reference< XNumberFormatter >& _rxNumberF,
                                 const Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
                                 const TColumnVector* pList,
                                 const OTypeInfoMap* _pInfoMap,
                                 SvStream& _rInputStream)
    :m_aDestColumns(_rxConnection->getMetaData().is() && _rxConnection->getMetaData()->supportsMixedCaseQuotedIdentifiers() == sal_True)
    ,m_xConnection(_rxConnection)
    ,m_xFormatter(_rxNumberF)
    ,m_xContext(_rxContext)
    ,m_pFormatter(NULL)
    ,m_rInputStream( _rInputStream )
    ,m_pTypeInfo()
    ,m_pColumnList(NULL)
    ,m_pInfoMap(NULL)
    ,m_nColumnPos(0)
    ,m_nRows(1)
    ,m_nRowCount(0)
    ,m_nDefToken( osl_getThreadTextEncoding() )
    ,m_bError(sal_False)
    ,m_bInTbl(sal_False)
    ,m_bHead(sal_True)
    ,m_bDontAskAgain(sal_False)
    ,m_bIsAutoIncrement(sal_False)
    ,m_bFoundTable(sal_False)
    ,m_bCheckOnly(sal_False)
    ,m_bAppendFirstLine(false)
{
    SAL_INFO("dbaccess.ui", "ODatabaseExport::ODatabaseExport" );
    DBG_CTOR(ODatabaseExport,NULL);
    try
    {
        SvtSysLocale aSysLocale;
        m_aLocale = aSysLocale.GetLanguageTag().getLocale();
    }
    catch(Exception&)
    {
    }

    Reference<XTablesSupplier> xTablesSup(m_xConnection,UNO_QUERY);
    if(xTablesSup.is())
        m_xTables = xTablesSup->getTables();

    Reference<XDatabaseMetaData> xMeta = m_xConnection->getMetaData();
    Reference<XResultSet> xSet = xMeta.is() ? xMeta->getTypeInfo() : Reference<XResultSet>();
    if(xSet.is())
    {
        ::connectivity::ORowSetValue aValue;
        ::std::vector<sal_Int32> aTypes;
        ::std::vector<sal_Bool> aNullable;
        Reference<XResultSetMetaData> xResultSetMetaData = Reference<XResultSetMetaDataSupplier>(xSet,UNO_QUERY_THROW)->getMetaData();
        Reference<XRow> xRow(xSet,UNO_QUERY_THROW);
        while(xSet->next())
        {
            if ( aTypes.empty() )
            {
                sal_Int32 nCount = xResultSetMetaData->getColumnCount();
                if ( nCount < 1 )
                    nCount = 18;
                aTypes.reserve(nCount+1);
                aNullable.reserve(nCount+1);
                aTypes.push_back(-1);
                aNullable.push_back(sal_False);
                for (sal_Int32 j = 1; j <= nCount ; ++j)
                {
                    aNullable.push_back(xResultSetMetaData->isNullable(j) != ColumnValue::NO_NULLS );
                    aTypes.push_back(xResultSetMetaData->getColumnType(j));
                }
            }

            sal_Int32 nPos = 1;
            OSL_ENSURE((nPos) < static_cast<sal_Int32>(aTypes.size()),"aTypes: Illegal index for vector");
            aValue.fill(nPos,aTypes[nPos],aNullable[nPos],xRow);
            OUString sTypeName = aValue;
            ++nPos;
            OSL_ENSURE((nPos) < static_cast<sal_Int32>(aTypes.size()),"aTypes: Illegal index for vector");
            aValue.fill(nPos,aTypes[nPos],aNullable[nPos],xRow);
            sal_Int32 nType = aValue;
            ++nPos;

            if( nType == DataType::VARCHAR )
            {
                m_pTypeInfo                 = TOTypeInfoSP(new OTypeInfo());

                m_pTypeInfo->aTypeName      = sTypeName;
                m_pTypeInfo->nType          = nType;

                OSL_ENSURE((nPos) < static_cast<sal_Int32>(aTypes.size()),"aTypes: Illegal index for vector");
                aValue.fill(nPos,aTypes[nPos],aNullable[nPos],xRow);
                m_pTypeInfo->nPrecision     = aValue;
                ++nPos;
                aValue.fill(nPos,aTypes[nPos],aNullable[nPos],xRow);
                m_pTypeInfo->aLiteralPrefix = aValue;
                ++nPos;
                aValue.fill(nPos,aTypes[nPos],aNullable[nPos],xRow);
                m_pTypeInfo->aLiteralSuffix = aValue;
                ++nPos;
                aValue.fill(nPos,aTypes[nPos],aNullable[nPos],xRow);
                m_pTypeInfo->aCreateParams  = aValue;
                ++nPos;
                aValue.fill(nPos,aTypes[nPos],aNullable[nPos],xRow);
                m_pTypeInfo->bNullable      = (sal_Int32)aValue == ColumnValue::NULLABLE;
                ++nPos;
                aValue.fill(nPos,aTypes[nPos],aNullable[nPos],xRow);
                m_pTypeInfo->bCaseSensitive = (sal_Bool)aValue;
                ++nPos;
                aValue.fill(nPos,aTypes[nPos],aNullable[nPos],xRow);
                m_pTypeInfo->nSearchType        = aValue;
                ++nPos;
                aValue.fill(nPos,aTypes[nPos],aNullable[nPos],xRow);
                m_pTypeInfo->bUnsigned      = (sal_Bool)aValue;
                ++nPos;
                aValue.fill(nPos,aTypes[nPos],aNullable[nPos],xRow);
                m_pTypeInfo->bCurrency      = (sal_Bool)aValue;
                ++nPos;
                aValue.fill(nPos,aTypes[nPos],aNullable[nPos],xRow);
                m_pTypeInfo->bAutoIncrement = (sal_Bool)aValue;
                ++nPos;
                aValue.fill(nPos,aTypes[nPos],aNullable[nPos],xRow);
                m_pTypeInfo->aLocalTypeName = aValue;
                ++nPos;
                aValue.fill(nPos,aTypes[nPos],aNullable[nPos],xRow);
                m_pTypeInfo->nMinimumScale  = aValue;
                ++nPos;
                aValue.fill(nPos,aTypes[nPos],aNullable[nPos],xRow);
                m_pTypeInfo->nMaximumScale  = aValue;

                // check if values are less than zero like it happens in a oracle jdbc driver
                if( m_pTypeInfo->nPrecision < 0)
                    m_pTypeInfo->nPrecision = 0;
                if( m_pTypeInfo->nMinimumScale < 0)
                    m_pTypeInfo->nMinimumScale = 0;
                if( m_pTypeInfo->nMaximumScale < 0)
                    m_pTypeInfo->nMaximumScale = 0;
                break;
            }
        }
    }
    if ( !m_pTypeInfo )
        m_pTypeInfo = TOTypeInfoSP(new OTypeInfo());
    SetColumnTypes(pList,_pInfoMap);
}

ODatabaseExport::~ODatabaseExport()
{
    DBG_DTOR(ODatabaseExport,NULL);
    m_pFormatter = NULL;
    ODatabaseExport::TColumns::iterator aIter = m_aDestColumns.begin();
    ODatabaseExport::TColumns::iterator aEnd  = m_aDestColumns.end();

    for(;aIter != aEnd;++aIter)
        delete aIter->second;
    m_vDestVector.clear();
    m_aDestColumns.clear();
}

void ODatabaseExport::insertValueIntoColumn()
{
    SAL_INFO("dbaccess.ui", "ODatabaseExport::insertValueIntoColumn" );
    DBG_CHKTHIS(ODatabaseExport,NULL);
    if(m_nColumnPos < sal_Int32(m_vDestVector.size()))
    {
        OFieldDescription* pField = m_vDestVector[m_nColumnPos]->second;
        if(pField)
        {
            sal_Int32 nNewPos = m_bIsAutoIncrement ? m_nColumnPos+1 : m_nColumnPos;
            OSL_ENSURE((nNewPos) < static_cast<sal_Int32>(m_vColumns.size()),"m_vColumns: Illegal index for vector");

            if ( (nNewPos) < static_cast<sal_Int32>(m_vColumns.size() ) )
            {
                sal_Int32 nPos = m_vColumns[nNewPos].first;
                if ( nPos != COLUMN_POSITION_NOT_FOUND )
                {
                    if ( !m_sTextToken.Len() && pField->IsNullable() )
                        m_pUpdateHelper->updateNull(nPos,pField->GetType());
                    else
                    {
                        OSL_ENSURE((nNewPos) < static_cast<sal_Int32>(m_vColumnTypes.size()),"Illegal index for vector");
                        if (m_vColumnTypes[nNewPos] != DataType::VARCHAR && m_vColumnTypes[nNewPos] != DataType::CHAR && m_vColumnTypes[nNewPos] != DataType::LONGVARCHAR )
                        {
                            SAL_INFO("dbaccess", "ODatabaseExport::insertValueIntoColumn != DataType::VARCHAR" );
                            ensureFormatter();
                            sal_Int32 nNumberFormat = 0;
                            double fOutNumber = 0.0;
                            bool bNumberFormatError = false;
                            if ( m_pFormatter && m_sNumToken.Len() )
                            {
                                LanguageType eNumLang = LANGUAGE_NONE;
                                sal_uInt32 nNumberFormat2( nNumberFormat );
                                fOutNumber = SfxHTMLParser::GetTableDataOptionsValNum(nNumberFormat2,eNumLang,m_sTextToken,m_sNumToken,*m_pFormatter);
                                if ( eNumLang != LANGUAGE_NONE )
                                {
                                    nNumberFormat2 = m_pFormatter->GetFormatForLanguageIfBuiltIn( nNumberFormat2, eNumLang );
                                    m_pFormatter->IsNumberFormat( m_sTextToken, nNumberFormat2, fOutNumber );
                                }
                                nNumberFormat = static_cast<sal_Int32>(nNumberFormat2);
                            }
                            else
                            {
                                Reference< XNumberFormatsSupplier >  xSupplier = m_xFormatter->getNumberFormatsSupplier();
                                Reference<XNumberFormatTypes> xNumType(xSupplier->getNumberFormats(),UNO_QUERY);
                                sal_Int16 nFormats[] = {
                                    NumberFormat::DATETIME
                                    ,NumberFormat::DATE
                                    ,NumberFormat::TIME
                                    ,NumberFormat::CURRENCY
                                    ,NumberFormat::NUMBER
                                    ,NumberFormat::LOGICAL
                                };
                                for (size_t i = 0; i < sizeof(nFormats)/sizeof(nFormats[0]); ++i)
                                {
                                    try
                                    {
                                        nNumberFormat = m_xFormatter->detectNumberFormat(xNumType->getStandardFormat(nFormats[i],m_aLocale),m_sTextToken);
                                        break;
                                    }
                                    catch(Exception&)
                                    {
                                    }
                                }
                                try
                                {
                                    fOutNumber = m_xFormatter->convertStringToNumber(nNumberFormat,m_sTextToken);
                                }
                                catch(Exception&)
                                {
                                    bNumberFormatError = true;
                                    m_pUpdateHelper->updateString(nPos,m_sTextToken);
                                }
                            }
                            if ( !bNumberFormatError )
                            {
                                try
                                {
                                    Reference< XNumberFormatsSupplier >  xSupplier = m_xFormatter->getNumberFormatsSupplier();
                                    Reference< XNumberFormats >         xFormats = xSupplier->getNumberFormats();
                                    Reference<XPropertySet> xProp = xFormats->getByKey(nNumberFormat);
                                    sal_Int16 nType = 0;
                                    xProp->getPropertyValue(PROPERTY_TYPE) >>= nType;
                                    switch(nType)
                                    {
                                        case NumberFormat::DATE:
                                            m_pUpdateHelper->updateDate(nPos,::dbtools::DBTypeConversion::toDate(fOutNumber,m_aNullDate));
                                            break;
                                        case NumberFormat::DATETIME:
                                            m_pUpdateHelper->updateTimestamp(nPos,::dbtools::DBTypeConversion::toDateTime(fOutNumber,m_aNullDate));
                                            break;
                                        case NumberFormat::TIME:
                                            m_pUpdateHelper->updateTime(nPos,::dbtools::DBTypeConversion::toTime(fOutNumber));
                                            break;
                                        default:
                                            m_pUpdateHelper->updateDouble(nPos,fOutNumber);
                                    }
                                }
                                catch(Exception&)
                                {
                                    m_pUpdateHelper->updateString(nPos,m_sTextToken);
                                }
                            }

                        }
                        else
                            m_pUpdateHelper->updateString(nPos,m_sTextToken);
                    }
                }
            }
            eraseTokens();
        }
    }
}

sal_Int16 ODatabaseExport::CheckString(const String& aCheckToken, sal_Int16 _nOldNumberFormat)
{
    SAL_INFO("dbaccess.ui", "ODatabaseExport::CheckString" );
    DBG_CHKTHIS(ODatabaseExport,NULL);
    double fOutNumber = 0.0;
    sal_Int16 nNumberFormat = 0;

    try
    {
        Reference< XNumberFormatsSupplier > xSupplier = m_xFormatter->getNumberFormatsSupplier();
        Reference< XNumberFormats >         xFormats = xSupplier->getNumberFormats();

        ensureFormatter();
        if ( m_pFormatter && m_sNumToken.Len() )
        {
            LanguageType eNumLang;
            sal_uInt32 nFormatKey(0);
            fOutNumber = SfxHTMLParser::GetTableDataOptionsValNum(nFormatKey,eNumLang,m_sTextToken,m_sNumToken,*m_pFormatter);
            if ( eNumLang != LANGUAGE_NONE )
            {
                nFormatKey = m_pFormatter->GetFormatForLanguageIfBuiltIn( nFormatKey, eNumLang );
                if ( !m_pFormatter->IsNumberFormat( m_sTextToken, nFormatKey, fOutNumber ) )
                    return NumberFormat::TEXT;
            }
            Reference<XPropertySet> xProp = xFormats->getByKey(nFormatKey);
            xProp->getPropertyValue(PROPERTY_TYPE) >>= nNumberFormat;
        }
        else
        {
            Reference<XNumberFormatTypes> xNumType(xFormats,UNO_QUERY);
            sal_Int32 nFormatKey = m_xFormatter->detectNumberFormat(xNumType->getStandardFormat(NumberFormat::ALL,m_aLocale),aCheckToken);
            fOutNumber = m_xFormatter->convertStringToNumber(nFormatKey,aCheckToken);

            Reference<XPropertySet> xProp = xFormats->getByKey(nFormatKey);
            sal_Int16 nType = 0;
            xProp->getPropertyValue(PROPERTY_TYPE) >>= nType;

            switch(nType)
            {
                case NumberFormat::ALL:
                    nNumberFormat = NumberFormat::ALL;
                    break;
                case NumberFormat::DEFINED:
                    nNumberFormat = NumberFormat::TEXT;
                    break;
                case NumberFormat::DATE:
                    switch(_nOldNumberFormat)
                    {
                        case NumberFormat::DATETIME:
                        case NumberFormat::TEXT:
                        case NumberFormat::DATE:
                            nNumberFormat = _nOldNumberFormat;
                            break;
                        case NumberFormat::ALL:
                            nNumberFormat = NumberFormat::DATE;
                            break;
                        default:
                            nNumberFormat = NumberFormat::TEXT;

                    }
                    break;
                case NumberFormat::TIME:
                    switch(_nOldNumberFormat)
                    {
                        case NumberFormat::DATETIME:
                        case NumberFormat::TEXT:
                        case NumberFormat::TIME:
                            nNumberFormat = _nOldNumberFormat;
                            break;
                        case NumberFormat::ALL:
                            nNumberFormat = NumberFormat::TIME;
                            break;
                        default:
                            nNumberFormat = NumberFormat::TEXT;
                            break;
                    }
                    break;
                case NumberFormat::CURRENCY:
                    switch(_nOldNumberFormat)
                    {
                        case NumberFormat::NUMBER:
                            nNumberFormat = NumberFormat::CURRENCY;
                            break;
                        case NumberFormat::CURRENCY:
                            nNumberFormat = _nOldNumberFormat;
                            break;
                        case NumberFormat::ALL:
                            nNumberFormat = NumberFormat::CURRENCY;
                            break;
                        default:
                            nNumberFormat = NumberFormat::TEXT;
                            break;
                    }
                    break;
                case NumberFormat::NUMBER:
                case NumberFormat::SCIENTIFIC:
                case NumberFormat::FRACTION:
                case NumberFormat::PERCENT:
                    switch(_nOldNumberFormat)
                    {
                        case NumberFormat::NUMBER:
                            nNumberFormat = _nOldNumberFormat;
                            break;
                        case NumberFormat::CURRENCY:
                            nNumberFormat = NumberFormat::CURRENCY;
                            break;
                        case NumberFormat::ALL:
                            nNumberFormat = nType;
                            break;
                        default:
                            nNumberFormat = NumberFormat::TEXT;
                            break;
                    }
                    break;
                case NumberFormat::TEXT:
                case NumberFormat::UNDEFINED:
                case NumberFormat::LOGICAL:
                    nNumberFormat = NumberFormat::TEXT; // Text "uberschreibt alles
                    break;
                case NumberFormat::DATETIME:
                    switch(_nOldNumberFormat)
                    {
                        case NumberFormat::DATETIME:
                        case NumberFormat::TEXT:
                        case NumberFormat::TIME:
                            nNumberFormat = _nOldNumberFormat;
                            break;
                        case NumberFormat::ALL:
                            nNumberFormat = NumberFormat::DATETIME;
                            break;
                        default:
                            nNumberFormat = NumberFormat::TEXT;
                            break;
                    }
                    break;
                default:
                    SAL_WARN("dbaccess.ui", "ODatabaseExport: Unbekanntes Format");
            }
        }
    }
    catch(Exception&)
    {
        nNumberFormat = NumberFormat::TEXT; // Text "uberschreibt alles
    }

    return nNumberFormat;
}

void ODatabaseExport::SetColumnTypes(const TColumnVector* _pList,const OTypeInfoMap* _pInfoMap)
{
    SAL_INFO("dbaccess.ui", "ODatabaseExport::SetColumnTypes" );
    DBG_CHKTHIS(ODatabaseExport,NULL);
    if(_pList && _pInfoMap)
    {
        OSL_ENSURE(m_vNumberFormat.size() == m_vColumnSize.size() && m_vColumnSize.size() == _pList->size(),"Illegal columns in list");
        Reference< XNumberFormatsSupplier > xSupplier = m_xFormatter->getNumberFormatsSupplier();
        Reference< XNumberFormats >         xFormats = xSupplier->getNumberFormats();
        TColumnVector::const_iterator aIter = _pList->begin();
        TColumnVector::const_iterator aEnd = _pList->end();
        for(sal_Int32 i= 0;aIter != aEnd && (i) < static_cast<sal_Int32>(m_vNumberFormat.size()) && (i) < static_cast<sal_Int32>(m_vColumnSize.size()) ;++aIter,++i)
        {
            sal_Int32 nDataType;
            sal_Int32 nLength(0),nScale(0);
            sal_Int16 nType = m_vNumberFormat[i] & ~NumberFormat::DEFINED;

            switch ( nType )
            {
                case NumberFormat::ALL:
                    nDataType  = DataType::DOUBLE;
                    break;
                case NumberFormat::DEFINED:
                    nDataType   = DataType::VARCHAR;
                    nLength     = ((m_vColumnSize[i] % 10 ) ? m_vColumnSize[i]/ 10 + 1: m_vColumnSize[i]/ 10) * 10;
                    break;
                case NumberFormat::DATE:
                    nDataType  = DataType::DATE;
                    break;
                case NumberFormat::TIME:
                    nDataType  = DataType::TIME;
                    break;
                case NumberFormat::DATETIME:
                    nDataType  = DataType::TIMESTAMP;
                    break;
                case NumberFormat::CURRENCY:
                    nDataType  = DataType::NUMERIC;
                    nScale      = 4;
                    nLength     = 19;
                    break;
                case NumberFormat::NUMBER:
                case NumberFormat::SCIENTIFIC:
                case NumberFormat::FRACTION:
                case NumberFormat::PERCENT:
                    nDataType  = DataType::DOUBLE;
                    break;
                case NumberFormat::TEXT:
                case NumberFormat::UNDEFINED:
                case NumberFormat::LOGICAL:
                default:
                    nDataType  = DataType::VARCHAR;
                    nLength     = ((m_vColumnSize[i] % 10 ) ? m_vColumnSize[i]/ 10 + 1: m_vColumnSize[i]/ 10) * 10;
                    break;
            }
            OTypeInfoMap::const_iterator aFind = _pInfoMap->find(nDataType);
            if(aFind != _pInfoMap->end())
            {
                (*aIter)->second->SetType(aFind->second);
                (*aIter)->second->SetPrecision(::std::min<sal_Int32>(aFind->second->nPrecision,nLength));
                (*aIter)->second->SetScale(::std::min<sal_Int32>(aFind->second->nMaximumScale,nScale));

                sal_Int32 nFormatKey = ::dbtools::getDefaultNumberFormat( nDataType,
                                    (*aIter)->second->GetScale(),
                                    (*aIter)->second->IsCurrency(),
                                    Reference< XNumberFormatTypes>(xFormats,UNO_QUERY),
                                    m_aLocale);

                (*aIter)->second->SetFormatKey(nFormatKey);
            }
        }
    }
}

void ODatabaseExport::CreateDefaultColumn(const OUString& _rColumnName)
{
    SAL_INFO("dbaccess.ui", "ODatabaseExport::CreateDefaultColumn" );
    DBG_CHKTHIS(ODatabaseExport,NULL);
    Reference< XDatabaseMetaData>  xDestMetaData(m_xConnection->getMetaData());
    sal_Int32 nMaxNameLen(xDestMetaData->getMaxColumnNameLength());
    OUString aAlias = _rColumnName;
    if ( isSQL92CheckEnabled(m_xConnection) )
        aAlias = ::dbtools::convertName2SQLName(_rColumnName,xDestMetaData->getExtraNameCharacters());

    if(nMaxNameLen && aAlias.getLength() > nMaxNameLen)
        aAlias = aAlias.copy(0, ::std::min<sal_Int32>( nMaxNameLen-1, aAlias.getLength() ) );

    OUString sName(aAlias);
    if(m_aDestColumns.find(sName) != m_aDestColumns.end())
    {
        sal_Int32 nPos = 0;
        sal_Int32 nCount = 2;
        while(m_aDestColumns.find(sName) != m_aDestColumns.end())
        {
            sName = aAlias;
            sName += OUString::number(++nPos);
            if(nMaxNameLen && sName.getLength() > nMaxNameLen)
            {
                aAlias = aAlias.copy(0,::std::min<sal_Int32>( nMaxNameLen-nCount, aAlias.getLength() ));
                sName = aAlias;
                sName += OUString::number(nPos);
                ++nCount;
            }
        }
    }
    aAlias = sName;
    // now create a column
    OFieldDescription* pField = new OFieldDescription();
    pField->SetType(m_pTypeInfo);
    pField->SetName(aAlias);
    pField->SetPrecision(::std::min<sal_Int32>((sal_Int32)255,m_pTypeInfo->nPrecision));
    pField->SetScale(0);
    pField->SetIsNullable(ColumnValue::NULLABLE);
    pField->SetAutoIncrement(sal_False);
    pField->SetPrimaryKey(sal_False);
    pField->SetCurrency(sal_False);

    TColumns::iterator aFind = m_aDestColumns.find( aAlias );
    if ( aFind != m_aDestColumns.end() )
    {
        delete aFind->second;
        m_aDestColumns.erase(aFind);
    }

    m_vDestVector.push_back(m_aDestColumns.insert(TColumns::value_type(aAlias,pField)).first);
}

sal_Bool ODatabaseExport::createRowSet()
{
    SAL_INFO("dbaccess.ui", "ODatabaseExport::createRowSet" );
    DBG_CHKTHIS(ODatabaseExport,NULL);
    m_pUpdateHelper.reset(new OParameterUpdateHelper(createPreparedStatment(m_xConnection->getMetaData(),m_xTable,m_vColumns)));

    return m_pUpdateHelper.get() != NULL;
}

sal_Bool ODatabaseExport::executeWizard(const OUString& _rTableName,const Any& _aTextColor,const FontDescriptor& _rFont)
{
    SAL_INFO("dbaccess.ui", "ODatabaseExport::executeWizard" );
    DBG_CHKTHIS(ODatabaseExport,NULL);

    bool bHaveDefaultTable =  !m_sDefaultTableName.isEmpty();
    OUString sTableName( bHaveDefaultTable ? m_sDefaultTableName : _rTableName );
    OCopyTableWizard aWizard(
        NULL,
        sTableName,
        bHaveDefaultTable ? CopyTableOperation::AppendData : CopyTableOperation::CopyDefinitionAndData,
        m_aDestColumns,
        m_vDestVector,
        m_xConnection,
        m_xFormatter,
        getTypeSelectionPageFactory(),
        m_rInputStream,
        m_xContext
    );

    sal_Bool bError = sal_False;
    try
    {
        if (aWizard.Execute())
        {
            switch(aWizard.getOperation())
            {
                case CopyTableOperation::CopyDefinitionAndData:
                case CopyTableOperation::AppendData:
                    {
                        m_xTable = aWizard.createTable();
                        bError = !m_xTable.is();
                        if(m_xTable.is())
                        {
                            m_xTable->setPropertyValue(PROPERTY_FONT,makeAny(_rFont));
                            if(_aTextColor.hasValue())
                                m_xTable->setPropertyValue(PROPERTY_TEXTCOLOR,_aTextColor);
                        }
                        m_bIsAutoIncrement  = aWizard.shouldCreatePrimaryKey();
                        m_vColumns          = aWizard.GetColumnPositions();
                        m_vColumnTypes      = aWizard.GetColumnTypes();
                        m_bAppendFirstLine  = !aWizard.UseHeaderLine();
                    }
                    break;
                default:
                    bError = sal_True; // there is no error but I have nothing more to do
            }
        }
        else
            bError = sal_True;

        if(!bError)
            bError = !createRowSet();
    }
    catch( const SQLException&)
    {
        ::dbaui::showError( ::dbtools::SQLExceptionInfo( ::cppu::getCaughtException() ), &aWizard, m_xContext );
        bError = sal_True;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return bError;
}

void ODatabaseExport::showErrorDialog(const ::com::sun::star::sdbc::SQLException& e)
{
    SAL_INFO("dbaccess.ui", "ODatabaseExport::showErrorDialog" );
    if(!m_bDontAskAgain)
    {
        String aMsg(e.Message);
        aMsg += '\n';
        aMsg += String( ModuleRes( STR_QRY_CONTINUE ) );
        OSQLWarningBox aBox( NULL, aMsg, WB_YES_NO | WB_DEF_NO );

        if (aBox.Execute() == RET_YES)
            m_bDontAskAgain = sal_True;
        else
            m_bError = sal_True;
    }
}

void ODatabaseExport::adjustFormat()
{
    SAL_INFO("dbaccess.ui", "ODatabaseExport::adjustFormat" );
    if ( m_sTextToken.Len() )
    {
        sal_Int32 nNewPos = m_bIsAutoIncrement ? m_nColumnPos+1 : m_nColumnPos;
        OSL_ENSURE((nNewPos) < static_cast<sal_Int32>(m_vColumns.size()),"Illegal index for vector");
        if ( (nNewPos) < static_cast<sal_Int32>(m_vColumns.size()) )
        {
            sal_Int32 nColPos = m_vColumns[nNewPos].first;
            if( nColPos != sal::static_int_cast< long >(CONTAINER_ENTRY_NOTFOUND))
            {
                --nColPos;
                OSL_ENSURE((nColPos) < static_cast<sal_Int32>(m_vNumberFormat.size()),"m_vFormatKey: Illegal index for vector");
                OSL_ENSURE((nColPos) < static_cast<sal_Int32>(m_vColumnSize.size()),"m_vColumnSize: Illegal index for vector");
                m_vNumberFormat[nColPos] = CheckString(m_sTextToken,m_vNumberFormat[nColPos]);
                m_vColumnSize[nColPos] = ::std::max<sal_Int32>((sal_Int32)m_vColumnSize[nColPos],(sal_Int32)m_sTextToken.Len());
            }
        }
        eraseTokens();
    }
}

void ODatabaseExport::eraseTokens()
{
    SAL_INFO("dbaccess.ui", "ODatabaseExport::eraseTokens" );
    m_sTextToken.Erase();
    m_sNumToken.Erase();
    m_sValToken.Erase();
}

void ODatabaseExport::ensureFormatter()
{
    SAL_INFO("dbaccess.ui", "ODatabaseExport::ensureFormatter" );
    if ( !m_pFormatter )
    {
        Reference< XNumberFormatsSupplier >  xSupplier = m_xFormatter->getNumberFormatsSupplier();
        Reference< XUnoTunnel > xTunnel(xSupplier,UNO_QUERY);
        SvNumberFormatsSupplierObj* pSupplierImpl = (SvNumberFormatsSupplierObj*)sal::static_int_cast< sal_IntPtr >(xTunnel->getSomething(SvNumberFormatsSupplierObj::getUnoTunnelId()));
        m_pFormatter = pSupplierImpl ? pSupplierImpl->GetNumberFormatter() : NULL;
        Reference<XPropertySet> xNumberFormatSettings = xSupplier->getNumberFormatSettings();
        xNumberFormatSettings->getPropertyValue("NullDate") >>= m_aNullDate;
    }
}

Reference< XPreparedStatement > ODatabaseExport::createPreparedStatment( const Reference<XDatabaseMetaData>& _xMetaData
                                                       ,const Reference<XPropertySet>& _xDestTable
                                                       ,const TPositions& _rvColumns)
{
    SAL_INFO("dbaccess.ui", "ODatabaseExport::createPreparedStatment" );
    OUString aSql(OUString("INSERT INTO "));
    OUString sComposedTableName = ::dbtools::composeTableName( _xMetaData, _xDestTable, ::dbtools::eInDataManipulation, false, false, true );

    aSql += sComposedTableName;
    aSql += OUString(" ( ");
    // set values and column names
    OUString aValues(" VALUES ( ");
    static OUString aPara("?,");
    static OUString aComma(",");

    OUString aQuote;
    if ( _xMetaData.is() )
        aQuote = _xMetaData->getIdentifierQuoteString();

    Reference<XColumnsSupplier> xDestColsSup(_xDestTable,UNO_QUERY_THROW);

    // create sql string and set column types
    Sequence< OUString> aDestColumnNames = xDestColsSup->getColumns()->getElementNames();
    if ( aDestColumnNames.getLength() == 0 )
    {
        return Reference< XPreparedStatement > ();
    }
    const OUString* pIter = aDestColumnNames.getConstArray();
    ::std::vector< OUString> aInsertList;
    aInsertList.resize(aDestColumnNames.getLength()+1);
    sal_Int32 i = 0;
    for(sal_uInt32 j=0; j < aInsertList.size() ;++i,++j)
    {
        ODatabaseExport::TPositions::const_iterator aFind = ::std::find_if(_rvColumns.begin(),_rvColumns.end(),
            ::o3tl::compose1(::std::bind2nd(::std::equal_to<sal_Int32>(),i+1),::o3tl::select2nd<ODatabaseExport::TPositions::value_type>()));
        if ( _rvColumns.end() != aFind && aFind->second != sal::static_int_cast< long >(CONTAINER_ENTRY_NOTFOUND) && aFind->first != sal::static_int_cast< long >(CONTAINER_ENTRY_NOTFOUND) )
        {
            OSL_ENSURE((aFind->first) < static_cast<sal_Int32>(aInsertList.size()),"aInsertList: Illegal index for vector");
            aInsertList[aFind->first] = ::dbtools::quoteName( aQuote,*(pIter+i));
        }
    }

    i = 1;
    // create the sql string
    ::std::vector< OUString>::iterator aInsertEnd = aInsertList.end();
    for (::std::vector< OUString>::iterator aInsertIter = aInsertList.begin(); aInsertIter != aInsertEnd; ++aInsertIter)
    {
        if ( !aInsertIter->isEmpty() )
        {
            aSql += *aInsertIter;
            aSql += aComma;
            aValues += aPara;
        }
    }

    aSql = aSql.replaceAt(aSql.getLength()-1,1,OUString(")"));
    aValues = aValues.replaceAt(aValues.getLength()-1,1,OUString(")"));

    aSql += aValues;
    // now create,fill and execute the prepared statement
    return Reference< XPreparedStatement >(_xMetaData->getConnection()->prepareStatement(aSql));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
