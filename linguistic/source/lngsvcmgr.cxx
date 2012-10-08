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


#include <com/sun/star/deployment/ExtensionManager.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/linguistic2/XSupportedLocales.hpp>
#include <com/sun/star/linguistic2/DictionaryListEventFlags.hpp>
#include <com/sun/star/linguistic2/LinguServiceEventFlags.hpp>

#include <tools/solar.h>
#include <unotools/lingucfg.hxx>
#include <comphelper/processfactory.hxx>
#include <i18npool/lang.h>
#include <i18npool/mslangid.hxx>
#include <cppuhelper/factory.hxx>
#include <comphelper/extract.hxx>
#include <rtl/logfile.hxx>

#include <boost/checked_delete.hpp>

#include "lngsvcmgr.hxx"
#include "lngopt.hxx"
#include "linguistic/misc.hxx"
#include "spelldsp.hxx"
#include "hyphdsp.hxx"
#include "thesdsp.hxx"
#include "gciterator.hxx"


using namespace com::sun::star;
using namespace linguistic;
using ::rtl::OUString;

// forward declarations
uno::Sequence< OUString > static GetLangSvcList( const uno::Any &rVal );
uno::Sequence< OUString > static GetLangSvc( const uno::Any &rVal );


static sal_Bool lcl_SeqHasString( const uno::Sequence< OUString > &rSeq, const OUString &rText )
{
    sal_Bool bRes = sal_False;

    sal_Int32 nLen = rSeq.getLength();
    if (nLen == 0 || rText.isEmpty())
        return bRes;

    const OUString *pSeq = rSeq.getConstArray();
    for (sal_Int32 i = 0;  i < nLen  &&  !bRes;  ++i)
    {
        if (rText == pSeq[i])
            bRes = sal_True;
    }
    return bRes;
}


static uno::Sequence< lang::Locale > GetAvailLocales(
        const uno::Sequence< OUString > &rSvcImplNames )
{
    uno::Sequence< lang::Locale > aRes;

    uno::Reference< lang::XMultiServiceFactory >  xFac( comphelper::getProcessServiceFactory() );
    sal_Int32 nNames = rSvcImplNames.getLength();
    if (nNames  &&  xFac.is())
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
                xSuppLoc = uno::Reference< linguistic2::XSupportedLocales >(
                        xFac->createInstanceWithArguments( pImplNames[i], aArgs ), uno::UNO_QUERY );
            }
            catch (uno::Exception &)
            {
                DBG_ASSERT( 0, "createInstanceWithArguments failed" );
            }

            if (xSuppLoc.is())
            {
                uno::Sequence< lang::Locale > aLoc( xSuppLoc->getLocales() );
                sal_Int32 nLoc = aLoc.getLength();
                for (sal_Int32 k = 0;  k < nLoc;  ++k)
                {
                    const lang::Locale *pLoc = aLoc.getConstArray();
                    LanguageType nLang = LocaleToLanguage( pLoc[k] );

                    // language not already added?
                    if (aLanguages.find( nLang ) == aLanguages.end())
                        aLanguages.insert( nLang );
                }
            }
            else
            {
                DBG_ASSERT( 0, "interface not supported by service" );
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
            pRes[i] = CreateLocale( nLang );
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

    sal_Bool    HasLanguage( sal_Int16 nLanguage ) const;
};


sal_Bool SvcInfo::HasLanguage( sal_Int16 nLanguage ) const
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
    public cppu::WeakImplHelper2
    <
        linguistic2::XLinguServiceEventListener,
        linguistic2::XDictionaryListEventListener
    >
{
    LngSvcMgr  &rMyManager;

    ::cppu::OInterfaceContainerHelper           aLngSvcMgrListeners;
    ::cppu::OInterfaceContainerHelper           aLngSvcEvtBroadcasters;
    uno::Reference< linguistic2::XDictionaryList >               xDicList;

    sal_Int16   nCombinedLngSvcEvt;

    // disallow copy-constructor and assignment-operator for now
    LngSvcMgrListenerHelper(const LngSvcMgrListenerHelper &);
    LngSvcMgrListenerHelper & operator = (const LngSvcMgrListenerHelper &);

    void    LaunchEvent( sal_Int16 nLngSvcEvtFlags );

    long Timeout();

public:
    LngSvcMgrListenerHelper( LngSvcMgr &rLngSvcMgr,
        const uno::Reference< linguistic2::XDictionaryList > &rxDicList );

    // lang::XEventListener
    virtual void SAL_CALL
        disposing( const lang::EventObject& rSource )
            throw(uno::RuntimeException);

    // linguistic2::XLinguServiceEventListener
    virtual void SAL_CALL
        processLinguServiceEvent( const linguistic2::LinguServiceEvent& aLngSvcEvent )
            throw(uno::RuntimeException);

    // linguistic2::XDictionaryListEventListener
    virtual void SAL_CALL
        processDictionaryListEvent(
                const linguistic2::DictionaryListEvent& rDicListEvent )
            throw(uno::RuntimeException);

    inline  sal_Bool    AddLngSvcMgrListener(
                        const uno::Reference< lang::XEventListener >& rxListener );
    inline  sal_Bool    RemoveLngSvcMgrListener(
                        const uno::Reference< lang::XEventListener >& rxListener );
    void    DisposeAndClear( const lang::EventObject &rEvtObj );
    sal_Bool    AddLngSvcEvtBroadcaster(
                        const uno::Reference< linguistic2::XLinguServiceEventBroadcaster > &rxBroadcaster );
    sal_Bool    RemoveLngSvcEvtBroadcaster(
                        const uno::Reference< linguistic2::XLinguServiceEventBroadcaster > &rxBroadcaster );

    void    AddLngSvcEvt( sal_Int16 nLngSvcEvt );
};


LngSvcMgrListenerHelper::LngSvcMgrListenerHelper(
        LngSvcMgr &rLngSvcMgr,
        const uno::Reference< linguistic2::XDictionaryList > &rxDicList  ) :
    rMyManager              ( rLngSvcMgr ),
    aLngSvcMgrListeners     ( GetLinguMutex() ),
    aLngSvcEvtBroadcasters  ( GetLinguMutex() ),
    xDicList                ( rxDicList )
{
    if (xDicList.is())
    {
        xDicList->addDictionaryListEventListener(
            (linguistic2::XDictionaryListEventListener *) this, sal_False );
    }

    nCombinedLngSvcEvt = 0;
}


void SAL_CALL LngSvcMgrListenerHelper::disposing( const lang::EventObject& rSource )
        throw(uno::RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    uno::Reference< uno::XInterface > xRef( rSource.Source );
    if ( xRef.is() )
    {
        aLngSvcMgrListeners   .removeInterface( xRef );
        aLngSvcEvtBroadcasters.removeInterface( xRef );
        if (xDicList == xRef)
            xDicList = 0;
    }
}

long LngSvcMgrListenerHelper::Timeout()
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    {
        // change event source to LinguServiceManager since the listeners
        // probably do not know (and need not to know) about the specific
        // SpellChecker's or Hyphenator's.
        linguistic2::LinguServiceEvent aEvtObj(
            static_cast<com::sun::star::linguistic2::XLinguServiceManager*>(&rMyManager), nCombinedLngSvcEvt );
        nCombinedLngSvcEvt = 0;

        if (rMyManager.pSpellDsp)
            rMyManager.pSpellDsp->FlushSpellCache();

        // pass event on to linguistic2::XLinguServiceEventListener's
        cppu::OInterfaceIteratorHelper aIt( aLngSvcMgrListeners );
        while (aIt.hasMoreElements())
        {
            uno::Reference< linguistic2::XLinguServiceEventListener > xRef( aIt.next(), uno::UNO_QUERY );
            if (xRef.is())
                xRef->processLinguServiceEvent( aEvtObj );
        }
    }
    return 0;
}


