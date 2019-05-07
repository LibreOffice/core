/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <memory>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <i18nlangtag/languagetag.hxx>
#include <o3tl/char16_t2wchar_t.hxx>
#include <rtl/bootstrap.hxx>
#include <spsuppStrings.hrc>
#include <unotools/resmgr.hxx>

#include <spsuppServ.hpp>
#include "res/resource.h"

namespace
{
OUString GetString(const char* pResId)
{
    static const std::locale* s_pLocale = []() -> const std::locale* {
        // Initialize soffice bootstrap: see getIniFileName_Impl for reference
        OUString sPath = o3tl::toU(GetLOPath());
        if (const sal_Int32 nDotPos = sPath.lastIndexOf('.'); nDotPos >= 0)
        {
            sPath = sPath.replaceAt(nDotPos, sPath.getLength() - nDotPos, SAL_CONFIGFILE(""));
            rtl::Bootstrap::setIniFilename(sPath);
            static const std::locale s_aLocale(Translate::Create("shell", LanguageTag("")));
            return &s_aLocale;
        }
        return nullptr;
    }();
    return s_pLocale ? Translate::get(pResId, *s_pLocale) : OUString();
}

INT_PTR CALLBACK EditOrRODlgproc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch (Msg)
    {
        case WM_INITDIALOG:
        {
            if (const wchar_t* sFilePath = reinterpret_cast<const wchar_t*>(lParam))
            {
                OUString sMsg = GetString(RID_STR_SP_VIEW_OR_EDIT)
                                    .replaceFirst("%DOCNAME", o3tl::toU(sFilePath));
                SetWindowTextW(GetDlgItem(hDlg, IDC_EDIT_OR_RO), o3tl::toW(sMsg.getStr()));
            }
            return TRUE; // set default focus
        }
        case WM_COMMAND:
        {
            WORD nId = LOWORD(wParam);
            switch (nId)
            {
                case IDCANCEL:
                case ID_RO:
                case ID_EDIT:
                    EndDialog(hDlg, nId);
                    return TRUE;
            }
            break;
        }
    }
    return FALSE;
}
} // namespace

Answer AskIfUserWantsToEdit(const wchar_t* sFilePath)
{
    Answer res = Answer::Cancel;
    INT_PTR nResult = DialogBoxParamW(GetHModule(), MAKEINTRESOURCEW(IDD_EDIT_OR_RO), nullptr,
                                      EditOrRODlgproc, reinterpret_cast<LPARAM>(sFilePath));
    if (nResult == ID_RO)
        res = Answer::ReadOnly;
    else if (nResult == ID_EDIT)
        res = Answer::Edit;
    return res;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
