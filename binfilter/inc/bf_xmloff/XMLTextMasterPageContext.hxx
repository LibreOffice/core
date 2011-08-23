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
#ifndef _XMLOFF_TEXTMASTERPAGECONTEXT_HXX_
#define _XMLOFF_TEXTMASTERPAGECONTEXT_HXX_

#include <bf_xmloff/xmlstyle.hxx>

namespace com { namespace sun { namespace star {
    namespace style { class XStyle; }
} } }
namespace binfilter {

class XMLTextMasterPageContext : public SvXMLStyleContext
{
    const ::rtl::OUString sIsPhysical;
    const ::rtl::OUString sPageStyleLayout;
    const ::rtl::OUString sFollowStyle;
    ::rtl::OUString		  sFollow;
    ::rtl::OUString		  sPageMasterName;

    ::com::sun::star::uno::Reference < ::com::sun::star::style::XStyle > xStyle;

    sal_Bool bInsertHeader;
    sal_Bool bInsertFooter;
    sal_Bool bInsertHeaderLeft;
    sal_Bool bInsertFooterLeft;
    sal_Bool bHeaderInserted;
    sal_Bool bFooterInserted;
    sal_Bool bHeaderLeftInserted;
    sal_Bool bFooterLeftInserted;

    ::com::sun::star::uno::Reference <
        ::com::sun::star::style::XStyle > Create();
protected:
    ::com::sun::star::uno::Reference <
        ::com::sun::star::style::XStyle > GetStyle() { return xStyle; }
public:

    TYPEINFO();

    XMLTextMasterPageContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            sal_Bool bOverwrite );
    virtual ~XMLTextMasterPageContext();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual SvXMLImportContext *CreateHeaderFooterContext(
            sal_uInt16 nPrefix,
            const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            const sal_Bool bFooter,
            const sal_Bool bLeft );

    virtual void Finish( sal_Bool bOverwrite );
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
