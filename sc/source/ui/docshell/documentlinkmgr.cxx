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

#include <comphelper/doublecheckedinit.hxx>
#include <documentlinkmgr.hxx>
#include <datastream.hxx>
#include <ddelink.hxx>
#include <webservicelink.hxx>
#include <strings.hrc>
#include <scresid.hxx>
#include <o3tl/deleter.hxx>
#include <svx/svdoole2.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

#include <memory>

namespace sc {

struct DocumentLinkManagerImpl
{
    SfxObjectShell* const mpShell;
    std::unique_ptr<DataStream, o3tl::default_delete<DataStream>> mpDataStream;
    std::atomic<sfx2::LinkManager*> mpLinkManager;

    DocumentLinkManagerImpl(const DocumentLinkManagerImpl&) = delete;
    const DocumentLinkManagerImpl& operator=(const DocumentLinkManagerImpl&) = delete;

    explicit DocumentLinkManagerImpl(SfxObjectShell* pShell)
        : mpShell(pShell), mpLinkManager(nullptr) {}

    ~DocumentLinkManagerImpl()
    {
        // Shared base links
        sfx2::LinkManager* linkManager = mpLinkManager;
        if (linkManager)
        {
            sfx2::SvLinkSources aTemp = linkManager->GetServers();
            for (const auto& pLinkSource : aTemp)
                pLinkSource->Closed();

            if (!linkManager->GetLinks().empty())
                linkManager->Remove(0, linkManager->GetLinks().size());
        }
        delete linkManager;
    }
};

DocumentLinkManager::DocumentLinkManager( SfxObjectShell* pShell ) :
    mpImpl(new DocumentLinkManagerImpl(pShell)) {}

DocumentLinkManager::~DocumentLinkManager()
{
}

void DocumentLinkManager::setDataStream( DataStream* p )
{
    mpImpl->mpDataStream.reset(p);
}

DataStream* DocumentLinkManager::getDataStream()
{
    return mpImpl->mpDataStream.get();
}

const DataStream* DocumentLinkManager::getDataStream() const
{
    return mpImpl->mpDataStream.get();
}

sfx2::LinkManager* DocumentLinkManager::getLinkManager( bool bCreate )
{
    if (bCreate && mpImpl->mpShell)
        return comphelper::doubleCheckedInit( mpImpl->mpLinkManager,
            [this]() { return new sfx2::LinkManager(mpImpl->mpShell); } );
    return mpImpl->mpLinkManager;
}

const sfx2::LinkManager* DocumentLinkManager::getExistingLinkManager() const
{
    return mpImpl->mpLinkManager;
}

bool DocumentLinkManager::idleCheckLinks()
{
    sfx2::LinkManager* pMgr = mpImpl->mpLinkManager;
    if (!pMgr)
        return false;

    bool bAnyLeft = false;
    const sfx2::SvBaseLinks& rLinks = pMgr->GetLinks();
    for (const auto & rLink : rLinks)
    {
        sfx2::SvBaseLink* pBase = rLink.get();
        ScDdeLink* pDdeLink = dynamic_cast<ScDdeLink*>(pBase);
        if (!pDdeLink || !pDdeLink->NeedsUpdate())
            continue;

        pDdeLink->TryUpdate();
        if (pDdeLink->NeedsUpdate()) // Was not successful?
            bAnyLeft = true;
    }

    return bAnyLeft;
}

bool DocumentLinkManager::hasDdeLinks() const
{
    return hasDdeOrOleOrWebServiceLinks(true, false, false);
}

bool DocumentLinkManager::hasDdeOrOleOrWebServiceLinks() const
{
    return hasDdeOrOleOrWebServiceLinks(true, true, true);
}

bool DocumentLinkManager::hasDdeOrOleOrWebServiceLinks(bool bDde, bool bOle, bool bWebService) const
{
    sfx2::LinkManager* pMgr = mpImpl->mpLinkManager;
    if (!pMgr)
        return false;

    const sfx2::SvBaseLinks& rLinks = pMgr->GetLinks();
    for (const auto & rLink : rLinks)
    {
        sfx2::SvBaseLink* pBase = rLink.get();
        if (bDde && dynamic_cast<ScDdeLink*>(pBase))
            return true;
        if (bOle && dynamic_cast<SdrEmbedObjectLink*>(pBase))
            return true;
        if (bWebService && dynamic_cast<ScWebServiceLink*>(pBase))
            return true;
    }

    return false;
}

bool DocumentLinkManager::updateDdeOrOleOrWebServiceLinks(weld::Window* pWin)
{
    sfx2::LinkManager* pMgr = mpImpl->mpLinkManager;
    if (!pMgr)
        return false;

    const sfx2::SvBaseLinks& rLinks = pMgr->GetLinks();

    // If the update takes longer, reset all values so that nothing
    // old (wrong) is left behind
    bool bAny = false;
    for (const auto & rLink : rLinks)
    {
        sfx2::SvBaseLink* pBase = rLink.get();

        SdrEmbedObjectLink* pOleLink = dynamic_cast<SdrEmbedObjectLink*>(pBase);
        if (pOleLink)
        {
            pOleLink->Update();
            continue;
        }

        ScWebServiceLink* pWebserviceLink = dynamic_cast<ScWebServiceLink*>(pBase);
        if (pWebserviceLink)
        {
            pWebserviceLink->Update();
            continue;
        }

        ScDdeLink* pDdeLink = dynamic_cast<ScDdeLink*>(pBase);
        if (!pDdeLink)
            continue;

        if (pDdeLink->Update())
            bAny = true;
        else
        {
            // Update failed.  Notify the user.
            const OUString& aFile = pDdeLink->GetTopic();
            const OUString& aElem = pDdeLink->GetItem();
            const OUString& aType = pDdeLink->GetAppl();

            OUStringBuffer aBuf;
            aBuf.append(ScResId(SCSTR_DDEDOC_NOT_LOADED));
            aBuf.append("\n\n");
            aBuf.append("Source : ");
            aBuf.append(aFile);
            aBuf.append("\nElement : ");
            aBuf.append(aElem);
            aBuf.append("\nType : ");
            aBuf.append(aType);
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pWin,
                                                      VclMessageType::Warning, VclButtonsType::Ok,
                                                      aBuf.makeStringAndClear()));
            xBox->run();
        }
    }

    pMgr->CloseCachedComps();

    return bAny;
}

