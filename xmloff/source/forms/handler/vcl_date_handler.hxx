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



#ifndef XMLOFF_VCL_DATE_HANDLER_HXX
#define XMLOFF_VCL_DATE_HANDLER_HXX

#include "property_handler_base.hxx"

//......................................................................................................................
namespace xmloff
{
//......................................................................................................................

    //==================================================================================================================
    //= VCLDateHandler
    //==================================================================================================================
    class VCLDateHandler : public PropertyHandlerBase
    {
    public:
        VCLDateHandler();

        // IPropertyHandler
        virtual ::rtl::OUString getAttributeValue( const PropertyValues& i_propertyValues ) const;
        virtual ::rtl::OUString getAttributeValue( const ::com::sun::star::uno::Any& i_propertyValue ) const;
        virtual bool getPropertyValues( const ::rtl::OUString i_attributeValue, PropertyValues& o_propertyValues ) const;
    };

//......................................................................................................................
} // namespace xmloff
//......................................................................................................................

#endif // XMLOFF_VCL_DATE_HANDLER_HXX
