/*************************************************************************
 *
 *  $RCSfile: topfrm.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mba $ $Date: 2000-10-16 14:34:00 $
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

#include "topfrm.hxx"

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <unotools/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMESSUPPLIER_HPP_
#include <com/sun/star/frame/XFramesSupplier.hpp>
#endif
#ifndef _TOOLKIT_UNOHLP_HXX
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef _UNO_COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif

#ifndef _SV_MENU_HXX
#include <vcl/menu.hxx>
#endif
#ifndef _SFXRECTITEM_HXX //autogen
#include <svtools/rectitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _IPENV_HXX //autogen
#include <so3/ipenv.hxx>
#endif
#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif
#include <vcl/dialog.hxx>
#include <svtools/urihelper.hxx>

#include "sfx.hrc"
#include "objsh.hxx"
#include "docfile.hxx"
#include "viewsh.hxx"
#include "bindings.hxx"
#include "dispatch.hxx"
#include "request.hxx"
#include "sfxdir.hxx"
#include "fsetobsh.hxx"
#include "objitem.hxx"
#include "objface.hxx"
#include "msg.hxx"
#include "interno.hxx"
#include "objshimp.hxx"
#include "workwin.hxx"
#include "sfxtypes.hxx"
#include "splitwin.hxx"
#include "appdata.hxx"
#include "arrdecl.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;

//------------------------------------------------------------------------

#define SfxTopViewFrame
#include "sfxslots.hxx"

DBG_NAME(SfxTopViewFrame);

class SfxTopFrame_Impl
{
public:
    Window*             pWindow;        // maybe external
    BOOL                bHidden;
    BOOL                bLockResize;
};

class SfxTopWindow_Impl : public Window
{
public:
    SfxTopFrame*        pFrame;
    Dialog*             pModalDialog;

    SfxTopWindow_Impl( SfxTopFrame* pF );
//        : Window( pF->pImp->pWindow, WB_CLIPCHILDREN | WB_NODIALOGCONTROL | WB_3DLOOK )
//        , pFrame( pF )
//        , pModalDialog( 0 )
//    { SetBackground(); }
    ~SfxTopWindow_Impl( );

    virtual void        StateChanged( StateChangedType nStateChange );
    virtual long        PreNotify( NotifyEvent& rNEvt );
    virtual long        Notify( NotifyEvent& rEvt );
    virtual void        Resize();
    virtual void        GetFocus();
    void                DoResize();
};

SfxTopWindow_Impl::SfxTopWindow_Impl( SfxTopFrame* pF )
        : Window( pF->pImp->pWindow, WB_CLIPCHILDREN | WB_NODIALOGCONTROL | WB_3DLOOK )
        , pFrame( pF )
        , pModalDialog( 0 )
{
    SetBackground();
}

SfxTopWindow_Impl::~SfxTopWindow_Impl( )
{
}

long SfxTopWindow_Impl::Notify( NotifyEvent& rNEvt )
{
    if ( pFrame->IsClosing_Impl() )
        return sal_False;

    if ( rNEvt.GetType() == EVENT_GETFOCUS )
    {
        SfxViewFrame* pView = pFrame->GetCurrentViewFrame();
        SfxViewFrame* pCurrent = SfxViewFrame::Current();
        SfxViewFrame* pContainer = pCurrent ? pCurrent->GetParentViewFrame_Impl() : NULL;
        if ( !pContainer )
            pContainer = pCurrent;
        if ( pView && pView != pContainer )
            pView->MakeActive_Impl( FALSE );
        return sal_True;
    }

    if ( rNEvt.GetType() == EVENT_EXECUTEDIALOG )
    {
        pModalDialog = (Dialog*) rNEvt.GetWindow();
        pFrame->GetCurrentViewFrame()->SetModalMode( sal_True );
        return sal_True;
    }
    else if ( rNEvt.GetType() == EVENT_ENDEXECUTEDIALOG )
    {
        pModalDialog = NULL;
        EnableInput( sal_True, sal_True );
        pFrame->GetCurrentViewFrame()->SetModalMode( sal_False );
        return sal_True;
    }

    return Window::Notify( rNEvt );
}

long SfxTopWindow_Impl::PreNotify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_MOUSEBUTTONDOWN )
        SfxSplitWindow::EndAutoShow_Impl();

    return Window::PreNotify( rNEvt );
}

void SfxTopWindow_Impl::GetFocus()
{
    if ( !pFrame->IsClosing_Impl() && pFrame && pFrame->GetCurrentViewFrame() )
        pFrame->GetCurrentViewFrame()->MakeActive_Impl( TRUE );
}

void SfxTopWindow_Impl::Resize()
{
    if ( IsReallyVisible() || IsReallyShown() )
        DoResize();
}

void SfxTopWindow_Impl::StateChanged( StateChangedType nStateChange )
{
    if ( nStateChange == STATE_CHANGE_INITSHOW )
    {
        SetSizePixel( GetParent()->GetOutputSizePixel() );
        DoResize();
    }
    else
        Window::StateChanged( nStateChange );
}

void SfxTopWindow_Impl::DoResize()
{
    if ( !pFrame->pImp->bLockResize )
        pFrame->Resize();
}

class SfxTopViewWin_Impl : public Window
{
friend class SfxInternalFrame;

    BOOL                bActive;
    SfxTopViewFrame*    pFrame;

public:
                        SfxTopViewWin_Impl( SfxTopViewFrame* p,
                                Window *pParent, WinBits nBits=0 ) :
                            Window( pParent, nBits | WB_BORDER | WB_CLIPCHILDREN ),
                            pFrame( p ),
                            bActive( FALSE )
                        {
//                            SetBorderStyle( WINDOW_BORDER_NOBORDER );
                        }

    virtual void        Resize();
    virtual void        StateChanged( StateChangedType nStateChange );
};

//--------------------------------------------------------------------
void SfxTopViewWin_Impl::StateChanged( StateChangedType nStateChange )
{
    if ( nStateChange == STATE_CHANGE_INITSHOW )
        pFrame->Resize();
    else
        Window::StateChanged( nStateChange );
}

void SfxTopViewWin_Impl::Resize()
{
    if ( IsReallyVisible() || IsReallyShown() )
        pFrame->Resize();
}

class SfxTopViewFrame_Impl
{
public:
    sal_Bool            bActive;
    Window*             pWindow;

                        SfxTopViewFrame_Impl()
                            : bActive( sal_False )
                            , pWindow( 0 )
                        {}
};

static AsynchronLink* pPendingCloser = 0;

SfxTopFrame* SfxTopFrame::Create( SfxObjectShell* pDoc, USHORT nViewId, BOOL bHidden, const SfxItemSet* pSet )
{
    Reference < XFrame > xDesktop ( ::utl::getProcessServiceFactory()->createInstance( DEFINE_CONST_UNICODE("com.sun.star.frame.Desktop") ), UNO_QUERY );
    Reference < XFrame > xFrame = xDesktop->findFrame( DEFINE_CONST_UNICODE("_blank"), 0 );
    SfxTopFrame *pFrame = Create( xFrame );
    pFrame->pImp->bHidden = bHidden;

    pFrame->SetItemSet_Impl( pSet );
    if ( pDoc )
    {
        if ( nViewId )
            pDoc->GetMedium()->GetItemSet()->Put( SfxUInt16Item( SID_VIEW_ID, nViewId ) );
        pFrame->InsertDocument( pDoc );
    }

    return pFrame;
}


SfxTopFrame* SfxTopFrame::Create( Reference < XFrame > xFrame )
{
    // create a new TopFrame to an external XFrame object ( wrap controller )
    if ( !xFrame.is() )
        DBG_ERROR( "Wrong parameter!" );

    Window* pWindow = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
    SfxTopFrame* pFrame = new SfxTopFrame( pWindow );
    pFrame->SetFrameInterface_Impl( xFrame );
    return pFrame;
}

SfxTopFrame::SfxTopFrame( Window* pExternal, sal_Bool bHidden )
    : SfxFrame( NULL )
    , pWindow( NULL )
{
    pImp = new SfxTopFrame_Impl;
    pImp->bHidden = bHidden;
    pImp->bLockResize = FALSE;
    InsertTopFrame_Impl( this );
    if ( pExternal )
    {
        pImp->pWindow = pExternal;
    }
    else
    {
        DBG_ERROR( "TopFrame without window created!" );
/*
        pImp->pWindow = new SfxTopFrameWindow_Impl( this );
        pImp->pWindow->SetActivateMode( ACTIVATE_MODE_GRABFOCUS );
        pImp->pWindow->SetPosSizePixel( Point( 20,20 ), Size( 800,600 ) );
        if ( GetFrameInterface().is() )
            GetFrameInterface()->initialize( VCLUnoHelper::GetInterface( pImp->pWindow ) );
        pImp->pWindow->Show();
 */
    }

    pWindow = new SfxTopWindow_Impl( this );
    pWindow->Show();
}

