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
#include <vcl/font.hxx>
#include <vcl/weld/ComboBox.hxx>
#include <vcl/weld/Entry.hxx>
#include <vcl/weld/TreeView.hxx>

namespace weld
{
// an entry + treeview pair, where the entry autocompletes from the
// treeview list, and selecting something in the list sets the
// entry to that text, i.e. a visually exploded ComboBox
class VCL_DLLPUBLIC EntryTreeView : virtual public ComboBox
{
private:
    DECL_DLLPRIVATE_LINK(ClickHdl, weld::TreeView&, void);
    DECL_DLLPRIVATE_LINK(ModifyHdl, weld::Entry&, void);

protected:
    std::unique_ptr<Entry> m_xEntry;
    std::unique_ptr<TreeView> m_xTreeView;

    virtual void do_insert(int pos, const OUString& rStr, const OUString* pId,
                           const OUString* pIconName, VirtualDevice* pImageSurface) override
    {
        m_xTreeView->insert(pos, rStr, pId, pIconName, pImageSurface);
    }

public:
    EntryTreeView(std::unique_ptr<Entry> xEntry, std::unique_ptr<TreeView> xTreeView);

    virtual void insert_vector(const std::vector<weld::ComboBoxEntry>& rItems,
                               bool bKeepExisting) override
    {
        m_xTreeView->freeze();
        if (!bKeepExisting)
            m_xTreeView->clear();
        for (const auto& rItem : rItems)
            append(rItem);
        m_xTreeView->thaw();
    }

    virtual int get_count() const override { return m_xTreeView->n_children(); }
    virtual void clear() override { m_xTreeView->clear(); }

    //by index
    virtual int get_active() const override { return m_xTreeView->get_selected_index(); }
    virtual void do_set_active(int pos) override
    {
        m_xTreeView->set_cursor(pos);
        m_xTreeView->select(pos);
        m_xEntry->set_text(m_xTreeView->get_selected_text());
    }
    virtual void remove(int pos) override { m_xTreeView->remove(pos); }

    //by text
    virtual OUString get_active_text() const override { return m_xEntry->get_text(); }
    virtual OUString get_text(int pos) const override { return m_xTreeView->get_text(pos); }
    virtual int find_text(const OUString& rStr) const override
    {
        return m_xTreeView->find_text(rStr);
    }

    //by id
    virtual OUString get_active_id() const override { return m_xTreeView->get_selected_id(); }
    virtual void do_set_active_id(const OUString& rStr) override
    {
        m_xTreeView->select_id(rStr);
        m_xEntry->set_text(m_xTreeView->get_selected_text());
    }
    virtual OUString get_id(int pos) const override { return m_xTreeView->get_id(pos); }
    virtual void set_id(int pos, const OUString& rId) override { m_xTreeView->set_id(pos, rId); }
    virtual int find_id(const OUString& rId) const override { return m_xTreeView->find_id(rId); }

    //entry related
    virtual bool has_entry() const override { return true; }
    virtual void set_entry_message_type(EntryMessageType eType) override
    {
        m_xEntry->set_message_type(eType);
    }
    virtual void set_entry_text(const OUString& rStr) override { m_xEntry->set_text(rStr); }
    virtual void set_entry_width_chars(int nChars) override { m_xEntry->set_width_chars(nChars); }
    virtual void set_entry_max_length(int nChars) override { m_xEntry->set_max_length(nChars); }
    virtual void select_entry_region(int nStartPos, int nEndPos) override
    {
        m_xEntry->select_region(nStartPos, nEndPos);
    }
    //if no text was selected, both rStartPos and rEndPos will be identical
    //and false will be returned
    virtual bool get_entry_selection_bounds(int& rStartPos, int& rEndPos) override
    {
        return m_xEntry->get_selection_bounds(rStartPos, rEndPos);
    }

    virtual void set_entry_placeholder_text(const OUString& rText) override
    {
        m_xEntry->set_placeholder_text(rText);
    }

    virtual void set_entry_editable(bool bEditable) override { m_xEntry->set_editable(bEditable); }

    virtual void cut_entry_clipboard() override { m_xEntry->cut_clipboard(); }

    virtual void copy_entry_clipboard() override { m_xEntry->copy_clipboard(); }

    virtual void paste_entry_clipboard() override { m_xEntry->paste_clipboard(); }

    virtual void set_entry_font(const vcl::Font& rFont) override { m_xEntry->set_font(rFont); }

    virtual vcl::Font get_entry_font() override { return m_xEntry->get_font(); }

    void connect_row_activated(const Link<TreeView&, bool>& rLink)
    {
        m_xTreeView->connect_row_activated(rLink);
    }

    virtual bool get_popup_shown() const override { return false; }

    void set_height_request_by_rows(int nRows);

    // Methods from weld::ComboBox subclass API that are not actually implemented/used
    virtual void insert_separator(int /*pos*/, const OUString& /*rId*/) override final
    {
        assert(false && "not implemented");
    }

    virtual void set_font(const vcl::Font&) override final { assert(false && "not implemented"); }

    virtual void set_custom_renderer(bool /*bOn*/) override final
    {
        assert(false && "not implemented");
    }

    virtual int get_max_mru_count() const override final
    {
        assert(false && "not implemented");
        return 0;
    }

    virtual void set_max_mru_count(int) override final { assert(false && "not implemented"); }

    virtual std::vector<OUString> get_mru_entries() const override final
    {
        assert(false && "not implemented");
        return {};
    }

    virtual void set_mru_entries(const std::vector<OUString>&) override final
    {
        assert(false && "not implemented");
    }

    virtual void set_max_drop_down_rows(int) override final { assert(false && "not implemented"); }

    virtual void set_item_menu(const OUString&, weld::Menu*) override final
    {
        assert(false && "not implemented");
    }

    int get_menu_button_width() const override final
    {
        assert(false && "not implemented");
        return 0;
    }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
