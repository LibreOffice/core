/*************************************************************************
 *
 *  $RCSfile: appopen.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mba $ $Date: 2000-09-28 11:38:01 $
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

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTATUSLISTENER_HPP_
#include <com/sun/star/frame/XStatusListener.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _SFXDOCTEMPL_HXX //autogen
#include <doctempl.hxx>
#endif
#ifndef _EXTATTR_HXX
#include <svtools/extattr.hxx>
#endif
#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif

#include <svtools/ehdl.hxx>
#include <svtools/sbxobj.hxx>
#include <svtools/urihelper.hxx>
#include <unotools/processfactory.hxx>

#pragma hdrstop

#include "app.hxx"
#include "appdata.hxx"
#include "bindings.hxx"
#include "cfgmgr.hxx"
#include "dispatch.hxx"
#include "docfile.hxx"
#include "docinf.hxx"
#include "fcontnr.hxx"
#include "fsetobsh.hxx"
#include "loadenv.hxx"
#include "new.hxx"
#include "objitem.hxx"
#include "objsh.hxx"
#include "objshimp.hxx"
#include "openflag.hxx"
#include "passwd.hxx"
#include "picklist.hxx"
#include "referers.hxx"
#include "request.hxx"
#include "sfxresid.hxx"
#include "viewsh.hxx"
#include "app.hrc"
#include "topfrm.hxx"
#include "appimp.hxx"
#include "iodlg.hxx"
#include "sfxuno.hxx"

#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>

using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::cppu;

//=========================================================================

class SfxOpenDocStatusListener_Impl : public WeakImplHelper1< XStatusListener >
{
public:
    BOOL    bFinished;
    BOOL    bSuccess;
    virtual void SAL_CALL   statusChanged( const FeatureStateEvent& Event ) throw(RuntimeException);
    virtual void SAL_CALL   disposing( const EventObject& Source ) throw(RuntimeException);
                            SfxOpenDocStatusListener_Impl()
                                : bFinished( FALSE )
                                , bSuccess( FALSE )
                            {}

};

void SAL_CALL SfxOpenDocStatusListener_Impl::statusChanged( const FeatureStateEvent& Event ) throw(RuntimeException)
{
    bSuccess = Event.IsEnabled;
    bFinished = TRUE;
}

void SAL_CALL SfxOpenDocStatusListener_Impl::disposing( const EventObject& Source ) throw(RuntimeException)
{
}

SfxObjectShellRef SfxApplication::DocAlreadyLoaded
(
    const String&   rName,      // Name des Dokuments mit Pfad
    BOOL            bSilent,    // TRUE: nicht nach neuer Sicht fragen
    BOOL            bActivate,   // soll bestehende Sicht aktiviert werden
    BOOL            bForbidVisible,
    const String*   pPostStr
)

/*  [Beschreibung]

    Stellt fest, ob ein Dokument mit dem Namen 'rName' bereits geladen
    ist und liefert einen Pointer darauf zu"uck.

    Ist das Dokument noch nicht geladen, wird ein 0-Pointer zur"uckgeliefert.
*/

{
    // zu suchenden Namen als URL aufbereiten
    INetURLObject aUrlToFind( rName, INET_PROT_FILE );
    String aPostString;
    if (  pPostStr )
        aPostString = *pPostStr;

    // noch offen?
    SfxObjectShellRef xDoc;

    if ( !aUrlToFind.HasError() )
    {
        // erst im Cache suchen
        MemCache_Impl &rCache = SfxPickList_Impl::Get()->GetMemCache();
        xDoc = rCache.Find( aUrlToFind, aPostString );

        // dann bei den normal geoeffneten Docs
        if ( !xDoc.Is() )
        {
            xDoc = SfxObjectShell::GetFirst( 0, FALSE ); // auch hidden Docs
            while( xDoc.Is() )
            {
                if ( xDoc->GetMedium() &&
                     xDoc->GetCreateMode() == SFX_CREATE_MODE_STANDARD &&
                     !xDoc->IsAbortingImport() && !xDoc->IsLoading() )
                {
                    // Vergleiche anhand der URLs
                    INetURLObject aUrl( xDoc->GetMedium()->GetName(), INET_PROT_FILE );
                    if ( !aUrl.HasError() && aUrl == aUrlToFind &&
                         (!bForbidVisible ||
                          !SfxViewFrame::GetFirst( xDoc, 0, TRUE )) &&
                         !xDoc->IsLoading())
                    {
                        SFX_ITEMSET_ARG(
                            xDoc->GetMedium()->GetItemSet(), pPostItem, SfxStringItem,
                            SID_POSTSTRING, FALSE );
                        SFX_ITEMSET_ARG(
                            xDoc->GetMedium()->GetItemSet(), pPost2Item,
                            SfxRefItem, SID_POSTLOCKBYTES, FALSE );
                        if( !pPost2Item && ( !pPostItem && (aPostString.Len()<1) ||
                            pPostItem && pPostItem->GetValue() == aPostString ))
                            break;
                    }
                }
                xDoc = SfxObjectShell::GetNext( *xDoc, 0, FALSE );
            }
        }
    }

    // gefunden?
    if ( xDoc.Is() && bActivate )
    {
        DBG_ASSERT(
            !bForbidVisible, "Unsichtbares kann nicht aktiviert werden" );

        SfxTopViewFrame *pFrame;
        for( pFrame = (SfxTopViewFrame*)
                 SfxViewFrame::GetFirst( xDoc, TYPE(SfxTopViewFrame) );
             pFrame && !pFrame->IsVisible();
             pFrame = (SfxTopViewFrame*)
                 SfxViewFrame::GetNext( *pFrame, xDoc, TYPE(SfxTopViewFrame) ) );
        if ( pFrame )
        {
            SfxViewFrame *pCur = SfxViewFrame::Current();
            if ( !bSilent && pFrame == pCur )
                InfoBox( 0, SfxResId(RID_DOCALREADYLOADED_DLG)).Execute();
            if ( bActivate )
            {
                pFrame->MakeActive_Impl( TRUE );
            }
        }
    }
    return xDoc;
}

//====================================================================

