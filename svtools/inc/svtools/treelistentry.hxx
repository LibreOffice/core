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

#ifndef __SVTOOLS_TREELISTENTRY_HXX__
#define __SVTOOLS_TREELISTENTRY_HXX__

#include "svtdllapi.h"
#include "tools/solar.h"

#include <vector>

class SvTreeEntryList;
class SvLBoxItem;

class SVT_DLLPUBLIC SvListEntry
{
friend class SvTreeList;
friend class SvListView;

private:
    SvListEntry*        pParent;
    SvTreeEntryList*    pChildren;
    sal_uLong           nAbsPos;
    sal_uLong           nListPos;

    void                SetListPositions();
    void                InvalidateChildrensListPositions()
    {
        nListPos |= 0x80000000;
    }

public:
                        SvListEntry();
                        SvListEntry( const SvListEntry& );
    virtual             ~SvListEntry();
    sal_Bool            HasChildren() { return (sal_Bool)(pChildren!=0); }
    sal_Bool            HasChildListPos() const
    {
        if( pParent && !(pParent->nListPos & 0x80000000) )
            return sal_True;
        else return sal_False;
    }

    sal_uLong           GetChildListPos() const
    {
        if( pParent && (pParent->nListPos & 0x80000000) )
            pParent->SetListPositions();
        return ( nListPos & 0x7fffffff );
    }

    virtual void        Clone( SvListEntry* pSource );
};


// Flags, die am Model haengen
#define SV_ENTRYFLAG_CHILDREN_ON_DEMAND   0x0001
#define SV_ENTRYFLAG_DISABLE_DROP       0x0002
#define SV_ENTRYFLAG_IN_USE             0x0004
// wird gesetzt, wenn RequestingChildren keine Children gestzt hat
#define SV_ENTRYFLAG_NO_NODEBMP         0x0008
// Eintrag hatte oder hat Kinder
#define SV_ENTRYFLAG_HAD_CHILDREN       0x0010

#define SV_ENTRYFLAG_USER_FLAGS         0xF000
#define SV_ENTRYFLAG_SEMITRANSPARENT    0x8000      // draw semi-transparent entry bitmaps

class SVT_DLLPUBLIC SvLBoxEntry : public SvListEntry
{
    friend class SvTreeListBox;

    std::vector<SvLBoxItem*> aItems;
    void*            pUserData;
    sal_uInt16       nEntryFlags;
    SVT_DLLPRIVATE void         DeleteItems_Impl();
public:

                SvLBoxEntry();
    virtual     ~SvLBoxEntry();

    sal_uInt16      ItemCount() const { return (sal_uInt16)aItems.size(); }
    // DARF NUR GERUFEN WERDEN, WENN DER EINTRAG NOCH NICHT IM MODEL
    // EINGEFUEGT IST, DA SONST FUER DAS ITEM KEINE VIEW-ABHAENGIGEN
    // DATEN ALLOZIERT WERDEN!
    void        AddItem( SvLBoxItem* pItem );
    void        ReplaceItem( SvLBoxItem* pNewItem, sal_uInt16 nPos );
    SvLBoxItem* GetItem( sal_uInt16 nPos ) const { return aItems[nPos]; }
    SvLBoxItem* GetFirstItem( sal_uInt16 nId );
    sal_uInt16 GetPos( SvLBoxItem* pItem ) const;
    void*       GetUserData() const { return pUserData; }
    void        SetUserData( void* pPtr ) { pUserData = pPtr; }
    virtual void Clone( SvListEntry* pSource );
    void        EnableChildrenOnDemand( sal_Bool bEnable=sal_True );
    sal_Bool        HasChildrenOnDemand() const { return (sal_Bool)((nEntryFlags & SV_ENTRYFLAG_CHILDREN_ON_DEMAND)!=0); }
    sal_Bool        HasInUseEmphasis() const    { return (sal_Bool)((nEntryFlags & SV_ENTRYFLAG_IN_USE)!=0); }

    sal_uInt16      GetFlags() const { return nEntryFlags; }
    void        SetFlags( sal_uInt16 nFlags ) { nEntryFlags = nFlags; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
