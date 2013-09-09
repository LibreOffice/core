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


#include <cppuhelper/factory.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <osl/file.hxx>
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/useroptions.hxx>
#include <cppuhelper/factory.hxx>   // helper for factories
#include <unotools/localfilehelper.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/linguistic2/DictionaryEventFlags.hpp>
#include <com/sun/star/linguistic2/DictionaryListEventFlags.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>

#include "defs.hxx"
#include "dlistimp.hxx"
#include "dicimp.hxx"
#include "lngopt.hxx"

using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;



static sal_Bool IsVers2OrNewer( const OUString& rFileURL, sal_uInt16& nLng, sal_Bool& bNeg );

static void AddInternal( const uno::Reference< XDictionary > &rDic,
                         const OUString& rNew );
static void AddUserData( const uno::Reference< XDictionary > &rDic );


class DicEvtListenerHelper :
    public cppu::WeakImplHelper1
    <
        XDictionaryEventListener
    >
{
    cppu::OInterfaceContainerHelper         aDicListEvtListeners;
    uno::Sequence< DictionaryEvent >        aCollectDicEvt;
    uno::Reference< XDictionaryList >       xMyDicList;

    sal_Int16                               nCondensedEvt;
    sal_Int16                               nNumCollectEvtListeners,
                                         nNumVerboseListeners;

public:
    DicEvtListenerHelper( const uno::Reference< XDictionaryList > &rxDicList );
    virtual ~DicEvtListenerHelper();

    // XEventListener
    virtual void SAL_CALL
        disposing( const EventObject& rSource )
            throw(RuntimeException);

    // XDictionaryEventListener
    virtual void SAL_CALL
        processDictionaryEvent( const DictionaryEvent& rDicEvent )
            throw(RuntimeException);

    // non-UNO functions
    void    DisposeAndClear( const EventObject &rEvtObj );

    sal_Bool    AddDicListEvtListener(
                const uno::Reference< XDictionaryListEventListener >& rxListener,
                sal_Bool bReceiveVerbose );
    sal_Bool    RemoveDicListEvtListener(
                const uno::Reference< XDictionaryListEventListener >& rxListener );
    sal_Int16   BeginCollectEvents();
    sal_Int16   EndCollectEvents();
    sal_Int16   FlushEvents();
    void    ClearEvents()   { nCondensedEvt = 0; }
};


DicEvtListenerHelper::DicEvtListenerHelper(
        const uno::Reference< XDictionaryList > &rxDicList ) :
    aDicListEvtListeners    ( GetLinguMutex() ),
    xMyDicList              ( rxDicList )
{
    nCondensedEvt   = 0;
    nNumCollectEvtListeners = nNumVerboseListeners  = 0;
}


DicEvtListenerHelper::~DicEvtListenerHelper()
{
    DBG_ASSERT(aDicListEvtListeners.getLength() == 0,
        "lng : event listeners are still existing");
}


void DicEvtListenerHelper::DisposeAndClear( const EventObject &rEvtObj )
{
    aDicListEvtListeners.disposeAndClear( rEvtObj );
}


void SAL_CALL DicEvtListenerHelper::disposing( const EventObject& rSource )
        throw(RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    uno::Reference< XInterface > xSrc( rSource.Source );

    // remove event object from EventListener list
    if (xSrc.is())
        aDicListEvtListeners.removeInterface( xSrc );

    // if object is a dictionary then remove it from the dictionary list
    // Note: this will probably happen only if someone makes a XDictionary
    // implementation of his own that is also a XComponent.
    uno::Reference< XDictionary > xDic( xSrc, UNO_QUERY );
    if (xDic.is())
    {
        xMyDicList->removeDictionary( xDic );
    }
}


