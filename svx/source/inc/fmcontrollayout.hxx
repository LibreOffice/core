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



#ifndef SVX_SOURCE_INC_FMCONTROLLAYOUT_HXX
#define SVX_SOURCE_INC_FMCONTROLLAYOUT_HXX

/** === begin UNO includes === **/
#include <com/sun/star/beans/XPropertySet.hpp>
/** === end UNO includes === **/

#ifndef SVX_SOURCE_FORM_FMDOCUMENTCLASSIFICATION_HXX
#include "fmdocumentclassification.hxx"
#endif
#include <unotools/confignode.hxx>

//........................................................................
namespace svxform
{
//........................................................................

    //====================================================================
    //= ControlLayouter
    //====================================================================
    class ControlLayouter
    {
    public:
        /** initializes the layout of a newly created form control (model)
        */
        static void initializeControlLayout(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControlModel,
                        DocumentType _eDocType
                    );

        /** determines whether for the given document type, dynamic control border coloring is enabled
        */
        static bool useDynamicBorderColor( DocumentType _eDocType );

        /** determines whether for the given document type, form controls should use the document's reference device
            for text rendering
        */
        static bool useDocumentReferenceDevice( DocumentType _eDocType );

        /** gets the "default" style in a document which can be used if some default text format is needed

            It depends on the type document type which concrete kind of style is returned, but it is expected to support
            the css.style.CharacterProperties service.

            @param _rxModel
                a form component.
        */
        static ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                    getDefaultDocumentTextStyle( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxModel );

    private:
        ControlLayouter();    // never implemented

    private:
        static ::utl::OConfigurationNode
            getLayoutSettings(
                DocumentType _eDocType
            );
    };

//........................................................................
} // namespace svxform
//........................................................................

#endif // SVX_SOURCE_INC_FMCONTROLLAYOUT_HXX

