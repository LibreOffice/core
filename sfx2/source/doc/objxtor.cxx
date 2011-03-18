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

#include "arrdecl.hxx"

#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#include <com/sun/star/util/XCloseListener.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XTitle.hpp>
#include <osl/mutex.hxx>

#include <tools/resary.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/svapp.hxx>
#include <svl/eitem.hxx>
#include <tools/rtti.hxx>
#include <svl/lstner.hxx>
#include <sfx2/sfxhelp.hxx>
#include <basic/sbstar.hxx>
#include <svl/stritem.hxx>
#include <basic/sbx.hxx>
#include <unotools/eventcfg.hxx>

#include <sfx2/objsh.hxx>
#include <sfx2/signaturestate.hxx>
#include <sfx2/sfxmodelfactory.hxx>

#include <basic/sbuno.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>
#include <unotools/printwarningoptions.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/script/DocumentDialogLibraryContainer.hpp>
#include <com/sun/star/script/DocumentScriptLibraryContainer.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <com/sun/star/document/XScriptInvocationContext.hpp>

#include <svl/urihelper.hxx>
#include <unotools/pathoptions.hxx>
#include <svl/sharecontrolfile.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbhelper.hxx>
#include <svtools/asynclink.hxx>
#include <tools/diagnose_ex.h>
#include <sot/clsids.hxx>

#include <sfx2/app.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/event.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>
#include "sfx2/sfxresid.hxx"
#include "objshimp.hxx"
#include "appbas.hxx"
#include "sfxtypes.hxx"
#include <sfx2/evntconf.hxx>
#include <sfx2/request.hxx>
#include "doc.hrc"
#include "sfxlocal.hrc"
#include "appdata.hxx"
#include <sfx2/appuno.hxx>
#include <sfx2/sfxsids.hrc>
#include "sfx2/basmgr.hxx"
#include "sfx2/QuerySaveDocument.hxx"
#include "helpid.hrc"
#include <sfx2/msg.hxx>
#include "appbaslib.hxx"
#include <sfx2/sfxbasemodel.hxx>

#include <basic/basicmanagerrepository.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::document;

using ::basic::BasicManagerRepository;
#include <uno/mapping.hxx>

//====================================================================

DBG_NAME(SfxObjectShell)

#define DocumentInfo
#include "sfxslots.hxx"

static WeakReference< XInterface > s_xCurrentComponent;

void lcl_UpdateAppBasicDocVars(  const Reference< XInterface >& _rxComponent, bool bClear = false )
{
    BasicManager* pAppMgr = SFX_APP()->GetBasicManager();
    if ( pAppMgr )
    {
        uno::Reference< beans::XPropertySet > xProps( _rxComponent, uno::UNO_QUERY );
        if ( xProps.is() )
        {
            try
            {
                // ThisVBADocObj contains a PropertyValue
                // Name  is ( the name of the VBA global to insert )
                // Value is the Object to insert.
                // ( note: at the moment the Value is actually the model so
                // it strictly is not necessary, however we do intend to store
                // not the model in basic but a custom object, so we keep this
                // level of indirection for future proofing )
                beans::PropertyValue aProp;
                xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ThisVBADocObj") ) ) >>= aProp;
                rtl::OString sTmp( rtl::OUStringToOString( aProp.Name, RTL_TEXTENCODING_UTF8 ) );
                const char* pAscii = sTmp.getStr();
                if ( bClear )
                    pAppMgr->SetGlobalUNOConstant( pAscii, uno::makeAny( uno::Reference< uno::XInterface >() ) );
                else
                    pAppMgr->SetGlobalUNOConstant( pAscii, aProp.Value );

            }
            catch( const uno::Exception& )
            {
            }
        }
    }
}

//=========================================================================

class SfxModelListener_Impl : public ::cppu::WeakImplHelper1< ::com::sun::star::util::XCloseListener >
{
    SfxObjectShell* mpDoc;
public:
    SfxModelListener_Impl( SfxObjectShell* pDoc ) : mpDoc(pDoc) {};
    virtual void SAL_CALL queryClosing( const com::sun::star::lang::EventObject& aEvent, sal_Bool bDeliverOwnership )
        throw ( com::sun::star::uno::RuntimeException, com::sun::star::util::CloseVetoException) ;
    virtual void SAL_CALL notifyClosing( const com::sun::star::lang::EventObject& aEvent ) throw ( com::sun::star::uno::RuntimeException ) ;
    virtual void SAL_CALL disposing( const com::sun::star::lang::EventObject& aEvent ) throw ( com::sun::star::uno::RuntimeException ) ;

};

void SAL_CALL SfxModelListener_Impl::queryClosing( const com::sun::star::lang::EventObject& , sal_Bool )
    throw ( com::sun::star::uno::RuntimeException, com::sun::star::util::CloseVetoException)
{
}

void SAL_CALL SfxModelListener_Impl::notifyClosing( const com::sun::star::lang::EventObject& ) throw ( com::sun::star::uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    mpDoc->Broadcast( SfxSimpleHint(SFX_HINT_DEINITIALIZING) );
}

