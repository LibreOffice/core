/*************************************************************************
 *
 *  $RCSfile: objxtor.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: mba $ $Date: 2001-03-09 17:54:06 $
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

#include "arrdecl.hxx"

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _SV_RESARY_HXX
#include <vcl/resary.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _RTTI_HXX //autogen
#include <tools/rtti.hxx>
#endif
#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif
#ifndef _SB_SBSTAR_HXX //autogen
#include <basic/sbstar.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#pragma hdrstop

#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif
#if SUPD<613//MUSTINI
#ifndef _SFXINIMGR_HXX //autogen
#include <svtools/iniman.hxx>
#endif
#endif
#include "objsh.hxx"

#ifndef _BASIC_SBUNO_HXX
#include <basic/sbuno.hxx>
#endif

#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif
#ifndef _EHDL_HXX
#include <svtools/ehdl.hxx>
#endif

#include <svtools/urihelper.hxx>
#include <svtools/pathoptions.hxx>

#include "picklist.hxx"
#include "docfac.hxx"
#include "docfile.hxx"
#include "event.hxx"
#include "cfgmgr.hxx"
#include "dispatch.hxx"
#include "viewsh.hxx"
#include "interno.hxx"
#include "sfxresid.hxx"
#include "objshimp.hxx"
#include "appbas.hxx"
#include "sfxtypes.hxx"
#include "evntconf.hxx"
#include "request.hxx"
#include "doc.hrc"
#include "sfxlocal.hrc"
#include "docinf.hxx"
#include "objuno.hxx"
#include "appdata.hxx"
#include "appuno.hxx"
#include "sfxsids.hrc"
#include "basmgr.hxx"

#ifndef _UNO_MAPPING_HXX_
#include <uno/mapping.hxx>
#endif

//====================================================================

DBG_NAME(SfxObjectShell);

#define DocumentInfo
#include "sfxslots.hxx"

extern AsynchronLink* pPendingCloser;

//=========================================================================

TYPEINIT1(SfxObjectShell, SfxShell);

//--------------------------------------------------------------------

SfxObjectShell_Impl::~SfxObjectShell_Impl()
{
    if ( pPendingCloser == pCloser )
        pPendingCloser = 0;
    delete pCloser;
}

// initializes a document from a file-description

SfxObjectShell::SfxObjectShell
(
    SfxObjectCreateMode eMode   /*  Zweck, zu dem die SfxObjectShell
                                    erzeugt wird:

                                    SFX_CREATE_MODE_EMBEDDED (default)
                                        als SO-Server aus einem anderen
                                        Dokument heraus

                                    SFX_CREATE_MODE_STANDARD,
                                        als normales, selbst"aendig ge"offnetes
                                        Dokument

                                    SFX_CREATE_MODE_PREVIEW
                                        um ein Preview durchzuf"uhren,
                                        ggf. werden weniger Daten ben"otigt

                                    SFX_CREATE_MODE_ORGANIZER
                                        um im Organizer dargestellt zu
                                        werden, hier werden keine Inhalte
                                        ben"otigt */
)

/*  [Beschreibung]

    Konstruktor der Klasse SfxObjectShell.
*/

:   pImp( new SfxObjectShell_Impl ),
    _pFactory( 0 ),
    eCreateMode(eMode),
    pStyleSheetPool(0),
    pMedium(0)
{
    DBG_CTOR(SfxObjectShell, 0);

    bHasName = sal_False;
    SfxShell::SetName( GetTitle() );
    nViewNo = 0;

    // Aggregation InPlaceObject+Automation
//(mba)    AddInterface( SvDispatch::ClassFactory() );

    SfxObjectShell *pThis = this;
    SfxObjectShellArr_Impl &rArr = SFX_APP()->GetObjectShells_Impl();
    rArr.C40_INSERT( SfxObjectShell, pThis, rArr.Count() );
    pImp->bInList = sal_True;
    pImp->nLoadedFlags = SFX_LOADED_ALL;
}

//--------------------------------------------------------------------

// virtual dtor of typical base-class SfxObjectShell

