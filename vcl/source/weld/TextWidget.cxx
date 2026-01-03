/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <vcl/transfer.hxx>
#include <vcl/unohelp2.hxx>
#include <vcl/weld/TextWidget.hxx>

namespace weld
{
void TextWidget::set_text(const OUString& rText)
{
    disable_notify_events();
    do_set_text(rText);
    enable_notify_events();
}

void TextWidget::select_region(int nStartPos, int nEndPos)
{
    disable_notify_events();
    do_select_region(nStartPos, nEndPos);
    enable_notify_events();
}

void TextWidget::replace_selection(const OUString& rText)
{
    disable_notify_events();
    do_replace_selection(rText);
    enable_notify_events();
}

void TextWidget::cut_clipboard()
{
    copy_clipboard();
    replace_selection(u""_ustr);
}

void TextWidget::copy_clipboard()
{
    int nSelectionStart = 0;
    int nSelectionEnd = 0;
    if (!get_selection_bounds(nSelectionStart, nSelectionEnd))
        return;

    const OUString sText = get_text();
    assert(nSelectionStart >= 0 && nSelectionStart <= sText.getLength() && nSelectionEnd >= 0
           && nSelectionEnd <= sText.getLength());
    const OUString sSelectedText = sText.copy(std::min(nSelectionStart, nSelectionEnd),
                                              std::abs(nSelectionEnd - nSelectionStart));
    vcl::unohelper::TextDataObject::CopyStringTo(sSelectedText, get_clipboard());
}

void TextWidget::paste_clipboard()
{
    TransferableDataHelper aDataHelper
        = TransferableDataHelper::CreateFromClipboard(get_clipboard());
    OUString sText;
    if (aDataHelper.GetString(SotClipboardFormatId::STRING, sText))
        replace_selection(sText);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
