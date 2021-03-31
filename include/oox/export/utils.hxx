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

#ifndef INCLUDED_OOX_EXPORT_UTILS_HXX
#define INCLUDED_OOX_EXPORT_UTILS_HXX

#include <sal/config.h>

#include <o3tl/unit_conversion.hxx>
#include <rtl/string.hxx>
#include <sal/types.h>

inline OString I32SHEX(sal_Int32 x)
{
    OString aStr = OString::number(x, 16);
    while (aStr.getLength() < 6)
        aStr = "0" + aStr;
    return aStr;
}

/**
 * @return const char* literal "true" for true value, or literal "false"
 *         for false value.
 */
static constexpr const char* ToPsz(bool b)
{
    return b ? "true" : "false";
}

/**
 * @return literal "1" for true value, or literal "0" for false value.
 */
static constexpr const char* ToPsz10(bool b)
{
    // xlsx seems to use "1" or "0" for boolean values.  I wonder it ever uses
    // the "true" "false" variant.
    return b ? "1" : "0";
}

static constexpr sal_Int64 PPTtoEMU( sal_Int32 nPPT )
{
    return o3tl::convert(nPPT, o3tl::Length::master, o3tl::Length::emu);
}

static constexpr sal_Int64 TwipsToEMU( sal_Int32 nTwips )
{
    return o3tl::convert(nTwips, o3tl::Length::twip, o3tl::Length::emu);
}

template <typename T>
OString write1000thOfAPercent(T number)
{
    return OString::number( lround(number * 1000.0) );
}

namespace oox::drawingml {
    enum DocumentType { DOCUMENT_DOCX, DOCUMENT_PPTX, DOCUMENT_XLSX };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
