/*************************************************************************
 *
 *  $RCSfile: childwin.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: as $ $Date: 2000-11-08 14:25:41 $
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

#if SUPD<613//MUSTINI
#ifndef _SFXINIMGR_HXX //autogen
#include <svtools/iniman.hxx>
#endif
#endif
#ifndef _TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif
#ifndef _RCID_H
#include <vcl/rcid.h>
#endif

#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#pragma hdrstop

#include "childwin.hxx"
#include "app.hxx"
#include "arrdecl.hxx"
#include "bindings.hxx"
#include "module.hxx"
#include "dockwin.hxx"
#include "dispatch.hxx"
#include "appdata.hxx"
#include <workwin.hxx>

static const sal_uInt16 nVersion = 1;

DBG_NAME(SfxChildWindow)

SV_IMPL_PTRARR( SfxChildWinContextArr_Impl, SfxChildWinContextFactory* );

struct SfxChildWindow_Impl
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >             xFrame;
    SfxChildWinFactory* pFact;
    sal_Bool                bHideNotDelete;
    sal_Bool                bVisible;
    sal_Bool                bHideAtToggle;
    SfxModule*          pContextModule;
    SfxWorkWindow*      pWorkWin;
};

//=========================================================================
SfxChildWindow::SfxChildWindow(Window *pParentWindow, sal_uInt16 nId)
    : pParent(pParentWindow)
    , eChildAlignment(SFX_ALIGN_NOALIGNMENT)
    , nType(nId)
    , pWindow(0L)
{
    pImp = new SfxChildWindow_Impl;
    pImp->pFact = 0L;
    pImp->bHideNotDelete = sal_False;
    pImp->bHideAtToggle = sal_False;
    pImp->bVisible = sal_True;
    pImp->pContextModule = NULL;
    pImp->pWorkWin = NULL;

    pContext = 0L;
    DBG_CTOR(SfxChildWindow,0);
}

//-------------------------------------------------------------------------
SfxChildWindow::~SfxChildWindow()
{
    DBG_DTOR(SfxChildWindow,0);
    if ( pContext )
        delete pContext;
    delete pWindow;
    delete pImp;
}

//-------------------------------------------------------------------------
SfxChildWindow* SfxChildWindow::CreateChildWindow( sal_uInt16 nId,
        Window *pParent, SfxBindings* pBindings, SfxChildWinInfo& rInfo)
{
    SfxChildWindow *pChild=0;
    SfxChildWinFactory* pFact=0;
    sal_uInt16 nOldMode = Application::GetSystemWindowMode();

    // Zuerst ChildWindow im SDT suchen; "Uberlagerungen m"ussen mit einem
    // ChildWindowContext realisiert werden
    SfxApplication *pApp = SFX_APP();
    SfxChildWinFactArr_Impl &rFactories = pApp->GetChildWinFactories_Impl();
    for ( sal_uInt16 nFactory = 0; nFactory < rFactories.Count(); ++nFactory )
    {
        pFact = rFactories[nFactory];
        if ( pFact->nId == nId )
        {
            SfxChildWinInfo& rFactInfo = pFact->aInfo;
            if ( rInfo.bVisible )
            {
                if ( pBindings )
                    pBindings->ENTERREGISTRATIONS();
                rFactInfo.nFlags |= rInfo.nFlags & SFX_CHILDWIN_FORCEDOCK;
                SfxChildWinInfo aInfo = rFactInfo;
                Application::SetSystemWindowMode( SYSTEMWINDOW_MODE_NOAUTOMODE );
                pChild = pFact->pCtor( pParent, nId, pBindings, &aInfo );
                Application::SetSystemWindowMode( nOldMode );
                rFactInfo.nFlags &= ~(rInfo.nFlags & SFX_CHILDWIN_FORCEDOCK );
                if ( pBindings )
                    pBindings->LEAVEREGISTRATIONS();
            }

            break;
        }
    }

    SfxDispatcher *pDisp = pBindings->GetDispatcher_Impl();
    SfxModule *pMod = pDisp ? pApp->GetActiveModule( pDisp->GetFrame() ) :0;
    if ( !pChild &&  pMod )
    {
        SfxChildWinFactArr_Impl *pFactories = pMod->GetChildWinFactories_Impl();
        if ( pFactories )
        {
            SfxChildWinFactArr_Impl &rFactories = *pFactories;
            for ( sal_uInt16 nFactory = 0; nFactory < rFactories.Count(); ++nFactory )
            {
                pFact = rFactories[nFactory];
                if ( pFact->nId == nId )
                {
                    SfxChildWinInfo& rFactInfo = pFact->aInfo;
                    if ( rInfo.bVisible )
                    {
                        if ( pBindings )
                            pBindings->ENTERREGISTRATIONS();
                        rFactInfo.nFlags |= rInfo.nFlags & SFX_CHILDWIN_FORCEDOCK;
                        SfxChildWinInfo aInfo = rFactInfo;
                        Application::SetSystemWindowMode( SYSTEMWINDOW_MODE_NOAUTOMODE );
                        pChild = pFact->pCtor( pParent, nId, pBindings, &aInfo );
                        Application::SetSystemWindowMode( nOldMode );
                        rFactInfo.nFlags &= ~(rInfo.nFlags & SFX_CHILDWIN_FORCEDOCK );
                        if ( pBindings )
                            pBindings->LEAVEREGISTRATIONS();
                    }

                    break;
                }
            }
        }
    }

    if ( pChild )
        pChild->SetFactory_Impl( pFact );

    DBG_ASSERT(pFact && (pChild || !rInfo.bVisible), "ChildWindow-Typ nicht registriert!");

    if ( pChild && !pChild->pWindow )
    {
        DELETEZ(pChild);
        DBG_WARNING("ChildWindow hat kein Fenster!");
    }

    return pChild;
}

//-------------------------------------------------------------------------
void SfxChildWindow::SaveStatus(const SfxChildWinInfo& rInfo)
{
//    if ( SFX_APP()->Get_Impl()->bBean )
//        return;
#if SUPD<613//MUSTINI
/* TODO: Which key we can use to save this information in our configuration? */
    SfxIniManager *pIniMgr = SFX_INIMANAGER();
    char cToken = pIniMgr->GetToken();;
    String aWinData('V');
    aWinData += String::CreateFromInt32( nVersion );
    aWinData += cToken;
    aWinData += String( pIniMgr->GetString( rInfo.aPos, rInfo.aSize ) );
    aWinData += cToken;
    aWinData += rInfo.bVisible ? 'V' : 'H';
    aWinData += cToken;
    aWinData += String::CreateFromInt32(rInfo.nFlags);
    if ( rInfo.aExtraString.Len() )
    {
        aWinData += cToken;
        aWinData += rInfo.aExtraString;
    }
    pIniMgr->Set( aWinData, SFX_KEY_WINDOW, GetType() );
