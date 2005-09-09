/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TokenContext.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 16:02:17 $
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

#ifndef _XMLOFF_TOKENCONTEXT_HXX
#define _XMLOFF_TOKENCONTEXT_HXX


//
// include for parent class and members
//

#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif


//
// forward declarations
//

namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
    namespace uno { template<typename T> class Reference; }
} } }
namespace rtl { class OUString; }
class SvXMLImport;

#define TOKEN_MAP_ENTRY(NAMESPACE,TOKEN) { XML_NAMESPACE_##NAMESPACE, xmloff::token::XML_##TOKEN, xmloff::token::XML_##TOKEN }

extern struct SvXMLTokenMapEntry aEmptyMap[1];

/** handle attributes through an SvXMLTokenMap */
class TokenContext : public SvXMLImportContext
{
protected:
    const SvXMLTokenMapEntry* mpAttributes;    /// static token map
    const SvXMLTokenMapEntry* mpChildren;      /// static token map

public:

    TokenContext( SvXMLImport& rImport,
                  USHORT nPrefix,
                  const ::rtl::OUString& rLocalName,
                  const SvXMLTokenMapEntry* pAttributes = NULL,
                  const SvXMLTokenMapEntry* pChildren = NULL );

    virtual ~TokenContext();


    //
    // implement SvXMLImportContext methods:
    //

    /** call HandleAttribute for each attribute in the token map;
     * create a warning for all others. Classes that wish to override
     * StartElement need to call the parent method. */
    virtual void StartElement(
        const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList>& xAttrList );

    /** call HandleChild for each child element in the token map;
     * create a warning for all others. Classes that wish to override
     * CreateChildCotnenxt may want to call the parent method for
     * handling of defaults. */
    virtual SvXMLImportContext* CreateChildContext(
        USHORT nPrefix,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList>& xAttrList );

    /** Create a warning for all non-namespace character
     * content. Classes that wish to deal with character content have
     * to overload this method anyway, and will thus get rid of the
     * warnings. */
    virtual void Characters( const ::rtl::OUString& rChars );

protected:
    /** will be called for each attribute */
    virtual void HandleAttribute(
        sal_uInt16 nToken,
        const rtl::OUString& rValue ) = 0;

    /** will be called for each child element */
    virtual SvXMLImportContext* HandleChild(
        sal_uInt16 nToken,

        // the following attributes are mainly to be used for child
        // context creation
        sal_uInt16 nPrefix,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList>& xAttrList ) = 0;
};

#endif
