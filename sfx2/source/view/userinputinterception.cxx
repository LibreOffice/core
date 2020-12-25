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


#include <sfx2/userinputinterception.hxx>

#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/awt/MouseEvent.hpp>
#include <com/sun/star/awt/InputEvent.hpp>
#include <com/sun/star/awt/KeyEvent.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/awt/XKeyHandler.hpp>
#include <com/sun/star/awt/XMouseClickHandler.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/uno/XInterface.hpp>

#include <comphelper/interfacecontainer2.hxx>
#include <cppuhelper/weak.hxx>
#include <vcl/event.hxx>
#include <vcl/window.hxx>
#include <osl/diagnose.h>


namespace sfx2
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::awt::MouseEvent;
    using ::com::sun::star::awt::KeyEvent;
    using ::com::sun::star::awt::InputEvent;
    using ::com::sun::star::awt::XKeyHandler;
    using ::com::sun::star::awt::XMouseClickHandler;
    using ::com::sun::star::lang::DisposedException;

    namespace MouseButton = ::com::sun::star::awt::MouseButton;
    namespace KeyModifier = ::com::sun::star::awt::KeyModifier;

    struct UserInputInterception_Data
    {
    public:
        ::cppu::OWeakObject&                m_rControllerImpl;
        ::comphelper::OInterfaceContainerHelper2   m_aKeyHandlers;
        ::comphelper::OInterfaceContainerHelper2   m_aMouseClickHandlers;

    public:
        UserInputInterception_Data( ::cppu::OWeakObject& _rControllerImpl, ::osl::Mutex& _rMutex )
            :m_rControllerImpl( _rControllerImpl )
            ,m_aKeyHandlers( _rMutex )
            ,m_aMouseClickHandlers( _rMutex )
        {
        }
    };

    namespace
    {
        template< class VCLEVENT >
        void lcl_initModifiers( InputEvent& _rEvent, const VCLEVENT& _rVclEvent )
        {
            _rEvent.Modifiers = 0;

            if ( _rVclEvent.IsShift() )
                _rEvent.Modifiers |= KeyModifier::SHIFT;
            if ( _rVclEvent.IsMod1() )
                _rEvent.Modifiers |= KeyModifier::MOD1;
            if ( _rVclEvent.IsMod2() )
                _rEvent.Modifiers |= KeyModifier::MOD2;
            if ( _rVclEvent.IsMod3() )
                _rEvent.Modifiers |= KeyModifier::MOD3;
        }

        void lcl_initKeyEvent( KeyEvent& rEvent, const ::KeyEvent& rEvt )
        {
            lcl_initModifiers( rEvent, rEvt.GetKeyCode() );

            rEvent.KeyCode = rEvt.GetKeyCode().GetCode();
            rEvent.KeyChar = rEvt.GetCharCode();
            rEvent.KeyFunc = sal::static_int_cast< sal_Int16 >( rEvt.GetKeyCode().GetFunction());
        }

        void lcl_initMouseEvent( MouseEvent& rEvent, const ::MouseEvent& rEvt )
        {
            lcl_initModifiers( rEvent, rEvt );

            rEvent.Buttons = 0;
            if ( rEvt.IsLeft() )
                rEvent.Buttons |= MouseButton::LEFT;
            if ( rEvt.IsRight() )
                rEvent.Buttons |= MouseButton::RIGHT;
            if ( rEvt.IsMiddle() )
                rEvent.Buttons |= MouseButton::MIDDLE;

            rEvent.X = rEvt.GetPosPixel().X();
            rEvent.Y = rEvt.GetPosPixel().Y();
            rEvent.ClickCount = rEvt.GetClicks();
            rEvent.PopupTrigger = false;
        }

    }


    //= UserInputInterception


    UserInputInterception::UserInputInterception( ::cppu::OWeakObject& _rControllerImpl, ::osl::Mutex& _rMutex )
        :m_pData( new UserInputInterception_Data( _rControllerImpl, _rMutex ) )
    {
    }


    UserInputInterception::~UserInputInterception()
    {
    }


    void UserInputInterception::addKeyHandler( const Reference< XKeyHandler >& _rxHandler )
    {
        if ( _rxHandler.is() )
            m_pData->m_aKeyHandlers.addInterface( _rxHandler );
    }


    void UserInputInterception::removeKeyHandler( const Reference< XKeyHandler >& _rxHandler )
    {
        m_pData->m_aKeyHandlers.removeInterface( _rxHandler );
    }


    void UserInputInterception::addMouseClickHandler( const Reference< XMouseClickHandler >& _rxHandler )
    {
        if ( _rxHandler.is() )
            m_pData->m_aMouseClickHandlers.addInterface( _rxHandler );
    }


    void UserInputInterception::removeMouseClickHandler( const Reference< XMouseClickHandler >& _rxHandler )
    {
        m_pData->m_aMouseClickHandlers.removeInterface( _rxHandler );
    }


    bool UserInputInterception::hasKeyHandlers() const
    {
        return m_pData->m_aKeyHandlers.getLength() > 0;
    }


    bool UserInputInterception::hasMouseClickListeners() const
    {
        return m_pData->m_aMouseClickHandlers.getLength() > 0;
    }


    bool UserInputInterception::handleNotifyEvent( const NotifyEvent& _rEvent )
    {
        Reference < XInterface > xHoldAlive( m_pData->m_rControllerImpl );

        MouseNotifyEvent nType = _rEvent.GetType();
        bool bHandled = false;

        switch ( nType )
        {
            case MouseNotifyEvent::KEYINPUT:
            case MouseNotifyEvent::KEYUP:
            {
                KeyEvent aEvent;
                lcl_initKeyEvent( aEvent, *_rEvent.GetKeyEvent() );
                if ( _rEvent.GetWindow() )
                    aEvent.Source = _rEvent.GetWindow()->GetComponentInterface();

                ::comphelper::OInterfaceIteratorHelper2 aIterator( m_pData->m_aKeyHandlers );
                while ( aIterator.hasMoreElements() )
                {
                    Reference< XKeyHandler > xHandler( static_cast< XKeyHandler* >( aIterator.next() ) );
                    if ( !xHandler.is() )
                        continue;

                    try
                    {
                        if ( nType == MouseNotifyEvent::KEYINPUT )
                            bHandled = xHandler->keyPressed( aEvent );
                        else
                            bHandled = xHandler->keyReleased( aEvent );
                    }
                    catch( const DisposedException& e )
                    {
                        if ( e.Context == xHandler )
                            aIterator.remove();
                    }
                    catch( const RuntimeException& )
                    {
                        throw;
                    }
                    catch( const Exception& )
                    {
                    }
                }
            }
            break;

            case MouseNotifyEvent::MOUSEBUTTONDOWN:
            case MouseNotifyEvent::MOUSEBUTTONUP:
            {
                MouseEvent aEvent;
                lcl_initMouseEvent( aEvent, *_rEvent.GetMouseEvent() );
                if ( _rEvent.GetWindow() )
                    aEvent.Source = _rEvent.GetWindow()->GetComponentInterface();

                ::comphelper::OInterfaceIteratorHelper2 aIterator( m_pData->m_aMouseClickHandlers );
                while ( aIterator.hasMoreElements() )
                {
                    Reference< XMouseClickHandler > xHandler( static_cast< XMouseClickHandler* >( aIterator.next() ) );
                    if ( !xHandler.is() )
                        continue;

                    try
                    {
                        if ( nType == MouseNotifyEvent::MOUSEBUTTONDOWN )
                            bHandled = xHandler->mousePressed( aEvent );
                        else
                            bHandled = xHandler->mouseReleased( aEvent );
                    }
                    catch( const DisposedException& e )
                    {
                        if ( e.Context == xHandler )
                            aIterator.remove();
                    }
                    catch( const RuntimeException& )
                    {
                        throw;
                    }
                    catch( const Exception& )
                    {
                    }
                }
            }
            break;

            default:
                OSL_FAIL( "UserInputInterception::handleNotifyEvent: illegal event type!" );
                break;
        }

        return bHandled;
    }


} // namespace sfx2


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