SfxObjectShell::~SfxObjectShell()
{
    DBG_DTOR(SfxObjectShell, 0);

    if ( IsEnableSetModified() )
        EnableSetModified( sal_False );

    // Niemals GetInPlaceObject() aufrufen, der Zugriff auf den
    // Ableitungszweig SfxInternObject ist wegen eines Compiler Bugs nicht
    // erlaubt
    SfxObjectShell::Close();
    if ( pImp->xModel.is() )
        pImp->xModel->dispose();

    DELETEX(pMedium);
    DELETEX(pImp->pEventConfig);
    DELETEX(pImp->pCfgMgr);
    DELETEX( pImp->pReloadTimer );

    SfxApplication *pSfxApp = SFX_APP();
    if ( USHRT_MAX != pImp->nVisualDocumentNumber )
        pSfxApp->ReleaseIndex(pImp->nVisualDocumentNumber);

    // Basic-Manager zerst"oren
    if ( pImp->pBasicMgr )
        DELETEX(pImp->pBasicMgr);

    if ( pSfxApp->GetDdeService() )
        pSfxApp->RemoveDdeTopic( this );

    delete pImp->pDocInfo;
    if ( pImp->xModel.is() )
        pImp->xModel = ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > ();

    delete pImp;
}

//--------------------------------------------------------------------

SfxObjectFactory& SfxObjectShell::GetFactory() const
{
    return *_pFactory;
}
//--------------------------------------------------------------------

void SfxObjectShell::ViewAssigned()

/*  [Beschreibung]

    Diese Methode wird gerufen, wenn eine ::com::sun::star::sdbcx::View zugewiesen wird.
*/

{
    // Spaetestens jetzt die Factory initialisieren (wegen HelpFileName)
    GetFactory().DoInitFactory();
}

//--------------------------------------------------------------------
// closes the Object and all its views

sal_Bool SfxObjectShell::Close()
{
    {DBG_CHKTHIS(SfxObjectShell, 0);}
    if ( !pImp->bClosing )
    {
        // falls noch ein Progress l"auft, nicht schlie\sen
        if ( GetProgress() )
            return sal_False;

        // aus Document-Liste austragen
        SfxApplication *pSfxApp = SFX_APP();
        SfxObjectShellArr_Impl &rDocs = pSfxApp->GetObjectShells_Impl();
        const SfxObjectShell *pThis = this;
        sal_uInt16 nPos = rDocs.GetPos(pThis);
        if ( nPos < rDocs.Count() )
            rDocs.Remove( nPos );
        pImp->bInList = sal_False;

        // Broadcasten (w"ahrend dessen festhalten)
        pImp->bClosing = sal_True;
        SfxObjectShellRef aRef(this);
/*
        // Ist leider zu sp"at, da kaum noch Macros laufen, wenn keine View
        // mehr da ist!
        if ( _pFactory && _pFactory->GetFlags() & SFXOBJECTSHELL_HASOPENDOC )
            // Event nur bei echten Dokumenten
            pSfxApp->NotifyEvent( SfxEventHint(SFX_EVENT_CLOSEDOC, this) );
*/
        Broadcast( SfxSimpleHint(SFX_HINT_DEINITIALIZING) );
        //pImp->bClosing = sal_False;
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
             ( !bOnlyVisible || SfxViewFrame::GetFirst( pSh, 0, sal_True )))
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
             ( !bOnlyVisible || SfxViewFrame::GetFirst( pSh, 0, sal_True )))
            return pSh;
    }
    return 0;
}

//--------------------------------------------------------------------

