/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once
#if 1

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
