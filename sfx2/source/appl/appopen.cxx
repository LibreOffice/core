/*************************************************************************
 *
 *  $RCSfile: appopen.cxx,v $
 *
 *  $Revision: 1.74 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 14:30:43 $
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
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XNOTIFYINGDISPATCH_HPP_
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_DISPATCHRESULTSTATE_HPP_
#include <com/sun/star/frame/DispatchResultState.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHRESULTLISTENER_HPP_
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_SYSTEM_XSYSTEMSHELLEXECUTE_HPP_
#include <com/sun/star/system/XSystemShellExecute.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XTYPEDETECTION_HPP_
#include <com/sun/star/document/XTypeDetection.hpp>
#endif
#ifndef _COM_SUN_STAR_SYSTEM_SYSTEMSHELLEXECUTEFLAGS_HPP_
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_MACROEXECMODE_HPP_
#include <com/sun/star/document/MacroExecMode.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_UPDATEDOCMODE_HPP_
#include <com/sun/star/document/UpdateDocMode.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONREQUEST_HPP_
#include <com/sun/star/task/XInteractionRequest.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
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
#include <unotools/localfilehelper.hxx>

#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif
#ifndef _SVTOOLS_TEMPLDLG_HXX
#include <svtools/templdlg.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#include <svtools/extendedsecurityoptions.hxx>
#include <svtools/docpasswdrequest.hxx>

#pragma hdrstop

#include "app.hxx"
#include "appdata.hxx"
#include "bindings.hxx"
#include "cfgmgr.hxx"
#include "dispatch.hxx"
#include "docfile.hxx"
#include "docinf.hxx"
#include "fcontnr.hxx"
#include "loadenv.hxx"
#include "new.hxx"
#include "objitem.hxx"
#include "objsh.hxx"
#include "objshimp.hxx"
#include "openflag.hxx"
#include "passwd.hxx"
#include "referers.hxx"
#include "request.hxx"
#include "sfxresid.hxx"
#include "viewsh.hxx"
#include "app.hrc"
#include "topfrm.hxx"
#include "appimp.hxx"
#include "sfxuno.hxx"
#include "objface.hxx"
#include "filedlghelper.hxx"
#include "docfac.hxx"

#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::system;
using namespace ::com::sun::star::task;
using namespace ::cppu;
using namespace ::sfx2;

//=========================================================================

class SfxOpenDocStatusListener_Impl : public WeakImplHelper1< XDispatchResultListener >
{
public:
    BOOL    bFinished;
    BOOL    bSuccess;
    virtual void SAL_CALL   dispatchFinished( const DispatchResultEvent& Event ) throw(RuntimeException);
    virtual void SAL_CALL   disposing( const EventObject& Source ) throw(RuntimeException);
                            SfxOpenDocStatusListener_Impl()
                                : bFinished( FALSE )
                                , bSuccess( FALSE )
                            {}
};

void SAL_CALL SfxOpenDocStatusListener_Impl::dispatchFinished( const DispatchResultEvent& aEvent ) throw(RuntimeException)
{
    bSuccess = ( aEvent.State == DispatchResultState::SUCCESS );
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
    INetURLObject aUrlToFind( rName );
    DBG_ASSERT( aUrlToFind.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL" );
    String aPostString;
    if (  pPostStr )
        aPostString = *pPostStr;

    // noch offen?
    SfxObjectShellRef xDoc;

    if ( !aUrlToFind.HasError() )
    {
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
                    INetURLObject aUrl( xDoc->GetMedium()->GetName() );
                    if ( !aUrl.HasError() && aUrl == aUrlToFind &&
                         (!bForbidVisible || !SfxViewFrame::GetFirst( xDoc, 0, TRUE )) &&
                         !xDoc->IsLoading())
                    {
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
    INetURLObject aObj( rFileName );
    DBG_ASSERT( aObj.GetProtocol() != INET_PROT_NOT_VALID, "Invalid URL" );

    if( ::utl::LocalFileHelper::IsLocalFile( rFileName ) )
    {
        String aFoundName;
        if( SFX_APP()->Get_Impl()->GetDocumentTemplates()->GetFull( String(), rLongName, aFoundName ) )
        {
            rInfo.SetTemplateFileName( aObj.GetMainURL(INetURLObject::DECODE_TO_IURI) );
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
    //Window *pWin,       // Parent des Dialogs
    SfxObjectShell* pDoc,
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
    if( ( !pFile->GetFilter() || pFile->GetFilter()->UsesStorage() ) )
    {
        SvStorageRef aRef = pFile->GetStorage();
        if(aRef.Is())
        {
            sal_Bool bIsEncrypted = sal_False;
            ::com::sun::star::uno::Any aAny;
            if ( aRef->GetProperty( ::rtl::OUString::createFromAscii("HasEncryptedEntries"), aAny ) )
                aAny >>= bIsEncrypted;
            else
            {
                SfxDocumentInfo aInfo;
                bIsEncrypted = ( aInfo.Load(aRef) && aInfo.IsPasswd() );
            }

            if ( bIsEncrypted )
            {
                Window* pWin = pDoc ? pDoc->GetDialogParent( pFile ) : NULL;
                if ( pWin )
                    pWin->Show();

                nRet = ERRCODE_SFX_CANTGETPASSWD;

                SfxItemSet *pSet = pFile->GetItemSet();
                if( pSet )
                {
                    Reference< ::com::sun::star::task::XInteractionHandler > xInteractionHandler;

                    SFX_ITEMSET_ARG( pSet, pxInteractionItem, SfxUnoAnyItem, SID_INTERACTIONHANDLER, sal_False );
                    if( pxInteractionItem && ( pxInteractionItem->GetValue() >>= xInteractionHandler )
                     && xInteractionHandler.is() )
                    {
                        RequestDocumentPassword* pPasswordRequest = new RequestDocumentPassword(
                            ::com::sun::star::task::PasswordRequestMode_PASSWORD_ENTER,
                            INetURLObject( pFile->GetOrigURL() ).GetName( INetURLObject::DECODE_WITH_CHARSET ) );

                        Reference< XInteractionRequest > rRequest( pPasswordRequest );
                        xInteractionHandler->handle( rRequest );

                        if ( pPasswordRequest->isPassword() )
                        {
                            pSet->Put( SfxStringItem( SID_PASSWORD, pPasswordRequest->getPassword() ) );
                            nRet = ERRCODE_NONE;
                        }
                        else
                            nRet = ERRCODE_IO_ABORT;
                    }
                }
            }
        }
    }
    return nRet;
}

//--------------------------------------------------------------------


ULONG SfxApplication::LoadTemplate( SfxObjectShellLock& xDoc, const String &rFileName, BOOL bCopy, SfxItemSet* pSet )
{
    const SfxFilter* pFilter = NULL;
    SfxMedium aMedium( rFileName,  ( STREAM_READ | STREAM_SHARE_DENYNONE ), FALSE );

    if ( !aMedium.GetStorage() )
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
        SfxBoolItem aHidden( SID_HIDDEN, TRUE );
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
            xDoc = SfxObjectShell::CreateObject( pFilter->GetServiceName() );

        SfxMedium *pMedium = new SfxMedium( rFileName, STREAM_STD_READ, FALSE, pFilter, pSet );
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
        SvStorageRef aTmpStor = new SvStorage( (xDoc->GetStorage()->GetVersion() >= SOFFICE_FILEFORMAT_60) ,String() );
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

        SetTemplate_Impl( aTmpStor, rFileName, String(), xDoc );
    }
    else
        SetTemplate_Impl( xDoc->GetStorage(), rFileName, String(), xDoc );

    xDoc->Broadcast( SfxDocumentInfoHint( &xDoc->GetDocInfo() ) );
    xDoc->SetNoName();
    xDoc->InvalidateName();
    xDoc->SetModified(FALSE);
    xDoc->ResetError();

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >  xModel ( xDoc->GetModel(), ::com::sun::star::uno::UNO_QUERY );
    if ( xModel.is() )
    {
        SfxItemSet* pNew = xDoc->GetMedium()->GetItemSet()->Clone();
        pNew->ClearItem( SID_PROGRESS_STATUSBAR_CONTROL );
        pNew->Put( SfxStringItem( SID_FILTER_NAME, xDoc->GetFactory().GetFilter(0)->GetFilterName() ) );
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs;
        TransformItems( SID_OPENDOC, *pNew, aArgs );
        sal_Int32 nLength = aArgs.getLength();
        aArgs.realloc( nLength + 1 );
        aArgs[nLength].Name = DEFINE_CONST_UNICODE("Title");
        aArgs[nLength].Value <<= ::rtl::OUString( xDoc->GetTitle( SFX_TITLE_DETECT ) );
        xModel->attachResource( ::rtl::OUString(), aArgs );
        delete pNew;
    }

    return xDoc->GetErrorCode();
}

//--------------------------------------------------------------------

void SfxApplication::LoadEa_Impl(SfxMedium &rMedium, SfxObjectShell& rObj)
{
    if ( !rMedium.GetStorage() )
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
    const String& rFact
)
{
    return InsertDocumentDialog( nFlags, rFact, 0 );
}

//--------------------------------------------------------------------

SfxMedium* SfxApplication::InsertDocumentDialog
(
    ULONG                   nFlags,
    const String& rFact,
    ULONG                   nHelpId
)
{
    SfxMedium *pMedium=0;
    SvStringsDtor* pURLList = NULL;
    String aFilter;
    SfxItemSet* pSet=0;
    ErrCode nErr = sfx2::FileOpenDialog_Impl( nFlags | SFXWB_INSERT | WB_3DLOOK, rFact, pURLList, aFilter, pSet, String(), nHelpId );
    if( pURLList && !nErr )
    {
        DBG_ASSERT( pURLList->Count() == 1, "invalid URLList count" );
        String aURL = *(pURLList->GetObject(0));
        pMedium = new SfxMedium(
                aURL, SFX_STREAM_READONLY, FALSE,
                GetFilterMatcher().GetFilter4FilterName( aFilter ), pSet );

        pMedium->UseInteractionHandler(TRUE);
        LoadEnvironment_ImplRef xLoader = new LoadEnvironment_Impl( pMedium );
        SfxFilterMatcher aMatcher( rFact );
        xLoader->SetFilterMatcher( &aMatcher );
        xLoader->Start();
        while( xLoader->GetState() != LoadEnvironment_Impl::DONE  )
            Application::Yield();
        pMedium = xLoader->GetMedium();
        if( pMedium && CheckPasswd_Impl( 0, SFX_APP()->GetPool(), pMedium ) == ERRCODE_ABORT )
            pMedium = NULL;
    }

    delete pURLList;
    return pMedium;
}

//--------------------------------------------------------------------

SfxMediumList* SfxApplication::InsertDocumentsDialog
(
    ULONG                   nFlags,
    const String& rFact,
    ULONG                   nHelpId
)
{
    SfxMediumList *pMediumList=new SfxMediumList;
    SvStringsDtor* pURLList = NULL;
    String aFilter;
    SfxItemSet* pSet=0;
    ErrCode nErr = sfx2::FileOpenDialog_Impl( nFlags | SFXWB_INSERT | SFXWB_MULTISELECTION | WB_3DLOOK, rFact, pURLList, aFilter, pSet, String(), nHelpId );
    DBG_ASSERT( pURLList, "invalid URLList" );
    if( pURLList && !nErr )
    {
        for ( USHORT n=0; n<pURLList->Count(); n++ )
        {
            String aURL = *(pURLList->GetObject(n));
            SfxMedium* pMedium = new SfxMedium(
                    aURL, SFX_STREAM_READONLY, FALSE,
                    GetFilterMatcher().GetFilter4FilterName( aFilter ), pSet );

            pMedium->UseInteractionHandler(TRUE);
            LoadEnvironment_ImplRef xLoader = new LoadEnvironment_Impl( pMedium );
            SfxFilterMatcher aMatcher( rFact );
            xLoader->SetFilterMatcher( &aMatcher );
            xLoader->Start();
            while( xLoader->GetState() != LoadEnvironment_Impl::DONE  )
                Application::Yield();
            pMedium = xLoader->GetMedium();
            if( pMedium && CheckPasswd_Impl( 0, GetPool(), pMedium ) != ERRCODE_ABORT )
            {
                pMediumList->Insert( pMedium );
            }
            else
                delete pMedium;
        }
    }

    delete pURLList;
    return pMediumList;
}

//--------------------------------------------------------------------

SfxMediumList* SfxApplication::InsertDocumentsDialog
(
    ULONG                   nFlags,
    const String& rFact
)
{
    return InsertDocumentsDialog( nFlags, rFact, 0 );
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

    xDoc = SfxObjectShell::CreateObjectByFactoryName( aFact );
    aParam = INetURLObject::decode( aParam, '%', INetURLObject::DECODE_WITH_CHARSET );
    if( xDoc.Is() )
        xDoc->DoInitNew_Impl( aParam );

    if ( xDoc.Is() )
    {
        if ( pSet )
        {
            SFX_ITEMSET_ARG( pSet, pTitleItem, SfxStringItem, SID_DOCINFO_TITLE, FALSE );
            if ( pTitleItem )
                xDoc->GetMedium()->GetItemSet()->Put( *pTitleItem );
        }

        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >  xModel ( xDoc->GetModel(), ::com::sun::star::uno::UNO_QUERY );
        if ( xModel.is() )
        {
            SfxItemSet* pNew = xDoc->GetMedium()->GetItemSet()->Clone();
            pNew->ClearItem( SID_PROGRESS_STATUSBAR_CONTROL );
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs;
            TransformItems( SID_OPENDOC, *pNew, aArgs );

            sal_Int32 nLength = aArgs.getLength();
            aArgs.realloc( nLength + 1 );

            aArgs[nLength].Name = DEFINE_CONST_UNICODE("Title");
            aArgs[nLength].Value <<= ::rtl::OUString( xDoc->GetTitle( SFX_TITLE_DETECT ) );

            xModel->attachResource( ::rtl::OUString(), aArgs );
            delete pNew;
        }
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
    else
        aFactory = SvtModuleOptions().GetDefaultModuleName();

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

        SFX_REQUEST_ARG(rReq, pSilent, SfxBoolItem, SID_SILENT, FALSE);
        if ( pSilent )
            xDoc->GetMedium()->GetItemSet()->Put( *pSilent );

        SFX_REQUEST_ARG(rReq, pFlags, SfxStringItem, SID_OPTIONS, FALSE);
        if ( pFlags )
            xDoc->GetMedium()->GetItemSet()->Put( *pFlags );
    }

    // View erzeugen
    SfxViewFrame* pViewFrame = 0;
    if ( xDoc.Is() )
    {
        SFX_REQUEST_ARG(rReq, pHidden, SfxBoolItem, SID_HIDDEN, FALSE);
        BOOL bHidden = FALSE;
        if ( pHidden )
        {
            xDoc->GetMedium()->GetItemSet()->Put( *pHidden, SID_HIDDEN );
            bHidden = pHidden->GetValue();
        }

        SFX_REQUEST_ARG(rReq, pViewId, SfxUInt16Item, SID_VIEW_ID, FALSE);
        USHORT nViewId = 0;
        if ( pViewId )
        {
            xDoc->GetMedium()->GetItemSet()->Put( *pViewId, SID_VIEW_ID );
            nViewId = pViewId->GetValue();
        }

        xDoc->SetActivateEvent_Impl( SFX_EVENT_CREATEDOC );
//      xDoc->Get_Impl()->nLoadedFlags = SFX_LOADED_ALL;
        const SfxItemSet* pInternalArgs = rReq.GetInternalArgs_Impl();
        if( pInternalArgs )
            xDoc->GetMedium()->GetItemSet()->Put( *pInternalArgs );
        BOOL bOwnsFrame = FALSE;
        SFX_REQUEST_ARG(rReq, pFrameItem, SfxFrameItem, SID_DOCFRAME, FALSE);

        SfxFrame* pFrame = NULL;
        if (pFrameItem)
            pFrame = pFrameItem->GetFrame();
        else
            pFrame = (SfxFrame*)SfxTopFrame::Create(xDoc, nViewId, bHidden, pInternalArgs);
        if ( pFrame )
        {
            if ( pFrame->GetCurrentDocument() == xDoc || pFrame->PrepareClose_Impl( TRUE, TRUE ) == TRUE )
            {
                if (bHidden)
                {
                    xDoc->RestoreNoDelete();
                    xDoc->OwnerLock( TRUE );
                    xDoc->Get_Impl()->bHiddenLockedByAPI = TRUE;
                }

                if ( pFrame->GetCurrentDocument() != xDoc )
                    pFrame->InsertDocument( xDoc );
                pViewFrame = pFrame->GetCurrentViewFrame();
            }
            else
                xDoc.Clear();
        }

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
        Window* pTopWin = GetTopWindow();
        SvtDocumentTemplateDialog* pDocTemplDlg = new SvtDocumentTemplateDialog( DIALOG_NO_PARENT );
        int nRet = pDocTemplDlg->Execute();
        sal_Bool bNewWin = sal_False;
        if ( nRet == RET_OK )
        {
            rReq.Done();
            if ( pTopWin != GetTopWindow() )
            {
                // the dialogue opens a document -> a new TopWindow appears
                pTopWin = GetTopWindow();
                bNewWin = sal_True;
            }
        }

        delete pDocTemplDlg;
        if ( bNewWin )
            // after the destruction of the dialogue its parent comes to top,
            // but we want that the new document is on top
            pTopWin->ToTop();

        return;
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

    ULONG lErr = 0;
    SfxItemSet* pSet = new SfxAllItemSet( GetPool() );
    pSet->Put( SfxBoolItem( SID_TEMPLATE, TRUE ) );
    if ( !bDirect )
    {
        SfxDocumentTemplates aTmpFac;
        if( !aTemplateFileName.Len() )
            aTmpFac.GetFull( aTemplateRegion, aTemplateName, aTemplateFileName );

        if( !aTemplateFileName.Len() )
            lErr = ERRCODE_SFX_TEMPLATENOTFOUND;
    }

    INetURLObject aObj( aTemplateFileName );
    SfxErrorContext aEC( ERRCTX_SFX_LOADTEMPLATE, aObj.PathToFileName() );

    if ( lErr != ERRCODE_NONE )
    {
        ULONG lFatalErr = ERRCODE_TOERROR(lErr);
        if ( lFatalErr )
            ErrorHandler::HandleError(lErr);
    }
    else
    {
        SfxCallMode eMode = SFX_CALLMODE_SYNCHRON;
        if ( IsPlugin() )
            eMode = SFX_CALLMODE_ASYNCHRON;

        const SfxPoolItem *pRet=0;
        SfxStringItem aReferer( SID_REFERER, DEFINE_CONST_UNICODE("private:user") );
        SfxStringItem aTarget( SID_TARGETNAME, DEFINE_CONST_UNICODE("_default") );
        if ( aTemplateFileName.Len() )
        {
            INetURLObject aObj( aTemplateFileName );
            DBG_ASSERT( aObj.GetProtocol() != INET_PROT_NOT_VALID, "Illegal URL!" );

            SfxStringItem aName( SID_FILE_NAME, aObj.GetMainURL( INetURLObject::NO_DECODE ) );
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

//---------------------------------------------------------------------------

void SfxApplication::OpenDocExec_Impl( SfxRequest& rReq )
{
    DBG_MEMTEST();

    USHORT nSID = rReq.GetSlot();
    SFX_REQUEST_ARG( rReq, pFileNameItem, SfxStringItem, SID_FILE_NAME, FALSE );
    if ( pFileNameItem )
    {
        String aCommand( pFileNameItem->GetValue() );
        const SfxSlot* pSlot = GetInterface()->GetSlot( aCommand );
        if ( pSlot )
        {
            pFileNameItem = NULL;
        }
        else
        {
            sal_Int32 nIndex = aCommand.SearchAscii("slot:");
            if ( !nIndex )
            {
                USHORT nSlotId = (USHORT) String( aCommand, 5, aCommand.Len()-5 ).ToInt32();
                if ( nSlotId == SID_OPENDOC )
                    pFileNameItem = NULL;
            }
        }
    }

    if ( !pFileNameItem )
    {
        // FileDialog ausf"uhren
        SvStringsDtor* pURLList = NULL;
        String aFilter;
        void* pDummy = 0; // wegen GCC und C272
        SfxItemSet* pSet = NULL;
        String aPath;
        if ( nSID == SID_OPENTEMPLATE )
        {
            aPath = SvtPathOptions().GetTemplatePath();
            aPath = aPath.GetToken(0,';');
        }

        ULONG nErr = sfx2::FileOpenDialog_Impl(
                WB_OPEN | SFXWB_MULTISELECTION | SFXWB_SHOWVERSIONS, String(), pURLList, aFilter, pSet, aPath );

        if ( nErr == ERRCODE_ABORT )
        {
            delete pURLList;
            return;
        }

        rReq.SetArgs( *(SfxAllItemSet*)pSet );
        if (aFilter.Len() >0 )
            rReq.AppendItem( SfxStringItem( SID_FILTER_NAME, aFilter ) );
        rReq.AppendItem( SfxStringItem( SID_TARGETNAME, String::CreateFromAscii("_default") ) );
        rReq.AppendItem( SfxStringItem( SID_REFERER, String::CreateFromAscii(SFX_REFERER_USER) ) );
        delete pSet;

        if ( pURLList->Count() > 1 )
        {
            if ( nSID == SID_OPENTEMPLATE )
                rReq.AppendItem( SfxBoolItem( SID_TEMPLATE, FALSE ) );

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
    }

    BOOL bHyperlinkUsed = FALSE;

    if ( SID_OPENURL == nSID )
    {
        // SID_OPENURL does the same as SID_OPENDOC!
        rReq.SetSlot( SID_OPENDOC );
        nSID = SID_OPENDOC;
    }
    else if ( nSID == SID_OPENTEMPLATE )
    {
        rReq.AppendItem( SfxBoolItem( SID_TEMPLATE, FALSE ) );
    }
    // pass URL to OS by using ShellExecuter or open it internal
    // if it seams to be an own format.
    /* Attention!
            There exist two possibilities to open hyperlinks:
            a) using SID_OPENHYPERLINK (new)
            b) using SID_BROWSE        (old)
     */
    else if ( nSID == SID_OPENHYPERLINK )
    {
        rReq.SetSlot( SID_OPENDOC );
        nSID = SID_OPENDOC;
        bHyperlinkUsed = TRUE;
    }

    // no else here! It's optional ...
    if (!bHyperlinkUsed)
    {
        SFX_REQUEST_ARG(rReq, pHyperLinkUsedItem, SfxBoolItem, SID_BROWSE, FALSE);
        if ( pHyperLinkUsedItem )
            bHyperlinkUsed = pHyperLinkUsedItem->GetValue();
        // no "official" item, so remove it from ItemSet before using UNO-API
        rReq.RemoveItem( SID_BROWSE );
    }

    SFX_REQUEST_ARG( rReq, pFileName, SfxStringItem, SID_FILE_NAME, FALSE );
    String aFileName = pFileName->GetValue();

    String aReferer;
    SFX_REQUEST_ARG( rReq, pRefererItem, SfxStringItem, SID_REFERER, FALSE );
    if ( pRefererItem )
        aReferer = pRefererItem->GetValue();

    SFX_REQUEST_ARG( rReq, pFileFlagsItem, SfxStringItem, SID_OPTIONS, FALSE);
    if ( pFileFlagsItem )
    {
        String aFileFlags = pFileFlagsItem->GetValue();
        aFileFlags.ToUpperAscii();
        if ( STRING_NOTFOUND != aFileFlags.Search( 0x0054 ) )               // T = 54h
        {
            rReq.RemoveItem( SID_TEMPLATE );
            rReq.AppendItem( SfxBoolItem( SID_TEMPLATE, TRUE ) );
        }

        if ( STRING_NOTFOUND != aFileFlags.Search( 0x0048 ) )               // H = 48h
        {
            rReq.RemoveItem( SID_HIDDEN );
            rReq.AppendItem( SfxBoolItem( SID_HIDDEN, TRUE ) );
        }

        if ( STRING_NOTFOUND != aFileFlags.Search( 0x0052 ) )               // R = 52h
        {
            rReq.RemoveItem( SID_DOC_READONLY );
            rReq.AppendItem( SfxBoolItem( SID_DOC_READONLY, TRUE ) );
        }

        if ( STRING_NOTFOUND != aFileFlags.Search( 0x0042 ) )               // B = 42h
        {
            rReq.RemoveItem( SID_PREVIEW );
            rReq.AppendItem( SfxBoolItem( SID_PREVIEW, TRUE ) );
        }

        if ( STRING_NOTFOUND != aFileFlags.Search( 0x0053 ) )               // S = 53h
        {
            // not supported anymore
            //rReq.RemoveItem( SID_SILENT );
            //rReq.AppendItem( SfxBoolItem( SID_SILENT, TRUE ) );
        }

        rReq.RemoveItem( SID_OPTIONS );
    }

    // Mark without URL cannot be handled by hyperlink code
    if ( bHyperlinkUsed && aFileName.Len() && aFileName.GetChar(0) != '#' )
    {
        Reference< ::com::sun::star::document::XTypeDetection > xTypeDetection(
                                                                    ::comphelper::getProcessServiceFactory()->createInstance(
                                                                    ::rtl::OUString::createFromAscii( "com.sun.star.comp.framework.TypeDetection" )),
                                                                    UNO_QUERY );
        if ( xTypeDetection.is() )
        {
            URL             aURL;
            ::rtl::OUString aTypeName;

            aURL.Complete = aFileName;
            Reference < XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance(
                                                    ::rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer" )), UNO_QUERY );
            xTrans->parseStrict( aURL );

            INetProtocol aINetProtocol = INetURLObject( aURL.Complete ).GetProtocol();
            SvtExtendedSecurityOptions aExtendedSecurityOptions;
            SvtExtendedSecurityOptions::OpenHyperlinkMode eMode = aExtendedSecurityOptions.GetOpenHyperlinkMode();
            if ( eMode == SvtExtendedSecurityOptions::OPEN_WITHSECURITYCHECK )
            {
                if ( aINetProtocol == INET_PROT_FILE )
                {
                    // Check if file URL is a directory. This is not insecure!
                    osl::Directory aDir( aURL.Main );
                    sal_Bool bIsDir = ( aDir.open() == osl::Directory::E_None );

                    if ( !bIsDir && !aExtendedSecurityOptions.IsSecureHyperlink( aURL.Complete ))
                    {
                        // Security check for local files depending on the extension
                        vos::OGuard aGuard( Application::GetSolarMutex() );
                        Window *pWindow = SFX_APP()->GetTopWindow();

                        String aSecurityWarningBoxTitle( SfxResId( RID_SECURITY_WARNING_TITLE ));
                        WarningBox  aSecurityWarningBox( pWindow, SfxResId( RID_SECURITY_WARNING_HYPERLINK ));
                        aSecurityWarningBox.SetText( aSecurityWarningBoxTitle );

                        // Replace %s with the real file name
                        String aMsgText = aSecurityWarningBox.GetMessText();
                        String aMainURL( aURL.Main );
                        String aFileName;

                        utl::LocalFileHelper::ConvertURLToPhysicalName( aMainURL, aFileName );
                        aMsgText.SearchAndReplaceAscii( "%s", aFileName );
                        aSecurityWarningBox.SetMessText( aMsgText );

                        if( aSecurityWarningBox.Execute() == RET_NO )
                            return;
                    }
                }
            }
            else if ( eMode == SvtExtendedSecurityOptions::OPEN_NEVER )
            {
                return;
            }

            aTypeName = xTypeDetection->queryTypeByURL( aURL.Main );
            SfxFilterMatcher& rMatcher = SFX_APP()->GetFilterMatcher();
            const SfxFilter* pFilter = rMatcher.GetFilter4EA( aTypeName );
            if ( !pFilter || !( pFilter->IsOwnFormat() ))
            {
                // hyperlink does not link to known type => special handling (http, ftp) browser and (file) OS
                Reference< XSystemShellExecute > xSystemShellExecute( ::comphelper::getProcessServiceFactory()->createInstance(
                                                    ::rtl::OUString::createFromAscii( "com.sun.star.system.SystemShellExecute" )), UNO_QUERY );
                if ( xSystemShellExecute.is() )
                {
                    if ( aINetProtocol == INET_PROT_FTP ||
                         aINetProtocol == INET_PROT_HTTP ||
                         aINetProtocol == INET_PROT_HTTPS )
                    {
                        try
                        {
                            // start browser
                            ::rtl::OUString aURLString( aURL.Complete );
                            xSystemShellExecute->execute( aURLString, ::rtl::OUString(), SystemShellExecuteFlags::DEFAULTS );
                        }
                        catch ( ::com::sun::star::lang::IllegalArgumentException& )
                        {
                            vos::OGuard aGuard( Application::GetSolarMutex() );
                            Window *pWindow = SFX_APP()->GetTopWindow();
                            ErrorBox( pWindow, SfxResId( MSG_ERR_WEBBROWSER_NOT_STARTED )).Execute();
                        }
                        catch ( ::com::sun::star::system::SystemShellExecuteException& )
                        {
                            vos::OGuard aGuard( Application::GetSolarMutex() );
                            Window *pWindow = SFX_APP()->GetTopWindow();
                            ErrorBox( pWindow, SfxResId( MSG_ERR_WEBBROWSER_NOT_STARTED )).Execute();
                        }

                        return;
                    }
                    else if ( aINetProtocol == INET_PROT_FILE )
                    {
                        BOOL bLoadInternal = FALSE;

                        // security reservation: => we have to check the referer before executing
                        if ( SFX_APP()->IsSecureURL( String(), &aReferer ) )
                        {
                            ::rtl::OUString aSysPathFileName;
                            ::osl::FileBase::RC nError = ::osl::FileBase::getSystemPathFromFileURL( aURL.Complete, aSysPathFileName );
                            if ( nError == ::osl::FileBase::E_None )
                            {
                                try
                                {
                                    // give os this file
                                    xSystemShellExecute->execute( aSysPathFileName, ::rtl::OUString(), SystemShellExecuteFlags::DEFAULTS );
                                }
                                catch ( ::com::sun::star::lang::IllegalArgumentException& )
                                {
                                    vos::OGuard aGuard( Application::GetSolarMutex() );
                                    Window *pWindow = SFX_APP()->GetTopWindow();
                                    ErrorBox( pWindow, SfxResId( MSG_ERR_EXTERNAL_APP_NOT_FOUND )).Execute();
                                }
                                catch ( ::com::sun::star::system::SystemShellExecuteException& )
                                {
                                    if ( !pFilter )
                                    {
                                        vos::OGuard aGuard( Application::GetSolarMutex() );
                                        Window *pWindow = SFX_APP()->GetTopWindow();
                                        ErrorBox( pWindow, SfxResId( MSG_ERR_EXTERNAL_APP_NOT_FOUND )).Execute();
                                    }
                                    else
                                    {
                                        rReq.RemoveItem( SID_TARGETNAME );
                                        rReq.AppendItem( SfxStringItem( SID_TARGETNAME, String::CreateFromAscii("_default") ) );
                                        bLoadInternal = TRUE;
                                    }
                                }
                            }
                        }
                        else
                        {
                            SfxErrorContext aCtx( ERRCTX_SFX_OPENDOC, aURL.Complete );
                            ErrorHandler::HandleError( ERRCODE_IO_ACCESSDENIED );
                        }

                        if ( !bLoadInternal )
                            return;
                    }
                    else if ( aINetProtocol == INET_PROT_MAILTO )
                    {
                        // don't dispatch mailto hyperlink to desktop dispatcher
                        rReq.RemoveItem( SID_TARGETNAME );
                        rReq.AppendItem( SfxStringItem( SID_TARGETNAME, String::CreateFromAscii("_self") ) );
                    }
                }
            }
            else
            {
                // hyperlink document must be loaded into a new frame
                rReq.RemoveItem( SID_TARGETNAME );
                rReq.AppendItem( SfxStringItem( SID_TARGETNAME, String::CreateFromAscii("_default") ) );
            }
        }
    }

    if ( !SFX_APP()->IsSecureURL( aFileName, &aReferer ) )
    {
        SfxErrorContext aCtx( ERRCTX_SFX_OPENDOC, aFileName );
        ErrorHandler::HandleError( ERRCODE_IO_ACCESSDENIED );
        return;
    }

    SFX_REQUEST_ARG(rReq, pFrmItem, SfxFrameItem, SID_DOCFRAME, FALSE);
    SfxFrame *pFrame = NULL;
    if ( pFrmItem )
        pFrame = pFrmItem->GetFrame();
    else if ( SfxViewFrame::Current() )
        pFrame = SfxViewFrame::Current()->GetFrame();

    // check if caller has set a callback
    SFX_REQUEST_ARG(rReq, pLinkItem, SfxLinkItem, SID_DONELINK, FALSE );

    // remove from Itemset, because it confuses the parameter transformation
    if ( pLinkItem )
        pLinkItem = (SfxLinkItem*) pLinkItem->Clone();

    rReq.RemoveItem( SID_DONELINK );

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

    // This request is a UI call. We have to set the right values inside the MediaDescriptor
    // for: InteractionHandler, StatusIndicator, MacroExecutionMode and DocTemplate.
    // But we have to look for already existing values or for real hidden requests.
    SFX_REQUEST_ARG(rReq, pPreviewItem, SfxBoolItem, SID_PREVIEW, FALSE);
    if (!bHidden && ( !pPreviewItem || !pPreviewItem->GetValue() ) )
    {
        SFX_REQUEST_ARG(rReq, pInteractionItem, SfxUnoAnyItem, SID_INTERACTIONHANDLER, FALSE);
        SFX_REQUEST_ARG(rReq, pMacroExecItem  , SfxUInt16Item, SID_MACROEXECMODE     , FALSE);
        SFX_REQUEST_ARG(rReq, pDocTemplateItem, SfxUInt16Item, SID_UPDATEDOCMODE     , FALSE);

        if (!pInteractionItem)
        {
            Reference < ::com::sun::star::task::XInteractionHandler > xHdl( ::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.comp.uui.UUIInteractionHandler")), UNO_QUERY );
            if (xHdl.is())
                rReq.AppendItem( SfxUnoAnyItem(SID_INTERACTIONHANDLER,::com::sun::star::uno::makeAny(xHdl)) );
        }
        if (!pMacroExecItem)
            rReq.AppendItem( SfxUInt16Item(SID_MACROEXECMODE,::com::sun::star::document::MacroExecMode::USE_CONFIG) );
        if (!pDocTemplateItem)
            rReq.AppendItem( SfxUInt16Item(SID_UPDATEDOCMODE,::com::sun::star::document::UpdateDocMode::ACCORDING_TO_CONFIG) );
    }

    // convert items to properties for framework API calls
    Sequence < PropertyValue > aArgs;
    TransformItems( SID_OPENDOC, *rReq.GetArgs(), aArgs );

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

    BOOL bIsBlankTarget = ( aTarget.compareToAscii( "_blank" ) == COMPARE_EQUAL || aTarget.compareToAscii( "_default" ) == COMPARE_EQUAL );
    Reference < XController > xController;
    if ( ( !bIsBlankTarget && pFrame ) || pLinkItem || !rReq.IsSynchronCall() )
    {
        // if a frame is given, it must be used for the starting point of the targetting mechanism
        // this code is also used if asynchron loading is possible, because loadComponent always is synchron
        Reference < XFrame > xFrame;
        if ( pFrame )
            xFrame = pFrame->GetFrameInterface();
        else
            xFrame = Reference < XFrame >( ::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.frame.Desktop")), UNO_QUERY );

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
        Reference < XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance( rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer" )), UNO_QUERY );
        xTrans->parseStrict( aURL );

        // load document using dispatch framework
        if( pLinkItem || rReq.IsSynchronCall() )
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
                Reference < XNotifyingDispatch > xNot( xDisp, UNO_QUERY );
                if ( xNot.is() )
                {
                    // create listener for notification of load success or fail
                    SfxOpenDocStatusListener_Impl* pListener = new SfxOpenDocStatusListener_Impl();
                    pListener->acquire();
                    xNot->dispatchWithNotification( aURL, aArgs, pListener );

                    // stay on the stack until result has been notified
                    while ( !pListener->bFinished )
                        Application::Yield();

                    if ( pListener->bSuccess )
                        // successful loading, get loaded controller
                        xController = xFrame->getController();

                    pListener->release();
                }
            }

            if ( !xController.is() && bIsBlankTarget )
            {
                // a blank frame would have been created in findFrame; in this case I am the owner and I must delete it
                Reference < XCloseable > xClose( xFrame, UNO_QUERY );
                if ( xClose.is() )
                    xClose->close(sal_True);
                else
                    xFrame->dispose();
            }
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
        // synchron loading without a given frame or as blank frame
        SFX_REQUEST_ARG( rReq, pFileNameItem, SfxStringItem, SID_FILE_NAME, FALSE );
        Reference < XComponentLoader > xDesktop( ::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.frame.Desktop")), UNO_QUERY );
        Reference < XComponent > xComp = xDesktop->loadComponentFromURL( pFileNameItem->GetValue(), aTarget, 0, aArgs );
        Reference < XModel > xModel( xComp, UNO_QUERY );
        if ( xModel.is() )
            xController = xModel->getCurrentController();
        else
            xController = Reference < XController >( xComp, UNO_QUERY );
    }
/*
    if ( bHidden )
        aTarget = String::CreateFromAscii("_blank");

    // if a frame is given, it must be used for the starting point of the targeting mechanism
    Reference < XFrame > xFrame;
    if ( pFrame )
        xFrame = pFrame->GetFrameInterface();
    else
        xFrame = Reference < XFrame >( ::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.frame.Desktop")), UNO_QUERY );

    if( aFileName.Len() && aFileName.GetChar(0) == '#' )
    {
        // Mark without URL
        SfxViewFrame *pView = pFrame ? pFrame->GetCurrentViewFrame() : 0;
        if ( !pView )
            pView = SfxViewFrame::Current();
        pView->GetViewShell()->JumpToMark( aFileName.Copy(1) );
        rReq.SetReturnValue( SfxViewFrameItem( 0, pView ) );
    }
    else
    {
        // evaluate target name
        try
        {
            Reference < XComponentLoader > xLoader( xFrame, UNO_QUERY );
            Reference < XModel > xModel( xLoader->loadComponentFromURL( aFileName, aTarget, FrameSearchFlag::GLOBAL|FrameSearchFlag::CREATE, aArgs ), UNO_QUERY );
            Reference < XController > xController;
            if ( xModel.is() )
                xController = xModel->getCurrentController();
    */
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

            if( ( bHidden || !bCreateView ) )
                pSh->RestoreNoDelete();
        }
    }
/*
        }
        catch (com::sun::star::uno::Exception&)
        {
        }
    }
*/
    if ( pLinkItem )
    {
        SfxPoolItem* pRet = rReq.GetReturnValue()->Clone();
        pLinkItem->GetValue().Call(pRet);
        delete pLinkItem;
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
