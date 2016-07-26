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


#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#include <com/sun/star/document/XTypeDetection.hpp>
#include <com/sun/star/container/XContainerQuery.hpp>

#include <comphelper/sequenceashashmap.hxx>

#include <sot/exchange.hxx>
#include <basic/sbmeth.hxx>
#include <basic/basmgr.hxx>
#include <basic/sbstar.hxx>
#include <basic/sbxobj.hxx>
#include <basic/sbxmeth.hxx>
#include <basic/sbxcore.hxx>
#include <vcl/msgbox.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <svl/lckbitem.hxx>
#include <svl/inettype.hxx>
#include <svl/rectitem.hxx>

#include <sot/storage.hxx>
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <com/sun/star/frame/FeatureStateEvent.hpp>
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/FrameActionEvent.hpp>
#include <com/sun/star/frame/FrameAction.hpp>
#include <com/sun/star/frame/XFrameLoader.hpp>
#include <com/sun/star/frame/XLoadEventListener.hpp>
#include <com/sun/star/frame/XFilterDetect.hpp>
#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <comphelper/processfactory.hxx>

#include <sal/types.h>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/ucb/XContent.hpp>
#include <unotools/pathoptions.hxx>
#include <unotools/moduleoptions.hxx>
#include <unotools/mediadescriptor.hxx>
#include <tools/urlobj.hxx>

#include <rtl/instance.hxx>

#include <svl/ctypeitm.hxx>
#include <svtools/sfxecode.hxx>
#include <unotools/syslocale.hxx>

#include <sfx2/sfxhelp.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfac.hxx>
#include "sfxtypes.hxx"
#include <sfx2/sfxuno.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/progress.hxx>
#include "openflag.hxx"
#include "bastyp.hrc"
#include <sfx2/sfxresid.hxx>
#include <sfx2/doctempl.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include "helper.hxx"
#include "fltlst.hxx"
#include <sfx2/request.hxx>
#include "arrdecl.hxx"
#include <o3tl/make_unique.hxx>

#include <vector>
#include <memory>
#include <functional>

#if defined(DBG_UTIL)
unsigned SfxStack::nLevel = 0;
#endif

using namespace com::sun::star;

namespace
{
    class theSfxFilterListener : public rtl::Static<SfxFilterListener, theSfxFilterListener> {};
    class SfxFilterArray
    {
        SfxFilterList_Impl aList;
    public:

        SfxFilterList_Impl& getList()
        {
            return aList;
        }
    };
    class theSfxFilterArray : public rtl::Static<SfxFilterArray, theSfxFilterArray > {};
}

static SfxFilterList_Impl* pFilterArr = nullptr;
static bool bFirstRead = true;

static void CreateFilterArr()
{
    pFilterArr = &theSfxFilterArray::get().getList();
    theSfxFilterListener::get();
}

inline OUString ToUpper_Impl( const OUString &rStr )
{
    return SvtSysLocale().GetCharClass().uppercase( rStr );
}

class SfxFilterContainer_Impl
{
public:
    OUString            aName;
    OUString            aServiceName;

    explicit SfxFilterContainer_Impl( const OUString& rName )
        : aName( rName )
    {
        aServiceName = SfxObjectShell::GetServiceNameFromFactory( rName );
    }
};

std::shared_ptr<const SfxFilter> SfxFilterContainer::GetFilter4EA(const OUString& rEA, SfxFilterFlags nMust, SfxFilterFlags nDont) const
{
    SfxFilterMatcher aMatch(pImpl->aName);
    return aMatch.GetFilter4EA(rEA, nMust, nDont);
}

std::shared_ptr<const SfxFilter> SfxFilterContainer::GetFilter4Extension(const OUString& rExt, SfxFilterFlags nMust, SfxFilterFlags nDont) const
{
    SfxFilterMatcher aMatch(pImpl->aName);
    return aMatch.GetFilter4Extension(rExt, nMust, nDont);
}

std::shared_ptr<const SfxFilter> SfxFilterContainer::GetFilter4FilterName(const OUString& rName, SfxFilterFlags nMust, SfxFilterFlags nDont) const
{
    SfxFilterMatcher aMatch(pImpl->aName);
    return aMatch.GetFilter4FilterName(rName, nMust, nDont);
}

std::shared_ptr<const SfxFilter> SfxFilterContainer::GetAnyFilter( SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    SfxFilterMatcher aMatch( pImpl->aName );
    return aMatch.GetAnyFilter( nMust, nDont );
}


SfxFilterContainer::SfxFilterContainer( const OUString& rName )
   : pImpl( new SfxFilterContainer_Impl( rName ) )
{
}


SfxFilterContainer::~SfxFilterContainer()
{
}


const OUString SfxFilterContainer::GetName() const
{
    return pImpl->aName;
}