void LngSvcMgrListenerHelper::AddLngSvcEvt( sal_Int16 nLngSvcEvt )
{
    nCombinedLngSvcEvt |= nLngSvcEvt;
    Timeout();
}


void SAL_CALL
    LngSvcMgrListenerHelper::processLinguServiceEvent(
            const linguistic2::LinguServiceEvent& rLngSvcEvent )
        throw(uno::RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );
    AddLngSvcEvt( rLngSvcEvent.nEvent );
}


void SAL_CALL
    LngSvcMgrListenerHelper::processDictionaryListEvent(
            const linguistic2::DictionaryListEvent& rDicListEvent )
        throw(uno::RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    sal_Int16 nDlEvt = rDicListEvent.nCondensedEvent;
    if (0 == nDlEvt)
        return;

    // we do keep the original event source here though...

    // pass event on to linguistic2::XDictionaryListEventListener's
    cppu::OInterfaceIteratorHelper aIt( aLngSvcMgrListeners );
    while (aIt.hasMoreElements())
    {
        uno::Reference< linguistic2::XDictionaryListEventListener > xRef( aIt.next(), uno::UNO_QUERY );
        if (xRef.is())
            xRef->processDictionaryListEvent( rDicListEvent );
    }

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

    if (rMyManager.pSpellDsp)
        rMyManager.pSpellDsp->FlushSpellCache();
    if (nLngSvcEvt)
        LaunchEvent( nLngSvcEvt );
}


void LngSvcMgrListenerHelper::LaunchEvent( sal_Int16 nLngSvcEvtFlags )
{
    linguistic2::LinguServiceEvent aEvt(
        static_cast<com::sun::star::linguistic2::XLinguServiceManager*>(&rMyManager), nLngSvcEvtFlags );

    // pass event on to linguistic2::XLinguServiceEventListener's
    cppu::OInterfaceIteratorHelper aIt( aLngSvcMgrListeners );
    while (aIt.hasMoreElements())
    {
        uno::Reference< linguistic2::XLinguServiceEventListener > xRef( aIt.next(), uno::UNO_QUERY );
        if (xRef.is())
            xRef->processLinguServiceEvent( aEvt );
    }
}


inline sal_Bool LngSvcMgrListenerHelper::AddLngSvcMgrListener(
        const uno::Reference< lang::XEventListener >& rxListener )
{
    aLngSvcMgrListeners.addInterface( rxListener );
    return sal_True;
}


inline sal_Bool LngSvcMgrListenerHelper::RemoveLngSvcMgrListener(
        const uno::Reference< lang::XEventListener >& rxListener )
{
    aLngSvcMgrListeners.removeInterface( rxListener );
    return sal_True;
}


void LngSvcMgrListenerHelper::DisposeAndClear( const lang::EventObject &rEvtObj )
{
    // call "disposing" for all listeners and clear list
    aLngSvcMgrListeners   .disposeAndClear( rEvtObj );

    // remove references to this object hold by the broadcasters
    cppu::OInterfaceIteratorHelper aIt( aLngSvcEvtBroadcasters );
    while (aIt.hasMoreElements())
    {
        uno::Reference< linguistic2::XLinguServiceEventBroadcaster > xRef( aIt.next(), uno::UNO_QUERY );
        if (xRef.is())
            RemoveLngSvcEvtBroadcaster( xRef );
    }

    // remove refernce to this object hold by the dictionary-list
    if (xDicList.is())
    {
        xDicList->removeDictionaryListEventListener(
            (linguistic2::XDictionaryListEventListener *) this );
        xDicList = 0;
    }
}


sal_Bool LngSvcMgrListenerHelper::AddLngSvcEvtBroadcaster(
        const uno::Reference< linguistic2::XLinguServiceEventBroadcaster > &rxBroadcaster )
{
    sal_Bool bRes = sal_False;
    if (rxBroadcaster.is())
    {
        aLngSvcEvtBroadcasters.addInterface( rxBroadcaster );
        rxBroadcaster->addLinguServiceEventListener(
                (linguistic2::XLinguServiceEventListener *) this );
    }
    return bRes;
}


sal_Bool LngSvcMgrListenerHelper::RemoveLngSvcEvtBroadcaster(
        const uno::Reference< linguistic2::XLinguServiceEventBroadcaster > &rxBroadcaster )
{
    sal_Bool bRes = sal_False;
    if (rxBroadcaster.is())
    {
        aLngSvcEvtBroadcasters.removeInterface( rxBroadcaster );
        rxBroadcaster->removeLinguServiceEventListener(
                (linguistic2::XLinguServiceEventListener *) this );
    }
    return bRes;
}




LngSvcMgr::LngSvcMgr()
    : utl::ConfigItem("Office.Linguistic")
    , aEvtListeners(GetLinguMutex())
{
    bDisposing = sal_False;

    pSpellDsp   = 0;
    pGrammarDsp = 0;
    pHyphDsp    = 0;
    pThesDsp    = 0;

    pAvailSpellSvcs     = 0;
    pAvailGrammarSvcs   = 0;
    pAvailHyphSvcs      = 0;
    pAvailThesSvcs      = 0;
    pListenerHelper     = 0;

    // request notify events when properties (i.e. something in the subtree) changes
    uno::Sequence< OUString > aNames(4);
    OUString *pNames = aNames.getArray();
    pNames[0] = "ServiceManager/SpellCheckerList";
    pNames[1] = "ServiceManager/GrammarCheckerList";
    pNames[2] = "ServiceManager/HyphenatorList";
    pNames[3] = "ServiceManager/ThesaurusList";
    EnableNotification( aNames );

    UpdateAll();

    aUpdateTimer.SetTimeout(500);
    aUpdateTimer.SetTimeoutHdl(LINK(this, LngSvcMgr, updateAndBroadcast));

    // request to be notified if an extension has been added/removed
    uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());

    uno::Reference<deployment::XExtensionManager> xExtensionManager;
    try {
        xExtensionManager = deployment::ExtensionManager::get(xContext);
    } catch ( const deployment::DeploymentException & ) {
        SAL_WARN( "linguistic", "no extension manager - should fire on mobile only" );
    }
    if (xExtensionManager.is())
    {
        xMB = uno::Reference<util::XModifyBroadcaster>(xExtensionManager, uno::UNO_QUERY_THROW);

        uno::Reference<util::XModifyListener> xListener(this);
        xMB->addModifyListener( xListener );
    }
}

// ::com::sun::star::util::XModifyListener
void LngSvcMgr::modified(const lang::EventObject&)
    throw(uno::RuntimeException)
{
    osl::MutexGuard aGuard(GetLinguMutex());
    //assume that if an extension has been added/removed that
    //it might be a dictionary extension, so drop our cache

    clearSvcInfoArray(pAvailSpellSvcs);
    clearSvcInfoArray(pAvailGrammarSvcs);
    clearSvcInfoArray(pAvailHyphSvcs);
    clearSvcInfoArray(pAvailThesSvcs);

    //schedule in an update to execute in the main thread
    aUpdateTimer.Start();
}

