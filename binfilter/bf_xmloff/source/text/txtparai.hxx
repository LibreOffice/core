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
#ifndef _XMLOFF_TEXTPARAI_HXX_
#define _XMLOFF_TEXTPARAI_HXX_


#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_ 
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _XMLOFF_XMLICTXT_HXX 
#include "xmlictxt.hxx"
#endif
namespace com { namespace sun { namespace star {
namespace text {  class XTextRange; }
namespace xml { namespace sax { class XAttributeList; } }
} } }
namespace binfilter {

class XMLHints_Impl;

#ifdef CONV_STAR_FONTS
#define CONV_FROM_STAR_BATS 1
#define CONV_FROM_STAR_MATH 2
#define CONV_STAR_FONT_FLAGS_VALID 4
#endif

class XMLParaContext : public SvXMLImportContext
{
    ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XTextRange > xStart;	// xub_StrLen nStart;
    ::rtl::OUString				sStyleName;
    sal_Int8 				nOutlineLevel;
    XMLHints_Impl			*pHints;
    sal_Bool				bIgnoreLeadingSpace;
    sal_Bool				bHeading;
#ifdef CONV_STAR_FONTS
    sal_uInt8				nStarFontsConvFlags;
#endif

public:

    TYPEINFO();

    XMLParaContext( SvXMLImport& rImport,
            sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            sal_Bool bHeading );

    virtual ~XMLParaContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
            const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual void Characters( const ::rtl::OUString& rChars );

};


}//end of namespace binfilter
#endif
