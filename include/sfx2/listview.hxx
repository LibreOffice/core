/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_LISTVIEW_HXX
#define INCLUDED_SFX2_LISTVIEW_HXX

#include <vcl/weld.hxx>
#include <map>

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
protected:
    std::unique_ptr<weld::TreeView> mxTreeView;

public:
    ListView(std::unique_ptr<weld::TreeView> xTreeView);
    ~ListView();

    void connect_changed(const Link<weld::TreeView&, void>& rLink)
    {
        mxTreeView->connect_changed(rLink);
    }

    void AppendItem(const OUString& rId, const OUString& rTitle, const OUString& rSubtitle,
                    const OUString& rPath, bool isDefault);

    void sortColumn(const int col);

    int get_sort_indicator_column();

    void sort();

    void updateIsDefaultColumn();

    void remove(const OUString& rId);

    void rename(const OUString& rId, const OUString& rTitle);

    void clearListView();

    void ShowListView() { mxTreeView->show(); }

    void HideListView() { mxTreeView->hide(); }

    void unselect_all() { mxTreeView->unselect_all(); }

    OUString get_selected_id() { return mxTreeView->get_selected_id(); }

    void select_id(const OUString& sId) { mxTreeView->select_id(sId); }

    int get_selected_index() { return mxTreeView->get_selected_index(); }

    std::vector<int> get_selected_rows() { return mxTreeView->get_selected_rows(); }

    bool IsListViewVisible() { return mxTreeView->is_visible(); }

    OUString get_id(int pos) { return mxTreeView->get_id(pos); }

    DECL_LINK(ColumnClickedHdl, const int, void);

protected:
    std::map<OUString, std::unique_ptr<ListViewItem>> mListViewItems;
};

struct ListViewItem
{
public:
    OUString maId;
    OUString maPath;
    OUString maModify;
    OUString maSize;
};

#endif // INCLUDED_SFX2_LISTVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