SfxTopFrame::~SfxTopFrame()
{
    RemoveTopFrame_Impl( this );
    DELETEZ( pWindow );
    delete pImp;
}

SystemWindow* SfxTopFrame::GetTopWindow_Impl() const
{
    if ( pImp->pWindow->IsSystemWindow() )
        return (SystemWindow*) pImp->pWindow;
    else
        return NULL;
}

Window& SfxTopFrame::GetWindow() const
{
    return *pWindow;
}

sal_Bool SfxTopFrame::Close()
{
    delete this;
    return sal_True;
}

void SfxTopFrame::LockResize_Impl( BOOL bLock )
{
    pImp->bLockResize = bLock;
}

void SfxTopFrame::SetMenuBar_Impl( MenuBar *pMenu )
{
    SystemWindow *pWin = GetTopWindow_Impl();
    if ( pWin && pWin->GetMenuBar() != pMenu )
    {
//(mba/task): WindowMenu-Hdl fehlt, dito PickMenu-Handling
        pWin->SetMenuBar( pMenu );
    }
}

MenuBar* SfxTopFrame::GetMenuBar_Impl() const
{
    SystemWindow *pWin = GetTopWindow_Impl();
    return pWin ? pWin->GetMenuBar() : NULL;
}

String SfxTopFrame::GetWindowData()
{
    String aActWinData;
    char cToken = SfxIniManager::GetToken();

    SfxViewFrame *pActFrame = SfxViewFrame::Current();
    SfxViewFrame *pFrame = GetCurrentViewFrame();
    const sal_Bool bActWin = ( pActFrame->GetTopViewFrame() == pFrame );

    // ::com::sun::star::sdbcx::User-Daten der ViewShell
    String aUserData;
    pFrame->GetViewShell()->WriteUserData(aUserData);

    // assemble ini-data
    String aWinData;
    aWinData += String::CreateFromInt32( pFrame->GetCurViewId() );
    aWinData += cToken;

    aWinData += '1';                    // former attribute "isfloating"
    aWinData += cToken;

    aWinData += SfxIniManager::GetString( pImp->pWindow->GetPosPixel(), pImp->pWindow->GetSizePixel() );

    // aktives kennzeichnen
    aWinData += cToken;
    aWinData += bActWin ? '1' : '0';

    aWinData += cToken;
    aWinData += aUserData;

    return aWinData;
}

