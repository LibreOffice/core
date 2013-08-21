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
#include "MasterPageObserver.hxx"
#include "MasterPagesSelector.hxx"
#include "MasterPageDescriptor.hxx"
#include "tools/ConfigurationAccess.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"

#include <algorithm>
#include <vector>

#include <comphelper/processfactory.hxx>
#include "unomodel.hxx"
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
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

static const OUString& GetPathToImpressConfigurationRoot (void)
{
    static const OUString sPathToImpressConfigurationRoot ("/org.openoffice.Office.Impress/");
    return sPathToImpressConfigurationRoot;
}
static const OUString& GetPathToSetNode (void)
{
    static const OUString sPathToSetNode("MultiPaneGUI/ToolPanel/RecentlyUsedMasterPages");
    return sPathToSetNode;
}


class Descriptor
{
public:
    OUString msURL;
    OUString msName;
    ::sd::toolpanel::controls::MasterPageContainer::Token maToken;
    Descriptor (const OUString& rsURL, const OUString& rsName)
        : msURL(rsURL),
          msName(rsName),
          maToken(::sd::toolpanel::controls::MasterPageContainer::NIL_TOKEN)
    {}
    Descriptor (::sd::toolpanel::controls::MasterPageContainer::Token aToken,
        const OUString& rsURL, const OUString& rsName)
        : msURL(rsURL),
          msName(rsName),
          maToken(aToken)
    {}
    class TokenComparator
    {
    public:
        TokenComparator(::sd::toolpanel::controls::MasterPageContainer::Token aToken)
            : maToken(aToken) {}
        bool operator () (const Descriptor& rDescriptor) const
            { return maToken==rDescriptor.maToken; }
    private:
        ::sd::toolpanel::controls::MasterPageContainer::Token maToken;
    };
};

} // end of anonymous namespace




namespace sd { namespace toolpanel { namespace controls {

class RecentlyUsedMasterPages::MasterPageList : public ::std::vector<Descriptor>
{
public:
    MasterPageList (void) {}
};


RecentlyUsedMasterPages* RecentlyUsedMasterPages::mpInstance = NULL;


RecentlyUsedMasterPages&  RecentlyUsedMasterPages::Instance (void)
{
    if (mpInstance == NULL)
    {
        ::osl::GetGlobalMutex aMutexFunctor;
        ::osl::MutexGuard aGuard (aMutexFunctor());
        if (mpInstance == NULL)
        {
            RecentlyUsedMasterPages* pInstance = new RecentlyUsedMasterPages();
            pInstance->LateInit();
            SdGlobalResourceContainer::Instance().AddResource (
                ::std::auto_ptr<SdGlobalResource>(pInstance));
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
            mpInstance = pInstance;
        }
    }
    else {
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
    }

    return *mpInstance;
}




RecentlyUsedMasterPages::RecentlyUsedMasterPages (void)
    : maListeners(),
      mpMasterPages(new MasterPageList()),
      mnMaxListSize(8),
      mpContainer(new MasterPageContainer())
{
}




RecentlyUsedMasterPages::~RecentlyUsedMasterPages (void)
{
    Link aLink (LINK(this,RecentlyUsedMasterPages,MasterPageContainerChangeListener));
    mpContainer->RemoveChangeListener(aLink);

    MasterPageObserver::Instance().RemoveEventListener(
        LINK(this,RecentlyUsedMasterPages,MasterPageChangeListener));
}




void RecentlyUsedMasterPages::LateInit (void)
{
    Link aLink (LINK(this,RecentlyUsedMasterPages,MasterPageContainerChangeListener));
    mpContainer->AddChangeListener(aLink);

    LoadPersistentValues ();
    MasterPageObserver::Instance().AddEventListener(
        LINK(this,RecentlyUsedMasterPages,MasterPageChangeListener));
}




void RecentlyUsedMasterPages::LoadPersistentValues (void)
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
        mpMasterPages->clear();
        mpMasterPages->reserve(aKeys.getLength());
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
                    String(),
                    sName,
                    false,
                    ::boost::shared_ptr<PageObjectProvider>(
                        new TemplatePageObjectProvider(sURL)),
                    ::boost::shared_ptr<PreviewProvider>(
                        new TemplatePreviewProvider(sURL))));
                // For user supplied templates we use a different
                // preview provider: The preview in the document shows
                // not only shapes on the master page but also shapes on
                // the foreground.  This is misleading and therefore
                // these previews are discarded and created directly
                // from the page objects.
                if (pDescriptor->GetURLClassification() == MasterPageDescriptor::URLCLASS_USER)
                    pDescriptor->mpPreviewProvider = ::boost::shared_ptr<PreviewProvider>(
                        new PagePreviewProvider());
                MasterPageContainer::Token aToken (mpContainer->PutMasterPage(pDescriptor));
                mpMasterPages->push_back(Descriptor(aToken,sURL,sName));
            }
        }

        ResolveList();
    }
    catch (Exception&)
    {
        // Ignore exception.
    }
}




