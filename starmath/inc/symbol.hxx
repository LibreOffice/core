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

#ifndef INCLUDED_STARMATH_INC_SYMBOL_HXX
#define INCLUDED_STARMATH_INC_SYMBOL_HXX

#include <map>
#include <vector>
#include <set>

#include "utility.hxx"


#define SYMBOL_NONE     0xFFFF

class SmSym
{
private:
    SmFace              m_aFace;
    OUString            m_aName;
    OUString            m_aExportName;
    OUString            m_aSetName;
    sal_UCS4            m_cChar;
    bool                m_bPredefined;

public:
    SmSym();
    SmSym(const OUString& rName, const vcl::Font& rFont, sal_UCS4 cChar,
          const OUString& rSet, bool bIsPredefined = false);
    SmSym(const SmSym& rSymbol);

    SmSym&      operator = (const SmSym& rSymbol);

    const vcl::Font&  GetFace() const { return m_aFace; }
    sal_UCS4        GetCharacter() const { return m_cChar; }
    const OUString&   GetName() const { return m_aName; }

    bool            IsPredefined() const        { return m_bPredefined; }
    const OUString& GetSymbolSetName() const    { return m_aSetName; }
    const OUString& GetExportName() const       { return m_aExportName; }
    void            SetExportName( const OUString &rName )        { m_aExportName = rName; }

    // true if rSymbol has the same name, font and character
    bool            IsEqualInUI( const SmSym& rSymbol ) const;
};

// type of the actual container to hold the symbols
typedef std::map< OUString, SmSym >    SymbolMap_t;

// vector of pointers to the actual symbols in the above container
typedef std::vector< const SmSym * >            SymbolPtrVec_t;


class SmSymbolManager
{
private:
    SymbolMap_t         m_aSymbols;
    bool                m_bModified;

public:
    SmSymbolManager();
    SmSymbolManager(const SmSymbolManager& rSymbolSetManager);
    ~SmSymbolManager();

    SmSymbolManager &   operator = (const SmSymbolManager& rSymbolSetManager);

    // symbol sets are for UI purpose only, thus we assemble them here
    std::set< OUString >      GetSymbolSetNames() const;
    const SymbolPtrVec_t    GetSymbolSet(  const OUString& rSymbolSetName );

    const SymbolPtrVec_t    GetSymbols() const;
    bool                    AddOrReplaceSymbol( const SmSym & rSymbol, bool bForceChange = false );
    void                    RemoveSymbol( const OUString & rSymbolName );

    SmSym       *   GetSymbolByName(const OUString& rSymbolName);
    const SmSym *   GetSymbolByName(const OUString& rSymbolName) const
    {
        return const_cast<SmSymbolManager *>(this)->GetSymbolByName(rSymbolName);
    }

    bool        IsModified() const          { return m_bModified; }
    void        SetModified(bool bModify)   { m_bModified = bModify; }

    void        Load();
    void        Save();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
