/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_i18npool.hxx"

#define TRANSLITERATION_ALL
#include <chartonum.hxx>
#include <data/numberchar.h>
#include <rtl/ustrbuf.hxx>

using namespace com::sun::star::uno;

namespace com { namespace sun { namespace star { namespace i18n {

#define TRANSLITERATION_CHARTONUM( name ) \
CharToNum##name::CharToNum##name() \
{ \
        nNativeNumberMode = 0; \
        tableSize = 0; \
        implementationName = "com.sun.star.i18n.Transliteration.CharToNum"#name; \
}
TRANSLITERATION_CHARTONUM( Fullwidth)
TRANSLITERATION_CHARTONUM( Lower_zh_CN)
TRANSLITERATION_CHARTONUM( Lower_zh_TW)
TRANSLITERATION_CHARTONUM( Upper_zh_CN)
TRANSLITERATION_CHARTONUM( Upper_zh_TW)
TRANSLITERATION_CHARTONUM( KanjiShort_ja_JP)
TRANSLITERATION_CHARTONUM( KanjiTraditional_ja_JP)
TRANSLITERATION_CHARTONUM( Lower_ko)
TRANSLITERATION_CHARTONUM( Upper_ko)
TRANSLITERATION_CHARTONUM( Hangul_ko)
TRANSLITERATION_CHARTONUM( Indic_ar)
TRANSLITERATION_CHARTONUM( EastIndic_ar)
TRANSLITERATION_CHARTONUM( Indic_hi)
TRANSLITERATION_CHARTONUM( _th)
#undef TRANSLITERATION_CHARTONUM

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