SfxObjectShell* SfxObjectShell::Current()
{
    SfxViewFrame *pFrame = SFX_APP()->GetViewFrame();
    return pFrame ? pFrame->GetObjectShell() : 0;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::IsInPrepareClose() const

/*  [Beschreibung]

    Diese Methode liefert sal_True, falls gerade ein PrepareClose laeuft.
*/

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
    sal_Bool    bUI,        // sal_True: Dialoge etc. erlaubt, sal_False: silent-mode
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
    if( pFirst && !pFirst->GetFrame()->PrepareClose_Impl( bUI, bForBrowsing ) )
        return sal_False;

    // prepare views for closing
    for ( SfxViewFrame* pFrm = SfxViewFrame::GetFirst(
        this, TYPE(SfxViewFrame));
          pFrm; pFrm = SfxViewFrame::GetNext( *pFrm, this ) )
    {
        DBG_ASSERT(pFrm->GetViewShell(),"KeineShell");
        if ( pFrm->GetViewShell() )
        {
            sal_uInt16 nRet = pFrm->GetViewShell()->PrepareClose( bUI, bForBrowsing );
            if ( nRet != sal_True )
                return nRet;
        }
    }

    if( GetInPlaceObject() && GetInPlaceObject()->GetClient())
    {
        pImp->bPreparedForClose = sal_True;
        return sal_True;
    }

    // ggf. nachfragen, ob gespeichert werden soll
        // nur fuer in sichtbaren Fenstern dargestellte Dokumente fragen
    SfxViewFrame *pFrame = SfxObjectShell::Current() == this
        ? SfxViewFrame::Current() : SfxViewFrame::GetFirst( this );
    while ( pFrame && (pFrame->GetFrameType() & SFXFRAME_SERVER ) )
        pFrame = SfxViewFrame::GetNext( *pFrame, this );

    SfxApplication *pSfxApp = SFX_APP();
    pSfxApp->NotifyEvent( SfxEventHint(SFX_EVENT_PREPARECLOSEDOC, this) );
    sal_Bool bClose = sal_False;
    if ( bUI && IsModified() )
    {
        if ( pFrame )
        {
            // minimierte restoren
            SfxFrame* pTop = pFrame->GetTopFrame();
            pSfxApp->SetViewFrame( pTop->GetCurrentViewFrame() );
            pFrame->GetFrame()->Appear();

            // fragen, ob gespeichert werden soll
            String aText( SfxResId( STR_QUERY_SAVE_DOCUMENT ) );
            aText.SearchAndReplace( DEFINE_CONST_UNICODE( "$(DOC)" ),
                                    GetTitle( SFX_TITLE_PICKLIST ) );
            /*HACK for plugin::destroy()*/
            // Don't show SAVE dialog in plugin mode! We save our document in every way.
            short nRet = RET_YES;
            if( SfxApplication::IsPlugin() == sal_False )
            {
                QueryBox aQBox( &pFrame->GetWindow(), WB_YES_NO_CANCEL | WB_DEF_YES, aText );
                aQBox.SetButtonText( BUTTONID_NO, SfxResId( STR_NOSAVEANDCLOSE ) );
                aQBox.SetButtonText( BUTTONID_YES, SfxResId( STR_SAVEDOC ) );
    //(mba)/task            if ( bForBrowsing )
    //                aQBox.AddButton( String( SfxResId( RID_STR_NEW_TASK ) ), RET_NEWTASK, BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_FOCUSBUTTON );
                nRet = aQBox.Execute();
            }
            /*HACK for plugin::destroy()*/

            if ( RET_YES == nRet )
            {
                sal_Bool bVersion = GetDocInfo().IsSaveVersionOnClose();

                // per Dispatcher speichern
                const SfxPoolItem *pPoolItem;
                if ( bVersion )
                {
                    SfxStringItem aItem( SID_VERSION, String( SfxResId( STR_AUTOMATICVERSION ) ) );
                    SfxBoolItem aWarnItem( SID_FAIL_ON_WARNING, TRUE );
                    const SfxPoolItem* ppArgs[] = { &aItem, &aWarnItem, 0 };
                    pPoolItem = pFrame->GetBindings().ExecuteSynchron( SID_SAVEDOC, ppArgs );
                }
                else
                {
                    SfxBoolItem aWarnItem( SID_FAIL_ON_WARNING, TRUE );
                    const SfxPoolItem* ppArgs[] = { &aWarnItem, 0 };
                    pPoolItem = pFrame->GetBindings().ExecuteSynchron( SID_SAVEDOC, ppArgs );
                }

                if ( !pPoolItem || ( pPoolItem->ISA(SfxBoolItem) && !( (const SfxBoolItem*) pPoolItem )->GetValue() ) )
                    return sal_False;
                else
                    bClose = sal_True;
            }
            else if ( RET_CANCEL == nRet )
                // abgebrochen
                return sal_False;
            else if ( RET_NEWTASK == nRet )
            {
                return RET_NEWTASK;
            }
            else
            {
                // Bei Nein nicht noch Informationlost
                bClose = sal_True;

                // nicht mehr aus Cache nachladen
                MemCache_Impl &rCache = SfxPickList_Impl::Get()->GetMemCache();
                rCache.RemoveObject( this );
            }
        }
    }

    // ggf. hinweisen, da\s unter Fremdformat gespeichert
    if( pMedium )
    {
        SFX_ITEMSET_ARG( pMedium->GetItemSet(), pIgnoreInformationLost,
                         SfxBoolItem, SID_DOC_IGNOREINFORMATIONLOST, sal_False);
        if( pIgnoreInformationLost && pIgnoreInformationLost->GetValue() )
            bUI = sal_False;
    }
    if ( bUI && !bClose && IsInformationLost() )
    {
        // minimierte restoren
        SfxFrame* pTop = pFrame->GetTopFrame();
        pSfxApp->SetViewFrame( pTop->GetCurrentViewFrame() );
        pFrame->GetFrame()->Appear();
        QueryBox aBox( &pFrame->GetWindow(), SfxResId(MSG_CONFIRM_FILTER));
        if ( RET_NO == aBox.Execute() )
            return sal_False;
    }

    pImp->bPreparedForClose = sal_True;
    return sal_True;
}

