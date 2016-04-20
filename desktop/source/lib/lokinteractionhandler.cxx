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

#include "lokinteractionhandler.hxx"

#include <boost/property_tree/json_parser.hpp>

#include <rtl/ref.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/task/XInteractionPassword2.hpp>
#include <com/sun/star/ucb/InteractiveNetworkConnectException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkOffLineException.hpp>

#include <com/sun/star/ucb/InteractiveIOException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkReadException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkResolveNameException.hpp>
#include <com/sun/star/ucb/InteractiveNetworkWriteException.hpp>

#define LOK_USE_UNSTABLE_API
#include <../../inc/lib/init.hxx>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <sfx2/lokhelper.hxx>
#include <comphelper/lok.hxx>

using namespace com::sun::star;

LOKInteractionHandler::LOKInteractionHandler(
        uno::Reference<uno::XComponentContext> const & /*rxContext*/,
        const OString& rCommand,
        desktop::LibLibreOffice_Impl *const pLOKit,
        desktop::LibLODocument_Impl *const pLOKDocument)
    : m_pLOKit(pLOKit)
    , m_pLOKDocument(pLOKDocument)
    , m_command(rCommand)
    , m_usePassword(false)
{
    assert(m_pLOKit);
}

LOKInteractionHandler::~LOKInteractionHandler()
{
}

OUString SAL_CALL LOKInteractionHandler::getImplementationName() throw (uno::RuntimeException, std::exception)
{
    return OUString("com.sun.star.comp.uui.LOKInteractionHandler");
}

sal_Bool SAL_CALL LOKInteractionHandler::supportsService(OUString const & rServiceName) throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL LOKInteractionHandler::getSupportedServiceNames() throw (uno::RuntimeException, std::exception)
{
    uno::Sequence< OUString > aNames(3);
    aNames[0] = "com.sun.star.task.InteractionHandler";
    // added to indicate support for configuration.backend.MergeRecoveryRequest
    aNames[1] = "com.sun.star.configuration.backend.InteractionHandler";
    aNames[2] = "com.sun.star.uui.InteractionHandler";
    // for backwards compatibility
    return aNames;
}

void SAL_CALL LOKInteractionHandler::initialize(uno::Sequence<uno::Any> const & /*rArguments*/) throw (uno::Exception, std::exception)
{
}

void SAL_CALL LOKInteractionHandler::handle(
        uno::Reference<task::XInteractionRequest> const & xRequest)
throw (uno::RuntimeException, std::exception)
{
    // just do the same thing in both cases
    handleInteractionRequest(xRequest);
}

void LOKInteractionHandler::postError(css::task::InteractionClassification classif, const char* kind, ErrCode code, const OUString &message)
{
    const char *classification = "error";
    switch (classif)
    {
        case task::InteractionClassification_ERROR: break;
        case task::InteractionClassification_WARNING: classification = "warning"; break;
        case task::InteractionClassification_INFO: classification = "info"; break;
        case task::InteractionClassification_QUERY: classification = "query"; break;
        default: assert(false); break;
    }

    // create the JSON representation
    boost::property_tree::ptree aTree;
    aTree.put("classification", classification);
    aTree.put("cmd", m_command.getStr());
    aTree.put("kind", kind);
    aTree.put("code", code);
    aTree.put("message", message.toUtf8());

    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aTree);

    std::size_t nView = comphelper::LibreOfficeKit::isViewCallback() ? SfxLokHelper::getView() : 0;
    if (m_pLOKDocument && m_pLOKDocument->mpCallbackFlushHandlers[nView])
        m_pLOKDocument->mpCallbackFlushHandlers[nView]->queue(LOK_CALLBACK_ERROR, aStream.str().c_str());
    else if (m_pLOKit->mpCallback)
        m_pLOKit->mpCallback(LOK_CALLBACK_ERROR, aStream.str().c_str(), m_pLOKit->mpCallbackData);
}

