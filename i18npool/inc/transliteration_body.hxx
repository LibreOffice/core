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
#ifndef INCLUDED_I18NPOOL_INC_TRANSLITERATION_BODY_HXX
#define INCLUDED_I18NPOOL_INC_TRANSLITERATION_BODY_HXX

#include <transliteration_commonclass.hxx>
#include <i18nutil/casefolding.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

class Transliteration_body : public transliteration_commonclass
{
public:
    Transliteration_body();

    // Methods which are shared.
    sal_Int16 SAL_CALL getType() throw(css::uno::RuntimeException, std::exception) override;

    OUString SAL_CALL transliterate(const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
        css::uno::Sequence< sal_Int32 >& offset) throw(css::uno::RuntimeException, std::exception) override;

        OUString SAL_CALL
        transliterateChar2String( sal_Unicode inChar)
        throw(css::uno::RuntimeException, std::exception) override;

        virtual sal_Unicode SAL_CALL
        transliterateChar2Char( sal_Unicode inChar)
        throw(css::uno::RuntimeException,
        css::i18n::MultipleCharsOutputException, std::exception) override;

    OUString SAL_CALL folding(const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
        css::uno::Sequence< sal_Int32 >& offset) throw(css::uno::RuntimeException, std::exception) override;

    sal_Bool SAL_CALL equals(
        const OUString& str1, sal_Int32 pos1, sal_Int32 nCount1, sal_Int32& nMatch1,
        const OUString& str2, sal_Int32 pos2, sal_Int32 nCount2, sal_Int32& nMatch2 )
        throw(css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence< OUString > SAL_CALL transliterateRange( const OUString& str1,
        const OUString& str2 ) throw(css::uno::RuntimeException, std::exception) override;

protected:
    sal_uInt8 nMappingType;
};

class Transliteration_u2l : public Transliteration_body
{
public:
    Transliteration_u2l();
};

class Transliteration_l2u : public Transliteration_body
{
public:
    Transliteration_l2u();
};

class Transliteration_casemapping : public Transliteration_body
{
public:
    Transliteration_casemapping();
    void SAL_CALL setMappingType(const sal_uInt8 rMappingType, const css::lang::Locale& rLocale );
};

class Transliteration_togglecase : public Transliteration_body
{
public:
    Transliteration_togglecase();
};

class Transliteration_titlecase : public Transliteration_body
{
public:
    Transliteration_titlecase();

    virtual OUString SAL_CALL transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, css::uno::Sequence< sal_Int32 >& offset  ) throw(css::uno::RuntimeException, std::exception) override;
};

class Transliteration_sentencecase : public Transliteration_body
{
public:
    Transliteration_sentencecase();

    virtual OUString SAL_CALL transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, css::uno::Sequence< sal_Int32 >& offset  ) throw(css::uno::RuntimeException, std::exception) override;
};

} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
