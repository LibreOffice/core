/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "QtInstanceWidget.hxx"

#include <QtWidgets/QComboBox>

class QtInstanceComboBox : public QtInstanceWidget, public virtual weld::ComboBox
{
    Q_OBJECT

    QComboBox* m_pComboBox;
    bool m_bSorted;

public:
    QtInstanceComboBox(QComboBox* pComboBox);

    virtual void insert(int nPos, const OUString& rStr, const OUString* pId,
                        const OUString* pIconName, VirtualDevice* pImageSurface) override;
    virtual void insert_vector(const std::vector<weld::ComboBoxEntry>& rItems,
                               bool bKeepExisting) override;

    virtual void insert_separator(int nPos, const OUString& rId) override;

    virtual int get_count() const override;
    virtual void make_sorted() override;
    virtual void clear() override;

    virtual int get_active() const override;
    virtual void set_active(int nPos) override;
    virtual void remove(int nPos) override;

    virtual OUString get_active_text() const override;
    virtual OUString get_text(int nPos) const override;
    virtual int find_text(const OUString& rStr) const override;

    virtual OUString get_active_id() const override;
    virtual void set_active_id(const OUString& rStr) override;
    virtual OUString get_id(int nPos) const override;
    virtual void set_id(int nRow, const OUString& rId) override;
    virtual int find_id(const OUString& rId) const override;

    virtual bool changed_by_direct_pick() const override;

    virtual bool has_entry() const override;
    virtual void set_entry_message_type(weld::EntryMessageType eType) override;
    virtual void set_entry_text(const OUString& rStr) override;
    virtual void set_entry_width_chars(int nChars) override;
    virtual void set_entry_max_length(int nChars) override;
    virtual void select_entry_region(int nStartPos, int nEndPos) override;
    virtual bool get_entry_selection_bounds(int& rStartPos, int& rEndPos) override;
    virtual void set_entry_completion(bool bEnable, bool bCaseSensitive = false) override;
    virtual void set_entry_placeholder_text(const OUString& rText) override;
    virtual void set_entry_editable(bool bEditable) override;
    virtual void cut_entry_clipboard() override;
    virtual void copy_entry_clipboard() override;
    virtual void paste_entry_clipboard() override;

    virtual void set_font(const vcl::Font& rFont) override;

    virtual void set_entry_font(const vcl::Font& rFont) override;
    virtual vcl::Font get_entry_font() override;

    virtual bool get_popup_shown() const override;

    virtual void set_custom_renderer(bool bOn) override;
    virtual VclPtr<VirtualDevice> create_render_virtual_device() const override;
    virtual void set_item_menu(const OUString& rIdent, weld::Menu* pMenu) override;
    virtual int get_menu_button_width() const override;

    virtual int get_max_mru_count() const override;
    virtual void set_max_mru_count(int nCount) override;
    virtual OUString get_mru_entries() const override;
    virtual void set_mru_entries(const OUString& rEntries) override;

    virtual void set_max_drop_down_rows(int nRows) override;

private:
    void sortItems();

private slots:
    void signalChanged();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
