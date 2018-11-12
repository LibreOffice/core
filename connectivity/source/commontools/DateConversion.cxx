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

#include <connectivity/dbconversion.hxx>
#include <connectivity/dbtools.hxx>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/sdb/XColumnUpdate.hpp>
#include <com/sun/star/sdb/XColumn.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/extract.hxx>
#include <TConnection.hxx>
#include <comphelper/numbers.hxx>
#include <comphelper/types.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <tools/diagnose_ex.h>


using namespace ::connectivity;
using namespace ::comphelper;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::dbtools;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::uno;

OUString DBTypeConversion::toSQLString(sal_Int32 eType, const Any& _rVal,
                                              const Reference< XTypeConverter >&  _rxTypeConverter)
{
    OUStringBuffer aRet;
    if (_rVal.hasValue())
    {
        try
        {
            switch (eType)
            {
                case DataType::INTEGER:
                case DataType::BIT:
                case DataType::BOOLEAN:
                case DataType::TINYINT:
                case DataType::SMALLINT:
                    if (_rVal.getValueType().getTypeClass() == css::uno::TypeClass_BOOLEAN)
                    {
                        if (::cppu::any2bool(_rVal))
                            aRet.append("1");
                        else
                            aRet.append("0");
                    }
                    else
                    {
                        OUString sTemp;
                        _rxTypeConverter->convertToSimpleType(_rVal, TypeClass_STRING) >>= sTemp;
                        aRet.append(sTemp);
                    }
                    break;
                case DataType::CHAR:
                case DataType::VARCHAR:
                case DataType::LONGVARCHAR:
                    aRet.append("'");
                    {
                        OUString aTemp;
                        _rxTypeConverter->convertToSimpleType(_rVal, TypeClass_STRING) >>= aTemp;
                        sal_Int32 nIndex = sal_Int32(-2);
                        const OUString sQuot("\'");
                        do
                        {
                            nIndex += 2;
                            nIndex = aTemp.indexOf(sQuot,nIndex);
                            if(nIndex != -1)
                                aTemp = aTemp.replaceAt(nIndex,sQuot.getLength(), "\'\'");
                        } while (nIndex != -1);

                        aRet.append(aTemp);
                    }
                    aRet.append("'");
                    break;
                case DataType::REAL:
                case DataType::DOUBLE:
                case DataType::DECIMAL:
                case DataType::NUMERIC:
                case DataType::BIGINT:
                default:
                    {
                        OUString sTemp;
                        _rxTypeConverter->convertToSimpleType(_rVal, TypeClass_STRING) >>= sTemp;
                        aRet.append(sTemp);
                    }
                    break;
                case DataType::TIMESTAMP:
                {
                    DateTime aDateTime;
                    bool bOk = false;
                    if (_rVal.getValueType().getTypeClass() == css::uno::TypeClass_DOUBLE)
                    {
                        double nValue = 0.0;
                        _rVal >>= nValue;
                        aDateTime = DBTypeConversion::toDateTime(nValue);
                        bOk = true;
                    }
                    else if (_rVal.getValueType().getTypeClass() == css::uno::TypeClass_STRING)
                    {
                        OUString sValue;
                        _rVal >>= sValue;
                        aDateTime = DBTypeConversion::toDateTime(sValue);
                        bOk = true;
                    }
                    else
                        bOk = _rVal >>= aDateTime;

                    OSL_ENSURE( bOk, "DBTypeConversion::toSQLString: _rVal is not datetime!");
                    // check if this is really a timestamp or only a date
                    if ( bOk )
                    {
                        aRet.append("{ts '");
                        aRet.append(DBTypeConversion::toDateTimeString(aDateTime));
                        aRet.append("'}");
                        break;
                    }
                    break;
                }
                case DataType::DATE:
                {
                    Date aDate;
                    bool bOk = false;
                    if (_rVal.getValueType().getTypeClass() == css::uno::TypeClass_DOUBLE)
                    {
                        double nValue = 0.0;
                        _rVal >>= nValue;
                        aDate = DBTypeConversion::toDate(nValue);
                        bOk = true;
                    }
                    else if (_rVal.getValueType().getTypeClass() == css::uno::TypeClass_STRING)
                    {
                        OUString sValue;
                        _rVal >>= sValue;
                        aDate = DBTypeConversion::toDate(sValue);
                        bOk = true;
                    }
                    else
                        bOk = _rVal >>= aDate;
                    OSL_ENSURE( bOk, "DBTypeConversion::toSQLString: _rVal is not date!");
                    aRet.append("{d '");
                    aRet.append(DBTypeConversion::toDateString(aDate));
                    aRet.append("'}");
                }   break;
                case DataType::TIME:
                {
                    css::util::Time aTime;
                    bool bOk = false;
                    if (_rVal.getValueType().getTypeClass() == css::uno::TypeClass_DOUBLE)
                    {
                        double nValue = 0.0;
                        _rVal >>= nValue;
                        aTime = DBTypeConversion::toTime(nValue);
                        bOk = true;
                    }
                    else if (_rVal.getValueType().getTypeClass() == css::uno::TypeClass_STRING)
                    {
                        OUString sValue;
                        _rVal >>= sValue;
                        aTime = DBTypeConversion::toTime(sValue);
                        bOk = true;
                    }
                    else
                        bOk = _rVal >>= aTime;
                    OSL_ENSURE( bOk,"DBTypeConversion::toSQLString: _rVal is not time!");
                    aRet.append("{t '");
                    aRet.append(DBTypeConversion::toTimeString(aTime));
                    aRet.append("'}");
                } break;
            }
        }
        catch ( const Exception&  )
        {
            OSL_FAIL("TypeConversion Error");
        }
    }
    else
        aRet.append(" NULL ");
    return aRet.makeStringAndClear();
}

