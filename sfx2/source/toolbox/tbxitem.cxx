/*************************************************************************
 *
 *  $RCSfile: tbxitem.cxx,v $
 *
 *  $Revision: 1.35 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 13:38:52 $
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

#ifdef SOLARIS
// HACK: prevent conflict between STLPORT and Workshop headers on Solaris 8
#include <ctime>
#endif

#include <string>           // prevent conflict with STL includes

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_MACROEXECMODE_HPP_
#include <com/sun/star/document/MacroExecMode.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_UPDATEDOCMODE_HPP_
#include <com/sun/star/document/UpdateDocMode.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_FRAME_XLAYOUTMANAGER_HPP_
#include <drafts/com/sun/star/frame/XLayoutManager.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_STATUS_ITEMSTATUS_HPP_
#include <com/sun/star/frame/status/ItemStatus.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_STATUS_ITEMSTATE_HPP_
#include <com/sun/star/frame/status/ItemState.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_XUIELEMENTFACTORY_HPP_
#include <drafts/com/sun/star/ui/XUIElementFactory.hpp>
#endif

#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef _SVT_IMAGEITM_HXX
#include <svtools/imageitm.hxx>
#endif
#ifndef _URLBMK_HXX //autogen
#include <svtools/urlbmk.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif

#include <svtools/imagemgr.hxx>
#include <comphelper/processfactory.hxx>
#include <framework/addonmenu.hxx>
#include <framework/addonsoptions.hxx>
#include <framework/menuconfiguration.hxx>
#include <framework/sfxhelperfunctions.hxx>
#include <vcl/taskpanelist.hxx>
#ifndef _TOOLKIT_UNOHLP_HXX
#include <toolkit/helper/vclunohelper.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_MENUOPTIONS_HXX
#include <svtools/menuoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MISCOPT_HXX
#include <svtools/miscopt.hxx>
#endif

#pragma hdrstop

#include "tbxctrl.hxx"
#include "mnumgr.hxx"
#include "stbmgr.hxx"
#include "dispatch.hxx"
#include "fltfnc.hxx"
#include "msg.hxx"
#include "msgpool.hxx"
#include "statcach.hxx"
#include "viewfrm.hxx"
#include "arrdecl.hxx"
#include "sfxtypes.hxx"
#include "macrconf.hxx"
#include "genlink.hxx"
#include "sfxresid.hxx"
#include "toolbox.hrc"
#include "sfx.hrc"
#include "module.hxx"
#include "appdata.hxx"
#include "docfile.hxx"
#include "docfac.hxx"
#include "referers.hxx"
#include "frmhtmlw.hxx"
#include "app.hxx"
#include "unoctitm.hxx"
#include "helpid.hrc"
#include "imagemgr.hxx"
#include "workwin.hxx"
#include "imgmgr.hxx"
#include "virtmenu.hxx"
#include "viewfrm.hxx"
#include "module.hxx"

//using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::frame::status;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::drafts::com::sun::star::frame;
using namespace ::drafts::com::sun::star::ui;

//--------------------------------------------------------------------

static USHORT nLastId_Impl = SID_NEWDOCDIRECT;

//====================================================================

SFX_IMPL_TOOLBOX_CONTROL_ARG(SfxToolBoxControl, SfxStringItem, TRUE);
SFX_IMPL_TOOLBOX_CONTROL(SfxAppToolBoxControl_Impl, SfxStringItem);
//SFX_IMPL_TOOLBOX_CONTROL(SfxDragToolBoxControl_Impl, SfxStringItem);
//SFX_IMPL_TOOLBOX_CONTROL(SfxHistoryToolBoxControl_Impl, SfxStringItem);
SFX_IMPL_TOOLBOX_CONTROL(SfxReloadToolBoxControl_Impl, SfxBoolItem);
SFX_IMPL_TOOLBOX_CONTROL(SfxAddonsToolBoxControl_Impl, SfxVoidItem);
SFX_IMPL_TOOLBOX_CONTROL(SfxRotatedToolboxControl, SfxStringItem );

svt::ToolboxController* SAL_CALL SfxToolBoxControllerFactory( const Reference< XFrame >& rFrame, ToolBox* pToolbox, unsigned short nID, const ::rtl::OUString& aCommandURL )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    URL aTargetURL;
    aTargetURL.Complete = aCommandURL;
    Reference < XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance( rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer" )), UNO_QUERY );
    xTrans->parseStrict( aTargetURL );

    SfxViewFrame*   pViewFrame = NULL;
    SfxObjectShell* pObjShell = NULL;
    Reference < XController > xController;
    Reference < XModel > xModel;
    if ( rFrame.is() )
    {
        xController = rFrame->getController();
        if ( xController.is() )
            xModel = xController->getModel();
    }

    if ( xModel.is() )
    {
        // Get tunnel from model to retrieve the SfxObjectShell pointer from it
        ::com::sun::star::uno::Reference < ::com::sun::star::lang::XUnoTunnel > xObj( xModel, UNO_QUERY );
        ::com::sun::star::uno::Sequence < sal_Int8 > aSeq( (sal_Int8*) SvGlobalName( SFX_GLOBAL_CLASSID ).GetBytes(), 16 );
        sal_Int64 nHandle = xObj->getSomething( aSeq );
        if ( nHandle )
            pObjShell = (SfxObjectShell*) (sal_Int32*) nHandle;
    }

    SfxModule*     pModule   = pObjShell ? pObjShell->GetModule() : NULL;
    SfxSlotPool*   pSlotPool = 0;

    if ( pModule )
        pSlotPool = pModule->GetSlotPool();
    else
        pSlotPool = &(SFX_APP()->GetSlotPool( NULL ));

    const SfxSlot* pSlot = pSlotPool->GetUnoSlot( aTargetURL.Path );
    if ( pSlot )
    {
        USHORT nSlotId = pSlot->GetSlotId();
        if ( nSlotId > 0 )
            return SfxToolBoxControl::CreateControl( nSlotId, nID, pToolbox, pModule );
    }

    return NULL;
}

struct SfxToolBoxControl_Impl
{
    ToolBox*                pBox;
    BOOL                    bShowString;
    USHORT                  nSelectModifier;
    SfxTbxCtrlFactory*      pFact;
    USHORT                  nTbxId;
    USHORT                  nSlotId;
    SfxPopupWindow*         mpFloatingWindow;
    SfxPopupWindow*         mpPopupWindow;
    Reference< XUIElement > mxUIElement;
};


//--------------------------------------------------------------------

SfxToolBoxControl::SfxToolBoxControl(
    USHORT          nSlotID,
    USHORT          nID,
    ToolBox&        rBox,
    BOOL            bShowStringItems     )
:   svt::ToolboxController()
{
    pImpl = new SfxToolBoxControl_Impl;

    pImpl->pBox = &rBox;
    pImpl->bShowString = bShowStringItems;
    pImpl->nSelectModifier = 0;
    pImpl->pFact = 0;
    pImpl->nTbxId = nID;
    pImpl->nSlotId = nSlotID;
    pImpl->mpFloatingWindow = 0;
    pImpl->mpPopupWindow = 0;

    //DBG( CheckConfigure_Impl(SFX_SLOT_TOOLBOXCONFIG) );
}

//--------------------------------------------------------------------

SfxToolBoxControl::~SfxToolBoxControl()
{
    if ( pImpl->mxUIElement.is() )
    {
        Reference< XComponent > xComponent( pImpl->mxUIElement, UNO_QUERY );
        xComponent->dispose();
    }
    pImpl->mxUIElement = 0;
    delete pImpl;
}

//--------------------------------------------------------------------

ToolBox& SfxToolBoxControl::GetToolBox() const
{
    return *pImpl->pBox;
}
unsigned short SfxToolBoxControl::GetId() const
{
    return pImpl->nTbxId;
}
unsigned short SfxToolBoxControl::GetSlotId() const
{
    return pImpl->nSlotId;
}

//--------------------------------------------------------------------

void SAL_CALL SfxToolBoxControl::dispose() throw (::com::sun::star::uno::RuntimeException)
{
    if ( m_bDisposed )
        return;

    svt::ToolboxController::dispose();

    // Remove and destroy our item window at our toolbox
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    Window* pWindow = pImpl->pBox->GetItemWindow( pImpl->nTbxId );
    pImpl->pBox->SetItemWindow( pImpl->nTbxId, 0 );
    delete pWindow;

    // Delete my popup windows
    delete pImpl->mpFloatingWindow;
    delete pImpl->mpPopupWindow;

    pImpl->mpFloatingWindow = 0;
    pImpl->mpPopupWindow = 0;
}

//--------------------------------------------------------------------

SfxToolBoxControl* SfxToolBoxControl::CreateControl( USHORT nSlotId, USHORT nTbxId, ToolBox *pBox, SfxModule* pMod  )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    SfxToolBoxControl *pCtrl;
    SfxApplication *pApp = SFX_APP();

    SfxSlotPool *pSlotPool;
    if ( pMod )
        pSlotPool = pMod->GetSlotPool();
    else
        pSlotPool = &pApp->GetSlotPool();
    TypeId aSlotType = pSlotPool->GetSlotType( nSlotId );
    if ( aSlotType )
    {
        if ( pMod )
        {
            SfxTbxCtrlFactArr_Impl *pFactories = pMod->GetTbxCtrlFactories_Impl();
            if ( pFactories )
            {
                SfxTbxCtrlFactArr_Impl &rFactories = *pFactories;
                for ( USHORT nFactory = 0; nFactory < rFactories.Count(); ++nFactory )
                    if ( rFactories[nFactory]->nTypeId == aSlotType &&
                         ( ( rFactories[nFactory]->nSlotId == 0 ) ||
                           ( rFactories[nFactory]->nSlotId == nSlotId) ) )
                    {
                        pCtrl = rFactories[nFactory]->pCtor( nSlotId, nTbxId, *pBox );
                        pCtrl->pImpl->pFact = rFactories[nFactory];
                        return pCtrl;
                    }
            }
        }

        SfxTbxCtrlFactArr_Impl &rFactories = pApp->GetTbxCtrlFactories_Impl();

        for ( USHORT nFactory = 0; nFactory < rFactories.Count(); ++nFactory )
            if ( rFactories[nFactory]->nTypeId == aSlotType &&
                 ( ( rFactories[nFactory]->nSlotId == 0 ) ||
                   ( rFactories[nFactory]->nSlotId == nSlotId) ) )
            {
                pCtrl = rFactories[nFactory]->pCtor( nSlotId, nTbxId, *pBox );
                pCtrl->pImpl->pFact = rFactories[nFactory];
                return pCtrl;
            }
    }

    return NULL;
}

SfxItemState SfxToolBoxControl::GetItemState(
    const SfxPoolItem* pState )
/*  [Beschreibung]

    Statische Methode zum Ermitteln des Status des SfxPoolItem-Pointers,
    in der Methode <SfxControllerItem::StateChanged(const SfxPoolItem*)>
    zu verwenden.

    [R"uckgabewert]

    SfxItemState        SFX_ITEM_UNKNOWN
                        Enabled, aber keine weitere Statusinformation
                        verf"ugbar. Typisch f"ur <Slot>s, die allenfalls
                        zeitweise disabled sind, aber ihre Darstellung sonst
                        nicht "andern.

                        SFX_ITEM_DISABLED
                        Disabled und keine weiter Statusinformation
                        verf"ugbar. Alle anderen ggf. angezeigten Werte sollten
                        auf den Default zur"uckgesetzt werden.

                        SFX_ITEM_DONTCARE
                        Enabled aber es waren nur uneindeutige Werte
                        verf"ugbar (also keine, die abgefragt werden k"onnen).

                        SFX_ITEM_AVAILABLE
                        Enabled und mit verf"ugbarem Wert, der von 'pState'
                        erfragbar ist. Der Typ ist dabei im gesamten
                        Programm eindeutig und durch den Slot festgelegt.
*/

