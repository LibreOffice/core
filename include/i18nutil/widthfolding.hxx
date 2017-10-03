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
#ifndef INCLUDED_I18NUTIL_WIDTHFOLDING_HXX
#define INCLUDED_I18NUTIL_WIDTHFOLDING_HXX

#include <sal/types.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <i18nutil/oneToOneMapping.hxx>
#include <i18nutil/i18nutildllapi.h>

namespace i18nutil {

#define WIDTHFOLDNIG_DONT_USE_COMBINED_VU 0x01

class I18NUTIL_DLLPUBLIC widthfolding
{
public:
    static oneToOneMapping& getfull2halfTable();
    static oneToOneMapping& gethalf2fullTable();

    static oneToOneMapping& getfull2halfTableForASC();
    static oneToOneMapping& gethalf2fullTableForJIS();

    static oneToOneMapping& getfullKana2halfKanaTable();
    static oneToOneMapping& gethalfKana2fullKanaTable();

    static OUString decompose_ja_voiced_sound_marks(const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, css::uno::Sequence< sal_Int32 >& offset, bool useOffset);
    static sal_Unicode decompose_ja_voiced_sound_marksChar2Char (sal_Unicode inChar);
    static OUString compose_ja_voiced_sound_marks(const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, css::uno::Sequence< sal_Int32 >& offset, bool useOffset, sal_Int32 nFlags = 0 );
    static sal_Unicode getCompositionChar(sal_Unicode c1, sal_Unicode c2);
};


}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