void SAL_CALL DicEvtListenerHelper::processDictionaryEvent(
            const DictionaryEvent& rDicEvent )
        throw(RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    uno::Reference< XDictionary > xDic( rDicEvent.Source, UNO_QUERY );
    DBG_ASSERT(xDic.is(), "lng : missing event source");

    // assert that there is a corresponding dictionary entry if one was
    // added or deleted
    uno::Reference< XDictionaryEntry > xDicEntry( rDicEvent.xDictionaryEntry, UNO_QUERY );
    DBG_ASSERT( !(rDicEvent.nEvent &
                    (DictionaryEventFlags::ADD_ENTRY | DictionaryEventFlags::DEL_ENTRY))
                || xDicEntry.is(),
                "lng : missing dictionary entry" );

    // evaluate DictionaryEvents and update data for next DictionaryListEvent
    DictionaryType eDicType = xDic->getDictionaryType();
    DBG_ASSERT(eDicType != DictionaryType_MIXED,
        "lng : unexpected dictionary type");
    if ((rDicEvent.nEvent & DictionaryEventFlags::ADD_ENTRY) && xDic->isActive())
        nCondensedEvt |= xDicEntry->isNegative() ?
            DictionaryListEventFlags::ADD_NEG_ENTRY :
            DictionaryListEventFlags::ADD_POS_ENTRY;
    if ((rDicEvent.nEvent & DictionaryEventFlags::DEL_ENTRY) && xDic->isActive())
        nCondensedEvt |= xDicEntry->isNegative() ?
            DictionaryListEventFlags::DEL_NEG_ENTRY :
            DictionaryListEventFlags::DEL_POS_ENTRY;
    if ((rDicEvent.nEvent & DictionaryEventFlags::ENTRIES_CLEARED) && xDic->isActive())
        nCondensedEvt |= eDicType == DictionaryType_NEGATIVE ?
            DictionaryListEventFlags::DEL_NEG_ENTRY :
            DictionaryListEventFlags::DEL_POS_ENTRY;
    if ((rDicEvent.nEvent & DictionaryEventFlags::CHG_LANGUAGE) && xDic->isActive())
        nCondensedEvt |= eDicType == DictionaryType_NEGATIVE ?
            DictionaryListEventFlags::DEACTIVATE_NEG_DIC
                | DictionaryListEventFlags::ACTIVATE_NEG_DIC :
            DictionaryListEventFlags::DEACTIVATE_POS_DIC
                | DictionaryListEventFlags::ACTIVATE_POS_DIC;
    if ((rDicEvent.nEvent & DictionaryEventFlags::ACTIVATE_DIC))
        nCondensedEvt |= eDicType == DictionaryType_NEGATIVE ?
            DictionaryListEventFlags::ACTIVATE_NEG_DIC :
            DictionaryListEventFlags::ACTIVATE_POS_DIC;
    if ((rDicEvent.nEvent & DictionaryEventFlags::DEACTIVATE_DIC))
        nCondensedEvt |= eDicType == DictionaryType_NEGATIVE ?
            DictionaryListEventFlags::DEACTIVATE_NEG_DIC :
            DictionaryListEventFlags::DEACTIVATE_POS_DIC;

    // update list of collected events if needs to be
    if (nNumVerboseListeners > 0)
    {
        sal_Int32 nColEvts = aCollectDicEvt.getLength();
        aCollectDicEvt.realloc( nColEvts + 1 );
        aCollectDicEvt.getArray()[ nColEvts ] = rDicEvent;
    }

    if (nNumCollectEvtListeners == 0 && nCondensedEvt != 0)
        FlushEvents();
}


sal_Bool DicEvtListenerHelper::AddDicListEvtListener(
            const uno::Reference< XDictionaryListEventListener >& xListener,
            sal_Bool /*bReceiveVerbose*/ )
{
    DBG_ASSERT( xListener.is(), "empty reference" );
    sal_Int32   nCount = aDicListEvtListeners.getLength();
    return aDicListEvtListeners.addInterface( xListener ) != nCount;
}


sal_Bool DicEvtListenerHelper::RemoveDicListEvtListener(
            const uno::Reference< XDictionaryListEventListener >& xListener )
{
    DBG_ASSERT( xListener.is(), "empty reference" );
    sal_Int32   nCount = aDicListEvtListeners.getLength();
    return aDicListEvtListeners.removeInterface( xListener ) != nCount;
}


