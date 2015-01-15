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
#ifndef INCLUDED_STARMATH_INC_PARSE_HXX
#define INCLUDED_STARMATH_INC_PARSE_HXX

#include <vcl/svapp.hxx>

#include <set>

#include "types.hxx"
#include "token.hxx"
#include "error.hxx"
#include "node.hxx"

#include <boost/noncopyable.hpp>

class SmParser : boost::noncopyable
{
    OUString        m_aBufferString;
    SmToken         m_aCurToken;
    SmNodeStack     m_aNodeStack;
    SmErrDescList   m_aErrDescList;
    int             m_nCurError;
    LanguageType    m_nLang;
    sal_Int32       m_nBufferIndex,
                    m_nTokenIndex;
    sal_Int32       m_Row,
                    m_nColOff;
    bool            bImportSymNames,
                    m_bExportSymNames;

    // map of used symbols (used to reduce file size by exporting only actually used symbols)
    std::set< OUString >   m_aUsedSymbols;

    //! locale where '.' is decimal separator!
    ::com::sun::star::lang::Locale m_aDotLoc;

#if OSL_DEBUG_LEVEL > 1
    bool            IsDelimiter( const OUString &rTxt, sal_Int32 nPos );
#endif
    void            NextToken();
    sal_Int32       GetTokenIndex() const   { return m_nTokenIndex; }
    void            Replace( sal_Int32 nPos, sal_Int32 nLen, const OUString &rText );

    inline bool     TokenInGroup( sal_uLong nGroup );

    // grammar
    void    Table();
    void    Line();
    void    Expression();
    void    Relation();
    void    Sum();
    void    Product();
    void    SubSup(sal_uLong nActiveGroup);
    void    OpSubSup();
    void    Power();
    void    Blank();
    void    Term(bool bGroupNumberIdent);
    void    Escape();
    void    Operator();
    void    Oper();
    void    UnOper();
    void    Align();
    void    FontAttribut();
    void    Attribut();
    void    Font();
    void    FontSize();
    void    Color();
    void    Brace();
    void    Bracebody(bool bIsLeftRight);
    void    Function();
    void    Binom();
    void    Stack();
    void    Matrix();
    void    Special();
    void    GlyphSpecial();
    // end of grammar

    LanguageType    GetLanguage() const { return m_nLang; }
    void            SetLanguage( LanguageType nNewLang ) { m_nLang = nNewLang; }

    void    Error(SmParseError Error);

    void    ClearUsedSymbols()                              { m_aUsedSymbols.clear(); }
    void    AddToUsedSymbols( const OUString &rSymbolName ) { m_aUsedSymbols.insert( rSymbolName ); }

public:
                 SmParser();

    /** Parse rBuffer to formula tree */
    SmNode      *Parse(const OUString &rBuffer);
    /** Parse rBuffer to formula subtree that constitutes an expression */
    SmNode      *ParseExpression(const OUString &rBuffer);

    const OUString & GetText() const { return m_aBufferString; };

    bool        IsImportSymbolNames() const        { return bImportSymNames; }
    void        SetImportSymbolNames(bool bVal)    { bImportSymNames = bVal; }
    bool        IsExportSymbolNames() const        { return m_bExportSymNames; }
    void        SetExportSymbolNames(bool bVal)    { m_bExportSymNames = bVal; }

    size_t      AddError(SmParseError Type, SmNode *pNode);
    const SmErrorDesc*  NextError();
    const SmErrorDesc*  PrevError();
    const SmErrorDesc*  GetError(size_t i);
    static const SmTokenTableEntry* GetTokenTableEntry( const OUString &rName );
    bool    IsUsedSymbol( const OUString &rSymbolName ) const { return m_aUsedSymbols.find( rSymbolName ) != m_aUsedSymbols.end(); }
    std::set< OUString >   GetUsedSymbols() const      { return m_aUsedSymbols; }
};


inline bool SmParser::TokenInGroup( sal_uLong nGroup)
{
    return (m_aCurToken.nGroup & nGroup) ? true : false;
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
