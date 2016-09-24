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

#include "vcl_date_handler.hxx"

#include <rtl/ustrbuf.hxx>

#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Date.hpp>

#include <sax/tools/converter.hxx>

#include <tools/diagnose_ex.h>
#include <tools/date.hxx>

namespace xmloff
{

    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::util::DateTime;
    using ::com::sun::star::util::Date;

    //= VCLDateHandler
    VCLDateHandler::VCLDateHandler()
    {
    }

    OUString VCLDateHandler::getAttributeValue( const PropertyValues& /*i_propertyValues*/ ) const
    {
        OSL_ENSURE( false, "VCLDateHandler::getAttributeValue: unexpected call!" );
        return OUString();
    }

    OUString VCLDateHandler::getAttributeValue( const Any& i_propertyValue ) const
    {
        Date aDate;
        OSL_VERIFY( i_propertyValue >>= aDate );

        DateTime aDateTime; // default-inited to 0
        aDateTime.Day = aDate.Day;
        aDateTime.Month = aDate.Month;
        aDateTime.Year = aDate.Year;

        OUStringBuffer aBuffer;
        ::sax::Converter::convertDateTime( aBuffer, aDateTime, nullptr );
        return aBuffer.makeStringAndClear();
    }

    bool VCLDateHandler::getPropertyValues( const OUString& i_attributeValue, PropertyValues& o_propertyValues ) const
    {
        DateTime aDateTime;
        Date aDate;
        if (::sax::Converter::parseDateTime( aDateTime, nullptr, i_attributeValue ))
        {
            aDate.Day = aDateTime.Day;
            aDate.Month = aDateTime.Month;
            aDate.Year = aDateTime.Year;
        }
        else
        {
            // compatibility format, before we wrote those values in XML-schema compatible form
            sal_Int32 nVCLDate(0);
            if (!::sax::Converter::convertNumber(nVCLDate, i_attributeValue))
            {
                OSL_ENSURE( false, "VCLDateHandler::getPropertyValues: unknown date format (no XML-schema date, no legacy integer)!" );
                return false;
            }
            aDate = ::Date(nVCLDate).GetUNODate();
        }

        const Any aPropertyValue( makeAny( aDate ) );

        OSL_ENSURE( o_propertyValues.size() == 1, "VCLDateHandler::getPropertyValues: date strings represent exactly one property - not more, not less!" );
        for (   PropertyValues::iterator prop = o_propertyValues.begin();
                prop != o_propertyValues.end();
                ++prop
            )
        {
            prop->second = aPropertyValue;
        }
        return true;
    }

} // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
