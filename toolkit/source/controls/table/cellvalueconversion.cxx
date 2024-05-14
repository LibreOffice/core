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

#include "cellvalueconversion.hxx"

#include <com/sun/star/util/NumberFormatsSupplier.hpp>
#include <com/sun/star/util/NumberFormatter.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <sal/log.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <tools/long.hxx>
#include <unotools/syslocale.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <comphelper/processfactory.hxx>

#include <limits>
#include <memory>

namespace svt
{
using namespace ::com::sun::star::uno;
using ::com::sun::star::util::XNumberFormatter;
using ::com::sun::star::util::NumberFormatter;
using ::com::sun::star::util::XNumberFormatsSupplier;
using ::com::sun::star::util::NumberFormatsSupplier;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::lang::Locale;
using ::com::sun::star::util::DateTime;
using ::com::sun::star::util::XNumberFormatTypes;

namespace NumberFormat = ::com::sun::star::util::NumberFormat;

//= helper

namespace
{
double lcl_convertDateToDays(sal_uInt16 const i_day, sal_uInt16 const i_month,
                             sal_Int16 const i_year)
{
    tools::Long const nNullDateDays = ::Date::DateToDays(1, 1, 1900);
    tools::Long const nValueDateDays = ::Date::DateToDays(i_day, i_month, i_year);

    return nValueDateDays - nNullDateDays;
}

double lcl_convertTimeToDays(tools::Long const i_hours, tools::Long const i_minutes,
                             tools::Long const i_seconds, tools::Long const i_100thSeconds)
{
    return tools::Time(i_hours, i_minutes, i_seconds, i_100thSeconds).GetTimeInDays();
}
}

//= StandardFormatNormalizer

StandardFormatNormalizer::StandardFormatNormalizer(Reference<XNumberFormatter> const& i_formatter,
                                                   ::sal_Int32 const i_numberFormatType)
    : m_nFormatKey(0)
{
    try
    {
        ENSURE_OR_THROW(i_formatter.is(), "StandardFormatNormalizer: no formatter!");
        Reference<XNumberFormatsSupplier> const xSupplier(i_formatter->getNumberFormatsSupplier(),
                                                          UNO_SET_THROW);
        Reference<XNumberFormatTypes> const xTypes(xSupplier->getNumberFormats(), UNO_QUERY_THROW);
        m_nFormatKey = xTypes->getStandardFormat(i_numberFormatType,
                                                 SvtSysLocale().GetLanguageTag().getLocale());
    }
    catch (const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("svtools.table");
    }
}

//= DoubleNormalization

namespace
{
class DoubleNormalization : public StandardFormatNormalizer
{
public:
    explicit DoubleNormalization(Reference<XNumberFormatter> const& i_formatter)
        : StandardFormatNormalizer(i_formatter, NumberFormat::NUMBER)
    {
    }

    virtual double convertToDouble(Any const& i_value) const override
    {
        double returnValue = std::numeric_limits<double>::quiet_NaN();
        OSL_VERIFY(i_value >>= returnValue);
        return returnValue;
    }
};

//= IntegerNormalization

class IntegerNormalization : public StandardFormatNormalizer
{
public:
    explicit IntegerNormalization(Reference<XNumberFormatter> const& i_formatter)
        : StandardFormatNormalizer(i_formatter, NumberFormat::NUMBER)
    {
    }

    virtual double convertToDouble(Any const& i_value) const override
    {
        sal_Int64 value(0);
        OSL_VERIFY(i_value >>= value);
        return value;
    }
};

//= BooleanNormalization

class BooleanNormalization : public StandardFormatNormalizer
{
public:
    explicit BooleanNormalization(Reference<XNumberFormatter> const& i_formatter)
        : StandardFormatNormalizer(i_formatter, NumberFormat::LOGICAL)
    {
    }

    virtual double convertToDouble(Any const& i_value) const override
    {
        bool value(false);
        OSL_VERIFY(i_value >>= value);
        return value ? 1 : 0;
    }
};

//= DateTimeNormalization

class DateTimeNormalization : public StandardFormatNormalizer
{
public:
    explicit DateTimeNormalization(Reference<XNumberFormatter> const& i_formatter)
        : StandardFormatNormalizer(i_formatter, NumberFormat::DATETIME)
    {
    }

    virtual double convertToDouble(Any const& i_value) const override
    {
        double returnValue = std::numeric_limits<double>::quiet_NaN();

        // extract actual UNO value
        DateTime aDateTimeValue;
        ENSURE_OR_RETURN(i_value >>= aDateTimeValue, "allowed for DateTime values only",
                         returnValue);

        // date part
        returnValue
            = lcl_convertDateToDays(aDateTimeValue.Day, aDateTimeValue.Month, aDateTimeValue.Year);

        // time part
        returnValue += lcl_convertTimeToDays(aDateTimeValue.Hours, aDateTimeValue.Minutes,
                                             aDateTimeValue.Seconds, aDateTimeValue.NanoSeconds);

        // done
        return returnValue;
    }
};

//= DateNormalization

class DateNormalization : public StandardFormatNormalizer
{
public:
    explicit DateNormalization(Reference<XNumberFormatter> const& i_formatter)
        : StandardFormatNormalizer(i_formatter, NumberFormat::DATE)
    {
    }

