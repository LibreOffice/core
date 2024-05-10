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

#include <sal/config.h>
#include <sal/log.hxx>

#include <com/sun/star/deployment/DeploymentException.hpp>
#include <com/sun/star/deployment/ExtensionManager.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/linguistic2/XSupportedLocales.hpp>
#include <com/sun/star/linguistic2/DictionaryListEventFlags.hpp>
#include <com/sun/star/linguistic2/LinguServiceEventFlags.hpp>
#include <com/sun/star/linguistic2/ProofreadingIterator.hpp>

#include <tools/debug.hxx>
#include <unotools/lingucfg.hxx>
#include <utility>
#include <vcl/svapp.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <i18nlangtag/lang.h>
#include <i18nlangtag/languagetag.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>

#include "lngsvcmgr.hxx"
#include <linguistic/misc.hxx>
#include "spelldsp.hxx"
#include "hyphdsp.hxx"
#include "thesdsp.hxx"
#include "gciterator.hxx"

using namespace com::sun::star;
using namespace linguistic;

uno::Sequence< OUString > static GetLangSvcList( const uno::Any &rVal );
uno::Sequence< OUString > static GetLangSvc( const uno::Any &rVal );

static bool lcl_SeqHasString( const uno::Sequence< OUString > &rSeq, const OUString &rText )
{
    return !rText.isEmpty()
        && comphelper::findValue(rSeq, rText) != -1;
}


static uno::Sequence< lang::Locale > GetAvailLocales(
        const uno::Sequence< OUString > &rSvcImplNames )
{
    uno::Sequence< lang::Locale > aRes;

    uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );
    if( rSvcImplNames.hasElements() )
    {
        std::set< LanguageType > aLanguages;

        // All of these services only use one arg, but need two args for compat reasons
        uno::Sequence< uno::Any > aArgs(2);
        aArgs.getArray()[0] <<= GetLinguProperties();

        // check all services for the supported languages and new
        // languages to the result

        for (const OUString& rImplName : rSvcImplNames)
        {
            uno::Reference< linguistic2::XSupportedLocales > xSuppLoc;
            try
            {
                xSuppLoc.set( xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                                 rImplName, aArgs, xContext ),
                              uno::UNO_QUERY );
            }
            catch (uno::Exception &)
            {
                SAL_WARN( "linguistic", "createInstanceWithArguments failed" );
            }

            if (xSuppLoc.is())
            {
                const uno::Sequence< lang::Locale > aLoc( xSuppLoc->getLocales() );
                for (const lang::Locale& rLoc : aLoc)
                {
                    LanguageType nLang = LinguLocaleToLanguage( rLoc );

                    // It's a set, so insertion fails if language was already added.
                    aLanguages.insert( nLang );
                }
            }
            else
            {
                SAL_WARN( "linguistic", "interface not supported by service" );
            }
        }

        // build return sequence
        std::vector<lang::Locale> aVec;
        aVec.reserve(aLanguages.size());

        std::transform(aLanguages.begin(), aLanguages.end(), std::back_inserter(aVec),
            [](const LanguageType& rLang) -> lang::Locale { return LanguageTag::convertToLocale(rLang); });

        aRes = comphelper::containerToSequence(aVec);
    }

    return aRes;
}


struct SvcInfo
{
    const OUString                  aSvcImplName;
    const std::vector< LanguageType >    aSuppLanguages;

    SvcInfo( OUString aSvcImplName_,
             std::vector< LanguageType >&& rSuppLanguages ) :
        aSvcImplName    (std::move(aSvcImplName_)),
        aSuppLanguages  (std::move(rSuppLanguages))
    {
    }

    bool    HasLanguage( LanguageType nLanguage ) const;
};


bool SvcInfo::HasLanguage( LanguageType nLanguage ) const
{
    for ( auto const & i : aSuppLanguages)
    {
        if (nLanguage == i)
            return true;
    }
    return false;
}

class LngSvcMgrListenerHelper :
    public cppu::WeakImplHelper
    <
        linguistic2::XLinguServiceEventListener,
        linguistic2::XDictionaryListEventListener
    >
{
    LngSvcMgr  &rMyManager;

    ::comphelper::OInterfaceContainerHelper2           aLngSvcMgrListeners;
    ::comphelper::OInterfaceContainerHelper2           aLngSvcEvtBroadcasters;
    uno::Reference< linguistic2::XSearchableDictionaryList >           xDicList;

    sal_Int16   nCombinedLngSvcEvt;

    void    LaunchEvent( sal_Int16 nLngSvcEvtFlags );

    void Timeout();

public:
    LngSvcMgrListenerHelper( LngSvcMgr &rLngSvcMgr,
        uno::Reference< linguistic2::XSearchableDictionaryList > xDicList );

    LngSvcMgrListenerHelper(const LngSvcMgrListenerHelper&) = delete;
    LngSvcMgrListenerHelper& operator=(const LngSvcMgrListenerHelper&) = delete;

    // lang::XEventListener
    virtual void SAL_CALL
        disposing( const lang::EventObject& rSource ) override;

    // linguistic2::XLinguServiceEventListener
    virtual void SAL_CALL
        processLinguServiceEvent( const linguistic2::LinguServiceEvent& aLngSvcEvent ) override;

    // linguistic2::XDictionaryListEventListener
    virtual void SAL_CALL
        processDictionaryListEvent(
                const linguistic2::DictionaryListEvent& rDicListEvent ) override;

    inline  void    AddLngSvcMgrListener(
                        const uno::Reference< lang::XEventListener >& rxListener );
    inline  void    RemoveLngSvcMgrListener(
                        const uno::Reference< lang::XEventListener >& rxListener );
    void    DisposeAndClear( const lang::EventObject &rEvtObj );
    void    AddLngSvcEvtBroadcaster(
                        const uno::Reference< linguistic2::XLinguServiceEventBroadcaster > &rxBroadcaster );
    void    RemoveLngSvcEvtBroadcaster(
                        const uno::Reference< linguistic2::XLinguServiceEventBroadcaster > &rxBroadcaster );

    void    AddLngSvcEvt( sal_Int16 nLngSvcEvt );
};


LngSvcMgrListenerHelper::LngSvcMgrListenerHelper(
        LngSvcMgr &rLngSvcMgr,
        uno::Reference< linguistic2::XSearchableDictionaryList > xDicList_ ) :
    rMyManager              ( rLngSvcMgr ),
    aLngSvcMgrListeners     ( GetLinguMutex() ),
    aLngSvcEvtBroadcasters  ( GetLinguMutex() ),
    xDicList                (std::move( xDicList_ ))
{
    if (xDicList.is())
    {
        xDicList->addDictionaryListEventListener(
            static_cast<linguistic2::XDictionaryListEventListener *>(this), false );
    }

    nCombinedLngSvcEvt = 0;
}


void SAL_CALL LngSvcMgrListenerHelper::disposing( const lang::EventObject& rSource )
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    uno::Reference< uno::XInterface > xRef( rSource.Source );
    if ( xRef.is() )
    {
        aLngSvcMgrListeners   .removeInterface( xRef );
        aLngSvcEvtBroadcasters.removeInterface( xRef );
        if (xDicList == xRef)
            xDicList = nullptr;
    }
}

void LngSvcMgrListenerHelper::Timeout()
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    {
        // change event source to LinguServiceManager since the listeners
        // probably do not know (and need not to know) about the specific
        // SpellChecker's or Hyphenator's.
        linguistic2::LinguServiceEvent aEvtObj(
            static_cast<css::linguistic2::XLinguServiceManager*>(&rMyManager), nCombinedLngSvcEvt );
        nCombinedLngSvcEvt = 0;

        if (rMyManager.mxSpellDsp.is())
            rMyManager.mxSpellDsp->FlushSpellCache();

        // pass event on to linguistic2::XLinguServiceEventListener's
        aLngSvcMgrListeners.notifyEach( &linguistic2::XLinguServiceEventListener::processLinguServiceEvent, aEvtObj );
    }
}


void LngSvcMgrListenerHelper::AddLngSvcEvt( sal_Int16 nLngSvcEvt )
{
    nCombinedLngSvcEvt |= nLngSvcEvt;
    Timeout();
}


void SAL_CALL
    LngSvcMgrListenerHelper::processLinguServiceEvent(
            const linguistic2::LinguServiceEvent& rLngSvcEvent )
{
    osl::MutexGuard aGuard( GetLinguMutex() );
    AddLngSvcEvt( rLngSvcEvent.nEvent );
}


