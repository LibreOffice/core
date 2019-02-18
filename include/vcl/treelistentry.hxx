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

#ifndef INCLUDED_VCL_TREELISTENTRY_HXX
#define INCLUDED_VCL_TREELISTENTRY_HXX

#include <vcl/dllapi.h>
#include <tools/solar.h>
#include <tools/color.hxx>
#include <vcl/treelistbox.hxx>
#include <vcl/treelistentries.hxx>
#include <o3tl/typed_flags_set.hxx>

#include <boost/optional.hpp>
#include <vector>
#include <memory>

// flags related to the model
enum class SvTLEntryFlags
{
    NONE                = 0x0000,
    CHILDREN_ON_DEMAND  = 0x0001,
    DISABLE_DROP        = 0x0002,
    // is set if RequestingChildren has not set any children
    NO_NODEBMP          = 0x0004,
    // entry had or has children
    HAD_CHILDREN        = 0x0010,
    SEMITRANSPARENT     = 0x8000,      // draw semi-transparent entry bitmaps
};
namespace o3tl
{
    template<> struct typed_flags<SvTLEntryFlags> : is_typed_flags<SvTLEntryFlags, 0x8017> {};
}

class VCL_DLLPUBLIC SvTreeListEntry
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
    void*               pUserData;
    SvTLEntryFlags      nEntryFlags;
    Color               maBackColor;
    boost::optional<Color> mxTextColor;

private:
    void ClearChildren();
    void SetListPositions();
    void InvalidateChildrensListPositions();

    SvTreeListEntry(const SvTreeListEntry& r) = delete;
    void operator=(SvTreeListEntry const&) = delete;

public:
    static const size_t ITEM_NOT_FOUND = SAL_MAX_SIZE;

    SvTreeListEntry();
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
    const SvLBoxItem* GetFirstItem(SvLBoxItemType eType) const;
    SvLBoxItem* GetFirstItem(SvLBoxItemType eType);
    size_t GetPos( const SvLBoxItem* pItem ) const;
    void*       GetUserData() const { return pUserData;}
    void        SetUserData( void* pPtr );
    void        EnableChildrenOnDemand( bool bEnable=true );
    bool        HasChildrenOnDemand() const;

    SvTLEntryFlags GetFlags() const { return nEntryFlags;}
    void SetFlags( SvTLEntryFlags nFlags );

    void SetBackColor( const Color& rColor ) { maBackColor = rColor; }
    const Color& GetBackColor() const { return maBackColor; }

    void SetTextColor( boost::optional<Color> xColor ) { mxTextColor = xColor; }
    boost::optional<Color> const & GetTextColor() const { return mxTextColor; }

    SvTreeListEntry* GetParent() const { return pParent; }

    SvTreeListEntry* NextSibling() const;
    SvTreeListEntry* PrevSibling() const;
    SvTreeListEntry* LastSibling() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
