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
#include <vcl/weld/weld.hxx>

namespace weld
{
/* Abstract base class for TreeView and IconView. */
class VCL_DLLPUBLIC ItemView : virtual public Widget
{
    OUString m_sSavedValue;

protected:
    virtual void do_set_cursor(const TreeIter& rIter) = 0;

    virtual void do_select(const TreeIter& rIter) = 0;
    virtual void do_unselect(const TreeIter& rIter) = 0;

    virtual void do_select_all() = 0;
    virtual void do_unselect_all() = 0;

    virtual void do_clear() = 0;

public:
    virtual std::unique_ptr<TreeIter> make_iterator(const TreeIter* pOrig = nullptr) const = 0;

    virtual bool get_iter_first(TreeIter& rIter) const = 0;
    // set iter to point to next node at the current level
    virtual bool iter_next_sibling(TreeIter& rIter) const = 0;

    virtual int get_iter_index_in_parent(const TreeIter& rIter) const = 0;

    virtual std::unique_ptr<TreeIter> get_iterator(int nPos) const = 0;

    virtual OUString get_id(int pos) const = 0;
    virtual OUString get_id(const TreeIter& rIter) const = 0;

    virtual OUString get_selected_id() const = 0;
    virtual OUString get_selected_text() const = 0;

    virtual bool get_selected(TreeIter* pIter) const = 0;

    virtual bool get_cursor(TreeIter* pIter) const = 0;
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

    void clear();

    void save_value() { m_sSavedValue = get_selected_text(); }
    OUString const& get_saved_value() const { return m_sSavedValue; }
    bool get_value_changed_from_saved() const { return m_sSavedValue != get_selected_text(); }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
