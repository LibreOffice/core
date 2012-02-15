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