{
    return !pState
                ? SFX_ITEM_DISABLED
                : IsInvalidItem(pState)
                    ? SFX_ITEM_DONTCARE
                    : pState->ISA(SfxVoidItem) && !pState->Which()
                        ? SFX_ITEM_UNKNOWN
                        : SFX_ITEM_AVAILABLE;
}

void SfxToolBoxControl::Dispatch(
    const Reference< XDispatchProvider >& rProvider,
    const rtl::OUString& rCommand,
    Sequence< ::PropertyValue >& aArgs )
{
    if ( rProvider.is() )
    {
        ::com::sun::star::util::URL aTargetURL;
        Reference< XURLTransformer > xURLTransformer( ::comphelper::getProcessServiceFactory()->createInstance(
                                                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                                                    UNO_QUERY );
        aTargetURL.Complete = rCommand;
        xURLTransformer->parseStrict( aTargetURL );

        Reference < XDispatch > xDispatch = rProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );
        if ( xDispatch.is() )
            xDispatch->dispatch( aTargetURL, aArgs );
    }
}

void SfxToolBoxControl::Dispatch( const ::rtl::OUString& aCommand, ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs )
{
    Reference < XController > xController;

    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( m_xFrame.is() )
        xController = m_xFrame->getController();

    Reference < XDispatchProvider > xProvider( xController, UNO_QUERY );
    if ( xProvider.is() )
    {
        ::com::sun::star::util::URL aTargetURL;
        Reference< XURLTransformer > xURLTransformer( ::comphelper::getProcessServiceFactory()->createInstance(
                                                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                                                    UNO_QUERY );
        aTargetURL.Complete = aCommand;
        xURLTransformer->parseStrict( aTargetURL );

        Reference < XDispatch > xDispatch = xProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );
        if ( xDispatch.is() )
            xDispatch->dispatch( aTargetURL, aArgs );
    }
}

// XInterface
Any SAL_CALL SfxToolBoxControl::queryInterface( const Type & rType )
throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                           SAL_STATIC_CAST( ::com::sun::star::awt::XDockableWindowListener*, this ) );
    return (aRet.hasValue() ? aRet : svt::ToolboxController::queryInterface( rType ));
}

void SAL_CALL SfxToolBoxControl::acquire() throw()
{
    OWeakObject::acquire();
}

void SAL_CALL SfxToolBoxControl::release() throw()
{
    OWeakObject::release();
}

void SAL_CALL SfxToolBoxControl::disposing( const ::com::sun::star::lang::EventObject& aEvent )
throw( ::com::sun::star::uno::RuntimeException )
{
    svt::ToolboxController::disposing( aEvent );
}

// XStatusListener
void SAL_CALL SfxToolBoxControl::statusChanged( const FeatureStateEvent& rEvent )
throw ( ::com::sun::star::uno::RuntimeException )
{
    SfxViewFrame* pViewFrame = NULL;
    Reference < XController > xController;

    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( m_xFrame.is() )
        xController = m_xFrame->getController();

    Reference < XDispatchProvider > xProvider( xController, UNO_QUERY );
    if ( xProvider.is() )
    {
        Reference < XDispatch > xDisp = xProvider->queryDispatch( rEvent.FeatureURL, ::rtl::OUString(), 0 );
        if ( xDisp.is() )
        {
            Reference< XUnoTunnel > xTunnel( xDisp, UNO_QUERY );
            SfxOfficeDispatch* pDisp = NULL;
            if ( xTunnel.is() )
            {
                sal_Int64 nImplementation = xTunnel->getSomething(SfxOfficeDispatch::impl_getStaticIdentifier());
                pDisp = (SfxOfficeDispatch*)(nImplementation);
            }

            if ( pDisp )
                pViewFrame = pDisp->GetDispatcher_Impl()->GetFrame();
        }
    }

    USHORT nSlotId = 0;
    SfxSlotPool& rPool = SFX_APP()->GetSlotPool( pViewFrame );
    const SfxSlot* pSlot = rPool.GetUnoSlot( rEvent.FeatureURL.Path );
    if ( pSlot )
        nSlotId = pSlot->GetSlotId();

    if ( nSlotId > 0 )
    {
        if ( rEvent.Requery )
            svt::ToolboxController::statusChanged( rEvent );
        else
        {
            SfxItemState eState = SFX_ITEM_DISABLED;
            SfxPoolItem* pItem = NULL;
            if ( rEvent.IsEnabled )
            {
                eState = SFX_ITEM_AVAILABLE;
                ::com::sun::star::uno::Type pType = rEvent.State.getValueType();

                if ( pType == ::getVoidCppuType() )
                {
                    pItem = new SfxVoidItem( nSlotId );
                    eState = SFX_ITEM_UNKNOWN;
                }
                else if ( pType == ::getBooleanCppuType() )
                {
                    sal_Bool bTemp ;
                    rEvent.State >>= bTemp ;
                    pItem = new SfxBoolItem( nSlotId, bTemp );
                }
                else if ( pType == ::getCppuType((const sal_uInt16*)0) )
                {
                    sal_uInt16 nTemp ;
                    rEvent.State >>= nTemp ;
                    pItem = new SfxUInt16Item( nSlotId, nTemp );
                }
                else if ( pType == ::getCppuType((const sal_uInt32*)0) )
                {
                    sal_uInt32 nTemp ;
                    rEvent.State >>= nTemp ;
                    pItem = new SfxUInt32Item( nSlotId, nTemp );
                }
                else if ( pType == ::getCppuType((const ::rtl::OUString*)0) )
                {
                    ::rtl::OUString sTemp ;
                    rEvent.State >>= sTemp ;
                    pItem = new SfxStringItem( nSlotId, sTemp );
                }
                else if ( pType == ::getCppuType((const ::com::sun::star::frame::status::ItemStatus*)0) )
                {
                    ItemStatus aItemStatus;
                    rEvent.State >>= aItemStatus;
                    eState = aItemStatus.State;
                    pItem = new SfxVoidItem( nSlotId );
                }
                else
                {
                    if ( pSlot )
                        pItem = pSlot->GetType()->CreateItem();
                    if ( pItem )
                    {
                        pItem->SetWhich( nSlotId );
                        pItem->PutValue( rEvent.State );
                    }
                    else
                        pItem = new SfxVoidItem( nSlotId );
                }
            }

            StateChanged( nSlotId, eState, pItem );
            delete pItem;
        }
    }
}

