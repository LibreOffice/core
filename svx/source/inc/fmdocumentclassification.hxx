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



#ifndef SVX_SOURCE_INC_FMDOCUMENTCLASSIFICATION_HXX
#define SVX_SOURCE_INC_FMDOCUMENTCLASSIFICATION_HXX

/** === begin UNO includes === **/
#include <com/sun/star/frame/XModel.hpp>
/** === end UNO includes === **/

//........................................................................
namespace svxform
{
//........................................................................

    //====================================================================
    //= DocumentType
    //====================================================================
    enum DocumentType
    {
        eTextDocument,
        eWebDocument,
        eSpreadsheetDocument,
        eDrawingDocument,
        ePresentationDocument,
        eEnhancedForm,
        eDatabaseForm,
        eDatabaseReport,

        eUnknownDocumentType
    };

    //====================================================================
    //= DocumentClassification
    //====================================================================
    class DocumentClassification
    {
    public:
        /** classifies a document model
        */
        static DocumentType classifyDocument(
                                const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxDocumentModel
                            ) SAL_THROW(());

        static DocumentType classifyHostDocument(
                                const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxFormComponent
                            ) SAL_THROW(());

        static  DocumentType getDocumentTypeForModuleIdentifier(
                                const ::rtl::OUString& _rModuleIdentifier
                            );

        static  ::rtl::OUString getModuleIdentifierForDocumentType(
                                DocumentType _eType
                            );
    };

//........................................................................
} // namespace svxform
//........................................................................

#endif // SVX_SOURCE_INC_FMDOCUMENTCLASSIFICATION_HXX

