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
    = { { u"MacArabic", u"Arabic (Apple Macintosh)" },
        { u"CP864", u"Arabic (DOS/OS2-864)" },
        { u"CP1006", u"Arabic (IBM-1006)" },
        { u"CP1256", u"Arabic (Windows-1256)" },
        { u"CP775", u"Baltic (DOS/OS2-775)" },
        { u"CP1257", u"Baltic (Windows-1257)" },
        { u"MacCeltic", u"Celtic (Apple Macintosh)" },
        { u"MacCyrillic", u"Cyrillic (Apple Macintosh)" },
        { u"CP855", u"Cyrillic (DOS/OS2-855)" },
        { u"CP866", u"Cyrillic (DOS/OS2-866/Russian)" },
        { u"CP1251", u"Cyrillic (Windows-1251)" },
        { u"MacCEurope", u"Eastern Europe (Apple Macintosh)" },
        { u"MacCroatian", u"Eastern Europe (Apple Macintosh/Croatian)" },
        { u"MacRomanian", u"Eastern Europe (Apple Macintosh/Romanian)" },
        { u"CP852", u"Eastern Europe (DOS/OS2-852)" },
        { u"CP1250", u"Eastern Europe (Windows-1250/WinLatin 2)" },
        { u"MacGreek", u"Greek (Apple Macintosh)" },
        { u"CP737", u"Greek (DOS/OS2-737)" },
        { u"CP869", u"Greek (DOS/OS2-869/Greek-2)" },
        { u"CP875", u"Greek (DOS/OS2-875)" },
        { u"CP1253", u"Greek (Windows-1253)" },
        { u"MacHebrew", u"Hebrew (Apple Macintosh)" },
        { u"CP424", u"Hebrew (DOS/OS2-424)" },
        { u"CP856", u"Hebrew (DOS/OS2-856)" },
        { u"CP862", u"Hebrew (DOS/OS2-862)" },
        { u"CP1255", u"Hebrew (Windows-1255)" },
        { u"CP500", u"International (DOS/OS2-500)" },
        { u"CP932", u"Japanese (Windows-932)" },
        { u"MacThai", u"Thai (Apple Macintosh)" },
        { u"CP874", u"Thai (DOS/OS2-874)" },
        { u"CP950", u"Traditional Chinese (Windows-950)" },
        { u"MacTurkish", u"Turkish (Apple Macintosh)" },
        { u"CP857", u"Turkish (DOS/OS2-857)" },
        { u"CP1026", u"Turkish (DOS/OS2-1026)" },
        { u"CP1254", u"Turkish (Windows-1254)" },
        { u"CP1258", u"Vietnamese (Windows-1258)" },
        { u"MacRoman", u"Western Europe (Apple Macintosh)" },
        { u"MacIceland", u"Western Europe (Apple Macintosh/Icelandic)" },
        { u"CP037", u"Western Europe (DOS/OS2-037/US-Canada)" },
        { u"CP437", u"Western Europe (DOS/OS2-437/US)" },
        { u"CP850", u"Western Europe (DOS/OS2-850)" },
        { u"CP860", u"Western Europe (DOS/OS2-860/Portuguese)" },
        { u"CP861", u"Western Europe (DOS/OS2-861/Icelandic)" },
        { u"CP863", u"Western Europe (DOS/OS2-863/French)" },
        { u"CP865", u"Western Europe (DOS/OS2-865/Nordic)" },
        { u"CP1252", u"Western Europe (Windows-1252/WinLatin 1)" } };

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
