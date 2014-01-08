/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <documentlinkmgr.hxx>
#include <datastream.hxx>
#include <sfx2/linkmgr.hxx>

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

namespace sc {

struct DocumentLinkManagerImpl : boost::noncopyable
{
    ScDocument& mrDoc;
    SfxObjectShell* mpShell;
    boost::scoped_ptr<DataStream> mpDataStream;
    boost::scoped_ptr<sfx2::LinkManager> mpLinkManager;

    DocumentLinkManagerImpl( ScDocument& rDoc, SfxObjectShell* pShell ) :
        mrDoc(rDoc), mpShell(pShell), mpDataStream(NULL), mpLinkManager(NULL) {}

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

DocumentLinkManager::DocumentLinkManager( ScDocument& rDoc, SfxObjectShell* pShell ) :
    mpImpl(new DocumentLinkManagerImpl(rDoc, pShell)) {}

DocumentLinkManager::~DocumentLinkManager()
{
    delete mpImpl;
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

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
