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
#include "precompiled_svtools.hxx"
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
    void ControlDependencyManager::enableOnRadioCheck( RadioButton& _rRadio, Window& _rDependentWindow1, Window& _rDependentWindow2, Window& _rDependentWindow3, Window& _rDependentWindow4 )
    {
        PDialogController pController( new RadioDependentEnabler( _rRadio ) );
        pController->addDependentWindow( _rDependentWindow1 );
        pController->addDependentWindow( _rDependentWindow2 );
        pController->addDependentWindow( _rDependentWindow3 );
        pController->addDependentWindow( _rDependentWindow4 );
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
    void ControlDependencyManager::enableOnRadioCheck( RadioButton& _rRadio, Window& _rDependentWindow1, Window& _rDependentWindow2, Window& _rDependentWindow3, Window& _rDependentWindow4, Window& _rDependentWindow5, Window& _rDependentWindow6 )
    {
        PDialogController pController( new RadioDependentEnabler( _rRadio ) );
        pController->addDependentWindow( _rDependentWindow1 );
        pController->addDependentWindow( _rDependentWindow2 );
        pController->addDependentWindow( _rDependentWindow3 );
        pController->addDependentWindow( _rDependentWindow4 );
        pController->addDependentWindow( _rDependentWindow5 );
        pController->addDependentWindow( _rDependentWindow6 );
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
    void ControlDependencyManager::enableOnCheckMark( CheckBox& _rBox, Window& _rDependentWindow1, Window& _rDependentWindow2, Window& _rDependentWindow3 )
    {
        PDialogController pController( new RadioDependentEnabler( _rBox ) );
        pController->addDependentWindow( _rDependentWindow1 );
        pController->addDependentWindow( _rDependentWindow2 );
        pController->addDependentWindow( _rDependentWindow3 );
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

    //---------------------------------------------------------------------
    void ControlDependencyManager::enableOnCheckMark( CheckBox& _rBox, Window& _rDependentWindow1, Window& _rDependentWindow2, Window& _rDependentWindow3, Window& _rDependentWindow4, Window& _rDependentWindow5 )
    {
        PDialogController pController( new RadioDependentEnabler( _rBox ) );
        pController->addDependentWindow( _rDependentWindow1 );
        pController->addDependentWindow( _rDependentWindow2 );
        pController->addDependentWindow( _rDependentWindow3 );
        pController->addDependentWindow( _rDependentWindow4 );
        pController->addDependentWindow( _rDependentWindow5 );
        m_pImpl->aControllers.push_back( pController );
    }

    //---------------------------------------------------------------------
    void ControlDependencyManager::enableOnCheckMark( CheckBox& _rBox, Window& _rDependentWindow1, Window& _rDependentWindow2, Window& _rDependentWindow3, Window& _rDependentWindow4, Window& _rDependentWindow5, Window& _rDependentWindow6 )
    {
        PDialogController pController( new RadioDependentEnabler( _rBox ) );
        pController->addDependentWindow( _rDependentWindow1 );
        pController->addDependentWindow( _rDependentWindow2 );
        pController->addDependentWindow( _rDependentWindow3 );
        pController->addDependentWindow( _rDependentWindow4 );
        pController->addDependentWindow( _rDependentWindow5 );
        pController->addDependentWindow( _rDependentWindow6 );
        m_pImpl->aControllers.push_back( pController );
    }

//........................................................................
} // namespace svt
//........................................................................