void SetTemplate_Impl( SvStorage *pStorage,
                       const String &rFileName,
                       const String &rLongName,
                       SfxObjectShell *pDoc)
{
    // DocInfo von pDoc 'plattmachen'
    SfxDocumentInfo &rInfo = pDoc->GetDocInfo();
    rInfo.Clear();

    // DocInfo vom Template laden
    SvStorageRef xTemplStor = new SvStorage( rFileName, STREAM_STD_READ );
    SfxDocumentInfo aTemplInfo;

    if ( aTemplInfo.Load( xTemplStor ) )
        rInfo.SetTemplateDate( aTemplInfo.GetChanged().GetTime() );

    // Template in DocInfo von pDoc eintragen
    INetURLObject aObj( rFileName, INET_PROT_FILE );
    if ( aObj.GetProtocol() == INET_PROT_FILE )
    {
        String aFoundName;
        if( SFX_APP()->Get_Impl()->GetDocumentTemplates()->GetFull( String(), rLongName, aFoundName ) )
        {
            rInfo.SetTemplateFileName( aObj.GetMainURL() );
            rInfo.SetTemplateName( rLongName );

            // wenn schon eine Config da ist, mu\s sie aus dem Template sein
            BOOL bHasConfig = (pDoc->GetConfigManager() != 0);
            rInfo.SetTemplateConfig( bHasConfig );
            pDoc->SetTemplateConfig( bHasConfig );
        }
    }

    // DocInfo in Stream schreiben
    pDoc->FlushDocInfo();
}

//--------------------------------------------------------------------

ULONG CheckPasswd_Impl
(
    Window *pWin,       // Parent des Dialogs
    SfxItemPool &rPool, // Pool, falls ein Set erzeugt werden mus
    SfxMedium *pFile    // das Medium, dessen Passwort gfs. erfragt werden soll
)

/*  [Beschreibung]

    Zu einem Medium das Passwort erfragen; funktioniert nur, wenn es sich
    um einen Storage handelt.
    Wenn in der Documentinfo das Passwort-Flag gesetzt ist, wird
    das Passwort vom Benutzer per Dialog erfragt und an dem Set
    des Mediums gesetzt; das Set wird, wenn nicht vorhanden, erzeugt.

*/

{
    ULONG nRet=0;
    if( ( !pFile->GetFilter() || pFile->GetFilter()->UsesStorage() ) && pFile->IsStorage() )
    {
        SvStorageRef aRef = pFile->GetStorage();
        if(aRef.Is())
        {
            SfxDocumentInfo *pInfo = new SfxDocumentInfo;
            if(pInfo->Load(aRef) && pInfo->IsPasswd())
            {
                SfxPasswordDialog *pDlg = new SfxPasswordDialog(pWin);
                String aTitle( pDlg->GetText() );
                aTitle += String::CreateFromAscii(" [");
                aTitle += INetURLObject( pFile->GetOrigURL() ).GetName();
                aTitle += String::CreateFromAscii("]");
                pDlg->SetText( aTitle );
                if(RET_OK == pDlg->Execute())
                {
                    SfxItemSet *pSet = pFile->GetItemSet();
                    if(!pSet)
                    {
                        pSet =
                            new SfxItemSet(rPool,
                                SID_PASSWORD, SID_PASSWORD, 0 );
                        pFile->SetItemSet(pSet);
                    }
                    pSet->Put(SfxStringItem(SID_PASSWORD, pDlg->GetPassword()));
                }
                else
                    nRet=ERRCODE_IO_ABORT;
                delete pDlg;
            }
            delete pInfo;
        }
    }
    return nRet;
}

//--------------------------------------------------------------------


SfxObjectShell* FindNoName_Impl( TypeId aDocType )

/*  [Beschreibung]

    Findet die erste unbenannte und unver"anderte SfxObjectShell vom
    angegeben Typ, f"ur das ein SfxMDIFrame exist
*/

{
    // suchen
    for ( SfxObjectShell *pFirst = SfxObjectShell::GetFirst();
          pFirst;
          pFirst = SfxObjectShell::GetNext(*pFirst) )
    {
        // passend?
        SfxTopViewFrame *pTopFrame = (SfxTopViewFrame*)
                SfxViewFrame::GetFirst(pFirst, TYPE(SfxTopViewFrame));
        if ( pTopFrame && !pFirst->HasName() && !pFirst->IsModified() &&
             pFirst->Type() == aDocType )
        {
            // => gefundenen zur"uckgeben
            return (pFirst->GetFlags() &
                    SFXOBJECTSHELL_DONTREPLACE ) ? 0 :  pFirst;
        }
    }

    // keins gefunden => 0 zur"uckgeben
    return 0;
}

ULONG SfxApplication::LoadTemplate( SfxObjectShellLock& xDoc, const String &rFileName,
    const String &rLongName, BOOL bCopy, SfxItemSet* pSet )
{
    BOOL bWithInfo = ( rLongName.Len() != 0 );
    const SfxFilter* pFilter = NULL;
    SfxMedium aMedium( rFileName,  ( STREAM_READ | STREAM_SHARE_DENYNONE ), FALSE );

    if ( aMedium.IsStorage() )
        aMedium.GetStorage();
    else
        aMedium.GetInStream();

    if ( aMedium.GetError() )
    {
        delete pSet;
        return aMedium.GetErrorCode();
    }

    ULONG nErr = GetFilterMatcher().GuessFilter( aMedium,&pFilter,SFX_FILTER_TEMPLATE, 0 );
    if ( 0 != nErr)
    {
        delete pSet;
        return ERRCODE_SFX_NOTATEMPLATE;
    }

    if( !pFilter || !pFilter->IsAllowedAsTemplate() )
    {
        delete pSet;
        return ERRCODE_SFX_NOTATEMPLATE;
    }

    if ( pFilter->GetFilterFlags() & SFX_FILTER_STARONEFILTER )
    {
        DBG_ASSERT( !xDoc.Is(), "Sorry, not implemented!" );
        delete pSet;
        SfxStringItem aName( SID_FILE_NAME, rFileName );
        SfxStringItem aReferer( SID_REFERER, String::CreateFromAscii("private:user") );
        SfxStringItem aFlags( SID_OPTIONS, String::CreateFromAscii("T") );
        SfxBoolItem aHidden( SID_VIEW, FALSE );
        const SfxPoolItem *pRet = GetDispatcher_Impl()->Execute( SID_OPENDOC, SFX_CALLMODE_SYNCHRON, &aName, &aHidden, &aReferer, &aFlags, 0L );
        const SfxObjectItem *pObj = PTR_CAST( SfxObjectItem, pRet );
        xDoc = PTR_CAST( SfxObjectShell, pObj->GetShell() );
        if ( !xDoc.Is() )
            return ERRCODE_SFX_DOLOADFAILED;
        xDoc->OwnerLock( FALSE );   // lock was set by hidden load
    }
    else
    {
        if ( !xDoc.Is() )
            xDoc = ((SfxFactoryFilterContainer*)pFilter->GetFilterContainer())->GetFactory().CreateObject();

        SfxMedium *pMedium = new SfxMedium( rFileName, STREAM_STD_READ, FALSE, TRUE, pFilter, pSet );
        if(!xDoc->DoLoad(pMedium))
        {
            ErrCode nErr = xDoc->GetErrorCode();
            xDoc->DoClose();
            xDoc.Clear();
            return nErr;
        }
    }

    if( bCopy )
    {
        SvStorageRef aTmpStor = new SvStorage(String());
        if( 0 != aTmpStor->GetError())
        {
            xDoc->DoClose();
            xDoc.Clear();
            return aTmpStor->GetErrorCode();
        }
        xDoc->GetStorage()->CopyTo( &aTmpStor );
        xDoc->DoHandsOff();
        if ( !xDoc->DoSaveCompleted( aTmpStor ) )
        {
            xDoc->DoClose();
            xDoc.Clear();
            return aTmpStor->GetErrorCode();
        }

        if ( bWithInfo )
            SetTemplate_Impl( aTmpStor, rFileName, rLongName, xDoc );
    }
    else if ( bWithInfo )
        SetTemplate_Impl( xDoc->GetStorage(), rFileName, rLongName, xDoc );

    if ( bWithInfo )
        xDoc->Broadcast( SfxDocumentInfoHint( &xDoc->GetDocInfo() ) );

    xDoc->SetNoName();
    xDoc->InvalidateName();
    xDoc->SetModified(FALSE);
    xDoc->ResetError();
    return xDoc->GetErrorCode();
}