void SAL_CALL SfxModelListener_Impl::disposing( const com::sun::star::lang::EventObject& _rEvent ) throw ( com::sun::star::uno::RuntimeException )
{
    // am I ThisComponent in AppBasic?
    SolarMutexGuard aSolarGuard;
    if ( SfxObjectShell::GetCurrentComponent() == _rEvent.Source )
    {
        lcl_UpdateAppBasicDocVars( SfxObjectShell::GetCurrentComponent(), true );
        // remove ThisComponent reference from AppBasic
        SfxObjectShell::SetCurrentComponent( Reference< XInterface >() );
    }

    if ( mpDoc->Get_Impl()->bHiddenLockedByAPI )
    {
        mpDoc->Get_Impl()->bHiddenLockedByAPI = sal_False;
        mpDoc->OwnerLock(sal_False);
    }
    else if ( !mpDoc->Get_Impl()->bClosing )
        // GCC stuerzt ab, wenn schon im dtor, also vorher Flag abfragen
        mpDoc->DoClose();
}

TYPEINIT1(SfxObjectShell, SfxShell);

//--------------------------------------------------------------------
SfxObjectShell_Impl::SfxObjectShell_Impl( SfxObjectShell& _rDocShell )
    :mpObjectContainer(0)
    ,pBasicManager( new SfxBasicManagerHolder )
    ,rDocShell( _rDocShell )
    ,aMacroMode( *this )
    ,pProgress( 0)
    ,nTime()
    ,nVisualDocumentNumber( USHRT_MAX)
    ,nDocumentSignatureState( SIGNATURESTATE_UNKNOWN )
    ,nScriptingSignatureState( SIGNATURESTATE_UNKNOWN )
    ,bInList( sal_False)
    ,bClosing( sal_False)
    ,bIsSaving( sal_False)
    ,bPasswd( sal_False)
    ,bIsTmp( sal_False)
    ,bIsNamedVisible( sal_False)
    ,bIsTemplate(sal_False)
    ,bIsAbortingImport ( sal_False)
    ,bImportDone ( sal_False)
    ,bInPrepareClose( sal_False )
    ,bPreparedForClose( sal_False )
    ,bWaitingForPicklist( sal_True )
    ,bForbidReload( sal_False )
    ,bBasicInitialized( sal_False )
    ,bIsPrintJobCancelable( sal_True )
    ,bOwnsStorage( sal_True )
    ,bNoBaseURL( sal_False )
    ,bInitialized( sal_False )
    ,bSignatureErrorIsShown( sal_False )
    ,bModelInitialized( sal_False )
    ,bPreserveVersions( sal_True )
    ,m_bMacroSignBroken( sal_False )
    ,m_bNoBasicCapabilities( sal_False )
    ,m_bDocRecoverySupport( sal_True )
    ,bQueryLoadTemplate( sal_True )
    ,bLoadReadonly( sal_False )
    ,bUseUserData( sal_True )
    ,bSaveVersionOnClose( sal_False )
    ,m_bSharedXMLFlag( sal_False )
    ,m_bAllowShareControlFileClean( sal_True )
    ,m_bConfigOptionsChecked( sal_False )
    ,lErr(ERRCODE_NONE)
    ,nEventId ( 0)
    ,pReloadTimer ( 0)
    ,pMarkData( 0 )
    ,nLoadedFlags ( SFX_LOADED_ALL )
    ,nFlagsInProgress( 0 )
    ,bModalMode( sal_False )
    ,bRunningMacro( sal_False )
    ,bReloadAvailable( sal_False )
    ,nAutoLoadLocks( 0 )
    ,pModule( 0 )
    ,eFlags( SFXOBJECTSHELL_UNDEFINED )
    ,bReadOnlyUI( sal_False )
    ,bHiddenLockedByAPI( sal_False )
    ,nStyleFilter( 0 )
    ,bDisposing( sal_False )
    ,m_bEnableSetModified( sal_True )
    ,m_bIsModified( sal_False )
    ,m_nMapUnit( MAP_100TH_MM )
    ,m_bCreateTempStor( sal_False )
    ,m_bIsInit( sal_False )
    ,m_bIncomplEncrWarnShown( sal_False )
    ,m_nModifyPasswordHash( 0 )
    ,m_bModifyPasswordEntered( sal_False )
{
    SfxObjectShell* pDoc = &_rDocShell;
    SfxObjectShellArr_Impl &rArr = SFX_APP()->GetObjectShells_Impl();
    rArr.C40_INSERT( SfxObjectShell, pDoc, rArr.Count() );
    bInList = sal_True;
}

//--------------------------------------------------------------------

SfxObjectShell_Impl::~SfxObjectShell_Impl()
{
    delete pBasicManager;
}

//--------------------------------------------------------------------

