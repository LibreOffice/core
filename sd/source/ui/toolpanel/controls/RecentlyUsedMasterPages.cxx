/*************************************************************************
 *
 *  $RCSfile: RecentlyUsedMasterPages.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:47:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "RecentlyUsedMasterPages.hxx"
#include "MasterPageObserver.hxx"
#include "MasterPagesSelector.hxx"

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

using namespace ::std;
using namespace ::rtl;
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
} // end of anonymous namespace




namespace sd { namespace toolpanel { namespace controls {


RecentlyUsedMasterPages* RecentlyUsedMasterPages::mpInstance = NULL;
::osl::Mutex RecentlyUsedMasterPages::maMutex;


RecentlyUsedMasterPages&  RecentlyUsedMasterPages::Instance (void)
{
    if (mpInstance == NULL)
    {
        ::osl::MutexGuard aGuard (maMutex);
        if (mpInstance == NULL)
        {
            mpInstance = new RecentlyUsedMasterPages ();
            mpInstance->LateInit();
        }
    }

    return *mpInstance;
}




RecentlyUsedMasterPages::RecentlyUsedMasterPages (void)
    : maListeners(),
      maMasterPages(),
      mnMaxListSize(8)
{
}




void RecentlyUsedMasterPages::LateInit (void)
{
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
            vector<pair<OUString,OUString> > aMasterPages;
            aMasterPages.reserve(aKeys.getLength());
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
                    aMasterPages.push_back (
                        pair<OUString,OUString>(sURL,sName));
                }
            }
            // Release references to the configuration.
            xSet = NULL;
            xRoot = NULL;

            for (unsigned int j=0; j<aMasterPages.size(); j++)
            {
                MasterPageContainer::Instance().PutMasterPage (
                    aMasterPages[j].first,
                    aMasterPages[j].second,
                    NULL,
                    Image());
                AddMasterPage (aMasterPages[j].second, false);
            }
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
            for (i=0; i<GetMasterPageCount(); i++)
            {
                // Create new child.
                OUString sKey (OUString::createFromAscii("index_"));
                sKey += OUString::valueOf(i);
                Reference<container::XNameReplace> xChild(
                    xChildFactory->createInstance(), UNO_QUERY);
                if (xChild.is())
                {
                    xSet->insertByName (sKey, makeAny(xChild));

                    aValue <<= OUString(GetURL(i));
                    xChild->replaceByName (sURLMemberName, aValue);

                    aValue <<= OUString(GetMasterPageName(i));
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




RecentlyUsedMasterPages::~RecentlyUsedMasterPages (void)
{
    MasterPageObserver::Instance().RemoveEventListener(
        LINK(this,RecentlyUsedMasterPages,MasterPageChangeListener));
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
    return maMasterPages.size();
}




String RecentlyUsedMasterPages::GetURL (int nIndex) const
{
    MasterPageContainer::Token aToken (maMasterPages[nIndex]);
    return MasterPageContainer::Instance().GetURLForToken (aToken);
}




String RecentlyUsedMasterPages::GetMasterPageName (int nIndex) const
{
    MasterPageContainer::Token aToken (maMasterPages[nIndex]);
    return MasterPageContainer::Instance().GetPageNameForToken (aToken);
}




SdPage* RecentlyUsedMasterPages::GetSlide (int nIndex) const
{
    MasterPageContainer::Token aToken (maMasterPages[nIndex]);
    return MasterPageContainer::Instance().GetSlideForToken (aToken);
}




SdPage* RecentlyUsedMasterPages::GetMasterPage (int nIndex) const
{
    MasterPageContainer::Token aToken (maMasterPages[nIndex]);
    return MasterPageContainer::Instance().GetPageObjectForToken (aToken);
}




Image RecentlyUsedMasterPages::GetMasterPagePreview (
    int nIndex, int nWidth) const
{
    MasterPageContainer::Token aToken (maMasterPages[nIndex]);
    return MasterPageContainer::Instance().GetPreviewForToken (aToken, nWidth);
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
            AddMasterPage (pEvent->mrMasterPageName);
            break;

        case MasterPageObserverEvent::ET_MASTER_PAGE_REMOVED:
            // Do not change the list of recently master pages (the deleted
            // page was recently used) but tell the listeners how may want
            // to update their lists of recently used master pages that are
            // not currently in used.
            SendEvent();
            break;
    }
    return 0;
}




void RecentlyUsedMasterPages::AddMasterPage (
    const String& rsName,
    bool bMakePersistent)
{
    MasterPageContainer::Token aToken (
        MasterPageContainer::Instance().GetTokenForPageName(rsName));
    // For the page to be inserted the token has to be valid and the page
    // has to have a valid URL.  This excludes master pages that do not come
    // from template files.
    if (aToken != MasterPageContainer::NIL_TOKEN
        && MasterPageContainer::Instance().GetURLForToken(aToken).Len()>0)
    {

        MasterPageList::iterator aIterator (
            ::std::find(maMasterPages.begin(),maMasterPages.end(),aToken));
        if (aIterator != maMasterPages.end())
        {
            // Move existing entry to head of list and thus make it most
            // recently used.
            maMasterPages.erase (aIterator);
        }
        maMasterPages.insert (maMasterPages.begin(), aToken);

        // Shorten list to maximal size.
        while (maMasterPages.size() > mnMaxListSize)
        {
            maMasterPages.pop_back ();
        }

        if (bMakePersistent)
            SavePersistentValues ();
        SendEvent();
    }
}


} } } // end of namespace ::sd::toolpanel::controls