    virtual double convertToDouble(Any const& i_value) const override
    {
        double returnValue = std::numeric_limits<double>::quiet_NaN();

        // extract
        css::util::Date aDateValue;
        ENSURE_OR_RETURN(i_value >>= aDateValue, "allowed for Date values only", returnValue);

        // convert
        returnValue = lcl_convertDateToDays(aDateValue.Day, aDateValue.Month, aDateValue.Year);

        // done
        return returnValue;
    }
};

//= TimeNormalization

class TimeNormalization : public StandardFormatNormalizer
{
public:
    explicit TimeNormalization(Reference<XNumberFormatter> const& i_formatter)
        : StandardFormatNormalizer(i_formatter, NumberFormat::TIME)
    {
    }

    virtual double convertToDouble(Any const& i_value) const override
    {
        double returnValue = std::numeric_limits<double>::quiet_NaN();

        // extract
        css::util::Time aTimeValue;
        ENSURE_OR_RETURN(i_value >>= aTimeValue, "allowed for tools::Time values only",
                         returnValue);

        // convert
        returnValue += lcl_convertTimeToDays(aTimeValue.Hours, aTimeValue.Minutes,
                                             aTimeValue.Seconds, aTimeValue.NanoSeconds);

        // done
        return returnValue;
    }
};
}

//= operations

bool CellValueConversion::ensureNumberFormatter()
{
    if (bAttemptedFormatterCreation)
        return xNumberFormatter.is();
    bAttemptedFormatterCreation = true;

    try
    {
        Reference<XComponentContext> xContext = ::comphelper::getProcessComponentContext();
        // a number formatter
        Reference<XNumberFormatter> const xFormatter(NumberFormatter::create(xContext),
                                                     UNO_QUERY_THROW);

        // a supplier of number formats
        Locale aLocale = SvtSysLocale().GetLanguageTag().getLocale();

        Reference<XNumberFormatsSupplier> const xSupplier
            = NumberFormatsSupplier::createWithLocale(xContext, aLocale);

        // ensure a NullDate we will assume later on
        css::util::Date const aNullDate(1, 1, 1900);
        Reference<XPropertySet> const xFormatSettings(xSupplier->getNumberFormatSettings(),
                                                      UNO_SET_THROW);
        xFormatSettings->setPropertyValue(u"NullDate"_ustr, Any(aNullDate));

        // knit
        xFormatter->attachNumberFormatsSupplier(xSupplier);

        // done
        xNumberFormatter = xFormatter;
    }
    catch (const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("svtools.table");
    }

    return xNumberFormatter.is();
}

bool CellValueConversion::getValueNormalizer(Type const& i_valueType,
                                             std::shared_ptr<StandardFormatNormalizer>& o_formatter)
{
    auto pos = aNormalizers.find(i_valueType.getTypeName());
    if (pos == aNormalizers.end())
    {
        // never encountered this type before
        o_formatter.reset();

        OUString const sTypeName(i_valueType.getTypeName());
        TypeClass const eTypeClass = i_valueType.getTypeClass();

        if (sTypeName == ::cppu::UnoType<DateTime>::get().getTypeName())
        {
            o_formatter = std::make_shared<DateTimeNormalization>(xNumberFormatter);
        }
        else if (sTypeName == ::cppu::UnoType<css::util::Date>::get().getTypeName())
        {
            o_formatter = std::make_shared<DateNormalization>(xNumberFormatter);
        }
        else if (sTypeName == ::cppu::UnoType<css::util::Time>::get().getTypeName())
        {
            o_formatter = std::make_shared<TimeNormalization>(xNumberFormatter);
        }
        else if (sTypeName == ::cppu::UnoType<sal_Bool>::get().getTypeName())
        {
            o_formatter = std::make_shared<BooleanNormalization>(xNumberFormatter);
        }
        else if (sTypeName == ::cppu::UnoType<double>::get().getTypeName()
                 || sTypeName == ::cppu::UnoType<float>::get().getTypeName())
        {
            o_formatter = std::make_shared<DoubleNormalization>(xNumberFormatter);
        }
        else if ((eTypeClass == TypeClass_BYTE) || (eTypeClass == TypeClass_SHORT)
                 || (eTypeClass == TypeClass_UNSIGNED_SHORT) || (eTypeClass == TypeClass_LONG)
                 || (eTypeClass == TypeClass_UNSIGNED_LONG) || (eTypeClass == TypeClass_HYPER))
        {
            o_formatter = std::make_shared<IntegerNormalization>(xNumberFormatter);
        }
        else
        {
            SAL_WARN("svtools.table", "unsupported type '" << sTypeName << "'!");
        }
        aNormalizers[sTypeName] = o_formatter;
    }
    else
        o_formatter = pos->second;

    return bool(o_formatter);
}

//= CellValueConversion

CellValueConversion::CellValueConversion()
    : xNumberFormatter()
    , bAttemptedFormatterCreation(false)
    , aNormalizers()
{
}

CellValueConversion::~CellValueConversion() {}

OUString CellValueConversion::convertToString(const Any& i_value)
{
    OUString sStringValue;
    if (!i_value.hasValue())
        return sStringValue;

    if (!(i_value >>= sStringValue))
    {
        if (ensureNumberFormatter())
        {
            std::shared_ptr<StandardFormatNormalizer> pNormalizer;
            if (getValueNormalizer(i_value.getValueType(), pNormalizer))
            {
                try
                {
                    double const formatterCompliantValue = pNormalizer->convertToDouble(i_value);
                    sal_Int32 const formatKey = pNormalizer->getFormatKey();
                    sStringValue = xNumberFormatter->convertNumberToString(formatKey,
                                                                           formatterCompliantValue);
                }
                catch (const Exception&)
                {
                    DBG_UNHANDLED_EXCEPTION("svtools.table");
                }
            }
        }
    }

    return sStringValue;
}

} // namespace svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