void SAL_CALL
    LngSvcMgrListenerHelper::processDictionaryListEvent(
            const linguistic2::DictionaryListEvent& rDicListEvent )
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    sal_Int16 nDlEvt = rDicListEvent.nCondensedEvent;
    if (0 == nDlEvt)
        return;

    // we do keep the original event source here though...

    // pass event on to linguistic2::XDictionaryListEventListener's
    aLngSvcMgrListeners.notifyEach( &linguistic2::XDictionaryListEventListener::processDictionaryListEvent, rDicListEvent );

    // "translate" DictionaryList event into linguistic2::LinguServiceEvent
    sal_Int16 nLngSvcEvt = 0;
    sal_Int16 const nSpellCorrectFlags =
            linguistic2::DictionaryListEventFlags::ADD_NEG_ENTRY        |
            linguistic2::DictionaryListEventFlags::DEL_POS_ENTRY        |
            linguistic2::DictionaryListEventFlags::ACTIVATE_NEG_DIC |
            linguistic2::DictionaryListEventFlags::DEACTIVATE_POS_DIC;
    if (0 != (nDlEvt & nSpellCorrectFlags))
        nLngSvcEvt |= linguistic2::LinguServiceEventFlags::SPELL_CORRECT_WORDS_AGAIN;

    sal_Int16 const nSpellWrongFlags =
            linguistic2::DictionaryListEventFlags::ADD_POS_ENTRY        |
            linguistic2::DictionaryListEventFlags::DEL_NEG_ENTRY        |
            linguistic2::DictionaryListEventFlags::ACTIVATE_POS_DIC |
            linguistic2::DictionaryListEventFlags::DEACTIVATE_NEG_DIC;
    if (0 != (nDlEvt & nSpellWrongFlags))
        nLngSvcEvt |= linguistic2::LinguServiceEventFlags::SPELL_WRONG_WORDS_AGAIN;

    sal_Int16 const nHyphenateFlags =
            linguistic2::DictionaryListEventFlags::ADD_POS_ENTRY        |
            linguistic2::DictionaryListEventFlags::DEL_POS_ENTRY        |
            linguistic2::DictionaryListEventFlags::ACTIVATE_POS_DIC |
            linguistic2::DictionaryListEventFlags::ACTIVATE_NEG_DIC;
    if (0 != (nDlEvt & nHyphenateFlags))
        nLngSvcEvt |= linguistic2::LinguServiceEventFlags::HYPHENATE_AGAIN;

    if (rMyManager.mxSpellDsp.is())
        rMyManager.mxSpellDsp->FlushSpellCache();
    if (nLngSvcEvt)
        LaunchEvent( nLngSvcEvt );
}


void LngSvcMgrListenerHelper::LaunchEvent( sal_Int16 nLngSvcEvtFlags )
{
    linguistic2::LinguServiceEvent aEvt(
        static_cast<css::linguistic2::XLinguServiceManager*>(&rMyManager), nLngSvcEvtFlags );

    // pass event on to linguistic2::XLinguServiceEventListener's
    aLngSvcMgrListeners.notifyEach( &linguistic2::XLinguServiceEventListener::processLinguServiceEvent, aEvt );
}


inline void LngSvcMgrListenerHelper::AddLngSvcMgrListener(
        const uno::Reference< lang::XEventListener >& rxListener )
{
    aLngSvcMgrListeners.addInterface( rxListener );
}


inline void LngSvcMgrListenerHelper::RemoveLngSvcMgrListener(
        const uno::Reference< lang::XEventListener >& rxListener )
{
    aLngSvcMgrListeners.removeInterface( rxListener );
}


void LngSvcMgrListenerHelper::DisposeAndClear( const lang::EventObject &rEvtObj )
{
    // call "disposing" for all listeners and clear list
    aLngSvcMgrListeners   .disposeAndClear( rEvtObj );

    // remove references to this object hold by the broadcasters
    comphelper::OInterfaceIteratorHelper2 aIt( aLngSvcEvtBroadcasters );
    while (aIt.hasMoreElements())
    {
        uno::Reference< linguistic2::XLinguServiceEventBroadcaster > xRef( aIt.next(), uno::UNO_QUERY );
        if (xRef.is())
            RemoveLngSvcEvtBroadcaster( xRef );
    }

    // remove reference to this object hold by the dictionary-list
    if (xDicList.is())
    {
        xDicList->removeDictionaryListEventListener(
            static_cast<linguistic2::XDictionaryListEventListener *>(this) );
        xDicList = nullptr;
    }
}


void LngSvcMgrListenerHelper::AddLngSvcEvtBroadcaster(
        const uno::Reference< linguistic2::XLinguServiceEventBroadcaster > &rxBroadcaster )
{
    if (rxBroadcaster.is())
    {
        aLngSvcEvtBroadcasters.addInterface( rxBroadcaster );
        rxBroadcaster->addLinguServiceEventListener(
                static_cast<linguistic2::XLinguServiceEventListener *>(this) );
    }
}


void LngSvcMgrListenerHelper::RemoveLngSvcEvtBroadcaster(
        const uno::Reference< linguistic2::XLinguServiceEventBroadcaster > &rxBroadcaster )
{
    if (rxBroadcaster.is())
    {
        aLngSvcEvtBroadcasters.removeInterface( rxBroadcaster );
        rxBroadcaster->removeLinguServiceEventListener(
                static_cast<linguistic2::XLinguServiceEventListener *>(this) );
    }
}


LngSvcMgr::LngSvcMgr()
    : utl::ConfigItem(u"Office.Linguistic"_ustr)
    , aEvtListeners(GetLinguMutex())
    , aUpdateIdle("LngSvcMgr aUpdateIdle")
{
    bDisposing = false;

    // request notify events when properties (i.e. something in the subtree) changes
    uno::Sequence< OUString > aNames{
        u"ServiceManager/SpellCheckerList"_ustr,
        u"ServiceManager/GrammarCheckerList"_ustr,
        u"ServiceManager/HyphenatorList"_ustr,
        u"ServiceManager/ThesaurusList"_ustr
    };
    EnableNotification( aNames );

    UpdateAll();

    aUpdateIdle.SetPriority(TaskPriority::LOWEST);
    aUpdateIdle.SetInvokeHandler(LINK(this, LngSvcMgr, updateAndBroadcast));

    // request to be notified if an extension has been added/removed
    uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());

    uno::Reference<deployment::XExtensionManager> xExtensionManager;
    try {
        xExtensionManager = deployment::ExtensionManager::get(xContext);
    } catch ( const uno::DeploymentException & ) {
        SAL_WARN( "linguistic", "no extension manager - should fire on mobile only" );
    } catch ( const deployment::DeploymentException & ) {
        SAL_WARN( "linguistic", "no extension manager - should fire on mobile only" );
    }
    if (xExtensionManager.is())
    {
        xMB.set(xExtensionManager, uno::UNO_QUERY_THROW);

        uno::Reference<util::XModifyListener> xListener(this);
        xMB->addModifyListener( xListener );
    }
}

// css::util::XModifyListener
void LngSvcMgr::modified(const lang::EventObject&)
{
    osl::MutexGuard aGuard(GetLinguMutex());
    //assume that if an extension has been added/removed that
    //it might be a dictionary extension, so drop our cache

    pAvailSpellSvcs.reset();
    pAvailGrammarSvcs.reset();
    pAvailHyphSvcs.reset();
    pAvailThesSvcs.reset();

    //schedule in an update to execute in the main thread
    aUpdateIdle.Start();
}

bool LngSvcMgr::joinThreads()
{
    if (mxGrammarDsp && !
        mxGrammarDsp->joinThreads())
        return false;
    return true;
}

//run update, and inform everyone that dictionaries (may) have changed, this
//needs to be run in the main thread because
//utl::ConfigChangeListener_Impl::changesOccurred grabs the SolarMutex and we
//get notified that an extension was added from an extension manager thread
IMPL_LINK_NOARG(LngSvcMgr, updateAndBroadcast, Timer *, void)
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    UpdateAll();

    if (mxListenerHelper.is())
    {
        mxListenerHelper->AddLngSvcEvt(
                linguistic2::LinguServiceEventFlags::SPELL_CORRECT_WORDS_AGAIN |
                linguistic2::LinguServiceEventFlags::SPELL_WRONG_WORDS_AGAIN |
                linguistic2::LinguServiceEventFlags::PROOFREAD_AGAIN |
                linguistic2::LinguServiceEventFlags::HYPHENATE_AGAIN );
    }
}

