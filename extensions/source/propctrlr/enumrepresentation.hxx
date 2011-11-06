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



#ifndef EXTENSIONS_SOURCE_PROPCTRLR_ENUMREPRESENTATION_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_ENUMREPRESENTATION_HXX

/** === begin UNO includes === **/
#include <com/sun/star/uno/Any.hxx>
/** === end UNO includes === **/
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>

#include <vector>

//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= IPropertyEnumRepresentation
    //====================================================================
    class SAL_NO_VTABLE IPropertyEnumRepresentation : public ::rtl::IReference
    {
    public:
        /** retrieves all descriptions of all possible values of the enumeration property
        */
        virtual ::std::vector< ::rtl::OUString > SAL_CALL getDescriptions(
            ) const = 0;

        /** converts a given description into a property value
        */
        virtual void SAL_CALL getValueFromDescription(
                const ::rtl::OUString& _rDescription,
                ::com::sun::star::uno::Any& _out_rValue
            ) const = 0;

        /** converts a given property value into a description
        */
        virtual ::rtl::OUString SAL_CALL getDescriptionForValue(
                const ::com::sun::star::uno::Any& _rEnumValue
            ) const = 0;

        virtual ~IPropertyEnumRepresentation() { };
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_ENUMREPRESENTATION_HXX

