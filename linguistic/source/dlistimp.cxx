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
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <unotools/useroptions.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <unotools/localfilehelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/linguistic2/DictionaryEventFlags.hpp>
#include <com/sun/star/linguistic2/DictionaryListEventFlags.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <svtools/strings.hrc>
#include <unotools/resmgr.hxx>
#include <sal/log.hxx>

#include "dlistimp.hxx"
#include "dicimp.hxx"
#include "lngopt.hxx"

using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;


static bool IsVers2OrNewer( const OUString& rFileURL, LanguageType& nLng, bool& bNeg, OUString& aDicName );

static void AddInternal( const uno::Reference< XDictionary > &rDic,
                         const OUString& rNew );
static void AddUserData( const uno::Reference< XDictionary > &rDic );


class DicEvtListenerHelper :
    public cppu::WeakImplHelper
    <
        XDictionaryEventListener
    >
{
    comphelper::OInterfaceContainerHelper2  aDicListEvtListeners;
    uno::Reference< XDictionaryList >       xMyDicList;

    sal_Int16                               nCondensedEvt;
    sal_Int16                               nNumCollectEvtListeners;

public:
    explicit DicEvtListenerHelper( const uno::Reference< XDictionaryList > &rxDicList );
    virtual ~DicEvtListenerHelper() override;

    // XEventListener
    virtual void SAL_CALL
        disposing( const EventObject& rSource ) override;

    // XDictionaryEventListener
    virtual void SAL_CALL
        processDictionaryEvent( const DictionaryEvent& rDicEvent ) override;

    // non-UNO functions
    void    DisposeAndClear( const EventObject &rEvtObj );

    bool    AddDicListEvtListener(
                const uno::Reference< XDictionaryListEventListener >& rxListener );
    bool    RemoveDicListEvtListener(
                const uno::Reference< XDictionaryListEventListener >& rxListener );
    sal_Int16   BeginCollectEvents() { return ++nNumCollectEvtListeners;}
    sal_Int16   EndCollectEvents();
    sal_Int16   FlushEvents();
    void    ClearEvents()   { nCondensedEvt = 0; }
};


DicEvtListenerHelper::DicEvtListenerHelper(
        const uno::Reference< XDictionaryList > &rxDicList ) :
    aDicListEvtListeners    ( GetLinguMutex() ),
    xMyDicList              ( rxDicList ),
    nCondensedEvt(0), nNumCollectEvtListeners(0)
{
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
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    uno::Reference< XDictionary > xDic( rDicEvent.Source, UNO_QUERY );
    DBG_ASSERT(xDic.is(), "lng : missing event source");

    // assert that there is a corresponding dictionary entry if one was
    // added or deleted
    DBG_ASSERT( !(rDicEvent.nEvent &
                    (DictionaryEventFlags::ADD_ENTRY | DictionaryEventFlags::DEL_ENTRY))
                || rDicEvent.xDictionaryEntry.is(),
                "lng : missing dictionary entry" );

    // evaluate DictionaryEvents and update data for next DictionaryListEvent
    DictionaryType eDicType = xDic->getDictionaryType();
    DBG_ASSERT(eDicType != DictionaryType_MIXED,
        "lng : unexpected dictionary type");
    if ((rDicEvent.nEvent & DictionaryEventFlags::ADD_ENTRY) && xDic->isActive())
        nCondensedEvt |= rDicEvent.xDictionaryEntry->isNegative() ?
            DictionaryListEventFlags::ADD_NEG_ENTRY :
            DictionaryListEventFlags::ADD_POS_ENTRY;
    if ((rDicEvent.nEvent & DictionaryEventFlags::DEL_ENTRY) && xDic->isActive())
        nCondensedEvt |= rDicEvent.xDictionaryEntry->isNegative() ?
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
    if (rDicEvent.nEvent & DictionaryEventFlags::ACTIVATE_DIC)
        nCondensedEvt |= eDicType == DictionaryType_NEGATIVE ?
            DictionaryListEventFlags::ACTIVATE_NEG_DIC :
            DictionaryListEventFlags::ACTIVATE_POS_DIC;
    if (rDicEvent.nEvent & DictionaryEventFlags::DEACTIVATE_DIC)
        nCondensedEvt |= eDicType == DictionaryType_NEGATIVE ?
            DictionaryListEventFlags::DEACTIVATE_NEG_DIC :
            DictionaryListEventFlags::DEACTIVATE_POS_DIC;

    if (nNumCollectEvtListeners == 0 && nCondensedEvt != 0)
        FlushEvents();
}


