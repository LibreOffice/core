/*************************************************************************
 *
 *  $RCSfile: dlistimp.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-11-17 12:37:34 $
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

#include "dlistimp.hxx"
#include "dicimp.hxx"

#ifndef _FSYS_HXX
#include <tools/fsys.hxx>
#endif
#ifndef _TOOLS_INTN_HXX
#include <tools/intn.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_USEROPTIONS_HXX
#include <svtools/useroptions.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#include <cppuhelper/factory.hxx>   // helper for factories

#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/linguistic2/DictionaryEventFlags.hpp>
#include <com/sun/star/linguistic2/DictionaryListEventFlags.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

//using namespace utl;
using namespace osl;
using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;

///////////////////////////////////////////////////////////////////////////

SV_IMPL_OBJARR(ActDicArray, ActDic);

///////////////////////////////////////////////////////////////////////////

#define BUFSIZE              256
#define VERS2_NOLANGUAGE    1024

static sal_Char         aBuf[ BUFSIZE ];
static const sal_Char*  aDicExt     = "dic";
static const sal_Char*  aVerStr2    = "WBSWG2";
static const sal_Char*  aVerStr5    = "WBSWG5";


// forward dedclarations

static BOOL IsVers2( const DirEntry& rPathName, USHORT& nLng, BOOL& bNeg,
                     sal_Char* pWordBuf);

static void AddInternal( Reference< XDictionary > &rDic,
                         const OUString& rNew );
static void AddUserData( const Reference< XDictionary > &rDic );

///////////////////////////////////////////////////////////////////////////

class DicEvtListenerHelper :
    public cppu::WeakImplHelper1
    <
        XDictionaryEventListener
    >
{
    cppu::OInterfaceContainerHelper     aDicListEvtListeners;
    Sequence< DictionaryEvent >         aCollectDicEvt;
    Reference< XDictionaryList >        xMyDicList;

    INT16                               nCondensedEvt;
    INT16                               nNumCollectEvtListeners,
                                         nNumVerboseListeners;

public:
    DicEvtListenerHelper( const Reference< XDictionaryList > &rxDicList );
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

    BOOL    AddDicListEvtListener(
                const Reference< XDictionaryListEventListener >& rxListener,
                BOOL bReceiveVerbose );
    BOOL    RemoveDicListEvtListener(
                const Reference< XDictionaryListEventListener >& rxListener );
    INT16   BeginCollectEvents();
    INT16   EndCollectEvents();
    INT16   FlushEvents();
};


DicEvtListenerHelper::DicEvtListenerHelper(
        const Reference< XDictionaryList > &rxDicList ) :
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
    MutexGuard  aGuard( GetLinguMutex() );

    Reference< XInterface > xSrc( rSource.Source );

    // remove event object from EventListener list
    if (xSrc.is())
        aDicListEvtListeners.removeInterface( xSrc );

    // if object is a dictionary then remove it from the dictionary list
    // Note: this will probably happen only if someone makes a XDictionary
    // implementation of his own that is also a XComponent.
    Reference< XDictionary > xDic( xSrc, UNO_QUERY );
    if (xDic.is())
    {
        xMyDicList->removeDictionary( xDic );
    }
}


void SAL_CALL DicEvtListenerHelper::processDictionaryEvent(
            const DictionaryEvent& rDicEvent )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    Reference< XDictionary > xDic( rDicEvent.Source, UNO_QUERY );
    DBG_ASSERT(xDic.is(), "lng : missing event source");

    // assert that there is a corresponding dictionary entry if one was
    // added or deleted
    Reference< XDictionaryEntry > xDicEntry( rDicEvent.xDictionaryEntry, UNO_QUERY );
    DBG_ASSERT( !(rDicEvent.nEvent &
                    (DictionaryEventFlags::ADD_ENTRY | DictionaryEventFlags::DEL_ENTRY))
                || xDicEntry.is(),
                "lng : missing dictionary entry" );

    //
    // evaluate DictionaryEvents and update data for next DictionaryListEvent
    //
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
        INT32 nColEvts = aCollectDicEvt.getLength();
        aCollectDicEvt.realloc( nColEvts + 1 );
        aCollectDicEvt.getArray()[ nColEvts ] = rDicEvent;
    }

    if (nNumCollectEvtListeners == 0 && nCondensedEvt != 0)
        FlushEvents();
}


BOOL DicEvtListenerHelper::AddDicListEvtListener(
            const Reference< XDictionaryListEventListener >& xListener,
            BOOL bReceiveVerbose )
{
    DBG_ASSERT( xListener.is(), "empty reference" );
    INT32   nCount = aDicListEvtListeners.getLength();
    return aDicListEvtListeners.addInterface( xListener ) != nCount;
}


BOOL DicEvtListenerHelper::RemoveDicListEvtListener(
            const Reference< XDictionaryListEventListener >& xListener )
{
    DBG_ASSERT( xListener.is(), "empty reference" );
    INT32   nCount = aDicListEvtListeners.getLength();
    return aDicListEvtListeners.removeInterface( xListener ) != nCount;
}


INT16 DicEvtListenerHelper::BeginCollectEvents()
{
    return ++nNumCollectEvtListeners;
}


INT16 DicEvtListenerHelper::EndCollectEvents()
{
    DBG_ASSERT(nNumCollectEvtListeners > 0, "lng: mismatched function call");
    if (nNumCollectEvtListeners > 0)
    {
        FlushEvents();
        nNumCollectEvtListeners--;
    }

    return nNumCollectEvtListeners;
}


INT16 DicEvtListenerHelper::FlushEvents()
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
        Reference< XDictionaryListEventListener > xRef( aIt.next(), UNO_QUERY );
        if (xRef.is())
            xRef->processDictionaryListEvent( aEvent );
    }

    // clear "list" of events
    nCondensedEvt = 0;
    aCollectDicEvt.realloc( 0 );

    return nNumCollectEvtListeners;
}


///////////////////////////////////////////////////////////////////////////


void DicList::MyAppExitListener::AtExit()
{
    // save (modified) dictionaries
    Sequence< Reference< XDictionary > > aDics( rMyDicList.getDictionaries() );
    Reference< XDictionary >    *pDic = aDics.getArray();
    INT32 nCount = aDics.getLength();
    for (INT32 i = 0;  i < nCount;  i++)
    {
        // save (modified) dictionaries
        Reference< frame::XStorable >  xStor( pDic[i] , UNO_QUERY );
        if (xStor.is())
        {
            try
            {
                if (!xStor->isReadonly() && xStor->hasLocation())
                    xStor->store();
            }
            catch(...)
            {
            }
        }
    }
}


DicList::DicList() :
    aEvtListeners   ( GetLinguMutex() )
{
    pDicEvtLstnrHelper  = new DicEvtListenerHelper( this );
    xDicEvtLstnrHelper  = pDicEvtLstnrHelper;
    bDisposing = FALSE;

    // look for dictionaries
    SvtPathOptions aPathOpt;
    searchForDictionaries( aDicList, aPathOpt.GetUserDictionaryPath() );
    searchForDictionaries( aDicList, aPathOpt.GetDictionaryPath() );

    // create IgnoreAllList dictionary with empty URL (non persistent)
    // and add it to list
    OUString aDicName( A2OU( "IgnoreAllList" ) );
    Reference< XDictionary > xIgnAll(
            createDictionary( aDicName, CreateLocale( LANGUAGE_NONE ),
                              DictionaryType_POSITIVE, OUString() ) );
    if (xIgnAll.is())
    {
        AddUserData( xIgnAll );
        xIgnAll->setActive( TRUE );
        addDictionary( xIgnAll );
    }

    pExitListener = new MyAppExitListener( *this );
    xExitListener = pExitListener;
    pExitListener->Activate();

    // evaluate list of dictionaries to be activated from configuration
    const Sequence< OUString > aActiveDics( aOpt.GetCfgActiveDictionaries() );
    const OUString *pActiveDic = aActiveDics.getConstArray();
    INT32 nLen = aActiveDics.getLength();
    for (INT32 i = 0;  i < nLen;  ++i)
    {
        if (pActiveDic[i].getLength())
        {
            Reference< XDictionary > xDic( getDictionaryByName( pActiveDic[i] ) );
            if (xDic.is())
                xDic->setActive( TRUE );
        }
    }
}

DicList::~DicList()
{
    pExitListener->Deactivate();
}


void DicList::searchForDictionaries( ActDicArray &rDicList,
                                     const String &rDicDir )
{
    MutexGuard  aGuard( GetLinguMutex() );

    // get DirEntry for dictionaries directory
    DirEntry aDicDir( rDicDir );
    aDicDir.ToAbs();

    // Alle auf Platte stehenden Dictionary-Names einlesen
    String aDirTmp( String::CreateFromAscii( "*.*" ) );
    aDicDir += DirEntry( aDirTmp );
    Dir aDir(aDicDir, FSYS_KIND_FILE, FSYS_SORT_END);

    String aDCN( String::CreateFromAscii( "dcn" ) );
    String aDCP( String::CreateFromAscii( "dcp" ) );
    INetURLObject   aURLObject;
    aURLObject.SetSmartProtocol( INET_PROT_FILE );
    for (USHORT i=0; i < aDir.Count(); ++i)
    {
        String  aName(aDir[i].GetName());
        USHORT  nLang = LANGUAGE_NONE;
        BOOL    bNeg  = FALSE;

        if(!::IsVers2(aDir[i], nLang, bNeg, aBuf))
        {
            // Wenn kein
            xub_StrLen nPos  = aName.Search('.');
            String aExt(aName.Copy(nPos + 1));
            aExt.ToLowerAscii();

            if(aExt == aDCN)       // negativ
                bNeg = TRUE;
            else if(aExt == aDCP)  // positiv
                bNeg = FALSE;
            else
                continue;          // andere Files
        }

        // Aufnehmen in die Liste der Dictionaries
        // Wenn existent nicht aufnehmen
        //
        INT16 nSystemLanguage = ::GetSystemLanguage();
        String aTmp1 = ToLower( aName, nSystemLanguage );
        String aTmp2;
        INT32 j;
        INT32  nCount = rDicList.Count();
        for(j = 0;  j < nCount;  j++)
        {
            aTmp2 = rDicList.GetObject( j ).xDic->getName().getStr();
            aTmp2 = ToLower( aTmp2, nSystemLanguage );
            if(aTmp1 == aTmp2)
                break;
        }
        if(j >= nCount)     // dictionary not yet in DicList
        {
            String aFileName (rDicDir);
            aFileName += DirEntry::GetAccessDelimiter();
            aFileName += aName;
            aURLObject.SetSmartURL( aFileName );
            DBG_ASSERT(!aURLObject.HasError(), "lng : invalid URL");

            DictionaryType eType = bNeg ? DictionaryType_NEGATIVE : DictionaryType_POSITIVE;
            Reference< XDictionary > xDic =
                    new DictionaryNeo( aName, nLang, eType,
                                       aURLObject.GetMainURL() );

            addDictionary( xDic );
            nCount++;
        }
    }
}


INT32 DicList::getDicPos(const Reference< XDictionary > &xDic)
{
    MutexGuard  aGuard( GetLinguMutex() );

    INT32 nPos = -1;
    INT32 n = aDicList.Count();
    for (INT32 i = 0;  i < n;  i++)
    {
        if (aDicList.GetObject(i).xDic == xDic)
            return i;
    }
    return nPos;
}


Reference< XInterface > SAL_CALL
    DicList_CreateInstance( const Reference< XMultiServiceFactory > & rSMgr )
            throw(Exception)
{
    Reference< XInterface > xService = (cppu::OWeakObject *) new DicList;
    return xService;
}

sal_Int16 SAL_CALL DicList::getCount() throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return aDicList.Count();
}

uno::Sequence< Reference< XDictionary > > SAL_CALL
        DicList::getDictionaries()
            throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    uno::Sequence< Reference< XDictionary > > aDics( aDicList.Count() );
    Reference< XDictionary > *pDic = aDics.getArray();

    INT32 n = aDics.getLength();
    for (INT32 i = 0;  i < n;  i++)
        pDic[i] = aDicList.GetObject(i).xDic;

    return aDics;
}

Reference< XDictionary > SAL_CALL
        DicList::getDictionaryByName( const OUString& aDictionaryName )
            throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    Reference< XDictionary > xDic;
    INT32 nCount = aDicList.Count();
    for (INT32 i = 0;  i < nCount;  i++)
    {
        const Reference< XDictionary > &rDic = aDicList.GetObject(i).xDic;
        if (rDic.is()  &&  rDic->getName() == aDictionaryName)
        {
            xDic = rDic;
            break;
        }
    }

    return xDic;
}

sal_Bool SAL_CALL DicList::addDictionary(
            const Reference< XDictionary >& xDictionary )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (bDisposing)
        return FALSE;

    BOOL bRes = FALSE;
    if (xDictionary.is())
    {
        aDicList.Insert( ActDic(xDictionary), aDicList.Count() );
        bRes = TRUE;

        // add listener helper to the dictionaries listener lists
        xDictionary->addDictionaryEventListener( xDicEvtLstnrHelper );
    }
    return bRes;
}

sal_Bool SAL_CALL
    DicList::removeDictionary( const Reference< XDictionary >& xDictionary )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (bDisposing)
        return FALSE;

    BOOL  bRes = FALSE;
    INT32 nPos = getDicPos( xDictionary );
    if (nPos >= 0)
    {
        // remove dictionary list from the dictionaries listener lists
        Reference< XDictionary > xDic( aDicList.GetObject( nPos ).xDic );
        DBG_ASSERT(xDic.is(), "lng : empty reference");
        if (xDic.is())
        {
            // deactivate dictionary if not already done
            xDic->setActive( FALSE );

            xDic->removeDictionaryEventListener( xDicEvtLstnrHelper );
        }

        aDicList.Remove(nPos);
        bRes = TRUE;
    }
    return bRes;
}

sal_Bool SAL_CALL DicList::addDictionaryListEventListener(
            const Reference< XDictionaryListEventListener >& xListener,
            sal_Bool bReceiveVerbose )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (bDisposing)
        return FALSE;

    DBG_ASSERT(!bReceiveVerbose, "lng : not yet supported");

    BOOL bRes = FALSE;
    if (xListener.is()) //! don't add empty references
    {
        bRes = pDicEvtLstnrHelper->
                        AddDicListEvtListener( xListener, bReceiveVerbose );
    }
    return bRes;
}

sal_Bool SAL_CALL DicList::removeDictionaryListEventListener(
            const Reference< XDictionaryListEventListener >& xListener )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (bDisposing)
        return FALSE;

    BOOL bRes = FALSE;
    if(xListener.is())
    {
        bRes = pDicEvtLstnrHelper->RemoveDicListEvtListener( xListener );
    }
    return bRes;
}

sal_Int16 SAL_CALL DicList::beginCollectEvents() throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return pDicEvtLstnrHelper->BeginCollectEvents();
}

sal_Int16 SAL_CALL DicList::endCollectEvents() throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return pDicEvtLstnrHelper->EndCollectEvents();
}

sal_Int16 SAL_CALL DicList::flushEvents() throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return pDicEvtLstnrHelper->FlushEvents();
}

Reference< XDictionary > SAL_CALL
    DicList::createDictionary( const OUString& rName, const Locale& rLocale,
            DictionaryType eDicType, const OUString& rURL )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    INT16 nLanguage = LocaleToLanguage( rLocale );
    return new DictionaryNeo( rName, nLanguage, eDicType, rURL );
}


Reference< XDictionaryEntry > SAL_CALL
    DicList::queryDictionaryEntry( const OUString& rWord, const Locale& rLocale,
            sal_Bool bSearchPosDics, sal_Bool bSearchSpellEntry )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return SearchDicList( this, rWord, LocaleToLanguage( rLocale ),
                            bSearchPosDics, bSearchSpellEntry );
}


void SAL_CALL
    DicList::dispose()
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing)
    {
        bDisposing = TRUE;
        EventObject aEvtObj( (XDictionaryList *) this );

        aEvtListeners.disposeAndClear( aEvtObj );
        if (pDicEvtLstnrHelper)
            pDicEvtLstnrHelper->DisposeAndClear( aEvtObj );

        INT16 nCount = aDicList.Count();
        for (INT16 i = 0;  i < nCount;  i++)
        {
            Reference< XDictionary > xDic( aDicList.GetObject(i).xDic , UNO_QUERY );

            // save (modified) dictionaries
            Reference< frame::XStorable >  xStor( xDic , UNO_QUERY );
            if (xStor.is())
            {
                try
                {
                    if (!xStor->isReadonly() && xStor->hasLocation())
                        xStor->store();
                }
                catch(...)
                {
                }
            }

            // release references to (members of) this object hold by
            // dictionaries
            if (xDic.is())
                xDic->removeDictionaryEventListener( xDicEvtLstnrHelper );
        }

        // set list of active dictionaries in configuration
        aOpt.SetCfgActiveDictionaries( Reference< XDictionaryList >( this ));
    }
}

void SAL_CALL
    DicList::addEventListener( const Reference< XEventListener >& rxListener )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
        aEvtListeners.addInterface( rxListener );
}

void SAL_CALL
    DicList::removeEventListener( const Reference< XEventListener >& rxListener )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
        aEvtListeners.removeInterface( rxListener );
}


///////////////////////////////////////////////////////////////////////////
// Service specific part
//

OUString SAL_CALL DicList::getImplementationName(  ) throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return getImplementationName_Static();
}


sal_Bool SAL_CALL DicList::supportsService( const OUString& ServiceName )
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


uno::Sequence< OUString > SAL_CALL DicList::getSupportedServiceNames(  )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    return getSupportedServiceNames_Static();
}


uno::Sequence< OUString > DicList::getSupportedServiceNames_Static() throw()
{
    MutexGuard  aGuard( GetLinguMutex() );

    uno::Sequence< OUString > aSNS( 1 );    // auch mehr als 1 Service moeglich
    aSNS.getArray()[0] = A2OU( SN_DICTIONARY_LIST );
    return aSNS;
}


sal_Bool SAL_CALL DicList_writeInfo(
    void * /*pServiceManager*/, registry::XRegistryKey * pRegistryKey )
{
    try
    {
        String aImpl( '/' );
        aImpl += DicList::getImplementationName_Static().getStr();
        aImpl.AppendAscii( "/UNO/SERVICES" );
        Reference< registry::XRegistryKey > xNewKey =
                pRegistryKey->createKey(aImpl );
        uno::Sequence< OUString > aServices =
                DicList::getSupportedServiceNames_Static();
        for( INT32 i = 0; i < aServices.getLength(); i++ )
            xNewKey->createKey( aServices.getConstArray()[i]);

        return sal_True;
    }
    catch(Exception &)
    {
        return sal_False;
    }
}


