/*************************************************************************
 *
 *  $RCSfile: DExport.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-15 09:09:46 $
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
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
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
#ifndef _MEMORY_
#include <memory>
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

// ==========================================================================
// ==========================================================================
// ODatabaseExport
// ==========================================================================
ODatabaseExport::ODatabaseExport(sal_Int32 nRows,
                                 const ::std::vector<sal_Int32> &_rColumnPositions,
                                 const Reference< XNumberFormatter >& _rxNumberF,
                                 const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM)
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
{
    m_nDefToken = gsl_getSystemTextEncoding();

    m_nRows += nRows;
    sal_Int32 nCount = 0;
    for(sal_Int32 j=0;j < (sal_Int32)m_vColumns.size();++j)
        if(m_vColumns[j] != CONTAINER_ENTRY_NOTFOUND)
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
        Any aValue = ConfigManager::GetDirectConfigProperty(ConfigManager::LOCALE);
        m_nLocale.Language = ::comphelper::getString(aValue);
    }
    catch(Exception&)
    {
    }

}
//---------------------------------------------------------------------------
ODatabaseExport::ODatabaseExport(const Reference< XConnection >& _rxConnection,
                                 const Reference< XNumberFormatter >& _rxNumberF,
                                 const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM)
    :m_xConnection(_rxConnection)
    ,m_nColumnPos(0)
    ,m_nRows(1)
    ,m_nRowCount(0)
    ,m_bError(FALSE)
    ,m_bInTbl(FALSE)
    ,m_xFormatter(_rxNumberF)
    ,m_bHead(TRUE)
    ,m_bDontAskAgain(FALSE)
    ,m_bIsAutoIncrement(FALSE)
    ,m_aDestColumns(_rxConnection->getMetaData()->storesMixedCaseQuotedIdentifiers())
    ,m_xFactory(_rM)
    ,m_pTypeInfo(NULL)
{
    try
    {
        Any aValue = ConfigManager::GetDirectConfigProperty(ConfigManager::LOCALE);
        m_nLocale.Language = ::comphelper::getString(aValue);
    }
    catch(Exception&)
    {
    }

    m_nDefToken = gsl_getSystemTextEncoding();
    Reference<XTablesSupplier> xTablesSup(m_xConnection,UNO_QUERY);
    if(xTablesSup.is())
        m_xTables = xTablesSup->getTables();

    Reference<XResultSet> xSet = m_xConnection->getMetaData()->getTypeInfo();
    if(xSet.is())
    {
        Reference<XRow> xRow(xSet,UNO_QUERY);
        while(xSet->next())
        {
            if(xRow->getInt(2) == DataType::VARCHAR)
            {
                m_pTypeInfo                 = new OTypeInfo();
                m_pTypeInfo->aTypeName      = xRow->getString (1);
                m_pTypeInfo->nType          = xRow->getShort (2);
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
}
//---------------------------------------------------------------------------
ODatabaseExport::~ODatabaseExport()
{
    delete m_pTypeInfo;
}
//------------------------------------------------------------------------------
String ODatabaseExport::ShortenFieldName( const String& rName, xub_StrLen nNewLength,
                                         const Reference< XNameAccess>& rxDestList )
{
    xub_StrLen nLength = nNewLength ? nNewLength : rName.Len();
    //////////////////////////////////////////////////////////////////////
    // Wenn es schon einen gekuerzten Namen gibt, Namen durchnumerieren (bis 99)
    String aNewName( rName.Copy( 0, nLength ));
    String aBaseName( rName.Copy( 0, nLength - 2 ));
    xub_StrLen i=1;
    while( rxDestList->hasByName(aNewName) && (i<100) )
    {
        aNewName = aBaseName;
        aNewName += String::CreateFromInt32(i);
        i++;
    }

    return aNewName;
}
// -----------------------------------------------------------------------------
void ODatabaseExport::insertValueIntoColumn()
{
    if(m_nColumnPos < sal_Int32(m_vDestVector.size()))
    {
        OFieldDescription* pField = m_vDestVector[m_nColumnPos]->second;
        if(pField)
        {
            sal_Int32 nPos = m_vColumns[m_bIsAutoIncrement ? m_nColumnPos+1 : m_nColumnPos];
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
            m_vFormatKey[m_vColumns[m_nColumnPos]] = nFormat; // wird sp"ater f"ur die Column gebraucht
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
        m_vFormatKey[m_vColumns[m_nColumnPos]] =    100;
        nFormat = NumberFormat::TEXT; // Text "uberschreibt alles
    }

    return nFormat;
}
// -----------------------------------------------------------------------------
void ODatabaseExport::SetColumnTypes(const TColumnVector* _pList,const OTypeInfoMap* _pInfoMap)
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
// -----------------------------------------------------------------------------
void ODatabaseExport::CreateDefaultColumn(const ::rtl::OUString& _rColumnName)
{
    Reference< XDatabaseMetaData>  xDestMetaData(m_xConnection->getMetaData());
    sal_Int32 nMaxNameLen(xDestMetaData->getMaxColumnNameLength());
    ::rtl::OUString aAlias(::dbtools::convertName2SQLName(_rColumnName,xDestMetaData->getExtraNameCharacters()));

    if(nMaxNameLen && aAlias.getLength() > nMaxNameLen)
        aAlias = aAlias.copy(0,aAlias.getLength() -(aAlias.getLength()-nMaxNameLen-2));

    ::rtl::OUString sName(aAlias);
    sal_Int32 nPos = 1;
    while(m_aDestColumns.find(sName) != m_aDestColumns.end())
    {
        sName = aAlias;
        sName += ::rtl::OUString::valueOf(++nPos);
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

    m_vDestVector.push_back(m_aDestColumns.insert(TColumns::value_type(aAlias,pField)).first);
}
// -----------------------------------------------------------------------------
sal_Bool ODatabaseExport::createRowSet()
{
    Reference<XResultSet> xDestSet = Reference< XResultSet >(m_xFactory->createInstance(::rtl::OUString::createFromAscii("com.sun.star.sdb.RowSet")),UNO_QUERY);
    Reference<XPropertySet > xProp(xDestSet,UNO_QUERY);
    if(xProp.is())
    {
        ::rtl::OUString sDestName;
        ::dbaui::composeTableName(m_xConnection->getMetaData(),m_xTable,sDestName,sal_False);

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
    m_xResultSetUpdate  = Reference< XResultSetUpdate>(xDestSet,UNO_QUERY);
    m_xRowUpdate        = Reference< XRowUpdate>(xDestSet,UNO_QUERY);

    return m_xResultSetUpdate.is() && m_xRowUpdate.is() && m_xResultSetMetaData.is();
}
// -----------------------------------------------------------------------------









