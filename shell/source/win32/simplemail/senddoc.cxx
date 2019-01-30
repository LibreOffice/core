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

#include <osl/diagnose.h>
#include <sal/macros.h>

#include <o3tl/char16_t2wchar_t.hxx>
#include <rtl/bootstrap.hxx>
#include <sfx2/strings.hrc>
#include <unotools/resmgr.hxx>
#include <i18nlangtag/languagetag.hxx>

#include <wchar.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mapi.h>
#include <MapiUnicodeHelp.h>

#include <string>
#include <vector>
#if OSL_DEBUG_LEVEL > 0
#include <sstream>
#endif
#include <stdexcept>

#if OSL_DEBUG_LEVEL > 0
    static void dumpParameter();
#endif

typedef std::vector<MapiRecipDescW> MapiRecipientList_t;
typedef std::vector<MapiFileDescW> MapiAttachmentList_t;

const int LEN_SMTP_PREFIX = 5; // "SMTP:"

namespace /* private */
{
    OUString gLangTag;
    OUString gBootstrap;
    std::wstring gFrom;
    std::wstring gSubject;
    std::wstring gBody;
    std::vector<std::wstring> gTo;
    std::vector<std::wstring> gCc;
    std::vector<std::wstring> gBcc;
    // Keep temp filepath and displayed name
    std::vector<std::pair<std::wstring, std::wstring>> gAttachments;
    int gMapiFlags = 0;
}

/**
    Add a prefix to an email address. MAPI requires that
    email addresses have an 'SMTP:' prefix.

    @param  aEmailAddress
    [in] the email address.

    @param  aPrefix
    [in] the prefix to be added to the email address.

    @returns
    the email address prefixed with the specified prefix.
*/
static std::wstring prefixEmailAddress(
    const std::wstring& aEmailAddress,
    const std::wstring& aPrefix = L"SMTP:")
{
    return (aPrefix + aEmailAddress);
}

/** @internal */
static void addRecipient(
    ULONG recipClass,
    const std::wstring& recipAddress,
    MapiRecipientList_t* pMapiRecipientList)
{
    MapiRecipDescW mrd;
    ZeroMemory(&mrd, sizeof(mrd));

    mrd.ulRecipClass = recipClass;
    mrd.lpszName = const_cast<wchar_t*>(recipAddress.c_str()) + LEN_SMTP_PREFIX;
    mrd.lpszAddress = const_cast<wchar_t*>(recipAddress.c_str());
    pMapiRecipientList->push_back(mrd);
}

/** @internal */
static void initRecipientList(MapiRecipientList_t* pMapiRecipientList)
{
    OSL_ASSERT(pMapiRecipientList->empty());

    // add to recipients
    for (const auto& address : gTo)
        addRecipient(MAPI_TO, address, pMapiRecipientList);

    // add cc recipients
    for (const auto& address : gCc)
        addRecipient(MAPI_CC, address, pMapiRecipientList);

    // add bcc recipients
    for (const auto& address : gBcc)
        addRecipient(MAPI_BCC, address, pMapiRecipientList);
}

/** @internal */
static void initAttachmentList(MapiAttachmentList_t* pMapiAttachmentList)
{
    OSL_ASSERT(pMapiAttachmentList->empty());

    for (const auto& attachment : gAttachments)
    {
        MapiFileDescW mfd;
        ZeroMemory(&mfd, sizeof(mfd));
        mfd.lpszPathName = const_cast<wchar_t*>(attachment.first.c_str());
        // MapiFileDesc documentation (https://msdn.microsoft.com/en-us/library/hh707272)
        // allows using here either nullptr, or a pointer to empty string. However,
        // for Outlook 2013, we cannot use nullptr here, and must point to a (possibly
        // empty) string: otherwise using MAPI_DIALOG_MODELESS results in MAPI_E_FAILURE.
        // See http://peach.ease.lsoft.com/scripts/wa-PEACH.exe?A2=MAPI-L;d2bf3060.1604
        // Since C++11, c_str() must return a pointer to single null character when the
        // string is empty, so we are OK here in case when there's no explicit file name
        // passed
        mfd.lpszFileName = const_cast<wchar_t*>(attachment.second.c_str());
        mfd.nPosition = sal::static_int_cast<ULONG>(-1);
        pMapiAttachmentList->push_back(mfd);
    }
}

