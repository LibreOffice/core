/*************************************************************************
 *
 *  $RCSfile: partwnd.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 11:28:01 $
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
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif

#include <toolkit/helper/vclunohelper.hxx>

#include "sfxsids.hrc"
#include "partwnd.hxx"
#include "bindings.hxx"
#include "dispatch.hxx"
#include "viewfrm.hxx"
#include "frame.hxx"
#include "sfxuno.hxx"

/*
// class SfxPartwinFrame_Impl ------------------------------------------

class SfxPartwinFrame_Impl : public SfxUnoFrame
{
public:
    SfxPopupStatusIndicator*        pIndicator;
    SfxPartDockWnd_Impl*            pBeamer;

    virtual void     SAL_CALL               initialize(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > & aPeer) throw ( ::com::sun::star::uno::RuntimeException );
    virtual SfxFrame*               CreateFrame( Window* pParent );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator >   SAL_CALL     getStatusIndicator(void) throw ( ::com::sun::star::uno::RuntimeException );
    void                            dispatch_Impl( const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs );
};

// -----------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > SAL_CALL SfxPartwinFrame_Impl::getStatusIndicator(void) throw ( ::com::sun::star::uno::RuntimeException )
{
    return pIndicator->GetInterface();
}

void SfxPartwinFrame_Impl::dispatch_Impl( const ::com::sun::star::util::URL& rURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rArgs )
{
    if ( !pBeamer )
        return;

    if ( rURL.Protocol.compareToAscii(".uno:") == 0 )
    {
        if ( rURL.Path.compareToAscii("Reload") == 0)
        {
            SfxUnoFrame::dispatch_Impl( rURL, rArgs );
            if ( pBeamer->IsAutoHide_Impl() )
                pBeamer->AutoShow_Impl( sal_True );
            return;
        }
    }
    else
        SfxUnoFrame::dispatch_Impl( rURL, rArgs );

    if ( rURL.Complete.len() )
    {
        if ( pBeamer->IsAutoHide_Impl() )
            pBeamer->AutoShow_Impl( sal_True );
    }
    else
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >  xTmp( this );
        SfxApplication* pApp = SFX_APP();
        pApp->SetChildWindow( SID_BROWSER, sal_False );
        pApp->GetBindings().Invalidate( SID_BROWSER );
    }
}


// -----------------------------------------------------------------------
void SAL_CALL SfxPartwinFrame_Impl::initialize( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > & aWindow ) throw ( ::com::sun::star::uno::RuntimeException )
{
    setName( rtl::OUString::createFromAscii("_partwindow") );
    SetContainerWindow_Impl( aWindow );
}

// -----------------------------------------------------------------------
SfxFrame* SfxPartwinFrame_Impl::CreateFrame( Window* pParent )
{
    return NULL;
}

*/

//****************************************************************************
// SfxPartChildWnd_Impl
//****************************************************************************

SFX_IMPL_DOCKINGWINDOW( SfxPartChildWnd_Impl, SID_BROWSER );

SfxPartChildWnd_Impl::SfxPartChildWnd_Impl
(
    Window* pParent,
    sal_uInt16 nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* pInfo
)
    : SfxChildWindow( pParent, nId )
{
    // Window erzeugen
    pWindow = new SfxPartDockWnd_Impl( pBindings, this, pParent, WB_STDDOCKWIN | WB_CLIPCHILDREN | WB_SIZEABLE | WB_3DLOOK );
    eChildAlignment = SFX_ALIGN_TOP;
    if ( pInfo )
        pInfo->nFlags |= SFX_CHILDWIN_FORCEDOCK;

    ((SfxDockingWindow*)pWindow)->SetFloatingSize( Size( 240, 240 ) );
    pWindow->SetSizePixel( Size( 240, 240 ) );

    ( ( SfxDockingWindow* ) pWindow )->Initialize( pInfo );
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
    SfxBindings* pBindings,
    SfxChildWindow* pChildWin,
    Window* pParent,
    WinBits nBits
)
    : SfxDockingWindow( pBindings, pChildWin, pParent, nBits )
{
    ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame > xFrame(
            ::comphelper::getProcessServiceFactory()->createInstance(
            DEFINE_CONST_UNICODE("com.sun.star.frame.Frame") ), ::com::sun::star::uno::UNO_QUERY );
    xFrame->initialize( VCLUnoHelper::GetInterface ( this ) );
    pChildWin->SetFrame( xFrame );
    if ( pBindings->GetDispatcher() )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFramesSupplier >
                xSupp ( pBindings->GetDispatcher()->GetFrame()->GetFrame()->GetFrameInterface(), ::com::sun::star::uno::UNO_QUERY );
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


