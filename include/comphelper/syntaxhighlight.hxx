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

#include <vector>

#include <boost/scoped_ptr.hpp>
#include <rtl/ustring.hxx>

#include <comphelper/comphelperdllapi.h>

#ifdef UNX
#include <sys/resource.h>
#endif

// Token-Typen TT_...
enum TokenTypes
{
    TT_UNKNOWN,
    TT_IDENTIFIER,
    TT_WHITESPACE,
    TT_NUMBER,
    TT_STRING,
    TT_EOL,
    TT_COMMENT,
    TT_ERROR,
    TT_OPERATOR,
    TT_KEYWORDS,
    TT_PARAMETER
};

struct HighlightPortion {
    sal_Int32 nBegin;
    sal_Int32 nEnd;
    TokenTypes tokenType;

    HighlightPortion(
        sal_Int32 theBegin, sal_Int32 theEnd, TokenTypes theTokenType):
        nBegin(theBegin), nEnd(theEnd), tokenType(theTokenType)
    {}
};

// Language mode of the Highlighter (possibly to be refined later with keyword
// lists, C comment flags)
enum HighlighterLanguage
{
    HIGHLIGHT_BASIC,
    HIGHLIGHT_SQL
};

//*** SyntaxHighlighter Class ***
// Concept: the Highlighter will be notified of all changes in the source
// (notifyChange) and returns the caller the range of lines, which based on the
// changes, need to be highlighted again. For this the Highlighter marks all
// lines internally whether or not C comments begin or end.
class COMPHELPER_DLLPUBLIC SyntaxHighlighter
{
    class Tokenizer;

    HighlighterLanguage eLanguage;
    boost::scoped_ptr<Tokenizer> m_tokenizer;
    char* m_pKeyWords;
    sal_uInt16 m_nKeyWordCount;

//  void initializeKeyWords( HighlighterLanguage eLanguage );

public:
    SyntaxHighlighter( void );
    ~SyntaxHighlighter( void );

    // (Re-)initialize Highlighter. The line-table will be completely erased,
    // meaning that on completion an empty Source is assumed.
    void initialize( HighlighterLanguage eLanguage_ );

    void notifyChange(const OUString* pChangedLines, sal_uInt32 nArrayLength);

    void getHighlightPortions( const OUString& rLine,
                               std::vector<HighlightPortion>& pPortions );

    HighlighterLanguage GetLanguage() { return eLanguage;}
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
