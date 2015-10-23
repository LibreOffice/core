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
#ifndef INCLUDED_XMLOFF_XMLTEXTMASTERPAGECONTEXT_HXX
#define INCLUDED_XMLOFF_XMLTEXTMASTERPAGECONTEXT_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <sal/types.h>

#include <xmloff/xmlstyle.hxx>

namespace com { namespace sun { namespace star {
    namespace style { class XStyle; }
} } }

class XMLOFF_DLLPUBLIC XMLTextMasterPageContext : public SvXMLStyleContext
{
    const OUString sIsPhysical;
    const OUString sFollowStyle;
    OUString       sFollow;
    OUString       sPageMasterName;

    css::uno::Reference < css::style::XStyle > xStyle;

    bool bInsertHeader;
    bool bInsertFooter;
    bool bInsertHeaderLeft;
    bool bInsertFooterLeft;
    bool bInsertHeaderFirst;
    bool bInsertFooterFirst;
    bool bHeaderInserted;
    bool bFooterInserted;
    bool bHeaderLeftInserted;
    bool bFooterLeftInserted;
    bool bHeaderFirstInserted;
    bool bFooterFirstInserted;

    SAL_DLLPRIVATE css::uno::Reference< css::style::XStyle > Create();
protected:
    css::uno::Reference< css::style::XStyle > GetStyle() { return xStyle; }
public:

    TYPEINFO_OVERRIDE();

    XMLTextMasterPageContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList,
            bool bOverwrite );
    virtual ~XMLTextMasterPageContext();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;

    virtual SvXMLImportContext *CreateHeaderFooterContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList,
            const bool bFooter,
            const bool bLeft,
            const bool bFirst );

    virtual void Finish( bool bOverwrite ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
