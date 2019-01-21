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

#include <config_folders.h>

#include <osl/diagnose.h>
#include <osl/process.h>
#include <rtl/bootstrap.hxx>
#include "smplmailclient.hxx"
#include "smplmailmsg.hxx"
#include <com/sun/star/system/SimpleMailClientFlags.hpp>
#include <com/sun/star/system/XSimpleMailMessage2.hpp>
#include <osl/file.hxx>
#include <o3tl/char16_t2wchar_t.hxx>
#include <tools/urlobj.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/syslocale.hxx>
#include <i18nlangtag/languagetag.hxx>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mapi.h>
#if defined GetTempPath
#undef GetTempPath
#endif

#include <process.h>
#include <vector>

using css::uno::UNO_QUERY;
using css::uno::Reference;
using css::uno::Exception;
using css::uno::RuntimeException;
using css::uno::Sequence;
using css::lang::IllegalArgumentException;

using css::system::XSimpleMailClient;
using css::system::XSimpleMailMessage;
using css::system::XSimpleMailMessage2;
using css::system::SimpleMailClientFlags::NO_USER_INTERFACE;
using css::system::SimpleMailClientFlags::NO_LOGON_DIALOG;

const OUString TO("--to");
const OUString CC("--cc");
const OUString BCC("--bcc");
const OUString FROM("--from");
const OUString SUBJECT("--subject");
const OUString BODY("--body");
const OUString ATTACH("--attach");
const OUString ATTACH_NAME("--attach-name");
const OUString FLAG_MAPI_DIALOG("--mapi-dialog");
const OUString FLAG_MAPI_LOGON_UI("--mapi-logon-ui");
const OUString FLAG_LANGTAG("--langtag");
const OUString FLAG_BOOTSTRAP("--bootstrap");

namespace /* private */
{
    /** @internal
        look if an alternative program is configured
        which should be used as senddoc executable */
    OUString getAlternativeSenddocUrl()
    {
        OUString altSenddocUrl;
        HKEY hkey;
        LONG lret = RegOpenKeyW(HKEY_CURRENT_USER, L"Software\\LibreOffice\\SendAsEMailClient", &hkey);
        if (lret == ERROR_SUCCESS)
        {
            wchar_t buff[MAX_PATH];
            LONG sz = sizeof(buff);
            lret = RegQueryValueW(hkey, nullptr, buff, &sz);
            if (lret == ERROR_SUCCESS)
            {
                osl::FileBase::getFileURLFromSystemPath(o3tl::toU(buff), altSenddocUrl);
            }
            RegCloseKey(hkey);
        }
        return altSenddocUrl;
    }

    /**
        Returns the absolute file Url of the senddoc executable.

        @returns
        the absolute file Url of the senddoc executable. In case
        of an error an empty string will be returned.
    */
    OUString getSenddocUrl()
    {
        OUString senddocUrl = getAlternativeSenddocUrl();

        if (senddocUrl.isEmpty())
        {
            senddocUrl = "$BRAND_BASE_DIR/" LIBO_LIBEXEC_FOLDER "/senddoc.exe";
            rtl::Bootstrap::expandMacros(senddocUrl); //TODO: detect failure
        }
        return senddocUrl;
    }

    /**
        Execute Senddoc.exe which a MAPI wrapper.

        @param rCommandArgs
        [in] the arguments to be passed to Senddoc.exe

        @returns
        <TRUE/> on success.
    */
    bool executeSenddoc(const std::vector<OUString>& rCommandArgs, bool bWait)
    {
        OUString senddocUrl = getSenddocUrl();
        if (senddocUrl.getLength() == 0)
            return false;

        oslProcessOption nProcOption = osl_Process_DETACHED | (bWait ? osl_Process_WAIT : 0);

        oslProcess proc;

        /* for efficiency reasons we are using a 'bad' cast here
        as a vector or OUStrings is nothing else than
        an array of pointers to rtl_uString's */
        oslProcessError err = osl_executeProcess(
            senddocUrl.pData,
            const_cast<rtl_uString**>(reinterpret_cast<rtl_uString * const *>(&rCommandArgs[0])),
            rCommandArgs.size(),
            nProcOption,
            nullptr,
            nullptr,
            nullptr,
            0,
            &proc);

        if (err != osl_Process_E_None)
            return false;

        if (!bWait)
            return true;

        oslProcessInfo procInfo;
        procInfo.Size = sizeof(oslProcessInfo);
        osl_getProcessInfo(proc, osl_Process_EXITCODE, &procInfo);
        osl_freeProcessHandle(proc);
        return (procInfo.Code == SUCCESS_SUCCESS);
    }
} // namespace private