//run update, and inform everyone that dictionaries (may) have changed, this
//needs to be run in the main thread because
//utl::ConfigChangeListener_Impl::changesOccurred grabs the SolarMutex and we
//get notified that an extension was added from an extension manager thread
IMPL_LINK_NOARG(LngSvcMgr, updateAndBroadcast)
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    UpdateAll();

    if (pListenerHelper)
    {
        pListenerHelper->AddLngSvcEvt(
                linguistic2::LinguServiceEventFlags::SPELL_CORRECT_WORDS_AGAIN |
                linguistic2::LinguServiceEventFlags::SPELL_WRONG_WORDS_AGAIN |
                linguistic2::LinguServiceEventFlags::PROOFREAD_AGAIN |
                linguistic2::LinguServiceEventFlags::HYPHENATE_AGAIN );
    }

    return 0;
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
    throw (uno::RuntimeException)
{
    stopListening();
}

void LngSvcMgr::clearSvcInfoArray(SvcInfoArray* &rpInfo)
{
    delete rpInfo;
    rpInfo = NULL;
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

    sal_Bool lcl_FindEntry( const OUString &rEntry, const Sequence< OUString > &rCfgSvcs )
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
            const Locale &rAvailLocale )
    {
        Sequence< OUString > aRes;

        OUString aCfgLocaleStr( MsLangId::convertLanguageToIsoString(
                                    LocaleToLanguage( rAvailLocale ) ) );

        Sequence< OUString > aNodeNames( rCfg.GetNodeNames(rLastFoundList) );
        sal_Bool bFound = lcl_FindEntry( aCfgLocaleStr, aNodeNames);

        if (bFound)
        {
            Sequence< OUString > aNames(1);
            OUString &rNodeName = aNames.getArray()[0];
            rNodeName = rLastFoundList;
            rNodeName += OUString::valueOf( (sal_Unicode)'/' );
            rNodeName += aCfgLocaleStr;
            Sequence< Any > aValues( rCfg.GetProperties( aNames ) );
            if (aValues.getLength())
            {
                OSL_ENSURE( aValues.getLength() == 1, "unexpected length of sequence" );
                Sequence< OUString > aSvcImplNames;
                if (aValues.getConstArray()[0] >>= aSvcImplNames)
                    aRes = aSvcImplNames;
                else
                {
                    OSL_FAIL( "type mismatch" );
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
            // add previously configuerd service first and append
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
        OUString aService( ::rtl::OUString::createFromAscii( apServices[k] ) );
        OUString aActiveList( ::rtl::OUString::createFromAscii( apCurLists[k] ) );
        OUString aLastFoundList( ::rtl::OUString::createFromAscii( apLastFoundLists[k] ) );
        sal_Int32 i;

        //
        // remove configured but not available language/services entries
        //
        Sequence< OUString > aNodeNames( aCfg.GetNodeNames( aActiveList ) );   // list of configured locales
        sal_Int32 nNodeNames = aNodeNames.getLength();
        const OUString *pNodeName = aNodeNames.getConstArray();
        for (i = 0;  i < nNodeNames;  ++i)
        {
            Locale aLocale( CreateLocale( MsLangId::convertIsoStringToLanguage(pNodeName[i]) ) );
            Sequence< OUString > aCfgSvcs( getConfiguredServices( aService, aLocale ));
            Sequence< OUString > aAvailSvcs( getAvailableServices( aService, aLocale ));

            aCfgSvcs = lcl_RemoveMissingEntries( aCfgSvcs, aAvailSvcs );

            aCurSvcs[k][ pNodeName[i] ] = aCfgSvcs;
        }

        //
        // add new available language/service entries
        // and
        // set last found services to currently available ones
        //
        Sequence< Locale > aAvailLocales( getAvailableLocales(aService) );
        sal_Int32 nAvailLocales = aAvailLocales.getLength();
        const Locale *pAvailLocale = aAvailLocales.getConstArray();
        for (i = 0;  i < nAvailLocales;  ++i)
        {
            OUString aCfgLocaleStr( MsLangId::convertLanguageToIsoString(
                                        LocaleToLanguage( pAvailLocale[i] ) ) );

            Sequence< OUString > aAvailSvcs( getAvailableServices( aService, pAvailLocale[i] ));

            aLastFoundSvcs[k][ aCfgLocaleStr ] = aAvailSvcs;

            Sequence< OUString > aLastSvcs(
                    lcl_GetLastFoundSvcs( aCfg, aLastFoundList , pAvailLocale[i] ));
            Sequence< OUString > aNewSvcs =
                    lcl_GetNewEntries( aLastSvcs, aAvailSvcs );

            Sequence< OUString > aCfgSvcs( aCurSvcs[k][ aCfgLocaleStr ] );

            // merge services list (previously configured to be listed first).
            aCfgSvcs = lcl_MergeSeq( aCfgSvcs, aNewSvcs );

            aCurSvcs[k][ aCfgLocaleStr ] = aCfgSvcs;
        }
    }

    //
    // write new data back to configuration
    //
    for (int k = 0;  k < nNumServices;  ++k)
    {
        for (int i = 0;  i < 2;  ++i)
        {
            const sal_Char *pSubNodeName = (i == 0) ? apCurLists[k] : apLastFoundLists[k];
            OUString aSubNodeName( ::rtl::OUString::createFromAscii(pSubNodeName) );

            list_entry_map_t &rCurMap = (i == 0) ? aCurSvcs[k] : aLastFoundSvcs[k];
            list_entry_map_t::const_iterator aIt( rCurMap.begin() );
            sal_Int32 nVals = static_cast< sal_Int32 >( rCurMap.size() );
            Sequence< PropertyValue > aNewValues( nVals );
            PropertyValue *pNewValue = aNewValues.getArray();
            while (aIt != rCurMap.end())
            {
                OUString aCfgEntryName( aSubNodeName );
                aCfgEntryName += OUString::valueOf( (sal_Unicode) '/' );
                aCfgEntryName += (*aIt).first;

                pNewValue->Name  = aCfgEntryName;
                pNewValue->Value <<= (*aIt).second;
                ++pNewValue;
                ++aIt;
            }
            OSL_ENSURE( pNewValue - aNewValues.getArray() == nVals,
                    "possible mismatch of sequence size and property number" );

            {
                // add new or replace existing entries.
                sal_Bool bRes = aCfg.ReplaceSetProperties( aSubNodeName, aNewValues );
                if (!bRes)
                {
#if OSL_DEBUG_LEVEL > 1
                    OSL_FAIL( "failed to set new configuration values" );
#endif
                }
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
        DBG_ASSERT( !aKeyText.isEmpty(), "unexpected key (lang::Locale) string" );
        if (0 == rName.compareTo( aSpellCheckerList, aSpellCheckerList.getLength() ))
        {
            // delete old cached data, needs to be acquired new on demand
			clearSvcInfoArray(pAvailSpellSvcs);

            OUString aNode( aSpellCheckerList );
            if (lcl_SeqHasString( aSpellCheckerListEntries, aKeyText ))
            {
                OUString aPropName( aNode );
                aPropName += OUString::valueOf( (sal_Unicode) '/' );
                aPropName += aKeyText;
                pNames[0] = aPropName;
                aValues = /*aCfg.*/GetProperties( aNames );
                uno::Sequence< OUString > aSvcImplNames;
                if (aValues.getLength())
                    aSvcImplNames = GetLangSvcList( aValues.getConstArray()[0] );

                LanguageType nLang = LANGUAGE_NONE;
                if (!aKeyText.isEmpty())
                    nLang = MsLangId::convertIsoStringToLanguage( aKeyText );

                GetSpellCheckerDsp_Impl( sal_False );     // don't set service list, it will be done below
                pSpellDsp->SetServiceList( CreateLocale(nLang), aSvcImplNames );
            }
        }
        else if (0 == rName.compareTo( aGrammarCheckerList, aGrammarCheckerList.getLength() ))
        {
            // delete old cached data, needs to be acquired new on demand
			clearSvcInfoArray(pAvailGrammarSvcs);

            OUString aNode( aGrammarCheckerList );
            if (lcl_SeqHasString( aGrammarCheckerListEntries, aKeyText ))
            {
                OUString aPropName( aNode );
                aPropName += OUString::valueOf( (sal_Unicode) '/' );
                aPropName += aKeyText;
                pNames[0] = aPropName;
                aValues = /*aCfg.*/GetProperties( aNames );
                uno::Sequence< OUString > aSvcImplNames;
                if (aValues.getLength())
                    aSvcImplNames = GetLangSvc( aValues.getConstArray()[0] );

                LanguageType nLang = LANGUAGE_NONE;
                if (!aKeyText.isEmpty())
                    nLang = MsLangId::convertIsoStringToLanguage( aKeyText );

                if (SvtLinguConfig().HasGrammarChecker())
                {
                    GetGrammarCheckerDsp_Impl( sal_False );   // don't set service list, it will be done below
                    pGrammarDsp->SetServiceList( CreateLocale(nLang), aSvcImplNames );
                }
            }
        }
        else if (0 == rName.compareTo( aHyphenatorList, aHyphenatorList.getLength() ))
        {
            // delete old cached data, needs to be acquired new on demand
			clearSvcInfoArray(pAvailHyphSvcs);

            OUString aNode( aHyphenatorList );
            if (lcl_SeqHasString( aHyphenatorListEntries, aKeyText ))
            {
                OUString aPropName( aNode );
                aPropName += OUString::valueOf( (sal_Unicode) '/' );
                aPropName += aKeyText;
                pNames[0] = aPropName;
                aValues = /*aCfg.*/GetProperties( aNames );
                uno::Sequence< OUString > aSvcImplNames;
                if (aValues.getLength())
                    aSvcImplNames = GetLangSvc( aValues.getConstArray()[0] );

                LanguageType nLang = LANGUAGE_NONE;
                if (!aKeyText.isEmpty())
                    nLang = MsLangId::convertIsoStringToLanguage( aKeyText );

                GetHyphenatorDsp_Impl( sal_False );   // don't set service list, it will be done below
                pHyphDsp->SetServiceList( CreateLocale(nLang), aSvcImplNames );
            }
        }
        else if (0 == rName.compareTo( aThesaurusList, aThesaurusList.getLength() ))
        {
            // delete old cached data, needs to be acquired new on demand
			clearSvcInfoArray(pAvailThesSvcs);

            OUString aNode( aThesaurusList );
            if (lcl_SeqHasString( aThesaurusListEntries, aKeyText ))
            {
                OUString aPropName( aNode );
                aPropName += OUString::valueOf( (sal_Unicode) '/' );
                aPropName += aKeyText;
                pNames[0] = aPropName;
                aValues = /*aCfg.*/GetProperties( aNames );
                uno::Sequence< OUString > aSvcImplNames;
                if (aValues.getLength())
                    aSvcImplNames = GetLangSvcList( aValues.getConstArray()[0] );

                LanguageType nLang = LANGUAGE_NONE;
                if (!aKeyText.isEmpty())
                    nLang = MsLangId::convertIsoStringToLanguage( aKeyText );

                GetThesaurusDsp_Impl( sal_False );  // don't set service list, it will be done below
                pThesDsp->SetServiceList( CreateLocale(nLang), aSvcImplNames );
            }
        }
        else
        {
            DBG_ASSERT( 0, "nofified for unexpected property" );
        }
    }
}


void LngSvcMgr::Commit()
{
    // everything necessary should have already been done by 'SaveCfgSvcs'
    // called from within 'setConfiguredServices'.
    // Also this class usually exits only when the Office i sbeing shutdown.
}


void LngSvcMgr::GetListenerHelper_Impl()
{
    if (!pListenerHelper)
    {
        pListenerHelper = new LngSvcMgrListenerHelper( *this, linguistic::GetDictionaryList() );
        xListenerHelper = (linguistic2::XLinguServiceEventListener *) pListenerHelper;
    }
}


void LngSvcMgr::GetSpellCheckerDsp_Impl( sal_Bool bSetSvcList )
{
    if (!pSpellDsp)
    {
        pSpellDsp   = new SpellCheckerDispatcher( *this );
        xSpellDsp   = pSpellDsp;
        if (bSetSvcList)
            SetCfgServiceLists( *pSpellDsp );
    }
}


void LngSvcMgr::GetGrammarCheckerDsp_Impl( sal_Bool bSetSvcList  )
{
    if (!pGrammarDsp && SvtLinguConfig().HasGrammarChecker())
    {
        //! since the grammar checking iterator needs to be a one instance service
        //! we need to create it the correct way!
        uno::Reference< linguistic2::XProofreadingIterator > xGCI;
        try
        {
            uno::Reference< lang::XMultiServiceFactory > xMgr(
                    comphelper::getProcessServiceFactory(), uno::UNO_QUERY_THROW );
            xGCI = uno::Reference< linguistic2::XProofreadingIterator >(
                    xMgr->createInstance( SN_GRAMMARCHECKINGITERATOR ), uno::UNO_QUERY_THROW );
        }
        catch (uno::Exception &)
        {
        }
        DBG_ASSERT( xGCI.is(), "instantiating grammar checking iterator failed" );

        if (xGCI.is())
        {
            pGrammarDsp    = dynamic_cast< GrammarCheckingIterator * >(xGCI.get());
            xGrammarDsp    = xGCI;
            DBG_ASSERT( pGrammarDsp, "failed to get implementation" );
            if (bSetSvcList)
                SetCfgServiceLists( *pGrammarDsp );
        }
    }
}


void LngSvcMgr::GetHyphenatorDsp_Impl( sal_Bool bSetSvcList  )
{
    if (!pHyphDsp)
    {
        pHyphDsp    = new HyphenatorDispatcher( *this );
        xHyphDsp    = pHyphDsp;
        if (bSetSvcList)
            SetCfgServiceLists( *pHyphDsp );
    }
}


void LngSvcMgr::GetThesaurusDsp_Impl( sal_Bool bSetSvcList  )
{
    if (!pThesDsp)
    {
        pThesDsp    = new ThesaurusDispatcher;
        xThesDsp    = pThesDsp;
        if (bSetSvcList)
            SetCfgServiceLists( *pThesDsp );
    }
}


void LngSvcMgr::GetAvailableSpellSvcs_Impl()
{
    if (!pAvailSpellSvcs)
    {
        pAvailSpellSvcs = new SvcInfoArray;

        uno::Reference< lang::XMultiServiceFactory >  xFac( comphelper::getProcessServiceFactory() );
        if (xFac.is())
        {
            uno::Reference< container::XContentEnumerationAccess > xEnumAccess( xFac, uno::UNO_QUERY );
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
                    if ( cppu::extractInterface( xCompFactory, aCurrent ) || ::cppu::extractInterface( xFactory, aCurrent ) )
                    {
                        try
                        {
                            uno::Reference < uno::XComponentContext > xContext(
                                comphelper::getComponentContext( xFac ) );
                            xSvc = uno::Reference< linguistic2::XSpellChecker >( ( xCompFactory.is() ? xCompFactory->createInstanceWithContext( xContext ) : xFactory->createInstance() ), uno::UNO_QUERY );
                        }
                        catch (const uno::Exception &)
                        {
                            DBG_ASSERT( 0, "createInstance failed" );
                        }
                    }

                    if (xSvc.is())
                    {
                        OUString            aImplName;
                        uno::Sequence< sal_Int16 >    aLanguages;
                        uno::Reference< XServiceInfo > xInfo( xSvc, uno::UNO_QUERY );
                        if (xInfo.is())
                            aImplName = xInfo->getImplementationName();
                        DBG_ASSERT( !aImplName.isEmpty(),
                                "empty implementation name" );
                        uno::Reference< linguistic2::XSupportedLocales > xSuppLoc( xSvc, uno::UNO_QUERY );
                        DBG_ASSERT( xSuppLoc.is(), "interfaces not supported" );
                        if (xSuppLoc.is()) {
                            uno::Sequence<lang::Locale> aLocaleSequence(xSuppLoc->getLocales());
                            aLanguages = LocaleSeqToLangSeq( aLocaleSequence );
                        }

                        pAvailSpellSvcs->push_back( new SvcInfo( aImplName, aLanguages ) );
                    }
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

        uno::Reference< lang::XMultiServiceFactory >  xFac( comphelper::getProcessServiceFactory() );
        if (xFac.is())
        {
            uno::Reference< container::XContentEnumerationAccess > xEnumAccess( xFac, uno::UNO_QUERY );
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
                    if ( cppu::extractInterface( xCompFactory, aCurrent ) || ::cppu::extractInterface( xFactory, aCurrent ) )
                    {
                        try
                        {
                            uno::Reference < uno::XComponentContext > xContext(
                                comphelper::getComponentContext( xFac ) );
                            xSvc = uno::Reference< linguistic2::XProofreader >( ( xCompFactory.is() ? xCompFactory->createInstanceWithContext( xContext ) : xFactory->createInstance() ), uno::UNO_QUERY );
                        }
                        catch (const uno::Exception &)
                        {
                            DBG_ASSERT( 0, "createInstance failed" );
                        }
                    }

                    if (xSvc.is())
                    {
                        OUString            aImplName;
                        uno::Sequence< sal_Int16 >   aLanguages;
                        uno::Reference< XServiceInfo > xInfo( xSvc, uno::UNO_QUERY );
                        if (xInfo.is())
                            aImplName = xInfo->getImplementationName();
                        DBG_ASSERT( !aImplName.isEmpty(),
                                "empty implementation name" );
                        uno::Reference< linguistic2::XSupportedLocales > xSuppLoc( xSvc, uno::UNO_QUERY );
                        DBG_ASSERT( xSuppLoc.is(), "interfaces not supported" );
                        if (xSuppLoc.is()) {
                            uno::Sequence<lang::Locale> aLocaleSequence(xSuppLoc->getLocales());
                            aLanguages = LocaleSeqToLangSeq( aLocaleSequence );
                        }

                        pAvailGrammarSvcs->push_back( new SvcInfo( aImplName, aLanguages ) );
                    }
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
        uno::Reference< lang::XMultiServiceFactory >  xFac( comphelper::getProcessServiceFactory() );
        if (xFac.is())
        {
            uno::Reference< container::XContentEnumerationAccess > xEnumAccess( xFac, uno::UNO_QUERY );
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
                    if ( cppu::extractInterface( xCompFactory, aCurrent ) || ::cppu::extractInterface( xFactory, aCurrent ) )
                    {
                        try
                        {
                            uno::Reference < uno::XComponentContext > xContext(
                                comphelper::getComponentContext( xFac ) );
                            xSvc = uno::Reference< linguistic2::XHyphenator >( ( xCompFactory.is() ? xCompFactory->createInstanceWithContext( xContext ) : xFactory->createInstance() ), uno::UNO_QUERY );

                        }
                        catch (const uno::Exception &)
                        {
                            DBG_ASSERT( 0, "createInstance failed" );
                        }
                    }

                    if (xSvc.is())
                    {
                        OUString            aImplName;
                        uno::Sequence< sal_Int16 >    aLanguages;
                        uno::Reference< XServiceInfo > xInfo( xSvc, uno::UNO_QUERY );
                        if (xInfo.is())
                            aImplName = xInfo->getImplementationName();
                        DBG_ASSERT( !aImplName.isEmpty(),
                                "empty implementation name" );
                        uno::Reference< linguistic2::XSupportedLocales > xSuppLoc( xSvc, uno::UNO_QUERY );
                        DBG_ASSERT( xSuppLoc.is(), "interfaces not supported" );
                        if (xSuppLoc.is()) {
                            uno::Sequence<lang::Locale> aLocaleSequence(xSuppLoc->getLocales());
                            aLanguages = LocaleSeqToLangSeq( aLocaleSequence );
                        }

                        pAvailHyphSvcs->push_back( new SvcInfo( aImplName, aLanguages ) );
                    }
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

        uno::Reference< lang::XMultiServiceFactory >  xFac( comphelper::getProcessServiceFactory() );
        if (xFac.is())
        {
            uno::Reference< container::XContentEnumerationAccess > xEnumAccess( xFac, uno::UNO_QUERY );
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
                    if ( cppu::extractInterface( xCompFactory, aCurrent ) || ::cppu::extractInterface( xFactory, aCurrent ) )
                    {
                        try
                        {
                            uno::Reference < uno::XComponentContext > xContext(
                                comphelper::getComponentContext( xFac ) );
                            xSvc = uno::Reference< linguistic2::XThesaurus >( ( xCompFactory.is() ? xCompFactory->createInstanceWithContext( xContext ) : xFactory->createInstance() ), uno::UNO_QUERY );
                        }
                        catch (const uno::Exception &)
                        {
                            DBG_ASSERT( 0, "createInstance failed" );
                        }
                    }

                    if (xSvc.is())
                    {
                        OUString            aImplName;
                        uno::Sequence< sal_Int16 >    aLanguages;
                        uno::Reference< XServiceInfo > xInfo( xSvc, uno::UNO_QUERY );
                        if (xInfo.is())
                            aImplName = xInfo->getImplementationName();
                        DBG_ASSERT( !aImplName.isEmpty(),
                                "empty implementation name" );
                        uno::Reference< linguistic2::XSupportedLocales > xSuppLoc( xSvc, uno::UNO_QUERY );
                        DBG_ASSERT( xSuppLoc.is(), "interfaces not supported" );
                        if (xSuppLoc.is()) {
                            uno::Sequence<lang::Locale> aLocaleSequence(xSuppLoc->getLocales());
                            aLanguages = LocaleSeqToLangSeq( aLocaleSequence );
                        }

                        pAvailThesSvcs->push_back( new SvcInfo( aImplName, aLanguages ) );
                    }
                }
            }
        }
    }
}


void LngSvcMgr::SetCfgServiceLists( SpellCheckerDispatcher &rSpellDsp )
{
    RTL_LOGFILE_CONTEXT( aLog, "linguistic: LngSvcMgr::SetCfgServiceLists - Spell" );

    rtl::OUString aNode("ServiceManager/SpellCheckerList");
    uno::Sequence< OUString > aNames( /*aCfg.*/GetNodeNames( aNode ) );
    OUString *pNames = aNames.getArray();
    sal_Int32 nLen = aNames.getLength();

    // append path prefix need for 'GetProperties' call below
    String aPrefix( aNode );
    aPrefix.Append( (sal_Unicode) '/' );
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
                String aLocaleStr( pNames[i] );
                xub_StrLen nSeperatorPos = aLocaleStr.SearchBackward( sal_Unicode( '/' ) );
                aLocaleStr = aLocaleStr.Copy( nSeperatorPos + 1 );
                lang::Locale aLocale( CreateLocale( MsLangId::convertIsoStringToLanguage(aLocaleStr) ) );
                rSpellDsp.SetServiceList( aLocale, aSvcImplNames );
            }
        }
    }
}


void LngSvcMgr::SetCfgServiceLists( GrammarCheckingIterator &rGrammarDsp )
{
    RTL_LOGFILE_CONTEXT( aLog, "linguistic: LngSvcMgr::SetCfgServiceLists - Grammar" );

    rtl::OUString aNode("ServiceManager/GrammarCheckerList");
    uno::Sequence< OUString > aNames( /*aCfg.*/GetNodeNames( aNode ) );
    OUString *pNames = aNames.getArray();
    sal_Int32 nLen = aNames.getLength();

    // append path prefix need for 'GetProperties' call below
    String aPrefix( aNode );
    aPrefix.Append( (sal_Unicode) '/' );
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

                String aLocaleStr( pNames[i] );
                xub_StrLen nSeperatorPos = aLocaleStr.SearchBackward( sal_Unicode( '/' ) );
                aLocaleStr = aLocaleStr.Copy( nSeperatorPos + 1 );
                lang::Locale aLocale( CreateLocale( MsLangId::convertIsoStringToLanguage(aLocaleStr) ) );
                rGrammarDsp.SetServiceList( aLocale, aSvcImplNames );
            }
        }
    }
}


