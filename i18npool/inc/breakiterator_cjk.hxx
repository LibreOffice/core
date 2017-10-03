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
#ifndef INCLUDED_I18NPOOL_INC_BREAKITERATOR_CJK_HXX
#define INCLUDED_I18NPOOL_INC_BREAKITERATOR_CJK_HXX

#include <breakiterator_unicode.hxx>
#include <xdictionary.hxx>

#include <memory>

namespace i18npool {

//  class BreakIterator_CJK

class BreakIterator_CJK : public BreakIterator_Unicode
{
public:
    BreakIterator_CJK();

    css::i18n::Boundary SAL_CALL nextWord( const OUString& Text, sal_Int32 nStartPos,
            const css::lang::Locale& nLocale, sal_Int16 WordType) override;
    css::i18n::Boundary SAL_CALL previousWord( const OUString& Text, sal_Int32 nStartPos,
            const css::lang::Locale& nLocale, sal_Int16 WordType) override;
    css::i18n::Boundary SAL_CALL getWordBoundary( const OUString& Text, sal_Int32 nPos,
            const css::lang::Locale& nLocale, sal_Int16 WordType, sal_Bool bDirection ) override;
    css::i18n::LineBreakResults SAL_CALL getLineBreak( const OUString& Text, sal_Int32 nStartPos,
        const css::lang::Locale& nLocale, sal_Int32 nMinBreakPos,
        const css::i18n::LineBreakHyphenationOptions& hOptions,
        const css::i18n::LineBreakUserOptions& bOptions ) override;

protected:
    std::unique_ptr<xdictionary> m_xDict;
    OUString hangingCharacters;
};

#define BREAKITERATOR_CJK( lang ) \
class BreakIterator_##lang : public BreakIterator_CJK {\
public:\
    BreakIterator_##lang (); \
};

BREAKITERATOR_CJK( zh )
BREAKITERATOR_CJK( zh_TW )
BREAKITERATOR_CJK( ja )
BREAKITERATOR_CJK( ko )

#undef BREAKITERATOR__CJK

}

#endif // INCLUDED_I18NPOOL_INC_BREAKITERATOR_CJK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
