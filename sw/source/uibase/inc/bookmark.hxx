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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_BOOKMARK_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_BOOKMARK_HXX

#include <sfx2/basedlgs.hxx>
#include <vcl/weld.hxx>
#include <IMark.hxx>
#include "condedit.hxx"

class SwWrtShell;
class SfxRequest;

class BookmarkTable
{
    std::unique_ptr<weld::TreeView> m_xControl;
    std::unique_ptr<weld::TreeIter> GetRowByBookmarkName(const OUString& sName);
public:
    BookmarkTable(std::unique_ptr<weld::TreeView> xControl);
    void                InsertBookmark(sw::mark::IMark* pMark);
    void                SelectByName(const OUString& sName);
    sw::mark::IMark*    GetBookmarkByName(const OUString& sName);
    OUString            GetNameProposal() const;

    void                unselect_all() { m_xControl->unselect_all(); }
    bool                has_focus() const { return m_xControl->has_focus(); }
    std::unique_ptr<weld::TreeIter> get_selected() const;
    void                clear() { m_xControl->clear(); }
    void                select(const weld::TreeIter& rIter) { m_xControl->select(rIter); }
    void                remove_selection() { m_xControl->remove_selection(); }
    OUString            get_id(const weld::TreeIter& rIter) const { return m_xControl->get_id(rIter); }
    void set_sort_indicator(TriState eState, int nColumn = -1) { m_xControl->set_sort_indicator(eState, nColumn); }
    void selected_foreach(const std::function<bool(weld::TreeIter&)>& func) { m_xControl->selected_foreach(func); }

    void connect_changed(const Link<weld::TreeView&, void>& rLink) { m_xControl->connect_changed(rLink); }
    void connect_row_activated(const Link<weld::TreeView&, bool>& rLink) { m_xControl->connect_row_activated(rLink); }
    void connect_column_clicked(const Link<int, void>& rLink) { m_xControl->connect_column_clicked(rLink); }
    void make_sorted() { m_xControl->make_sorted(); }
    bool get_sort_order() const { return m_xControl->get_sort_order(); }
    void set_sort_order(bool bAscending) { m_xControl->set_sort_order(bAscending); }
    int get_sort_column() const { return m_xControl->get_sort_column(); }
    void set_sort_column(int nColumn) { m_xControl->set_sort_column(nColumn); }

    static const OUStringLiteral aForbiddenChars;
    static const char     cSeparator;
};

class SwInsertBookmarkDlg : public SfxDialogController
{
    SwWrtShell&                         rSh;
    std::vector<std::pair<sw::mark::IMark*, OUString>> aTableBookmarks;
    sal_Int32                           m_nLastBookmarksCount;
    bool                                m_bSorted;
    bool m_bAreProtected;

    std::unique_ptr<weld::Entry> m_xEditBox;
    std::unique_ptr<weld::Button> m_xInsertBtn;
    std::unique_ptr<weld::Button> m_xDeleteBtn;
    std::unique_ptr<weld::Button> m_xGotoBtn;
    std::unique_ptr<weld::Button> m_xRenameBtn;
    std::unique_ptr<weld::CheckButton> m_xHideCB;
    std::unique_ptr<weld::Label> m_xConditionFT;
    std::unique_ptr<ConditionEdit> m_xConditionED;
    std::unique_ptr<BookmarkTable> m_xBookmarksBox;
    std::unique_ptr<weld::Label> m_xForbiddenChars;

    DECL_LINK(ModifyHdl, weld::Entry&, void);
    DECL_LINK(InsertHdl, weld::Button&, void);
    DECL_LINK(DeleteHdl, weld::Button&, void);
    DECL_LINK(RenameHdl, weld::Button&, void);
    DECL_LINK(GotoHdl, weld::Button&, void);
    DECL_LINK(SelectionChangedHdl, weld::TreeView&, void);
    DECL_LINK(DoubleClickHdl, weld::TreeView&, bool);
    DECL_LINK(HeaderBarClick, int, void);
    DECL_LINK(ChangeHideHdl, weld::ToggleButton&, void);

    // Fill table with bookmarks
    void PopulateTable();
    /**
     * Check if displayed bookmarks are up-to date, if not update them.
     * @return True if no update was needed.
     */
    bool ValidateBookmarks();
    bool HaveBookmarksChanged();
    void GotoSelectedBookmark();

public:
    SwInsertBookmarkDlg(weld::Window* pParent, SwWrtShell& rSh);
    virtual ~SwInsertBookmarkDlg() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
