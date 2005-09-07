/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: appchild.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:32:26 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef GCC
#pragma hdrstop
#endif

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
//#include "ipfrm.hxx"
//#include "ipenv.hxx"
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

SfxWorkWindow* SfxApplication::GetWorkWindow_Impl(const SfxViewFrame *pFrame) const
{
    if ( pFrame )
        return pFrame->GetFrame()->GetWorkWindow_Impl();
    else if ( pViewFrame )
        return pViewFrame->GetFrame()->GetWorkWindow_Impl();
    else
        return NULL;
}

//--------------------------------------------------------------------

SfxHelpPI* SfxApplication::GetHelpPI()
{
    return NULL;
}
