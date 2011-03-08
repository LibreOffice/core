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
/*
Service Name
  com.sun.star.i18n.Transliteration     ... dispatching UNO object and generic methods
  com.sun.star.i18n.Transliteration.u2l ... upper to lower
  com.sun.star.i18n.Transliteration.l2u ... lower to upper
  com.sun.star.i18n.Transliteration.caseignore ... case ignore
  com.sun.star.i18n.Transliteration.l10n ... other

  We have to adopt different service name for internal UNO objects.
  Case realted functionality vary depending on Locale.


Implementation Name
  For geneic service: com.sun.star.i18n.Transliteration,
         com.sun.star.i18n.Transliteration

  com.sun.star.i18n.Transliteration.u2l
  com.sun.star.i18n.Transliteration.l2u
  com.sun.star.i18n.Transliteration.caseignore
  For these there services above,

         com.sun.star.i18n.Transliteration.ja_JP
                                            ^^^^^
                        Locale name

  For com.sun.star.i18n.l10n.Transliteration service,
         com.sun.star.i18n.Transliteration.HALFWIDTH_FULLWIDTH,
         com.sun.star.i18n.Transliteration.FULLWIDTH_HALFWIDTH,
         com.sun.star.i18n.Transliteration.IGNORE_WIDTH,
         com.sun.star.i18n.Transliteration.KATAKANA_HIRAGANA,
         com.sun.star.i18n.Transliteration.HIRAGANA_KATAKANA,
         com.sun.star.i18n.Transliteration.IGNORE_KANA,

          ... These objects above can be used in every Locale context.

*/


#ifndef _I18N_SERVICENAME_HXX_
#define _I18N_SERVICENAME_HXX_



#define TRLT_SERVICELNAME "com.sun.star.i18n.Transliteration"
#define TRLT_SERVICELNAME_PREFIX TRLT_SERVICELNAME "."
#define TRLT_SERVICELNAME_U2L   TRLT_SERVICELNAME_PREFIX "u2l"
#define TRLT_SERVICELNAME_L2U   TRLT_SERVICELNAME_PREFIX "l2u"
#define TRLT_SERVICELNAME_CASEIGNORE TRLT_SERVICELNAME_PREFIX "caseignore"
#define TRLT_SERVICELNAME_L10N  TRLT_SERVICELNAME_PREFIX "l10n"

#define TRLT_IMPLNAME "com.sun.star.i18n.Transliteration"
#define TRLT_IMPLNAME_PREFIX_LEN    34
#define TRLT_IMPLNAME_PREFIX "com.sun.star.i18n.Transliteration."
//                1...5...901...5...901...5...901..4  --> 34

#define TRLT_IMPLNAME_L10N  TRLT_IMPLNAME_PREFIX "l10n"
#define TRLT_IMPLNAME_FH  TRLT_IMPLNAME_PREFIX "FULLWIDTH_HALFWIDTH" // for test
#define TRLT_IMPLNAME_Ja_JP TRLT_IMPLNAME_PREFIX "ja_JP"

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