void RecentlyUsedMasterPages::SavePersistentValues (void)
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
        MasterPageList::const_iterator iDescriptor;
        sal_Int32 nIndex(0);
        for (iDescriptor=mpMasterPages->begin();
                iDescriptor!=mpMasterPages->end();
                ++iDescriptor,++nIndex)
        {
            // Create new child.
            OUString sKey ("index_");
            sKey += OUString::number(nIndex);
            Reference<container::XNameReplace> xChild(
                xChildFactory->createInstance(), UNO_QUERY);
            if (xChild.is())
            {
                xSet->insertByName (sKey, makeAny(xChild));

                aValue <<= OUString(iDescriptor->msURL);
                xChild->replaceByName (sURLMemberName, aValue);

                aValue <<= OUString(iDescriptor->msName);
                xChild->replaceByName (sNameMemberName, aValue);
            }
        }

        // Write the data back to disk.
        aConfiguration.CommitChanges();
    }
    catch (Exception&)
    {
        // Ignore exception.
    }
}




void RecentlyUsedMasterPages::AddEventListener (const Link& rEventListener)
{
    if (::std::find (
        maListeners.begin(),
        maListeners.end(),
        rEventListener) == maListeners.end())
    {
        maListeners.push_back (rEventListener);
    }
}




void RecentlyUsedMasterPages::RemoveEventListener (const Link& rEventListener)
{
    maListeners.erase (
        ::std::find (
            maListeners.begin(),
            maListeners.end(),
            rEventListener));
}




int RecentlyUsedMasterPages::GetMasterPageCount (void) const
{
    return mpMasterPages->size();
}




MasterPageContainer::Token RecentlyUsedMasterPages::GetTokenForIndex (sal_uInt32 nIndex) const
{
    if(nIndex<mpMasterPages->size())
        return (*mpMasterPages)[nIndex].maToken;
    else
        return MasterPageContainer::NIL_TOKEN;
}




void RecentlyUsedMasterPages::SendEvent (void)
{
    ::std::vector<Link>::iterator aLink (maListeners.begin());
    ::std::vector<Link>::iterator aEnd (maListeners.end());
    while (aLink!=aEnd)
    {
        aLink->Call (NULL);
        ++aLink;
    }
}




IMPL_LINK(RecentlyUsedMasterPages, MasterPageChangeListener,
    MasterPageObserverEvent*, pEvent)
{
    switch (pEvent->meType)
    {
        case MasterPageObserverEvent::ET_MASTER_PAGE_ADDED:
        case MasterPageObserverEvent::ET_MASTER_PAGE_EXISTS:
            AddMasterPage(
                mpContainer->GetTokenForStyleName(pEvent->mrMasterPageName));
            break;

        case MasterPageObserverEvent::ET_MASTER_PAGE_REMOVED:
            // Do not change the list of recently master pages (the deleted
            // page was recently used) but tell the listeners.  They may want
            // to update their lists.
            SendEvent();
            break;
    }
    return 0;
}




IMPL_LINK(RecentlyUsedMasterPages, MasterPageContainerChangeListener,
    MasterPageContainerChangeEvent*, pEvent)
{
    if (pEvent != NULL)
        switch (pEvent->meEventType)
        {
            case MasterPageContainerChangeEvent::CHILD_ADDED:
            case MasterPageContainerChangeEvent::CHILD_REMOVED:
            case MasterPageContainerChangeEvent::INDEX_CHANGED:
            case MasterPageContainerChangeEvent::INDEXES_CHANGED:
                ResolveList();
                break;

            default:
                // Ignored.
                break;
        }
    return 0;
}




void RecentlyUsedMasterPages::AddMasterPage (
    MasterPageContainer::Token aToken,
    bool bMakePersistent)
{
    // For the page to be inserted the token has to be valid and the page
    // has to have a valid URL.  This excludes master pages that do not come
    // from template files.
    if (aToken != MasterPageContainer::NIL_TOKEN
        && mpContainer->GetURLForToken(aToken).Len()>0)
    {

        MasterPageList::iterator aIterator (
            ::std::find_if(mpMasterPages->begin(),mpMasterPages->end(),
                Descriptor::TokenComparator(aToken)));
        if (aIterator != mpMasterPages->end())
        {
            // When an entry for the given token already exists then remove
            // it now and insert it later at the head of the list.
            mpMasterPages->erase (aIterator);
        }

        mpMasterPages->insert(mpMasterPages->begin(),
            Descriptor(
                aToken,
                mpContainer->GetURLForToken(aToken),
                mpContainer->GetStyleNameForToken(aToken)));

        // Shorten list to maximal size.
        while (mpMasterPages->size() > mnMaxListSize)
        {
            mpMasterPages->pop_back ();
        }

        if (bMakePersistent)
            SavePersistentValues ();
        SendEvent();
    }
}




void RecentlyUsedMasterPages::ResolveList (void)
{
    bool bNotify (false);

    MasterPageList::iterator iDescriptor;
    for (iDescriptor=mpMasterPages->begin(); iDescriptor!=mpMasterPages->end(); ++iDescriptor)
    {
        if (iDescriptor->maToken == MasterPageContainer::NIL_TOKEN)
        {
            MasterPageContainer::Token aToken (mpContainer->GetTokenForURL(iDescriptor->msURL));
            iDescriptor->maToken = aToken;
            if (aToken != MasterPageContainer::NIL_TOKEN)
                bNotify = true;
        }
        else
        {
            if ( ! mpContainer->HasToken(iDescriptor->maToken))
            {
                iDescriptor->maToken = MasterPageContainer::NIL_TOKEN;
                bNotify = true;
            }
        }
    }

    if (bNotify)
        SendEvent();
}


} } } // end of namespace ::sd::toolpanel::controls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
