/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <prewin.h>
#include <postwin.h>

#include <comphelper/windowserrorstring.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <o3tl/char16_t2wchar_t.hxx>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <spsuppStrings.hrc>
#include <systools/win32/extended_max_path.hxx>
#include <unotools/resmgr.hxx>
#include "res/spsuppDlg.h"

// Since we need to show localized messages to user before starting LibreOffice, we need to
// bootstrap part of LO (l10n machinery). This implies loading some LO libraries, and since
// there are ActiveX controls for both x86 and x64 for use in corresponding clients, they
// can't both load the libraries that exist only for one architecture, like sal. Thus we need
// a dedicated helper process, which is launched by ActiveX, and handle user interactions.

namespace
{
const OUString& GetSofficeExe()
{
    static const OUString s_sPath = []() {
        OUString result;
        wchar_t sPath[EXTENDED_MAX_PATH];
        if (GetModuleFileNameW(nullptr, sPath, std::size(sPath)) == 0)
            return result;
        wchar_t* pSlashPos = wcsrchr(sPath, L'\\');
        if (pSlashPos == nullptr)
            return result;
        wcscpy(pSlashPos + 1, L"soffice.exe");
        result = o3tl::toU(sPath);
        return result;
    }();
    return s_sPath;
}

OUString GetString(TranslateId pResId)
{
    static const std::locale s_pLocale = [] {
        // Initialize soffice bootstrap: see getIniFileName_Impl for reference
        OUString sPath = GetSofficeExe();
        if (const sal_Int32 nDotPos = sPath.lastIndexOf('.'); nDotPos >= 0)
        {
            sPath = sPath.replaceAt(nDotPos, sPath.getLength() - nDotPos, SAL_CONFIGFILE(u""));
            if (osl::FileBase::getFileURLFromSystemPath(sPath, sPath) == osl::FileBase::E_None)
                rtl::Bootstrap::setIniFilename(sPath);
        }
        return Translate::Create("shell", LanguageTag("")); // Use system language
    }();
    return Translate::get(pResId, s_pLocale);
}

INT_PTR CALLBACK EditOrRODlgproc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch (Msg)
    {
        case WM_INITDIALOG:
        {
            if (const wchar_t* sFilePath = reinterpret_cast<const wchar_t*>(lParam))
            {
                OUString sMsg = GetString(RID_STR_SP_VIEW_OR_EDIT_TITLE);
                SetWindowTextW(hDlg, o3tl::toW(sMsg.getStr()));
                sMsg = GetString(RID_STR_SP_VIEW_OR_EDIT_MESSAGE)
                           .replaceFirst("%DOCNAME", o3tl::toU(sFilePath));
                SetWindowTextW(GetDlgItem(hDlg, IDC_EDIT_OR_RO), o3tl::toW(sMsg.getStr()));
                sMsg = GetString(RID_STR_SP_VIEW_OR_EDIT_VIEW);
                SetWindowTextW(GetDlgItem(hDlg, ID_RO), o3tl::toW(sMsg.getStr()));
                sMsg = GetString(RID_STR_SP_VIEW_OR_EDIT_EDIT);
                SetWindowTextW(GetDlgItem(hDlg, ID_EDIT), o3tl::toW(sMsg.getStr()));
                sMsg = GetString(RID_STR_SP_VIEW_OR_EDIT_CANCEL);
                SetWindowTextW(GetDlgItem(hDlg, IDCANCEL), o3tl::toW(sMsg.getStr()));
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

enum class Answer
{
    Cancel,
    ReadOnly,
    Edit
};

Answer AskIfUserWantsToEdit(const wchar_t* sFilePath)
{
    Answer res = Answer::Cancel;
    INT_PTR nResult = DialogBoxParamW(nullptr, MAKEINTRESOURCEW(IDD_EDIT_OR_RO), nullptr,
                                      EditOrRODlgproc, reinterpret_cast<LPARAM>(sFilePath));
    if (nResult == ID_RO)
        res = Answer::ReadOnly;
    else if (nResult == ID_EDIT)
        res = Answer::Edit;
    return res;
}

// Returns ERROR_SUCCESS or Win32 error code
DWORD LOStart(const wchar_t* sModeArg, const wchar_t* sFilePath)
{
    OUString sCmdLine = "\"" + GetSofficeExe() + "\" " + o3tl::toU(sModeArg) + " \""
                        + o3tl::toU(sFilePath) + "\"";
    LPWSTR pCmdLine = const_cast<LPWSTR>(o3tl::toW(sCmdLine.getStr()));

    STARTUPINFOW si{ .cb = sizeof(si), .dwFlags = STARTF_USESHOWWINDOW, .wShowWindow = SW_SHOW };
    PROCESS_INFORMATION pi{};
    if (!CreateProcessW(nullptr, pCmdLine, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
    {
        DWORD dwError = GetLastError();
        const OUString sErrorMsg = "Could not start LibreOffice. Error is 0x"
                                   + OUString::number(dwError, 16) + ":\n\n"
                                   + comphelper::WindowsErrorString(dwError);

        // Report the error to user and return error
        MessageBoxW(nullptr, o3tl::toW(sErrorMsg.getStr()), nullptr, MB_ICONERROR);
        return dwError;
    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return ERROR_SUCCESS;
}

int CreateNewDocument(LPCWSTR TemplateLocation, LPCWSTR /*DefaultSaveLocation*/)
{
    // TODO: resolve the program from varProgID (nullptr -> default?)
    DWORD nResult = LOStart(L"-n", TemplateLocation);
    return nResult == ERROR_SUCCESS ? 0 : 2;
}

// UseLocalCopy would be either "0" or "1", denoting boolean value
int EditDocument(LPCWSTR DocumentLocation, LPCWSTR /*UseLocalCopy*/, LPCWSTR /*varProgID*/)
{
    // TODO: resolve the program from varProgID (nullptr -> default?)
    DWORD nResult = LOStart(L"-o", DocumentLocation);
    return nResult == ERROR_SUCCESS ? 0 : 2;
}

// Possible values for OpenType
//
// "0" When checked out, or when the document library does not require check out, the user can read or edit the document
// "1" When another user has checked it out, the user can only read the document
// "2" When the current user has checked it out, the user can only edit the document
// "3" When the document is not checked out and the document library requires that documents be checked out to be edited, the user can only read the document, or check it out and edit it
// "4" When the current user has checked it out, the user can only edit the local copy of the document
int ViewDocument(LPCWSTR DocumentLocation, LPCWSTR OpenType, LPCWSTR varProgID)
{
    if (wcscmp(OpenType, L"0") == 0)
    {
        switch (AskIfUserWantsToEdit(DocumentLocation))
        {
            case Answer::Cancel:
                return 1;
            case Answer::ReadOnly:
                break;
            case Answer::Edit:
                return EditDocument(DocumentLocation, L"0", varProgID);
        }
    }
    // TODO: resolve the program from varProgID (nullptr -> default?)
    DWORD nResult = LOStart(L"--view", DocumentLocation);
    return nResult == ERROR_SUCCESS ? 0 : 2;
}
} // namespace

// Returns 0 on success, 1 when operation wasn't performed because user cancelled, 2 on an error
int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
    int argc = 0;
    const LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argc < 2)
        return 2; // Wrong argument count

    if (wcscmp(argv[1], L"CreateNewDocument") == 0)
    {
        if (argc != 4)
            return 2; // Wrong argument count
        return CreateNewDocument(argv[2], argv[3]);
    }

    if (wcscmp(argv[1], L"ViewDocument") == 0)
    {
        if (argc != 4 && argc != 5)
            return 2; // Wrong argument count
        LPCWSTR pProgId = argc == 5 ? argv[4] : nullptr;
        return ViewDocument(argv[2], argv[3], pProgId);
    }

    if (wcscmp(argv[1], L"EditDocument") == 0)
    {
        if (argc != 4 && argc != 5)
            return 2; // Wrong argument count
        LPCWSTR pProgId = argc == 5 ? argv[4] : nullptr;
        return EditDocument(argv[2], argv[3], pProgId);
    }

    return 2; // Wrong command
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
