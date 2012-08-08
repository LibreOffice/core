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

#include <svl/intitem.hxx>
#include <svl/languageoptions.hxx>
#include <editeng/editengdllapi.h>

// class SvxScriptTypeItem ----------------------------------------------

/* [Description]

    This item describes the script type of the selected text and is only
    used for the user interface.
*/

EDITENG_DLLPUBLIC sal_uInt16 GetI18NScriptTypeOfLanguage( sal_uInt16 nLang );
sal_uInt16 GetItemScriptType( short nI18NType );
short  GetI18NScriptType( sal_uInt16 nItemType );

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

    SvxScriptSetItem( sal_uInt16 nSlotId, SfxItemPool& rPool );

    virtual SfxPoolItem* Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem* Create( SvStream &, sal_uInt16 nVersion ) const;

    static const SfxPoolItem* GetItemOfScriptSet( const SfxItemSet& rSet,
                                                    sal_uInt16 nWhich );
    inline const SfxPoolItem* GetItemOfScriptSet( sal_uInt16 _nWhich ) const
    { return SvxScriptSetItem::GetItemOfScriptSet( GetItemSet(), _nWhich ); }

    static const SfxPoolItem* GetItemOfScript( sal_uInt16 nSlotId, const SfxItemSet& rSet, sal_uInt16 nScript );

    const SfxPoolItem* GetItemOfScript( sal_uInt16 nScript ) const;

    void PutItemForScriptType( sal_uInt16 nScriptType, const SfxPoolItem& rItem );

    static void GetWhichIds( sal_uInt16 nSlotId, const SfxItemSet& rSet, sal_uInt16& rLatin, sal_uInt16& rAsian, sal_uInt16& rComplex);

    void GetWhichIds( sal_uInt16& rLatin, sal_uInt16& rAsian, sal_uInt16& rComplex) const;

    static void GetSlotIds( sal_uInt16 nSlotId, sal_uInt16& rLatin, sal_uInt16& rAsian,
                                            sal_uInt16& rComplex );
    inline void GetSlotIds( sal_uInt16& rLatin, sal_uInt16& rAsian,
                                sal_uInt16& rComplex ) const
    { GetSlotIds( Which(), rLatin, rAsian, rComplex ); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
