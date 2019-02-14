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

#include "RecentlyUsedMasterPages.hxx"
#include "MasterPageContainerProviders.hxx"
#include <MasterPageObserver.hxx>
#include "MasterPagesSelector.hxx"
#include "MasterPageDescriptor.hxx"
#include <tools/ConfigurationAccess.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>

#include <algorithm>
#include <memory>
#include <vector>

#include <unomodel.hxx>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <unotools/confignode.hxx>
#include <osl/doublecheckedlocking.h>
#include <osl/getglobalmutex.hxx>

using namespace ::std;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace {

OUString GetPathToImpressConfigurationRoot()
{
    return OUString("/org.openoffice.Office.Impress/");
}
OUString GetPathToSetNode()
{
    return OUString("MultiPaneGUI/ToolPanel/RecentlyUsedMasterPages");
}

} // end of anonymous namespace

namespace sd { namespace sidebar {

RecentlyUsedMasterPages* RecentlyUsedMasterPages::mpInstance = nullptr;

RecentlyUsedMasterPages&  RecentlyUsedMasterPages::Instance()
{
    if (mpInstance == nullptr)
    {
        ::osl::GetGlobalMutex aMutexFunctor;
        ::osl::MutexGuard aGuard (aMutexFunctor());
        if (mpInstance == nullptr)
        {
            RecentlyUsedMasterPages* pInstance = new RecentlyUsedMasterPages();
            pInstance->LateInit();
            SdGlobalResourceContainer::Instance().AddResource (
                ::std::unique_ptr<SdGlobalResource>(pInstance));
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
            mpInstance = pInstance;
        }
    }
    else {
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
    }

    return *mpInstance;
}

static constexpr size_t gnMaxListSize(8);

RecentlyUsedMasterPages::RecentlyUsedMasterPages()
    : maListeners(),
      mvMasterPages(),
      mpContainer(new MasterPageContainer())
{
}

RecentlyUsedMasterPages::~RecentlyUsedMasterPages()
{
    Link<MasterPageContainerChangeEvent&,void> aLink (LINK(this,RecentlyUsedMasterPages,MasterPageContainerChangeListener));
    mpContainer->RemoveChangeListener(aLink);

    MasterPageObserver::Instance().RemoveEventListener(
        LINK(this,RecentlyUsedMasterPages,MasterPageChangeListener));
}

void RecentlyUsedMasterPages::LateInit()
{
    Link<MasterPageContainerChangeEvent&,void> aLink (LINK(this,RecentlyUsedMasterPages,MasterPageContainerChangeListener));
    mpContainer->AddChangeListener(aLink);

    LoadPersistentValues ();
    MasterPageObserver::Instance().AddEventListener(
        LINK(this,RecentlyUsedMasterPages,MasterPageChangeListener));
}

void RecentlyUsedMasterPages::LoadPersistentValues()
{
    try
    {
        tools::ConfigurationAccess aConfiguration (
            GetPathToImpressConfigurationRoot(),
            tools::ConfigurationAccess::READ_ONLY);
        Reference<container::XNameAccess> xSet (
            aConfiguration.GetConfigurationNode(GetPathToSetNode()),
            UNO_QUERY);
        if ( ! xSet.is())
            return;

        const OUString sURLMemberName("URL");
        const OUString sNameMemberName("Name");
        OUString sURL;
        OUString sName;

        // Read the names and URLs of the master pages.
        Sequence<OUString> aKeys (xSet->getElementNames());
        mvMasterPages.clear();
        mvMasterPages.reserve(aKeys.getLength());
        for (int i=0; i<aKeys.getLength(); i++)
        {
            Reference<container::XNameAccess> xSetItem (
                xSet->getByName(aKeys[i]), UNO_QUERY);
            if (xSetItem.is())
            {
                Any aURL (xSetItem->getByName(sURLMemberName));
                Any aName (xSetItem->getByName(sNameMemberName));
                aURL >>= sURL;
                aName >>= sName;
                SharedMasterPageDescriptor pDescriptor (new MasterPageDescriptor(
                    MasterPageContainer::TEMPLATE,
                    -1,
                    sURL,
                    OUString(),
                    sName,
                    false,
                    std::shared_ptr<PageObjectProvider>(
                        new TemplatePageObjectProvider(sURL)),
                    std::shared_ptr<PreviewProvider>(
                        new TemplatePreviewProvider(sURL))));
                // For user supplied templates we use a different
                // preview provider: The preview in the document shows
                // not only shapes on the master page but also shapes on
                // the foreground.  This is misleading and therefore
                // these previews are discarded and created directly
                // from the page objects.
                if (pDescriptor->GetURLClassification() == MasterPageDescriptor::URLCLASS_USER)
                    pDescriptor->mpPreviewProvider = std::shared_ptr<PreviewProvider>(
                        new PagePreviewProvider());
                MasterPageContainer::Token aToken (mpContainer->PutMasterPage(pDescriptor));
                mvMasterPages.emplace_back(aToken,sURL,sName);
            }
        }

        ResolveList();
    }
    catch (Exception&)
    {
        // Ignore exception.
    }
}

void RecentlyUsedMasterPages::SavePersistentValues()
{
    try
    {
        tools::ConfigurationAccess aConfiguration (
            GetPathToImpressConfigurationRoot(),
            tools::ConfigurationAccess::READ_WRITE);
        Reference<container::XNameContainer> xSet (
            aConfiguration.GetConfigurationNode(GetPathToSetNode()),
            UNO_QUERY);
        if ( ! xSet.is())
            return;

        // Clear the set.
        Sequence<OUString> aKeys (xSet->getElementNames());
        sal_Int32 i;
        for (i=0; i<aKeys.getLength(); i++)
            xSet->removeByName (aKeys[i]);

        // Fill it with the URLs of this object.
        const OUString sURLMemberName("URL");
        const OUString sNameMemberName("Name");
        Any aValue;
        Reference<lang::XSingleServiceFactory> xChildFactory (
            xSet, UNO_QUERY);
        if ( ! xChildFactory.is())
            return;
        sal_Int32 nIndex(0);
        for (const auto& rDescriptor : mvMasterPages)
        {
            // Create new child.
            OUString sKey ("index_");
            sKey += OUString::number(nIndex);
            Reference<container::XNameReplace> xChild(
                xChildFactory->createInstance(), UNO_QUERY);
            if (xChild.is())
            {
                xSet->insertByName (sKey, makeAny(xChild));

                aValue <<= rDescriptor.msURL;
                xChild->replaceByName (sURLMemberName, aValue);

                aValue <<= rDescriptor.msName;
                xChild->replaceByName (sNameMemberName, aValue);
            }
            ++nIndex;
        }

        // Write the data back to disk.
        aConfiguration.CommitChanges();
    }
    catch (Exception&)
    {
        // Ignore exception.
    }
}

void RecentlyUsedMasterPages::AddEventListener (const Link<LinkParamNone*,void>& rEventListener)
{
    if (::std::find (
        maListeners.begin(),
        maListeners.end(),
        rEventListener) == maListeners.end())
    {
        maListeners.push_back (rEventListener);
    }
}

void RecentlyUsedMasterPages::RemoveEventListener (const Link<LinkParamNone*,void>& rEventListener)
{
    maListeners.erase (
        ::std::find (
            maListeners.begin(),
            maListeners.end(),
            rEventListener));
}

int RecentlyUsedMasterPages::GetMasterPageCount() const
{
    return mvMasterPages.size();
}

MasterPageContainer::Token RecentlyUsedMasterPages::GetTokenForIndex (sal_uInt32 nIndex) const
{
    if(nIndex<mvMasterPages.size())
        return mvMasterPages[nIndex].maToken;
    else
        return MasterPageContainer::NIL_TOKEN;
}

void RecentlyUsedMasterPages::SendEvent()
{
    for (auto& aLink : maListeners)
    {
        aLink.Call(nullptr);
    }
}

IMPL_LINK(RecentlyUsedMasterPages, MasterPageChangeListener,
    MasterPageObserverEvent&, rEvent, void)
{
    switch (rEvent.meType)
    {
        case MasterPageObserverEvent::ET_MASTER_PAGE_ADDED:
        case MasterPageObserverEvent::ET_MASTER_PAGE_EXISTS:
            AddMasterPage(
                mpContainer->GetTokenForStyleName(rEvent.mrMasterPageName));
            break;

        case MasterPageObserverEvent::ET_MASTER_PAGE_REMOVED:
            // Do not change the list of recently master pages (the deleted
            // page was recently used) but tell the listeners.  They may want
            // to update their lists.
            SendEvent();
            break;
    }
}

IMPL_LINK(RecentlyUsedMasterPages, MasterPageContainerChangeListener,
    MasterPageContainerChangeEvent&, rEvent, void)
{
    switch (rEvent.meEventType)
    {
        case MasterPageContainerChangeEvent::EventType::CHILD_ADDED:
        case MasterPageContainerChangeEvent::EventType::CHILD_REMOVED:
        case MasterPageContainerChangeEvent::EventType::INDEX_CHANGED:
            ResolveList();
            break;

        default:
            // Ignored.
            break;
    }
}

void RecentlyUsedMasterPages::AddMasterPage (
    MasterPageContainer::Token aToken)
{
    // For the page to be inserted the token has to be valid and the page
    // has to have a valid URL.  This excludes master pages that do not come
    // from template files.
    if (aToken == MasterPageContainer::NIL_TOKEN
        || mpContainer->GetURLForToken(aToken).isEmpty())
        return;

    MasterPageList::iterator aIterator (
        ::std::find_if(mvMasterPages.begin(),mvMasterPages.end(),
            Descriptor::TokenComparator(aToken)));
    if (aIterator != mvMasterPages.end())
    {
        // When an entry for the given token already exists then remove
        // it now and insert it later at the head of the list.
        mvMasterPages.erase (aIterator);
    }

    mvMasterPages.insert(mvMasterPages.begin(),
        Descriptor(
            aToken,
            mpContainer->GetURLForToken(aToken),
            mpContainer->GetStyleNameForToken(aToken)));

    // Shorten list to maximal size.
    while (mvMasterPages.size() > gnMaxListSize)
    {
        mvMasterPages.pop_back ();
    }

    SavePersistentValues ();
    SendEvent();
}

void RecentlyUsedMasterPages::ResolveList()
{
    bool bNotify (false);

    for (auto& rDescriptor : mvMasterPages)
    {
        if (rDescriptor.maToken == MasterPageContainer::NIL_TOKEN)
        {
            MasterPageContainer::Token aToken (mpContainer->GetTokenForURL(rDescriptor.msURL));
            rDescriptor.maToken = aToken;
            if (aToken != MasterPageContainer::NIL_TOKEN)
                bNotify = true;
        }
        else
        {
            if ( ! mpContainer->HasToken(rDescriptor.maToken))
            {
                rDescriptor.maToken = MasterPageContainer::NIL_TOKEN;
                bNotify = true;
            }
        }
    }

    if (bNotify)
        SendEvent();
}

} } // end of namespace sd::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
