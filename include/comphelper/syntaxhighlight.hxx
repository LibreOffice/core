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
#ifndef _COMPHELPER_SYNTAXHIGHLIGHT_HXX
#define _COMPHELPER_SYNTAXHIGHLIGHT_HXX

#include <vector>
#include <rtl/ustring.hxx>

#include <comphelper/comphelperdllapi.h>


#if defined CDECL
#undef CDECL
#endif

// for the bsearch
#ifdef WNT
#define CDECL _cdecl
#endif
#if defined(UNX)
#define CDECL
#endif
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

struct HighlightPortion { sal_uInt16 nBegin; sal_uInt16 nEnd; TokenTypes tokenType; };


typedef std::vector<HighlightPortion> HighlightPortions;

/////////////////////////////////////////////////////////////////////////
// Auxiliary class to support JavaScript modules, next to find functions which
// will later will be used for syntax highlighting

// Flags for character properties
#define CHAR_START_IDENTIFIER   0x0001
#define CHAR_IN_IDENTIFIER      0x0002
#define CHAR_START_NUMBER       0x0004
#define CHAR_IN_NUMBER          0x0008
#define CHAR_IN_HEX_NUMBER      0x0010
#define CHAR_IN_OCT_NUMBER      0x0020
#define CHAR_START_STRING       0x0040
#define CHAR_OPERATOR           0x0080
#define CHAR_SPACE              0x0100
#define CHAR_EOL                0x0200

#define CHAR_EOF                0x00


// Language mode of the Highlighter (possibly to be refined later with keyword
// lists, C comment flags)
enum HighlighterLanguage
{
    HIGHLIGHT_BASIC,
    HIGHLIGHT_SQL
};

class SimpleTokenizer_Impl
{
    HighlighterLanguage aLanguage;
    // Character information tables
    sal_uInt16 aCharTypeTab[256];

    const sal_Unicode* mpStringBegin;
    const sal_Unicode* mpActualPos;

    // Lines and columns
    sal_uInt32 nLine;
    sal_uInt32 nCol;

    sal_Unicode peekChar( void )    { return *mpActualPos; }
    sal_Unicode getChar( void )     { nCol++; return *mpActualPos++; }

    // Auxiliary function: testing of the character flags
    sal_Bool testCharFlags( sal_Unicode c, sal_uInt16 nTestFlags );

    // Get new token, EmptyString == nothing more over there
    sal_Bool getNextToken( /*out*/TokenTypes& reType,
        /*out*/const sal_Unicode*& rpStartPos, /*out*/const sal_Unicode*& rpEndPos );

    const char** ppListKeyWords;
    sal_uInt16 nKeyWordCount;

public:
    SimpleTokenizer_Impl( HighlighterLanguage aLang = HIGHLIGHT_BASIC );
    ~SimpleTokenizer_Impl( void );

    sal_uInt16 parseLine( sal_uInt32 nLine, const OUString* aSource );
    void getHighlightPortions( sal_uInt32 nParseLine, const OUString& rLine,
                                                    /*out*/HighlightPortions& portions );
    void setKeyWords( const char** ppKeyWords, sal_uInt16 nCount );
};


//*** SyntaxHighlighter Class ***
// Concept: the Highlighter will be notified of all changes in the source
// (notifyChange) and returns the caller the range of lines, which based on the
// changes, need to be highlighted again. For this the Highlighter marks all
// lines internally whether or not C comments begin or end.
class COMPHELPER_DLLPUBLIC SyntaxHighlighter
{
    HighlighterLanguage eLanguage;
    SimpleTokenizer_Impl* m_pSimpleTokenizer;
    char* m_pKeyWords;
    sal_uInt16 m_nKeyWordCount;

//  void initializeKeyWords( HighlighterLanguage eLanguage );

public:
    SyntaxHighlighter( void );
    ~SyntaxHighlighter( void );

    // (Re-)initialize Highlighter. The line-table will be completely erased,
    // meaning that on completion an empty Source is assumed.
    // notifyChange() can only be given line 0
    void initialize( HighlighterLanguage eLanguage_ );

    void notifyChange( sal_uInt32 nLine, sal_Int32 nLineCountDifference,
                                const OUString* pChangedLines, sal_uInt32 nArrayLength);

    void getHighlightPortions( sal_uInt32 nLine, const OUString& rLine,
                                            HighlightPortions& pPortions );

    HighlighterLanguage GetLanguage() { return eLanguage;}
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
