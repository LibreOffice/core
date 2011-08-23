/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <salhelper/simplereferenceobject.hxx>
#include <vcl/font.hxx>
#include <tools/list.hxx>
#include <tools/debug.hxx>
#include <tools/dynary.hxx>
#include <bf_svtools/lstner.hxx>
#include <bf_svtools/svarray.hxx>

#include "utility.hxx"
#include <smmod.hxx>
namespace binfilter {

#define SS_ATTR_ACCESS		0x80

#define SYMBOLSET_NONE	0xFFFF
#define SYMBOL_NONE		0xFFFF

class SmSymSetManager;

////////////////////////////////////////////////////////////////////////////////



/*N*/ inline const String GetUiSymbolName( const String &rExportSymbolName )
/*N*/ {
/*N*/     return SM_MOD1()->GetLocSymbolData().GetUiSymbolName( rExportSymbolName );
/*N*/ }



/*N*/ inline const String GetUiSymbolSetName( const String &rExportSymbolSetName )
/*N*/ {
/*N*/     return SM_MOD1()->GetLocSymbolData().GetUiSymbolSetName( rExportSymbolSetName );
/*N*/ }

////////////////////////////////////////////////////////////////////////////////

class SmSym
{
    friend class SmSymSetManager;


    SmFace				 Face;
    String				 Name;
    String				 aExportName;
    String				 aSetName;
    SmSym			   	*pHashNext;
    SmSymSetManager		*pSymSetManager;
    sal_Unicode			 Character;
    BYTE				 Attribut;
    BOOL				 bPredefined;
    BOOL                 bDocSymbol;

public:
    SmSym();
    SmSym(const SmSym& rSymbol);
    SmSym(const String& rName, const Font& rFont, sal_Unicode cChar,
          const String& rSet, BOOL bIsPredefined = FALSE);

    SmSym&	   	operator = (const SmSym& rSymbol);


    const Font& 	GetFace() const { return Face; }
    sal_Unicode		GetCharacter() const { return Character; }
    const String&	GetName() const { return Name; }

    void            SetFace( const Font& rFont )        { Face = rFont; }
    void            SetCharacter( sal_Unicode cChar )   { Character = cChar; }
    void            SetName( const String &rTxt )       { Name = rTxt; }

    BOOL			IsPredefined() const	{ return bPredefined; }
    const String &	GetSetName() const		{ return aSetName; }
    void            SetSetName( const String &rName )    { aSetName = rName; }
    const String &	GetExportName() const	{ return aExportName; }
    void			SetExportName( const String &rName )	{ aExportName = rName; }

    BOOL            IsDocSymbol() const         { return bDocSymbol; }
    void            SetDocSymbol( BOOL bVal )   { bDocSymbol = bVal; }
};

DECLARE_LIST(SmListSym, SmSym *)//STRIP008 DECLARE_LIST(SmListSym, SmSym *);
SV_DECL_PTRARR( SymbolArray, SmSym *, 32, 32 )//STRIP008 ;

/**************************************************************************/

class SmSymSet
{
    friend class SmSymSetManager;


    SmListSym		  	 SymbolList;
    String				 Name;
    SmSymSetManager		*pSymSetManager;

public:
    SmSymSet(const String& rName);


    const String&	GetName() const { return Name; }
    USHORT			GetCount() const { return (USHORT) SymbolList.Count(); }

    const SmSym&	GetSymbol(USHORT SymbolNo) const
    {
        DBG_ASSERT(SymbolList.GetObject(SymbolNo), "Symbol nicht vorhanden");
        return *SymbolList.GetObject(SymbolNo);
    }

    USHORT      AddSymbol(SmSym* pSymbol);
};

DECLARE_DYNARRAY(SmArraySymSet, SmSymSet *)

/**************************************************************************/



struct SmSymSetManager_Impl
{
    SmArraySymSet       SymbolSets;
    SmSymSetManager &   rSymSetMgr;
    SmSym**             HashEntries;
    USHORT              NoSymbolSets;
    USHORT              NoHashEntries;
    BOOL                Modified;

    SmSymSetManager_Impl( SmSymSetManager &rMgr, USHORT HashTableSize );

};


class SmSymSetManager : public SfxListener
{
    friend struct SmSymSetManager_Impl;

    SmSymSetManager_Impl *pImpl;


    UINT32		GetHashIndex(const String& rSymbolName);
    void        EnterHashTable(SmSym& rSymbol);
    void		EnterHashTable(SmSymSet& rSymbolSet);
    void		FillHashTable();

public:
    SmSymSetManager(USHORT HashTableSize = 137);



    USHORT		AddSymbolSet(SmSymSet* pSymbolSet);
    void		ChangeSymbolSet(SmSymSet* pSymbolSet);
    USHORT		GetSymbolSetPos(const String& rSymbolSetName) const;
        USHORT      GetSymbolSetCount() const { return pImpl->NoSymbolSets; }
    SmSymSet   *GetSymbolSet(USHORT SymbolSetNo) const 
    { 
        return pImpl->SymbolSets.Get(SymbolSetNo);
    }

    SmSym	    *	GetSymbolByName(const String& rSymbolName);
    const SmSym *	GetSymbolByName(const String& rSymbolName) const
    {
        return ((SmSymSetManager *) this)->GetSymbolByName(rSymbolName);
    }

        void            AddReplaceSymbol( const SmSym & rSymbol );
        USHORT          GetSymbolCount() const;
        const SmSym *   GetSymbolByPos( USHORT nPos ) const;

    BOOL 		IsModified() const { return pImpl->Modified; }
    void		SetModified(BOOL Modify) { pImpl->Modified = Modify; }

        void            Load();
};

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
