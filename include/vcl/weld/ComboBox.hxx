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
class Menu;

struct VCL_DLLPUBLIC ComboBoxEntry
{
    OUString sString;
    OUString sId;
    OUString sImage;
    ComboBoxEntry(OUString _aString)
        : sString(std::move(_aString))
    {
    }
    ComboBoxEntry(OUString _aString, OUString _aId)
        : sString(std::move(_aString))
        , sId(std::move(_aId))
    {
    }
    ComboBoxEntry(OUString _aString, OUString _aId, OUString _aImage)
        : sString(std::move(_aString))
        , sId(std::move(_aId))
        , sImage(std::move(_aImage))
    {
    }
};

/// A widget used to choose from a list of items.
class VCL_DLLPUBLIC ComboBox : virtual public Widget
{
private:
    OUString m_sSavedValue;
    std::vector<OUString> m_aSavedValues;

public:
    // OUString is the id of the row, it may be null to measure the height of a generic line
    typedef std::tuple<vcl::RenderContext&, const tools::Rectangle&, bool, const OUString&>
        render_args;

protected:
    Link<ComboBox&, void> m_aChangeHdl;
    Link<ComboBox&, void> m_aPopupToggledHdl;
    Link<ComboBox&, bool> m_aEntryActivateHdl;
    Link<OUString&, bool> m_aEntryInsertTextHdl;

    friend class ::LOKTrigger;

    void signal_changed()
    {
        if (notify_events_disabled())
            return;
        m_aChangeHdl.Call(*this);
    }

    virtual void signal_popup_toggled() { m_aPopupToggledHdl.Call(*this); }

    Link<render_args, void> m_aRenderHdl;
    void signal_custom_render(vcl::RenderContext& rDevice, const tools::Rectangle& rRect,
                              bool bSelected, const OUString& rId)
    {
        m_aRenderHdl.Call(render_args(rDevice, rRect, bSelected, rId));
    }

    Link<vcl::RenderContext&, Size> m_aGetSizeHdl;
    Size signal_custom_get_size(vcl::RenderContext& rDevice) { return m_aGetSizeHdl.Call(rDevice); }

    virtual void do_insert(int pos, const OUString& rStr, const OUString* pId,
                           const OUString* pIconName, VirtualDevice* pImageSurface)
        = 0;
    virtual void do_set_active(int pos) = 0;
    virtual void do_set_active_id(const OUString& rStr) = 0;

public:
    void insert(int pos, const OUString& rStr, const OUString* pId, const OUString* pIconName,
                VirtualDevice* pImageSurface);
    virtual void insert_vector(const std::vector<weld::ComboBoxEntry>& rItems, bool bKeepExisting)
        = 0;
    void insert(int pos, const weld::ComboBoxEntry& rItem)
    {
        insert(pos, rItem.sString, rItem.sId.isEmpty() ? nullptr : &rItem.sId,
               rItem.sImage.isEmpty() ? nullptr : &rItem.sImage, nullptr);
    }
    void insert_text(int pos, const OUString& rStr)
    {
        insert(pos, rStr, nullptr, nullptr, nullptr);
    }
    void append(const weld::ComboBoxEntry& rItem) { insert(-1, rItem); }
    void append_text(const OUString& rStr) { insert(-1, rStr, nullptr, nullptr, nullptr); }
    void append(const OUString& rId, const OUString& rStr)
    {
        insert(-1, rStr, &rId, nullptr, nullptr);
    }
    void append(const OUString& rId, const OUString& rStr, const OUString& rImage)
    {
        insert(-1, rStr, &rId, &rImage, nullptr);
    }
    void append(const OUString& rId, const OUString& rStr, VirtualDevice& rImage)
    {
        insert(-1, rStr, &rId, nullptr, &rImage);
    }
    void append(int pos, const OUString& rId, const OUString& rStr)
    {
        insert(pos, rStr, &rId, nullptr, nullptr);
    }
    virtual void insert_separator(int pos, const OUString& rId) = 0;
    void append_separator(const OUString& rId) { insert_separator(-1, rId); }

    virtual int get_count() const = 0;
    virtual void make_sorted() = 0;
    virtual void clear() = 0;

    //by index, returns -1 if nothing is selected
    virtual int get_active() const = 0;
    void set_active(int pos);
    virtual void remove(int pos) = 0;

