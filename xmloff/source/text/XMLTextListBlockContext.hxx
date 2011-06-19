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

#ifndef _XMLTEXTLISTBLOCKCONTEXT_HXX
#define _XMLTEXTLISTBLOCKCONTEXT_HXX

#include <com/sun/star/container/XIndexReplace.hpp>
#include <xmloff/xmlictxt.hxx>

class XMLTextImportHelper;

class XMLTextListBlockContext : public SvXMLImportContext
{
    XMLTextImportHelper&    mrTxtImport;

    ::com::sun::star::uno::Reference<
                ::com::sun::star::container::XIndexReplace > mxNumRules;

    // text:style-name property of <list> element
    ::rtl::OUString         msListStyleName;
    ::rtl::OUString         sXmlId;

    SvXMLImportContextRef   mxParentListBlock;

    sal_Int16               mnLevel;
    sal_Bool                mbRestartNumbering;
    sal_Bool                mbSetDefaults;

    // text:id property of <list> element, only valid for root <list> element
    ::rtl::OUString msListId;
    // text:continue-list property of <list> element, only valid for root <list> element
    ::rtl::OUString msContinueListId;

public:

    TYPEINFO();

    // add optional parameter <bRestartNumberingAtSubList>
    XMLTextListBlockContext(
                SvXMLImport& rImport,
                XMLTextImportHelper& rTxtImp,
                sal_uInt16 nPrfx,
                const ::rtl::OUString& rLName,
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
                const sal_Bool bRestartNumberingAtSubList = sal_False );
    virtual ~XMLTextListBlockContext();

    virtual void EndElement();

    SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                const ::rtl::OUString& rLocalName,
                 const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    const ::rtl::OUString& GetListStyleName() const { return msListStyleName; }
    sal_Int16 GetLevel() const { return mnLevel; }
    sal_Bool IsRestartNumbering() const { return mbRestartNumbering; }
    void ResetRestartNumbering() { mbRestartNumbering = sal_False; }

    const ::com::sun::star::uno::Reference <
        ::com::sun::star::container::XIndexReplace >& GetNumRules() const
        { return mxNumRules; }

    const ::rtl::OUString& GetListId() const;
    const ::rtl::OUString& GetContinueListId() const;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
