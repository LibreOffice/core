/*************************************************************************
 *
 *  $RCSfile: DExport.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-16 15:52:30 $
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
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
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
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif
#define CONTAINER_ENTRY_NOTFOUND    ((ULONG)0xFFFFFFFF)

using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;

// ==========================================================================
// ==========================================================================
// ODatabaseExport
// ==========================================================================
ODatabaseExport::ODatabaseExport(sal_Int32 nRows,sal_Int32 nColumns,const Reference< XNumberFormatter >& _rxNumberF)
    :m_vColumns(nColumns,0)
    ,m_nColumnPos(0)
    ,m_nRows(1)
    ,m_nRowCount(0)
    ,m_bError(FALSE)
    ,m_bInTbl(FALSE)
    ,m_xFormatter(_rxNumberF)
    ,m_bHead(TRUE)
    ,m_bDontAskAgain(FALSE)
    ,m_bIsAutoIncrement(FALSE)
{
    m_nDefToken = gsl_getSystemTextEncoding();

    m_nRows += nRows;
    m_pColumnSize = new sal_Int32[nColumns];
    m_pFormatKeys = new sal_Int32[nColumns];
    for(sal_Int32 i=0;i<nColumns;++i)
    {
        m_pColumnSize[i] = 0;
        m_pFormatKeys[i] = 0;
    }

}
//---------------------------------------------------------------------------
ODatabaseExport::ODatabaseExport(const Reference< XConnection >& _rxConnection,
                                 const Reference< XNumberFormatter >& _rxNumberF)
    :m_xConnection(_rxConnection)
    ,m_nColumnPos(0)
    ,m_nRows(1)
    ,m_nRowCount(0)
    ,m_bError(FALSE)
    ,m_bInTbl(FALSE)
    ,m_xFormatter(_rxNumberF)
    ,m_pColumnSize(0)
    ,m_pFormatKeys(0)
    ,m_bHead(TRUE)
    ,m_bDontAskAgain(FALSE)
    ,m_bIsAutoIncrement(FALSE)
{
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
                m_sVarChar = xRow->getString(1);
                break;
            }
        }
    }
}
//---------------------------------------------------------------------------
ODatabaseExport::~ODatabaseExport()
{
    delete m_pColumnSize;
    delete m_pFormatKeys;
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
    if(m_nColumnPos >= m_xColumnsIdx->getCount())
    {
        Reference<XPropertySet> xColumn;
        m_xColumnsIdx->getByIndex(m_nColumnPos) >>= xColumn;
        if(xColumn.is())
        {
            if(!xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_ISROWVERSION) ||
               ! ::cppu::any2bool(xColumn->getPropertyValue(PROPERTY_ISROWVERSION)))
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
                        if (m_vColumnTypes[nPos] != DataType::VARCHAR)
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
                            fOutNumber = m_xFormatter->convertStringToNumber(nNumberFormat,m_sTextToken);
                            m_xRowUpdate->updateDouble(nPos,::dbtools::DBTypeConversion::toStandardDbDate(::dbtools::DBTypeConversion::getStandardDate(),fOutNumber));
                        }
                        else
                            m_xRowUpdate->updateString(nPos,m_sTextToken);
                    }
                }
                m_sTextToken.Erase();
            }
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
            m_pFormatKeys[m_nColumnPos] = nFormat; // wird sp"ater f"ur die Column gebraucht
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
        m_pFormatKeys[m_nColumnPos] =   100;
        nFormat = NumberFormat::TEXT; // Text "uberschreibt alles
    }

    return nFormat;
}
// -----------------------------------------------------------------------------
void ODatabaseExport::SetColumnTypes(const Reference< XNameAccess>& rList)
{
    if(!m_xConnection.is())
        return;
    for(sal_Int32 i=0;i < m_xColumnsIdx->getCount();i++)
    {
        Reference<XPropertySet> xColumn;
        m_xColumnsIdx->getByIndex(m_nColumnPos) >>= xColumn;

        sal_Int32 nDataType;
        sal_Int32 nLength(0),nScale(0);
        switch(m_vColumns[i])
        {
            case NumberFormat::ALL:
                nDataType  = DataType::DOUBLE;
                break;
            case NumberFormat::DEFINED:
                nDataType   = DataType::VARCHAR;
                nLength     = ((m_pColumnSize[i] % 10 ) ? m_pColumnSize[i]/ 10 + 1: m_pColumnSize[i]/ 10) * 10;
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
                nDataType  = DataType::VARCHAR;
                nLength     = ((m_pColumnSize[i] % 10 ) ? m_pColumnSize[i]/ 10 + 1: m_pColumnSize[i]/ 10) * 10;
                break;

            default:
                OSL_ENSURE(0,"ODatabaseExport: Unbekanntes Format");
        }

//      pColumn->Put(SbaDataFieldTypeItem(SBA_DEF_FLTTYPE,(SbaFieldType) nDataType ));
//      pColumn->Put(SfxUInt32Item(SBA_DEF_FLTLENGTH,nLength));
//      pColumn->Put(SfxUInt16Item(SBA_DEF_FLTSCALE,nScale));
//      pColumn->Put(SfxUInt32Item(SBA_DEF_FMTVALUE, m_pFormatKeys[i]));
        xColumn->setPropertyValue(PROPERTY_FORMATKEY,makeAny(m_pFormatKeys[i]));
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
    aAlias = ::dbtools::createUniqueName(m_xColumns,aAlias);
    // now create a column
    Reference<XDataDescriptorFactory> xColumnFactory(m_xColumns,UNO_QUERY);
    Reference<XPropertySet> xColumn = xColumnFactory->createDataDescriptor();

    xColumn->setPropertyValue(PROPERTY_NAME,makeAny(aAlias));
    xColumn->setPropertyValue(PROPERTY_TYPE,makeAny(DataType::VARCHAR));
    xColumn->setPropertyValue(PROPERTY_TYPENAME,makeAny(m_sVarChar));
    xColumn->setPropertyValue(PROPERTY_PRECISION,makeAny((sal_Int32)255));
    xColumn->setPropertyValue(PROPERTY_SCALE,makeAny((sal_Int32)0));
    xColumn->setPropertyValue(PROPERTY_ISNULLABLE, makeAny(ColumnValue::NULLABLE));
    xColumn->setPropertyValue(PROPERTY_ISAUTOINCREMENT,::cppu::bool2any(sal_False));
    xColumn->setPropertyValue(PROPERTY_ISCURRENCY,::cppu::bool2any(sal_False));

    Reference<XAppend> xAppend(m_xColumns,UNO_QUERY);
    OSL_ENSURE(xAppend.is(),"No XAppend Interface!");
    xAppend->appendByDescriptor(xColumn);
}







