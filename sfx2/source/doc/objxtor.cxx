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

#include <config_features.h>

#include "arrdecl.hxx"
#include <map>

#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#include <com/sun/star/util/XCloseListener.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XTitle.hpp>
#include <osl/mutex.hxx>
#include <rtl/instance.hxx>
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
#include <comphelper/string.hxx>

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
#include <comphelper/classids.hxx>

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
#include <basic/basmgr.hxx>
#include <svtools/svtools.hrc>
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

#include "sfxslots.hxx"

namespace {

class theCurrentComponent : public rtl::Static< WeakReference< XInterface >, theCurrentComponent > {};

#ifndef DISABLE_SCRIPTING

// remember all registered components for VBA compatibility, to be able to remove them on disposing the model
typedef ::std::map< XInterface*, OString > VBAConstantNameMap;
static VBAConstantNameMap s_aRegisteredVBAConstants;

OString lclGetVBAGlobalConstName( const Reference< XInterface >& rxComponent )
{
    OSL_ENSURE( rxComponent.is(), "lclGetVBAGlobalConstName - missing component" );

    VBAConstantNameMap::iterator aIt = s_aRegisteredVBAConstants.find( rxComponent.get() );
    if( aIt != s_aRegisteredVBAConstants.end() )
        return aIt->second;

    uno::Reference< beans::XPropertySet > xProps( rxComponent, uno::UNO_QUERY );
    if( xProps.is() ) try
    {
        OUString aConstName;
        xProps->getPropertyValue("ThisVBADocObj") >>= aConstName;
        return OUStringToOString( aConstName, RTL_TEXTENCODING_ASCII_US );
    }
    catch (const uno::Exception&) // not supported
    {
    }
    return OString();
}

#endif

} // namespace

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
        // remove ThisComponent reference from AppBasic
        SfxObjectShell::SetCurrentComponent( Reference< XInterface >() );
    }

#ifndef DISABLE_SCRIPTING
    /*  Remove VBA component from AppBasic. As every application registers its
        own current component, the disposed component may not be the "current
        component" of the SfxObjectShell. */
    if ( _rEvent.Source.is() )
    {
        VBAConstantNameMap::iterator aIt = s_aRegisteredVBAConstants.find( _rEvent.Source.get() );
        if ( aIt != s_aRegisteredVBAConstants.end() )
        {
            if ( BasicManager* pAppMgr = SFX_APP()->GetBasicManager() )
                pAppMgr->SetGlobalUNOConstant( aIt->second.getStr(), Any( Reference< XInterface >() ) );
            s_aRegisteredVBAConstants.erase( aIt );
        }
    }
#endif

    if ( !mpDoc->Get_Impl()->bClosing )
        // GCC crashes when already in the destructor, so first query the Flag
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
    ,nTime( DateTime::EMPTY )
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
    rArr.push_back( pDoc );
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
    ,   bIsInGenerateThumbnail ( sal_False )
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
    bHasName( sal_False ),
    bIsInGenerateThumbnail ( sal_False )
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

    DELETEX(AutoReloadTimer_Impl, pImp->pReloadTimer );

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

#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
        if ( IsDocShared() )
            FreeSharedFile();
