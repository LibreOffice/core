/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include <svl/stritem.hxx>
#ifndef GCC
#endif
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/frame/status/ItemStatus.hpp>
#include <com/sun/star/frame/status/ItemState.hpp>
#include <com/sun/star/awt/MouseButton.hpp>

#include <vcl/status.hxx>

#include <sfx2/app.hxx>
#include "sfx2/stbitem.hxx"
#include "sfxtypes.hxx"
#include <sfx2/msg.hxx>
#include "arrdecl.hxx"
#include <sfx2/bindings.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/module.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/unoctitm.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfx.hrc>

#include <comphelper/processfactory.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif
#include <toolkit/helper/convert.hxx>

using namespace ::com::sun::star;

//--------------------------------------------------------------------

sal_uInt16 SfxStatusBarControl::convertAwtToVCLMouseButtons( sal_Int16 nAwtMouseButtons )
{
    sal_uInt16 nVCLMouseButtons( 0 );

    if ( nAwtMouseButtons & awt::MouseButton::LEFT )
        nVCLMouseButtons |= MOUSE_LEFT;
    if ( nAwtMouseButtons & awt::MouseButton::RIGHT )
        nVCLMouseButtons |= MOUSE_RIGHT;
    if ( nAwtMouseButtons & awt::MouseButton::MIDDLE )
        nVCLMouseButtons |= MOUSE_MIDDLE;

    return nVCLMouseButtons;
}

//--------------------------------------------------------------------

svt::StatusbarController* SAL_CALL SfxStatusBarControllerFactory(
    const uno::Reference< frame::XFrame >& rFrame,
    StatusBar* pStatusBar,
    unsigned short nID,
    const ::rtl::OUString& aCommandURL )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    util::URL aTargetURL;
    aTargetURL.Complete = aCommandURL;
    uno::Reference < util::XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance(
        rtl::OUString::createFromAscii( "com.sun.star.util.URLTransformer" )), uno::UNO_QUERY );
    xTrans->parseStrict( aTargetURL );

    SfxObjectShell* pObjShell = NULL;
    uno::Reference < frame::XController > xController;
    uno::Reference < frame::XModel > xModel;
    if ( rFrame.is() )
    {
        xController = rFrame->getController();
        if ( xController.is() )
            xModel = xController->getModel();
    }

    if ( xModel.is() )
    {
        // Get tunnel from model to retrieve the SfxObjectShell pointer from it
        ::com::sun::star::uno::Reference < ::com::sun::star::lang::XUnoTunnel > xObj( xModel, uno::UNO_QUERY );
        ::com::sun::star::uno::Sequence < sal_Int8 > aSeq = SvGlobalName( SFX_GLOBAL_CLASSID ).GetByteSequence();
        if ( xObj.is() )
        {
            sal_Int64 nHandle = xObj->getSomething( aSeq );
            if ( nHandle )
                        pObjShell = reinterpret_cast< SfxObjectShell* >( sal::static_int_cast< sal_IntPtr >( nHandle ));
        }
    }

    SfxModule*     pModule   = pObjShell ? pObjShell->GetModule() : NULL;
    SfxSlotPool*   pSlotPool = 0;

    if ( pModule )
        pSlotPool = pModule->GetSlotPool();
    else
        pSlotPool = &(SfxSlotPool::GetSlotPool( NULL ));

    const SfxSlot* pSlot = pSlotPool->GetUnoSlot( aTargetURL.Path );
    if ( pSlot )
    {
        sal_uInt16 nSlotId = pSlot->GetSlotId();
        if ( nSlotId > 0 )
        {
            rtl::OString aCmd(".uno:");
            aCmd += pSlot->GetUnoName();
            pStatusBar->SetHelpId( nSlotId, aCmd );
            return SfxStatusBarControl::CreateControl( nSlotId, nID, pStatusBar, pModule );
        }
    }

    return NULL;
}

//--------------------------------------------------------------------

SfxStatusBarControl::SfxStatusBarControl
(
    sal_uInt16      nSlotID,            /* Slot-Id, mit der diese Instanz
                                       verbunden wird. Wurde bei der
                                       Registrierung eine Slot-Id != 0
                                       angegeben, ist dies immer die dort
                                       angegebene. */
    sal_uInt16      nCtrlID,            /* ID of this controller in the status bar */

    StatusBar&  rBar                /* Referenz auf die StatusBar, f"ur die
                                       dieses Control erzeugt wurde. */
)

