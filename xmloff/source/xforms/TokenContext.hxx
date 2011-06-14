/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _XMLOFF_TOKENCONTEXT_HXX
#define _XMLOFF_TOKENCONTEXT_HXX


//
// include for parent class and members
//

#include <xmloff/xmlictxt.hxx>


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
                  sal_uInt16 nPrefix,
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
        sal_uInt16 nPrefix,
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
