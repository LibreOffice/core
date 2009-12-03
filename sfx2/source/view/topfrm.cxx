/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: topfrm.cxx,v $
 * $Revision: 1.97 $
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
#ifndef GCC
#endif

#include <sfx2/viewfrm.hxx>
#include <sfx2/signaturestate.hxx>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/frame/XFrame.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#ifndef _TOOLKIT_UNOHLP_HXX
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef _UNO_COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#include <com/sun/star/container/XIndexAccess.hpp>
#ifndef _COM_SUN_STAR_CONTAINER_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/XMaterialHolder.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/document/XViewDataSupplier.hpp>
#include <vcl/menu.hxx>
#include <svtools/rectitem.hxx>
#include <svtools/intitem.hxx>
#include <svtools/eitem.hxx>
#include <svtools/stritem.hxx>
#include <svtools/asynclink.hxx>
#include <svtools/sfxecode.hxx>
#include <vcl/dialog.hxx>
#include <svtools/urihelper.hxx>
#include <svtools/moduleoptions.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/bootstrap.hxx>
#include <tools/diagnose_ex.h>

#include <sfxresid.hxx>

// wg. pTopFrames
#include "appdata.hxx"
#include <sfx2/app.hxx>
#include <sfx2/sfx.hrc>
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <sfx2/objitem.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/msg.hxx>
#include "objshimp.hxx"
#include "workwin.hxx"
#include "sfxtypes.hxx"
#include "splitwin.hxx"
#include "arrdecl.hxx"
#include "sfxhelp.hxx"
#include <sfx2/fcontnr.hxx>
#include <sfx2/docfac.hxx>
#include "statcach.hxx"
#include <sfx2/event.hxx>
#include "impframe.hxx"
#include "impviewframe.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using ::com::sun::star::document::XViewDataSupplier;

//------------------------------------------------------------------------

#include <comphelper/sequenceashashmap.hxx>
static ::rtl::OUString GetModuleName_Impl( const ::rtl::OUString& sDocService )
{
    uno::Reference< container::XNameAccess > xMM( ::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.frame.ModuleManager")), uno::UNO_QUERY );
    ::rtl::OUString sVar;
    if ( !xMM.is() )
        return sVar;

    try
    {
        ::comphelper::SequenceAsHashMap aAnalyzer( xMM->getByName(sDocService) );
        sVar = aAnalyzer.getUnpackedValueOrDefault( ::rtl::OUString::createFromAscii("ooSetupFactoryUIName"), ::rtl::OUString() );
    }
    catch( uno::Exception& )
    {
        sVar = ::rtl::OUString();
    }

    return sVar;
}

class SfxTopWindow_Impl : public Window
{
public:
    SfxFrame*           pFrame;

    SfxTopWindow_Impl( SfxFrame* pF, Window& i_rExternalWindow );
    ~SfxTopWindow_Impl( );

    virtual void        DataChanged( const DataChangedEvent& rDCEvt );
    virtual void        StateChanged( StateChangedType nStateChange );
    virtual long        PreNotify( NotifyEvent& rNEvt );
    virtual long        Notify( NotifyEvent& rEvt );
    virtual void        Resize();
    virtual void        GetFocus();
    void                DoResize();
    DECL_LINK(          CloserHdl, void* );
};

SfxTopWindow_Impl::SfxTopWindow_Impl( SfxFrame* pF, Window& i_rExternalWindow )
        : Window( &i_rExternalWindow, WB_BORDER | WB_CLIPCHILDREN | WB_NODIALOGCONTROL | WB_3DLOOK )
        , pFrame( pF )
{
}

SfxTopWindow_Impl::~SfxTopWindow_Impl( )
{
}

void SfxTopWindow_Impl::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );
    SfxWorkWindow *pWorkWin = pFrame->GetWorkWindow_Impl();
    if ( pWorkWin )
        pWorkWin->DataChanged_Impl( rDCEvt );
}

long SfxTopWindow_Impl::Notify( NotifyEvent& rNEvt )
{
    if ( pFrame->IsClosing_Impl() || !pFrame->GetFrameInterface().is() )
        return sal_False;

    SfxViewFrame* pView = pFrame->GetCurrentViewFrame();
    if ( !pView || !pView->GetObjectShell() )
        return Window::Notify( rNEvt );

    if ( rNEvt.GetType() == EVENT_GETFOCUS )
    {
        if ( pView->GetViewShell() && !pView->GetViewShell()->GetUIActiveIPClient_Impl() && !pFrame->IsInPlace() )
        {
            DBG_TRACE("SfxFrame: GotFocus");
            pView->MakeActive_Impl( FALSE );
        }

        // TODO/LATER: do we still need this code?
        Window* pWindow = rNEvt.GetWindow();
        ULONG nHelpId  = 0;
        while ( !nHelpId && pWindow )
        {
            nHelpId = pWindow->GetHelpId();
            pWindow = pWindow->GetParent();
        }

        if ( nHelpId )
            SfxHelp::OpenHelpAgent( pFrame, nHelpId );

        // if focus was on an external window, the clipboard content might have been changed
        pView->GetBindings().Invalidate( SID_PASTE );
        pView->GetBindings().Invalidate( SID_PASTE_SPECIAL );
        return sal_True;
    }
    else if( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        if ( pView->GetViewShell()->KeyInput( *rNEvt.GetKeyEvent() ) )
            return TRUE;
    }
    else if ( rNEvt.GetType() == EVENT_EXECUTEDIALOG /*|| rNEvt.GetType() == EVENT_INPUTDISABLE*/ )
    {
        pView->SetModalMode( sal_True );
        return sal_True;
    }
    else if ( rNEvt.GetType() == EVENT_ENDEXECUTEDIALOG /*|| rNEvt.GetType() == EVENT_INPUTENABLE*/ )
    {
        //EnableInput( sal_True, sal_True );
        pView->SetModalMode( sal_False );
        return sal_True;
    }

    return Window::Notify( rNEvt );
}

