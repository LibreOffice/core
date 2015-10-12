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
    virtual ~BreakIterator_Unicode();

    virtual sal_Int32 SAL_CALL previousCharacters( const OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 nCharacterIteratorMode, sal_Int32 nCount,
        sal_Int32& nDone ) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL nextCharacters( const OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& rLocale, sal_Int16 nCharacterIteratorMode, sal_Int32 nCount,
        sal_Int32& nDone ) throw(com::sun::star::uno::RuntimeException, std::exception) override;

    virtual Boundary SAL_CALL previousWord( const OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual Boundary SAL_CALL nextWord( const OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual Boundary SAL_CALL getWordBoundary( const OUString& Text, sal_Int32 nPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType, sal_Bool bDirection )
        throw(com::sun::star::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL beginOfSentence( const OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale ) throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL endOfSentence( const OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale ) throw(com::sun::star::uno::RuntimeException, std::exception) override;

    virtual LineBreakResults SAL_CALL getLineBreak( const OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int32 nMinBreakPos,
        const LineBreakHyphenationOptions& hOptions, const LineBreakUserOptions& bOptions )
        throw(com::sun::star::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;

protected:
    const sal_Char *cBreakIterator, *lineRule;
    Boundary result; // for word break iterator

    struct BI_Data
    {
        OUString aICUText;
        UText *ut;
        icu::BreakIterator *aBreakIterator;
        com::sun::star::lang::Locale maLocale;

        BI_Data()
            : ut(NULL)
            , aBreakIterator(NULL)
        {
        }
        ~BI_Data()
        {
            utext_close(ut);
        }

    } character, sentence, line, *icuBI;
    BI_Data words[4]; // 4 is css::i18n::WordType enumeration size

    sal_Int16 aBreakType;

    void SAL_CALL loadICUBreakIterator(const com::sun::star::lang::Locale& rLocale,
        sal_Int16 rBreakType, sal_Int16 rWordType, const sal_Char* name, const OUString& rText) throw(com::sun::star::uno::RuntimeException);
};

} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