// XToolbarController
void SAL_CALL SfxToolBoxControl::execute( sal_Int16 KeyModifier ) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    Select( (USHORT)KeyModifier );
}
void SAL_CALL SfxToolBoxControl::click() throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    Click();
}

void SAL_CALL SfxToolBoxControl::doubleClick() throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    DoubleClick();
}

Reference< ::com::sun::star::awt::XWindow > SAL_CALL SfxToolBoxControl::createPopupWindow() throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    Window* pWindow = CreatePopupWindow();
    if ( pWindow )
        return VCLUnoHelper::GetInterface( pWindow );
    else
        return Reference< ::com::sun::star::awt::XWindow >();
}

Reference< ::com::sun::star::awt::XWindow > SAL_CALL SfxToolBoxControl::createItemWindow( const Reference< ::com::sun::star::awt::XWindow >& rParent ) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    return VCLUnoHelper::GetInterface( CreateItemWindow( VCLUnoHelper::GetWindow( rParent )));
}

// XDockableWindowListener
void SAL_CALL SfxToolBoxControl::startDocking( const ::com::sun::star::awt::DockingEvent& e )
throw (::com::sun::star::uno::RuntimeException)
{
}
::com::sun::star::awt::DockingData SAL_CALL SfxToolBoxControl::docking( const ::com::sun::star::awt::DockingEvent& e )
throw (::com::sun::star::uno::RuntimeException)
{
    return ::com::sun::star::awt::DockingData();
}

void SAL_CALL SfxToolBoxControl::endDocking( const ::com::sun::star::awt::EndDockingEvent& e )
throw (::com::sun::star::uno::RuntimeException)
{
}

sal_Bool SAL_CALL SfxToolBoxControl::prepareToggleFloatingMode( const ::com::sun::star::lang::EventObject& e )
throw (::com::sun::star::uno::RuntimeException)
{
    return sal_False;
}

void SAL_CALL SfxToolBoxControl::toggleFloatingMode( const ::com::sun::star::lang::EventObject& e )
throw (::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL SfxToolBoxControl::closed( const ::com::sun::star::lang::EventObject& e )
throw (::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL SfxToolBoxControl::endPopupMode( const ::com::sun::star::awt::EndPopupModeEvent& e )
throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    ::rtl::OUString aSubToolBarResName;
    if ( pImpl->mxUIElement.is() )
    {
        Reference< XPropertySet > xPropSet( pImpl->mxUIElement, UNO_QUERY );
        if ( xPropSet.is() )
        {
            try
            {
                xPropSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ResourceURL" ))) >>= aSubToolBarResName;
            }
            catch ( com::sun::star::beans::UnknownPropertyException& )
            {
            }
            catch ( com::sun::star::lang::WrappedTargetException& )
            {
            }
        }

        Reference< XComponent > xComponent( pImpl->mxUIElement, UNO_QUERY );
        xComponent->dispose();
    }
    pImpl->mxUIElement = 0;

    // if the toolbar was teared-off recreate it and place it at the given position
    if( e.bTearoff )
    {
        Reference< XUIElement >     xUIElement;
        Reference< XLayoutManager > xLayoutManager = getLayoutManager();

        if ( !xLayoutManager.is() )
            return;

        xLayoutManager->createElement( aSubToolBarResName );
        xUIElement = xLayoutManager->getElement( aSubToolBarResName );
        if ( xUIElement.is() )
        {
            Reference< ::com::sun::star::awt::XWindow > xParent = m_xFrame->getContainerWindow();

            Window* pParent = VCLUnoHelper::GetWindow( xParent );
            xLayoutManager->hideElement( aSubToolBarResName );
            xLayoutManager->floatWindow( aSubToolBarResName );

            Reference< ::com::sun::star::awt::XWindow > xSubToolBar( xUIElement->getRealInterface(), UNO_QUERY );
            if ( xSubToolBar.is() )
            {
                Window*  pTbxWindow = VCLUnoHelper::GetWindow( xSubToolBar );
                ToolBox* pToolBar( 0 );
                if ( pTbxWindow && pTbxWindow->GetType() == WINDOW_TOOLBOX )
                    pToolBar = (ToolBox *)pTbxWindow;

                ::Size aSize = pToolBar->CalcPopupWindowSizePixel();    // use the same size as for popup mode
                pToolBar->SetSizePixel( aSize );

                xLayoutManager->setElementPos( aSubToolBarResName, e.FloatingPosition );
                xLayoutManager->showElement( aSubToolBarResName );
            }
        }
    }
}

void SfxToolBoxControl::createAndPositionSubToolBar( const ::rtl::OUString& rSubToolBarResName )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    if ( pImpl->pBox )
    {
        USHORT nItemId = pImpl->pBox->GetDownItemId();

        if ( !nItemId )
            return;

        // create element with factory
        Reference< XMultiServiceFactory >   xServiceManager = getServiceManager();
        Reference< XFrame >                 xFrame          = getFrameInterface();
        Reference< XUIElement >             xUIElement;
        Reference< XUIElementFactory >      xUIEementFactory;

        xUIEementFactory = Reference< XUIElementFactory >(
                xServiceManager->createInstance(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                        "drafts.com.sun.star.ui.UIElementFactoryManager" ))), UNO_QUERY );

        Sequence< PropertyValue > aPropSeq( 2 );
        aPropSeq[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
        aPropSeq[0].Value <<= xFrame;
        aPropSeq[1].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Persistent" ));
        aPropSeq[1].Value <<= sal_False;

        try
        {
            xUIElement = xUIEementFactory->createUIElement( rSubToolBarResName, aPropSeq );
        }
        catch ( ::com::sun::star::container::NoSuchElementException& )
        {
        }
        catch ( IllegalArgumentException& )
        {
        }

        if ( xUIElement.is() )
        {
            Reference< ::com::sun::star::awt::XWindow > xParent = m_xFrame->getContainerWindow();

            Window* pParent = VCLUnoHelper::GetWindow( xParent );

            Reference< ::com::sun::star::awt::XWindow > xSubToolBar( xUIElement->getRealInterface(), UNO_QUERY );
            if ( xSubToolBar.is() )
            {
                Reference< ::com::sun::star::awt::XDockableWindow > xDockWindow( xSubToolBar, UNO_QUERY );
                xDockWindow->addDockableWindowListener( Reference< ::com::sun::star::awt::XDockableWindowListener >(
                    static_cast< OWeakObject * >( this ), UNO_QUERY ));
                xDockWindow->enableDocking( sal_True );

                // keep refererence to UIElement to avoid its destruction
                if ( pImpl->mxUIElement.is() )
                {
                    Reference< XComponent > xComponent( pImpl->mxUIElement, UNO_QUERY );
                    xComponent->dispose();
                }
                pImpl->mxUIElement = xUIElement;

                Window*  pTbxWindow = VCLUnoHelper::GetWindow( xSubToolBar );
                ToolBox* pToolBar( 0 );
                if ( pTbxWindow && pTbxWindow->GetType() == WINDOW_TOOLBOX )
                    pToolBar = (ToolBox *)pTbxWindow;

                // calc and set size for popup mode
                ::Size aSize = pToolBar->CalcPopupWindowSizePixel();
                pToolBar->SetSizePixel( aSize );

                ::Point aPos = pImpl->pBox->GetItemPopupPosition( nItemId, aSize );
                aPos = pParent->OutputToScreenPixel( pParent->AbsoluteScreenToOutputPixel( pImpl->pBox->OutputToAbsoluteScreenPixel( aPos ) ) );

                ::com::sun::star::awt::Point aPoint;
                aPoint.X = aPos.X();
                aPoint.Y = aPos.Y();

                ::Rectangle aRect( aPos, aSize );

                Window::GetDockingManager()->StartPopupMode( pToolBar, aRect );
            }
        }
    }
}

