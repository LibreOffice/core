/*************************************************************************
 *
 *  $RCSfile: impframe.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:36 $
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

#include "impframe.hxx"

#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

#include "frame.hxx"
#include "arrdecl.hxx"
#include <urlframe.hxx>
#include <bindings.hxx>
#include <app.hxx>
#include <appdata.hxx>
#include <appuno.hxx>
#include <childwin.hxx>
#include <viewfrm.hxx>

#include <osl/mutex.hxx>
#include "sfxuno.hxx"

using namespace vos;

class SfxDummyCtrl_Impl : public ::com::sun::star::frame::XFrameActionListener,
                            public ::com::sun::star::lang::XTypeProvider,
                            public ::cppu::OWeakObject
{
public:
    ::osl::Mutex                                aMutex;
    ::cppu::OMultiTypeInterfaceContainerHelper  aLC;

    SfxDummyController_Impl*            pController;

    SFX_DECL_XINTERFACE_XTYPEPROVIDER


                                        SfxDummyCtrl_Impl()
                                            : aLC( aMutex )
                                            , pController(0)
                                        {}

                                        ~SfxDummyCtrl_Impl();

    virtual void     SAL_CALL                       frameAction(const ::com::sun::star::frame::FrameActionEvent& aEvent) throw( ::com::sun::star::uno::RuntimeException );
    virtual void     SAL_CALL                           disposing( const ::com::sun::star::lang::EventObject& Source ) throw( ::com::sun::star::uno::RuntimeException );
};

SFX_IMPL_XINTERFACE_2( SfxDummyCtrl_Impl, OWeakObject, ::com::sun::star::frame::XFrameActionListener, ::com::sun::star::lang::XEventListener )
SFX_IMPL_XTYPEPROVIDER_2( SfxDummyCtrl_Impl, ::com::sun::star::frame::XFrameActionListener, ::com::sun::star::lang::XEventListener )

SfxDummyCtrl_Impl::~SfxDummyCtrl_Impl()
{
}

void SAL_CALL SfxDummyCtrl_Impl::frameAction(const ::com::sun::star::frame::FrameActionEvent& aEvent) throw( ::com::sun::star::uno::RuntimeException )
{
    if ( pController && aEvent.Frame == pController->getFrame() && aEvent.Action == ::com::sun::star::frame::FrameAction_FRAME_ACTIVATED && pController->xWindow.is() )
        pController->xWindow->setFocus();
}

void SAL_CALL  SfxDummyCtrl_Impl::disposing( const ::com::sun::star::lang::EventObject& Source ) throw( ::com::sun::star::uno::RuntimeException )
{
    if ( pController )
        pController->getFrame()->removeFrameActionListener( this );
}

SfxDummyController_Impl::SfxDummyController_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > & xComponent )
    : xWindow( xComponent )
{
    pImp = new SfxDummyCtrl_Impl;
    pImp->pController = this;
    pImp->acquire();
}

SfxDummyController_Impl::~SfxDummyController_Impl()
{
    pImp->pController = NULL;
    pImp->release();
}

SFX_IMPL_XINTERFACE_2( SfxDummyController_Impl, OWeakObject,::com::sun::star::frame::XController,::com::sun::star::lang::XComponent )
SFX_IMPL_XTYPEPROVIDER_2( SfxDummyController_Impl, ::com::sun::star::frame::XController,::com::sun::star::lang::XComponent )

void SAL_CALL SfxDummyController_Impl::attachFrame( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > & xArg ) throw( ::com::sun::star::uno::RuntimeException )
{
    xFrame = xArg;
    xFrame->addFrameActionListener( pImp );
}

sal_Bool SAL_CALL SfxDummyController_Impl::attachModel( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & xModel ) throw( ::com::sun::star::uno::RuntimeException )
{
    return sal_False;
}

sal_Bool SAL_CALL SfxDummyController_Impl::suspend( sal_Bool bSuspend ) throw( ::com::sun::star::uno::RuntimeException )
{
    if ( bSuspend )
        getFrame()->removeFrameActionListener( pImp );
    else
        getFrame()->addFrameActionListener( pImp );
    return sal_True;
}

::com::sun::star::uno::Any SAL_CALL SfxDummyController_Impl::getViewData() throw( ::com::sun::star::uno::RuntimeException )
{
    return ::com::sun::star::uno::Any();
}

void SAL_CALL SfxDummyController_Impl::restoreViewData( const ::com::sun::star::uno::Any& Value ) throw( ::com::sun::star::uno::RuntimeException )
{
}

::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > SAL_CALL SfxDummyController_Impl::getFrame() throw( ::com::sun::star::uno::RuntimeException )
{
    return xFrame;
}

::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > SAL_CALL  SfxDummyController_Impl::getModel() throw( ::com::sun::star::uno::RuntimeException )
{
    return ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > ();
}

void SAL_CALL SfxDummyController_Impl::dispose() throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::lang::EventObject aObject;
    aObject.Source = (::com::sun::star::frame::XController*)this;
    pImp->aLC.disposeAndClear(aObject);
}

void SAL_CALL SfxDummyController_Impl::addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener ) throw( ::com::sun::star::uno::RuntimeException )
{
    pImp->aLC.addInterface( ::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >*)0), aListener );
}

void SAL_CALL SfxDummyController_Impl::removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener ) throw( ::com::sun::star::uno::RuntimeException )
{
    pImp->aLC.removeInterface( ::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >*)0), aListener );
}

void SfxFrame_Impl::AppendPickEntry( SfxFramePickEntry_Impl* pEntry )
{
#ifdef DBG_UTIL
    ByteString aStr( "BrowseHistory : " );
    ByteString aTrace;
#endif
    if ( !pHistory )
        pHistory = new SfxFrameHistory_Impl;

    // Sind schon Eintr"age drin ?
    sal_uInt32 nCount = pHistory->Count();
#ifdef DBG_UTIL
    aTrace = aStr;
    aTrace += ByteString("Count = ");
    aTrace += ByteString::CreateFromInt32( nCount );
    DBG_TRACE( aTrace.GetBuffer() );
#endif
    if ( nCount )
    {
        sal_uInt32 nCurHisEntry = pHistory->GetCurPos();
#ifdef DBG_UTIL
        aTrace = aStr;
        aTrace += ByteString("Current position = ");
        aTrace += ByteString::CreateFromInt32( nCurHisEntry );
        DBG_TRACE( aTrace.GetBuffer() );
#endif
        if ( LIST_ENTRY_NOTFOUND != nCurHisEntry )
        {
            // UndoEntries ueber dem aktuellen Eintrag werden geloescht
            for ( sal_uInt32 nEntry = pHistory->Count();
                  nEntry-- > nCurHisEntry + 1;  )
            {
#ifdef DBG_UTIL
                aTrace = aStr;
                aTrace += ByteString("deleting #");
                aTrace += ByteString::CreateFromInt32( nEntry );
                DBG_TRACE( aTrace.GetBuffer() );
#endif
                delete pHistory->Remove( nEntry );
            }
        }

        nCount = pHistory->Count();
        if ( nCount == HISTORY_MAXSIZE )
            delete pHistory->Remove( 0UL );
        nCurHisEntry = pHistory->GetCurPos();
    }

#ifdef DBG_UTIL
    aTrace = aStr;
    aTrace += ByteString("Inserting ");
    aTrace += ByteString( U2S( pEntry->pDescriptor->GetActualURL().GetMainURL() ) );
    DBG_TRACE( aTrace.GetBuffer() );
#endif
    pHistory->Insert( pEntry, pHistory->Count() );
}

void SfxFrame_Impl::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    SfxSimpleHint* pHint = PTR_CAST( SfxSimpleHint, &rHint );
    if( pHint && pHint->GetId() == SFX_HINT_CANCELLABLE && pCurrentViewFrame )
    {
        // vom Cancel-Manager
        SfxBindings &rBind = pCurrentViewFrame->GetBindings();
        rBind.Invalidate( SID_BROWSE_STOP );
        if ( !rBind.IsInRegistrations() )
            rBind.Update( SID_BROWSE_STOP );
        rBind.Invalidate( SID_BROWSE_STOP );
    }
}