void LngSvcMgr::stopListening()
{
    osl::MutexGuard aGuard(GetLinguMutex());

    if (!xMB.is())
        return;

    try
    {
            uno::Reference<util::XModifyListener>  xListener(this);
            xMB->removeModifyListener(xListener);
    }
    catch (const uno::Exception&)
    {
    }

    xMB.clear();
}

void LngSvcMgr::disposing(const lang::EventObject&)
{
    stopListening();
}

#if defined __GNUC__ && !defined __clang__ && __GNUC__ == 14
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
LngSvcMgr::~LngSvcMgr()
{
    stopListening();

    // memory for pSpellDsp, pHyphDsp, pThesDsp, pListenerHelper
    // will be freed in the destructor of the respective Reference's
    // xSpellDsp, xGrammarDsp, xHyphDsp, xThesDsp

    pAvailSpellSvcs.reset();
    pAvailGrammarSvcs.reset();
    pAvailHyphSvcs.reset();
    pAvailThesSvcs.reset();
}
#if defined __GNUC__ && !defined __clang__ && __GNUC__ == 14
#pragma GCC diagnostic pop
#endif

namespace
{
    using lang::Locale;
    using uno::Any;
    using uno::Sequence;

    bool lcl_FindEntry( const OUString &rEntry, const Sequence< OUString > &rCfgSvcs )
    {
        return comphelper::findValue(rCfgSvcs, rEntry) != -1;
    }

    bool lcl_FindEntry( const OUString &rEntry, const std::vector< OUString > &rCfgSvcs )
    {
        return std::find(rCfgSvcs.begin(), rCfgSvcs.end(), rEntry) != rCfgSvcs.end();
    }

    Sequence< OUString > lcl_GetLastFoundSvcs(
            SvtLinguConfig const &rCfg,
            const OUString &rLastFoundList ,
            const OUString& rCfgLocaleStr )
    {
        Sequence< OUString > aRes;

        Sequence< OUString > aNodeNames( rCfg.GetNodeNames(rLastFoundList) );
        bool bFound = lcl_FindEntry( rCfgLocaleStr, aNodeNames);

        if (bFound)
        {
            Sequence< OUString > aNames { rLastFoundList + "/" + rCfgLocaleStr };
            Sequence< Any > aValues( rCfg.GetProperties( aNames ) );
            if (aValues.hasElements())
            {
                SAL_WARN_IF( aValues.getLength() != 1, "linguistic", "unexpected length of sequence" );
                Sequence< OUString > aSvcImplNames;
                if (aValues.getConstArray()[0] >>= aSvcImplNames)
                    aRes = aSvcImplNames;
                else
                {
                    SAL_WARN( "linguistic", "type mismatch" );
                }
            }
        }

        return aRes;
    }

    Sequence< OUString > lcl_RemoveMissingEntries(
            const Sequence< OUString > &rCfgSvcs,
            const Sequence< OUString > &rAvailSvcs )
    {
        std::vector<OUString> aRes;
        aRes.reserve(rCfgSvcs.getLength());

        std::copy_if(rCfgSvcs.begin(), rCfgSvcs.end(), std::back_inserter(aRes),
            [&rAvailSvcs](const OUString& entry) { return lcl_SeqHasString(rAvailSvcs, entry); });

        return comphelper::containerToSequence(aRes);
    }

    Sequence< OUString > lcl_GetNewEntries(
            const Sequence< OUString > &rLastFoundSvcs,
            const Sequence< OUString > &rAvailSvcs )
    {
        std::vector<OUString> aRes;
        aRes.reserve(rAvailSvcs.getLength());

        std::copy_if(rAvailSvcs.begin(), rAvailSvcs.end(), std::back_inserter(aRes),
            [&rLastFoundSvcs](const OUString& rEntry) {
                return !rEntry.isEmpty() && !lcl_FindEntry( rEntry, rLastFoundSvcs ); });

        return comphelper::containerToSequence(aRes);
    }

    Sequence< OUString > lcl_MergeSeq(
            const Sequence< OUString > &rCfgSvcs,
            const Sequence< OUString > &rNewSvcs )
    {
        std::vector<OUString> aRes;
        aRes.reserve(rCfgSvcs.getLength() + rNewSvcs.getLength());

        auto lVecNotHasString = [&aRes](const OUString& rEntry)
            { return !rEntry.isEmpty() && !lcl_FindEntry(rEntry, aRes); };

        // add previously configured service first and append
        // new found services at the end
        for (const Sequence< OUString > &rSeq : { rCfgSvcs, rNewSvcs })
        {
            std::copy_if(rSeq.begin(), rSeq.end(), std::back_inserter(aRes), lVecNotHasString);
        }

        return comphelper::containerToSequence(aRes);
    }
}

void LngSvcMgr::UpdateAll()
{
    using beans::PropertyValue;
    using lang::Locale;
    using uno::Sequence;

    typedef std::map< OUString, Sequence< OUString > > list_entry_map_t;

    SvtLinguConfig aCfg;

    const int nNumServices = 4;
    static constexpr OUString apServices[nNumServices] =  { SN_SPELLCHECKER, SN_GRAMMARCHECKER, SN_HYPHENATOR, SN_THESAURUS };
    const char * const apCurLists[nNumServices]       =  { "ServiceManager/SpellCheckerList",       "ServiceManager/GrammarCheckerList",       "ServiceManager/HyphenatorList",       "ServiceManager/ThesaurusList" };
    const char * const apLastFoundLists[nNumServices] =  { "ServiceManager/LastFoundSpellCheckers", "ServiceManager/LastFoundGrammarCheckers", "ServiceManager/LastFoundHyphenators", "ServiceManager/LastFoundThesauri" };

    // usage of indices as above: 0 = spell checker, 1 = grammar checker, 2 = hyphenator, 3 = thesaurus
    std::vector< list_entry_map_t > aLastFoundSvcs(nNumServices);
    std::vector< list_entry_map_t > aCurSvcs(nNumServices);

    for (int k = 0;  k < nNumServices;  ++k)
    {
        OUString const & aService = apServices[k];
        OUString aActiveList( OUString::createFromAscii( apCurLists[k] ) );
        OUString aLastFoundList( OUString::createFromAscii( apLastFoundLists[k] ) );


        // remove configured but not available language/services entries

        const Sequence< OUString > aNodeNames( aCfg.GetNodeNames( aActiveList ) );   // list of configured locales
        for (const OUString& rNodeName : aNodeNames)
        {
            Locale aLocale( LanguageTag::convertToLocale( rNodeName));
            Sequence< OUString > aCfgSvcs( getConfiguredServices( aService, aLocale ));
            Sequence< OUString > aAvailSvcs( getAvailableServices( aService, aLocale ));

            aCfgSvcs = lcl_RemoveMissingEntries( aCfgSvcs, aAvailSvcs );

            aCurSvcs[k][ rNodeName ] = aCfgSvcs;
        }


        // add new available language/service entries
        // and
        // set last found services to currently available ones

        const Sequence< Locale > aAvailLocales( getAvailableLocales(aService) );
        for (const Locale& rAvailLocale : aAvailLocales)
        {
            OUString aCfgLocaleStr( LanguageTag::convertToBcp47( rAvailLocale));

            Sequence< OUString > aAvailSvcs( getAvailableServices( aService, rAvailLocale ));

            aLastFoundSvcs[k][ aCfgLocaleStr ] = aAvailSvcs;

            Sequence< OUString > aLastSvcs(
                    lcl_GetLastFoundSvcs( aCfg, aLastFoundList , aCfgLocaleStr ));
            Sequence< OUString > aNewSvcs =
                    lcl_GetNewEntries( aLastSvcs, aAvailSvcs );

            Sequence< OUString > aCfgSvcs( aCurSvcs[k][ aCfgLocaleStr ] );

            // merge services list (previously configured to be listed first).
            aCfgSvcs = lcl_MergeSeq( aCfgSvcs, aNewSvcs );

            aCurSvcs[k][ aCfgLocaleStr ] = aCfgSvcs;
        }
    }


    // write new data back to configuration

    for (int k = 0;  k < nNumServices;  ++k)
    {
        for (int i = 0;  i < 2;  ++i)
        {
            const char *pSubNodeName = (i == 0) ? apCurLists[k] : apLastFoundLists[k];
            OUString aSubNodeName( OUString::createFromAscii(pSubNodeName) );

            list_entry_map_t &rCurMap = (i == 0) ? aCurSvcs[k] : aLastFoundSvcs[k];
            sal_Int32 nVals = static_cast< sal_Int32 >( rCurMap.size() );
            Sequence< PropertyValue > aNewValues( nVals );
            PropertyValue *pNewValue = aNewValues.getArray();
            for (auto const& elem : rCurMap)
            {
                pNewValue->Name = aSubNodeName + "/" + elem.first;
                pNewValue->Value <<= elem.second;
                ++pNewValue;
            }
            OSL_ENSURE( pNewValue - aNewValues.getConstArray() == nVals,
                    "possible mismatch of sequence size and property number" );

            {
                // add new or replace existing entries.
                bool bRes = aCfg.ReplaceSetProperties( aSubNodeName, aNewValues );
                SAL_WARN_IF(!bRes, "linguistic", "failed to set new configuration values");
            }
        }
    }

    //The new settings in the configuration get applied ! because we are
    //listening to the configuration for changes of the relevant ! properties
    //and Notify applies the new settings.
}

