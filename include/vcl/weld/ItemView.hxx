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
protected:
    virtual void do_select(int pos) = 0;
    virtual void do_unselect(int pos) = 0;
    virtual void do_clear() = 0;

public:
    virtual std::unique_ptr<TreeIter> make_iterator(const TreeIter* pOrig = nullptr) const = 0;

    virtual std::unique_ptr<TreeIter> get_iterator(int nPos) const = 0;

    virtual bool get_selected(TreeIter* pIter) const = 0;
    virtual bool get_cursor(TreeIter* pIter) const = 0;

    // Don't select when frozen, select after thaw. Note selection doesn't survive a freeze.
    void select(int pos);
    void unselect(int pos);
    virtual void select_all() = 0;
    virtual void unselect_all() = 0;

    // return the number of toplevel nodes
    virtual int n_children() const = 0;

    void clear();
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