/*  [Beschreibung]

    Konstruktor der Klasse SfxStatusBarControl. Die Subclasses werden
    bei Bedarf per Factory vom SFx erzeugt.

    Instanzen dieser Basisklasse werden f"ur alle StatusBar-Felder
    erzeugt, f"ur die keine speziellen registriert wurden.
*/

:   svt::StatusbarController(),
    nSlotId( nSlotID ),
    nId( nCtrlID ),
    pBar( &rBar )
{
}

//--------------------------------------------------------------------

SfxStatusBarControl::~SfxStatusBarControl()

/*  [Beschreibung]

    Destruktor der Klasse SfxStatusBarControl. Die Instanzen dieser
    Klasse und deren Subklassen werden vom SFx zerst"ort.
*/

{}

//--------------------------------------------------------------------
// XInterface
uno::Any SAL_CALL SfxStatusBarControl::queryInterface( const uno::Type & rType )
throw( uno::RuntimeException)
{
    return svt::StatusbarController::queryInterface( rType );
}

void SAL_CALL SfxStatusBarControl::acquire() throw()
{
    OWeakObject::acquire();
}

void SAL_CALL SfxStatusBarControl::release() throw()
{
    OWeakObject::release();
}

//--------------------------------------------------------------------
// XEventListener
void SAL_CALL SfxStatusBarControl::disposing( const lang::EventObject& aEvent )
throw( uno::RuntimeException )
{
    svt::StatusbarController::disposing( aEvent );
}

//--------------------------------------------------------------------
// XComponent
void SAL_CALL SfxStatusBarControl::dispose()
throw (uno::RuntimeException)
{
    svt::StatusbarController::dispose();
}

//--------------------------------------------------------------------
// XStatusListener
void SAL_CALL SfxStatusBarControl::statusChanged( const frame::FeatureStateEvent& rEvent )
throw ( ::com::sun::star::uno::RuntimeException )
{
    SfxViewFrame* pViewFrame = NULL;
    uno::Reference < frame::XController > xController;

    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( m_xFrame.is() )
        xController = m_xFrame->getController();

    uno::Reference < frame::XDispatchProvider > xProvider( xController, uno::UNO_QUERY );
    if ( xProvider.is() )
    {
        uno::Reference < frame::XDispatch > xDisp = xProvider->queryDispatch( rEvent.FeatureURL, ::rtl::OUString(), 0 );
        if ( xDisp.is() )
        {
            uno::Reference< lang::XUnoTunnel > xTunnel( xDisp, uno::UNO_QUERY );
            SfxOfficeDispatch* pDisp = NULL;
            if ( xTunnel.is() )
            {
                sal_Int64 nImplementation = xTunnel->getSomething(SfxOfficeDispatch::impl_getStaticIdentifier());
                pDisp = reinterpret_cast< SfxOfficeDispatch* >(sal::static_int_cast< sal_IntPtr >( nImplementation ));
            }

            if ( pDisp )
                pViewFrame = pDisp->GetDispatcher_Impl()->GetFrame();
        }
    }

    sal_uInt16 nSlotID = 0;
    SfxSlotPool& rPool = SfxSlotPool::GetSlotPool( pViewFrame );
    const SfxSlot* pSlot = rPool.GetUnoSlot( rEvent.FeatureURL.Path );
    if ( pSlot )
        nSlotID = pSlot->GetSlotId();

    if ( nSlotID > 0 )
    {
        if ( rEvent.Requery )
            svt::StatusbarController::statusChanged( rEvent );
        else
        {
            SfxItemState eState = SFX_ITEM_DISABLED;
            SfxPoolItem* pItem = NULL;
            if ( rEvent.IsEnabled )
            {
                eState = SFX_ITEM_AVAILABLE;
                uno::Type pType = rEvent.State.getValueType();

                if ( pType == ::getVoidCppuType() )
                {
                    pItem = new SfxVoidItem( nSlotID );
                    eState = SFX_ITEM_UNKNOWN;
                }
                else if ( pType == ::getBooleanCppuType() )
                {
                    sal_Bool bTemp = 0;
                    rEvent.State >>= bTemp ;
                    pItem = new SfxBoolItem( nSlotID, bTemp );
                }
                else if ( pType == ::getCppuType((const sal_uInt16*)0) )
                {
                    sal_uInt16 nTemp = 0;
                    rEvent.State >>= nTemp ;
                    pItem = new SfxUInt16Item( nSlotID, nTemp );
                }
                else if ( pType == ::getCppuType((const sal_uInt32*)0) )
                {
                    sal_uInt32 nTemp = 0;
                    rEvent.State >>= nTemp ;
                    pItem = new SfxUInt32Item( nSlotID, nTemp );
                }
                else if ( pType == ::getCppuType((const ::rtl::OUString*)0) )
                {
                    ::rtl::OUString sTemp ;
                    rEvent.State >>= sTemp ;
                    pItem = new SfxStringItem( nSlotID, sTemp );
                }
                else if ( pType == ::getCppuType((const ::com::sun::star::frame::status::ItemStatus*)0) )
                {
                    frame::status::ItemStatus aItemStatus;
                    rEvent.State >>= aItemStatus;
                    eState = aItemStatus.State;
                    pItem = new SfxVoidItem( nSlotID );
                }
                else
                {
                    if ( pSlot )
                        pItem = pSlot->GetType()->CreateItem();
                    if ( pItem )
                    {
                        pItem->SetWhich( nSlotID );
                        pItem->PutValue( rEvent.State );
                    }
                    else
                        pItem = new SfxVoidItem( nSlotID );
                }
            }

            StateChanged( nSlotID, eState, pItem );
            delete pItem;
        }
    }
}

