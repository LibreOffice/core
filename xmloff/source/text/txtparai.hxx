/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: txtparai.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-08 17:06:34 $
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
#ifndef _XMLOFF_TEXTPARAI_HXX_
#define _XMLOFF_TEXTPARAI_HXX_


#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif

class XMLHints_Impl;
namespace com { namespace sun { namespace star {
namespace text {  class XTextRange; }
namespace xml { namespace sax { class XAttributeList; } }
} } }

#ifdef CONV_STAR_FONTS
#define CONV_FROM_STAR_BATS 1
#define CONV_FROM_STAR_MATH 2
#define CONV_STAR_FONT_FLAGS_VALID 4
#endif

class XMLParaContext : public SvXMLImportContext
{
    ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XTextRange > xStart;    // xub_StrLen nStart;
    ::rtl::OUString             sStyleName;
    ::rtl::OUString             sId;
    sal_Int8                nOutlineLevel;
    XMLHints_Impl           *pHints;
    sal_Bool                bIgnoreLeadingSpace;
    sal_Bool                bHeading;
    sal_Bool                bIsListHeader;
    sal_Bool                bIsRestart;
    sal_Int16               nStartValue;
#ifdef CONV_STAR_FONTS
    sal_uInt8               nStarFontsConvFlags;
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


#endif