std::shared_ptr<const SfxFilter> SfxFilterContainer::GetDefaultFilter_Impl( const OUString& rName )
{
    // Try to find out the type of factory.
    // Interpret given name as Service- and ShortName!
    SvtModuleOptions aOpt;
    SvtModuleOptions::EFactory eFactory = SvtModuleOptions::ClassifyFactoryByServiceName(rName);
    if (eFactory == SvtModuleOptions::EFactory::UNKNOWN_FACTORY)
        eFactory = SvtModuleOptions::ClassifyFactoryByShortName(rName);

    // could not classify factory by its service nor by its short name.
    // Must be an unknown factory! => return NULL
    if (eFactory == SvtModuleOptions::EFactory::UNKNOWN_FACTORY)
        return nullptr;

    // For the following code we need some additional information.
    OUString sServiceName   = aOpt.GetFactoryName(eFactory);
    OUString sDefaultFilter = aOpt.GetFactoryDefaultFilter(eFactory);

    // Try to get the default filter. Don't forget to verify it.
    // May the set default filter does not exists any longer or
    // does not fit the given factory.
    const SfxFilterMatcher aMatcher;
    std::shared_ptr<const SfxFilter> pFilter = aMatcher.GetFilter4FilterName(sDefaultFilter);

    if (
        pFilter &&
        !pFilter->GetServiceName().equalsIgnoreAsciiCase(sServiceName)
       )
    {
        pFilter = nullptr;
    }

    // If at least no default filter could be located - use any filter of this
    // factory.
    if (!pFilter)
    {
        if ( bFirstRead )
            ReadFilters_Impl();

        for (std::shared_ptr<const SfxFilter>& pCheckFilter : *pFilterArr)
        {
            if ( pCheckFilter->GetServiceName().equalsIgnoreAsciiCase(sServiceName) )
            {
                pFilter = pCheckFilter;
                break;
            }
        }
    }

    return pFilter;
}


// Impl-Data is shared between all FilterMatchers of the same factory
class SfxFilterMatcher_Impl
{
public:
    OUString     aName;
    mutable SfxFilterList_Impl* pList;      // is created on demand

    void InitForIterating() const;
    void Update() const;
    explicit SfxFilterMatcher_Impl(const OUString &rName)
        : aName(rName)
        , pList(nullptr)
    {
    }
    ~SfxFilterMatcher_Impl()
    {
        // SfxFilterMatcher_Impl::InitForIterating() will set pList to
        // either the global filter array matcher pFilterArr, or to
        // a new SfxFilterList_Impl.
        if (pList != pFilterArr)
            delete pList;
    }
};

namespace
{
    typedef std::vector<std::unique_ptr<SfxFilterMatcher_Impl> > SfxFilterMatcherArr_Impl;
    static SfxFilterMatcherArr_Impl aImplArr;
    static int nSfxFilterMatcherCount;

    SfxFilterMatcher_Impl & getSfxFilterMatcher_Impl(const OUString &rName)
    {
        OUString aName;

        if (!rName.isEmpty())
            aName = SfxObjectShell::GetServiceNameFromFactory(rName);

        // find the impl-Data of any comparable FilterMatcher that was created
        // previously
        for (std::unique_ptr<SfxFilterMatcher_Impl>& aImpl : aImplArr)
            if (aImpl->aName == aName)
                return *aImpl.get();

        // first Matcher created for this factory
        aImplArr.push_back(o3tl::make_unique<SfxFilterMatcher_Impl>(aName));
        return *aImplArr.back().get();
    }
}

SfxFilterMatcher::SfxFilterMatcher( const OUString& rName )
    : m_rImpl( getSfxFilterMatcher_Impl(rName) )
{
    ++nSfxFilterMatcherCount;
}

SfxFilterMatcher::SfxFilterMatcher()
    : m_rImpl( getSfxFilterMatcher_Impl(OUString()) )
{
    // global FilterMatcher always uses global filter array (also created on
    // demand)
    ++nSfxFilterMatcherCount;
}

SfxFilterMatcher::~SfxFilterMatcher()
{
    --nSfxFilterMatcherCount;
    if (nSfxFilterMatcherCount == 0)
        aImplArr.clear();
}

void SfxFilterMatcher_Impl::Update() const
{
    if ( pList )
    {
        // this List was already used
        pList->clear();
        for (std::shared_ptr<const SfxFilter>& pFilter : *pFilterArr)
        {
            if ( pFilter->GetServiceName() == aName )
                pList->push_back( pFilter );
        }
    }
}

void SfxFilterMatcher_Impl::InitForIterating() const
{
    if ( pList )
        return;

    if ( bFirstRead )
        // global filter array has not been created yet
        SfxFilterContainer::ReadFilters_Impl();

    if ( !aName.isEmpty() )
    {
        // matcher of factory: use only filters of that document type
        pList = new SfxFilterList_Impl;
        Update();
    }
    else
    {
        // global matcher: use global filter array
        pList = pFilterArr;
    }
}

std::shared_ptr<const SfxFilter> SfxFilterMatcher::GetAnyFilter( SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    m_rImpl.InitForIterating();
    for (std::shared_ptr<const SfxFilter>& pFilter : *m_rImpl.pList)
    {
        SfxFilterFlags nFlags = pFilter->GetFilterFlags();
        if ( (nFlags & nMust) == nMust && !(nFlags & nDont ) )
            return pFilter;
    }

    return nullptr;
}


