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

#ifndef _XMLOFF_XMLICTXT_HXX
#define _XMLOFF_XMLICTXT_HXX

#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _REF_HXX
#include <tools/ref.hxx>
#endif
#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif
namespace binfilter {

class SvXMLNamespaceMap;
class SvXMLImport;

class SvXMLImportContext : public SvRefBase
{
    friend class SvXMLImport;

    SvXMLImport& rImport;

    USHORT		nPrefix;
    ::rtl::OUString aLocalName;

    SvXMLNamespaceMap	*pRewindMap;

    SvXMLNamespaceMap  *GetRewindMap() const { return pRewindMap; }
    void SetRewindMap( SvXMLNamespaceMap *p ) { pRewindMap = p; }

protected:

    SvXMLImport& GetImport() { return rImport; }
    const SvXMLImport& GetImport() const { return rImport; }

public:
    TYPEINFO();

    USHORT GetPrefix() const { return nPrefix; }
    const ::rtl::OUString& GetLocalName() const { return aLocalName; }

    // A contexts constructor does anything that is required if an element
    // starts. Namespace processing has been done already.
    // Note that virtual methods cannot be used inside constructors. Use
    // StartElement instead if this is required.
    SvXMLImportContext( SvXMLImport& rImport, USHORT nPrfx,
                        const ::rtl::OUString& rLName );

    // A contexts destructor does anything that is required if an element
    // ends. By default, nothing is done.
    // Note that virtual methods cannot be used inside destructors. Use
    // EndElement instead if this is required.
    virtual ~SvXMLImportContext();

    // Create a childs element context. By default, the import's
    // CreateContext method is called to create a new default context.
    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                   const ::rtl::OUString& rLocalName,
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
};

SV_DECL_REF( SvXMLImportContext )
SV_IMPL_REF( SvXMLImportContext )

}//end of namespace binfilter
#endif	//  _XMLOFF_XMLICTXT_HXX

