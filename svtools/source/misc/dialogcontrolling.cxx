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

#include <svtools/dialogcontrolling.hxx>
#include <vcl/window.hxx>

#include <algorithm>
#include <functional>

//........................................................................
namespace svt
{
//........................................................................

    //=====================================================================
    //= IWindowOperator
    //=====================================================================
    //---------------------------------------------------------------------
    IWindowOperator::~IWindowOperator()
    {
    }

    //=====================================================================
    //= IWindowEventFilter
    //=====================================================================
    //---------------------------------------------------------------------
    IWindowEventFilter::~IWindowEventFilter()
    {
    }

    //=====================================================================
    //= DialogController_Data
    //=====================================================================
    struct DialogController_Data
    {
        Window&                     rInstigator;
        ::std::vector< Window* >    aConcernedWindows;
        PWindowEventFilter          pEventFilter;
        PWindowOperator             pOperator;

        DialogController_Data( Window& _rInstigator, const PWindowEventFilter _pEventFilter, const PWindowOperator _pOperator )
            :rInstigator( _rInstigator )
            ,pEventFilter( _pEventFilter )
            ,pOperator( _pOperator )
        {
        }
    };

    //=====================================================================
    //= DialogController
    //=====================================================================
    //---------------------------------------------------------------------
    DialogController::DialogController( Window& _rInstigator, const PWindowEventFilter& _pEventFilter,
            const PWindowOperator& _pOperator )
        :m_pImpl( new DialogController_Data( _rInstigator, _pEventFilter, _pOperator ) )
    {
        DBG_ASSERT( m_pImpl->pEventFilter.get() && m_pImpl->pOperator.get(),
            "DialogController::DialogController: invalid filter and/or operator!" );

        m_pImpl->rInstigator.AddEventListener( LINK( this, DialogController, OnWindowEvent ) );
    }

    //---------------------------------------------------------------------
    DialogController::~DialogController()
    {
        reset();
    }

    //---------------------------------------------------------------------
    void DialogController::reset()
    {
        m_pImpl->rInstigator.RemoveEventListener( LINK( this, DialogController, OnWindowEvent ) );
        m_pImpl->aConcernedWindows.clear();
        m_pImpl->pEventFilter.reset();
        m_pImpl->pOperator.reset();
    }

    //---------------------------------------------------------------------
    void DialogController::addDependentWindow( Window& _rWindow )
    {
        m_pImpl->aConcernedWindows.push_back( &_rWindow );

        VclWindowEvent aEvent( &_rWindow, 0, NULL );
        impl_update( aEvent, _rWindow );
    }

    //---------------------------------------------------------------------
    IMPL_LINK( DialogController, OnWindowEvent, const VclWindowEvent*, _pEvent )
    {
        if ( m_pImpl->pEventFilter->payAttentionTo( *_pEvent ) )
            impl_updateAll( *_pEvent );
        return 0L;
    }

    //---------------------------------------------------------------------
    void DialogController::impl_updateAll( const VclWindowEvent& _rTriggerEvent )
    {
        for ( ::std::vector< Window* >::iterator loop = m_pImpl->aConcernedWindows.begin();
                loop != m_pImpl->aConcernedWindows.end();
                ++loop
            )
            impl_update( _rTriggerEvent, *(*loop) );
    }

    //---------------------------------------------------------------------
    void DialogController::impl_update( const VclWindowEvent& _rTriggerEvent, Window& _rWindow )
    {
        m_pImpl->pOperator->operateOn( _rTriggerEvent, _rWindow );
    }

    //=====================================================================
    //= ControlDependencyManager_Data
    //=====================================================================
    struct ControlDependencyManager_Data
    {
        ::std::vector< PDialogController >  aControllers;
    };

    //=====================================================================
    //= ControlDependencyManager
    //=====================================================================
    //---------------------------------------------------------------------
    ControlDependencyManager::ControlDependencyManager()
        :m_pImpl( new ControlDependencyManager_Data )
    {
    }

    //---------------------------------------------------------------------
    ControlDependencyManager::~ControlDependencyManager()
    {
    }

    //---------------------------------------------------------------------
    namespace
    {
        struct ResetDialogController : public ::std::unary_function< const PDialogController&, void >
        {
            void operator()( const PDialogController& _pController )
            {
                _pController->reset();
            }
        };
    }