//--------------------------------------------------------------------

void SfxApplication::LoadEa_Impl(SfxMedium &rMedium, SfxObjectShell& rObj)
{
    if ( !rMedium.IsStorage() )
        return;
    const SfxFilter *pFilter = rMedium.GetFilter();
    if ( !pFilter || !pFilter->IsOwnFormat() )
        return;
    SvStorage *pStor = rMedium.GetStorage();
    if ( !pStor )
        return;
    SvStream *pStream = pStor->GetTargetSvStream();
    if ( pStream && pStream->IsA() == ID_FILESTREAM )
    {
        SvEaMgr aEaMgr(*(SvFileStream *)pStream);
        String aBuffer;
        // Langnamen merken f"ur Titel und erneutes Setzen
        // beim Speichern
        if ( aEaMgr.GetLongName(aBuffer) )
            rMedium.SetLongName(aBuffer);
        if ( aEaMgr.GetComment(aBuffer) )
        {
            SfxDocumentInfo *pInfo = &rObj.GetDocInfo();
            // Kommentar aus der WPS mit DocInfo abgleichen
            pInfo->SetComment(aBuffer);
        }
    }
}

//--------------------------------------------------------------------

SfxMedium* SfxApplication::InsertDocumentDialog
(
    ULONG                   nFlags,
    const SfxObjectFactory& rFact
)
{
    SfxMedium *pMedium;
    while(1)
    {
        pMedium = 0;
        SvStringsDtor* pURLList = NULL;
        String aFilter;
        SfxItemSet* pSet;
        ErrCode nErr = FileOpenDialog_Impl( nFlags | SFXWB_INSERT | WB_3DLOOK, rFact, pURLList, aFilter, pSet );
        if( !nErr )
        {
            DBG_ASSERT( pURLList, "invalid URLList" );
            DBG_ASSERT( pURLList->Count() == 1, "invalid URLList count" );
            String aURL = *(pURLList->GetObject(0));
            pMedium = new SfxMedium(
                    aURL, SFX_STREAM_READONLY, FALSE, TRUE,
                    GetFilterMatcher().GetFilter( aFilter ), pSet );

            LoadEnvironment_ImplRef xLoader = new LoadEnvironment_Impl( pMedium );
            SfxFilterMatcher aMatcher( rFact.GetFilterContainer() );
            xLoader->SetFilterMatcher( &aMatcher );
            xLoader->Start();
            while( xLoader->GetState() != LoadEnvironment_Impl::DONE  )
                Application::Yield();
            pMedium = xLoader->GetMedium();

            if( pMedium )
                if( CheckPasswd_Impl( 0, SFX_APP()->GetPool(), pMedium ) ==
                    ERRCODE_ABORT )
                {
                    DELETEZ( pMedium );
                }
        }
        delete pURLList;
        break;
    }
    return pMedium;
}


//--------------------------------------------------------------------

void SfxApplication::NewFramesetExec_Impl( SfxRequest& rReq )
{
    DBG_MEMTEST();
/*
    // Factory-RegNo kann per Parameter angegeben sein
    SfxErrorContext aEc(ERRCTX_SFX_NEWDOCDIRECT);

    SfxFrameSetObjectShell *pDoc =
                new SfxFrameSetObjectShell( SFX_CREATE_MODE_STANDARD );
    SfxObjectShellRef xDoc(pDoc);
    pDoc->Initialize( String() );
    xDoc->SetActivateEvent_Impl( SFX_EVENT_CREATEDOC );

    SFX_REQUEST_ARG(rReq, pFrameItem, SfxFrameItem,
                    SID_DOCFRAME, FALSE);
    if ( pFrameItem && pFrameItem->GetFrame() )
        pFrameItem->GetFrame()->InsertDocument( pDoc );
    else
        CreateView_Impl( rReq, pDoc, TRUE );

    GetAppDispatcher_Impl()->Execute( SID_EDIT_FRAMESET );
 */
}

//--------------------------------------------------------------------
SfxObjectShellLock SfxApplication::NewDoc_Impl( const String& rFact, const SfxItemSet *pSet )
{
    SfxObjectShellLock xDoc;
    const SfxObjectFactory* pFactory = 0;
    String aFact( rFact );
    String aPrefix = String::CreateFromAscii( "private:factory/" );
    if ( aPrefix.Len() == aFact.Match( aPrefix ) )
        aFact.Erase( 0, aPrefix.Len() );
    USHORT nPos = aFact.Search( '?' );
    String aParam;
    if ( nPos != STRING_NOTFOUND )
    {
        aParam = aFact.Copy( nPos, aFact.Len() );
        aFact.Erase( nPos, aFact.Len() );
        aParam.Erase(0,1);
    }

    WildCard aSearchedFac( aFact.EraseAllChars('4').ToUpperAscii() );
    for( USHORT n = SfxObjectFactory::GetObjectFactoryCount_Impl(); !pFactory && n--; )
    {
        pFactory = &SfxObjectFactory::GetObjectFactory_Impl( n );
        if( !aSearchedFac.Matches( String::CreateFromAscii( pFactory->GetShortName() ).ToUpperAscii() ) )
            pFactory = 0;
    }

    if( !pFactory )
        pFactory = &SfxObjectFactory::GetDefaultFactory();

    // Objekt erzeugen
    USHORT nSlotId = pFactory->GetCreateNewSlotId();
    if ( pSet )
    {
        SFX_ITEMSET_ARG( pSet, pFrmItem, SfxFrameItem, SID_DOCFRAME, FALSE);
        if ( pFrmItem && pFrmItem->GetFrame() && !pFrmItem->GetFrame()->IsTop() )
            // In SubFrames ohne Dialog laden
            nSlotId = 0;
        SFX_ITEMSET_ARG( pSet, pSilentItem, SfxBoolItem, SID_SILENT, FALSE);
        if ( pSilentItem && pSilentItem->GetValue() )
            nSlotId = 0;
    }

    if ( nSlotId )
    {
        const SfxFrameItem* pFrmItem = NULL;
        if ( pSet )
            pFrmItem = (const SfxFrameItem*) SfxRequest::GetItem( pSet, SID_DOCFRAME, FALSE, TYPE(SfxFrameItem) );
        SfxBoolItem aItem( SID_NEWDOCDIRECT, TRUE );
        if ( pFrmItem && pFrmItem->GetFrame() && !pFrmItem->GetFrame()->GetCurrentDocument() )
            GetDispatcher_Impl()->Execute( nSlotId, SFX_CALLMODE_SYNCHRON, &aItem, pFrmItem, 0L );
        else
            GetDispatcher_Impl()->Execute( nSlotId, SFX_CALLMODE_ASYNCHRON, &aItem, pFrmItem, 0L );
    }
    else
    {
        xDoc = pFactory->CreateObject();
        aParam = INetURLObject::decode( aParam, '%', INetURLObject::DECODE_WITH_CHARSET );
        if( xDoc.Is() )
            xDoc->DoInitNew_Impl( aParam );
    }

    return xDoc;
}