sal_Int16 DicEvtListenerHelper::BeginCollectEvents()
{
    return ++nNumCollectEvtListeners;
}


sal_Int16 DicEvtListenerHelper::EndCollectEvents()
{
    DBG_ASSERT(nNumCollectEvtListeners > 0, "lng: mismatched function call");
    if (nNumCollectEvtListeners > 0)
    {
        FlushEvents();
        nNumCollectEvtListeners--;
    }

    return nNumCollectEvtListeners;
}


sal_Int16 DicEvtListenerHelper::FlushEvents()
{
    if (0 != nCondensedEvt)
    {
        // build DictionaryListEvent to pass on to listeners
        uno::Sequence< DictionaryEvent > aDicEvents;
        if (nNumVerboseListeners > 0)
            aDicEvents = aCollectDicEvt;
        DictionaryListEvent aEvent( xMyDicList, nCondensedEvt, aDicEvents );

        // pass on event
        cppu::OInterfaceIteratorHelper aIt( aDicListEvtListeners );
        while (aIt.hasMoreElements())
        {
            uno::Reference< XDictionaryListEventListener > xRef( aIt.next(), UNO_QUERY );
            if (xRef.is())
                xRef->processDictionaryListEvent( aEvent );
        }

        // clear "list" of events
        nCondensedEvt = 0;
        aCollectDicEvt.realloc( 0 );
    }

    return nNumCollectEvtListeners;
}




void DicList::MyAppExitListener::AtExit()
{
    rMyDicList.SaveDics();
}


DicList::DicList() :
    aEvtListeners   ( GetLinguMutex() )
{
    pDicEvtLstnrHelper  = new DicEvtListenerHelper( this );
    xDicEvtLstnrHelper  = pDicEvtLstnrHelper;
    bDisposing = sal_False;
    bInCreation = sal_False;

    pExitListener = new MyAppExitListener( *this );
    xExitListener = pExitListener;
    pExitListener->Activate();
}

DicList::~DicList()
{
    pExitListener->Deactivate();
}


void DicList::SearchForDictionaries(
    DictionaryVec_t&rDicList,
    const OUString &rDicDirURL,
    sal_Bool bIsWriteablePath )
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    const uno::Sequence< OUString > aDirCnt( utl::LocalFileHelper::
                                        GetFolderContents( rDicDirURL, sal_False ) );
    const OUString *pDirCnt = aDirCnt.getConstArray();
    sal_Int32 nEntries = aDirCnt.getLength();

    OUString aDCN("dcn");
    OUString aDCP("dcp");
    for (sal_Int32 i = 0;  i < nEntries;  ++i)
    {
        OUString  aURL( pDirCnt[i] );
        sal_uInt16  nLang = LANGUAGE_NONE;
        sal_Bool    bNeg  = sal_False;

        if(!::IsVers2OrNewer( aURL, nLang, bNeg ))
        {
            // When not
            sal_Int32 nPos  = aURL.indexOf('.');
            OUString aExt( aURL.copy(nPos + 1).toAsciiLowerCase() );

            if (aDCN.equals(aExt))       // negativ
                bNeg = sal_True;
            else if (aDCP.equals(aExt))  // positiv
                bNeg = sal_False;
            else
                continue;          // andere Files
        }

        // Record in the list of Dictoinaries
        // When it already exists don't record
        sal_Int16 nSystemLanguage = MsLangId::getSystemLanguage();
        OUString aTmp1 = ToLower( aURL, nSystemLanguage );
        sal_Int32 nPos = aTmp1.lastIndexOf( '/' );
        if (-1 != nPos)
            aTmp1 = aTmp1.copy( nPos + 1 );
        OUString aTmp2;
        size_t j;
        size_t nCount = rDicList.size();
        for(j = 0;  j < nCount;  j++)
        {
            aTmp2 = rDicList[j]->getName().getStr();
            aTmp2 = ToLower( aTmp2, nSystemLanguage );
            if(aTmp1 == aTmp2)
                break;
        }
        if(j >= nCount)     // dictionary not yet in DicList
        {
            // get decoded dictionary file name
            INetURLObject aURLObj( aURL );
            OUString aDicName = aURLObj.getName( INetURLObject::LAST_SEGMENT,
                        true, INetURLObject::DECODE_WITH_CHARSET,
                        RTL_TEXTENCODING_UTF8 );

            DictionaryType eType = bNeg ? DictionaryType_NEGATIVE : DictionaryType_POSITIVE;
            uno::Reference< XDictionary > xDic =
                        new DictionaryNeo( aDicName, nLang, eType, aURL, bIsWriteablePath );

            addDictionary( xDic );
            nCount++;
        }
    }
}


