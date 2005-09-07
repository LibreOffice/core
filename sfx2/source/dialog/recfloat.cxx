/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: recfloat.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:29:56 $
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

// includes *******************************************************************

#pragma hdrstop

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHRECORDERSUPPLIER_HPP_
#include <com/sun/star/frame/XDispatchRecorderSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODULEMANAGER_HPP_
#include <com/sun/star/frame/XModuleManager.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#include <svtools/eitem.hxx>
#include <svtools/generictoolboxcontroller.hxx>
#include <vcl/msgbox.hxx>
#include <comphelper/processfactory.hxx>

#include "recfloat.hxx"
#include "dialog.hrc"
#include "sfxresid.hxx"
#include "app.hxx"
#include "bindings.hxx"
#include "dispatch.hxx"
#include "viewfrm.hxx"
#include "viewsh.hxx"
#include "imagemgr.hxx"

using namespace ::com::sun::star;

static rtl::OUString GetLabelFromCommandURL( const rtl::OUString& rCommandURL, const uno::Reference< frame::XFrame >& xFrame )
{
    rtl::OUString aLabel;
    rtl::OUString aModuleIdentifier;
    uno::Reference< container::XNameAccess > xUICommandLabels;
    uno::Reference< lang::XMultiServiceFactory > xServiceManager;
    uno::Reference< container::XNameAccess > xUICommandDescription;
    uno::Reference< ::com::sun::star::frame::XModuleManager > xModuleManager;

    static uno::WeakReference< lang::XMultiServiceFactory > xTmpServiceManager;
    static uno::WeakReference< container::XNameAccess >     xTmpNameAccess;
    static uno::WeakReference< ::com::sun::star::frame::XModuleManager > xTmpModuleMgr;

    xServiceManager = xTmpServiceManager;
    if ( !xServiceManager.is() )
    {
        xServiceManager = ::comphelper::getProcessServiceFactory();
        xTmpServiceManager = xServiceManager;
    }

    xUICommandDescription = xTmpNameAccess;
    if ( !xUICommandDescription.is() )
    {
        xUICommandDescription = uno::Reference< container::XNameAccess >(
                                    xServiceManager->createInstance(
                                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                            "com.sun.star.frame.UICommandDescription" ))),
                                    uno::UNO_QUERY );
        xTmpNameAccess = xUICommandDescription;
    }

    xModuleManager = xTmpModuleMgr;
    if ( !xModuleManager.is() )
    {
        xModuleManager = uno::Reference< ::com::sun::star::frame::XModuleManager >(
            xServiceManager->createInstance(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.frame.ModuleManager" ))),
            uno::UNO_QUERY_THROW );
        xTmpModuleMgr = xModuleManager;
    }

    // Retrieve label from UI command description service
    try
    {
        try
        {
            aModuleIdentifier = xModuleManager->identify( xFrame );
        }
        catch( uno::Exception& )
        {
        }

        if ( xUICommandDescription.is() )
        {
            uno::Any a = xUICommandDescription->getByName( aModuleIdentifier );
            uno::Reference< container::XNameAccess > xUICommands;
            a >>= xUICommandLabels;
        }
    }
    catch ( uno::Exception& )
    {
    }

    if ( xUICommandLabels.is() )
    {
        try
        {
            if ( rCommandURL.getLength() > 0 )
            {
                uno::Sequence< beans::PropertyValue > aPropSeq;
                uno::Any a( xUICommandLabels->getByName( rCommandURL ));
                if ( a >>= aPropSeq )
                {
                    for ( sal_Int32 i = 0; i < aPropSeq.getLength(); i++ )
                    {
                        if ( aPropSeq[i].Name.equalsAscii( "Label" ))
                        {
                            aPropSeq[i].Value >>= aLabel;
                            break;
                        }
                    }
                }
            }
        }
        catch (uno::Exception& )
        {
        }
    }

    return aLabel;
}

SFX_IMPL_FLOATINGWINDOW( SfxRecordingFloatWrapper_Impl, SID_RECORDING_FLOATWINDOW );

SfxRecordingFloatWrapper_Impl::SfxRecordingFloatWrapper_Impl( Window* pParent ,
                                                USHORT nId ,
                                                SfxBindings* pBind ,
                                                SfxChildWinInfo* pInfo )
                    : SfxChildWindow( pParent , nId )
                    , pBindings( pBind )
{
    pWindow = new SfxRecordingFloat_Impl( pBindings, this, pParent );
    SetWantsFocus( FALSE );
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
    ( ( SfxFloatingWindow* ) pWindow )->Initialize( pInfo );
}

