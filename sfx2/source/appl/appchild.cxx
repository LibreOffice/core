/*************************************************************************
 *
 *  $RCSfile: appchild.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:26 $
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

#pragma hdrstop

#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif

#include "app.hxx"
#include "appdata.hxx"
#include "workwin.hxx"
#include "childwin.hxx"
#include "arrdecl.hxx"
#include "templdlg.hxx"
#include "sfxhelp.hxx"
#include "ipfrm.hxx"
#include "ipenv.hxx"
#include "request.hxx"
#include "bindings.hxx"
#include "dispatch.hxx"
#include "sfxtypes.hxx"
#include "module.hxx"
#include "sfxsids.hrc"

//=========================================================================

ResId SfxApplication::GetCurrentObjectBar( USHORT nPosId ) const

/*  [Beschreibung]

    Mit dieser Methode kann die ResId der an der angegebenen Positions-Nummer
    aktuell stehenden Symbol-Leiste erfragt werden. Als Positions-Nummern
    k"onnen verwendet werden:

        SFX_OBJECTBAR_APPLICATION
        SFX_OBJECTBAR_OBJECT
        SFX_OBJECTBAR_TOOLS
        SFX_OBJECTBAR_MACRO
        SFX_OBJECTBAR_FULLSCREEN
        SFX_OBJECTBAR_RECORDING
        SFX_OBJECTBAR_OPTIONS
        SFX_OBJECTBAR_USERDEF1
        SFX_OBJECTBAR_USERDEF2
        SFX_OBJECTBAR_USERDEF3

    Ist an der Position keine Symbol-Leiste vorhanden, dann wird eine ResId
    mit einer Id von 0 und ohne ResManager zur"uckgegeben.


    [Anmerkung]

    Da intern keine fertige ResId verf"ugbar ist, mu\s leider eine Instanz
    returnt werden.
*/

{
    ResId aResId(0, 0);
    SfxWorkWindow* pWork = GetWorkWindow_Impl(SfxViewFrame::Current());
    if ( pWork )
        pWork->GetObjectBar_Impl(nPosId, aResId);
    return aResId;
}

//--------------------------------------------------------------------

void SfxApplication::RegisterChildWindow( SfxModule *pMod, SfxChildWinFactory *pFact )
{
    if ( pMod )
    {
        pMod->RegisterChildWindow( pFact );
        return;
    }

    if (!pAppData_Impl->pFactArr)
        pAppData_Impl->pFactArr = new SfxChildWinFactArr_Impl;

//#ifdef DBG_UTIL
    for (USHORT nFactory=0; nFactory<pAppData_Impl->pFactArr->Count(); ++nFactory)
    {
        if (pFact->nId ==  (*pAppData_Impl->pFactArr)[nFactory]->nId)
        {
            pAppData_Impl->pFactArr->Remove( nFactory );
//          DBG_ERROR("ChildWindow mehrfach registriert!");
//          return;
        }
    }
//#endif

    pAppData_Impl->pFactArr->C40_INSERT(
        SfxChildWinFactory, pFact, pAppData_Impl->pFactArr->Count() );
}

void SfxApplication::RegisterChildWindowContext( SfxModule *pMod, USHORT nId,
        SfxChildWinContextFactory *pFact)
{
    SfxChildWinFactArr_Impl *pFactories;
    SfxChildWinFactory *pF = NULL;
    if ( pMod )
    {
        // Modul "ubergeben, ChildwindowFactory dort suchen
        pFactories = pMod->GetChildWinFactories_Impl();
        if ( pFactories )
        {
            USHORT nCount = pFactories->Count();
            for (USHORT nFactory=0; nFactory<nCount; ++nFactory)
            {
                SfxChildWinFactory *pFac = (*pFactories)[nFactory];
                if ( nId == pFac->nId )
                {
                    // Factory gefunden, Context dort registrieren
                    pF = pFac;
                    break;
                }
            }
        }
    }

    if ( !pF )
    {
        // Factory an der Application suchen
        DBG_ASSERT( pAppData_Impl, "Keine AppDaten!" );
        DBG_ASSERT( pAppData_Impl->pFactArr, "Keine Factories!" );

        pFactories = pAppData_Impl->pFactArr;
        USHORT nCount = pFactories->Count();
        for (USHORT nFactory=0; nFactory<nCount; ++nFactory)
        {
            SfxChildWinFactory *pFac = (*pFactories)[nFactory];
            if ( nId == pFac->nId )
            {
                if ( pMod )
                {
                    // Wenn der Context von einem Modul registriert wurde,
                    // mu\s die ChildwindowFactory auch dort zur Verf"ugung
                    // stehen, sonst m"u\ste sich die Contextfactory im DLL-Exit
                    // wieder abmelden !
                    pF = new SfxChildWinFactory( pFac->pCtor, pFac->nId,
                            pFac->nPos );
                    pMod->RegisterChildWindow( pF );
                }
                else
                    pF = pFac;
                break;
            }
        }
    }

    if ( pF )
    {
        if ( !pF->pArr )
            pF->pArr = new SfxChildWinContextArr_Impl;
        pF->pArr->C40_INSERT( SfxChildWinContextFactory, pFact, pF->pArr->Count() );
        return;
    }

    DBG_ERROR( "Kein ChildWindow fuer diesen Context!" );
}