void SfxApplication::NewDocDirectExec_Impl( SfxRequest& rReq )
{
    DBG_MEMTEST();

    SFX_REQUEST_ARG(rReq, pHidden, SfxBoolItem, SID_HIDDEN, FALSE);
//(mba)/task
/*
    if ( !pHidden || !pHidden->GetValue() )
        Application::GetAppWindow()->EnterWait();
*/
    SfxObjectShellLock xDoc;
    BOOL bNewView = TRUE;

    // Factory-RegNo kann per Parameter angegeben sein
    SfxErrorContext aEc(ERRCTX_SFX_NEWDOCDIRECT);
    const SfxItemSet *pArgs = rReq.GetArgs();
    const SfxObjectFactory* pFactory = 0;
    String aFactory;
    rReq.AppendItem( SfxBoolItem( SID_TEMPLATE, TRUE ) );
    SFX_REQUEST_ARG( rReq, pFactoryName, SfxStringItem, SID_NEWDOCDIRECT, FALSE );
    if( pFactoryName )
        aFactory = pFactoryName->GetValue();

    SFX_REQUEST_ARG( rReq, pFileFlagsItem, SfxStringItem, SID_OPTIONS, FALSE);
    if ( pFileFlagsItem )
    {
        // Werte auf einzelne Items verteilen
        String aFileFlags = pFileFlagsItem->GetValue();
        aFileFlags.ToUpperAscii();
        if ( STRING_NOTFOUND != aFileFlags.Search( 0x0054 ) )               // T = 54h
            rReq.AppendItem( SfxBoolItem( SID_TEMPLATE, TRUE ) );
        if ( STRING_NOTFOUND != aFileFlags.Search( 0x0048 ) )               // H = 48h
            rReq.AppendItem( SfxBoolItem( SID_HIDDEN, TRUE ) );
        if ( STRING_NOTFOUND != aFileFlags.Search( 0x0052 ) )               // R = 52h
            rReq.AppendItem( SfxBoolItem( SID_DOC_READONLY, TRUE ) );
        if ( STRING_NOTFOUND != aFileFlags.Search( 0x0042 ) )               // B = 42h
            rReq.AppendItem( SfxBoolItem( SID_PREVIEW, TRUE ) );
        if ( STRING_NOTFOUND != aFileFlags.Search( 0x0053 ) )               // S = 53h
            rReq.AppendItem( SfxBoolItem( SID_SILENT, TRUE ) );
    }

    xDoc = NewDoc_Impl( aFactory, rReq.GetArgs() );
    if ( xDoc.Is() )
    {
        SFX_REQUEST_ARG(rReq, pReadonly, SfxBoolItem, SID_DOC_READONLY, FALSE);
        if ( pReadonly )
            xDoc->GetMedium()->GetItemSet()->Put( *pReadonly );

        SFX_REQUEST_ARG(rReq, pPreview, SfxBoolItem, SID_PREVIEW, FALSE);
        if ( pPreview )
            xDoc->GetMedium()->GetItemSet()->Put( *pPreview );

        SFX_REQUEST_ARG(rReq, pFlags, SfxStringItem, SID_OPTIONS, FALSE);
        if ( pFlags )
            xDoc->GetMedium()->GetItemSet()->Put( *pFlags );

        SfxStringItem aPath( SID_TARGETPATH, String() );
        SFX_REQUEST_ARG(rReq, pReferer, SfxStringItem, SID_REFERER, FALSE);
        SFX_REQUEST_ARG(rReq, pPath, SfxStringItem, SID_TARGETPATH, FALSE);
        if ( pReferer )
        {
            // Wenn eine Factory-URL aus einem Bookmark stammt, wird kein Targetpath gesetzt;
            // stammt sie aus dem Neu - oder Startmenue, k"onnte der Request von einem Contextmenu
            // kommen. Daher wird der "ubergebene Targetpath "ubernommen.
            // Ist dieser allerdings selbst wieder Start - oder Neumen"u, wird das beim Speichern
            // "ubergangen ( ->objstor.cxx )
            xDoc->GetMedium()->GetItemSet()->Put( *pReferer, SID_REFERER );
            INetURLObject aURLObj;
            aURLObj.SetSmartURL( pReferer->GetValue() );

            BOOL bBookmark=TRUE;
            if( aURLObj.GetProtocol() == INET_PROT_FILE )
            {
                SfxIniManager* pMgr = GetIniManager();
                SfxIniKey aKey[] =
                {
                    SFX_KEY_NEW_DIR,
                    SFX_KEY_STARTMENU_DIR,
//                  SFX_KEY_TEMPLATE_PATH,
                    0
                };
                USHORT nPathLevel = aURLObj.getSegmentCount();
                USHORT nIndex = 0;
                while ( aKey[ nIndex ] )
                {
                    INetURLObject aNewPathObj( pMgr->Get( aKey[ nIndex ] ), INET_PROT_FILE );
                    USHORT nNewLevel = aNewPathObj.getSegmentCount();
                    int nOffset = nPathLevel;
                    nOffset -= nNewLevel;
                    if ( nOffset >= 0 )
                    {
                        INetURLObject aTempObj = aURLObj;
                        for ( ; nOffset > 0; nOffset-- )
                            aTempObj.removeSegment();
                        if ( aTempObj == aNewPathObj )
                        {
                            bBookmark = FALSE;
                            break;
                        }
                    }
                    ++nIndex;
                }
            }

            if ( !bBookmark && pPath )
                aPath.SetValue( pPath->GetValue() );
        }

        xDoc->GetMedium()->GetItemSet()->Put( aPath, SID_TARGETPATH );
    }

    // View erzeugen
    SfxViewFrame* pViewFrame = 0;
    if ( xDoc.Is() )
    {
        SFX_REQUEST_ARG(rReq, pHidden, SfxBoolItem, SID_HIDDEN, FALSE);
        if ( pHidden )
            xDoc->GetMedium()->GetItemSet()->Put( *pHidden, SID_HIDDEN );

        SFX_REQUEST_ARG(rReq, pViewId, SfxUInt16Item, SID_VIEW_ID, FALSE);
        if ( pViewId )
            xDoc->GetMedium()->GetItemSet()->Put( *pViewId, SID_VIEW_ID );

        xDoc->SetActivateEvent_Impl( SFX_EVENT_CREATEDOC );
//      xDoc->Get_Impl()->nLoadedFlags = SFX_LOADED_ALL;
        const SfxItemSet* pInternalArgs = rReq.GetInternalArgs_Impl();
        if( pInternalArgs )
            xDoc->GetMedium()->GetItemSet()->Put( *pInternalArgs );
        BOOL bOwnsFrame = FALSE;
        SfxFrame* pFrame = GetTargetFrame_Impl( rReq.GetArgs(), bOwnsFrame );

        if ( pFrame->PrepareClose_Impl( TRUE, TRUE ) == TRUE )
        {
            if ( pHidden && pHidden->GetValue() )
            {
                xDoc->RestoreNoDelete();
                xDoc->OwnerLock( TRUE );
            }
            pFrame->InsertDocument( xDoc );
            pViewFrame = pFrame->GetCurrentViewFrame();
        }
        else if ( pFrame->PrepareClose_Impl( TRUE, TRUE ) == RET_NEWTASK )
        {
            pFrame = SfxTopFrame::Create();
            bOwnsFrame = TRUE;
            DBG_ERROR( "NYI!" );
            xDoc.Clear();
        }
        else
            xDoc.Clear();
/*
        SFX_REQUEST_ARG(rReq, pTargetItem, SfxStringItem,
                        SID_TARGETNAME, FALSE);
        String aTargetName;
        if( pTargetItem ) pTargetItem->GetValue();
        if ( aTargetName.Len() &&
             aTargetName.CompareIgnoreCaseToAscii("_blank") != COMPARE_EQUAL &&
             aTargetName.CompareIgnoreCaseToAscii("_top") != COMPARE_EQUAL )
            pFrame->SetFrameName( aTargetName );
*/
        rReq.SetReturnValue( SfxFrameItem( 0, pFrame ) );
    }

//(mba)/task
/*
    if ( !pHidden || !pHidden->GetValue() )
        Application::GetAppWindow()->LeaveWait();
 */
}

