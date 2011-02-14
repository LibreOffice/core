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

#include "vcl_time_handler.hxx"
#include "xmloff/xmluconv.hxx"

#include <com/sun/star/util/DateTime.hpp>

#include <tools/diagnose_ex.h>
#include <tools/time.hxx>

//......................................................................................................................
namespace xmloff
{
//......................................................................................................................

    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::util::DateTime;

    //==================================================================================================================
    //= VCLTimeHandler
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    VCLTimeHandler::VCLTimeHandler()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString VCLTimeHandler::getAttributeValue( const PropertyValues& /*i_propertyValues*/ ) const
    {
        OSL_ENSURE( false, "VCLTimeHandler::getAttributeValue: unexpected call!" );
        return ::rtl::OUString();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString VCLTimeHandler::getAttributeValue( const Any& i_propertyValue ) const
    {
        sal_Int32 nVCLTime(0);
        OSL_VERIFY( i_propertyValue >>= nVCLTime );
        ::Time aVCLTime( nVCLTime );

        DateTime aDateTime; // default-inited to 0
        aDateTime.Hours = aVCLTime.GetHour();
        aDateTime.Minutes = aVCLTime.GetMin();
        aDateTime.Seconds = aVCLTime.GetSec();
        aDateTime.HundredthSeconds = aVCLTime.Get100Sec();

        ::rtl::OUStringBuffer aBuffer;
        SvXMLUnitConverter::convertTime( aBuffer, aDateTime );
        return aBuffer.makeStringAndClear();
    }

    //------------------------------------------------------------------------------------------------------------------
    bool VCLTimeHandler::getPropertyValues( const ::rtl::OUString i_attributeValue, PropertyValues& o_propertyValues ) const
    {
        sal_Int32 nVCLTime(0);

        DateTime aDateTime;
        if ( SvXMLUnitConverter::convertTime( aDateTime, i_attributeValue ) )
        {
            ::Time aVCLTime( aDateTime.Hours, aDateTime.Minutes, aDateTime.Seconds, aDateTime.HundredthSeconds );
            nVCLTime = aVCLTime.GetTime();
        }
        else
        {
            // compatibility format, before we wrote those values in XML-schema compatible form
            if ( !SvXMLUnitConverter::convertNumber( nVCLTime, i_attributeValue ) )
            {
                OSL_ENSURE( false, "VCLTimeHandler::getPropertyValues: unknown time format (no XML-schema time, no legacy integer)!" );
                return false;
            }
        }

        const Any aPropertyValue( makeAny( nVCLTime ) );

        OSL_ENSURE( o_propertyValues.size() == 1, "VCLTimeHandler::getPropertyValues: time strings represent exactly one property - not more, not less!" );
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
