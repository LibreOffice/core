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


#include "fltfnc.hxx"
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
#include <comphelper/mediadescriptor.hxx>
#include <tools/urlobj.hxx>

#include <rtl/logfile.hxx>
#include <rtl/instance.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::beans;
#include <svl/ctypeitm.hxx>
#include <svtools/sfxecode.hxx>
#include <unotools/syslocale.hxx>

#include "sfx2/sfxhelp.hxx"
#include <sfx2/docfilt.hxx>
#include <sfx2/docfac.hxx>
#include "sfxtypes.hxx"
#include <sfx2/sfxuno.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/progress.hxx>
#include "openflag.hxx"
#include "bastyp.hrc"
#include "sfx2/sfxresid.hxx"
#include <sfx2/doctempl.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include "helper.hxx"
#include "fltlst.hxx"
#include <sfx2/request.hxx>
#include "arrdecl.hxx"
#include <sfx2/appuno.hxx>

#include <boost/ptr_container/ptr_vector.hpp>
#include <functional>

namespace
{
    class theSfxFilterListener : public rtl::Static<SfxFilterListener, theSfxFilterListener> {};
}

static SfxFilterList_Impl* pFilterArr = 0;
static sal_Bool bFirstRead = sal_True;

static void CreateFilterArr()
{
    pFilterArr = new SfxFilterList_Impl;
    theSfxFilterListener::get();
}

//----------------------------------------------------------------
inline String ToUpper_Impl( const String &rStr )
{
    return SvtSysLocale().GetCharClass().uppercase( rStr );
}

//----------------------------------------------------------------
class SfxFilterContainer_Impl
{
public:
    String              aName;
    String              aServiceName;

                        SfxFilterContainer_Impl( const String& rName )
                            : aName( rName )
                        {
                            aServiceName = SfxObjectShell::GetServiceNameFromFactory( rName );
                        }
};

#define IMPL_FORWARD_LOOP( aMethod, ArgType, aArg )         \
const SfxFilter* SfxFilterContainer::aMethod( ArgType aArg, SfxFilterFlags nMust, SfxFilterFlags nDont ) const \
{\
    SfxFilterMatcher aMatch( pImpl->aName ); \
    return aMatch.aMethod( aArg, nMust, nDont ); \
}

IMPL_FORWARD_LOOP( GetFilter4EA, const String&, rEA );
IMPL_FORWARD_LOOP( GetFilter4Extension, const String&, rExt );
IMPL_FORWARD_LOOP( GetFilter4FilterName, const String&, rName );

const SfxFilter* SfxFilterContainer::GetAnyFilter( SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    SfxFilterMatcher aMatch( pImpl->aName );
    return aMatch.GetAnyFilter( nMust, nDont );
}

//----------------------------------------------------------------

SfxFilterContainer::SfxFilterContainer( const String& rName )
{
    pImpl = new SfxFilterContainer_Impl( rName );
}

//----------------------------------------------------------------

SfxFilterContainer::~SfxFilterContainer()
{
    delete pImpl;
}

//----------------------------------------------------------------

const String SfxFilterContainer::GetName() const
{
    return pImpl->aName;
}

const SfxFilter* SfxFilterContainer::GetDefaultFilter_Impl( const String& rName )
{
    // Try to find out the type of factory.
    // Interpret given name as Service- and ShortName!
    SvtModuleOptions aOpt;
    SvtModuleOptions::EFactory eFactory = aOpt.ClassifyFactoryByServiceName(rName);
    if (eFactory == SvtModuleOptions::E_UNKNOWN_FACTORY)
        eFactory = aOpt.ClassifyFactoryByShortName(rName);

    // could not classify factory by its service nor by its short name.
    // Must be an unknown factory! => return NULL
    if (eFactory == SvtModuleOptions::E_UNKNOWN_FACTORY)
        return NULL;

    // For the following code we need some additional informations.
    String sServiceName   = aOpt.GetFactoryName(eFactory);
    String sShortName     = aOpt.GetFactoryShortName(eFactory);
    String sDefaultFilter = aOpt.GetFactoryDefaultFilter(eFactory);

    // Try to get the default filter. Dont fiorget to verify it.
    // May the set default filter does not exists any longer or
    // does not fit the given factory.
    const SfxFilterMatcher aMatcher;
    const SfxFilter* pFilter = aMatcher.GetFilter4FilterName(sDefaultFilter);

    if (
        pFilter &&
        !pFilter->GetServiceName().equalsIgnoreAsciiCase(sServiceName)
       )
    {
        pFilter = 0;
    }

    // If at least no default filter could be located - use any filter of this
    // factory.
    if (!pFilter)
    {
        if ( bFirstRead )
            ReadFilters_Impl();

        for ( size_t i = 0, n = pFilterArr->size(); i < n; ++i )
        {
            const SfxFilter* pCheckFilter = pFilterArr->at( i );
            if ( pCheckFilter->GetServiceName().equalsIgnoreAsciiCase(sServiceName) )
            {
                pFilter = pCheckFilter;
                break;
            }
        }
    }

    return pFilter;
}


