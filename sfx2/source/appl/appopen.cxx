/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_sfx2.hxx"
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/system/XSystemShellExecute.hpp>
#include <com/sun/star/document/XTypeDetection.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/task/ErrorCodeRequest.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase1.hxx>
#include <rtl/ustring.hxx>


#include <comphelper/storagehelper.hxx>
#include <comphelper/synchronousdispatch.hxx>
#include <comphelper/configurationhelper.hxx>
#include <comphelper/sequenceasvector.hxx>

#include <vcl/wrkwin.hxx>
#include <svl/intitem.hxx>
#include <vcl/msgbox.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <sfx2/doctempl.hxx>
#include <svtools/sfxecode.hxx>
#include <framework/preventduplicateinteraction.hxx>
#include <svtools/ehdl.hxx>
#include <basic/sbxobj.hxx>
#include <svl/urihelper.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/moduleoptions.hxx>
#include <svtools/templdlg.hxx>
#include <osl/file.hxx>
#include <unotools/extendedsecurityoptions.hxx>
#include <comphelper/docpasswordhelper.hxx>
#include <vcl/svapp.hxx>

#include <osl/mutex.hxx>

#include <rtl/logfile.hxx>

#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/new.hxx>
#include <sfx2/objitem.hxx>
#include <sfx2/objsh.hxx>
#include <svl/slstitm.hxx>
#include "objshimp.hxx"
#include "openflag.hxx"
#include <sfx2/passwd.hxx>
#include "referers.hxx"
#include <sfx2/request.hxx>
#include "sfxresid.hxx"
#include <sfx2/viewsh.hxx>
#include "app.hrc"
#include <sfx2/viewfrm.hxx>
#include <sfx2/sfxuno.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/event.hxx>

#define _SVSTDARR_STRINGSDTOR
#include <svl/svstdarr.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::system;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::container;
using namespace ::cppu;
using namespace ::sfx2;

namespace css = ::com::sun::star;

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

void SAL_CALL SfxOpenDocStatusListener_Impl::disposing( const EventObject& ) throw(RuntimeException)
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
                         (!bForbidVisible || !SfxViewFrame::GetFirst( xDoc, TRUE )) &&
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

        SfxViewFrame* pFrame;
        for( pFrame = SfxViewFrame::GetFirst( xDoc );
             pFrame && !pFrame->IsVisible();
             pFrame = SfxViewFrame::GetNext( *pFrame, xDoc ) ) ;
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

void SetTemplate_Impl( const String &rFileName,
                        const String &rLongName,
                        SfxObjectShell *pDoc)
{
    // write TemplateName to DocumentInfo of document
    // TemplateDate stays as default (=current date)
    pDoc->ResetFromTemplate( rLongName, rFileName );
}

//--------------------------------------------------------------------

class SfxDocPasswordVerifier : public ::comphelper::IDocPasswordVerifier
{
public:
    inline explicit     SfxDocPasswordVerifier( const Reference< embed::XStorage >& rxStorage ) :
                            mxStorage( rxStorage ) {}

    virtual ::comphelper::DocPasswordVerifierResult
                        verifyPassword( const ::rtl::OUString& rPassword );

private:
    Reference< embed::XStorage > mxStorage;
};

::comphelper::DocPasswordVerifierResult SfxDocPasswordVerifier::verifyPassword( const ::rtl::OUString& rPassword )
{
    ::comphelper::DocPasswordVerifierResult eResult = ::comphelper::DocPasswordVerifierResult_WRONG_PASSWORD;
    try
    {
        // check the password
        // if the password correct is the stream will be opened successfuly
        // and immediatelly closed
        ::comphelper::OStorageHelper::SetCommonStoragePassword( mxStorage, rPassword );

        mxStorage->openStreamElement(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "content.xml" ) ),
                embed::ElementModes::READ | embed::ElementModes::NOCREATE );

        // no exception -> success
        eResult = ::comphelper::DocPasswordVerifierResult_OK;
    }
    catch( const packages::WrongPasswordException& )
    {
        eResult = ::comphelper::DocPasswordVerifierResult_WRONG_PASSWORD;
    }
    catch( const uno::Exception& )
    {
        // unknown error, do not try to ask again
        eResult = ::comphelper::DocPasswordVerifierResult_ABORT;
    }
    return eResult;
}