#endif
    pImp->pFact->aInfo = rInfo;
}

//-------------------------------------------------------------------------
void SfxChildWindow::SetAlignment(SfxChildAlignment eAlign)
{
    DBG_CHKTHIS(SfxChildWindow,0);

    eChildAlignment = eAlign;
}

//-------------------------------------------------------------------------
void SfxChildWindow::SetPosSizePixel(const Point& rPoint, Size& rSize)
{
    DBG_CHKTHIS(SfxChildWindow,0);

    pWindow->SetPosSizePixel(rPoint, rSize);
}

//-------------------------------------------------------------------------
SfxChildWinInfo SfxChildWindow::GetInfo() const
{
    DBG_CHKTHIS(SfxChildWindow,0);

    SfxChildWinInfo aInfo;
    aInfo.aPos  = pWindow->GetPosPixel();
    aInfo.aSize = pWindow->GetSizePixel();
    aInfo.bVisible = pImp->bVisible;
    aInfo.nFlags = 0;
    return aInfo;
}

//-------------------------------------------------------------------------
sal_uInt16 SfxChildWindow::GetPosition()
{
    return pImp->pFact->nPos;
}

//-------------------------------------------------------------------------
void SfxChildWindow::InitializeChildWinFactory_Impl( sal_uInt16 nId, SfxChildWinInfo& rInfo )
{
    // Konfiguration aus Ini-Manager laden
#if SUPD<613//MUSTINI
    SfxIniManager *pAppIniMgr = SFX_APP()->GetAppIniManager();
    String aWinData( pAppIniMgr->Get( SFX_KEY_WINDOW, nId ) );
#else
    String aWinData;
#endif

    if ( /*!SFX_APP()->Get_Impl()->bBean && */ aWinData.Len() )
    {
        // Nach Versionskennung suchen
        if ( aWinData.GetChar((sal_uInt16)0) != 0x0056 ) // 'V' = 56h
            // Keine Versionskennung, daher nicht verwenden
            return;

        // 'V' l"oschen
        aWinData.Erase(0,1);

        // Version lesen
#if SUPD<613//MUSTINI
        char cToken = pAppIniMgr->GetToken();
#else
        char cToken = ',';
#endif
        sal_uInt16 nPos = aWinData.Search( cToken );
        sal_uInt16 nVersion = (sal_uInt16) aWinData.Copy( 0, nPos+1 ).ToInt32();
        aWinData.Erase(0,nPos+1);

#if SUPD<613//MUSTINI
        // Gr"o\se und Position laden
        Point aPos;
        Size  aSize;
        if ( pAppIniMgr->GetPosSize(
                aWinData.Copy( 0 , aWinData.Search( cToken ) ),
                aPos, aSize ) )
        {
            rInfo.aPos = aPos;
            rInfo.aSize = aSize;
        }
#endif
        aWinData.Erase(0,aWinData.Search(cToken));

        // Sichtbarkeit laden: ist als ein char codiert
        rInfo.bVisible = (aWinData.Copy(1,1) == 0x0056); // 'V' = 56h
        aWinData.Erase(1,1);
#if SUPD<613//MUSTINI
        nPos = aWinData.Search( pAppIniMgr->GetToken(), 2 );
#else
        nPos = aWinData.Search( cToken, 2 );
#endif
        if (nPos != STRING_NOTFOUND)
        {
            // es gibt noch Extra-Information
            rInfo.nFlags   = aWinData.Copy( 2 , nPos-2 ).ToInt32();
            aWinData.Erase(2,nPos-2);
            rInfo.aExtraString = aWinData.Copy( 3 );
            aWinData.Erase(3);
        }
        else
        {
            // Zeile ist nach Flags zu Ende
            rInfo.nFlags   = aWinData.Copy( 2 ).ToInt32();
            aWinData.Erase(2);
        }
    }

//    if ( SFX_APP()->Get_Impl()->bBean )
//    {
//        if ( rInfo.bVisible != 42 )
//            rInfo.bVisible = sal_False;
//    }
}

