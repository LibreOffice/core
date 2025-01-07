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

#include <QtWidgets/QLineEdit>

class QtInstanceEntry : public QtInstanceWidget, public virtual weld::Entry
{
    Q_OBJECT;

    QLineEdit* m_pLineEdit;

public:
    QtInstanceEntry(QLineEdit* pLineEdit);

    virtual void set_text(const OUString& rText) override;
    virtual OUString get_text() const override;
    virtual void set_width_chars(int nChars) override;
    virtual int get_width_chars() const override;
    virtual void set_max_length(int nChars) override;
    virtual void select_region(int nStartPos, int nEndPos) override;
    virtual bool get_selection_bounds(int& rStartPos, int& rEndPos) override;
    virtual void replace_selection(const OUString& rText) override;
    virtual void set_position(int nCursorPos) override;
    virtual int get_position() const override;
    virtual void set_editable(bool bEditable) override;
    virtual bool get_editable() const override;
    virtual void set_visibility(bool bVisible) override;
    virtual void set_message_type(weld::EntryMessageType eType) override;
    virtual void set_placeholder_text(const OUString& rText) override;

    virtual void set_overwrite_mode(bool bOn) override;
    virtual bool get_overwrite_mode() const override;

    virtual void set_font(const vcl::Font& rFont) override;
    virtual void set_font_color(const Color& rColor) override;

    virtual void cut_clipboard() override;
    virtual void copy_clipboard() override;
    virtual void paste_clipboard() override;

    virtual void set_alignment(TxtAlign eXAlign) override;

    static void setMessageType(QLineEdit& rLineEdit, weld::EntryMessageType eType);

protected Q_SLOTS:
    void handleTextChanged();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