long SfxTopWindow_Impl::PreNotify( NotifyEvent& rNEvt )
{
    USHORT nType = rNEvt.GetType();
    if ( nType == EVENT_KEYINPUT || nType == EVENT_KEYUP )
    {
        SfxViewFrame* pView = pFrame->GetCurrentViewFrame();
        SfxViewShell* pShell = pView ? pView->GetViewShell() : NULL;
        if ( pShell && pShell->HasKeyListeners_Impl() && pShell->HandleNotifyEvent_Impl( rNEvt ) )
            return sal_True;
    }
    else if ( nType == EVENT_MOUSEBUTTONUP || nType == EVENT_MOUSEBUTTONDOWN )
    {
        Window* pWindow = rNEvt.GetWindow();
        SfxViewFrame* pView = pFrame->GetCurrentViewFrame();
        SfxViewShell* pShell = pView ? pView->GetViewShell() : NULL;
        if ( pShell )
            if ( pWindow == pShell->GetWindow() || pShell->GetWindow()->IsChild( pWindow ) )
                if ( pShell->HasMouseClickListeners_Impl() && pShell->HandleNotifyEvent_Impl( rNEvt ) )
                    return sal_True;
    }

    if ( nType == EVENT_MOUSEBUTTONDOWN )
    {
        Window* pWindow = rNEvt.GetWindow();
        const MouseEvent* pMEvent = rNEvt.GetMouseEvent();
        Point aPos = pWindow->OutputToScreenPixel( pMEvent->GetPosPixel() );
        SfxWorkWindow *pWorkWin = pFrame->GetWorkWindow_Impl();
        if ( pWorkWin )
            pWorkWin->EndAutoShow_Impl( aPos );
    }

    return Window::PreNotify( rNEvt );
}

void SfxTopWindow_Impl::GetFocus()
{
    if ( pFrame && !pFrame->IsClosing_Impl() && pFrame->GetCurrentViewFrame() && pFrame->GetFrameInterface().is() )
        pFrame->GetCurrentViewFrame()->MakeActive_Impl( TRUE );
}

void SfxTopWindow_Impl::Resize()
{
    if ( IsReallyVisible() || IsReallyShown() || GetOutputSizePixel().Width() )
        DoResize();
}

void SfxTopWindow_Impl::StateChanged( StateChangedType nStateChange )
{
    if ( nStateChange == STATE_CHANGE_INITSHOW )
    {
        pFrame->pImp->bHidden = FALSE;
        if ( pFrame->IsInPlace() )
            // TODO/MBA: workaround for bug in LayoutManager: the final resize does not get through because the
            // LayoutManager works asynchronously and between resize and time execution the DockingAcceptor was exchanged so that
            // the resize event never is sent to the component
            SetSizePixel( GetParent()->GetOutputSizePixel() );

        DoResize();
        SfxViewFrame* pView = pFrame->GetCurrentViewFrame();
        if ( pView )
            pView->GetBindings().GetWorkWindow_Impl()->ShowChilds_Impl();
    }

    Window::StateChanged( nStateChange );
}

void SfxTopWindow_Impl::DoResize()
{
    if ( !pFrame->pImp->bLockResize )
        pFrame->Resize();
}

//--------------------------------------------------------------------
void SfxTopViewWin_Impl::StateChanged( StateChangedType nStateChange )
{
    if ( nStateChange == STATE_CHANGE_INITSHOW )
    {
        SfxObjectShell* pDoc = pFrame->GetObjectShell();
        if ( pDoc && !pFrame->IsVisible_Impl() )
            pFrame->Show();

        pFrame->Resize();
    }
    else
        Window::StateChanged( nStateChange );
}

void SfxTopViewWin_Impl::Resize()
{
    if ( IsReallyVisible() || IsReallyShown() || GetOutputSizePixel().Width() )
        pFrame->Resize();
}

class SfxTopViewFrame_Impl
{
public:

                        SfxTopViewFrame_Impl()
                        {}
};

static String _getTabString()
{
    String result;

    Reference < XMaterialHolder > xHolder(
        ::comphelper::getProcessServiceFactory()->createInstance(
        DEFINE_CONST_UNICODE("com.sun.star.tab.tabreg") ), UNO_QUERY );
    if (xHolder.is())
    {
        rtl::OUString aTabString;
        Sequence< NamedValue > sMaterial;
        if (xHolder->getMaterial() >>= sMaterial) {
            for (int i=0; i < sMaterial.getLength(); i++) {
                if ((sMaterial[i].Name.equalsAscii("title")) &&
                    (sMaterial[i].Value >>= aTabString))
                {
                    result += ' ';
                    result += String(aTabString);
                }
            }
        }
    }
    return result;
}

