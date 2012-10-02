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
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <rtl/uri.hxx>
#include <osl/thread.hxx>

#include "simplemapi.hxx"

#define WIN32_LEAN_AND_MEAN
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <tchar.h>

#include <iostream>
#include <vector>
#include <sstream>
#include <stdexcept>

#if OSL_DEBUG_LEVEL > 2
    void dumpParameter();
#endif

typedef std::vector<std::string> StringList_t;
typedef StringList_t::const_iterator StringListIterator_t;
typedef std::vector<MapiRecipDesc> MapiRecipientList_t;
typedef std::vector<MapiFileDesc> MapiAttachmentList_t;

const int LEN_SMTP_PREFIX = 5; // "SMTP:"

namespace /* private */
{
    std::string gFrom;
    std::string gSubject;
    std::string gBody;
    StringList_t gTo;
    StringList_t gCc;
    StringList_t gBcc;
    StringList_t gAttachments;
    int gMapiFlags = 0;
}

/**
    Add a prefix to an email address. MAPI requires that that
    email addresses have an 'SMTP:' prefix.

    @param  aEmailAddress
    [in] the email address.

    @param  aPrefix
    [in] the prefix to be added to the email address.

    @returns
    the email address prefixed with the specified prefix.
*/
inline std::string prefixEmailAddress(
    const std::string& aEmailAddress,
    const std::string& aPrefix = "SMTP:")
{
    return (aPrefix + aEmailAddress);
}

/** @internal */
void addRecipient(
    ULONG recipClass,
    const std::string& recipAddress,
    MapiRecipientList_t* pMapiRecipientList)
{
    MapiRecipDesc mrd;
    ZeroMemory(&mrd, sizeof(mrd));

    mrd.ulRecipClass = recipClass;
    mrd.lpszName = const_cast<char*>(recipAddress.c_str()) + LEN_SMTP_PREFIX;
    mrd.lpszAddress = const_cast<char*>(recipAddress.c_str());
    pMapiRecipientList->push_back(mrd);
}

/** @internal */
void initRecipientList(MapiRecipientList_t* pMapiRecipientList)
{
    OSL_ASSERT(pMapiRecipientList->empty());

    // add to recipients
    StringListIterator_t iter = gTo.begin();
    StringListIterator_t iter_end = gTo.end();
    for (; iter != iter_end; ++iter)
        addRecipient(MAPI_TO, *iter, pMapiRecipientList);

    // add cc recipients
    iter = gCc.begin();
    iter_end = gCc.end();
    for (; iter != iter_end; ++iter)
        addRecipient(MAPI_CC, *iter, pMapiRecipientList);

    // add bcc recipients
    iter = gBcc.begin();
    iter_end = gBcc.end();
    for (; iter != iter_end; ++iter)
        addRecipient(MAPI_BCC, *iter, pMapiRecipientList);
}

/** @internal */
void initAttachementList(MapiAttachmentList_t* pMapiAttachmentList)
{
    OSL_ASSERT(pMapiAttachmentList->empty());

    StringListIterator_t iter = gAttachments.begin();
    StringListIterator_t iter_end = gAttachments.end();
    for (/**/; iter != iter_end; ++iter)
    {
        MapiFileDesc mfd;
        ZeroMemory(&mfd, sizeof(mfd));
        mfd.lpszPathName = const_cast<char*>(iter->c_str());
        mfd.nPosition = sal::static_int_cast<ULONG>(-1);
        pMapiAttachmentList->push_back(mfd);
    }
}

/** @internal */
void initMapiOriginator(MapiRecipDesc* pMapiOriginator)
{
    ZeroMemory(pMapiOriginator, sizeof(MapiRecipDesc));

    pMapiOriginator->ulRecipClass = MAPI_ORIG;
    pMapiOriginator->lpszName = const_cast<char*>("");
    pMapiOriginator->lpszAddress = const_cast<char*>(gFrom.c_str());
}

/** @internal */
void initMapiMessage(
    MapiRecipDesc* aMapiOriginator,
    MapiRecipientList_t& aMapiRecipientList,
    MapiAttachmentList_t& aMapiAttachmentList,
    MapiMessage* pMapiMessage)
{
    ZeroMemory(pMapiMessage, sizeof(MapiMessage));

    try {
         rtl_uString *subject = NULL;
         rtl_uString_newFromAscii(&subject, const_cast<char*>(gSubject.c_str()));
         rtl_uString *decoded_subject = NULL;
         rtl_uriDecode(subject, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8, &decoded_subject);
         rtl::OUString ou_subject(decoded_subject);
         pMapiMessage->lpszSubject = strdup(OUStringToOString(ou_subject, osl_getThreadTextEncoding(), RTL_UNICODETOTEXT_FLAGS_UNDEFINED_QUESTIONMARK).getStr());
    }
    catch (...) {
    pMapiMessage->lpszSubject = const_cast<char*>(gSubject.c_str());
    }
    pMapiMessage->lpszNoteText = (gBody.length() ? const_cast<char*>(gBody.c_str()) : NULL);
    pMapiMessage->lpOriginator = aMapiOriginator;
    pMapiMessage->lpRecips = aMapiRecipientList.size() ? &aMapiRecipientList[0] : 0;
    pMapiMessage->nRecipCount = aMapiRecipientList.size();
    pMapiMessage->lpFiles = &aMapiAttachmentList[0];
    pMapiMessage->nFileCount = aMapiAttachmentList.size();
}

