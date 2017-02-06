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

#include <com/sun/star/deployment/DeploymentException.hpp>
#include <com/sun/star/deployment/ExtensionManager.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/linguistic2/XSupportedLocales.hpp>
#include <com/sun/star/linguistic2/DictionaryListEventFlags.hpp>
#include <com/sun/star/linguistic2/LinguServiceEventFlags.hpp>
#include <com/sun/star/linguistic2/ProofreadingIterator.hpp>

#include <unotools/lingucfg.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/processfactory.hxx>
#include <i18nlangtag/lang.h>
#include <i18nlangtag/languagetag.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <o3tl/make_unique.hxx>

#include "lngsvcmgr.hxx"
#include "lngopt.hxx"
#include "lngreg.hxx"
#include "linguistic/misc.hxx"
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
    bool bRes = false;

    sal_Int32 nLen = rSeq.getLength();
    if (nLen == 0 || rText.isEmpty())
        return bRes;

    const OUString *pSeq = rSeq.getConstArray();
    for (sal_Int32 i = 0;  i < nLen  &&  !bRes;  ++i)
    {
        if (rText == pSeq[i])
            bRes = true;
    }
    return bRes;
}


static uno::Sequence< lang::Locale > GetAvailLocales(
        const uno::Sequence< OUString > &rSvcImplNames )
{
    uno::Sequence< lang::Locale > aRes;

    uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );
    sal_Int32 nNames = rSvcImplNames.getLength();
    if( nNames )
    {
        std::set< LanguageType > aLanguages;

        //! since we're going to create one-instance services we have to
        //! supply their arguments even if we would not need them here...
        uno::Sequence< uno::Any > aArgs(2);
        aArgs.getArray()[0] <<= GetLinguProperties();

        // check all services for the supported languages and new
        // languages to the result
        const OUString *pImplNames = rSvcImplNames.getConstArray();
        sal_Int32 i;

        for (i = 0;  i < nNames;  ++i)
        {
            uno::Reference< linguistic2::XSupportedLocales > xSuppLoc;
            try
            {
                xSuppLoc.set( xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                                 pImplNames[i], aArgs, xContext ),
                              uno::UNO_QUERY );
            }
            catch (uno::Exception &)
            {
                SAL_WARN( "linguistic", "createInstanceWithArguments failed" );
            }

            if (xSuppLoc.is())
            {
                uno::Sequence< lang::Locale > aLoc( xSuppLoc->getLocales() );
                sal_Int32 nLoc = aLoc.getLength();
                for (sal_Int32 k = 0;  k < nLoc;  ++k)
                {
                    const lang::Locale *pLoc = aLoc.getConstArray();
                    LanguageType nLang = LinguLocaleToLanguage( pLoc[k] );

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
        sal_Int32 nLanguages = static_cast< sal_Int32 >(aLanguages.size());
        aRes.realloc( nLanguages );
        lang::Locale *pRes = aRes.getArray();
        std::set< LanguageType >::const_iterator aIt( aLanguages.begin() );
        for (i = 0;  aIt != aLanguages.end();  ++aIt, ++i)
        {
            LanguageType nLang = *aIt;
            pRes[i] = LanguageTag::convertToLocale( nLang );
        }
    }

    return aRes;
}


struct SvcInfo
{
    const OUString                  aSvcImplName;
    const uno::Sequence< sal_Int16 >    aSuppLanguages;

    SvcInfo( const OUString &rSvcImplName,
             const uno::Sequence< sal_Int16 >  &rSuppLanguages ) :
        aSvcImplName    (rSvcImplName),
        aSuppLanguages  (rSuppLanguages)
    {
    }

    bool    HasLanguage( sal_Int16 nLanguage ) const;
};


bool SvcInfo::HasLanguage( sal_Int16 nLanguage ) const
{
    sal_Int32 nCnt = aSuppLanguages.getLength();
    const sal_Int16 *pLang = aSuppLanguages.getConstArray();
    sal_Int32 i;

    for ( i = 0;  i < nCnt;  ++i)
    {
        if (nLanguage == pLang[i])
            break;
    }
    return i < nCnt;
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
        const uno::Reference< linguistic2::XSearchableDictionaryList > &rxDicList );

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

    inline  bool    AddLngSvcMgrListener(
                        const uno::Reference< lang::XEventListener >& rxListener );
    inline  bool    RemoveLngSvcMgrListener(
                        const uno::Reference< lang::XEventListener >& rxListener );
    void    DisposeAndClear( const lang::EventObject &rEvtObj );
    bool    AddLngSvcEvtBroadcaster(
                        const uno::Reference< linguistic2::XLinguServiceEventBroadcaster > &rxBroadcaster );
    bool    RemoveLngSvcEvtBroadcaster(
                        const uno::Reference< linguistic2::XLinguServiceEventBroadcaster > &rxBroadcaster );

    void    AddLngSvcEvt( sal_Int16 nLngSvcEvt );
};


LngSvcMgrListenerHelper::LngSvcMgrListenerHelper(
        LngSvcMgr &rLngSvcMgr,
        const uno::Reference< linguistic2::XSearchableDictionaryList > &rxDicList  ) :
    rMyManager              ( rLngSvcMgr ),
    aLngSvcMgrListeners     ( GetLinguMutex() ),
    aLngSvcEvtBroadcasters  ( GetLinguMutex() ),
    xDicList                ( rxDicList )
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
    sal_Int16 nSpellCorrectFlags =
            linguistic2::DictionaryListEventFlags::ADD_NEG_ENTRY        |
            linguistic2::DictionaryListEventFlags::DEL_POS_ENTRY        |
            linguistic2::DictionaryListEventFlags::ACTIVATE_NEG_DIC |
            linguistic2::DictionaryListEventFlags::DEACTIVATE_POS_DIC;
    if (0 != (nDlEvt & nSpellCorrectFlags))
        nLngSvcEvt |= linguistic2::LinguServiceEventFlags::SPELL_CORRECT_WORDS_AGAIN;

    sal_Int16 nSpellWrongFlags =
            linguistic2::DictionaryListEventFlags::ADD_POS_ENTRY        |
            linguistic2::DictionaryListEventFlags::DEL_NEG_ENTRY        |
            linguistic2::DictionaryListEventFlags::ACTIVATE_POS_DIC |
            linguistic2::DictionaryListEventFlags::DEACTIVATE_NEG_DIC;
    if (0 != (nDlEvt & nSpellWrongFlags))
        nLngSvcEvt |= linguistic2::LinguServiceEventFlags::SPELL_WRONG_WORDS_AGAIN;

    sal_Int16 nHyphenateFlags =
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


