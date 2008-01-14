/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: partwnd.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 17:28:22 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

// includes ******************************************************************

#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATOR_HPP_
#include <com/sun/star/task/XStatusIndicator.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_CLOSEVETOEXCEPTION_HPP_
#include <com/sun/star/util/CloseVetoException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif

#include <toolkit/helper/vclunohelper.hxx>

#include <sfx2/sfxsids.hrc>
#include "partwnd.hxx"
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/sfxuno.hxx>

//****************************************************************************
// SfxPartChildWnd_Impl
//****************************************************************************

SFX_IMPL_DOCKINGWINDOW( SfxPartChildWnd_Impl, SID_BROWSER );

SfxPartChildWnd_Impl::SfxPartChildWnd_Impl
(
    Window* pParentWnd,
    sal_uInt16 nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* pInfo
)
    : SfxChildWindow( pParentWnd, nId )
{
    // Window erzeugen
    pWindow = new SfxPartDockWnd_Impl( pBindings, this, pParentWnd, WB_STDDOCKWIN | WB_CLIPCHILDREN | WB_SIZEABLE | WB_3DLOOK );
    eChildAlignment = SFX_ALIGN_TOP;
    if ( pInfo )
        pInfo->nFlags |= SFX_CHILDWIN_FORCEDOCK;

    ((SfxDockingWindow*)pWindow)->SetFloatingSize( Size( 175, 175 ) );
    pWindow->SetSizePixel( Size( 175, 175 ) );

    ( ( SfxDockingWindow* ) pWindow )->Initialize( pInfo );
    SetHideNotDelete( TRUE );
}

SfxPartChildWnd_Impl::~SfxPartChildWnd_Impl()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xFrame = GetFrame();

    // If xFrame=NULL release pMgr! Because this window lives longer then the manager!
    // In these case we got a xFrame->dispose() call from outside ... and has release our
    // frame reference in our own DisposingListener.
    // But don't do it, if xFrame already exist. Then dispose() must come from inside ...
    // and we need a valid pMgr for further operations ...

    SfxPartDockWnd_Impl* pWin = (SfxPartDockWnd_Impl*) pWindow;
//    if( pWin != NULL && !xFrame.is() )
//        pWin->ReleaseChildWindow_Impl();
/*
    // Release frame and window.
    // If frame reference is valid here ... start dieing from inside by calling dispose().
    SetFrame( NULL );
    pWindow = NULL;
*/
    if ( pWin && xFrame == pWin->GetBindings().GetActiveFrame() )
        pWin->GetBindings().SetActiveFrame( NULL );
/*
    if( xFrame.is() )
    {
        try
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseable > xCloseable  ( xFrame, ::com::sun::star::uno::UNO_QUERY );
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > xDisposeable( xFrame, ::com::sun::star::uno::UNO_QUERY );
            if (xCloseable.is())
                xCloseable->close(sal_True);
            else
            if (xDisposeable.is())
                xDisposeable->dispose();
        }
        catch( ::com::sun::star::util::CloseVetoException& )
        {
        }
        catch( ::com::sun::star::lang::DisposedException& )
        {
        }
    }
 */
}

sal_Bool SfxPartChildWnd_Impl::QueryClose()
{
    return ( (SfxPartDockWnd_Impl*)pWindow )->QueryClose();
}

//****************************************************************************
// SfxPartDockWnd_Impl
//****************************************************************************

SfxPartDockWnd_Impl::SfxPartDockWnd_Impl
(
    SfxBindings* pBind,
    SfxChildWindow* pChildWin,
    Window* pParent,
    WinBits nBits
)
    : SfxDockingWindow( pBind, pChildWin, pParent, nBits )
{
    ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame > xFrame(
            ::comphelper::getProcessServiceFactory()->createInstance(
            DEFINE_CONST_UNICODE("com.sun.star.frame.Frame") ), ::com::sun::star::uno::UNO_QUERY );
    xFrame->initialize( VCLUnoHelper::GetInterface ( this ) );

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xPropSet(
        xFrame, ::com::sun::star::uno::UNO_QUERY );
    try
    {
        const ::rtl::OUString aLayoutManager( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" ));
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xLMPropSet;

        ::com::sun::star::uno::Any a = xPropSet->getPropertyValue( aLayoutManager );
        if ( a >>= xLMPropSet )
        {
            const ::rtl::OUString aAutomaticToolbars( RTL_CONSTASCII_USTRINGPARAM( "AutomaticToolbars" ));
            xLMPropSet->setPropertyValue( aAutomaticToolbars, ::com::sun::star::uno::Any( sal_False ));
        }
    }
    catch( ::com::sun::star::uno::RuntimeException& )
    {
        throw;
    }
    catch( ::com::sun::star::uno::Exception& )
    {
    }

    pChildWin->SetFrame( xFrame );
    if ( pBind->GetDispatcher() )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFramesSupplier >
                xSupp ( pBind->GetDispatcher()->GetFrame()->GetFrame()->GetFrameInterface(), ::com::sun::star::uno::UNO_QUERY );
        if ( xSupp.is() )
            xSupp->getFrames()->append( xFrame );
    }
    else
        DBG_ERROR("Bindings without Dispatcher!");
}

//****************************************************************************

SfxPartDockWnd_Impl::~SfxPartDockWnd_Impl()
{
}

//****************************************************************************

Rectangle impl_Rectangle_Struct2Object( const ::com::sun::star::awt::Rectangle& aRectangleStruct )
{
    return Rectangle(aRectangleStruct.X,aRectangleStruct.Y,aRectangleStruct.Width,aRectangleStruct.Height);
}

void SfxPartDockWnd_Impl::Resize()

/*  [Beschreibung]
    Anpassung der Gr"osse der Controls an die neue Windowgr"osse
*/

{
    SfxDockingWindow::Resize();
}

//****************************************************************************

sal_Bool SfxPartDockWnd_Impl::QueryClose()
{
    sal_Bool bClose = sal_True;
    SfxChildWindow* pChild = GetChildWindow_Impl();
    if( pChild )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xFrame = pChild->GetFrame();
        if( xFrame.is() )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >  xCtrl = xFrame->getController();
            if( xCtrl.is() )
                bClose = xCtrl->suspend( sal_True );
        }
    }

    return bClose;
}

//****************************************************************************

long SfxPartDockWnd_Impl::Notify( NotifyEvent& rEvt )
{
    if( rEvt.GetType() == EVENT_GETFOCUS )
    {
        SfxChildWindow* pChild = GetChildWindow_Impl();
        if( pChild )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xFrame = pChild->GetFrame();
            if( xFrame.is() )
                xFrame->activate();
        }
    }

    return SfxDockingWindow::Notify( rEvt );
}

void SfxPartDockWnd_Impl::FillInfo( SfxChildWinInfo& rInfo ) const
{
    SfxDockingWindow::FillInfo( rInfo );
    rInfo.bVisible = sal_False;
}


