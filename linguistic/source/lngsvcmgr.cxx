/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lngsvcmgr.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 16:28:25 $
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
#include "precompiled_linguistic.hxx"

#include <cppuhelper/factory.hxx>   // helper for factories
#include <com/sun/star/registry/XRegistryKey.hpp>
#ifndef _COM_SUN_STAR_CONTAINER_XCONTENTENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XSUPPORTEDLOCALES_HPP_
#include <com/sun/star/linguistic2/XSupportedLocales.hpp>
#endif

#include <com/sun/star/linguistic2/DictionaryListEventFlags.hpp>
#include <com/sun/star/linguistic2/LinguServiceEventFlags.hpp>

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef INCLUDED_I18NPOOL_MSLANGID_HXX
#include <i18npool/mslangid.hxx>
#endif
#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif
#ifndef _SVTOOLS_LINGUCFG_HXX_
#include <svtools/lingucfg.hxx>
#endif
#include <cppuhelper/extract.hxx>
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif
#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

#include "lngsvcmgr.hxx"
#include "lngopt.hxx"
#include "misc.hxx"
#include "spelldsp.hxx"
#include "hyphdsp.hxx"
#include "thesdsp.hxx"


using namespace utl;
using namespace osl;
using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;


SV_DECL_VARARR_SORT( SortedINT16Array, INT16, 32, 32)
SV_IMPL_VARARR_SORT( SortedINT16Array, INT16 );

// forward declarations
Sequence< OUString > static GetLangSvcList( const Any &rVal );
Sequence< OUString > static GetLangSvc( const Any &rVal );

///////////////////////////////////////////////////////////////////////////

static BOOL lcl_SeqHasString( const Sequence< OUString > &rSeq, const OUString &rText )
{
    BOOL bRes = FALSE;

    INT32 nLen = rSeq.getLength();
    if (nLen == 0 || rText.getLength() == 0)
        return bRes;

    const OUString *pSeq = rSeq.getConstArray();
    for (INT32 i = 0;  i < nLen  &&  !bRes;  ++i)
    {
        if (rText == pSeq[i])
            bRes = TRUE;
    }
    return bRes;
}

///////////////////////////////////////////////////////////////////////////

static Sequence< Locale > GetAvailLocales(
        const Sequence< OUString > &rSvcImplNames )
{
    Sequence< Locale > aRes;

    Reference< XMultiServiceFactory >  xFac( getProcessServiceFactory() );
    INT32 nNames = rSvcImplNames.getLength();
    if (nNames  &&  xFac.is())
    {
        SortedINT16Array aLanguages;

        //! since we're going to create one-instance services we have to
        //! supply their arguments even if we would not need them here...
        Sequence< Any > aArgs(2);
        aArgs.getArray()[0] <<= GetLinguProperties();

        // check all services for the supported languages and new
        // languages to the result
        const OUString *pImplNames = rSvcImplNames.getConstArray();
        INT32 i;

        for ( i = 0;  i < nNames;  ++i)
        {
            Reference< XSupportedLocales > xSuppLoc(
                    xFac->createInstanceWithArguments( pImplNames[i], aArgs ),
                    UNO_QUERY );
            if (xSuppLoc.is())
            {
                Sequence< Locale > aLoc( xSuppLoc->getLocales() );
                INT32 nLoc = aLoc.getLength();
                for (INT32 k = 0;  k < nLoc;  ++k)
                {
                    const Locale *pLoc = aLoc.getConstArray();
                    INT16 nLang = LocaleToLanguage( pLoc[k] );

                    // language not already added?
                    if (!aLanguages.Seek_Entry( nLang ))
                        aLanguages.Insert( nLang );
                }
            }
            else
                DBG_ERROR( "interface not supported by service" );
        }

        // build return sequence
        INT16 nLanguages = aLanguages.Count();
        aRes.realloc( nLanguages );
        Locale *pRes = aRes.getArray();
        for (i = 0;  i < nLanguages;  ++i)
        {
            INT16 nLang = aLanguages[(USHORT) i];
            pRes[i] = CreateLocale( nLang );
        }
    }

    return aRes;
}

///////////////////////////////////////////////////////////////////////////

struct SvcInfo
{
    const ::rtl::OUString                           aSvcImplName;
    const ::com::sun::star::uno::Sequence< INT16 >  aSuppLanguages;

    SvcInfo( const ::rtl::OUString &rSvcImplName,
             const ::com::sun::star::uno::Sequence< INT16 > &rSuppLanguages ) :
        aSvcImplName    (rSvcImplName),
        aSuppLanguages  (rSuppLanguages)
    {
    }

    BOOL    HasLanguage( INT16 nLanguage ) const;
};


BOOL SvcInfo::HasLanguage( INT16 nLanguage ) const
{
    INT32 nCnt = aSuppLanguages.getLength();
    const INT16 *pLang = aSuppLanguages.getConstArray();
    INT32 i;

    for ( i = 0;  i < nCnt;  ++i)
    {
        if (nLanguage == pLang[i])
            break;
    }
    return i < nCnt;
}


typedef SvcInfo * PTR_SVCINFO;
SV_DECL_PTRARR_DEL( SvcInfoArray, PTR_SVCINFO, 16, 16 )
SV_IMPL_PTRARR( SvcInfoArray, PTR_SVCINFO * );


///////////////////////////////////////////////////////////////////////////


#ifdef NEVER

// not used anymore (see SetAvailableCfgServiceLists)

static void SetAvailableServiceLists( LinguDispatcher &rDispatcher,
        const SvcInfoArray &rInfoArray )
{
    USHORT nSvcs = rInfoArray.Count();

    // build list of all available languages
    SortedINT16Array aLanguages;
    USHORT i;
    for (i = 0;  i < nSvcs;  ++i)
    {
        const Sequence< INT16 > &rSuppLang = rInfoArray[i]->aSuppLanguages;
        INT32 nLang = rSuppLang.getLength();
        const INT16 *pSuppLang = rSuppLang.getConstArray();
        for (INT32 j = 0;  j < nLang;  ++j)
        {
            // language not already added?
            if (!aLanguages.Seek_Entry( pSuppLang[j] ))
                aLanguages.Insert( pSuppLang[j] );
        }
    }

    // set service list per language to all available services supporting
    // that language
    INT16 nLanguages = aLanguages.Count();
    for (i = 0;  i < nLanguages;  ++i)
    {
        INT16 nActLang = aLanguages[i];
        Sequence< OUString > aSvcImplNames( nSvcs );
        OUString *pSvcImplName = aSvcImplNames.getArray();
        INT32 nSeqCnt = 0;
        for (USHORT j = 0;  j < nSvcs;  ++j)
        {
            const SvcInfo &rSvcInfo = *rInfoArray[j];
            if (rSvcInfo.HasLanguage( nActLang ))
                pSvcImplName[ nSeqCnt++ ] = rSvcInfo.aSvcImplName;
        }
        aSvcImplNames.realloc( nSeqCnt );

        rDispatcher.SetServiceList( CreateLocale( nActLang ), aSvcImplNames );
    }
}

