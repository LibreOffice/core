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

#include <documentlinkmgr.hxx>
#include <datastream.hxx>
#include <ddelink.hxx>
#include <strings.hrc>
#include <scresid.hxx>

#include <svx/svdoole2.hxx>
#include <vcl/layout.hxx>

#include <memory>

namespace sc {

struct DocumentLinkManagerImpl
{
    SfxObjectShell* mpShell;
    std::unique_ptr<DataStream> mpDataStream;
    std::unique_ptr<sfx2::LinkManager> mpLinkManager;

    DocumentLinkManagerImpl(const DocumentLinkManagerImpl&) = delete;
    const DocumentLinkManagerImpl& operator=(const DocumentLinkManagerImpl&) = delete;

    explicit DocumentLinkManagerImpl(SfxObjectShell* pShell)
        : mpShell(pShell), mpDataStream(nullptr), mpLinkManager(nullptr) {}

    ~DocumentLinkManagerImpl()
    {
        // Shared base links
        if (mpLinkManager)
        {
            sfx2::SvLinkSources aTemp = mpLinkManager->GetServers();
            for (sfx2::SvLinkSources::const_iterator it = aTemp.begin(); it != aTemp.end(); ++it)
                (*it)->Closed();

            if (!mpLinkManager->GetLinks().empty())
                mpLinkManager->Remove(0, mpLinkManager->GetLinks().size());
        }
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
    if (!mpImpl->mpLinkManager && bCreate && mpImpl->mpShell)
        mpImpl->mpLinkManager.reset(new sfx2::LinkManager(mpImpl->mpShell));
    return mpImpl->mpLinkManager.get();
}

const sfx2::LinkManager* DocumentLinkManager::getExistingLinkManager() const
{
    return mpImpl->mpLinkManager.get();
}

bool DocumentLinkManager::idleCheckLinks()
{
    if (!mpImpl->mpLinkManager)
        return false;

    bool bAnyLeft = false;
    const sfx2::SvBaseLinks& rLinks = mpImpl->mpLinkManager->GetLinks();
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
    return hasDdeOrOleLinks(true, false);
}

bool DocumentLinkManager::hasDdeOrOleLinks() const
{
    return hasDdeOrOleLinks(true, true);
}

bool DocumentLinkManager::hasDdeOrOleLinks(bool bDde, bool bOle) const
{
    if (!mpImpl->mpLinkManager)
        return false;

    const sfx2::SvBaseLinks& rLinks = mpImpl->mpLinkManager->GetLinks();
    for (const auto & rLink : rLinks)
    {
        sfx2::SvBaseLink* pBase = rLink.get();
        if (bDde && dynamic_cast<ScDdeLink*>(pBase))
            return true;
        if (bOle && dynamic_cast<SdrEmbedObjectLink*>(pBase))
            return true;
    }

    return false;
}

bool DocumentLinkManager::updateDdeOrOleLinks( vcl::Window* pWin )
{
    if (!mpImpl->mpLinkManager)
        return false;

    sfx2::LinkManager* pMgr = mpImpl->mpLinkManager.get();
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

        ScDdeLink* pDdeLink = dynamic_cast<ScDdeLink*>(pBase);
        if (!pDdeLink)
            continue;

        if (pDdeLink->Update())
            bAny = true;
        else
        {
            // Update failed.  Notify the user.
            OUString aFile = pDdeLink->GetTopic();
            OUString aElem = pDdeLink->GetItem();
            OUString aType = pDdeLink->GetAppl();

            OUStringBuffer aBuf;
            aBuf.append(ScResId(SCSTR_DDEDOC_NOT_LOADED));
            aBuf.append("\n\n");
            aBuf.append("Source : ");
            aBuf.append(aFile);
            aBuf.append("\nElement : ");
            aBuf.append(aElem);
            aBuf.append("\nType : ");
            aBuf.append(aType);
            ScopedVclPtrInstance< MessageDialog > aBox(pWin, aBuf.makeStringAndClear());
            aBox->Execute();
        }
    }

    pMgr->CloseCachedComps();

    return bAny;
}

void DocumentLinkManager::updateDdeLink( const OUString& rAppl, const OUString& rTopic, const OUString& rItem )
{
    if (!mpImpl->mpLinkManager)
        return;

    sfx2::LinkManager* pMgr = mpImpl->mpLinkManager.get();
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
    if (!mpImpl->mpLinkManager)
        return 0;

    size_t nDdeCount = 0;
    const sfx2::SvBaseLinks& rLinks = mpImpl->mpLinkManager->GetLinks();
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

void DocumentLinkManager::disconnectDdeLinks()
{
    if (!mpImpl->mpLinkManager)
        return;

    const sfx2::SvBaseLinks& rLinks = mpImpl->mpLinkManager->GetLinks();
    for (const auto & rLink : rLinks)
    {
        ::sfx2::SvBaseLink* pBase = rLink.get();
        ScDdeLink* pDdeLink = dynamic_cast<ScDdeLink*>(pBase);
        if (pDdeLink)
            pDdeLink->Disconnect();
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