//--------------------------------------------------------------------

sal_uInt32 CheckPasswd_Impl
(
    SfxObjectShell*  pDoc,
    SfxItemPool&     /*rPool*/, // Pool, falls ein Set erzeugt werden mus
    SfxMedium*       pFile      // das Medium, dessen Passwort gfs. erfragt werden soll
)

/*  [Beschreibung]

    Zu einem Medium das Passwort erfragen; funktioniert nur, wenn es sich
    um einen Storage handelt.
    Wenn in der Documentinfo das Passwort-Flag gesetzt ist, wird
    das Passwort vom Benutzer per Dialog erfragt und an dem Set
    des Mediums gesetzt; das Set wird, wenn nicht vorhanden, erzeugt.
*/
{
    ULONG nRet = ERRCODE_NONE;

    if( ( !pFile->GetFilter() || pFile->IsStorage() ) )
    {
        uno::Reference< embed::XStorage > xStorage = pFile->GetStorage( sal_True );
        if( xStorage.is() )
        {
            uno::Reference< beans::XPropertySet > xStorageProps( xStorage, uno::UNO_QUERY );
            if ( xStorageProps.is() )
            {
                sal_Bool bIsEncrypted = sal_False;
                try {
                    xStorageProps->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HasEncryptedEntries")) )
                        >>= bIsEncrypted;
                } catch( uno::Exception& )
                {
                    // TODO/LATER:
                    // the storage either has no encrypted elements or it's just
                    // does not allow to detect it, probably it should be implemented laiter
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
                        Reference< ::com::sun::star::task::XInteractionHandler > xInteractionHandler = pFile->GetInteractionHandler();
                        if( xInteractionHandler.is() )
                        {
                            // use the comphelper password helper to request a password
                            ::rtl::OUString aDocumentName = INetURLObject( pFile->GetOrigURL() ).GetMainURL( INetURLObject::DECODE_WITH_CHARSET );
                            SfxDocPasswordVerifier aVerifier( xStorage );
                            ::rtl::OUString aPassword = ::comphelper::DocPasswordHelper::requestAndVerifyDocPassword(
                                aVerifier, ::rtl::OUString(), xInteractionHandler, aDocumentName, comphelper::DocPasswordRequestType_STANDARD );

                            if ( aPassword.getLength() > 0 )
                            {
                                pSet->Put( SfxStringItem( SID_PASSWORD, aPassword ) );

                                try
                                {
                                    // update the version list of the medium using the new password
                                    pFile->GetVersionList();
                                }
                                catch( uno::Exception& )
                                {
                                    // TODO/LATER: set the error code
                                }

                                nRet = ERRCODE_NONE;
                            }
                            else
                                nRet = ERRCODE_IO_ABORT;
                        }
                    }
                }
            }
            else
            {
                OSL_ENSURE( sal_False, "A storage must implement XPropertySet interface!" );
                nRet = ERRCODE_SFX_CANTGETPASSWD;
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

    if ( !aMedium.GetStorage( sal_True ).is() )
        aMedium.GetInStream();

    if ( aMedium.GetError() )
    {
        delete pSet;
        return aMedium.GetErrorCode();
    }

    aMedium.UseInteractionHandler( TRUE );
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
        if ( pObj )
            xDoc = PTR_CAST( SfxObjectShell, pObj->GetShell() );
        else
        {
            const SfxViewFrameItem *pView = PTR_CAST( SfxViewFrameItem, pRet );
            if ( pView )
            {
                SfxViewFrame *pFrame = pView->GetFrame();
                if ( pFrame )
                    xDoc = pFrame->GetObjectShell();
            }
        }

        if ( !xDoc.Is() )
            return ERRCODE_SFX_DOLOADFAILED;
    }
    else
    {
        if ( !xDoc.Is() )
            xDoc = SfxObjectShell::CreateObject( pFilter->GetServiceName() );

        SfxMedium *pMedium = new SfxMedium( rFileName, STREAM_STD_READ, FALSE, pFilter, pSet );
        if(!xDoc->DoLoad(pMedium))
        {
            ErrCode nErrCode = xDoc->GetErrorCode();
            xDoc->DoClose();
            xDoc.Clear();
            return nErrCode;
        }
    }

    if( bCopy )
    {
        try
        {
            // TODO: introduce error handling

            uno::Reference< embed::XStorage > xTempStorage = ::comphelper::OStorageHelper::GetTemporaryStorage();
            if( !xTempStorage.is() )
                throw uno::RuntimeException();

               xDoc->GetStorage()->copyToStorage( xTempStorage );

            if ( !xDoc->DoSaveCompleted( new SfxMedium( xTempStorage, String() ) ) )
                throw uno::RuntimeException();
        }
        catch( uno::Exception& )
        {
            xDoc->DoClose();
            xDoc.Clear();

            // TODO: transfer correct error outside
            return ERRCODE_SFX_GENERAL;
        }

        SetTemplate_Impl( rFileName, String(), xDoc );
    }
    else
        SetTemplate_Impl( rFileName, String(), xDoc );

    xDoc->SetNoName();
    xDoc->InvalidateName();
    xDoc->SetModified(FALSE);
    xDoc->ResetError();

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >  xModel ( xDoc->GetModel(), ::com::sun::star::uno::UNO_QUERY );
    if ( xModel.is() )
    {
        SfxItemSet* pNew = xDoc->GetMedium()->GetItemSet()->Clone();
        pNew->ClearItem( SID_PROGRESS_STATUSBAR_CONTROL );
        pNew->ClearItem( SID_FILTER_NAME );
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

void SfxApplication::NewDocDirectExec_Impl( SfxRequest& rReq )
{
    DBG_MEMTEST();

    SFX_REQUEST_ARG( rReq, pFactoryItem, SfxStringItem, SID_NEWDOCDIRECT, FALSE);
    String aFactName;
    if ( pFactoryItem )
        aFactName = pFactoryItem->GetValue();
   else
        aFactName = SvtModuleOptions().GetDefaultModuleName();


    SfxRequest aReq( SID_OPENDOC, SFX_CALLMODE_SYNCHRON, GetPool() );
    String aFact = String::CreateFromAscii("private:factory/");
    aFact += aFactName;
    aReq.AppendItem( SfxStringItem( SID_FILE_NAME, aFact ) );
    aReq.AppendItem( SfxFrameItem( SID_DOCFRAME, GetFrame() ) );
    aReq.AppendItem( SfxStringItem( SID_TARGETNAME, String::CreateFromAscii( "_default" ) ) );

    // TODO/LATER: Should the other arguments be transfered as well?
    SFX_REQUEST_ARG( rReq, pDefaultPathItem, SfxStringItem, SID_DEFAULTFILEPATH, FALSE);
    if ( pDefaultPathItem )
        aReq.AppendItem( *pDefaultPathItem );
    SFX_REQUEST_ARG( rReq, pDefaultNameItem, SfxStringItem, SID_DEFAULTFILENAME, FALSE);
    if ( pDefaultNameItem )
        aReq.AppendItem( *pDefaultNameItem );

    SFX_APP()->ExecuteSlot( aReq );
    const SfxViewFrameItem* pItem = PTR_CAST( SfxViewFrameItem, aReq.GetReturnValue() );
    if ( pItem )
        rReq.SetReturnValue( SfxFrameItem( 0, pItem->GetFrame() ) );
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

    String  aTemplateRegion, aTemplateName, aTemplateFileName;
    BOOL    bDirect = FALSE; // "uber FileName anstelle Region/Template
    SfxErrorContext aEc(ERRCTX_SFX_NEWDOC);
    if ( !pTemplNameItem && !pTemplFileNameItem )
    {
        Window* pTopWin = GetTopWindow();
        SvtDocumentTemplateDialog* pDocTemplDlg = new SvtDocumentTemplateDialog( NULL );
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
        if ( bNewWin && pTopWin )
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

        const SfxPoolItem *pRet=0;
        SfxStringItem aReferer( SID_REFERER, DEFINE_CONST_UNICODE("private:user") );
        SfxStringItem aTarget( SID_TARGETNAME, DEFINE_CONST_UNICODE("_default") );
        if ( aTemplateFileName.Len() )
        {
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

namespace {

/**
 * Check if a given filter type should open the hyperlinked document
 * natively.
 *
 * @param rFilter filter object
 */
bool lcl_isFilterNativelySupported(const SfxFilter& rFilter)
{
    if (rFilter.IsOwnFormat())
        return true;

    ::rtl::OUString aName = rFilter.GetFilterName();
    if (aName.indexOf(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MS Excel"))) == 0)
        // We can handle all Excel variants natively.
        return true;

    return false;
}

}

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
        // get FileName from dialog
        SvStringsDtor* pURLList = NULL;
        String aFilter;
        SfxItemSet* pSet = NULL;
        String aPath;
        SFX_REQUEST_ARG( rReq, pFolderNameItem, SfxStringItem, SID_PATH, FALSE );
        if ( pFolderNameItem )
            aPath = pFolderNameItem->GetValue();
        else if ( nSID == SID_OPENTEMPLATE )
        {
            aPath = SvtPathOptions().GetTemplatePath();
            sal_Int32 nTokenCount = aPath.GetTokenCount( ';' );
            aPath = aPath.GetToken(
                sal::static_int_cast< xub_StrLen >(
                    nTokenCount ? ( nTokenCount - 1 ) : 0 ),
                ';' );
        }

        sal_Int16 nDialog = SFX2_IMPL_DIALOG_CONFIG;
        SFX_REQUEST_ARG( rReq, pSystemDialogItem, SfxBoolItem, SID_FILE_DIALOG, FALSE );
        if ( pSystemDialogItem )
            nDialog = pSystemDialogItem->GetValue() ? SFX2_IMPL_DIALOG_SYSTEM : SFX2_IMPL_DIALOG_OOO;

        String sStandardDir;

        SFX_REQUEST_ARG( rReq, pStandardDirItem, SfxStringItem, SID_STANDARD_DIR, FALSE );
        if ( pStandardDirItem )
            sStandardDir = pStandardDirItem->GetValue();

        ::com::sun::star::uno::Sequence< ::rtl::OUString >  aBlackList;

        SFX_REQUEST_ARG( rReq, pBlackListItem, SfxStringListItem, SID_BLACK_LIST, FALSE );
        if ( pBlackListItem )
            pBlackListItem->GetStringList( aBlackList );


        ULONG nErr = sfx2::FileOpenDialog_Impl(
                WB_OPEN | SFXWB_MULTISELECTION | SFXWB_SHOWVERSIONS, String(), pURLList, aFilter, pSet, &aPath, nDialog, sStandardDir, aBlackList );

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

        if ( pURLList->Count() )
        {
            if ( nSID == SID_OPENTEMPLATE )
                rReq.AppendItem( SfxBoolItem( SID_TEMPLATE, FALSE ) );

            // This helper wraps an existing (or may new created InteractionHandler)
            // intercept all incoming interactions and provide usefull informations
            // later if the following transaction was finished.

            ::framework::PreventDuplicateInteraction*                 pHandler       = new ::framework::PreventDuplicateInteraction(::comphelper::getProcessServiceFactory());
            css::uno::Reference< css::task::XInteractionHandler >     xHandler       (static_cast< css::task::XInteractionHandler* >(pHandler), css::uno::UNO_QUERY);
            css::uno::Reference< css::task::XInteractionHandler >     xWrappedHandler;

            // wrap existing handler or create new UUI handler
            SFX_REQUEST_ARG(rReq, pInteractionItem, SfxUnoAnyItem, SID_INTERACTIONHANDLER, FALSE);
            if (pInteractionItem)
            {
                pInteractionItem->GetValue() >>= xWrappedHandler;
                rReq.RemoveItem( SID_INTERACTIONHANDLER );
            }
            if (xWrappedHandler.is())
                pHandler->setHandler(xWrappedHandler);
            else
                pHandler->useDefaultUUIHandler();
            rReq.AppendItem( SfxUnoAnyItem(SID_INTERACTIONHANDLER,::com::sun::star::uno::makeAny(xHandler)) );

            // define rules for this handler
            css::uno::Type                                            aInteraction = ::getCppuType(static_cast< css::task::ErrorCodeRequest* >(0));
            ::framework::PreventDuplicateInteraction::InteractionInfo aRule        (aInteraction, 1);
            pHandler->addInteractionRule(aRule);

            for ( USHORT i = 0; i < pURLList->Count(); ++i )
            {
                String aURL = *(pURLList->GetObject(i));
                rReq.RemoveItem( SID_FILE_NAME );
                rReq.AppendItem( SfxStringItem( SID_FILE_NAME, aURL ) );

                // synchron ausf"uhren, damit beim Reschedulen nicht schon das n"achste Dokument
                // geladen wird
                // TODO/LATER: use URLList argument and always remove one document after another, each step in asychronous execution, until finished
                // but only if reschedule is a problem
                GetDispatcher_Impl()->Execute( SID_OPENDOC, SFX_CALLMODE_SYNCHRON, *rReq.GetArgs() );

                // check for special interaction "NO MORE DOCUMENTS ALLOWED" and
                // break loop then. Otherwise we risk showing the same interaction more then once.
                if ( pHandler->getInteractionInfo(aInteraction, &aRule) )
                {
                    if (aRule.m_nCallCount > 0)
                    {
                        if (aRule.m_xRequest.is())
                        {
                            css::task::ErrorCodeRequest aRequest;
                            if (aRule.m_xRequest->getRequest() >>= aRequest)
                            {
                                if (aRequest.ErrCode ==
                                    sal::static_int_cast< sal_Int32 >(
                                        ERRCODE_SFX_NOMOREDOCUMENTSALLOWED))
                                    break;
                            }
                        }
                    }
                }
            }

            delete pURLList;
            return;
        }
        delete pURLList;
    }

    if ( !rReq.IsSynchronCall() )
    {
        // now check wether a stream is already there
        // if not: download it in a thread and restart the call
        // return;
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

        rReq.RemoveItem( SID_OPTIONS );
    }

    // Mark without URL cannot be handled by hyperlink code
    if ( bHyperlinkUsed && aFileName.Len() && aFileName.GetChar(0) != '#' )
    {
        Reference< ::com::sun::star::document::XTypeDetection > xTypeDetection(
                                                                    ::comphelper::getProcessServiceFactory()->createInstance(
                                                                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.TypeDetection"))),
                                                                    UNO_QUERY );
        if ( xTypeDetection.is() )
        {
            URL             aURL;
            ::rtl::OUString aTypeName;

            aURL.Complete = aFileName;
            Reference < XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance(
                                                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.URLTransformer"))), UNO_QUERY );
            xTrans->parseStrict( aURL );

            INetProtocol aINetProtocol = INetURLObject( aURL.Complete ).GetProtocol();
            SvtExtendedSecurityOptions aExtendedSecurityOptions;
            SvtExtendedSecurityOptions::OpenHyperlinkMode eMode = aExtendedSecurityOptions.GetOpenHyperlinkMode();

            if ( eMode == SvtExtendedSecurityOptions::OPEN_NEVER && aINetProtocol != INET_PROT_VND_SUN_STAR_HELP )
            {
                SolarMutexGuard aGuard;
                Window *pWindow = SFX_APP()->GetTopWindow();

                String aSecurityWarningBoxTitle( SfxResId( RID_SECURITY_WARNING_TITLE ));
                WarningBox  aSecurityWarningBox( pWindow, SfxResId( RID_SECURITY_WARNING_NO_HYPERLINKS ));
                aSecurityWarningBox.SetText( aSecurityWarningBoxTitle );
                aSecurityWarningBox.Execute();
                return;
            }

            aTypeName = xTypeDetection->queryTypeByURL( aURL.Main );
            SfxFilterMatcher& rMatcher = SFX_APP()->GetFilterMatcher();
            const SfxFilter* pFilter = rMatcher.GetFilter4EA( aTypeName );
            if (!pFilter || !lcl_isFilterNativelySupported(*pFilter))
            {
                // hyperlink does not link to own type => special handling (http, ftp) browser and (other external protocols) OS
                Reference< XSystemShellExecute > xSystemShellExecute( ::comphelper::getProcessServiceFactory()->createInstance(
                                                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.system.SystemShellExecute"))), UNO_QUERY );
                if ( xSystemShellExecute.is() )
                {
                    if ( aINetProtocol == INET_PROT_MAILTO )
                    {
                        // don't dispatch mailto hyperlink to desktop dispatcher
                        rReq.RemoveItem( SID_TARGETNAME );
                        rReq.AppendItem( SfxStringItem( SID_TARGETNAME, String::CreateFromAscii("_self") ) );
                    }
                    else if ( aINetProtocol == INET_PROT_FTP ||
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
                            SolarMutexGuard aGuard;
                            Window *pWindow = SFX_APP()->GetTopWindow();
                            ErrorBox( pWindow, SfxResId( MSG_ERR_NO_WEBBROWSER_FOUND )).Execute();
                        }
                        catch ( ::com::sun::star::system::SystemShellExecuteException& )
                        {
                            SolarMutexGuard aGuard;
                            Window *pWindow = SFX_APP()->GetTopWindow();
                            ErrorBox( pWindow, SfxResId( MSG_ERR_NO_WEBBROWSER_FOUND )).Execute();
                        }

                        return;
                    }
                    else
                    {
                        // check for "internal" protocols that should not be forwarded to the system
                        Sequence < ::rtl::OUString > aProtocols(2);

                        // add special protocols that always should be treated as internal
                        aProtocols[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:*"));
                        aProtocols[1] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("vnd.sun.star.*"));

                        try
                        {
                            // get registered protocol handlers from configuration
                            Reference < XNameAccess > xAccess( ::comphelper::ConfigurationHelper::openConfig( ::comphelper::getProcessServiceFactory(),
                                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.Office.ProtocolHandler/HandlerSet")), ::comphelper::ConfigurationHelper::E_READONLY ), UNO_QUERY );
                            if ( xAccess.is() )
                            {
                                Sequence < ::rtl::OUString > aNames = xAccess->getElementNames();
                                for ( sal_Int32 nName = 0; nName < aNames.getLength(); nName ++)
                                {
                                    Reference < XPropertySet > xSet;
                                    Any aRet = xAccess->getByName( aNames[nName] );
                                    aRet >>= xSet;
                                    if ( xSet.is() )
                                    {
                                        // copy protocols
                                        aRet = xSet->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Protocols")) );
                                        Sequence < ::rtl::OUString > aTmp;
                                        aRet >>= aTmp;

                                        // todo: add operator+= to SequenceAsVector class and use SequenceAsVector for aProtocols
                                        sal_Int32 nLength = aProtocols.getLength();
                                        aProtocols.realloc( nLength+aTmp.getLength() );
                                        for ( sal_Int32 n=0; n<aTmp.getLength(); n++ )
                                            aProtocols[(++nLength)-1] = aTmp[n];
                                    }
                                }
                            }
                        }
                        catch ( Exception& )
                        {
                            // registered protocols could not be read
                        }

                        sal_Bool bFound = sal_False;
                        for ( sal_Int32 nProt=0; nProt<aProtocols.getLength(); nProt++ )
                        {
                            WildCard aPattern(aProtocols[nProt]);
                            if ( aPattern.Matches( aURL.Complete ) )
                            {
                                bFound = sal_True;
                                break;
                            }
                        }

                        if ( !bFound )
                        {
                            BOOL bLoadInternal = FALSE;

                            // security reservation: => we have to check the referer before executing
                            if (SFX_APP()->IsSecureURL(rtl::OUString(), &aReferer))
                            {
                                ::rtl::OUString aURLString( aURL.Complete );

                                try
                                {
                                    // give os this file
                                    xSystemShellExecute->execute( aURLString, ::rtl::OUString(), SystemShellExecuteFlags::DEFAULTS );
                                }
                                catch ( ::com::sun::star::lang::IllegalArgumentException& )
                                {
                                    SolarMutexGuard aGuard;
                                    Window *pWindow = SFX_APP()->GetTopWindow();
                                    ErrorBox( pWindow, SfxResId( MSG_ERR_NO_WEBBROWSER_FOUND )).Execute();
                                }
                                catch ( ::com::sun::star::system::SystemShellExecuteException& )
                                {
                                    if ( !pFilter )
                                    {
                                        SolarMutexGuard aGuard;
                                        Window *pWindow = SFX_APP()->GetTopWindow();
                                        ErrorBox( pWindow, SfxResId( MSG_ERR_NO_WEBBROWSER_FOUND )).Execute();
                                    }
                                    else
                                    {
                                        rReq.RemoveItem( SID_TARGETNAME );
                                        rReq.AppendItem( SfxStringItem( SID_TARGETNAME, String::CreateFromAscii("_default") ) );
                                        bLoadInternal = TRUE;
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

    if ( !SFX_APP()->IsSecureURL( INetURLObject(aFileName), &aReferer ) )
    {
        SfxErrorContext aCtx( ERRCTX_SFX_OPENDOC, aFileName );
        ErrorHandler::HandleError( ERRCODE_IO_ACCESSDENIED );
        return;
    }

    SfxFrame* pTargetFrame = NULL;
    Reference< XFrame > xTargetFrame;

    SFX_REQUEST_ARG(rReq, pFrameItem, SfxFrameItem, SID_DOCFRAME, FALSE);
    if ( pFrameItem )
        pTargetFrame = pFrameItem->GetFrame();

    if ( !pTargetFrame )
    {
        SFX_REQUEST_ARG(rReq, pUnoFrameItem, SfxUnoFrameItem, SID_FILLFRAME, FALSE);
        if ( pUnoFrameItem )
            xTargetFrame = pUnoFrameItem->GetFrame();
    }

    if ( !pTargetFrame && !xTargetFrame.is() && SfxViewFrame::Current() )
        pTargetFrame = &SfxViewFrame::Current()->GetFrame();

    // check if caller has set a callback
    SFX_REQUEST_ARG(rReq, pLinkItem, SfxLinkItem, SID_DONELINK, FALSE );

    // remove from Itemset, because it confuses the parameter transformation
    if ( pLinkItem )
        pLinkItem = (SfxLinkItem*) pLinkItem->Clone();

    rReq.RemoveItem( SID_DONELINK );

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
            Reference < ::com::sun::star::task::XInteractionHandler > xHdl( ::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.uui.UUIInteractionHandler"))), UNO_QUERY );
            if (xHdl.is())
                rReq.AppendItem( SfxUnoAnyItem(SID_INTERACTIONHANDLER,::com::sun::star::uno::makeAny(xHdl)) );
        }
        if (!pMacroExecItem)
            rReq.AppendItem( SfxUInt16Item(SID_MACROEXECMODE,::com::sun::star::document::MacroExecMode::USE_CONFIG) );
        if (!pDocTemplateItem)
            rReq.AppendItem( SfxUInt16Item(SID_UPDATEDOCMODE,::com::sun::star::document::UpdateDocMode::ACCORDING_TO_CONFIG) );
    }

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
    {
        aTarget = String::CreateFromAscii("_blank");
        DBG_ASSERT( rReq.IsSynchronCall() || pLinkItem, "Hidden load process must be done synchronously!" );
    }

    Reference < XController > xController;
        // if a frame is given, it must be used for the starting point of the targetting mechanism
        // this code is also used if asynchronous loading is possible, because loadComponent always is synchron
        if ( !xTargetFrame.is() )
        {
            if ( pTargetFrame )
            {
                xTargetFrame = pTargetFrame->GetFrameInterface();
            }
            else
            {
                xTargetFrame.set( ::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop"))), UNO_QUERY );
            }
        }

        // make URL ready
        SFX_REQUEST_ARG( rReq, pURLItem, SfxStringItem, SID_FILE_NAME, FALSE );
        aFileName = pURLItem->GetValue();
        if( aFileName.Len() && aFileName.GetChar(0) == '#' ) // Mark without URL
        {
            SfxViewFrame *pView = pTargetFrame ? pTargetFrame->GetCurrentViewFrame() : 0;
            if ( !pView )
                pView = SfxViewFrame::Current();
            pView->GetViewShell()->JumpToMark( aFileName.Copy(1) );
            rReq.SetReturnValue( SfxViewFrameItem( 0, pView ) );
            return;
        }

        // convert items to properties for framework API calls
        Sequence < PropertyValue > aArgs;
        TransformItems( SID_OPENDOC, *rReq.GetArgs(), aArgs );

        // TODO/LATER: either remove LinkItem or create an asynchronous process for it
        if( bHidden || pLinkItem || rReq.IsSynchronCall() )
        {
            // if loading must be done synchron, we must wait for completion to get a return value
            // find frame by myself; I must konw the exact frame to get the controller for the return value from it
            Reference < XComponent > xComp;

            try
            {
                xComp = ::comphelper::SynchronousDispatch::dispatch( xTargetFrame, aFileName, aTarget, 0, aArgs );
            }
            catch(const RuntimeException&)
            {
                throw;
            }
            catch(const ::com::sun::star::uno::Exception&)
            {
            }

            Reference < XModel > xModel( xComp, UNO_QUERY );
            if ( xModel.is() )
                xController = xModel->getCurrentController();
            else
                xController = Reference < XController >( xComp, UNO_QUERY );

        }
        else
        {
            URL aURL;
            aURL.Complete = aFileName;
            Reference < XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.URLTransformer"))), UNO_QUERY );
            xTrans->parseStrict( aURL );

            Reference < XDispatchProvider > xProv( xTargetFrame, UNO_QUERY );
            Reference < XDispatch > xDisp = xProv.is() ? xProv->queryDispatch( aURL, aTarget, FrameSearchFlag::ALL ) : Reference < XDispatch >();;
            RTL_LOGFILE_PRODUCT_CONTEXT( aLog2, "PERFORMANCE - SfxApplication::OpenDocExec_Impl" );
            if ( xDisp.is() )
                xDisp->dispatch( aURL, aArgs );
        }

    if ( xController.is() )
    {
        // try to find the SfxFrame for the controller
        SfxFrame* pCntrFrame = NULL;
        for ( SfxViewShell* pShell = SfxViewShell::GetFirst( 0, FALSE ); pShell; pShell = SfxViewShell::GetNext( *pShell, 0, FALSE ) )
        {
            if ( pShell->GetController() == xController )
            {
                pCntrFrame = &pShell->GetViewFrame()->GetFrame();
                break;
            }
        }

        if ( pCntrFrame )
        {
            SfxObjectShell* pSh = pCntrFrame->GetCurrentDocument();
            DBG_ASSERT( pSh, "Controller without ObjectShell ?!" );

            rReq.SetReturnValue( SfxViewFrameItem( 0, pCntrFrame->GetCurrentViewFrame() ) );

            if ( bHidden )
                pSh->RestoreNoDelete();
        }
    }

    if ( pLinkItem )
    {
        SfxPoolItem* pRet = rReq.GetReturnValue()->Clone();
        pLinkItem->GetValue().Call(pRet);
        delete pLinkItem;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