//--------------------------------------------------------------------

SfxChildWinFactArr_Impl& SfxApplication::GetChildWinFactories_Impl() const
{
    return ( *(pAppData_Impl->pFactArr));
}

//--------------------------------------------------------------------
#if SUPD<604
void SfxApplication::SetChildWindow(USHORT nId, BOOL bOn)
{
    if ( pViewFrame )
        pViewFrame->SetChildWindow( nId, bOn );
}

//--------------------------------------------------------------------

void SfxApplication::ToggleChildWindow(USHORT nId)
{

    if ( pViewFrame )
        pViewFrame->ToggleChildWindow( nId );
}

//--------------------------------------------------------------------

BOOL SfxApplication::HasChildWindow( USHORT nId )
{
    if ( pViewFrame )
        return pViewFrame->HasChildWindow(nId);
    else
        return FALSE;
}

//--------------------------------------------------------------------

BOOL SfxApplication::KnowsChildWindow( USHORT nId )
{
    if ( pViewFrame )
        return pViewFrame->KnowsChildWindow(nId);
    else
        return FALSE;
}

//--------------------------------------------------------------------

void SfxApplication::ShowChildWindow( USHORT nId, BOOL bVisible )
{
    if ( pViewFrame )
        pViewFrame->ShowChildWindow(nId, bVisible);
}

//--------------------------------------------------------------------

SfxChildWindow* SfxApplication::GetChildWindow(USHORT nId)
{
    if ( pViewFrame )
        return pViewFrame->GetChildWindow(nId);
    else
        return NULL;
}

#endif

//--------------------------------------------------------------------

SfxTemplateDialog* SfxApplication::GetTemplateDialog()
{
    if ( pViewFrame )
    {
        SfxChildWindow *pChild = pViewFrame->GetChildWindow(SfxTemplateDialogWrapper::GetChildWindowId());
        return pChild ? (SfxTemplateDialog*) pChild->GetWindow() : 0;
    }

    return NULL;
}

//--------------------------------------------------------------------

SfxHelpPI* SfxApplication::GetHelpPI()
{
    if ( pViewFrame )
    {
        SfxChildWindow *pChild = pViewFrame->GetChildWindow(SfxHelpPIWrapper::GetChildWindowId());
        return pChild ? (SfxHelpPI*) pChild->GetWindow() : 0;
    }

    return NULL;
}

//--------------------------------------------------------------------

SfxWorkWindow* SfxApplication::GetWorkWindow_Impl(const SfxViewFrame *pFrame) const
{
    SfxInPlaceFrame *pIPFrame = PTR_CAST(SfxInPlaceFrame,pFrame);
    if ( pIPFrame )
        return pIPFrame->GetEnv_Impl()->GetWorkWindow();
    else
    {
        if ( pFrame )
            return pFrame->GetFrame()->GetWorkWindow_Impl();
        else if ( pViewFrame )
        {
            SfxInPlaceFrame *pIPFrame = PTR_CAST(SfxInPlaceFrame,pViewFrame);
            if ( pIPFrame )
                return pViewFrame->GetParentViewFrame_Impl()->GetFrame()->GetWorkWindow_Impl();
//                return pIPFrame->GetEnv_Impl()->GetWorkWindow();
            else
                return pViewFrame->GetFrame()->GetWorkWindow_Impl();
        }
        else
            return NULL;
    }
}