void LngSvcMgr::SetCfgServiceLists( HyphenatorDispatcher &rHyphDsp )
{
    RTL_LOGFILE_CONTEXT( aLog, "linguistic: LngSvcMgr::SetCfgServiceLists - Hyph" );

    rtl::OUString aNode("ServiceManager/HyphenatorList");
    uno::Sequence< OUString > aNames( /*aCfg.*/GetNodeNames( aNode ) );
    OUString *pNames = aNames.getArray();
    sal_Int32 nLen = aNames.getLength();

    // append path prefix need for 'GetProperties' call below
    String aPrefix( aNode );
    aPrefix.Append( (sal_Unicode) '/' );
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

                String aLocaleStr( pNames[i] );
                xub_StrLen nSeperatorPos = aLocaleStr.SearchBackward( sal_Unicode( '/' ) );
                aLocaleStr = aLocaleStr.Copy( nSeperatorPos + 1 );
                lang::Locale aLocale( CreateLocale( MsLangId::convertIsoStringToLanguage(aLocaleStr) ) );
                rHyphDsp.SetServiceList( aLocale, aSvcImplNames );
            }
        }
    }
}


void LngSvcMgr::SetCfgServiceLists( ThesaurusDispatcher &rThesDsp )
{
    RTL_LOGFILE_CONTEXT( aLog, "linguistic: LngSvcMgr::SetCfgServiceLists - Thes" );

    rtl::OUString aNode("ServiceManager/ThesaurusList");
    uno::Sequence< OUString > aNames( /*aCfg.*/GetNodeNames( aNode ) );
    OUString *pNames = aNames.getArray();
    sal_Int32 nLen = aNames.getLength();

    // append path prefix need for 'GetProperties' call below
    String aPrefix( aNode );
    aPrefix.Append( (sal_Unicode) '/' );
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
                String aLocaleStr( pNames[i] );
                xub_StrLen nSeperatorPos = aLocaleStr.SearchBackward( sal_Unicode( '/' ) );
                aLocaleStr = aLocaleStr.Copy( nSeperatorPos + 1 );
                lang::Locale aLocale( CreateLocale( MsLangId::convertIsoStringToLanguage(aLocaleStr) ) );
                rThesDsp.SetServiceList( aLocale, aSvcImplNames );
            }
        }
    }
}


