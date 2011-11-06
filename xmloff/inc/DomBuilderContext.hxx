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



#ifndef _XMLOFF_DOMBUILDERCONTEXT_HXX
#define _XMLOFF_DOMBUILDERCONTEXT_HXX


//
// include for parent class and members
//

#include <xmloff/xmlictxt.hxx>


//
// forward declarations
//

namespace com { namespace sun { namespace star {
    namespace xml { namespace dom {
        class XNode;
        class XDocument;
    } }
    namespace xml { namespace sax {
        class XAttributeList;
    } }
} } }
namespace rtl {
    class OUString;
}
class SvXMLImport;
class SvXMLImportContext;

/**
 * DomBuilderContext creates a DOM tree suitable for in-memory processing of
 * XML data from a sequence of SAX events */
class DomBuilderContext : public SvXMLImportContext
{
    com::sun::star::uno::Reference<com::sun::star::xml::dom::XNode> mxNode;

public:

    /** default constructor: create new DOM tree */
    DomBuilderContext( SvXMLImport& rImport,
                       sal_uInt16 nPrefix,
                       const ::rtl::OUString& rLocalName );

    /** constructor: create DOM subtree under the given node */
    DomBuilderContext( SvXMLImport& rImport,
                       sal_uInt16 nPrefix,
                       const ::rtl::OUString& rLocalName,
                       com::sun::star::uno::Reference<com::sun::star::xml::dom::XNode>& );

    virtual ~DomBuilderContext();


    //
    // access to the DOM tree
    //

    /** access the DOM tree */
    com::sun::star::uno::Reference<com::sun::star::xml::dom::XDocument> getTree();

    /** access this context's DOM sub-tree */
    com::sun::star::uno::Reference<com::sun::star::xml::dom::XNode> getNode();


    //
    // implement SvXMLImportContext methods:
    //

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void StartElement(
        const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();

    virtual void Characters( const ::rtl::OUString& rChars );
};

#endif