//--------------------------------------------------------------------
#if SUPD<604
void SfxApplication::ChildWindowExecute( SfxRequest &rReq )

/*  [Beschreibung]

    Diese Methode kann in der Execute-Methode f"ur das ein- und ausschalten
    von Child-Windows eingesetzt werden, um dieses inkl. API-Anbindung zu
    implementieren.

    Einfach in der IDL als 'ExecuteMethod' eintragen.
*/

{
    // Parameter auswerten
    USHORT nSID = rReq.GetSlot();

    SFX_REQUEST_ARG(rReq, pShowItem, SfxBoolItem, nSID, FALSE);
    BOOL bShow = FALSE;
    BOOL bHasChild = HasChildWindow(nSID);
    bShow = pShowItem ? pShowItem->GetValue() : !bHasChild;

    if ( bShow && ( nSID == SID_HELP_PI ) )
    {
        // Nur starten wenn Hilfe installiert...
        SfxHelp_Impl* pHelp = (SfxHelp_Impl*)Application::GetHelp();
        if ( !pHelp || !pHelp->CheckHelpFile( TRUE ) )
            return;
    }

    // ausf"uhren
    if ( !pShowItem || bShow != bHasChild )
    {
        // Vor dem Toggle ausf"uhren, denn hartes Anschalten hat Vorrang
        // vor weichem Anschalten
        if ( nSID == SID_BROWSER )
            SfxFrame::BeamerSwitched_Impl( bShow );

        ToggleChildWindow( nSID );

        if ( bShow && nSID == SID_BROWSER )
        {
            // Beamer soll sichtbar sein, da explizit eingeschaltet
            SfxChildWindow* pChild = SFX_APP()->GetChildWindow( SID_BROWSER );
            if ( pChild )
            {
//                SfxExplorerHorizDockWnd_Impl* pBeamer = (SfxExplorerHorizDockWnd_Impl*) pChild->GetWindow();
//                pBeamer->AutoShow_Impl( TRUE );
            }
        }
    }

    GetBindings().Invalidate( nSID );
    GetDispatcher().Update_Impl( TRUE );

    if ( bShow && ( nSID == SID_HELP_PI ) )
        GetpApp()->FocusChanged();  // Hilfe passend zum FocusWindow...

    // ggf. recorden
    if ( !rReq.IsAPI() )
        rReq.AppendItem( SfxBoolItem( nSID, bShow ) );

    rReq.Done();
}

//--------------------------------------------------------------------

void SfxApplication::ChildWindowState( SfxItemSet& rState )

/*  [Beschreibung]

    Diese Methode kann in der Status-Methode f"ur das Ein- und Ausschalt-
    Zustand von Child-Windows eingesetzt werden, um dieses zu implementieren.

    Einfach in der IDL als 'StateMethod' eintragen.
*/

{
    SfxWhichIter aIter( rState );
    for ( USHORT nSID = aIter.FirstWhich(); nSID; nSID = aIter.NextWhich() )
    {
        if ( nSID == SID_CUSTOMIZETOOLBOX )
        {
            if ( pViewFrame &&
                 pViewFrame->GetFrame()->HasComponent() &&
                 pViewFrame->GetViewShell()->GetMenuBar_Impl() )
                rState.DisableItem(nSID);
        }
        else if ( nSID == SID_HYPERLINK_DIALOG )
        {
            const SfxPoolItem* pDummy = NULL;
            SfxItemState eState = GetDispatcher().QueryState( SID_HYPERLINK_SETLINK, pDummy );
            if ( SFX_ITEM_DISABLED == eState )
                rState.DisableItem(nSID);
            else
            {
                if ( KnowsChildWindow(nSID) )
                    rState.Put( SfxBoolItem( nSID, HasChildWindow(nSID)) );
                else
                    rState.DisableItem(nSID);
            }
        }
        else if ( nSID == SID_MAIL_CHILDWIN )
//! (pb) temporary till the implementation is finished
            rState.DisableItem(nSID);
        else if ( nSID == SID_HELP_PI )
//! (pb) what about help?
            rState.DisableItem(nSID);
        else if ( KnowsChildWindow(nSID) )
            rState.Put( SfxBoolItem( nSID, HasChildWindow(nSID) ) );
        else
            rState.DisableItem(nSID);
    }
}

#endif
