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

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <cppuhelper/implbase.hxx>
#include "i18npooldllapi.h"

namespace i18npool
{
class I18NPOOL_DLLPUBLIC BreakIterator
    : public cppu::WeakImplHelper<css::i18n::XBreakIterator, css::lang::XServiceInfo>
{
public:
    BreakIterator();
    virtual ~BreakIterator() override;

    virtual sal_Int32 nextCharacters(const OUString& Text, sal_Int32 nStartPos,
                                              const css::lang::Locale& rLocale,
                                              sal_Int16 nCharacterIteratorMode, sal_Int32 nCount,
                                              sal_Int32& nDone) override = 0;
    virtual sal_Int32 previousCharacters(const OUString& Text, sal_Int32 nStartPos,
                                                  const css::lang::Locale& nLocale,
                                                  sal_Int16 nCharacterIteratorMode,
                                                  sal_Int32 nCount, sal_Int32& nDone) override = 0;

    virtual css::i18n::Boundary previousWord(const OUString& Text, sal_Int32 nStartPos,
                                                      const css::lang::Locale& nLocale,
                                                      sal_Int16 WordType) override = 0;
    virtual css::i18n::Boundary nextWord(const OUString& Text, sal_Int32 nStartPos,
                                                  const css::lang::Locale& nLocale,
                                                  sal_Int16 WordType) override = 0;
    virtual css::i18n::Boundary getWordBoundary(const OUString& Text, sal_Int32 nPos,
                                                         const css::lang::Locale& nLocale,
                                                         sal_Int16 WordType,
                                                         bool bDirection) override = 0;

    virtual bool isBeginWord(const OUString& Text, sal_Int32 nPos,
                                      const css::lang::Locale& nLocale,
                                      sal_Int16 WordType) override = 0;
    virtual bool isEndWord(const OUString& Text, sal_Int32 nPos,
                                    const css::lang::Locale& nLocale,
                                    sal_Int16 WordType) override = 0;
    virtual sal_Int16 getWordType(const OUString& Text, sal_Int32 nPos,
                                           const css::lang::Locale& nLocale) override = 0;

    virtual sal_Int32 beginOfSentence(const OUString& Text, sal_Int32 nStartPos,
                                               const css::lang::Locale& nLocale) override = 0;
    virtual sal_Int32 endOfSentence(const OUString& Text, sal_Int32 nStartPos,
                                             const css::lang::Locale& nLocale) override = 0;

    virtual css::i18n::LineBreakResults
    getLineBreak(const OUString& Text, sal_Int32 nStartPos, const css::lang::Locale& nLocale,
                 sal_Int32 nMinBreakPos, const css::i18n::LineBreakHyphenationOptions& hOptions,
                 const css::i18n::LineBreakUserOptions& bOptions) override = 0;

    virtual sal_Int16 getScriptType(const OUString& Text, sal_Int32 nPos) override = 0;
    virtual sal_Int32 beginOfScript(const OUString& Text, sal_Int32 nStartPos,
                                             sal_Int16 ScriptType) override = 0;
    virtual sal_Int32 endOfScript(const OUString& Text, sal_Int32 nStartPos,
                                           sal_Int16 ScriptType) override = 0;
    virtual sal_Int32 previousScript(const OUString& Text, sal_Int32 nStartPos,
                                              sal_Int16 ScriptType) override = 0;
    virtual sal_Int32 nextScript(const OUString& Text, sal_Int32 nStartPos,
                                          sal_Int16 ScriptType) override = 0;

    virtual sal_Int32 beginOfCharBlock(const OUString& Text, sal_Int32 nStartPos,
                                                const css::lang::Locale& nLocale,
                                                sal_Int16 CharType) override = 0;
    virtual sal_Int32 endOfCharBlock(const OUString& Text, sal_Int32 nStartPos,
                                              const css::lang::Locale& nLocale,
                                              sal_Int16 CharType) override = 0;
    virtual sal_Int32 previousCharBlock(const OUString& Text, sal_Int32 nStartPos,
                                                 const css::lang::Locale& nLocale,
                                                 sal_Int16 CharType) override = 0;
    virtual sal_Int32 nextCharBlock(const OUString& Text, sal_Int32 nStartPos,
                                             const css::lang::Locale& nLocale,
                                             sal_Int16 CharType) override = 0;

    //XServiceInfo
    virtual OUString getImplementationName() override = 0;
    virtual bool supportsService(const OUString& ServiceName) override = 0;
    virtual cpo::uno::Sequence<OUString> getSupportedServiceNames() override = 0;

    // optimised variants that do not require the call-site to allocate a temporary OUString
    virtual sal_Int32 endOfScript(std::u16string_view Text, sal_Int32 nStartPos,
                                  sal_Int16 ScriptType)
        = 0;
    virtual sal_Int32 beginOfScript(std::u16string_view Text, sal_Int32 nStartPos,
                                    sal_Int16 ScriptType)
        = 0;
};

} // namespace i18npool