//----------------------------------------------------------------

// Impl-Data is shared between all FilterMatchers of the same factory
class SfxFilterMatcher_Impl
{
public:
    ::rtl::OUString     aName;
    SfxFilterList_Impl* pList;      // is created on demand

    void InitForIterating() const;
    void Update();
    SfxFilterMatcher_Impl(const ::rtl::OUString &rName)
        : aName(rName)
        , pList(0)
    {
    }
};

namespace
{
    typedef boost::ptr_vector<SfxFilterMatcher_Impl> SfxFilterMatcherArr_Impl;
    static SfxFilterMatcherArr_Impl aImplArr;
    static int nSfxFilterMatcherCount;

    class hasName :
        public std::unary_function<SfxFilterMatcher_Impl, bool>
    {
    private:
        const rtl::OUString& mrName;
    public:
        hasName(const rtl::OUString &rName) : mrName(rName) {}
        bool operator() (const SfxFilterMatcher_Impl& rImpl) const
        {
            return rImpl.aName == mrName;
        }
    };

    SfxFilterMatcher_Impl & getSfxFilterMatcher_Impl(const rtl::OUString &rName)
    {
        rtl::OUString aName;

        if (!rName.isEmpty())
            aName = SfxObjectShell::GetServiceNameFromFactory(rName);

        // find the impl-Data of any comparable FilterMatcher that was created
        // previously
        SfxFilterMatcherArr_Impl::iterator aEnd = aImplArr.end();
        SfxFilterMatcherArr_Impl::iterator aIter =
            std::find_if(aImplArr.begin(), aEnd, hasName(rName));
        if (aIter != aEnd)
            return *aIter;

        // first Matcher created for this factory
        SfxFilterMatcher_Impl *pImpl = new SfxFilterMatcher_Impl(aName);
        aImplArr.push_back(pImpl);
        return *pImpl;
    }
}

SfxFilterMatcher::SfxFilterMatcher( const String& rName )
    : m_rImpl( getSfxFilterMatcher_Impl(rName) )
{
    ++nSfxFilterMatcherCount;
}

SfxFilterMatcher::SfxFilterMatcher()
    : m_rImpl( getSfxFilterMatcher_Impl(::rtl::OUString()) )
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

void SfxFilterMatcher_Impl::Update()
{
    if ( pList )
    {
        // this List was already used
        pList->clear();
        for ( size_t i = 0, n = pFilterArr->size(); i < n; ++i )
        {
            SfxFilter* pFilter = pFilterArr->at( i );
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
        ((SfxFilterMatcher_Impl*)this)->pList = new SfxFilterList_Impl;
        ((SfxFilterMatcher_Impl*)this)->Update();
    }
    else
    {
        // global matcher: use global filter array
        ((SfxFilterMatcher_Impl*)this)->pList = pFilterArr;
    }
}

const SfxFilter* SfxFilterMatcher::GetAnyFilter( SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    m_rImpl.InitForIterating();
    for ( size_t i = 0, n = m_rImpl.pList->size(); i < n; ++i )
    {
        const SfxFilter* pFilter = m_rImpl.pList->at( i );
        SfxFilterFlags nFlags = pFilter->GetFilterFlags();
        if ( (nFlags & nMust) == nMust && !(nFlags & nDont ) )
            return pFilter;
    }

    return NULL;
}

//----------------------------------------------------------------

sal_uInt32  SfxFilterMatcher::GuessFilterIgnoringContent(
    SfxMedium& rMedium,
    const SfxFilter**ppFilter,
    SfxFilterFlags nMust,
    SfxFilterFlags nDont ) const
{
    Reference< XTypeDetection > xDetection( ::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString("com.sun.star.document.TypeDetection")), UNO_QUERY );
    ::rtl::OUString sTypeName;
    try
    {
        sTypeName = xDetection->queryTypeByURL( rMedium.GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) );
    }
    catch( Exception& )
    {
    }

    *ppFilter = NULL;
    if ( !sTypeName.isEmpty() )
    {
        // make sure filter list is initialized
        m_rImpl.InitForIterating();
        *ppFilter = GetFilter4EA( sTypeName, nMust, nDont );
    }

    return *ppFilter ? ERRCODE_NONE : ERRCODE_ABORT;
}

//----------------------------------------------------------------

