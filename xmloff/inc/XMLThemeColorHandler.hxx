/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <docmodel/uno/UnoThemeColor.hxx>

using namespace ::xmloff::token;
using namespace css;

class XMLThemeColorHandler : public XMLPropertyHandler
{
public:
    bool importXML(const OUString& /*rStrImpValue*/, css::uno::Any& /*rValue*/,
                   const SvXMLUnitConverter&) const override
    {
        return false;
    }

    bool exportXML(OUString& /*rStrExpValue*/, const css::uno::Any& /*rValue*/,
                   const SvXMLUnitConverter&) const override
    {
        return false;
    }

    bool equals(const css::uno::Any& rAny1, const css::uno::Any& rAny2) const override
    {
        uno::Reference<util::XThemeColor> xThemeColor1;
        uno::Reference<util::XThemeColor> xThemeColor2;
        rAny1 >>= xThemeColor1;
        rAny2 >>= xThemeColor2;
        model::ThemeColor aThemeColor1;
        model::ThemeColor aThemeColor2;
        model::theme::setFromXThemeColor(aThemeColor1, xThemeColor1);
        model::theme::setFromXThemeColor(aThemeColor2, xThemeColor2);

        return aThemeColor1 == aThemeColor2;
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