Reference<XSimpleMailMessage> SAL_CALL CSmplMailClient::createSimpleMailMessage()
{
    return Reference<XSimpleMailMessage>(new CSmplMailMsg());
}

namespace {
// We cannot use the session-local temporary directory for the attachment,
// because it will get removed upon program exit; and it must be alive for
// senddoc process lifetime. So we use base temppath for the attachments,
// and let the senddoc to do the cleanup if it was started successfully.
// This function works like Desktop::CreateTemporaryDirectory()
OUString InitBaseTempDirURL()
{
    // No need to intercept an exception here, since
    // Desktop::CreateTemporaryDirectory() has ensured that path manager is available
    SvtPathOptions aOpt;
    OUString aRetURL = aOpt.GetTempPath();
    if (aRetURL.isEmpty())
    {
        osl::File::getTempDirURL(aRetURL);
    }
    if (aRetURL.endsWith("/"))
        aRetURL = aRetURL.copy(0, aRetURL.getLength() - 1);

    return aRetURL;
}

const OUString& GetBaseTempDirURL()
{
    static const OUString aRetURL(InitBaseTempDirURL());
    return aRetURL;
}
}

OUString CSmplMailClient::CopyAttachment(const OUString& sOrigAttachURL, OUString& sUserVisibleName)
{
    // We do two things here:
    // 1. Make the attachment temporary filename to not contain any fancy characters possible in
    // original filename, that could confuse mailer, and extract the original filename to explicitly
    // define it;
    // 2. Allow the copied files be outside of the session's temporary directory, and thus not be
    // removed in Desktop::RemoveTemporaryDirectory() if soffice process gets closed before the
    // mailer finishes using them.

    maAttachmentFiles.emplace_back(std::make_unique<utl::TempFile>(&GetBaseTempDirURL()));
    maAttachmentFiles.back()->EnableKillingFile();
    INetURLObject aFilePathObj(maAttachmentFiles.back()->GetURL());
    OUString sNewAttachmentURL = aFilePathObj.GetMainURL(INetURLObject::DecodeMechanism::NONE);
    if (osl::File::copy(sOrigAttachURL, sNewAttachmentURL) == osl::FileBase::RC::E_None)
    {
        INetURLObject url(sOrigAttachURL, INetURLObject::EncodeMechanism::WasEncoded);
        sUserVisibleName = url.getName(INetURLObject::LAST_SEGMENT, true,
            INetURLObject::DecodeMechanism::WithCharset);
    }
    else
    {
        // Failed to copy original; the best effort is to use original file. It is possible that
        // the file gets deleted before used in spawned process; but let's hope... the worst thing
        // is the absent attachment file anyway.
        sNewAttachmentURL = sOrigAttachURL;
        maAttachmentFiles.pop_back();
    }
    return sNewAttachmentURL;
}

void CSmplMailClient::ReleaseAttachments()
{
    for (auto& pTempFile : maAttachmentFiles)
    {
        if (pTempFile)
            pTempFile->EnableKillingFile(false);
    }
    maAttachmentFiles.clear();
}

