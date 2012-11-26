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



#ifndef _XMLOFF_PERSATTRLISTTCONTEXT_HXX
#define _XMLOFF_PERSATTRLISTTCONTEXT_HXX

#include <xmloff/xmltoken.hxx>
#include "TransformerContext.hxx"


class XMLPersAttrListTContext : public XMLTransformerContext
{

    ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList > m_xAttrList;
    ::rtl::OUString m_aElemQName;
    sal_uInt16 m_nActionMap;

protected:

    void SetExportQName( const ::rtl::OUString& r ) { m_aElemQName = r; }

public:
    // A contexts constructor does anything that is required if an element
    // starts. Namespace processing has been done already.
    // Note that virtual methods cannot be used inside constructors. Use
    // StartElement instead if this is required.
    XMLPersAttrListTContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName );

    // attr list persistence + attribute processing
    XMLPersAttrListTContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName,
                              sal_uInt16 nActionMap );

    // attr list persistence + renaming
    XMLPersAttrListTContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName,
                              sal_uInt16 nPrefix,
                              ::xmloff::token::XMLTokenEnum eToken );

    // attr list persistence + renaming + attribute processing
    XMLPersAttrListTContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName,
                              sal_uInt16 nPrefix,
                              ::xmloff::token::XMLTokenEnum eToken,
                              sal_uInt16 nActionMap );

    // A contexts destructor does anything that is required if an element
    // ends. By default, nothing is done.
    // Note that virtual methods cannot be used inside destructors. Use
    // EndElement instead if this is required.
    virtual ~XMLPersAttrListTContext();

    // Create a childs element context. By default, the import's
    // CreateContext method is called to create a new default context.
    virtual XMLTransformerContext *CreateChildContext( sal_uInt16 nPrefix,
                                   const ::rtl::OUString& rLocalName,
                                   const ::rtl::OUString& rQName,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    // StartElement is called after a context has been constructed and
    // before a elements context is parsed. It may be used for actions that
    // require virtual methods. The default is to do nothing.
    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    // EndElement is called before a context will be destructed, but
    // after a elements context has been parsed. It may be used for actions
    // that require virtual methods. The default is to do nothing.
    virtual void EndElement();

    // This method is called for all characters that are contained in the
    // current element.
    virtual void Characters( const ::rtl::OUString& rChars );

    virtual sal_Bool IsPersistent() const;
    virtual void Export();
    virtual void ExportContent();

    const ::rtl::OUString& GetExportQName() const { return m_aElemQName; }

    void AddAttribute( sal_uInt16 nAPrefix,
        ::xmloff::token::XMLTokenEnum eAToken,
           ::xmloff::token::XMLTokenEnum eVToken );

    void AddAttribute( sal_uInt16 nAPrefix,
        ::xmloff::token::XMLTokenEnum eAToken,
        const ::rtl::OUString & rValue );

    ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList >
        GetAttrList() const;
};

#endif  //  _XMLOFF_PERSATTRLISTTCONTEXT_HXX