/** @internal */
static void initMapiOriginator(MapiRecipDescW* pMapiOriginator)
{
    ZeroMemory(pMapiOriginator, sizeof(*pMapiOriginator));

    pMapiOriginator->ulRecipClass = MAPI_ORIG;
    pMapiOriginator->lpszName = const_cast<wchar_t*>(L"");
    pMapiOriginator->lpszAddress = const_cast<wchar_t*>(gFrom.c_str());
}

/** @internal */
static void initMapiMessage(
    MapiRecipDescW* aMapiOriginator,
    MapiRecipientList_t& aMapiRecipientList,
    MapiAttachmentList_t& aMapiAttachmentList,
    MapiMessageW* pMapiMessage)
{
    ZeroMemory(pMapiMessage, sizeof(*pMapiMessage));

    pMapiMessage->lpszSubject = const_cast<wchar_t*>(gSubject.c_str());
    pMapiMessage->lpszNoteText = (gBody.length() ? const_cast<wchar_t*>(gBody.c_str()) : nullptr);
    pMapiMessage->lpOriginator = aMapiOriginator;
    pMapiMessage->lpRecips = aMapiRecipientList.size() ? &aMapiRecipientList[0] : nullptr;
    pMapiMessage->nRecipCount = aMapiRecipientList.size();
    if (!aMapiAttachmentList.empty())
        pMapiMessage->lpFiles = &aMapiAttachmentList[0];
    pMapiMessage->nFileCount = aMapiAttachmentList.size();
}

const wchar_t* const KnownParameters[] =
{
    L"--to",
    L"--cc",
    L"--bcc",
    L"--from",
    L"--subject",
    L"--body",
    L"--attach",
    L"--mapi-dialog",
    L"--mapi-logon-ui",
    L"--langtag",
    L"--bootstrap",
};

/** @internal */
static bool isKnownParameter(const wchar_t* aParameterName)
{
    for (const wchar_t* KnownParameter : KnownParameters)
        if (_wcsicmp(aParameterName, KnownParameter) == 0)
            return true;

    return false;
}

/** @internal */
static void initParameter(int argc, wchar_t* argv[])
{
    for (int i = 1; i < argc; i++)
    {
        if (!isKnownParameter(argv[i]))
        {
            OSL_FAIL("Wrong parameter received");
            continue;
        }

        if (_wcsicmp(argv[i], L"--mapi-dialog") == 0)
        {
            // Outlook 2013+; for earlier versions this equals to MAPI_DIALOG
            gMapiFlags |= MAPI_DIALOG_MODELESS;
        }
        else if (_wcsicmp(argv[i], L"--mapi-logon-ui") == 0)
        {
            gMapiFlags |= MAPI_LOGON_UI;
        }
        else if ((i+1) < argc) // is the value of a parameter available too?
        {
            if (_wcsicmp(argv[i], L"--to") == 0)
                gTo.push_back(prefixEmailAddress(argv[i+1]));
            else if (_wcsicmp(argv[i], L"--cc") == 0)
                gCc.push_back(prefixEmailAddress(argv[i+1]));
            else if (_wcsicmp(argv[i], L"--bcc") == 0)
                gBcc.push_back(prefixEmailAddress(argv[i+1]));
            else if (_wcsicmp(argv[i], L"--from") == 0)
                gFrom = prefixEmailAddress(argv[i+1]);
            else if (_wcsicmp(argv[i], L"--subject") == 0)
                gSubject = argv[i+1];
            else if (_wcsicmp(argv[i], L"--body") == 0)
                gBody = argv[i+1];
            else if (_wcsicmp(argv[i], L"--attach") == 0)
            {
                std::wstring sPath(argv[i + 1]);
                // An attachment may optionally be immediately followed by --attach-name and user-visible name
                std::wstring sName;
                if ((i + 3) < argc && _wcsicmp(argv[i+2], L"--attach-name") == 0)
                {
                    sName = argv[i+3];
                    i += 2;
                }
                gAttachments.emplace_back(sPath, sName);
            }
            else if (_wcsicmp(argv[i], L"--langtag") == 0)
                gLangTag = o3tl::toU(argv[i+1]);
            else if (_wcsicmp(argv[i], L"--bootstrap") == 0)
                gBootstrap = o3tl::toU(argv[i+1]);

            i++;
        }
    }
}

