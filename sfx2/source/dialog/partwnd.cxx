/*************************************************************************
 *
 *  $RCSfile: partwnd.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:31 $
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
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#include <toolkit/helper/vclunohelper.hxx>

#include "partwnd.hxx"
#include "unoframe.hxx"
#include "progind.hxx"
#include "sfxsids.hrc"
#include "app.hxx"
#include "bindings.hxx"

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
        pApp->SetChildWindow( SID_PARTWIN, sal_False );
        pApp->GetBindings().Invalidate( SID_PARTWIN );
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

//****************************************************************************
// SfxPartChildWnd_Impl
//****************************************************************************

SFX_IMPL_DOCKINGWINDOW( SfxPartChildWnd_Impl, SID_PARTWIN );

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
    pWindow = new SfxPartDockWnd_Impl(
        pBindings, this, pParent, WB_STDDOCKWIN | WB_CLIPCHILDREN | WB_SIZEABLE | WB_3DLOOK );
    eChildAlignment = SFX_ALIGN_TOP;
//  pWindow->SetHelpId ( HID_NAVIGATOR_WINDOW );
    ((SfxDockingWindow*)pWindow)->SetFloatingSize( Size( 240, 240 ) );
    pWindow->SetSizePixel( Size( 240, 240 ) );

    ( ( SfxDockingWindow* ) pWindow )->Initialize( pInfo );
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
) :

    SfxDockingWindow( pBindings, pChildWin, pParent, nBits ),
    _pFrame( new SfxPartwinFrame_Impl )

{
    _pFrame->acquire();
    _pFrame->pBeamer = this;
    _pFrame->initialize( VCLUnoHelper::GetInterface ( this ) );
    _pFrame->pIndicator = new SfxPopupStatusIndicator( this );
    pChildWin->SetFrame( _pFrame );
}

//****************************************************************************

SfxPartDockWnd_Impl::~SfxPartDockWnd_Impl()
{
    _pFrame->SetContainerWindow_Impl( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > () );
    DELETEZ( _pFrame->pIndicator );
    _pFrame->pBeamer = NULL;
    _pFrame->dispose();
    _pFrame->release();
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
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >  xWindow( _pFrame->getComponentWindow() );
    if ( xWindow.is() )
    {
        Size aSize( GetOutputSizePixel() );
        Rectangle aRect( impl_Rectangle_Struct2Object(xWindow->getPosSize()) );
        Point aPos( aRect.TopLeft() );
        Size aNewSize(  aSize.Width() - 2 * aPos.X(), aSize.Height() - aPos.Y() );
        if ( _pFrame->pIndicator->IsVisible() )
        {
            long nHeight = _pFrame->pIndicator->CalcWindowSizePixel().Height();
            aNewSize.Height() -= nHeight;
            _pFrame->pIndicator->SetPosSizePixel( 0, aPos.Y() + aNewSize.Height(), aNewSize.Width(), nHeight, WINDOW_POSSIZE_POSSIZE );
        }

        xWindow->setPosSize( aPos.X(), aPos.Y(), aNewSize.Width(), aNewSize.Height(), ::com::sun::star::awt::PosSize::SIZE );
    }

    SfxDockingWindow::Resize();
}

//****************************************************************************

sal_Bool SfxPartDockWnd_Impl::QueryClose()
{
    sal_Bool bClose = sal_True;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >  xCtrl = _pFrame->getController();

    if ( xCtrl.is() )
        bClose = xCtrl->suspend( sal_True );
    return bClose;;
}

//****************************************************************************

long SfxPartDockWnd_Impl::Notify( NotifyEvent& rEvt )
{
    if ( rEvt.GetType() == EVENT_GETFOCUS )
        _pFrame->activate();

    return SfxDockingWindow::Notify( rEvt );
}

void SfxPartDockWnd_Impl::FillInfo( SfxChildWinInfo& rInfo ) const
{
    SfxDockingWindow::FillInfo( rInfo );
    rInfo.bVisible = sal_False;
}