void LngSvcMgr::Notify( const uno::Sequence< OUString > &rPropertyNames )
{
    static constexpr OUString aSpellCheckerList( u"ServiceManager/SpellCheckerList"_ustr );
    static constexpr OUString aGrammarCheckerList( u"ServiceManager/GrammarCheckerList"_ustr );
    static constexpr OUString aHyphenatorList( u"ServiceManager/HyphenatorList"_ustr );
    static constexpr OUString aThesaurusList( u"ServiceManager/ThesaurusList"_ustr );

    const uno::Sequence< OUString > aSpellCheckerListEntries( GetNodeNames( aSpellCheckerList ) );
    const uno::Sequence< OUString > aGrammarCheckerListEntries( GetNodeNames( aGrammarCheckerList ) );
    const uno::Sequence< OUString > aHyphenatorListEntries( GetNodeNames( aHyphenatorList ) );
    const uno::Sequence< OUString > aThesaurusListEntries( GetNodeNames( aThesaurusList ) );

    uno::Sequence< uno::Any > aValues;
    uno::Sequence< OUString > aNames( 1 );
    OUString *pNames = aNames.getArray();

    for (const OUString& rName : rPropertyNames)
    {
        // property names look like
        // "ServiceManager/ThesaurusList/de-CH"

        sal_Int32 nKeyStart;
        nKeyStart = rName.lastIndexOf( '/' );
        OUString aKeyText;
        if (nKeyStart != -1)
            aKeyText = rName.copy( nKeyStart + 1 );
        SAL_WARN_IF( aKeyText.isEmpty(), "linguistic", "unexpected key (lang::Locale) string" );
        if (rName.startsWith( aSpellCheckerList ))
        {
            osl::MutexGuard aGuard(GetLinguMutex());

            // delete old cached data, needs to be acquired new on demand
            pAvailSpellSvcs.reset();

            if (lcl_SeqHasString( aSpellCheckerListEntries, aKeyText ))
            {
                pNames[0] = aSpellCheckerList + "/" + aKeyText;
                aValues = /*aCfg.*/GetProperties( aNames );
                uno::Sequence< OUString > aSvcImplNames;
                if (aValues.hasElements())
                    aSvcImplNames = GetLangSvcList( aValues.getConstArray()[0] );

                LanguageType nLang = LANGUAGE_NONE;
                if (!aKeyText.isEmpty())
                    nLang = LanguageTag::convertToLanguageType( aKeyText );

                GetSpellCheckerDsp_Impl( false );     // don't set service list, it will be done below
                mxSpellDsp->SetServiceList( LanguageTag::convertToLocale(nLang), aSvcImplNames );
            }
        }
        else if (rName.startsWith( aGrammarCheckerList ))
        {
            osl::MutexGuard aGuard(GetLinguMutex());

            // delete old cached data, needs to be acquired new on demand
            pAvailGrammarSvcs.reset();

            if (lcl_SeqHasString( aGrammarCheckerListEntries, aKeyText ))
            {
                pNames[0] = aGrammarCheckerList + "/" + aKeyText;
                aValues = /*aCfg.*/GetProperties( aNames );
                uno::Sequence< OUString > aSvcImplNames;
                if (aValues.hasElements())
                    aSvcImplNames = GetLangSvc( aValues.getConstArray()[0] );

                LanguageType nLang = LANGUAGE_NONE;
                if (!aKeyText.isEmpty())
                    nLang = LanguageTag::convertToLanguageType( aKeyText );

                if (SvtLinguConfig().HasGrammarChecker())
                {
                    GetGrammarCheckerDsp_Impl( false );   // don't set service list, it will be done below
                    mxGrammarDsp->SetServiceList( LanguageTag::convertToLocale(nLang), aSvcImplNames );
                }
            }
        }
        else if (rName.startsWith( aHyphenatorList ))
        {
            osl::MutexGuard aGuard(GetLinguMutex());

            // delete old cached data, needs to be acquired new on demand
            pAvailHyphSvcs.reset();

            if (lcl_SeqHasString( aHyphenatorListEntries, aKeyText ))
            {
                pNames[0] = aHyphenatorList + "/" + aKeyText;
                aValues = /*aCfg.*/GetProperties( aNames );
                uno::Sequence< OUString > aSvcImplNames;
                if (aValues.hasElements())
                    aSvcImplNames = GetLangSvc( aValues.getConstArray()[0] );

                LanguageType nLang = LANGUAGE_NONE;
                if (!aKeyText.isEmpty())
                    nLang = LanguageTag::convertToLanguageType( aKeyText );

                GetHyphenatorDsp_Impl( false );   // don't set service list, it will be done below
                mxHyphDsp->SetServiceList( LanguageTag::convertToLocale(nLang), aSvcImplNames );
            }
        }
        else if (rName.startsWith( aThesaurusList ))
        {
            osl::MutexGuard aGuard(GetLinguMutex());

            // delete old cached data, needs to be acquired new on demand
            pAvailThesSvcs.reset();

            if (lcl_SeqHasString( aThesaurusListEntries, aKeyText ))
            {
                pNames[0] = aThesaurusList + "/" + aKeyText;
                aValues = /*aCfg.*/GetProperties( aNames );
                uno::Sequence< OUString > aSvcImplNames;
                if (aValues.hasElements())
                    aSvcImplNames = GetLangSvcList( aValues.getConstArray()[0] );

                LanguageType nLang = LANGUAGE_NONE;
                if (!aKeyText.isEmpty())
                    nLang = LanguageTag::convertToLanguageType( aKeyText );

                GetThesaurusDsp_Impl( false );  // don't set service list, it will be done below
                mxThesDsp->SetServiceList( LanguageTag::convertToLocale(nLang), aSvcImplNames );
            }
        }
        else
        {
            SAL_WARN( "linguistic", "notified for unexpected property" );
        }
    }
}


void LngSvcMgr::ImplCommit()
{
    // everything necessary should have already been done by 'SaveCfgSvcs'
    // called from within 'setConfiguredServices'.
    // Also this class usually exits only when the Office is being shutdown.
}


void LngSvcMgr::GetListenerHelper_Impl()
{
    if (!mxListenerHelper.is())
    {
        mxListenerHelper = new LngSvcMgrListenerHelper( *this, linguistic::GetDictionaryList() );
    }
}


void LngSvcMgr::GetSpellCheckerDsp_Impl( bool bSetSvcList )
{
    if (!mxSpellDsp.is())
    {
        mxSpellDsp = new SpellCheckerDispatcher( *this );
        if (bSetSvcList)
            SetCfgServiceLists( *mxSpellDsp );
    }
}


