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
#include <texttonum.hxx>
#include <data/numberchar.h>
#include <rtl/ustrbuf.hxx>

using namespace com::sun::star::uno;

namespace com { namespace sun { namespace star { namespace i18n {

#define TRANSLITERATION_TEXTTONUM( name ) \
TextToNum##name::TextToNum##name() \
{ \
        nNativeNumberMode = 0; \
        tableSize = 0; \
        transliterationName = "TextToNum"#name; \
        implementationName = "com.sun.star.i18n.Transliteration.TextToNum"#name; \
}

TRANSLITERATION_TEXTTONUM( Lower_zh_CN)
TRANSLITERATION_TEXTTONUM( Upper_zh_CN)
TRANSLITERATION_TEXTTONUM( Lower_zh_TW)
TRANSLITERATION_TEXTTONUM( Upper_zh_TW)
TRANSLITERATION_TEXTTONUM( FormalLower_ko)
TRANSLITERATION_TEXTTONUM( FormalUpper_ko)
TRANSLITERATION_TEXTTONUM( FormalHangul_ko)
TRANSLITERATION_TEXTTONUM( InformalLower_ko)
TRANSLITERATION_TEXTTONUM( InformalUpper_ko)
TRANSLITERATION_TEXTTONUM( InformalHangul_ko)
TRANSLITERATION_TEXTTONUM( KanjiLongTraditional_ja_JP)
TRANSLITERATION_TEXTTONUM( KanjiLongModern_ja_JP)
#undef TRANSLITERATION_TEXTTONUM

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