bool DicEvtListenerHelper::AddDicListEvtListener(
            const uno::Reference< XDictionaryListEventListener >& xListener )
{
    DBG_ASSERT( xListener.is(), "empty reference" );
    sal_Int32   nCount = aDicListEvtListeners.getLength();
    return aDicListEvtListeners.addInterface( xListener ) != nCount;
}


bool DicEvtListenerHelper::RemoveDicListEvtListener(
            const uno::Reference< XDictionaryListEventListener >& xListener )
{
    DBG_ASSERT( xListener.is(), "empty reference" );
    sal_Int32   nCount = aDicListEvtListeners.getLength();
    return aDicListEvtListeners.removeInterface( xListener ) != nCount;
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
        DictionaryListEvent aEvent( xMyDicList, nCondensedEvt, aDicEvents );

        // pass on event
        aDicListEvtListeners.notifyEach( &XDictionaryListEventListener::processDictionaryListEvent, aEvent );

        // clear "list" of events
        nCondensedEvt = 0;
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
    mxDicEvtLstnrHelper  = new DicEvtListenerHelper( this );
    bDisposing = false;
    bInCreation = false;

    mxExitListener = new MyAppExitListener( *this );
    mxExitListener->Activate();
}

DicList::~DicList()
{
    mxExitListener->Deactivate();
}


void DicList::SearchForDictionaries(
    DictionaryVec_t&rDicList,
    const OUString &rDicDirURL,
    bool bIsWriteablePath )
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    const uno::Sequence< OUString > aDirCnt( utl::LocalFileHelper::
                                        GetFolderContents( rDicDirURL, false ) );

    for (const OUString& aURL : aDirCnt)
    {
        LanguageType nLang = LANGUAGE_NONE;
        bool         bNeg  = false;
        OUString     aDicTitle = "";

        if(!::IsVers2OrNewer( aURL, nLang, bNeg, aDicTitle ))
        {
            // When not
            sal_Int32 nPos  = aURL.indexOf('.');
            OUString aExt( aURL.copy(nPos + 1).toAsciiLowerCase() );

            if ("dcn" == aExt)       // negative
                bNeg = true;
            else if ("dcp" == aExt)  // positive
                bNeg = false;
            else
                continue;          // other files
        }

        // Record in the list of Dictionaries
        // When it already exists don't record
        LanguageType nSystemLanguage = MsLangId::getSystemLanguage();
        OUString aTmp1 = ToLower( aURL, nSystemLanguage );
        sal_Int32 nPos = aTmp1.lastIndexOf( '/' );
        if (-1 != nPos)
            aTmp1 = aTmp1.copy( nPos + 1 );
        OUString aTmp2;
        size_t j;
        size_t nCount = rDicList.size();
        for(j = 0;  j < nCount;  j++)
        {
            aTmp2 = rDicList[j]->getName();
            aTmp2 = ToLower( aTmp2, nSystemLanguage );
            if(aTmp1 == aTmp2)
                break;
        }
        if(j >= nCount)     // dictionary not yet in DicList
        {
            // get decoded dictionary file name
            INetURLObject aURLObj( aURL );
            OUString aDicName = aURLObj.getName( INetURLObject::LAST_SEGMENT,
                        true, INetURLObject::DecodeMechanism::WithCharset );

            DictionaryType eType = bNeg ? DictionaryType_NEGATIVE : DictionaryType_POSITIVE;
            uno::Reference< XDictionary > xDic =
                        new DictionaryNeo( aDicTitle.isEmpty() ? aDicName : aDicTitle, nLang, eType, aURL, bIsWriteablePath );

            addDictionary( xDic );
            nCount++;
        }
    }
}


sal_Int32 DicList::GetDicPos(const uno::Reference< XDictionary > &xDic)
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    DictionaryVec_t& rDicList = GetOrCreateDicList();
    size_t n = rDicList.size();
    for (size_t i = 0;  i < n;  i++)
    {
        if ( rDicList[i] == xDic )
            return i;
    }
    return -1;
}

sal_Int16 SAL_CALL DicList::getCount()
{
    osl::MutexGuard aGuard( GetLinguMutex() );
    return static_cast< sal_Int16 >(GetOrCreateDicList().size());
}

uno::Sequence< uno::Reference< XDictionary > > SAL_CALL
        DicList::getDictionaries()
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    DictionaryVec_t& rDicList = GetOrCreateDicList();

    return comphelper::containerToSequence(rDicList);
}

uno::Reference< XDictionary > SAL_CALL
        DicList::getDictionaryByName( const OUString& aDictionaryName )
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
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    if (bDisposing)
        return false;

    bool bRes = false;
    if (xDictionary.is())
    {
        DictionaryVec_t& rDicList = GetOrCreateDicList();
        rDicList.push_back( xDictionary );
        bRes = true;

        // add listener helper to the dictionaries listener lists
        xDictionary->addDictionaryEventListener( mxDicEvtLstnrHelper );
    }
    return bRes;
}