sal_uInt32  SfxFilterMatcher::GuessFilter( SfxMedium& rMedium, const SfxFilter**ppFilter, SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    return GuessFilterControlDefaultUI( rMedium, ppFilter, nMust, nDont, sal_True );
}

//----------------------------------------------------------------

sal_uInt32  SfxFilterMatcher::GuessFilterControlDefaultUI( SfxMedium& rMedium, const SfxFilter** ppFilter, SfxFilterFlags nMust, SfxFilterFlags nDont, sal_Bool /*bDefUI*/ ) const
{
    const SfxFilter* pOldFilter = *ppFilter;

    // no detection service -> nothing to do !
    Reference< XTypeDetection > xDetection( ::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString("com.sun.star.document.TypeDetection")), UNO_QUERY );
    if (!xDetection.is())
        return ERRCODE_ABORT;

    ::rtl::OUString sTypeName;
    try
    {
        // open the stream one times only ...
        // Otherwhise it will be tried more then once and show the same interaction more then once ...

        ::rtl::OUString sURL( rMedium.GetURLObject().GetMainURL( INetURLObject::NO_DECODE ) );
        ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > xInStream = rMedium.GetInputStream();
        rtl::OUString aFilterName;

        // stream exists => deep detection (with preselection ... if possible)
        if (xInStream.is())
        {
            ::comphelper::MediaDescriptor aDescriptor;

            aDescriptor[::comphelper::MediaDescriptor::PROP_URL()               ] <<= sURL;
            aDescriptor[::comphelper::MediaDescriptor::PROP_INPUTSTREAM()       ] <<= xInStream;
            aDescriptor[::comphelper::MediaDescriptor::PROP_INTERACTIONHANDLER()] <<= rMedium.GetInteractionHandler();

            if ( !m_rImpl.aName.isEmpty() )
                aDescriptor[::comphelper::MediaDescriptor::PROP_DOCUMENTSERVICE()] <<= m_rImpl.aName;

            if ( pOldFilter )
            {
                aDescriptor[::comphelper::MediaDescriptor::PROP_TYPENAME()  ] <<= ::rtl::OUString( pOldFilter->GetTypeName()   );
                aDescriptor[::comphelper::MediaDescriptor::PROP_FILTERNAME()] <<= ::rtl::OUString( pOldFilter->GetFilterName() );
            }

            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > lDescriptor = aDescriptor.getAsConstPropertyValueList();
            sTypeName = xDetection->queryTypeByDescriptor(lDescriptor, sal_True); // lDescriptor is used as In/Out param ... dont use aDescriptor.getAsConstPropertyValueList() directly!

            for (sal_Int32 i = 0; i < lDescriptor.getLength(); ++i)
            {
                if (lDescriptor[i].Name == "FilterName")
                    // Type detection picked a preferred filter for this format.
                    aFilterName = lDescriptor[i].Value.get<rtl::OUString>();
            }
        }
        // no stream exists => try flat detection without preselection as fallback
        else
            sTypeName = xDetection->queryTypeByURL(sURL);

        if (!sTypeName.isEmpty())
        {
            const SfxFilter* pFilter = NULL;
            if (!aFilterName.isEmpty())
                // Type detection returned a suitable filter for this.  Use it.
                pFilter = SfxFilter::GetFilterByName(aFilterName);

            if (!pFilter)
            {
                // detect filter by given type
                // In case of this matcher is bound to a particular document type:
                // If there is no acceptable type for this document at all, the type detection has possibly returned something else.
                // The DocumentService property is only a preselection, and all preselections are considered as optional!
                // This "wrong" type will be sorted out now because we match only allowed filters to the detected type
                ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > lQuery(1);
                lQuery[0].Name = ::rtl::OUString("Name");
                lQuery[0].Value <<= sTypeName;

                pFilter = GetFilterForProps(lQuery, nMust, nDont);
            }

            if (pFilter)
            {
                *ppFilter = pFilter;
                return ERRCODE_NONE;
            }
        }
    }
    catch(const Exception&)
    {}

    return ERRCODE_ABORT;
}

