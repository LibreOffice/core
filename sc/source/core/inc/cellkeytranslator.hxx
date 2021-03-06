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

#include <formula/opcode.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <vector>
#include <memory>
#include <unordered_map>

namespace com::sun::star::lang
{
struct Locale;
}

struct TransItem;

struct ScCellKeyword
{
    const char* mpName;
    OpCode meOpCode;
    const css::lang::Locale& mrLocale;

    ScCellKeyword(const char* pName, OpCode eOpCode, const css::lang::Locale& rLocale);
};

typedef std::unordered_map<OUString, ::std::vector<ScCellKeyword>> ScCellKeywordHashMap;

/** Translate cell function keywords.

    This class provides a convenient way to translate a string keyword used as
    a cell function argument.  Since Calc's built-in cell functions don't
    localize string keywords, this class is used mainly to deal with an Excel
    document where string names may be localized.

    To use, simply call the

       ScCellKeywordTranslator::transKeyword(...)

    function.

    Note that when the locale and/or the opcode is specified, the function
    tries to find a string with matching locale and/or opcode. But when it
    fails to find one that satisfies the specified locale and/or opcode, it
    returns a translated string with non-matching locale and/or opcode if
    available. */
class ScCellKeywordTranslator
{
public:
    static void transKeyword(OUString& rName, const css::lang::Locale* pLocale, OpCode eOpCode);
    ~ScCellKeywordTranslator();

private:
    ScCellKeywordTranslator();

    void init();
    void addToMap(const OUString& rKey, const char* pName, const css::lang::Locale& rLocale,
                  OpCode eOpCode);
    void addToMap(const TransItem* pItems, const css::lang::Locale& rLocale);

    static ::std::unique_ptr<ScCellKeywordTranslator> spInstance;
    ScCellKeywordHashMap maStringNameMap;
    ::utl::TransliterationWrapper maTransWrapper;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