    //---------------------------------------------------------------------
    void ControlDependencyManager::clear()
    {
        ::std::for_each( m_pImpl->aControllers.begin(), m_pImpl->aControllers.end(), ResetDialogController() );
        m_pImpl->aControllers.clear();
    }

    //---------------------------------------------------------------------
    void ControlDependencyManager::addController( const PDialogController& _pController )
    {
        OSL_ENSURE( _pController.get() != NULL, "ControlDependencyManager::addController: invalid controller, this will crash, sooner or later!" );
        m_pImpl->aControllers.push_back( _pController );
    }

    //---------------------------------------------------------------------
    void ControlDependencyManager::enableOnRadioCheck( RadioButton& _rRadio, Window& _rDependentWindow )
    {
        PDialogController pController( new RadioDependentEnabler( _rRadio ) );
        pController->addDependentWindow( _rDependentWindow );
        m_pImpl->aControllers.push_back( pController );
    }

    //---------------------------------------------------------------------
    void ControlDependencyManager::enableOnRadioCheck( RadioButton& _rRadio, Window& _rDependentWindow1, Window& _rDependentWindow2 )
    {
        PDialogController pController( new RadioDependentEnabler( _rRadio ) );
        pController->addDependentWindow( _rDependentWindow1 );
        pController->addDependentWindow( _rDependentWindow2 );
        m_pImpl->aControllers.push_back( pController );
    }

    //---------------------------------------------------------------------
    void ControlDependencyManager::enableOnRadioCheck( RadioButton& _rRadio, Window& _rDependentWindow1, Window& _rDependentWindow2, Window& _rDependentWindow3 )
    {
        PDialogController pController( new RadioDependentEnabler( _rRadio ) );
        pController->addDependentWindow( _rDependentWindow1 );
        pController->addDependentWindow( _rDependentWindow2 );
        pController->addDependentWindow( _rDependentWindow3 );
        m_pImpl->aControllers.push_back( pController );
    }

    //---------------------------------------------------------------------
    void ControlDependencyManager::enableOnRadioCheck( RadioButton& _rRadio, Window& _rDependentWindow1, Window& _rDependentWindow2, Window& _rDependentWindow3, Window& _rDependentWindow4, Window& _rDependentWindow5 )
    {
        PDialogController pController( new RadioDependentEnabler( _rRadio ) );
        pController->addDependentWindow( _rDependentWindow1 );
        pController->addDependentWindow( _rDependentWindow2 );
        pController->addDependentWindow( _rDependentWindow3 );
        pController->addDependentWindow( _rDependentWindow4 );
        pController->addDependentWindow( _rDependentWindow5 );
        m_pImpl->aControllers.push_back( pController );
    }

    //---------------------------------------------------------------------
    void ControlDependencyManager::enableOnCheckMark( CheckBox& _rBox, Window& _rDependentWindow )
    {
        PDialogController pController( new RadioDependentEnabler( _rBox ) );
        pController->addDependentWindow( _rDependentWindow );
        m_pImpl->aControllers.push_back( pController );
    }

    //---------------------------------------------------------------------
    void ControlDependencyManager::enableOnCheckMark( CheckBox& _rBox, Window& _rDependentWindow1, Window& _rDependentWindow2 )
    {
        PDialogController pController( new RadioDependentEnabler( _rBox ) );
        pController->addDependentWindow( _rDependentWindow1 );
        pController->addDependentWindow( _rDependentWindow2 );
        m_pImpl->aControllers.push_back( pController );
    }

    //---------------------------------------------------------------------
    void ControlDependencyManager::enableOnCheckMark( CheckBox& _rBox, Window& _rDependentWindow1, Window& _rDependentWindow2, Window& _rDependentWindow3, Window& _rDependentWindow4 )
    {
        PDialogController pController( new RadioDependentEnabler( _rBox ) );
        pController->addDependentWindow( _rDependentWindow1 );
        pController->addDependentWindow( _rDependentWindow2 );
        pController->addDependentWindow( _rDependentWindow3 );
        pController->addDependentWindow( _rDependentWindow4 );
        m_pImpl->aControllers.push_back( pController );
    }

//........................................................................
} // namespace svt
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