//--------------------------------------------------------------------
// XStatusbarController

::sal_Bool SAL_CALL SfxStatusBarControl::mouseButtonDown(
    const awt::MouseEvent& rMouseEvent )
throw ( uno::RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    ::Point aPos( rMouseEvent.X, rMouseEvent.Y );

    ::MouseEvent aMouseEvent( aPos,
                              (sal_uInt16)rMouseEvent.ClickCount,
                              0,
                              convertAwtToVCLMouseButtons( rMouseEvent.Buttons ),
                              0 );

    return MouseButtonDown( aMouseEvent );
}

//--------------------------------------------------------------------

::sal_Bool SAL_CALL SfxStatusBarControl::mouseMove(
    const awt::MouseEvent& rMouseEvent )
throw (uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    ::Point aPos( rMouseEvent.X, rMouseEvent.Y );

    ::MouseEvent aMouseEvent( aPos,
                              (sal_uInt16)rMouseEvent.ClickCount,
                              0,
                              convertAwtToVCLMouseButtons( rMouseEvent.Buttons ),
                              0 );
    return MouseMove( aMouseEvent );
}

//--------------------------------------------------------------------

::sal_Bool SAL_CALL SfxStatusBarControl::mouseButtonUp(
    const ::awt::MouseEvent& rMouseEvent )
throw ( uno::RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    ::Point aPos( rMouseEvent.X, rMouseEvent.Y );

    ::MouseEvent aMouseEvent( aPos,
                              (sal_uInt16)rMouseEvent.ClickCount,
                              0,
                              convertAwtToVCLMouseButtons( rMouseEvent.Buttons ),
                              0 );
    return MouseButtonUp( aMouseEvent );
}

//--------------------------------------------------------------------

void SAL_CALL SfxStatusBarControl::command(
    const awt::Point& rPos,
    ::sal_Int32 nCommand,
    ::sal_Bool /*bMouseEvent*/,
    const ::com::sun::star::uno::Any& /*aData*/ )
throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    ::Point aPos( rPos.X, rPos.Y );
    CommandEvent aCmdEvent( aPos, (sal_uInt16)nCommand, sal_True, NULL );

    Command( aCmdEvent );
}

//--------------------------------------------------------------------

void SAL_CALL SfxStatusBarControl::paint(
    const uno::Reference< awt::XGraphics >& xGraphics,
    const awt::Rectangle& rOutputRectangle,
    ::sal_Int32 nItemId,
    ::sal_Int32 nStyle )
throw ( ::uno::RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    OutputDevice* pOutDev = VCLUnoHelper::GetOutputDevice( xGraphics );;
    if ( pOutDev )
    {
        ::Rectangle aRect = VCLRectangle( rOutputRectangle );
        UserDrawEvent aUserDrawEvent( pOutDev, aRect, (sal_uInt16)nItemId, (sal_uInt16)nStyle );
        Paint( aUserDrawEvent );
    }
}

