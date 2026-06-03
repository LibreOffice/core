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
#include <vcl/weld/TreeIter.hxx>
#include <vcl/weld/Widget.hxx>

namespace weld
{
/* Abstract base class for TreeView and IconView. */
class VCL_DLLPUBLIC ItemView : virtual public Widget
{
    OUString m_sSavedValue;

    Link<const TreeIter&, bool> m_aItemActivatedHdl;
    Link<ItemView&, void> m_aSelectionChangeHdl;

protected:
    bool signal_item_activated(const TreeIter& rIter);
    void signal_selection_changed();

    virtual void do_set_cursor(const TreeIter& rIter) = 0;

    virtual void do_select(const TreeIter& rIter) = 0;
    virtual void do_unselect(const TreeIter& rIter) = 0;

    virtual void do_select_all() = 0;
    virtual void do_unselect_all() = 0;

    virtual void do_remove(const TreeIter& rIter) = 0;

    virtual void do_clear() = 0;

public:
    virtual std::unique_ptr<TreeIter> make_iterator(const TreeIter* pOrig = nullptr) const = 0;

    virtual bool get_iter_first(TreeIter& rIter) const = 0;
    // set iter to point to next node at the current level
    virtual bool iter_next_sibling(TreeIter& rIter) const = 0;

    virtual int get_iter_index_in_parent(const TreeIter& rIter) const = 0;

    virtual std::unique_ptr<TreeIter> get_iterator(int nPos) const = 0;

    OUString get_id(int pos) const;
    virtual OUString get_id(const TreeIter& rIter) const = 0;
    void set_id(int pos, const OUString& rId);
    virtual void set_id(const TreeIter& rIter, const OUString& rId) = 0;

    OUString get_selected_id() const;
    virtual OUString get_selected_text() const = 0;

    virtual std::unique_ptr<weld::TreeIter> get_selected() const = 0;
    int get_selected_index() const;

    int get_cursor_index() const;
    virtual std::unique_ptr<TreeIter> get_cursor() const = 0;
    void set_cursor(const TreeIter& rIter);

    // Don't select when frozen, select after thaw. Note selection doesn't survive a freeze.
    void select(int pos);
    void select(const TreeIter& rIter);

    void unselect(int pos);
    void unselect(const TreeIter& rIter);

    void select_all();
    void unselect_all();

    // return the number of toplevel nodes
    virtual int n_children() const = 0;

    void remove(int pos);
    void remove(const TreeIter& rIter);

    void clear();

    // call func on each selected element until func returns true or we run out of elements
    virtual void selected_foreach(const std::function<bool(TreeIter&)>& func) = 0;

    void save_value() { m_sSavedValue = get_selected_text(); }
    bool get_value_changed_from_saved() const { return m_sSavedValue != get_selected_text(); }

    /* An item/row is "activated" when the user double clicks a treeview row or IconView item.
     * It may also be emitted when it is selected and Space or Enter is pressed.

       A return value of "true" means the activation has been handled, a "false" propagates
       the activation to the default handler which expands/collapses the row, if possible.
    */
    void connect_item_activated(const Link<const TreeIter&, bool>& rLink)
    {
        m_aItemActivatedHdl = rLink;
    }

    void connect_selection_changed(const Link<ItemView&, void>& rLink)
    {
        m_aSelectionChangeHdl = rLink;
    }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