sal_uInt32  SfxFilterMatcher::GuessFilterIgnoringContent(
    SfxMedium& rMedium,
    std::shared_ptr<const SfxFilter>& rpFilter ) const
{
    uno::Reference<document::XTypeDetection> xDetection(
        comphelper::getProcessServiceFactory()->createInstance("com.sun.star.document.TypeDetection"), uno::UNO_QUERY);

    OUString sTypeName;
    try
    {
        sTypeName = xDetection->queryTypeByURL( rMedium.GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) );
    }
    catch (uno::Exception&)
    {
    }

    rpFilter = nullptr;
    if ( !sTypeName.isEmpty() )
    {
        // make sure filter list is initialized
        m_rImpl.InitForIterating();
        rpFilter = GetFilter4EA( sTypeName );
    }

    return rpFilter ? ERRCODE_NONE : ERRCODE_ABORT;
}


sal_uInt32  SfxFilterMatcher::GuessFilter( SfxMedium& rMedium, std::shared_ptr<const SfxFilter>& rpFilter, SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    return GuessFilterControlDefaultUI( rMedium, rpFilter, nMust, nDont );
}


sal_uInt32  SfxFilterMatcher::GuessFilterControlDefaultUI( SfxMedium& rMedium, std::shared_ptr<const SfxFilter>& rpFilter, SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    std::shared_ptr<const SfxFilter> pOldFilter = rpFilter;

    // no detection service -> nothing to do !
    uno::Reference<document::XTypeDetection> xDetection(
        comphelper::getProcessServiceFactory()->createInstance("com.sun.star.document.TypeDetection"), uno::UNO_QUERY);

    if (!xDetection.is())
        return ERRCODE_ABORT;

    OUString sTypeName;
    try
    {
        // open the stream one times only ...
        // Otherwhise it will be tried more than once and show the same interaction more than once ...

        OUString sURL( rMedium.GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) );
        uno::Reference< io::XInputStream > xInStream = rMedium.GetInputStream();
        OUString aFilterName;

        // stream exists => deep detection (with preselection ... if possible)
        if (xInStream.is())
        {
            utl::MediaDescriptor aDescriptor;

            aDescriptor[utl::MediaDescriptor::PROP_URL()               ] <<= sURL;
            aDescriptor[utl::MediaDescriptor::PROP_INPUTSTREAM()       ] <<= xInStream;
            aDescriptor[utl::MediaDescriptor::PROP_INTERACTIONHANDLER()] <<= rMedium.GetInteractionHandler();
            SfxStringItem const * it = static_cast<SfxStringItem const *>(
                rMedium.GetItemSet()->GetItem(SID_REFERER));
            if (it != nullptr) {
                aDescriptor[utl::MediaDescriptor::PROP_REFERRER()]
                    <<= it->GetValue();
            }

            if ( !m_rImpl.aName.isEmpty() )
                aDescriptor[utl::MediaDescriptor::PROP_DOCUMENTSERVICE()] <<= m_rImpl.aName;

            if ( pOldFilter )
            {
                aDescriptor[utl::MediaDescriptor::PROP_TYPENAME()  ] <<= OUString( pOldFilter->GetTypeName()   );
                aDescriptor[utl::MediaDescriptor::PROP_FILTERNAME()] <<= OUString( pOldFilter->GetFilterName() );
            }

            uno::Sequence< beans::PropertyValue > lDescriptor = aDescriptor.getAsConstPropertyValueList();
            sTypeName = xDetection->queryTypeByDescriptor(lDescriptor, true); // lDescriptor is used as In/Out param ... don't use aDescriptor.getAsConstPropertyValueList() directly!

            for (sal_Int32 i = 0; i < lDescriptor.getLength(); ++i)
            {
                if (lDescriptor[i].Name == "FilterName")
                    // Type detection picked a preferred filter for this format.
                    aFilterName = lDescriptor[i].Value.get<OUString>();
            }
        }
        // no stream exists => try flat detection without preselection as fallback
        else
            sTypeName = xDetection->queryTypeByURL(sURL);

        if (!sTypeName.isEmpty())
        {
            std::shared_ptr<const SfxFilter> pNewFilter;
            if (!aFilterName.isEmpty())
                // Type detection returned a suitable filter for this.  Use it.
                pNewFilter = SfxFilter::GetFilterByName(aFilterName);

            // fdo#78742 respect requested document service if set
            if (!pNewFilter || (!m_rImpl.aName.isEmpty()
                             && m_rImpl.aName != pNewFilter->GetServiceName()))
            {
                // detect filter by given type
                // In case of this matcher is bound to a particular document type:
                // If there is no acceptable type for this document at all, the type detection has possibly returned something else.
                // The DocumentService property is only a preselection, and all preselections are considered as optional!
                // This "wrong" type will be sorted out now because we match only allowed filters to the detected type
                uno::Sequence< beans::NamedValue > lQuery { { "Name", css::uno::makeAny(sTypeName) } };

                pNewFilter = GetFilterForProps(lQuery, nMust, nDont);
            }

            if (pNewFilter)
            {
                rpFilter = pNewFilter;
                return ERRCODE_NONE;
            }
        }
    }
    catch (const uno::Exception&)
    {}

    return ERRCODE_ABORT;
}


