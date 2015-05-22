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

#include <WPFTEncodingDialog.hxx>

namespace writerperfect
{

namespace
{

struct EncodingImplementation
{

    static OUString const(s_encodings[]);

    static inline int numEncodings();
    static void insertEncodings(ListBox *box);
    static void selectEncoding(ListBox *box, const OUString &encoding);
    static OUString getEncoding(ListBox *box);
};

OUString const(EncodingImplementation::s_encodings[])=
{
    "MacArabic", "Arabic (Apple Macintosh)",
    "CP864", "Arabic (DOS/OS2-864)",
    "CP1006", "Arabic (IBM-1006)",
    "CP1256", "Arabic (Windows-1256)",
    "CP775", "Baltic (DOS/OS2-775)",
    "CP1257", "Baltic (Windows-1257)",
    "MacCeltic", "Celtic (Apple Macintosh)",
    "MacCyrillic", "Cyrillic (Apple Macintosh)",
    "CP855", "Cyrillic (DOS/OS2-855)",
    "CP866", "Cyrillic (DOS/OS2-866/Russian)",
    "CP1251", "Cyrillic (Windows-1251)",
    "MacCEurope", "Eastern Europe (Apple Macintosh)",
    "MacCroatian", "Eastern Europe (Apple Macintosh/Croatian)",
    "MacRomanian", "Eastern Europe (Apple Macintosh/Romanian)",
    "CP852", "Eastern Europe (DOS/OS2-852)",
    "CP1250", "Eastern Europe (Windows-1250/WinLatin 2)",
    "MacGreek", "Greek (Apple Macintosh)",
    "CP737", "Greek (DOS/OS2-737)",
    "CP869", "Greek (DOS/OS2-869/Greek-2)",
    "CP875", "Greek (DOS/OS2-875)",
    "CP1253", "Greek (Windows-1253)",
    "MacHebrew", "Hebrew (Apple Macintosh)",
    "CP424", "Hebrew (DOS/OS2-424)",
    "CP856", "Hebrew (DOS/OS2-856)",
    "CP862", "Hebrew (DOS/OS2-862)",
    "CP1255", "Hebrew (Windows-1255)",
    "CP500", "International (DOS/OS2-500)",
    "MacThai", "Thai (Apple Macintosh)",
    "CP874", "Thai (DOS/OS2-874)",
    "MacTurkish", "Turkish (Apple Macintosh)",
    "CP857", "Turkish (DOS/OS2-857)",
    "CP1026", "Turkish (DOS/OS2-1026)",
    "CP1254", "Turkish (Windows-1254)",
    "CP1258", "Vietnamese (Windows-1258)",
    "MacRoman", "Western Europe (Apple Macintosh)",
    "MacIceland", "Western Europe (Apple Macintosh/Icelandic)",
    "CP037", "Western Europe (DOS/OS2-037/US-Canada)",
    "CP437", "Western Europe (DOS/OS2-437/US)",
    "CP850", "Western Europe (DOS/OS2-850)",
    "CP860", "Western Europe (DOS/OS2-860/Portuguese)",
    "CP861", "Western Europe (DOS/OS2-861/Icelandic)",
    "CP863", "Western Europe (DOS/OS2-863/French)",
    "CP865", "Western Europe (DOS/OS2-865/Nordic)",
    "CP1252", "Western Europe (Windows-1252/WinLatin 1)"
};

inline int EncodingImplementation::numEncodings()
{
    return int(sizeof(s_encodings)/(2*sizeof(const OUString *)));
}

void EncodingImplementation::insertEncodings(ListBox *box)
{
    sal_IntPtr num=sal_IntPtr(numEncodings());
    for (sal_IntPtr i=0; i<num; ++i)
    {
        sal_IntPtr nAt=box->InsertEntry(s_encodings[2*i+1]);
        box->SetEntryData(nAt, reinterpret_cast<void *>(i));
    }
}

void EncodingImplementation::selectEncoding(ListBox *box, const OUString &encoding)
{
    sal_IntPtr num=sal_IntPtr(numEncodings());
    for (sal_IntPtr i=0; i<num; ++i)
    {
        if (encoding!=s_encodings[2*i]) continue;
        box->SelectEntryPos(i);
        return;
    }
}

OUString EncodingImplementation::getEncoding(ListBox *box)
{
    sal_IntPtr pos = reinterpret_cast<sal_IntPtr>(box->GetSelectEntryData());
    if (pos<0||pos>=numEncodings())
        return OUString();
    return s_encodings[2*pos];
}

}

WPFTEncodingDialog::WPFTEncodingDialog(
    const OUString &title, const OUString &encoding)
    :   ModalDialog(nullptr, "WPFTEncodingDialog", "writerperfect/ui/wpftencodingdialog.ui"),
        m_pLbCharset(), m_pBtnOk(), m_pBtnCancel(), m_userHasCancelled(false)
{
    get(m_pLbCharset, "comboboxtext");
    get(m_pBtnOk, "ok");
    get(m_pBtnCancel, "cancel");

    m_pBtnCancel->SetClickHdl(LINK(this, WPFTEncodingDialog, CancelHdl));

    EncodingImplementation::insertEncodings(m_pLbCharset);
    m_pLbCharset->SetStyle(m_pLbCharset->GetStyle() | WB_SORT);
    // m_pLbCharset->set_height_request(6 * m_pLbCharset->GetTextHeight());
    m_pLbCharset->SetDoubleClickHdl(LINK(this, WPFTEncodingDialog, DoubleClickHdl));
    EncodingImplementation::selectEncoding(m_pLbCharset, encoding);
    m_pLbCharset->Show();

    SetText(title);
}

WPFTEncodingDialog::~WPFTEncodingDialog()
{
    disposeOnce();
}

OUString WPFTEncodingDialog::GetEncoding() const
{
    return EncodingImplementation::getEncoding(m_pLbCharset);
}

IMPL_LINK_NOARG(WPFTEncodingDialog, CancelHdl)
{
    m_userHasCancelled=true;
    Close();
    return 0;
}

IMPL_LINK(WPFTEncodingDialog, DoubleClickHdl, ListBox *, pLb)
{
    if (pLb == m_pLbCharset)
    {
        m_pBtnOk->Click();
    }
    return 0;
}

void WPFTEncodingDialog::dispose()
{
    m_pLbCharset.disposeAndClear();
    m_pBtnOk.disposeAndClear();
    m_pBtnCancel.disposeAndClear();
    ModalDialog::dispose();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