//----------------------------------------------------------------
sal_Bool SfxFilterMatcher::IsFilterInstalled_Impl( const SfxFilter* pFilter )
{
    if ( pFilter->GetFilterFlags() & SFX_FILTER_MUSTINSTALL )
    {
        // Here could a  re-installation be offered
        String aText( SfxResId(STR_FILTER_NOT_INSTALLED).toString() );
        aText.SearchAndReplaceAscii( "$(FILTER)", pFilter->GetUIName() );
        QueryBox aQuery( NULL, WB_YES_NO | WB_DEF_YES, aText );
        short nRet = aQuery.Execute();
        if ( nRet == RET_YES )
        {
#ifdef DBG_UTIL
            // Start Setup
            InfoBox( NULL, DEFINE_CONST_UNICODE("Here should the Setup now be starting!") ).Execute();
#endif
            // Installation must still give feedback if it worked or not,
            // then the  Filterflag be deleted
        }

        return ( !(pFilter->GetFilterFlags() & SFX_FILTER_MUSTINSTALL) );
    }
    else if ( pFilter->GetFilterFlags() & SFX_FILTER_CONSULTSERVICE )
    {
        String aText( SfxResId(STR_FILTER_CONSULT_SERVICE).toString() );
        aText.SearchAndReplaceAscii( "$(FILTER)", pFilter->GetUIName() );
        InfoBox ( NULL, aText ).Execute();
        return sal_False;
    }
    else
        return sal_True;
}


sal_uInt32 SfxFilterMatcher::DetectFilter( SfxMedium& rMedium, const SfxFilter**ppFilter, sal_Bool /*bPlugIn*/, sal_Bool bAPI ) const
/*  [Description]

    Here the Filter selection box is pulled up. Otherwise GuessFilter
 */

{
    const SfxFilter* pOldFilter = rMedium.GetFilter();
    if ( pOldFilter )
    {
        if( !IsFilterInstalled_Impl( pOldFilter ) )
            pOldFilter = 0;
        else
        {
            SFX_ITEMSET_ARG( rMedium.GetItemSet(), pSalvageItem, SfxStringItem, SID_DOC_SALVAGE, sal_False);
            if ( ( pOldFilter->GetFilterFlags() & SFX_FILTER_PACKED ) && pSalvageItem )
                // Salvage is always done without packing
                pOldFilter = 0;
        }
    }

    const SfxFilter* pFilter = pOldFilter;

    sal_Bool bPreview = rMedium.IsPreview_Impl();
    SFX_ITEMSET_ARG(rMedium.GetItemSet(), pReferer, SfxStringItem, SID_REFERER, sal_False);
    if ( bPreview && rMedium.IsRemote() && ( !pReferer || pReferer->GetValue().CompareToAscii("private:searchfolder:",21 ) != COMPARE_EQUAL ) )
        return ERRCODE_ABORT;

    ErrCode nErr = GuessFilter( rMedium, &pFilter );
    if ( nErr == ERRCODE_ABORT )
        return nErr;

    if ( nErr == ERRCODE_IO_PENDING )
    {
        *ppFilter = pFilter;
        return nErr;
    }

    if ( !pFilter )
    {
        const SfxFilter* pInstallFilter = NULL;

        // Now test the filter which are not installed (ErrCode is irrelevant)
        GuessFilter( rMedium, &pInstallFilter, SFX_FILTER_IMPORT, SFX_FILTER_CONSULTSERVICE );
        if ( pInstallFilter )
        {
            if ( IsFilterInstalled_Impl( pInstallFilter ) )
                // Maybe the filter was installed was installed afterwards.
                pFilter = pInstallFilter;
        }
        else
        {
          // Now test the filter, which first must be obtained by Star
          // (ErrCode is irrelevant)
            GuessFilter( rMedium, &pInstallFilter, SFX_FILTER_IMPORT, 0 );
            if ( pInstallFilter )
                IsFilterInstalled_Impl( pInstallFilter );
        }
    }

    sal_Bool bHidden = bPreview;
    SFX_ITEMSET_ARG( rMedium.GetItemSet(), pFlags, SfxStringItem, SID_OPTIONS, sal_False);
    if ( !bHidden && pFlags )
    {
        String aFlags( pFlags->GetValue() );
        aFlags.ToUpperAscii();
        if( STRING_NOTFOUND != aFlags.Search( 'H' ) )
            bHidden = sal_True;
    }
    *ppFilter = pFilter;

    if ( bHidden || (bAPI && nErr == ERRCODE_SFX_CONSULTUSER) )
        nErr = pFilter ? ERRCODE_NONE : ERRCODE_ABORT;
    return nErr;
}

