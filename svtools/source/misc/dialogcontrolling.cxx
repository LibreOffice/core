/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dialogcontrolling.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 16:28:56 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#ifndef SVTOOLS_DIALOGCONTROLLING_HXX
#include "dialogcontrolling.hxx"
#endif

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif

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
    DialogController::DialogController( Window& _rInstigator, const PWindowEventFilter _pEventFilter,
            const PWindowOperator _pOperator )
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
        impl_update( _rWindow );
    }

    //---------------------------------------------------------------------
    IMPL_LINK( DialogController, OnWindowEvent, const VclSimpleEvent*, _pEvent )
    {
        if ( m_pImpl->pEventFilter->payAttentionTo( *_pEvent ) )
            impl_updateAll();
        return 0L;
    }

    //---------------------------------------------------------------------
    void DialogController::impl_updateAll()
    {
        for ( ::std::vector< Window* >::iterator loop = m_pImpl->aConcernedWindows.begin();
                loop != m_pImpl->aConcernedWindows.end();
                ++loop
            )
            impl_update( *(*loop) );
    }

    //---------------------------------------------------------------------
    void DialogController::impl_update( Window& _rWindow )
    {
        m_pImpl->pOperator->operateOn( _rWindow );
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

