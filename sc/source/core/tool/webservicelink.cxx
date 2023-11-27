/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/bindings.hxx>

#include <com/sun/star/ucb/XSimpleFileAccess3.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/io/XInputStream.hpp>

#include <webservicelink.hxx>
#include <brdcst.hxx>
#include <document.hxx>
#include <sc.hrc>

ScWebServiceLink::ScWebServiceLink(ScDocument* pD, const OUString& rURL)
    : ::sfx2::SvBaseLink(SfxLinkUpdateMode::ALWAYS, SotClipboardFormatId::STRING)
    , pDoc(pD)
    , aURL(rURL)
    , bHasResult(false)
{
}

ScWebServiceLink::~ScWebServiceLink() {}

sfx2::SvBaseLink::UpdateResult ScWebServiceLink::DataChanged(const OUString&, const css::uno::Any&)
{
    aResult.clear();
    bHasResult = false;

    if (comphelper::LibreOfficeKit::isActive())
    {
        SAL_WARN("sc.ui", "ScWebServiceLink::DataChanged: blocked access to external file: \""
                              << aURL << "\"");
        return ERROR_GENERAL;
    }

    css::uno::Reference<css::ucb::XSimpleFileAccess3> xFileAccess
        = css::ucb::SimpleFileAccess::create(comphelper::getProcessComponentContext());
    if (!xFileAccess.is())
        return ERROR_GENERAL;

    css::uno::Reference<css::io::XInputStream> xStream;
    try
    {
        xStream = xFileAccess->openFileRead(aURL);
    }
    catch (...)
    {
        // don't let any exceptions pass
        return ERROR_GENERAL;
    }
    if (!xStream.is())
        return ERROR_GENERAL;

    const sal_Int32 BUF_LEN = 8000;
    css::uno::Sequence<sal_Int8> buffer(BUF_LEN);
    OStringBuffer aBuffer(64000);

    sal_Int32 nRead = 0;
    while ((nRead = xStream->readBytes(buffer, BUF_LEN)) == BUF_LEN)
        aBuffer.append(reinterpret_cast<const char*>(buffer.getConstArray()), nRead);

    if (nRead > 0)
        aBuffer.append(reinterpret_cast<const char*>(buffer.getConstArray()), nRead);

    xStream->closeInput();

    aResult = OStringToOUString(aBuffer.makeStringAndClear(), RTL_TEXTENCODING_UTF8);
    bHasResult = true;

    //  Something happened...
    if (HasListeners())
    {
        Broadcast(ScHint(SfxHintId::ScDataChanged, ScAddress()));
        pDoc->TrackFormulas(); // must happen immediately
        pDoc->StartTrackTimer();
    }

    return SUCCESS;
}

void ScWebServiceLink::ListenersGone()
{
    ScDocument* pStackDoc = pDoc; // member pDoc can't be used after removing the link

    sfx2::LinkManager* pLinkMgr = pDoc->GetLinkManager();
    pLinkMgr->Remove(this); // deletes this

    if (pLinkMgr->GetLinks().empty()) // deleted the last one ?
    {
        SfxBindings* pBindings = pStackDoc->GetViewBindings(); // don't use member pDoc!
        if (pBindings)
            pBindings->Invalidate(SID_LINKS);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
