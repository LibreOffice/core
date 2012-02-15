/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