sal_Bool SfxTopFrame::InsertDocument( SfxObjectShell* pDoc )
/* [Beschreibung]
 */
{
    // Spezielle Bedingungen testen: nicht im ModalMode!
    if ( !SfxFrame::InsertDocument( pDoc ) )
        return sal_False;

    SfxObjectShell *pOld = GetCurrentDocument();

    // Position und Groesse testen
    // Wenn diese schon gesetzt sind, soll offensichtlich nicht noch
    // LoadWindows_Impl aufgerufen werden ( z.B. weil dieses ein CreateFrame()
    // an einer Task aufgerufen hat! )
    const SfxItemSet* pSet = GetItemSet_Impl();
    if ( !pSet )
        pSet = pDoc->GetMedium()->GetItemSet();
    SetItemSet_Impl(0);

    // Position und Gr"o\se
    SFX_ITEMSET_ARG(
        pSet, pAreaItem, SfxRectangleItem, SID_VIEW_POS_SIZE, sal_False );
    // ::com::sun::star::sdbcx::View-Id
    SFX_ITEMSET_ARG(
        pSet, pViewIdItem, SfxUInt16Item, SID_VIEW_ID, sal_False );
    // Zoom
    SFX_ITEMSET_ARG(
        pSet, pModeItem, SfxUInt16Item, SID_VIEW_ZOOM_MODE, sal_False );
    // Hidden
    SFX_ITEMSET_ARG(
        pSet, pHidItem, SfxBoolItem,    SID_HIDDEN, sal_False);
    // ViewDaten
    SFX_ITEMSET_ARG(
        pSet, pViewDataItem, SfxStringItem, SID_USER_DATA, sal_False );

    if ( pHidItem )
        pImp->bHidden = pHidItem->GetValue();

    if( !pImp->bHidden )
        pDoc->OwnerLock( sal_True );

    // Wenn z.B. eine Fenstergr"o\se gesetzt wurde, soll keine Fensterinformation
    // aus den Dokument geladen werden, z.B. weil InsertDocument seinerseits
    // aus LoadWindows_Impl aufgerufen wurde!
    if ( pDoc && !pAreaItem && !pViewIdItem && !pModeItem &&
            !pImp->bHidden && pDoc->LoadWindows_Impl( this ) )
    {
        pDoc->OwnerLock( sal_False );
        return sal_True;
    }

    if ( pDoc )
    {
        UpdateHistory( pDoc );
        UpdateDescriptor( pDoc );
    }

    SfxFrameSetObjectShell *pFDoc = PTR_CAST( SfxFrameSetObjectShell, pDoc );
    if ( pFDoc )
        SetFrameType_Impl( GetFrameType() | SFXFRAME_FRAMESET );
    else
        SetFrameType_Impl( GetFrameType() & ~SFXFRAME_FRAMESET );

    sal_Bool bBrowsing = sal_True;
    BOOL bSetFocus = GetWindow().HasChildPathFocus( TRUE );
    SfxViewFrame *pFrame = GetCurrentViewFrame();
    if ( pFrame )
    {
        sal_Bool bChildActivated = sal_False;
        if ( pFrame->GetActiveChildFrame_Impl() && pFrame->GetActiveChildFrame_Impl() == SfxViewFrame::Current() )
        {
//            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFramesSupplier >  xFrames( GetFrameInterface(), ::com::sun::star::uno::UNO_QUERY );
//            if ( xFrames.is() )
//                xFrames->setActiveFrame( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > () );
            pFrame->SetActiveChildFrame_Impl(0);
            SFX_APP()->SetViewFrame( pFrame );
            bChildActivated = sal_True;
        }

        if ( pFrame->GetObjectShell() )
            pFrame->ReleaseObjectShell_Impl( sal_False );
        if ( pViewIdItem )
            pFrame->SetViewData_Impl( pViewIdItem->GetValue(), String() );
        if ( pDoc )
            pFrame->SetObjectShell_Impl( *pDoc );
    }
    else
    {
        bBrowsing = sal_False;
        pFrame = new SfxTopViewFrame( this, pDoc, pViewIdItem ? pViewIdItem->GetValue() : 0 );
    }

    if ( pViewDataItem )
    {
        if ( pDoc->Get_Impl()->nLoadedFlags & SFX_LOADED_MAINDOCUMENT )
            pFrame->GetViewShell()->ReadUserData( pViewDataItem->GetValue(), sal_True );
        else
        {
            // Daten setzen, die in FinishedLoading ausgewertet werden
            MarkData_Impl*& rpMark = pDoc->Get_Impl()->pMarkData;
            if (!rpMark)
                rpMark = new MarkData_Impl;
            rpMark->pFrame = pFrame;
            rpMark->aUserData = pViewDataItem->GetValue();
        }
    }

    // Position und Groesse setzen
    sal_uInt16 nWinMode = pModeItem ? pModeItem->GetValue() : 1;
    if ( pAreaItem && !pOld )
    {
        Window *pWin = pImp->pWindow;

        // Groesse setzen
        const Rectangle aWinRect( pAreaItem->GetValue() );
        const Size aAppWindow( pImp->pWindow->GetDesktopRectPixel().GetSize() );
        Point aPos( aWinRect.TopLeft() );
        Size aSz(aWinRect.GetSize());
        if ( aSz.Width() && aSz.Height() )
        {
            aPos.X() = Min(aPos.X(),
                            long(aAppWindow.Width() - aSz.Width() + aSz.Width() / 2) );
            aPos.Y() = Min(aPos.Y(),
                            long( aAppWindow.Height() - aSz.Height() + aSz.Height() / 2) );
            if ( aPos.X() + aSz.Width() <
                    aAppWindow.Width() + aSz.Width() / 2 &&
                    aPos.Y() + aSz.Height() <
                    aAppWindow.Height() + aSz.Height() / 2 )
            {
                pWin->SetPosPixel( aPos );
                pWin->SetOutputSizePixel( aSz );
            }
        }
    }

    if ( !pImp->bHidden )
    {
        pFrame->Show();
        GetWindow().Show();
        pFrame->MakeActive_Impl( TRUE );
        pDoc->OwnerLock( sal_False );
        GetFrameInterface()->getContainerWindow()->setVisible( sal_True );
        if( GetTopWindow_Impl()->HasFocus() )
            pFrame->MakeActive_Impl( TRUE );
    }

    if ( bSetFocus )
        // if the old component had the focus when it was destroyed, the focus has been transferred to
        // the ViewFrameWindow
        GrabFocusOnComponent_Impl();

    // Jetzt UpdateTitle, hidden TopFrames haben sonst keinen Namen!
    pFrame->UpdateTitle();
    return sal_True;
}