namespace {

/// Just approve the interaction.
void selectApproved(uno::Sequence<uno::Reference<task::XInteractionContinuation>> const &rContinuations)
{
    for (sal_Int32 i = 0; i < rContinuations.getLength(); ++i)
    {
        uno::Reference<task::XInteractionApprove> xApprove(rContinuations[i], uno::UNO_QUERY);
        if (xApprove.is())
            xApprove->select();
    }
}

}

bool LOKInteractionHandler::handleIOException(const css::uno::Sequence<css::uno::Reference<css::task::XInteractionContinuation>> &rContinuations, const css::uno::Any& rRequest)
{
    ucb::InteractiveIOException aIoException;
    if (!(rRequest >>= aIoException))
        return false;

    static ErrCode const aErrorCode[ucb::IOErrorCode_WRONG_VERSION + 1] =
    {
        ERRCODE_IO_ABORT,
        ERRCODE_IO_ACCESSDENIED,
        ERRCODE_IO_ALREADYEXISTS,
        ERRCODE_IO_BADCRC,
        ERRCODE_IO_CANTCREATE,
        ERRCODE_IO_CANTREAD,
        ERRCODE_IO_CANTSEEK,
        ERRCODE_IO_CANTTELL,
        ERRCODE_IO_CANTWRITE,
        ERRCODE_IO_CURRENTDIR,
        ERRCODE_IO_DEVICENOTREADY,
        ERRCODE_IO_NOTSAMEDEVICE,
        ERRCODE_IO_GENERAL,
        ERRCODE_IO_INVALIDACCESS,
        ERRCODE_IO_INVALIDCHAR,
        ERRCODE_IO_INVALIDDEVICE,
        ERRCODE_IO_INVALIDLENGTH,
        ERRCODE_IO_INVALIDPARAMETER,
        ERRCODE_IO_ISWILDCARD,
        ERRCODE_IO_LOCKVIOLATION,
        ERRCODE_IO_MISPLACEDCHAR,
        ERRCODE_IO_NAMETOOLONG,
        ERRCODE_IO_NOTEXISTS,
        ERRCODE_IO_NOTEXISTSPATH,
        ERRCODE_IO_NOTSUPPORTED,
        ERRCODE_IO_NOTADIRECTORY,
        ERRCODE_IO_NOTAFILE,
        ERRCODE_IO_OUTOFSPACE,
        ERRCODE_IO_TOOMANYOPENFILES,
        ERRCODE_IO_OUTOFMEMORY,
        ERRCODE_IO_PENDING,
        ERRCODE_IO_RECURSIVE,
        ERRCODE_IO_UNKNOWN,
        ERRCODE_IO_WRITEPROTECTED,
        ERRCODE_IO_WRONGFORMAT,
        ERRCODE_IO_WRONGVERSION,
    };

    postError(aIoException.Classification, "io", aErrorCode[aIoException.Code], "");
    selectApproved(rContinuations);

    return true;
}

bool LOKInteractionHandler::handleNetworkException(const uno::Sequence<uno::Reference<task::XInteractionContinuation>> &rContinuations, const uno::Any &rRequest)
{
    ucb::InteractiveNetworkException aNetworkException;
    if (!(rRequest >>= aNetworkException))
        return false;

    ErrCode nErrorCode;
    OUString aMessage;

    ucb::InteractiveNetworkOffLineException aOffLineException;
    ucb::InteractiveNetworkResolveNameException aResolveNameException;
    ucb::InteractiveNetworkConnectException aConnectException;
    ucb::InteractiveNetworkReadException aReadException;
    ucb::InteractiveNetworkWriteException aWriteException;
    if (rRequest >>= aOffLineException)
    {
        nErrorCode = ERRCODE_INET_OFFLINE;
    }
    else if (rRequest >>= aResolveNameException)
    {
        nErrorCode = ERRCODE_INET_NAME_RESOLVE;
        aMessage = aResolveNameException.Server;
    }
    else if (rRequest >>= aConnectException)
    {
        nErrorCode = ERRCODE_INET_CONNECT;
        aMessage = aConnectException.Server;
    }
    else if (rRequest >>= aReadException)
    {
        nErrorCode = ERRCODE_INET_READ;
        aMessage = aReadException.Diagnostic;
    }
    else if (rRequest >>= aWriteException)
    {
        nErrorCode = ERRCODE_INET_WRITE;
        aMessage = aWriteException.Diagnostic;
    }
    else
    {
        nErrorCode = ERRCODE_INET_GENERAL;
    }

    postError(aNetworkException.Classification, "network", nErrorCode, aMessage);
    selectApproved(rContinuations);

    return true;
}