sal_Bool SAL_CALL
    DicList::removeDictionary( const uno::Reference< XDictionary >& xDictionary )
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    if (bDisposing)
        return false;

    bool  bRes = false;
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
            xDic->setActive( false );

            xDic->removeDictionaryEventListener( mxDicEvtLstnrHelper );
        }

        // remove element at nPos
        rDicList.erase( rDicList.begin() + nPos );
        bRes = true;
    }
    return bRes;
}

sal_Bool SAL_CALL DicList::addDictionaryListEventListener(
            const uno::Reference< XDictionaryListEventListener >& xListener,
            sal_Bool bReceiveVerbose )
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    if (bDisposing)
        return false;

    DBG_ASSERT(!bReceiveVerbose, "lng : not yet supported");

    bool bRes = false;
    if (xListener.is()) //! don't add empty references
    {
        bRes = mxDicEvtLstnrHelper->AddDicListEvtListener( xListener );
    }
    return bRes;
}

sal_Bool SAL_CALL DicList::removeDictionaryListEventListener(
            const uno::Reference< XDictionaryListEventListener >& xListener )
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    if (bDisposing)
        return false;

    bool bRes = false;
    if(xListener.is())
    {
        bRes = mxDicEvtLstnrHelper->RemoveDicListEvtListener( xListener );
    }
    return bRes;
}

sal_Int16 SAL_CALL DicList::beginCollectEvents()
{
    osl::MutexGuard aGuard( GetLinguMutex() );
    return mxDicEvtLstnrHelper->BeginCollectEvents();
}

sal_Int16 SAL_CALL DicList::endCollectEvents()
{
    osl::MutexGuard aGuard( GetLinguMutex() );
    return mxDicEvtLstnrHelper->EndCollectEvents();
}

sal_Int16 SAL_CALL DicList::flushEvents()
{
    osl::MutexGuard aGuard( GetLinguMutex() );
    return mxDicEvtLstnrHelper->FlushEvents();
}

uno::Reference< XDictionary > SAL_CALL
    DicList::createDictionary( const OUString& rName, const Locale& rLocale,
            DictionaryType eDicType, const OUString& rURL )
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    LanguageType nLanguage = LinguLocaleToLanguage( rLocale );
    bool bIsWriteablePath = rURL.match( GetDictionaryWriteablePath() );
    return new DictionaryNeo( rName, nLanguage, eDicType, rURL, bIsWriteablePath );
}


uno::Reference< XDictionaryEntry > SAL_CALL
    DicList::queryDictionaryEntry( const OUString& rWord, const Locale& rLocale,
            sal_Bool bSearchPosDics, sal_Bool bSearchSpellEntry )
{
    osl::MutexGuard aGuard( GetLinguMutex() );
    return SearchDicList( this, rWord, LinguLocaleToLanguage( rLocale ),
                            bSearchPosDics, bSearchSpellEntry );
}


void SAL_CALL
    DicList::dispose()
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    if (bDisposing)
        return;

    bDisposing = true;
    EventObject aEvtObj( static_cast<XDictionaryList *>(this) );

    aEvtListeners.disposeAndClear( aEvtObj );
    if (mxDicEvtLstnrHelper.is())
        mxDicEvtLstnrHelper->DisposeAndClear( aEvtObj );

    //! avoid creation of dictionaries if not already done
    if ( !aDicList.empty() )
    {
        DictionaryVec_t& rDicList = GetOrCreateDicList();
        size_t nCount = rDicList.size();
        for (size_t i = 0;  i < nCount;  i++)
        {
            // save (modified) dictionaries
            uno::Reference< frame::XStorable >  xStor( rDicList[i] , UNO_QUERY );
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
            if (rDicList[i].is())
                rDicList[i]->removeDictionaryEventListener( mxDicEvtLstnrHelper );
        }
    }
    mxDicEvtLstnrHelper.clear();
}

void SAL_CALL
    DicList::addEventListener( const uno::Reference< XEventListener >& rxListener )
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
        aEvtListeners.addInterface( rxListener );
}

void SAL_CALL
    DicList::removeEventListener( const uno::Reference< XEventListener >& rxListener )
{
    osl::MutexGuard aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
        aEvtListeners.removeInterface( rxListener );
}

