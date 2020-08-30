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

#ifndef INCLUDED_XMLOFF_INC_PAGEMASTERIMPORTCONTEXT_HXX
#define INCLUDED_XMLOFF_INC_PAGEMASTERIMPORTCONTEXT_HXX

#include <xmloff/prstylei.hxx>
#include <xmloff/xmlimp.hxx>

class PageStyleContext final : public XMLPropStyleContext
{
private:
    OUString sPageUsage;
    bool                m_bIsFillStyleAlreadyConverted : 1;

    virtual void SetAttribute( sal_uInt16 nPrefixKey,
                               const OUString& rLocalName,
                               const OUString& rValue ) override;

public:


    PageStyleContext( SvXMLImport& rImport,
            SvXMLStylesContext& rStyles,
            bool bDefaultStyle);
    virtual ~PageStyleContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

    // don't call this
    virtual void FillPropertySet(
            const css::uno::Reference< css::beans::XPropertySet > & rPropSet ) override;
    void FillPropertySet_PageStyle(
            const css::uno::Reference< css::beans::XPropertySet > & rPropSet,
            XMLPropStyleContext * pDrawingPageStyle);

    //text grid enhancement
    virtual void SetDefaults() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
