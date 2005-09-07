/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewsh.cxx,v $
 *
 *  $Revision: 1.57 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:33:05 $
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

#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif
#ifndef _EHDL_HXX
#include <svtools/ehdl.hxx>
#endif

#ifndef _COM_SUN_STAR_FRAME_XLAYOUTMANAGER_HPP_
#include <com/sun/star/frame/XLayoutManager.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_EMBEDSTATES_HPP_
#include <com/sun/star/embed/EmbedStates.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_EMBEDMISC_HPP_
#include <com/sun/star/embed/EmbedMisc.hpp>
#endif

#include <tools/urlobj.hxx>
#include <svtools/pathoptions.hxx>
#include <svtools/miscopt.hxx>
#include <svtools/soerr.hxx>
#ifndef INCLUDED_SVTOOLS_INTERNALOPTIONS_HXX
#include <svtools/internaloptions.hxx>
#endif

#include <svtools/javaoptions.hxx>
#include <basic/basmgr.hxx>
#include <basic/sbuno.hxx>
#include <framework/actiontriggerhelper.hxx>
#include <comphelper/processfactory.hxx>

#ifndef GCC
#pragma hdrstop
#endif

#include "viewsh.hxx"
#include "viewimp.hxx"
#include "sfxresid.hxx"
#include "request.hxx"
#include "templdlg.hxx"
#include "printer.hxx"
#include "docfile.hxx"
#include "dispatch.hxx"
#include "arrdecl.hxx"
#include "intfrm.hxx"
#include "docfac.hxx"
#include "view.hrc"
#include "objuno.hxx"
#include "sfxlocal.hrc"
#include "sfxbasecontroller.hxx"
#include "topfrm.hxx"
#include "mailmodel.hxx"
#include "event.hxx"
#include "appdata.hxx"
#include "fcontnr.hxx"
#include "ipclient.hxx"
#include "workwin.hxx"

// #110897#
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;

//=========================================================================
DBG_NAME(SfxViewShell);

#define SfxViewShell
#include "sfxslots.hxx"

//=========================================================================

SFX_IMPL_INTERFACE(SfxViewShell,SfxShell,SfxResId(0))
{
    SFX_CHILDWINDOW_REGISTRATION( SID_MAIL_CHILDWIN );
}

TYPEINIT2(SfxViewShell,SfxShell,SfxListener);

//--------------------------------------------------------------------

