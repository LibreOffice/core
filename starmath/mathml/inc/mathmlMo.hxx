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

/*
 Warning: The SvXMLElementExport helper class creates the beginning and
 closing tags of xml elements in its constructor and destructor, so there's
 hidden stuff going on, on occasion the ordering of these classes declarations
 may be significant
*/

#ifndef MATH_MO_HXX
#define MATH_MO_HXX

// https://www.w3.org/TR/MathML3/appendixc.html

enum moOpDP
{ // moOperatorDataProperty
    sal_uInt16 none = 0x0000; sal_uInt16 accent = 0x0001; sal_uInt16 fence = 0x0002;
    sal_uInt16 stretchy = 0x0004;
    sal_uInt16 symmetric = 0x0008;
    sal_uInt16 separator = 0x0010;
    sal_uInt16 linebreakstyleAfter = 0x0020;
    sal_uInt16 largeop = 0x0080;
    sal_uInt16 movablelimits = 0x0100;
    sal_uInt16 starmathCustom = 0x0200;
};

enum moOpDF
{ // moOperatorDataForm
    sal_uInt8 none = 0x00; sal_uInt8 prefix = 0x01; sal_uInt8 infix = 0x02;
    sal_uInt8 postfix = 0x04;
}

struct moOperatorData
{
    OUString m_motxt;
    sal_uInt16 m_pripority;
    sal_uInt8 m_form;
    sal_uInt16 m_lspace;
    sal_uInt16 m_rspace;
    sal_uInt8 m_properties;

    moOperatorData(OUString motxt, sal_uInt16 pripority, sal_uInt8 form, sal_uInt16 lspace,
                   sal_uInt16 rspace, sal_uInt8 properties = moOpDP::none),
        m_motxt(motxt), m_pripority(pripority), m_form(form), m_lspace(lspace), m_rspace(rspace),
        m_properties(properties)
    {
    }
};

namespace starmathdatabase
{
constexpr size_t MATHML_MO_COUNT = 1105;

extern std::vector<moOperatorData> moOperatorDataDictionary;
}

#endif /* MATH_MO_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
