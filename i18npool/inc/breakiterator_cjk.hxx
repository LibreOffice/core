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
#pragma once

#include "breakiterator_unicode.hxx"
#include <optional>
#include <memory>

namespace i18npool {


class BreakIterator_CJK : public BreakIterator_Unicode
{
public:
    BreakIterator_CJK();

    css::i18n::LineBreakResults SAL_CALL getLineBreak( const OUString& Text, sal_Int32 nStartPos,
        const css::lang::Locale& nLocale, sal_Int32 nMinBreakPos,
        const css::i18n::LineBreakHyphenationOptions& hOptions,
        const css::i18n::LineBreakUserOptions& bOptions ) override;

protected:
    OUString hangingCharacters;
};

class BreakIterator_zh final : public BreakIterator_CJK {
public:
    BreakIterator_zh();
};
class BreakIterator_zh_TW final : public BreakIterator_CJK {
public:
    BreakIterator_zh_TW();
};
class BreakIterator_ja final : public BreakIterator_CJK {
public:
    BreakIterator_ja();
};
class BreakIterator_ko final : public BreakIterator_CJK {
public:
    BreakIterator_ko();
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