//--------------------------------------------------------------------

BasicManager* SfxObjectShell::GetBasicManager() const
{
    return HasBasic() ? pImp->pBasicMgr : SFX_APP()->GetBasicManager();
}

sal_Bool SfxObjectShell::HasBasic() const
{
    if ( !pImp->bBasicInitialized )
    {
        String aName( GetMedium()->GetName() );
        ((SfxObjectShell*)this)->InitBasicManager_Impl( GetStorage(), aName.Len() ? &aName : NULL );
    }
    return pImp->pBasicMgr != NULL;
}

//--------------------------------------------------------------------

StarBASIC* SfxObjectShell::GetBasic() const
{
    return GetBasicManager()->GetLib(0);
}

//--------------------------------------------------------------------

void SfxObjectShell::InitBasicManager_Impl
(
    SvStorage*  pStor       /* Storage, aus dem das Dokument geladen wird
                               (aus <SvPersist::Load()>) bzw. 0, falls es
                               sich um ein neues Dokument handelt
                               (aus <SvPersist::InitNew()>). */
    , const String* pName
)
/*  [Beschreibung]

    Erzeugt einen Dokument-BasicManager und l"adt diesen ggf. (pStor != 0)
    aus dem Storage.


    [Anmerkung]

    Diese Methode mu"s aus den "Uberladungen von <SvPersist::Load()> (mit
    dem pStor aus dem Parameter von Load()) sowie aus der "Uberladung
    von <SvPersist::InitNew()> (mit pStor = 0) gerufen werden.
*/