void * SAL_CALL DicList_getFactory( const sal_Char * pImplName,
        XMultiServiceFactory * pServiceManager, void *  )
{
    void * pRet = 0;
    if ( !DicList::getImplementationName_Static().compareToAscii( pImplName ) )
    {
        Reference< XSingleServiceFactory > xFactory =
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

///////////////////////////////////////////////////////////////////////////

xub_StrLen lcl_GetToken( String &rToken,
            const String &rText, xub_StrLen nPos, const String &rDelim )
{
    xub_StrLen nRes = STRING_LEN;

    if (rText.Len() == 0  ||  nPos >= rText.Len())
        rToken = String();
    else if (rDelim.Len() == 0)
    {
        rToken = rText;
        if (rToken.Len())
            nRes = rText.Len();
    }
    else
    {
        xub_StrLen  i;
        for (i = nPos;  i < rText.Len();  ++i)
        {
            if (STRING_NOTFOUND != rDelim.Search( rText.GetChar(i) ))
                break;
        }

        if (i >= rText.Len())   // delimeter not found
            rToken  = rText.Copy( nPos );
        else
            rToken  = rText.Copy( nPos, (INT32) i - nPos );
        nRes    = i + 1;    // continue after found delimeter
    }

    return nRes;
}


static void AddInternal(
        const Reference<XDictionary> &rDic,
        const OUString& rNew )
{
    if (rDic.is())
    {
        //! TL TODO: word iterator should be used to break up the text
        static const char *pDefWordDelim =
                "!\"#$%&'()*+,-./:;<=>?[]\\_^`{|}~\t \n";
        ByteString aDummy( pDefWordDelim );
        String aDelim( aDummy , RTL_TEXTENCODING_MS_1252 );
        aDelim.EraseAllChars( '.' );

        String      aToken;
        xub_StrLen  nPos = 0;
        while (STRING_LEN !=
                    (nPos = lcl_GetToken( aToken, rNew, nPos, aDelim )))
        {
            if( aToken.Len()  &&  !IsNumeric( aToken ) )
            {
                rDic->add( aToken, FALSE, OUString() );
            }
        }
    }
}

static void AddUserData( const Reference< XDictionary > &rDic )
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

///////////////////////////////////////////////////////////////////////////

#pragma optimize("ge",off)

static BOOL IsVers2( const DirEntry& rPathName, USHORT& nLng, BOOL& bNeg,
                     sal_Char* pWordBuf)
{
    String aDIC( String::CreateFromAscii( aDicExt ) );
    String aExt(rPathName.GetExtension());
    aExt.ToLowerAscii();

    if(aExt != aDIC)
        return FALSE;

    // get binary files URL
    INetURLObject aURLObj;
    aURLObj.SetSmartProtocol( INET_PROT_FILE );
    aURLObj.SetSmartURL( rPathName.GetFull() );
    String aFileURL( aURLObj.GetMainURL() );

    // get stream to be used
    SfxMedium aMedium( aFileURL,
            STREAM_READ | STREAM_SHARE_DENYWRITE, FALSE );
    aMedium.SetTransferPriority( SFX_TFPRIO_SYNCHRON );
    SvStream *pStream = aMedium.GetInStream();
    if (!pStream || pStream->GetError())
        return FALSE;

    // Header einlesen
    USHORT      nLen;
    sal_Char    nTmp;

    *pStream >> nLen;
    if( nLen >= BUFSIZE )
        return FALSE;
    pStream->Read( pWordBuf, nLen);
    *( pWordBuf + nLen ) = 0;
    // Version 2.0 oder Version 5.0 ?
    if( !strcmp( pWordBuf, aVerStr2 ) || !strcmp( pWordBuf, aVerStr5 ) )
    {
        // Sprache des Dictionaries
        *pStream >> nLng;

        if ( VERS2_NOLANGUAGE == nLng )
            nLng = LANGUAGE_NONE;

        // Negativ-Flag
        *pStream >> nTmp;
        bNeg = (BOOL)nTmp;

        return TRUE;
    }
    return FALSE;
}

///////////////////////////////////////////////////////////////////////////