/**
    Assemble a command line for SendDoc.exe out of the members
    of the supplied SimpleMailMessage.

    @param xSimpleMailMessage
    [in] the mail message.

    @param aFlags
    [in] different flags to be used with the simple mail service.

    @param rCommandArgs
    [in|out] a buffer for the command line arguments. The buffer
    is assumed to be empty.

    @throws css::lang::IllegalArgumentException
    if an invalid file URL has been detected in the attachment list.
*/
void CSmplMailClient::assembleCommandLine(
    const Reference<XSimpleMailMessage>& xSimpleMailMessage,
    sal_Int32 aFlag, std::vector<OUString>& rCommandArgs)
{
    OSL_ENSURE(rCommandArgs.empty(), "Provided command argument buffer not empty");

    Reference<XSimpleMailMessage2> xMessage( xSimpleMailMessage, UNO_QUERY );
    if (xMessage.is())
    {
        OUString body = xMessage->getBody();
        if (body.getLength()>0)
        {
            rCommandArgs.push_back(BODY);
            rCommandArgs.push_back(body);
        }
    }

    OUString to = xSimpleMailMessage->getRecipient();
    if (to.getLength() > 0)
    {
        rCommandArgs.push_back(TO);
        rCommandArgs.push_back(to);
    }

    Sequence<OUString> ccRecipients = xSimpleMailMessage->getCcRecipient();
    for (int i = 0; i < ccRecipients.getLength(); i++)
    {
        rCommandArgs.push_back(CC);
        rCommandArgs.push_back(ccRecipients[i]);
    }

    Sequence<OUString> bccRecipients = xSimpleMailMessage->getBccRecipient();
    for (int i = 0; i < bccRecipients.getLength(); i++)
    {
        rCommandArgs.push_back(BCC);
        rCommandArgs.push_back(bccRecipients[i]);
    }

    OUString from = xSimpleMailMessage->getOriginator();
    if (from.getLength() > 0)
    {
        rCommandArgs.push_back(FROM);
        rCommandArgs.push_back(from);
    }

    OUString subject = xSimpleMailMessage->getSubject();
    if (subject.getLength() > 0)
    {
        rCommandArgs.push_back(SUBJECT);
        rCommandArgs.push_back(subject);
    }

    for (const auto& attachment : xSimpleMailMessage->getAttachement())
    {
        OUString sDisplayName;
        OUString sTempFileURL(CopyAttachment(attachment, sDisplayName));
        OUString sysPath;
        osl::FileBase::RC err = osl::FileBase::getSystemPathFromFileURL(sTempFileURL, sysPath);
        if (err != osl::FileBase::E_None)
            throw IllegalArgumentException(
                "Invalid attachment file URL",
                static_cast<XSimpleMailClient*>(this),
                1);

        rCommandArgs.push_back(ATTACH);
        rCommandArgs.push_back(sysPath);
        if (!sDisplayName.isEmpty())
        {
            rCommandArgs.push_back(ATTACH_NAME);
            rCommandArgs.push_back(sDisplayName);
        }
    }

    if (!(aFlag & NO_USER_INTERFACE))
        rCommandArgs.push_back(FLAG_MAPI_DIALOG);

    if (!(aFlag & NO_LOGON_DIALOG))
        rCommandArgs.push_back(FLAG_MAPI_LOGON_UI);

    rCommandArgs.push_back(FLAG_LANGTAG);
    rCommandArgs.push_back(SvtSysLocale().GetUILanguageTag().getBcp47());

    rtl::Bootstrap aBootstrap;
    OUString sBootstrapPath;
    aBootstrap.getIniName(sBootstrapPath);
    if (!sBootstrapPath.isEmpty())
    {
        rCommandArgs.push_back(FLAG_BOOTSTRAP);
        rCommandArgs.push_back(sBootstrapPath);
    }

}

void SAL_CALL CSmplMailClient::sendSimpleMailMessage(
    const Reference<XSimpleMailMessage>& xSimpleMailMessage, sal_Int32 aFlag)
{
    validateParameter(xSimpleMailMessage, aFlag);

    std::vector<OUString> senddocParams;
    assembleCommandLine(xSimpleMailMessage, aFlag, senddocParams);

    const bool bWait = aFlag & NO_USER_INTERFACE;
    if (!executeSenddoc(senddocParams, bWait))
        throw Exception(
            "Send email failed",
            static_cast<XSimpleMailClient*>(this));
    // Let the launched senddoc to cleanup the attachments temporary files
    if (!bWait)
        ReleaseAttachments();
}

void CSmplMailClient::validateParameter(
    const Reference<XSimpleMailMessage>& xSimpleMailMessage, sal_Int32 aFlag )
{
    if (!xSimpleMailMessage.is())
        throw IllegalArgumentException(
            "Empty mail message reference",
            static_cast<XSimpleMailClient*>(this),
            1);

    OSL_ENSURE(!(aFlag & NO_LOGON_DIALOG), "Flag NO_LOGON_DIALOG has currently no effect");

    // check the flags, the allowed range is 0 - (2^n - 1)
    if (aFlag < 0 || aFlag > 3)
        throw IllegalArgumentException(
            "Invalid flag value",
            static_cast<XSimpleMailClient*>(this),
            2);

    // check if a recipient is specified of the flags NO_USER_INTERFACE is specified
    if ((aFlag & NO_USER_INTERFACE) && !xSimpleMailMessage->getRecipient().getLength())
        throw IllegalArgumentException(
            "No recipient specified",
            static_cast<XSimpleMailClient*>(this),
            1);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
