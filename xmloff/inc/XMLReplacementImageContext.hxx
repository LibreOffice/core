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



#ifndef _XMLOFF_XMLREPLACEMENTIMAGECONTEXT_HXX
#define _XMLOFF_XMLREPLACEMENTIMAGECONTEXT_HXX

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <xmloff/xmlictxt.hxx>


namespace com { namespace sun { namespace star {
    namespace beans { class XPropertySet; }
    namespace io { class XOutputStream; } } } }

class XMLReplacementImageContext : public SvXMLImportContext
{
    ::com::sun::star::uno::Reference <
        ::com::sun::star::io::XOutputStream > m_xBase64Stream;
::com::sun::star::uno::Reference <
        ::com::sun::star::beans::XPropertySet > m_xPropSet;

    ::rtl::OUString m_sHRef;
    const ::rtl::OUString m_sGraphicURL;

public:

    XMLReplacementImageContext( SvXMLImport& rImport,
            sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & rAttrList,
            const ::com::sun::star::uno::Reference <
                ::com::sun::star::beans::XPropertySet >& rPropSet );
    virtual ~XMLReplacementImageContext();

    virtual void EndElement();

    SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                const ::rtl::OUString& rLocalName,
                 const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

};

#endif  //  _XMLOFF_XMLREPLACEMENTIMAGECONTEXT_HXX

