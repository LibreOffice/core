/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/dllapi.h>
#include <vcl/weld/ItemView.hxx>

namespace weld
{
typedef std::tuple<OUString&, const TreeIter&> encoded_image_query;

class VCL_DLLPUBLIC IconView : virtual public ItemView
{
    friend class ::LOKTrigger;

private:
    OUString m_sSavedValue;

protected:
    Link<IconView&, void> m_aSelectionChangeHdl;
    Link<IconView&, bool> m_aItemActivatedHdl;
    Link<const TreeIter&, OUString> m_aQueryTooltipHdl;
    Link<const encoded_image_query&, bool> m_aGetPropertyTreeElemHdl;

    void signal_selection_changed()
    {
        if (notify_events_disabled())
            return;
        m_aSelectionChangeHdl.Call(*this);
    }

    bool signal_item_activated()
    {
        if (notify_events_disabled())
            return true;
        return m_aItemActivatedHdl.Call(*this);
    }

    OUString signal_query_tooltip(const TreeIter& rIter) const
    {
        if (notify_events_disabled())
            return {};
        return m_aQueryTooltipHdl.Call(rIter);
    }

    virtual void do_insert(int pos, const OUString* pStr, const OUString* pId,
                           const OUString* pIconName, TreeIter* pRet)
        = 0;
    virtual void do_insert(int pos, const OUString* pStr, const OUString* pId, const Bitmap* pIcon,
                           TreeIter* pRet)
        = 0;
    virtual void do_remove(int pos) = 0;
    virtual void do_set_cursor(const TreeIter& rIter) = 0;
    virtual void do_scroll_to_item(const TreeIter& rIter) = 0;

public:
    virtual int get_item_width() const = 0;
    virtual void set_item_width(int width) = 0;

    void insert(int pos, const OUString* pStr, const OUString* pId, const OUString* pIconName,
                TreeIter* pRet)
    {
        disable_notify_events();
        do_insert(pos, pStr, pId, pIconName, pRet);
        enable_notify_events();
    }

    void insert(int pos, const OUString* pStr, const OUString* pId, const Bitmap* pIcon,
                TreeIter* pRet)
    {
        disable_notify_events();
        do_insert(pos, pStr, pId, pIcon, pRet);
        enable_notify_events();
    }

    virtual void insert_separator(int pos, const OUString* pId) = 0;

    void append(const OUString& rId, const OUString& rStr, const OUString& rImage)
    {
        insert(-1, &rStr, &rId, &rImage, nullptr);
    }

    void append(const OUString& rId, const OUString& rStr, const Bitmap* pImage)
    {
        insert(-1, &rStr, &rId, pImage, nullptr);
    }

    void append_separator(const OUString& rId) { insert_separator(-1, &rId); }

    void connect_selection_changed(const Link<IconView&, void>& rLink)
    {
        m_aSelectionChangeHdl = rLink;
    }

    /* A row is "activated" when the user double clicks a treeview row. It may
       also be emitted when a row is selected and Space or Enter is pressed.

       a return of "true" means the activation has been handled, a "false" propagates
       the activation to the default handler which expands/collapses the row, if possible.
    */
    void connect_item_activated(const Link<IconView&, bool>& rLink) { m_aItemActivatedHdl = rLink; }

    virtual void connect_query_tooltip(const Link<const TreeIter&, OUString>& rLink)
    {
        m_aQueryTooltipHdl = rLink;
    }

    // 0: OUString, 1: TreeIter, returns true if supported
    virtual void connect_get_image(const Link<const encoded_image_query&, bool>& rLink)
    {
        m_aGetPropertyTreeElemHdl = rLink;
    }

    virtual OUString get_selected_id() const = 0;

    virtual int count_selected_items() const = 0;

    virtual OUString get_selected_text() const = 0;

    //by index. Don't select when frozen, select after thaw. Note selection doesn't survive a freeze.
    virtual OUString get_id(int pos) const = 0;

    virtual void set_image(int pos, VirtualDevice& rDevice) = 0;
    virtual void set_text(int pos, const OUString& rText) = 0;
    virtual void set_id(int pos, const OUString& rId) = 0;
    virtual void set_item_accessible_name(int pos, const OUString& rName) = 0;
    virtual void set_item_tooltip_text(int pos, const OUString& rToolTip) = 0;

    void remove(int pos)
    {
        disable_notify_events();
        do_remove(pos);
        enable_notify_events();
    }

    tools::Rectangle get_rect(int pos) const;

    //via iter
    virtual tools::Rectangle get_rect(const TreeIter& rIter) const = 0;

    void set_cursor(const TreeIter& rIter)
    {
        disable_notify_events();
        do_set_cursor(rIter);
        enable_notify_events();
    }

    virtual bool get_iter_first(TreeIter& rIter) const = 0;
    virtual OUString get_id(const TreeIter& rIter) const = 0;
    virtual OUString get_text(const TreeIter& rIter) const = 0;
    virtual bool iter_next_sibling(TreeIter& rIter) const = 0;

    void scroll_to_item(const TreeIter& rIter)
    {
        disable_notify_events();
        do_scroll_to_item(rIter);
        enable_notify_events();
    }

    // call func on each selected element until func returns true or we run out of elements
    virtual void selected_foreach(const std::function<bool(TreeIter&)>& func) = 0;

    //all of them. Don't select when frozen, select after thaw. Note selection doesn't survive a freeze.
    virtual void select_all() = 0;
    virtual void unselect_all() = 0;

    // return the number of toplevel nodes
    virtual int n_children() const = 0;

    void save_value() { m_sSavedValue = get_selected_text(); }
    OUString const& get_saved_value() const { return m_sSavedValue; }
    bool get_value_changed_from_saved() const { return m_sSavedValue != get_selected_text(); }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