SfxFrame* SfxFrame::Create( SfxObjectShell* pDoc, USHORT nViewId, bool bHidden, const SfxItemSet* pSet )
{
    Reference < XFrame > xDesktop ( ::comphelper::getProcessServiceFactory()->createInstance( DEFINE_CONST_UNICODE("com.sun.star.frame.Desktop") ), UNO_QUERY );
    SfxFrame *pFrame = NULL;
    BOOL bNewView = FALSE;
    if ( pSet )
    {
        SFX_ITEMSET_ARG( pSet, pItem, SfxBoolItem, SID_OPEN_NEW_VIEW, sal_False );
        bNewView = pItem && pItem->GetValue();
    }

    if ( pDoc && !bHidden && !bNewView )
    {
        URL aTargetURL;
        aTargetURL.Complete = pDoc->GetMedium()->GetURLObject().GetMainURL( INetURLObject::NO_DECODE );

        BOOL bIsBasic = FALSE;
        if ( !aTargetURL.Complete.getLength() )
        {
            String sFactory = String::CreateFromAscii(pDoc->GetFactory().GetShortName());
            bIsBasic = (sFactory.CompareIgnoreCaseToAscii("sbasic")==COMPARE_EQUAL);

            if (!bIsBasic)
            {
                String aURL = String::CreateFromAscii("private:factory/");
                aURL += sFactory;
                aTargetURL.Complete = aURL;
            }
        }

        if (bIsBasic)
        {
            Reference < XFramesSupplier > xSupplier( xDesktop, UNO_QUERY );
            if (xSupplier.is())
            {
                Reference < XIndexAccess > xContainer(xSupplier->getFrames(), UNO_QUERY);
                if (xContainer.is())
                {
                    Reference< ::com::sun::star::frame::XModuleManager > xCheck(::comphelper::getProcessServiceFactory()->createInstance( rtl::OUString::createFromAscii("com.sun.star.frame.ModuleManager" )), UNO_QUERY);
                    sal_Int32 nCount = xContainer->getCount();
                    for (sal_Int32 i=0; i<nCount; ++i)
                    {
                        try
                        {
                            Reference < XFrame > xFrame;
                            if (!(xContainer->getByIndex(i) >>= xFrame) || !xFrame.is())
                                continue;
                            ::rtl::OUString sModule = xCheck->identify(xFrame);
                            if (sModule.equalsAscii("com.sun.star.frame.StartModule"))
                            {
                                pFrame = Create(xFrame);
                                break;
                            }
                        }
                        catch(const Exception&) {}
                    }
                }
            }
        }
        else
        {
            Reference < XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance( rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer" )), UNO_QUERY );
            xTrans->parseStrict( aTargetURL );

            Reference < ::com::sun::star::frame::XDispatchProvider > xProv( xDesktop, UNO_QUERY );
            Reference < ::com::sun::star::frame::XDispatch > xDisp;
            if ( xProv.is() )
            {
                Sequence < ::com::sun::star::beans::PropertyValue > aSeq(1);
                aSeq[0].Name = ::rtl::OUString::createFromAscii("Model");
                aSeq[0].Value <<= pDoc->GetModel();
                ::rtl::OUString aTargetFrame( ::rtl::OUString::createFromAscii("_default") );
                xDisp = xProv->queryDispatch( aTargetURL, aTargetFrame , 0 );
                if ( xDisp.is() )
                    xDisp->dispatch( aTargetURL, aSeq );
            }

            SfxFrameArr_Impl& rArr = *SFX_APP()->Get_Impl()->pTopFrames;
            for( USHORT nPos = rArr.Count(); nPos--; )
            {
                SfxFrame *pF = rArr[ nPos ];
                if ( pF->GetCurrentDocument() == pDoc )
                {
                    pFrame = pF;
                    break;
                }
            }
        }
    }

    if ( !pFrame  )
    {
        Reference < XFrame > xFrame = xDesktop->findFrame( DEFINE_CONST_UNICODE("_blank"), 0 );
        pFrame = Create( xFrame );
    }

    pFrame->pImp->bHidden = bHidden;
    Window* pWindow = pFrame->GetTopWindow_Impl();
    if ( pWindow && pDoc )
    {
        ::rtl::OUString aDocServiceName( pDoc->GetFactory().GetDocumentServiceName() );
        ::rtl::OUString aProductName;
        ::utl::ConfigManager::GetDirectConfigProperty(::utl::ConfigManager::PRODUCTNAME) >>= aProductName;
        String aTitle = pDoc->GetTitle( SFX_TITLE_DETECT );
        aTitle += String::CreateFromAscii( " - " );
        aTitle += String(aProductName);
        aTitle += ' ';
        aTitle += String( GetModuleName_Impl( aDocServiceName ) );
#ifndef PRODUCT
        ::rtl::OUString aDefault;
        aTitle += DEFINE_CONST_UNICODE(" [");
        String aVerId( utl::Bootstrap::getBuildIdData( aDefault ));
        aTitle += aVerId;
        aTitle += ']';
#endif

        // append TAB string if available
        aTitle += _getTabString();

        /* AS_TITLE
        pWindow->SetText( aTitle );
        */

        /* AS_ICON
        if( pWindow->GetType() == WINDOW_WORKWINDOW )
        {
            SvtModuleOptions::EFactory eFactory;
            if( SvtModuleOptions::ClassifyFactoryByName( aDocServiceName, eFactory ) )
            {
                WorkWindow* pWorkWindow = (WorkWindow*)pWindow;
                pWorkWindow->SetIcon( (sal_uInt16) SvtModuleOptions().GetFactoryIcon( eFactory ) );
            }
        }
        */
    }

    if ( pDoc && pDoc != pFrame->GetCurrentDocument() )
    {
        if ( nViewId )
            pDoc->GetMedium()->GetItemSet()->Put( SfxUInt16Item( SID_VIEW_ID, nViewId ) );
        pFrame->InsertDocument_Impl( *pDoc, pSet ? *pSet : *pDoc->GetMedium()->GetItemSet() );
        if ( pWindow && !bHidden )
            pWindow->Show();
    }

    return pFrame;
}

SfxFrame* SfxFrame::Create( SfxObjectShell* pDoc, Window& rWindow, USHORT nViewId, bool bHidden, const SfxItemSet* pSet )
{
    Reference < ::com::sun::star::lang::XMultiServiceFactory > xFact( ::comphelper::getProcessServiceFactory() );
    Reference < XFramesSupplier > xDesktop ( xFact->createInstance( DEFINE_CONST_UNICODE("com.sun.star.frame.Desktop") ), UNO_QUERY );
    Reference < XFrame > xFrame( xFact->createInstance( DEFINE_CONST_UNICODE("com.sun.star.frame.Frame") ), UNO_QUERY );

    uno::Reference< awt::XWindow2 > xWin( VCLUnoHelper::GetInterface ( &rWindow ), uno::UNO_QUERY );
    xFrame->initialize( xWin.get() );
    if ( xDesktop.is() )
        xDesktop->getFrames()->append( xFrame );

    if ( xWin.is() && xWin->isActive() )
        xFrame->activate();

    SfxFrame* pFrame = new SfxFrame( rWindow, false );
    pFrame->SetFrameInterface_Impl( xFrame );
    pFrame->pImp->bHidden = bHidden;

    if ( pDoc )
    {
        if ( nViewId )
            pDoc->GetMedium()->GetItemSet()->Put( SfxUInt16Item( SID_VIEW_ID, nViewId ) );
        pFrame->InsertDocument_Impl( *pDoc, pSet ? *pSet : *pDoc->GetMedium()->GetItemSet() );
    }

    return pFrame;
}

SfxFrame* SfxFrame::Create( Reference < XFrame > xFrame )
{
    // create a new TopFrame to an external XFrame object ( wrap controller )
    ENSURE_OR_THROW( xFrame.is(), "NULL frame not allowed" );
    Window* pWindow = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
    ENSURE_OR_THROW( pWindow, "frame without container window not allowed" );

    SfxFrame* pFrame = new SfxFrame( *pWindow, false );
    pFrame->SetFrameInterface_Impl( xFrame );
    return pFrame;
}

SfxFrame::SfxFrame( Window& i_rExternalWindow, bool i_bHidden )
    :pParentFrame( NULL )
    ,pChildArr( NULL )
    ,pImp( NULL )
    ,pWindow( NULL )
{
    Construct_Impl();

    pImp->bHidden = i_bHidden;
    InsertTopFrame_Impl( this );
    pImp->pExternalWindow = &i_rExternalWindow;

    pWindow = new SfxTopWindow_Impl( this, i_rExternalWindow );
}

void SfxFrame::SetPresentationMode( BOOL bSet )
{
    if ( GetCurrentViewFrame() )
        GetCurrentViewFrame()->GetWindow().SetBorderStyle( bSet ? WINDOW_BORDER_NOBORDER : WINDOW_BORDER_NORMAL );

    Reference< com::sun::star::beans::XPropertySet > xPropSet( GetFrameInterface(), UNO_QUERY );
    Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;

    if ( xPropSet.is() )
    {
        Any aValue = xPropSet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" )));
        aValue >>= xLayoutManager;
    }

    if ( xLayoutManager.is() )
        xLayoutManager->setVisible( !bSet ); // we don't want to have ui in presentation mode

    SetMenuBarOn_Impl( !bSet );
    if ( GetWorkWindow_Impl() )
        GetWorkWindow_Impl()->SetDockingAllowed( !bSet );
    if ( GetCurrentViewFrame() )
        GetCurrentViewFrame()->GetDispatcher()->Update_Impl( TRUE );
}

SystemWindow* SfxFrame::GetSystemWindow() const
{
    return GetTopWindow_Impl();
}

SystemWindow* SfxFrame::GetTopWindow_Impl() const
{
    if ( pImp->pExternalWindow->IsSystemWindow() )
        return (SystemWindow*) pImp->pExternalWindow;
    else
        return NULL;
}

Window& SfxFrame::GetWindow() const
{
    return *pWindow;
}

sal_Bool SfxFrame::Close()
{
    delete this;
    return sal_True;
}

void SfxFrame::LockResize_Impl( BOOL bLock )
{
    pImp->bLockResize = bLock;
}

IMPL_LINK( SfxTopWindow_Impl, CloserHdl, void*, EMPTYARG )
{
    if ( pFrame && !pFrame->PrepareClose_Impl( TRUE ) )
        return 0L;

    if ( pFrame )
        pFrame->GetCurrentViewFrame()->GetBindings().Execute( SID_CLOSEWIN, 0, 0, SFX_CALLMODE_ASYNCHRON );
    return 0L;
}

void SfxFrame::SetMenuBarOn_Impl( BOOL bOn )
{
    pImp->bMenuBarOn = bOn;

    Reference< com::sun::star::beans::XPropertySet > xPropSet( GetFrameInterface(), UNO_QUERY );
    Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager;

    if ( xPropSet.is() )
    {
        Any aValue = xPropSet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" )));
        aValue >>= xLayoutManager;
    }

    if ( xLayoutManager.is() )
    {
        rtl::OUString aMenuBarURL( RTL_CONSTASCII_USTRINGPARAM( "private:resource/menubar/menubar" ));

        if ( bOn )
            xLayoutManager->showElement( aMenuBarURL );
        else
            xLayoutManager->hideElement( aMenuBarURL );
    }
}

BOOL SfxFrame::IsMenuBarOn_Impl() const
{
    return pImp->bMenuBarOn;
}

void SfxFrame::PositionWindow_Impl( const Rectangle& rWinArea ) const
{
    Window *pWin = pImp->pExternalWindow;

    // Groesse setzen
    const Size aAppWindow( pImp->pExternalWindow->GetDesktopRectPixel().GetSize() );
    Point aPos( rWinArea.TopLeft() );
    Size aSz( rWinArea.GetSize() );
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

namespace
{
    bool lcl_getViewDataAndID( const Reference< XModel >& _rxDocument, Sequence< PropertyValue >& _o_viewData, USHORT& _o_viewId )
    {
        _o_viewData.realloc(0);
        _o_viewId = 0;

        Reference< XViewDataSupplier > xViewDataSupplier( _rxDocument, UNO_QUERY );
        Reference< XIndexAccess > xViewData;
        if ( xViewDataSupplier.is() )
            xViewData = xViewDataSupplier->getViewData();

        if ( !xViewData.is() || ( xViewData->getCount() == 0 ) )
            return false;

        // obtain the ViewID from the view data
        _o_viewId = 0;
        if ( xViewData->getByIndex( 0 ) >>= _o_viewData )
        {
            ::comphelper::NamedValueCollection aNamedUserData( _o_viewData );
            ::rtl::OUString sViewId = aNamedUserData.getOrDefault( "ViewId", ::rtl::OUString() );
            if ( sViewId.getLength() )
            {
                sViewId = sViewId.copy( 4 );    // format is like in "view3"
                _o_viewId = USHORT( sViewId.toInt32() );
            }
        }
        return true;
    }
}

sal_Bool SfxFrame::InsertDocument_Impl( SfxObjectShell& rDoc, const SfxItemSet& rSet )
/* [Beschreibung]
 */
{
    OSL_PRECOND( rDoc.GetMedium(), "SfxFrame::InsertDocument_Impl: no medium -> no view!");
    if ( !rDoc.GetMedium() )
        return sal_False;

    OSL_PRECOND( GetCurrentViewFrame() == NULL,
        "SfxObjectShell::InsertDocument_Impl: no support (anymore) for loading into a non-empty frame!" );
        // Since some refactoring in CWS autorecovery, this shouldn't happen anymore. Frame re-usage is nowadays
        // done in higher layers, namely in the framework.
    if ( GetCurrentViewFrame() != NULL )
        return sal_False;

    OSL_PRECOND( GetCurrentDocument() == NULL,
        "SfxFrame::InsertDocument_Impl: re-using an Sfx(Top)Frame is not supported anymore!" );

    SFX_ITEMSET_ARG( &rSet, pAreaItem,   SfxRectangleItem,   SID_VIEW_POS_SIZE,  sal_False );    // position and size
    SFX_ITEMSET_ARG( &rSet, pViewIdItem, SfxUInt16Item,      SID_VIEW_ID,        sal_False );    // view ID
    SFX_ITEMSET_ARG( &rSet, pModeItem,   SfxUInt16Item,      SID_VIEW_ZOOM_MODE, sal_False );    // zoom
    SFX_ITEMSET_ARG( &rSet, pHidItem,    SfxBoolItem,        SID_HIDDEN,         sal_False );    // hidden
    SFX_ITEMSET_ARG( &rSet, pViewDataItem, SfxStringItem,    SID_USER_DATA,      sal_False );    // view data
    SFX_ITEMSET_ARG( &rSet, pEditItem,   SfxBoolItem,        SID_VIEWONLY,       sal_False );    // view only
    SFX_ITEMSET_ARG( &rSet, pPluginMode, SfxUInt16Item,      SID_PLUGIN_MODE,    sal_False );    // plugin (external inplace)
    SFX_ITEMSET_ARG( &rSet, pJumpItem,   SfxStringItem,      SID_JUMPMARK,       sal_False );    // jump (GotoBookmark)

    // hidden?
    OSL_PRECOND( !pImp->bHidden,
        "SfxFrame::InsertDocument_Impl: quite unexpected ... the below logic might not work in all cases here ..." );
    pImp->bHidden = pHidItem ? pHidItem->GetValue() : false;

    // plugin mode
    const USHORT nPluginMode = pPluginMode ? pPluginMode->GetValue() : 0;

    // view only?
    if ( pEditItem && pEditItem->GetValue() )
        SetMenuBarOn_Impl( FALSE );

    // view ID
    USHORT nViewId = pViewIdItem ? pViewIdItem->GetValue() : 0;

    if( !pImp->bHidden )
        rDoc.OwnerLock( sal_True );

    Sequence< PropertyValue > aUserData;
    bool bClearPosSizeZoom = false;
    bool bReadUserData = false;

    // if no view-related data exists in the set, then obtain the view data from the model
    if ( !pJumpItem && !pViewDataItem && !pPluginMode && !pAreaItem && !pViewIdItem && !pModeItem )
    {
        if ( lcl_getViewDataAndID( rDoc.GetModel(), aUserData, nViewId ) )
        {
            SfxItemSet* pMediumSet = rDoc.GetMedium()->GetItemSet();

            // clear the user data item in the medium - we'll use aUserData below
            pMediumSet->ClearItem( SID_USER_DATA );
            pMediumSet->Put( SfxUInt16Item( SID_VIEW_ID, nViewId ) );

            bClearPosSizeZoom = bReadUserData = true;
        }
    }

    UpdateDescriptor( &rDoc );

    if ( nPluginMode && ( nPluginMode != 2 ) )
        SetInPlace_Impl( TRUE );

    SfxViewFrame* pViewFrame = new SfxViewFrame( this, &rDoc );
    if ( !pViewFrame->SwitchToViewShell_Impl( nViewId ) )
    {   // TODO: better error handling? Under which conditions can this fail?
        OSL_ENSURE( false, "SfxFrame::InsertDocument_Impl: something went wrong while creating the SfxViewFrame!" );
        pViewFrame->DoClose();
        return sal_False;
    }

    if ( nPluginMode == 1 )
    {
        pViewFrame->ForceOuterResize_Impl( FALSE );
        pViewFrame->GetBindings().HidePopups(TRUE);

        // MBA: layoutmanager of inplace frame starts locked and invisible
        GetWorkWindow_Impl()->MakeVisible_Impl( FALSE );
        GetWorkWindow_Impl()->Lock_Impl( TRUE );

        GetWindow().SetBorderStyle( WINDOW_BORDER_NOBORDER );
        pViewFrame->GetWindow().SetBorderStyle( WINDOW_BORDER_NOBORDER );
    }

    OSL_ENSURE( ( rDoc.Get_Impl()->nLoadedFlags & SFX_LOADED_MAINDOCUMENT ) == SFX_LOADED_MAINDOCUMENT,
        "SfxFrame::InsertDocument_Impl: so this code wasn't dead?" );
        // Before CWS autorecovery, there was code which postponed setting the ViewData/Mark to a later time
        // (SfxObjectShell::PositionView_Impl), but it seems this branch was never used, since this method
        // here is never called before the load process finished.
    if ( pViewDataItem )
    {
        pViewFrame->GetViewShell()->ReadUserData( pViewDataItem->GetValue(), sal_True );
    }
    else if( pJumpItem )
    {
        pViewFrame->GetViewShell()->JumpToMark( pJumpItem->GetValue() );
    }

    // Position und Groesse setzen
    if ( pAreaItem )
        PositionWindow_Impl( pAreaItem->GetValue() );

    if ( !pImp->bHidden )
    {
        if ( rDoc.IsHelpDocument() || ( nPluginMode == 2 ) )
            pViewFrame->GetDispatcher()->HideUI( TRUE );
        else
            pViewFrame->GetDispatcher()->HideUI( FALSE );

        if ( IsInPlace() )
            pViewFrame->LockAdjustPosSizePixel();

        if ( ( nPluginMode == 3 ) )
            GetWorkWindow_Impl()->SetInternalDockingAllowed(FALSE);

        if ( !IsInPlace() )
            pViewFrame->GetDispatcher()->Update_Impl();
        pViewFrame->Show();
        GetWindow().Show();
        if ( !IsInPlace() || ( nPluginMode == 3 ) )
            pViewFrame->MakeActive_Impl( GetFrameInterface()->isActive() );
        rDoc.OwnerLock( sal_False );

        if ( IsInPlace() )
        {
            pViewFrame->UnlockAdjustPosSizePixel();
            // force resize for OLE server to fix layout problems of writer and math
            // see i53651
            if ( nPluginMode == 3 )
                pViewFrame->Resize(TRUE);
        }
    }
    else
    {
        DBG_ASSERT( !IsInPlace() && !pPluginMode, "Special modes not compatible with hidden mode!" );
        GetWindow().Show();
    }

    // Jetzt UpdateTitle, hidden TopFrames haben sonst keinen Namen!
    pViewFrame->UpdateTitle();

    if ( !IsInPlace() )
    {
        pViewFrame->Resize(TRUE);
    }

    SFX_APP()->NotifyEvent( SfxEventHint(SFX_EVENT_VIEWCREATED, GlobalEventConfig::GetEventName( STR_EVENT_VIEWCREATED ), &rDoc ) );

    if ( bClearPosSizeZoom )
    {
        SfxItemSet* pMediumSet = rDoc.GetMedium()->GetItemSet();
        pMediumSet->ClearItem( SID_VIEW_POS_SIZE );
        pMediumSet->ClearItem( SID_WIN_POSSIZE );
        pMediumSet->ClearItem( SID_VIEW_ZOOM_MODE );
    }

    if ( bReadUserData )
    {
        // UserData hier einlesen, da es ansonsten immer mit bBrowse=TRUE
        // aufgerufen wird, beim Abspeichern wurde aber bBrowse=FALSE verwendet
        if ( pViewFrame && pViewFrame->GetViewShell() && aUserData.getLength() )
        {
            pViewFrame->GetViewShell()->ReadUserDataSequence( aUserData, TRUE );
        }
    }

    return GetCurrentDocument() == &rDoc;
}


//========================================================================

long SfxViewFrameClose_Impl( void* /*pObj*/, void* pArg )
{
    ((SfxViewFrame*)pArg)->GetFrame()->DoClose();
    return 0;
}

//--------------------------------------------------------------------
String SfxViewFrame::UpdateTitle()

/*  [Beschreibung]

    Mit dieser Methode kann der SfxViewFrame gezwungen werden, sich sofort
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
                    for ( SfxViewFrame *pTop = SfxViewFrame::GetFirst( this );
                          pTop;
                          pTop = SfxViewFrame::GetNext( this );
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
    DBG_CHKTHIS(SfxViewFrame, 0);

    const SfxObjectFactory &rFact = GetObjectShell()->GetFactory();
    pImp->aFactoryName = String::CreateFromAscii( rFact.GetShortName() );

    SfxObjectShell *pObjSh = GetObjectShell();
    if ( !pObjSh )
        return String();

//    if  ( pObjSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
//        // kein UpdateTitle mit Embedded-ObjectShell
//        return String();

    const SfxMedium *pMedium = pObjSh->GetMedium();
    String aURL;
    GetFrame();  // -Wall required??
    if ( pObjSh->HasName() )
    {
        INetURLObject aTmp( pMedium->GetName() );
        aURL = aTmp.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
    }

    if ( aURL != pImp->aActualURL )
        // URL hat sich ge"andert
        pImp->aActualURL = aURL;

    // gibt es noch eine weitere View?
    sal_uInt16 nViews=0;
    for ( SfxViewFrame *pView= GetFirst(pObjSh);
          pView && nViews<2;
          pView = GetNext(*pView,pObjSh) )
        if ( ( pView->GetFrameType() & SFXFRAME_HASTITLE ) &&
             !IsDowning_Impl())
            nViews++;

    // Titel des Fensters
    String aTitle;
    if ( nViews == 2 || pImp->nDocViewNo > 1 )
        // dann die Nummer dranh"angen
        aTitle = pObjSh->UpdateTitle( NULL, pImp->nDocViewNo );
    else
        aTitle = pObjSh->UpdateTitle();

    // Name des SbxObjects
    String aSbxName = pObjSh->SfxShell::GetName();
    if ( IsVisible_Impl() )
    {
        aSbxName += ':';
        aSbxName += String::CreateFromInt32(pImp->nDocViewNo);
    }

    SetName( aSbxName );
    pImp->aFrameTitle = aTitle;
    GetBindings().Invalidate( SID_FRAMETITLE );
    GetBindings().Invalidate( SID_CURRENT_URL );

    ::rtl::OUString aProductName;
    ::utl::ConfigManager::GetDirectConfigProperty(::utl::ConfigManager::PRODUCTNAME) >>= aProductName;

    aTitle += String::CreateFromAscii( " - " );
    aTitle += String(aProductName);
    aTitle += ' ';
    ::rtl::OUString aDocServiceName( GetObjectShell()->GetFactory().GetDocumentServiceName() );
    aTitle += String( GetModuleName_Impl( aDocServiceName ) );
#ifndef PRODUCT
    ::rtl::OUString aDefault;
    aTitle += DEFINE_CONST_UNICODE(" [");
    String aVerId( utl::Bootstrap::getBuildIdData( aDefault ));
    aTitle += aVerId;
    aTitle += ']';
#endif

    // append TAB string if available
    aTitle += _getTabString();

    GetBindings().Invalidate( SID_NEWDOCDIRECT );

    /* AS_TITLE
    Window* pWindow = GetFrame()->GetTopWindow_Impl();
    if ( pWindow && pWindow->GetText() != aTitle )
        pWindow->SetText( aTitle );
    */
    return aTitle;
}

void SfxViewFrame::Exec_Impl(SfxRequest &rReq )
{
    // Wenn gerade die Shells ausgetauscht werden...
    if ( !GetObjectShell() || !GetViewShell() )
        return;

    switch ( rReq.GetSlot() )
    {
        case SID_SHOWPOPUPS :
        {
            SFX_REQUEST_ARG(rReq, pShowItem, SfxBoolItem, SID_SHOWPOPUPS, FALSE);
            BOOL bShow = pShowItem ? pShowItem->GetValue() : TRUE;
            SFX_REQUEST_ARG(rReq, pIdItem, SfxUInt16Item, SID_CONFIGITEMID, FALSE);
            USHORT nId = pIdItem ? pIdItem->GetValue() : 0;

            // ausfuehren
            SfxWorkWindow *pWorkWin = GetFrame()->GetWorkWindow_Impl();
            if ( bShow )
            {
                // Zuerst die Floats auch anzeigbar machen
                pWorkWin->MakeChildsVisible_Impl( bShow );
                GetDispatcher()->Update_Impl( TRUE );

                // Dann anzeigen
                GetBindings().HidePopups( !bShow );
            }
            else
            {
                // Alles hiden
                SfxBindings *pBind = &GetBindings();
                while ( pBind )
                {
                    pBind->HidePopupCtrls_Impl( !bShow );
                    pBind = pBind->GetSubBindings_Impl();
                }

                pWorkWin->HidePopups_Impl( !bShow, TRUE, nId );
                pWorkWin->MakeChildsVisible_Impl( bShow );
            }

            Invalidate( rReq.GetSlot() );
            rReq.Done();
            break;
        }

        case SID_ACTIVATE:
        {
            MakeActive_Impl( TRUE );
            rReq.SetReturnValue( SfxObjectItem( 0, this ) );
            break;
        }

        case SID_WIN_POSSIZE:
            break;

        case SID_NEWDOCDIRECT :
        {
            SFX_REQUEST_ARG( rReq, pFactoryItem, SfxStringItem, SID_NEWDOCDIRECT, FALSE);
            String aFactName;
            if ( pFactoryItem )
                aFactName = pFactoryItem->GetValue();
            else if ( pImp->aFactoryName.Len() )
                aFactName = pImp->aFactoryName;
            else
            {
                DBG_ERROR("Missing argument!");
                break;
            }

            SfxRequest aReq( SID_OPENDOC, SFX_CALLMODE_SYNCHRON, GetPool() );
            String aFact = String::CreateFromAscii("private:factory/");
            aFact += aFactName;
            aReq.AppendItem( SfxStringItem( SID_FILE_NAME, aFact ) );
            aReq.AppendItem( SfxFrameItem( SID_DOCFRAME, GetFrame() ) );
            aReq.AppendItem( SfxStringItem( SID_TARGETNAME, String::CreateFromAscii( "_blank" ) ) );
            SFX_APP()->ExecuteSlot( aReq );
            const SfxViewFrameItem* pItem = PTR_CAST( SfxViewFrameItem, aReq.GetReturnValue() );
            if ( pItem )
                rReq.SetReturnValue( SfxFrameItem( 0, pItem->GetFrame() ) );
            break;
        }

        case SID_CLOSEWIN:
        {
            // disable CloseWin, if frame is not a task
            Reference < XCloseable > xTask( GetFrame()->GetFrameInterface(),  UNO_QUERY );
            if ( !xTask.is() )
                break;

            if ( GetViewShell()->PrepareClose() )
            {
                // weitere Views auf dasselbe Doc?
                SfxObjectShell *pDocSh = GetObjectShell();
                int bOther = sal_False;
                for ( const SfxViewFrame* pFrame = SfxViewFrame::GetFirst( pDocSh );
                      !bOther && pFrame;
                      pFrame = SfxViewFrame::GetNext( *pFrame, pDocSh ) )
                    bOther = (pFrame != this);

                // Doc braucht nur gefragt zu werden, wenn keine weitere View
                sal_Bool bClosed = sal_False;
                sal_Bool bUI = TRUE;
                if ( ( bOther || pDocSh->PrepareClose( bUI ) ) )
                {
                    if ( !bOther )
                        pDocSh->SetModified( FALSE );
                    rReq.Done(); // unbedingt vor Close() rufen!
                    bClosed = sal_False;
                    try
                    {
                        xTask->close(sal_True);
                        bClosed = sal_True;
                    }
                    catch( CloseVetoException& )
                    {
                        bClosed = sal_False;
                    }
                }

                rReq.SetReturnValue( SfxBoolItem( rReq.GetSlot(), bClosed ));
            }
            return;
        }
    }

    rReq.Done();
}

void SfxViewFrame::GetState_Impl( SfxItemSet &rSet )
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
            case SID_NEWDOCDIRECT :
            {
                if ( pImp->aFactoryName.Len() )
                {
                    String aFact = String::CreateFromAscii("private:factory/");
                    aFact += pImp->aFactoryName;
                    rSet.Put( SfxStringItem( nWhich, aFact ) );
                }
                break;
            }

            case SID_NEWWINDOW:
                rSet.DisableItem(nWhich);
                break;

            case SID_CLOSEWIN:
            {
                // disable CloseWin, if frame is not a task
                Reference < XCloseable > xTask( GetFrame()->GetFrameInterface(),  UNO_QUERY );
                if ( !xTask.is() )
                    rSet.DisableItem(nWhich);
                break;
            }

            case SID_SHOWPOPUPS :
                break;

            case SID_WIN_POSSIZE:
            {
                rSet.Put( SfxRectangleItem( nWhich, Rectangle(
                        GetWindow().GetPosPixel(), GetWindow().GetSizePixel() ) ) );
                break;
            }

            case SID_OBJECT:
                if ( GetViewShell() && GetViewShell()->GetVerbs().getLength() && !GetObjectShell()->IsInPlaceActive() )
                {
                    uno::Any aAny;
                    aAny <<= GetViewShell()->GetVerbs();
                    rSet.Put( SfxUnoAnyItem( USHORT( SID_OBJECT ), aAny ) );
                }
                else
                    rSet.DisableItem( SID_OBJECT );
                break;

            default:
                DBG_ERROR( "invalid message-id" );
            }
        }
        ++pRanges;
    }
}