//--------------------------------------------------------------------

void SfxApplication::NewDocExec_Impl( SfxRequest& rReq )
{
    DBG_MEMTEST();

    // keine Parameter vom BASIC nur Factory angegeben?
    SFX_REQUEST_ARG(rReq, pTemplNameItem, SfxStringItem, SID_TEMPLATE_NAME, FALSE);
    SFX_REQUEST_ARG(rReq, pTemplFileNameItem, SfxStringItem, SID_FILE_NAME, FALSE);
    SFX_REQUEST_ARG(rReq, pTemplRegionNameItem, SfxStringItem, SID_TEMPLATE_REGIONNAME, FALSE);

    SfxObjectShellLock xDoc;
    BOOL bNewView = TRUE;

    String  aTemplateRegion, aTemplateName, aTemplateFileName;
    BOOL    bDirect = FALSE; // "uber FileName anstelle Region/Template
    SfxErrorContext aEc(ERRCTX_SFX_NEWDOC);
    if ( !pTemplNameItem && !pTemplFileNameItem )
    {
        SfxNewFileDialog *pDlg = CreateNewDialog();
        if ( RET_OK == pDlg->Execute() )
        {
            if ( pDlg->IsTemplate() )
            {
                aTemplateName = pDlg->GetTemplateName();
                aTemplateRegion = pDlg->GetTemplateRegion();
                aTemplateFileName = pDlg->GetTemplateFileName();
                rReq.AppendItem( SfxStringItem( SID_TEMPLATE_NAME, aTemplateName) );
                rReq.AppendItem( SfxStringItem( SID_TEMPLATE_REGIONNAME, aTemplateRegion) );
                rReq.AppendItem( SfxStringItem( SID_FILE_NAME, aTemplateFileName) );
                delete pDlg;
            }
            else
            {
                delete pDlg;
                bDirect = TRUE;
                return;
            }
        }
        else
        {
            delete pDlg;
            return;
        }
    }
    else
    {
        // Template-Name
        if ( pTemplNameItem )
            aTemplateName = pTemplNameItem->GetValue();

        // Template-Region
        if ( pTemplRegionNameItem )
            aTemplateRegion = pTemplRegionNameItem->GetValue();

        // Template-File-Name
        if ( pTemplFileNameItem )
        {
            aTemplateFileName = pTemplFileNameItem->GetValue();
            bDirect = TRUE;
        }
    }

    INetURLObject aObj( aTemplateName, INET_PROT_FILE );
    SfxErrorContext aEC( ERRCTX_SFX_LOADTEMPLATE, aObj.PathToFileName() );
//! (pb) MaxLen?         DirEntry(aTemplateName).GetFull( FSYS_STYLE_HOST,FALSE,20));

    ULONG lErr = 0;
    SfxItemSet* pSet = new SfxAllItemSet( GetPool() );
    pSet->Put( SfxBoolItem( SID_TEMPLATE, TRUE ) );
    if ( !bDirect )
    {
        SfxDocumentTemplates aTmpFac;
        aTmpFac.Construct();
        if( !aTemplateFileName.Len() )
            aTmpFac.GetFull( aTemplateRegion, aTemplateName, aTemplateFileName );

        if( !aTemplateFileName.Len() )
            lErr = ERRCODE_SFX_TEMPLATENOTFOUND;
    }

    if ( lErr != ERRCODE_NONE )
    {
        ULONG lFatalErr = ERRCODE_TOERROR(lErr);
        if ( lFatalErr )
            ErrorHandler::HandleError(lErr);
    }
    else
    {
        const SfxPoolItem *pRet=0;
        SfxCallMode eMode = rReq.IsSynchronCall() ? SFX_CALLMODE_SYNCHRON : SFX_CALLMODE_ASYNCHRON;
        SfxStringItem aReferer( SID_REFERER, DEFINE_CONST_UNICODE("private:user") );
        SfxStringItem aTarget( SID_TARGETNAME, DEFINE_CONST_UNICODE("_blank") );
        if ( aTemplateFileName.Len() )
        {
            INetURLObject aObj( aTemplateFileName, INET_PROT_FILE );
            SfxStringItem aName( SID_FILE_NAME, aObj.GetMainURL() );
            SfxStringItem aTemplName( SID_TEMPLATE_NAME, aTemplateName );
            SfxStringItem aTemplRegionName( SID_TEMPLATE_REGIONNAME, aTemplateRegion );
            pRet = GetDispatcher_Impl()->Execute( SID_OPENDOC, eMode, &aName, &aTarget, &aReferer, &aTemplName, &aTemplRegionName, 0L );
        }
        else
        {
            SfxStringItem aName( SID_FILE_NAME, DEFINE_CONST_UNICODE("private:factory") );
            pRet = GetDispatcher_Impl()->Execute( SID_OPENDOC, eMode, &aName, &aTarget, &aReferer, 0L );
        }

        if ( pRet )
            rReq.SetReturnValue( *pRet );
    }
}


