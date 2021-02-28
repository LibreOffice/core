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

#pragma once

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <vector>

// https://www.w3.org/TR/MathML3/appendixc.html

enum class moOpDP : sal_uInt16
{ // moOperatorDataProperty
    nonedp = 0x0000,
    accent = 0x0001,
    fence = 0x0002,
    stretchy = 0x0004,
    symmetric = 0x0008,
    separator = 0x0010,
    linebreakstyleAfter = 0x0020,
    largeop = 0x0080,
    movablelimits = 0x0100,
    starmathCustom = 0x0200,
    starmathCustomMo = 0x0400,
    stretchyfence = 0x0006,
    movablelargeop = 0x0180
};

enum class moOpDF : sal_uInt16
{ // moOperatorDataForm
    nonedf = 0x0000,
    prefix = 0x0001,
    infix = 0x0002,
    postfix = 0x0004,
    prepostfix = 0x0005
};

struct moOperatorData
{
    OUString m_motxt;
    moOpDF m_form;
    sal_uInt16 m_priority;
    sal_uInt16 m_lspace;
    sal_uInt16 m_rspace;
    moOpDP m_properties;

    moOperatorData(OUString motxt, moOpDF form, sal_uInt16 priority, sal_uInt16 lspace,
                   sal_uInt16 rspace, moOpDP properties)
        : m_motxt(motxt)
        , m_form(form)
        , m_priority(priority)
        , m_lspace(lspace)
        , m_rspace(rspace)
        , m_properties(properties)
    {
    }
};

inline moOpDF operator|(moOpDF a, moOpDF b)
{
    return static_cast<moOpDF>(static_cast<sal_uInt16>(a) | static_cast<sal_uInt16>(b));
}

inline moOpDF operator&(moOpDF a, moOpDF b)
{
    return static_cast<moOpDF>(static_cast<sal_uInt16>(a) & static_cast<sal_uInt16>(b));
}

inline moOpDP operator|(moOpDP a, moOpDP b)
{
    return static_cast<moOpDP>(static_cast<sal_uInt16>(a) | static_cast<sal_uInt16>(b));
}

inline moOpDP operator&(moOpDP a, moOpDP b)
{
    return static_cast<moOpDP>(static_cast<sal_uInt16>(a) & static_cast<sal_uInt16>(b));
}

namespace starmathdatabase
{
constexpr size_t MATHML_MO_COUNT = 1100;

extern std::vector<moOperatorData> moOperatorDataDictionary;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
