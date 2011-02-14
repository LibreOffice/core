/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_linguistic.hxx"

#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/linguistic2/XSupportedLocales.hpp>
#include <com/sun/star/linguistic2/DictionaryListEventFlags.hpp>
#include <com/sun/star/linguistic2/LinguServiceEventFlags.hpp>

#include <tools/solar.h>
#include <unotools/lingucfg.hxx>
#include <unotools/processfactory.hxx>
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

///////////////////////////////////////////////////////////////////////////

static sal_Bool lcl_SeqHasString( const uno::Sequence< OUString > &rSeq, const OUString &rText )
{
    sal_Bool bRes = sal_False;

    sal_Int32 nLen = rSeq.getLength();
    if (nLen == 0 || rText.getLength() == 0)
        return bRes;

    const OUString *pSeq = rSeq.getConstArray();
    for (sal_Int32 i = 0;  i < nLen  &&  !bRes;  ++i)
    {
        if (rText == pSeq[i])
            bRes = sal_True;
    }
    return bRes;
}

///////////////////////////////////////////////////////////////////////////

static uno::Sequence< lang::Locale > GetAvailLocales(
        const uno::Sequence< OUString > &rSvcImplNames )
{
    uno::Sequence< lang::Locale > aRes;

    uno::Reference< lang::XMultiServiceFactory >  xFac( utl::getProcessServiceFactory() );
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

///////////////////////////////////////////////////////////////////////////

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


///////////////////////////////////////////////////////////////////////////


void LngSvcMgr::SetAvailableCfgServiceLists( LinguDispatcher &rDispatcher,
        const SvcInfoArray &rAvailSvcs )
{
    // get list of nodenames to look at for their service list
    const char *pEntryName = 0;
    sal_Bool bHasLangSvcList = sal_True;
    switch (rDispatcher.GetDspType())
    {
        case LinguDispatcher::DSP_SPELL     : pEntryName = "ServiceManager/SpellCheckerList";    break;
        case LinguDispatcher::DSP_GRAMMAR   : pEntryName = "ServiceManager/GrammarCheckerList";
                                              bHasLangSvcList = sal_False;
                                              break;
        case LinguDispatcher::DSP_HYPH      : pEntryName = "ServiceManager/HyphenatorList";
                                              bHasLangSvcList = sal_False;
                                              break;
        case LinguDispatcher::DSP_THES      : pEntryName = "ServiceManager/ThesaurusList";  break;
        default :
            DBG_ASSERT( 0, "unexpected case" );
    }
    String  aNode( String::CreateFromAscii( pEntryName ) );
    uno::Sequence < OUString > aNodeNames( /*aCfg.*/GetNodeNames( aNode ) );


    sal_Int32 nLen = aNodeNames.getLength();
    const OUString *pNodeNames = aNodeNames.getConstArray();
    for (sal_Int32 i = 0;  i < nLen;  ++i)
    {
        uno::Sequence< OUString >   aSvcImplNames;

        uno::Sequence< OUString >    aNames( 1 );
        OUString *pNames = aNames.getArray();

        OUString aPropName( aNode );
        aPropName += OUString::valueOf( (sal_Unicode) '/' );
        aPropName += pNodeNames[i];
        pNames[0] = aPropName;

        uno::Sequence< uno::Any > aValues = /*aCfg.*/GetProperties( aNames );
        if (aValues.getLength())
        {
            // get list of configured service names for the
            // current node (language)
            const uno::Any &rValue = aValues.getConstArray()[0];
            if (bHasLangSvcList)
                aSvcImplNames = GetLangSvcList( rValue );
            else
                aSvcImplNames = GetLangSvc( rValue );

            sal_Int32 nSvcs = aSvcImplNames.getLength();
            if (nSvcs)
            {
                const OUString *pImplNames = aSvcImplNames.getConstArray();

                LanguageType nLang = MsLangId::convertIsoStringToLanguage( pNodeNames[i] );

                // build list of available services from those
                sal_Int32 nCnt = 0;
                uno::Sequence< OUString > aAvailSvcs( nSvcs );
                OUString *pAvailSvcs = aAvailSvcs.getArray();
                for (sal_Int32 k = 0;  k < nSvcs;  ++k)
                {
                    // check for availability of the service
                    size_t nAvailSvcs = rAvailSvcs.size();
                    for (size_t m = 0;  m < nAvailSvcs;  ++m)
                    {
                        const SvcInfo &rSvcInfo = *rAvailSvcs[m];
                        if (rSvcInfo.aSvcImplName == pImplNames[k]  &&
                            rSvcInfo.HasLanguage( nLang ))
                        {
                            pAvailSvcs[ nCnt++ ] = rSvcInfo.aSvcImplName;
                            break;
                        }
                    }
                }

                if (nCnt)
                {
                    aAvailSvcs.realloc( nCnt );
                    rDispatcher.SetServiceList( CreateLocale( nLang ), aAvailSvcs );
                }
            }
        }
    }
}


///////////////////////////////////////////////////////////////////////////


class LngSvcMgrListenerHelper :
    public cppu::WeakImplHelper2
    <
        linguistic2::XLinguServiceEventListener,
        linguistic2::XDictionaryListEventListener
    >
{
    LngSvcMgr  &rMyManager;
//    Timer       aLaunchTimer;

    //cppu::OMultiTypeInterfaceContainerHelper  aListeners;
    ::cppu::OInterfaceContainerHelper           aLngSvcMgrListeners;
    ::cppu::OInterfaceContainerHelper           aLngSvcEvtBroadcasters;
    uno::Reference< linguistic2::XDictionaryList >               xDicList;
    uno::Reference< uno::XInterface >                        xMyEvtObj;

    sal_Int16   nCombinedLngSvcEvt;

    // disallow copy-constructor and assignment-operator for now
    LngSvcMgrListenerHelper(const LngSvcMgrListenerHelper &);
    LngSvcMgrListenerHelper & operator = (const LngSvcMgrListenerHelper &);

    void    LaunchEvent( sal_Int16 nLngSvcEvtFlags );

//  DECL_LINK( TimeOut, Timer* );
    long Timeout();

public:
    LngSvcMgrListenerHelper( LngSvcMgr &rLngSvcMgr,
            const uno::Reference< uno::XInterface > &rxSource,
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
        const uno::Reference< uno::XInterface > &rxSource,
        const uno::Reference< linguistic2::XDictionaryList > &rxDicList  ) :
    rMyManager              ( rLngSvcMgr ),
    aLngSvcMgrListeners     ( GetLinguMutex() ),
    aLngSvcEvtBroadcasters  ( GetLinguMutex() ),
    xDicList                ( rxDicList ),
    xMyEvtObj               ( rxSource )
{
    if (xDicList.is())
    {
        xDicList->addDictionaryListEventListener(
            (linguistic2::XDictionaryListEventListener *) this, sal_False );
    }

    //! The timer is used to 'sum up' different events in order to reduce the
    //! number of events forwarded.
    //! (This may happen already if a property was changed that has several
    //! listeners, and each of them is launching an event of it's own!)
    //! Thus this behaviour is necessary to avoid unecessary actions of
    //! this objects listeners!
//  aLaunchTimer.SetTimeout( 2000 );
//  aLaunchTimer.SetTimeoutHdl( LINK( this, LngSvcMgrListenerHelper, TimeOut ) );
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


//IMPL_LINK( LngSvcMgrListenerHelper, TimeOut, Timer*, pTimer )
long LngSvcMgrListenerHelper::Timeout()
{
    osl::MutexGuard aGuard( GetLinguMutex() );

//  if (&aLaunchTimer == pTimer)
    {
        // change event source to LinguServiceManager since the listeners
        // probably do not know (and need not to know) about the specific
        // SpellChecker's or Hyphenator's.
        linguistic2::LinguServiceEvent aEvtObj( xMyEvtObj, nCombinedLngSvcEvt );
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
//  aLaunchTimer.Start();
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

    //
    // "translate" DictionaryList event into linguistic2::LinguServiceEvent
    //
    sal_Int16 nLngSvcEvt = 0;
    //
    sal_Int16 nSpellCorrectFlags =
            linguistic2::DictionaryListEventFlags::ADD_NEG_ENTRY        |
            linguistic2::DictionaryListEventFlags::DEL_POS_ENTRY        |
            linguistic2::DictionaryListEventFlags::ACTIVATE_NEG_DIC |
            linguistic2::DictionaryListEventFlags::DEACTIVATE_POS_DIC;
    if (0 != (nDlEvt & nSpellCorrectFlags))
        nLngSvcEvt |= linguistic2::LinguServiceEventFlags::SPELL_CORRECT_WORDS_AGAIN;
    //
    sal_Int16 nSpellWrongFlags =
            linguistic2::DictionaryListEventFlags::ADD_POS_ENTRY        |
            linguistic2::DictionaryListEventFlags::DEL_NEG_ENTRY        |
            linguistic2::DictionaryListEventFlags::ACTIVATE_POS_DIC |
            linguistic2::DictionaryListEventFlags::DEACTIVATE_NEG_DIC;
    if (0 != (nDlEvt & nSpellWrongFlags))
        nLngSvcEvt |= linguistic2::LinguServiceEventFlags::SPELL_WRONG_WORDS_AGAIN;
    //
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
    linguistic2::LinguServiceEvent aEvt( xMyEvtObj, nLngSvcEvtFlags );

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


///////////////////////////////////////////////////////////////////////////


LngSvcMgr::LngSvcMgr() :
    utl::ConfigItem( String::CreateFromAscii( "Office.Linguistic" ) ),
    aEvtListeners   ( GetLinguMutex() )
{
    bHasAvailSpellLocales   =
    bHasAvailGrammarLocales =
    bHasAvailHyphLocales    =
    bHasAvailThesLocales    =
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
    pNames[0] = A2OU( "ServiceManager/SpellCheckerList" );
    pNames[1] = A2OU( "ServiceManager/GrammarCheckerList" );
    pNames[2] = A2OU( "ServiceManager/HyphenatorList" );
    pNames[3] = A2OU( "ServiceManager/ThesaurusList" );
    EnableNotification( aNames );
}

void LngSvcMgr::clearSvcInfoArray(SvcInfoArray* pInfo)
{
    if (pInfo)
    {
        std::for_each(pInfo->begin(), pInfo->end(), boost::checked_deleter<SvcInfo>());
        delete pInfo;
    }
}

LngSvcMgr::~LngSvcMgr()
{
    // memory for pSpellDsp, pHyphDsp, pThesDsp, pListenerHelper
    // will be freed in the destructor of the respective Reference's
    // xSpellDsp, xGrammarDsp, xHyphDsp, xThesDsp

    clearSvcInfoArray(pAvailSpellSvcs);
    clearSvcInfoArray(pAvailGrammarSvcs);
    clearSvcInfoArray(pAvailHyphSvcs);
    clearSvcInfoArray(pAvailThesSvcs);
}


void LngSvcMgr::Notify( const uno::Sequence< OUString > &rPropertyNames )
{
    const OUString aSpellCheckerList( A2OU("ServiceManager/SpellCheckerList") );
    const OUString aGrammarCheckerList( A2OU("ServiceManager/GrammarCheckerList") );
    const OUString aHyphenatorList( A2OU("ServiceManager/HyphenatorList") );
    const OUString aThesaurusList( A2OU("ServiceManager/ThesaurusList") );

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
        DBG_ASSERT( aKeyText.getLength() != 0, "unexpected key (lang::Locale) string" );
        if (0 == rName.compareTo( aSpellCheckerList, aSpellCheckerList.getLength() ))
        {
            // delete old cached data, needs to be acquired new on demand
            clearSvcInfoArray(pAvailSpellSvcs);     pAvailSpellSvcs = 0;

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
                if (0 != aKeyText.getLength())
                    nLang = MsLangId::convertIsoStringToLanguage( aKeyText );

                GetSpellCheckerDsp_Impl( sal_False );     // don't set service list, it will be done below
                pSpellDsp->SetServiceList( CreateLocale(nLang), aSvcImplNames );
            }
        }
        else if (0 == rName.compareTo( aGrammarCheckerList, aGrammarCheckerList.getLength() ))
        {
            // delete old cached data, needs to be acquired new on demand
            clearSvcInfoArray(pAvailGrammarSvcs);      pAvailGrammarSvcs = 0;

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
                if (0 != aKeyText.getLength())
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
            clearSvcInfoArray(pAvailHyphSvcs);      pAvailHyphSvcs = 0;

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
                if (0 != aKeyText.getLength())
                    nLang = MsLangId::convertIsoStringToLanguage( aKeyText );

                GetHyphenatorDsp_Impl( sal_False );   // don't set service list, it will be done below
                pHyphDsp->SetServiceList( CreateLocale(nLang), aSvcImplNames );
            }
        }
        else if (0 == rName.compareTo( aThesaurusList, aThesaurusList.getLength() ))
        {
            // delete old cached data, needs to be acquired new on demand
            clearSvcInfoArray(pAvailThesSvcs);      pAvailThesSvcs = 0;

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
                if (0 != aKeyText.getLength())
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
        pListenerHelper = new LngSvcMgrListenerHelper( *this,
                (XLinguServiceManager *) this, linguistic::GetDictionaryList() );
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
                    utl::getProcessServiceFactory(), uno::UNO_QUERY_THROW );
            xGCI = uno::Reference< linguistic2::XProofreadingIterator >(
                    xMgr->createInstance( A2OU( SN_GRAMMARCHECKINGITERATOR ) ), uno::UNO_QUERY_THROW );
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

        uno::Reference< lang::XMultiServiceFactory >  xFac( utl::getProcessServiceFactory() );
        if (xFac.is())
        {
            uno::Reference< container::XContentEnumerationAccess > xEnumAccess( xFac, uno::UNO_QUERY );
            uno::Reference< container::XEnumeration > xEnum;
            if (xEnumAccess.is())
                xEnum = xEnumAccess->createContentEnumeration(
                        A2OU( SN_SPELLCHECKER ) );

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
                            uno::Reference < uno::XComponentContext > xContext;
                            uno::Reference< beans::XPropertySet > xProps( xFac, uno::UNO_QUERY );

                            xProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))) >>= xContext;
                            xSvc = uno::Reference< linguistic2::XSpellChecker >( ( xCompFactory.is() ? xCompFactory->createInstanceWithContext( xContext ) : xFactory->createInstance() ), uno::UNO_QUERY );
                        }
                        catch (uno::Exception &rEx)
                        {
                            (void) rEx;
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
                        DBG_ASSERT( aImplName.getLength(),
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

        uno::Reference< lang::XMultiServiceFactory >  xFac( utl::getProcessServiceFactory() );
        if (xFac.is())
        {
            uno::Reference< container::XContentEnumerationAccess > xEnumAccess( xFac, uno::UNO_QUERY );
            uno::Reference< container::XEnumeration > xEnum;
            if (xEnumAccess.is())
                xEnum = xEnumAccess->createContentEnumeration(
                        A2OU( SN_GRAMMARCHECKER ) );

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
                            uno::Reference < uno::XComponentContext > xContext;
                            uno::Reference< beans::XPropertySet > xProps( xFac, uno::UNO_QUERY );

                            xProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))) >>= xContext;
                            xSvc = uno::Reference< linguistic2::XProofreader >( ( xCompFactory.is() ? xCompFactory->createInstanceWithContext( xContext ) : xFactory->createInstance() ), uno::UNO_QUERY );
                        }
                        catch (uno::Exception &rEx)
                        {
                            (void) rEx;
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
                        DBG_ASSERT( aImplName.getLength(),
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
        uno::Reference< lang::XMultiServiceFactory >  xFac( utl::getProcessServiceFactory() );
        if (xFac.is())
        {
            uno::Reference< container::XContentEnumerationAccess > xEnumAccess( xFac, uno::UNO_QUERY );
            uno::Reference< container::XEnumeration > xEnum;
            if (xEnumAccess.is())
                xEnum = xEnumAccess->createContentEnumeration( A2OU( SN_HYPHENATOR ) );

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
                            uno::Reference < uno::XComponentContext > xContext;
                            uno::Reference< beans::XPropertySet > xProps( xFac, uno::UNO_QUERY );

                            xProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))) >>= xContext;
                            xSvc = uno::Reference< linguistic2::XHyphenator >( ( xCompFactory.is() ? xCompFactory->createInstanceWithContext( xContext ) : xFactory->createInstance() ), uno::UNO_QUERY );

                        }
                        catch (uno::Exception &rEx)
                        {
                            (void) rEx;
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
                        DBG_ASSERT( aImplName.getLength(),
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

        uno::Reference< lang::XMultiServiceFactory >  xFac( utl::getProcessServiceFactory() );
        if (xFac.is())
        {
            uno::Reference< container::XContentEnumerationAccess > xEnumAccess( xFac, uno::UNO_QUERY );
            uno::Reference< container::XEnumeration > xEnum;
            if (xEnumAccess.is())
                xEnum = xEnumAccess->createContentEnumeration(
                        A2OU( SN_THESAURUS ) );

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
                            uno::Reference < uno::XComponentContext > xContext;
                            uno::Reference< beans::XPropertySet > xProps( xFac, uno::UNO_QUERY );

                            xProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))) >>= xContext;
                            xSvc = uno::Reference< linguistic2::XThesaurus >( ( xCompFactory.is() ? xCompFactory->createInstanceWithContext( xContext ) : xFactory->createInstance() ), uno::UNO_QUERY );
                        }
                        catch (uno::Exception &rEx)
                        {
                            (void) rEx;
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
                        DBG_ASSERT( aImplName.getLength(),
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

    String  aNode( String::CreateFromAscii( "ServiceManager/SpellCheckerList" ) );
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
#if OSL_DEBUG_LEVEL > 1
//                sal_Int32 nSvcs = aSvcImplNames.getLength();
//                const OUString *pSvcImplNames = aSvcImplNames.getConstArray();
#endif
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

    String  aNode( String::CreateFromAscii( "ServiceManager/GrammarCheckerList" ) );
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

#if OSL_DEBUG_LEVEL > 1
//                sal_Int32 nSvcs = aSvcImplNames.getLength();
//                const OUString *pSvcImplNames = aSvcImplNames.getConstArray();
#endif
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

    String  aNode( String::CreateFromAscii( "ServiceManager/HyphenatorList" ) );
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

#if OSL_DEBUG_LEVEL > 1
//                sal_Int32 nSvcs = aSvcImplNames.getLength();
//                const OUString *pSvcImplNames = aSvcImplNames.getConstArray();
#endif
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

    String  aNode( String::CreateFromAscii( "ServiceManager/ThesaurusList" ) );
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
#if OSL_DEBUG_LEVEL > 1
//                sal_Int32 nSvcs = aSvcImplNames.getLength();
//                const OUString *pSvcImplNames = aSvcImplNames.getConstArray();
#endif
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
    getAvailableLocales( A2OU( SN_SPELLCHECKER ));
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
    getAvailableLocales( A2OU( SN_HYPHENATOR ));
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
    getAvailableLocales( A2OU( SN_THESAURUS ));
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
        // don't used cached data here (force re-evaluation in order to have downloaded dictionaries
        // already found without the need to restart the office
        clearSvcInfoArray(pAvailSpellSvcs);  pAvailSpellSvcs = 0;
        GetAvailableSpellSvcs_Impl();
        pInfoArray = pAvailSpellSvcs;
    }
    else if (0 == rServiceName.compareToAscii( SN_GRAMMARCHECKER ))
    {
        // don't used cached data here (force re-evaluation in order to have downloaded dictionaries
        // already found without the need to restart the office
        clearSvcInfoArray(pAvailGrammarSvcs);  pAvailGrammarSvcs = 0;
        GetAvailableGrammarSvcs_Impl();
        pInfoArray = pAvailGrammarSvcs;
    }
    else if (0 == rServiceName.compareToAscii( SN_HYPHENATOR ))
    {
        // don't used cached data here (force re-evaluation in order to have downloaded dictionaries
        // already found without the need to restart the office
        clearSvcInfoArray(pAvailHyphSvcs);  pAvailHyphSvcs = 0;
        GetAvailableHyphSvcs_Impl();
        pInfoArray = pAvailHyphSvcs;
    }
    else if (0 == rServiceName.compareToAscii( SN_THESAURUS ))
    {
        // don't used cached data here (force re-evaluation in order to have downloaded dictionaries
        // already found without the need to restart the office
        clearSvcInfoArray(pAvailThesSvcs);  pAvailThesSvcs = 0;
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
        for (size_t i = 0;  i < nMaxCnt;  ++i)
        {
            const SvcInfo *pInfo = (*pInfoArray)[i];
            if (LANGUAGE_NONE == nLanguage
                || (pInfo && pInfo->HasLanguage( nLanguage )))
            {
                pImplName[ nCnt++ ] = pInfo->aSvcImplName;
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
    sal_Bool                *pHasAvailLocales   = NULL;
    if (0 == rServiceName.compareToAscii( SN_SPELLCHECKER ))
    {
        pAvailLocales       = &aAvailSpellLocales;
        pHasAvailLocales    = &bHasAvailSpellLocales;
    }
    else if (0 == rServiceName.compareToAscii( SN_GRAMMARCHECKER ))
    {
        pAvailLocales       = &aAvailGrammarLocales;
        pHasAvailLocales    = &bHasAvailGrammarLocales;
    }
    else if (0 == rServiceName.compareToAscii( SN_HYPHENATOR ))
    {
        pAvailLocales       = &aAvailHyphLocales;
        pHasAvailLocales    = &bHasAvailHyphLocales;
    }
    else if (0 == rServiceName.compareToAscii( SN_THESAURUS ))
    {
        pAvailLocales       = &aAvailThesLocales;
        pHasAvailLocales    = &bHasAvailThesLocales;
    }

    // about pHasAvailLocales: nowadays (with OOo lingu in SO) we want to know immediately about
    // new downloaded dictionaries and have them ready right away if the Tools/Options...
    // is used to activate them. Thus we can not rely anymore on buffered data.
    if (pAvailLocales  /*&&  pHasAvailLocales */)
    {
//      if (!*pHasAvailLocales)
//      {
            *pAvailLocales = GetAvailLocales(
                    getAvailableServices( rServiceName, lang::Locale() ) );
//          *pHasAvailLocales = sal_True;
//      }
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
//    const OUString *pImplNames = rServiceImplNames.getConstArray();
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
                SaveCfgSvcs( A2OU( SN_SPELLCHECKER ) );

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
                SaveCfgSvcs( A2OU( SN_GRAMMARCHECKER ) );

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
                SaveCfgSvcs( A2OU( SN_HYPHENATOR ) );

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
                SaveCfgSvcs( A2OU( SN_THESAURUS ) );
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
        aLocales = getAvailableLocales( A2OU( SN_SPELLCHECKER ) );
    }
    else if (0 == rServiceName.CompareToAscii( SN_GRAMMARCHECKER ))
    {
        if (!pGrammarDsp)
            GetGrammarCheckerDsp_Impl();
        pDsp = pGrammarDsp;
        aLocales = getAvailableLocales( A2OU( SN_GRAMMARCHECKER ) );
    }
    else if (0 == rServiceName.CompareToAscii( SN_HYPHENATOR ))
    {
        if (!pHyphDsp)
            GetHyphenatorDsp_Impl();
        pDsp = pHyphDsp;
        aLocales = getAvailableLocales( A2OU( SN_HYPHENATOR ) );
    }
    else if (0 == rServiceName.CompareToAscii( SN_THESAURUS ))
    {
        if (!pThesDsp)
            GetThesaurusDsp_Impl();
        pDsp = pThesDsp;
        aLocales = getAvailableLocales( A2OU( SN_THESAURUS ) );
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
        OUString aNodeName( A2OU(pNodeName) );

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
                DBG_ASSERT( aImplName.getLength(), "service impl-name missing" );
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
        if ((rVal >>= aImplName) && aImplName.getLength() != 0)
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


///////////////////////////////////////////////////////////////////////////

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
        OUString aNode( OUString::createFromAscii( "ServiceManager/SpellCheckerList" ));
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
        OUString aNode( OUString::createFromAscii( "ServiceManager/GrammarCheckerList" ));
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
        OUString aNode( OUString::createFromAscii( "ServiceManager/HyphenatorList" ));
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
        OUString aNode( OUString::createFromAscii( "ServiceManager/ThesaurusList" ));
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
        lang::EventObject aEvtObj( (XLinguServiceManager *) this );
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


sal_Bool LngSvcMgr::RemoveLngSvcEvtBroadcaster(
            const uno::Reference< linguistic2::XLinguServiceEventBroadcaster > &rxBroadcaster )
{
    sal_Bool bRes = sal_False;
    if (rxBroadcaster.is())
    {
        DBG_ASSERT( pListenerHelper, "pListenerHelper non existent" );
        if (!pListenerHelper)
            GetListenerHelper_Impl();
        bRes = pListenerHelper->RemoveLngSvcEvtBroadcaster( rxBroadcaster );
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

    uno::Sequence< OUString > aSNS( 1 );    // auch mehr als 1 Service moeglich
    aSNS.getArray()[0] = A2OU( SN_LINGU_SERVCICE_MANAGER );
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


///////////////////////////////////////////////////////////////////////////

