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
#include <o3tl/typed_flags_set.hxx>

#include <vector>
#include <memory>

// flags related to the model
enum class SvTLEntryFlags
{
    NONE                = 0x0000,
    CHILDREN_ON_DEMAND  = 0x0001,
    DISABLE_DROP        = 0x0002,
    IN_USE              = 0x0004,
    // is set if RequestingChildren has not set any children
    NO_NODEBMP          = 0x0008,
    // entry had or has children
    HAD_CHILDREN        = 0x0010,
    SEMITRANSPARENT     = 0x8000,      // draw semi-transparent entry bitmaps
};
namespace o3tl
{
    template<> struct typed_flags<SvTLEntryFlags> : is_typed_flags<SvTLEntryFlags, 0x801f> {};
}


class SVT_DLLPUBLIC SvTreeListEntry
{
    friend class SvTreeList;
    friend class SvListView;
    friend class SvTreeListBox;

    typedef std::vector<std::unique_ptr<SvLBoxItem>> ItemsType;

    SvTreeListEntry*    pParent;
    SvTreeListEntries   m_Children;
    sal_uLong           nAbsPos;
    sal_uLong           nListPos;
    ItemsType           m_Items;
    bool                bIsMarked;
    void*               pUserData;
    SvTLEntryFlags      nEntryFlags;
    Color               maBackColor;

private:
    void ClearChildren();
    void SetListPositions();
    void InvalidateChildrensListPositions();

    void operator=(SvTreeListEntry const&) = delete;

public:
    static const size_t ITEM_NOT_FOUND = SAL_MAX_SIZE;

    SvTreeListEntry();
    SvTreeListEntry(const SvTreeListEntry& r);
    virtual ~SvTreeListEntry();

    bool HasChildren() const;
    bool HasChildListPos() const;
    sal_uLong GetChildListPos() const;

    SvTreeListEntries& GetChildEntries() { return m_Children; }
    const SvTreeListEntries& GetChildEntries() const { return m_Children; }

    void Clone(SvTreeListEntry* pSource);

    size_t ItemCount() const;

    // MAY ONLY BE CALLED IF THE ENTRY HAS NOT YET BEEN INSERTED INTO
    // THE MODEL, AS OTHERWISE NO VIEW-DEPENDENT DATA ARE ALLOCATED
    // FOR THE ITEM!
    void        AddItem(std::unique_ptr<SvLBoxItem> pItem);
    void ReplaceItem(std::unique_ptr<SvLBoxItem> pNewItem, size_t nPos);
    const SvLBoxItem& GetItem( size_t nPos ) const;
    SvLBoxItem& GetItem( size_t nPos );
    const SvLBoxItem* GetFirstItem( sal_uInt16 nId ) const;
    SvLBoxItem* GetFirstItem( sal_uInt16 nId );
    size_t GetPos( const SvLBoxItem* pItem ) const;
    void*       GetUserData() const { return pUserData;}
    void        SetUserData( void* pPtr );
    void        EnableChildrenOnDemand( bool bEnable=true );
    bool        HasChildrenOnDemand() const;
    bool        HasInUseEmphasis() const;

    SvTLEntryFlags GetFlags() const { return nEntryFlags;}
    void SetFlags( SvTLEntryFlags nFlags );

    void SetMarked( bool IsMarked ) { bIsMarked = IsMarked; }

    void SetBackColor( const Color& aColor ) { maBackColor = aColor; }
    const Color& GetBackColor() const { return maBackColor; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
