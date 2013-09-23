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
#ifndef _SFXMACITEM_HXX
#define _SFXMACITEM_HXX

// class SvxMacroItem ----------------------------------------------------

#include <rtl/ustring.hxx>
#include <svl/svldllapi.h>
#include <svl/poolitem.hxx>
#include <tools/rtti.hxx>
#include <tools/debug.hxx>
#include <map>

class SvStream;

#define SVX_MACRO_LANGUAGE_JAVASCRIPT "JavaScript"
#define SVX_MACRO_LANGUAGE_STARBASIC "StarBasic"
#define SVX_MACRO_LANGUAGE_SF "Script"

DBG_NAMEEX(SvxMacroItem)

// -----------------------------------------------------------------------

//Ein Macro

enum ScriptType
{
    STARBASIC,
    JAVASCRIPT,
    EXTENDED_STYPE
};

class SVL_DLLPUBLIC SvxMacro
{
    OUString aMacName;
    OUString aLibName;
    ScriptType eType;

public:

    SvxMacro( const OUString &rMacName, const OUString &rLanguage);

    SvxMacro( const OUString &rMacName, const OUString &rLibName,
                ScriptType eType); //  = STARBASIC entfernt

    const OUString &GetLibName() const        { return aLibName; }
    const OUString &GetMacName() const        { return aMacName; }
    OUString GetLanguage()const;

    ScriptType GetScriptType() const        { return eType; }

    bool HasMacro() const { return !aMacName.isEmpty(); }

    SvxMacro& operator=( const SvxMacro& rBase );
};

inline SvxMacro::SvxMacro( const OUString &rMacName, const OUString &rLibName,
                            ScriptType eTyp )
    : aMacName( rMacName ), aLibName( rLibName ), eType( eTyp )
{}

//Macro Table, zerstoert die Pointer im DTor!

typedef std::map<sal_uInt16, SvxMacro> SvxMacroTable;

#define SVX_MACROTBL_VERSION31  0
#define SVX_MACROTBL_VERSION40  1

#define SVX_MACROTBL_AKTVERSION SVX_MACROTBL_VERSION40

class SVL_DLLPUBLIC SvxMacroTableDtor
{
private:
    SvxMacroTable aSvxMacroTable;
public:
    inline SvxMacroTableDtor() {}
    inline SvxMacroTableDtor( const SvxMacroTableDtor &rCpy ) : aSvxMacroTable(rCpy.aSvxMacroTable) { }

    SvxMacroTableDtor& operator=( const SvxMacroTableDtor &rCpy );
    int operator==( const SvxMacroTableDtor& rOther ) const;

    // loescht alle Eintraege
    void clear() { aSvxMacroTable.clear(); }

    SvStream&   Read( SvStream &, sal_uInt16 nVersion = SVX_MACROTBL_AKTVERSION );
    SvStream&   Write( SvStream & ) const;

    sal_uInt16 GetVersion() const       { return SVX_MACROTBL_AKTVERSION; }

    SvxMacroTable::iterator begin() { return aSvxMacroTable.begin(); }
    SvxMacroTable::const_iterator begin() const { return aSvxMacroTable.begin(); }
    SvxMacroTable::iterator end() { return aSvxMacroTable.end(); }
    SvxMacroTable::const_iterator end () const { return aSvxMacroTable.end(); }
    SvxMacroTable::size_type size() const { return aSvxMacroTable.size(); }
    bool empty() const { return aSvxMacroTable.empty(); }

    // returns NULL if no entry exists, or a pointer to the internal value
    const SvxMacro* Get(sal_uInt16 nEvent) const;
    // returns NULL if no entry exists, or a pointer to the internal value
    SvxMacro* Get(sal_uInt16 nEvent);
    // return true if the key exists
    bool IsKeyValid(sal_uInt16 nEvent) const;
    // This stores a copy of the rMacro parameter
    SvxMacro& Insert(sal_uInt16 nEvent, const SvxMacro& rMacro);
    // If the entry exists, remove it from the map and release it's storage
    sal_Bool Erase(sal_uInt16 nEvent);
};


/*
[Beschreibung]
Dieses Item beschreibt eine Makro-Tabelle.
*/

class SVL_DLLPUBLIC SvxMacroItem: public SfxPoolItem
{
public:
    TYPEINFO();

    explicit inline SvxMacroItem ( const sal_uInt16 nId );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper * = 0 ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, sal_uInt16) const;
    virtual SvStream&       Store(SvStream &, sal_uInt16 nItemVersion ) const;
    virtual sal_uInt16          GetVersion( sal_uInt16 nFileFormatVersion ) const;

    inline const SvxMacroTableDtor& GetMacroTable() const { return aMacroTable;}
    inline void SetMacroTable( const SvxMacroTableDtor& rTbl ) { aMacroTable = rTbl; }

    inline const SvxMacro& GetMacro( sal_uInt16 nEvent ) const;
    inline sal_Bool HasMacro( sal_uInt16 nEvent ) const;
           void SetMacro( sal_uInt16 nEvent, const SvxMacro& );
    inline sal_Bool DelMacro( sal_uInt16 nEvent );

private:
    SvxMacroTableDtor aMacroTable;

    inline SvxMacroItem( const SvxMacroItem& );
    SvxMacroItem &operator=( const SvxMacroItem & );
};

inline SvxMacroItem::SvxMacroItem( const sal_uInt16 nId )
    : SfxPoolItem( nId )
{}
inline SvxMacroItem::SvxMacroItem( const SvxMacroItem &rCpy )
    : SfxPoolItem( rCpy ),
    aMacroTable( rCpy.GetMacroTable() )
{}

inline sal_Bool SvxMacroItem::HasMacro( sal_uInt16 nEvent ) const
{
    return aMacroTable.IsKeyValid( nEvent );
}
inline const SvxMacro& SvxMacroItem::GetMacro( sal_uInt16 nEvent ) const
{
    return *(aMacroTable.Get(nEvent));
}
inline sal_Bool SvxMacroItem::DelMacro( sal_uInt16 nEvent )
{
    return aMacroTable.Erase(nEvent);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