//--------------------------------------------------------------------

void SAL_CALL SfxStatusBarControl::click()
throw ( uno::RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    Click();
}

//--------------------------------------------------------------------

void SAL_CALL SfxStatusBarControl::doubleClick()
throw ( uno::RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    DoubleClick();
}

//--------------------------------------------------------------------
// old sfx2 interface
//--------------------------------------------------------------------

void SfxStatusBarControl::StateChanged
(
    sal_uInt16              nSID,
    SfxItemState        eState,
    const SfxPoolItem*  pState  /* Zeiger auf ein SfxPoolItem, welches nur
                                   innerhalb dieses Methodenaufrufs g"ultig
                                   ist. Es kann ein 0-Pointer, ein Pointer
                                   auf ein SfxVoidItem oder auf den Typ, f"ur
                                   den die Subclass von SfxStatusBarControl
                                   registriert ist vorkommen. */
)

/*  [Beschreibung]

    Die Basisimplementation versteht Items vom Type SfxStringItem, bei
    denen der Text in das Status-Zeilen-Feld eingetragen wird und
    SfxVoidItem, bei denen das Feld geleert wird. Die Basisimplementierng
    sollte in "uberladenen Methoden nicht gerufen werden.
*/

{
    DBG_MEMTEST();
    DBG_ASSERT( pBar != 0, "setting state to dangling StatusBar" );

    const SfxStringItem* pStr = PTR_CAST( SfxStringItem, pState );
    if ( eState == SFX_ITEM_AVAILABLE && pStr )
        pBar->SetItemText( nSID, pStr->GetValue() );
    else
    {
        DBG_ASSERT( eState != SFX_ITEM_AVAILABLE || pState->ISA(SfxVoidItem),
                    "wrong SfxPoolItem subclass in SfxStatusBarControl" );
        pBar->SetItemText( nSID, String() );
    }
}

//--------------------------------------------------------------------

sal_Bool SfxStatusBarControl::MouseButtonDown( const MouseEvent & )

/*  [Beschreibung]

    Diese virtuelle Methode ist eine Weiterleitung des Events
    MouseButtonDown() der StatusBar, falls die Maus-Position innerhalb
    des Bereichs des betreffenden Items ist, oder die Maus von diesem
    Control mit <SfxStatusBarControl::CaptureMouse()> gecaptured wurde.

    Die Defaultimplementierung ist leer und gibt FALSE zur"uck.


    [Rueckgabewert]

    sal_Bool                TRUE
                        das Event wurde bearbeitet und soll nicht an
                        die StatusBar weitergeleitet werden

                        FALSE
                        das Event wurde nicht bearbeitet und soll an
                        die StatusBar weitergeleitet werden
*/

{
    return sal_False;
}

//--------------------------------------------------------------------

sal_Bool SfxStatusBarControl::MouseMove( const MouseEvent & )

/*  [Beschreibung]

    Diese virtuelle Methode ist eine Weiterleitung des Events
    MouseMove() der StatusBar, falls die Maus-Position innerhalb
    des Bereichs des betreffenden Items ist, oder die Maus von diesem
    Control mit <SfxStatusBarControl::CaptureMouse()> gecaptured wurde.

    Die Defaultimplementierung ist leer und gibt FALSE zur"uck.


    [Rueckgabewert]

    sal_Bool                TRUE
                        das Event wurde bearbeitet und soll nicht an
                        die StatusBar weitergeleitet werden

                        FALSE
                        das Event wurde nicht bearbeitet und soll an
                        die StatusBar weitergeleitet werden
*/

{
    return sal_False;
}

//--------------------------------------------------------------------

sal_Bool SfxStatusBarControl::MouseButtonUp( const MouseEvent & )

/*  [Beschreibung]

    Diese virtuelle Methode ist eine Weiterleitung des Events
    MouseButtonUp() der StatusBar, falls die Maus-Position innerhalb
    des Bereichs des betreffenden Items ist, oder die Maus von diesem
    Control mit <SfxStatusBarControl::CaptureMouse()> gecaptured wurde.

    Die Defaultimplementierung ist leer und gibt FALSE zur"uck.


    [Rueckgabewert]

    sal_Bool                TRUE
                        das Event wurde bearbeitet und soll nicht an
                        die StatusBar weitergeleitet werden

                        FALSE
                        das Event wurde nicht bearbeitet und soll an
                        die StatusBar weitergeleitet werden
*/