void DocumentLinkManager::updateDdeLink( const OUString& rAppl, const OUString& rTopic, const OUString& rItem )
{
    sfx2::LinkManager* pMgr = mpImpl->mpLinkManager;
    if (!pMgr)
        return;

    const sfx2::SvBaseLinks& rLinks = pMgr->GetLinks();

    for (const auto & rLink : rLinks)
    {
        ::sfx2::SvBaseLink* pBase = rLink.get();
        ScDdeLink* pDdeLink = dynamic_cast<ScDdeLink*>(pBase);
        if (!pDdeLink)
            continue;

        if ( pDdeLink->GetAppl() == rAppl &&
             pDdeLink->GetTopic() == rTopic &&
             pDdeLink->GetItem() == rItem )
        {
            pDdeLink->TryUpdate();
            // Could be multiple (Mode), so continue searching
        }
    }
}

size_t DocumentLinkManager::getDdeLinkCount() const
{
    sfx2::LinkManager* pMgr = mpImpl->mpLinkManager;
    if (!pMgr)
        return 0;

    size_t nDdeCount = 0;
    const sfx2::SvBaseLinks& rLinks = pMgr->GetLinks();
    for (const auto & rLink : rLinks)
    {
        ::sfx2::SvBaseLink* pBase = rLink.get();
        ScDdeLink* pDdeLink = dynamic_cast<ScDdeLink*>(pBase);
        if (!pDdeLink)
            continue;

        ++nDdeCount;
    }

    return nDdeCount;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