#endif


void LngSvcMgr::SetAvailableCfgServiceLists( LinguDispatcher &rDispatcher,
        const SvcInfoArray &rAvailSvcs )
{
    //SvtLinguConfig aCfg;

    // get list of nodenames to look at for their service list
    const char *pEntryName = 0;
    BOOL bHasLangSvcList = TRUE;
    switch (rDispatcher.GetDspType())
    {
        case LinguDispatcher::DSP_SPELL : pEntryName = "ServiceManager/SpellCheckerList";    break;
        case LinguDispatcher::DSP_HYPH  : pEntryName = "ServiceManager/HyphenatorList";
                                          bHasLangSvcList = FALSE;
                                          break;
        case LinguDispatcher::DSP_THES  : pEntryName = "ServiceManager/ThesaurusList";  break;
        default :
            DBG_ERROR( "unexpected case" );
    }
    String  aNode( String::CreateFromAscii( pEntryName ) );
    Sequence < OUString > aNodeNames( /*aCfg.*/GetNodeNames( aNode ) );


    INT32 nLen = aNodeNames.getLength();
    const OUString *pNodeNames = aNodeNames.getConstArray();
    for (INT32 i = 0;  i < nLen;  ++i)
    {
        Sequence< OUString >    aSvcImplNames;

        Sequence< OUString >    aNames( 1 );
        OUString *pNames = aNames.getArray();

        OUString aPropName( aNode );
        aPropName += OUString::valueOf( (sal_Unicode) '/' );
        aPropName += pNodeNames[i];
        pNames[0] = aPropName;

        Sequence< Any > aValues = /*aCfg.*/GetProperties( aNames );
        if (aValues.getLength())
        {
            // get list of configured service names for the
            // current node (language)
            const Any &rValue = aValues.getConstArray()[0];
            if (bHasLangSvcList)
                aSvcImplNames = GetLangSvcList( rValue );
            else
                aSvcImplNames = GetLangSvc( rValue );

            INT32 nSvcs = aSvcImplNames.getLength();
            if (nSvcs)
            {
                const OUString *pImplNames = aSvcImplNames.getConstArray();

                INT16 nLang = MsLangId::convertIsoStringToLanguage( pNodeNames[i] );

                // build list of available services from those
                INT32 nCnt = 0;
                Sequence< OUString > aAvailSvcs( nSvcs );
                OUString *pAvailSvcs = aAvailSvcs.getArray();
                for (INT32 k = 0;  k < nSvcs;  ++k)
                {
                    // check for availability of the service
                    USHORT nAvailSvcs = rAvailSvcs.Count();
                    for (USHORT m = 0;  m < nAvailSvcs;  ++m)
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
        XLinguServiceEventListener,
        XDictionaryListEventListener
    >
{
    Timer aLaunchTimer;

    //cppu::OMultiTypeInterfaceContainerHelper  aListeners;
    ::cppu::OInterfaceContainerHelper           aLngSvcMgrListeners;
    ::cppu::OInterfaceContainerHelper           aLngSvcEvtBroadcasters;
    Reference< XDictionaryList >                xDicList;
    Reference< XInterface >                     xMyEvtObj;

    INT16   nCombinedLngSvcEvt;

    // disallow copy-constructor and assignment-operator for now
    LngSvcMgrListenerHelper(const LngSvcMgrListenerHelper &);
    LngSvcMgrListenerHelper & operator = (const LngSvcMgrListenerHelper &);

    void    LaunchEvent( INT16 nLngSvcEvtFlags );

    DECL_LINK( TimeOut, Timer* );

public:
    LngSvcMgrListenerHelper( const Reference< XInterface > &rxSource,
            const Reference< XDictionaryList > &rxDicList );

    // XEventListener
    virtual void SAL_CALL
        disposing( const EventObject& rSource )
            throw(RuntimeException);

    // XLinguServiceEventListener
    virtual void SAL_CALL
        processLinguServiceEvent( const LinguServiceEvent& aLngSvcEvent )
            throw(RuntimeException);

    // XDictionaryListEventListener
    virtual void SAL_CALL
        processDictionaryListEvent(
                const DictionaryListEvent& rDicListEvent )
            throw(RuntimeException);

    inline  BOOL    AddLngSvcMgrListener(
                        const Reference< XEventListener >& rxListener );
    inline  BOOL    RemoveLngSvcMgrListener(
                        const Reference< XEventListener >& rxListener );
    void    DisposeAndClear( const EventObject &rEvtObj );
    BOOL    AddLngSvcEvtBroadcaster(
                        const Reference< XLinguServiceEventBroadcaster > &rxBroadcaster );
    BOOL    RemoveLngSvcEvtBroadcaster(
                        const Reference< XLinguServiceEventBroadcaster > &rxBroadcaster );

    void    AddLngSvcEvt( INT16 nLngSvcEvt );
};


LngSvcMgrListenerHelper::LngSvcMgrListenerHelper(
        const Reference< XInterface > &rxSource,
        const Reference< XDictionaryList > &rxDicList  ) :
    aLngSvcMgrListeners     ( GetLinguMutex() ),
    aLngSvcEvtBroadcasters  ( GetLinguMutex() ),
    xDicList                ( rxDicList ),
    xMyEvtObj               ( rxSource )
{
    if (xDicList.is())
    {
        xDicList->addDictionaryListEventListener(
            (XDictionaryListEventListener *) this, FALSE );
    }

    //! The timer is used to 'sum up' different events in order to reduce the
    //! number of events forwarded.
    //! (This may happen already if a property was changed that has several
    //! listeners, and each of them is launching an event of it's own!)
    //! Thus this behaviour is necessary to avoid unecessary actions of
    //! this objects listeners!
    aLaunchTimer.SetTimeout( 2000 );
    aLaunchTimer.SetTimeoutHdl( LINK( this, LngSvcMgrListenerHelper, TimeOut ) );
    nCombinedLngSvcEvt = 0;
}


void SAL_CALL LngSvcMgrListenerHelper::disposing( const EventObject& rSource )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    Reference< XInterface > xRef( rSource.Source );
    if ( xRef.is() )
    {
        aLngSvcMgrListeners   .removeInterface( xRef );
        aLngSvcEvtBroadcasters.removeInterface( xRef );
        if (xDicList == xRef)
            xDicList = 0;
    }
}


IMPL_LINK( LngSvcMgrListenerHelper, TimeOut, Timer*, pTimer )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (&aLaunchTimer == pTimer)
    {
        // change event source to LinguServiceManager since the listeners
        // probably do not know (and need not to know) about the specific
        // SpellChecker's or Hyphenator's.
        LinguServiceEvent aEvtObj( xMyEvtObj, nCombinedLngSvcEvt );
        nCombinedLngSvcEvt = 0;

        // pass event on to XLinguServiceEventListener's
        cppu::OInterfaceIteratorHelper aIt( aLngSvcMgrListeners );
        while (aIt.hasMoreElements())
        {
            Reference< XLinguServiceEventListener > xRef( aIt.next(), UNO_QUERY );
            if (xRef.is())
                xRef->processLinguServiceEvent( aEvtObj );
        }
    }
    return 0;
}


void LngSvcMgrListenerHelper::AddLngSvcEvt( INT16 nLngSvcEvt )
{
    nCombinedLngSvcEvt |= nLngSvcEvt;
    aLaunchTimer.Start();
}


void SAL_CALL
    LngSvcMgrListenerHelper::processLinguServiceEvent(
            const LinguServiceEvent& rLngSvcEvent )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    AddLngSvcEvt( rLngSvcEvent.nEvent );
}


void SAL_CALL
    LngSvcMgrListenerHelper::processDictionaryListEvent(
            const DictionaryListEvent& rDicListEvent )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    INT16 nDlEvt = rDicListEvent.nCondensedEvent;
    if (0 == nDlEvt)
        return;

    // we do keep the original event source here though...

    // pass event on to XDictionaryListEventListener's
    cppu::OInterfaceIteratorHelper aIt( aLngSvcMgrListeners );
    while (aIt.hasMoreElements())
    {
        Reference< XDictionaryListEventListener > xRef( aIt.next(), UNO_QUERY );
        if (xRef.is())
            xRef->processDictionaryListEvent( rDicListEvent );
    }

    //
    // "translate" DictionaryList event into LinguServiceEvent
    //
    INT16 nLngSvcEvt = 0;
    //
    INT16 nSpellCorrectFlags =
            DictionaryListEventFlags::ADD_NEG_ENTRY     |
            DictionaryListEventFlags::DEL_POS_ENTRY     |
            DictionaryListEventFlags::ACTIVATE_NEG_DIC  |
            DictionaryListEventFlags::DEACTIVATE_POS_DIC;
    if (0 != (nDlEvt & nSpellCorrectFlags))
        nLngSvcEvt |= LinguServiceEventFlags::SPELL_CORRECT_WORDS_AGAIN;
    //
    INT16 nSpellWrongFlags =
            DictionaryListEventFlags::ADD_POS_ENTRY     |
            DictionaryListEventFlags::DEL_NEG_ENTRY     |
            DictionaryListEventFlags::ACTIVATE_POS_DIC  |
            DictionaryListEventFlags::DEACTIVATE_NEG_DIC;
    if (0 != (nDlEvt & nSpellWrongFlags))
        nLngSvcEvt |= LinguServiceEventFlags::SPELL_WRONG_WORDS_AGAIN;
    //
    INT16 nHyphenateFlags =
            DictionaryListEventFlags::ADD_POS_ENTRY     |
            DictionaryListEventFlags::DEL_POS_ENTRY     |
            DictionaryListEventFlags::ACTIVATE_POS_DIC  |
            DictionaryListEventFlags::ACTIVATE_NEG_DIC;
    if (0 != (nDlEvt & nHyphenateFlags))
        nLngSvcEvt |= LinguServiceEventFlags::HYPHENATE_AGAIN;
    //
    if (nLngSvcEvt)
        LaunchEvent( nLngSvcEvt );
}


void LngSvcMgrListenerHelper::LaunchEvent( INT16 nLngSvcEvtFlags )
{
    LinguServiceEvent aEvt( xMyEvtObj, nLngSvcEvtFlags );

    // pass event on to XLinguServiceEventListener's
    cppu::OInterfaceIteratorHelper aIt( aLngSvcMgrListeners );
    while (aIt.hasMoreElements())
    {
        Reference< XLinguServiceEventListener > xRef( aIt.next(), UNO_QUERY );
        if (xRef.is())
            xRef->processLinguServiceEvent( aEvt );
    }
}


inline BOOL LngSvcMgrListenerHelper::AddLngSvcMgrListener(
        const Reference< XEventListener >& rxListener )
{
    aLngSvcMgrListeners.addInterface(
        /*::getCppuType((const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XEventListener >*)0), */
        rxListener );
    return TRUE;
}


inline BOOL LngSvcMgrListenerHelper::RemoveLngSvcMgrListener(
        const Reference< XEventListener >& rxListener )
{
    aLngSvcMgrListeners.removeInterface(
        /*::getCppuType((const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XEventListener >*)0), */
        rxListener );
    return TRUE;
}


void LngSvcMgrListenerHelper::DisposeAndClear( const EventObject &rEvtObj )
{
    // call "disposing" for all listeners and clear list
    aLngSvcMgrListeners   .disposeAndClear( rEvtObj );

    // remove references to this object hold by the broadcasters
    cppu::OInterfaceIteratorHelper aIt( aLngSvcEvtBroadcasters );
    while (aIt.hasMoreElements())
    {
        Reference< XLinguServiceEventBroadcaster > xRef( aIt.next(), UNO_QUERY );
        if (xRef.is())
            RemoveLngSvcEvtBroadcaster( xRef );
    }

    // remove refernce to this object hold by the dictionary-list
    if (xDicList.is())
    {
        xDicList->removeDictionaryListEventListener(
            (XDictionaryListEventListener *) this );
        xDicList = 0;
    }
}


BOOL LngSvcMgrListenerHelper::AddLngSvcEvtBroadcaster(
        const Reference< XLinguServiceEventBroadcaster > &rxBroadcaster )
{
    BOOL bRes = FALSE;
    if (rxBroadcaster.is())
    {
        aLngSvcEvtBroadcasters.addInterface( rxBroadcaster );
        rxBroadcaster->addLinguServiceEventListener(
                (XLinguServiceEventListener *) this );
    }
    return bRes;
}


BOOL LngSvcMgrListenerHelper::RemoveLngSvcEvtBroadcaster(
        const Reference< XLinguServiceEventBroadcaster > &rxBroadcaster )
{
    BOOL bRes = FALSE;
    if (rxBroadcaster.is())
    {
        aLngSvcEvtBroadcasters.removeInterface( rxBroadcaster );
        rxBroadcaster->removeLinguServiceEventListener(
                (XLinguServiceEventListener *) this );
    }
    return bRes;
}


///////////////////////////////////////////////////////////////////////////


LngSvcMgr::LngSvcMgr() :
    utl::ConfigItem( String::CreateFromAscii( "Office.Linguistic" ) ),
    aEvtListeners   ( GetLinguMutex() )
{
    bHasAvailSpellLocales   =
    bHasAvailHyphLocales    =
    bHasAvailThesLocales    =
    bDisposing = FALSE;

    pSpellDsp   = 0;
    pHyphDsp    = 0;
    pThesDsp    = 0;

    pAvailSpellSvcs = 0;
    pAvailHyphSvcs  = 0;
    pAvailThesSvcs  = 0;
    pListenerHelper = 0;

    // request notify events when properties (i.e. something in the subtree) changes
    Sequence< OUString > aNames(3);
    OUString *pNames = aNames.getArray();
    pNames[0] = A2OU( "ServiceManager/SpellCheckerList" );
    pNames[1] = A2OU( "ServiceManager/HyphenatorList" );
    pNames[2] = A2OU( "ServiceManager/ThesaurusList" );
    EnableNotification( aNames );
}


LngSvcMgr::~LngSvcMgr()
{
    // memory for pSpellDsp, pHyphDsp, pThesDsp, pListenerHelper
    // will be freed in the destructor of the respective Reference's
    // xSpellDsp, xHyphDsp, xThesDsp

    delete pAvailSpellSvcs;
    delete pAvailHyphSvcs;
    delete pAvailThesSvcs;
}


void LngSvcMgr::Notify( const Sequence< OUString > &rPropertyNames )
{
    const OUString aSpellCheckerList( A2OU("ServiceManager/SpellCheckerList") );
    const OUString aHyphenatorList( A2OU("ServiceManager/HyphenatorList") );
    const OUString aThesaurusList( A2OU("ServiceManager/ThesaurusList") );

    const Sequence< OUString > aSpellCheckerListEntries( GetNodeNames( aSpellCheckerList ) );
    const Sequence< OUString > aHyphenatorListEntries( GetNodeNames( aHyphenatorList ) );
    const Sequence< OUString > aThesaurusListEntries( GetNodeNames( aThesaurusList ) );

    Sequence< Any > aValues;
    Sequence< OUString > aNames( 1 );
    OUString *pNames = aNames.getArray();

    sal_Int32 nLen = rPropertyNames.getLength();
    const OUString *pPropertyNames = rPropertyNames.getConstArray();
    for (sal_Int32 i = 0;  i < nLen;  ++i)
    {
        // property names look like
        // "ServiceManager/ThesaurusList/cfg:any['de-CH']"

        const OUString &rName = pPropertyNames[i];
        sal_Int32 nKeyStart, nKeyEnd;
        nKeyStart = rName.indexOf( A2OU("['"), 0 );
        nKeyEnd   = rName.indexOf( A2OU("']"), nKeyStart + 2);
        OUString aKeyText;
        if (nKeyStart != -1 && nKeyEnd != -1)
            aKeyText = rName.copy( nKeyStart + 2, nKeyEnd - nKeyStart - 2);
        DBG_ASSERT( aKeyText.getLength() != 0, "unexpected key (Locale) string" );
        if (0 == rName.compareTo( aSpellCheckerList, aSpellCheckerList.getLength() ))
        {
            // delete old cached data, needs to be acquired new on demand
            delete pAvailSpellSvcs;     pAvailSpellSvcs = 0;

            OUString aNode( aSpellCheckerList );
            if (lcl_SeqHasString( aSpellCheckerListEntries, aKeyText ))
            {
                OUString aPropName( aNode );
                aPropName += OUString::valueOf( (sal_Unicode) '/' );
                aPropName += aKeyText;
                pNames[0] = aPropName;
                aValues = /*aCfg.*/GetProperties( aNames );
                Sequence< OUString > aSvcImplNames;
                if (aValues.getLength())
                    aSvcImplNames = GetLangSvcList( aValues.getConstArray()[0] );

                LanguageType nLang = LANGUAGE_NONE;
                if (0 != aKeyText.getLength())
                    nLang = MsLangId::convertIsoStringToLanguage( aKeyText );

                GetSpellCheckerDsp_Impl( sal_False );     // don't set service list, it will be done below
                pSpellDsp->SetServiceList( CreateLocale(nLang), aSvcImplNames );
            }
        }
        else if (0 == rName.compareTo( aHyphenatorList, aHyphenatorList.getLength() ))
        {
            // delete old cached data, needs to be acquired new on demand
            delete pAvailHyphSvcs;      pAvailHyphSvcs = 0;

            OUString aNode( aHyphenatorList );
            if (lcl_SeqHasString( aHyphenatorListEntries, aKeyText ))
            {
                OUString aPropName( aNode );
                aPropName += OUString::valueOf( (sal_Unicode) '/' );
                aPropName += aKeyText;
                pNames[0] = aPropName;
                aValues = /*aCfg.*/GetProperties( aNames );
                Sequence< OUString > aSvcImplNames;
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
            delete pAvailThesSvcs;      pAvailThesSvcs = 0;

            OUString aNode( aThesaurusList );
            if (lcl_SeqHasString( aThesaurusListEntries, aKeyText ))
            {
                OUString aPropName( aNode );
                aPropName += OUString::valueOf( (sal_Unicode) '/' );
                aPropName += aKeyText;
                pNames[0] = aPropName;
                aValues = /*aCfg.*/GetProperties( aNames );
                Sequence< OUString > aSvcImplNames;
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
            DBG_ERROR( "nofified for unexpected property" );
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
        pListenerHelper = new LngSvcMgrListenerHelper(
                (XLinguServiceManager *) this, linguistic::GetDictionaryList() );
        xListenerHelper = (XLinguServiceEventListener *) pListenerHelper;
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

        Reference< XMultiServiceFactory >  xFac( getProcessServiceFactory() );
        if (xFac.is())
        {
            Reference< XContentEnumerationAccess > xEnumAccess( xFac, UNO_QUERY );
            Reference< XEnumeration > xEnum;
            if (xEnumAccess.is())
                xEnum = xEnumAccess->createContentEnumeration(
                        A2OU( SN_SPELLCHECKER ) );

            if (xEnum.is())
            {
                while (xEnum->hasMoreElements())
                {
                    Any aCurrent = xEnum->nextElement();
                    Reference< XSingleServiceFactory > xFactory;

                    if (!::cppu::extractInterface( xFactory, aCurrent ))
                        continue;

                    Reference< XSpellChecker > xSvc( xFactory->createInstance(),
                                                     UNO_QUERY );
                    if (xSvc.is())
                    {
                        OUString            aImplName;
                        Sequence< INT16 >   aLanguages;
                        Reference< XServiceInfo > xInfo( xSvc, UNO_QUERY );
                        if (xInfo.is())
                            aImplName = xInfo->getImplementationName();
                        DBG_ASSERT( aImplName.getLength(),
                                "empty implementation name" );
                        Reference< XSupportedLocales > xSuppLoc( xSvc, UNO_QUERY );
                        DBG_ASSERT( xSuppLoc.is(), "interfaces not supported" );
                        if (xSuppLoc.is()) {
                            Sequence<Locale> aLocaleSequence(xSuppLoc->getLocales());
                            aLanguages = LocaleSeqToLangSeq( aLocaleSequence );
                        }

                        pAvailSpellSvcs->Insert( new SvcInfo( aImplName, aLanguages ),
                                            pAvailSpellSvcs->Count() );
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

        Reference< XMultiServiceFactory >  xFac( getProcessServiceFactory() );
        if (xFac.is())
        {
            Reference< XContentEnumerationAccess > xEnumAccess( xFac, UNO_QUERY );
            Reference< XEnumeration > xEnum;
            if (xEnumAccess.is())
                xEnum = xEnumAccess->createContentEnumeration(
                        A2OU( SN_HYPHENATOR ) );

            if (xEnum.is())
            {
                while (xEnum->hasMoreElements())
                {
                    Any aCurrent = xEnum->nextElement();
                    Reference< XSingleServiceFactory > xFactory;

                    if (!::cppu::extractInterface( xFactory, aCurrent ))
                        continue;

                    Reference< XHyphenator > xSvc( xFactory->createInstance(),
                                                   UNO_QUERY );
                    if (xSvc.is())
                    {
                        OUString            aImplName;
                        Sequence< INT16 >   aLanguages;
                        Reference< XServiceInfo > xInfo( xSvc, UNO_QUERY );
                        if (xInfo.is())
                            aImplName = xInfo->getImplementationName();
                        DBG_ASSERT( aImplName.getLength(),
                                "empty implementation name" );
                        Reference< XSupportedLocales > xSuppLoc( xSvc, UNO_QUERY );
                        DBG_ASSERT( xSuppLoc.is(), "interfaces not supported" );
                        if (xSuppLoc.is()) {
                            Sequence<Locale> aLocaleSequence(xSuppLoc->getLocales());
                            aLanguages = LocaleSeqToLangSeq( aLocaleSequence );
                        }

                        pAvailHyphSvcs->Insert( new SvcInfo( aImplName, aLanguages ),
                                            pAvailHyphSvcs->Count() );
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

        Reference< XMultiServiceFactory >  xFac( getProcessServiceFactory() );
        if (xFac.is())
        {
            Reference< XContentEnumerationAccess > xEnumAccess( xFac, UNO_QUERY );
            Reference< XEnumeration > xEnum;
            if (xEnumAccess.is())
                xEnum = xEnumAccess->createContentEnumeration(
                        A2OU( SN_THESAURUS ) );

            if (xEnum.is())
            {
                while (xEnum->hasMoreElements())
                {
                    Any aCurrent = xEnum->nextElement();
                    Reference< XSingleServiceFactory > xFactory;

                    if (!::cppu::extractInterface( xFactory, aCurrent ))
                        continue;

                    Reference< XThesaurus > xSvc( xFactory->createInstance(),
                                                  UNO_QUERY );
                    if (xSvc.is())
                    {
                        OUString            aImplName;
                        Sequence< INT16 >   aLanguages;
                        Reference< XServiceInfo > xInfo( xSvc, UNO_QUERY );
                        if (xInfo.is())
                            aImplName = xInfo->getImplementationName();
                        DBG_ASSERT( aImplName.getLength(),
                                "empty implementation name" );
                        Reference< XSupportedLocales > xSuppLoc( xSvc, UNO_QUERY );
                        DBG_ASSERT( xSuppLoc.is(), "interfaces not supported" );
                        if (xSuppLoc.is()) {
                            Sequence<Locale> aLocaleSequence(xSuppLoc->getLocales());
                            aLanguages = LocaleSeqToLangSeq( aLocaleSequence );
                        }

                        pAvailThesSvcs->Insert( new SvcInfo( aImplName, aLanguages ),
                                            pAvailThesSvcs->Count() );
                    }
                }
            }
        }
    }
}


void LngSvcMgr::SetCfgServiceLists( SpellCheckerDispatcher &rSpellDsp )
{
    RTL_LOGFILE_CONTEXT( aLog, "linguistic: LngSvcMgr::SetCfgServiceLists - Spell" );

    //SvtLinguConfig aCfg;

    String  aNode( String::CreateFromAscii( "ServiceManager/SpellCheckerList" ) );
    Sequence< OUString > aNames( /*aCfg.*/GetNodeNames( aNode ) );
    OUString *pNames = aNames.getArray();
    INT32 nLen = aNames.getLength();

    // append path prefix need for 'GetProperties' call below
    String aPrefix( aNode );
    aPrefix.Append( (sal_Unicode) '/' );
    for (int i = 0;  i < nLen;  ++i)
    {
        OUString aTmp( aPrefix );
        aTmp += pNames[i];
        pNames[i] = aTmp;
    }

    Sequence< Any > aValues( /*aCfg.*/GetProperties( aNames ) );
    if (nLen  &&  nLen == aValues.getLength())
    {
        const Any *pValues = aValues.getConstArray();
        for (INT32 i = 0;  i < nLen;  ++i)
        {
            Sequence< OUString > aSvcImplNames;
            if (pValues[i] >>= aSvcImplNames)
            {
#if OSL_DEBUG_LEVEL > 1
//                INT32 nSvcs = aSvcImplNames.getLength();
//                const OUString *pSvcImplNames = aSvcImplNames.getConstArray();
#endif
                String aLocaleStr( pNames[i] );
                xub_StrLen nSeperatorPos = aLocaleStr.SearchBackward( sal_Unicode( '/' ) );
                aLocaleStr = aLocaleStr.Copy( nSeperatorPos + 1 );
                Locale aLocale( CreateLocale( MsLangId::convertIsoStringToLanguage(aLocaleStr) ) );
                rSpellDsp.SetServiceList( aLocale, aSvcImplNames );
            }
        }
    }
}


void LngSvcMgr::SetCfgServiceLists( HyphenatorDispatcher &rHyphDsp )
{
    RTL_LOGFILE_CONTEXT( aLog, "linguistic: LngSvcMgr::SetCfgServiceLists - Hyph" );

    //SvtLinguConfig aCfg;

    String  aNode( String::CreateFromAscii( "ServiceManager/HyphenatorList" ) );
    Sequence< OUString > aNames( /*aCfg.*/GetNodeNames( aNode ) );
    OUString *pNames = aNames.getArray();
    INT32 nLen = aNames.getLength();

    // append path prefix need for 'GetProperties' call below
    String aPrefix( aNode );
    aPrefix.Append( (sal_Unicode) '/' );
    for (int i = 0;  i < nLen;  ++i)
    {
        OUString aTmp( aPrefix );
        aTmp += pNames[i];
        pNames[i] = aTmp;
    }

    Sequence< Any > aValues( /*aCfg.*/GetProperties( aNames ) );
    if (nLen  &&  nLen == aValues.getLength())
    {
        const Any *pValues = aValues.getConstArray();
        for (INT32 i = 0;  i < nLen;  ++i)
        {
            Sequence< OUString > aSvcImplNames;
            if (pValues[i] >>= aSvcImplNames)
            {
                // there should only be one hyphenator in use per language...
                if (aSvcImplNames.getLength() > 1)
                    aSvcImplNames.realloc(1);

#if OSL_DEBUG_LEVEL > 1
//                INT32 nSvcs = aSvcImplNames.getLength();
//                const OUString *pSvcImplNames = aSvcImplNames.getConstArray();
#endif
                String aLocaleStr( pNames[i] );
                xub_StrLen nSeperatorPos = aLocaleStr.SearchBackward( sal_Unicode( '/' ) );
                aLocaleStr = aLocaleStr.Copy( nSeperatorPos + 1 );
                Locale aLocale( CreateLocale( MsLangId::convertIsoStringToLanguage(aLocaleStr) ) );
                rHyphDsp.SetServiceList( aLocale, aSvcImplNames );
            }
        }
    }
}


void LngSvcMgr::SetCfgServiceLists( ThesaurusDispatcher &rThesDsp )
{
    RTL_LOGFILE_CONTEXT( aLog, "linguistic: LngSvcMgr::SetCfgServiceLists - Thes" );

    //SvtLinguConfig aCfg;

    String  aNode( String::CreateFromAscii( "ServiceManager/ThesaurusList" ) );
    Sequence< OUString > aNames( /*aCfg.*/GetNodeNames( aNode ) );
    OUString *pNames = aNames.getArray();
    INT32 nLen = aNames.getLength();

    // append path prefix need for 'GetProperties' call below
    String aPrefix( aNode );
    aPrefix.Append( (sal_Unicode) '/' );
    for (int i = 0;  i < nLen;  ++i)
    {
        OUString aTmp( aPrefix );
        aTmp += pNames[i];
        pNames[i] = aTmp;
    }

    Sequence< Any > aValues( /*aCfg.*/GetProperties( aNames ) );
    if (nLen  &&  nLen == aValues.getLength())
    {
        const Any *pValues = aValues.getConstArray();
        for (INT32 i = 0;  i < nLen;  ++i)
        {
            Sequence< OUString > aSvcImplNames;
            if (pValues[i] >>= aSvcImplNames)
            {
#if OSL_DEBUG_LEVEL > 1
//                INT32 nSvcs = aSvcImplNames.getLength();
//                const OUString *pSvcImplNames = aSvcImplNames.getConstArray();
#endif
                String aLocaleStr( pNames[i] );
                xub_StrLen nSeperatorPos = aLocaleStr.SearchBackward( sal_Unicode( '/' ) );
                aLocaleStr = aLocaleStr.Copy( nSeperatorPos + 1 );
                Locale aLocale( CreateLocale( MsLangId::convertIsoStringToLanguage(aLocaleStr) ) );
                rThesDsp.SetServiceList( aLocale, aSvcImplNames );
            }
        }
    }
}


Reference< XSpellChecker > SAL_CALL
    LngSvcMgr::getSpellChecker()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
#if OSL_DEBUG_LEVEL > 1
    getAvailableLocales( A2OU( SN_SPELLCHECKER ));
#endif

    Reference< XSpellChecker >  xRes;
    if (!bDisposing)
    {
        if (!xSpellDsp.is())
            GetSpellCheckerDsp_Impl();
        xRes = xSpellDsp;
    }
    return xRes;
}


Reference< XHyphenator > SAL_CALL
    LngSvcMgr::getHyphenator()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
#if OSL_DEBUG_LEVEL > 1
    getAvailableLocales( A2OU( SN_HYPHENATOR ));
#endif

    Reference< XHyphenator >    xRes;
    if (!bDisposing)
    {
        if (!xHyphDsp.is())
            GetHyphenatorDsp_Impl();
        xRes = xHyphDsp;
    }
    return xRes;
}


Reference< XThesaurus > SAL_CALL
    LngSvcMgr::getThesaurus()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
#if OSL_DEBUG_LEVEL > 1
    getAvailableLocales( A2OU( SN_THESAURUS ));
#endif

    Reference< XThesaurus > xRes;
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
            const Reference< XEventListener >& xListener )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    BOOL bRes = FALSE;
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
            const Reference< XEventListener >& xListener )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    BOOL bRes = FALSE;
    if (!bDisposing  &&  xListener.is())
    {
        DBG_ASSERT( pListenerHelper, "listener removed without being added" );
        if (!pListenerHelper)
            GetListenerHelper_Impl();
        bRes = pListenerHelper->RemoveLngSvcMgrListener( xListener );
    }
    return bRes;
}


Sequence< OUString > SAL_CALL
    LngSvcMgr::getAvailableServices(
            const OUString& rServiceName,
            const Locale& rLocale )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    Sequence< OUString > aRes;
    const SvcInfoArray *pInfoArray = 0;

    if (0 == rServiceName.compareToAscii( SN_SPELLCHECKER ))
    {
        // don't used cached data here (force re-evaluation in order to have downloaded dictionaries
        // already found without the need to restart the office
        delete pAvailSpellSvcs;  pAvailSpellSvcs = 0;
//      if (!pAvailSpellSvcs)
        GetAvailableSpellSvcs_Impl();
        pInfoArray = pAvailSpellSvcs;
    }
    else if (0 == rServiceName.compareToAscii( SN_HYPHENATOR ))
    {
        // don't used cached data here (force re-evaluation in order to have downloaded dictionaries
        // already found without the need to restart the office
        delete pAvailHyphSvcs;  pAvailHyphSvcs = 0;
//      if (!pAvailHyphSvcs)
        GetAvailableHyphSvcs_Impl();
        pInfoArray = pAvailHyphSvcs;
    }
    else if (0 == rServiceName.compareToAscii( SN_THESAURUS ))
    {
        // don't used cached data here (force re-evaluation in order to have downloaded dictionaries
        // already found without the need to restart the office
        delete pAvailThesSvcs;  pAvailThesSvcs = 0;
//      if (!pAvailThesSvcs)
        GetAvailableThesSvcs_Impl();
        pInfoArray = pAvailThesSvcs;
    }

    if (pInfoArray)
    {
        // resize to max number of entries
        USHORT nMaxCnt = pInfoArray->Count();
        aRes.realloc( nMaxCnt );
        OUString *pImplName = aRes.getArray();

        USHORT nCnt = 0;
        INT16 nLanguage = LocaleToLanguage( rLocale );
        for (USHORT i = 0;  i < nMaxCnt;  ++i)
        {
            const SvcInfo *pInfo = pInfoArray->GetObject(i);
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


Sequence< Locale > SAL_CALL
    LngSvcMgr::getAvailableLocales(
            const OUString& rServiceName )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    Sequence< Locale > aRes;

    Sequence< Locale >  *pAvailLocales      = NULL;
    BOOL                *pHasAvailLocales   = NULL;
    if (0 == rServiceName.compareToAscii( SN_SPELLCHECKER ))
    {
        pAvailLocales       = &aAvailSpellLocales;
        pHasAvailLocales    = &bHasAvailSpellLocales;
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
                    getAvailableServices( rServiceName, Locale() ) );
//          *pHasAvailLocales = TRUE;
//      }
        aRes = *pAvailLocales;
    }

    return aRes;
}

static BOOL IsEqSvcList( const Sequence< OUString > &rList1,
                        const Sequence< OUString > &rList2 )
{
    // returns TRUE iff both sequences are equal

    BOOL bRes = FALSE;
    INT32 nLen = rList1.getLength();
    if (rList2.getLength() == nLen)
    {
        const OUString *pStr1 = rList1.getConstArray();
        const OUString *pStr2 = rList2.getConstArray();
        bRes = TRUE;
        for (INT32 i = 0;  i < nLen  &&  bRes;  ++i)
        {
            if (*pStr1++ != *pStr2++)
                bRes = FALSE;
        }
    }
    return bRes;
}


void SAL_CALL
    LngSvcMgr::setConfiguredServices(
            const OUString& rServiceName,
            const Locale& rLocale,
            const Sequence< OUString >& rServiceImplNames )
        throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT( aLog, "linguistic: LngSvcMgr::setConfiguredServices" );

    MutexGuard  aGuard( GetLinguMutex() );

#if OSL_DEBUG_LEVEL > 1
//    const OUString *pImplNames = rServiceImplNames.getConstArray();
#endif

    INT16 nLanguage = LocaleToLanguage( rLocale );
    if (LANGUAGE_NONE != nLanguage)
    {
        if (0 == rServiceName.compareToAscii( SN_SPELLCHECKER ))
        {
            if (!xSpellDsp.is())
                GetSpellCheckerDsp_Impl();
            BOOL bChanged = !IsEqSvcList( rServiceImplNames,
                                          pSpellDsp->GetServiceList( rLocale ) );
            if (bChanged)
            {
                pSpellDsp->SetServiceList( rLocale, rServiceImplNames );
                SaveCfgSvcs( A2OU( SN_SPELLCHECKER ) );

                if (pListenerHelper  &&  bChanged)
                    pListenerHelper->AddLngSvcEvt(
                            LinguServiceEventFlags::SPELL_CORRECT_WORDS_AGAIN |
                            LinguServiceEventFlags::SPELL_WRONG_WORDS_AGAIN );
            }
        }
        else if (0 == rServiceName.compareToAscii( SN_HYPHENATOR ))
        {
            if (!xHyphDsp.is())
                GetHyphenatorDsp_Impl();
            BOOL bChanged = !IsEqSvcList( rServiceImplNames,
                                          pHyphDsp->GetServiceList( rLocale ) );
            if (bChanged)
            {
                pHyphDsp->SetServiceList( rLocale, rServiceImplNames );
                SaveCfgSvcs( A2OU( SN_HYPHENATOR ) );

                if (pListenerHelper  &&  bChanged)
                    pListenerHelper->AddLngSvcEvt(
                            LinguServiceEventFlags::HYPHENATE_AGAIN );
            }
        }
        else if (0 == rServiceName.compareToAscii( SN_THESAURUS ))
        {
            if (!xThesDsp.is())
                GetThesaurusDsp_Impl();
            BOOL bChanged = !IsEqSvcList( rServiceImplNames,
                                          pThesDsp->GetServiceList( rLocale ) );
            if (bChanged)
            {
                pThesDsp->SetServiceList( rLocale, rServiceImplNames );
                SaveCfgSvcs( A2OU( SN_THESAURUS ) );
            }
        }
    }
}


BOOL LngSvcMgr::SaveCfgSvcs( const String &rServiceName )
{
    RTL_LOGFILE_CONTEXT( aLog, "linguistic: LngSvcMgr::SaveCfgSvcs" );

    BOOL bRes = FALSE;

    LinguDispatcher *pDsp = 0;
    Sequence< Locale > aLocales;

    if (0 == rServiceName.CompareToAscii( SN_SPELLCHECKER ))
    {
        if (!pSpellDsp)
            GetSpellCheckerDsp_Impl();
        pDsp = pSpellDsp;
//        aLocales = xSpellDsp->getLocales();
        aLocales = getAvailableLocales( A2OU( SN_SPELLCHECKER ) );
    }
    else if (0 == rServiceName.CompareToAscii( SN_HYPHENATOR ))
    {
        if (!pHyphDsp)
            GetHyphenatorDsp_Impl();
        pDsp = pHyphDsp;
//        aLocales = xHyphDsp->getLocales();
        aLocales = getAvailableLocales( A2OU( SN_HYPHENATOR ) );
    }
    else if (0 == rServiceName.CompareToAscii( SN_THESAURUS ))
    {
        if (!pThesDsp)
            GetThesaurusDsp_Impl();
        pDsp = pThesDsp;
//        aLocales = xThesDsp->getLocales();
        aLocales = getAvailableLocales( A2OU( SN_THESAURUS ) );
    }

    if (pDsp  &&  aLocales.getLength())
    {
        //SvtLinguConfig aCfg;

        INT32 nLen = aLocales.getLength();
        const Locale *pLocale = aLocales.getConstArray();

        Sequence< PropertyValue > aValues( nLen );
        PropertyValue *pValues = aValues.getArray();
        PropertyValue *pValue  = pValues;

        // get node name to be used
        const char *pNodeName = NULL;
        if (pDsp == pSpellDsp)
            pNodeName = "ServiceManager/SpellCheckerList";
        else if (pDsp == pThesDsp)
            pNodeName = "ServiceManager/ThesaurusList";
        else if (pDsp == pHyphDsp)
            pNodeName = "ServiceManager/HyphenatorList";
        else
            DBG_ERROR( "node name missing" );
        OUString aNodeName( A2OU(pNodeName) );

        for (INT32 i = 0;  i < nLen;  ++i)
        {
            Sequence< OUString > aSvcImplNames;
            aSvcImplNames = pDsp->GetServiceList( pLocale[i] );

#if OSL_DEBUG_LEVEL > 1
            INT32 nSvcs = aSvcImplNames.getLength();
            const OUString *pSvcImplName = aSvcImplNames.getConstArray();
            for (INT32 j = 0;  j < nSvcs;  ++j)
            {
                OUString aImplName( pSvcImplName[j] );
            }
#endif
            // build value to be written back to configuration
            Any aCfgAny;
            if (pDsp == pHyphDsp  &&  aSvcImplNames.getLength() > 1)
                aSvcImplNames.realloc(1);   // there should be only one entry for hyphenators (because they are not chained)
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


static Sequence< OUString > GetLangSvcList( const Any &rVal )
{
    Sequence< OUString > aRes;

    if (rVal.hasValue())
    {
        rVal >>= aRes;
#if OSL_DEBUG_LEVEL > 1
        INT32 nSvcs = aRes.getLength();
        if (nSvcs)
        {
            const OUString *pSvcName = aRes.getConstArray();
            for (INT32 j = 0;  j < nSvcs;  ++j)
            {
                OUString aImplName( pSvcName[j] );
                DBG_ASSERT( aImplName.getLength(), "service impl-name missing" );
            }
        }
#endif
    }

    return aRes;
}


static Sequence< OUString > GetLangSvc( const Any &rVal )
{
    Sequence< OUString > aRes;
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
            DBG_ERROR( "GetLangSvc: unexpected type encountered" );
    }

    return aRes;
}


///////////////////////////////////////////////////////////////////////////

Sequence< OUString > SAL_CALL
    LngSvcMgr::getConfiguredServices(
            const OUString& rServiceName,
            const Locale& rLocale )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    Sequence< OUString > aSvcImplNames;

    INT16 nLanguage = LocaleToLanguage( rLocale );
    OUString aCfgLocale( MsLangId::convertLanguageToIsoString( nLanguage ) );

    //SvtLinguConfig aCfg;

    Sequence< Any > aValues;
    Sequence< OUString > aNames( 1 );
    OUString *pNames = aNames.getArray();
    if ( 0 == rServiceName.compareToAscii( SN_SPELLCHECKER ) )
    {
        OUString aNode( OUString::createFromAscii( "ServiceManager/SpellCheckerList" ));
        const Sequence< OUString > aNodeEntries( GetNodeNames( aNode ) );
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
    else if ( 0 == rServiceName.compareToAscii( SN_HYPHENATOR ) )
    {
        OUString aNode( OUString::createFromAscii( "ServiceManager/HyphenatorList" ));
        const Sequence< OUString > aNodeEntries( GetNodeNames( aNode ) );
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
        const Sequence< OUString > aNodeEntries( GetNodeNames( aNode ) );
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
//    const OUString *pImplNames = aSvcImplNames.getConstArray();
#endif
    return aSvcImplNames;
}


void SAL_CALL
    LngSvcMgr::dispose()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing)
    {
        bDisposing = TRUE;

        // require listeners to release this object
        EventObject aEvtObj( (XLinguServiceManager *) this );
        aEvtListeners.disposeAndClear( aEvtObj );

        if (pListenerHelper)
            pListenerHelper->DisposeAndClear( aEvtObj );
    }
}


void SAL_CALL
    LngSvcMgr::addEventListener(
            const Reference< XEventListener >& xListener )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing  &&  xListener.is())
    {
        aEvtListeners.addInterface(
            /*::getCppuType((const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XEventListener >*)0), */
            xListener );
    }
}


void SAL_CALL
    LngSvcMgr::removeEventListener(
            const Reference< XEventListener >& xListener )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (xListener.is())
    {
        aEvtListeners.removeInterface(
            /*::getCppuType((const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XEventListener >*)0), */
            xListener );
    }
}


BOOL LngSvcMgr::AddLngSvcEvtBroadcaster(
            const Reference< XLinguServiceEventBroadcaster > &rxBroadcaster )
{
    BOOL bRes = FALSE;
    if (rxBroadcaster.is())
    {
        if (!pListenerHelper)
            GetListenerHelper_Impl();
        bRes = pListenerHelper->AddLngSvcEvtBroadcaster( rxBroadcaster );
    }
    return bRes;
}


BOOL LngSvcMgr::RemoveLngSvcEvtBroadcaster(
            const Reference< XLinguServiceEventBroadcaster > &rxBroadcaster )
{
    BOOL bRes = FALSE;
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
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return getImplementationName_Static();
}


sal_Bool SAL_CALL
    LngSvcMgr::supportsService( const OUString& ServiceName )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    uno::Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();
    for( INT32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return TRUE;
    return FALSE;
}


Sequence< OUString > SAL_CALL
    LngSvcMgr::getSupportedServiceNames()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return getSupportedServiceNames_Static();
}


uno::Sequence< OUString > LngSvcMgr::getSupportedServiceNames_Static()
        throw()
{
    MutexGuard  aGuard( GetLinguMutex() );

    uno::Sequence< OUString > aSNS( 1 );    // auch mehr als 1 Service moeglich
    aSNS.getArray()[0] = A2OU( SN_LINGU_SERVCICE_MANAGER );
    return aSNS;
}


Reference< XInterface > SAL_CALL LngSvcMgr_CreateInstance(
            const Reference< XMultiServiceFactory > & /*rSMgr*/ )
        throw(Exception)
{
    Reference< XInterface > xService = (cppu::OWeakObject*) new LngSvcMgr;
    return xService;
}



sal_Bool SAL_CALL LngSvcMgr_writeInfo(
            void * /*pServiceManager*/,
            registry::XRegistryKey * pRegistryKey )
{
    try
    {
        String aImpl( '/' );
        aImpl += LngSvcMgr::getImplementationName_Static().getStr();
        aImpl.AppendAscii( "/UNO/SERVICES" );
        Reference< registry::XRegistryKey > xNewKey =
            pRegistryKey->createKey( aImpl );
        uno::Sequence< OUString > aServices = LngSvcMgr::getSupportedServiceNames_Static();
        for( INT32 i = 0; i < aServices.getLength(); i++ )
            xNewKey->createKey( aServices.getConstArray()[i] );

        return sal_True;
    }
    catch(Exception &)
    {
        return sal_False;
    }
}

void * SAL_CALL LngSvcMgr_getFactory(
            const sal_Char * pImplName,
            XMultiServiceFactory * pServiceManager,
            void * /*pRegistryKey*/ )
{

    void * pRet = 0;
    if ( !LngSvcMgr::getImplementationName_Static().compareToAscii( pImplName ) )
    {
        Reference< XSingleServiceFactory > xFactory =
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