static void ShowError(ULONG nMAPIResult)
{
    if (!gBootstrap.isEmpty())
        rtl::Bootstrap::setIniFilename(gBootstrap);
    LanguageTag aLangTag(gLangTag);
    std::locale aLocale = Translate::Create("sfx", aLangTag);
    OUString sMessage = Translate::get(STR_ERROR_SEND_MAIL_CODE, aLocale);
    OUString sErrorId;
    switch (nMAPIResult)
    {
    case MAPI_E_FAILURE:
        sErrorId = "MAPI_E_FAILURE";
        break;
    case MAPI_E_LOGON_FAILURE:
        sErrorId = "MAPI_E_LOGON_FAILURE";
        break;
    case MAPI_E_DISK_FULL:
        sErrorId = "MAPI_E_DISK_FULL";
        break;
    case MAPI_E_INSUFFICIENT_MEMORY:
        sErrorId = "MAPI_E_INSUFFICIENT_MEMORY";
        break;
    case MAPI_E_ACCESS_DENIED:
        sErrorId = "MAPI_E_ACCESS_DENIED";
        break;
    case MAPI_E_TOO_MANY_SESSIONS:
        sErrorId = "MAPI_E_ACCESS_DENIED";
        break;
    case MAPI_E_TOO_MANY_FILES:
        sErrorId = "MAPI_E_TOO_MANY_FILES";
        break;
    case MAPI_E_TOO_MANY_RECIPIENTS:
        sErrorId = "MAPI_E_TOO_MANY_RECIPIENTS";
        break;
    case MAPI_E_ATTACHMENT_NOT_FOUND:
        sErrorId = "MAPI_E_ATTACHMENT_NOT_FOUND";
        break;
    case MAPI_E_ATTACHMENT_OPEN_FAILURE:
        sErrorId = "MAPI_E_ATTACHMENT_OPEN_FAILURE";
        break;
    case MAPI_E_ATTACHMENT_WRITE_FAILURE:
        sErrorId = "MAPI_E_ATTACHMENT_WRITE_FAILURE";
        break;
    case MAPI_E_UNKNOWN_RECIPIENT:
        sErrorId = "MAPI_E_UNKNOWN_RECIPIENT";
        break;
    case MAPI_E_BAD_RECIPTYPE:
        sErrorId = "MAPI_E_BAD_RECIPTYPE";
        break;
    case MAPI_E_NO_MESSAGES:
        sErrorId = "MAPI_E_NO_MESSAGES";
        break;
    case MAPI_E_INVALID_MESSAGE:
        sErrorId = "MAPI_E_INVALID_MESSAGE";
        break;
    case MAPI_E_TEXT_TOO_LARGE:
        sErrorId = "MAPI_E_TEXT_TOO_LARGE";
        break;
    case MAPI_E_INVALID_SESSION:
        sErrorId = "MAPI_E_INVALID_SESSION";
        break;
    case MAPI_E_TYPE_NOT_SUPPORTED:
        sErrorId = "MAPI_E_TYPE_NOT_SUPPORTED";
        break;
    case MAPI_E_AMBIGUOUS_RECIPIENT:
        sErrorId = "MAPI_E_AMBIGUOUS_RECIPIENT";
        break;
    case MAPI_E_MESSAGE_IN_USE:
        sErrorId = "MAPI_E_MESSAGE_IN_USE";
        break;
    case MAPI_E_NETWORK_FAILURE:
        sErrorId = "MAPI_E_NETWORK_FAILURE";
        break;
    case MAPI_E_INVALID_EDITFIELDS:
        sErrorId = "MAPI_E_INVALID_EDITFIELDS";
        break;
    case MAPI_E_INVALID_RECIPS:
        sErrorId = "MAPI_E_INVALID_RECIPS";
        break;
    case MAPI_E_NOT_SUPPORTED:
        sErrorId = "MAPI_E_NOT_SUPPORTED";
        break;
    case MAPI_E_UNICODE_NOT_SUPPORTED:
        sErrorId = "MAPI_E_UNICODE_NOT_SUPPORTED";
        break;
    default:
        sErrorId = OUString::number(nMAPIResult);
    }
    sMessage = sMessage.replaceAll("$1", sErrorId);
    OUString sTitle(Translate::get(STR_ERROR_SEND_MAIL_HEADER, aLocale));

    MessageBoxW(nullptr, o3tl::toW(sMessage.getStr()), o3tl::toW(sTitle.getStr()),
        MB_OK | MB_ICONINFORMATION);
}