const SfxFilter* SfxFilterMatcher::GetFilterForProps( const com::sun::star::uno::Sequence < ::com::sun::star::beans::NamedValue >& aSeq, SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceManager = ::comphelper::getProcessServiceFactory();
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerQuery > xTypeCFG;
    if( xServiceManager.is() == sal_True )
        xTypeCFG   = ::com::sun::star::uno::Reference < com::sun::star::container::XContainerQuery >( xServiceManager->createInstance( DEFINE_CONST_UNICODE( "com.sun.star.document.TypeDetection" ) ), ::com::sun::star::uno::UNO_QUERY );
    if ( xTypeCFG.is() )
    {
        // make query for all types matching the properties
        ::com::sun::star::uno::Reference < com::sun::star::container::XEnumeration > xEnum = xTypeCFG->createSubSetEnumerationByProperties( aSeq );
        while ( xEnum->hasMoreElements() )
        {
            ::comphelper::SequenceAsHashMap aProps( xEnum->nextElement() );
            ::rtl::OUString aValue;

            // try to get the preferred filter (works without loading all filters!)
            if ( (aProps[::rtl::OUString("PreferredFilter")] >>= aValue) && !aValue.isEmpty() )
            {
                const SfxFilter* pFilter = SfxFilter::GetFilterByName( aValue );
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
                        aProps[::rtl::OUString("Name")] >>= aValue;
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

    return 0;
}

const SfxFilter* SfxFilterMatcher::GetFilter4Mime( const ::rtl::OUString& rMediaType, SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    if ( m_rImpl.pList )
    {
        for ( size_t i = 0, n = m_rImpl.pList->size(); i < n; ++i )
        {
            const SfxFilter* pFilter = m_rImpl.pList->at( i );
            SfxFilterFlags nFlags = pFilter->GetFilterFlags();
            if ( (nFlags & nMust) == nMust && !(nFlags & nDont ) && pFilter->GetMimeType() == rMediaType )
                return pFilter;
        }

        return 0;
    }

    com::sun::star::uno::Sequence < com::sun::star::beans::NamedValue > aSeq(1);
    aSeq[0].Name = ::rtl::OUString("MediaType");
    aSeq[0].Value <<= rMediaType;
    return GetFilterForProps( aSeq, nMust, nDont );
}

const SfxFilter* SfxFilterMatcher::GetFilter4EA( const String& rType,SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    if ( m_rImpl.pList )
    {
        const SfxFilter* pFirst = 0;
        for ( size_t i = 0, n = m_rImpl.pList->size(); i < n; ++i )
        {
            const SfxFilter* pFilter = m_rImpl.pList->at( i );
            SfxFilterFlags nFlags = pFilter->GetFilterFlags();
            if ( (nFlags & nMust) == nMust && !(nFlags & nDont ) && pFilter->GetTypeName() == rType )
            {
                if (nFlags & SFX_FILTER_PREFERED)
                    return pFilter;
                if (!pFirst)
                    pFirst = pFilter;
            }
        }
        if (pFirst)
            return pFirst;

        return 0;
    }

    com::sun::star::uno::Sequence < com::sun::star::beans::NamedValue > aSeq(1);
    aSeq[0].Name = ::rtl::OUString("Name");
    aSeq[0].Value <<= ::rtl::OUString( rType );
    return GetFilterForProps( aSeq, nMust, nDont );
}

const SfxFilter* SfxFilterMatcher::GetFilter4Extension( const String& rExt, SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    if ( m_rImpl.pList )
    {
        for ( size_t i = 0, n = m_rImpl.pList->size(); i < n; ++i )
        {
            const SfxFilter* pFilter = m_rImpl.pList->at( i );
            SfxFilterFlags nFlags = pFilter->GetFilterFlags();
            if ( (nFlags & nMust) == nMust && !(nFlags & nDont ) )
            {
                String sWildCard = ToUpper_Impl( pFilter->GetWildcard().getGlob() );
                String sExt      = ToUpper_Impl( rExt );

                if (!sExt.Len())
                    continue;

                if (sExt.GetChar(0) != (sal_Unicode)'.')
                    sExt.Insert((sal_Unicode)'.', 0);

                WildCard aCheck(sWildCard, ';');
                if (aCheck.Matches(sExt))
                    return pFilter;
            }
        }

        return 0;
    }

    // Use extension without dot!
    String sExt( rExt );
    if ( sExt.Len() && ( sExt.GetChar(0) == (sal_Unicode)'.' ))
        sExt.Erase(0,1);

    com::sun::star::uno::Sequence < com::sun::star::beans::NamedValue > aSeq(1);
    aSeq[0].Name = ::rtl::OUString("Extensions");
    ::com::sun::star::uno::Sequence < ::rtl::OUString > aExts(1);
    aExts[0] = sExt;
    aSeq[0].Value <<= aExts;
    return GetFilterForProps( aSeq, nMust, nDont );
}

const SfxFilter* SfxFilterMatcher::GetFilter4ClipBoardId( sal_uInt32 nId, SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    if (nId == 0)
        return 0;

    com::sun::star::uno::Sequence < com::sun::star::beans::NamedValue > aSeq(1);
    ::rtl::OUString aName = SotExchange::GetFormatName( nId );
    aSeq[0].Name = ::rtl::OUString("ClipboardFormat");
    aSeq[0].Value <<= aName;
    return GetFilterForProps( aSeq, nMust, nDont );
}

const SfxFilter* SfxFilterMatcher::GetFilter4UIName( const String& rName, SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    m_rImpl.InitForIterating();
    const SfxFilter* pFirstFilter=0;
    for ( size_t i = 0, n = m_rImpl.pList->size(); i < n; ++i )
    {
        const SfxFilter* pFilter = m_rImpl.pList->at( i );
        SfxFilterFlags nFlags = pFilter->GetFilterFlags();
        if ( (nFlags & nMust) == nMust &&
             !(nFlags & nDont ) && pFilter->GetUIName() == rName )
        {
            if ( pFilter->GetFilterFlags() & SFX_FILTER_PREFERED )
                return pFilter;
            else if ( !pFirstFilter )
                pFirstFilter = pFilter;
        }
    }
    return pFirstFilter;
}

const SfxFilter* SfxFilterMatcher::GetFilter4FilterName( const String& rName, SfxFilterFlags nMust, SfxFilterFlags nDont ) const
{
    String aName( rName );
    sal_uInt16 nIndex = aName.SearchAscii(": ");
    if (  nIndex != STRING_NOTFOUND )
    {
        OSL_FAIL("Old filter name used!");
        aName = rName.Copy( nIndex + 2 );
    }

    if ( bFirstRead )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceManager = ::comphelper::getProcessServiceFactory();
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >     xFilterCFG                                                ;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >     xTypeCFG                                                  ;
        if( xServiceManager.is() == sal_True )
        {
            xFilterCFG = ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >( xServiceManager->createInstance( DEFINE_CONST_UNICODE( "com.sun.star.document.FilterFactory" ) ), ::com::sun::star::uno::UNO_QUERY );
            xTypeCFG   = ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >( xServiceManager->createInstance( DEFINE_CONST_UNICODE( "com.sun.star.document.TypeDetection" ) ), ::com::sun::star::uno::UNO_QUERY );
        }

        if( xFilterCFG.is() && xTypeCFG.is() )
        {
            if ( !pFilterArr )
                CreateFilterArr();
            else
            {
                for ( size_t i = 0, n = pFilterArr->size(); i < n; ++i )
                {
                    const SfxFilter* pFilter = pFilterArr->at( i );
                    SfxFilterFlags nFlags = pFilter->GetFilterFlags();
                    if ( (nFlags & nMust) == nMust && !(nFlags & nDont ) && pFilter->GetFilterName().CompareIgnoreCaseToAscii( aName ) == COMPARE_EQUAL )
                        return pFilter;
                }
            }

            SfxFilterContainer::ReadSingleFilter_Impl( rName, xTypeCFG, xFilterCFG, sal_False );
        }
    }

    SfxFilterList_Impl* pList = m_rImpl.pList;
    if ( !pList )
        pList = pFilterArr;

    for ( size_t i = 0, n = pList->size(); i < n; ++i )
    {
        const SfxFilter* pFilter = pList->at( i );
        SfxFilterFlags nFlags = pFilter->GetFilterFlags();
        if ( (nFlags & nMust) == nMust && !(nFlags & nDont ) && pFilter->GetFilterName().CompareIgnoreCaseToAscii( aName ) == COMPARE_EQUAL )
            return pFilter;
    }

    return NULL;
}

IMPL_STATIC_LINK( SfxFilterMatcher, MaybeFileHdl_Impl, String*, pString )
{
    const SfxFilter* pFilter = pThis->GetFilter4Extension( *pString, SFX_FILTER_IMPORT );
    if (pFilter && !pFilter->GetWildcard().Matches( String() ) &&
        !pFilter->GetWildcard().Matches(DEFINE_CONST_UNICODE("*.*")) &&
        !pFilter->GetWildcard().Matches(rtl::OUString('*'))
       )
    {
        return sal_True;
    }
    return sal_False;
}

//----------------------------------------------------------------

SfxFilterMatcherIter::SfxFilterMatcherIter(
    const SfxFilterMatcher& rMatcher,
    SfxFilterFlags nOrMaskP, SfxFilterFlags nAndMaskP )
    : nOrMask( nOrMaskP ), nAndMask( nAndMaskP ),
      nCurrent(0), m_rMatch(rMatcher.m_rImpl)
{
    if( nOrMask == 0xffff ) //Due to falty build on s
        nOrMask = 0;
    m_rMatch.InitForIterating();
}

//----------------------------------------------------------------

const SfxFilter* SfxFilterMatcherIter::Find_Impl()
{
    const SfxFilter* pFilter = 0;
    while( nCurrent < m_rMatch.pList->size() )
    {
        pFilter = m_rMatch.pList->at( nCurrent++ );
        SfxFilterFlags nFlags = pFilter->GetFilterFlags();
        if( ((nFlags & nOrMask) == nOrMask ) && !(nFlags & nAndMask ) )
            break;
        pFilter = 0;
    }

    return pFilter;
}

const SfxFilter* SfxFilterMatcherIter::First()
{
    nCurrent = 0;
    return Find_Impl();
}

//----------------------------------------------------------------

const SfxFilter* SfxFilterMatcherIter::Next()
{
    return Find_Impl();
}

/*---------------------------------------------------------------
    helper to build own formated string from given stringlist by
    using given seperator
  ---------------------------------------------------------------*/
::rtl::OUString implc_convertStringlistToString( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& lList     ,
                                                 const sal_Unicode&                                        cSeperator,
                                                 const ::rtl::OUString&                                    sPrefix   )
{
    ::rtl::OUStringBuffer   sString ( 1000 )           ;
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
            sString.append( cSeperator );
        }
    }
    return sString.makeStringAndClear();
}


