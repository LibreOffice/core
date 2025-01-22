/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceTextView.hxx>
#include <QtInstanceTextView.moc>

#include <vcl/qt/QtUtils.hxx>

#include <QtGui/QTextCursor>

QtInstanceTextView::QtInstanceTextView(QPlainTextEdit* pTextEdit)
    : QtInstanceWidget(pTextEdit)
    , m_pTextEdit(pTextEdit)
{
    assert(m_pTextEdit);
}

void QtInstanceTextView::set_text(const OUString& rText)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { m_pTextEdit->setPlainText(toQString(rText)); });
}

OUString QtInstanceTextView::get_text() const
{
    SolarMutexGuard g;
    OUString sText;
    GetQtInstance().RunInMainThread([&] { sText = toOUString(m_pTextEdit->toPlainText()); });
    return sText;
}

void QtInstanceTextView::select_region(int nStartPos, int nEndPos)
{
    SolarMutexGuard g;

    GetQtInstance().RunInMainThread([&] {
        if (nEndPos == -1)
            nEndPos = m_pTextEdit->toPlainText().length();

        QTextCursor aCursor = m_pTextEdit->textCursor();
        aCursor.setPosition(nStartPos);
        aCursor.setPosition(nEndPos, QTextCursor::KeepAnchor);
        m_pTextEdit->setTextCursor(aCursor);
    });
}

bool QtInstanceTextView::get_selection_bounds(int& rStartPos, int& rEndPos)
{
    SolarMutexGuard g;

    bool bHasSelection = false;
    GetQtInstance().RunInMainThread([&] {
        QTextCursor aCursor = m_pTextEdit->textCursor();
        bHasSelection = aCursor.hasSelection();
        rStartPos = aCursor.selectionStart();
        rEndPos = aCursor.selectionEnd();
    });

    return bHasSelection;
}

void QtInstanceTextView::replace_selection(const OUString& rText)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { m_pTextEdit->insertPlainText(toQString(rText)); });
}

void QtInstanceTextView::set_editable(bool bEditable)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread([&] { m_pTextEdit->setReadOnly(!bEditable); });
}

bool QtInstanceTextView::get_editable() const
{
    SolarMutexGuard g;
    bool bEditable = false;
    GetQtInstance().RunInMainThread([&] { bEditable = !m_pTextEdit->isReadOnly(); });

    return bEditable;
}

void QtInstanceTextView::set_monospace(bool) { assert(false && "Not implemented yet"); }

void QtInstanceTextView::set_max_length(int) { assert(false && "Not implemented yet"); }

void QtInstanceTextView::set_font(const vcl::Font&) { assert(false && "Not implemented yet"); }

void QtInstanceTextView::set_font_color(const Color& rColor) { setFontColor(rColor); }

bool QtInstanceTextView::can_move_cursor_with_up() const
{
    assert(false && "Not implemented yet");
    return false;
}

bool QtInstanceTextView::can_move_cursor_with_down() const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceTextView::cut_clipboard() { assert(false && "Not implemented yet"); }

void QtInstanceTextView::copy_clipboard() { assert(false && "Not implemented yet"); }

void QtInstanceTextView::paste_clipboard() { assert(false && "Not implemented yet"); }

void QtInstanceTextView::set_alignment(TxtAlign) { assert(false && "Not implemented yet"); }

int QtInstanceTextView::vadjustment_get_value() const
{
    assert(false && "Not implemented yet");
    return -1;
}

int QtInstanceTextView::vadjustment_get_upper() const
{
    assert(false && "Not implemented yet");
    return -1;
}

int QtInstanceTextView::vadjustment_get_lower() const
{
    assert(false && "Not implemented yet");
    return -1;
}

int QtInstanceTextView::vadjustment_get_page_size() const
{
    assert(false && "Not implemented yet");
    return -1;
}

void QtInstanceTextView::vadjustment_set_value(int) { assert(false && "Not implemented yet"); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
