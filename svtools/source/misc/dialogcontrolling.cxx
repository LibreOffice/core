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
#include <osl/diagnose.h>
#include <tools/debug.hxx>

#include <algorithm>


namespace svt
{


    //= IWindowOperator


    IWindowOperator::~IWindowOperator()
    {
    }


    //= IWindowEventFilter


    IWindowEventFilter::~IWindowEventFilter()
    {
    }


    //= DialogController_Data

    struct DialogController_Data
    {
        VclPtr<vcl::Window>                  xInstigator;
        ::std::vector< VclPtr<vcl::Window> > aConcernedWindows;
        PWindowEventFilter          pEventFilter;
        PWindowOperator             pOperator;

        DialogController_Data( vcl::Window& _xInstigator, const PWindowEventFilter& _pEventFilter, const PWindowOperator& _pOperator )
            :xInstigator( &_xInstigator )
            ,pEventFilter( _pEventFilter )
            ,pOperator( _pOperator )
        {
        }
    };


    //= DialogController


    DialogController::DialogController( vcl::Window& _xInstigator, const PWindowEventFilter& _pEventFilter,
            const PWindowOperator& _pOperator )
        :m_pImpl( new DialogController_Data( _xInstigator, _pEventFilter, _pOperator ) )
    {
        DBG_ASSERT( m_pImpl->pEventFilter.get() && m_pImpl->pOperator.get(),
            "DialogController::DialogController: invalid filter and/or operator!" );

        m_pImpl->xInstigator->AddEventListener( LINK( this, DialogController, OnWindowEvent ) );
    }


    DialogController::~DialogController()
    {
        reset();
    }


    void DialogController::reset()
    {
        if (m_pImpl->xInstigator)
            m_pImpl->xInstigator->RemoveEventListener( LINK( this, DialogController, OnWindowEvent ) );
        m_pImpl->xInstigator.clear();
        m_pImpl->aConcernedWindows.clear();
        m_pImpl->pEventFilter.reset();
        m_pImpl->pOperator.reset();
    }


    void DialogController::addDependentWindow( vcl::Window& _rWindow )
    {
        m_pImpl->aConcernedWindows.emplace_back(&_rWindow );

        VclWindowEvent aEvent( &_rWindow, VclEventId::NONE, nullptr );
        impl_update( aEvent, _rWindow );
    }


    IMPL_LINK( DialogController, OnWindowEvent, VclWindowEvent&, _rEvent, void )
    {
        if ( !m_pImpl->pEventFilter->payAttentionTo( _rEvent ) )
            return;
        for (auto const& concernedWindow : m_pImpl->aConcernedWindows)
            impl_update(_rEvent, *concernedWindow);
    }


    void DialogController::impl_update( const VclWindowEvent& _rTriggerEvent, vcl::Window& _rWindow )
    {
        m_pImpl->pOperator->operateOn( _rTriggerEvent, _rWindow );
    }


    //= ControlDependencyManager_Data

    struct ControlDependencyManager_Data
    {
        ::std::vector< std::shared_ptr<DialogController> >  aControllers;
    };


    //= ControlDependencyManager


    ControlDependencyManager::ControlDependencyManager()
        :m_pImpl( new ControlDependencyManager_Data )
    {
    }


    ControlDependencyManager::~ControlDependencyManager()
    {
    }


    void ControlDependencyManager::addController( const std::shared_ptr<DialogController>& _pController )
    {
        OSL_ENSURE(_pController != nullptr, "ControlDependencyManager::addController: invalid "
                                            "controller, this will crash, sooner or later!");
        m_pImpl->aControllers.push_back( _pController );
    }


    void ControlDependencyManager::enableOnRadioCheck( RadioButton& _rRadio, vcl::Window& _rDependentWindow )
    {
        std::shared_ptr<DialogController> pController( new RadioDependentEnabler( _rRadio ) );
        pController->addDependentWindow( _rDependentWindow );
        m_pImpl->aControllers.push_back( pController );
    }


    void ControlDependencyManager::enableOnRadioCheck( RadioButton& _rRadio, vcl::Window& _rDependentWindow1, vcl::Window& _rDependentWindow2 )
    {
        std::shared_ptr<DialogController> pController( new RadioDependentEnabler( _rRadio ) );
        pController->addDependentWindow( _rDependentWindow1 );
        pController->addDependentWindow( _rDependentWindow2 );
        m_pImpl->aControllers.push_back( pController );
    }


    void ControlDependencyManager::enableOnRadioCheck( RadioButton& _rRadio, vcl::Window& _rDependentWindow1, vcl::Window& _rDependentWindow2, vcl::Window& _rDependentWindow3 )
    {
        std::shared_ptr<DialogController> pController( new RadioDependentEnabler( _rRadio ) );
        pController->addDependentWindow( _rDependentWindow1 );
        pController->addDependentWindow( _rDependentWindow2 );
        pController->addDependentWindow( _rDependentWindow3 );
        m_pImpl->aControllers.push_back( pController );
    }


    void ControlDependencyManager::enableOnRadioCheck( RadioButton& _rRadio, vcl::Window& _rDependentWindow1, vcl::Window& _rDependentWindow2, vcl::Window& _rDependentWindow3, vcl::Window& _rDependentWindow4, vcl::Window& _rDependentWindow5 )
    {
        std::shared_ptr<DialogController> pController( new RadioDependentEnabler( _rRadio ) );
        pController->addDependentWindow( _rDependentWindow1 );
        pController->addDependentWindow( _rDependentWindow2 );
        pController->addDependentWindow( _rDependentWindow3 );
        pController->addDependentWindow( _rDependentWindow4 );
        pController->addDependentWindow( _rDependentWindow5 );
        m_pImpl->aControllers.push_back( pController );
    }

} // namespace svt


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
