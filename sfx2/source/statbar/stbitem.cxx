/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stbitem.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:21:55 $
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
#ifndef GCC
#pragma hdrstop
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
#ifndef _COM_SUN_STAR_FRAME_STATUS_ITEMSTATUS_HPP_
#include <com/sun/star/frame/status/ItemStatus.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_STATUS_ITEMSTATE_HPP_
#include <com/sun/star/frame/status/ItemState.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_MOUSEBUTTON_HPP_
#include <com/sun/star/awt/MouseButton.hpp>
#endif

#include <vcl/status.hxx>

#include "stbitem.hxx"
#include "sfxtypes.hxx"
#include "msg.hxx"
#include "arrdecl.hxx"
#include "bindings.hxx"
#include "msgdescr.hxx"
#include "msgpool.hxx"
#include "module.hxx"
#include "dispatch.hxx"
#include "unoctitm.hxx"
#include "objsh.hxx"
#include "sfx.hrc"

#include <comphelper/processfactory.hxx>
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif
#ifndef _TOOLKIT_HELPER_CONVERT_HXX_
#include <toolkit/helper/convert.hxx>
#endif

using namespace ::com::sun::star;

//--------------------------------------------------------------------

USHORT SfxStatusBarControl::convertAwtToVCLMouseButtons( sal_Int16 nAwtMouseButtons )
{
    USHORT nVCLMouseButtons( 0 );

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

    SfxViewFrame*   pViewFrame = NULL;
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
                pObjShell = (SfxObjectShell*) (sal_Int32*) nHandle;
        }
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
            return SfxStatusBarControl::CreateControl( nSlotId, nID, pStatusBar, pModule );
    }

    return NULL;
}

//--------------------------------------------------------------------

SfxStatusBarControl::SfxStatusBarControl
(
    USHORT      nSlotId,            /* Slot-Id, mit der diese Instanz
                                       verbunden wird. Wurde bei der
                                       Registrierung eine Slot-Id != 0
                                       angegeben, ist dies immer die dort
                                       angegebene. */
    USHORT      nId,                /* ID of this controller in the status bar */

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
    pBar( &rBar ),
    nSlotId( nSlotId ),
    nId( nId )
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
                    frame::status::ItemStatus aItemStatus;
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

//--------------------------------------------------------------------
// XStatusbarController

::sal_Bool SAL_CALL SfxStatusBarControl::mouseButtonDown(
    const awt::MouseEvent& rMouseEvent )
throw ( uno::RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    ::Point aPos( rMouseEvent.X, rMouseEvent.Y );

    ::MouseEvent aMouseEvent( aPos,
                              (USHORT)rMouseEvent.ClickCount,
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
                              (USHORT)rMouseEvent.ClickCount,
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
                              (USHORT)rMouseEvent.ClickCount,
                              0,
                              convertAwtToVCLMouseButtons( rMouseEvent.Buttons ),
                              0 );
    return MouseButtonUp( aMouseEvent );
}

//--------------------------------------------------------------------

void SAL_CALL SfxStatusBarControl::command(
    const awt::Point& rPos,
    ::sal_Int32 nCommand,
    ::sal_Bool bMouseEvent,
    const ::com::sun::star::uno::Any& aData )
throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    ::Point aPos( rPos.X, rPos.Y );
    CommandEvent aCmdEvent( aPos, (USHORT)nCommand, TRUE, NULL );

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
        UserDrawEvent aUserDrawEvent( pOutDev, aRect, (USHORT)nItemId, (USHORT)nStyle );
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
    USHORT              nSID,
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

BOOL SfxStatusBarControl::MouseButtonDown( const MouseEvent & )

/*  [Beschreibung]

    Diese virtuelle Methode ist eine Weiterleitung des Events
    MouseButtonDown() der StatusBar, falls die Maus-Position innerhalb
    des Bereichs des betreffenden Items ist, oder die Maus von diesem
    Control mit <SfxStatusBarControl::CaptureMouse()> gecaptured wurde.

    Die Defaultimplementierung ist leer und gibt FALSE zur"uck.


    [Rueckgabewert]

    BOOL                TRUE
                        das Event wurde bearbeitet und soll nicht an
                        die StatusBar weitergeleitet werden

                        FALSE
                        das Event wurde nicht bearbeitet und soll an
                        die StatusBar weitergeleitet werden
*/

{
    return FALSE;
}

//--------------------------------------------------------------------

BOOL SfxStatusBarControl::MouseMove( const MouseEvent & )

/*  [Beschreibung]

    Diese virtuelle Methode ist eine Weiterleitung des Events
    MouseMove() der StatusBar, falls die Maus-Position innerhalb
    des Bereichs des betreffenden Items ist, oder die Maus von diesem
    Control mit <SfxStatusBarControl::CaptureMouse()> gecaptured wurde.

    Die Defaultimplementierung ist leer und gibt FALSE zur"uck.


    [Rueckgabewert]

    BOOL                TRUE
                        das Event wurde bearbeitet und soll nicht an
                        die StatusBar weitergeleitet werden

                        FALSE
                        das Event wurde nicht bearbeitet und soll an
                        die StatusBar weitergeleitet werden
*/

{
    return FALSE;
}

//--------------------------------------------------------------------

BOOL SfxStatusBarControl::MouseButtonUp( const MouseEvent & )

/*  [Beschreibung]

    Diese virtuelle Methode ist eine Weiterleitung des Events
    MouseButtonUp() der StatusBar, falls die Maus-Position innerhalb
    des Bereichs des betreffenden Items ist, oder die Maus von diesem
    Control mit <SfxStatusBarControl::CaptureMouse()> gecaptured wurde.

    Die Defaultimplementierung ist leer und gibt FALSE zur"uck.


    [Rueckgabewert]

    BOOL                TRUE
                        das Event wurde bearbeitet und soll nicht an
                        die StatusBar weitergeleitet werden

                        FALSE
                        das Event wurde nicht bearbeitet und soll an
                        die StatusBar weitergeleitet werden
*/

{
    return FALSE;
}

//--------------------------------------------------------------------

void SfxStatusBarControl::Command( const CommandEvent &rEvt )

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
    const UserDrawEvent& rUDEvt  /* Referenz auf einen UserDrawEvent */
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
//  ( (SfxStatusBar_Impl*) pBar )->CaptureMouse(*this);
}