//--------------------------------------------------------------------

void SfxToolBoxControl::SetPopupWindow( SfxPopupWindow* pWindow )
{
    pImpl->mpPopupWindow = pWindow;
    pImpl->mpPopupWindow->SetPopupModeEndHdl( LINK( this, SfxToolBoxControl, PopupModeEndHdl ));
    pImpl->mpPopupWindow->SetDeleteLink_Impl( LINK( this, SfxToolBoxControl, ClosePopupWindow ));
}

//--------------------------------------------------------------------

IMPL_LINK( SfxToolBoxControl, PopupModeEndHdl, void *, EMPTYARG )
{
    if ( pImpl->mpPopupWindow->IsVisible() )
    {
        // Replace floating window with popup window and destroy
        // floating window instance.
        delete pImpl->mpFloatingWindow;
        pImpl->mpFloatingWindow = pImpl->mpPopupWindow;
        pImpl->mpPopupWindow    = 0;
    }
    else
    {
        // Popup window has been closed by the user. No replacement, instance
        // will destroy itself.
        pImpl->mpPopupWindow = 0;
    }

    return 1;
}

//--------------------------------------------------------------------
IMPL_LINK( SfxToolBoxControl, ClosePopupWindow, SfxPopupWindow *, pWindow )
{
    if ( pWindow == pImpl->mpFloatingWindow )
        pImpl->mpFloatingWindow = 0;
    else
        pImpl->mpPopupWindow = 0;

    return 1;
}

//--------------------------------------------------------------------

void SfxToolBoxControl::StateChanged
(
    USHORT              nId,
    SfxItemState        eState,
    const SfxPoolItem*  pState
)
{
    DBG_MEMTEST();
    DBG_ASSERT( pImpl->pBox != 0, "setting state to dangling ToolBox" );

    if ( GetId() >= SID_OBJECTMENU0 && GetId() <= SID_OBJECTMENU_LAST )
        return;

    // enabled/disabled-Flag pauschal korrigieren
    pImpl->pBox->EnableItem( GetId(), eState != SFX_ITEM_DISABLED );

    USHORT nItemBits = pImpl->pBox->GetItemBits( GetId() );
    nItemBits &= ~TIB_CHECKABLE;
    TriState eTri = STATE_NOCHECK;
    switch ( eState )
    {
        case SFX_ITEM_AVAILABLE:
        {
            if ( pState->ISA(SfxBoolItem) )
            {
                // BoolItem fuer checken
                if ( ((const SfxBoolItem*)pState)->GetValue() )
                    eTri = STATE_CHECK;
                nItemBits |= TIB_CHECKABLE;
            }
            else if ( pState->ISA(SfxEnumItemInterface) &&
                ((SfxEnumItemInterface *)pState)->HasBoolValue())
            {
                // EnumItem wie Bool behandeln
                if ( ((const SfxEnumItemInterface *)pState)->GetBoolValue() )
                    eTri = STATE_CHECK;
                nItemBits |= TIB_CHECKABLE;
            }
            else if ( pImpl->bShowString && pState->ISA(SfxStringItem) )
                pImpl->pBox->SetItemText(nId, ((const SfxStringItem*)pState)->GetValue() );
            break;
        }

        case SFX_ITEM_DONTCARE:
        {
            eTri = STATE_DONTKNOW;
            nItemBits |= TIB_CHECKABLE;
        }
    }

    pImpl->pBox->SetItemState( GetId(), eTri );
    pImpl->pBox->SetItemBits( GetId(), nItemBits );
}

//--------------------------------------------------------------------

void SfxToolBoxControl::Select( USHORT nModifier )
{
    pImpl->nSelectModifier = nModifier;
    Select( BOOL((nModifier & KEY_MOD1)!=0) );
}

//--------------------------------------------------------------------

void SfxToolBoxControl::Select( BOOL bMod1 )
{
    svt::ToolboxController::execute( pImpl->nSelectModifier );
}

//--------------------------------------------------------------------

void SfxToolBoxControl::DoubleClick()
{
}

//--------------------------------------------------------------------

void SfxToolBoxControl::Click()
{
}

//--------------------------------------------------------------------

SfxPopupWindowType SfxToolBoxControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_NONE;
}

//--------------------------------------------------------------------

SfxPopupWindow* SfxToolBoxControl::CreatePopupWindow()
{
    return 0;
}

SfxPopupWindow* SfxToolBoxControl::CreatePopupWindowCascading()
{
    return 0;
}

//--------------------------------------------------------------------

Window* SfxToolBoxControl::CreateItemWindow( Window * )
{
    return 0;
}

//--------------------------------------------------------------------

SfxFrameStatusListener::SfxFrameStatusListener(
    const Reference< XMultiServiceFactory >& rServiceManager,
    const Reference< XFrame >& xFrame,
    SfxStatusListenerInterface* pCallee ) :
    svt::FrameStatusListener( rServiceManager, xFrame ),
    m_pCallee( pCallee )
{
}

//--------------------------------------------------------------------

SfxFrameStatusListener::~SfxFrameStatusListener()
{
}

//--------------------------------------------------------------------

// XStatusListener
void SAL_CALL SfxFrameStatusListener::statusChanged( const ::com::sun::star::frame::FeatureStateEvent& rEvent )
throw ( ::com::sun::star::uno::RuntimeException )
{
    SfxViewFrame* pViewFrame = NULL;
    Reference < XController > xController;

    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( m_xFrame.is() )
        xController = m_xFrame->getController();

    Reference < XDispatchProvider > xProvider( xController, UNO_QUERY );
    if ( xProvider.is() )
    {
        Reference < XDispatch > xDisp = xProvider->queryDispatch( rEvent.FeatureURL, ::rtl::OUString(), 0 );
        if ( xDisp.is() )
        {
            Reference< XUnoTunnel > xTunnel( xDisp, UNO_QUERY );
            SfxOfficeDispatch* pDisp = NULL;
            if ( xTunnel.is() )
            {
                sal_Int64 nImplementation = xTunnel->getSomething(SfxOfficeDispatch::impl_getStaticIdentifier());
                pDisp = (SfxOfficeDispatch*)(nImplementation);
            }

            if ( pDisp )
                pViewFrame = pDisp->GetDispatcher_Impl()->GetFrame();
        }
    }

    USHORT nSlotId = 0;
    SfxSlotPool& rPool = SFX_APP()->GetSlotPool( pViewFrame );
    const SfxSlot* pSlot = rPool.GetUnoSlot( rEvent.FeatureURL.Path );
    if ( pSlot )
        nSlotId = pSlot->GetSlotId();

    if ( nSlotId > 0 )
    {
        if ( rEvent.Requery )
        {
            // requery for the notified state
            addStatusListener( rEvent.FeatureURL.Complete );
        }
        else
        {
            SfxItemState eState = SFX_ITEM_DISABLED;
            SfxPoolItem* pItem = NULL;
            if ( rEvent.IsEnabled )
            {
                eState = SFX_ITEM_AVAILABLE;
                ::com::sun::star::uno::Type pType = rEvent.State.getValueType();

                if ( pType == ::getVoidCppuType() )
                {
                    pItem = new SfxVoidItem( nSlotId );
                    eState = SFX_ITEM_UNKNOWN;
                }
                else if ( pType == ::getBooleanCppuType() )
                {
                    sal_Bool bTemp ;
                    rEvent.State >>= bTemp ;
                    pItem = new SfxBoolItem( nSlotId, bTemp );
                }
                else if ( pType == ::getCppuType((const sal_uInt16*)0) )
                {
                    sal_uInt16 nTemp ;
                    rEvent.State >>= nTemp ;
                    pItem = new SfxUInt16Item( nSlotId, nTemp );
                }
                else if ( pType == ::getCppuType((const sal_uInt32*)0) )
                {
                    sal_uInt32 nTemp ;
                    rEvent.State >>= nTemp ;
                    pItem = new SfxUInt32Item( nSlotId, nTemp );
                }
                else if ( pType == ::getCppuType((const ::rtl::OUString*)0) )
                {
                    ::rtl::OUString sTemp ;
                    rEvent.State >>= sTemp ;
                    pItem = new SfxStringItem( nSlotId, sTemp );
                }
                else if ( pType == ::getCppuType((const ::com::sun::star::frame::status::ItemStatus*)0) )
                {
                    ItemStatus aItemStatus;
                    rEvent.State >>= aItemStatus;
                    eState = aItemStatus.State;
                    pItem = new SfxVoidItem( nSlotId );
                }
                else
                {
                    if ( pSlot )
                        pItem = pSlot->GetType()->CreateItem();
                    if ( pItem )
                    {
                        pItem->SetWhich( nSlotId );
                        pItem->PutValue( rEvent.State );
                    }
                    else
                        pItem = new SfxVoidItem( nSlotId );
                }
            }

            if ( m_pCallee )
                m_pCallee->StateChanged( nSlotId, eState, pItem );
            delete pItem;
        }
    }
}