SfxObjectShell::SfxObjectShell( const sal_uInt64 i_nCreationFlags )
    :   pImp( new SfxObjectShell_Impl( *this ) )
    ,   pMedium(0)
    ,   pStyleSheetPool(0)
    ,   eCreateMode( ( i_nCreationFlags & SFXMODEL_EMBEDDED_OBJECT ) ? SFX_CREATE_MODE_EMBEDDED : SFX_CREATE_MODE_STANDARD )
    ,   bHasName( sal_False )
{
    DBG_CTOR(SfxObjectShell, 0);

    const bool bScriptSupport = ( i_nCreationFlags & SFXMODEL_DISABLE_EMBEDDED_SCRIPTS ) == 0;
    if ( !bScriptSupport )
        SetHasNoBasic();

    const bool bDocRecovery = ( i_nCreationFlags & SFXMODEL_DISABLE_DOCUMENT_RECOVERY ) == 0;
    if ( !bDocRecovery )
        pImp->m_bDocRecoverySupport = sal_False;
}

//--------------------------------------------------------------------

// initializes a document from a file-description

SfxObjectShell::SfxObjectShell
(
    SfxObjectCreateMode eMode   /*  Purpose, io which the SfxObjectShell
                                    is created:

                                    SFX_CREATE_MODE_EMBEDDED (default)
                                        as SO-Server from within another
                                        Document

                                    SFX_CREATE_MODE_STANDARD,
                                        as a normal Document open stand-alone

                                    SFX_CREATE_MODE_PREVIEW
                                        to enable a Preview, if possible are
                                        only little information is needed

                                    SFX_CREATE_MODE_ORGANIZER
                                        to be displayed in the Organizer, here
                                        notning of the contents is used  */
)

/*  [Description]

    Constructor of the class SfxObjectShell.
*/

:   pImp( new SfxObjectShell_Impl( *this ) ),
    pMedium(0),
    pStyleSheetPool(0),
    eCreateMode(eMode),
    bHasName( sal_False )
{
    DBG_CTOR(SfxObjectShell, 0);
}

//--------------------------------------------------------------------

// virtual destructor of typical base-class SfxObjectShell

SfxObjectShell::~SfxObjectShell()
{
    DBG_DTOR(SfxObjectShell, 0);

    if ( IsEnableSetModified() )
        EnableSetModified( sal_False );

    // Never call GetInPlaceObject(), the access to the derivative branch
    // SfxInternObject is not allowed because of a compiler bug
    SfxObjectShell::Close();
    pImp->pBaseModel.set( NULL );

    DELETEX(pImp->pReloadTimer );

    SfxApplication *pSfxApp = SFX_APP();
    if ( USHRT_MAX != pImp->nVisualDocumentNumber )
        pSfxApp->ReleaseIndex(pImp->nVisualDocumentNumber);

    // Destroy Basic-Manager
    pImp->pBasicManager->reset( NULL );

    if ( pSfxApp->GetDdeService() )
        pSfxApp->RemoveDdeTopic( this );

    pImp->pBaseModel.set( NULL );

    // don't call GetStorage() here, in case of Load Failure it's possible that a storage was never assigned!
    if ( pMedium && pMedium->HasStorage_Impl() && pMedium->GetStorage( sal_False ) == pImp->m_xDocStorage )
        pMedium->CanDisposeStorage_Impl( sal_False );

    if ( pImp->mpObjectContainer )
    {
        pImp->mpObjectContainer->CloseEmbeddedObjects();
        delete pImp->mpObjectContainer;
    }

    if ( pImp->bOwnsStorage && pImp->m_xDocStorage.is() )
        pImp->m_xDocStorage->dispose();

    if ( pMedium )
    {
        pMedium->CloseAndReleaseStreams_Impl();

        if ( IsDocShared() )
            FreeSharedFile();

        DELETEX( pMedium );
    }

    // The removing of the temporary file must be done as the latest step in the document destruction
    if ( pImp->aTempName.Len() )
    {
        String aTmp;
        ::utl::LocalFileHelper::ConvertPhysicalNameToURL( pImp->aTempName, aTmp );
        ::utl::UCBContentHelper::Kill( aTmp );
    }

    delete pImp;
}

//--------------------------------------------------------------------

void SfxObjectShell::Stamp_SetPrintCancelState(sal_Bool bState)
{
    pImp->bIsPrintJobCancelable = bState;
}

//--------------------------------------------------------------------

sal_Bool SfxObjectShell::Stamp_GetPrintCancelState() const
{
    return pImp->bIsPrintJobCancelable;
}

//--------------------------------------------------------------------

void SfxObjectShell::ViewAssigned()

/*  [Description]

    This method is called when a view is assigned.
*/

{
}

//--------------------------------------------------------------------
// closes the Object and all its views

