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

#include <rtl/string.hxx>
#include <rtl/textenc.h>
#include <sal/types.h>

#define I32S(x) OString::number( (sal_Int32) x ).getStr()
#define I64S(x) OString::number( (sal_Int64) x ).getStr()

inline OString I32SHEX_(sal_Int32 x)
{
    OString aStr = OString::number(x, 16);
    if (aStr.getLength() % 2 != 0)
        aStr = OString("0") + aStr;
    return aStr.getStr();
}
#define I32SHEX(x) I32SHEX_(x).getStr()

#define DS(x) OString::number( (double) x ).getStr()
#define IS(x) OString::number( x ).getStr()
#define BS(x) (x ? "1":"0")
#define USS(x) OUStringToOString( x, RTL_TEXTENCODING_UTF8 ).getStr()

static inline sal_Int64 PPTtoEMU( sal_Int32 nPPT )
{
    return (sal_Int64)( (double)nPPT * 1587.5 );
}

static inline sal_Int64 TwipsToEMU( sal_Int32 nTwips )
{
    return sal_Int64( nTwips ) * 635;
}

template <typename T>
OString write1000thOfAPercent(T number)
{
    return OString::number( number * 1000 );
}

namespace oox { namespace drawingml {
    enum DocumentType { DOCUMENT_DOCX, DOCUMENT_PPTX, DOCUMENT_XLSX };
} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
