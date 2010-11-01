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
#ifndef _L10N_TRANSLITERATION_NUMTOCHAR_HXX_
#define _L10N_TRANSLITERATION_NUMTOCHAR_HXX_

#include <transliteration_Numeric.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

#define TRANSLITERATION_NUMTOCHAR( name ) \
class NumToChar##name : public transliteration_Numeric \
{ \
public: \
        NumToChar##name (); \
};

#ifdef TRANSLITERATION_ALL
TRANSLITERATION_NUMTOCHAR(Lower_zh_CN)
TRANSLITERATION_NUMTOCHAR(Upper_zh_CN)
TRANSLITERATION_NUMTOCHAR(Lower_zh_TW)
TRANSLITERATION_NUMTOCHAR(Upper_zh_TW)
TRANSLITERATION_NUMTOCHAR(Upper_ko)
TRANSLITERATION_NUMTOCHAR(Hangul_ko)
TRANSLITERATION_NUMTOCHAR(Lower_ko)
TRANSLITERATION_NUMTOCHAR(KanjiShort_ja_JP)
TRANSLITERATION_NUMTOCHAR(KanjiTraditional_ja_JP)
TRANSLITERATION_NUMTOCHAR(Fullwidth)
TRANSLITERATION_NUMTOCHAR(Indic_ar)
TRANSLITERATION_NUMTOCHAR(EastIndic_ar)
TRANSLITERATION_NUMTOCHAR(Indic_hi)
TRANSLITERATION_NUMTOCHAR(_th)
TRANSLITERATION_NUMTOCHAR(Halfwidth)
#endif
#undef TRANSLITERATION_NUMTOCHAR

} } } }

#endif // _L10N_TRANSLITERATION_NUMTOCHAR_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
