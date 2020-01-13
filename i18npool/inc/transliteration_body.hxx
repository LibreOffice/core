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

#include "transliteration_commonclass.hxx"
#include <i18nutil/casefolding.hxx>

namespace i18npool {

class Transliteration_body : public transliteration_commonclass
{
public:
    Transliteration_body();

    // Methods which are shared.
    sal_Int16 SAL_CALL getType() override;

    OUString transliterateImpl(const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
        css::uno::Sequence< sal_Int32 >& offset, bool useOffset) override;

        OUString SAL_CALL
        transliterateChar2String( sal_Unicode inChar) override;

        virtual sal_Unicode SAL_CALL
        transliterateChar2Char( sal_Unicode inChar) override;

    OUString foldingImpl(const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
        css::uno::Sequence< sal_Int32 >& offset, bool useOffset) override;

    sal_Bool SAL_CALL equals(
        const OUString& str1, sal_Int32 pos1, sal_Int32 nCount1, sal_Int32& nMatch1,
        const OUString& str2, sal_Int32 pos2, sal_Int32 nCount2, sal_Int32& nMatch2 ) override;

    css::uno::Sequence< OUString > SAL_CALL transliterateRange( const OUString& str1,
        const OUString& str2 ) override;

protected:
    MappingType nMappingType;
};

class Transliteration_u2l final : public Transliteration_body
{
public:
    Transliteration_u2l();
};

class Transliteration_l2u final : public Transliteration_body
{
public:
    Transliteration_l2u();
};

class Transliteration_casemapping final : public Transliteration_body
{
public:
    Transliteration_casemapping();
    void setMappingType(const MappingType rMappingType, const css::lang::Locale& rLocale );
};

class Transliteration_togglecase final : public Transliteration_body
{
public:
    Transliteration_togglecase();
};

class Transliteration_titlecase final : public Transliteration_body
{
public:
    Transliteration_titlecase();

    virtual OUString transliterateImpl( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
                                        css::uno::Sequence< sal_Int32 >& offset, bool useOffset ) override;
};

class Transliteration_sentencecase final : public Transliteration_body
{
public:
    Transliteration_sentencecase();

    virtual OUString transliterateImpl( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
                                        css::uno::Sequence< sal_Int32 >& offset, bool useOffset ) override;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
