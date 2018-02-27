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
    void dumpParameter();
#endif

typedef std::vector<std::wstring> StringList_t;
typedef StringList_t::const_iterator StringListIterator_t;
typedef std::vector<MapiRecipDescW> MapiRecipientList_t;
typedef std::vector<MapiFileDescW> MapiAttachmentList_t;

const int LEN_SMTP_PREFIX = 5; // "SMTP:"

namespace /* private */
{
    std::wstring gFrom;
    std::wstring gSubject;
    std::wstring gBody;
    StringList_t gTo;
    StringList_t gCc;
    StringList_t gBcc;
    StringList_t gAttachments;
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
inline std::wstring prefixEmailAddress(
    const std::wstring& aEmailAddress,
    const std::wstring& aPrefix = L"SMTP:")
{
    return (aPrefix + aEmailAddress);
}

/** @internal */
void addRecipient(
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
void initRecipientList(MapiRecipientList_t* pMapiRecipientList)
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
void initAttachmentList(MapiAttachmentList_t* pMapiAttachmentList)
{
    OSL_ASSERT(pMapiAttachmentList->empty());

    for (const auto& attachment : gAttachments)
    {
        MapiFileDescW mfd;
        ZeroMemory(&mfd, sizeof(mfd));
        mfd.lpszPathName = const_cast<wchar_t*>(attachment.c_str());
        mfd.nPosition = sal::static_int_cast<ULONG>(-1);
        pMapiAttachmentList->push_back(mfd);
    }
}

/** @internal */
void initMapiOriginator(MapiRecipDescW* pMapiOriginator)
{
    ZeroMemory(pMapiOriginator, sizeof(*pMapiOriginator));

    pMapiOriginator->ulRecipClass = MAPI_ORIG;
    pMapiOriginator->lpszName = const_cast<wchar_t*>(L"");
    pMapiOriginator->lpszAddress = const_cast<wchar_t*>(gFrom.c_str());
}

/** @internal */
void initMapiMessage(
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
    pMapiMessage->lpFiles = &aMapiAttachmentList[0];
    pMapiMessage->nFileCount = aMapiAttachmentList.size();
}

const wchar_t* const KnownParameter[] =
{
    L"--to",
    L"--cc",
    L"--bcc",
    L"--from",
    L"--subject",
    L"--body",
    L"--attach",
    L"--mapi-dialog",
    L"--mapi-logon-ui"
};

const size_t nKnownParameter = SAL_N_ELEMENTS(KnownParameter);

/** @internal */
bool isKnownParameter(const wchar_t* aParameterName)
{
    for (size_t i = 0; i < nKnownParameter; i++)
        if (_wcsicmp(aParameterName, KnownParameter[i]) == 0)
            return true;

    return false;
}

/** @internal */
void initParameter(int argc, wchar_t* argv[])
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
            gMapiFlags |= MAPI_DIALOG;
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
                gAttachments.push_back(argv[i+1]);

            i++;
        }
    }
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
        // we have to set the flag MAPI_NEW_SESSION,
        // because in the case Outlook xxx (not Outlook Express!)
        // is installed as Exchange and Mail Client a Profile
        // selection dialog must appear because we specify no
        // profile name, so the user has to specify a profile

        LHANDLE hSession = 0;

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
            oss << "--attach " << attachment << std::endl;

        if (gMapiFlags & MAPI_DIALOG)
            oss << "--mapi-dialog" << std::endl;

        if (gMapiFlags & MAPI_LOGON_UI)
            oss << "--mapi-logon-ui" << std::endl;

        MessageBoxW(nullptr, oss.str().c_str(), L"Arguments", MB_OK | MB_ICONINFORMATION);
    }
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