//========================================================================

long SfxViewFrameClose_Impl( void* pObj, void* pArg )
{
    ((SfxViewFrame*)pArg)->GetFrame()->DoClose();
    return 0;
}

TYPEINIT1(SfxTopViewFrame, SfxViewFrame);

//--------------------------------------------------------------------
SFX_IMPL_INTERFACE(SfxTopViewFrame,SfxViewFrame,ResId(0,0))
{
}

//--------------------------------------------------------------------
String SfxTopViewFrame::UpdateTitle()

/*  [Beschreibung]

    Mit dieser Methode kann der SfxTopViewFrame gezwungen werden, sich sofort
    den neuen Titel vom der <SfxObjectShell> zu besorgen.

    [Anmerkung]

    Dies ist z.B. dann notwendig, wenn man der SfxObjectShell als SfxListener
    zuh"ort und dort auf den <SfxSimpleHint> SFX_HINT_TITLECHANGED reagieren
    m"ochte, um dann die Titel seiner Views abzufragen. Diese Views (SfxTopViewFrames)
    jedoch sind ebenfalls SfxListener und da die Reihenfolge der Benachrichtigung
    nicht feststeht, mu\s deren Titel-Update vorab erzwungen werden.


    [Beispiel]

    void SwDocShell::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
    {
        if ( rHint.IsA(TYPE(SfxSimpleHint)) )
        {
            switch( ( (SfxSimpleHint&) rHint ).GetId() )
            {
                case SFX_HINT_TITLECHANGED:
                    for ( SfxTopViewFrame *pTop = (SfxTopViewFrame*)
                                SfxViewFrame::GetFirst(this, TYPE(SfxTopViewFrame));
                          pTop;
                          pTop = (SfxTopViewFrame*)
                                SfxViewFrame::GetNext(this, TYPE(SfxTopViewFrame));
                    {
                        pTop->UpdateTitle();
                        ... pTop->GetName() ...
                    }
                    break;
                ...
            }
        }
    }
*/

