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

#ifndef INCLUDED_XMLOFF_SOURCE_DRAW_XIMPBODY_HXX
#define INCLUDED_XMLOFF_SOURCE_DRAW_XIMPBODY_HXX

#include <xmloff/xmlictxt.hxx>
#include "sdxmlimp_impl.hxx"
#include <xmloff/nmspmap.hxx>
#include "ximppage.hxx"

// draw:page context

class SdXMLDrawPageContext : public SdXMLGenericPageContext
{
    OUString               maContextName;
    OUString               maMasterPageName;
    OUString               maStyleName;
    OUString               maHREF;

    bool                   mbHadSMILNodes;

public:
    SdXMLDrawPageContext( SdXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes);
    virtual ~SdXMLDrawPageContext();

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList ) override;
    virtual void EndElement() override;

};

// office:body context

class SdXMLBodyContext : public SvXMLImportContext
{
    const SdXMLImport& GetSdImport() const { return static_cast<const SdXMLImport&>(GetImport()); }
    SdXMLImport& GetSdImport() { return static_cast<SdXMLImport&>(GetImport()); }

public:
    SdXMLBodyContext( SdXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLocalName );
    virtual ~SdXMLBodyContext();

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix, const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList ) override;
};

#endif // INCLUDED_XMLOFF_SOURCE_DRAW_XIMPBODY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