inline bool LngSvcMgrListenerHelper::AddLngSvcMgrListener(
        const uno::Reference< lang::XEventListener >& rxListener )
{
    aLngSvcMgrListeners.addInterface( rxListener );
    return true;
}


inline bool LngSvcMgrListenerHelper::RemoveLngSvcMgrListener(
        const uno::Reference< lang::XEventListener >& rxListener )
{
    aLngSvcMgrListeners.removeInterface( rxListener );
    return true;
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


bool LngSvcMgrListenerHelper::AddLngSvcEvtBroadcaster(
        const uno::Reference< linguistic2::XLinguServiceEventBroadcaster > &rxBroadcaster )
{
    bool bRes = false;
    if (rxBroadcaster.is())
    {
        aLngSvcEvtBroadcasters.addInterface( rxBroadcaster );
        rxBroadcaster->addLinguServiceEventListener(
                static_cast<linguistic2::XLinguServiceEventListener *>(this) );
    }
    return bRes;
}


bool LngSvcMgrListenerHelper::RemoveLngSvcEvtBroadcaster(
        const uno::Reference< linguistic2::XLinguServiceEventBroadcaster > &rxBroadcaster )
{
    bool bRes = false;
    if (rxBroadcaster.is())
    {
        aLngSvcEvtBroadcasters.removeInterface( rxBroadcaster );
        rxBroadcaster->removeLinguServiceEventListener(
                static_cast<linguistic2::XLinguServiceEventListener *>(this) );
    }
    return bRes;
}


LngSvcMgr::LngSvcMgr()
    : utl::ConfigItem("Office.Linguistic")
    , aEvtListeners(GetLinguMutex())
{
    bDisposing = false;

    pAvailSpellSvcs     = nullptr;
    pAvailGrammarSvcs   = nullptr;
    pAvailHyphSvcs      = nullptr;
    pAvailThesSvcs      = nullptr;

    // request notify events when properties (i.e. something in the subtree) changes
    uno::Sequence< OUString > aNames(4);
    OUString *pNames = aNames.getArray();
    pNames[0] = "ServiceManager/SpellCheckerList";
    pNames[1] = "ServiceManager/GrammarCheckerList";
    pNames[2] = "ServiceManager/HyphenatorList";
    pNames[3] = "ServiceManager/ThesaurusList";
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
    {
        osl::MutexGuard aGuard(GetLinguMutex());
        //assume that if an extension has been added/removed that
        //it might be a dictionary extension, so drop our cache

        clearSvcInfoArray(pAvailSpellSvcs);
        clearSvcInfoArray(pAvailGrammarSvcs);
        clearSvcInfoArray(pAvailHyphSvcs);
        clearSvcInfoArray(pAvailThesSvcs);
    }

    {
        SolarMutexGuard aGuard;
        //schedule in an update to execute in the main thread
        aUpdateIdle.Start();
    }
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

    if (xMB.is())
    {
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
}

void LngSvcMgr::disposing(const lang::EventObject&)
{
    stopListening();
}

void LngSvcMgr::clearSvcInfoArray(SvcInfoArray* &rpInfo)
{
    delete rpInfo;
    rpInfo = nullptr;
}

LngSvcMgr::~LngSvcMgr()
{
    stopListening();

    // memory for pSpellDsp, pHyphDsp, pThesDsp, pListenerHelper
    // will be freed in the destructor of the respective Reference's
    // xSpellDsp, xGrammarDsp, xHyphDsp, xThesDsp

    clearSvcInfoArray(pAvailSpellSvcs);
    clearSvcInfoArray(pAvailGrammarSvcs);
    clearSvcInfoArray(pAvailHyphSvcs);
    clearSvcInfoArray(pAvailThesSvcs);
}

namespace
{
    using lang::Locale;
    using uno::Any;
    using uno::Sequence;

    bool lcl_FindEntry( const OUString &rEntry, const Sequence< OUString > &rCfgSvcs )
    {
        sal_Int32 nRes = -1;
        sal_Int32 nEntries = rCfgSvcs.getLength();
        const OUString *pEntry = rCfgSvcs.getConstArray();
        for (sal_Int32 i = 0;  i < nEntries && nRes == -1;  ++i)
        {
            if (rEntry == pEntry[i])
                nRes = i;
        }
        return nRes != -1;
    }

    Sequence< OUString > lcl_GetLastFoundSvcs(
            SvtLinguConfig &rCfg,
            const OUString &rLastFoundList ,
            const OUString& rCfgLocaleStr )
    {
        Sequence< OUString > aRes;

        Sequence< OUString > aNodeNames( rCfg.GetNodeNames(rLastFoundList) );
        bool bFound = lcl_FindEntry( rCfgLocaleStr, aNodeNames);

        if (bFound)
        {
            Sequence< OUString > aNames(1);
            OUString &rNodeName = aNames.getArray()[0];
            rNodeName = rLastFoundList + "/" + rCfgLocaleStr;
            Sequence< Any > aValues( rCfg.GetProperties( aNames ) );
            if (aValues.getLength())
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
        Sequence< OUString > aRes( rCfgSvcs.getLength() );
        OUString *pRes = aRes.getArray();
        sal_Int32 nCnt = 0;

        sal_Int32 nEntries = rCfgSvcs.getLength();
        const OUString *pEntry = rCfgSvcs.getConstArray();
        for (sal_Int32 i = 0;  i < nEntries;  ++i)
        {
            if (!pEntry[i].isEmpty() && lcl_FindEntry( pEntry[i], rAvailSvcs ))
                pRes[ nCnt++ ] = pEntry[i];
        }

        aRes.realloc( nCnt );
        return aRes;
    }

    Sequence< OUString > lcl_GetNewEntries(
            const Sequence< OUString > &rLastFoundSvcs,
            const Sequence< OUString > &rAvailSvcs )
    {
        sal_Int32 nLen = rAvailSvcs.getLength();
        Sequence< OUString > aRes( nLen );
        OUString *pRes = aRes.getArray();
        sal_Int32 nCnt = 0;

        const OUString *pEntry = rAvailSvcs.getConstArray();
        for (sal_Int32 i = 0;  i < nLen;  ++i)
        {
            if (!pEntry[i].isEmpty() && !lcl_FindEntry( pEntry[i], rLastFoundSvcs ))
                pRes[ nCnt++ ] = pEntry[i];
        }

        aRes.realloc( nCnt );
        return aRes;
    }

    Sequence< OUString > lcl_MergeSeq(
            const Sequence< OUString > &rCfgSvcs,
            const Sequence< OUString > &rNewSvcs )
    {
        Sequence< OUString > aRes( rCfgSvcs.getLength() + rNewSvcs.getLength() );
        OUString *pRes = aRes.getArray();
        sal_Int32 nCnt = 0;

        for (sal_Int32 k = 0;  k < 2;  ++k)
        {
            // add previously configured service first and append
            // new found services at the end
            const Sequence< OUString > &rSeq = k == 0 ? rCfgSvcs : rNewSvcs;

            sal_Int32 nLen = rSeq.getLength();
            const OUString *pEntry = rSeq.getConstArray();
            for (sal_Int32 i = 0;  i < nLen;  ++i)
            {
                if (!pEntry[i].isEmpty() && !lcl_FindEntry( pEntry[i], aRes ))
                    pRes[ nCnt++ ] = pEntry[i];
            }
        }

        aRes.realloc( nCnt );
        return aRes;
    }
}

void LngSvcMgr::UpdateAll()
{
    using beans::PropertyValue;
    using lang::Locale;
    using uno::Sequence;

    typedef OUString OUstring_t;
    typedef Sequence< OUString > Sequence_OUString_t;
    typedef std::map< OUstring_t, Sequence_OUString_t > list_entry_map_t;

    SvtLinguConfig aCfg;

    const int nNumServices = 4;
    const sal_Char * apServices[nNumServices]       =  { SN_SPELLCHECKER, SN_GRAMMARCHECKER, SN_HYPHENATOR, SN_THESAURUS };
    const sal_Char * apCurLists[nNumServices]       =  { "ServiceManager/SpellCheckerList",       "ServiceManager/GrammarCheckerList",       "ServiceManager/HyphenatorList",       "ServiceManager/ThesaurusList" };
    const sal_Char * apLastFoundLists[nNumServices] =  { "ServiceManager/LastFoundSpellCheckers", "ServiceManager/LastFoundGrammarCheckers", "ServiceManager/LastFoundHyphenators", "ServiceManager/LastFoundThesauri" };

    // usage of indices as above: 0 = spell checker, 1 = grammar checker, 2 = hyphenator, 3 = thesaurus
    std::vector< list_entry_map_t > aLastFoundSvcs(nNumServices);
    std::vector< list_entry_map_t > aCurSvcs(nNumServices);

    for (int k = 0;  k < nNumServices;  ++k)
    {
        OUString aService( OUString::createFromAscii( apServices[k] ) );
        OUString aActiveList( OUString::createFromAscii( apCurLists[k] ) );
        OUString aLastFoundList( OUString::createFromAscii( apLastFoundLists[k] ) );
        sal_Int32 i;


        // remove configured but not available language/services entries

        Sequence< OUString > aNodeNames( aCfg.GetNodeNames( aActiveList ) );   // list of configured locales
        sal_Int32 nNodeNames = aNodeNames.getLength();
        const OUString *pNodeName = aNodeNames.getConstArray();
        for (i = 0;  i < nNodeNames;  ++i)
        {
            Locale aLocale( LanguageTag::convertToLocale( pNodeName[i]));
            Sequence< OUString > aCfgSvcs( getConfiguredServices( aService, aLocale ));
            Sequence< OUString > aAvailSvcs( getAvailableServices( aService, aLocale ));

            aCfgSvcs = lcl_RemoveMissingEntries( aCfgSvcs, aAvailSvcs );

            aCurSvcs[k][ pNodeName[i] ] = aCfgSvcs;
        }


        // add new available language/service entries
        // and
        // set last found services to currently available ones

        Sequence< Locale > aAvailLocales( getAvailableLocales(aService) );
        sal_Int32 nAvailLocales = aAvailLocales.getLength();
        const Locale *pAvailLocale = aAvailLocales.getConstArray();
        for (i = 0;  i < nAvailLocales;  ++i)
        {
            OUString aCfgLocaleStr( LanguageTag::convertToBcp47( pAvailLocale[i]));

            Sequence< OUString > aAvailSvcs( getAvailableServices( aService, pAvailLocale[i] ));

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
            const sal_Char *pSubNodeName = (i == 0) ? apCurLists[k] : apLastFoundLists[k];
            OUString aSubNodeName( OUString::createFromAscii(pSubNodeName) );

            list_entry_map_t &rCurMap = (i == 0) ? aCurSvcs[k] : aLastFoundSvcs[k];
            list_entry_map_t::const_iterator aIt( rCurMap.begin() );
            sal_Int32 nVals = static_cast< sal_Int32 >( rCurMap.size() );
            Sequence< PropertyValue > aNewValues( nVals );
            PropertyValue *pNewValue = aNewValues.getArray();
            while (aIt != rCurMap.end())
            {
                pNewValue->Name = aSubNodeName + "/" + (*aIt).first;
                pNewValue->Value <<= (*aIt).second;
                ++pNewValue;
                ++aIt;
            }
            OSL_ENSURE( pNewValue - aNewValues.getArray() == nVals,
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
    const OUString aSpellCheckerList( "ServiceManager/SpellCheckerList" );
    const OUString aGrammarCheckerList( "ServiceManager/GrammarCheckerList" );
    const OUString aHyphenatorList( "ServiceManager/HyphenatorList" );
    const OUString aThesaurusList( "ServiceManager/ThesaurusList" );

    const uno::Sequence< OUString > aSpellCheckerListEntries( GetNodeNames( aSpellCheckerList ) );
    const uno::Sequence< OUString > aGrammarCheckerListEntries( GetNodeNames( aGrammarCheckerList ) );
    const uno::Sequence< OUString > aHyphenatorListEntries( GetNodeNames( aHyphenatorList ) );
    const uno::Sequence< OUString > aThesaurusListEntries( GetNodeNames( aThesaurusList ) );

    uno::Sequence< uno::Any > aValues;
    uno::Sequence< OUString > aNames( 1 );
    OUString *pNames = aNames.getArray();

    sal_Int32 nLen = rPropertyNames.getLength();
    const OUString *pPropertyNames = rPropertyNames.getConstArray();
    for (sal_Int32 i = 0;  i < nLen;  ++i)
    {
        // property names look like
        // "ServiceManager/ThesaurusList/de-CH"

        const OUString &rName = pPropertyNames[i];
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
            clearSvcInfoArray(pAvailSpellSvcs);

            if (lcl_SeqHasString( aSpellCheckerListEntries, aKeyText ))
            {
                pNames[0] = aSpellCheckerList + "/" + aKeyText;
                aValues = /*aCfg.*/GetProperties( aNames );
                uno::Sequence< OUString > aSvcImplNames;
                if (aValues.getLength())
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
            clearSvcInfoArray(pAvailGrammarSvcs);

            if (lcl_SeqHasString( aGrammarCheckerListEntries, aKeyText ))
            {
                pNames[0] = aGrammarCheckerList + "/" + aKeyText;
                aValues = /*aCfg.*/GetProperties( aNames );
                uno::Sequence< OUString > aSvcImplNames;
                if (aValues.getLength())
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
            clearSvcInfoArray(pAvailHyphSvcs);

            if (lcl_SeqHasString( aHyphenatorListEntries, aKeyText ))
            {
                pNames[0] = aHyphenatorList + "/" + aKeyText;
                aValues = /*aCfg.*/GetProperties( aNames );
                uno::Sequence< OUString > aSvcImplNames;
                if (aValues.getLength())
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
            clearSvcInfoArray(pAvailThesSvcs);

            if (lcl_SeqHasString( aThesaurusListEntries, aKeyText ))
            {
                pNames[0] = aThesaurusList + "/" + aKeyText;
                aValues = /*aCfg.*/GetProperties( aNames );
                uno::Sequence< OUString > aSvcImplNames;
                if (aValues.getLength())
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
            SAL_WARN( "linguistic", "nofified for unexpected property" );
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
    if (!mxGrammarDsp.is() && SvtLinguConfig().HasGrammarChecker())
    {
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
    if (!pAvailSpellSvcs)
    {
        pAvailSpellSvcs = new SvcInfoArray;

        uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );

        uno::Reference< container::XContentEnumerationAccess > xEnumAccess( xContext->getServiceManager(), uno::UNO_QUERY );
        uno::Reference< container::XEnumeration > xEnum;
        if (xEnumAccess.is())
            xEnum = xEnumAccess->createContentEnumeration( SN_SPELLCHECKER );

        if (xEnum.is())
        {
            while (xEnum->hasMoreElements())
            {
                uno::Any aCurrent = xEnum->nextElement();
                uno::Reference< lang::XSingleComponentFactory > xCompFactory;
                uno::Reference< lang::XSingleServiceFactory > xFactory;

                uno::Reference< linguistic2::XSpellChecker > xSvc;
                xCompFactory.set(aCurrent, css::uno::UNO_QUERY);
                if (!xCompFactory.is())
                {
                    xFactory.set(aCurrent, css::uno::UNO_QUERY);
                }
                if ( xCompFactory.is() || xFactory.is() )
                {
                    try
                    {
                        xSvc.set( ( xCompFactory.is() ? xCompFactory->createInstanceWithContext( xContext ) : xFactory->createInstance() ), uno::UNO_QUERY );
                    }
                    catch (const uno::Exception &)
                    {
                        SAL_WARN( "linguistic", "createInstance failed" );
                    }
                }

                if (xSvc.is())
                {
                    OUString            aImplName;
                    uno::Sequence< sal_Int16 >    aLanguages;
                    uno::Reference< XServiceInfo > xInfo( xSvc, uno::UNO_QUERY );
                    if (xInfo.is())
                        aImplName = xInfo->getImplementationName();
                    SAL_WARN_IF( aImplName.isEmpty(), "linguistic", "empty implementation name" );
                    uno::Reference< linguistic2::XSupportedLocales > xSuppLoc( xSvc, uno::UNO_QUERY );
                    SAL_WARN_IF( !xSuppLoc.is(), "linguistic", "interfaces not supported" );
                    if (xSuppLoc.is()) {
                        uno::Sequence<lang::Locale> aLocaleSequence(xSuppLoc->getLocales());
                        aLanguages = LocaleSeqToLangSeq( aLocaleSequence );
                    }

                    pAvailSpellSvcs->push_back( o3tl::make_unique<SvcInfo>( aImplName, aLanguages ) );
                }
            }
        }
    }
}


void LngSvcMgr::GetAvailableGrammarSvcs_Impl()
{
    if (!pAvailGrammarSvcs)
    {
        pAvailGrammarSvcs = new SvcInfoArray;

        uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );

        uno::Reference< container::XContentEnumerationAccess > xEnumAccess( xContext->getServiceManager(), uno::UNO_QUERY );
        uno::Reference< container::XEnumeration > xEnum;
        if (xEnumAccess.is())
            xEnum = xEnumAccess->createContentEnumeration( SN_GRAMMARCHECKER );

        if (xEnum.is())
        {
            while (xEnum->hasMoreElements())
            {
                uno::Any aCurrent = xEnum->nextElement();
                uno::Reference< lang::XSingleComponentFactory > xCompFactory;
                uno::Reference< lang::XSingleServiceFactory > xFactory;

                uno::Reference< linguistic2::XProofreader > xSvc;
                xCompFactory.set(aCurrent, css::uno::UNO_QUERY);
                if (!xCompFactory.is())
                {
                    xFactory.set(aCurrent, css::uno::UNO_QUERY);
                }
                if ( xCompFactory.is() || xFactory.is() )
                {
                    try
                    {
                        xSvc.set( ( xCompFactory.is() ? xCompFactory->createInstanceWithContext( xContext ) : xFactory->createInstance() ), uno::UNO_QUERY );
                    }
                    catch (const uno::Exception &)
                    {
                        SAL_WARN( "linguistic", "createInstance failed" );
                    }
                }

                if (xSvc.is() && pAvailGrammarSvcs)
                {
                    OUString            aImplName;
                    uno::Sequence< sal_Int16 >   aLanguages;
                    uno::Reference< XServiceInfo > xInfo( xSvc, uno::UNO_QUERY );
                    if (xInfo.is())
                        aImplName = xInfo->getImplementationName();
                    SAL_WARN_IF( aImplName.isEmpty(), "linguistic", "empty implementation name" );
                    uno::Reference< linguistic2::XSupportedLocales > xSuppLoc( xSvc, uno::UNO_QUERY );
                    SAL_WARN_IF( !xSuppLoc.is(), "linguistic", "interfaces not supported" );
                    if (xSuppLoc.is())
                    {
                        uno::Sequence<lang::Locale> aLocaleSequence(xSuppLoc->getLocales());
                        aLanguages = LocaleSeqToLangSeq( aLocaleSequence );
                    }

                    pAvailGrammarSvcs->push_back( o3tl::make_unique<SvcInfo>( aImplName, aLanguages ) );
                }
            }
        }
    }
}


void LngSvcMgr::GetAvailableHyphSvcs_Impl()
{
    if (!pAvailHyphSvcs)
    {
        pAvailHyphSvcs = new SvcInfoArray;
        uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );

        uno::Reference< container::XContentEnumerationAccess > xEnumAccess( xContext->getServiceManager(), uno::UNO_QUERY );
        uno::Reference< container::XEnumeration > xEnum;
        if (xEnumAccess.is())
            xEnum = xEnumAccess->createContentEnumeration( SN_HYPHENATOR );

        if (xEnum.is())
        {
            while (xEnum->hasMoreElements())
            {
                uno::Any aCurrent = xEnum->nextElement();
                uno::Reference< lang::XSingleComponentFactory > xCompFactory;
                uno::Reference< lang::XSingleServiceFactory > xFactory;

                uno::Reference< linguistic2::XHyphenator > xSvc;
                xCompFactory.set(aCurrent, css::uno::UNO_QUERY);
                if (!xCompFactory.is())
                {
                    xFactory.set(aCurrent, css::uno::UNO_QUERY);
                }
                if ( xCompFactory.is() || xFactory.is() )
                {
                    try
                    {
                        xSvc.set( ( xCompFactory.is() ? xCompFactory->createInstanceWithContext( xContext ) : xFactory->createInstance() ), uno::UNO_QUERY );
                    }
                    catch (const uno::Exception &)
                    {
                        SAL_WARN( "linguistic", "createInstance failed" );
                    }
                }
                if (xSvc.is())
                {
                    OUString            aImplName;
                    uno::Sequence< sal_Int16 >    aLanguages;
                    uno::Reference< XServiceInfo > xInfo( xSvc, uno::UNO_QUERY );
                    if (xInfo.is())
                        aImplName = xInfo->getImplementationName();
                    SAL_WARN_IF( aImplName.isEmpty(), "linguistic", "empty implementation name" );
                    uno::Reference< linguistic2::XSupportedLocales > xSuppLoc( xSvc, uno::UNO_QUERY );
                    SAL_WARN_IF( !xSuppLoc.is(), "linguistic", "interfaces not supported" );
                    if (xSuppLoc.is())
                    {
                        uno::Sequence<lang::Locale> aLocaleSequence(xSuppLoc->getLocales());
                        aLanguages = LocaleSeqToLangSeq( aLocaleSequence );
                    }
                    pAvailHyphSvcs->push_back( o3tl::make_unique<SvcInfo>( aImplName, aLanguages ) );
                }
            }
        }
    }
}


void LngSvcMgr::GetAvailableThesSvcs_Impl()
{
    if (!pAvailThesSvcs)
    {
        pAvailThesSvcs = new SvcInfoArray;

        uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );

        uno::Reference< container::XContentEnumerationAccess > xEnumAccess( xContext->getServiceManager(), uno::UNO_QUERY );
        uno::Reference< container::XEnumeration > xEnum;
        if (xEnumAccess.is())
            xEnum = xEnumAccess->createContentEnumeration( SN_THESAURUS );

        if (xEnum.is())
        {
            while (xEnum->hasMoreElements())
            {
                uno::Any aCurrent = xEnum->nextElement();
                uno::Reference< lang::XSingleComponentFactory > xCompFactory;
                uno::Reference< lang::XSingleServiceFactory > xFactory;

                uno::Reference< linguistic2::XThesaurus > xSvc;
                xCompFactory.set(aCurrent, css::uno::UNO_QUERY);
                if (!xCompFactory.is())
                {
                    xFactory.set(aCurrent, css::uno::UNO_QUERY);
                }
                if ( xCompFactory.is() || xFactory.is() )
                {
                    try
                    {
                        xSvc.set( ( xCompFactory.is() ? xCompFactory->createInstanceWithContext( xContext ) : xFactory->createInstance() ), uno::UNO_QUERY );
                    }
                    catch (const uno::Exception &)
                    {
                       SAL_WARN( "linguistic", "createInstance failed" );
                    }
                }
                if (xSvc.is())
                {
                    OUString            aImplName;
                    uno::Sequence< sal_Int16 >    aLanguages;
                    uno::Reference< XServiceInfo > xInfo( xSvc, uno::UNO_QUERY );
                    if (xInfo.is())
                        aImplName = xInfo->getImplementationName();
                    SAL_WARN_IF( aImplName.isEmpty(), "linguistic", "empty implementation name" );
                    uno::Reference< linguistic2::XSupportedLocales > xSuppLoc( xSvc, uno::UNO_QUERY );
                    SAL_WARN_IF( !xSuppLoc.is(), "linguistic", "interfaces not supported" );
                    if (xSuppLoc.is())
                    {
                        uno::Sequence<lang::Locale> aLocaleSequence(xSuppLoc->getLocales());
                        aLanguages = LocaleSeqToLangSeq( aLocaleSequence );
                    }

                    pAvailThesSvcs->push_back( o3tl::make_unique<SvcInfo>( aImplName, aLanguages ) );
                }
            }
        }
    }
}


void LngSvcMgr::SetCfgServiceLists( SpellCheckerDispatcher &rSpellDsp )
{
    SAL_INFO( "linguistic", "linguistic: LngSvcMgr::SetCfgServiceLists - Spell" );

    OUString aNode("ServiceManager/SpellCheckerList");
    uno::Sequence< OUString > aNames( /*aCfg.*/GetNodeNames( aNode ) );
    OUString *pNames = aNames.getArray();
    sal_Int32 nLen = aNames.getLength();

    // append path prefix need for 'GetProperties' call below
    OUString aPrefix( aNode );
    aPrefix += "/";
    for (int i = 0;  i < nLen;  ++i)
    {
        OUString aTmp( aPrefix );
        aTmp += pNames[i];
        pNames[i] = aTmp;
    }

    uno::Sequence< uno::Any > aValues( /*aCfg.*/GetProperties( aNames ) );
    if (nLen  &&  nLen == aValues.getLength())
    {
        const uno::Any *pValues = aValues.getConstArray();
        for (sal_Int32 i = 0;  i < nLen;  ++i)
        {
            uno::Sequence< OUString > aSvcImplNames;
            if (pValues[i] >>= aSvcImplNames)
            {
                OUString aLocaleStr( pNames[i] );
                sal_Int32 nSeparatorPos = aLocaleStr.lastIndexOf( '/' );
                aLocaleStr = aLocaleStr.copy( nSeparatorPos + 1 );
                rSpellDsp.SetServiceList( LanguageTag::convertToLocale(aLocaleStr), aSvcImplNames );
            }
        }
    }
}


void LngSvcMgr::SetCfgServiceLists( GrammarCheckingIterator &rGrammarDsp )
{
    SAL_INFO( "linguistic", "linguistic: LngSvcMgr::SetCfgServiceLists - Grammar" );

    OUString aNode("ServiceManager/GrammarCheckerList");
    uno::Sequence< OUString > aNames( /*aCfg.*/GetNodeNames( aNode ) );
    OUString *pNames = aNames.getArray();
    sal_Int32 nLen = aNames.getLength();

    // append path prefix need for 'GetProperties' call below
    OUString aPrefix( aNode );
    aPrefix += "/";
    for (int i = 0;  i < nLen;  ++i)
    {
        OUString aTmp( aPrefix );
        aTmp += pNames[i];
        pNames[i] = aTmp;
    }

    uno::Sequence< uno::Any > aValues( /*aCfg.*/GetProperties( aNames ) );
    if (nLen  &&  nLen == aValues.getLength())
    {
        const uno::Any *pValues = aValues.getConstArray();
        for (sal_Int32 i = 0;  i < nLen;  ++i)
        {
            uno::Sequence< OUString > aSvcImplNames;
            if (pValues[i] >>= aSvcImplNames)
            {
                // there should only be one grammar checker in use per language...
                if (aSvcImplNames.getLength() > 1)
                    aSvcImplNames.realloc(1);

                OUString aLocaleStr( pNames[i] );
                sal_Int32 nSeparatorPos = aLocaleStr.lastIndexOf( '/' );
                aLocaleStr = aLocaleStr.copy( nSeparatorPos + 1 );
                rGrammarDsp.SetServiceList( LanguageTag::convertToLocale(aLocaleStr), aSvcImplNames );
            }
        }
    }
}


void LngSvcMgr::SetCfgServiceLists( HyphenatorDispatcher &rHyphDsp )
{
    SAL_INFO( "linguistic", "linguistic: LngSvcMgr::SetCfgServiceLists - Hyph" );

    OUString aNode("ServiceManager/HyphenatorList");
    uno::Sequence< OUString > aNames( /*aCfg.*/GetNodeNames( aNode ) );
    OUString *pNames = aNames.getArray();
    sal_Int32 nLen = aNames.getLength();

    // append path prefix need for 'GetProperties' call below
    OUString aPrefix( aNode );
    aPrefix += "/";
    for (int i = 0;  i < nLen;  ++i)
    {
        OUString aTmp( aPrefix );
        aTmp += pNames[i];
        pNames[i] = aTmp;
    }

    uno::Sequence< uno::Any > aValues( /*aCfg.*/GetProperties( aNames ) );
    if (nLen  &&  nLen == aValues.getLength())
    {
        const uno::Any *pValues = aValues.getConstArray();
        for (sal_Int32 i = 0;  i < nLen;  ++i)
        {
            uno::Sequence< OUString > aSvcImplNames;
            if (pValues[i] >>= aSvcImplNames)
            {
                // there should only be one hyphenator in use per language...
                if (aSvcImplNames.getLength() > 1)
                    aSvcImplNames.realloc(1);

                OUString aLocaleStr( pNames[i] );
                sal_Int32 nSeparatorPos = aLocaleStr.lastIndexOf( '/' );
                aLocaleStr = aLocaleStr.copy( nSeparatorPos + 1 );
                rHyphDsp.SetServiceList( LanguageTag::convertToLocale(aLocaleStr), aSvcImplNames );
            }
        }
    }
}


void LngSvcMgr::SetCfgServiceLists( ThesaurusDispatcher &rThesDsp )
{
    SAL_INFO( "linguistic", "linguistic: LngSvcMgr::SetCfgServiceLists - Thes" );

    OUString aNode("ServiceManager/ThesaurusList");
    uno::Sequence< OUString > aNames( /*aCfg.*/GetNodeNames( aNode ) );
    OUString *pNames = aNames.getArray();
    sal_Int32 nLen = aNames.getLength();

    // append path prefix need for 'GetProperties' call below
    OUString aPrefix( aNode );
    aPrefix += "/";
    for (int i = 0;  i < nLen;  ++i)
    {
        OUString aTmp( aPrefix );
        aTmp += pNames[i];
        pNames[i] = aTmp;
    }

    uno::Sequence< uno::Any > aValues( /*aCfg.*/GetProperties( aNames ) );
    if (nLen  &&  nLen == aValues.getLength())
    {
        const uno::Any *pValues = aValues.getConstArray();
        for (sal_Int32 i = 0;  i < nLen;  ++i)
        {
            uno::Sequence< OUString > aSvcImplNames;
            if (pValues[i] >>= aSvcImplNames)
            {
                OUString aLocaleStr( pNames[i] );
                sal_Int32 nSeparatorPos = aLocaleStr.lastIndexOf( '/' );
                aLocaleStr = aLocaleStr.copy( nSeparatorPos + 1 );
                rThesDsp.SetServiceList( LanguageTag::convertToLocale(aLocaleStr), aSvcImplNames );
            }
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

    bool bRes = false;
    if (!bDisposing  &&  xListener.is())
    {
        if (!mxListenerHelper.is())
            GetListenerHelper_Impl();
        bRes = mxListenerHelper->AddLngSvcMgrListener( xListener );
    }
    return bRes;
}


sal_Bool SAL_CALL
    LngSvcMgr::removeLinguServiceManagerListener(
            const uno::Reference< lang::XEventListener >& xListener )
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    bool bRes = false;
    if (!bDisposing  &&  xListener.is())
    {
        DBG_ASSERT( mxListenerHelper.is(), "listener removed without being added" );
        if (!mxListenerHelper.is())
            GetListenerHelper_Impl();
        bRes = mxListenerHelper->RemoveLngSvcMgrListener( xListener );
    }
    return bRes;
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
        pInfoArray = pAvailSpellSvcs;
    }
    else if (rServiceName == SN_GRAMMARCHECKER)
    {
        GetAvailableGrammarSvcs_Impl();
        pInfoArray = pAvailGrammarSvcs;
    }
    else if (rServiceName == SN_HYPHENATOR)
    {
        GetAvailableHyphSvcs_Impl();
        pInfoArray = pAvailHyphSvcs;
    }
    else if (rServiceName == SN_THESAURUS)
    {
        GetAvailableThesSvcs_Impl();
        pInfoArray = pAvailThesSvcs;
    }

    if (pInfoArray)
    {
        // resize to max number of entries
        size_t nMaxCnt = pInfoArray->size();
        aRes.realloc( nMaxCnt );
        OUString *pImplName = aRes.getArray();

        sal_uInt16 nCnt = 0;
        LanguageType nLanguage = LinguLocaleToLanguage( rLocale );
        for (size_t i = 0;  i < nMaxCnt; ++i)
        {
            const SvcInfo &rInfo = *(*pInfoArray)[i].get();
            if (LinguIsUnspecified( nLanguage )
                || rInfo.HasLanguage( nLanguage ))
            {
                pImplName[ nCnt++ ] = rInfo.aSvcImplName;
            }
        }

        // resize to actual number of entries
        if (nCnt != nMaxCnt)
            aRes.realloc( nCnt );
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
    // returns true iff both sequences are equal

    bool bRes = false;
    sal_Int32 nLen = rList1.getLength();
    if (rList2.getLength() == nLen)
    {
        const OUString *pStr1 = rList1.getConstArray();
        const OUString *pStr2 = rList2.getConstArray();
        bRes = true;
        for (sal_Int32 i = 0;  i < nLen  &&  bRes;  ++i)
        {
            if (*pStr1++ != *pStr2++)
                bRes = false;
        }
    }
    return bRes;
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
    if (!LinguIsUnspecified( nLanguage))
    {
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

                if (mxListenerHelper.is() && bChanged)
                    mxListenerHelper->AddLngSvcEvt(
                            linguistic2::LinguServiceEventFlags::SPELL_CORRECT_WORDS_AGAIN |
                            linguistic2::LinguServiceEventFlags::SPELL_WRONG_WORDS_AGAIN );
            }
        }
        else if (rServiceName == SN_GRAMMARCHECKER)
        {
            if (!mxGrammarDsp.is())
                GetGrammarCheckerDsp_Impl();
            bool bChanged = !IsEqSvcList( rServiceImplNames,
                                          mxGrammarDsp->GetServiceList( rLocale ) );
            if (bChanged)
            {
                mxGrammarDsp->SetServiceList( rLocale, rServiceImplNames );
                SaveCfgSvcs( SN_GRAMMARCHECKER );

                if (mxListenerHelper.is() && bChanged)
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

                if (mxListenerHelper.is() && bChanged)
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
}


bool LngSvcMgr::SaveCfgSvcs( const OUString &rServiceName )
{
    SAL_INFO( "linguistic", "linguistic: LngSvcMgr::SaveCfgSvcs" );

    bool bRes = false;

    LinguDispatcher *pDsp = nullptr;
    uno::Sequence< lang::Locale > aLocales;

    if (rServiceName == SN_SPELLCHECKER)
    {
        if (!mxSpellDsp.get())
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

    if (pDsp  &&  aLocales.getLength())
    {
        sal_Int32 nLen = aLocales.getLength();
        const lang::Locale *pLocale = aLocales.getConstArray();

        uno::Sequence< beans::PropertyValue > aValues( nLen );
        beans::PropertyValue *pValues = aValues.getArray();
        beans::PropertyValue *pValue  = pValues;

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

        for (sal_Int32 i = 0;  i < nLen;  ++i)
        {
            uno::Sequence< OUString > aSvcImplNames;
            aSvcImplNames = pDsp->GetServiceList( pLocale[i] );

            // build value to be written back to configuration
            uno::Any aCfgAny;
            if ((pDsp == mxHyphDsp.get() || pDsp == mxGrammarDsp.get()) && aSvcImplNames.getLength() > 1)
                aSvcImplNames.realloc(1);   // there should be only one entry for hyphenators or grammar checkers (because they are not chained)
            aCfgAny <<= aSvcImplNames;
            DBG_ASSERT( aCfgAny.hasValue(), "missing value for 'Any' type" );

            OUString aCfgLocaleStr( LanguageTag::convertToBcp47( pLocale[i]));
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
        sal_Int32 nSvcs = aRes.getLength();
        if (nSvcs)
        {
            const OUString *pSvcName = aRes.getConstArray();
            for (sal_Int32 j = 0;  j < nSvcs;  ++j)
            {
                SAL_WARN_IF( pSvcName[j].isEmpty(), "linguistic", "service impl-name missing" );
            }
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
        OUString aNode( "ServiceManager/SpellCheckerList");
        const uno::Sequence< OUString > aNodeEntries( GetNodeNames( aNode ) );
        if (lcl_SeqHasString( aNodeEntries, aCfgLocale ))
        {
            pNames[0] = aNode + "/" + aCfgLocale;
            aValues = /*aCfg.*/GetProperties( aNames );
            if (aValues.getLength())
                aSvcImplNames = GetLangSvcList( aValues.getConstArray()[0] );
        }
    }
    else if ( rServiceName == SN_GRAMMARCHECKER )
    {
        OUString aNode( "ServiceManager/GrammarCheckerList");
        const uno::Sequence< OUString > aNodeEntries( GetNodeNames( aNode ) );
        if (lcl_SeqHasString( aNodeEntries, aCfgLocale ))
        {
            pNames[0] = aNode + "/" + aCfgLocale;
            aValues = /*aCfg.*/GetProperties( aNames );
            if (aValues.getLength())
                aSvcImplNames = GetLangSvc( aValues.getConstArray()[0] );
        }
    }
    else if ( rServiceName == SN_HYPHENATOR )
    {
        OUString aNode( "ServiceManager/HyphenatorList");
        const uno::Sequence< OUString > aNodeEntries( GetNodeNames( aNode ) );
        if (lcl_SeqHasString( aNodeEntries, aCfgLocale ))
        {
            pNames[0] = aNode + "/" + aCfgLocale;
            aValues = /*aCfg.*/GetProperties( aNames );
            if (aValues.getLength())
                aSvcImplNames = GetLangSvc( aValues.getConstArray()[0] );
        }
    }
    else if ( rServiceName == SN_THESAURUS )
    {
        OUString aNode( "ServiceManager/ThesaurusList");
        const uno::Sequence< OUString > aNodeEntries( GetNodeNames( aNode ) );
        if (lcl_SeqHasString( aNodeEntries, aCfgLocale ))
        {
            pNames[0] = aNode + "/" + aCfgLocale;
            aValues = /*aCfg.*/GetProperties( aNames );
            if (aValues.getLength())
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
    bool bRes = false;
    if (rxBroadcaster.is())
    {
        if (!mxListenerHelper.is())
            GetListenerHelper_Impl();
        bRes = mxListenerHelper->AddLngSvcEvtBroadcaster( rxBroadcaster );
    }
    return bRes;
}


OUString SAL_CALL
    LngSvcMgr::getImplementationName()
{
    return getImplementationName_Static();
}


sal_Bool SAL_CALL
    LngSvcMgr::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}


uno::Sequence< OUString > SAL_CALL
    LngSvcMgr::getSupportedServiceNames()
{
    return getSupportedServiceNames_Static();
}


uno::Sequence< OUString > LngSvcMgr::getSupportedServiceNames_Static()
        throw()
{
    uno::Sequence< OUString > aSNS { "com.sun.star.linguistic2.LinguServiceManager" };
    return aSNS;
}

/// @throws uno::Exception
uno::Reference< uno::XInterface > SAL_CALL LngSvcMgr_CreateInstance(
            const uno::Reference< lang::XMultiServiceFactory > & /*rSMgr*/ )
{
    uno::Reference< uno::XInterface > xService = static_cast<cppu::OWeakObject*>(new LngSvcMgr);
    return xService;
}

void * SAL_CALL LngSvcMgr_getFactory(
            const sal_Char * pImplName,
            lang::XMultiServiceFactory * pServiceManager,
            void * /*pRegistryKey*/ )
{

    void * pRet = nullptr;
    if ( LngSvcMgr::getImplementationName_Static().equalsAscii( pImplName ) )
    {
        uno::Reference< lang::XSingleServiceFactory > xFactory =
            cppu::createOneInstanceFactory(
                pServiceManager,
                LngSvcMgr::getImplementationName_Static(),
                LngSvcMgr_CreateInstance,
                LngSvcMgr::getSupportedServiceNames_Static());
        // acquire, because we return an interface pointer instead of a reference
        xFactory->acquire();
        pRet = xFactory.get();
    }
    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
