/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <config_options.h>
#include <xmloff/dllapi.h>

#include <xmloff/XMLElementPropertyContext.hxx>
#include <xmloff/xmlictxt.hxx>
#include <docmodel/color/ComplexColor.hxx>

class UNLESS_MERGELIBS(XMLOFF_DLLPUBLIC) XMLComplexColorImport
{
private:
    model::ComplexColor& mrComplexColor;

public:
    XMLComplexColorImport(model::ComplexColor& rComplexColor);
    void fillAttributes(const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList);
    bool
    handleTransformContext(sal_Int32 nElement,
                           const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList);
};

class XMLOFF_DLLPUBLIC XMLPropertyComplexColorContext final : public XMLElementPropertyContext
{
private:
    model::ComplexColor maComplexColor;
    sal_Int32 mnRootElement;

    XMLComplexColorImport maComplexColorImport;

public:
    XMLPropertyComplexColorContext(
        SvXMLImport& rImport, sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList,
        const XMLPropertyState& rProp, std::vector<XMLPropertyState>& rProps);

    css::uno::Reference<css::xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& AttrList) override;

    void SAL_CALL endFastElement(sal_Int32 nElement) override;

    const model::ComplexColor& getComplexColor() { return maComplexColor; }
};

class XMLOFF_DLLPUBLIC XMLComplexColorContext final : public SvXMLImportContext
{
private:
    XMLComplexColorImport maComplexColorImport;

public:
    XMLComplexColorContext(SvXMLImport& rImport, model::ComplexColor& rComplexColor,
                           const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList);

    css::uno::Reference<css::xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& AttrList) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