void SfxChildWindow::CreateContext( sal_uInt16 nContextId, SfxBindings& rBindings )
{
    SfxChildWindowContext *pCon = NULL;
    SfxChildWindow *pChild=0;
    SfxChildWinFactory* pFact=0;
    SfxApplication *pApp = SFX_APP();
    SfxDispatcher *pDisp = rBindings.GetDispatcher_Impl();
    SfxModule *pMod = pDisp ? pApp->GetActiveModule( pDisp->GetFrame() ) :0;
    if ( pMod )
    {
        SfxChildWinFactArr_Impl *pFactories = pMod->GetChildWinFactories_Impl();
        if ( pFactories )
        {
            SfxChildWinFactArr_Impl &rFactories = *pFactories;
            for ( sal_uInt16 nFactory = 0; nFactory < rFactories.Count(); ++nFactory )
            {
                pFact = rFactories[nFactory];
                if ( pFact->nId == GetType() )
                {
                    DBG_ASSERT( pFact->pArr, "Kein Kontext angemeldet!" );
                    if ( !pFact->pArr )
                        break;

                    SfxChildWinContextFactory *pConFact=0;
                    for ( sal_uInt16 n=0; n<pFact->pArr->Count(); ++n )
                    {
                        pConFact = (*pFact->pArr)[n];
                        rBindings.ENTERREGISTRATIONS();
                        if ( pConFact->nContextId == nContextId )
                        {
                            SfxChildWinInfo aInfo = pFact->aInfo;
                            pCon = pConFact->pCtor( GetWindow(), &rBindings, &aInfo );
                            pCon->nContextId = pConFact->nContextId;
                            pImp->pContextModule = pMod;
                        }
                        rBindings.LEAVEREGISTRATIONS();
                    }
                    break;
                }
            }
        }
    }

    if ( !pCon )
    {
        SfxChildWinFactArr_Impl &rFactories = pApp->GetChildWinFactories_Impl();
        for ( sal_uInt16 nFactory = 0; nFactory < rFactories.Count(); ++nFactory )
        {
            pFact = rFactories[nFactory];
            if ( pFact->nId == GetType() )
            {
                DBG_ASSERT( pFact->pArr, "Kein Kontext angemeldet!" );
                if ( !pFact->pArr )
                    break;

                SfxChildWinContextFactory *pConFact=0;
                for ( sal_uInt16 n=0; n<pFact->pArr->Count(); ++n )
                {
                    pConFact = (*pFact->pArr)[n];
                    rBindings.ENTERREGISTRATIONS();
                    if ( pConFact->nContextId == nContextId )
                    {
                        SfxChildWinInfo aInfo = pFact->aInfo;
                        pCon = pConFact->pCtor( GetWindow(), &rBindings, &aInfo );
                        pCon->nContextId = pConFact->nContextId;
                        pImp->pContextModule = NULL;
                    }
                    rBindings.LEAVEREGISTRATIONS();
                }
                break;
            }
        }
    }

    if ( !pCon )
    {
        DBG_ERROR( "Kein geeigneter Context gefunden!" );
        return;
    }

    if ( pContext )
        delete( pContext );
    pContext = pCon;
    pContext->GetWindow()->SetSizePixel( pWindow->GetOutputSizePixel() );
    pContext->GetWindow()->Show();
}

