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
    OUString("MacArabic"), OUString("Arabic (Apple Macintosh)"),
    OUString("CP864"), OUString("Arabic (DOS/OS2-864)"),
    OUString("CP1006"), OUString("Arabic (IBM-1006)"),
    OUString("CP1256"), OUString("Arabic (Windows-1256)"),
    OUString("CP775"), OUString("Baltic (DOS/OS2-775)"),
    OUString("CP1257"), OUString("Baltic (Windows-1257)"),
    OUString("MacCeltic"), OUString("Celtic (Apple Macintosh)"),
    OUString("MacCyrillic"), OUString("Cyrillic (Apple Macintosh)"),
    OUString("CP855"), OUString("Cyrillic (DOS/OS2-855)"),
    OUString("CP866"), OUString("Cyrillic (DOS/OS2-866/Russian)"),
    OUString("CP1251"), OUString("Cyrillic (Windows-1251)"),
    OUString("MacCEurope"), OUString("Eastern Europe (Apple Macintosh)"),
    OUString("MacCroatian"), OUString("Eastern Europe (Apple Macintosh/Croatian)"),
    OUString("MacRomanian"), OUString("Eastern Europe (Apple Macintosh/Romanian)"),
    OUString("CP852"), OUString("Eastern Europe (DOS/OS2-852)"),
    OUString("CP1250"), OUString("Eastern Europe (Windows-1250/WinLatin 2)"),
    OUString("MacGreek"), OUString("Greek (Apple Macintosh)"),
    OUString("CP737"), OUString("Greek (DOS/OS2-737)"),
    OUString("CP869"), OUString("Greek (DOS/OS2-869/Greek-2)"),
    OUString("CP875"), OUString("Greek (DOS/OS2-875)"),
    OUString("CP1253"), OUString("Greek (Windows-1253)"),
    OUString("MacHebrew"), OUString("Hebrew (Apple Macintosh)"),
    OUString("CP424"), OUString("Hebrew (DOS/OS2-424)"),
    OUString("CP856"), OUString("Hebrew (DOS/OS2-856)"),
    OUString("CP862"), OUString("Hebrew (DOS/OS2-862)"),
    OUString("CP1255"), OUString("Hebrew (Windows-1255)"),
    OUString("CP500"), OUString("International (DOS/OS2-500)"),
    OUString("MacThai"), OUString("Thai (Apple Macintosh)"),
    OUString("CP874"), OUString("Thai (DOS/OS2-874)"),
    OUString("MacTurkish"), OUString("Turkish (Apple Macintosh)"),
    OUString("CP857"), OUString("Turkish (DOS/OS2-857)"),
    OUString("CP1026"), OUString("Turkish (DOS/OS2-1026)"),
    OUString("CP1254"), OUString("Turkish (Windows-1254)"),
    OUString("CP1258"), OUString("Vietnamese (Windows-1258)"),
    OUString("MacRoman"), OUString("Western Europe (Apple Macintosh)"),
    OUString("MacIceland"), OUString("Western Europe (Apple Macintosh/Icelandic)"),
    OUString("CP037"), OUString("Western Europe (DOS/OS2-037/US-Canada)"),
    OUString("CP437"), OUString("Western Europe (DOS/OS2-437/US)"),
    OUString("CP850"), OUString("Western Europe (DOS/OS2-850)"),
    OUString("CP860"), OUString("Western Europe (DOS/OS2-860/Portuguese)"),
    OUString("CP861"), OUString("Western Europe (DOS/OS2-861/Icelandic)"),
    OUString("CP863"), OUString("Western Europe (DOS/OS2-863/French)"),
    OUString("CP865"), OUString("Western Europe (DOS/OS2-865/Nordic)"),
    OUString("CP1252"), OUString("Western Europe (Windows-1252/WinLatin 1)")
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