sal_Int32 DicList::GetDicPos(const uno::Reference< XDictionary > &xDic)
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    sal_Int32 nPos = -1;
    DictionaryVec_t& rDicList = GetOrCreateDicList();
    size_t n = rDicList.size();
    for (size_t i = 0;  i < n;  i++)
    {
        if ( rDicList[i] == xDic )
            return i;
    }
    return nPos;
}


uno::Reference< XInterface > SAL_CALL
    DicList_CreateInstance( const uno::Reference< XMultiServiceFactory > & /*rSMgr*/ )
            throw(Exception)
{
    uno::Reference< XInterface > xService = (cppu::OWeakObject *) new DicList;
    return xService;
}

sal_Int16 SAL_CALL DicList::getCount() throw(RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );
    return static_cast< sal_Int16 >(GetOrCreateDicList().size());
}

uno::Sequence< uno::Reference< XDictionary > > SAL_CALL
        DicList::getDictionaries()
            throw(RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    DictionaryVec_t& rDicList = GetOrCreateDicList();

    uno::Sequence< uno::Reference< XDictionary > > aDics( rDicList.size() );
    uno::Reference< XDictionary > *pDic = aDics.getArray();

    sal_Int32 n = (sal_uInt16) aDics.getLength();
    for (sal_Int32 i = 0;  i < n;  i++)
        pDic[i] = rDicList[i];

    return aDics;
}

uno::Reference< XDictionary > SAL_CALL
        DicList::getDictionaryByName( const OUString& aDictionaryName )
            throw(RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    uno::Reference< XDictionary > xDic;
    DictionaryVec_t& rDicList = GetOrCreateDicList();
    size_t nCount = rDicList.size();
    for (size_t i = 0;  i < nCount;  i++)
    {
        const uno::Reference< XDictionary > &rDic = rDicList[i];
        if (rDic.is()  &&  rDic->getName() == aDictionaryName)
        {
            xDic = rDic;
            break;
        }
    }

    return xDic;
}

sal_Bool SAL_CALL DicList::addDictionary(
            const uno::Reference< XDictionary >& xDictionary )
        throw(RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    if (bDisposing)
        return sal_False;

    sal_Bool bRes = sal_False;
    if (xDictionary.is())
    {
        DictionaryVec_t& rDicList = GetOrCreateDicList();
        rDicList.push_back( xDictionary );
        bRes = sal_True;

        // add listener helper to the dictionaries listener lists
        xDictionary->addDictionaryEventListener( xDicEvtLstnrHelper );
    }
    return bRes;
}

sal_Bool SAL_CALL
    DicList::removeDictionary( const uno::Reference< XDictionary >& xDictionary )
        throw(RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    if (bDisposing)
        return sal_False;

    sal_Bool  bRes = sal_False;
    sal_Int32 nPos = GetDicPos( xDictionary );
    if (nPos >= 0)
    {
        // remove dictionary list from the dictionaries listener lists
        DictionaryVec_t& rDicList = GetOrCreateDicList();
        uno::Reference< XDictionary > xDic( rDicList[ nPos ] );
        DBG_ASSERT(xDic.is(), "lng : empty reference");
        if (xDic.is())
        {
            // deactivate dictionary if not already done
            xDic->setActive( sal_False );

            xDic->removeDictionaryEventListener( xDicEvtLstnrHelper );
        }

        // remove element at nPos
        rDicList.erase( rDicList.begin() + nPos );
        bRes = sal_True;
    }
    return bRes;
}