SfxRecordingFloatWrapper_Impl::~SfxRecordingFloatWrapper_Impl()
{
    SfxBoolItem aItem( FN_PARAM_1, TRUE );
    com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > xRecorder = pBindings->GetRecorder();
    if ( xRecorder.is() )
        pBindings->GetDispatcher()->Execute( SID_STOP_RECORDING, SFX_CALLMODE_SYNCHRON, &aItem, 0L );
}

sal_Bool SfxRecordingFloatWrapper_Impl::QueryClose()
{
    // asking for recorded macro should be replaced if index access is available!
    BOOL bRet = TRUE;
    com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > xRecorder = pBindings->GetRecorder();
    if ( xRecorder.is() && xRecorder->getRecordedMacro().getLength() )
    {
        QueryBox aBox( GetWindow(), WB_YES_NO | WB_DEF_NO , String( SfxResId( STR_MACRO_LOSS ) ) );
        aBox.SetText( String( SfxResId(STR_CANCEL_RECORDING) ) );
        bRet = ( aBox.Execute() == RET_YES );
    }

    return bRet;
}

SfxRecordingFloat_Impl::SfxRecordingFloat_Impl(
    SfxBindings* pBindings ,
    SfxChildWindow* pChildWin ,
    Window* pParent )
    : SfxFloatingWindow( pBindings ,
                         pChildWin ,
                         pParent ,
                         SfxResId( SID_RECORDING_FLOATWINDOW ) )
    , pWrapper( pChildWin )
    , aTbx( this, SfxResId(SID_RECORDING_FLOATWINDOW) )
{
    // Retrieve label from helper function
    uno::Reference< frame::XFrame > xFrame = GetBindings().GetActiveFrame();
    rtl::OUString aCommandStr( RTL_CONSTASCII_USTRINGPARAM( ".uno:StopRecording" ));
    aTbx.SetItemText( SID_STOP_RECORDING, GetLabelFromCommandURL( aCommandStr, xFrame ));

    // Determine size of toolbar
    Size aSize = aTbx.CalcWindowSizePixel();
    aTbx.SetPosSizePixel( Point(), aSize );
    SetOutputSizePixel( aSize );

    // create a generic toolbox controller for our internal toolbox
    svt::GenericToolboxController* pController = new svt::GenericToolboxController(
                                                    ::comphelper::getProcessServiceFactory(),
                                                    xFrame,
                                                    &aTbx,
                                                    SID_STOP_RECORDING,
                                                    aCommandStr );
    xStopRecTbxCtrl = uno::Reference< frame::XToolbarController >(
                            static_cast< cppu::OWeakObject* >( pController ),
                        uno::UNO_QUERY );
    uno::Reference< util::XUpdatable > xUpdate( xStopRecTbxCtrl, uno::UNO_QUERY );
    if ( xUpdate.is() )
        xUpdate->update();

    aTbx.SetSelectHdl( LINK( this, SfxRecordingFloat_Impl, Select ) );

    // start recording
    SfxBoolItem aItem( SID_RECORDMACRO, TRUE );
    GetBindings().GetDispatcher()->Execute( SID_RECORDMACRO, SFX_CALLMODE_SYNCHRON, &aItem, 0L );
}

SfxRecordingFloat_Impl::~SfxRecordingFloat_Impl()
{
    try
    {
        if ( xStopRecTbxCtrl.is() )
        {
            uno::Reference< lang::XComponent > xComp( xStopRecTbxCtrl, uno::UNO_QUERY );
            xComp->dispose();
        }
    }
    catch ( uno::Exception& )
    {
    }
}

BOOL SfxRecordingFloat_Impl::Close()
{
    BOOL bRet = SfxFloatingWindow::Close();
    return bRet;
}

void SfxRecordingFloat_Impl::FillInfo( SfxChildWinInfo& rInfo ) const
{
    SfxFloatingWindow::FillInfo( rInfo );
    rInfo.bVisible = sal_False;
}

void SfxRecordingFloat_Impl::StateChanged( StateChangedType nStateChange )
{
    if ( nStateChange == STATE_CHANGE_INITSHOW )
    {
        SfxViewFrame *pFrame = GetBindings().GetDispatcher_Impl()->GetFrame();
        Window* pEditWin = pFrame->GetViewShell()->GetWindow();

        Point aPoint = pEditWin->OutputToScreenPixel( pEditWin->GetPosPixel() );
        aPoint = GetParent()->ScreenToOutputPixel( aPoint );
        aPoint.X() += 20;
        aPoint.Y() += 10;
        SetPosPixel( aPoint );
    }

    SfxFloatingWindow::StateChanged( nStateChange );
}

IMPL_LINK( SfxRecordingFloat_Impl, Select, ToolBox*, pToolBar )
{
    sal_Int16   nKeyModifier( (sal_Int16)aTbx.GetModifier() );
    if ( xStopRecTbxCtrl.is() )
        xStopRecTbxCtrl->execute( nKeyModifier );

    return 1;
}
