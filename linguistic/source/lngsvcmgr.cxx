/*************************************************************************
 *
 *  $RCSfile: lngsvcmgr.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: tl $ $Date: 2001-02-27 14:29:18 $
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
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
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


SV_DECL_VARARR_SORT( SortedINT16Array, INT16, 32, 32);
SV_IMPL_VARARR_SORT( SortedINT16Array, INT16 );

// forward declarations
Sequence< OUString > static GetLangSvcList( const Any &rVal );
Sequence< OUString > static GetLangSvc( const Any &rVal );

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
        for (INT32 i = 0;  i < nNames;  ++i)
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

        // build returnes sequence
        INT16 nLanguages = aLanguages.Count();
        aRes.realloc( nLanguages );
        Locale *pRes = aRes.getArray();
        for (i = 0;  i < nLanguages;  ++i)
        {
            INT16 nLang = aLanguages[i];
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
    for (INT32 i = 0;  i < nCnt;  ++i)
    {
        if (nLanguage == pLang[i])
            break;
    }
    return i < nCnt;
}


typedef SvcInfo * PTR_SVCINFO;
SV_DECL_PTRARR_DEL( SvcInfoArray, PTR_SVCINFO, 16, 16 );
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


void SetAvailableCfgServiceLists( LinguDispatcher &rDispatcher,
        const SvcInfoArray &rAvailSvcs )
{
    String aRoot( String::CreateFromAscii( "Office.Linguistic/ServiceManager" ) );
    SvtLinguConfigItem aCfg( aRoot );

    // get list of nodenames to look at for their service list
    const char *pEntryName = 0;
    BOOL bHasLangSvcList = TRUE;
    switch (rDispatcher.GetDspType())
    {
        case LinguDispatcher::DSP_SPELL : pEntryName = "SpellCheckerList";  break;
        case LinguDispatcher::DSP_HYPH  : pEntryName = "HyphenatorList";
                                          bHasLangSvcList = FALSE;
                                          break;
        case LinguDispatcher::DSP_THES  : pEntryName = "ThesaurusList"; break;
        default :
            DBG_ERROR( "unexpected case" );
    }
    String  aNode( String::CreateFromAscii( pEntryName ) );
    Sequence < OUString > aNodeNames( aCfg.GetNodeNames( aNode ) );


    INT32 nLen = aNodeNames.getLength();
    const OUString *pNodeNames = aNodeNames.getConstArray();
    for (INT32 i = 0;  i < nLen;  ++i)
    {
        Sequence< OUString >    aSvcImplNames;

        Sequence< OUString >    aName( 1 );
        OUString *pNames = aName.getArray();

        OUString aPropName( aNode );
        aPropName += OUString::valueOf( (sal_Unicode) '/' );
        aPropName += pNodeNames[i];
        pNames[0] = aPropName;

        Sequence< Any > aValues = aCfg.GetProperties( aName );
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

                INT16 nLang = ConvertIsoStringToLanguage( pNodeNames[i] );

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


void SAL_CALL
    LngSvcMgrListenerHelper::processLinguServiceEvent(
            const LinguServiceEvent& rLngSvcEvent )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    nCombinedLngSvcEvt |= rLngSvcEvent.nEvent;
    aLaunchTimer.Start();
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
    aEvtListeners   ( GetLinguMutex() )
{
    bHasAvailSpellLocales   =
    bHasAvailHyphLocales    =
    bHasAvailThesLocales    =
    bIsModified = bDisposing = FALSE;

    pSpellDsp   = 0;
    pHyphDsp    = 0;
    pThesDsp    = 0;

    pAvailSpellSvcs = 0;
    pAvailHyphSvcs  = 0;
    pAvailThesSvcs  = 0;
    pListenerHelper = 0;

    aSaveTimer.SetTimeout( 5000 );
    aSaveTimer.SetTimeoutHdl( LINK( this, LngSvcMgr, TimeOut ));
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


void LngSvcMgr::GetListenerHelper_Impl()
{
    if (!pListenerHelper)
    {
        pListenerHelper = new LngSvcMgrListenerHelper(
                (XLinguServiceManager *) this, linguistic::GetDictionaryList() );
        xListenerHelper = (XLinguServiceEventListener *) pListenerHelper;
    }
}


void LngSvcMgr::GetSpellCheckerDsp_Impl()
{
    if (!pSpellDsp)
    {
        pSpellDsp   = new SpellCheckerDispatcher( *this );
        xSpellDsp   = pSpellDsp;
        SetCfgServiceLists( *pSpellDsp );
    }
}


void LngSvcMgr::GetHyphenatorDsp_Impl()
{
    if (!pHyphDsp)
    {
        pHyphDsp    = new HyphenatorDispatcher( *this );
        xHyphDsp    = pHyphDsp;
        SetCfgServiceLists( *pHyphDsp );
    }
}


void LngSvcMgr::GetThesaurusDsp_Impl()
{
    if (!pThesDsp)
    {
        pThesDsp    = new ThesaurusDispatcher;
        xThesDsp    = pThesDsp;
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
                        if (xSuppLoc.is())
                            aLanguages = LocaleSeqToLangSeq( xSuppLoc->getLocales() );

                        pAvailSpellSvcs->Insert( new SvcInfo( aImplName, aLanguages ),
                                            pAvailSpellSvcs->Count() );

                        // TL_TODO:
                        // when HRO has the functionality provided to unload
                        // the DLLs it should be done here!
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
                        if (xSuppLoc.is())
                            aLanguages = LocaleSeqToLangSeq( xSuppLoc->getLocales() );

                        pAvailHyphSvcs->Insert( new SvcInfo( aImplName, aLanguages ),
                                            pAvailHyphSvcs->Count() );

                        // TL_TODO:
                        // when HRO has the functionality provided to unload
                        // the DLLs it should be done here!
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
                        if (xSuppLoc.is())
                            aLanguages = LocaleSeqToLangSeq( xSuppLoc->getLocales() );

                        pAvailThesSvcs->Insert( new SvcInfo( aImplName, aLanguages ),
                                            pAvailThesSvcs->Count() );

                        // TL_TODO:
                        // when HRO has the functionality provided to unload
                        // the DLLs it should be done here!
                    }
                }
            }
        }
    }
}


void LngSvcMgr::SetCfgServiceLists( SpellCheckerDispatcher &rSpellDsp )
{
    if (!pAvailSpellSvcs)
        GetAvailableSpellSvcs_Impl();
    SetAvailableCfgServiceLists( rSpellDsp, *pAvailSpellSvcs );
}


void LngSvcMgr::SetCfgServiceLists( HyphenatorDispatcher &rHyphDsp )
{
    if (!pAvailHyphSvcs)
        GetAvailableHyphSvcs_Impl();
    SetAvailableCfgServiceLists( rHyphDsp, *pAvailHyphSvcs );
}


void LngSvcMgr::SetCfgServiceLists( ThesaurusDispatcher &rThesDsp )
{
    if (!pAvailThesSvcs)
        GetAvailableThesSvcs_Impl();
    SetAvailableCfgServiceLists( rThesDsp, *pAvailThesSvcs );
}


Reference< XSpellChecker > SAL_CALL
    LngSvcMgr::getSpellChecker()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
#ifdef DEBUG
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
#ifdef DEBUG
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
#ifdef DEBUG
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
        if (!pAvailSpellSvcs)
            GetAvailableSpellSvcs_Impl();
        pInfoArray = pAvailSpellSvcs;
    }
    else if (0 == rServiceName.compareToAscii( SN_HYPHENATOR ))
    {
        if (!pAvailHyphSvcs)
            GetAvailableHyphSvcs_Impl();
        pInfoArray = pAvailHyphSvcs;
    }
    else if (0 == rServiceName.compareToAscii( SN_THESAURUS ))
    {
        if (!pAvailThesSvcs)
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
        if (nCnt && nCnt != nMaxCnt)
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

    if (pAvailLocales  &&  pHasAvailLocales)
    {
        if (!*pHasAvailLocales)
        {
            *pAvailLocales = GetAvailLocales(
                    getAvailableServices( rServiceName, Locale() ) );
            *pHasAvailLocales = TRUE;
        }
        aRes = *pAvailLocales;
    }

    return aRes;
}


void SAL_CALL
    LngSvcMgr::setConfiguredServices(
            const OUString& rServiceName,
            const Locale& rLocale,
            const Sequence< OUString >& rServiceImplNames )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    BOOL bCfgChgSuccess = TRUE;
    INT16 nLanguage = LocaleToLanguage( rLocale );

    if (bCfgChgSuccess  &&  LANGUAGE_NONE != nLanguage)
    {
        if (0 == rServiceName.compareToAscii( SN_SPELLCHECKER ))
        {
            if (!xSpellDsp.is())
                GetSpellCheckerDsp_Impl();
            pSpellDsp->SetServiceList( rLocale, rServiceImplNames );
            bIsModified = TRUE;
        }
        else if (0 == rServiceName.compareToAscii( SN_HYPHENATOR ))
        {
            if (!xHyphDsp.is())
                GetHyphenatorDsp_Impl();
            pHyphDsp->SetServiceList( rLocale, rServiceImplNames );
            bIsModified = TRUE;
        }
        else if (0 == rServiceName.compareToAscii( SN_THESAURUS ))
        {
            if (!xThesDsp.is())
                GetThesaurusDsp_Impl();
            pThesDsp->SetServiceList( rLocale, rServiceImplNames );
            bIsModified = TRUE;
        }
        if( bIsModified )
            aSaveTimer.Start(); // in order to write changes to the configuration
    }
}


BOOL LngSvcMgr::SaveCfgSvcs( const String &rServiceName )
{
    BOOL bRes = FALSE;

    LinguDispatcher *pDsp = 0;
    Sequence< Locale > aLocales;

    if (0 == rServiceName.CompareToAscii( SN_SPELLCHECKER ))
    {
        if (!pSpellDsp)
            GetSpellCheckerDsp_Impl();
        pDsp = pSpellDsp;
        aLocales = xSpellDsp->getLocales();
    }
    else if (0 == rServiceName.CompareToAscii( SN_HYPHENATOR ))
    {
        if (!pHyphDsp)
            GetHyphenatorDsp_Impl();
        pDsp = pHyphDsp;
        aLocales = xHyphDsp->getLocales();
    }
    else if (0 == rServiceName.CompareToAscii( SN_THESAURUS ))
    {
        if (!pThesDsp)
            GetThesaurusDsp_Impl();
        pDsp = pThesDsp;
        aLocales = xThesDsp->getLocales();
    }

    if (pDsp  &&  aLocales.getLength())
    {
        String aRoot( String::CreateFromAscii( "Office.Linguistic/ServiceManager" ) );
        SvtLinguConfigItem aCfg( aRoot );

        INT32 nLen = aLocales.getLength();
        const Locale *pLocale = aLocales.getConstArray();
        for (INT32 i = 0;  i < nLen;  ++i)
        {
            Sequence< OUString > aSvcImplNames;
            aSvcImplNames = pDsp->GetServiceList( pLocale[i] );

#ifdef DEBUG
            INT32 nSvcs = aSvcImplNames.getLength();
            const OUString *pSvcImplName = aSvcImplNames.getConstArray();
            for (INT32 j = 0;  j < nSvcs;  ++j)
            {
                OUString aImplName( pSvcImplName[j] );
            }
#endif
            // build value to be written back to configuration
            Any aCfgAny;
            if (pDsp == pHyphDsp)
            {
                if (aSvcImplNames.getLength())
                    aCfgAny <<= aSvcImplNames.getConstArray()[0];
            }
            else
                aCfgAny <<= aSvcImplNames;
            DBG_ASSERT( aCfgAny.hasValue(), "missing value for 'Any' type" );

            // get node name to be used
            const char *pNodeName = NULL;
            if (pDsp == pSpellDsp)
                pNodeName = "SpellCheckerList";
            else if (pDsp == pThesDsp)
                pNodeName = "ThesaurusList";
            else if (pDsp == pHyphDsp)
                pNodeName = "HyphenatorList";
            else
                DBG_ERROR( "node name missing" );
            OUString aNodeName( A2OU(pNodeName) );

            // build property value(s) to be set
            Sequence< PropertyValue > aValues( 1 );
            PropertyValue &rPropVal = aValues.getArray()[0];
            OUString aCfgLocaleStr( ConvertLanguageToIsoString(
                                        LocaleToLanguage( pLocale[i] ) ) );
            rPropVal.Value  = aCfgAny;
            rPropVal.Name   = aNodeName;
            rPropVal.Name  += OUString::valueOf( (sal_Unicode)'/' );
            rPropVal.Name  += aCfgLocaleStr;

            // add new or replace existing entry.
            //! First argument needs to be of <... cfg:type="set" ...> for
            //! this function call. Second argument is sequence of
            //! PropertyValues (name + value)
            if (pNodeName)
                bRes |= aCfg.SetSetProperties( aNodeName, aValues );
        }
    }

    if( bRes )
        bIsModified = FALSE;

    return bRes;
}


IMPL_LINK( LngSvcMgr, TimeOut, Timer*, p )
{
    if( bIsModified )
    {
        SaveCfgSvcs( A2OU( SN_SPELLCHECKER ) );
        SaveCfgSvcs( A2OU( SN_HYPHENATOR ) );
        SaveCfgSvcs( A2OU( SN_THESAURUS ) );
    }
    return 0;
}

static Sequence< OUString > GetLangSvcList( const Any &rVal )
{
    Sequence< OUString > aRes;

    if (rVal.hasValue())
    {
        rVal >>= aRes;
#ifdef DEBUG
        INT32 nSvcs = aRes.getLength();
        if (nSvcs)
        {
            const OUString *pSvcName = aRes.getConstArray();
            for (INT32 j = 0;  j < nSvcs;  ++j)
            {
                OUString aImplName( pSvcName[j] );
            }
        }
#endif
    }

    return aRes;
}


static Sequence< OUString > GetLangSvc( const Any &rVal )
{
    Sequence< OUString > aRes(1);

    if (rVal.hasValue())
    {
        rVal >>= aRes.getArray()[0];
#ifdef DEBUG
        OUString aImplName( aRes.getConstArray()[0] );
#endif
    }
    else
    {
        aRes.realloc( 0 );
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
    String aCfgLocale( ConvertLanguageToIsoString( nLanguage ) );

    String aRoot( String::CreateFromAscii( "Office.Linguistic/ServiceManager" ) );
    SvtLinguConfigItem aCfg( aRoot );

    Sequence< Any > aValues;
    Sequence< OUString > aName( 1 );
    OUString *pNames = aName.getArray();
    if ( 0 == rServiceName.compareToAscii( SN_SPELLCHECKER ) )
    {
        String aPropName( String::CreateFromAscii( "SpellCheckerList/" ) );
        aPropName += aCfgLocale;
        pNames[0] = aPropName;
        aValues = aCfg.GetProperties( aName );
        if (aValues.getLength())
            aSvcImplNames = GetLangSvcList( aValues.getConstArray()[0] );
    }
    else if ( 0 == rServiceName.compareToAscii( SN_THESAURUS ) )
    {
        String aPropName( String::CreateFromAscii( "ThesaurusList/" ) );
        aPropName += aCfgLocale;
        pNames[0] = aPropName;
        aValues = aCfg.GetProperties( aName );
        if (aValues.getLength())
            aSvcImplNames = GetLangSvcList( aValues.getConstArray()[0] );
    }
    else if ( 0 == rServiceName.compareToAscii( SN_HYPHENATOR ) )
    {
        String aPropName( String::CreateFromAscii( "HyphenatorList/" ) );
        aPropName += aCfgLocale;
        pNames[0] = aPropName;
        aValues = aCfg.GetProperties( aName );
        if (aValues.getLength())
            aSvcImplNames = GetLangSvc( aValues.getConstArray()[0] );
    }

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
            const Reference< XMultiServiceFactory > & rSMgr )
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

