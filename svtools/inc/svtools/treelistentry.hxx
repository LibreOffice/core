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
#include <boost/ptr_container/ptr_vector.hpp>

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

class SvTreeEntryList;
class SvLBoxItem;
class SvTreeListEntry;

typedef boost::ptr_vector<SvTreeListEntry> SvTreeListEntries;

class SVT_DLLPUBLIC SvTreeListEntry
{
    friend class SvTreeList;
    friend class SvListView;
    friend class SvTreeListBox;

    SvTreeListEntry*    pParent;
    SvTreeListEntries   maChildren;
    sal_uLong           nAbsPos;
    sal_uLong           nListPos;
    std::vector<SvLBoxItem*> aItems;
    void*            pUserData;
    sal_uInt16       nEntryFlags;

private:
    void ClearChildren();
    void SetListPositions();
    void InvalidateChildrensListPositions();
    void DeleteItems_Impl();

public:
    SvTreeListEntry();
    SvTreeListEntry(const SvTreeListEntry& r);
    virtual ~SvTreeListEntry();

    bool HasChildren() const;
    bool HasChildListPos() const;
    sal_uLong GetChildListPos() const;

    void Clone(SvTreeListEntry* pSource);

    sal_uInt16 ItemCount() const;

    // DARF NUR GERUFEN WERDEN, WENN DER EINTRAG NOCH NICHT IM MODEL
    // EINGEFUEGT IST, DA SONST FUER DAS ITEM KEINE VIEW-ABHAENGIGEN
    // DATEN ALLOZIERT WERDEN!
    void        AddItem( SvLBoxItem* pItem );
    void        ReplaceItem( SvLBoxItem* pNewItem, sal_uInt16 nPos );
    SvLBoxItem* GetItem( sal_uInt16 nPos ) const;
    SvLBoxItem* GetFirstItem( sal_uInt16 nId ) const;
    sal_uInt16 GetPos( SvLBoxItem* pItem ) const;
    void*       GetUserData() const;
    void        SetUserData( void* pPtr );
    void        EnableChildrenOnDemand( bool bEnable=true );
    bool        HasChildrenOnDemand() const;
    bool        HasInUseEmphasis() const;

    sal_uInt16 GetFlags() const;
    void SetFlags( sal_uInt16 nFlags );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
