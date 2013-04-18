/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _LINEEND_HXX
#define _LINEEND_HXX

#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include "tools/toolsdllapi.h"

enum LineEnd { LINEEND_CR, LINEEND_LF, LINEEND_CRLF };

inline LineEnd GetSystemLineEnd()
{
#if defined WNT
    return LINEEND_CRLF;
#else
    return LINEEND_LF;
#endif
}

TOOLS_DLLPUBLIC OString convertLineEnd(const OString &rIn, LineEnd eLineEnd);
TOOLS_DLLPUBLIC OUString convertLineEnd(const OUString &rIn, LineEnd eLineEnd);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
