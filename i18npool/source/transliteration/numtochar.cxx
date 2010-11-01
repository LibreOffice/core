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

// prevent internal compiler error with MSVC6SP3
#include <utility>

#define TRANSLITERATION_ALL
#include <numtochar.hxx>
#include <com/sun/star/i18n/NativeNumberMode.hpp>

namespace com { namespace sun { namespace star { namespace i18n {


#define TRANSLITERATION_NUMTOCHAR( name, number ) \
NumToChar##name::NumToChar##name() \
{ \
        nNativeNumberMode = number; \
        tableSize = 0; \
        transliterationName = "NumToChar"#name; \
        implementationName = "com.sun.star.i18n.Transliteration.NumToChar"#name; \
}

using namespace com::sun::star::i18n::NativeNumberMode;

TRANSLITERATION_NUMTOCHAR( Halfwidth, NATNUM0 )
TRANSLITERATION_NUMTOCHAR( Fullwidth, NATNUM3 )
TRANSLITERATION_NUMTOCHAR( Lower_zh_CN, NATNUM1 )
TRANSLITERATION_NUMTOCHAR( Lower_zh_TW, NATNUM1 )
TRANSLITERATION_NUMTOCHAR( Upper_zh_CN, NATNUM2 )
TRANSLITERATION_NUMTOCHAR( Upper_zh_TW, NATNUM2 )
TRANSLITERATION_NUMTOCHAR( KanjiShort_ja_JP, NATNUM1 )
TRANSLITERATION_NUMTOCHAR( KanjiTraditional_ja_JP, NATNUM2 )
TRANSLITERATION_NUMTOCHAR( Lower_ko, NATNUM1 )
TRANSLITERATION_NUMTOCHAR( Upper_ko, NATNUM2 )
TRANSLITERATION_NUMTOCHAR( Hangul_ko, NATNUM9 )
TRANSLITERATION_NUMTOCHAR( Indic_ar, NATNUM1 )
TRANSLITERATION_NUMTOCHAR( EastIndic_ar, NATNUM1 )
TRANSLITERATION_NUMTOCHAR( Indic_hi, NATNUM1 )
TRANSLITERATION_NUMTOCHAR( _th, NATNUM1 )
#undef TRANSLITERATION_NUMTOCHAR

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