Date DBTypeConversion::getNULLDate(const Reference< XNumberFormatsSupplier > &xSupplier)
{
    OSL_ENSURE(xSupplier.is(), "getNULLDate : the formatter doesn't implement a supplier !");
    if (xSupplier.is())
    {
        try
        {
            // get the null date
            Date aDate;
            xSupplier->getNumberFormatSettings()->getPropertyValue("NullDate") >>= aDate;
            return aDate;
        }
        catch ( const Exception&  )
        {
        }
    }

    return getStandardDate();
}

void DBTypeConversion::setValue(const Reference<XColumnUpdate>& xVariant,
                                const Reference<XNumberFormatter>& xFormatter,
                                const Date& rNullDate,
                                const OUString& rString,
                                sal_Int32 nKey,
                                sal_Int16 nFieldType,
                                sal_Int16 nKeyType)
{
    if (!rString.isEmpty())
    {
        // Does the String need to be formatted?
        sal_Int16 nTypeClass = nKeyType & ~NumberFormat::DEFINED;
        bool bTextFormat = nTypeClass == NumberFormat::TEXT;
        sal_Int32 nKeyToUse  = bTextFormat ? 0 : nKey;
        sal_Int16 nRealUsedTypeClass = nTypeClass;
        // for a Text-Format the formatter needs some more freedom, otherwise
        // convertStringToNumber will throw a NotNumericException
        try
        {
            double fValue = xFormatter->convertStringToNumber(nKeyToUse, rString);
            Reference< XNumberFormats > xFormats(xFormatter->getNumberFormatsSupplier()->getNumberFormats());
            Reference< XNumberFormatTypes > xFormatTypes(xFormats, UNO_QUERY);
            sal_Int32 nStandardKey(0);
            if(xFormatTypes.is())
            {
                const Reference< XPropertySet > xFormatProps(xFormats->getByKey(nKeyToUse));
                if (xFormatProps.is())
                {
                    css::lang::Locale loc;
                    if (xFormatProps->getPropertyValue("Locale") >>= loc)
                        nStandardKey = xFormatTypes->getStandardIndex(loc);
                    else
                    {
                        assert(false);
                    }
                }
                else
                {
                    SAL_WARN("connectivity.commontools", "no format by key " << nKeyToUse);
                }
            }
            else
            {
                assert(false);
            }
            // Why use nStandardKey rather than nKeyToUse here? I'm not sure, but "it was always like that".
            // Previously had hardcoded 0 instead of nStandardKey, which led to problems with dates
            // because of differences M/D/Y vs D/M/Y. This at least fixes those problems, but possibly
            // nKeyToUse is an even better choice than nStandardKey.
            // OTOH, using nKeyToUse nullifies the special treatment for percent formats,
            //       leading to "5" (in a percent format) to be understood as "500%" instead of "5%".
            sal_Int32 nRealUsedKey = xFormatter->detectNumberFormat(nStandardKey, rString);
            if (nRealUsedKey != nKeyToUse)
                nRealUsedTypeClass = getNumberFormatType(xFormatter, nRealUsedKey) & ~NumberFormat::DEFINED;

            // and again a special treatment, this time for percent formats
            if ((NumberFormat::NUMBER == nRealUsedTypeClass) && (NumberFormat::PERCENT == nTypeClass))
            {   // formatting should be "percent", but the String provides just a simple number -> adjust
                OUString sExpanded = rString + "%";
                fValue = xFormatter->convertStringToNumber(nKeyToUse, sExpanded);
            }

            switch (nRealUsedTypeClass)
            {
                case NumberFormat::DATE:
                case NumberFormat::DATETIME:
                case NumberFormat::TIME:
                    DBTypeConversion::setValue(xVariant,rNullDate,fValue,nRealUsedTypeClass);
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
            case css::sdbc::DataType::CHAR:
            case css::sdbc::DataType::VARCHAR:
            case css::sdbc::DataType::LONGVARCHAR:
                xVariant->updateString(rString);
                break;
            default:
                xVariant->updateNull();
        }
    }
}


void DBTypeConversion::setValue(const Reference<XColumnUpdate>& xVariant,
                                const Date& rNullDate,
                                const double& rValue,
                                sal_Int16 nKeyType)
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
            {
                double nValue = rValue;
//              Reference<XPropertySet> xProp(xVariant,UNO_QUERY);
//              if (    xProp.is()
//                  &&  xProp->getPropertySetInfo()->hasPropertyByName(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISSIGNED))
//                  && !::comphelper::getBOOL(xProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISSIGNED))) )
//              {
//                  switch (::comphelper::getINT32(xProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE))))
//                  {
//                      case DataType::TINYINT:
//                          nValue = static_cast<sal_uInt8>(rValue);
//                          break;
//                      case DataType::SMALLINT:
//                          nValue = static_cast<sal_uInt16>(rValue);
//                          break;
//                      case DataType::INTEGER:
//                          nValue = static_cast<sal_uInt32>(rValue);
//                          break;
//                      case DataType::BIGINT:
//                          nValue = static_cast<sal_uInt64>(rValue);
//                          break;
//                  }
//              }
                xVariant->updateDouble(nValue);
            }
    }
}


