/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RecentlyUsedMasterPages.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 18:51:39 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "RecentlyUsedMasterPages.hxx"
#include "MasterPageObserver.hxx"
#include "MasterPagesSelector.hxx"
#include "MasterPageDescriptor.hxx"

#include "drawdoc.hxx"
#include "sdpage.hxx"

#include <algorithm>
#include <vector>

#include <comphelper/processfactory.hxx>
#include "unomodel.hxx"
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESSUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGES_HPP_
#include <com/sun/star/drawing/XDrawPages.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif
#include <tools/urlobj.hxx>
#include <unotools/confignode.hxx>
#ifndef INCLUDED_OSL_DOUBLECHECKEDLOCKING_H
#include <osl/doublecheckedlocking.h>
#endif
#ifndef INCLUDED_OSL_GETGLOBALMUTEX_HXX
#include <osl/getglobalmutex.hxx>
#endif

using namespace ::std;
using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


namespace {

static const OUString& GetConfigurationProviderServiceName (void)
{
    static const OUString sConfigurationProviderServiceName (
        RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.configuration.ConfigurationProvider"));
    return sConfigurationProviderServiceName;
}
static const OUString& GetPathToImpressConfigurationRoot (void)
{
    static const OUString sPathToImpressConfigurationRoot (
        RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.Office.Impress/"));
    return sPathToImpressConfigurationRoot;
}
static const OUString& GetPathToSetNode (void)
{
    static const OUString sPathToSetNode(
        RTL_CONSTASCII_USTRINGPARAM(
            "MultiPaneGUI/ToolPanel/RecentlyUsedMasterPages"));
    return sPathToSetNode;
}


class Descriptor
{
public:
    ::rtl::OUString msURL;
    ::rtl::OUString msName;
    ::sd::toolpanel::controls::MasterPageContainer::Token maToken;
    Descriptor (const ::rtl::OUString& rsURL, const ::rtl::OUString& rsName)
        : msURL(rsURL),
          msName(rsName),
          maToken(::sd::toolpanel::controls::MasterPageContainer::NIL_TOKEN)
    {}
    Descriptor (::sd::toolpanel::controls::MasterPageContainer::Token aToken,
        const ::rtl::OUString& rsURL, const ::rtl::OUString& rsName)
        : msURL(rsURL),
          msName(rsName),
          maToken(aToken)
    {}
    class TokenComparator
    { public:
        TokenComparator(::sd::toolpanel::controls::MasterPageContainer::Token aToken)
            : maToken(aToken) {}
        bool operator () (const Descriptor& rDescriptor)
        { return maToken==rDescriptor.maToken; }
    private: ::sd::toolpanel::controls::MasterPageContainer::Token maToken;
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
    else
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();

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




Reference<XInterface> RecentlyUsedMasterPages::OpenConfiguration (
    const OUString& rsRootName,
    bool bReadOnly)
{
    Reference<XInterface> xRoot;
    try
    {
        Reference<lang::XMultiServiceFactory> xProvider (
            ::comphelper::getProcessServiceFactory()->createInstance(
                rsRootName),
            UNO_QUERY);
        if (xProvider.is())
        {
            Sequence<Any> aCreationArguments(3);
            aCreationArguments[0] = makeAny(beans::PropertyValue(
                OUString(
                    RTL_CONSTASCII_USTRINGPARAM("nodepath")),
                0,
                makeAny(GetPathToImpressConfigurationRoot()),
                beans::PropertyState_DIRECT_VALUE));
            aCreationArguments[1] = makeAny(beans::PropertyValue(
                OUString(RTL_CONSTASCII_USTRINGPARAM("depth")),
                0,
                makeAny((sal_Int32)-1),
                beans::PropertyState_DIRECT_VALUE));
            aCreationArguments[2] = makeAny(beans::PropertyValue(
                OUString(RTL_CONSTASCII_USTRINGPARAM("lazywrite")),
                0,
                makeAny(true),
                beans::PropertyState_DIRECT_VALUE));
            OUString sAccessService;
            if (bReadOnly)
                sAccessService = OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.configuration.ConfigurationAccess"));
            else
                sAccessService = OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.configuration.ConfigurationUpdateAccess"));

            xRoot = xProvider->createInstanceWithArguments(
                sAccessService, aCreationArguments);
        }
    }
    catch (Exception& rException)
    {
        OSL_TRACE ("caught exception while opening configuration: %s",
            ::rtl::OUStringToOString(rException.Message,
                RTL_TEXTENCODING_UTF8).getStr());
    }

    return xRoot;
}




Reference<XInterface> RecentlyUsedMasterPages::GetConfigurationNode (
    const Reference<XInterface>& xRoot,
    const OUString& sPathToNode)
{
    Reference<XInterface> xNode;

    try
    {
        Reference<container::XHierarchicalNameAccess> xHierarchy (
            xRoot, UNO_QUERY);
        if (xHierarchy.is())
        {
            xHierarchy->getByHierarchicalName(sPathToNode) >>= xNode;
        }
    }
    catch (Exception& rException)
    {
        OSL_TRACE ("caught exception while getting configuration node %s: %s",
            ::rtl::OUStringToOString(sPathToNode,
                RTL_TEXTENCODING_UTF8).getStr(),
            ::rtl::OUStringToOString(rException.Message,
                RTL_TEXTENCODING_UTF8).getStr());
    }

    return xNode;
}




void RecentlyUsedMasterPages::LoadPersistentValues (void)
{
    try
    {
        do
        {
            Reference<XInterface> xRoot (
                OpenConfiguration(GetConfigurationProviderServiceName(),true));
            if ( ! xRoot.is())
                break;
            Reference<container::XNameAccess> xSet (
                GetConfigurationNode(xRoot, GetPathToSetNode()),
                UNO_QUERY);
            if ( ! xSet.is())
                break;

            const String sURLMemberName (OUString::createFromAscii("URL"));
            const String sNameMemberName (OUString::createFromAscii("Name"));
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
            // Release references to the configuration.
            xSet = NULL;
            xRoot = NULL;

            ResolveList();
        }
        while (false);
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
        do
        {
            Reference<util::XChangesBatch> xRoot (
                OpenConfiguration(GetConfigurationProviderServiceName(),false),
                UNO_QUERY);
            if ( ! xRoot.is())
                break;
            Reference<container::XNameContainer> xSet (
                GetConfigurationNode(xRoot, GetPathToSetNode()),
                UNO_QUERY);
            if ( ! xSet.is())
                break;

            // Clear the set.
            Sequence<OUString> aKeys (xSet->getElementNames());
            sal_Int32 i;
            for (i=0; i<aKeys.getLength(); i++)
                xSet->removeByName (aKeys[i]);

            // Fill it with the URLs of this object.
            const String sURLMemberName (OUString::createFromAscii("URL"));
            const String sNameMemberName (OUString::createFromAscii("Name"));
            Any aValue;
            Reference<lang::XSingleServiceFactory> xChildFactory (
                xSet, UNO_QUERY);
            if ( ! xChildFactory.is())
                break;
            MasterPageList::const_iterator iDescriptor;
            sal_Int32 nIndex(0);
            for (iDescriptor=mpMasterPages->begin();
                 iDescriptor!=mpMasterPages->end();
                 ++iDescriptor,++nIndex)
            {
                // Create new child.
                OUString sKey (OUString::createFromAscii("index_"));
                sKey += OUString::valueOf(nIndex);
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
            xRoot->commitChanges();
        }
        while (false);
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