{
    return sal_False;
}

//--------------------------------------------------------------------

void SfxStatusBarControl::Command( const CommandEvent& )

/*  [Beschreibung]

    Diese virtuelle Methode wird gerufen, wenn f"ur dieses SfxStatusBarControl
    ein CommandEvent f"ur erkannt wurde.

    Die Defaultimplementierung ist leer.
*/

{
}

//--------------------------------------------------------------------

void SfxStatusBarControl::Click()

/*  [Beschreibung]

    Diese virtuelle Methode wird gerufen, wenn der Anwender mit der Maus
    in das zu diesem Control geh"orige Feld der Statuszeile klickt.

    Die Defaultimplementierung ist leer.
*/

{
}

//--------------------------------------------------------------------

void SfxStatusBarControl::DoubleClick()

/*  [Beschreibung]

    Diese virtuelle Methode wird gerufen, wenn der Anwender mit der Maus
    in das zu diesem Control geh"orige Feld der Statuszeile doppel-klickt.
*/

{
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aArgs;
    execute( aArgs );
}

//--------------------------------------------------------------------

void SfxStatusBarControl::Paint
(
    const UserDrawEvent& /* Referenz auf einen UserDrawEvent */
)

/*  [Beschreibung]

    Diese virtuelle Methode wird gerufen, falls das betreffende Feld
    mit SIB_USERDRAW gekennzeichnet ist, um den Inhalt zu zeichnen.
    Die Ausgabe mu"s auf dem in durch rUDEvt.GetDevice() erh"altlichen
    OutputDevice innerhalb des durch rUDEvt.GetRect() angegebenenen
    Rechtecks erfolgen.

    Die Defaultimplementierung ist leer.
*/

{
}

//--------------------------------------------------------------------

void SfxStatusBarControl::CaptureMouse()
{
}

//--------------------------------------------------------------------

void SfxStatusBarControl::ReleaseMouse()
{
}

//--------------------------------------------------------------------

SfxStatusBarControl* SfxStatusBarControl::CreateControl
(
    sal_uInt16     nSlotID,
    sal_uInt16     nStbId,
    StatusBar* pBar,
    SfxModule* pMod
)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    SfxApplication *pApp = SFX_APP();

    SfxSlotPool *pSlotPool;
    if ( pMod )
        pSlotPool = pMod->GetSlotPool();
    else
        pSlotPool = &SfxSlotPool::GetSlotPool();

    TypeId aSlotType = pSlotPool->GetSlotType(nSlotID);
    if ( aSlotType )
    {
        if ( pMod )
        {
            SfxStbCtrlFactArr_Impl *pFactories = pMod->GetStbCtrlFactories_Impl();
            if ( pFactories )
            {
                SfxStbCtrlFactArr_Impl &rFactories = *pFactories;
                for ( sal_uInt16 nFactory = 0; nFactory < rFactories.Count(); ++nFactory )
                if ( rFactories[nFactory]->nTypeId == aSlotType &&
                     ( ( rFactories[nFactory]->nSlotId == 0 ) ||
                     ( rFactories[nFactory]->nSlotId == nSlotID) ) )
                    return rFactories[nFactory]->pCtor( nSlotID, nStbId, *pBar );
            }
        }

        SfxStbCtrlFactArr_Impl &rFactories = pApp->GetStbCtrlFactories_Impl();
        for ( sal_uInt16 nFactory = 0; nFactory < rFactories.Count(); ++nFactory )
        if ( rFactories[nFactory]->nTypeId == aSlotType &&
             ( ( rFactories[nFactory]->nSlotId == 0 ) ||
             ( rFactories[nFactory]->nSlotId == nSlotID) ) )
            return rFactories[nFactory]->pCtor( nSlotID, nStbId, *pBar );
    }

    return NULL;
}

//--------------------------------------------------------------------
void SfxStatusBarControl::RegisterStatusBarControl(SfxModule* pMod, SfxStbCtrlFactory* pFact)
{
    SFX_APP()->RegisterStatusBarControl_Impl( pMod, pFact );
}
//--------------------------------------------------------------------