double DBTypeConversion::getValue( const Reference< XColumn >& i_column, const Date& i_relativeToNullDate )
{
    try
    {
        const Reference< XPropertySet > xProp( i_column, UNO_QUERY_THROW );

        const sal_Int32 nColumnType = ::comphelper::getINT32( xProp->getPropertyValue( OMetaConnection::getPropMap().getNameByIndex( PROPERTY_ID_TYPE ) ) );
        switch ( nColumnType )
        {
        case DataType::DATE:
            return toDouble( i_column->getDate(), i_relativeToNullDate );

        case DataType::TIME:
            return toDouble( i_column->getTime() );

        case DataType::TIMESTAMP:
            return toDouble( i_column->getTimestamp(), i_relativeToNullDate );

        default:
            {
                bool bIsSigned = true;
                OSL_VERIFY( xProp->getPropertyValue( OMetaConnection::getPropMap().getNameByIndex( PROPERTY_ID_ISSIGNED ) ) >>= bIsSigned );
                if ( !bIsSigned )
                {
                    switch ( nColumnType)
                    {
                        case DataType::TINYINT:
                            return static_cast<double>(static_cast<sal_uInt8>(i_column->getByte()));
                        case DataType::SMALLINT:
                            return static_cast<double>(static_cast<sal_uInt16>(i_column->getShort()));
                        case DataType::INTEGER:
                            return static_cast<double>(static_cast<sal_uInt32>(i_column->getInt()));
                        case DataType::BIGINT:
                            return static_cast<double>(static_cast<sal_uInt64>(i_column->getLong()));
                    }
                }
            }
            return i_column->getDouble();
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("connectivity.commontools");
        return 0.0;
    }
}

OUString DBTypeConversion::getFormattedValue(const Reference< XPropertySet>& _xColumn,
                                           const Reference<XNumberFormatter>& _xFormatter,
                                           const css::lang::Locale& _rLocale,
                                           const Date& _rNullDate)
{
    OSL_ENSURE(_xColumn.is() && _xFormatter.is(), "DBTypeConversion::getFormattedValue: invalid arg !");
    if (!_xColumn.is() || !_xFormatter.is())
        return OUString();

    sal_Int32 nKey(0);
    try
    {
        _xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FORMATKEY)) >>= nKey;
    }
    catch (const Exception& )
    {
        OSL_FAIL("DBTypeConversion::getValue: caught an exception while asking for the format key!");
    }

    if (!nKey)
    {
        Reference<XNumberFormats> xFormats( _xFormatter->getNumberFormatsSupplier()->getNumberFormats() );

        nKey = ::dbtools::getDefaultNumberFormat(_xColumn,
                                           Reference< XNumberFormatTypes > (xFormats, UNO_QUERY),
                                           _rLocale);

    }

    sal_Int16 nKeyType = getNumberFormatType(_xFormatter, nKey) & ~NumberFormat::DEFINED;

    return DBTypeConversion::getFormattedValue(Reference< XColumn > (_xColumn, UNO_QUERY), _xFormatter, _rNullDate, nKey, nKeyType);
}


