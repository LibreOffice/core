/*************************************************************************
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

#include "precompiled_xmloff.hxx"

#include "vcl_date_handler.hxx"
#include "xmloff/xmluconv.hxx"

#include <com/sun/star/util/DateTime.hpp>

#include <tools/diagnose_ex.h>
#include <tools/date.hxx>

//......................................................................................................................
namespace xmloff
{
//......................................................................................................................

    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::util::DateTime;

    //==================================================================================================================
    //= VCLDateHandler
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    VCLDateHandler::VCLDateHandler()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString VCLDateHandler::getAttributeValue( const PropertyValues& /*i_propertyValues*/ ) const
    {
        OSL_ENSURE( false, "VCLDateHandler::getAttributeValue: unexpected call!" );
        return ::rtl::OUString();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString VCLDateHandler::getAttributeValue( const Any& i_propertyValue ) const
    {
        sal_Int32 nVCLDate(0);
        OSL_VERIFY( i_propertyValue >>= nVCLDate );
        ::Date aVCLDate( nVCLDate );

        DateTime aDateTime; // default-inited to 0
        aDateTime.Day = aVCLDate.GetDay();
        aDateTime.Month = aVCLDate.GetMonth();
        aDateTime.Year = aVCLDate.GetYear();

        ::rtl::OUStringBuffer aBuffer;
        SvXMLUnitConverter::convertDateTime( aBuffer, aDateTime, sal_False );
        return aBuffer.makeStringAndClear();
    }

    //------------------------------------------------------------------------------------------------------------------
    bool VCLDateHandler::getPropertyValues( const ::rtl::OUString i_attributeValue, PropertyValues& o_propertyValues ) const
    {
        sal_Int32 nVCLDate(0);

        DateTime aDateTime;
        if ( SvXMLUnitConverter::convertDateTime( aDateTime, i_attributeValue ) )
        {
            ::Date aVCLDate( aDateTime.Day, aDateTime.Month, aDateTime.Year );
            nVCLDate = aVCLDate.GetDate();
        }
        else
        {
            // compatibility format, before we wrote those values in XML-schema compatible form
            if ( !SvXMLUnitConverter::convertNumber( nVCLDate, i_attributeValue ) )
            {
                OSL_ENSURE( false, "VCLDateHandler::getPropertyValues: unknown date format (no XML-schema date, no legacy integer)!" );
                return false;
            }
        }

        const Any aPropertyValue( makeAny( nVCLDate ) );

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

//......................................................................................................................
} // namespace xmloff
//......................................................................................................................
