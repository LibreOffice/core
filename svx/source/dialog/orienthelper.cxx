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

#include "svx/orienthelper.hxx"

#include <vector>
#include <utility>

#include <vcl/button.hxx>
#include "svx/dialcontrol.hxx"

namespace svx {



struct OrientationHelper_Impl
{
    typedef std::pair< VclPtr<vcl::Window>, TriState >  WindowPair;
    typedef std::vector< WindowPair >       WindowVec;

    DialControl&        mrCtrlDial;
    CheckBox&           mrCbStacked;
    WindowVec           maWinVec;
    bool                mbEnabled;
    bool                mbVisible;

    explicit            OrientationHelper_Impl( DialControl& rCtrlDial, CheckBox& rCbStacked );

    void                AddDependentWindow( vcl::Window& rWindow, TriState eDisableIfStacked );

    void                EnableDependentWindows();
    void                EnableWindow( vcl::Window& rWindow, TriState eDisableIfStacked );

    void                ShowDependentWindows();

    DECL_LINK( ClickHdl, void* );
};



OrientationHelper_Impl::OrientationHelper_Impl( DialControl& rCtrlDial, CheckBox& rCbStacked ) :
    mrCtrlDial( rCtrlDial ),
    mrCbStacked( rCbStacked ),
    mbEnabled( rCtrlDial.IsEnabled() ),
    mbVisible( rCtrlDial.IsVisible() )
{
    maWinVec.push_back( WindowPair( &mrCtrlDial, TRISTATE_TRUE ) );
    maWinVec.push_back( WindowPair( &mrCbStacked, TRISTATE_INDET ) );
    mrCbStacked.SetClickHdl( LINK( this, OrientationHelper_Impl, ClickHdl ) );
}

void OrientationHelper_Impl::AddDependentWindow( vcl::Window& rWindow, TriState eDisableIfStacked )
{
    maWinVec.push_back( std::make_pair( &rWindow, eDisableIfStacked ) );
    EnableWindow( rWindow, eDisableIfStacked );
}

void OrientationHelper_Impl::EnableDependentWindows()
{
    for( WindowVec::iterator aIt = maWinVec.begin(), aEnd = maWinVec.end(); aIt != aEnd; ++aIt )
        EnableWindow( *aIt->first, aIt->second );
}

void OrientationHelper_Impl::EnableWindow( vcl::Window& rWindow, TriState eDisableIfStacked )
{
    bool bDisableOnStacked = false;
    switch( eDisableIfStacked )
    {
        // TRISTATE_TRUE: Disable window, if stacked text is turned on or "don't know".
        case TRISTATE_TRUE:   bDisableOnStacked = (mrCbStacked.GetState() != TRISTATE_FALSE);  break;
        // TRISTATE_FALSE: Disable window, if stacked text is turned off or "don't know".
        case TRISTATE_FALSE: bDisableOnStacked = (mrCbStacked.GetState() != TRISTATE_TRUE);    break;
        default: ;//prevent warning
    }
    rWindow.Enable( mbEnabled && !bDisableOnStacked );
}

void OrientationHelper_Impl::ShowDependentWindows()
{
    for( WindowVec::iterator aIt = maWinVec.begin(), aEnd = maWinVec.end(); aIt != aEnd; ++aIt )
        aIt->first->Show( mbVisible );
}

IMPL_LINK_NOARG(OrientationHelper_Impl, ClickHdl)
{
    EnableDependentWindows();
    return 0L;
}



OrientationHelper::OrientationHelper( DialControl& rCtrlDial, NumericField& rNfRotation, CheckBox& rCbStacked ) :
    mpImpl( new OrientationHelper_Impl( rCtrlDial, rCbStacked ) )
{
    rCtrlDial.SetLinkedField( &rNfRotation );
    mpImpl->EnableDependentWindows();
    mpImpl->ShowDependentWindows();
}

OrientationHelper::~OrientationHelper()
{
}

void OrientationHelper::AddDependentWindow( vcl::Window& rWindow, TriState eDisableIfStacked )
{
    mpImpl->AddDependentWindow( rWindow, eDisableIfStacked );
}

void OrientationHelper::Enable( bool bEnable )
{
    mpImpl->mbEnabled = bEnable;
    mpImpl->EnableDependentWindows();
}

void OrientationHelper::Show( bool bShow )
{
    mpImpl->mbVisible = bShow;
    mpImpl->ShowDependentWindows();
}

void OrientationHelper::SetStackedState( TriState eState )
{
    if( eState != GetStackedState() )
    {
        mpImpl->mrCbStacked.SetState( eState );
        mpImpl->EnableDependentWindows();
    }
}

TriState OrientationHelper::GetStackedState() const
{
    return mpImpl->mrCbStacked.GetState();
}

void OrientationHelper::EnableStackedTriState( bool bEnable )
{
    mpImpl->mrCbStacked.EnableTriState( bEnable );
}



OrientStackedWrapper::OrientStackedWrapper( OrientationHelper& rOrientHlp ) :
    SingleControlWrapperType( rOrientHlp )
{
}

bool OrientStackedWrapper::IsControlDontKnow() const
{
    return GetControl().GetStackedState() == TRISTATE_INDET;
}

void OrientStackedWrapper::SetControlDontKnow( bool bSet )
{
    GetControl().EnableStackedTriState( bSet );
    GetControl().SetStackedState( bSet ? TRISTATE_INDET : TRISTATE_FALSE );
}

bool OrientStackedWrapper::GetControlValue() const
{
    return GetControl().GetStackedState() == TRISTATE_TRUE;
}

void OrientStackedWrapper::SetControlValue( bool bValue )
{
    GetControl().SetStackedState( bValue ? TRISTATE_TRUE : TRISTATE_FALSE );
}



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