//--------------------------------------------------------------------

void SfxStatusBarControl::ReleaseMouse()
{
//  ( (SfxStatusBar_Impl*) pBar )->ReleaseMouse(*this);
}

//--------------------------------------------------------------------

SfxStatusBarControl* SfxStatusBarControl::CreateControl
(
    USHORT     nSlotId,
    USHORT     nStbId,
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
        pSlotPool = &pApp->GetSlotPool();

    TypeId aSlotType = pSlotPool->GetSlotType(nSlotId);
    if ( aSlotType )
    {
        if ( pMod )
        {
            SfxStbCtrlFactArr_Impl *pFactories = pMod->GetStbCtrlFactories_Impl();
            if ( pFactories )
            {
                SfxStbCtrlFactArr_Impl &rFactories = *pFactories;
                for ( USHORT nFactory = 0; nFactory < rFactories.Count(); ++nFactory )
                if ( rFactories[nFactory]->nTypeId == aSlotType &&
                     ( ( rFactories[nFactory]->nSlotId == 0 ) ||
                     ( rFactories[nFactory]->nSlotId == nSlotId) ) )
                    return rFactories[nFactory]->pCtor( nSlotId, nStbId, *pBar );
            }
        }

        SfxStbCtrlFactArr_Impl &rFactories = pApp->GetStbCtrlFactories_Impl();
        for ( USHORT nFactory = 0; nFactory < rFactories.Count(); ++nFactory )
        if ( rFactories[nFactory]->nTypeId == aSlotType &&
             ( ( rFactories[nFactory]->nSlotId == 0 ) ||
             ( rFactories[nFactory]->nSlotId == nSlotId) ) )
            return rFactories[nFactory]->pCtor( nSlotId, nStbId, *pBar );
    }

    return NULL;
}
//--------------------------------------------------------------------