SfxChildWindowContext::SfxChildWindowContext( sal_uInt16 nId )
    : pWindow( NULL )
    , nContextId( nId )
{
}

SfxChildWindowContext::~SfxChildWindowContext()
{
    delete pWindow;
}

FloatingWindow* SfxChildWindowContext::GetFloatingWindow() const
{
    Window *pParent = pWindow->GetParent();
    if ( pParent->GetType() == RSC_DOCKINGWINDOW || pParent->GetType() == RSC_TOOLBOX )
    {
        return ((DockingWindow*)pParent)->GetFloatingWindow();
    }
    else if ( pParent->GetType() == RSC_FLOATINGWINDOW )
    {
        return (FloatingWindow*) pParent;
    }
    else
    {
        DBG_ERROR("Kein FloatingWindow-Context!");
        return NULL;
    }
}

SfxChildAlignment SfxChildWindowContext::GetAlignment() const
{
    Window *pParent = pWindow->GetParent();
    if ( pParent->GetType() == RSC_DOCKINGWINDOW )
    {
        return ((SfxDockingWindow*)pParent)->GetAlignment();
    }
    else if ( pParent->GetType() == RSC_TOOLBOX )
    {
        HACK(noch nicht verwendet und noch nicht implementiert);
        return SFX_ALIGN_NOALIGNMENT;
    }
    else
        return SFX_ALIGN_NOALIGNMENT;
}

void SfxChildWindowContext::Resizing( Size& rSize )
{
}

sal_Bool SfxChildWindowContext::Close()
{
    return sal_True;
}

void SfxChildWindow::SetFactory_Impl( SfxChildWinFactory *pF )
{
    pImp->pFact = pF;
}

void SfxChildWindow::SetHideNotDelete( sal_Bool bOn )
{
    pImp->bHideNotDelete = bOn;
}

sal_Bool SfxChildWindow::IsHideNotDelete() const
{
    return pImp->bHideNotDelete;
}

void SfxChildWindow::SetHideAtToggle( sal_Bool bOn )
{
    pImp->bHideAtToggle = bOn;
}

sal_Bool SfxChildWindow::IsHideAtToggle() const
{
    return pImp->bHideAtToggle;
}