{
    DBG_CHKTHIS(SfxTopViewFrame, 0);

    String aTitle = SfxViewFrame::UpdateTitle();
    aTitle += String::CreateFromAscii( " - " );
    aTitle += Application::GetDisplayName();

    if ( GetFrame()->GetWindow().GetText() != aTitle )
    {
        GetFrame()->GetWindow().SetText( aTitle );
        if ( GetTopFrame_Impl()->GetTopWindow_Impl() )
            GetTopFrame_Impl()->GetTopWindow_Impl()->SetText( aTitle );
    }

    return aTitle;
}

//--------------------------------------------------------------------
void SfxTopViewFrame::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    {DBG_CHKTHIS(SfxTopViewFrame, 0);}

    if( IsDowning_Impl())
        return;
    // we know only SimpleHints
    if ( rHint.IsA(TYPE(SfxSimpleHint)) )
    {
        switch( ( (SfxSimpleHint&) rHint ).GetId() )
        {
            case SFX_HINT_MODECHANGED:
            case SFX_HINT_TITLECHANGED:
                // when the document changes its title, change views too
                UpdateTitle();
                break;

            case SFX_HINT_DYING:
                // on all other changes force repaint
                GetFrame()->DoClose();
                return;
                break;

        }
    }

    SfxViewFrame::SFX_NOTIFY( rBC, rBCType, rHint, rHintType );
}

//--------------------------------------------------------------------
sal_Bool SfxTopViewFrame::Close()
{
    {DBG_CHKTHIS(SfxTopViewFrame, 0);}

    // Modaler Dialog oben ??
//  if ( pImp->GetModalDialog() )
//      return sal_False;

    // eigentliches Schlie\sen
    if ( SfxViewFrame::Close() )
    {
        SfxApplication *pSfxApp = SFX_APP();
        if (pSfxApp->GetViewFrame() == this)
            pSfxApp->SetViewFrame(0);

        // Da der Dispatcher leer ger"aumt wird, kann man ihn auch nicht mehr
        // vern"unftig verwenden - also besser still legen
        GetDispatcher()->Lock(sal_True);
        delete this;
        return sal_True;
    }

    return sal_False;
}

