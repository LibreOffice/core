/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: orienthelper.cxx,v $
 * $Revision: 1.6 $
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
#include "precompiled_svx.hxx"
#include "orienthelper.hxx"

#include <vector>
#include <utility>

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include "dialcontrol.hxx"

namespace svx {

// ============================================================================

struct OrientationHelper_Impl
{
    typedef std::pair< Window*, TriState >  WindowPair;
    typedef std::vector< WindowPair >       WindowVec;

    OrientationHelper&  mrParent;
    WindowVec           maWinVec;
    svx::DialControl&   mrCtrlDial;
    CheckBox&           mrCbStacked;

    explicit            OrientationHelper_Impl(
                            OrientationHelper& rParent,
                            svx::DialControl& rCtrlDial,
                            CheckBox& rCbStacked );

    void                AddDependentWindow( Window& rWindow, TriState eDisableIfStacked );

    void                EnableDependentWindows();
    void                EnableWindow( Window& rWindow, TriState eDisableIfStacked );

    void                ShowDependentWindows();

    DECL_LINK( ClickHdl, void* );
};

// ----------------------------------------------------------------------------

OrientationHelper_Impl::OrientationHelper_Impl(
        OrientationHelper& rParent, svx::DialControl& rCtrlDial, CheckBox& rCbStacked ) :
    mrParent( rParent ),
    mrCtrlDial( rCtrlDial ),
    mrCbStacked( rCbStacked )
{
    maWinVec.push_back( WindowPair( &mrCtrlDial, STATE_CHECK ) );
    maWinVec.push_back( WindowPair( &mrCbStacked, STATE_DONTKNOW ) );
    mrCbStacked.SetClickHdl( LINK( this, OrientationHelper_Impl, ClickHdl ) );
}

void OrientationHelper_Impl::AddDependentWindow( Window& rWindow, TriState eDisableIfStacked )
{
    maWinVec.push_back( std::make_pair( &rWindow, eDisableIfStacked ) );
    EnableWindow( rWindow, eDisableIfStacked );
}

void OrientationHelper_Impl::EnableDependentWindows()
{
    for( WindowVec::iterator aIt = maWinVec.begin(), aEnd = maWinVec.end(); aIt != aEnd; ++aIt )
        EnableWindow( *aIt->first, aIt->second );
}

void OrientationHelper_Impl::EnableWindow( Window& rWindow, TriState eDisableIfStacked )
{
    bool bEnabled = mrParent.IsEnabled();
    bool bDisableOnStacked = false;
    switch( eDisableIfStacked )
    {
        // STATE_CHECK: Disable window, if stacked text is turned on or "don't know".
        case STATE_CHECK:   bDisableOnStacked = (mrCbStacked.GetState() != STATE_NOCHECK);  break;
        // STATE_NOCHECK: Disable window, if stacked text is turned off or "don't know".
        case STATE_NOCHECK: bDisableOnStacked = (mrCbStacked.GetState() != STATE_CHECK);    break;
        default: ;//prevent warning
    }
    rWindow.Enable( bEnabled && !bDisableOnStacked );
}

void OrientationHelper_Impl::ShowDependentWindows()
{
    bool bVisible = mrParent.IsVisible();
    for( WindowVec::iterator aIt = maWinVec.begin(), aEnd = maWinVec.end(); aIt != aEnd; ++aIt )
        aIt->first->Show( bVisible );
}

IMPL_LINK( OrientationHelper_Impl, ClickHdl, void*, EMPTYARG )
{
    EnableDependentWindows();
    return 0L;
}

// ============================================================================

OrientationHelper::OrientationHelper(
        Window* pParent, svx::DialControl& rCtrlDial, CheckBox& rCbStacked ) :
    Window( pParent ),
    mpImpl( new OrientationHelper_Impl( *this, rCtrlDial, rCbStacked ) )
{
    mpImpl->EnableDependentWindows();
}

OrientationHelper::OrientationHelper(
        Window* pParent,
        svx::DialControl& rCtrlDial, NumericField& rNfRotation, CheckBox& rCbStacked ) :
    Window( pParent ),
    mpImpl( new OrientationHelper_Impl( *this, rCtrlDial, rCbStacked ) )
{
    mpImpl->mrCtrlDial.SetLinkedField( &rNfRotation );
    mpImpl->EnableDependentWindows();
}

OrientationHelper::~OrientationHelper()
{
}

void OrientationHelper::StateChanged( StateChangedType nStateChange )
{
    if( nStateChange == STATE_CHANGE_ENABLE )
        mpImpl->EnableDependentWindows();
    if( nStateChange == STATE_CHANGE_VISIBLE )
        mpImpl->ShowDependentWindows();
    Window::StateChanged( nStateChange );
}

void OrientationHelper::AddDependentWindow( Window& rWindow, TriState eDisableIfStacked )
{
    mpImpl->AddDependentWindow( rWindow, eDisableIfStacked );
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

// ============================================================================

OrientStackedWrapper::OrientStackedWrapper( OrientationHelper& rOrientHlp ) :
    SingleControlWrapperType( rOrientHlp )
{
}

bool OrientStackedWrapper::IsControlDontKnow() const
{
    return GetControl().GetStackedState() == STATE_DONTKNOW;
}

void OrientStackedWrapper::SetControlDontKnow( bool bSet )
{
    GetControl().EnableStackedTriState( bSet );
    GetControl().SetStackedState( bSet ? STATE_DONTKNOW : STATE_NOCHECK );
}

bool OrientStackedWrapper::GetControlValue() const
{
    return GetControl().GetStackedState() == STATE_CHECK;
}

void OrientStackedWrapper::SetControlValue( bool bValue )
{
    GetControl().SetStackedState( bValue ? STATE_CHECK : STATE_NOCHECK );
}

// ============================================================================

} // namespace svx