bool SfxFilterMatcher::IsFilterInstalled_Impl( const std::shared_ptr<const SfxFilter>& pFilter )
{
    if ( pFilter->GetFilterFlags() & SfxFilterFlags::MUSTINSTALL )
    {
        // Here could a  re-installation be offered
        OUString aText( SfxResId(STR_FILTER_NOT_INSTALLED).toString() );
        aText = aText.replaceFirst( "$(FILTER)", pFilter->GetUIName() );
        ScopedVclPtrInstance< QueryBox > aQuery(nullptr, WB_YES_NO | WB_DEF_YES, aText);
        short nRet = aQuery->Execute();
        if ( nRet == RET_YES )
        {
#ifdef DBG_UTIL
            // Start Setup
            ScopedVclPtrInstance<InfoBox>( nullptr, "Here should the Setup now be starting!" )->Execute();
#endif
            // Installation must still give feedback if it worked or not,
            // then the  Filterflag be deleted
        }

        return ( !(pFilter->GetFilterFlags() & SfxFilterFlags::MUSTINSTALL) );
    }
    else if ( pFilter->GetFilterFlags() & SfxFilterFlags::CONSULTSERVICE )
    {
        OUString aText( SfxResId(STR_FILTER_CONSULT_SERVICE).toString() );
        aText = aText.replaceFirst( "$(FILTER)", pFilter->GetUIName() );
        ScopedVclPtrInstance<InfoBox>( nullptr, aText )->Execute();
        return false;
    }
    else
        return true;
}


sal_uInt32 SfxFilterMatcher::DetectFilter( SfxMedium& rMedium, std::shared_ptr<const SfxFilter>& rpFilter ) const
/*  [Description]

    Here the Filter selection box is pulled up. Otherwise GuessFilter
 */

{
    std::shared_ptr<const SfxFilter> pOldFilter = rMedium.GetFilter();
    if ( pOldFilter )
    {
        if( !IsFilterInstalled_Impl( pOldFilter ) )
            pOldFilter = nullptr;
        else
        {
            const SfxStringItem* pSalvageItem = SfxItemSet::GetItem<SfxStringItem>(rMedium.GetItemSet(), SID_DOC_SALVAGE, false);
            if ( ( pOldFilter->GetFilterFlags() & SfxFilterFlags::PACKED ) && pSalvageItem )
                // Salvage is always done without packing
                pOldFilter = nullptr;
        }
    }

    std::shared_ptr<const SfxFilter> pFilter = pOldFilter;

    bool bPreview = rMedium.IsPreview_Impl();
    const SfxStringItem* pReferer = SfxItemSet::GetItem<SfxStringItem>(rMedium.GetItemSet(), SID_REFERER, false);
    if ( bPreview && rMedium.IsRemote() && ( !pReferer || !pReferer->GetValue().match("private:searchfolder:") ) )
        return ERRCODE_ABORT;

    ErrCode nErr = GuessFilter( rMedium, pFilter );
    if ( nErr == ERRCODE_ABORT )
        return nErr;

    if ( nErr == ERRCODE_IO_PENDING )
    {
        rpFilter = pFilter;
        return nErr;
    }

    if ( !pFilter )
    {
        std::shared_ptr<const SfxFilter> pInstallFilter;

        // Now test the filter which are not installed (ErrCode is irrelevant)
        GuessFilter( rMedium, pInstallFilter, SfxFilterFlags::IMPORT, SfxFilterFlags::CONSULTSERVICE );
        if ( pInstallFilter )
        {
            if ( IsFilterInstalled_Impl( pInstallFilter ) )
                // Maybe the filter was installed afterwards.
                pFilter = pInstallFilter;
        }
        else
        {
          // Now test the filter, which first must be obtained by Star
          // (ErrCode is irrelevant)
            GuessFilter( rMedium, pInstallFilter, SfxFilterFlags::IMPORT, SfxFilterFlags::NONE );
            if ( pInstallFilter )
                IsFilterInstalled_Impl( pInstallFilter );
        }
    }

    bool bHidden = bPreview;
    const SfxStringItem* pFlags = SfxItemSet::GetItem<SfxStringItem>(rMedium.GetItemSet(), SID_OPTIONS, false);
    if ( !bHidden && pFlags )
    {
        OUString aFlags( pFlags->GetValue() );
        aFlags = aFlags.toAsciiUpperCase();
        if( -1 != aFlags.indexOf( 'H' ) )
            bHidden = true;
    }
    rpFilter = pFilter;

    if ( bHidden )
        nErr = pFilter ? ERRCODE_NONE : ERRCODE_ABORT;
    return nErr;
}

