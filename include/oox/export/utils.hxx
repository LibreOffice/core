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

inline OString I32S_(sal_Int32 x) { return OString::number(x); }
inline OString I64S_(sal_Int64 x) { return OString::number(x); }
#define I32S(x) I32S_(x).getStr()
#define I64S(x) I64S_(x).getStr()
#define IS(x) OString::number( x ).getStr()

/**
 * @return const char* literal "true" for true value, or literal "false"
 *         for false value.
 */
static inline const char* ToPsz(bool b)
{
    return b ? "true" : "false";
}

/**
 * @return literal "1" for true value, or literal "0" for false value.
 */
static inline const char* ToPsz10(bool b)
{
    // xlsx seems to use "1" or "0" for boolean values.  I wonder it ever uses
    // the "true" "false" variant.
    return b ? "1" : "0";
}

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