sal_Bool SAL_CALL DicList::addDictionaryListEventListener(
            const uno::Reference< XDictionaryListEventListener >& xListener,
            sal_Bool bReceiveVerbose )
        throw(RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    if (bDisposing)
        return sal_False;

    DBG_ASSERT(!bReceiveVerbose, "lng : not yet supported");

    sal_Bool bRes = sal_False;
    if (xListener.is()) //! don't add empty references
    {
        bRes = pDicEvtLstnrHelper->
                        AddDicListEvtListener( xListener, bReceiveVerbose );
    }
    return bRes;
}

sal_Bool SAL_CALL DicList::removeDictionaryListEventListener(
            const uno::Reference< XDictionaryListEventListener >& xListener )
        throw(RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    if (bDisposing)
        return sal_False;

    sal_Bool bRes = sal_False;
    if(xListener.is())
    {
        bRes = pDicEvtLstnrHelper->RemoveDicListEvtListener( xListener );
    }
    return bRes;
}

sal_Int16 SAL_CALL DicList::beginCollectEvents() throw(RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );
    return pDicEvtLstnrHelper->BeginCollectEvents();
}

sal_Int16 SAL_CALL DicList::endCollectEvents() throw(RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );
    return pDicEvtLstnrHelper->EndCollectEvents();
}

sal_Int16 SAL_CALL DicList::flushEvents() throw(RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );
    return pDicEvtLstnrHelper->FlushEvents();
}

uno::Reference< XDictionary > SAL_CALL
    DicList::createDictionary( const OUString& rName, const Locale& rLocale,
            DictionaryType eDicType, const OUString& rURL )
        throw(RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    sal_Int16 nLanguage = LinguLocaleToLanguage( rLocale );
    bool bIsWriteablePath = rURL.match( GetDictionaryWriteablePath(), 0 );
    return new DictionaryNeo( rName, nLanguage, eDicType, rURL, bIsWriteablePath );
}


uno::Reference< XDictionaryEntry > SAL_CALL
    DicList::queryDictionaryEntry( const OUString& rWord, const Locale& rLocale,
            sal_Bool bSearchPosDics, sal_Bool bSearchSpellEntry )
        throw(RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );
    return SearchDicList( this, rWord, LinguLocaleToLanguage( rLocale ),
                            bSearchPosDics, bSearchSpellEntry );
}


void SAL_CALL
    DicList::dispose()
        throw(RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    if (!bDisposing)
    {
        bDisposing = sal_True;
        EventObject aEvtObj( (XDictionaryList *) this );

        aEvtListeners.disposeAndClear( aEvtObj );
        if (pDicEvtLstnrHelper)
            pDicEvtLstnrHelper->DisposeAndClear( aEvtObj );

        //! avoid creation of dictionaries if not already done
        if ( !aDicList.empty() )
        {
            DictionaryVec_t& rDicList = GetOrCreateDicList();
            size_t nCount = rDicList.size();
            for (size_t i = 0;  i < nCount;  i++)
            {
                uno::Reference< XDictionary > xDic( rDicList[i], UNO_QUERY );

                // save (modified) dictionaries
                uno::Reference< frame::XStorable >  xStor( xDic , UNO_QUERY );
                if (xStor.is())
                {
                    try
                    {
                        if (!xStor->isReadonly() && xStor->hasLocation())
                            xStor->store();
                    }
                    catch(Exception &)
                    {
                    }
                }

                // release references to (members of) this object hold by
                // dictionaries
                if (xDic.is())
                    xDic->removeDictionaryEventListener( xDicEvtLstnrHelper );
            }
        }
        xDicEvtLstnrHelper.clear();
    }
}

void SAL_CALL
    DicList::addEventListener( const uno::Reference< XEventListener >& rxListener )
        throw(RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
        aEvtListeners.addInterface( rxListener );
}

void SAL_CALL
    DicList::removeEventListener( const uno::Reference< XEventListener >& rxListener )
        throw(RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
        aEvtListeners.removeInterface( rxListener );
}