//-------------------------------------------------------------------------

SfxFrame* SfxApplication::GetTargetFrame_Impl( const SfxItemSet* pSet, BOOL& rbOwner  )
{
    SFX_ITEMSET_ARG(pSet, pViewItem, SfxBoolItem, SID_VIEW, FALSE);

    if ( pViewItem && !pViewItem->GetValue() )
        // ohne View laden
        return NULL;

    SFX_ITEMSET_ARG(pSet, pFrmItem, SfxFrameItem, SID_DOCFRAME, FALSE);
    SFX_ITEMSET_ARG(pSet, pTargetItem, SfxStringItem, SID_TARGETNAME, FALSE);
    SFX_ITEMSET_ARG(pSet, pBoolItem, SfxBoolItem, SID_ONLYSUBFRAMES, FALSE);
    SFX_ITEMSET_ARG(pSet, pRefererItem, SfxStringItem, SID_REFERER, FALSE);
    SFX_ITEMSET_ARG(pSet, pBrowsingItem, SfxUInt16Item, SID_BROWSEMODE, FALSE);
    SFX_ITEMSET_ARG(pSet, pPreviewItem, SfxBoolItem, SID_PREVIEW, FALSE);

    // Frame als Parameter mitgegeben?
    SfxFrame *pFrame = NULL;
    if ( pFrmItem )
        pFrame = pFrmItem->GetFrame();
    SfxFrame *pSource = pFrame;
    SfxFrame *pStart = pFrame;

    if ( pPreviewItem && pPreviewItem->GetValue() )
    {
        DBG_ASSERT( pFrame, "Preview without frame!" );
        return pFrame;
    }

    String aTargetName;
    if ( pRefererItem && ( !pBrowsingItem || pBrowsingItem->GetValue() == BROWSE_NORMAL ) )
    {
        INetURLObject aURLObj;
        aURLObj.SetSmartURL( pRefererItem->GetValue() );
        if ( aURLObj.GetProtocol() == INET_PROT_FILE && ( !pFrame || pFrame->IsTop() ) )
        {
            SfxIniManager* pIniMgr = GetIniManager();
            INetURLObject aStartMenuObj( pIniMgr->Get( SFX_KEY_STARTMENU_DIR ), INET_PROT_FILE );
            INetURLObject aQuickStartObj( pIniMgr->Get( SFX_KEY_QUICKSTART_DIR ), INET_PROT_FILE );
            if ( aURLObj == aStartMenuObj || aURLObj == aQuickStartObj )
            {
                aTargetName = String::CreateFromAscii( "_blank" );
                pTargetItem = NULL;
            }
        }
    }

    if ( pTargetItem && pTargetItem->GetValue().Len() )
    {
        // Wenn ein TargetItem mitgegeben wurde, aber kein Frame, den
        // Current ViewFrame nehmen
        SfxViewFrame* pCurViewFrame = SfxViewFrame::Current();
        if ( !pCurViewFrame )
            pCurViewFrame = SfxViewFrame::GetFirst();

        if ( !pFrame && pCurViewFrame )
        {
            pFrame = pCurViewFrame->GetFrame();
            pStart = pFrame;
        }

        if ( pFrame )
        {
            aTargetName = pTargetItem->GetValue();

            // Wenn kein TargetName, dann den vom Current Document
            SfxObjectShell* pCur = SfxObjectShell::Current();
            if( !aTargetName.Len() && pCur )
                aTargetName = pCur->GetDocInfo().GetDefaultTarget();
        }
    }
    else if ( pFrame && (pFrame->GetFrameName().CompareToAscii("mail-body") == COMPARE_EQUAL) )
    {
        // Hack for MailDocument in Office 5.2
        aTargetName = String::CreateFromAscii("_blank");
    }

    BOOL bNewTask =
            aTargetName.CompareToAscii("_blank")    == COMPARE_EQUAL ||
            aTargetName.CompareToAscii("_null")     == COMPARE_EQUAL;

    if ( pFrame )
    {
        if ( pBoolItem && pBoolItem->GetValue() )
        {
            DBG_ASSERT( aTargetName.Len(), "OnlySubFrames, aber kein Name!" );
            pFrame = pFrame->SearchChildrenForName_Impl( aTargetName );
        }
        else
        {
            if ( !bNewTask || pFrame->GetFrameName().Len() || pFrame->GetCurrentDocument() || pFrame->GetParentFrame() )
                // Auch bei leerem TargetName suchen wg. SmartSelf
                // _blank nur suchen, wenn pFrame nicht schon _blank ist!
                pFrame = pFrame->SearchFrame( aTargetName );
        }
    }

    BOOL bName = ( pFrame == NULL );
    if ( pFrame && ( !pBrowsingItem || pBrowsingItem->GetValue() == BROWSE_NORMAL ) )
    {
        // Es wurde ein Frame gefunden; dessen Doc darf nicht ersetzt werden,
        // wenn es der Desktop ist oder wenn es embedded ist
        // Ausnahme: neuerdings k"onnen wir auch innerhalb des Desktops browsen, wenn dieser ein Frameset ist
        SfxObjectShell* pCur = pFrame->GetCurrentDocument();
        if( pCur &&
            ( ( pCur->GetFlags() & SFXOBJECTSHELL_DONTREPLACE ) ||
              pCur->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED ) )
        {
            pFrame = 0;
            pSource = 0;
            pStart = 0;
        }
    }

    // Kein Frame gefunden -> Neuen Frame nehmen
    if ( !pFrame )
    {
        SFX_ITEMSET_ARG( pSet, pHiddenItem, SfxBoolItem, SID_HIDDEN, FALSE);
        BOOL bHidden =  pHiddenItem && pHiddenItem->GetValue();
        pFrame = SfxTopFrame::Create( NULL, 0, bHidden );
        rbOwner = TRUE;
        if ( !bNewTask && bName )
            pFrame->SetFrameName( aTargetName );
    }
    else
        rbOwner = FALSE;

    return pFrame;
}

