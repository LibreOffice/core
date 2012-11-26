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



#ifndef _XMLOFF_XMLCHANGEIMPORTCONTEXT_HXX
#define _XMLOFF_XMLCHANGEIMPORTCONTEXT_HXX


#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/uno/Reference.h>


namespace com { namespace sun { namespace star {
    namespace xml { namespace sax {
        class XAttributeList;
    } }
} } }
namespace rtl {
    class OUString;
}



/**
 * import change tracking/redlining markers
 * <text:change>, <text:change-start>, <text:change-end>
 */
class XMLChangeImportContext : public SvXMLImportContext
{
    sal_Bool bIsStart;
    sal_Bool bIsEnd;
    sal_Bool bIsOutsideOfParagraph;

public:

    /**
     * import a change mark
     * (<text:change>, <text:change-start>, <text:change-end>)
     * Note: a <text:change> mark denotes start and end of a change
     * simultaniously, so both bIsStart and bIsEnd parameters would
     * be set true.
     */
    XMLChangeImportContext(
        SvXMLImport& rImport,
        sal_Int16 nPrefix,
        const ::rtl::OUString& rLocalName,
        sal_Bool bIsStart,  /// mark start of a change
        sal_Bool bIsEnd,    /// mark end of a change
        /// true if change mark is encountered outside of a paragraph
        /// (usually before a section or table)
        sal_Bool bIsOutsideOfParagraph = sal_False);

    ~XMLChangeImportContext();

    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList> & xAttrList);
};

#endif