void DicList::_CreateDicList()
{
    bInCreation = sal_True;

    // look for dictionaries
    const OUString aWriteablePath( GetDictionaryWriteablePath() );
    uno::Sequence< OUString > aPaths( GetDictionaryPaths() );
    const OUString *pPaths = aPaths.getConstArray();
    for (sal_Int32 i = 0;  i < aPaths.getLength();  ++i)
    {
        const sal_Bool bIsWriteablePath = (pPaths[i] == aWriteablePath);
        SearchForDictionaries( aDicList, pPaths[i], bIsWriteablePath );
    }

    // create IgnoreAllList dictionary with empty URL (non persistent)
    // and add it to list
    OUString aDicName( "IgnoreAllList" );
    uno::Reference< XDictionary > xIgnAll(
            createDictionary( aDicName, LinguLanguageToLocale( LANGUAGE_NONE ),
                              DictionaryType_POSITIVE, OUString() ) );
    if (xIgnAll.is())
    {
        AddUserData( xIgnAll );
        xIgnAll->setActive( sal_True );
        addDictionary( xIgnAll );
    }


    // evaluate list of dictionaries to be activated from configuration
    //! to suppress overwriting the list of active dictionaries in the
    //! configuration with incorrect arguments during the following
    //! activation of the dictionaries
    pDicEvtLstnrHelper->BeginCollectEvents();
    const uno::Sequence< OUString > aActiveDics( aOpt.GetActiveDics() );
    const OUString *pActiveDic = aActiveDics.getConstArray();
    sal_Int32 nLen = aActiveDics.getLength();
    for (sal_Int32 i = 0;  i < nLen;  ++i)
    {
        if (!pActiveDic[i].isEmpty())
        {
            uno::Reference< XDictionary > xDic( getDictionaryByName( pActiveDic[i] ) );
            if (xDic.is())
                xDic->setActive( sal_True );
        }
    }

    // suppress collected events during creation of the dictionary list.
    // there should be no events during creation.
    pDicEvtLstnrHelper->ClearEvents();

    pDicEvtLstnrHelper->EndCollectEvents();

    bInCreation = sal_False;
}


void DicList::SaveDics()
{
    // save dics only if they have already been used/created.
    //! don't create them just for the purpose of saving them !
    if ( !aDicList.empty() )
    {
        // save (modified) dictionaries
        DictionaryVec_t& rDicList = GetOrCreateDicList();
        size_t nCount = rDicList.size();;
        for (size_t i = 0;  i < nCount;  i++)
        {
            // save (modified) dictionaries
            uno::Reference< frame::XStorable >  xStor( rDicList[i], UNO_QUERY );
            if (xStor.is())
            {
                try
                {
                    if (!xStor->isReadonly() && xStor->hasLocation())
                        xStor->store();
                }
                catch(Exception &)
                {
                }
            }
        }
    }
}


// Service specific part

OUString SAL_CALL DicList::getImplementationName(  ) throw(RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );
    return getImplementationName_Static();
}


sal_Bool SAL_CALL DicList::supportsService( const OUString& ServiceName )
        throw(RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    uno::Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;
    return sal_False;
}


uno::Sequence< OUString > SAL_CALL DicList::getSupportedServiceNames(  )
        throw(RuntimeException)
{
    osl::MutexGuard aGuard( GetLinguMutex() );
    return getSupportedServiceNames_Static();
}


uno::Sequence< OUString > DicList::getSupportedServiceNames_Static() throw()
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    uno::Sequence< OUString > aSNS( 1 );   // more than 1 service possible
    aSNS.getArray()[0] = "com.sun.star.linguistic2.DictionaryList";
    return aSNS;
}

void * SAL_CALL DicList_getFactory( const sal_Char * pImplName,
        XMultiServiceFactory * pServiceManager, void *  )
{
    void * pRet = 0;
    if ( !DicList::getImplementationName_Static().compareToAscii( pImplName ) )
    {
        uno::Reference< XSingleServiceFactory > xFactory =
            cppu::createOneInstanceFactory(
                pServiceManager,
                DicList::getImplementationName_Static(),
                DicList_CreateInstance,
                DicList::getSupportedServiceNames_Static());
        // acquire, because we return an interface pointer instead of a reference
        xFactory->acquire();
        pRet = xFactory.get();
    }
    return pRet;
}


