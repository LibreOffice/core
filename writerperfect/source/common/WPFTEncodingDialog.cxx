/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <cstddef>
#include <utility>

#include <WPFTEncodingDialog.hxx>

namespace writerperfect
{
namespace
{
std::pair<OUStringLiteral, OUStringLiteral> const s_encodings[]
    = { { "MacArabic", "Arabic (Apple Macintosh)" },
        { "CP864", "Arabic (DOS/OS2-864)" },
        { "CP1006", "Arabic (IBM-1006)" },
        { "CP1256", "Arabic (Windows-1256)" },
        { "CP775", "Baltic (DOS/OS2-775)" },
        { "CP1257", "Baltic (Windows-1257)" },
        { "MacCeltic", "Celtic (Apple Macintosh)" },
        { "MacCyrillic", "Cyrillic (Apple Macintosh)" },
        { "CP855", "Cyrillic (DOS/OS2-855)" },
        { "CP866", "Cyrillic (DOS/OS2-866/Russian)" },
        { "CP1251", "Cyrillic (Windows-1251)" },
        { "MacCEurope", "Eastern Europe (Apple Macintosh)" },
        { "MacCroatian", "Eastern Europe (Apple Macintosh/Croatian)" },
        { "MacRomanian", "Eastern Europe (Apple Macintosh/Romanian)" },
        { "CP852", "Eastern Europe (DOS/OS2-852)" },
        { "CP1250", "Eastern Europe (Windows-1250/WinLatin 2)" },
        { "MacGreek", "Greek (Apple Macintosh)" },
        { "CP737", "Greek (DOS/OS2-737)" },
        { "CP869", "Greek (DOS/OS2-869/Greek-2)" },
        { "CP875", "Greek (DOS/OS2-875)" },
        { "CP1253", "Greek (Windows-1253)" },
        { "MacHebrew", "Hebrew (Apple Macintosh)" },
        { "CP424", "Hebrew (DOS/OS2-424)" },
        { "CP856", "Hebrew (DOS/OS2-856)" },
        { "CP862", "Hebrew (DOS/OS2-862)" },
        { "CP1255", "Hebrew (Windows-1255)" },
        { "CP500", "International (DOS/OS2-500)" },
        { "CP932", "Japanese (Windows-932)" },
        { "MacThai", "Thai (Apple Macintosh)" },
        { "CP874", "Thai (DOS/OS2-874)" },
        { "CP950", "Traditional Chinese (Windows-950)" },
        { "MacTurkish", "Turkish (Apple Macintosh)" },
        { "CP857", "Turkish (DOS/OS2-857)" },
        { "CP1026", "Turkish (DOS/OS2-1026)" },
        { "CP1254", "Turkish (Windows-1254)" },
        { "CP1258", "Vietnamese (Windows-1258)" },
        { "MacRoman", "Western Europe (Apple Macintosh)" },
        { "MacIceland", "Western Europe (Apple Macintosh/Icelandic)" },
        { "CP037", "Western Europe (DOS/OS2-037/US-Canada)" },
        { "CP437", "Western Europe (DOS/OS2-437/US)" },
        { "CP850", "Western Europe (DOS/OS2-850)" },
        { "CP860", "Western Europe (DOS/OS2-860/Portuguese)" },
        { "CP861", "Western Europe (DOS/OS2-861/Icelandic)" },
        { "CP863", "Western Europe (DOS/OS2-863/French)" },
        { "CP865", "Western Europe (DOS/OS2-865/Nordic)" },
        { "CP1252", "Western Europe (Windows-1252/WinLatin 1)" } };

std::size_t const numEncodings = SAL_N_ELEMENTS(s_encodings);

void insertEncodings(weld::ComboBox& box)
{
    for (std::size_t i = 0; i < numEncodings; ++i)
        box.append(s_encodings[i].first, s_encodings[i].second);
}

void selectEncoding(weld::ComboBox& box, const OUString& encoding) { box.set_active_id(encoding); }

OUString getEncoding(const weld::ComboBox& box) { return box.get_active_id(); }
}

WPFTEncodingDialog::WPFTEncodingDialog(weld::Window* pParent, const OUString& title,
                                       const OUString& encoding)
    : GenericDialogController(pParent, "writerperfect/ui/wpftencodingdialog.ui",
                              "WPFTEncodingDialog")
    , m_userHasCancelled(false)
    , m_xLbCharset(m_xBuilder->weld_combo_box("comboboxtext"))
    , m_xBtnOk(m_xBuilder->weld_button("ok"))
    , m_xBtnCancel(m_xBuilder->weld_button("cancel"))
{
    m_xBtnCancel->connect_clicked(LINK(this, WPFTEncodingDialog, CancelHdl));

    insertEncodings(*m_xLbCharset);
    m_xLbCharset->make_sorted();
    selectEncoding(*m_xLbCharset, encoding);

    m_xDialog->set_title(title);
}

WPFTEncodingDialog::~WPFTEncodingDialog() {}

OUString WPFTEncodingDialog::GetEncoding() const { return getEncoding(*m_xLbCharset); }

IMPL_LINK_NOARG(WPFTEncodingDialog, CancelHdl, weld::Button&, void)
{
    m_userHasCancelled = true;
    m_xDialog->response(RET_CANCEL);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
