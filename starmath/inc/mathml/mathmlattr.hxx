/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <tools/fract.hxx>

// MathML 3: 2.1.5.1 Syntax notation used in the MathML specification
// <https://www.w3.org/TR/MathML/chapter2.html#id.2.1.5.1>
// MathML 2: 2.4.4.2 Attributes with units
// <https://www.w3.org/TR/MathML2/chapter2.html#fund.attval>
// MathML 3: 2.1.5.2 Length Valued Attributes
// <https://www.w3.org/TR/MathML/chapter2.html#fund.units>

enum class MathMLLengthUnit
{
    None,
    Em,
    Ex,
    Px,
    In,
    Cm,
    Mm,
    Pt,
    Pc,
    Percent
};

struct MathMLAttributeLengthValue
{
    Fraction aNumber;
    MathMLLengthUnit eUnit;
    MathMLAttributeLengthValue()
        : eUnit(MathMLLengthUnit::None)
    {
    }
};

sal_Int32 ParseMathMLAttributeLengthValue(const OUString& rStr, MathMLAttributeLengthValue& rV);

// MathML 3: 3.2.2 Mathematics style attributes common to token elements
// <https://www.w3.org/TR/MathML3/chapter3.html#presm.commatt>

enum class MathMLMathvariantValue
{
    Normal,
    Bold,
    Italic,
    BoldItalic,
    DoubleStruck,
    BoldFraktur,
    Script,
    BoldScript,
    Fraktur,
    SansSerif,
    BoldSansSerif,
    SansSerifItalic,
    SansSerifBoldItalic,
    Monospace,
    Initial,
    Tailed,
    Looped,
    Stretched
};

bool GetMathMLMathvariantValue(const OUString& rStr, MathMLMathvariantValue& rV);

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
