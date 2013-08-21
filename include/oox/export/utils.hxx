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

#ifndef _OOX_EXPORT_UTILS_HXX_
#define _OOX_EXPORT_UTILS_HXX_

#define I32S(x) OString::number( (sal_Int32) x ).getStr()
#define I64S(x) OString::number( (sal_Int64) x ).getStr()
#define IS(x) OString::number( x ).getStr()
#define USS(x) OUStringToOString( x, RTL_TEXTENCODING_UTF8 ).getStr()

#ifndef DBG
#  if OSL_DEBUG_LEVEL > 0
#    define DBG(x) x
#  else
#    define DBG(x)
#  endif
#endif

// ---------------------------------------------------------------------------------------------

static inline sal_Int64 PPTtoEMU( sal_Int32 nPPT )
{
    return (sal_Int64)( (double)nPPT * 1587.5 );
}

// ---------------------------------------------------------------------------------------------

static inline sal_Int64 MM100toEMU( sal_Int32 nMM100 )
{
    return (sal_Int64)nMM100 * 360;
}

// ---------------------------------------------------------------------------------------------

static inline sal_Int64 TwipsToEMU( sal_Int32 nTwips )
{
    return sal_Int64( nTwips ) * 635;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
