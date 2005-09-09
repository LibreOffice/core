/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DomBuilderContext.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:35:15 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _XMLOFF_DOMBUILDERCONTEXT_HXX
#define _XMLOFF_DOMBUILDERCONTEXT_HXX


//
// include for parent class and members
//

#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include "Reference.hxx"
#endif


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
                       USHORT nPrefix,
                       const ::rtl::OUString& rLocalName );

    /** constructor: create DOM subtree under the given node */
    DomBuilderContext( SvXMLImport& rImport,
                       USHORT nPrefix,
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
        USHORT nPrefix,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void StartElement(
        const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void EndElement();

    virtual void Characters( const ::rtl::OUString& rChars );
};

#endif