sal_Bool SfxObjectShell::Close()
{
    {DBG_CHKTHIS(SfxObjectShell, 0);}
    SfxObjectShellRef aRef(this);
    if ( !pImp->bClosing )
    {
        // Do not close if a progress is still running
        if ( !pImp->bDisposing && GetProgress() )
            return sal_False;

        pImp->bClosing = sal_True;
        Reference< util::XCloseable > xCloseable( GetBaseModel(), UNO_QUERY );

        if ( xCloseable.is() )
        {
            try
            {
                xCloseable->close( sal_True );
            }
            catch( Exception& )
            {
                pImp->bClosing = sal_False;
            }
        }

        if ( pImp->bClosing )
        {
            // remove from Document list
            SfxApplication *pSfxApp = SFX_APP();
            SfxObjectShellArr_Impl &rDocs = pSfxApp->GetObjectShells_Impl();
            const SfxObjectShell *pThis = this;
            sal_uInt16 nPos = rDocs.GetPos(pThis);
            if ( nPos < rDocs.Count() )
                rDocs.Remove( nPos );
            pImp->bInList = sal_False;
        }
    }

    return sal_True;
}

//--------------------------------------------------------------------

// returns a pointer the first SfxDocument of specified type

SfxObjectShell* SfxObjectShell::GetFirst
(
    const TypeId* pType ,
    sal_Bool            bOnlyVisible
)
{
    SfxObjectShellArr_Impl &rDocs = SFX_APP()->GetObjectShells_Impl();

    // seach for a SfxDocument of the specified type
    for ( sal_uInt16 nPos = 0; nPos < rDocs.Count(); ++nPos )
    {
        SfxObjectShell* pSh = rDocs.GetObject( nPos );
        if ( bOnlyVisible && pSh->IsPreview() && pSh->IsReadOnly() )
            continue;

        if ( ( !pType || pSh->IsA(*pType) ) &&
             ( !bOnlyVisible || SfxViewFrame::GetFirst( pSh, sal_True )))
            return pSh;
    }

    return 0;
}
//--------------------------------------------------------------------

// returns a pointer to the next SfxDocument of specified type behind *pDoc

SfxObjectShell* SfxObjectShell::GetNext
(
    const SfxObjectShell&   rPrev,
    const TypeId*           pType,
    sal_Bool                    bOnlyVisible
)
{
    SfxObjectShellArr_Impl &rDocs = SFX_APP()->GetObjectShells_Impl();

    // refind the specified predecessor
    sal_uInt16 nPos;
    for ( nPos = 0; nPos < rDocs.Count(); ++nPos )
        if ( rDocs.GetObject(nPos) == &rPrev )
            break;

    // search for the next SfxDocument of the specified type
    for ( ++nPos; nPos < rDocs.Count(); ++nPos )
    {
        SfxObjectShell* pSh = rDocs.GetObject( nPos );
        if ( bOnlyVisible && pSh->IsPreview() && pSh->IsReadOnly() )
            continue;

        if ( ( !pType || pSh->IsA(*pType) ) &&
             ( !bOnlyVisible || SfxViewFrame::GetFirst( pSh, sal_True )))
            return pSh;
    }
    return 0;
}

//--------------------------------------------------------------------

