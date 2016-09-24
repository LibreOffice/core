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
#ifndef INCLUDED_COMPHELPER_SYNTAXHIGHLIGHT_HXX
#define INCLUDED_COMPHELPER_SYNTAXHIGHLIGHT_HXX

#include <rtl/ustring.hxx>

#include <comphelper/comphelperdllapi.h>

#ifdef UNX
#include <sys/resource.h>
#endif

#include <vector>
#include <memory>

enum class TokenType
{
    Unknown,
    Identifier,
    Whitespace,
    Number,
    String,
    EOL,
    Comment,
    Error,
    Operator,
    Keywords,
    Parameter,
    LAST = Parameter
};

struct HighlightPortion {
    sal_Int32 nBegin;
    sal_Int32 nEnd;
    TokenType tokenType;

    HighlightPortion(
        sal_Int32 theBegin, sal_Int32 theEnd, TokenType theTokenType):
        nBegin(theBegin), nEnd(theEnd), tokenType(theTokenType)
    {}
};

// Language mode of the Highlighter (possibly to be refined later with keyword
// lists, C comment flags)
enum class HighlighterLanguage
{
    Basic,
    SQL
};

class COMPHELPER_DLLPUBLIC SyntaxHighlighter
{
    class Tokenizer;

    HighlighterLanguage eLanguage;
    std::unique_ptr<Tokenizer> m_tokenizer;

    SyntaxHighlighter(const SyntaxHighlighter&) = delete;
    SyntaxHighlighter& operator=(const SyntaxHighlighter&) = delete;
public:
    SyntaxHighlighter(HighlighterLanguage language);
    ~SyntaxHighlighter();

    void getHighlightPortions( const OUString& rLine,
                               std::vector<HighlightPortion>& pPortions ) const;

    HighlighterLanguage GetLanguage();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