std::shared_ptr<const SfxFilter> SfxFilterMatcher::GetFilterForProps( const css::uno::Sequence < beans::NamedValue >& aSeq, SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    uno::Reference< lang::XMultiServiceFactory > xServiceManager = ::comphelper::getProcessServiceFactory();
    uno::Reference< container::XContainerQuery > xTypeCFG;
    if( xServiceManager.is() )
        xTypeCFG.set( xServiceManager->createInstance( "com.sun.star.document.TypeDetection" ), uno::UNO_QUERY );
    if ( xTypeCFG.is() )
    {
        // make query for all types matching the properties
        uno::Reference < css::container::XEnumeration > xEnum = xTypeCFG->createSubSetEnumerationByProperties( aSeq );
        while ( xEnum->hasMoreElements() )
        {
            ::comphelper::SequenceAsHashMap aProps( xEnum->nextElement() );
            OUString aValue;

            // try to get the preferred filter (works without loading all filters!)
            if ( (aProps[OUString("PreferredFilter")] >>= aValue) && !aValue.isEmpty() )
            {
                std::shared_ptr<const SfxFilter> pFilter = SfxFilter::GetFilterByName( aValue );
                if ( !pFilter || (pFilter->GetFilterFlags() & nMust) != nMust || (pFilter->GetFilterFlags() & nDont ) )
                    // check for filter flags
                    // pFilter == 0: if preferred filter is a Writer filter, but Writer module is not installed
                    continue;

                if ( !m_rImpl.aName.isEmpty() )
                {
                    // if this is not the global FilterMatcher: check if filter matches the document type
                    if ( pFilter->GetServiceName() != m_rImpl.aName )
                    {
                        // preferred filter belongs to another document type; now we must search the filter
                        m_rImpl.InitForIterating();
                        aProps[OUString("Name")] >>= aValue;
                        pFilter = GetFilter4EA( aValue, nMust, nDont );
                        if ( pFilter )
                            return pFilter;
                    }
                    else
                        return pFilter;
                }
                else
                    return pFilter;
            }
        }
    }

    return nullptr;
}

std::shared_ptr<const SfxFilter> SfxFilterMatcher::GetFilter4Mime( const OUString& rMediaType, SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    if ( m_rImpl.pList )
    {
        for (std::shared_ptr<const SfxFilter>& pFilter : *m_rImpl.pList)
        {
            SfxFilterFlags nFlags = pFilter->GetFilterFlags();
            if ( (nFlags & nMust) == nMust && !(nFlags & nDont ) && pFilter->GetMimeType() == rMediaType )
                return pFilter;
        }

        return nullptr;
    }

    css::uno::Sequence < css::beans::NamedValue > aSeq { { "MediaType", css::uno::makeAny(rMediaType) } };
    return GetFilterForProps( aSeq, nMust, nDont );
}

std::shared_ptr<const SfxFilter> SfxFilterMatcher::GetFilter4EA( const OUString& rType, SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    if ( m_rImpl.pList )
    {
        std::shared_ptr<const SfxFilter> pFirst;
        for (std::shared_ptr<const SfxFilter>& pFilter : *m_rImpl.pList)
        {
            SfxFilterFlags nFlags = pFilter->GetFilterFlags();
            if ( (nFlags & nMust) == nMust && !(nFlags & nDont ) && pFilter->GetTypeName() == rType )
            {
                if (nFlags & SfxFilterFlags::PREFERED)
                    return pFilter;
                if (!pFirst)
                    pFirst = pFilter;
            }
        }
        if (pFirst)
            return pFirst;

        return nullptr;
    }

    css::uno::Sequence < css::beans::NamedValue > aSeq { { "Name", css::uno::makeAny(rType) } };
    return GetFilterForProps( aSeq, nMust, nDont );
}

std::shared_ptr<const SfxFilter> SfxFilterMatcher::GetFilter4Extension( const OUString& rExt, SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    if ( m_rImpl.pList )
    {
        for (std::shared_ptr<const SfxFilter>& pFilter : *m_rImpl.pList)
        {
            SfxFilterFlags nFlags = pFilter->GetFilterFlags();
            if ( (nFlags & nMust) == nMust && !(nFlags & nDont ) )
            {
                OUString sWildCard = ToUpper_Impl( pFilter->GetWildcard().getGlob() );
                OUString sExt      = ToUpper_Impl( rExt );

                if (sExt.isEmpty())
                    continue;

                if (sExt[0] != (sal_Unicode)'.')
                    sExt = "." + sExt;

                WildCard aCheck(sWildCard, ';');
                if (aCheck.Matches(sExt))
                    return pFilter;
            }
        }

        return nullptr;
    }

    // Use extension without dot!
    OUString sExt( rExt );
    if ( sExt.startsWith(".") )
        sExt = sExt.copy(1);

    css::uno::Sequence < css::beans::NamedValue > aSeq
        { { "Extensions", css::uno::makeAny(uno::Sequence < OUString > { sExt } ) } };
    return GetFilterForProps( aSeq, nMust, nDont );
}

std::shared_ptr<const SfxFilter> SfxFilterMatcher::GetFilter4ClipBoardId( SotClipboardFormatId nId, SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    if (nId == SotClipboardFormatId::NONE)
        return nullptr;

    css::uno::Sequence < css::beans::NamedValue > aSeq
        { { "ClipboardFormat", css::uno::makeAny(SotExchange::GetFormatName( nId )) } };
    return GetFilterForProps( aSeq, nMust, nDont );
}

std::shared_ptr<const SfxFilter> SfxFilterMatcher::GetFilter4UIName( const OUString& rName, SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    m_rImpl.InitForIterating();
    std::shared_ptr<const SfxFilter> pFirstFilter;
    for (std::shared_ptr<const SfxFilter>& pFilter : *m_rImpl.pList)
    {
        SfxFilterFlags nFlags = pFilter->GetFilterFlags();
        if ( (nFlags & nMust) == nMust &&
             !(nFlags & nDont ) && pFilter->GetUIName() == rName )
        {
            if ( pFilter->GetFilterFlags() & SfxFilterFlags::PREFERED )
                return pFilter;
            else if ( !pFirstFilter )
                pFirstFilter = pFilter;
        }
    }
    return pFirstFilter;
}