OUString DBTypeConversion::getFormattedValue(const Reference<XColumn>& xVariant,
                                   const Reference<XNumberFormatter>& xFormatter,
                                   const Date& rNullDate,
                                   sal_Int32 nKey,
                                   sal_Int16 nKeyType)
{
    OUString aString;
    if (xVariant.is())
    {
        try
        {
            switch (nKeyType & ~NumberFormat::DEFINED)
            {
                case NumberFormat::DATE:
                case NumberFormat::DATETIME:
                {
                    // get a value which represents the given date, relative to the given null date
                    double fValue = getValue( xVariant, rNullDate );
                    if ( !xVariant->wasNull() )
                    {
                         // get the null date of the formatter
                         Date aFormatterNullDate( rNullDate );
                         try
                         {
                             Reference< XNumberFormatsSupplier > xSupplier( xFormatter->getNumberFormatsSupplier(), UNO_SET_THROW );
                             Reference< XPropertySet > xFormatterSettings( xSupplier->getNumberFormatSettings(), UNO_SET_THROW );
                             OSL_VERIFY( xFormatterSettings->getPropertyValue("NullDate") >>= aFormatterNullDate );
                         }
                         catch( const Exception& )
                         {
                            DBG_UNHANDLED_EXCEPTION("connectivity.commontools");
                         }
                         // get a value which represents the given date, relative to the null date of the formatter
                         fValue -= toDays( rNullDate, aFormatterNullDate );
                         // format this value
                         aString = xFormatter->convertNumberToString( nKey, fValue );
                    }
                }
                break;
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
