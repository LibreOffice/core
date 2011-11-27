/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include "vcl_time_handler.hxx"

#include <rtl/ustrbuf.hxx>

#include <com/sun/star/util/Duration.hpp>

#include <sax/tools/converter.hxx>

#include <tools/diagnose_ex.h>
#include <tools/time.hxx>

//......................................................................................................................
namespace xmloff
{
//......................................................................................................................

    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::util::Duration;

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

        Duration aDuration; // default-inited to 0
        aDuration.Hours = aVCLTime.GetHour();
        aDuration.Minutes = aVCLTime.GetMin();
        aDuration.Seconds = aVCLTime.GetSec();
        aDuration.MilliSeconds = aVCLTime.Get100Sec() * 10;

        ::rtl::OUStringBuffer aBuffer;
        ::sax::Converter::convertDuration( aBuffer, aDuration );
        return aBuffer.makeStringAndClear();
    }

    //------------------------------------------------------------------------------------------------------------------
    bool VCLTimeHandler::getPropertyValues( const ::rtl::OUString i_attributeValue, PropertyValues& o_propertyValues ) const
    {
        sal_Int32 nVCLTime(0);

        Duration aDuration;
        if (::sax::Converter::convertDuration( aDuration, i_attributeValue ))
        {
            ::Time aVCLTime(aDuration.Hours, aDuration.Minutes,
                    aDuration.Seconds, aDuration.MilliSeconds / 10);
            nVCLTime = aVCLTime.GetTime();
        }
        else
        {
            // compatibility format, before we wrote those values in XML-schema compatible form
            if (!::sax::Converter::convertNumber(nVCLTime, i_attributeValue))
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
