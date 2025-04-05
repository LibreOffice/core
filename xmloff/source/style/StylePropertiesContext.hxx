/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <xmloff/xmlprcon.hxx>

class StylePropertiesContext : public SvXMLPropertySetContext
{
public:
    StylePropertiesContext(SvXMLImport& rImport, sal_Int32 nElement,
                           const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList,
                           sal_uInt32 nFamily, std::vector<XMLPropertyState>& rProps,
                           SvXMLImportPropertyMapper* pMap);

    virtual ~StylePropertiesContext() override;

    using SvXMLPropertySetContext::createFastChildContext;

    virtual css::uno::Reference<css::xml::sax::XFastContextHandler> createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList,
        std::vector<XMLPropertyState>& rProperties, const XMLPropertyState& rProperty) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
