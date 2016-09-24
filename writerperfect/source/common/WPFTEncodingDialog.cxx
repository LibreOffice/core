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

std::pair<OUStringLiteral, OUStringLiteral> const s_encodings[] =
{
    {OUStringLiteral("MacArabic"), OUStringLiteral("Arabic (Apple Macintosh)")},
    {OUStringLiteral("CP864"), OUStringLiteral("Arabic (DOS/OS2-864)")},
    {OUStringLiteral("CP1006"), OUStringLiteral("Arabic (IBM-1006)")},
    {OUStringLiteral("CP1256"), OUStringLiteral("Arabic (Windows-1256)")},
    {OUStringLiteral("CP775"), OUStringLiteral("Baltic (DOS/OS2-775)")},
    {OUStringLiteral("CP1257"), OUStringLiteral("Baltic (Windows-1257)")},
    {OUStringLiteral("MacCeltic"), OUStringLiteral("Celtic (Apple Macintosh)")},
    {
        OUStringLiteral("MacCyrillic"),
        OUStringLiteral("Cyrillic (Apple Macintosh)")
    },
    {OUStringLiteral("CP855"), OUStringLiteral("Cyrillic (DOS/OS2-855)")},
    {
        OUStringLiteral("CP866"),
        OUStringLiteral("Cyrillic (DOS/OS2-866/Russian)")
    },
    {OUStringLiteral("CP1251"), OUStringLiteral("Cyrillic (Windows-1251)")},
    {
        OUStringLiteral("MacCEurope"),
        OUStringLiteral("Eastern Europe (Apple Macintosh)")
    },
    {
        OUStringLiteral("MacCroatian"),
        OUStringLiteral("Eastern Europe (Apple Macintosh/Croatian)")
    },
    {
        OUStringLiteral("MacRomanian"),
        OUStringLiteral("Eastern Europe (Apple Macintosh/Romanian)")
    },
    {OUStringLiteral("CP852"), OUStringLiteral("Eastern Europe (DOS/OS2-852)")},
    {
        OUStringLiteral("CP1250"),
        OUStringLiteral("Eastern Europe (Windows-1250/WinLatin 2)")
    },
    {OUStringLiteral("MacGreek"), OUStringLiteral("Greek (Apple Macintosh)")},
    {OUStringLiteral("CP737"), OUStringLiteral("Greek (DOS/OS2-737)")},
    {OUStringLiteral("CP869"), OUStringLiteral("Greek (DOS/OS2-869/Greek-2)")},
    {OUStringLiteral("CP875"), OUStringLiteral("Greek (DOS/OS2-875)")},
    {OUStringLiteral("CP1253"), OUStringLiteral("Greek (Windows-1253)")},
    {OUStringLiteral("MacHebrew"), OUStringLiteral("Hebrew (Apple Macintosh)")},
    {OUStringLiteral("CP424"), OUStringLiteral("Hebrew (DOS/OS2-424)")},
    {OUStringLiteral("CP856"), OUStringLiteral("Hebrew (DOS/OS2-856)")},
    {OUStringLiteral("CP862"), OUStringLiteral("Hebrew (DOS/OS2-862)")},
    {OUStringLiteral("CP1255"), OUStringLiteral("Hebrew (Windows-1255)")},
    {OUStringLiteral("CP500"), OUStringLiteral("International (DOS/OS2-500)")},
    {OUStringLiteral("CP932"), OUStringLiteral("Japanese (Windows-932)")},
    {OUStringLiteral("MacThai"), OUStringLiteral("Thai (Apple Macintosh)")},
    {OUStringLiteral("CP874"), OUStringLiteral("Thai (DOS/OS2-874)")},
    {
        OUStringLiteral("CP950"),
        OUStringLiteral("Traditional Chinese (Windows-950)")
    },
    {
        OUStringLiteral("MacTurkish"),
        OUStringLiteral("Turkish (Apple Macintosh)")
    },
    {OUStringLiteral("CP857"), OUStringLiteral("Turkish (DOS/OS2-857)")},
    {OUStringLiteral("CP1026"), OUStringLiteral("Turkish (DOS/OS2-1026)")},
    {OUStringLiteral("CP1254"), OUStringLiteral("Turkish (Windows-1254)")},
    {OUStringLiteral("CP1258"), OUStringLiteral("Vietnamese (Windows-1258)")},
    {
        OUStringLiteral("MacRoman"),
        OUStringLiteral("Western Europe (Apple Macintosh)")
    },
    {
        OUStringLiteral("MacIceland"),
        OUStringLiteral("Western Europe (Apple Macintosh/Icelandic)")
    },
    {
        OUStringLiteral("CP037"),
        OUStringLiteral("Western Europe (DOS/OS2-037/US-Canada)")
    },
    {
        OUStringLiteral("CP437"),
        OUStringLiteral("Western Europe (DOS/OS2-437/US)")
    },
    {OUStringLiteral("CP850"), OUStringLiteral("Western Europe (DOS/OS2-850)")},
    {
        OUStringLiteral("CP860"),
        OUStringLiteral("Western Europe (DOS/OS2-860/Portuguese)")
    },
    {
        OUStringLiteral("CP861"),
        OUStringLiteral("Western Europe (DOS/OS2-861/Icelandic)")
    },
    {
        OUStringLiteral("CP863"),
        OUStringLiteral("Western Europe (DOS/OS2-863/French)")
    },
    {
        OUStringLiteral("CP865"),
        OUStringLiteral("Western Europe (DOS/OS2-865/Nordic)")
    },
    {
        OUStringLiteral("CP1252"),
        OUStringLiteral("Western Europe (Windows-1252/WinLatin 1)")
    }
};

std::size_t const numEncodings = SAL_N_ELEMENTS(s_encodings);

void insertEncodings(ListBox *box)
{
    for (std::size_t i=0; i<numEncodings; ++i)
    {
        sal_IntPtr nAt=box->InsertEntry(s_encodings[i].second);
        box->SetEntryData(
            nAt, reinterpret_cast<void *>(static_cast<sal_uIntPtr>(i)));
    }
}

void selectEncoding(ListBox *box, const OUString &encoding)
{
    for (std::size_t i=0; i<numEncodings; ++i)
    {
        if (encoding!=s_encodings[i].first) continue;
        box->SelectEntryPos(i);
        return;
    }
}

OUString getEncoding(ListBox *box)
{
    sal_uIntPtr pos = reinterpret_cast<sal_uIntPtr>(box->GetSelectEntryData());
    if (pos>=numEncodings)
        return OUString();
    return s_encodings[pos].first;
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

    insertEncodings(m_pLbCharset);
    m_pLbCharset->SetStyle(m_pLbCharset->GetStyle() | WB_SORT);
    // m_pLbCharset->set_height_request(6 * m_pLbCharset->GetTextHeight());
    m_pLbCharset->SetDoubleClickHdl(LINK(this, WPFTEncodingDialog, DoubleClickHdl));
    selectEncoding(m_pLbCharset, encoding);
    m_pLbCharset->Show();

    SetText(title);
}

WPFTEncodingDialog::~WPFTEncodingDialog()
{
    disposeOnce();
}

OUString WPFTEncodingDialog::GetEncoding() const
{
    return getEncoding(m_pLbCharset);
}

IMPL_LINK_NOARG_TYPED(WPFTEncodingDialog, CancelHdl, Button *, void)
{
    m_userHasCancelled=true;
    Close();
}

IMPL_LINK_TYPED(WPFTEncodingDialog, DoubleClickHdl, ListBox &, rLb, void)
{
    if (&rLb == m_pLbCharset)
    {
        m_pBtnOk->Click();
    }
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
