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
#ifndef INCLUDED_I18NPOOL_INC_BREAKITERATOR_UNICODE_HXX
#define INCLUDED_I18NPOOL_INC_BREAKITERATOR_UNICODE_HXX

#include <breakiteratorImpl.hxx>

#include <unicode/brkiter.h>
#include <memory>
#include <unordered_map>

namespace com { namespace sun { namespace star { namespace i18n {

#define LOAD_CHARACTER_BREAKITERATOR    0
#define LOAD_WORD_BREAKITERATOR         1
#define LOAD_SENTENCE_BREAKITERATOR     2
#define LOAD_LINE_BREAKITERATOR         3


//  class BreakIterator_Unicode

class BreakIterator_Unicode : public BreakIteratorImpl
{
public:
    BreakIterator_Unicode();
    virtual ~BreakIterator_Unicode() override;

    virtual sal_Int32 SAL_CALL previousCharacters( const OUString& Text, sal_Int32 nStartPos,
        const css::lang::Locale& nLocale, sal_Int16 nCharacterIteratorMode, sal_Int32 nCount,
        sal_Int32& nDone ) override;
    virtual sal_Int32 SAL_CALL nextCharacters( const OUString& Text, sal_Int32 nStartPos,
        const css::lang::Locale& rLocale, sal_Int16 nCharacterIteratorMode, sal_Int32 nCount,
        sal_Int32& nDone ) override;

    virtual Boundary SAL_CALL previousWord( const OUString& Text, sal_Int32 nStartPos,
        const css::lang::Locale& nLocale, sal_Int16 WordType) override;
    virtual Boundary SAL_CALL nextWord( const OUString& Text, sal_Int32 nStartPos,
        const css::lang::Locale& nLocale, sal_Int16 WordType) override;
    virtual Boundary SAL_CALL getWordBoundary( const OUString& Text, sal_Int32 nPos,
        const css::lang::Locale& nLocale, sal_Int16 WordType, sal_Bool bDirection ) override;

    virtual sal_Int32 SAL_CALL beginOfSentence( const OUString& Text, sal_Int32 nStartPos,
        const css::lang::Locale& nLocale ) override;
    virtual sal_Int32 SAL_CALL endOfSentence( const OUString& Text, sal_Int32 nStartPos,
        const css::lang::Locale& nLocale ) override;

    virtual LineBreakResults SAL_CALL getLineBreak( const OUString& Text, sal_Int32 nStartPos,
        const css::lang::Locale& nLocale, sal_Int32 nMinBreakPos,
        const LineBreakHyphenationOptions& hOptions, const LineBreakUserOptions& bOptions ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

protected:
    const sal_Char *cBreakIterator, *lineRule;

    /** Used as map value. */
    struct BI_ValueData
    {
        OUString                                maICUText;
        UText*                                  mpUt;
        std::shared_ptr< icu::BreakIterator >   mpBreakIterator;

        BI_ValueData() : mpUt(nullptr)
        {
        }
        ~BI_ValueData()
        {
            utext_close(mpUt);
        }
    };

    struct BI_Data
    {
        std::shared_ptr< BI_ValueData > mpValue;
        OString                         maBIMapKey;
    } character, sentence, line, *icuBI;
    BI_Data words[4]; // 4 is css::i18n::WordType enumeration size

    /// @throws css::uno::RuntimeException
    void SAL_CALL loadICUBreakIterator(const css::lang::Locale& rLocale,
        sal_Int16 rBreakType, sal_Int16 rWordType, const sal_Char* name, const OUString& rText);

public:
    typedef std::unordered_map< OString, std::shared_ptr< BI_ValueData >, OStringHash > BIMap;
};

} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
