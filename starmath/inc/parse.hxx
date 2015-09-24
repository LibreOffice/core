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
#include <memory>
#include <set>
#include <vector>

#include "types.hxx"
#include "token.hxx"
#include "error.hxx"
#include "node.hxx"

class SmParser
{
    OUString        m_aBufferString;
    SmToken         m_aCurToken;
    SmNodeStack     m_aNodeStack;
    std::vector<std::unique_ptr<SmErrorDesc>> m_aErrDescList;
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

    SmParser(const SmParser&) SAL_DELETED_FUNCTION;
    SmParser& operator=(const SmParser&) SAL_DELETED_FUNCTION;

#if OSL_DEBUG_LEVEL > 1
    bool            IsDelimiter( const OUString &rTxt, sal_Int32 nPos );
#endif
    void            NextToken();
    sal_Int32       GetTokenIndex() const   { return m_nTokenIndex; }
    void            Replace( sal_Int32 nPos, sal_Int32 nLen, const OUString &rText );

    inline bool     TokenInGroup( sal_uLong nGroup );

    // grammar
    void    DoTable();
    void    DoLine();
    void    DoExpression();
    void    DoRelation();
    void    DoSum();
    void    DoProduct();
    void    DoSubSup(sal_uLong nActiveGroup);
    void    DoOpSubSup();
    void    DoPower();
    void    DoBlank();
    void    DoTerm(bool bGroupNumberIdent);
    void    DoEscape();
    void    DoOperator();
    void    DoOper();
    void    DoUnOper();
    void    DoAlign();
    void    DoFontAttribut();
    void    DoAttribut();
    void    DoFont();
    void    DoFontSize();
    void    DoColor();
    void    DoBrace();
    void    DoBracebody(bool bIsLeftRight);
    void    DoFunction();
    void    DoBinom();
    void    DoStack();
    void    DoMatrix();
    void    DoSpecial();
    void    DoGlyphSpecial();
    // end of grammar

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
    std::set< OUString >   GetUsedSymbols() const      { return m_aUsedSymbols; }
};


inline bool SmParser::TokenInGroup( sal_uLong nGroup)
{
    return (m_aCurToken.nGroup & nGroup) != 0;
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
