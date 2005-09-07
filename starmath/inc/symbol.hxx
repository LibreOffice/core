/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: symbol.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:00:32 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef SYMBOL_HXX
#define SYMBOL_HXX

#ifndef _VOS_REFERNCE_HXX_
#include <vos/refernce.hxx>
#endif
#ifndef _FONT_HXX //autogen
#include <vcl/font.hxx>
#endif
#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _DYNARY_HXX
#include <tools/dynary.hxx>
#endif
#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif
#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif

#ifndef UTILITY_HXX
#include "utility.hxx"
#endif
#ifndef _SMMOD_HXX
#include <smmod.hxx>
#endif

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

DECLARE_LIST(SmListSym, SmSym *);
SV_DECL_PTRARR( SymbolArray, SmSym *, 32, 32 );

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
    void            SetName(String& rName);
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

        void            Load();
    void            Save();
};

#endif