SfxTopViewFrame::SfxTopViewFrame
(
    SfxFrame*           pFrame,
    SfxObjectShell*     pObjShell,
    sal_uInt16              nViewId
)

/*  [Beschreibung]

    Ctor des SfxTopViewFrame f"ur eine <SfxObjectShell> aus der Ressource.
    Die 'nViewId' der zu erzeugenden <SfxViewShell> kann angegeben werden
    (default ist die zuerst registrierte SfxViewShell-Subklasse).
*/

    : SfxViewFrame( *(new SfxBindings), pFrame, pObjShell, SFXFRAME_HASTITLE )
{
    DBG_CTOR(SfxTopViewFrame, 0);

    pCloser = 0;
    pImp = new SfxTopViewFrame_Impl;

//(mba)/task    if ( !pFrame->GetTask() )
    {
        pImp->pWindow = new SfxTopViewWin_Impl( this, &pFrame->GetWindow() );
        pImp->pWindow->SetSizePixel( pFrame->GetWindow().GetOutputSizePixel() );
        SetWindow_Impl( pImp->pWindow );
        pFrame->SetOwnsBindings_Impl( sal_True );
        pFrame->CreateWorkWindow_Impl();
    }

    sal_uInt32 nType = SFXFRAME_OWNSDOCUMENT | SFXFRAME_HASTITLE;
    if ( pObjShell && pObjShell->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
        nType |= SFXFRAME_EXTERNAL;
    GetFrame()->SetFrameType_Impl( GetFrame()->GetFrameType() | nType );

    // ViewShell erzeugen
    if ( pObjShell )
        SwitchToViewShell_Impl( nViewId );

    // initiale Gr"o\se festlegen
    if ( GetViewShell()->UseObjectSize() )
    {
        // Zuerst die logischen Koordinaten von IP-Objekt und EditWindow
        // ber"ucksichtigen
        SfxInPlaceObject *pIPObj = GetObjectShell()->GetInPlaceObject();
        DBG_ASSERT( pIPObj, "UseObjectSize aber kein IP-Object" );
        Window *pWindow = GetViewShell()->GetWindow();

        // Da in den Applikationen bei der R"ucktransformation immer die
        // Eckpunkte tranformiert werden und nicht die Size (um die Ecken
        // alignen zu k"onnen), transformieren wir hier auch die Punkte, um
        // m"oglichst wenig Rundungsfehler zu erhalten.
    /*
        Rectangle aRect = pWindow->LogicToLogic( pIPObj->GetVisArea(),
                                        pIPObj->GetMapUnit(),
                                        pWindow->GetMapMode() );
        aRect = pWindow->LogicToPixel( aRect );
    */
        Rectangle aRect = pWindow->LogicToPixel( pIPObj->GetVisArea() );
        Size aSize = aRect.GetSize();
        GetViewShell()->GetWindow()->SetSizePixel( aSize );
        DoAdjustPosSizePixel(GetViewShell(), Point(), aSize );
    }
}

//------------------------------------------------------------------------
SfxTopViewFrame::~SfxTopViewFrame()
{
    DBG_DTOR(SfxTopViewFrame, 0);

    SetDowning_Impl();

    SfxApplication *pApp = SFX_APP();
    if ( pApp->GetViewFrame() == this )
        pApp->SetViewFrame(NULL);

    ReleaseObjectShell_Impl();
    if ( pPendingCloser == pCloser )
        pPendingCloser = 0;
    delete pCloser;
    if ( GetFrame()->OwnsBindings_Impl() )
        // Die Bindings l"oscht der Frame!
        KillDispatcher_Impl();

    delete pImp->pWindow;
    delete pImp;
}

//------------------------------------------------------------------------
void SfxTopViewFrame::InvalidateBorderImpl
(
    const SfxViewShell*
)
{
    if ( GetViewShell() && GetWindow().IsVisible() )
    {
        if ( GetViewShell()->UseObjectSize() )
        {
            // Zun"achst die Gr"o\se des MDI-Fensters berechnen

            DoAdjustPosSizePixel( GetViewShell(), Point(),
                              GetViewShell()->GetWindow()->GetSizePixel() );

            // Da nach einem InnerResize die Position des EditFensters und
            // damit auch der Tools nocht stimmt, mu\s nun noch einmal von
            // au\sen resized werden !

            ForceOuterResize_Impl(sal_True);
        }

        DoAdjustPosSizePixel( (SfxViewShell *) GetViewShell(), Point(),
                                        GetWindow().GetOutputSizePixel() );
        if ( GetViewShell()->UseObjectSize() )
            ForceOuterResize_Impl(sal_False);
    }
}

//------------------------------------------------------------------------
sal_Bool SfxTopViewFrame::SetBorderPixelImpl( const SfxViewShell *pVSh, const SvBorder &rBorder )
{
    if( SfxViewFrame::SetBorderPixelImpl( GetViewShell(), rBorder ) )
    {
        if ( IsResizeInToOut_Impl() )
        {
            Size aSize = pVSh->GetWindow()->GetOutputSizePixel();
            aSize.Width() += rBorder.Left() + rBorder.Right();
            aSize.Height() += rBorder.Top() + rBorder.Bottom();
            GetWindow().SetOutputSizePixel( aSize );
        }
        else
        {
            Point aPoint;
            Rectangle aEditArea( aPoint, GetWindow().GetOutputSizePixel() );
            aEditArea.Left() += rBorder.Left();
            aEditArea.Right() -= rBorder.Right();
            aEditArea.Top() += rBorder.Top();
            aEditArea.Bottom() -= rBorder.Bottom();
            pVSh->GetWindow()->SetPosSizePixel( aEditArea.TopLeft(), aEditArea.GetSize() );
        }
        return sal_True;

    }
    return sal_False;
}

void SfxTopViewFrame::Exec_Impl(SfxRequest &rReq )
{
    // Wenn gerade die Shells ausgetauscht werden...
    if ( !GetObjectShell() || !GetViewShell() )
        return;

    switch ( rReq.GetSlot() )
    {
        case SID_ACTIVATE:
        {
            MakeActive_Impl( TRUE );
            rReq.SetReturnValue( SfxObjectItem( 0, this ) );
            break;
        }

        case SID_OPTIMIZEWIN:
        case SID_WIN_MINIMIZED:
        case SID_RESTOREWIN:
        case SID_WIN_MAXIMIZED:
        case SID_WIN_POSSIZE:
            break;

        case SID_CLOSEWIN:
        {
            if ( GetViewShell()->PrepareClose() )
            {
                // weitere ::com::sun::star::sdbcx::View auf dasselbe Doc?
                SfxObjectShell *pDocSh = GetObjectShell();
                int bOther = sal_False;
                for ( const SfxTopViewFrame *pFrame =
                          (SfxTopViewFrame *)SfxViewFrame::GetFirst( pDocSh, TYPE(SfxTopViewFrame) );
                      !bOther && pFrame;
                      pFrame = (SfxTopViewFrame *)SfxViewFrame::GetNext( *pFrame, pDocSh, TYPE(SfxTopViewFrame) ) )
                    bOther = (pFrame != this);

                // Doc braucht nur gefragt zu werden, wenn keine weitere ::com::sun::star::sdbcx::View
                sal_Bool bClosed = sal_False;
                if ( ( bOther || pDocSh->PrepareClose() ) )
                {
                    rReq.Done(); // unbedingt vor Close() rufen!
                    if ( rReq.IsAPI() )
                    {
                        if( !pCloser )
                        {
                            if ( pPendingCloser )
                                pPendingCloser->ForcePendingCall();
                            pCloser = new AsynchronLink(
                                Link( 0, SfxViewFrameClose_Impl ) );
                            pCloser->Call( this );
                            pPendingCloser = pCloser;
                        }
                        bClosed = sal_True;
                    }
                    else
                        bClosed = GetFrame()->DoClose();
                }

                rReq.SetReturnValue( SfxBoolItem( rReq.GetSlot(), bClosed ));
            }
            return;
        }
    }

    rReq.Done();
}

void SfxTopViewFrame::GetState_Impl( SfxItemSet &rSet )
{
    SfxObjectShell *pDocSh = GetObjectShell();

    if ( !pDocSh )
        return;

    const sal_uInt16 *pRanges = rSet.GetRanges();
    DBG_ASSERT(pRanges, "Set ohne Bereich");
    while ( *pRanges )
    {
        for ( sal_uInt16 nWhich = *pRanges++; nWhich <= *pRanges; ++nWhich )
        {
            switch(nWhich)
            {
            case SID_OPTIMIZEWIN:
            case SID_NEWWINDOW:
                rSet.DisableItem(nWhich);
                break;

            case SID_CLOSEWIN:
                break;

            case SID_RESTOREWIN:
                rSet.DisableItem(nWhich);
                break;

            case SID_WIN_MINIMIZED:
                rSet.DisableItem( nWhich );
                break;

            case SID_WIN_MAXIMIZED:
                rSet.DisableItem( nWhich );
                break;

            case SID_WIN_POSSIZE:
            {
                rSet.Put( SfxRectangleItem( nWhich, Rectangle(
                        GetWindow().GetPosPixel(), GetWindow().GetSizePixel() ) ) );
                break;
            }

            default:
                DBG_ERROR( "invalid message-id" );
            }
        }
        ++pRanges;
    }
}

void SfxTopViewFrame::INetExecute_Impl( SfxRequest &rRequest )
{
    sal_uInt16 nSlotId = rRequest.GetSlot();
    SfxApplication* pApp = SFX_APP();
    SfxIniManager*  pIniMgr  = pApp->GetIniManager();

    switch( nSlotId )
    {
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_BROWSE_FORWARD:
        case SID_BROWSE_BACKWARD:
        {
            // Anzeige der n"achsten oder vorherigen Seite aus der History
            SFX_REQUEST_ARG( rRequest, pSteps, SfxUInt16Item, nSlotId, sal_False );
            GetFrame()->Browse( nSlotId == SID_BROWSE_FORWARD, pSteps ? pSteps->GetValue() : 1,
                (rRequest.GetModifier() & KEY_MOD1) != 0 );
            break;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_BROWSE_HOME:
        {
            // Anzeige Homepage
            String aHome = pIniMgr->Get( SFX_KEY_INET_HOME );
            if( aHome.Len() )
            {
                SfxStringItem aUrl( SID_FILE_NAME,
                    URIHelper::SmartRelToAbs( aHome ) );
                SfxFrameItem aView( SID_DOCFRAME, this );
                SfxBoolItem aBrowsing( SID_BROWSING, sal_True );
                SfxStringItem aTarget( SID_TARGETNAME, DEFINE_CONST_UNICODE( "_blank" ) );
                SfxStringItem aReferer( SID_REFERER, DEFINE_CONST_UNICODE( "private:user" ) );
                GetDispatcher()->Execute( SID_OPENURL, SFX_CALLMODE_RECORD,
                        &aUrl, &aView, &aBrowsing, &aReferer,
                        ( rRequest.GetModifier() & KEY_MOD1 ) ? &aTarget : 0L,
                        0L );
            }
            break;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_CREATELINK:
        {
/*! (pb) we need new implementation to create a link
*/
            break;
        }
    }
    // Recording
    rRequest.Done();
}

void SfxTopViewFrame::INetState_Impl( SfxItemSet &rItemSet )
{
    if ( !GetFrame()->CanBrowseForward() )
        rItemSet.DisableItem( SID_BROWSE_FORWARD );

    if ( !GetFrame()->CanBrowseBackward() )
        rItemSet.DisableItem( SID_BROWSE_BACKWARD );

    // Add/SaveTo-::com::sun::star::text::Bookmark bei BASIC-IDE, QUERY-EDITOR etc. disablen
    SfxObjectShell *pDocSh = GetObjectShell();
    sal_Bool bPseudo = pDocSh &&
                !( pDocSh->GetFactory().GetFlags() & SFXOBJECTSHELL_HASOPENDOC );
    sal_Bool bEmbedded = pDocSh &&
                pDocSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED;
    if ( !pDocSh || bPseudo || bEmbedded || !pDocSh->HasName() )
        rItemSet.DisableItem( SID_CREATELINK );
}

void SfxTopViewFrame::SetZoomFactor( const Fraction &rZoomX, const Fraction &rZoomY )
{
    GetViewShell()->SetZoomFactor( rZoomX, rZoomY );
}

void SfxTopViewFrame::Activate( sal_Bool bMDI )
{
    DBG_ASSERT(GetViewShell(), "Keine Shell");
    if ( bMDI && !pImp->bActive )
        pImp->bActive = sal_True;
//(mba): hier evtl. wie in Beanframe NotifyEvent ?!
}

void SfxTopViewFrame::Deactivate( sal_Bool bMDI )
{
    DBG_ASSERT(GetViewShell(), "Keine Shell");
    if ( bMDI && pImp->bActive )
        pImp->bActive = sal_False;
//(mba): hier evtl. wie in Beanframe NotifyEvent ?!
}

