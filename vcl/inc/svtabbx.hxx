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

#pragma once

#include <config_options.h>
#include <vcl/dllapi.h>
#include <vcl/toolkit/treelistbox.hxx>

#include <tools/debug.hxx>

#include <vector>

enum class SvTabListBoxRole
{
    Unknown,
    Tree,       // hierarchical, single-column
    TreeGrid,   // hierarchical, multi-column
    ListBox,    // flat, single-column
    Grid        // flat, multi-column
};

class UNLESS_MERGELIBS_MORE(VCL_DLLPUBLIC) SvTabListBox : public SvTreeListBox
{
private:
    std::vector<SvLBoxTab>      mvTabList;
    OUString                    aCurEntry;
    SvTabListBoxRole            m_eRole;

    Link<SvTreeListEntry*, bool> m_aEditingEntryHdl;
    Link<const EntryItemText&, bool> m_aEditedEntryHdl;

protected:
    static std::u16string_view  GetToken( std::u16string_view sStr, sal_Int32 &nIndex );

    virtual void                SetTabs() override;
    virtual void InitEntry(SvTreeListEntry& rEntry, const OUString&, const Image&,
                           const Image&) override;

    OUString                    GetTabEntryText( sal_uInt32 nPos, sal_uInt16 nCol ) const;
    SvTreeListEntry*            GetEntryOnPos( sal_uInt32 _nEntryPos ) const;
    SvTreeListEntry*            GetChildOnPos( SvTreeListEntry* _pParent, sal_uInt32 _nEntryPos, sal_uInt32& _rPos ) const;

public:
    SvTabListBox( vcl::Window* pParent, WinBits );
    virtual ~SvTabListBox() override;
    virtual void dispose() override;
    void SetTabs(const std::vector<tools::Long>& rTabPositions);
    void SetTabWidth(sal_uInt16 nTab, tools::Long tabWidth);
    using SvTreeListBox::GetTab;
    tools::Long            GetLogicTab( sal_uInt16 nTab );

    void SetEditingEntryHdl(const Link<SvTreeListEntry*, bool>& rLink)
    {
        m_aEditingEntryHdl = rLink;
    }

    void SetEditedEntryHdl(const Link<const EntryItemText&, bool>& rLink)
    {
        m_aEditedEntryHdl = rLink;
    }

    virtual bool EditingEntry(SvTreeListEntry* pEntry) override
    {
        return m_aEditingEntryHdl.Call(pEntry);
    }

    virtual bool EditedEntry(SvTreeListEntry& rEntry, const SvLBoxItem& rItem,
                             const OUString& rNewText) override
    {
        return m_aEditedEntryHdl.Call(EntryItemText(rEntry, rItem, rNewText));
    }

    // the default NotifyStartDrag is weird to me, and defaults to enabling all
    // possibilities when drag starts, while restricting it to some subset of
    // the configured drag drop mode would make more sense to me, but I'm not
    // going to change the baseclass
    virtual DragDropMode NotifyStartDrag() override { return GetDragDropMode(); }

    virtual SvTreeListEntry*    InsertEntry( const OUString& rText, SvTreeListEntry* pParent = nullptr,
                                         bool bChildrenOnDemand = false,
                                         sal_uInt32 nPos=TREELIST_APPEND, OUString* pUserData = nullptr ) override;

    virtual SvTreeListEntry* InsertEntryToColumn( const OUString&, SvTreeListEntry* pParent,
                                 sal_uInt32 nPos, sal_uInt16 nCol, OUString* pUserData = nullptr );

    virtual OUString GetEntryText( SvTreeListEntry* pEntry ) const override;
    static OUString  GetEntryText( const SvTreeListEntry*, sal_uInt16 nCol );
    OUString         GetEntryText( sal_uInt32 nPos, sal_uInt16 nCol = 0xffff ) const;
    using SvTreeListBox::SetEntryText;
    OUString         GetCellText( sal_uInt32 nPos, sal_uInt16 nCol ) const;

    void             SetTabAlignCenter(sal_uInt16 nTab);
    void             SetTabEditable( sal_uInt16 nTab, bool bEditable );
    // Note that to make a tab visible, the width must also be set to a non-zero value
    void             SetTabVisible( sal_uInt16 nTab, bool bVisible );
    bool             GetTabVisible( sal_uInt16 nTab );

    void             SetRole(SvTabListBoxRole e) { m_eRole = e; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
