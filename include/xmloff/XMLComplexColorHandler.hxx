/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <xmloff/dllapi.h>
#include <xmloff/xmlprhdl.hxx>
#include <docmodel/uno/UnoComplexColor.hxx>

using namespace css;

class XMLOFF_DLLPUBLIC XMLComplexColorHandler : public XMLPropertyHandler
{
public:
    bool importXML(const OUString& /*rStrImpValue*/, css::uno::Any& /*rValue*/,
                   const SvXMLUnitConverter&) const override final
    {
        return false;
    }

    bool exportXML(OUString& /*rStrExpValue*/, const css::uno::Any& /*rValue*/,
                   const SvXMLUnitConverter&) const override final
    {
        return false;
    }

    bool equals(const css::uno::Any& rAny1, const css::uno::Any& rAny2) const override final
    {
        uno::Reference<util::XComplexColor> xComplexColor1;
        uno::Reference<util::XComplexColor> xComplexColor2;
        rAny1 >>= xComplexColor1;
        rAny2 >>= xComplexColor2;
        model::ComplexColor aComplexColor1 = model::color::getFromXComplexColor(xComplexColor1);
        model::ComplexColor aComplexColor2 = model::color::getFromXComplexColor(xComplexColor2);
        return aComplexColor1 == aComplexColor2;
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
