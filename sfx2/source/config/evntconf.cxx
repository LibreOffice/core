/*************************************************************************
 *
 *  $RCSfile: evntconf.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:29 $
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

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _BASMGR_HXX //autogen
#include <basic/basmgr.hxx>
#endif
#ifndef _SB_SBMOD_HXX //autogen
#include <basic/sbmod.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif
#pragma hdrstop

#include "evntconf.hxx"

#include "macrconf.hxx"
#include "docfile.hxx"
#include "app.hxx"
#include "objsh.hxx"
#include "dispatch.hxx"
#include "config.hrc"
#include "sfxresid.hxx"
#include "cfgmgr.hxx"
#include "macropg.hxx"
#include "docinf.hxx"

// -----------------------------------------------------------------------

static const USHORT nCompatVersion = 3;
static const USHORT nOldVersion = 4;
static const USHORT nVersion = 5;

TYPEINIT1(SfxEventHint, SfxHint);

// class SfxMacroQueryDlg_Impl -------------------------------------------

class SfxMacroQueryDlg_Impl : public ModalDialog
{
    FixedImage              aIcon;
    FixedText               aFTMessage;
    CheckBox                aCBWarning;
    OKButton                aOKButton;
    CancelButton            aCancelButton;

public:
                            SfxMacroQueryDlg_Impl( const ResId& rResId );
    void                    SetWarningState( BOOL bOn )
                            { aCBWarning.Check( !bOn ); }
    BOOL                    GetWarningState() const
                            { return !aCBWarning.IsChecked(); }
    void                    HideCheckBox()
                            { aCBWarning.Hide(); }
};

// -----------------------------------------------------------------------

SfxMacroQueryDlg_Impl::SfxMacroQueryDlg_Impl( const ResId& rResId ) :
    ModalDialog( NULL, rResId ),
    aIcon( this, ResId( ICN_MESSAGE ) ),
    aFTMessage( this, ResId( FT_MESSAGE ) ),
    aCBWarning( this, ResId( CB_WARNING ) ),
    aOKButton( this, ResId( BTN_OK ) ),
    aCancelButton( this, ResId( BTN_CANCEL ) )
{
    FreeResource();
    aCBWarning.Show(FALSE);
    long nY = aCBWarning.GetPosPixel().Y();
    aOKButton.SetPosPixel( Point( aOKButton.GetPosPixel().X(), nY ) );
    aCancelButton.SetPosPixel( Point( aCancelButton.GetPosPixel().X(), nY ) );
    SetOutputSizePixel( Size( GetOutputSizePixel().Width(),
                              nY + aOKButton.GetSizePixel().Height() + 3 ) );
    aIcon.SetImage( QueryBox::GetStandardImage() );
}

// class SfxAsyncEvent_Impl ----------------------------------------------

class SfxAsyncEvent_Impl : public SfxListener
{
    String          aArgs;
    SfxObjectShell* pSh;
    const SvxMacro* pMacro;
    Timer *pTimer;

public:

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
    SfxAsyncEvent_Impl( SfxObjectShell *pDoc, const SvxMacro *pMacro, const String& rArgs );
    ~SfxAsyncEvent_Impl();
    DECL_LINK( TimerHdl, Timer*);
};

// -----------------------------------------------------------------------

void SfxAsyncEvent_Impl::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    SfxSimpleHint* pHint = PTR_CAST( SfxSimpleHint, &rHint );
    if( pHint && pHint->GetId() == SFX_HINT_DYING && pTimer->IsActive() )
    {
        pTimer->Stop();
        delete this;
    }
}

// -----------------------------------------------------------------------

SfxAsyncEvent_Impl::SfxAsyncEvent_Impl( SfxObjectShell *pDoc, const SvxMacro *pMac, const String& rArgs )
 : aArgs( rArgs )
 , pSh( pDoc )
 , pMacro( pMac )
{
    if( pSh ) StartListening( *pSh );
    pTimer = new Timer;
    pTimer->SetTimeoutHdl( LINK(this, SfxAsyncEvent_Impl, TimerHdl) );
    pTimer->SetTimeout( 0 );
    pTimer->Start();
}

// -----------------------------------------------------------------------

SfxAsyncEvent_Impl::~SfxAsyncEvent_Impl()
{
    delete pTimer;
}

// -----------------------------------------------------------------------

IMPL_LINK(SfxAsyncEvent_Impl, TimerHdl, Timer*, pTimer)
{
    pTimer->Stop();
    ScriptType eSType = pMacro->GetScriptType();
    BOOL bIsBasic = ( eSType == STARBASIC );
    if ( bIsBasic && StarBASIC::IsRunning() )
        // Neues eventgebundenes Macro erst ausf"uhren, wenn gerade kein anderes Macro mehr l"auft
        pTimer->Start();
    else
    {
        SFX_APP()->GetMacroConfig()->ExecuteMacro( pSh, pMacro, aArgs );
        delete this;
    }

    return 0L;
}

//==========================================================================

SfxEventConfiguration::SfxEventConfiguration()
 : pDocEventConfig( NULL )
{
    // Array zum Ermitteln der Bindungen
    pEventArr = new SfxEventArr_Impl;

    // Einen default entry eingf"ugen
    const SfxEvent_Impl *pEvent = new SfxEvent_Impl(String(), 0);
    pEventArr->Insert(pEvent, 0);

    // Die EventConfig f"ur globale Bindungen
    pAppEventConfig =
            new SfxEventConfigItem_Impl( SFX_ITEMTYPE_APPEVENTCONFIG, this );
}

//==========================================================================

SfxEventConfiguration::~SfxEventConfiguration()
{
    for (USHORT n=0; n<pEventArr->Count(); n++)
        delete (*pEventArr)[n];
    delete pEventArr;
    delete pAppEventConfig;
}

//==========================================================================

void SfxEventConfiguration::RegisterEvent(USHORT nId, const String& rName)
{
    USHORT nCount = pEventArr->Count();
    const SfxEvent_Impl *pEvent = new SfxEvent_Impl(rName, nId);
    pEventArr->Insert(pEvent, nCount);
}

//==========================================================================

const String& SfxEventConfiguration::GetEventName(USHORT nId) const
{
    DBG_ASSERT(pEventArr,"Keine Events angemeldet!");
    USHORT nCount = pEventArr->Count();
    for (USHORT n=1; n<nCount; n++)
    {
        if ((*pEventArr)[n]->nEventId == nId)
        {
            return (*pEventArr)[n]->aEventName;
        }
    }

    DBG_ERROR("Event nicht gefunden!");
    return (*pEventArr)[0]->aEventName;
}

//==========================================================================

USHORT SfxEventConfiguration::GetEventId(const String& rName) const
{
    DBG_ASSERT(pEventArr,"Keine Events angemeldet!");
    USHORT nCount = pEventArr->Count();
    for (USHORT n=1; n<nCount; n++)
    {
        if ((*pEventArr)[n]->aEventName == rName)
        {
            return (*pEventArr)[n]->nEventId;
        }
    }

    DBG_ERROR("Event nicht gefunden!");
    return SFX_NO_EVENT;
}

void SfxEventConfiguration::ConfigureEvent( USHORT nId, const String& rMacro, SfxObjectShell *pObjSh )
{
    String aLibrary, aModule, aFunction;
    if ( STRING_NOTFOUND == rMacro.Search( '(' ) )
    {
        // interne Syntax: Library.Module.Function
        aFunction = rMacro.GetToken( 2, '.' );
        aModule = rMacro.GetToken( 1, '.' );
        aLibrary = rMacro.GetToken( 0, '.' );
    }
    else
    {
        // UI-Syntax: Function(Library.Module)
        aFunction = rMacro.GetToken( 0, '(' );
        String aModLib( rMacro.GetToken( 1, '(' ) );
        aLibrary = aModLib.GetToken( 0, '.' );
        aModule = aModLib.GetToken( 1, '.' ).GetToken( 0, ')' );
    }

    //Ist ein DocBasic-Macro oder nicht ?
    BOOL bDocBasic = FALSE;
    if ( pObjSh )
    {
        BasicManager *pMgr = pObjSh->GetBasicManager();
        StarBASIC* pLib = pMgr->GetLib( aLibrary );
        if ( pLib )
        {
            SbModule* pMod =
                pLib->FindModule( aModule );
            if ( pMod && pMod->GetMethods()->Find( aFunction, SbxCLASS_METHOD ) )
                bDocBasic = TRUE;
        }
    }

    // Event konfigurieren
    String aCode( aLibrary );
    if ( aCode.Len() )
    {
        aCode += '.';
        aCode += aModule;
        aCode += '.';
        aCode += aFunction;
    }

    String aBasic;
    if( bDocBasic )
        aBasic = pObjSh->GetTitle();
    else
        aBasic = SFX_APP()->GetName();
    SvxMacro aMacro( aCode, aBasic, STARBASIC );
    ConfigureEvent( nId, aMacro, pObjSh );
}

void SfxEventConfiguration::ConfigureEvent( USHORT nId, const SvxMacro& rMacro, SfxObjectShell *pDoc )
{
    SvxMacro *pMacro = NULL;
    if ( rMacro.GetMacName().Len() )
        pMacro = new SvxMacro( rMacro.GetMacName(), rMacro.GetLibName(), rMacro.GetScriptType() );
    if ( pDoc )
    {
        pDocEventConfig = pDoc->GetEventConfig_Impl( TRUE );
        pDocEventConfig->ConfigureEvent( nId, pMacro );
    }
    else
    {
        // globale Bindung
        pAppEventConfig->ConfigureEvent( nId, pMacro );
    }
}

//==========================================================================

void SfxEventConfiguration::ExecuteEvent(
    USHORT nId, SfxObjectShell *pDoc, FASTBOOL bSynchron, const String& rArgs )
{
    pDocEventConfig = pDoc ? pDoc->GetEventConfig_Impl() : NULL;
    const SvxMacro* pMacro=NULL;
    BOOL bDocMacro = TRUE;
    if ( pDocEventConfig )
        pMacro = pDocEventConfig->aMacroTable.Seek( nId );
    if ( !pMacro )
    {
        pMacro = pAppEventConfig->aMacroTable.Seek( nId );
        bDocMacro = FALSE;
    }
    else
    {
        bDocMacro = ( pMacro->GetLibName() != SFX_APP()->GetName()
         && ! pMacro->GetLibName().EqualsAscii("StarDesktop") );
    }

    if ( pMacro )
    {
        SfxApplication *pApp = SFX_APP();
        SfxMacroConfig *pCfg = pApp->GetMacroConfig();

        if ( bDocMacro )
        {
            if ( pCfg->CheckMacro( pDoc, pMacro ) )
            {
                BOOL bRet = !Warn_Impl( pDoc, pMacro );

                // Und gleich abspeichern, wenn die Config in einer Vorlage
                // sitzt und es das Event "CreateDoc" war
                SfxDocumentInfo rDocInfo = pDoc->GetDocInfo();
                if ( nId == SFX_EVENT_CREATEDOC && pDoc->GetConfigManager()->IsModified() &&
                            rDocInfo.HasTemplateConfig() )
                {
                    const String aTemplName( rDocInfo.GetTemplateName() );
                    if ( aTemplName.Len() )
                    {
                        const String aTemplFileName( rDocInfo.GetTemplateFileName() );
                        SvStorageRef aStor = new SvStorage( aTemplFileName );
                        if ( SVSTREAM_OK == aStor->GetError() )
                            pDoc->GetConfigManager()->SaveConfig( aStor );
                    }
                }

                if ( bRet )
                    return;
            }
        }

        if ( bSynchron )
            pCfg->ExecuteMacro( pDoc, pMacro, rArgs );
        else
            new SfxAsyncEvent_Impl( pDoc, pMacro, rArgs );
    }
}

//==========================================================================

const SvxMacro* SfxEventConfiguration::GetMacroForEventId
(
    USHORT          nId,
    SfxObjectShell* pDoc
)
{
    pDocEventConfig = pDoc ? pDoc->GetEventConfig_Impl() : NULL;
    const SvxMacro* pMacro=NULL;
    if ( pDocEventConfig )
        pMacro = pDocEventConfig->aMacroTable.Seek( nId );
    if ( !pMacro )
        pMacro = pAppEventConfig->aMacroTable.Seek( nId );
    return pMacro;
}

const SfxMacroInfo* SfxEventConfiguration::GetMacroInfo
(
    USHORT          nId,
    SfxObjectShell* pDoc
) const
{
    DBG_ASSERT(pEventArr,"Keine Events angemeldet!");

    SfxEventConfigItem_Impl *pDocEventConfig = pDoc ? pDoc->GetEventConfig_Impl() : NULL;
    const SvxMacro* pMacro=NULL;
    if ( pDocEventConfig )
        pMacro = pDocEventConfig->aMacroTable.Seek( nId );
    if ( !pMacro )
        pMacro = pAppEventConfig->aMacroTable.Seek( nId );

    return SFX_APP()->GetMacroConfig()->GetMacroInfo_Impl( pMacro );
}

//==========================================================================

SfxEventConfigItem_Impl::SfxEventConfigItem_Impl( USHORT nConfigId,
    SfxEventConfiguration *pCfg )
    : SfxConfigItem( nConfigId )
    , aMacroTable( 2, 2 )
    , pEvConfig( pCfg )
{
    SetInternal(TRUE);
    SetIndividual(TRUE);
}

void SfxEventConfigItem_Impl::Init( SfxConfigManager *pMgr )
{
    if ( GetConfigManager() == pMgr )
        Initialize();
    else
        ReInitialize( pMgr );
}

//==========================================================================

int SfxEventConfigItem_Impl::Load(SvStream& rStream)
{
    USHORT nFileVersion, nCount;
    rStream >> nFileVersion;
    if ( nFileVersion < nCompatVersion || nFileVersion > nVersion )
        return SfxConfigItem::WARNING_VERSION;

    if ( nFileVersion <= nOldVersion )
    {
        SvUShorts aSlotArray;
        if (!aSlotArray.Count())
        {
            USHORT nCount = pEvConfig->GetEventCount();
            for (USHORT n=0; n<nCount; n++)
                aSlotArray.Insert((USHORT)0 ,n);
        }
        else
        {
            for (USHORT n=0; n<aSlotArray.Count(); n++)
            {
                if (aSlotArray[n])
                    SFX_APP()->GetMacroConfig()->ReleaseSlotId(aSlotArray[n]);
                aSlotArray[n] = 0;
            }
        }

        if ( nFileVersion > nCompatVersion )
        {
            USHORT nWarn;
            rStream >> nWarn;
            bWarning = ( nWarn & 0x01 ) != 0;
            bAlwaysWarning = ( nWarn & 0x02 ) != 0;
        }
        else
            bWarning = bAlwaysWarning = FALSE;

        rStream >> nCount;
        USHORT nId;
        USHORT i;
        for (i=0; i<nCount; i++)
        {
            SfxMacroInfo aInfo( GetConfigManager_Impl()->GetObjectShell() );
            rStream >> nId >> aInfo;

            USHORT nCount = aSlotArray.Count();
            for (USHORT n=0; n<nCount; n++)
            {
                if ((*pEvConfig->pEventArr)[n+1]->nEventId == nId)
                {
                    aSlotArray[n] = aInfo.GetSlotId();
                    break;
                }
            }
        }

        aMacroTable.DelDtor();
        SfxMacroConfig *pMC = SFX_APP()->GetMacroConfig();
        for (i=0; i<aSlotArray.Count(); i++)
        {
            if (aSlotArray[i])
            {
                const SfxMacroInfoPtr pInfo = pMC->GetMacroInfo(aSlotArray[i]);
                SvxMacro *pMacro = new SvxMacro( pInfo->GetQualifiedName(),
                    pInfo->GetBasicName(), STARBASIC );
                aMacroTable.Insert( (*pEvConfig->pEventArr)[i+1]->nEventId, pMacro );
            }
        }
    }
    else
    {
        USHORT nWarn;
        rStream >> nWarn;
        bWarning = ( nWarn & 0x01 ) != 0;
        bAlwaysWarning = ( nWarn & 0x02 ) != 0;
        aMacroTable.Read( rStream );
    }

    return SfxConfigItem::ERR_OK;
}

//==========================================================================

BOOL SfxEventConfigItem_Impl::Store(SvStream& rStream)
{
    USHORT nWarn=0;
    if ( bWarning )
        nWarn |= 0x01;
    if ( bAlwaysWarning )
        nWarn |= 0x02;
    rStream << nVersion << nWarn;
    aMacroTable.Write( rStream );
    return TRUE;
}

//==========================================================================

BOOL SfxEventConfigItem_Impl::Reconfigure(SvStream& rStream, BOOL bDefault)
{
    return SfxConfigItem::Reconfigure(rStream, bDefault);
}

//==========================================================================

String SfxEventConfigItem_Impl::GetName() const
{
    return String(SfxResId(STR_EVENTCONFIG));
}

//==========================================================================

void SfxEventConfigItem_Impl::UseDefault()
{
    bWarning = TRUE;
    bAlwaysWarning = FALSE;

    aMacroTable.DelDtor();
    SfxConfigItem::UseDefault();
}

//==========================================================================

SfxEventConfigItem_Impl::~SfxEventConfigItem_Impl()
{
}

//==========================================================================

void SfxEventConfigItem_Impl::ConfigureEvent( USHORT nId, SvxMacro *pMacro )
{
    if ( aMacroTable.Seek( nId ) )
    {
        if ( pMacro )
            aMacroTable.Replace( nId, pMacro );
        else
            aMacroTable.Remove( nId );
    }
    else if ( pMacro )
        aMacroTable.Insert( nId, pMacro );
    SetDefault(FALSE);
}

BOOL SfxEventConfiguration::IsWarningEnabled() const
{
    return pAppEventConfig->bWarning;
}

BOOL SfxEventConfiguration::IsWarningForced() const
{
    return pAppEventConfig->bAlwaysWarning;
}

void SfxEventConfiguration::SetWarningEnabled( BOOL bOn )
{
    if ( pAppEventConfig->bWarning != bOn )
    {
        pAppEventConfig->bWarning = bOn;
        pAppEventConfig->SetDefault( FALSE );
    }
}

void SfxEventConfiguration::SetWarningForced( BOOL bOn )
{
    if ( pAppEventConfig->bAlwaysWarning != bOn )
    {
        pAppEventConfig->bAlwaysWarning = bOn;
        pAppEventConfig->SetDefault( FALSE );
    }
}


BOOL SfxEventConfiguration::Warn_Impl( SfxObjectShell *pDoc, const SvxMacro* pMacro )
{
    BOOL bWarn = FALSE;
    if ( pDoc )
    {
        pDocEventConfig = pDoc ? pDoc->GetEventConfig_Impl() : NULL;
        if ( !pDocEventConfig )
            return TRUE;            // IsSecure ???

        // Wenn das Macro sowieso nicht ausgef"uhrt wird, mu\s auch nicht gefragt werden
        if ( pMacro->GetScriptType() == STARBASIC && !pDoc->IsSecure() )
            return FALSE;

        // Bei dokumentgebundenen Macros WarningStatus checken
        // Wenn "Immer warnen" angeschaltet ist, Warnung ausgeben
        bWarn = pAppEventConfig->bAlwaysWarning;
        if ( bWarn )
        {
            SfxMacroQueryDlg_Impl aBox ( SfxResId( DLG_MACROQUERY ) );
            if ( aBox.Execute() )
                bWarn = FALSE;
        }
    }

    return !bWarn;
}

void SfxEventConfiguration::AddEvents( SfxMacroTabPage* pPage ) const
{
    DBG_ASSERT(pEventArr,"Keine Events angemeldet!");
    USHORT nCount = pEventArr->Count();
    for (USHORT n=1; n<nCount; n++)
        pPage->AddEvent( (*pEventArr)[n]->aEventName, (*pEventArr)[n]->nEventId );
}

SvxMacroTableDtor* SfxEventConfiguration::GetAppEventTable()
{
    return &pAppEventConfig->aMacroTable;
}

SvxMacroTableDtor* SfxEventConfiguration::GetDocEventTable( SfxObjectShell*pDoc )
{
    pDocEventConfig = pDoc ? pDoc->GetEventConfig_Impl() : NULL;
    if ( pDocEventConfig )
        return &pDocEventConfig->aMacroTable;
    return NULL;
}

void SfxEventConfiguration::SetAppEventTable( const SvxMacroTableDtor& rTable )
{
    pAppEventConfig->aMacroTable = rTable;
    pAppEventConfig->SetDefault(FALSE);
}

void SfxEventConfiguration::SetDocEventTable( SfxObjectShell *pDoc,
    const SvxMacroTableDtor& rTable )
{
    if ( pDoc )
    {
        pDocEventConfig = pDoc->GetEventConfig_Impl( TRUE );
        pDocEventConfig->aMacroTable = rTable;
        pDocEventConfig->SetDefault(FALSE);
    }
    else
        DBG_ERROR( "Kein Dokument!" );
}


