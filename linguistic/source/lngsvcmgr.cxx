/*************************************************************************
 *
 *  $RCSfile: lngsvcmgr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-11-17 12:37:40 $
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

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

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

#include "lngsvcmgr.hxx"
#include "misc.hxx"
#include "spelldsp.hxx"
#include "hyphdsp.hxx"
#include "thesdsp.hxx"

#include <cppuhelper/extract.hxx>
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif
#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif


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


void SetAvailableServiceLists( const SvcInfoArray &rInfoArray,
        LinguDispatcher &rDispatcher )
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


///////////////////////////////////////////////////////////////////////////


class LngSvcMgrListenerHelper :
    public cppu::WeakImplHelper2
    <
        XLinguServiceEventListener,
        XDictionaryListEventListener
    >
{
    //cppu::OMultiTypeInterfaceContainerHelper  aListeners;
    ::cppu::OInterfaceContainerHelper           aLngSvcMgrListeners;
    ::cppu::OInterfaceContainerHelper           aLngSvcEvtBroadcasters;
    Reference< XDictionaryList >                xDicList;
    Reference< XInterface >                     xMyEvtObj;

    // disallow copy-constructor and assignment-operator for now
    LngSvcMgrListenerHelper(const LngSvcMgrListenerHelper &);
    LngSvcMgrListenerHelper & operator = (const LngSvcMgrListenerHelper &);

    void    LaunchEvent( INT16 nLngSvcEvtFlags );

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


void SAL_CALL
    LngSvcMgrListenerHelper::processLinguServiceEvent(
            const LinguServiceEvent& rLngSvcEvent )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    // change event source to LinguServiceManager since the listeners
    // probably do not know (and need not to know) about the specific
    // SpellChecker's or Hyphenator's.
    LinguServiceEvent aEvtObj( rLngSvcEvent );
    aEvtObj.Source = xMyEvtObj;

    // pass event on to XLinguServiceEventListener's
    cppu::OInterfaceIteratorHelper aIt( aLngSvcMgrListeners );
    while (aIt.hasMoreElements())
    {
        Reference< XLinguServiceEventListener > xRef( aIt.next(), UNO_QUERY );
        if (xRef.is())
            xRef->processLinguServiceEvent( aEvtObj );
    }
}


void SAL_CALL
    LngSvcMgrListenerHelper::processDictionaryListEvent(
            const DictionaryListEvent& rDicListEvent )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    // we do keep the original event source here though...

    // pass event on to XDictionaryListEventListener's
    cppu::OInterfaceIteratorHelper aIt( aLngSvcMgrListeners );
    while (aIt.hasMoreElements())
    {
        Reference< XDictionaryListEventListener > xRef( aIt.next(), UNO_QUERY );
        if (xRef.is())
            xRef->processDictionaryListEvent( rDicListEvent );
    }

    // "translate" DictionaryList event into LinguServiceEvent
    INT16 nLngSvcEvt = 0;
    INT16 nDlEvt = rDicListEvent.nCondensedEvent;
    if ( (nDlEvt & DictionaryListEventFlags::ADD_NEG_ENTRY) ||
         (nDlEvt & DictionaryListEventFlags::DEL_POS_ENTRY) ||
         (nDlEvt & DictionaryListEventFlags::ACTIVATE_NEG_DIC) ||
         (nDlEvt & DictionaryListEventFlags::DEACTIVATE_POS_DIC) )
        nLngSvcEvt |= LinguServiceEventFlags::SPELL_CORRECT_WORDS_AGAIN;
    if ( (nDlEvt & DictionaryListEventFlags::ADD_POS_ENTRY) ||
         (nDlEvt & DictionaryListEventFlags::DEL_NEG_ENTRY) ||
         (nDlEvt & DictionaryListEventFlags::ACTIVATE_POS_DIC) ||
         (nDlEvt & DictionaryListEventFlags::DEACTIVATE_NEG_DIC) )
        nLngSvcEvt |= LinguServiceEventFlags::SPELL_WRONG_WORDS_AGAIN;
    if ( (nDlEvt & DictionaryListEventFlags::ADD_POS_ENTRY) ||
         (nDlEvt & DictionaryListEventFlags::DEL_POS_ENTRY) ||
         (nDlEvt & DictionaryListEventFlags::ACTIVATE_POS_DIC) ||
         (nDlEvt & DictionaryListEventFlags::ACTIVATE_NEG_DIC) )
        nLngSvcEvt |= LinguServiceEventFlags::HYPHENATE_AGAIN;
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
    bDisposing = FALSE;

    pSpellDsp   = 0;
    pHyphDsp    = 0;
    pThesDsp    = 0;

    pSpellSvcs  = 0;
    pHyphSvcs   = 0;
    pThesSvcs   = 0;
    pListenerHelper = 0;
}


LngSvcMgr::~LngSvcMgr()
{
    // memory for pSpellDsp, pHyphDsp, pThesDsp, pListenerHelper
    // will be freed in the destructor of the respective Reference's
    // xSpellDsp, xHyphDsp, xThesDsp

    delete pSpellSvcs;
    delete pHyphSvcs;
    delete pThesSvcs;
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
    if (!pSpellSvcs)
    {
        pSpellSvcs = new SvcInfoArray;

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

                        pSpellSvcs->Insert( new SvcInfo( aImplName, aLanguages ),
                                            pSpellSvcs->Count() );

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
    if (!pHyphSvcs)
    {
        pHyphSvcs = new SvcInfoArray;

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

                        pHyphSvcs->Insert( new SvcInfo( aImplName, aLanguages ),
                                            pHyphSvcs->Count() );

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
    if (!pThesSvcs)
    {
        pThesSvcs = new SvcInfoArray;

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

                        pThesSvcs->Insert( new SvcInfo( aImplName, aLanguages ),
                                            pThesSvcs->Count() );

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
    // TL_TODO:
    // the services obtained from the configuration should be set here

    if (!pSpellSvcs)
        GetAvailableSpellSvcs_Impl();
    SetAvailableServiceLists( *pSpellSvcs, rSpellDsp );
}


void LngSvcMgr::SetCfgServiceLists( HyphenatorDispatcher &rHyphDsp )
{
    // TL_TODO:
    // the services obtained from the configuration should be set here

    if (!pHyphSvcs)
        GetAvailableHyphSvcs_Impl();
    SetAvailableServiceLists( *pHyphSvcs, rHyphDsp );
}


void LngSvcMgr::SetCfgServiceLists( ThesaurusDispatcher &rThesDsp )
{
    // TL_TODO:
    // the services obtained from the configuration should be set here

    if (!pThesSvcs)
        GetAvailableThesSvcs_Impl();
    SetAvailableServiceLists( *pThesSvcs, rThesDsp );
}


Reference< XSpellChecker > SAL_CALL
    LngSvcMgr::getSpellChecker()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

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
        if (!pSpellSvcs)
            GetAvailableSpellSvcs_Impl();
        pInfoArray = pSpellSvcs;
    }
    else if (0 == rServiceName.compareToAscii( SN_HYPHENATOR ))
    {
        if (!pHyphSvcs)
            GetAvailableHyphSvcs_Impl();
        pInfoArray = pHyphSvcs;
    }
    else if (0 == rServiceName.compareToAscii( SN_THESAURUS ))
    {
        if (!pThesSvcs)
            GetAvailableThesSvcs_Impl();
        pInfoArray = pThesSvcs;
    }

    if (pInfoArray)
    {
        // resize to max number of entries
        INT32 nMaxCnt = pInfoArray->Count();
        aRes.realloc( nMaxCnt );
        OUString *pImplName = aRes.getArray();

        INT32 nCnt = 0;
        INT16 nLanguage = LocaleToLanguage( rLocale );
        for (INT32 i = 0;  i < nMaxCnt;  ++i)
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


void SAL_CALL
    LngSvcMgr::setConfiguredServices(
            const OUString& rServiceName,
            const Locale& rLocale,
            const Sequence< OUString >& rServiceImplNames )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    // TL_TODO:
    // write code to access the configuration and set the following
    // variable accordingly
    BOOL bCfgChgSuccess = TRUE;
    INT16 nLanguage = LocaleToLanguage( rLocale );

    if (bCfgChgSuccess  &&  LANGUAGE_NONE != nLanguage)
    {
        if (0 == rServiceName.compareToAscii( SN_SPELLCHECKER ))
            pSpellDsp->SetServiceList( rLocale, rServiceImplNames );
        else if (0 == rServiceName.compareToAscii( SN_HYPHENATOR ))
            pHyphDsp->SetServiceList( rLocale, rServiceImplNames );
        else if (0 == rServiceName.compareToAscii( SN_THESAURUS ))
            pThesDsp->SetServiceList( rLocale, rServiceImplNames );
    }
}


Sequence< OUString > SAL_CALL
    LngSvcMgr::getConfiguredServices(
            const OUString& rServiceName,
            const Locale& rLocale )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return Sequence< OUString > ();
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