std::shared_ptr<const SfxFilter> SfxFilterMatcher::GetFilter4FilterName( const OUString& rName, SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    OUString aName( rName );
    sal_Int32 nIndex = aName.indexOf(": ");
    if (  nIndex != -1 )
    {
        SAL_WARN( "sfx.bastyp", "Old filter name used!");
        aName = rName.copy( nIndex + 2 );
    }

    if ( bFirstRead )
    {
        uno::Reference< lang::XMultiServiceFactory > xServiceManager = ::comphelper::getProcessServiceFactory();
        uno::Reference< container::XNameAccess >     xFilterCFG                                                ;
        uno::Reference< container::XNameAccess >     xTypeCFG                                                  ;
        if( xServiceManager.is() )
        {
            xFilterCFG.set( xServiceManager->createInstance(  "com.sun.star.document.FilterFactory" ), uno::UNO_QUERY );
            xTypeCFG.set( xServiceManager->createInstance(  "com.sun.star.document.TypeDetection" ), uno::UNO_QUERY );
        }

        if( xFilterCFG.is() && xTypeCFG.is() )
        {
            if ( !pFilterArr )
                CreateFilterArr();
            else
            {
                for (std::shared_ptr<const SfxFilter>& pFilter : *pFilterArr)
                {
                    SfxFilterFlags nFlags = pFilter->GetFilterFlags();
                    if ((nFlags & nMust) == nMust && !(nFlags & nDont) && pFilter->GetFilterName().equalsIgnoreAsciiCase(aName))
                        return pFilter;
                }
            }

            SfxFilterContainer::ReadSingleFilter_Impl( rName, xTypeCFG, xFilterCFG, false );
        }
    }

    SfxFilterList_Impl* pList = m_rImpl.pList;
    if ( !pList )
        pList = pFilterArr;

    for (std::shared_ptr<const SfxFilter>& pFilter : *pList)
    {
        SfxFilterFlags nFlags = pFilter->GetFilterFlags();
        if ( (nFlags & nMust) == nMust && !(nFlags & nDont ) && pFilter->GetFilterName().equalsIgnoreAsciiCase(aName))
            return pFilter;
    }

    return nullptr;
}

IMPL_LINK_TYPED( SfxFilterMatcher, MaybeFileHdl_Impl, OUString*, pString, bool )
{
    std::shared_ptr<const SfxFilter> pFilter = GetFilter4Extension( *pString );
    if (pFilter && !pFilter->GetWildcard().Matches( OUString() ) &&
        !pFilter->GetWildcard().Matches("*.*") &&
        !pFilter->GetWildcard().Matches(OUString('*'))
       )
    {
        return true;
    }
    return false;
}


SfxFilterMatcherIter::SfxFilterMatcherIter(
    const SfxFilterMatcher& rMatcher,
    SfxFilterFlags nOrMaskP, SfxFilterFlags nAndMaskP )
    : nOrMask( nOrMaskP ), nAndMask( nAndMaskP ),
      nCurrent(0), m_rMatch(rMatcher.m_rImpl)
{
    if( nOrMask == static_cast<SfxFilterFlags>(0xffff) ) //Due to faulty build on s
        nOrMask = SfxFilterFlags::NONE;
    m_rMatch.InitForIterating();
}


std::shared_ptr<const SfxFilter> SfxFilterMatcherIter::Find_Impl()
{
    std::shared_ptr<const SfxFilter> pFilter;
    while( nCurrent < m_rMatch.pList->size() )
    {
        pFilter = (*m_rMatch.pList)[nCurrent++];
        SfxFilterFlags nFlags = pFilter->GetFilterFlags();
        if( ((nFlags & nOrMask) == nOrMask ) && !(nFlags & nAndMask ) )
            break;
        pFilter = nullptr;
    }

    return pFilter;
}

std::shared_ptr<const SfxFilter> SfxFilterMatcherIter::First()
{
    nCurrent = 0;
    return Find_Impl();
}


std::shared_ptr<const SfxFilter> SfxFilterMatcherIter::Next()
{
    return Find_Impl();
}

/*---------------------------------------------------------------
    helper to build own formatted string from given stringlist by
    using given separator
  ---------------------------------------------------------------*/
OUString implc_convertStringlistToString( const uno::Sequence< OUString >& lList     ,
                                                 const sal_Unicode&                                        cSeparator,
                                                 const OUString&                                    sPrefix   )
{
    OUStringBuffer   sString ( 1000 )           ;
    sal_Int32               nCount  = lList.getLength();
    sal_Int32               nItem   = 0                ;
    for( nItem=0; nItem<nCount; ++nItem )
    {
        if( !sPrefix.isEmpty() )
        {
            sString.append( sPrefix );
        }
        sString.append( lList[nItem] );
        if( nItem+1<nCount )
        {
            sString.append( cSeparator );
        }
    }
    return sString.makeStringAndClear();
}


