/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <xmloff/xmlprhdl.hxx>

using namespace ::xmloff::token;

class XMLThemeColorHandler : public XMLPropertyHandler
{
public:
    bool importXML(const OUString& rStrImpValue, css::uno::Any& rValue,
                   const SvXMLUnitConverter&) const override
    {
        sal_Int16 nValue;
        bool bReturn = SvXMLUnitConverter::convertEnum(nValue, rStrImpValue, pXML_ThemeColor_Enum);

        if (bReturn)
            rValue <<= nValue;

        return bReturn;
    }

    bool exportXML(OUString& rStrExpValue, const css::uno::Any& rValue,
                   const SvXMLUnitConverter&) const override
    {
        sal_Int16 nThemeIndex;

        if (!(rValue >>= nThemeIndex) || nThemeIndex == -1) // Default
            return false;

        OUStringBuffer aOutBuffer;
        bool bReturn = SvXMLUnitConverter::convertEnum(aOutBuffer, nThemeIndex,
                                                       pXML_ThemeColor_Enum, XML_NONE);
        rStrExpValue = aOutBuffer.makeStringAndClear();

        return bReturn;
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