uno::Reference< linguistic2::XSpellChecker > SAL_CALL
    LngSvcMgr::getSpellChecker()
        throw(uno::RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );
#if OSL_DEBUG_LEVEL > 1
    getAvailableLocales( SN_SPELLCHECKER );
#endif

    uno::Reference< linguistic2::XSpellChecker > xRes;
    if (!bDisposing)
    {
        if (!xSpellDsp.is())
            GetSpellCheckerDsp_Impl();
        xRes = xSpellDsp;
    }
    return xRes;
}


uno::Reference< linguistic2::XHyphenator > SAL_CALL
    LngSvcMgr::getHyphenator()
        throw(uno::RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );
#if OSL_DEBUG_LEVEL > 1
    getAvailableLocales( SN_HYPHENATOR );
#endif

    uno::Reference< linguistic2::XHyphenator >   xRes;
    if (!bDisposing)
    {
        if (!xHyphDsp.is())
            GetHyphenatorDsp_Impl();
        xRes = xHyphDsp;
    }
    return xRes;
}


uno::Reference< linguistic2::XThesaurus > SAL_CALL
    LngSvcMgr::getThesaurus()
        throw(uno::RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );
#if OSL_DEBUG_LEVEL > 1
    getAvailableLocales( SN_THESAURUS );
