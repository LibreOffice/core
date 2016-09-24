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
/*
Service Name
  com.sun.star.i18n.Transliteration     ... dispatching UNO object and generic methods
  com.sun.star.i18n.Transliteration.u2l ... upper to lower
  com.sun.star.i18n.Transliteration.l2u ... lower to upper
  com.sun.star.i18n.Transliteration.caseignore ... case ignore
  com.sun.star.i18n.Transliteration.l10n ... other

  We have to adopt different service name for internal UNO objects.
  Case related functionality vary depending on Locale.


Implementation Name
  For generic service: com.sun.star.i18n.Transliteration,
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


#ifndef INCLUDED_I18NPOOL_INC_SERVICENAME_HXX
#define INCLUDED_I18NPOOL_INC_SERVICENAME_HXX

#define TRLT_SERVICELNAME "com.sun.star.i18n.Transliteration"
#define TRLT_SERVICELNAME_L10N  TRLT_SERVICELNAME ".l10n"

#define TRLT_IMPLNAME_PREFIX "com.sun.star.i18n.Transliteration."
//                1...5...901...5...901...5...901..4  --> 34

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
