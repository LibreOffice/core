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

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <vector>

// https://www.w3.org/TR/MathML3/appendixc.html

enum class : sal_uInt16 moOpDP{ // moOperatorDataProperty
                                nonedp = 0x0000,
                                accent = 0x0001,
                                fence = 0x0002,
                                stretchy = 0x0004,
                                symmetric = 0x0008,
                                separator = 0x0010,
                                linebreakstyleAfter = 0x0020,
                                largeop = 0x0080,
                                movablelimits = 0x0100,
                                starmathCustom = 0x0200
};

enum class : sal_uInt16 moOpDF{ // moOperatorDataForm
                                nonedf = 0x0000, prefix = 0x0001, infix = 0x0002, postfix = 0x0004
};

struct moOperatorData
{
    OUString m_motxt;
    sal_uInt16 m_priority;
    sal_uInt16 m_form;
    sal_uInt16 m_lspace;
    sal_uInt16 m_rspace;
    sal_uInt16 m_properties;

    moOperatorData(OUString motxt, sal_uInt16 priority, moOpDF form, sal_uInt16 lspace,
                   sal_uInt16 rspace, moOpDP properties = moOpDP::nonedp)
        : m_motxt(motxt)
        , m_priority(priority)
        , m_form(form)
        , m_lspace(lspace)
        , m_rspace(rspace)
        , m_properties(properties)
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