void SfxViewFrame::INetExecute_Impl( SfxRequest &rRequest )
{
    sal_uInt16 nSlotId = rRequest.GetSlot();
    switch( nSlotId )
    {
        case SID_BROWSE_FORWARD:
        case SID_BROWSE_BACKWARD:
            OSL_ENSURE( false, "SfxViewFrame::INetExecute_Impl: SID_BROWSE_FORWARD/BACKWARD are dead!" );
            break;
        case SID_CREATELINK:
        {
/*! (pb) we need new implementation to create a link
*/
            break;
        }
        case SID_FOCUSURLBOX:
        {
            SfxStateCache *pCache = GetBindings().GetAnyStateCache_Impl( SID_OPENURL );
            if( pCache )
            {
                SfxControllerItem* pCtrl = pCache->GetItemLink();
                while( pCtrl )
                {
                    pCtrl->StateChanged( SID_FOCUSURLBOX, SFX_ITEM_UNKNOWN, 0 );
                    pCtrl = pCtrl->GetItemLink();
                }
            }
        }
    }

    // Recording
    rRequest.Done();
}

void SfxViewFrame::INetState_Impl( SfxItemSet &rItemSet )
{
    rItemSet.DisableItem( SID_BROWSE_FORWARD );
    rItemSet.DisableItem( SID_BROWSE_BACKWARD );

    // Add/SaveToBookmark bei BASIC-IDE, QUERY-EDITOR etc. disablen
    SfxObjectShell *pDocSh = GetObjectShell();
    sal_Bool bPseudo = pDocSh && !( pDocSh->GetFactory().GetFlags() & SFXOBJECTSHELL_HASOPENDOC );
    sal_Bool bEmbedded = pDocSh && pDocSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED;
    if ( !pDocSh || bPseudo || bEmbedded || !pDocSh->HasName() )
        rItemSet.DisableItem( SID_CREATELINK );
}

void SfxViewFrame::SetZoomFactor( const Fraction &rZoomX, const Fraction &rZoomY )
{
    GetViewShell()->SetZoomFactor( rZoomX, rZoomY );
}

void SfxViewFrame::Activate( sal_Bool bMDI )
{
    DBG_ASSERT(GetViewShell(), "Keine Shell");
    if ( bMDI )
        pImp->bActive = sal_True;
//(mba): hier evtl. wie in Beanframe NotifyEvent ?!
}

void SfxViewFrame::Deactivate( sal_Bool bMDI )
{
    DBG_ASSERT(GetViewShell(), "Keine Shell");
    if ( bMDI )
        pImp->bActive = sal_False;
//(mba): hier evtl. wie in Beanframe NotifyEvent ?!
}