//--------------------------------------------------------------------

SfxPopupWindow::SfxPopupWindow(
    USHORT nId,
    const Reference< XFrame >& rFrame,
    WinBits nBits ) :
    FloatingWindow( SFX_APP()->GetTopWindow(), nBits )
    , m_nId( nId )
    , m_bFloating(FALSE)
    , m_pStatusListener( 0 )
    , m_bCascading( FALSE )
    , m_xFrame( rFrame )
{
    m_xServiceManager = ::comphelper::getProcessServiceFactory();

    Window* pWindow = SFX_APP()->GetTopWindow();
    while ( pWindow && !pWindow->IsSystemWindow() )
        pWindow = pWindow->GetParent();

    if ( pWindow )
        ((SystemWindow *)pWindow)->GetTaskPaneList()->AddWindow( this );
}

//--------------------------------------------------------------------

SfxPopupWindow::SfxPopupWindow(
    USHORT nId,
    const Reference< XFrame >& rFrame,
    const ResId &rId ) :
    FloatingWindow( SFX_APP()->GetTopWindow(), rId )
    , m_nId( nId )
    , m_bFloating(FALSE)
    , m_pStatusListener( 0 )
    , m_bCascading( FALSE )
    , m_xFrame( rFrame )
{
    m_xServiceManager = ::comphelper::getProcessServiceFactory();

    Window* pWindow = SFX_APP()->GetTopWindow();
    while ( pWindow && !pWindow->IsSystemWindow() )
        pWindow = pWindow->GetParent();

    if ( pWindow )
        ((SystemWindow *)pWindow)->GetTaskPaneList()->AddWindow( this );
}

//--------------------------------------------------------------------

SfxPopupWindow::~SfxPopupWindow()
{
    if ( m_xStatusListener.is() )
    {
        m_xStatusListener->dispose();
        m_xStatusListener.clear();
    }

    Window* pWindow = SFX_APP()->GetTopWindow();
    while ( pWindow && !pWindow->IsSystemWindow() )
        pWindow = pWindow->GetParent();

    if ( pWindow )
        ((SystemWindow *)pWindow)->GetTaskPaneList()->RemoveWindow( this );
}

//--------------------------------------------------------------------

SfxFrameStatusListener* SfxPopupWindow::GetOrCreateStatusListener()
{
    if ( !m_xStatusListener.is() )
    {
        m_pStatusListener = new SfxFrameStatusListener(
                                    m_xServiceManager,
                                    m_xFrame,
                                    this );
        m_xStatusListener = Reference< XComponent >( static_cast< cppu::OWeakObject* >(
                                                        m_pStatusListener ), UNO_QUERY );
    }

    return m_pStatusListener;
}

//--------------------------------------------------------------------

void SfxPopupWindow::BindListener()
{
    GetOrCreateStatusListener();
    if ( m_xStatusListener.is() )
        m_pStatusListener->bindListener();
}

//--------------------------------------------------------------------

void SfxPopupWindow::UnbindListener()
{
    GetOrCreateStatusListener();
    if ( m_xStatusListener.is() )
        m_pStatusListener->unbindListener();
}

//--------------------------------------------------------------------

void SfxPopupWindow::AddStatusListener( const rtl::OUString& rCommandURL )
{
    GetOrCreateStatusListener();
    if ( m_xStatusListener.is() )
        m_pStatusListener->addStatusListener( rCommandURL );
}

//--------------------------------------------------------------------

void SfxPopupWindow::RemoveStatusListener( const rtl::OUString& rCommandURL )
{
    GetOrCreateStatusListener();
    if ( m_xStatusListener.is() )
        m_pStatusListener->removeStatusListener( rCommandURL );
}

//--------------------------------------------------------------------

void SfxPopupWindow::UpdateStatus( const rtl::OUString& rCommandURL )
{
    GetOrCreateStatusListener();
    if ( m_xStatusListener.is() )
        m_pStatusListener->updateStatus( rCommandURL );
}

//--------------------------------------------------------------------

BOOL SfxPopupWindow::Close()
{
    m_bFloating = FALSE;
    FloatingWindow::Close();

    Delete(0);
    return TRUE;
}

//--------------------------------------------------------------------

void SfxPopupWindow::PopupModeEnd()
{
    //! to allow PopupModeEndHdl to be called
    FloatingWindow::PopupModeEnd();

    if ( IsVisible() )
    {
        // wurde abgerissen
        DeleteFloatingWindow();
        m_bFloating = TRUE;
    }
    else
        Close();
}

//--------------------------------------------------------------------

void SfxPopupWindow::DeleteFloatingWindow()
{
    if ( m_bFloating )
    {
        Hide();
        Delete(0);
    }
}

//--------------------------------------------------------------------

void SfxPopupWindow::MouseMove( const ::MouseEvent& rMEvt )
{
    if ( m_bCascading == FALSE )
        FloatingWindow::MouseMove( rMEvt );
    else
    {
        // MouseMove-Event an die Children forwarden
        ::Point       aPos = rMEvt.GetPosPixel();
        ::Point       aScrPos = OutputToScreenPixel( aPos );
        int i = 0;
        Window* pWindow = GetChild( i );
        while ( pWindow )
        {
            ::MouseEvent aMEvt( pWindow->ScreenToOutputPixel( aScrPos ),
                              rMEvt.GetClicks(), rMEvt.GetMode(),
                              rMEvt.GetButtons(), rMEvt.GetModifier() );
            pWindow->MouseMove( rMEvt );
            pWindow->Update();
            i++;
            pWindow = GetChild( i );
        }
    }
}

//--------------------------------------------------------------------

void SfxPopupWindow::StartCascading()
{
    m_bCascading= TRUE;
}

void SfxPopupWindow::EndCascading()
{
    m_bCascading = FALSE;
}

//--------------------------------------------------------------------

SfxPopupWindow* SfxPopupWindow::Clone() const

/*  [Beschreibung]

    Diese Methode mu\s "uberladen werden, um dieses Popup auch im
    Presentations-Modus anzuzeigen. Sie wird gerufen, wenn ein Show()
    sinnlos w"are, da der Parent nicht das Presentations-Window ist.
    Beim neu erzeugen wird automatisch das neue Top-Window verwendet, so
    da\s der Parent das Presentations-Window ist und das neue Popup somit
    sichtbar ist.
*/

{
    return 0;
}

//--------------------------------------------------------------------

void SfxPopupWindow::StateChanged( USHORT nSID, SfxItemState eState,
                                   const SfxPoolItem* pState )
