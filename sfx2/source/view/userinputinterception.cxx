/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
 ***********************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#include "sfx2/userinputinterception.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
/** === end UNO includes === **/

#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/weak.hxx>
#include <vcl/event.hxx>
#include <vcl/window.hxx>

//........................................................................
namespace sfx2
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::awt::MouseEvent;
    using ::com::sun::star::awt::KeyEvent;
    using ::com::sun::star::awt::InputEvent;
    using ::com::sun::star::awt::XKeyHandler;
    using ::com::sun::star::awt::XMouseClickHandler;
    using ::com::sun::star::lang::DisposedException;
    /** === end UNO using === **/
    namespace MouseButton = ::com::sun::star::awt::MouseButton;
    namespace KeyModifier = ::com::sun::star::awt::KeyModifier;

    struct UserInputInterception_Data
    {
    public:
        ::cppu::OWeakObject&                m_rControllerImpl;
        ::cppu::OInterfaceContainerHelper   m_aKeyHandlers;
        ::cppu::OInterfaceContainerHelper   m_aMouseClickHandlers;

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
        template< class VLCEVENT >
        void lcl_initModifiers( InputEvent& _rEvent, const VLCEVENT& _rVclEvent )
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
            rEvent.PopupTrigger = sal_False;
        }

    }

    //====================================================================
    //= UserInputInterception
    //====================================================================
    //--------------------------------------------------------------------
    UserInputInterception::UserInputInterception( ::cppu::OWeakObject& _rControllerImpl, ::osl::Mutex& _rMutex )
        :m_pData( new UserInputInterception_Data( _rControllerImpl, _rMutex ) )
    {
    }

    //--------------------------------------------------------------------
    UserInputInterception::~UserInputInterception()
    {
    }

    //--------------------------------------------------------------------
    void UserInputInterception::addKeyHandler( const Reference< XKeyHandler >& _rxHandler ) throw (RuntimeException)
    {
        if ( _rxHandler.is() )
            m_pData->m_aKeyHandlers.addInterface( _rxHandler );
    }

    //--------------------------------------------------------------------
    void UserInputInterception::removeKeyHandler( const Reference< XKeyHandler >& _rxHandler ) throw (RuntimeException)
    {
        m_pData->m_aKeyHandlers.removeInterface( _rxHandler );
    }

    //--------------------------------------------------------------------
    void UserInputInterception::addMouseClickHandler( const Reference< XMouseClickHandler >& _rxHandler ) throw (RuntimeException)
    {
        if ( _rxHandler.is() )
            m_pData->m_aMouseClickHandlers.addInterface( _rxHandler );
    }

    //--------------------------------------------------------------------
    void UserInputInterception::removeMouseClickHandler( const Reference< XMouseClickHandler >& _rxHandler ) throw (RuntimeException)
    {
        m_pData->m_aMouseClickHandlers.removeInterface( _rxHandler );
    }

    //--------------------------------------------------------------------
    bool UserInputInterception::hasKeyHandlers() const
    {
        return m_pData->m_aKeyHandlers.getLength() > 0;
    }

    //--------------------------------------------------------------------
    bool UserInputInterception::hasMouseClickListeners() const
    {
        return m_pData->m_aMouseClickHandlers.getLength() > 0;
    }

    //--------------------------------------------------------------------
    bool UserInputInterception::handleNotifyEvent( const NotifyEvent& _rEvent )
    {
        Reference < XInterface > xHoldAlive( m_pData->m_rControllerImpl );

        USHORT nType = _rEvent.GetType();
        bool bHandled = false;

        switch ( nType )
        {
            case EVENT_KEYINPUT:
            case EVENT_KEYUP:
            {
                KeyEvent aEvent;
                lcl_initKeyEvent( aEvent, *_rEvent.GetKeyEvent() );
                if ( _rEvent.GetWindow() )
                    aEvent.Source = _rEvent.GetWindow()->GetComponentInterface();

                ::cppu::OInterfaceIteratorHelper aIterator( m_pData->m_aKeyHandlers );
                while ( aIterator.hasMoreElements() )
                {
                    Reference< XKeyHandler > xHandler( static_cast< XKeyHandler* >( aIterator.next() ) );
                    if ( !xHandler.is() )
                        continue;

                    try
                    {
                        if ( nType == EVENT_KEYINPUT )
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

            case EVENT_MOUSEBUTTONDOWN:
            case EVENT_MOUSEBUTTONUP:
            {
                MouseEvent aEvent;
                lcl_initMouseEvent( aEvent, *_rEvent.GetMouseEvent() );
                if ( _rEvent.GetWindow() )
                    aEvent.Source = _rEvent.GetWindow()->GetComponentInterface();

                ::cppu::OInterfaceIteratorHelper aIterator( m_pData->m_aMouseClickHandlers );
                while ( aIterator.hasMoreElements() )
                {
                    Reference< XMouseClickHandler > xHandler( static_cast< XMouseClickHandler* >( aIterator.next() ) );
                    if ( !xHandler.is() )
                        continue;

                    try
                    {
                        if ( nType == EVENT_MOUSEBUTTONDOWN )
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

//........................................................................
} // namespace sfx2
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