{
    StarBASIC *pAppBasic = SFX_APP()->GetBasic();
    DBG_ASSERT( !pImp->bBasicInitialized && !pImp->pBasicMgr, "Lokaler BasicManager bereits vorhanden");

    pImp->bBasicInitialized = TRUE;
    if ( pStor )
    {
        String aOldURL = INetURLObject::GetBaseURL();
        String aNewURL;
        if( HasName() )
            aNewURL = GetMedium()->GetName();
        else
        {
            aNewURL = GetDocInfo().GetTemplateFileName();
            // Bei Templates keine ::com::sun::star::util::URL...
            aNewURL = URIHelper::SmartRelToAbs( aNewURL );
        }
        INetURLObject::SetBaseURL( aNewURL );

        // load BASIC-manager
        SfxErrorContext aErrContext( ERRCTX_SFX_LOADBASIC, GetTitle() );
#if SUPD<613//MUSTINI
        SfxIniManager *pIniMgr = SFX_APP()->GetIniManager();
        String aAppBasicDir( pIniMgr->Get(SFX_KEY_BASIC_PATH) );
#else
        String aAppBasicDir = SvtPathOptions().GetBasicPath();
#endif
        pImp->pBasicMgr = new SfxBasicManager( *pStor, pAppBasic, &aAppBasicDir );
        if ( pImp->pBasicMgr->HasErrors() )
        {
            // handle errors
            BasicError *pErr = pImp->pBasicMgr->GetFirstError();
            while ( pErr )
            {
                // show message to user
                if ( ERRCODE_BUTTON_CANCEL ==
                     ErrorHandler::HandleError( pErr->GetErrorId() ) )
                {
                    // user wants to break loading of BASIC-manager
                    delete pImp->pBasicMgr;
                    pStor = 0;
                    break;
                }
                pErr = pImp->pBasicMgr->GetNextError();
            }
        }

        INetURLObject::SetBaseURL( aOldURL );
    }

    // not loaded?
    if ( !pStor )
    {
        // create new BASIC-manager
        StarBASIC *pBas = new StarBASIC(pAppBasic);
        pBas->SetFlag( SBX_EXTSEARCH );
        pImp->pBasicMgr = new SfxBasicManager( pBas );
    }

    // damit auch Dialoge etc. 'qualifiziert' angesprochen werden k"onnen
    StarBASIC *pBas = pImp->pBasicMgr->GetLib(0);
    // Initialize Uno
    //pBas->setRoot( GetModel() );
    sal_Bool bWasModified = pBas->IsModified();
    pBas->SetParent( pAppBasic );

    // Properties im Doc-BASIC
    // ThisComponent
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  xInterface ( GetModel(), ::com::sun::star::uno::UNO_QUERY );
    ::com::sun::star::uno::Any aComponent;
    aComponent <<= xInterface;
    SbxObjectRef xUnoObj = GetSbUnoObject( DEFINE_CONST_UNICODE("ThisComponent"), aComponent );
    xUnoObj->SetFlag( SBX_DONTSTORE );
    pBas->Insert( xUnoObj );

    // Modify-Flag wird bei MakeVariable gesetzt
    pBas->SetModified( bWasModified );
}

//--------------------------------------------------------------------
#if 0 //(mba)
SotObjectRef SfxObjectShell::CreateAggObj( const SotFactory* pFact )
{
    // SvDispatch?
    SotFactory* pDispFact = SvDispatch::ClassFactory();
    if( pFact == pDispFact )
        return( (SfxShellObject*)GetSbxObject() );

    // sonst unbekannte Aggregation
    DBG_ERROR("unkekannte Factory");
    SotObjectRef aSvObjectRef;
    return aSvObjectRef;
}
#endif

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

void SfxObjectShell::SetLastMark_Impl( const String &rMark )
{
    pImp->aMark = rMark;
}

//--------------------------------------------------------------------

const String& SfxObjectShell::GetLastMark_Impl() const
{
    return pImp->aMark;
}

//--------------------------------------------------------------------

SfxObjectShell* SfxObjectShell::GetObjectShell()
{
    return this;
}

//--------------------------------------------------------------------

SEQUENCE< OUSTRING > SfxObjectShell::GetEventNames()
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    ResStringArray aEventNames( SfxResId( EVENT_NAMES_ARY ) );
    USHORT nCount = aEventNames.Count();

    SEQUENCE < OUSTRING > aSequence( nCount );

    OUSTRING* pNames = aSequence.getArray();

    for ( USHORT i=0; i<nCount; i++ )
    {
        pNames[i] = aEventNames.GetString( i );
    }

    return aSequence;
}

//--------------------------------------------------------------------
/* ASDBG
void SfxObjectShell::SetModel( SfxModel* pModel )
{
    if ( pImp->xModel.is() )
        DBG_WARNING( "Model already set!" );
    pImp->xModel = pModel;
}

//--------------------------------------------------------------------

XModel* SfxObjectShell::GetModel()
{
    return pImp->xModel;
}
*/
//--------------------------------------------------------------------

void SfxObjectShell::SetModel( SfxBaseModel* pModel )
{
    if ( pImp->xModel.is() )
        DBG_WARNING( "Model already set!" );

    pImp->xModel = pModel;
}

//--------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > SfxObjectShell::GetModel()
{
    return pImp->xModel;
}

void SfxObjectShell::SetBaseModel( SfxBaseModel* pModel )
{
    if ( pImp->xModel.is() )
        DBG_WARNING( "Model already set!" );

    pImp->xModel = pModel;
}

//--------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > SfxObjectShell::GetBaseModel()
{
    return pImp->xModel;
}