xub_StrLen lcl_GetToken( OUString &rToken,
            const OUString &rText, xub_StrLen nPos, const OUString &rDelim )
{
    xub_StrLen nRes = STRING_LEN;

    if (rText.isEmpty() ||  nPos >= rText.getLength())
        rToken = "";
    else if (rDelim.isEmpty())
    {
        rToken = rText;
        if (!rToken.isEmpty())
            nRes = rText.getLength();
    }
    else
    {
        xub_StrLen  i;
        for (i = nPos;  i < rText.getLength();  ++i)
        {
            if (-1 != rDelim.indexOf( rText[i] ))
                break;
        }

        if (i >= rText.getLength())   // delimeter not found
            rToken  = rText.copy( nPos );
        else
            rToken  = rText.copy( nPos, (sal_Int32) i - nPos );
        nRes    = i + 1;    // continue after found delimeter
    }

    return nRes;
}


static void AddInternal(
        const uno::Reference<XDictionary> &rDic,
        const OUString& rNew )
{
    if (rDic.is())
    {
        //! TL TODO: word iterator should be used to break up the text
        static const char aDefWordDelim[] =
                "!\"#$%&'()*+,-/:;<=>?[]\\_^`{|}~\t \n";
        OUString aDelim(RTL_CONSTASCII_USTRINGPARAM(aDefWordDelim));
        OSL_ENSURE(aDelim.indexOf(static_cast<sal_Unicode>('.')) == -1,
            "ensure no '.'");

        OUString      aToken;
        xub_StrLen  nPos = 0;
        while (STRING_LEN !=
                    (nPos = lcl_GetToken( aToken, rNew, nPos, aDelim )))
        {
            if( !aToken.isEmpty()  &&  !IsNumeric( aToken ) )
            {
                rDic->add( aToken, sal_False, OUString() );
            }
        }
    }
}

static void AddUserData( const uno::Reference< XDictionary > &rDic )
{
    if (rDic.is())
    {
        SvtUserOptions aUserOpt;
        AddInternal( rDic, aUserOpt.GetFullName() );
        AddInternal( rDic, aUserOpt.GetCompany() );
        AddInternal( rDic, aUserOpt.GetStreet() );
        AddInternal( rDic, aUserOpt.GetCity() );
        AddInternal( rDic, aUserOpt.GetTitle() );
        AddInternal( rDic, aUserOpt.GetPosition() );
        AddInternal( rDic, aUserOpt.GetEmail() );
    }
}

static sal_Bool IsVers2OrNewer( const OUString& rFileURL, sal_uInt16& nLng, sal_Bool& bNeg )
{
    if (rFileURL.isEmpty())
        return sal_False;
    OUString aDIC("dic");
    OUString aExt;
    sal_Int32 nPos = rFileURL.lastIndexOf( '.' );
    if (-1 != nPos)
        aExt = rFileURL.copy( nPos + 1 ).toAsciiLowerCase();

    if (aDIC != aExt)
        return sal_False;

    // get stream to be used
    uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );

    // get XInputStream stream
    uno::Reference< io::XInputStream > xStream;
    try
    {
        uno::Reference< ucb::XSimpleFileAccess3 > xAccess( ucb::SimpleFileAccess::create(xContext) );
        xStream = xAccess->openFileRead( rFileURL );
    }
    catch (const uno::Exception &)
    {
        DBG_ASSERT( 0, "failed to get input stream" );
    }
    DBG_ASSERT( xStream.is(), "failed to get stream for read" );
    if (!xStream.is())
        return sal_False;

    SvStreamPtr pStream = SvStreamPtr( utl::UcbStreamHelper::CreateStream( xStream ) );

    int nDicVersion = ReadDicVersion(pStream, nLng, bNeg);
    if (2 == nDicVersion || nDicVersion >= 5)
        return sal_True;

    return sal_False;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