    //by text
    virtual OUString get_active_text() const = 0;
    void set_active_text(const OUString& rStr) { set_active(find_text(rStr)); }
    virtual OUString get_text(int pos) const = 0;
    virtual int find_text(const OUString& rStr) const = 0;
    void remove_text(const OUString& rText) { remove(find_text(rText)); }

    //by id
    virtual OUString get_active_id() const = 0;
    void set_active_id(const OUString& rStr);
    virtual OUString get_id(int pos) const = 0;
    virtual void set_id(int row, const OUString& rId) = 0;
    virtual int find_id(const OUString& rId) const = 0;
    void remove_id(const OUString& rId) { remove(find_id(rId)); }

    /* m_aChangeHdl is called when the active item is changed. The can be due
       to the user selecting a different item from the list or while typing
       into the entry of a combo box with an entry.

       Use changed_by_direct_pick() to discover whether an item was actually explicitly
       selected, e.g. from the menu.
     */
    void connect_changed(const Link<ComboBox&, void>& rLink) { m_aChangeHdl = rLink; }

    virtual bool changed_by_direct_pick() const = 0;

    virtual void connect_popup_toggled(const Link<ComboBox&, void>& rLink)
    {
        m_aPopupToggledHdl = rLink;
    }

    //entry related
    virtual bool has_entry() const = 0;
    virtual void set_entry_message_type(EntryMessageType eType) = 0;
    virtual void set_entry_text(const OUString& rStr) = 0;
    virtual void set_entry_width_chars(int nChars) = 0;
    virtual void set_entry_max_length(int nChars) = 0;
    virtual void select_entry_region(int nStartPos, int nEndPos) = 0;
    virtual bool get_entry_selection_bounds(int& rStartPos, int& rEndPos) = 0;
    virtual void set_entry_completion(bool bEnable, bool bCaseSensitive = false) = 0;
    virtual void set_entry_placeholder_text(const OUString& rText) = 0;
    virtual void set_entry_editable(bool bEditable) = 0;
    virtual void cut_entry_clipboard() = 0;
    virtual void copy_entry_clipboard() = 0;
    virtual void paste_entry_clipboard() = 0;

    // font size is in points, not pixels, e.g. see Window::[G]etPointFont
    virtual void set_font(const vcl::Font& rFont) = 0;

    // font size is in points, not pixels, e.g. see Window::[G]etPointFont
    virtual void set_entry_font(const vcl::Font& rFont) = 0;
    virtual vcl::Font get_entry_font() = 0;

    virtual bool get_popup_shown() const = 0;

    void connect_entry_insert_text(const Link<OUString&, bool>& rLink)
    {
        m_aEntryInsertTextHdl = rLink;
    }

    // callback returns true to indicated no further processing of activate wanted
    void connect_entry_activate(const Link<ComboBox&, bool>& rLink) { m_aEntryActivateHdl = rLink; }

    void save_value() { m_sSavedValue = get_active_text(); }

    void save_values_by_id(const OUString& rId)
    {
        m_aSavedValues.push_back(get_text(find_id(rId)));
    }

    OUString const& get_saved_value() const { return m_sSavedValue; }
    OUString const& get_saved_values(int pos) const { return m_aSavedValues[pos]; }
    bool get_value_changed_from_saved() const { return m_sSavedValue != get_active_text(); }
    bool get_values_changed_from_saved() const;

    void removeSavedValues() { m_aSavedValues.clear(); }

    // for custom rendering a row
    void connect_custom_get_size(const Link<vcl::RenderContext&, Size>& rLink)
    {
        m_aGetSizeHdl = rLink;
    }
    void connect_custom_render(const Link<render_args, void>& rLink) { m_aRenderHdl = rLink; }
    // call set_custom_renderer after setting custom callbacks
    virtual void set_custom_renderer(bool bOn) = 0;
    // set a sub menu for a entry, only works with custom rendering
    virtual void set_item_menu(const OUString& rIdent, weld::Menu* pMenu) = 0;
    // get the width needed to show the menu launcher in a custom row
    virtual int get_menu_button_width() const = 0;

    // for mru support
    virtual int get_max_mru_count() const = 0;
    virtual void set_max_mru_count(int nCount) = 0;
    virtual std::vector<OUString> get_mru_entries() const = 0;
    virtual void set_mru_entries(const std::vector<OUString>& rEntries) = 0;

    // Backwards compatibility, should be avoided to allow
    // UI consistency.
    virtual void set_max_drop_down_rows(int nRows) = 0;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
