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


#ifndef _EXTENSIONS_PROPCTRLR_LINEDESCRIPTOR_HXX_
#define _EXTENSIONS_PROPCTRLR_LINEDESCRIPTOR_HXX_

#include <vcl/image.hxx>
/** === begin UNO includes === **/
#include <com/sun/star/inspection/LineDescriptor.hpp>
/** === end UNO includes === **/

//............................................................................
namespace pcr
{
//............................................................................

    //========================================================================
    //= OLineDescriptor
    //========================================================================
    struct OLineDescriptor : public ::com::sun::star::inspection::LineDescriptor
    {
        ::rtl::OUString             sName;              // the name of the property
        ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyHandler >
                                    xPropertyHandler;   // the handler for this property
        ::com::sun::star::uno::Any  aValue;             // the current value of the property

        bool                        bUnknownValue    : 1;   // is the property value currently "unknown"? (PropertyState_AMBIGUOUS)
        bool                        bReadOnly        : 1;

        OLineDescriptor()
            :bUnknownValue( false )
            ,bReadOnly( false )
        {
        }

        void assignFrom( const ::com::sun::star::inspection::LineDescriptor& _rhs )
        {
            ::com::sun::star::inspection::LineDescriptor::operator=( _rhs );
        }
    };

//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_LINEDESCRIPTOR_HXX_

