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

#ifndef INCLUDED_SVTOOLS_TREELISTENTRY_HXX
#define INCLUDED_SVTOOLS_TREELISTENTRY_HXX

#include <svtools/svtdllapi.h>
#include <tools/solar.h>
#include <svtools/treelistbox.hxx>
#include <svtools/treelistentries.hxx>

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


class SVT_DLLPUBLIC SvTreeListEntry
{
    friend class SvTreeList;
    friend class SvListView;
    friend class SvTreeListBox;

    typedef boost::ptr_vector<SvLBoxItem> ItemsType;

    SvTreeListEntry*    pParent;
    SvTreeListEntries   maChildren;
    sal_uLong           nAbsPos;
    sal_uLong           nListPos;
    ItemsType           maItems;
    bool                bIsMarked;
    void*               pUserData;
    sal_uInt16          nEntryFlags;

private:
    void ClearChildren();
    void SetListPositions();
    void InvalidateChildrensListPositions();

public:
    static const size_t ITEM_NOT_FOUND = SAL_MAX_SIZE;

    SvTreeListEntry();
    SvTreeListEntry(const SvTreeListEntry& r);
    virtual ~SvTreeListEntry();

    bool HasChildren() const;
    bool HasChildListPos() const;
    sal_uLong GetChildListPos() const;

    SvTreeListEntries& GetChildEntries();
    const SvTreeListEntries& GetChildEntries() const;

    void Clone(SvTreeListEntry* pSource);

    size_t ItemCount() const;

    // DARF NUR GERUFEN WERDEN, WENN DER EINTRAG NOCH NICHT IM MODEL
    // EINGEFUEGT IST, DA SONST FUER DAS ITEM KEINE VIEW-ABHAENGIGEN
    // DATEN ALLOZIERT WERDEN!
    void        AddItem( SvLBoxItem* pItem );
    void ReplaceItem( SvLBoxItem* pNewItem, size_t nPos );
    const SvLBoxItem* GetItem( size_t nPos ) const;
    SvLBoxItem* GetItem( size_t nPos );
    const SvLBoxItem* GetFirstItem( sal_uInt16 nId ) const;
    SvLBoxItem* GetFirstItem( sal_uInt16 nId );
    size_t GetPos( const SvLBoxItem* pItem ) const;
    void*       GetUserData() const;
    void        SetUserData( void* pPtr );
    void        EnableChildrenOnDemand( bool bEnable=true );
    bool        HasChildrenOnDemand() const;
    bool        HasInUseEmphasis() const;

    sal_uInt16 GetFlags() const;
    void SetFlags( sal_uInt16 nFlags );

    bool GetIsMarked() const { return bIsMarked; }
    void SetMarked( bool IsMarked ) { bIsMarked = IsMarked; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