sal_Bool SfxChildWinInfo::GetExtraData_Impl
(
    SfxChildAlignment   *pAlign,
    SfxChildAlignment   *pLastAlign,
    Size                *pSize,
    sal_uInt16              *pLine,
    sal_uInt16              *pPos
)   const
{
    // ung"ultig?
    if ( !aExtraString.Len() )
        return sal_False;
    String aStr;
    sal_uInt16 nPos = aExtraString.SearchAscii("AL:");
    if ( nPos == STRING_NOTFOUND )
        return sal_False;

    // Versuche, den Alignment-String "ALIGN:(...)" einzulesen; wenn
    // er nicht vorhanden ist, liegt eine "altere Version vor
    if ( nPos != STRING_NOTFOUND )
    {
        sal_uInt16 n1 = aExtraString.Search('(', nPos);
        if ( n1 != STRING_NOTFOUND )
        {
            sal_uInt16 n2 = aExtraString.Search(')', n1);
            if ( n2 != STRING_NOTFOUND )
            {
                // Alignment-String herausschneiden
                aStr = aExtraString.Copy(nPos, n2 - nPos + 1);
                aStr.Erase(nPos, n1-nPos+1);
            }
        }
    }

    // Zuerst das Alignment extrahieren
    if ( !aStr.Len() )
        return sal_False;
    if ( pAlign )
        *pAlign = (SfxChildAlignment) (sal_uInt16) aStr.ToInt32();

    // Dann das LastAlignment
    nPos = aStr.Search(',');
    if ( nPos == STRING_NOTFOUND )
        return sal_False;
    aStr.Erase(0, nPos+1);
    if ( pLastAlign )
        *pLastAlign = (SfxChildAlignment) (sal_uInt16) aStr.ToInt32();

    // Dann die Splitting-Informationen
    nPos = aStr.Search(',');
    if ( nPos == STRING_NOTFOUND )
        // Dockt nicht in einem Splitwindow
        return sal_True;
    aStr.Erase(0, nPos+1);
#if SUPD<613//MUSTINI
    SfxIniManager *pAppIniMgr = SFX_APP()->GetAppIniManager();
    Point aPos;
    Size aSize;
    if ( pAppIniMgr->GetPosSize( aStr, aPos, aSize ) )
    {
        if ( pSize )
            *pSize = aSize;
        if ( pLine )
            *pLine = (sal_uInt16) aPos.X();
        if ( pPos )
            *pPos = (sal_uInt16) aPos.Y();
        return sal_True;
    }
#endif
    return sal_False;
}

sal_Bool SfxChildWindow::IsVisible() const
{
    return pImp->bVisible;
}

void SfxChildWindow::SetVisible_Impl( sal_Bool bVis )
{
    pImp->bVisible = bVis;
}

void SfxChildWindow::Hide()
{
    switch ( pWindow->GetType() )
    {
        case RSC_DOCKINGWINDOW :
            ((DockingWindow*)pWindow)->Hide();
            break;
        case RSC_TOOLBOX :
            ((ToolBox*)pWindow)->Hide();
            break;
        default:
            pWindow->Hide();
            break;
    }
}



void SfxChildWindow::Show()
{
    switch ( pWindow->GetType() )
    {
        case RSC_DOCKINGWINDOW :
            ((DockingWindow*)pWindow)->Show();
            break;
        case RSC_TOOLBOX :
            ((ToolBox*)pWindow)->Show();
            break;
        default:
            pWindow->Show();
            break;
    }
}

Window* SfxChildWindow::GetContextWindow( SfxModule *pModule ) const
{
    return pModule == pImp->pContextModule && pContext ? pContext->GetWindow(): 0;
}

void SfxChildWindow::SetWorkWindow_Impl( SfxWorkWindow* pWin )
{
    pImp->pWorkWin = pWin;
    if ( pWindow->HasChildPathFocus() )
        pImp->pWorkWin->SetActiveChild_Impl( pWindow );
}

SfxWorkWindow* SfxChildWindow::GetWorkWindow_Impl() const
{
    return pImp->pWorkWin;
}

void SfxChildWindow::Activate_Impl()
{
    if(pImp->pWorkWin!=NULL) //@#60568#
        pImp->pWorkWin->SetActiveChild_Impl( pWindow );
}

void SfxChildWindow::Deactivate_Impl()
{
//  pImp->pWorkWin->SetActiveChild_Impl( NULL );
}

sal_Bool SfxChildWindow::QueryClose()
{
    if ( pImp->xFrame.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >  xCtrl = pImp->xFrame->getController();
        if ( xCtrl.is() )
            xCtrl->suspend( sal_True );
    }

    return sal_True;
}

::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >  SfxChildWindow::GetFrame()
{
    return pImp->xFrame;
}

void SfxChildWindow::SetFrame( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > & rFrame )
{
    pImp->xFrame = rFrame;
}

sal_Bool SfxChildWindow::CanGetFocus() const
{
    return !(pImp->pFact->aInfo.nFlags & SFX_CHILDWIN_CANTGETFOCUS);
}