void SfxFilterContainer::ReadSingleFilter_Impl(
    const OUString& rName,
    const uno::Reference< container::XNameAccess >& xTypeCFG,
    const uno::Reference< container::XNameAccess >& xFilterCFG,
    bool bUpdate
    )
{
    OUString sFilterName( rName );
    SfxFilterList_Impl& rList = *pFilterArr;
    uno::Sequence< beans::PropertyValue > lFilterProperties;
    uno::Any aResult;
    try
    {
        aResult = xFilterCFG->getByName( sFilterName );
    }
    catch( container::NoSuchElementException& )
    {
        aResult = uno::Any();
    }

    if( aResult >>= lFilterProperties )
    {
        // collect information to add filter to container
        // (attention: some information aren't available on filter directly ... you must search for corresponding type too!)
        SfxFilterFlags       nFlags          = SfxFilterFlags::NONE;
        SotClipboardFormatId nClipboardId    = SotClipboardFormatId::NONE;
        sal_Int32       nDocumentIconId = 0 ;
        sal_Int32       nFormatVersion  = 0 ;
        OUString sMimeType           ;
        OUString sType               ;
        OUString sUIName             ;
        OUString sHumanName          ;
        OUString sDefaultTemplate    ;
        OUString sUserData           ;
        OUString sExtension          ;
        OUString sPattern            ;
        OUString sServiceName        ;

        // first get directly available properties
        sal_Int32 nFilterPropertyCount = lFilterProperties.getLength();
        sal_Int32 nFilterProperty      = 0                            ;
        for( nFilterProperty=0; nFilterProperty<nFilterPropertyCount; ++nFilterProperty )
        {
            if ( lFilterProperties[nFilterProperty].Name == "FileFormatVersion" )
            {
                lFilterProperties[nFilterProperty].Value >>= nFormatVersion;
            }
            else if ( lFilterProperties[nFilterProperty].Name == "TemplateName" )
            {
                lFilterProperties[nFilterProperty].Value >>= sDefaultTemplate;
            }
            else if ( lFilterProperties[nFilterProperty].Name == "Flags" )
            {
                sal_Int32 nTmp(0);
                lFilterProperties[nFilterProperty].Value >>= nTmp;
                assert((nTmp & ~o3tl::typed_flags<SfxFilterFlags>::mask) == 0);
                nFlags = static_cast<SfxFilterFlags>(nTmp);
            }
            else if ( lFilterProperties[nFilterProperty].Name == "UIName" )
            {
                lFilterProperties[nFilterProperty].Value >>= sUIName;
            }
            else if ( lFilterProperties[nFilterProperty].Name == "UserData" )
            {
                uno::Sequence< OUString > lUserData;
                lFilterProperties[nFilterProperty].Value >>= lUserData;
                sUserData = implc_convertStringlistToString( lUserData, ',', OUString() );
            }
            else if ( lFilterProperties[nFilterProperty].Name == "DocumentService" )
            {
                lFilterProperties[nFilterProperty].Value >>= sServiceName;
            }
            else if (lFilterProperties[nFilterProperty].Name == "ExportExtension")
            {
                // Extension preferred by the filter.  This takes precedence
                // over those that are given in the file format type.
                lFilterProperties[nFilterProperty].Value >>= sExtension;
                sExtension = "*." + sExtension;
            }
            else if ( lFilterProperties[nFilterProperty].Name == "Type" )
            {
                lFilterProperties[nFilterProperty].Value >>= sType;
                // Try to get filter .. but look for any exceptions!
                // May be filter was deleted by another thread ...
                try
                {
                    aResult = xTypeCFG->getByName( sType );
                }
                catch (const container::NoSuchElementException&)
                {
                    aResult = uno::Any();
                }

                uno::Sequence< beans::PropertyValue > lTypeProperties;
                if( aResult >>= lTypeProperties )
                {
                    // get indirect available properties then (types)
                    sal_Int32 nTypePropertyCount = lTypeProperties.getLength();
                    sal_Int32 nTypeProperty      = 0                          ;
                    for( nTypeProperty=0; nTypeProperty<nTypePropertyCount; ++nTypeProperty )
                    {
                        if ( lTypeProperties[nTypeProperty].Name == "ClipboardFormat" )
                        {
                            lTypeProperties[nTypeProperty].Value >>= sHumanName;
                        }
                        else if ( lTypeProperties[nTypeProperty].Name == "DocumentIconID" )
                        {
                            lTypeProperties[nTypeProperty].Value >>= nDocumentIconId;
                        }
                        else if ( lTypeProperties[nTypeProperty].Name == "MediaType" )
                        {
                            lTypeProperties[nTypeProperty].Value >>= sMimeType;
                        }
                        else if ( lTypeProperties[nTypeProperty].Name == "Extensions" )
                        {
                            if (sExtension.isEmpty())
                            {
                                uno::Sequence< OUString > lExtensions;
                                lTypeProperties[nTypeProperty].Value >>= lExtensions;
                                sExtension = implc_convertStringlistToString( lExtensions, ';', "*." );
                            }
                        }
                        else if ( lTypeProperties[nTypeProperty].Name == "URLPattern" )
                        {
                                uno::Sequence< OUString > lPattern;
                                lTypeProperties[nTypeProperty].Value >>= lPattern;
                                sPattern = implc_convertStringlistToString( lPattern, ';', OUString() );
                        }
                    }
                }
            }
        }

        if ( sServiceName.isEmpty() )
            return;

        // old formats are found ... using HumanPresentableName!
        if( !sHumanName.isEmpty() )
        {
            nClipboardId = SotExchange::RegisterFormatName( sHumanName );

            // For external filters ignore clipboard IDs
            if(nFlags & SfxFilterFlags::STARONEFILTER)
            {
                nClipboardId = SotClipboardFormatId::NONE;
            }
        }
        // register SfxFilter
        // first erase module name from old filter names!
        // e.g: "scalc: DIF" => "DIF"
        sal_Int32 nStartRealName = sFilterName.indexOf( ": " );
        if( nStartRealName != -1 )
        {
            SAL_WARN( "sfx.bastyp", "Old format, not supported!");
            sFilterName = sFilterName.copy( nStartRealName+2 );
        }

        std::shared_ptr<const SfxFilter> pFilter = bUpdate ? SfxFilter::GetFilterByName( sFilterName ) : nullptr;
        if (!pFilter)
        {
            pFilter.reset(new SfxFilter( sFilterName             ,
                                     sExtension              ,
                                     nFlags                  ,
                                     nClipboardId            ,
                                     sType                   ,
                                     (sal_uInt16)nDocumentIconId ,
                                     sMimeType               ,
                                     sUserData               ,
                                     sServiceName ));
            rList.push_back( pFilter );
        }
        else
        {
            SfxFilter* pFilt = const_cast<SfxFilter*>(pFilter.get());
            pFilt->maFilterName  = sFilterName;
            pFilt->aWildCard    = WildCard(sExtension, ';');
            pFilt->nFormatType  = nFlags;
            pFilt->lFormat      = nClipboardId;
            pFilt->aTypeName    = sType;
            pFilt->nDocIcon     = (sal_uInt16)nDocumentIconId;
            pFilt->aMimeType    = sMimeType;
            pFilt->aUserData    = sUserData;
            pFilt->aServiceName = sServiceName;
        }

        SfxFilter* pFilt = const_cast<SfxFilter*>(pFilter.get());

        // Don't forget to set right UIName!
        // Otherwise internal name is used as fallback ...
        pFilt->SetUIName( sUIName );
        pFilt->SetDefaultTemplate( sDefaultTemplate );
        if( nFormatVersion )
        {
            pFilt->SetVersion( nFormatVersion );
        }
        pFilt->SetURLPattern(sPattern);
    }
}

