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

#ifndef INCLUDED_SW_SOURCE_FILTER_HTML_PARCSS1_HXX
#define INCLUDED_SW_SOURCE_FILTER_HTML_PARCSS1_HXX

// tokens of the CSS1 parser
enum CSS1Token
{
    CSS1_NULL,

    CSS1_IDENT,
    CSS1_STRING,
    CSS1_NUMBER,
    CSS1_PERCENTAGE,
    CSS1_LENGTH,    // absolute length in 1/100 MM
    CSS1_PIXLENGTH, // length in pixels
    CSS1_EMS,
    CSS1_EMX,
    CSS1_HEXCOLOR,

    CSS1_DOT_W_WS,
    CSS1_DOT_WO_WS,
    CSS1_COLON,
    CSS1_SLASH,
    CSS1_PLUS,
    CSS1_MINUS,
    CSS1_OBRACE,
    CSS1_CBRACE,
    CSS1_SEMICOLON,
    CSS1_COMMA,
    CSS1_HASH,

    CSS1_IMPORT_SYM,
    CSS1_PAGE_SYM, // Feature: PrintExt

    CSS1_IMPORTANT_SYM,

    CSS1_URL,
    CSS1_RGB
};

enum CSS1ParserState
{
    CSS1_PAR_ACCEPTED = 0,
    CSS1_PAR_WORKING
};

enum CSS1SelectorType
{
    CSS1_SELTYPE_ELEMENT,
    CSS1_SELTYPE_ELEM_CLASS,
    CSS1_SELTYPE_CLASS,
    CSS1_SELTYPE_ID,
    CSS1_SELTYPE_PSEUDO,
    CSS1_SELTYPE_PAGE // Feature: PrintExt
};

/** A simple selector
 *
 * This class represents a simple selector, e.g.
 * - a HTML element name
 * - a HTML element name with a class (separated by a dot)
 * - a class (without a dot)
 * - an ID (set with ID=xxx)
 * - a pseudo element
 *
 * These simple selectors are chained in a list to complete selectors
 */
class CSS1Selector
{
    CSS1SelectorType const eType; // the type
    OUString const aSelector;     // the selector itself
    CSS1Selector *pNext;    // the following component

public:
    CSS1Selector( CSS1SelectorType eTyp, const OUString &rSel )
        : eType(eTyp), aSelector( rSel ), pNext( nullptr )
    {}

    ~CSS1Selector();

    CSS1SelectorType GetType() const { return eType; }
    const OUString& GetString() const { return aSelector; }

    void SetNext( CSS1Selector *pNxt ) { pNext = pNxt; }
    const CSS1Selector *GetNext() const { return pNext; }
};

/** a subexpression of a CSS1 declaration
 *
 * It contains
 * - the type of this expression (= token)
 * - the value as string (and/or double, with algebraic sign for NUMBER and LENGTH)
 * - the operator with that it is connected with the *predecessor* expression
 */
struct CSS1Expression
{
    sal_Unicode cOp; // type of the link with its predecessor
    CSS1Token eType; // type of the expression
    OUString aValue; // value as string
    double nValue;   // value as number (TWIPs for LENGTH)
    CSS1Expression *pNext; // the following component

public:
    CSS1Expression( CSS1Token eTyp, const OUString &rVal,
                    double nVal, sal_Unicode cO = 0 )
        : cOp(cO), eType(eTyp), aValue(rVal), nValue(nVal), pNext(nullptr)
    {}

    ~CSS1Expression();

    inline void Set( CSS1Token eTyp, const OUString &rVal, double nVal );

    CSS1Token GetType() const { return eType; }
    const OUString& GetString() const { return aValue; }
    double GetNumber() const { return nValue; }
    inline sal_uInt32 GetULength() const;
    inline sal_Int32 GetSLength() const;
    sal_Unicode GetOp() const { return cOp; }

    void GetURL( OUString& rURL ) const;
    bool GetColor( Color &rRGB ) const;

    void SetNext( CSS1Expression *pNxt ) { pNext = pNxt; }
    const CSS1Expression *GetNext() const { return pNext; }
};

inline void CSS1Expression::Set( CSS1Token eTyp, const OUString &rVal,
                                 double nVal )
{
    cOp = 0; eType = eTyp; aValue = rVal; nValue = nVal; pNext = nullptr;
}

inline sal_uInt32 CSS1Expression::GetULength() const
{
    return nValue < 0. ? 0UL : static_cast<sal_uInt32>(nValue + .5);
}

inline sal_Int32 CSS1Expression::GetSLength() const
{
    return static_cast<sal_Int32>(nValue + (nValue < 0. ? -.5 : .5 ));
}

/** Parser of a style element/option
 *
 * This class parses the content of a style element or a style option and preprocesses it.
 *
 * The result of the parser is forwarded to derived parsers by the methods SelectorParsed()
 * and DeclarationParsed(). Example:
 * H1, H2 { font-weight: bold; text-align: right }
 *  |  |                    |                  |
 *  |  |                    |                  DeclP( 'text-align', 'right' )
 *  |  |                    DeclP( 'font-weight', 'bold' )
 *  |  SelP( 'H2', false )
 *  SelP( 'H1', true )
 */
class CSS1Parser
{
    bool bWhiteSpace : 1; // read a whitespace?
    bool bEOF : 1; // is end of "file"?

    sal_Unicode cNextCh; // next character

    sal_Int32 nInPos; // current position in the input string

    sal_uInt32 nlLineNr; // current row number
    sal_uInt32 nlLinePos; // current column number

    double nValue; // value of the token as number

    CSS1ParserState eState; // current state of the parser
    CSS1Token nToken; // the current token

    OUString aIn; // the string to parse
    OUString aToken; // token as string

    /// prepare parsing
    void InitRead( const OUString& rIn );

    /// @returns the next character to parse
    sal_Unicode GetNextChar();

    /// @returns the next token to parse
    CSS1Token GetNextToken();

    /// Is the parser still working?
    bool IsParserWorking() const { return CSS1_PAR_WORKING == eState; }

    bool IsEOF() const { return bEOF; }

    // parse parts of the grammar
    void ParseRule();
    std::unique_ptr<CSS1Selector> ParseSelector();
    std::unique_ptr<CSS1Expression> ParseDeclaration( OUString& rProperty );

protected:
    void ParseStyleSheet();

    /** parse the content of a HTML style element
     *
     * For each selector and each declaration the methods SelectorParsed()
     * or DeclarationParsed() need to be called afterwards
     *
     * @param rIn the style element as string
     */
    void ParseStyleSheet( const OUString& rIn );

    /** parse the content of a HTML style option
     *
     * For each selector and each declaration the methods SelectorParsed()
     * or DeclarationParsed() need to be called afterwards.
     *
     * @param rIn the style option as string
     * @return true if ???
     */
    void ParseStyleOption( const OUString& rIn );

    /** Called after a selector was parsed.
     *
     * @param pSelector The selector that was parsed
     * @param bFirst if true, a new declaration starts with this selector
     */
    virtual void SelectorParsed( std::unique_ptr<CSS1Selector> pSelector, bool bFirst );

    /** Called after a declaration or property was parsed
     *
     * @param rProperty The declaration/property
     * @param pExpr ???
     */
    virtual void DeclarationParsed( const OUString& rProperty,
                                    std::unique_ptr<CSS1Expression> pExpr );

public:
    CSS1Parser();
    virtual ~CSS1Parser();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
