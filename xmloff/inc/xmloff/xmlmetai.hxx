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



#ifndef XMLOFF_XMLMETAI_HXX
#define XMLOFF_XMLMETAI_HXX

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include <xmloff/xmlictxt.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>


/// handles the top-level office:document-meta element of meta.xml documents
// NB: virtual inheritance is needed so that the context that handles the
//     flat xml file format can multiply inherit properly
class XMLOFF_DLLPUBLIC SvXMLMetaDocumentContext
    : public virtual SvXMLImportContext
{
private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::document::XDocumentProperties> mxDocProps;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XDocumentHandler> mxDocBuilder;

public:
    SvXMLMetaDocumentContext(SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::document::XDocumentProperties>& xDocProps,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XDocumentHandler>& xDocBuilder);

    virtual ~SvXMLMetaDocumentContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
        const rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void StartElement( const ::com::sun::star::uno::Reference<
             ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void EndElement();

public:
    static void setBuildId(const ::rtl::OUString & rGenerator,
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& xImportInfo );
};

#endif // _XMLOFF_XMLMETAI_HXX