void SfxFilterContainer::ReadFilters_Impl( bool bUpdate )
{
    if ( !pFilterArr )
        CreateFilterArr();

    bFirstRead = false;
    SfxFilterList_Impl& rList = *pFilterArr;

    try
    {
        // get the FilterFactory service to access the registered filters ... and types!
        uno::Reference< lang::XMultiServiceFactory > xServiceManager = ::comphelper::getProcessServiceFactory();
        uno::Reference< container::XNameAccess >     xFilterCFG                                                ;
        uno::Reference< container::XNameAccess >     xTypeCFG                                                  ;
        if( xServiceManager.is() )
        {
            xFilterCFG.set( xServiceManager->createInstance(  "com.sun.star.document.FilterFactory" ), uno::UNO_QUERY );
            xTypeCFG.set( xServiceManager->createInstance(  "com.sun.star.document.TypeDetection" ), uno::UNO_QUERY );
        }

        if( xFilterCFG.is() && xTypeCFG.is() )
        {
            // select right query to get right set of filters for search modul
            uno::Sequence< OUString > lFilterNames = xFilterCFG->getElementNames();
            if ( lFilterNames.getLength() )
            {
                // If list of filters already exist ...
                // ReadExternalFilters must work in update mode.
                // Best way seems to mark all filters NOT_INSTALLED
                // and change it back for all valid filters afterwards.
                if( !rList.empty() )
                {
                    bUpdate = true;
                    for (std::shared_ptr<const SfxFilter>& pFilter : rList)
                    {
                        SfxFilter* pNonConstFilter = const_cast<SfxFilter*>(pFilter.get());
                        pNonConstFilter->nFormatType |= SFX_FILTER_NOTINSTALLED;
                    }
                }

                // get all properties of filters ... put it into the filter container
                sal_Int32 nFilterCount = lFilterNames.getLength();
                sal_Int32 nFilter=0;
                for( nFilter=0; nFilter<nFilterCount; ++nFilter )
                {
                    // Try to get filter .. but look for any exceptions!
                    // May be filter was deleted by another thread ...
                    OUString sFilterName = lFilterNames[nFilter];
                    ReadSingleFilter_Impl( sFilterName, xTypeCFG, xFilterCFG, bUpdate );
                }
            }
        }
    }
    catch(const uno::Exception&)
    {
        SAL_WARN( "sfx.bastyp", "SfxFilterContainer::ReadFilter()\nException detected. Possible not all filters could be cached.\n" );
    }

    if ( bUpdate )
    {
        // global filter arry was modified, factory specific ones might need an
        // update too
        for (auto& aImpl : aImplArr)
            aImpl->Update();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