/*  [Bescheibung]

    Siehe auch <SfxControllerItem::StateChanged()>. Au\serdem wird
    bei eState==SFX_ITEM_DISABLED das Popup gehided und in allen anderen
    F"allen, falls es floating ist, wieder angezeigt. Daher mu\s die
    Basisklasse i.d.R. gerufen werden.

    Es findet wegen des Parents eine Sonderbehandlung f"ur den
    Presentationsmodus statt.

*/

{
    if ( SFX_ITEM_DISABLED == eState )
    {
        Hide();
    }
    else if ( m_bFloating )
    {
        Show( TRUE, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
    }
}

//--------------------------------------------------------------------

IMPL_LINK( SfxPopupWindow, Delete, void *, pvoid )
{
    if ( m_aDeleteLink.IsSet() )
        m_aDeleteLink.Call( this );
    delete this;
    return 0;
}

//--------------------------------------------------------------------

SfxRotatedToolboxControl::SfxRotatedToolboxControl( USHORT nSlotID, USHORT nId, ToolBox& rBox ) :
    SfxToolBoxControl( nSlotID, nId, rBox )
{
    addStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ImageOrientation" )));
}

//--------------------------------------------------------------------

SfxRotatedToolboxControl::~SfxRotatedToolboxControl()
{
}

//--------------------------------------------------------------------

void SfxRotatedToolboxControl::StateChanged( USHORT nSlotId, SfxItemState eState, const SfxPoolItem* pState )
{
    USHORT   nId    = GetId();
    ToolBox& rTbx   = GetToolBox();

    if ( SID_IMAGE_ORIENTATION == nSlotId )
    {
        const SfxImageItem* pItem = PTR_CAST( SfxImageItem, pState );
        if ( pItem )
        {
            sal_Int32 lRotation( pItem->GetRotation() );
            rTbx.SetItemImageMirrorMode( nId, FALSE );
            rTbx.SetItemImageAngle( nId, lRotation );
        }
    }
    else
        SfxToolBoxControl::StateChanged( nSlotId, eState, pState );
}

//--------------------------------------------------------------------

SfxAppToolBoxControl_Impl::SfxAppToolBoxControl_Impl( USHORT nSlotId, USHORT nId, ToolBox& rBox )
    : SfxToolBoxControl( nSlotId, nId, rBox )
    , bBigImages( FALSE )
    , pMenu( 0 )
{
    rBox.SetHelpId( nId, HID_TBXCONTROL_FILENEW );
    rBox.SetItemBits( nId,  rBox.GetItemBits( nId ) | TIB_DROPDOWN);

    // Determine the current background color of the menus
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    m_bWasHiContrastMode    = rSettings.GetMenuColor().IsDark();
    m_bShowMenuImages       = SvtMenuOptions().IsMenuIconsEnabled();

    SetImage( String() );
}

SfxAppToolBoxControl_Impl::~SfxAppToolBoxControl_Impl()
{
    delete pMenu;
}

//_____________________________________________________
/*
    it return the existing state of the given URL in the popupmenu of this toolbox control.

    If the given URL can be located as an action command of one menu item of the
    popup menu of this control, we return TRUE. Otherwhise we return FALSE.
    Further we return a fallback URL, in case we have to return FALSE. Because
    the outside code must select a valid item of the popup menu everytime ...
    and we define it here. By the way this m ethod was written to handle
    error situations gracefully. E.g. it can be called during creation time
    but then we have no valid menu. For this case we know another fallback URL.
    Then we return the private:factory/ URL of the default factory.

    @param  *pMenu
                pounts to the popup menu, on which item we try to locate the given URL
                Can be NULL! Search will be supressed then.

    @param  sURL
                the URL for searching

    @param  pFallback
                contains the fallback URL in case we return FALSE
                Must point to valid memory!

    @return TRUE - if URL could be located as an item of the popup menu.
            FALSE - otherwhise.
*/
BOOL Impl_ExistURLInMenu( const PopupMenu *pMenu     ,
                          const String    &sURL      ,
                                String    *pFallback )
{
    BOOL bValidFallback = FALSE;
    if (pMenu && sURL.Len())
    {
        USHORT c = pMenu->GetItemCount();
        for (USHORT p=0; p<c; ++p)
        {
            USHORT nId = pMenu->GetItemId(p);
            String aCmd( pMenu->GetItemCommand(nId) );

            if (!bValidFallback && aCmd.Len())
            {
                *pFallback = aCmd;
                bValidFallback = TRUE;
            }

            if (sURL.Equals(aCmd))
                return TRUE;
        }
    }

    if (!bValidFallback)
    {
        *pFallback  = DEFINE_CONST_UNICODE("private:factory/");
        *pFallback += String(SvtModuleOptions().GetDefaultModuleName());
    }

    return FALSE;
}

long Select_Impl( void* pHdl, void* pVoid );

SfxPopupWindow* SfxAppToolBoxControl_Impl::CreatePopupWindow()
{
    SfxApplication* pApp = SFX_APP();
    ToolBox& rBox = GetToolBox();
    ::Rectangle aRect( rBox.GetItemRect( GetId() ) );

    USHORT nId = GetId();
    BOOL bNew = FALSE;

    if ( !pMenu )
    {
        ::framework::MenuConfiguration aConf( m_xServiceManager );
        if ( m_aCommandURL.equalsAscii( ".uno:AddDirect" ))
            pMenu = aConf.CreateBookmarkMenu( m_xFrame, BOOKMARK_NEWMENU );
        else
            pMenu = aConf.CreateBookmarkMenu( m_xFrame, BOOKMARK_WIZARDMENU );
    }

    if( pMenu )
    {
        pMenu->SetSelectHdl( Link( NULL, Select_Impl ) );
        pMenu->SetActivateHdl( LINK( this, SfxAppToolBoxControl_Impl, Activate ));
        rBox.SetItemDown( GetId(), TRUE );
        USHORT nSelected = pMenu->Execute( &rBox, aRect, POPUPMENU_EXECUTE_DOWN );
        if ( nSelected )
        {
            aLastURL = pMenu->GetItemCommand( nSelected );
            SetImage( pMenu->GetItemCommand( nSelected ) );
        }

        rBox.SetItemDown( GetId(), FALSE );
    }

    return 0;
}

void SfxAppToolBoxControl_Impl::SetImage( const String &rURL )
{
    /* We accept URL's here only, which exist as items of our internal popup menu.
       All other ones will be ignored and a fallback is used ... */
    String aURL = rURL;
    String sFallback;
    BOOL bValid = Impl_ExistURLInMenu(pMenu,aURL,&sFallback);
    if (!bValid)
        aURL = sFallback;

    BOOL bBig = ( SfxImageManager::GetCurrentSymbolSet() == SFX_SYMBOLS_LARGE );
    GetToolBox().SetItemImage( GetId(),
                               SvFileInformationManager::GetImage( INetURLObject( aURL ),
                               bBig,
                                GetToolBox().GetBackground().GetColor().IsDark() ) );
    aLastURL = aURL;
}

void SfxAppToolBoxControl_Impl::StateChanged
(
    USHORT              nSlotId,
    SfxItemState        eState,
    const SfxPoolItem*  pState
)
{
    if ( pState && pState->ISA(SfxStringItem) )
    {
        // Important step for following SetImage() call!
        // It needs the valid pMenu item to fullfill it's specification
        // to check for supported URLs ...
        if ( !pMenu )
        {
            ::framework::MenuConfiguration aConf( m_xServiceManager );
            // This toolbox controller is used for two popup menus (new documents and wizards!). Create the correct
            // popup menu according to the slot ID our controller has been initialized with.
            if ( nSlotId == SID_NEWDOCDIRECT )
                pMenu = aConf.CreateBookmarkMenu( m_xFrame, BOOKMARK_NEWMENU );
            else
                pMenu = aConf.CreateBookmarkMenu( m_xFrame, BOOKMARK_WIZARDMENU );
        }

        GetToolBox().EnableItem( GetId(), eState != SFX_ITEM_DISABLED );
        SetImage(((const SfxStringItem*)pState)->GetValue());
    }
    else
        SfxToolBoxControl::StateChanged( nSlotId, eState, pState );
}

//--------------------------------------------------------------------

void SfxAppToolBoxControl_Impl::Select( BOOL bMod1 )
{
    if( aLastURL.Len() )
    {
        URL                            aTargetURL;
        Reference< XDispatch >         xDispatch;
        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );

        if ( xDispatchProvider.is() )
        {
            aTargetURL.Complete = aLastURL;
            Reference < XURLTransformer > xTrans( m_xServiceManager->createInstance(
                                                    rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer" )),
                                                UNO_QUERY );
            xTrans->parseStrict( aTargetURL );

            ::rtl::OUString aTarget( ::rtl::OUString::createFromAscii( "_default" ));
            if ( pMenu )
            {
                ::framework::MenuConfiguration::Attributes* pMenuAttributes =
                    (::framework::MenuConfiguration::Attributes*)pMenu->GetUserValue( pMenu->GetCurItemId() );

                if ( pMenuAttributes )
                    aTarget = pMenuAttributes->aTargetFrame;
            }

            xDispatch = xDispatchProvider->queryDispatch( aTargetURL, aTarget, 0 );

            if ( xDispatch.is() )
            {
                Sequence< PropertyValue > aArgs( 1 );

                aArgs[0].Name = ::rtl::OUString::createFromAscii( "Referer" );
                aArgs[0].Value = makeAny( ::rtl::OUString::createFromAscii( SFX_REFERER_USER ));

                ExecuteInfo* pExecuteInfo = new ExecuteInfo;
                pExecuteInfo->xDispatch     = xDispatch;
                pExecuteInfo->aTargetURL    = aTargetURL;
                pExecuteInfo->aArgs         = aArgs;
                Application::PostUserEvent( STATIC_LINK(0, SfxAppToolBoxControl_Impl, ExecuteHdl_Impl), pExecuteInfo );
            }
        }
    }
    else
        SfxToolBoxControl::Select( bMod1 );
}

//--------------------------------------------------------------------
long Select_Impl( void* pHdl, void* pVoid )
{
    Menu* pMenu = (Menu*)pVoid;
    String aURL( pMenu->GetItemCommand( pMenu->GetCurItemId() ) );

    if( !aURL.Len() )
        return 0;

    Reference < ::com::sun::star::frame::XFramesSupplier > xDesktop =
            Reference < ::com::sun::star::frame::XFramesSupplier >( ::comphelper::getProcessServiceFactory()->createInstance(
                                                                        DEFINE_CONST_UNICODE("com.sun.star.frame.Desktop") ), UNO_QUERY );
    Reference < ::com::sun::star::frame::XFrame > xFrame( xDesktop, UNO_QUERY );

    URL aTargetURL;
    aTargetURL.Complete = aURL;
    Reference < XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance(
                                            rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer" )),
                                          UNO_QUERY );
    xTrans->parseStrict( aTargetURL );

    Reference < XDispatchProvider > xProv( xFrame, UNO_QUERY );
    Reference < XDispatch > xDisp;
    if ( xProv.is() )
    {
        if ( aTargetURL.Protocol.compareToAscii("slot:") == COMPARE_EQUAL )
            xDisp = xProv->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );
        else
        {
            ::rtl::OUString aTargetFrame( ::rtl::OUString::createFromAscii( "_blank" ) );
            ::framework::MenuConfiguration::Attributes* pMenuAttributes =
                (::framework::MenuConfiguration::Attributes*)pMenu->GetUserValue( pMenu->GetCurItemId() );

            if ( pMenuAttributes )
                aTargetFrame = pMenuAttributes->aTargetFrame;

            xDisp = xProv->queryDispatch( aTargetURL, aTargetFrame , 0 );
        }
    }

    if ( xDisp.is() )
    {
        SfxAppToolBoxControl_Impl::ExecuteInfo* pExecuteInfo = new SfxAppToolBoxControl_Impl::ExecuteInfo;
        pExecuteInfo->xDispatch     = xDisp;
        pExecuteInfo->aTargetURL    = aTargetURL;
        pExecuteInfo->aArgs         = Sequence< PropertyValue >();
        Application::PostUserEvent( STATIC_LINK( 0, SfxAppToolBoxControl_Impl, ExecuteHdl_Impl), pExecuteInfo );
    }

    return TRUE;
}