void DicList::CreateDicList()
{
    bInCreation = true;

    // look for dictionaries
    const OUString aWriteablePath( GetDictionaryWriteablePath() );
    std::vector< OUString > aPaths( GetDictionaryPaths() );
    for (const OUString & aPath : aPaths)
    {
        const bool bIsWriteablePath = (aPath == aWriteablePath);
        SearchForDictionaries( aDicList, aPath, bIsWriteablePath );
    }

    // create IgnoreAllList dictionary with empty URL (non persistent)
    // and add it to list
    std::locale loc(Translate::Create("svt"));
    uno::Reference< XDictionary > xIgnAll(
            createDictionary( Translate::get(STR_DESCRIPTION_IGNOREALLLIST, loc), LinguLanguageToLocale( LANGUAGE_NONE ),
                              DictionaryType_POSITIVE, OUString() ) );
    if (xIgnAll.is())
    {
        AddUserData( xIgnAll );
        xIgnAll->setActive( true );
        addDictionary( xIgnAll );
    }


    // evaluate list of dictionaries to be activated from configuration
    //! to suppress overwriting the list of active dictionaries in the
    //! configuration with incorrect arguments during the following
    //! activation of the dictionaries
    mxDicEvtLstnrHelper->BeginCollectEvents();
    const uno::Sequence< OUString > aActiveDics( aOpt.GetActiveDics() );
    for (const OUString& rActiveDic : aActiveDics)
    {
        if (!rActiveDic.isEmpty())
        {
            uno::Reference< XDictionary > xDic( getDictionaryByName( rActiveDic ) );
            if (xDic.is())
                xDic->setActive( true );
        }
    }

    // suppress collected events during creation of the dictionary list.
    // there should be no events during creation.
    mxDicEvtLstnrHelper->ClearEvents();

    mxDicEvtLstnrHelper->EndCollectEvents();

    bInCreation = false;
}


void DicList::SaveDics()
{
    // save dics only if they have already been used/created.
    //! don't create them just for the purpose of saving them !
    if ( aDicList.empty() )
        return;

    // save (modified) dictionaries
    DictionaryVec_t& rDicList = GetOrCreateDicList();
    size_t nCount = rDicList.size();
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


// Service specific part

OUString SAL_CALL DicList::getImplementationName(  )
{
    return "com.sun.star.lingu2.DicList";
}


sal_Bool SAL_CALL DicList::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL DicList::getSupportedServiceNames(  )
{
    return { "com.sun.star.linguistic2.DictionaryList" };
}



static sal_Int32 lcl_GetToken( OUString &rToken,
            const OUString &rText, sal_Int32 nPos, const OUString &rDelim )
{
    sal_Int32 nRes = -1;

    if (rText.isEmpty() ||  nPos >= rText.getLength())
        rToken.clear();
    else if (rDelim.isEmpty())
    {
        rToken = rText;
        if (!rToken.isEmpty())
            nRes = rText.getLength();
    }
    else
    {
        sal_Int32 i;
        for (i = nPos; i < rText.getLength(); ++i)
        {
            if (-1 != rDelim.indexOf( rText[i] ))
                break;
        }

        if (i >= rText.getLength())   // delimiter not found
            rToken  = rText.copy( nPos );
        else
            rToken  = rText.copy( nPos, i - nPos );
        nRes    = i + 1;    // continue after found delimiter
    }

    return nRes;
}


static void AddInternal(
        const uno::Reference<XDictionary> &rDic,
        const OUString& rNew )
{
    if (!rDic.is())
        return;

    //! TL TODO: word iterator should be used to break up the text
    OUString aDelim("!\"#$%&'()*+,-/:;<=>?[]\\_^`{|}~\t \n");
    OSL_ENSURE(aDelim.indexOf(u'.') == -1,
        "ensure no '.'");

    OUString      aToken;
    sal_Int32 nPos = 0;
    while (-1 != (nPos = lcl_GetToken( aToken, rNew, nPos, aDelim )))
    {
        if( !aToken.isEmpty()  &&  !IsNumeric( aToken ) )
        {
            rDic->add( aToken, false, OUString() );
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

static bool IsVers2OrNewer( const OUString& rFileURL, LanguageType& nLng, bool& bNeg, OUString& aDicName )
{
    if (rFileURL.isEmpty())
        return false;
    OUString aExt;
    sal_Int32 nPos = rFileURL.lastIndexOf( '.' );
    if (-1 != nPos)
        aExt = rFileURL.copy( nPos + 1 ).toAsciiLowerCase();

    if (aExt != "dic")
        return false;

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
        SAL_WARN( "linguistic", "failed to get input stream" );
    }
    DBG_ASSERT( xStream.is(), "failed to get stream for read" );
    if (!xStream.is())
        return false;

    std::unique_ptr<SvStream> pStream( utl::UcbStreamHelper::CreateStream( xStream ) );

    int nDicVersion = ReadDicVersion(*pStream, nLng, bNeg, aDicName);
    return 2 == nDicVersion || nDicVersion >= 5;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
linguistic_DicList_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(static_cast<cppu::OWeakObject*>(new DicList()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
