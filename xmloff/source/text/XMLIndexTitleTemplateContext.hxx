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



#ifndef _XMLOFF_XMLINDEXTITLETEMPLATECONTEXT_HXX_
#define _XMLOFF_XMLINDEXTITLETEMPLATECONTEXT_HXX_

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/uno/Reference.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>


namespace com { namespace sun { namespace star {
    namespace beans { class XPropertySet; }
    namespace xml { namespace sax { class XAttributeList; } }
} } }


/**
 * Import index title templates
 */
class XMLIndexTitleTemplateContext : public SvXMLImportContext
{

    const ::rtl::OUString sTitle;
    const ::rtl::OUString sParaStyleHeading;

    // paragraph style
    ::rtl::OUString sStyleName;
    sal_Bool bStyleNameOK;

    // content
    ::rtl::OUStringBuffer sContent;

    // TOC property set
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & rTOCPropertySet;

public:

     XMLIndexTitleTemplateContext(
        SvXMLImport& rImport,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropSet,
        sal_uInt16 nPrfx,
        const ::rtl::OUString& rLocalName);

    ~XMLIndexTitleTemplateContext();

protected:

    /** process parameters */
    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList);

    /** set values */
    virtual void EndElement();

    /** pick up title characters */
    virtual void Characters(const ::rtl::OUString& sString);
};

#endif