void LngSvcMgr::GetGrammarCheckerDsp_Impl( bool bSetSvcList  )
{
    if (mxGrammarDsp.is() || !SvtLinguConfig().HasGrammarChecker())
        return;

    //! since the grammar checking iterator needs to be a one instance service
    //! we need to create it the correct way!
    uno::Reference< linguistic2::XProofreadingIterator > xGCI;
    try
    {
        xGCI = linguistic2::ProofreadingIterator::create( comphelper::getProcessComponentContext() );
    }
    catch (const uno::Exception &)
    {
    }
    SAL_WARN_IF( !xGCI.is(), "linguistic", "instantiating grammar checking iterator failed" );

    if (xGCI.is())
    {
        mxGrammarDsp = dynamic_cast< GrammarCheckingIterator * >(xGCI.get());
        SAL_WARN_IF( mxGrammarDsp == nullptr, "linguistic", "failed to get implementation" );
        if (bSetSvcList && mxGrammarDsp.is())
            SetCfgServiceLists( *mxGrammarDsp );
    }
}


void LngSvcMgr::GetHyphenatorDsp_Impl( bool bSetSvcList  )
{
    if (!mxHyphDsp.is())
    {
        mxHyphDsp = new HyphenatorDispatcher( *this );
        if (bSetSvcList)
            SetCfgServiceLists( *mxHyphDsp );
    }
}


void LngSvcMgr::GetThesaurusDsp_Impl( bool bSetSvcList  )
{
    if (!mxThesDsp.is())
    {
        mxThesDsp = new ThesaurusDispatcher;
        if (bSetSvcList)
            SetCfgServiceLists( *mxThesDsp );
    }
}


void LngSvcMgr::GetAvailableSpellSvcs_Impl()
{
    if (pAvailSpellSvcs)
        return;

    pAvailSpellSvcs.emplace();

    uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );

    uno::Reference< container::XContentEnumerationAccess > xEnumAccess( xContext->getServiceManager(), uno::UNO_QUERY );
    uno::Reference< container::XEnumeration > xEnum;
    if (xEnumAccess.is())
        xEnum = xEnumAccess->createContentEnumeration( SN_SPELLCHECKER );

    if (!xEnum.is())
        return;

    while (xEnum->hasMoreElements())
    {
        uno::Any aCurrent = xEnum->nextElement();
        uno::Reference< lang::XSingleComponentFactory > xCompFactory;
        uno::Reference< lang::XSingleServiceFactory > xFactory;

        xCompFactory.set(aCurrent, css::uno::UNO_QUERY);
        if (!xCompFactory.is())
        {
            xFactory.set(aCurrent, css::uno::UNO_QUERY);
        }
        if ( xCompFactory.is() || xFactory.is() )
        {
            try
            {
                uno::Reference< linguistic2::XSpellChecker > xSvc( ( xCompFactory.is() ? xCompFactory->createInstanceWithContext( xContext ) : xFactory->createInstance() ), uno::UNO_QUERY_THROW );

                OUString            aImplName;
                std::vector< LanguageType >   aLanguages;
                uno::Reference< XServiceInfo > xInfo( xSvc, uno::UNO_QUERY );
                if (xInfo.is())
                    aImplName = xInfo->getImplementationName();
                SAL_WARN_IF( aImplName.isEmpty(), "linguistic", "empty implementation name" );
                uno::Sequence<lang::Locale> aLocaleSequence(xSvc->getLocales());
                aLanguages = LocaleSeqToLangVec( aLocaleSequence );

                pAvailSpellSvcs->push_back( SvcInfo( aImplName, std::move(aLanguages) ) );
            }
            catch (const uno::Exception &)
            {
                SAL_WARN( "linguistic", "createInstance failed" );
            }
        }
    }
}


void LngSvcMgr::GetAvailableGrammarSvcs_Impl()
{
    if (pAvailGrammarSvcs)
        return;

    pAvailGrammarSvcs.emplace();

    uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );

    uno::Reference< container::XContentEnumerationAccess > xEnumAccess( xContext->getServiceManager(), uno::UNO_QUERY );
    uno::Reference< container::XEnumeration > xEnum;
    if (xEnumAccess.is())
        xEnum = xEnumAccess->createContentEnumeration( SN_GRAMMARCHECKER );

    if (!xEnum.is())
        return;

    while (xEnum->hasMoreElements())
    {
        uno::Any aCurrent = xEnum->nextElement();
        uno::Reference< lang::XSingleComponentFactory > xCompFactory;
        uno::Reference< lang::XSingleServiceFactory > xFactory;

        xCompFactory.set(aCurrent, css::uno::UNO_QUERY);
        if (!xCompFactory.is())
        {
            xFactory.set(aCurrent, css::uno::UNO_QUERY);
        }
        if ( xCompFactory.is() || xFactory.is() )
        {
            try
            {
                uno::Reference< linguistic2::XProofreader > xSvc(
                    xCompFactory.is()
                        ? xCompFactory->createInstanceWithContext(xContext)
                        : xFactory->createInstance(),
                    uno::UNO_QUERY_THROW);

                OUString            aImplName;
                std::vector< LanguageType >    aLanguages;
                uno::Reference< XServiceInfo > xInfo( xSvc, uno::UNO_QUERY );
                if (xInfo.is())
                    aImplName = xInfo->getImplementationName();
                SAL_WARN_IF( aImplName.isEmpty(), "linguistic", "empty implementation name" );
                uno::Sequence<lang::Locale> aLocaleSequence(xSvc->getLocales());
                aLanguages = LocaleSeqToLangVec( aLocaleSequence );

                pAvailGrammarSvcs->push_back( SvcInfo( aImplName, std::move(aLanguages) ) );
            }
            catch (const uno::Exception &)
            {
                SAL_WARN( "linguistic", "createInstance failed" );
            }
        }

    }
}


void LngSvcMgr::GetAvailableHyphSvcs_Impl()
{
    if (pAvailHyphSvcs)
        return;

    pAvailHyphSvcs.emplace();
    uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );

    uno::Reference< container::XContentEnumerationAccess > xEnumAccess( xContext->getServiceManager(), uno::UNO_QUERY );
    uno::Reference< container::XEnumeration > xEnum;
    if (xEnumAccess.is())
        xEnum = xEnumAccess->createContentEnumeration( SN_HYPHENATOR );

    if (!xEnum.is())
        return;

    while (xEnum->hasMoreElements())
    {
        uno::Any aCurrent = xEnum->nextElement();
        uno::Reference< lang::XSingleComponentFactory > xCompFactory;
        uno::Reference< lang::XSingleServiceFactory > xFactory;

        xCompFactory.set(aCurrent, css::uno::UNO_QUERY);
        if (!xCompFactory.is())
        {
            xFactory.set(aCurrent, css::uno::UNO_QUERY);
        }
        if ( xCompFactory.is() || xFactory.is() )
        {
            try
            {
                uno::Reference< linguistic2::XHyphenator > xSvc( ( xCompFactory.is() ? xCompFactory->createInstanceWithContext( xContext ) : xFactory->createInstance() ), uno::UNO_QUERY_THROW );
                OUString            aImplName;
                std::vector< LanguageType >    aLanguages;
                uno::Reference< XServiceInfo > xInfo( xSvc, uno::UNO_QUERY );
                if (xInfo.is())
                    aImplName = xInfo->getImplementationName();
                SAL_WARN_IF( aImplName.isEmpty(), "linguistic", "empty implementation name" );
                uno::Sequence<lang::Locale> aLocaleSequence(xSvc->getLocales());
                aLanguages = LocaleSeqToLangVec( aLocaleSequence );
                pAvailHyphSvcs->push_back( SvcInfo( aImplName, std::move(aLanguages) ) );
            }
            catch (const uno::Exception &)
            {
                SAL_WARN( "linguistic", "createInstance failed" );
            }
        }
    }
}


