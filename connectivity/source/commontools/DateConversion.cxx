/*************************************************************************
 *
 *  $RCSfile: DateConversion.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 08:23:13 $
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


#ifndef _DBHELPER_DBCONVERSION_HXX_
#include "connectivity/dbconversion.hxx"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif

#ifndef _COM_SUN_STAR_SCRIPT_XTYPECONVERTER_HPP_
#include <com/sun/star/script/XTypeConverter.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_NUMBERFORMAT_HPP_
#include <com/sun/star/util/NumberFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTYPES_HPP_
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOLUMNUPDATE_HPP_
#include <com/sun/star/sdb/XColumnUpdate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOLUMN_HPP_
#include <com/sun/star/sdb/XColumn.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif
#ifndef _COMPHELPER_NUMBERS_HXX_
#include <comphelper/numbers.hxx>
#endif


using namespace ::connectivity;
using namespace ::comphelper;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::dbtools;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
// -----------------------------------------------------------------------------
::rtl::OUString DBTypeConversion::toSQLString(sal_Int32 eType, const Any& _rVal, sal_Bool bQuote,
                                              const Reference< XTypeConverter >&  _rxTypeConverter)
{
    ::rtl::OUString aRet;
    if (_rVal.hasValue())
    {
        try
        {
            switch (eType)
            {
                case DataType::INTEGER:
                case DataType::BIT:
                case DataType::TINYINT:
                case DataType::SMALLINT:
                    if (_rVal.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_BOOLEAN)
                    {
                        if (::cppu::any2bool(_rVal))
                            aRet = ::rtl::OUString::createFromAscii("1");
                        else
                            aRet = ::rtl::OUString::createFromAscii("0");
                    }
                    else
                        _rxTypeConverter->convertToSimpleType(_rVal, TypeClass_STRING) >>= aRet;
                    break;
                case DataType::CHAR:
                case DataType::VARCHAR:
                    if (bQuote)
                        aRet += ::rtl::OUString::createFromAscii("'");
                    {
                        ::rtl::OUString aTemp;
                        _rxTypeConverter->convertToSimpleType(_rVal, TypeClass_STRING) >>= aTemp;
                        sal_Int32 nIndex = (sal_Int32)-1;
                        ::rtl::OUString sQuot(RTL_CONSTASCII_USTRINGPARAM("\'"));
                        ::rtl::OUString sQuotToReplace(RTL_CONSTASCII_USTRINGPARAM("\'\'"));
                        do
                        {
                            nIndex += 2;
                            nIndex = aTemp.indexOf(sQuot,nIndex);
                            if(nIndex != -1)
                                aTemp = aTemp.replaceAt(nIndex,sQuot.getLength(),sQuotToReplace);
                        } while (nIndex != -1);

                        aRet += aTemp;
                    }
                    if (bQuote)
                        aRet += ::rtl::OUString::createFromAscii("'");
                    break;
                case DataType::REAL:
                case DataType::DOUBLE:
                case DataType::DECIMAL:
                case DataType::NUMERIC:
                case DataType::BIGINT:
                    _rxTypeConverter->convertToSimpleType(_rVal, TypeClass_STRING) >>= aRet;
                    break;
                case DataType::TIMESTAMP:
                {
                    DateTime aDateTime;

                    // check if this is really a timestamp or only a date
                    if ((_rVal >>= aDateTime) &&
                        (aDateTime.Hours || aDateTime.Minutes || aDateTime.Seconds || aDateTime.HundredthSeconds))
                    {
                        if (bQuote) aRet += ::rtl::OUString::createFromAscii("{TS '");
                        aRet += DBTypeConversion::toDateTimeString(aDateTime);
                        if (bQuote) aRet += ::rtl::OUString::createFromAscii("'}");
                        break;
                    }
                    // else continue
                }
                case DataType::DATE:
                {
                    Date aDate;
                    sal_Bool bRet = _rVal >>= aDate;
                    OSL_ENSURE(bRet,"DBTypeConversion::toSQLString: _rVal is not date!");
                    if (bQuote) aRet += ::rtl::OUString::createFromAscii("{D '");
                    aRet += DBTypeConversion::toDateString(aDate);;
                    if (bQuote) aRet += ::rtl::OUString::createFromAscii("'}");
                }   break;
                case DataType::TIME:
                {
                    Time aTime;
                    sal_Bool bRet = _rVal >>= aTime;
                    OSL_ENSURE(bRet,"DBTypeConversion::toSQLString: _rVal is not time!");
                    if (bQuote) aRet += ::rtl::OUString::createFromAscii("{T '");
                    aRet += DBTypeConversion::toTimeString(aTime);
                    if (bQuote) aRet += ::rtl::OUString::createFromAscii("'}");
                } break;
                default:
                    _rxTypeConverter->convertToSimpleType(_rVal, TypeClass_STRING) >>= aRet;
            }
        }
        catch ( const Exception&  )
        {
            OSL_ENSURE(0,"TypeConversion Error");
        }
    }
    else
        aRet = ::rtl::OUString::createFromAscii(" NULL ");
    return aRet;
}
// -----------------------------------------------------------------------------
Date DBTypeConversion::getNULLDate(const Reference< XNumberFormatsSupplier > &xSupplier)
{
    OSL_ENSURE(xSupplier.is(), "getNULLDate : the formatter doesn't implement a supplier !");
    if (xSupplier.is())
    {
        try
        {
            // get the null date
            Date aDate;
            xSupplier->getNumberFormatSettings()->getPropertyValue(::rtl::OUString::createFromAscii("NullDate")) >>= aDate;
            return aDate;
        }
        catch ( const Exception&  )
        {
        }
    }

    return getStandardDate();
}
// -----------------------------------------------------------------------------
void DBTypeConversion::setValue(const Reference<XColumnUpdate>& xVariant,
                                const Reference<XNumberFormatter>& xFormatter,
                                const Date& rNullDate,
                                const ::rtl::OUString& rString,
                                sal_Int32 nKey,
                                sal_Int16 nFieldType,
                                sal_Int16 nKeyType) throw(::com::sun::star::lang::IllegalArgumentException)
{
    double fValue = 0;
    if (rString.getLength())
    {
            // Muss der String formatiert werden?
        sal_Int16 nTypeClass = nKeyType & ~NumberFormat::DEFINED;
        sal_Bool bTextFormat = nTypeClass == NumberFormat::TEXT;
        sal_Int32 nKeyToUse  = bTextFormat ? 0 : nKey;
        sal_Int16 nRealUsedTypeClass = nTypeClass;
            // bei einem Text-Format muessen wir dem Formatter etwas mehr Freiheiten einraeumen, sonst
            // wirft convertStringToNumber eine NotNumericException
        try
        {
            fValue = xFormatter->convertStringToNumber(nKeyToUse, rString);
            sal_Int32 nRealUsedKey = xFormatter->detectNumberFormat(0, rString);
            if (nRealUsedKey != nKeyToUse)
                nRealUsedTypeClass = getNumberFormatType(xFormatter, nRealUsedKey) & ~NumberFormat::DEFINED;

            // und noch eine Sonderbehandlung, diesmal fuer Prozent-Formate
            if ((NumberFormat::NUMBER == nRealUsedTypeClass) && (NumberFormat::PERCENT == nTypeClass))
            {   // die Formatierung soll eigentlich als Prozent erfolgen, aber der String stellt nur eine
                // einfache Nummer dar -> anpassen
                ::rtl::OUString sExpanded(rString);
                static ::rtl::OUString s_sPercentSymbol = ::rtl::OUString::createFromAscii("%");
                    // need a method to add a sal_Unicode to a string, 'til then we use a static string const Exception&
                sExpanded += s_sPercentSymbol;
                fValue = xFormatter->convertStringToNumber(nKeyToUse, sExpanded);
            }

            switch (nRealUsedTypeClass)
            {
                case NumberFormat::DATE:
                case NumberFormat::DATETIME:
                case NumberFormat::TIME:
                    DBTypeConversion::setValue(xVariant,rNullDate,fValue,nRealUsedTypeClass);
                    //  xVariant->updateDouble(toStandardDbDate(rNullDate, fValue));
                    break;
                case NumberFormat::CURRENCY:
                case NumberFormat::NUMBER:
                case NumberFormat::SCIENTIFIC:
                case NumberFormat::FRACTION:
                case NumberFormat::PERCENT:
                    xVariant->updateDouble(fValue);
                    break;
                default:
                    xVariant->updateString(rString);
            }
        }
        catch(const Exception& )
        {
            xVariant->updateString(rString);
        }
    }
    else
    {
        switch (nFieldType)
        {
            case ::com::sun::star::sdbc::DataType::CHAR:
            case ::com::sun::star::sdbc::DataType::VARCHAR:
            case ::com::sun::star::sdbc::DataType::LONGVARCHAR:
                xVariant->updateString(rString);
                break;
            default:
                xVariant->updateNull();
        }
    }
}

//------------------------------------------------------------------------------
void DBTypeConversion::setValue(const Reference<XColumnUpdate>& xVariant,
                                const Date& rNullDate,
                                const double& rValue,
                                sal_Int16 nKeyType) throw(::com::sun::star::lang::IllegalArgumentException)
{
    switch (nKeyType & ~NumberFormat::DEFINED)
    {
        case NumberFormat::DATE:
            xVariant->updateDate(toDate( rValue, rNullDate));
            break;
        case NumberFormat::DATETIME:
            xVariant->updateTimestamp(toDateTime(rValue,rNullDate));
            break;
        case NumberFormat::TIME:
            xVariant->updateTime(toTime(rValue));
            break;
        default:
            xVariant->updateDouble(rValue);
    }
}

//------------------------------------------------------------------------------
double DBTypeConversion::getValue(const Reference<XColumn>& xVariant,
                                  const Date& rNullDate,
                                  sal_Int16 nKeyType)
{
    try
    {
        switch (nKeyType & ~NumberFormat::DEFINED)
        {
            case NumberFormat::DATE:
                return toDouble( xVariant->getDate(), rNullDate);
            case NumberFormat::DATETIME:
                return toDouble(xVariant->getTimestamp(),rNullDate);
            case NumberFormat::TIME:
                return toDouble(xVariant->getTime());
            default:
                return xVariant->getDouble();
        }
    }
    catch(const Exception& )
    {
        return 0.0;
    }
}
//------------------------------------------------------------------------------
::rtl::OUString DBTypeConversion::getValue(const Reference< ::com::sun::star::beans::XPropertySet>& _xColumn,
                                           const Reference<XNumberFormatter>& _xFormatter,
                                           const ::com::sun::star::lang::Locale& _rLocale,
                                           const Date& _rNullDate)
{
    OSL_ENSURE(_xColumn.is() && _xFormatter.is(), "DBTypeConversion::getValue: invalid arg !");
    if (!_xColumn.is() || !_xFormatter.is())
        return ::rtl::OUString();

    sal_Int32 nKey;
    try
    {
        _xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FORMATKEY)) >>= nKey;
    }
    catch (const Exception& )
    {
    }

    if (!nKey)
    {
        Reference<XNumberFormats> xFormats( _xFormatter->getNumberFormatsSupplier()->getNumberFormats() );
        Reference<XNumberFormatTypes> xTypeList(_xFormatter->getNumberFormatsSupplier()->getNumberFormats(), UNO_QUERY);

        nKey = ::dbtools::getDefaultNumberFormat(_xColumn,
                                           Reference< XNumberFormatTypes > (xFormats, UNO_QUERY),
                                           _rLocale);

    }

    sal_Int16 nKeyType = getNumberFormatType(_xFormatter, nKey) & ~NumberFormat::DEFINED;

    return DBTypeConversion::getValue(Reference< XColumn > (_xColumn, UNO_QUERY), _xFormatter, _rNullDate, nKey, nKeyType);
}

//------------------------------------------------------------------------------
::rtl::OUString DBTypeConversion::getValue(const Reference<XColumn>& xVariant,
                                   const Reference<XNumberFormatter>& xFormatter,
                                   const Date& rNullDate,
                                   sal_Int32 nKey,
                                   sal_Int16 nKeyType)
{
    ::rtl::OUString aString;
    if (xVariant.is())
    {
        try
        {
            switch (nKeyType & ~NumberFormat::DEFINED)
            {
                case NumberFormat::DATE:
                case NumberFormat::DATETIME:
                {
                    double fValue = getValue(xVariant,rNullDate,nKeyType);
                    if (!xVariant->wasNull())
                        aString = xFormatter->convertNumberToString(nKey, toNullDate(rNullDate, fValue));
                }   break;
                case NumberFormat::TIME:
                case NumberFormat::NUMBER:
                case NumberFormat::SCIENTIFIC:
                case NumberFormat::FRACTION:
                case NumberFormat::PERCENT:
                {
                    double fValue = xVariant->getDouble();
                    if (!xVariant->wasNull())
                        aString = xFormatter->convertNumberToString(nKey, fValue);
                }   break;
                case NumberFormat::CURRENCY:
                {
                    double fValue = xVariant->getDouble();
                    if (!xVariant->wasNull())
                        aString = xFormatter->getInputString(nKey, fValue);
                }   break;
                case NumberFormat::TEXT:
                    aString = xFormatter->formatString(nKey, xVariant->getString());
                    break;
                default:
                    aString = xVariant->getString();
            }
        }
        catch(const Exception& )
        {
            aString = xVariant->getString();
        }
    }
    return aString;
}
//------------------------------------------------------------------
