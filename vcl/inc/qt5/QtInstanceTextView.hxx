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

#include <QtWidgets/QPlainTextEdit>

class QtInstanceTextView : public QtInstanceWidget, public virtual weld::TextView
{
    Q_OBJECT

    QPlainTextEdit* m_pTextEdit;

public:
    QtInstanceTextView(QPlainTextEdit* pTextEdit);

    virtual void set_text(const OUString& rText) override;
    virtual OUString get_text() const override;
    virtual void select_region(int nStartPos, int nEndPos) override;
    virtual bool get_selection_bounds(int& rStartPos, int& rEndPos) override;
    virtual void replace_selection(const OUString& rText) override;
    virtual void set_editable(bool bEditable) override;
    virtual bool get_editable() const override;
    virtual void set_monospace(bool bMonospace) override;
    virtual void set_max_length(int nChars) override;

    virtual void set_font(const vcl::Font& rFont) override;
    virtual void set_font_color(const Color& rColor) override;

    virtual bool can_move_cursor_with_up() const override;
    virtual bool can_move_cursor_with_down() const override;

    virtual void cut_clipboard() override;
    virtual void copy_clipboard() override;
    virtual void paste_clipboard() override;

    virtual void set_alignment(TxtAlign eXAlign) override;

    virtual int vadjustment_get_value() const override;
    virtual int vadjustment_get_upper() const override;
    virtual int vadjustment_get_page_size() const override;
    virtual void vadjustment_set_value(int nValue) override;

private Q_SLOTS:
    void handleCursorPositionChanged();
    void handleTextChanged();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