void LngSvcMgr::GetAvailableThesSvcs_Impl()
{
    if (pAvailThesSvcs)
        return;

    pAvailThesSvcs.emplace();

    uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );

    uno::Reference< container::XContentEnumerationAccess > xEnumAccess( xContext->getServiceManager(), uno::UNO_QUERY );
    uno::Reference< container::XEnumeration > xEnum;
    if (xEnumAccess.is())
        xEnum = xEnumAccess->createContentEnumeration( SN_THESAURUS );

    if (!xEnum.is())
        return;

    while (xEnum->hasMoreElements())
    {
        uno::Any aCurrent = xEnum->nextElement();
        uno::Reference< lang::XSingleComponentFactory > xCompFactory;
        uno::Reference< lang::XSingleServiceFactory > xFactory;

        xCompFactory.set(aCurrent, css::uno::UNO_QUERY);
        if (!xCompFactory.is())
        {
            xFactory.set(aCurrent, css::uno::UNO_QUERY);
        }
        if ( xCompFactory.is() || xFactory.is() )
        {
            try
            {
                uno::Reference< linguistic2::XThesaurus > xSvc( ( xCompFactory.is() ? xCompFactory->createInstanceWithContext( xContext ) : xFactory->createInstance() ), uno::UNO_QUERY_THROW );

                OUString            aImplName;
                std::vector< LanguageType >    aLanguages;
                uno::Reference< XServiceInfo > xInfo( xSvc, uno::UNO_QUERY );
                if (xInfo.is())
                    aImplName = xInfo->getImplementationName();
                SAL_WARN_IF( aImplName.isEmpty(), "linguistic", "empty implementation name" );
                uno::Sequence<lang::Locale> aLocaleSequence(xSvc->getLocales());
                aLanguages = LocaleSeqToLangVec( aLocaleSequence );

                pAvailThesSvcs->push_back( SvcInfo( aImplName, std::move(aLanguages) ) );
            }
            catch (const uno::Exception &)
            {
               SAL_WARN( "linguistic", "createInstance failed" );
            }
        }
    }
}


void LngSvcMgr::SetCfgServiceLists( SpellCheckerDispatcher &rSpellDsp )
{
    SAL_INFO( "linguistic", "linguistic: LngSvcMgr::SetCfgServiceLists - Spell" );

    OUString aNode(u"ServiceManager/SpellCheckerList"_ustr);
    uno::Sequence< OUString > aNames( /*aCfg.*/GetNodeNames( aNode ) );

    // append path prefix need for 'GetProperties' call below
    OUString aPrefix = aNode + "/";
    for (OUString & name : asNonConstRange(aNames))
    {
        name = aPrefix + name;
    }

    const uno::Sequence< uno::Any > aValues( /*aCfg.*/GetProperties( aNames ) );
    if (!(aNames.hasElements()  &&  aNames.getLength() == aValues.getLength()))
        return;

    const OUString *pNames = aNames.getConstArray();
    for (const uno::Any& rValue : aValues)
    {
        uno::Sequence< OUString > aSvcImplNames;
        if (rValue >>= aSvcImplNames)
        {
            OUString aLocaleStr( *pNames++ );
            sal_Int32 nSeparatorPos = aLocaleStr.lastIndexOf( '/' );
            aLocaleStr = aLocaleStr.copy( nSeparatorPos + 1 );
            rSpellDsp.SetServiceList( LanguageTag::convertToLocale(aLocaleStr), aSvcImplNames );
        }
    }
}


void LngSvcMgr::SetCfgServiceLists( GrammarCheckingIterator &rGrammarDsp )
{
    SAL_INFO( "linguistic", "linguistic: LngSvcMgr::SetCfgServiceLists - Grammar" );

    OUString aNode(u"ServiceManager/GrammarCheckerList"_ustr);
    uno::Sequence< OUString > aNames( /*aCfg.*/GetNodeNames( aNode ) );

    // append path prefix need for 'GetProperties' call below
    OUString aPrefix = aNode  + "/";
    for (OUString & name : asNonConstRange(aNames))
    {
        name = aPrefix + name;
    }

    const uno::Sequence< uno::Any > aValues( /*aCfg.*/GetProperties( aNames ) );
    if (!(aNames.hasElements()  &&  aNames.getLength() == aValues.getLength()))
        return;

    const OUString *pNames = aNames.getConstArray();
    for (const uno::Any& rValue : aValues)
    {
        uno::Sequence< OUString > aSvcImplNames;
        if (rValue >>= aSvcImplNames)
        {
            // there should only be one grammar checker in use per language...
            if (aSvcImplNames.getLength() > 1)
                aSvcImplNames.realloc(1);

            OUString aLocaleStr( *pNames++ );
            sal_Int32 nSeparatorPos = aLocaleStr.lastIndexOf( '/' );
            aLocaleStr = aLocaleStr.copy( nSeparatorPos + 1 );
            rGrammarDsp.SetServiceList( LanguageTag::convertToLocale(aLocaleStr), aSvcImplNames );
        }
    }
}


void LngSvcMgr::SetCfgServiceLists( HyphenatorDispatcher &rHyphDsp )
{
    SAL_INFO( "linguistic", "linguistic: LngSvcMgr::SetCfgServiceLists - Hyph" );

    OUString aNode(u"ServiceManager/HyphenatorList"_ustr);
    uno::Sequence< OUString > aNames( /*aCfg.*/GetNodeNames( aNode ) );

    // append path prefix need for 'GetProperties' call below
    OUString aPrefix = aNode + "/";
    for (OUString & name : asNonConstRange(aNames))
    {
        name = aPrefix + name;
    }

    const uno::Sequence< uno::Any > aValues( /*aCfg.*/GetProperties( aNames ) );
    if (!(aNames.hasElements()  &&  aNames.getLength() == aValues.getLength()))
        return;

    const OUString *pNames = aNames.getConstArray();
    for (const uno::Any& rValue : aValues)
    {
        uno::Sequence< OUString > aSvcImplNames;
        if (rValue >>= aSvcImplNames)
        {
            // there should only be one hyphenator in use per language...
            if (aSvcImplNames.getLength() > 1)
                aSvcImplNames.realloc(1);

            OUString aLocaleStr( *pNames++ );
            sal_Int32 nSeparatorPos = aLocaleStr.lastIndexOf( '/' );
            aLocaleStr = aLocaleStr.copy( nSeparatorPos + 1 );
            rHyphDsp.SetServiceList( LanguageTag::convertToLocale(aLocaleStr), aSvcImplNames );
        }
    }
}


void LngSvcMgr::SetCfgServiceLists( ThesaurusDispatcher &rThesDsp )
{
    SAL_INFO( "linguistic", "linguistic: LngSvcMgr::SetCfgServiceLists - Thes" );

    OUString aNode(u"ServiceManager/ThesaurusList"_ustr);
    uno::Sequence< OUString > aNames( /*aCfg.*/GetNodeNames( aNode ) );

    // append path prefix need for 'GetProperties' call below
    OUString aPrefix = aNode + "/";
    for (OUString & name : asNonConstRange(aNames))
    {
        name = aPrefix + name;
    }

    const uno::Sequence< uno::Any > aValues( /*aCfg.*/GetProperties( aNames ) );
    if (!(aNames.hasElements()  &&  aNames.getLength() == aValues.getLength()))
        return;

    const OUString *pNames = aNames.getConstArray();
    for (const uno::Any& rValue : aValues)
    {
        uno::Sequence< OUString > aSvcImplNames;
        if (rValue >>= aSvcImplNames)
        {
            OUString aLocaleStr( *pNames++ );
            sal_Int32 nSeparatorPos = aLocaleStr.lastIndexOf( '/' );
            aLocaleStr = aLocaleStr.copy( nSeparatorPos + 1 );
            rThesDsp.SetServiceList( LanguageTag::convertToLocale(aLocaleStr), aSvcImplNames );
        }
    }
}


uno::Reference< linguistic2::XSpellChecker > SAL_CALL
    LngSvcMgr::getSpellChecker()
{
    osl::MutexGuard aGuard( GetLinguMutex() );
#if OSL_DEBUG_LEVEL > 0
    getAvailableLocales(SN_SPELLCHECKER);
#endif

    uno::Reference< linguistic2::XSpellChecker > xRes;
    if (!bDisposing)
    {
        if (!mxSpellDsp.is())
            GetSpellCheckerDsp_Impl();
        xRes = mxSpellDsp.get();
    }
    return xRes;
}


uno::Reference< linguistic2::XHyphenator > SAL_CALL
    LngSvcMgr::getHyphenator()
{
    osl::MutexGuard aGuard( GetLinguMutex() );
#if OSL_DEBUG_LEVEL > 0
    getAvailableLocales(SN_HYPHENATOR);
#endif
    uno::Reference< linguistic2::XHyphenator >   xRes;
    if (!bDisposing)
    {
        if (!mxHyphDsp.is())
            GetHyphenatorDsp_Impl();
        xRes = mxHyphDsp.get();
    }
    return xRes;
}


