/*************************************************************************
 *
 *  $RCSfile: DExport.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:04:45 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
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
#ifndef DBAUI_DATABASEEXPORT_HXX
#include "DExport.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDATADESCRIPTORFACTORY_HPP_
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XAPPEND_HPP_
#include <com/sun/star/sdbcx/XAppend.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_KEYTYPE_HPP_
#include <com/sun/star/sdbcx/KeyType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATASUPPLIER_HPP_
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_NUMBERFORMAT_HPP_
#include <com/sun/star/util/NumberFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTYPES_HPP_
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _DBU_MISC_HRC_
#include "dbu_misc.hrc"
#endif
#ifndef _DBHELPER_DBCONVERSION_HXX_
#include <connectivity/dbconversion.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef DBAUI_TYPEINFO_HXX
#include "TypeInfo.hxx"
#endif
#ifndef DBAUI_FIELDDESCRIPTIONS_HXX
#include "FieldDescriptions.hxx"
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#include <memory>
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTDESCRIPTOR_HPP_
#include <com/sun/star/awt/FontDescriptor.hpp>
#endif
#ifndef DBAUI_WIZ_COPYTABLEDIALOG_HXX
#include "WCopyTable.hxx"
#endif
#ifndef DBAUI_WIZ_EXTENDPAGES_HXX
#include "WExtendPages.hxx"
#endif
#ifndef DBAUI_WIZ_NAMEMATCHING_HXX
#include "WNameMatch.hxx"
#endif
#ifndef DBAUI_WIZ_COLUMNSELECT_HXX
#include "WColumnSelect.hxx"
#endif
#ifndef DBAUI_WIZARD_CPAGE_HXX
#include "WCPage.hxx"
#endif
#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include <svtools/syslocale.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_SQLWARNING_HPP_
#include <com/sun/star/sdbc/SQLWarning.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif


#define CONTAINER_ENTRY_NOTFOUND    ((ULONG)0xFFFFFFFF)

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

// ==========================================================================
// ==========================================================================
// ODatabaseExport
// ==========================================================================
DBG_NAME(ODatabaseExport);
ODatabaseExport::ODatabaseExport(sal_Int32 nRows,
                                 const TPositions &_rColumnPositions,
                                 const Reference< XNumberFormatter >& _rxNumberF,
                                 const Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM,
                                 const TColumnVector* pList,
                                 const OTypeInfoMap* _pInfoMap)
    :m_nColumnPos(0)
    ,m_nRows(1)
    ,m_nRowCount(0)
    ,m_bError(FALSE)
    ,m_bInTbl(FALSE)
    ,m_xFormatter(_rxNumberF)
    ,m_bHead(TRUE)
    ,m_bDontAskAgain(FALSE)
    ,m_bIsAutoIncrement(FALSE)
    ,m_aDestColumns(sal_True)
    ,m_xFactory(_rM)
    ,m_pTypeInfo(NULL)
    ,m_vColumnSize(0)
    ,m_vFormatKey(0)
    ,m_vColumns(_rColumnPositions)
    ,m_bFoundTable(sal_False)
    ,m_bCheckOnly(sal_False)
{
    DBG_CTOR(ODatabaseExport,NULL);

    m_nDefToken = gsl_getSystemTextEncoding();

    m_nRows += nRows;
    sal_Int32 nCount = 0;
    for(sal_Int32 j=0;j < (sal_Int32)m_vColumns.size();++j)
        if(m_vColumns[j].first != CONTAINER_ENTRY_NOTFOUND)
            ++nCount;

    m_vColumnSize.resize(nCount);
    m_vFormatKey.resize(nCount);
    for(sal_Int32 i=0;i<nCount;++i)
    {
        m_vColumnSize[i] = 0;
        m_vFormatKey[i] = 0;
    }

    try
    {
        SvtSysLocale aSysLocale;
        m_nLocale = aSysLocale.GetLocaleData().getLocale();
//      Any aValue = ConfigManager::GetDirectConfigProperty(ConfigManager::LOCALE);
//      m_nLocale.Language = ::comphelper::getString(aValue);
//      String sLanguage, sCountry;
//      ConvertLanguageToIsoNames(Window::GetSettings().GetLanguage(), sLanguage, sCountry);
//      m_nLocale = Locale(sLanguage, sCountry, ::rtl::OUString());
    }
    catch(Exception&)
    {
    }

    SetColumnTypes(pList,_pInfoMap);
}
//---------------------------------------------------------------------------
ODatabaseExport::ODatabaseExport(const Reference< XConnection >& _rxConnection,
                                 const Reference< XNumberFormatter >& _rxNumberF,
                                 const Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM,
                                 const TColumnVector* pList,
                                 const OTypeInfoMap* _pInfoMap)
    :m_xConnection(_rxConnection)
    ,m_nColumnPos(0)
    ,m_nRows(1)
    ,m_nRowCount(0)
    ,m_bError(sal_False)
    ,m_bInTbl(sal_False)
    ,m_xFormatter(_rxNumberF)
    ,m_bHead(TRUE)
    ,m_bDontAskAgain(sal_False)
    ,m_bIsAutoIncrement(sal_False)
    ,m_aDestColumns(_rxConnection->getMetaData().is() && _rxConnection->getMetaData()->storesMixedCaseQuotedIdentifiers() == sal_True)
    ,m_xFactory(_rM)
    ,m_pTypeInfo(NULL)
    ,m_bFoundTable(sal_False)
    ,m_bCheckOnly(sal_False)
{
    DBG_CTOR(ODatabaseExport,NULL);
    try
    {
        SvtSysLocale aSysLocale;
        m_nLocale = aSysLocale.GetLocaleData().getLocale();
//      Any aValue = ConfigManager::GetDirectConfigProperty(ConfigManager::LOCALE);
//      m_nLocale.Language = ::comphelper::getString(aValue);
//      String sLanguage, sCountry;
//      ConvertLanguageToIsoNames(Window::GetSettings().GetLanguage(), sLanguage, sCountry);
//      m_nLocale = Locale(sLanguage, sCountry, ::rtl::OUString());
    }
    catch(Exception&)
    {
    }

    m_nDefToken = gsl_getSystemTextEncoding();
    Reference<XTablesSupplier> xTablesSup(m_xConnection,UNO_QUERY);
    if(xTablesSup.is())
        m_xTables = xTablesSup->getTables();

    Reference<XDatabaseMetaData> xMeta = m_xConnection->getMetaData();
    Reference<XResultSet> xSet = xMeta.is() ? xMeta->getTypeInfo() : Reference<XResultSet>();
    if(xSet.is())
    {
        Reference<XRow> xRow(xSet,UNO_QUERY);
        while(xSet->next())
        {
            ::rtl::OUString sTypeName = xRow->getString (1);
            sal_Int16 nType = xRow->getShort(2);
            if( nType == DataType::VARCHAR)
            {
                m_pTypeInfo                 = TOTypeInfoSP(new OTypeInfo());
                m_pTypeInfo->aTypeName      = sTypeName;
                m_pTypeInfo->nType          = nType;
                m_pTypeInfo->nPrecision     = xRow->getInt (3);
                m_pTypeInfo->aLiteralPrefix = xRow->getString (4);
                m_pTypeInfo->aLiteralSuffix = xRow->getString (5);
                m_pTypeInfo->aCreateParams  = xRow->getString (6);
                m_pTypeInfo->bNullable      = xRow->getInt (7) == ColumnValue::NULLABLE;
                m_pTypeInfo->bCaseSensitive = xRow->getBoolean (8);
                m_pTypeInfo->nSearchType    = xRow->getShort (9);
                m_pTypeInfo->bUnsigned      = xRow->getBoolean (10);
                m_pTypeInfo->bCurrency      = xRow->getBoolean (11);
                m_pTypeInfo->bAutoIncrement = xRow->getBoolean (12);
                m_pTypeInfo->aLocalTypeName = xRow->getString (13);
                m_pTypeInfo->nMinimumScale  = xRow->getShort (14);
                m_pTypeInfo->nMaximumScale  = xRow->getShort (15);
                m_pTypeInfo->nNumPrecRadix  = xRow->getInt (18);
                break;
            }
        }
    }
    SetColumnTypes(pList,_pInfoMap);
}
//---------------------------------------------------------------------------
ODatabaseExport::~ODatabaseExport()
{
    DBG_DTOR(ODatabaseExport,NULL);
    ODatabaseExport::TColumns::iterator aIter = m_aDestColumns.begin();
    ODatabaseExport::TColumns::iterator aEnd  = m_aDestColumns.end();

    for(;aIter != aEnd;++aIter)
        delete aIter->second;
    m_vDestVector.clear();
    m_aDestColumns.clear();
}
// -----------------------------------------------------------------------------
void ODatabaseExport::insertValueIntoColumn()
{
    DBG_CHKTHIS(ODatabaseExport,NULL);
    if(m_nColumnPos < sal_Int32(m_vDestVector.size()))
    {
        OFieldDescription* pField = m_vDestVector[m_nColumnPos]->second;
        if(pField)
        {
            OSL_ENSURE((m_bIsAutoIncrement ? m_nColumnPos+1 : m_nColumnPos) < m_vColumns.size(),"Illegal index for vector");
            sal_Int32 nPos = m_vColumns[m_bIsAutoIncrement ? m_nColumnPos+1 : m_nColumnPos].first;
            if(nPos != CONTAINER_ENTRY_NOTFOUND)
            {
//                  if(m_nDefToken != LANGUAGE_DONTKNOW) // falls Sprache anders als Systemsprache
//                      m_pNF->ChangeIntl((LanguageType)m_nDefToken);

                if(!m_sTextToken.Len() && m_xResultSetMetaData->isNullable(nPos))
                    m_xRowUpdate->updateNull(nPos);
                else
                {
                    sal_Int32 nNumberFormat = 0;
                    double fOutNumber = 0.0;
                    OSL_ENSURE((m_bIsAutoIncrement ? m_nColumnPos+1 : m_nColumnPos) < m_vColumnTypes.size(),"Illegal index for vector");
                    if (m_vColumnTypes[m_bIsAutoIncrement ? m_nColumnPos+1 : m_nColumnPos] != DataType::VARCHAR)
                    {
                        Reference<XNumberFormatTypes> xNumType(m_xFormatter->getNumberFormatsSupplier()->getNumberFormats(),UNO_QUERY);

                        try
                        {
                            nNumberFormat = m_xFormatter->detectNumberFormat(xNumType->getStandardFormat(NumberFormat::DATETIME,m_nLocale),m_sTextToken);
                        }
                        catch(Exception&)
                        {
                            try
                            {
                                nNumberFormat = m_xFormatter->detectNumberFormat(xNumType->getStandardFormat(NumberFormat::DATE,m_nLocale),m_sTextToken);
                            }
                            catch(Exception&)
                            {
                                try
                                {
                                    nNumberFormat = m_xFormatter->detectNumberFormat(xNumType->getStandardFormat(NumberFormat::TIME,m_nLocale),m_sTextToken);
                                }
                                catch(Exception&)
                                {
                                    try
                                    {
                                        nNumberFormat = m_xFormatter->detectNumberFormat(xNumType->getStandardFormat(NumberFormat::NUMBER,m_nLocale),m_sTextToken);
                                    }
                                    catch(Exception&)
                                    {
                                    }
                                }
                            }
                        }
                        try
                        {
                            fOutNumber = m_xFormatter->convertStringToNumber(nNumberFormat,m_sTextToken);
                            m_xRowUpdate->updateDouble(nPos,::dbtools::DBTypeConversion::toStandardDbDate(::dbtools::DBTypeConversion::getStandardDate(),fOutNumber));
                        }
                        catch(Exception&)
                        {
                            m_xRowUpdate->updateString(nPos,m_sTextToken);
                        }

                    }
                    else
                        m_xRowUpdate->updateString(nPos,m_sTextToken);
                }
            }
            m_sTextToken.Erase();
        }
    }
}
// -----------------------------------------------------------------------------
sal_Int32 ODatabaseExport::CheckString(const String& aCheckToken, sal_Int32 _nOldFormat)
{
    DBG_CHKTHIS(ODatabaseExport,NULL);
    sal_Int32 F_Index = 0;
    double fOutNumber = 0.0;
    sal_Int32 nFormat = 0;

    try
    {
        Reference<XNumberFormats> xFormats = m_xFormatter->getNumberFormatsSupplier()->getNumberFormats();
        Reference<XNumberFormatTypes> xNumType(xFormats,UNO_QUERY);
        nFormat = m_xFormatter->detectNumberFormat(xNumType->getStandardFormat(NumberFormat::ALL,m_nLocale),aCheckToken);
        Reference<XPropertySet> xProp = xFormats->getByKey(nFormat);
        sal_Int16 nType = 0;
        xProp->getPropertyValue(PROPERTY_TYPE) >>= nType;
        fOutNumber = m_xFormatter->convertStringToNumber(nFormat,aCheckToken);

        {
            OSL_ENSURE((m_nColumnPos) < m_vColumns.size(),"Illegal index for vector");
            m_vFormatKey[m_vColumns[m_nColumnPos].first] = nFormat; // wird sp"ater f"ur die Column gebraucht
            switch(nType)
            {
                case NumberFormat::ALL:
                    nFormat = NumberFormat::ALL;
                    break;
                case NumberFormat::DEFINED:
                    nFormat = NumberFormat::TEXT;
                    break;
                case NumberFormat::DATE:
                    switch(_nOldFormat)
                    {
                        case NumberFormat::DATETIME:
                        case NumberFormat::TEXT:
                        case NumberFormat::DATE:
                            break;
                        case NumberFormat::ALL:
                            nFormat = NumberFormat::DATE;
                            break;
                        default:
                            nFormat = NumberFormat::TEXT;

                    }
                    break;
                case NumberFormat::TIME:
                    switch(_nOldFormat)
                    {
                        case NumberFormat::DATETIME:
                        case NumberFormat::TEXT:
                        case NumberFormat::TIME:
                            break;
                        case NumberFormat::ALL:
                            nFormat = NumberFormat::TIME;
                            break;
                        default:
                            nFormat = NumberFormat::TEXT;
                            break;
                    }
                    break;
                case NumberFormat::CURRENCY:
                    switch(_nOldFormat)
                    {
                        case NumberFormat::NUMBER:
                            nFormat = NumberFormat::CURRENCY;
                            break;
                        case NumberFormat::CURRENCY:
                            break;
                        case NumberFormat::ALL:
                            nFormat = NumberFormat::CURRENCY;
                            break;
                        default:
                            nFormat = NumberFormat::TEXT;
                            break;
                    }
                    break;
                case NumberFormat::NUMBER:
                case NumberFormat::SCIENTIFIC:
                case NumberFormat::FRACTION:
                case NumberFormat::PERCENT:
                    switch(_nOldFormat)
                    {
                        case NumberFormat::NUMBER:
                            break;
                        case NumberFormat::CURRENCY:
                            nFormat = NumberFormat::CURRENCY;
                            break;
                        case NumberFormat::ALL:
                            nFormat = nType;
                            break;
                        default:
                            nFormat = NumberFormat::TEXT;
                            break;
                    }
                    break;
                case NumberFormat::TEXT:
                case NumberFormat::UNDEFINED:
                case NumberFormat::LOGICAL:
                    nFormat = NumberFormat::TEXT; // Text "uberschreibt alles
                    break;
                case NumberFormat::DATETIME:
                    switch(_nOldFormat)
                    {
                        case NumberFormat::DATETIME:
                        case NumberFormat::TEXT:
                        case NumberFormat::TIME:
                            break;
                        case NumberFormat::ALL:
                            nFormat = NumberFormat::DATETIME;
                            break;
                        default:
                            nFormat = NumberFormat::TEXT;
                            break;
                    }
                    break;
                default:
                    OSL_ENSURE(0,"ODatabaseExport: Unbekanntes Format");
            }
        }
    }
    catch(Exception&)
    {
        OSL_ENSURE((m_nColumnPos) < m_vColumns.size(),"Illegal index for vector");
        m_vFormatKey[m_vColumns[m_nColumnPos].first] =  100;
        nFormat = NumberFormat::TEXT; // Text "uberschreibt alles
    }

    return nFormat;
}
// -----------------------------------------------------------------------------
void ODatabaseExport::SetColumnTypes(const TColumnVector* _pList,const OTypeInfoMap* _pInfoMap)
{
    DBG_CHKTHIS(ODatabaseExport,NULL);
    if(_pList && _pInfoMap)
    {
        TColumnVector::const_iterator aIter = _pList->begin();
        for(sal_Int32 i=0;aIter != _pList->end();++aIter,++i)
        {
            sal_Int32 nDataType;
            sal_Int32 nLength(0),nScale(0);
            switch(m_vFormatKey[i])
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
            }
            (*aIter)->second->SetFormatKey(m_vFormatKey[i]);
        }
    }
}
// -----------------------------------------------------------------------------
void ODatabaseExport::CreateDefaultColumn(const ::rtl::OUString& _rColumnName)
{
    DBG_CHKTHIS(ODatabaseExport,NULL);
    Reference< XDatabaseMetaData>  xDestMetaData(m_xConnection->getMetaData());
    sal_Int32 nMaxNameLen(xDestMetaData->getMaxColumnNameLength());
    ::rtl::OUString aAlias = _rColumnName;
    if ( isSQL92CheckEnabled(m_xConnection) )
        aAlias = ::dbtools::convertName2SQLName(_rColumnName,xDestMetaData->getExtraNameCharacters());

    if(nMaxNameLen && aAlias.getLength() > nMaxNameLen)
        aAlias = aAlias.copy(0, ::std::min<sal_Int32>( nMaxNameLen, aAlias.getLength() ) );

    ::rtl::OUString sName(aAlias);
    if(m_aDestColumns.find(sName) != m_aDestColumns.end())
    {
        aAlias = aAlias.copy(0,::std::min<sal_Int32>( nMaxNameLen-1, aAlias.getLength() ));

        sal_Int32 nPos = 0;
        sal_Int32 nCount = 2;
        while(m_aDestColumns.find(sName) != m_aDestColumns.end())
        {
            sName = aAlias;
            sName += ::rtl::OUString::valueOf(++nPos);
            if(nMaxNameLen && sName.getLength() > nMaxNameLen)
            {
                aAlias = aAlias.copy(0,::std::min<sal_Int32>( nMaxNameLen-nCount, aAlias.getLength() ));
                sName = aAlias;
                sName += ::rtl::OUString::valueOf(nPos);
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
// -----------------------------------------------------------------------------
sal_Bool ODatabaseExport::createRowSet()
{
    DBG_CHKTHIS(ODatabaseExport,NULL);
    Reference<XResultSet> xDestSet(m_xFactory->createInstance(::rtl::OUString::createFromAscii("com.sun.star.sdb.RowSet")),UNO_QUERY);
    Reference<XPropertySet > xProp(xDestSet,UNO_QUERY);
    if(xProp.is())
    {
        ::rtl::OUString sDestName = ::dbtools::composeTableName(m_xConnection->getMetaData(),m_xTable,sal_False,::dbtools::eInDataManipulation);

        xProp->setPropertyValue(PROPERTY_ACTIVECONNECTION,makeAny(m_xConnection));
        xProp->setPropertyValue(PROPERTY_COMMANDTYPE,makeAny(CommandType::TABLE));
        xProp->setPropertyValue(PROPERTY_COMMAND,makeAny(sDestName));
        xProp->setPropertyValue(PROPERTY_IGNORERESULT,::cppu::bool2any(sal_True));
        Reference<XRowSet> xRowSet(xProp,UNO_QUERY);
        xRowSet->execute();
        Reference< XResultSetMetaDataSupplier> xSrcMetaSup(xRowSet,UNO_QUERY);
        m_xResultSetMetaData = xSrcMetaSup->getMetaData();
        OSL_ENSURE(m_xResultSetMetaData.is(),"No ResultSetMetaData!");
    }
    m_xResultSetUpdate.set(xDestSet,UNO_QUERY);
    m_xRowUpdate.set(xDestSet,UNO_QUERY);

    return m_xResultSetUpdate.is() && m_xRowUpdate.is() && m_xResultSetMetaData.is();
}
// -----------------------------------------------------------------------------
sal_Bool ODatabaseExport::executeWizard(const ::rtl::OUString& _sTableName,const Any& _aTextColor,const FontDescriptor& _rFont)
{
    DBG_CHKTHIS(ODatabaseExport,NULL);
    OCopyTableWizard aWizard(NULL,_sTableName,m_aDestColumns,m_vDestVector,m_xConnection,m_xFormatter,m_xFactory);

    sal_Bool bError = sal_False;
    try
    {
        aWizard.fillTypeInfo();

        OCopyTable*         pPage1 = new OCopyTable(&aWizard,COPY, sal_False,OCopyTableWizard::WIZARD_DEF_DATA);
        OWizNameMatching*   pPage2 = new OWizNameMatching(&aWizard);
        OWizColumnSelect*   pPage3 = new OWizColumnSelect(&aWizard);
        OWizTypeSelect*     pPage4 = createPage(&aWizard);

        aWizard.AddWizardPage(pPage1);
        aWizard.AddWizardPage(pPage2);
        aWizard.AddWizardPage(pPage3);
        aWizard.AddWizardPage(pPage4);

        aWizard.ActivatePage();

        if (aWizard.Execute())
        {
            switch(aWizard.getCreateStyle())
            {
                case OCopyTableWizard::WIZARD_DEF_DATA:
                case OCopyTableWizard::WIZARD_APPEND_DATA:
                    {
                        m_xTable = aWizard.createTable();
                        bError = !m_xTable.is();
                        if(m_xTable.is())
                        {
                            m_xTable->setPropertyValue(PROPERTY_FONT,makeAny(_rFont));
                            if(_aTextColor.hasValue())
                                m_xTable->setPropertyValue(PROPERTY_TEXTCOLOR,_aTextColor);
                        }
                        m_bIsAutoIncrement  = aWizard.isAutoincrementEnabled();
                        m_vColumns          = aWizard.GetColumnPositions();
                        m_vColumnTypes      = aWizard.GetColumnTypes();
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
    catch(SQLContext& e)
    {
        ::dbaui::showError(::dbtools::SQLExceptionInfo(e),&aWizard,m_xFactory);
        bError = sal_True;
    }
    catch(SQLWarning& e)
    {

        ::dbaui::showError(::dbtools::SQLExceptionInfo(e),&aWizard,m_xFactory);
        bError = sal_True;
    }
    catch(SQLException& e)
    {


        ::dbaui::showError(::dbtools::SQLExceptionInfo(e),&aWizard,m_xFactory);
        bError = sal_True;
    }
    catch(Exception& )
    {
        OSL_ENSURE(sal_False, "ODatabaseExport::executeWizard: caught a generic exception!");
    }

    return bError;
}
// -----------------------------------------------------------------------------









