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
#ifndef _SVX_SCRIPTTYPEITEM_HXX
#define _SVX_SCRIPTTYPEITEM_HXX

// include ---------------------------------------------------------------

#include <svl/intitem.hxx>
#include <svl/languageoptions.hxx>
#include <editeng/editengdllapi.h>

// class SvxScriptTypeItem ----------------------------------------------

/* [Description]

    This item describes the script type of the selected text and is only
    used for the user interface.
*/

EDITENG_DLLPUBLIC USHORT GetI18NScriptTypeOfLanguage( USHORT nLang );
USHORT GetItemScriptType( short nI18NType );
short  GetI18NScriptType( USHORT nItemType );

class EDITENG_DLLPUBLIC SvxScriptTypeItem : public SfxUInt16Item
{
public:
    TYPEINFO();

    SvxScriptTypeItem( sal_uInt16 nType = SCRIPTTYPE_LATIN );
    virtual SfxPoolItem* Clone( SfxItemPool *pPool = 0 ) const;

};

class EDITENG_DLLPUBLIC SvxScriptSetItem : public SfxSetItem
{
public:
    TYPEINFO();

    SvxScriptSetItem( USHORT nSlotId, SfxItemPool& rPool );

    virtual SfxPoolItem* Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem* Create( SvStream &, USHORT nVersion ) const;

    static const SfxPoolItem* GetItemOfScriptSet( const SfxItemSet& rSet,
                                                    USHORT nWhich );
    inline const SfxPoolItem* GetItemOfScriptSet( USHORT _nWhich ) const
    { return SvxScriptSetItem::GetItemOfScriptSet( GetItemSet(), _nWhich ); }

    static const SfxPoolItem* GetItemOfScript( USHORT nSlotId, const SfxItemSet& rSet, USHORT nScript );

    const SfxPoolItem* GetItemOfScript( USHORT nScript ) const;

    void PutItemForScriptType( USHORT nScriptType, const SfxPoolItem& rItem );

    static void GetWhichIds( USHORT nSlotId, const SfxItemSet& rSet, USHORT& rLatin, USHORT& rAsian, USHORT& rComplex);

    void GetWhichIds( USHORT& rLatin, USHORT& rAsian, USHORT& rComplex) const;

    static void GetSlotIds( USHORT nSlotId, USHORT& rLatin, USHORT& rAsian,
                                            USHORT& rComplex );
    inline void GetSlotIds( USHORT& rLatin, USHORT& rAsian,
                                USHORT& rComplex ) const
    { GetSlotIds( Which(), rLatin, rAsian, rComplex ); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