void SfxViewShell::ExecMisc_Impl( SfxRequest &rReq )
{
    const USHORT nId = rReq.GetSlot();
    switch( nId )
    {
        case SID_STYLE_FAMILY :
        {
            SFX_REQUEST_ARG(rReq, pItem, SfxUInt16Item, nId, FALSE);
            if (pItem)
                pImp->nFamily = pItem->GetValue();
            break;
        }

        case SID_STYLE_CATALOG:
        {
            SfxTemplateCatalog aCatalog(
                SFX_APP()->GetTopWindow(), &GetViewFrame()->GetBindings());
            aCatalog.Execute();
            rReq.Ignore();
            break;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        case SID_MAIL_SENDDOCASPDF:
        case SID_MAIL_SENDDOC:
        {
            SfxObjectShell* pDoc = GetObjectShell();
            if ( pDoc && pDoc->QueryHiddenInformation(
                            WhenSaving, &GetViewFrame()->GetWindow() ) != RET_YES )
                break;

            if ( SvtInternalOptions().MailUIEnabled() )
            {
                GetViewFrame()->SetChildWindow( SID_MAIL_CHILDWIN, TRUE );
            }
            else
            {
                SfxMailModel_Impl   aModel( &GetViewFrame()->GetBindings() );

                SFX_REQUEST_ARG(rReq, pMailSubject, SfxStringItem, SID_MAIL_SUBJECT, FALSE );
                if ( pMailSubject )
                    aModel.SetSubject( pMailSubject->GetValue() );

                SFX_REQUEST_ARG(rReq, pMailRecipient, SfxStringItem, SID_MAIL_RECIPIENT, FALSE );
                if ( pMailRecipient )
                {
                    String aRecipient( pMailRecipient->GetValue() );
                    String aMailToStr( String::CreateFromAscii( "mailto:" ));

                    if ( aRecipient.Search( aMailToStr ) == 0 )
                        aRecipient = aRecipient.Erase( 0, aMailToStr.Len() );
                    aModel.AddAddress( aRecipient, SfxMailModel_Impl::ROLE_TO );
                }

                SfxMailModel_Impl::SendMailResult eResult = SfxMailModel_Impl::SEND_MAIL_ERROR;
                if ( nId == SID_MAIL_SENDDOCASPDF )
                    eResult = aModel.Send( SfxMailModel_Impl::TYPE_ASPDF );
                else
                    eResult = aModel.Send( SfxMailModel_Impl::TYPE_SELF );

                if ( eResult == SfxMailModel_Impl::SEND_MAIL_ERROR )
                {
                    InfoBox aBox( SFX_APP()->GetTopWindow(), SfxResId( MSG_ERROR_SEND_MAIL ));
                    aBox.Execute();
                    rReq.Ignore();
                }
                else
                    rReq.Done();
            }

            break;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        case SID_PLUGINS_ACTIVE:
        {
            SFX_REQUEST_ARG(rReq, pShowItem, SfxBoolItem, nId, FALSE);
            BOOL bActive = pShowItem ? pShowItem->GetValue() : !pImp->bPlugInsActive;
            // ggf. recorden
            if ( !rReq.IsAPI() )
                rReq.AppendItem( SfxBoolItem( nId, bActive ) );

            // Jetzt schon DONE aufrufen, da die Argumente evtl. einen Pool
            // benutzen, der demn"achst weg ist
            rReq.Done(TRUE);

            // ausfuehren
            if ( !pShowItem || bActive != pImp->bPlugInsActive )
            {
                SfxFrame *pFrame = GetFrame()->GetTopFrame();
                if ( pFrame != GetFrame()->GetFrame() )
                {
                    // FramesetDocument
                    SfxViewShell *pShell = pFrame->GetCurrentViewFrame()->GetViewShell();
                    if ( pShell->GetInterface()->GetSlot( nId ) )
                        pShell->ExecuteSlot( rReq );
                    break;
                }

                SfxFrameIterator aIter( *pFrame );
                while ( pFrame )
                {
                    if ( pFrame->GetCurrentViewFrame() )
                    {
                        SfxViewShell *pView = pFrame->GetCurrentViewFrame()->GetViewShell();
                        if ( pView )
                        {
                            pView->pImp->bPlugInsActive = bActive;
                            Rectangle aVisArea = GetObjectShell()->GetVisArea();
                            VisAreaChanged(aVisArea);

                            // the plugins might need change in their state
                            SfxInPlaceClientList *pClients = pView->GetIPClientList_Impl(FALSE);
                            if ( pClients )
                            {
                                for (USHORT n=0; n < pClients->Count(); n++)
                                {
                                    SfxInPlaceClient* pIPClient = pClients->GetObject(n);
                                    if ( pIPClient )
                                        pView->CheckIPClient_Impl( pIPClient, aVisArea );
                                }
                            }
                        }
                    }

                    if ( !pFrame->GetParentFrame() )
                        pFrame = aIter.FirstFrame();
                    else
                        pFrame = aIter.NextFrame( *pFrame );
                }
            }

            break;
        }
    }
}

//--------------------------------------------------------------------

void SfxViewShell::GetState_Impl( SfxItemSet &rSet )
{
    DBG_CHKTHIS(SfxViewShell, 0);

    SfxWhichIter aIter( rSet );
    for ( USHORT nSID = aIter.FirstWhich(); nSID; nSID = aIter.NextWhich() )
    {
        switch ( nSID )
        {
            case SID_STYLE_CATALOG:
            {
                if ( !GetViewFrame()->KnowsChildWindow( SID_STYLE_DESIGNER ) )
                    rSet.DisableItem( nSID );
                break;
            }

            // Printer-Funktionen
            case SID_PRINTDOC:
            case SID_PRINTDOCDIRECT:
            case SID_SETUPPRINTER:
            {
                BOOL bEnabled = pImp->bCanPrint && !pImp->nPrinterLocks;
                if ( bEnabled )
                {
                    SfxPrinter *pPrinter = GetPrinter(FALSE);
                    bEnabled = !pPrinter || !pPrinter->IsPrinting();
                }
                if ( !bEnabled )
                {
                    rSet.DisableItem( SID_PRINTDOC );
                    rSet.DisableItem( SID_PRINTDOCDIRECT );
                    rSet.DisableItem( SID_SETUPPRINTER );
                }
                break;
            }

            // Mail-Funktionen
            case SID_MAIL_SENDDOCASPDF:
            case SID_MAIL_SENDDOC:
            {
                BOOL bEnable = !GetViewFrame()->HasChildWindow( SID_MAIL_CHILDWIN );
                if ( !bEnable )
                    rSet.DisableItem( nSID );
                break;
            }

            // PlugIns running
            case SID_PLUGINS_ACTIVE:
            {
                rSet.Put( SfxBoolItem( SID_PLUGINS_ACTIVE, !pImp->bPlugInsActive) );
                break;
            }
/*
            // SelectionText
            case SID_SELECTION_TEXT:
            {
                rSet.Put( SfxStringItem( SID_SELECTION_TEXT, GetSelectionText() ) );
                break;
            }

            // SelectionTextExt
            case SID_SELECTION_TEXT_EXT:
            {
                rSet.Put( SfxStringItem( SID_SELECTION_TEXT_EXT, GetSelectionText(TRUE) ) );
                break;
            }
*/
            case SID_STYLE_FAMILY :
            {
                rSet.Put( SfxUInt16Item( SID_STYLE_FAMILY, pImp->nFamily ) );
                break;
            }
        }
    }
}

//--------------------------------------------------------------------

void SfxViewShell::SetZoomFactor( const Fraction &rZoomX,
                                  const Fraction &rZoomY )
{
    DBG_ASSERT( GetWindow(), "no window" );
    MapMode aMap( GetWindow()->GetMapMode() );
    aMap.SetScaleX( rZoomX );
    aMap.SetScaleY( rZoomY );
    GetWindow()->SetMapMode( aMap );
}

//--------------------------------------------------------------------
ErrCode SfxViewShell::DoVerb(long nVerb)

/*  [Beschreibung]

    Virtuelle Methode, um am selektierten Objekt ein Verb auszuf"uhren.
    Da dieses Objekt nur den abgeleiteten Klassen bekannt ist, muss DoVerb
    dort "uberschrieben werden.

*/

{
    return ERRCODE_SO_NOVERBS;
}

//--------------------------------------------------------------------

void SfxViewShell::OutplaceActivated( sal_Bool bActive, SfxInPlaceClient* pClient )
{
    if ( !bActive )
        GetFrame()->GetFrame()->Appear();
}

//--------------------------------------------------------------------

void SfxViewShell::InplaceActivating( SfxInPlaceClient* pClient )
{
    // TODO/LATER: painting of the bitmap can be stopped, it is required if CLIPCHILDREN problem #i25788# is not solved,
    // but may be the bug will not affect the real office vcl windows, then it is not required
}

//--------------------------------------------------------------------

void SfxViewShell::InplaceDeactivated( SfxInPlaceClient* pClient )
{
    // TODO/LATER: paint the replacement image in normal way if the painting was stopped
}

//--------------------------------------------------------------------

void SfxViewShell::UIActivating( SfxInPlaceClient* pClient )
{
    uno::Reference < frame::XFrame > xOwnFrame( pFrame->GetFrame()->GetFrameInterface() );
    uno::Reference < frame::XFramesSupplier > xParentFrame( xOwnFrame->getCreator(), uno::UNO_QUERY );
    if ( xParentFrame.is() )
        xParentFrame->setActiveFrame( xOwnFrame );

    pFrame->GetBindings().HidePopups(TRUE);
    pFrame->GetDispatcher()->Update_Impl( TRUE );
}

//--------------------------------------------------------------------

void SfxViewShell::UIDeactivated( SfxInPlaceClient* pClient )
{
    if ( !pFrame->GetFrame()->IsClosing_Impl() ||
        SFX_APP()->GetViewFrame() != pFrame )
            pFrame->GetDispatcher()->Update_Impl( TRUE );
    pFrame->GetBindings().HidePopups(FALSE);

    uno::Reference < frame::XFrame > xOwnFrame( pFrame->GetFrame()->GetFrameInterface() );
    uno::Reference < frame::XFramesSupplier > xParentFrame( xOwnFrame->getCreator(), uno::UNO_QUERY );
    if ( xParentFrame.is() )
        xParentFrame->setActiveFrame( uno::Reference < frame::XFrame >() );
}

//--------------------------------------------------------------------

SfxInPlaceClient* SfxViewShell::FindIPClient
(
    const uno::Reference < embed::XEmbeddedObject >& xObj,
    Window*             pObjParentWin
)   const
{
    SfxInPlaceClientList *pClients = GetIPClientList_Impl(FALSE);
    if ( !pClients )
        return 0;

    if( !pObjParentWin )
        pObjParentWin = GetWindow();
    for (USHORT n=0; n < pClients->Count(); n++)
    {
        SfxInPlaceClient *pIPClient = (SfxInPlaceClient*) pClients->GetObject(n);
        if ( pIPClient->GetObject() == xObj && pIPClient->GetEditWin() == pObjParentWin )
            return pIPClient;
    }

    return 0;
}

//--------------------------------------------------------------------

SfxInPlaceClient* SfxViewShell::GetIPClient() const
{
    return GetUIActiveClient();
}

//--------------------------------------------------------------------

SfxInPlaceClient* SfxViewShell::GetUIActiveClient() const
{
    SfxInPlaceClientList *pClients = GetIPClientList_Impl(FALSE);
    if ( !pClients )
        return 0;

    for (USHORT n=0; n < pClients->Count(); n++)
    {
        SfxInPlaceClient* pIPClient = pClients->GetObject(n);
        if ( pIPClient->IsObjectUIActive() )
            return pIPClient;
    }

    return NULL;
}

//--------------------------------------------------------------------

void SfxViewShell::Activate( BOOL bMDI )
{
    DBG_CHKTHIS(SfxViewShell, 0);
    if ( bMDI )
    {
        SfxObjectShell *pSh = GetViewFrame()->GetObjectShell();
        if ( pSh->GetModel().is() )
            pSh->GetModel()->setCurrentController( GetViewFrame()->GetFrame()->GetController() );

        SfxObjectShell::SetWorkingDocument( pSh );
    }
}

//--------------------------------------------------------------------

void SfxViewShell::Deactivate(BOOL bMDI)
{
    DBG_CHKTHIS(SfxViewShell, 0);
    if ( bMDI )
    {
        SfxObjectShell *pSh = GetViewFrame()->GetObjectShell();
        if ( pSh == SfxObjectShell::GetWorkingDocument() )
            SfxObjectShell::SetWorkingDocument( 0 );
    }
}

//--------------------------------------------------------------------

void SfxViewShell::AdjustPosSizePixel
(
    const Point&    rToolOffset,// linke obere Ecke der Tools im Frame-Window
    const Size&     rSize       // gesamte zur Verf"ugung stehende Gr"o\se
)

{
    DBG_CHKTHIS(SfxViewShell, 0);
}

//--------------------------------------------------------------------

void SfxViewShell::Move()

/*  [Beschreibung]

    Diese virtuelle Methode wird gerufen, wenn das Fenster, in dem die
    SfxViewShell dargestellt wird eine StarView-Move() Nachricht erh"alt.

    Die Basisimplementierung braucht nicht gerufen zu werden.


    [Anmerkung]

    Diese Methode kann dazu verwendet werden, eine Selektion abzubrechen,
    um durch das Moven des Fensters erzeugte Maus-Bewegungen anzufangen.

    Zur Zeit funktioniert die Benachrichtigung nicht In-Place.
*/

{
}

//--------------------------------------------------------------------

void SfxViewShell::OuterResizePixel
(
    const Point&    rToolOffset,// linke obere Ecke der Tools im Frame-Window
    const Size&     rSize       // gesamte zur Verf"ugung stehende Gr"o\se
)

/*  [Beschreibung]

    Diese Methode muss ueberladen werden, um auf "Anderungen der Groesse
    der View zu reagieren. Dabei definieren wir die View als das Edit-Window
    zuz"uglich der um das Edit-Window angeordnenten Tools (z.B. Lineale).

    Das Edit-Window darf weder in Gr"o\se noch Position ver"andert werden.

    Die Vis-Area der SfxObjectShell, dessen Skalierung und Position
    d"urfen hier ver"andert werden. Der Hauptanwendungsfall ist dabei,
    das Ver"andern der Gr"o\se der Vis-Area.

    "Andert sich durch die neue Berechnung der Border, so mu\s dieser
    mit <SfxViewShell::SetBorderPixel(const SvBorder&)> gesetzt werden.
    Erst nach Aufruf von 'SetBorderPixel' ist das Positionieren von
    Tools erlaubt.


    [Beispiel]

    void AppViewSh::OuterViewResizePixel( const Point &rOfs, const Size &rSz )
    {
        // Tool-Positionen und Gr"o\sen von au\sen berechnen, NICHT setzen!
        // (wegen folgender Border-Berechnung)
        Point aHLinPos...; Size aHLinSz...;
        ...

        // Border f"ur Tools passend zu rSize berechnen und setzen
        SvBorder aBorder...
        SetBorderPixel( aBorder ); // ab jetzt sind Positionierungen erlaubt

        // Tools anordnen
        pHLin->SetPosSizePixel( aHLinPos, aHLinSz );
        ...
    }


    [Querverweise]

    <SfxViewShell::InnerResizePixel(const Point&,const Size& rSize)>
*/

{
    DBG_CHKTHIS(SfxViewShell, 0);
    SetBorderPixel( SvBorder() );
}

//--------------------------------------------------------------------

void SfxViewShell::InnerResizePixel
(
    const Point&    rToolOffset,// linke obere Ecke der Tools im Frame-Window
    const Size&     rSize       // dem Edit-Win zur Verf"ugung stehende Gr"o\se
)

/*  [Beschreibung]

    Diese Methode muss ueberladen werden, um auf "Anderungen der Groesse
    des Edit-Windows zu reagieren.

    Das Edit-Window darf weder in Gr"o\se noch Position ver"andert werden.
    Weder die Vis-Area der SfxObjectShell noch dessen Skalierung oder
    Position d"urfen ver"andert werden.

    "Andert sich durch die neue Berechnung der Border, so mu\s dieser
    mit <SfxViewShell::SetBorderPixel(const SvBorder&)> gesetzt werden.
    Erst nach Aufruf von 'SetBorderPixel' ist das Positionieren von
    Tools erlaubt.


    [Beispiel]

    void AppViewSh::InnerViewResizePixel( const Point &rOfs, const Size &rSz )
    {
        // Tool-Positionen und Gr"o\sen von innen berechnen, NICHT setzen!
        // (wegen folgender Border-Berechnung)
        Point aHLinPos...; Size aHLinSz...;
        ...

        // Border f"ur Tools passend zu rSz berechnen und setzen
        SvBorder aBorder...
        SetBorderPixel( aBorder ); // ab jetzt sind Positionierungen erlaubt

        // Tools anordnen
        pHLin->SetPosSizePixel( aHLinPos, aHLinSz );
        ...
    }


    [Querverweise]

    <SfxViewShell::OuterResizePixel(const Point&,const Size& rSize)>
*/

{
    DBG_CHKTHIS(SfxViewShell, 0);
    SetBorderPixel( SvBorder() );
}

//--------------------------------------------------------------------

void SfxViewShell::InvalidateBorder()
{
    DBG_CHKTHIS(SfxViewShell, 0);
    DBG_ASSERT( GetViewFrame(), "SfxViewShell without SfxViewFrame" );

    GetViewFrame()->InvalidateBorderImpl( this );
    if ( pImp->pController )
        pImp->pController->BorderWidthsChanged_Impl();
}

//--------------------------------------------------------------------

void SfxViewShell::SetBorderPixel( const SvBorder &rBorder )
{
    DBG_CHKTHIS(SfxViewShell, 0);
    DBG_ASSERT( GetViewFrame(), "SfxViewShell without SfxViewFrame" );

    //if ( rBorder != GetBorderPixel())
    {
        GetViewFrame()->SetBorderPixelImpl( this, rBorder );

        // notify related controller that border size is changed
        if ( pImp->pController )
            pImp->pController->BorderWidthsChanged_Impl();
    }
}

//--------------------------------------------------------------------

const SvBorder& SfxViewShell::GetBorderPixel() const
{
    DBG_CHKTHIS(SfxViewShell, 0);
    DBG_ASSERT( GetViewFrame(), "SfxViewShell without SfxViewFrame" );

    return GetViewFrame()->GetBorderPixelImpl( this );
}

//--------------------------------------------------------------------

void SfxViewShell::SetWindow
(
    Window*     pViewPort   // Pointer auf das Datenfenster bzw. 0 im Destruktor
)

/*  [Beschreibung]

    Mit dieser Methode wird der SfxViewShell das Datenfenster mitgeteilt.
    Dieses wird f"ur den In-Place-Container und f"ur das korrekte
    Wiederherstellen des Focus ben"otigt.

    Selbst In-Place-aktiv ist das Umsetzen des ViewPort-Windows verboten.
*/

{
    if( pWindow == pViewPort )
        return;

    // ggf. vorhandene IP-Clients disconnecten
    DisconnectAllClients();

    //TODO: should we have a "ReconnectAllClients" method?
    DiscardClients_Impl();

    // View-Port austauschen
    BOOL bHadFocus = pWindow ? pWindow->HasChildPathFocus( TRUE ) : FALSE;
    pWindow = pViewPort;

    if( pWindow )
    {
        // Disable automatic GUI mirroring (right-to-left) for document windows
        pWindow->EnableRTL( FALSE );
    }

    if ( bHadFocus && pWindow )
        SFX_APP()->GrabFocus( pWindow );
}

//--------------------------------------------------------------------

Size SfxViewShell::GetOptimalSizePixel() const
{
    DBG_ERROR( "Useless call!" );
    return Size();
}

//------------------------------------------------------------------------

SfxViewShell::SfxViewShell
(
    SfxViewFrame*   pViewFrame,     /*  <SfxViewFrame>, in dem diese View
                                        dargestellt wird */
    USHORT          nFlags          /*  siehe <SfxViewShell-Flags> */
)

:   SfxShell(this)
    ,pImp( new SfxViewShell_Impl )
#if !SFX_VIEWSH_INCLUDES_CLIENTSH_HXX
    ,pIPClientList( 0 )
#endif
    ,pFrame(pViewFrame)
    ,pSubShell(0)
    ,pWindow(0)
    ,bMaximizeFirst( 0 != (nFlags & SFX_VIEW_MAXIMIZE_FIRST) )
    ,bOptimizeEach(0 != (nFlags & SFX_VIEW_OPTIMIZE_EACH))
    ,bNoNewWindow( 0 != (nFlags & SFX_VIEW_NO_NEWWINDOW) )
{
    DBG_CTOR(SfxViewShell, 0);

    pImp->pController = 0;
    pImp->bIsShowView =
        !(SFX_VIEW_NO_SHOW == (nFlags & SFX_VIEW_NO_SHOW));

    pImp->bUseObjectSize = FALSE;
//        SFX_CREATE_MODE_EMBEDDED==pFrame->GetObjectShell()->GetCreateMode() &&
//        SFX_VIEW_OBJECTSIZE_EMBEDDED == (nFlags & SFX_VIEW_OBJECTSIZE_EMBEDDED);
    pImp->bCanPrint = SFX_VIEW_CAN_PRINT == (nFlags & SFX_VIEW_CAN_PRINT);
    pImp->bFrameSetImpl = nFlags & SFX_VIEW_IMPLEMENTED_AS_FRAMESET;
    pImp->bHasPrintOptions =
        SFX_VIEW_HAS_PRINTOPTIONS == (nFlags & SFX_VIEW_HAS_PRINTOPTIONS);
    pImp->bPlugInsActive = TRUE;
    pImp->bGotOwnerShip = FALSE;
    pImp->bGotFrameOwnerShip = FALSE;
    if ( pFrame->GetParentViewFrame() )
        pImp->bPlugInsActive = pFrame->GetParentViewFrame()->GetViewShell()->pImp->bPlugInsActive;
    pImp->eScroll = SCROLLING_DEFAULT;
    pImp->nPrinterLocks = 0;
    pImp->pMenuBarResId = 0;
    pImp->pAccelResId = 0;
    pImp->pAccel = 0;
    pImp->pMenu = 0;
    pImp->bControllerSet = FALSE;
    pImp->bOwnsMenu = TRUE;
    pImp->nFamily = 0xFFFF;                 // undefined, default set by TemplateDialog
    SetMargin( pFrame->GetMargin_Impl() );

    SetPool( &pViewFrame->GetObjectShell()->GetPool() );
    StartListening(*SFX_APP());

    // in Liste eintragen
    const SfxViewShell *pThis = this; // wegen der kranken Array-Syntax
    SfxViewShellArr_Impl &rViewArr = SFX_APP()->GetViewShells_Impl();
    rViewArr.Insert(pThis, rViewArr.Count() );
}

//--------------------------------------------------------------------

SfxViewShell::~SfxViewShell()
{
    DBG_DTOR(SfxViewShell, 0);

    // aus Liste austragen
    const SfxViewShell *pThis = this;
    SfxViewShellArr_Impl &rViewArr = SFX_APP()->GetViewShells_Impl();
    rViewArr.Remove( rViewArr.GetPos(pThis) );
//  if ( GetViewFrame()->GetFrame()->GetFrameSet_Impl() == pImp->pSetDescr )
//      GetViewFrame()->GetFrame()->SetFrameSet_Impl( NULL );
//  delete pImp->pSetDescr;

/*
    if ( pImp->pMenu && pImp->bOwnsMenu )
    {
        SfxTopViewFrame* pTopView = PTR_CAST( SfxTopViewFrame, GetViewFrame()->GetTopViewFrame() );
        SfxTopFrame *pTop = pTopView ? pTopView->GetTopFrame_Impl() : NULL;
        if ( pTop )
        {
            Menu* pMenu = pImp->pMenu->GetMenu()->GetSVMenu();
            if ( pMenu == pTop->GetMenuBar_Impl() )
                pTop->SetMenuBar_Impl( 0 );
        }

        delete pImp->pMenu;
    }
*/
    if ( pImp->pController )
    {
        pImp->pController->ReleaseShell_Impl();
        pImp->pController->release();
    }

    delete pImp->pMenuBarResId;
    if (pImp->pAccExec)
    {
        delete pImp->pAccExec;
        pImp->pAccExec = 0;
    }
    delete pImp;
    delete pIPClientList;
}

//--------------------------------------------------------------------

USHORT SfxViewShell::PrepareClose
(
    BOOL    bUI,     // TRUE: Dialoge etc. erlaubt, FALSE: silent-mode
    BOOL bForBrowsing
)
{
    SfxPrinter *pPrinter = GetPrinter();
    if ( pPrinter && pPrinter->IsPrinting() )
    {
        if ( bUI )
        {
            InfoBox aInfoBox( &GetViewFrame()->GetWindow(), SfxResId( MSG_CANT_CLOSE ) );
            aInfoBox.Execute();
        }

        return FALSE;
    }

    if( GetViewFrame()->IsInModalMode() )
        return FALSE;

    if( bUI && GetViewFrame()->GetDispatcher()->IsLocked() )
        return FALSE;

    return TRUE;
}

//--------------------------------------------------------------------

SfxViewShell* SfxViewShell::Current()
{
    SfxViewFrame *pCurrent = SfxViewFrame::Current();
    return pCurrent ? pCurrent->GetViewShell() : NULL;
}

//--------------------------------------------------------------------

SdrView* SfxViewShell::GetDrawView() const

/*  [Beschreibung]

    Diese virtuelle Methode mu\s von den Subklassen "uberladen werden, wenn
    der Property-Editor zur Verf"ugung stehen soll.

    Die Default-Implementierung liefert immer 0.
*/

{
    return 0;
}

//--------------------------------------------------------------------

String SfxViewShell::GetSelectionText
(
    BOOL bCompleteWords     /*  FALSE (default)
                                Nur der tats"achlich selektierte Text wird
                                zur"uckgegeben.

                                TRUE
                                Der selektierte Text wird soweit erweitert,
                                da\s nur ganze W"orter zur"uckgegeben werden.
                                Als Worttrenner gelten White-Spaces und die
                                Satzzeichen ".,;" sowie einfache und doppelte
                                Anf"uhrungszeichen.
                            */
)

/*  [Beschreibung]

    Diese Methode kann von Anwendungsprogrammierer "uberladen werden,
    um einen Text zur"uckzuliefern, der in der aktuellen Selektion
    steht. Dieser wird z.B. beim Versenden (email) verwendet.

    Mit "CompleteWords == TRUE" ger"ufen, reicht z.B. auch der Cursor,
    der in einer URL steht, um die gesamte URL zu liefern.
*/

{
    return String();
}

//--------------------------------------------------------------------

BOOL SfxViewShell::HasSelection( BOOL ) const

/*  [Beschreibung]

    Mit dieser virtuellen Methode kann z.B. ein Dialog abfragen, ob in der
    aktuellen View etwas selektiert ist. Wenn der Parameter <BOOL> TRUE ist,
    wird abgefragt, ob Text selektiert ist.
*/

{
    return FALSE;
}

//--------------------------------------------------------------------

void SfxViewShell::SetSubShell( SfxShell *pShell )

/*  [Beschreibung]

    Mit dieser Methode kann eine Selektions- oder Cursor-Shell angemeldet
    werden, die automatisch unmittelbar nach der SfxViewShell auf den
    SfxDispatcher gepusht wird, und automatisch umittelbar vor ihr
    gepoppt wird.

    Ist die SfxViewShell-Instanz bereits gepusht, dann wird pShell
    sofort ebenfalls gepusht. Wird mit SetSubShell eine andere SfxShell
    Instanz angemeldet, als vorher angemeldet war, wird die zuvor angemeldete
    ggf. automatisch gepoppt. Mit pShell==0 kann daher die aktuelle
    Sub-Shell abgemeldet werden.
*/

{
    // ist diese ViewShell "uberhaupt aktiv?
    SfxDispatcher *pDisp = pFrame->GetDispatcher();
    if ( pDisp->IsActive(*this) )
    {
        // Dispatcher updaten
        if ( pSubShell )
            pDisp->Pop(*pSubShell);
        if ( pShell )
            pDisp->Push(*pShell);
        pDisp->Flush();
    }

    pSubShell = pShell;
}

void SfxViewShell::AddSubShell( SfxShell& rShell )
{
    pImp->aArr.Insert( &rShell, pImp->aArr.Count() );
    SfxDispatcher *pDisp = pFrame->GetDispatcher();
    if ( pDisp->IsActive(*this) )
    {
        pDisp->Push(rShell);
        pDisp->Flush();
    }
}

void SfxViewShell::RemoveSubShell( SfxShell* pShell )
{
    SfxDispatcher *pDisp = pFrame->GetDispatcher();
    if ( !pShell )
    {
        USHORT nCount = pImp->aArr.Count();
        if ( pDisp->IsActive(*this) )
        {
            for ( USHORT n=nCount; n>0; n-- )
                pDisp->Pop( *pImp->aArr[n-1] );
            pDisp->Flush();
        }

        pImp->aArr.Remove(0, nCount);
    }
    else
    {
        USHORT nPos = pImp->aArr.GetPos( pShell );
        if ( nPos != 0xFFFF )
        {
            pImp->aArr.Remove( nPos );
            if ( pDisp->IsActive(*this) )
            {
                pDisp->RemoveShell_Impl( *pShell );
                pDisp->Flush();
            }
        }
    }
}

SfxShell* SfxViewShell::GetSubShell( USHORT nNo )
{
    USHORT nCount = pImp->aArr.Count();
    if ( nNo<nCount )
        return pImp->aArr[nCount-nNo-1];
    return NULL;
}

void SfxViewShell::PushSubShells_Impl( BOOL bPush )
{
    USHORT nCount = pImp->aArr.Count();
    SfxDispatcher *pDisp = pFrame->GetDispatcher();
    if ( bPush )
    {
        for ( USHORT n=0; n<nCount; n++ )
            pDisp->Push( *pImp->aArr[n] );

//      HACK(evtl. PushSubShells fuer SW virtuell machen oder im SW umbauen)
//      Notify( *this, SfxSimpleHint( SFX_HINT_RESERVED4 ) );
    }
    else if ( nCount )
    {
        pDisp->Pop( *pImp->aArr[0], SFX_SHELL_POP_UNTIL );

//      HACK(evtl. PushSubShells fuer SW virtuell machen oder im SW umbauen)
//      Notify( *this, SfxSimpleHint( SFX_HINT_RESERVED3 ) );
    }

    pDisp->Flush();
}

//--------------------------------------------------------------------

void SfxViewShell::WriteUserData( String &, BOOL bBrowse )
{
}

//--------------------------------------------------------------------

void SfxViewShell::ReadUserData(const String &, BOOL bBrowse )
{
}

void SfxViewShell::ReadUserDataSequence ( const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse )
{
}
void SfxViewShell::WriteUserDataSequence ( ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse )
{
}


//--------------------------------------------------------------------
// returns the first shell of spec. type viewing the specified doc.

SfxViewShell* SfxViewShell::GetFirst
(
    const TypeId*   pType,
    BOOL            bOnlyVisible
)
{
    SfxViewShellArr_Impl &rShells = SFX_APP()->GetViewShells_Impl();

    // search for a SfxViewShell of the specified type
    for ( USHORT nPos = 0; nPos < rShells.Count(); ++nPos )
    {
        SfxViewShell *pShell = rShells.GetObject(nPos);
        if ( !pType || pShell->IsA(*pType) )
            return pShell;
    }

    return 0;
}

//--------------------------------------------------------------------
// returns the next shell of spec. type viewing the specified doc.

SfxViewShell* SfxViewShell::GetNext
(
    const SfxViewShell& rPrev,
    const TypeId*       pType,
    BOOL                bOnlyVisible
)
{
    SfxViewShellArr_Impl &rShells = SFX_APP()->GetViewShells_Impl();

    // refind the specified predecessor
    USHORT nPos;
    for ( nPos = 0; nPos < rShells.Count(); ++nPos )
        if ( rShells.GetObject(nPos) == &rPrev )
            break;

    // search for a Frame of the specified type
    for ( ++nPos; nPos < rShells.Count(); ++nPos )
    {
        SfxViewShell *pShell = rShells.GetObject(nPos);
        if ( !pType || pShell->IsA(*pType) )
            return pShell;
    }
    return 0;
}

//--------------------------------------------------------------------

void SfxViewShell::SFX_NOTIFY( SfxBroadcaster& rBC,
                            const TypeId& rBCType,
                            const SfxHint& rHint,
                            const TypeId& rHintType )
{
    if ( rHint.IsA(TYPE(SfxEventHint)) )
    {
        switch ( ((SfxEventHint&)rHint).GetEventId() )
        {
            case SFX_EVENT_LOADFINISHED:
            {
                if ( GetController().is() )
                {
                    SfxItemSet* pSet = GetObjectShell()->GetMedium()->GetItemSet();
                    SFX_ITEMSET_ARG( pSet, pItem, SfxUnoAnyItem, SID_VIEW_DATA, sal_False );
                    if ( pItem )
                        pImp->pController->restoreViewData( pItem->GetValue() );
                    pSet->ClearItem( SID_VIEW_DATA );
                }
                break;
            }
        }
    }
}

//--------------------------------------------------------------------

BOOL SfxViewShell::ExecKey_Impl(const KeyEvent& aKey)
{
    if (!pImp->pAccExec)
    {
        pImp->pAccExec = ::svt::AcceleratorExecute::createAcceleratorHelper();
        pImp->pAccExec->init(::comphelper::getProcessServiceFactory(), pFrame->GetFrame()->GetFrameInterface());
    }

    return pImp->pAccExec->execute(aKey.GetKeyCode());
}

//--------------------------------------------------------------------

FASTBOOL SfxViewShell::KeyInput( const KeyEvent &rKeyEvent )

/*  [Beschreibung]

    Diese Methode f"uhrt das KeyEvent 'rKeyEvent' "uber die an dieser
    SfxViewShell direkt oder indirekt (z.B. via Applikation) konfigurierten
    Tasten (Accelerator) aus.


    [R"uckgabewert]

    FASTBOOL                TRUE
                            die Taste ist konfiguriert, der betreffende
                            Handler wurde gerufen

                            FALSE
                            die Taste ist nicht konfiguriert, es konnte
                            also kein Handler gerufen werden


    [Querverweise]
    <SfxApplication::KeyInput(const KeyEvent&)>
*/
{
    return ExecKey_Impl(rKeyEvent);
}

FASTBOOL SfxViewShell::GlobalKeyInput_Impl( const KeyEvent &rKeyEvent )
{
    return ExecKey_Impl(rKeyEvent);
}

//--------------------------------------------------------------------

void SfxViewShell::ShowCursor( FASTBOOL bOn )

/*  [Beschreibung]

    Diese Methode mu\s von Subklassen "uberladen werden, damit vom SFx
    aus der Cursor ein- und ausgeschaltet werden kann. Dies geschieht
    z.B. bei laufendem <SfxProgress>.
*/

{
}

//--------------------------------------------------------------------

void SfxViewShell::GotFocus() const

/*  [Beschreibung]

    Diese Methode mu\s vom Applikationsentwickler gerufen werden, wenn
    das Edit-Window den Focus erhalten hat. Der SFx hat so z.B. die
    M"oglichkeit, den Accelerator einzuschalten.


    [Anmerkung]

    <StarView> liefert leider keine M"oglichkeit, solche Events
    'von der Seite' einzuh"angen.
*/

{
}

//--------------------------------------------------------------------
void SfxViewShell::ResetAllClients( SfxInPlaceClient *pIP, BOOL bDisconnect )

/*  [Beschreibung]

    Diese Methode dient dazu, bei UI-Aktivierung eins IPClients alle anderen
    noch bestehenden Verbindungen zu trennen.

*/

{

    // SO2 stellt sicher, da\s nur ein Object gleichzeitig UI-aktiv ist.
    // Aus Speicherplatzgr"unden werden aber alle Objekte, die nicht aktiv
    // sind oder sein m"ussen, disconnected.
    SfxInPlaceClientList *pClients = GetIPClientList_Impl(FALSE);
    if ( !pClients )
        return;

    for ( USHORT n=0; n < pClients->Count(); n++ )
    {
        SfxInPlaceClient* pIPClient = pClients->GetObject(n);
        if( pIPClient != pIP && pIPClient->GetObject().is() && !(pIPClient->GetObjectMiscStatus() & SVOBJ_MISCSTATUS_ACTIVATEWHENVISIBLE) )
            pIPClient->SetObjectState( bDisconnect ? embed::EmbedStates::LOADED : embed::EmbedStates::RUNNING );
    }
}

//--------------------------------------------------------------------

void SfxViewShell::DisconnectAllClients()
{
    SfxInPlaceClientList *pClients = GetIPClientList_Impl(FALSE);
    if ( !pClients )
        return;

    for ( USHORT n=0; n<pClients->Count(); )
        // clients will remove themselves from the list
        delete pClients->GetObject(n);
}

//--------------------------------------------------------------------

BOOL SfxViewShell::UseObjectSize() const
{
    return pImp->bUseObjectSize;
}

//--------------------------------------------------------------------

void SfxViewShell::QueryObjAreaPixel( Rectangle& ) const
{
}

//--------------------------------------------------------------------

void SfxViewShell::AdjustVisArea(const Rectangle& rRect)
{
    DBG_ASSERT (pFrame, "Kein Frame?");
    if ( UseObjectSize() )
    {
        Point aPos = rRect.TopLeft();
        Size aSize = GetObjectShell()->GetVisArea().GetSize();
        GetObjectShell()->SetVisArea( Rectangle(aPos, aSize) );
    }
    else
        GetObjectShell()->SetVisArea( rRect );
}

//--------------------------------------------------------------------

void SfxViewShell::VisAreaChanged(const Rectangle& rVisArea)
{
    SfxInPlaceClientList *pClients = GetIPClientList_Impl(FALSE);
    if ( !pClients )
        return;

    for (USHORT n=0; n < pClients->Count(); n++)
    {
        SfxInPlaceClient* pIPClient = pClients->GetObject(n);
        if ( pIPClient->IsObjectInPlaceActive() )
            // client is active, notify client that the VisArea might have changed
            pIPClient->VisAreaChanged();
    }
}

//--------------------------------------------------------------------
void SfxViewShell::CheckIPClient_Impl( SfxInPlaceClient *pIPClient, const Rectangle& rVisArea )
{
    if ( GetObjectShell()->IsInClose() )
        return;

    sal_Bool bAlwaysActive =
        ( ( pIPClient->GetObjectMiscStatus() & embed::EmbedMisc::EMBED_ACTIVATEIMMEDIATELY ) != 0 );
    sal_Bool bActiveWhenVisible =
        ( ( pIPClient->GetObjectMiscStatus() & embed::EmbedMisc::MS_EMBED_ACTIVATEWHENVISIBLE ) != 0 );

    // this method is called when either a client is created or the "Edit/Plugins" checkbox is checked
    if ( !pIPClient->IsObjectInPlaceActive() && pImp->bPlugInsActive )
    {
           // object in client is currently not active
           // check if the object wants to be activated always or when it becomes at least partially visible
           // TODO/LATER: maybe we should use the scaled area instead of the ObjArea?!
           if ( bAlwaysActive || bActiveWhenVisible && rVisArea.IsOver( pIPClient->GetObjArea() ) )
        {
            try
            {
                pIPClient->GetObject()->changeState( embed::EmbedStates::INPLACE_ACTIVE );
            }
            catch ( uno::Exception& )
            {
            }
        }
    }
    else if ( !pImp->bPlugInsActive )
    {
           // object in client is currently active and "Edit/Plugins" checkbox is selected
           // check if the object wants to be activated always or when it becomes at least partially visible
        // in this case selecting of the "Edit/Plugin" checkbox should let such objects deactivate
           if ( bAlwaysActive || bActiveWhenVisible )
               pIPClient->GetObject()->changeState( embed::EmbedStates::RUNNING );
    }
}

//--------------------------------------------------------------------

BOOL SfxViewShell::PlugInsActive() const
{
    return pImp->bPlugInsActive;
}

//--------------------------------------------------------------------
void SfxViewShell::DiscardClients_Impl()

/*  [Beschreibung]

    Diese Methode dient dazu, vor dem Schlie\sen eines Dokuments das
    Speichern der Objekte zu verhindern, wenn der Benutzer Schlie\en ohne
    Speichern gew"ahlt hatte.
*/

{
    SfxInPlaceClientList *pClients = GetIPClientList_Impl(FALSE);
    if ( !pClients )
        return;

    for (USHORT n=0; n < pClients->Count(); )
        delete pClients->GetObject(n);
}

//--------------------------------------------------------------------

SfxScrollingMode SfxViewShell::GetScrollingMode() const
{
    return pImp->eScroll;
}

//--------------------------------------------------------------------

void SfxViewShell::SetScrollingMode( SfxScrollingMode eMode )
{
    pImp->eScroll = eMode;
}

//--------------------------------------------------------------------

SfxObjectShell* SfxViewShell::GetObjectShell()
{
    return GetViewFrame()->GetObjectShell();
}

//--------------------------------------------------------------------

const Size& SfxViewShell::GetMargin() const
{
    return pImp->aMargin;
}

//--------------------------------------------------------------------

void SfxViewShell::SetMargin( const Size& rSize )
{
    // Der default-Margin wurde "geeicht" mit www.apple.com !!
    Size aMargin = rSize;
    if ( aMargin.Width() == -1 )
        aMargin.Width() = DEFAULT_MARGIN_WIDTH;
    if ( aMargin.Height() == -1 )
        aMargin.Height() = DEFAULT_MARGIN_HEIGHT;

    if ( aMargin != pImp->aMargin )
    {
        pImp->aMargin = aMargin;
        MarginChanged();
    }
}

//--------------------------------------------------------------------

void SfxViewShell::MarginChanged()
{
}

//--------------------------------------------------------------------

BOOL SfxViewShell::IsShowView_Impl() const
{
    return pImp->bIsShowView;
}

//--------------------------------------------------------------------

SfxFrame* SfxViewShell::GetSmartSelf( SfxFrame* pSelf, SfxMedium& rMedium )
{
    return pSelf;
}

//--------------------------------------------------------------------

BOOL SfxViewShell::IsImplementedAsFrameset_Impl( ) const
{
    return pImp->bFrameSetImpl;
}

//------------------------------------------------------------------------

void SfxViewShell::JumpToMark( const String& rMark )
{
    SfxStringItem aMarkItem( SID_JUMPTOMARK, rMark );
    GetViewFrame()->GetDispatcher()->Execute(
            SID_JUMPTOMARK,
            SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD,
            &aMarkItem, 0L );
}

//------------------------------------------------------------------------

SfxInPlaceClientList* SfxViewShell::GetIPClientList_Impl( BOOL bCreate ) const
{
    if ( !pIPClientList && bCreate )
        ( (SfxViewShell*) this )->pIPClientList = new SfxInPlaceClientList;
    return pIPClientList;
}

void SfxViewShell::ReleaseMenuBar_Impl()
{
    pImp->bOwnsMenu = FALSE;
}

SfxMenuBarManager* SfxViewShell::GetMenuBar_Impl( BOOL bPlugin )
{
    // get the accelerators

    Reference < XPropertySet > xPropSet( GetViewFrame()->GetFrame()->GetFrameInterface(), UNO_QUERY );
    if ( xPropSet.is() )
    {
        Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;

        if ( xPropSet.is() )
        {
            Any aValue = xPropSet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" )));
            aValue >>= xLayoutManager;
        }

        if ( xLayoutManager.is() )
        {
            rtl::OUString aMenuBarURL( RTL_CONSTASCII_USTRINGPARAM( "private:resource/menubar/menubar" ));
            Reference< ::com::sun::star::ui::XUIElement > xMenuBarElement( xLayoutManager->getElement( aMenuBarURL ));
            if ( !xMenuBarElement.is() )
                GetObjectShell()->CreateMenuBarManager_Impl( GetViewFrame() );
        }
    }

    return NULL;
}

void SfxViewShell::SetController( SfxBaseController* pController )
{
    pImp->pController = pController;
    pImp->pController->acquire();
    pImp->bControllerSet = TRUE;
}

Reference < XController > SfxViewShell::GetController()
{
    return pImp->pController;
}

void SfxViewShell::AddContextMenuInterceptor_Impl( const REFERENCE< XCONTEXTMENUINTERCEPTOR >& xInterceptor )
{
    pImp->aInterceptorContainer.addInterface( xInterceptor );
}

void SfxViewShell::RemoveContextMenuInterceptor_Impl( const REFERENCE< XCONTEXTMENUINTERCEPTOR >& xInterceptor )
{
    pImp->aInterceptorContainer.removeInterface( xInterceptor );
}

::cppu::OInterfaceContainerHelper& SfxViewShell::GetContextMenuInterceptors() const
{
    return pImp->aInterceptorContainer;
}

void Change( Menu* pMenu, SfxViewShell* pView )
{
    SfxDispatcher *pDisp = pView->GetViewFrame()->GetDispatcher();
    USHORT nCount = pMenu->GetItemCount();
    for ( USHORT nPos=0; nPos<nCount; ++nPos )
    {
        USHORT nId = pMenu->GetItemId(nPos);
        String aCmd = pMenu->GetItemCommand(nId);
        PopupMenu* pPopup = pMenu->GetPopupMenu(nId);
        if ( nId < 5000 )
        {
            if ( aCmd.CompareToAscii(".uno:", 5) == 0 )
            {
                SfxShell *pShell=0;
                USHORT nIdx;
                for (nIdx=0; (pShell=pDisp->GetShell(nIdx)); nIdx++)
                {
                    const SfxInterface *pIFace = pShell->GetInterface();
                    const SfxSlot* pSlot = pIFace->GetSlot( aCmd );
                    if ( pSlot )
                    {
                        pMenu->InsertItem( pSlot->GetSlotId(), pMenu->GetItemText( nId ), pMenu->GetItemBits( nId ), nPos );
                        pMenu->RemoveItem( nPos+1 );
                        break;
                    }
                }
            }
        }
        if ( pPopup )
        {
            Change( pPopup, pView );
        }
    }
}


BOOL SfxViewShell::TryContextMenuInterception( Menu& rIn, Menu*& rpOut, ::com::sun::star::ui::ContextMenuExecuteEvent aEvent )
{
    rpOut = NULL;
    BOOL bModified = FALSE;

    // create container from menu
    // #110897#
    // aEvent.ActionTriggerContainer = ::framework::ActionTriggerHelper::CreateActionTriggerContainerFromMenu( &rIn );
    aEvent.ActionTriggerContainer = ::framework::ActionTriggerHelper::CreateActionTriggerContainerFromMenu(
        ::comphelper::getProcessServiceFactory(), &rIn );

    // get selection from controller
    aEvent.Selection = ::com::sun::star::uno::Reference < ::com::sun::star::view::XSelectionSupplier > ( GetController(), ::com::sun::star::uno::UNO_QUERY );

    // call interceptors
    ::cppu::OInterfaceIteratorHelper aIt( pImp->aInterceptorContainer );
    while( aIt.hasMoreElements() )
    {
        try
        {
            ::com::sun::star::ui::ContextMenuInterceptorAction eAction =
                ((::com::sun::star::ui::XContextMenuInterceptor*)aIt.next())->notifyContextMenuExecute( aEvent );
            switch ( eAction )
            {
                case ::com::sun::star::ui::ContextMenuInterceptorAction_CANCELLED :
                    // interceptor does not want execution
                    return FALSE;
                    break;
                case ::com::sun::star::ui::ContextMenuInterceptorAction_EXECUTE_MODIFIED :
                    // interceptor wants his modified menu to be executed
                    bModified = TRUE;
                    break;
                case ::com::sun::star::ui::ContextMenuInterceptorAction_CONTINUE_MODIFIED :
                    // interceptor has modified menu, but allows for calling other interceptors
                    bModified = TRUE;
                    continue;
                    break;
                case ::com::sun::star::ui::ContextMenuInterceptorAction_IGNORED :
                    // interceptor is indifferent
                    continue;
                    break;
                default:
                    DBG_ERROR("Wrong return value of ContextMenuInterceptor!");
                    continue;
                    break;
            }
        }
        catch( ::com::sun::star::uno::RuntimeException& )
        {
            aIt.remove();
        }

        break;
    }

    if ( bModified )
    {
        // container was modified, create a new window out of it
        rpOut = new PopupMenu;
        ::framework::ActionTriggerHelper::CreateMenuFromActionTriggerContainer( rpOut, aEvent.ActionTriggerContainer );

        Change( rpOut, this );
    }

    return TRUE;
}

void SfxViewShell::TakeOwnerShip_Impl()
{
    // currently there is only one reason to take OwnerShip: a hidden frame is printed
    // so the ViewShell will check this on EndPrint (->prnmon.cxx)
    pImp->bGotOwnerShip = TRUE;
}

void SfxViewShell::TakeFrameOwnerShip_Impl()
{
    // currently there is only one reason to take OwnerShip: a hidden frame is printed
    // so the ViewShell will check this on EndPrint (->prnmon.cxx)
    pImp->bGotFrameOwnerShip = TRUE;
}

void SfxViewShell::CheckOwnerShip_Impl()
{
    BOOL bSuccess = FALSE;
    if( pImp->bGotOwnerShip )
    {
        com::sun::star::uno::Reference < com::sun::star::util::XCloseable > xModel(
                GetObjectShell()->GetModel(), com::sun::star::uno::UNO_QUERY );
        if ( xModel.is() )
        {
            try
            {
                // this call will destroy this object in case of success!
                xModel->close( sal_True );
                bSuccess = TRUE;
            }
            catch ( com::sun::star::util::CloseVetoException& )
            {
            }
        }
    }

    if( !bSuccess && pImp->bGotFrameOwnerShip )
    {
        // document couldn't be closed or it shouldn't, now try at least to close the frame
        com::sun::star::uno::Reference < com::sun::star::util::XCloseable > xFrame(
                GetViewFrame()->GetFrame()->GetFrameInterface(), com::sun::star::uno::UNO_QUERY );
        if ( xFrame.is() )
        {
            try
            {
                xFrame->close( sal_True );
            }
            catch ( com::sun::star::util::CloseVetoException& )
            {
            }
        }
    }
}

long SfxViewShell::HandleNotifyEvent_Impl( NotifyEvent& rEvent )
{
    if ( pImp->pController )
        return pImp->pController->HandleEvent_Impl( rEvent );
    return 0;
}

BOOL SfxViewShell::HasKeyListeners_Impl()
{
    return pImp->pController ? pImp->pController->HasKeyListeners_Impl() : FALSE;
}

BOOL SfxViewShell::HasMouseClickListeners_Impl()
{
    return pImp->pController ? pImp->pController->HasMouseClickListeners_Impl() : FALSE;
}

void SfxViewShell::SetAdditionalPrintOptions( const com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue >& rOpts )
{
    pImp->aPrintOpts = rOpts;
     GetObjectShell()->Broadcast( SfxPrintingHint( -3, NULL, NULL, rOpts ) );
}

BOOL SfxViewShell::Escape()
{
    return GetViewFrame()->GetBindings().Execute( SID_TERMINATE_INPLACEACTIVATION );
}
