/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: symbol.hxx,v $
 * $Revision: 1.17 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef SYMBOL_HXX
#define SYMBOL_HXX

#include <vos/refernce.hxx>
#ifndef _FONT_HXX //autogen
#include <vcl/font.hxx>
#endif
#include <tools/list.hxx>
#include <tools/debug.hxx>
#include <tools/dynary.hxx>
#include <svl/lstner.hxx>
#include <svl/svarray.hxx>
#include "utility.hxx"
#include <smmod.hxx>

#define SS_ATTR_ACCESS      0x80

#define SYMBOLSET_NONE  0xFFFF
#define SYMBOL_NONE     0xFFFF

class SmSymSetManager;

////////////////////////////////////////////////////////////////////////////////

inline const String GetExportSymbolName( const String &rUiSymbolName )
{
    return SM_MOD1()->GetLocSymbolData().GetExportSymbolName( rUiSymbolName );
}


inline const String GetUiSymbolName( const String &rExportSymbolName )
{
    return SM_MOD1()->GetLocSymbolData().GetUiSymbolName( rExportSymbolName );
}

inline const String GetExportSymbolSetName( const String &rUiSymbolSetName )
{
    return SM_MOD1()->GetLocSymbolData().GetExportSymbolSetName( rUiSymbolSetName );
}


inline const String GetUiSymbolSetName( const String &rExportSymbolSetName )
{
    return SM_MOD1()->GetLocSymbolData().GetUiSymbolSetName( rExportSymbolSetName );
}

////////////////////////////////////////////////////////////////////////////////

class SmSym
{
    friend class SmSymSetManager;

    SmFace               Face;
    String               Name;
    String               aExportName;
    String               aSetName;
    SmSym               *pHashNext;
    SmSymSetManager     *pSymSetManager;
    sal_Unicode          Character;
    BYTE                 Attribut;
    BOOL                 bPredefined;
    BOOL                 bDocSymbol;

public:
    SmSym();
    SmSym(const SmSym& rSymbol);
    SmSym(const String& rName, const Font& rFont, sal_Unicode cChar,
          const String& rSet, BOOL bIsPredefined = FALSE);

    SmSym&      operator = (const SmSym& rSymbol);

    const Font&     GetFace() const { return Face; }
    sal_Unicode     GetCharacter() const { return Character; }
    const String&   GetName() const { return Name; }

    void            SetFace( const Font& rFont )        { Face = rFont; }
    void            SetCharacter( sal_Unicode cChar )   { Character = cChar; }
    void            SetName( const String &rTxt )       { Name = rTxt; }

    BOOL            IsPredefined() const    { return bPredefined; }
    const String &  GetSetName() const      { return aSetName; }
    void            SetSetName( const String &rName )    { aSetName = rName; }
    const String &  GetExportName() const   { return aExportName; }
    void            SetExportName( const String &rName )    { aExportName = rName; }

    BOOL            IsDocSymbol() const         { return bDocSymbol; }
    void            SetDocSymbol( BOOL bVal )   { bDocSymbol = bVal; }
};

DECLARE_LIST(SmListSym, SmSym *)
SV_DECL_PTRARR( SymbolArray, SmSym *, 32, 32 )

/**************************************************************************/

class SmSymSet
{
    friend class SmSymSetManager;

    SmListSym            SymbolList;
    String               Name;
    SmSymSetManager     *pSymSetManager;

public:
    SmSymSet();
    SmSymSet(const SmSymSet& rSymbolSet);
    SmSymSet(const String& rName);
    ~SmSymSet();

    SmSymSet&   operator = (const SmSymSet& rSymbolSet);

    const String&   GetName() const { return Name; }
    USHORT          GetCount() const { return (USHORT) SymbolList.Count(); }

    const SmSym&    GetSymbol(USHORT SymbolNo) const
    {
        DBG_ASSERT(SymbolList.GetObject(SymbolNo), "Symbol nicht vorhanden");
        return *SymbolList.GetObject(SymbolNo);
    }

    USHORT      AddSymbol(SmSym* pSymbol);
    void        DeleteSymbol(USHORT SymbolNo);
    SmSym *     RemoveSymbol(USHORT SymbolNo);
    USHORT      GetSymbolPos(const String& rName);
};

DECLARE_DYNARRAY(SmArraySymSet, SmSymSet *)

/**************************************************************************/

class SmSymbolDialog;


struct SmSymSetManager_Impl
{
    SmArraySymSet       SymbolSets;
    SmSymSetManager &   rSymSetMgr;
    SmSym**             HashEntries;
    USHORT              NoSymbolSets;
    USHORT              NoHashEntries;
    BOOL                Modified;

    SmSymSetManager_Impl( SmSymSetManager &rMgr, USHORT HashTableSize );
    ~SmSymSetManager_Impl();

    SmSymSetManager_Impl & operator = ( const SmSymSetManager_Impl &rImpl );
};


class SmSymSetManager : public SfxListener
{
    friend struct SmSymSetManager_Impl;

    SmSymSetManager_Impl *pImpl;

    virtual void SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType,
                        const SfxHint& rHint, const TypeId& rHintType);

    UINT32      GetHashIndex(const String& rSymbolName);
    void        EnterHashTable(SmSym& rSymbol);
    void        EnterHashTable(SmSymSet& rSymbolSet);
    void        FillHashTable();
    void        Init();
    void        Exit();

public:
    SmSymSetManager(USHORT HashTableSize = 137);
    SmSymSetManager(const SmSymSetManager& rSymbolSetManager);
    ~SmSymSetManager();

    SmSymSetManager&   operator = (const SmSymSetManager& rSymbolSetManager);

    void        GetSymbols( std::vector< SmSym > &rSymbols ) const;


    USHORT      AddSymbolSet(SmSymSet* pSymbolSet);
    void        ChangeSymbolSet(SmSymSet* pSymbolSet);
    void        DeleteSymbolSet(USHORT SymbolSetNo);
    USHORT      GetSymbolSetPos(const String& rSymbolSetName) const;
    USHORT      GetSymbolSetCount() const { return pImpl->NoSymbolSets; }
    SmSymSet   *GetSymbolSet(USHORT SymbolSetNo) const
    {
        return pImpl->SymbolSets.Get(SymbolSetNo);
    }

    SmSym       *   GetSymbolByName(const String& rSymbolName);
    const SmSym *   GetSymbolByName(const String& rSymbolName) const
    {
        return ((SmSymSetManager *) this)->GetSymbolByName(rSymbolName);
    }

    void            AddReplaceSymbol( const SmSym & rSymbol );
    USHORT          GetSymbolCount() const;
    const SmSym *   GetSymbolByPos( USHORT nPos ) const;

    BOOL        IsModified() const { return pImpl->Modified; }
    void        SetModified(BOOL Modify) { pImpl->Modified = Modify; }

    void        Load();
    void        Save();
};

#endif