IMPL_LINK( SfxAppToolBoxControl_Impl, Activate, Menu *, pMenu )
{
    if ( pMenu )
    {
        const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
        BOOL bIsHiContrastMode  = rSettings.GetMenuColor().IsDark();
        BOOL bShowMenuImages    = SvtMenuOptions().IsMenuIconsEnabled();

        if (( bIsHiContrastMode != m_bWasHiContrastMode ) ||
            ( bShowMenuImages   != m_bShowMenuImages    )    )
        {
            m_bWasHiContrastMode = bIsHiContrastMode;
            m_bShowMenuImages    = bShowMenuImages;

            USHORT nCount = pMenu->GetItemCount();
            for ( USHORT nSVPos = 0; nSVPos < nCount; nSVPos++ )
            {
                USHORT nId = pMenu->GetItemId( nSVPos );
                if ( pMenu->GetItemType( nSVPos ) != MENUITEM_SEPARATOR )
                {
                    if ( bShowMenuImages )
                    {
                        sal_Bool        bImageSet = sal_False;
                        ::rtl::OUString aImageId;
                        ::framework::MenuConfiguration::Attributes* pMenuAttributes =
                            (::framework::MenuConfiguration::Attributes*)pMenu->GetUserValue( nId );

                        if ( pMenuAttributes )
                            aImageId = pMenuAttributes->aImageId; // Retrieve image id from menu attributes

                        if ( aImageId.getLength() > 0 )
                        {
                            Reference< ::com::sun::star::frame::XFrame > xFrame;
                            Image aImage = GetImage( xFrame, aImageId, FALSE, bIsHiContrastMode );
                            if ( !!aImage )
                            {
                                bImageSet = sal_True;
                                pMenu->SetItemImage( nId, aImage );
                            }
                        }

                        String aCmd( pMenu->GetItemCommand( nId ) );
                        if ( !bImageSet && aCmd.Len() )
                        {
                            Image aImage = SvFileInformationManager::GetImage( aCmd, FALSE, bIsHiContrastMode );
                            if ( !!aImage )
                                pMenu->SetItemImage( nId, aImage );
                        }
                    }
                    else
                        pMenu->SetItemImage( nId, Image() );
                }
            }
        }

        return TRUE;
    }

    return FALSE;
}

//--------------------------------------------------------------------

IMPL_STATIC_LINK( SfxAppToolBoxControl_Impl, ExecuteHdl_Impl, ExecuteInfo*, pExecuteInfo )
{
    try
    {
        // Asynchronous execution as this can lead to our own destruction!
        // Framework can recycle our current frame and the layout manager disposes all user interface
        // elements if a component gets detached from its frame!
        pExecuteInfo->xDispatch->dispatch( pExecuteInfo->aTargetURL, pExecuteInfo->aArgs );
    }
    catch ( Exception& )
    {
    }

    delete pExecuteInfo;
    return 0;
}

//--------------------------------------------------------------------

void SfxAppToolBoxControl_Impl::Click( )
{
}

//--------------------------------------------------------------------

SfxAddonsToolBoxControl_Impl::SfxAddonsToolBoxControl_Impl( USHORT nSlotId, USHORT nId, ToolBox& rBox )
    : SfxToolBoxControl( nSlotId, nId, rBox )
    , bBigImages( FALSE )
    , pMenu( 0 )
{
    rBox.SetItemBits( nId,  rBox.GetItemBits( nId ) | TIB_DROPDOWN);

    // Determine the current background color of the menus
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    m_bWasHiContrastMode    = rSettings.GetMenuColor().IsDark();
    m_bShowMenuImages       = SvtMenuOptions().IsMenuIconsEnabled();
}

SfxAddonsToolBoxControl_Impl::~SfxAddonsToolBoxControl_Impl()
{
    delete pMenu;
}