uno::Reference< linguistic2::XThesaurus > SAL_CALL
    LngSvcMgr::getThesaurus()
{
    osl::MutexGuard aGuard( GetLinguMutex() );
#if OSL_DEBUG_LEVEL > 0
    getAvailableLocales(SN_THESAURUS);
#endif
    uno::Reference< linguistic2::XThesaurus >    xRes;
    if (!bDisposing)
    {
        if (!mxThesDsp.is())
            GetThesaurusDsp_Impl();
        xRes = mxThesDsp.get();
    }
    return xRes;
}


sal_Bool SAL_CALL
    LngSvcMgr::addLinguServiceManagerListener(
            const uno::Reference< lang::XEventListener >& xListener )
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    if (bDisposing || !xListener.is())
        return false;

    if (!mxListenerHelper.is())
        GetListenerHelper_Impl();
    mxListenerHelper->AddLngSvcMgrListener( xListener );
    return true;
}


sal_Bool SAL_CALL
    LngSvcMgr::removeLinguServiceManagerListener(
            const uno::Reference< lang::XEventListener >& xListener )
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    if (bDisposing || !xListener.is())
        return false;

    DBG_ASSERT( mxListenerHelper.is(), "listener removed without being added" );
    if (!mxListenerHelper.is())
        GetListenerHelper_Impl();
    mxListenerHelper->RemoveLngSvcMgrListener( xListener );
    return true;
}


uno::Sequence< OUString > SAL_CALL
    LngSvcMgr::getAvailableServices(
            const OUString& rServiceName,
            const lang::Locale& rLocale )
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    uno::Sequence< OUString > aRes;
    const SvcInfoArray *pInfoArray = nullptr;

    if (rServiceName == SN_SPELLCHECKER)
    {
        GetAvailableSpellSvcs_Impl();
        pInfoArray = &*pAvailSpellSvcs;
    }
    else if (rServiceName == SN_GRAMMARCHECKER)
    {
        GetAvailableGrammarSvcs_Impl();
        pInfoArray = &*pAvailGrammarSvcs;
    }
    else if (rServiceName == SN_HYPHENATOR)
    {
        GetAvailableHyphSvcs_Impl();
        pInfoArray = &*pAvailHyphSvcs;
    }
    else if (rServiceName == SN_THESAURUS)
    {
        GetAvailableThesSvcs_Impl();
        pInfoArray = &*pAvailThesSvcs;
    }

    if (pInfoArray)
    {
        std::vector<OUString> aVec;
        aVec.reserve(pInfoArray->size());

        LanguageType nLanguage = LinguLocaleToLanguage( rLocale );
        for (const auto& rInfo : *pInfoArray)
        {
            if (LinguIsUnspecified( nLanguage )
                || rInfo.HasLanguage( nLanguage ))
            {
                aVec.push_back(rInfo.aSvcImplName);
            }
        }

        aRes = comphelper::containerToSequence(aVec);
    }

    return aRes;
}


uno::Sequence< lang::Locale > SAL_CALL
    LngSvcMgr::getAvailableLocales(
            const OUString& rServiceName )
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    uno::Sequence< lang::Locale > aRes;

    uno::Sequence< lang::Locale >  *pAvailLocales     = nullptr;
    if (rServiceName == SN_SPELLCHECKER)
        pAvailLocales       = &aAvailSpellLocales;
    else if (rServiceName == SN_GRAMMARCHECKER)
        pAvailLocales       = &aAvailGrammarLocales;
    else if (rServiceName == SN_HYPHENATOR)
        pAvailLocales       = &aAvailHyphLocales;
    else if (rServiceName == SN_THESAURUS)
        pAvailLocales       = &aAvailThesLocales;

    // Nowadays (with OOo lingu in SO) we want to know immediately about
    // new downloaded dictionaries and have them ready right away if the Tools/Options...
    // is used to activate them. Thus we can not rely anymore on buffered data.
    if (pAvailLocales)
    {
        *pAvailLocales = GetAvailLocales(getAvailableServices(rServiceName, lang::Locale()));
        aRes = *pAvailLocales;
    }

    return aRes;
}

static bool IsEqSvcList( const uno::Sequence< OUString > &rList1,
                         const uno::Sequence< OUString > &rList2 )
{
    // returns true if both sequences are equal
    return rList1.getLength() == rList2.getLength()
        && std::equal(rList1.begin(), rList1.end(), rList2.begin(), rList2.end());
}


void SAL_CALL
    LngSvcMgr::setConfiguredServices(
            const OUString& rServiceName,
            const lang::Locale& rLocale,
            const uno::Sequence< OUString >& rServiceImplNames )
{
    SAL_INFO( "linguistic", "linguistic: LngSvcMgr::setConfiguredServices" );

    osl::MutexGuard aGuard( GetLinguMutex() );

    LanguageType nLanguage = LinguLocaleToLanguage( rLocale );
    if (LinguIsUnspecified( nLanguage))
        return;

    if (rServiceName == SN_SPELLCHECKER)
    {
        if (!mxSpellDsp.is())
            GetSpellCheckerDsp_Impl();
        bool bChanged = !IsEqSvcList( rServiceImplNames,
                                      mxSpellDsp->GetServiceList( rLocale ) );
        if (bChanged)
        {
            mxSpellDsp->SetServiceList( rLocale, rServiceImplNames );
            SaveCfgSvcs( SN_SPELLCHECKER );

            if (mxListenerHelper)
                mxListenerHelper->AddLngSvcEvt(
                        linguistic2::LinguServiceEventFlags::SPELL_CORRECT_WORDS_AGAIN |
                        linguistic2::LinguServiceEventFlags::SPELL_WRONG_WORDS_AGAIN );
        }
    }
    else if (rServiceName == SN_GRAMMARCHECKER)
    {
        if (!mxGrammarDsp.is())
            GetGrammarCheckerDsp_Impl();
        if (!mxGrammarDsp) // e.g., when !SvtLinguConfig().HasGrammarChecker()
            return;
        bool bChanged = !IsEqSvcList( rServiceImplNames,
                                      mxGrammarDsp->GetServiceList( rLocale ) );
        if (bChanged)
        {
            mxGrammarDsp->SetServiceList( rLocale, rServiceImplNames );
            SaveCfgSvcs( SN_GRAMMARCHECKER );

            if (mxListenerHelper)
                mxListenerHelper->AddLngSvcEvt(
                        linguistic2::LinguServiceEventFlags::PROOFREAD_AGAIN );
        }
    }
    else if (rServiceName == SN_HYPHENATOR)
    {
        if (!mxHyphDsp.is())
            GetHyphenatorDsp_Impl();
        bool bChanged = !IsEqSvcList( rServiceImplNames,
                                      mxHyphDsp->GetServiceList( rLocale ) );
        if (bChanged)
        {
            mxHyphDsp->SetServiceList( rLocale, rServiceImplNames );
            SaveCfgSvcs( SN_HYPHENATOR );

            if (mxListenerHelper)
                mxListenerHelper->AddLngSvcEvt(
                        linguistic2::LinguServiceEventFlags::HYPHENATE_AGAIN );
        }
    }
    else if (rServiceName == SN_THESAURUS)
    {
        if (!mxThesDsp.is())
            GetThesaurusDsp_Impl();
        bool bChanged = !IsEqSvcList( rServiceImplNames,
                                      mxThesDsp->GetServiceList( rLocale ) );
        if (bChanged)
        {
            mxThesDsp->SetServiceList( rLocale, rServiceImplNames );
            SaveCfgSvcs( SN_THESAURUS );
        }
    }
}


