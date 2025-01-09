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
#ifndef INCLUDED_SVL_MACITEM_HXX
#define INCLUDED_SVL_MACITEM_HXX

// class SvxMacroItem ----------------------------------------------------

#include <rtl/ustring.hxx>
#include <svl/svldllapi.h>
#include <svl/poolitem.hxx>
#include <map>
#include <utility>

class SvStream;
enum class SvMacroItemId : sal_uInt16;

inline constexpr OUString SVX_MACRO_LANGUAGE_JAVASCRIPT  = u"JavaScript"_ustr;
inline constexpr OUString SVX_MACRO_LANGUAGE_STARBASIC = u"StarBasic"_ustr;
inline constexpr OUString SVX_MACRO_LANGUAGE_SF = u"Script"_ustr;

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

    SvxMacro( OUString aMacName, const OUString &rLanguage);

    SvxMacro( OUString aMacName, OUString aLibName,
                ScriptType eType); //  = STARBASIC removes

    const OUString &GetLibName() const        { return aLibName; }
    const OUString &GetMacName() const        { return aMacName; }
    const OUString &GetLanguage()const;

    ScriptType GetScriptType() const        { return eType; }

    bool HasMacro() const { return !aMacName.isEmpty(); }
};

inline SvxMacro::SvxMacro( OUString _aMacName, OUString _aLibName,
                            ScriptType eTyp )
    : aMacName(std::move( _aMacName )), aLibName(std::move( _aLibName )), eType( eTyp )
{}

// Macro Table, destroys the pointers in the DTor!
typedef std::map<SvMacroItemId, SvxMacro> SvxMacroTable;

#define SVX_MACROTBL_VERSION31  0
#define SVX_MACROTBL_VERSION40  1

class SVL_DLLPUBLIC SvxMacroTableDtor
{
private:
    SvxMacroTable aSvxMacroTable;
public:
    SvxMacroTableDtor() {}
    SvxMacroTableDtor( const SvxMacroTableDtor &rCpy ) : aSvxMacroTable(rCpy.aSvxMacroTable) { }

    SvxMacroTableDtor& operator=( const SvxMacroTableDtor &rCpy );
    bool operator==( const SvxMacroTableDtor& rOther ) const;

    void        Read( SvStream & );
    SvStream&   Write( SvStream & ) const;

    bool empty() const { return aSvxMacroTable.empty(); }

    // returns NULL if no entry exists, or a pointer to the internal value
    const SvxMacro* Get(SvMacroItemId nEvent) const;
    // returns NULL if no entry exists, or a pointer to the internal value
    SvxMacro* Get(SvMacroItemId nEvent);
    // return true if the key exists
    bool IsKeyValid(SvMacroItemId nEvent) const;
    // This stores a copy of the rMacro parameter
    SvxMacro& Insert(SvMacroItemId nEvent, const SvxMacro& rMacro);
    // If the entry exists, remove it from the map and release it's storage
    void Erase(SvMacroItemId nEvent);
};


/*
This item describes a Macro table.
*/

class SVL_DLLPUBLIC SvxMacroItem final : public SfxPoolItem
{
public:
    DECLARE_ITEM_TYPE_FUNCTION(SvxMacroItem)
    explicit inline SvxMacroItem ( const sal_uInt16 nId );

    // "pure virtual methods" of SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText,
                                  const IntlWrapper& ) const override;
    virtual SvxMacroItem* Clone( SfxItemPool *pPool = nullptr ) const override;

    const SvxMacroTableDtor& GetMacroTable() const { return aMacroTable;}
    void SetMacroTable( const SvxMacroTableDtor& rTbl ) { aMacroTable = rTbl; }

    inline const SvxMacro& GetMacro( SvMacroItemId nEvent ) const;
    inline bool HasMacro( SvMacroItemId nEvent ) const;
           void SetMacro( SvMacroItemId nEvent, const SvxMacro& );

private:
    SvxMacroTableDtor aMacroTable;

    SvxMacroItem( const SvxMacroItem& ) = default;
};

inline SvxMacroItem::SvxMacroItem( const sal_uInt16 nId )
    : SfxPoolItem( nId )
{}

inline bool SvxMacroItem::HasMacro( SvMacroItemId nEvent ) const
{
    return aMacroTable.IsKeyValid( nEvent );
}
inline const SvxMacro& SvxMacroItem::GetMacro( SvMacroItemId nEvent ) const
{
    return *(aMacroTable.Get(nEvent));
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
