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
#ifndef INCLUDED_EDITENG_SCRIPTTYPEITEM_HXX
#define INCLUDED_EDITENG_SCRIPTTYPEITEM_HXX

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
    TYPEINFO_OVERRIDE();

    explicit SvxScriptTypeItem( sal_uInt16 nType = SCRIPTTYPE_LATIN );
    virtual SfxPoolItem* Clone( SfxItemPool *pPool = 0 ) const SAL_OVERRIDE;

};

class EDITENG_DLLPUBLIC SvxScriptSetItem : public SfxSetItem
{
public:
    TYPEINFO_OVERRIDE();

    SvxScriptSetItem( sal_uInt16 nSlotId, SfxItemPool& rPool );

    virtual SfxPoolItem* Clone( SfxItemPool *pPool = 0 ) const SAL_OVERRIDE;
    virtual SfxPoolItem* Create( SvStream &, sal_uInt16 nVersion ) const SAL_OVERRIDE;

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