void SfxFilterContainer::ReadSingleFilter_Impl(
    const ::rtl::OUString& rName,
    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& xTypeCFG,
    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& xFilterCFG,
    sal_Bool bUpdate
    )
{
    ::rtl::OUString sFilterName( rName );
    SfxFilterList_Impl& rList = *pFilterArr;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > lFilterProperties                           ;
    ::com::sun::star::uno::Any aResult;
    try
    {
        aResult = xFilterCFG->getByName( sFilterName );
    }
    catch( ::com::sun::star::container::NoSuchElementException& )
    {
        aResult = ::com::sun::star::uno::Any();
    }

    if( aResult >>= lFilterProperties )
    {
        // collect informations to add filter to container
        // (attention: some informations aren't available on filter directly ... you must search for corresponding type too!)
        sal_Int32       nFlags          = 0 ;
        sal_Int32       nClipboardId    = 0 ;
        sal_Int32       nDocumentIconId = 0 ;
        sal_Int32       nFormatVersion  = 0 ;
        ::rtl::OUString sMimeType           ;
        ::rtl::OUString sType               ;
        ::rtl::OUString sUIName             ;
        ::rtl::OUString sHumanName          ;
        ::rtl::OUString sDefaultTemplate    ;
        ::rtl::OUString sUserData           ;
        ::rtl::OUString sExtension          ;
        ::rtl::OUString sPattern            ;
        ::rtl::OUString sServiceName        ;

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
                lFilterProperties[nFilterProperty].Value >>= nFlags;
            }
            else if ( lFilterProperties[nFilterProperty].Name == "UIName" )
            {
                lFilterProperties[nFilterProperty].Value >>= sUIName;
            }
            else if ( lFilterProperties[nFilterProperty].Name == "UserData" )
            {
                ::com::sun::star::uno::Sequence< ::rtl::OUString > lUserData;
                lFilterProperties[nFilterProperty].Value >>= lUserData;
                sUserData = implc_convertStringlistToString( lUserData, ',', ::rtl::OUString() );
            }
            else if ( lFilterProperties[nFilterProperty].Name == "DocumentService" )
            {
                lFilterProperties[nFilterProperty].Value >>= sServiceName;
            }
            else if (lFilterProperties[nFilterProperty].Name == "FileSaveExtension")
            {
                // Extension preferred by the filter.  This takes precedence
                // over those that are given in the file format type.
                lFilterProperties[nFilterProperty].Value >>= sExtension;
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
                catch (const ::com::sun::star::container::NoSuchElementException&)
                {
                    aResult = ::com::sun::star::uno::Any();
                }

                ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > lTypeProperties;
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
                                ::com::sun::star::uno::Sequence< ::rtl::OUString > lExtensions;
                                lTypeProperties[nTypeProperty].Value >>= lExtensions;
                                sExtension = implc_convertStringlistToString( lExtensions, ';', DEFINE_CONST_UNICODE("*.") );
                            }
                        }
                        else if ( lTypeProperties[nTypeProperty].Name == "URLPattern" )
                        {
                                ::com::sun::star::uno::Sequence< ::rtl::OUString > lPattern;
                                lTypeProperties[nTypeProperty].Value >>= lPattern;
                                sPattern = implc_convertStringlistToString( lPattern, ';', ::rtl::OUString() );
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
            if((nFlags & SFX_FILTER_STARONEFILTER) == SFX_FILTER_STARONEFILTER)
            {
                nClipboardId = 0;
            }
        }
        // register SfxFilter
        // first erase module name from old filter names!
        // e.g: "scalc: DIF" => "DIF"
        sal_Int32 nStartRealName = sFilterName.indexOf( DEFINE_CONST_UNICODE(": "), 0 );
        if( nStartRealName != -1 )
        {
            OSL_FAIL("Old format, not supported!");
            sFilterName = sFilterName.copy( nStartRealName+2 );
        }

        SfxFilter* pFilter = bUpdate ? (SfxFilter*) SfxFilter::GetFilterByName( sFilterName ) : 0;
        sal_Bool bNew = sal_False;
        if (!pFilter)
        {
            bNew = sal_True;
            pFilter = new SfxFilter( sFilterName             ,
                                     sExtension              ,
                                     nFlags                  ,
                                     nClipboardId            ,
                                     sType                   ,
                                     (sal_uInt16)nDocumentIconId ,
                                     sMimeType               ,
                                     sUserData               ,
                                     sServiceName );
        }
        else
        {
            pFilter->aFilterName  = sFilterName;
            pFilter->aWildCard    = WildCard(sExtension, ';');
            pFilter->nFormatType  = nFlags;
            pFilter->lFormat      = nClipboardId;
            pFilter->aTypeName    = sType;
            pFilter->nDocIcon     = (sal_uInt16)nDocumentIconId;
            pFilter->aMimeType    = sMimeType;
            pFilter->aUserData    = sUserData;
            pFilter->aServiceName = sServiceName;
        }

        // Don't forget to set right UIName!
        // Otherwise internal name is used as fallback ...
        pFilter->SetUIName( sUIName );
        pFilter->SetDefaultTemplate( sDefaultTemplate );
        if( nFormatVersion )
        {
            pFilter->SetVersion( nFormatVersion );
        }
        pFilter->SetURLPattern(sPattern);

        if (bNew)
            rList.push_back( pFilter );
    }
}

