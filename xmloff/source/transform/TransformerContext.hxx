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



#ifndef _XMLOFF_TRANSFORMERCONTEXT_HXX
#define _XMLOFF_TRANSFORMERCONTEXT_HXX

#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <tools/solar.h>
#include <salhelper/simplereferenceobject.hxx>
#include <rtl/ustring.hxx>
#include <xmloff/xmltoken.hxx>

class SvXMLNamespaceMap;
class XMLTransformerBase;

class XMLTransformerContext : public ::salhelper::SimpleReferenceObject
{
    friend class XMLTransformerBase;

    XMLTransformerBase& m_rTransformer;

    ::rtl::OUString m_aQName;

    SvXMLNamespaceMap   *m_pRewindMap;

    SvXMLNamespaceMap  *GetRewindMap() const { return m_pRewindMap; }
    void SetRewindMap( SvXMLNamespaceMap *p ) { m_pRewindMap = p; }

protected:

    XMLTransformerBase& GetTransformer() { return m_rTransformer; }
    const XMLTransformerBase& GetTransformer() const { return m_rTransformer; }

    void SetQName( const ::rtl::OUString& rQName ) { m_aQName = rQName; }

public:
    const ::rtl::OUString& GetQName() const { return m_aQName; }
    sal_Bool HasQName( sal_uInt16 nPrefix,
                       ::xmloff::token::XMLTokenEnum eToken ) const;
    sal_Bool HasNamespace( sal_uInt16 nPrefix ) const;

    // A contexts constructor does anything that is required if an element
    // starts. Namespace processing has been done already.
    // Note that virtual methods cannot be used inside constructors. Use
    // StartElement instead if this is required.
    XMLTransformerContext( XMLTransformerBase& rTransformer,
                        const ::rtl::OUString& rQName );

    // A contexts destructor does anything that is required if an element
    // ends. By default, nothing is done.
    // Note that virtual methods cannot be used inside destructors. Use
    // EndElement instead if this is required.
    virtual ~XMLTransformerContext();

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
    // current element. The default is to ignore them.
    virtual void Characters( const ::rtl::OUString& rChars );

    // Is the current context a persistent one (i.e. one that saves is content
    // rather than exporting it directly?
    virtual sal_Bool IsPersistent() const;

    // Export the whole element. By default, nothing is done here
    virtual void Export();

    // Export the element content. By default, nothing is done here
    virtual void ExportContent();
};


#endif  //  _XMLOFF_TRANSFORMERCONTEXT_HXX

