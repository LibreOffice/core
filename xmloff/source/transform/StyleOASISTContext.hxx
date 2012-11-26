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



#ifndef _XMLOFF_STYLEOASISTCONTEXT_HXX
#define _XMLOFF_STYLEOASISTCONTEXT_HXX

#include <rtl/ref.hxx>
#include "DeepTContext.hxx"


class XMLTransformerActions;
class XMLPropertiesTContext_Impl;
class XMLStyleOASISTContext : public XMLPersElemContentTContext
{
    ::rtl::Reference< XMLPropertiesTContext_Impl > m_xPropContext;

    sal_Bool m_bPersistent;
    sal_Bool m_bControlStyle;

    ::rtl::OUString m_aStyleFamily;
public:
    XMLStyleOASISTContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName,
                              sal_Bool bPersistent=sal_False );
    XMLStyleOASISTContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName,
                              sal_uInt16 nPrefix,
                              ::xmloff::token::XMLTokenEnum eToken,
                              sal_Bool bPersistent=sal_False );

    virtual ~XMLStyleOASISTContext();

    virtual XMLTransformerContext *CreateChildContext( sal_uInt16 nPrefix,
                                   const ::rtl::OUString& rLocalName,
                                   const ::rtl::OUString& rQName,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();
    virtual void Characters( const ::rtl::OUString& rChars );

    virtual void ExportContent();

    virtual sal_Bool IsPersistent() const;

    static XMLTransformerActions *CreateTransformerActions( sal_uInt16 nType );
};

#endif  //  _XMLOFF_STYLEOASISTCONTEXT_HXX