#endif

    uno::Reference< linguistic2::XThesaurus >    xRes;
    if (!bDisposing)
    {
        if (!xThesDsp.is())
            GetThesaurusDsp_Impl();
        xRes = xThesDsp;
    }
    return xRes;
}


sal_Bool SAL_CALL
    LngSvcMgr::addLinguServiceManagerListener(
            const uno::Reference< lang::XEventListener >& xListener )
        throw(uno::RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    sal_Bool bRes = sal_False;
    if (!bDisposing  &&  xListener.is())
    {
        if (!pListenerHelper)
            GetListenerHelper_Impl();
        bRes = pListenerHelper->AddLngSvcMgrListener( xListener );
    }
    return bRes;
}


sal_Bool SAL_CALL
    LngSvcMgr::removeLinguServiceManagerListener(
            const uno::Reference< lang::XEventListener >& xListener )
        throw(uno::RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    sal_Bool bRes = sal_False;
    if (!bDisposing  &&  xListener.is())
    {
        DBG_ASSERT( pListenerHelper, "listener removed without being added" );
        if (!pListenerHelper)
            GetListenerHelper_Impl();
        bRes = pListenerHelper->RemoveLngSvcMgrListener( xListener );
    }
    return bRes;
}


uno::Sequence< OUString > SAL_CALL
    LngSvcMgr::getAvailableServices(
            const OUString& rServiceName,
            const lang::Locale& rLocale )
        throw(uno::RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    uno::Sequence< OUString > aRes;
    const SvcInfoArray *pInfoArray = 0;

    if (0 == rServiceName.compareToAscii( SN_SPELLCHECKER ))
    {
        GetAvailableSpellSvcs_Impl();
        pInfoArray = pAvailSpellSvcs;
    }
    else if (0 == rServiceName.compareToAscii( SN_GRAMMARCHECKER ))
    {
        GetAvailableGrammarSvcs_Impl();
        pInfoArray = pAvailGrammarSvcs;
    }
    else if (0 == rServiceName.compareToAscii( SN_HYPHENATOR ))
    {
        GetAvailableHyphSvcs_Impl();
        pInfoArray = pAvailHyphSvcs;
    }
    else if (0 == rServiceName.compareToAscii( SN_THESAURUS ))
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
        LanguageType nLanguage = LocaleToLanguage( rLocale );
        for (size_t i = 0;  i < nMaxCnt; ++i)
        {
            const SvcInfo &rInfo = (*pInfoArray)[i];
            if (LANGUAGE_NONE == nLanguage
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
        throw(uno::RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    uno::Sequence< lang::Locale > aRes;

    uno::Sequence< lang::Locale >  *pAvailLocales     = NULL;
    if (0 == rServiceName.compareToAscii( SN_SPELLCHECKER ))
        pAvailLocales       = &aAvailSpellLocales;
    else if (0 == rServiceName.compareToAscii( SN_GRAMMARCHECKER ))
        pAvailLocales       = &aAvailGrammarLocales;
    else if (0 == rServiceName.compareToAscii( SN_HYPHENATOR ))
        pAvailLocales       = &aAvailHyphLocales;
    else if (0 == rServiceName.compareToAscii( SN_THESAURUS ))
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

static sal_Bool IsEqSvcList( const uno::Sequence< OUString > &rList1,
                        const uno::Sequence< OUString > &rList2 )
{
    // returns sal_True iff both sequences are equal

    sal_Bool bRes = sal_False;
    sal_Int32 nLen = rList1.getLength();
    if (rList2.getLength() == nLen)
    {
        const OUString *pStr1 = rList1.getConstArray();
        const OUString *pStr2 = rList2.getConstArray();
        bRes = sal_True;
        for (sal_Int32 i = 0;  i < nLen  &&  bRes;  ++i)
        {
            if (*pStr1++ != *pStr2++)
                bRes = sal_False;
        }
    }
    return bRes;
}


void SAL_CALL
    LngSvcMgr::setConfiguredServices(
            const OUString& rServiceName,
            const lang::Locale& rLocale,
            const uno::Sequence< OUString >& rServiceImplNames )
        throw(uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT( aLog, "linguistic: LngSvcMgr::setConfiguredServices" );

    osl::MutexGuard aGuard( GetLinguMutex() );

#if OSL_DEBUG_LEVEL > 1
#endif

    LanguageType nLanguage = LocaleToLanguage( rLocale );
    if (LANGUAGE_NONE != nLanguage)
    {
        if (0 == rServiceName.compareToAscii( SN_SPELLCHECKER ))
        {
            if (!xSpellDsp.is())
                GetSpellCheckerDsp_Impl();
            sal_Bool bChanged = !IsEqSvcList( rServiceImplNames,
                                          pSpellDsp->GetServiceList( rLocale ) );
            if (bChanged)
            {
                pSpellDsp->SetServiceList( rLocale, rServiceImplNames );
                SaveCfgSvcs( rtl::OUString(SN_SPELLCHECKER) );

                if (pListenerHelper  &&  bChanged)
                    pListenerHelper->AddLngSvcEvt(
                            linguistic2::LinguServiceEventFlags::SPELL_CORRECT_WORDS_AGAIN |
                            linguistic2::LinguServiceEventFlags::SPELL_WRONG_WORDS_AGAIN );
            }
        }
        else if (0 == rServiceName.compareToAscii( SN_GRAMMARCHECKER ))
        {
            if (!xGrammarDsp.is())
                GetGrammarCheckerDsp_Impl();
            sal_Bool bChanged = !IsEqSvcList( rServiceImplNames,
                                          pGrammarDsp->GetServiceList( rLocale ) );
            if (bChanged)
            {
                pGrammarDsp->SetServiceList( rLocale, rServiceImplNames );
                SaveCfgSvcs( rtl::OUString(SN_GRAMMARCHECKER) );

                if (pListenerHelper  &&  bChanged)
                    pListenerHelper->AddLngSvcEvt(
                            linguistic2::LinguServiceEventFlags::PROOFREAD_AGAIN );
            }
        }
        else if (0 == rServiceName.compareToAscii( SN_HYPHENATOR ))
        {
            if (!xHyphDsp.is())
                GetHyphenatorDsp_Impl();
            sal_Bool bChanged = !IsEqSvcList( rServiceImplNames,
                                          pHyphDsp->GetServiceList( rLocale ) );
            if (bChanged)
            {
                pHyphDsp->SetServiceList( rLocale, rServiceImplNames );
                SaveCfgSvcs( rtl::OUString(SN_HYPHENATOR) );

                if (pListenerHelper  &&  bChanged)
                    pListenerHelper->AddLngSvcEvt(
                            linguistic2::LinguServiceEventFlags::HYPHENATE_AGAIN );
            }
        }
        else if (0 == rServiceName.compareToAscii( SN_THESAURUS ))
        {
            if (!xThesDsp.is())
                GetThesaurusDsp_Impl();
            sal_Bool bChanged = !IsEqSvcList( rServiceImplNames,
                                          pThesDsp->GetServiceList( rLocale ) );
            if (bChanged)
            {
                pThesDsp->SetServiceList( rLocale, rServiceImplNames );
                SaveCfgSvcs( rtl::OUString(SN_THESAURUS) );
            }
        }
    }
}


sal_Bool LngSvcMgr::SaveCfgSvcs( const String &rServiceName )
{
    RTL_LOGFILE_CONTEXT( aLog, "linguistic: LngSvcMgr::SaveCfgSvcs" );

    sal_Bool bRes = sal_False;

    LinguDispatcher *pDsp = 0;
    uno::Sequence< lang::Locale > aLocales;

    if (0 == rServiceName.CompareToAscii( SN_SPELLCHECKER ))
    {
        if (!pSpellDsp)
            GetSpellCheckerDsp_Impl();
        pDsp = pSpellDsp;
        aLocales = getAvailableLocales( SN_SPELLCHECKER );
    }
    else if (0 == rServiceName.CompareToAscii( SN_GRAMMARCHECKER ))
    {
        if (!pGrammarDsp)
            GetGrammarCheckerDsp_Impl();
        pDsp = pGrammarDsp;
        aLocales = getAvailableLocales( SN_GRAMMARCHECKER );
    }
    else if (0 == rServiceName.CompareToAscii( SN_HYPHENATOR ))
    {
        if (!pHyphDsp)
            GetHyphenatorDsp_Impl();
        pDsp = pHyphDsp;
        aLocales = getAvailableLocales( SN_HYPHENATOR );
    }
    else if (0 == rServiceName.CompareToAscii( SN_THESAURUS ))
    {
        if (!pThesDsp)
            GetThesaurusDsp_Impl();
        pDsp = pThesDsp;
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
        const char *pNodeName = NULL;
        if (pDsp == pSpellDsp)
            pNodeName = "ServiceManager/SpellCheckerList";
        else if (pDsp == pGrammarDsp)
            pNodeName = "ServiceManager/GrammarCheckerList";
        else if (pDsp == pHyphDsp)
            pNodeName = "ServiceManager/HyphenatorList";
        else if (pDsp == pThesDsp)
            pNodeName = "ServiceManager/ThesaurusList";
        else
        {
            DBG_ASSERT( 0, "node name missing" );
        }
        OUString aNodeName( ::rtl::OUString::createFromAscii(pNodeName) );

        for (sal_Int32 i = 0;  i < nLen;  ++i)
        {
            uno::Sequence< OUString > aSvcImplNames;
            aSvcImplNames = pDsp->GetServiceList( pLocale[i] );

#if OSL_DEBUG_LEVEL > 1
            sal_Int32 nSvcs = aSvcImplNames.getLength();
            const OUString *pSvcImplName = aSvcImplNames.getConstArray();
            for (sal_Int32 j = 0;  j < nSvcs;  ++j)
            {
                OUString aImplName( pSvcImplName[j] );
            }
#endif
            // build value to be written back to configuration
            uno::Any aCfgAny;
            if ((pDsp == pHyphDsp || pDsp == pGrammarDsp) && aSvcImplNames.getLength() > 1)
                aSvcImplNames.realloc(1);   // there should be only one entry for hyphenators or grammar checkers (because they are not chained)
            aCfgAny <<= aSvcImplNames;
            DBG_ASSERT( aCfgAny.hasValue(), "missing value for 'Any' type" );

            OUString aCfgLocaleStr( MsLangId::convertLanguageToIsoString(
                                        LocaleToLanguage( pLocale[i] ) ) );
            pValue->Value = aCfgAny;
            pValue->Name  = aNodeName;
            pValue->Name += OUString::valueOf( (sal_Unicode) '/' );
            pValue->Name += aCfgLocaleStr;
            pValue++;
        }
        {
        RTL_LOGFILE_CONTEXT( aLog, "linguistic: LngSvcMgr::SaveCfgSvcs - ReplaceSetProperties" );
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
#if OSL_DEBUG_LEVEL > 1
        sal_Int32 nSvcs = aRes.getLength();
        if (nSvcs)
        {
            const OUString *pSvcName = aRes.getConstArray();
            for (sal_Int32 j = 0;  j < nSvcs;  ++j)
            {
                OUString aImplName( pSvcName[j] );
                DBG_ASSERT( !aImplName.isEmpty(), "service impl-name missing" );
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
            DBG_ASSERT( 0, "GetLangSvc: unexpected type encountered" );
        }
    }

    return aRes;
}



uno::Sequence< OUString > SAL_CALL
    LngSvcMgr::getConfiguredServices(
            const OUString& rServiceName,
            const lang::Locale& rLocale )
        throw(uno::RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    uno::Sequence< OUString > aSvcImplNames;

    LanguageType nLanguage = LocaleToLanguage( rLocale );
    OUString aCfgLocale( MsLangId::convertLanguageToIsoString( nLanguage ) );

    uno::Sequence< uno::Any > aValues;
    uno::Sequence< OUString > aNames( 1 );
    OUString *pNames = aNames.getArray();
    if ( 0 == rServiceName.compareToAscii( SN_SPELLCHECKER ) )
    {
        OUString aNode( "ServiceManager/SpellCheckerList");
        const uno::Sequence< OUString > aNodeEntries( GetNodeNames( aNode ) );
        if (lcl_SeqHasString( aNodeEntries, aCfgLocale ))
        {
            OUString aPropName( aNode );
            aPropName += OUString::valueOf( (sal_Unicode) '/' );
            aPropName += aCfgLocale;
            pNames[0] = aPropName;
            aValues = /*aCfg.*/GetProperties( aNames );
            if (aValues.getLength())
                aSvcImplNames = GetLangSvcList( aValues.getConstArray()[0] );
        }
    }
    else if ( 0 == rServiceName.compareToAscii( SN_GRAMMARCHECKER ) )
    {
        OUString aNode( "ServiceManager/GrammarCheckerList");
        const uno::Sequence< OUString > aNodeEntries( GetNodeNames( aNode ) );
        if (lcl_SeqHasString( aNodeEntries, aCfgLocale ))
        {
            OUString aPropName( aNode );
            aPropName += OUString::valueOf( (sal_Unicode) '/' );
            aPropName += aCfgLocale;
            pNames[0] = aPropName;
            aValues = /*aCfg.*/GetProperties( aNames );
            if (aValues.getLength())
                aSvcImplNames = GetLangSvc( aValues.getConstArray()[0] );
        }
    }
    else if ( 0 == rServiceName.compareToAscii( SN_HYPHENATOR ) )
    {
        OUString aNode( "ServiceManager/HyphenatorList");
        const uno::Sequence< OUString > aNodeEntries( GetNodeNames( aNode ) );
        if (lcl_SeqHasString( aNodeEntries, aCfgLocale ))
        {
            OUString aPropName( aNode );
            aPropName += OUString::valueOf( (sal_Unicode) '/' );
            aPropName += aCfgLocale;
            pNames[0] = aPropName;
            aValues = /*aCfg.*/GetProperties( aNames );
            if (aValues.getLength())
                aSvcImplNames = GetLangSvc( aValues.getConstArray()[0] );
        }
    }
    else if ( 0 == rServiceName.compareToAscii( SN_THESAURUS ) )
    {
        OUString aNode( "ServiceManager/ThesaurusList");
        const uno::Sequence< OUString > aNodeEntries( GetNodeNames( aNode ) );
        if (lcl_SeqHasString( aNodeEntries, aCfgLocale ))
        {
            OUString aPropName( aNode );
            aPropName += OUString::valueOf( (sal_Unicode) '/' );
            aPropName += aCfgLocale;
            pNames[0] = aPropName;
            aValues = /*aCfg.*/GetProperties( aNames );
            if (aValues.getLength())
                aSvcImplNames = GetLangSvcList( aValues.getConstArray()[0] );
        }
    }

#if OSL_DEBUG_LEVEL > 1
    const OUString *pImplNames = aSvcImplNames.getConstArray();
    (void) pImplNames;
#endif
    return aSvcImplNames;
}


void SAL_CALL
    LngSvcMgr::dispose()
        throw(uno::RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    if (!bDisposing)
    {
        bDisposing = sal_True;

        // require listeners to release this object
        lang::EventObject aEvtObj( static_cast<XLinguServiceManager*>(this) );
        aEvtListeners.disposeAndClear( aEvtObj );

        if (pListenerHelper)
            pListenerHelper->DisposeAndClear( aEvtObj );
    }
}


void SAL_CALL
    LngSvcMgr::addEventListener(
            const uno::Reference< lang::XEventListener >& xListener )
        throw(uno::RuntimeException)
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
        throw(uno::RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    if (xListener.is())
    {
        aEvtListeners.removeInterface( xListener );
    }
}


sal_Bool LngSvcMgr::AddLngSvcEvtBroadcaster(
            const uno::Reference< linguistic2::XLinguServiceEventBroadcaster > &rxBroadcaster )
{
    sal_Bool bRes = sal_False;
    if (rxBroadcaster.is())
    {
        if (!pListenerHelper)
            GetListenerHelper_Impl();
        bRes = pListenerHelper->AddLngSvcEvtBroadcaster( rxBroadcaster );
    }
    return bRes;
}


OUString SAL_CALL
    LngSvcMgr::getImplementationName()
        throw(uno::RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );
    return getImplementationName_Static();
}


sal_Bool SAL_CALL
    LngSvcMgr::supportsService( const OUString& ServiceName )
        throw(uno::RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    uno::Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;
    return sal_False;
}


uno::Sequence< OUString > SAL_CALL
    LngSvcMgr::getSupportedServiceNames()
        throw(uno::RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );
    return getSupportedServiceNames_Static();
}


uno::Sequence< OUString > LngSvcMgr::getSupportedServiceNames_Static()
        throw()
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    uno::Sequence< OUString > aSNS( 1 );    // more than 1 service possible
    aSNS.getArray()[0] = SN_LINGU_SERVCICE_MANAGER;
    return aSNS;
}


uno::Reference< uno::XInterface > SAL_CALL LngSvcMgr_CreateInstance(
            const uno::Reference< lang::XMultiServiceFactory > & /*rSMgr*/ )
        throw(uno::Exception)
{
    uno::Reference< uno::XInterface > xService = (cppu::OWeakObject*) new LngSvcMgr;
    return xService;
}

void * SAL_CALL LngSvcMgr_getFactory(
            const sal_Char * pImplName,
            lang::XMultiServiceFactory * pServiceManager,
            void * /*pRegistryKey*/ )
{

    void * pRet = 0;
    if ( !LngSvcMgr::getImplementationName_Static().compareToAscii( pImplName ) )
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
