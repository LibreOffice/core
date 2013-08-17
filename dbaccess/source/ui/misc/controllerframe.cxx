/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <dbaccess/controllerframe.hxx>
#include <dbaccess/IController.hxx>

#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/document/XDocumentEventBroadcaster.hpp>
#include <com/sun/star/frame/XController2.hpp>

#include <cppuhelper/implbase1.hxx>
#include <rtl/ref.hxx>
#include <sfx2/objsh.hxx>
#include <tools/diagnose_ex.h>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>

namespace dbaui
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::frame::XFrame;
    using ::com::sun::star::frame::FrameAction;
    using ::com::sun::star::frame::FrameAction_FRAME_ACTIVATED;
    using ::com::sun::star::frame::FrameAction_FRAME_UI_ACTIVATED;
    using ::com::sun::star::frame::FrameAction_FRAME_DEACTIVATING;
    using ::com::sun::star::frame::FrameAction_FRAME_UI_DEACTIVATING;
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::frame::XController;
    using ::com::sun::star::frame::XController2;
    using ::com::sun::star::frame::XFramesSupplier;
    using ::com::sun::star::sdb::XOfficeDatabaseDocument;
    using ::com::sun::star::awt::XTopWindow;
    using ::com::sun::star::awt::XTopWindowListener;
    using ::com::sun::star::awt::XWindow2;
    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::document::XDocumentEventBroadcaster;
    using ::com::sun::star::awt::XWindow;

    // FrameWindowActivationListener
    typedef ::cppu::WeakImplHelper1 <   XTopWindowListener
                                    >   FrameWindowActivationListener_Base;
    class FrameWindowActivationListener : public FrameWindowActivationListener_Base
    {
    public:
        FrameWindowActivationListener( ControllerFrame_Data& _rData );

        void dispose();

    protected:
        ~FrameWindowActivationListener();

        // XTopWindowListener
        virtual void SAL_CALL windowOpened( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL windowClosing( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL windowClosed( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL windowMinimized( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL windowNormalized( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL windowActivated( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL windowDeactivated( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    private:
        void impl_checkDisposed_throw() const;
        void impl_registerOnFrameContainerWindow_nothrow( bool _bRegister );

    private:
        ControllerFrame_Data*   m_pData;
    };

    // ControllerFrame_Data
    struct ControllerFrame_Data
    {
        ControllerFrame_Data( IController& _rController )
            :m_rController( _rController )
            ,m_xFrame()
            ,m_xDocEventBroadcaster()
            ,m_pListener()
            ,m_bActive( false )
            ,m_bIsTopLevelDocumentWindow( false )
        {
        }

        IController&                                        m_rController;
        Reference< XFrame >                                 m_xFrame;
        Reference< XDocumentEventBroadcaster >              m_xDocEventBroadcaster;
        ::rtl::Reference< FrameWindowActivationListener >   m_pListener;
        bool                                                m_bActive;
        bool                                                m_bIsTopLevelDocumentWindow;
    };

    // helper
    static void lcl_setFrame_nothrow( ControllerFrame_Data& _rData, const Reference< XFrame >& _rxFrame )
    {
        // release old listener
        if ( _rData.m_pListener.get() )
        {
            _rData.m_pListener->dispose();
            _rData.m_pListener = NULL;
        }

        // remember new frame
        _rData.m_xFrame = _rxFrame;

        // create new listener
        if ( _rData.m_xFrame.is() )
            _rData.m_pListener = new FrameWindowActivationListener( _rData );

        // at this point in time, we can assume the controller also has a model set, if it supports models
        try
        {
            Reference< XController > xController( _rData.m_rController.getXController(), UNO_SET_THROW );
            Reference< XModel > xModel( xController->getModel() );
            if ( xModel.is() )
                _rData.m_xDocEventBroadcaster.set( xModel, UNO_QUERY );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    static bool lcl_isActive_nothrow( const Reference< XFrame >& _rxFrame )
    {
        bool bIsActive = false;
        try
        {
            if ( _rxFrame.is() )
            {
                Reference< XWindow2 > xWindow( _rxFrame->getContainerWindow(), UNO_QUERY_THROW );
                bIsActive = xWindow->isActive();
            }

        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return bIsActive;
    }

    /** updates various global and local states with a new active component

        In particular, the following are updated
            * the global working document (aka Basic's ThisComponent in the application
            Basic), with our controller's model, or the controller itself if there is no such
            model.
    */
    static void lcl_updateActiveComponents_nothrow( const ControllerFrame_Data& _rData )
    {
        try
        {
            Reference< XController > xCompController( _rData.m_rController.getXController() );
            OSL_ENSURE( xCompController.is(), "lcl_updateActiveComponents_nothrow: can't do anything without a controller!" );
            if ( !xCompController.is() )
                return;

            if ( _rData.m_bActive && _rData.m_bIsTopLevelDocumentWindow )
            {
                // set the "current component" at the SfxObjectShell
                Reference< XModel > xModel( xCompController->getModel() );
                Reference< XInterface > xCurrentComponent;
                if ( xModel.is() )
                    xCurrentComponent = xModel;
                else
                    xCurrentComponent = xCompController;
                SfxObjectShell::SetCurrentComponent( xCurrentComponent );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    /** broadcasts the OnFocus resp. OnUnfocus event
    */
    static void lcl_notifyFocusChange_nothrow( ControllerFrame_Data& _rData, bool _bActive )
    {
        try
        {
            if ( _rData.m_xDocEventBroadcaster.is() )
            {
                OUString sEventName = _bActive ? OUString("OnFocus") : OUString("OnUnfocus");
                Reference< XController2 > xController( _rData.m_rController.getXController(), UNO_QUERY_THROW );
                _rData.m_xDocEventBroadcaster->notifyDocumentEvent( sEventName, xController, Any() );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    static void lcl_updateActive_nothrow( ControllerFrame_Data& _rData, bool _bActive )
    {
        if ( _rData.m_bActive == _bActive )
            return;
        _rData.m_bActive = _bActive;

        lcl_updateActiveComponents_nothrow( _rData );
        lcl_notifyFocusChange_nothrow( _rData, _bActive );
    }

    FrameWindowActivationListener::FrameWindowActivationListener( ControllerFrame_Data& _rData )
        :m_pData( &_rData )
    {
        impl_registerOnFrameContainerWindow_nothrow( true );
    }

    FrameWindowActivationListener::~FrameWindowActivationListener()
    {
    }

    void FrameWindowActivationListener::dispose()
    {
        impl_registerOnFrameContainerWindow_nothrow( false );
        m_pData = NULL;
    }

    void FrameWindowActivationListener::impl_registerOnFrameContainerWindow_nothrow( bool _bRegister )
    {
        OSL_ENSURE( m_pData && m_pData->m_xFrame.is(), "FrameWindowActivationListener::impl_registerOnFrameContainerWindow_nothrow: no frame!" );
        if ( !m_pData || !m_pData->m_xFrame.is() )
            return;

        try
        {
            void ( SAL_CALL XTopWindow::*pListenerAction )( const Reference< XTopWindowListener >& ) =
                _bRegister ? &XTopWindow::addTopWindowListener : &XTopWindow::removeTopWindowListener;

            const Reference< XWindow > xContainerWindow( m_pData->m_xFrame->getContainerWindow(), UNO_SET_THROW );
            if ( _bRegister )
            {
                const Window* pContainerWindow = VCLUnoHelper::GetWindow( xContainerWindow );
                ENSURE_OR_THROW( pContainerWindow, "no Window implementation for the frame's container window!" );

                m_pData->m_bIsTopLevelDocumentWindow = ( pContainerWindow->GetExtendedStyle() & WB_EXT_DOCUMENT ) != 0;
            }

            const Reference< XTopWindow > xFrameContainer( xContainerWindow, UNO_QUERY );
            if ( xFrameContainer.is() )
                (xFrameContainer.get()->*pListenerAction)( this );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    void FrameWindowActivationListener::impl_checkDisposed_throw() const
    {
        if ( !m_pData )
            throw DisposedException( OUString(), *const_cast< FrameWindowActivationListener* >( this ) );
    }

    void SAL_CALL FrameWindowActivationListener::windowOpened( const EventObject& /*_rEvent*/ ) throw (RuntimeException)
    {
        // not interested in
    }

    void SAL_CALL FrameWindowActivationListener::windowClosing( const EventObject& /*_rEvent*/ ) throw (RuntimeException)
    {
        // not interested in
    }

    void SAL_CALL FrameWindowActivationListener::windowClosed( const EventObject& /*_rEvent*/ ) throw (RuntimeException)
    {
        // not interested in
    }

    void SAL_CALL FrameWindowActivationListener::windowMinimized( const EventObject& /*_rEvent*/ ) throw (RuntimeException)
    {
        // not interested in
    }

    void SAL_CALL FrameWindowActivationListener::windowNormalized( const EventObject& /*_rEvent*/ ) throw (RuntimeException)
    {
        // not interested in
    }

    void SAL_CALL FrameWindowActivationListener::windowActivated( const EventObject& /*_rEvent*/ ) throw (RuntimeException)
    {
        impl_checkDisposed_throw();
        lcl_updateActive_nothrow( *m_pData, true );
    }

    void SAL_CALL FrameWindowActivationListener::windowDeactivated( const EventObject& /*_rEvent*/ ) throw (RuntimeException)
    {
        impl_checkDisposed_throw();
        lcl_updateActive_nothrow( *m_pData, false );
    }

    void SAL_CALL FrameWindowActivationListener::disposing( const EventObject& /*_rEvent*/ ) throw (RuntimeException)
    {
        dispose();
    }

    // ControllerFrame
    ControllerFrame::ControllerFrame( IController& _rController )
        :m_pData( new ControllerFrame_Data( _rController ) )
    {
    }

    ControllerFrame::~ControllerFrame()
    {
    }

    const Reference< XFrame >& ControllerFrame::attachFrame( const Reference< XFrame >& _rxFrame )
    {
        // set new frame, including listener handling
        lcl_setFrame_nothrow( *m_pData, _rxFrame );

        // determine whether we're active
        m_pData->m_bActive = lcl_isActive_nothrow( m_pData->m_xFrame );

        // update active component
        if ( m_pData->m_bActive )
        {
            lcl_updateActiveComponents_nothrow( *m_pData );
            lcl_notifyFocusChange_nothrow( *m_pData, true );
        }

        return m_pData->m_xFrame;
    }

    const Reference< XFrame >& ControllerFrame::getFrame() const
    {
        return m_pData->m_xFrame;
    }

    bool ControllerFrame::isActive() const
    {
        return m_pData->m_bActive;
    }

    void ControllerFrame::frameAction( FrameAction _eAction )
    {
        bool bActive = m_pData->m_bActive;

        switch ( _eAction )
        {
            case FrameAction_FRAME_ACTIVATED:
            case FrameAction_FRAME_UI_ACTIVATED:
                bActive = true;
                break;

            case FrameAction_FRAME_DEACTIVATING:
            case FrameAction_FRAME_UI_DEACTIVATING:
                bActive = false;
                break;

            default:
                break;
        }

        lcl_updateActive_nothrow( *m_pData, bActive );
    }

} // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