bool LngSvcMgr::SaveCfgSvcs( std::u16string_view rServiceName )
{
    SAL_INFO( "linguistic", "linguistic: LngSvcMgr::SaveCfgSvcs" );

    bool bRes = false;

    LinguDispatcher *pDsp = nullptr;
    uno::Sequence< lang::Locale > aLocales;

    if (rServiceName == SN_SPELLCHECKER)
    {
        if (!mxSpellDsp)
            GetSpellCheckerDsp_Impl();
        pDsp = mxSpellDsp.get();
        aLocales = getAvailableLocales( SN_SPELLCHECKER );
    }
    else if (rServiceName == SN_GRAMMARCHECKER)
    {
        if (!mxGrammarDsp.is())
            GetGrammarCheckerDsp_Impl();
        pDsp = mxGrammarDsp.get();
        aLocales = getAvailableLocales( SN_GRAMMARCHECKER );
    }
    else if (rServiceName == SN_HYPHENATOR)
    {
        if (!mxHyphDsp.is())
            GetHyphenatorDsp_Impl();
        pDsp = mxHyphDsp.get();
        aLocales = getAvailableLocales( SN_HYPHENATOR );
    }
    else if (rServiceName == SN_THESAURUS)
    {
        if (!mxThesDsp.is())
            GetThesaurusDsp_Impl();
        pDsp = mxThesDsp.get();
        aLocales = getAvailableLocales( SN_THESAURUS );
    }

    if (pDsp  &&  aLocales.hasElements())
    {
        uno::Sequence< beans::PropertyValue > aValues( aLocales.getLength() );
        beans::PropertyValue *pValue = aValues.getArray();

        // get node name to be used
        const char *pNodeName = nullptr;
        if (pDsp == mxSpellDsp.get())
            pNodeName = "ServiceManager/SpellCheckerList";
        else if (pDsp == mxGrammarDsp.get())
            pNodeName = "ServiceManager/GrammarCheckerList";
        else if (pDsp == mxHyphDsp.get())
            pNodeName = "ServiceManager/HyphenatorList";
        else if (pDsp == mxThesDsp.get())
            pNodeName = "ServiceManager/ThesaurusList";
        else
        {
            SAL_WARN( "linguistic", "node name missing" );
        }
        OUString aNodeName( OUString::createFromAscii(pNodeName) );

        for (const lang::Locale& rLocale : aLocales)
        {
            uno::Sequence< OUString > aSvcImplNames = pDsp->GetServiceList( rLocale );

            // build value to be written back to configuration
            uno::Any aCfgAny;
            if ((pDsp == mxHyphDsp.get() || pDsp == mxGrammarDsp.get()) && aSvcImplNames.getLength() > 1)
                aSvcImplNames.realloc(1);   // there should be only one entry for hyphenators or grammar checkers (because they are not chained)
            aCfgAny <<= aSvcImplNames;
            DBG_ASSERT( aCfgAny.hasValue(), "missing value for 'Any' type" );

            OUString aCfgLocaleStr( LanguageTag::convertToBcp47( rLocale));
            pValue->Value = aCfgAny;
            pValue->Name  = aNodeName + "/" + aCfgLocaleStr;
            pValue++;
        }
        {
        SAL_INFO( "linguistic", "linguistic: LngSvcMgr::SaveCfgSvcs - ReplaceSetProperties" );
        // change, add new or replace existing entries.
        bRes |= /*aCfg.*/ReplaceSetProperties( aNodeName, aValues );
        }
    }

    return bRes;
}


static uno::Sequence< OUString > GetLangSvcList( const uno::Any &rVal )
{
    uno::Sequence< OUString > aRes;

    if (rVal.hasValue())
    {
        rVal >>= aRes;
#if OSL_DEBUG_LEVEL > 0
        for (const OUString& rSvcName : aRes)
        {
            SAL_WARN_IF( rSvcName.isEmpty(), "linguistic", "service impl-name missing" );
        }
#endif
    }

    return aRes;
}


static uno::Sequence< OUString > GetLangSvc( const uno::Any &rVal )
{
    uno::Sequence< OUString > aRes;
    if (!rVal.hasValue())
        return aRes;

    // allowing for a sequence here as well (even though it should only
    // be a string) makes coding easier in other places since one needs
    // not make a special case for writing a string only and not a
    // sequence of strings.
    if (rVal >>= aRes)
    {
        // but only the first string should be used.
        if (aRes.getLength() > 1)
            aRes.realloc(1);
    }
    else
    {
        OUString aImplName;
        if ((rVal >>= aImplName) && !aImplName.isEmpty())
        {
            aRes.realloc(1);
            aRes.getArray()[0] = aImplName;
        }
        else
        {
            SAL_WARN( "linguistic", "GetLangSvc: unexpected type encountered" );
        }
    }

    return aRes;
}


uno::Sequence< OUString > SAL_CALL
    LngSvcMgr::getConfiguredServices(
            const OUString& rServiceName,
            const lang::Locale& rLocale )
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    uno::Sequence< OUString > aSvcImplNames;

    OUString aCfgLocale( LanguageTag::convertToBcp47( rLocale) );

    uno::Sequence< uno::Any > aValues;
    uno::Sequence< OUString > aNames( 1 );
    OUString *pNames = aNames.getArray();
    if ( rServiceName == SN_SPELLCHECKER )
    {
        OUString aNode( u"ServiceManager/SpellCheckerList"_ustr);
        const uno::Sequence< OUString > aNodeEntries( GetNodeNames( aNode ) );
        if (lcl_SeqHasString( aNodeEntries, aCfgLocale ))
        {
            pNames[0] = aNode + "/" + aCfgLocale;
            aValues = /*aCfg.*/GetProperties( aNames );
            if (aValues.hasElements())
                aSvcImplNames = GetLangSvcList( aValues.getConstArray()[0] );
        }
    }
    else if ( rServiceName == SN_GRAMMARCHECKER )
    {
        OUString aNode( u"ServiceManager/GrammarCheckerList"_ustr);
        const uno::Sequence< OUString > aNodeEntries( GetNodeNames( aNode ) );
        if (lcl_SeqHasString( aNodeEntries, aCfgLocale ))
        {
            pNames[0] = aNode + "/" + aCfgLocale;
            aValues = /*aCfg.*/GetProperties( aNames );
            if (aValues.hasElements())
                aSvcImplNames = GetLangSvc( aValues.getConstArray()[0] );
        }
    }
    else if ( rServiceName == SN_HYPHENATOR )
    {
        OUString aNode( u"ServiceManager/HyphenatorList"_ustr);
        const uno::Sequence< OUString > aNodeEntries( GetNodeNames( aNode ) );
        if (lcl_SeqHasString( aNodeEntries, aCfgLocale ))
        {
            pNames[0] = aNode + "/" + aCfgLocale;
            aValues = /*aCfg.*/GetProperties( aNames );
            if (aValues.hasElements())
                aSvcImplNames = GetLangSvc( aValues.getConstArray()[0] );
        }
    }
    else if ( rServiceName == SN_THESAURUS )
    {
        OUString aNode( u"ServiceManager/ThesaurusList"_ustr);
        const uno::Sequence< OUString > aNodeEntries( GetNodeNames( aNode ) );
        if (lcl_SeqHasString( aNodeEntries, aCfgLocale ))
        {
            pNames[0] = aNode + "/" + aCfgLocale;
            aValues = /*aCfg.*/GetProperties( aNames );
            if (aValues.hasElements())
                aSvcImplNames = GetLangSvcList( aValues.getConstArray()[0] );
        }
    }

    return aSvcImplNames;
}


void SAL_CALL
    LngSvcMgr::dispose()
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    if (!bDisposing)
    {
        bDisposing = true;

        // require listeners to release this object
        lang::EventObject aEvtObj( static_cast<XLinguServiceManager*>(this) );
        aEvtListeners.disposeAndClear( aEvtObj );

        if (mxListenerHelper.is())
            mxListenerHelper->DisposeAndClear( aEvtObj );
    }
}


void SAL_CALL
    LngSvcMgr::addEventListener(
            const uno::Reference< lang::XEventListener >& xListener )
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    if (!bDisposing  &&  xListener.is())
    {
        aEvtListeners.addInterface( xListener );
    }
}


void SAL_CALL
    LngSvcMgr::removeEventListener(
            const uno::Reference< lang::XEventListener >& xListener )
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    if (xListener.is())
    {
        aEvtListeners.removeInterface( xListener );
    }
}


bool LngSvcMgr::AddLngSvcEvtBroadcaster(
            const uno::Reference< linguistic2::XLinguServiceEventBroadcaster > &rxBroadcaster )
{
    if (!rxBroadcaster.is())
        return false;
    if (!mxListenerHelper.is())
        GetListenerHelper_Impl();
    mxListenerHelper->AddLngSvcEvtBroadcaster( rxBroadcaster );
    return true;
}


OUString SAL_CALL
    LngSvcMgr::getImplementationName()
{
    return u"com.sun.star.lingu2.LngSvcMgr"_ustr;
}


sal_Bool SAL_CALL
    LngSvcMgr::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}


uno::Sequence< OUString > SAL_CALL
    LngSvcMgr::getSupportedServiceNames()
{
    return { u"com.sun.star.linguistic2.LinguServiceManager"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
linguistic_LngSvcMgr_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new LngSvcMgr());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