const char* KnownParameter[] =
{
    "--to",
    "--cc",
    "--bcc",
    "--from",
    "--subject",
    "--body",
    "--attach",
    "--mapi-dialog",
    "--mapi-logon-ui"
};

const size_t nKnownParameter = (sizeof(KnownParameter)/sizeof(KnownParameter[0]));

/** @internal */
bool isKnownParameter(const char* aParameterName)
{
    for (size_t i = 0; i < nKnownParameter; i++)
        if (_tcsicmp(aParameterName, KnownParameter[i]) == 0)
            return true;

    return false;
}

/** @internal */
void initParameter(int argc, char* argv[])
{
    for (int i = 1; i < argc; i++)
    {
        if (!isKnownParameter(argv[i]))
        {
            OSL_FAIL("Wrong parameter received");
            continue;
        }

        if ((_tcsicmp(argv[i], TEXT("--mapi-dialog")) == 0))
        {
            gMapiFlags |= MAPI_DIALOG;
        }
        else if ((_tcsicmp(argv[i], TEXT("--mapi-logon-ui")) == 0))
        {
            gMapiFlags |= MAPI_LOGON_UI;
        }
        else if ((i+1) < argc) // is the value of a parameter available too?
        {
            if (_tcsicmp(argv[i], TEXT("--to")) == 0)
                gTo.push_back(prefixEmailAddress(argv[i+1]));
            else if (_tcsicmp(argv[i], TEXT("--cc")) == 0)
                gCc.push_back(prefixEmailAddress(argv[i+1]));
            else if (_tcsicmp(argv[i], TEXT("--bcc")) == 0)
                gBcc.push_back(prefixEmailAddress(argv[i+1]));
            else if (_tcsicmp(argv[i], TEXT("--from")) == 0)
                gFrom = prefixEmailAddress(argv[i+1]);
            else if (_tcsicmp(argv[i], TEXT("--subject")) == 0)
                gSubject = argv[i+1];
            else if (_tcsicmp(argv[i], TEXT("--body")) == 0)
                gBody = argv[i+1];
            else if ((_tcsicmp(argv[i], TEXT("--attach")) == 0))
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
int main(int argc, char* argv[])
{

    initParameter(argc, argv);

#if OSL_DEBUG_LEVEL > 2
    dumpParameter();
#endif

    ULONG ulRet = MAPI_E_FAILURE;

    try
    {
        CSimpleMapi mapi;

        // we have to set the flag MAPI_NEW_SESSION,
        // because in the case Outlook xxx (not Outlook Express!)
        // is installed as Exchange and Mail Client a Profile
        // selection dialog must appear because we specify no
        // profile name, so the user has to specify a profile
        FLAGS flFlag = MAPI_NEW_SESSION | MAPI_LOGON_UI;
        LHANDLE hSession;
        ulRet = mapi.MAPILogon(0, NULL, NULL, flFlag, 0L, &hSession);

        if (ulRet == SUCCESS_SUCCESS)
        {
            MapiRecipDesc mapiOriginator;
            MapiRecipientList_t mapiRecipientList;
            MapiAttachmentList_t mapiAttachmentList;
            MapiMessage mapiMsg;

            initMapiOriginator(&mapiOriginator);
            initRecipientList(&mapiRecipientList);
            initAttachementList(&mapiAttachmentList);
            initMapiMessage((gFrom.length() ? &mapiOriginator : NULL), mapiRecipientList, mapiAttachmentList, &mapiMsg);

            ulRet = mapi.MAPISendMail(hSession, 0, &mapiMsg, gMapiFlags, 0);

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

            mapi.MAPILogoff(hSession, 0, 0, 0);
        }
    }
    catch (const std::runtime_error&
    #if OSL_DEBUG_LEVEL > 0
    ex
    #endif
    )
    {
        OSL_FAIL(ex.what());
    }
    return ulRet;
}

#if OSL_DEBUG_LEVEL > 2
    void dumpParameter()
    {
        std::ostringstream oss;

        if (gFrom.length() > 0)
            oss << "--from" << " " << gFrom << std::endl;

        if (gSubject.length() > 0)
            oss << "--subject" << " " << gSubject << std::endl;

        if (gBody.length() > 0)
            oss << "--body" << " " << gBody << std::endl;

        StringListIterator_t iter = gTo.begin();
        StringListIterator_t iter_end = gTo.end();
        for (/**/;iter != iter_end; ++iter)
            oss << "--to" << " " << *iter << std::endl;

        iter = gCc.begin();
        iter_end = gCc.end();
        for (/**/;iter != iter_end; ++iter)
            oss << "--cc" << " " << *iter << std::endl;

        iter = gBcc.begin();
        iter_end = gBcc.end();
        for (/**/;iter != iter_end; ++iter)
            oss << "--bcc" << " " << *iter << std::endl;

        iter = gAttachments.begin();
        iter_end = gAttachments.end();
        for (/**/;iter != iter_end; ++iter)
            oss << "--attach" << " " << *iter << std::endl;

        if (gMapiFlags & MAPI_DIALOG)
            oss << "--mapi-dialog" << std::endl;

        if (gMapiFlags & MAPI_LOGON_UI)
            oss << "--mapi-logon-ui" << std::endl;

        MessageBox(NULL, oss.str().c_str(), "Arguments", MB_OK | MB_ICONINFORMATION);
    }
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