//---------------------------------------------------------------------------

void SfxApplication::OpenDocExec_Impl( SfxRequest& rReq )
{
    DBG_MEMTEST();

    USHORT nSID = rReq.GetSlot();
    SFX_REQUEST_ARG( rReq, pFileNameItem, SfxStringItem, SID_FILE_NAME, FALSE );
    if ( !pFileNameItem )
    {
        // FileDialog ausf"uhren
        SvStringsDtor* pURLList = NULL;
        String aFilter;
        void* pDummy = 0; // wegen GCC und C272
        SfxItemSet* pSet;
        ULONG nErr = FileOpenDialog_Impl(
                WB_OPEN | SFXWB_MULTISELECTION | SFXWB_SHOWVERSIONS, *(SfxObjectFactory*)pDummy, pURLList, aFilter, pSet );
        if ( nErr == ERRCODE_ABORT )
        {
            delete pURLList;
            return;
        }

        rReq.SetArgs( *(SfxAllItemSet*)pSet );
        rReq.AppendItem( SfxStringItem( SID_FILTER_NAME, aFilter ) );
        rReq.AppendItem( SfxStringItem( SID_TARGETNAME, String::CreateFromAscii("_blank") ) );
        delete pSet;

        if ( pURLList->Count() > 1 )
        {
            rReq.AppendItem( SfxStringItem( SID_REFERER, String::CreateFromAscii(SFX_REFERER_USER) ) );

            for ( USHORT i = 0; i < pURLList->Count(); ++i )
            {
                String aURL = *(pURLList->GetObject(i));
                rReq.RemoveItem( SID_FILE_NAME );
                rReq.AppendItem( SfxStringItem( SID_FILE_NAME, aURL ) );

                // synchron ausf"uhren, damit beim Reschedulen nicht schon das n"achste Dokument
                // geladen wird
                GetDispatcher_Impl()->Execute( SID_OPENDOC, SFX_CALLMODE_SYNCHRON, *rReq.GetArgs() );
            }
            delete pURLList;
            return;
        }
        else
        {
            String aURL;
            if ( pURLList->Count() == 1 )
                aURL = *(pURLList->GetObject(0));
            rReq.AppendItem( SfxStringItem( SID_FILE_NAME, aURL ) );
            delete pURLList;
        }

        rReq.AppendItem( SfxStringItem( SID_REFERER, String::CreateFromAscii(SFX_REFERER_USER) ) );
    }

    if ( SID_OPENURL == nSID )
    {
        // SID_OPENURL does the same as SID_OPENDOC!
        rReq.SetSlot( SID_OPENDOC );
        nSID = SID_OPENDOC;
    }
    else if ( nSID == SID_OPENTEMPLATE )
    {
        rReq.AppendItem( SfxBoolItem( SID_TEMPLATE, TRUE ) );
    }

    SFX_REQUEST_ARG(rReq, pFrmItem, SfxFrameItem, SID_DOCFRAME, FALSE);
    SfxFrame *pFrame = NULL;
    if ( pFrmItem )
        pFrame = pFrmItem->GetFrame();
    else if ( SfxViewFrame::Current() )
        pFrame = SfxViewFrame::Current()->GetFrame();

    // check if caller has set a callback
    SFX_REQUEST_ARG(rReq, pLinkItem, SfxLinkItem, SID_DONELINK, FALSE );

    // check if caller wants to create a view
    BOOL bCreateView = TRUE;
    SFX_REQUEST_ARG( rReq, pCreateViewItem, SfxBoolItem, SID_VIEW, FALSE );
    if ( pCreateViewItem && !pCreateViewItem->GetValue() )
        bCreateView = FALSE;

    // we can't load without a view - switch to hidden view
    if ( !bCreateView )
        rReq.AppendItem( SfxBoolItem( SID_HIDDEN, TRUE ) );

    // check if the view must be hidden
    BOOL bHidden = FALSE;
    SFX_REQUEST_ARG(rReq, pHidItem, SfxBoolItem, SID_HIDDEN, FALSE);
    if ( pHidItem )
        bHidden = pHidItem->GetValue();

    // convert items to properties for framework API calls
    Sequence < PropertyValue > aArgs;
    TransformItems( SID_OPENDOC, *rReq.GetArgs(), aArgs );

    Reference < XController > xController;
    if ( pFrame || pLinkItem || !rReq.IsSynchronCall() )
    {
        // extract target name
        ::rtl::OUString aTarget;
        SFX_REQUEST_ARG(rReq, pTargetItem, SfxStringItem, SID_TARGETNAME, FALSE);
        if ( pTargetItem )
            aTarget = pTargetItem->GetValue();
        else
        {
            SFX_REQUEST_ARG( rReq, pNewViewItem, SfxBoolItem, SID_OPEN_NEW_VIEW, FALSE );
            if ( pNewViewItem && pNewViewItem->GetValue() )
                aTarget = String::CreateFromAscii("_blank" );
        }

        if ( bHidden )
            aTarget = String::CreateFromAscii("_blank");

        // if a frame is given, it must be used for the starting point of the targetting mechanism
        // this code is also used if asynchron loading is possible, because loadComponent always is synchron
        Reference < XFrame > xFrame;
        if ( pFrame )
            xFrame = pFrame->GetFrameInterface();
        else
            xFrame = Reference < XFrame >( ::utl::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.frame.Desktop")), UNO_QUERY );

        // make URL ready
        URL aURL;
        SFX_REQUEST_ARG( rReq, pFileNameItem, SfxStringItem, SID_FILE_NAME, FALSE );
        String aFileName = pFileNameItem->GetValue();

        INetURLObject aObj;
        SfxObjectShell* pCur = pFrame ? pFrame->GetCurrentDocument() : 0;
        if ( !pCur )
            pCur = SfxObjectShell::Current();
        if( aFileName.Len() && aFileName.GetChar(0) == '#' ) // Mark without URL
        {
            SfxViewFrame *pView = pFrame ? pFrame->GetCurrentViewFrame() : 0;
            if ( !pView )
                pView = SfxViewFrame::Current();
            pView->GetViewShell()->JumpToMark( aFileName.Copy(1) );
            rReq.SetReturnValue( SfxViewFrameItem( 0, pView ) );
            return;
        }

        aURL.Complete = aFileName;
        Reference < XURLTransformer > xTrans( ::utl::getProcessServiceFactory()->createInstance( rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer" )), UNO_QUERY );
        xTrans->parseStrict( aURL );

        // load document using dispatch framework
        if( !pLinkItem && rReq.IsSynchronCall() )
        {
            // if loading must be done synchron, we must wait for completion to get a return value
            // find frame by myself; I must konw the exact frame to get the controller for the return value from it
            if( aTarget.getLength() )
                xFrame = xFrame->findFrame( aTarget, FrameSearchFlag::ALL );

            // targeting has been resolved, so target name must not be used in queryDispatch
            Reference < XDispatchProvider > xProv( xFrame, UNO_QUERY );
            Reference < XDispatch > xDisp = xProv.is() ? xProv->queryDispatch( aURL, ::rtl::OUString(), 0 ) : Reference < XDispatch >();
            if ( xDisp.is() )
            {
                // create listener for notification of load success or fail
                SfxOpenDocStatusListener_Impl* pListener = new SfxOpenDocStatusListener_Impl();
                pListener->acquire();
                xDisp->addStatusListener( pListener, aURL );
                xDisp->dispatch( aURL, aArgs );

                // stay on the stack until result has been notified
                while ( !pListener->bFinished )
                    Application::Yield();

                if ( pListener->bSuccess )
                    // successful loading, get loaded controller
                    xController = xFrame->getController();

                xDisp->removeStatusListener( pListener, aURL );
                pListener->release();
            }

            if ( !xController.is() && aTarget.compareToAscii( "_blank" ) == COMPARE_EQUAL )
                // a blank frame would have been created in findFrame; in this case I am the owner and I must delete it
                xFrame->dispose();
        }
        else
        {
            // otherwise we just dispatch and that's it
            Reference < XDispatchProvider > xProv( xFrame, UNO_QUERY );
            Reference < XDispatch > xDisp = xProv.is() ? xProv->queryDispatch( aURL, aTarget, FrameSearchFlag::ALL ) : Reference < XDispatch >();;
            if ( xDisp.is() )
                xDisp->dispatch( aURL, aArgs );
        }
    }
    else
    {
        // synchron loading without a given frame
        SFX_REQUEST_ARG( rReq, pFileNameItem, SfxStringItem, SID_FILE_NAME, FALSE );
        Reference < XComponentLoader > xDesktop( ::utl::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.frame.Desktop")), UNO_QUERY );
        Reference < XModel > xModel( xDesktop->loadComponentFromURL( pFileNameItem->GetValue(), ::rtl::OUString::createFromAscii("_blank"), 0, aArgs ), UNO_QUERY );
        if ( xModel.is() )
            xController = xModel->getCurrentController();
    }

    if ( xController.is() )
    {
        // try to find the SfxFrame for the controller
        SfxFrame* pFrame = NULL;
        for ( SfxViewShell* pShell = SfxViewShell::GetFirst( 0, FALSE ); pShell; pShell = SfxViewShell::GetNext( *pShell, 0, FALSE ) )
        {
            if ( pShell->GetController() == xController )
            {
                pFrame = pShell->GetViewFrame()->GetFrame();
                break;
            }
        }

        if ( pFrame )
        {
            SfxObjectShell* pSh = pFrame->GetCurrentDocument();
            DBG_ASSERT( pSh, "Controller without ObjectShell ?!" );

            if ( bCreateView )
                rReq.SetReturnValue( SfxViewFrameItem( 0, pFrame->GetCurrentViewFrame() ) );
            else
                rReq.SetReturnValue( SfxObjectItem( 0, pSh ) );

            SFX_REQUEST_ARG(rReq, pExecItem, SfxExecuteItem, SID_AFTEROPENEVENT, FALSE );
            if( pExecItem )
                pFrame->GetDispatcher_Impl()->Execute( *pExecItem );

            if( ( bHidden || !bCreateView ) )
            {
                pSh->RestoreNoDelete();
                // Locking is now done in LoadEnvironment_Impl, otherwise it would be too late!
//                pSh->OwnerLock( TRUE );
            }
        }
    }
}

//--------------------------------------------------------------------

SfxViewFrame *SfxApplication::CreateView_Impl
(
    SfxRequest&         rReq,
    SfxObjectShell*     pDoc,
    FASTBOOL            bNewView,   // neue View erzwingen
    FASTBOOL            bHidden
)
{
    SfxViewFrame* pFrame = CreateView_Impl( rReq.GetArgs(), pDoc, bNewView, bHidden );
    rReq.SetReturnValue( SfxViewFrameItem( pFrame ) );
    return pFrame;
}

SfxViewFrame *SfxApplication::CreateView_Impl
(
    const SfxItemSet*   pSet,
    SfxObjectShell*     pDoc,
    FASTBOOL            bNewView,   // neue View erzwingen
    FASTBOOL            bHidden
)
{
    DBG_MEMTEST();

    SfxFrame* pFrame = SfxTopFrame::Create( pDoc, 0, bHidden, pSet );
    return pFrame->GetCurrentViewFrame();
}

//--------------------------------------------------------------------

void SfxApplication::CreateDocState_Impl(SfxItemSet &rSet)
{
    DBG_MEMTEST();
    const USHORT *pRanges = rSet.GetRanges();
    DBG_ASSERT(pRanges, "Set ohne Bereich");
    while(*pRanges)
    {
        for(USHORT nWhich = *pRanges++; nWhich <= *pRanges; ++nWhich)
        {
            switch(nWhich)
            {
                case SID_CURRENT_URL:
                {
                    SfxViewFrame *pFrame = pViewFrame;
                    if ( pFrame )
                    {
                        // Bei internem InPlace den ContainerFrame nehmen
                        if ( pFrame->GetParentViewFrame_Impl() )
                            pFrame = pFrame->GetParentViewFrame_Impl();

                        // URL des aktiven Frames anzeigen; wenn es ein Frame
                        // in einem als Frameset implementierten Dokument ist, dann
                        // die URL des Framesets anzeigen ( Explorer, Mail ).
                        if ( pFrame->GetParentViewFrame() )
                        {
                            if ( pFrame->GetParentViewFrame()->GetViewShell()->
                                IsImplementedAsFrameset_Impl() )
                            pFrame = pFrame->GetParentViewFrame();
                        }

                        rSet.Put( SfxStringItem( nWhich, pFrame->GetActualPresentationURL_Impl() ) );
                    }
                    else
                        rSet.Put( SfxStringItem( nWhich, String() ) );
                    break;
                }
            }
        }
        ++pRanges;
    }
}