#endif
        DELETEX( SfxMedium, pMedium );
    }

    // The removing of the temporary file must be done as the latest step in the document destruction
    if ( !pImp->aTempName.isEmpty() )
    {
        OUString aTmp;
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

bool SfxObjectShell::Close()
{
    {DBG_CHKTHIS(SfxObjectShell, 0);}
    SfxObjectShellRef aRef(this);
    if ( !pImp->bClosing )
    {
        // Do not close if a progress is still running
        if ( !pImp->bDisposing && GetProgress() )
            return false;

        pImp->bClosing = sal_True;
        Reference< util::XCloseable > xCloseable( GetBaseModel(), UNO_QUERY );

        if ( xCloseable.is() )
        {
            try
            {
                xCloseable->close( sal_True );
            }
            catch (const Exception&)
            {
                pImp->bClosing = sal_False;
            }
        }

        if ( pImp->bClosing )
        {
            // remove from Document list
            SfxApplication *pSfxApp = SFX_APP();
            SfxObjectShellArr_Impl &rDocs = pSfxApp->GetObjectShells_Impl();
            SfxObjectShellArr_Impl::iterator it = std::find( rDocs.begin(), rDocs.end(), this );
            if ( it != rDocs.end() )
                rDocs.erase( it );
            pImp->bInList = sal_False;
        }
    }

    return true;
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
    for ( sal_uInt16 nPos = 0; nPos < rDocs.size(); ++nPos )
    {
        SfxObjectShell* pSh = rDocs[ nPos ];
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
    for ( nPos = 0; nPos < rDocs.size(); ++nPos )
        if ( rDocs[nPos] == &rPrev )
            break;

    // search for the next SfxDocument of the specified type
    for ( ++nPos; nPos < rDocs.size(); ++nPos )
    {
        SfxObjectShell* pSh = rDocs[ nPos ];
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
            const Reference< XTitle > xTitle( *pImp->pBaseModel.get(), UNO_QUERY_THROW );
            const OUString     sTitle = xTitle->getTitle ();
            nRet = ExecuteQuerySaveDocument(&pFrame->GetWindow(),sTitle);
        }
        /*HACK for plugin::destroy()*/

        if ( RET_YES == nRet )
        {
            // Save by each Dispatcher
            const SfxPoolItem *pPoolItem;
            if ( IsSaveVersionOnClose() )
            {
                SfxStringItem aItem( SID_DOCINFO_COMMENTS, SfxResId(STR_AUTOMATICVERSION).toString() );
                SfxBoolItem aWarnItem( SID_FAIL_ON_WARNING, bUI );
                const SfxPoolItem* ppArgs[] = { &aItem, &aWarnItem, 0 };
                pPoolItem = pFrame->GetBindings().ExecuteSynchron( SID_SAVEDOC, ppArgs );
            }
            else
            {
                SfxBoolItem aWarnItem( SID_FAIL_ON_WARNING, bUI );
                const SfxPoolItem* ppArgs[] = { &aWarnItem, 0 };
                pPoolItem = pFrame->GetBindings().ExecuteSynchron( IsReadOnlyMedium() ? SID_SAVEASDOC : SID_SAVEDOC, ppArgs );
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
#ifndef DISABLE_SCRIPTING
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
#endif

//--------------------------------------------------------------------

BasicManager* SfxObjectShell::GetBasicManager() const
{
#ifdef DISABLE_SCRIPTING
    return NULL;
#else
    BasicManager* pBasMgr = lcl_getBasicManagerForDocument( *this );
    if ( !pBasMgr )
        pBasMgr = SFX_APP()->GetBasicManager();
    return pBasMgr;
#endif
}

//--------------------------------------------------------------------

void SfxObjectShell::SetHasNoBasic()
{
    pImp->m_bNoBasicCapabilities = sal_True;
}

//--------------------------------------------------------------------

sal_Bool SfxObjectShell::HasBasic() const
{
#ifdef DISABLE_SCRIPTING
    return sal_False;
#else
    if ( pImp->m_bNoBasicCapabilities )
        return sal_False;

    if ( !pImp->bBasicInitialized )
        const_cast< SfxObjectShell* >( this )->InitBasicManager_Impl();

    return pImp->pBasicManager->isValid();
#endif
}

//--------------------------------------------------------------------
#ifndef DISABLE_SCRIPTING
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
            catch (const Exception&)
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
        return _rxContainer;
    }
}
#endif
//--------------------------------------------------------------------

Reference< XLibraryContainer > SfxObjectShell::GetDialogContainer()
{
#ifndef DISABLE_SCRIPTING
    if ( !pImp->m_bNoBasicCapabilities )
        return lcl_getOrCreateLibraryContainer( false, pImp->xDialogLibraries, GetModel() );

    BasicManager* pBasMgr = lcl_getBasicManagerForDocument( *this );
    if ( pBasMgr )
        return pBasMgr->GetDialogLibraryContainer().get();

    OSL_FAIL( "SfxObjectShell::GetDialogContainer: falling back to the application - is this really expected here?" );
#endif
    return SFX_APP()->GetDialogContainer();
}

//--------------------------------------------------------------------

Reference< XLibraryContainer > SfxObjectShell::GetBasicContainer()
{
#ifndef DISABLE_SCRIPTING
    if ( !pImp->m_bNoBasicCapabilities )
        return lcl_getOrCreateLibraryContainer( true, pImp->xBasicLibraries, GetModel() );

    BasicManager* pBasMgr = lcl_getBasicManagerForDocument( *this );
    if ( pBasMgr )
        return pBasMgr->GetScriptLibraryContainer().get();

    OSL_FAIL( "SfxObjectShell::GetBasicContainer: falling back to the application - is this really expected here?" );
#endif
    return SFX_APP()->GetBasicContainer();
}

//--------------------------------------------------------------------

StarBASIC* SfxObjectShell::GetBasic() const
{
#ifdef DISABLE_SCRIPTING
    return NULL;
#else
    return GetBasicManager()->GetLib(0);
#endif
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
#ifndef DISABLE_SCRIPTING
    DBG_ASSERT( !pImp->bBasicInitialized && !pImp->pBasicManager->isValid(), "Lokaler BasicManager bereits vorhanden");
    pImp->pBasicManager->reset( BasicManagerRepository::getDocumentBasicManager( GetModel() ) );
    DBG_ASSERT( pImp->pBasicManager->isValid(), "SfxObjectShell::InitBasicManager_Impl: did not get a BasicManager!" );
    pImp->bBasicInitialized = sal_True;
#endif
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

uno::Sequence< OUString > SfxObjectShell::GetEventNames()
{
    static uno::Sequence< OUString >* pEventNameContainer = NULL;

    if ( !pEventNameContainer )
    {
        SolarMutexGuard aGuard;
        if ( !pEventNameContainer )
        {
            static uno::Sequence< OUString > aEventNameContainer = GlobalEventConfig().getElementNames();
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
    WeakReference< XInterface >& rTheCurrentComponent = theCurrentComponent::get();

    Reference< XInterface > xOldCurrentComp(rTheCurrentComponent);
    if ( _rxComponent == xOldCurrentComp )
        // nothing to do
        return;
    // note that "_rxComponent.get() == s_xCurrentComponent.get().get()" is /sufficient/, but not
    // /required/ for "_rxComponent == s_xCurrentComponent.get()".
    // In other words, it's still possible that we here do something which is not necessary,
    // but we should have filtered quite some unnecessary calls already.

#ifndef DISABLE_SCRIPTING
    BasicManager* pAppMgr = SFX_APP()->GetBasicManager();
    rTheCurrentComponent = _rxComponent;
    if ( pAppMgr )
    {
        // set "ThisComponent" for Basic
        pAppMgr->SetGlobalUNOConstant( "ThisComponent", Any( _rxComponent ) );

        // set new current component for VBA compatibility
        if ( _rxComponent.is() )
        {
            OString aVBAConstName = lclGetVBAGlobalConstName( _rxComponent );
            if ( !aVBAConstName.isEmpty() )
            {
                pAppMgr->SetGlobalUNOConstant( aVBAConstName.getStr(), Any( _rxComponent ) );
                s_aRegisteredVBAConstants[ _rxComponent.get() ] = aVBAConstName;
            }
        }
        // no new component passed -> remove last registered VBA component
        else if ( xOldCurrentComp.is() )
        {
            OString aVBAConstName = lclGetVBAGlobalConstName( xOldCurrentComp );
            if ( !aVBAConstName.isEmpty() )
            {
                pAppMgr->SetGlobalUNOConstant( aVBAConstName.getStr(), Any( Reference< XInterface >() ) );
                s_aRegisteredVBAConstants.erase( xOldCurrentComp.get() );
            }
        }
    }
#endif
}

Reference< XInterface > SfxObjectShell::GetCurrentComponent()
{
    return theCurrentComponent::get();
}


OUString SfxObjectShell::GetServiceNameFromFactory( const OUString& rFact )
{
    //! Remove everything behind name!
    OUString aFact( rFact );
    OUString aPrefix("private:factory/");
    if ( aFact.startsWith( aPrefix ) )
        aFact = aFact.copy( aPrefix.getLength() );
    sal_Int32 nPos = aFact.indexOf( '?' );
    OUString aParam;
    if ( nPos != -1 )
    {
        aParam = aFact.copy( nPos );
        aFact = aFact.copy( 0, nPos );
        aParam = aParam.copy(1);
    }
    aFact = comphelper::string::remove(aFact, '4');
    aFact = aFact.toAsciiLowerCase();

    // HACK: sometimes a real document service name is given here instead of
    // a factory short name. Set return value directly to this service name as fallback
    // in case next lines of code does nothing ...
    // use rFact instead of normed aFact value !
    OUString aServiceName = rFact;

    if ( aFact == "swriter" )
    {
        aServiceName = "com.sun.star.text.TextDocument";
    }
    else if ( aFact == "sweb" || aFact == "swriter/web" )
    {
        aServiceName = "com.sun.star.text.WebDocument";
    }
    else if ( aFact == "sglobal" || aFact == "swriter/globaldocument" )
    {
        aServiceName = "com.sun.star.text.GlobalDocument";
    }
    else if ( aFact == "scalc" )
    {
        aServiceName = "com.sun.star.sheet.SpreadsheetDocument";
    }
    else if ( aFact == "sdraw" )
    {
        aServiceName = "com.sun.star.drawing.DrawingDocument";
    }
    else if ( aFact == "simpress" )
    {
        aServiceName = "com.sun.star.presentation.PresentationDocument";
    }
    else if ( aFact == "schart" )
    {
        aServiceName = "com.sun.star.chart.ChartDocument";
    }
    else if ( aFact == "smath" )
    {
        aServiceName = "com.sun.star.formula.FormulaProperties";
    }
#ifndef DISABLE_SCRIPTING
    else if ( aFact == "sbasic" )
    {
        aServiceName = "com.sun.star.script.BasicIDE";
    }
#endif
#ifndef DISABLE_DBCONNECTIVITY
    else if ( aFact == "sdatabase" )
    {
        aServiceName = "com.sun.star.sdb.OfficeDatabaseDocument";
    }
#endif

    return aServiceName;
}

SfxObjectShell* SfxObjectShell::CreateObjectByFactoryName( const OUString& rFact, SfxObjectCreateMode eMode )
{
    return CreateObject( GetServiceNameFromFactory( rFact ), eMode );
}


SfxObjectShell* SfxObjectShell::CreateObject( const OUString& rServiceName, SfxObjectCreateMode eCreateMode )
{
    if ( !rServiceName.isEmpty() )
    {
        uno::Reference < frame::XModel > xDoc( ::comphelper::getProcessServiceFactory()->createInstance( rServiceName ), UNO_QUERY );
        if ( xDoc.is() )
        {
            uno::Reference < lang::XUnoTunnel > xObj( xDoc, UNO_QUERY );
            uno::Sequence < sal_Int8 > aSeq( SvGlobalName( SFX_GLOBAL_CLASSID ).GetByteSequence() );
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

Reference<lang::XComponent> SfxObjectShell::CreateAndLoadComponent( const SfxItemSet& rSet, SfxFrame* pFrame )
{
    uno::Sequence < beans::PropertyValue > aProps;
    TransformItems( SID_OPENDOC, rSet, aProps );
    SFX_ITEMSET_ARG(&rSet, pFileNameItem, SfxStringItem, SID_FILE_NAME, sal_False);
    SFX_ITEMSET_ARG(&rSet, pTargetItem, SfxStringItem, SID_TARGETNAME, sal_False);
    OUString aURL;
    OUString aTarget("_blank");
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
        xLoader = uno::Reference < frame::XComponentLoader >( frame::Desktop::create(comphelper::getProcessComponentContext()), uno::UNO_QUERY );

    Reference <lang::XComponent> xComp;
    try
    {
        xComp = xLoader->loadComponentFromURL(aURL, aTarget, 0, aProps);
    }
    catch (const uno::Exception&)
    {
    }

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


bool SfxObjectShell::SetProtectionPassword( const OUString & /*rPassword*/ )
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