void SfxFilterContainer::ReadFilters_Impl( sal_Bool bUpdate )
{
    RTL_LOGFILE_CONTEXT( aMeasure, "sfx2 (as96863) ::SfxFilterContainer::ReadFilters" );
    if ( !pFilterArr )
        CreateFilterArr();

    bFirstRead = sal_False;
    SfxFilterList_Impl& rList = *pFilterArr;

    try
    {
        // get the FilterFactory service to access the registered filters ... and types!
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceManager = ::comphelper::getProcessServiceFactory();
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >     xFilterCFG                                                ;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >     xTypeCFG                                                  ;
        if( xServiceManager.is() == sal_True )
        {
            xFilterCFG = ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >( xServiceManager->createInstance( DEFINE_CONST_UNICODE( "com.sun.star.document.FilterFactory" ) ), ::com::sun::star::uno::UNO_QUERY );
            xTypeCFG   = ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >( xServiceManager->createInstance( DEFINE_CONST_UNICODE( "com.sun.star.document.TypeDetection" ) ), ::com::sun::star::uno::UNO_QUERY );
        }

        if(
            ( xFilterCFG.is() == sal_True ) &&
            ( xTypeCFG.is()   == sal_True )
          )
        {
            // select right query to get right set of filters for search modul
            ::com::sun::star::uno::Sequence< ::rtl::OUString > lFilterNames = xFilterCFG->getElementNames();
            if ( lFilterNames.getLength() )
            {
                // If list of filters already exist ...
                // ReadExternalFilters must work in update mode.
                // Best way seams to mark all filters NOT_INSTALLED
                // and change it back for all valid filters afterwards.
                if( !rList.empty() )
                {
                    bUpdate = sal_True;
                    SfxFilter* pFilter;
                    for ( size_t i = 0, n = rList.size(); i < n; ++i )
                    {
                        pFilter = NULL;
                        pFilter = rList[ i ];
                        pFilter->nFormatType |= SFX_FILTER_NOTINSTALLED;
                    }
                }

                // get all properties of filters ... put it into the filter container
                sal_Int32 nFilterCount = lFilterNames.getLength();
                sal_Int32 nFilter=0;
                for( nFilter=0; nFilter<nFilterCount; ++nFilter )
                {
                    // Try to get filter .. but look for any exceptions!
                    // May be filter was deleted by another thread ...
                    ::rtl::OUString sFilterName = lFilterNames[nFilter];
                    ReadSingleFilter_Impl( sFilterName, xTypeCFG, xFilterCFG, bUpdate );
                }
            }
        }
    }
    catch(const ::com::sun::star::uno::Exception&)
    {
        DBG_ASSERT( sal_False, "SfxFilterContainer::ReadFilter()\nException detected. Possible not all filters could be cached.\n" );
    }

    if ( bUpdate )
    {
        // global filter arry was modified, factory specific ones might need an
        // update too
        std::for_each(aImplArr.begin(), aImplArr.end(),
            std::mem_fun_ref(&SfxFilterMatcher_Impl::Update));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
