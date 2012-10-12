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

#ifndef SYMBOL_HXX
#define SYMBOL_HXX

#include <vcl/font.hxx>
#include <svl/lstner.hxx>

#include <map>
#include <vector>
#include <set>
#include <functional>
#include <algorithm>

#include "unomodel.hxx"
#include "utility.hxx"
#include "smmod.hxx"


#define SYMBOLSET_NONE      0xFFFF
#define SYMBOL_NONE     0xFFFF


////////////////////////////////////////////////////////////////////////////////

inline const OUString GetExportSymbolName( const OUString &rUiSymbolName )
{
    return SM_MOD()->GetLocSymbolData().GetExportSymbolName( rUiSymbolName );
}


inline const OUString GetUiSymbolName( const OUString &rExportSymbolName )
{
    return SM_MOD()->GetLocSymbolData().GetUiSymbolName( rExportSymbolName );
}

inline const OUString GetExportSymbolSetName( const OUString &rUiSymbolSetName )
{
    return SM_MOD()->GetLocSymbolData().GetExportSymbolSetName( rUiSymbolSetName );
}


inline const OUString GetUiSymbolSetName( const OUString &rExportSymbolSetName )
{
    return SM_MOD()->GetLocSymbolData().GetUiSymbolSetName( rExportSymbolSetName );
}

////////////////////////////////////////////////////////////////////////////////

class SmSym
{
private:
    SmFace              m_aFace;
    OUString            m_aName;
    OUString            m_aExportName;
    OUString            m_aSetName;
    sal_UCS4            m_cChar;
    bool                m_bPredefined;
    bool                m_bDocSymbol;

public:
    SmSym();
    SmSym(const OUString& rName, const Font& rFont, sal_UCS4 cChar,
          const OUString& rSet, bool bIsPredefined = false);
    SmSym(const SmSym& rSymbol);

    SmSym&      operator = (const SmSym& rSymbol);

    const Font&     GetFace() const { return m_aFace; }
    sal_UCS4        GetCharacter() const { return m_cChar; }
    const OUString&   GetName() const { return m_aName; }

    void            SetFace( const Font& rFont )        { m_aFace = rFont; }
    void            SetCharacter( sal_UCS4 cChar )   { m_cChar = cChar; }

    bool            IsPredefined() const        { return m_bPredefined; }
    const OUString& GetSymbolSetName() const    { return m_aSetName; }
    void            SetSymbolSetName( const OUString &rName )     { m_aSetName = rName; }
    const OUString& GetExportName() const       { return m_aExportName; }
    void            SetExportName( const OUString &rName )        { m_aExportName = rName; }

    bool            IsDocSymbol() const         { return m_bDocSymbol; }
    void            SetDocSymbol( bool bVal )   { m_bDocSymbol = bVal; }

    // true if rSymbol has the same name, font and character
    bool            IsEqualInUI( const SmSym& rSymbol ) const;
};

// type of the actual container to hold the symbols
typedef std::map< OUString, SmSym >    SymbolMap_t;

// vector of pointers to the actual symbols in the above container
typedef std::vector< const SmSym * >            SymbolPtrVec_t;

struct lt_SmSymPtr : public std::binary_function< const SmSym *, const SmSym *, bool >
{
    bool operator() ( const SmSym *pSym1, const SmSym *pSym2 ) const
    {
        return pSym1->GetCharacter() < pSym2->GetCharacter();
    }
};


class SmSymbolManager : public SfxListener
{
private:
    SymbolMap_t         m_aSymbols;
    bool                m_bModified;

    virtual void SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType,
                        const SfxHint& rHint, const TypeId& rHintType);

public:
    SmSymbolManager();
    SmSymbolManager(const SmSymbolManager& rSymbolSetManager);
    ~SmSymbolManager();

    SmSymbolManager &   operator = (const SmSymbolManager& rSymbolSetManager);

    // symbol sets are for UI purpose only, thus we assemble them here
    std::set< OUString >      GetSymbolSetNames() const;
    const SymbolPtrVec_t    GetSymbolSet(  const OUString& rSymbolSetName );

    sal_uInt16                  GetSymbolCount() const  { return static_cast< sal_uInt16 >(m_aSymbols.size()); }
    const SymbolPtrVec_t    GetSymbols() const;
    bool                    AddOrReplaceSymbol( const SmSym & rSymbol, bool bForceChange = false );
    void                    RemoveSymbol( const OUString & rSymbolName );

    SmSym       *   GetSymbolByName(const OUString& rSymbolName);
    const SmSym *   GetSymbolByName(const OUString& rSymbolName) const
    {
        return ((SmSymbolManager *) this)->GetSymbolByName(rSymbolName);
    }

    bool        IsModified() const          { return m_bModified; }
    void        SetModified(bool bModify)   { m_bModified = bModify; }

    void        Load();
    void        Save();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
