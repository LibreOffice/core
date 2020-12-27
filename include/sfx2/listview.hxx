/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/weld.hxx>

enum TemplateViewMode
{
    eListView,
    eThumbnailView
};
class SfxDocumentTemplates;
class TemplateContainerItem;
struct ListViewItem;

class ListView
{
public:
    ListView(std::unique_ptr<weld::TreeView> xTreeView);
    ~ListView();

    void AppendItem(const OUString& rId, const OUString& rTitle, const OUString& rSubtitle,
                    const OUString& rPath, bool bDefault);

    void AppendRow(const OUString& rImage, const OUString& rTitle, const OUString& rSubtitle,
                   const OUString& rApplication, const OUString& rModify, const OUString& rSize,
                   const OUString& rId);

    void UpdateRow(int nIndex, const OUString& rImage, const OUString& rTitle,
                   const OUString& rSubtitle, const OUString& rApplication, const OUString& rModify,
                   const OUString& rSize, const OUString& rId);

    void ReloadRows();

    bool UpdateRows();

    void sortColumn(const int col);

    void sort();

    void clearListView();

    void ShowListView() { mxTreeView->show(); }

    void HideListView() { mxTreeView->hide(); }

    void unselect_all() { mxTreeView->unselect_all(); }

    void grab_focus() { mxTreeView->grab_focus(); }

    void remove(const OUString& rId);

    void rename(const OUString& rId, const OUString& rTitle);

    void refreshDefaultColumn();

protected:
    sal_uInt16 get_nId(int pos);

    void select_id(const OUString& sId) { mxTreeView->select_id(sId); }

    int get_selected_index() { return mxTreeView->get_selected_index(); }

    std::vector<int> get_selected_rows() { return mxTreeView->get_selected_rows(); }

    bool IsListViewVisible() { return mxTreeView->is_visible(); }

    OUString get_id(int pos) { return mxTreeView->get_id(pos); }

    void set_cursor(int pos) { mxTreeView->set_cursor(pos); }

    int get_cursor_index() { return mxTreeView->get_cursor_index(); }

    sal_uInt16 get_cursor_nId() { return get_nId(mxTreeView->get_cursor_index()); }

    void select(int pos) { mxTreeView->select(pos); }

    int get_index(sal_uInt16 nId) { return mxTreeView->find_id(OUString::number(nId)); }

    DECL_LINK(ColumnClickedHdl, const int, void);

    DECL_LINK(QueryTooltipHdl, const weld::TreeIter&, OUString);

protected:
    std::unique_ptr<weld::TreeView> mxTreeView;
    std::vector<std::unique_ptr<ListViewItem>> mListViewItems;
    Link<weld::TreeView&, void> maSelectionChangedHdl;
    int mnSortColumn;
};

struct ListViewItem
{
public:
    OUString maId;
    OUString maTitle;
    OUString maSubtitle;
    OUString maApplication;
    OUString maPath;
    bool mbDefault;

    /** Last modify time in seconds since 1/1/1970. */
    sal_uInt32 mnModify;
    /** Size in bytes of the file. */
    sal_uInt64 mnSize;

    OUString maDisplayModify;
    OUString maDisplaySize;
    OUString maDisplayPath;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