SfxObjectShell* SfxObjectShell::Current()
{
    SfxViewFrame *pFrame = SfxViewFrame::Current();
    return pFrame ? pFrame->GetObjectShell() : 0;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::IsInPrepareClose() const
{
    return pImp->bInPrepareClose;
}

//------------------------------------------------------------------------

struct BoolEnv_Impl
{
    SfxObjectShell_Impl* pImp;
    BoolEnv_Impl( SfxObjectShell_Impl* pImpP) : pImp( pImpP )
    { pImpP->bInPrepareClose = sal_True; }
    ~BoolEnv_Impl() { pImp->bInPrepareClose = sal_False; }
};


sal_uInt16 SfxObjectShell::PrepareClose
(
    sal_Bool    bUI,  // sal_True: Dialog and so on is allowed
                      // sal_False: silent-mode
    sal_Bool    bForBrowsing
)
{
    if( pImp->bInPrepareClose || pImp->bPreparedForClose )
        return sal_True;
    BoolEnv_Impl aBoolEnv( pImp );

    // DocModalDialog?
    if ( IsInModalMode() )
        return sal_False;

    SfxViewFrame* pFirst = SfxViewFrame::GetFirst( this );
    if( pFirst && !pFirst->GetFrame().PrepareClose_Impl( bUI, bForBrowsing ) )
        return sal_False;

    // prepare views for closing
    for ( SfxViewFrame* pFrm = SfxViewFrame::GetFirst( this );
          pFrm; pFrm = SfxViewFrame::GetNext( *pFrm, this ) )
    {
        DBG_ASSERT(pFrm->GetViewShell(),"No Shell");
        if ( pFrm->GetViewShell() )
        {
            sal_uInt16 nRet = pFrm->GetViewShell()->PrepareClose( bUI, bForBrowsing );
            if ( nRet != sal_True )
                return nRet;
        }
    }

    SfxApplication *pSfxApp = SFX_APP();
    pSfxApp->NotifyEvent( SfxEventHint(SFX_EVENT_PREPARECLOSEDOC, GlobalEventConfig::GetEventName(STR_EVENT_PREPARECLOSEDOC), this) );

    if( GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
    {
        pImp->bPreparedForClose = sal_True;
        return sal_True;
    }

    // Ask if possible if it should be saved
    // only ask for the Document in the visable window
    SfxViewFrame *pFrame = SfxObjectShell::Current() == this
        ? SfxViewFrame::Current() : SfxViewFrame::GetFirst( this );

    if ( bUI && IsModified() && pFrame )
    {
        // restore minimized
        SfxFrame& rTop = pFrame->GetTopFrame();
        SfxViewFrame::SetViewFrame( rTop.GetCurrentViewFrame() );
        pFrame->GetFrame().Appear();

        // Ask if to save
        short nRet = RET_YES;
        {
            //initiate help agent to inform about "print modifies the document"
            SvtPrintWarningOptions aPrintOptions;
            if (aPrintOptions.IsModifyDocumentOnPrintingAllowed() &&
                HasName() && getDocProperties()->getPrintDate().Month > 0)
            {
                SfxHelp::OpenHelpAgent( &pFirst->GetFrame(), HID_CLOSE_WARNING );
            }
            const Reference< XTitle > xTitle( *pImp->pBaseModel.get(), UNO_QUERY_THROW );
            const ::rtl::OUString     sTitle = xTitle->getTitle ();
            nRet = ExecuteQuerySaveDocument(&pFrame->GetWindow(),sTitle);
        }
        /*HACK for plugin::destroy()*/

        if ( RET_YES == nRet )
        {
            // Save by each Dispatcher
            const SfxPoolItem *pPoolItem;
            if ( IsSaveVersionOnClose() )
            {
                SfxStringItem aItem( SID_DOCINFO_COMMENTS, String( SfxResId( STR_AUTOMATICVERSION ) ) );
                SfxBoolItem aWarnItem( SID_FAIL_ON_WARNING, bUI );
                const SfxPoolItem* ppArgs[] = { &aItem, &aWarnItem, 0 };
                pPoolItem = pFrame->GetBindings().ExecuteSynchron( SID_SAVEDOC, ppArgs );
            }
            else
            {
                SfxBoolItem aWarnItem( SID_FAIL_ON_WARNING, bUI );
                const SfxPoolItem* ppArgs[] = { &aWarnItem, 0 };
                pPoolItem = pFrame->GetBindings().ExecuteSynchron( SID_SAVEDOC, ppArgs );
            }

            if ( !pPoolItem || pPoolItem->ISA(SfxVoidItem) || ( pPoolItem->ISA(SfxBoolItem) && !( (const SfxBoolItem*) pPoolItem )->GetValue() ) )
                return sal_False;
        }
        else if ( RET_CANCEL == nRet )
            // Cancelled
            return sal_False;
        else if ( RET_NEWTASK == nRet )
        {
            return RET_NEWTASK;
        }
    }

    pImp->bPreparedForClose = sal_True;
    return sal_True;
}

//--------------------------------------------------------------------
namespace
{
    static BasicManager* lcl_getBasicManagerForDocument( const SfxObjectShell& _rDocument )
    {
        if ( !_rDocument.Get_Impl()->m_bNoBasicCapabilities )
        {
            if ( !_rDocument.Get_Impl()->bBasicInitialized )
                const_cast< SfxObjectShell& >( _rDocument ).InitBasicManager_Impl();
            return _rDocument.Get_Impl()->pBasicManager->get();
        }

        // assume we do not have Basic ourself, but we can refer to another
        // document which does (by our model's XScriptInvocationContext::getScriptContainer).
        // In this case, we return the BasicManager of this other document.

        OSL_ENSURE( !Reference< XEmbeddedScripts >( _rDocument.GetModel(), UNO_QUERY ).is(),
            "lcl_getBasicManagerForDocument: inconsistency: no Basic, but an XEmbeddedScripts?" );
        Reference< XModel > xForeignDocument;
        Reference< XScriptInvocationContext > xContext( _rDocument.GetModel(), UNO_QUERY );
        if ( xContext.is() )
        {
            xForeignDocument.set( xContext->getScriptContainer(), UNO_QUERY );
            OSL_ENSURE( xForeignDocument.is() && xForeignDocument != _rDocument.GetModel(),
                "lcl_getBasicManagerForDocument: no Basic, but providing ourself as script container?" );
        }

        BasicManager* pBasMgr = NULL;
        if ( xForeignDocument.is() )
            pBasMgr = ::basic::BasicManagerRepository::getDocumentBasicManager( xForeignDocument );

        return pBasMgr;
    }
}

//--------------------------------------------------------------------

BasicManager* SfxObjectShell::GetBasicManager() const
{
    BasicManager* pBasMgr = lcl_getBasicManagerForDocument( *this );
    if ( !pBasMgr )
        pBasMgr = SFX_APP()->GetBasicManager();
    return pBasMgr;
}

//--------------------------------------------------------------------

void SfxObjectShell::SetHasNoBasic()
{
    pImp->m_bNoBasicCapabilities = sal_True;
}

//--------------------------------------------------------------------

sal_Bool SfxObjectShell::HasBasic() const
{
    if ( pImp->m_bNoBasicCapabilities )
        return sal_False;

    if ( !pImp->bBasicInitialized )
        const_cast< SfxObjectShell* >( this )->InitBasicManager_Impl();

    return pImp->pBasicManager->isValid();
}

//--------------------------------------------------------------------
namespace
{
    const Reference< XLibraryContainer >&
    lcl_getOrCreateLibraryContainer( bool _bScript, Reference< XLibraryContainer >& _rxContainer,
        const Reference< XModel >& _rxDocument )
    {
        if ( !_rxContainer.is() )
        {
            try
            {
                Reference< XStorageBasedDocument > xStorageDoc( _rxDocument, UNO_QUERY );
                const Reference< XComponentContext > xContext(
                    ::comphelper::getProcessComponentContext() );
                _rxContainer.set (   _bScript
                                ?   DocumentScriptLibraryContainer::create(
                                        xContext, xStorageDoc )
                                :   DocumentDialogLibraryContainer::create(
                                        xContext, xStorageDoc )
                                ,   UNO_QUERY_THROW );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
        return _rxContainer;
    }
}

//--------------------------------------------------------------------

Reference< XLibraryContainer > SfxObjectShell::GetDialogContainer()
{
    if ( !pImp->m_bNoBasicCapabilities )
        return lcl_getOrCreateLibraryContainer( false, pImp->xDialogLibraries, GetModel() );

    BasicManager* pBasMgr = lcl_getBasicManagerForDocument( *this );
    if ( pBasMgr )
        return pBasMgr->GetDialogLibraryContainer().get();

    OSL_FAIL( "SfxObjectShell::GetDialogContainer: falling back to the application - is this really expected here?" );
    return SFX_APP()->GetDialogContainer();
}

//--------------------------------------------------------------------

Reference< XLibraryContainer > SfxObjectShell::GetBasicContainer()
{
    if ( !pImp->m_bNoBasicCapabilities )
        return lcl_getOrCreateLibraryContainer( true, pImp->xBasicLibraries, GetModel() );

    BasicManager* pBasMgr = lcl_getBasicManagerForDocument( *this );
    if ( pBasMgr )
        return pBasMgr->GetScriptLibraryContainer().get();

    OSL_FAIL( "SfxObjectShell::GetBasicContainer: falling back to the application - is this really expected here?" );
    return SFX_APP()->GetBasicContainer();
}

//--------------------------------------------------------------------

StarBASIC* SfxObjectShell::GetBasic() const
{
    return GetBasicManager()->GetLib(0);
}

//--------------------------------------------------------------------

void SfxObjectShell::InitBasicManager_Impl()
/*  [Description]

    Creates a document's BasicManager and loads it, if we are already based on
    a storage.

    [Note]

    This method has to be called  through the overloading of
    <SvPersist::Load()> (With the PStore from the parameters of load ())
    and from the overloading of <SvPersist::InitNew()> (with PStore = 0).
*/

{
    /*  #163556# (DR) - Handling of recursive calls while creating the Bacic
        manager.

        It is possible that (while creating the Basic manager) the code that
        imports the Basic storage wants to access the Basic manager again.
        Especially in VBA compatibility mode, there is code that wants to
        access the "VBA Globals" object which is stored as global UNO constant
        in the Basic manager.

        To achieve correct handling of the recursive calls of this function
        from lcl_getBasicManagerForDocument(), the implementation of the
        function BasicManagerRepository::getDocumentBasicManager() has been
        changed to return the Basic manager currently under construction, when
        called repeatedly.

        The variable pImp->bBasicInitialized will be set to sal_True after
        construction now, to ensure that the recursive call of the function
        lcl_getBasicManagerForDocument() will be routed into this function too.

        Calling BasicManagerHolder::reset() twice is not a big problem, as it
        does not take ownership but stores only the raw pointer. Owner of all
        Basic managers is the global BasicManagerRepository instance.
     */
    DBG_ASSERT( !pImp->bBasicInitialized && !pImp->pBasicManager->isValid(), "Lokaler BasicManager bereits vorhanden");
    pImp->pBasicManager->reset( BasicManagerRepository::getDocumentBasicManager( GetModel() ) );
    DBG_ASSERT( pImp->pBasicManager->isValid(), "SfxObjectShell::InitBasicManager_Impl: did not get a BasicManager!" );
    pImp->bBasicInitialized = sal_True;
}

//--------------------------------------------------------------------

sal_uInt16 SfxObjectShell::Count()
{
    return SFX_APP()->GetObjectShells_Impl().Count();
}

//--------------------------------------------------------------------

sal_Bool SfxObjectShell::DoClose()
{
    return Close();
}

//--------------------------------------------------------------------

SfxObjectShell* SfxObjectShell::GetObjectShell()
{
    return this;
}

//--------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > SfxObjectShell::GetEventNames()
{
    static uno::Sequence< ::rtl::OUString >* pEventNameContainer = NULL;

    if ( !pEventNameContainer )
    {
        SolarMutexGuard aGuard;
        if ( !pEventNameContainer )
        {
            static uno::Sequence< ::rtl::OUString > aEventNameContainer = GlobalEventConfig().getElementNames();
            pEventNameContainer = &aEventNameContainer;
        }
    }

    return *pEventNameContainer;
}

//--------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > SfxObjectShell::GetModel() const
{
    return GetBaseModel();
}

void SfxObjectShell::SetBaseModel( SfxBaseModel* pModel )
{
    OSL_ENSURE( !pImp->pBaseModel.is() || pModel == NULL, "Model already set!" );
    pImp->pBaseModel.set( pModel );
    if ( pImp->pBaseModel.is() )
    {
        pImp->pBaseModel->addCloseListener( new SfxModelListener_Impl(this) );
    }
}

//--------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > SfxObjectShell::GetBaseModel() const
{
    return pImp->pBaseModel.get();
}

void SfxObjectShell::SetAutoStyleFilterIndex(sal_uInt16 nSet)
{
    pImp->nStyleFilter = nSet;
}

sal_uInt16 SfxObjectShell::GetAutoStyleFilterIndex()
{
    return pImp->nStyleFilter;
}


void SfxObjectShell::SetCurrentComponent( const Reference< XInterface >& _rxComponent )
{
    Reference< XInterface > xTest(s_xCurrentComponent);
    if ( _rxComponent == xTest )
        // nothing to do
        return;
    // note that "_rxComponent.get() == s_xCurrentComponent.get().get()" is /sufficient/, but not
    // /required/ for "_rxComponent == s_xCurrentComponent.get()".
    // In other words, it's still possible that we here do something which is not necessary,
    // but we should have filtered quite some unnecessary calls already.

    BasicManager* pAppMgr = SFX_APP()->GetBasicManager();
    s_xCurrentComponent = _rxComponent;
    if ( pAppMgr )
    {
        lcl_UpdateAppBasicDocVars( _rxComponent );
        pAppMgr->SetGlobalUNOConstant( "ThisComponent", makeAny( _rxComponent ) );
    }

#if OSL_DEBUG_LEVEL > 0
    const char* pComponentImplName = _rxComponent.get() ? typeid( *_rxComponent.get() ).name() : "void";
    OSL_TRACE( "current component is a %s\n", pComponentImplName );
#endif
}

Reference< XInterface > SfxObjectShell::GetCurrentComponent()
{
    return s_xCurrentComponent;
}


String SfxObjectShell::GetServiceNameFromFactory( const String& rFact )
{
    //! Remove everything behind name!
    String aFact( rFact );
    String aPrefix = String::CreateFromAscii( "private:factory/" );
    if ( aPrefix.Len() == aFact.Match( aPrefix ) )
        aFact.Erase( 0, aPrefix.Len() );
    sal_uInt16 nPos = aFact.Search( '?' );
    String aParam;
    if ( nPos != STRING_NOTFOUND )
    {
        aParam = aFact.Copy( nPos, aFact.Len() );
        aFact.Erase( nPos, aFact.Len() );
        aParam.Erase(0,1);
    }
    aFact.EraseAllChars('4').ToLowerAscii();

    // HACK: sometimes a real document service name is given here instead of
    // a factory short name. Set return value directly to this service name as fallback
    // in case next lines of code does nothing ...
    // use rFact instead of normed aFact value !
    ::rtl::OUString aServiceName = rFact;

    if ( aFact.EqualsAscii("swriter") )
    {
        aServiceName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextDocument"));
    }
    else if ( aFact.EqualsAscii("sweb") || aFact.EqualsAscii("swriter/web") )
    {
        aServiceName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.WebDocument"));
    }
    else if ( aFact.EqualsAscii("sglobal") || aFact.EqualsAscii("swriter/globaldocument") )
    {
        aServiceName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.GlobalDocument"));
    }
    else if ( aFact.EqualsAscii("scalc") )
    {
        aServiceName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.SpreadsheetDocument"));
    }
    else if ( aFact.EqualsAscii("sdraw") )
    {
        aServiceName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.DrawingDocument"));
    }
    else if ( aFact.EqualsAscii("simpress") )
    {
        aServiceName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.PresentationDocument"));
    }
    else if ( aFact.EqualsAscii("schart") )
    {
        aServiceName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.chart.ChartDocument"));
    }
    else if ( aFact.EqualsAscii("smath") )
    {
        aServiceName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.formula.FormulaProperties"));
    }
    else if ( aFact.EqualsAscii("sbasic") )
    {
        aServiceName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.script.BasicIDE"));
    }
    else if ( aFact.EqualsAscii("sdatabase") )
    {
        aServiceName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.OfficeDatabaseDocument"));
    }

    return aServiceName;
}

SfxObjectShell* SfxObjectShell::CreateObjectByFactoryName( const String& rFact, SfxObjectCreateMode eMode )
{
    return CreateObject( GetServiceNameFromFactory( rFact ), eMode );
}


SfxObjectShell* SfxObjectShell::CreateObject( const String& rServiceName, SfxObjectCreateMode eCreateMode )
{
    if ( rServiceName.Len() )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::frame::XModel > xDoc(
        ::comphelper::getProcessServiceFactory()->createInstance( rServiceName ), UNO_QUERY );
        if ( xDoc.is() )
        {
            ::com::sun::star::uno::Reference < ::com::sun::star::lang::XUnoTunnel > xObj( xDoc, UNO_QUERY );
            ::com::sun::star::uno::Sequence < sal_Int8 > aSeq( SvGlobalName( SFX_GLOBAL_CLASSID ).GetByteSequence() );
            sal_Int64 nHandle = xObj->getSomething( aSeq );
            if ( nHandle )
            {
                SfxObjectShell* pRet = reinterpret_cast< SfxObjectShell* >( sal::static_int_cast< sal_IntPtr >( nHandle ));
                pRet->SetCreateMode_Impl( eCreateMode );
                return pRet;
            }
        }
    }

    return 0;
}

SfxObjectShell* SfxObjectShell::CreateAndLoadObject( const SfxItemSet& rSet, SfxFrame* pFrame )
{
    Reference<lang::XComponent> xComp = CreateAndLoadComponent(rSet, pFrame);
    return GetShellFromComponent(xComp);
}

Reference<lang::XComponent> SfxObjectShell::CreateAndLoadComponent( const SfxItemSet& rSet, SfxFrame* pFrame )
{
    uno::Sequence < beans::PropertyValue > aProps;
    TransformItems( SID_OPENDOC, rSet, aProps );
    SFX_ITEMSET_ARG(&rSet, pFileNameItem, SfxStringItem, SID_FILE_NAME, sal_False);
    SFX_ITEMSET_ARG(&rSet, pTargetItem, SfxStringItem, SID_TARGETNAME, sal_False);
    ::rtl::OUString aURL;
    ::rtl::OUString aTarget(RTL_CONSTASCII_USTRINGPARAM("_blank"));
    if ( pFileNameItem )
        aURL = pFileNameItem->GetValue();
    if ( pTargetItem )
        aTarget = pTargetItem->GetValue();

    uno::Reference < frame::XComponentLoader > xLoader;
    if ( pFrame )
    {
        xLoader = uno::Reference < frame::XComponentLoader >( pFrame->GetFrameInterface(), uno::UNO_QUERY );
    }
    else
        xLoader = uno::Reference < frame::XComponentLoader >( comphelper::getProcessServiceFactory()->createInstance(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) ), uno::UNO_QUERY );

    Reference <lang::XComponent> xComp;
    try
    {
        xComp = xLoader->loadComponentFromURL(aURL, aTarget, 0, aProps);
    }
    catch( uno::Exception& )
    {}

    return xComp;
}

SfxObjectShell* SfxObjectShell::GetShellFromComponent( const Reference<lang::XComponent>& xComp )
{
    try
    {
        Reference<lang::XUnoTunnel> xTunnel(xComp, UNO_QUERY_THROW);
        Sequence <sal_Int8> aSeq( SvGlobalName( SFX_GLOBAL_CLASSID ).GetByteSequence() );
        sal_Int64 nHandle = xTunnel->getSomething( aSeq );
        if (!nHandle)
            return NULL;

        return reinterpret_cast< SfxObjectShell* >(sal::static_int_cast< sal_IntPtr >(  nHandle ));
    }
    catch (const Exception&)
    {
    }

    return NULL;
}

void SfxObjectShell::SetInitialized_Impl( const bool i_fromInitNew )
{
    pImp->bInitialized = sal_True;
    if ( i_fromInitNew )
    {
        SetActivateEvent_Impl( SFX_EVENT_CREATEDOC );
        SFX_APP()->NotifyEvent( SfxEventHint( SFX_EVENT_DOCCREATED, GlobalEventConfig::GetEventName(STR_EVENT_DOCCREATED), this ) );
    }
    else
    {
        SFX_APP()->NotifyEvent( SfxEventHint( SFX_EVENT_LOADFINISHED, GlobalEventConfig::GetEventName(STR_EVENT_LOADFINISHED), this ) );
    }
}


bool SfxObjectShell::IsChangeRecording() const
{
    // currently this function needs to be overwritten by Writer and Calc only
    DBG_ASSERT( 0, "function not implemented" );
    return false;
}


bool SfxObjectShell::HasChangeRecordProtection() const
{
    // currently this function needs to be overwritten by Writer and Calc only
    DBG_ASSERT( 0, "function not implemented" );
    return false;
}


void SfxObjectShell::SetChangeRecording( bool /*bActivate*/ )
{
    // currently this function needs to be overwritten by Writer and Calc only
    DBG_ASSERT( 0, "function not implemented" );
}


bool SfxObjectShell::SetProtectionPassword( const String & /*rPassword*/ )
{
    // currently this function needs to be overwritten by Writer and Calc only
    DBG_ASSERT( 0, "function not implemented" );
    return false;
}


bool SfxObjectShell::GetProtectionHash( /*out*/ ::com::sun::star::uno::Sequence< sal_Int8 > & /*rPasswordHash*/ )
{
    // currently this function needs to be overwritten by Writer and Calc only
    DBG_ASSERT( 0, "function not implemented" );
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
