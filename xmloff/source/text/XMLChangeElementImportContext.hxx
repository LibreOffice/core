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




#ifndef _XMLOFF_XMLCHANGEELEMENTIMPORTCONTEXT_HXX
#define _XMLOFF_XMLCHANGEELEMENTIMPORTCONTEXT_HXX

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
class XMLChangedRegionImportContext;



/**
 * Import <text:deletion> and <text:insertion> elements contained in a
 * <text:changed-region> element.
 */
class XMLChangeElementImportContext : public SvXMLImportContext
{
    sal_Bool bAcceptContent;
    XMLChangedRegionImportContext& rChangedRegion;

public:

    XMLChangeElementImportContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        /// accept text content (paragraphs) in element as redline content?
        sal_Bool bAcceptContent,
        /// context of enclosing <text:changed-region> element
        XMLChangedRegionImportContext& rParent);

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList);

    // #107848#
    // Start- and EndElement are needed here to set the inside_deleted_section
    // flag at the corresponding TextImportHelper
    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    // #107848#
    virtual void EndElement();
};

#endif
