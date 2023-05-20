/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>
#include "XMLElementPropertyContext.hxx"
#include <docmodel/uno/UnoComplexColor.hxx>

class XMLComplexColorContext final : public XMLElementPropertyContext
{
    model::ComplexColor maComplexColor;
    sal_Int32 mnRootElement;

public:
    XMLComplexColorContext(SvXMLImport& rImport, sal_Int32 nElement,
                           const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList,
                           const XMLPropertyState& rProp, std::vector<XMLPropertyState>& rProps);

    css::uno::Reference<css::xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& AttrList) override;

    void SAL_CALL endFastElement(sal_Int32 nElement) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