bool LOKInteractionHandler::handlePasswordRequest(const uno::Sequence<uno::Reference<task::XInteractionContinuation>> &rContinuations, const uno::Any &rRequest)
{
    task::DocumentPasswordRequest2 passwordRequest;
    if (!(rRequest >>= passwordRequest))
        return false;

    if (m_pLOKit->mpCallback &&
        m_pLOKit->hasOptionalFeature((passwordRequest.IsRequestPasswordToModify)
                ? LOK_FEATURE_DOCUMENT_PASSWORD_TO_MODIFY
                : LOK_FEATURE_DOCUMENT_PASSWORD))
    {
        OString const url(passwordRequest.Name.toUtf8());
        m_pLOKit->mpCallback(passwordRequest.IsRequestPasswordToModify
                ? LOK_CALLBACK_DOCUMENT_PASSWORD_TO_MODIFY
                : LOK_CALLBACK_DOCUMENT_PASSWORD,
                url.getStr(),
                m_pLOKit->mpCallbackData);

        // block until SetPassword is called
        m_havePassword.wait();
        m_havePassword.reset();
    }

    for (sal_Int32 i = 0; i < rContinuations.getLength(); ++i)
    {
        if (m_usePassword)
        {
            if (passwordRequest.IsRequestPasswordToModify)
            {
                uno::Reference<task::XInteractionPassword2> const xIPW2(rContinuations[i], uno::UNO_QUERY);
                xIPW2->setPasswordToModify(m_Password);
                xIPW2->select();
            }
            else
            {
                uno::Reference<task::XInteractionPassword> const xIPW(rContinuations[i], uno::UNO_QUERY);
                if (xIPW.is())
                {
                    xIPW->setPassword(m_Password);
                    xIPW->select();
                }
            }
        }
        else
        {
            if (passwordRequest.IsRequestPasswordToModify)
            {
                uno::Reference<task::XInteractionPassword2> const xIPW2(rContinuations[i], uno::UNO_QUERY);
                xIPW2->setRecommendReadOnly(true);
                xIPW2->select();
            }
            else
            {
                uno::Reference<task::XInteractionAbort> const xAbort(rContinuations[i], uno::UNO_QUERY);
                if (xAbort.is())
                {
                    xAbort->select();
                }
            }
        }
    }
    return true;
}

sal_Bool SAL_CALL LOKInteractionHandler::handleInteractionRequest(
        const uno::Reference<task::XInteractionRequest>& xRequest) throw (uno::RuntimeException, std::exception)
{
    uno::Sequence<uno::Reference<task::XInteractionContinuation>> const &rContinuations = xRequest->getContinuations();
    uno::Any const request(xRequest->getRequest());

    if (handleIOException(rContinuations, request))
        return true;

    if (handleNetworkException(rContinuations, request))
        return true;

    if (handlePasswordRequest(rContinuations, request))
        return true;

    // TODO: perform more interactions 'for real' like the above
    selectApproved(rContinuations);

    return true;
}

void LOKInteractionHandler::SetPassword(char const*const pPassword)
{
    if (pPassword)
    {
        m_Password = OUString(pPassword, strlen(pPassword), RTL_TEXTENCODING_UTF8);
        m_usePassword = true;
    }
    else
    {
        m_usePassword = false;
    }
    m_havePassword.set();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