void SfxAddonsToolBoxControl_Impl::RefreshMenuImages( Menu* pSVMenu )
{
    framework::AddonsOptions    aAddonOptions;

    Reference<com::sun::star::frame::XFrame> aXFrame( m_xFrame );
    USHORT nCount = pSVMenu->GetItemCount();
    for ( USHORT nPos = 0; nPos < nCount; nPos++ )
    {
        USHORT nId = pSVMenu->GetItemId( nPos );
        if ( pSVMenu->GetItemType( nPos ) != MENUITEM_SEPARATOR )
        {
            if ( m_bShowMenuImages )
            {
                sal_Bool        bImageSet = sal_False;
                ::rtl::OUString aImageId;

                ::framework::MenuConfiguration::Attributes* pMenuAttributes =
                    (::framework::MenuConfiguration::Attributes*)pSVMenu->GetUserValue( nId );

                if ( pMenuAttributes )
                    aImageId = pMenuAttributes->aImageId; // Retrieve image id from menu attributes

                if ( aImageId.getLength() > 0 )
                {
                    Image aImage = GetImage( aXFrame, aImageId, FALSE, m_bWasHiContrastMode );
                    if ( !!aImage )
                    {
                        bImageSet = sal_True;
                        pSVMenu->SetItemImage( nId, aImage );
                    }
                }

                if ( !bImageSet )
                {
                    rtl::OUString aMenuItemCommand = pSVMenu->GetItemCommand( nId );
                    Image aImage = GetImage( aXFrame, aMenuItemCommand, FALSE, m_bWasHiContrastMode );
                    if ( !aImage )
                        aImage = aAddonOptions.GetImageFromURL( aMenuItemCommand, FALSE, m_bWasHiContrastMode );

                    pSVMenu->SetItemImage( nId, aImage );
                }
            }
            else
                pSVMenu->SetItemImage( nId, Image() );

            // Go recursive through the sub-menus
            PopupMenu* pPopup = pSVMenu->GetPopupMenu( nId );
            if ( pPopup )
                RefreshMenuImages( pPopup );
        }
    }
}

void SfxAddonsToolBoxControl_Impl::StateChanged
(
    USHORT              nSlotId,
    SfxItemState        eState,
    const SfxPoolItem*  pState
)
{
    SfxToolBoxControl::StateChanged( nSlotId, eState, pState );
}

//--------------------------------------------------------------------

void SfxAddonsToolBoxControl_Impl::Select( BOOL bMod1 )
{
    SfxApplication* pApp = SFX_APP();
    ToolBox& rBox = GetToolBox();
    ::Rectangle aRect(  rBox.GetItemRect( GetId() ) );

    USHORT nId = GetId();
    BOOL bNew = FALSE;

    if ( !pMenu )
    {
        Reference< com::sun::star::frame::XFrame > aXFrame( m_xFrame );
        pMenu = framework::AddonMenuManager::CreateAddonMenu( aXFrame );
        RefreshMenuImages( pMenu );
    }

    if( pMenu )
    {
        const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
        BOOL bIsHiContrastMode  = rSettings.GetMenuColor().IsDark();
        BOOL bShowMenuImages    = SvtMenuOptions().IsMenuIconsEnabled();

        if (( bIsHiContrastMode != m_bWasHiContrastMode ) ||
            ( bShowMenuImages   != m_bShowMenuImages    )    )
        {
            m_bWasHiContrastMode = bIsHiContrastMode;
            m_bShowMenuImages    = bShowMenuImages;
            RefreshMenuImages( pMenu );
        }

        rBox.SetItemDown( GetId(), TRUE );
        pMenu->Execute( &rBox, aRect, POPUPMENU_EXECUTE_DOWN );
        rBox.SetItemDown( GetId(), FALSE );
    }
}

//--------------------------------------------------------------------

void SfxAddonsToolBoxControl_Impl::Click( )
{
    SfxToolBoxControl::Click();
}

//--------------------------------------------------------------------
/*

SfxDragToolBoxControl_Impl::SfxDragToolBoxControl_Impl( USHORT nId, ToolBox& rBox )
    :   SfxToolBoxControl( nId, rBox )
{
}

Window* SfxDragToolBoxControl_Impl::CreateItemWindow( Window *pParent )
{
    return new SfxDragButton_Impl( pParent );
}

void SfxDragToolBoxControl_Impl::Select( BOOL bMod1 )
{
    SfxToolBoxControl::Select( bMod1 );
//    GetBindings().Execute( SID_CREATELINK, NULL, 0, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
}

SfxDragButton_Impl::SfxDragButton_Impl( Window *pParent )
    : FixedImage( pParent )
{
    SetBackground( pParent->GetBackground() );
    Image aImage( SfxResId( RID_GREPIMAGE ) );
    SetImage( aImage );
    Size aSize( aImage.GetSizePixel() );
    aSize.Width() += 6;
    SetSizePixel( aSize );
}

void SfxDragButton_Impl::Command ( const CommandEvent& rCEvt )
{
    if( rCEvt.GetCommand() != COMMAND_STARTDRAG )
        return;
}

void SfxDragButton_Impl::MouseMove( const ::MouseEvent& rEvt )
{
    SetPointer( Pointer( POINTER_HAND ) );
}

void SfxDragButton_Impl::MouseButtonDown( const ::MouseEvent& rEvt )
{
}
*/
/*
SfxHistoryToolBoxControl_Impl::SfxHistoryToolBoxControl_Impl( USHORT nId, ToolBox& rBox )
  : SfxToolBoxControl( nId, rBox )
{
    aTimer.SetTimeout( 250 );
    aTimer.SetTimeoutHdl( LINK( this, SfxHistoryToolBoxControl_Impl, Timeout ) );
}

void SfxHistoryToolBoxControl_Impl::Select( BOOL bMod1 )
{
    aTimer.Stop();
    SfxToolBoxControl::Select( bMod1 );
}

IMPL_LINK( SfxHistoryToolBoxControl_Impl, Timeout, Timer *, pTimer )
{
    ToolBox& rBox = GetToolBox();

    rBox.SetItemDown( GetId(), TRUE );

    SfxApplication* pApp = SFX_APP();
    ::Rectangle aRect(  rBox.GetItemRect( GetId() ) );
    aRect.SetPos( rBox.OutputToScreenPixel( aRect.TopLeft() ) );


//  SfxFrame *pTop = GetBindings().GetDispatcher()->GetFrame()->GetFrame()->GetTopFrame();
//  if ( pTop->ExecuteHistoryMenu_Impl( GetId(), aRect, POPUPMENU_EXECUTE_UP ) )
//      rBox.SetItemDown( GetId(), FALSE );

    return 0;
}

void SfxHistoryToolBoxControl_Impl::Click( )
{
    aTimer.Start();
    SfxToolBoxControl::Click();
}
*/

SfxReloadToolBoxControl_Impl::SfxReloadToolBoxControl_Impl( USHORT nSlotId, USHORT nId, ToolBox& rBox )
  : SfxToolBoxControl( nSlotId, nId, rBox )
{
}

void SfxReloadToolBoxControl_Impl::Select( USHORT nModifier )
{
    URL                            aTargetURL;
    Reference< XDispatch >         xDispatch;
    Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );

    if ( xDispatchProvider.is() )
    {
        aTargetURL.Complete = m_aCommandURL;
        Reference < XURLTransformer > xTrans( m_xServiceManager->createInstance(
                                                rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer" )),
                                            UNO_QUERY );
        xTrans->parseStrict( aTargetURL );
        xDispatch = xDispatchProvider->queryDispatch( aTargetURL, rtl::OUString(), 0 );
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Reload" ));
        aArgs[0].Value = makeAny( sal_Bool( TRUE ));
        if ( xDispatch.is() )
            xDispatch->dispatch( aTargetURL, aArgs );
    }
}

/*
SfxUnoToolBoxControl* SfxToolBoxControl::CreateControl( const String& rCmd,
        USHORT nId, ToolBox *pBox, SfxBindings &rBindings )
{
    return new SfxUnoToolBoxControl( rCmd, nId, pBox, rBindings );
}

SfxUnoToolBoxControl::SfxUnoToolBoxControl( const String& rCmd, USHORT nId,
    ToolBox *pBox, SfxBindings& rBindings )
    : SfxToolBoxControl( nId, *pBox, rBindings )
{
    UnBind();
    pUnoCtrl = new SfxUnoControllerItem( this, rBindings, rCmd );
    pUnoCtrl->acquire();
    pUnoCtrl->GetNewDispatch();
}

SfxUnoToolBoxControl::~SfxUnoToolBoxControl()
{
    pUnoCtrl->UnBind();
    pUnoCtrl->release();
}

void SfxUnoToolBoxControl::Select( BOOL bMod1 )
{
    pUnoCtrl->Execute();
}
*/

