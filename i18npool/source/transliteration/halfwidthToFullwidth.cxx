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

#include <i18nutil/widthfolding.hxx>

#include <transliteration_OneToOne.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;


namespace i18npool {

halfwidthToFullwidth::halfwidthToFullwidth()
{
    func = nullptr;
    table = &i18nutil::widthfolding::gethalf2fullTable();
    transliterationName = "halfwidthToFullwidth";
    implementationName = "com.sun.star.i18n.Transliteration.HALFWIDTH_FULLWIDTH";
}

OUString
halfwidthToFullwidth::transliterateImpl( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset, bool useOffset )
{
    // One to One mapping
    const OUString& newStr = transliteration_OneToOne::transliterateImpl( inStr, startPos, nCount, offset, false);

    // Composition: KA + voice-mark --> GA
    return i18nutil::widthfolding::compose_ja_voiced_sound_marks ( newStr, 0, newStr.getLength(), offset, useOffset );
}

HALFWIDTHKATAKANA_FULLWIDTHKATAKANA::HALFWIDTHKATAKANA_FULLWIDTHKATAKANA()
{
    func = nullptr;
    table = &i18nutil::widthfolding::gethalfKana2fullKanaTable();
    transliterationName = "halfwidthKatakanaToFullwidthKatakana";
    implementationName = "com.sun.star.i18n.Transliteration.HALFWIDTHKATAKANA_FULLWIDTHKATAKANA";
}

OUString
HALFWIDTHKATAKANA_FULLWIDTHKATAKANA::transliterateImpl( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset, bool useOffset )
{
    // One to One mapping
    const OUString& newStr = transliteration_OneToOne::transliterateImpl( inStr, startPos, nCount, offset, false);

    // Composition: KA + voice-mark --> GA
    return i18nutil::widthfolding::compose_ja_voiced_sound_marks ( newStr, 0, newStr.getLength(), offset, useOffset );
}

HALFWIDTH_FULLWIDTH_LIKE_JIS::HALFWIDTH_FULLWIDTH_LIKE_JIS()
{
    func = nullptr;
    table = &i18nutil::widthfolding::gethalf2fullTableForJIS();
    transliterationName = "halfwidthToFullwidthLikeJIS";
    implementationName = "com.sun.star.i18n.Transliteration.HALFWIDTH_FULLWIDTH_LIKE_JIS";
}

OUString
HALFWIDTH_FULLWIDTH_LIKE_JIS::transliterateImpl( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset, bool useOffset )
{
    // One to One mapping
    const OUString& newStr = transliteration_OneToOne::transliterateImpl( inStr, startPos, nCount, offset, false);

    // Composition: KA + voice-mark --> GA
    return i18nutil::widthfolding::compose_ja_voiced_sound_marks ( newStr, 0, newStr.getLength(), offset, useOffset, WIDTHFOLDING_DONT_USE_COMBINED_VU );
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