/**
    Main.
    NOTE: Because this is program only serves implementation
    purposes and should not be used by any end user the
    parameter checking is very limited. Every unknown parameter
    will be ignored.
*/
int wmain(int argc, wchar_t* argv[])
{

    initParameter(argc, argv);

#if OSL_DEBUG_LEVEL > 0
    dumpParameter();
#endif

    ULONG ulRet = MAPI_E_FAILURE;

    try
    {
        LHANDLE const hSession = 0;

        MapiRecipDescW mapiOriginator;
        MapiRecipientList_t mapiRecipientList;
        MapiAttachmentList_t mapiAttachmentList;
        MapiMessageW mapiMsg;

        initMapiOriginator(&mapiOriginator);
        initRecipientList(&mapiRecipientList);
        initAttachmentList(&mapiAttachmentList);
        initMapiMessage((gFrom.length() ? &mapiOriginator : nullptr), mapiRecipientList, mapiAttachmentList, &mapiMsg);

        ulRet = MAPISendMailHelper(hSession, 0, &mapiMsg, gMapiFlags, 0);

        // There is no point in treating an aborted mail sending
        // dialog as an error to be returned as our exit
        // status. If the user decided to abort sending a document
        // as mail, OK, that is not an error.

        // Also, it seems that GroupWise makes MAPISendMail()
        // return MAPI_E_USER_ABORT even if the mail sending
        // dialog was not aborted by the user, and the mail was
        // actually sent just fine. See bnc#660241 (visible to
        // Novell people only, sorry).

        if (ulRet == MAPI_E_USER_ABORT)
            ulRet = SUCCESS_SUCCESS;

    }
    catch (const std::runtime_error& ex)
    {
        OSL_FAIL(ex.what());
    }

    // Now cleanup the temporary attachment files
    for (const auto& rAttachment : gAttachments)
        DeleteFileW(rAttachment.first.c_str());

    // Only show the error message if UI was requested
    if ((ulRet != SUCCESS_SUCCESS) && (gMapiFlags & (MAPI_DIALOG | MAPI_LOGON_UI)))
        ShowError(ulRet);

    return ulRet;
}

#if OSL_DEBUG_LEVEL > 0
    void dumpParameter()
    {
        std::wostringstream oss;

        if (gFrom.length() > 0)
            oss << "--from " << gFrom << std::endl;

        if (gSubject.length() > 0)
            oss << "--subject " << gSubject << std::endl;

        if (gBody.length() > 0)
            oss << "--body " << gBody << std::endl;

        for (const auto& address : gTo)
            oss << "--to " << address << std::endl;

        for (const auto& address : gCc)
            oss << "--cc " << address << std::endl;

        for (const auto& address : gBcc)
            oss << "--bcc " << address << std::endl;

        for (const auto& attachment : gAttachments)
        {
            oss << "--attach " << attachment.first << std::endl;
            if (!attachment.second.empty())
                oss << "--attach-name " << attachment.second << std::endl;
        }

        if (gMapiFlags & MAPI_DIALOG)
            oss << "--mapi-dialog" << std::endl;

        if (gMapiFlags & MAPI_LOGON_UI)
            oss << "--mapi-logon-ui" << std::endl;

        if (!gLangTag.isEmpty())
            oss << "--langtag " << gLangTag << std::endl;

        if (!gBootstrap.isEmpty())
            oss << "--bootstrap " << gBootstrap << std::endl;

        MessageBoxW(nullptr, oss.str().c_str(), L"Arguments", MB_OK | MB_ICONINFORMATION);
    }
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
