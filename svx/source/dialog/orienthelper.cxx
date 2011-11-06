/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include "svx/orienthelper.hxx"

#include <vector>
#include <utility>

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include "svx/dialcontrol.hxx"

namespace svx {

// ============================================================================

struct OrientationHelper_Impl
{
    typedef std::pair< Window*, TriState >  WindowPair;
    typedef std::vector< WindowPair >       WindowVec;

    DialControl&        mrCtrlDial;
    CheckBox&           mrCbStacked;
    WindowVec           maWinVec;
    bool                mbEnabled;
    bool                mbVisible;

    explicit            OrientationHelper_Impl( DialControl& rCtrlDial, CheckBox& rCbStacked );

    void                AddDependentWindow( Window& rWindow, TriState eDisableIfStacked );

    void                EnableDependentWindows();
    void                EnableWindow( Window& rWindow, TriState eDisableIfStacked );

    void                ShowDependentWindows();

    DECL_LINK( ClickHdl, void* );
};

// ----------------------------------------------------------------------------

OrientationHelper_Impl::OrientationHelper_Impl( DialControl& rCtrlDial, CheckBox& rCbStacked ) :
    mrCtrlDial( rCtrlDial ),
    mrCbStacked( rCbStacked ),
    mbEnabled( rCtrlDial.IsEnabled() ),
    mbVisible( rCtrlDial.IsVisible() )
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
    bool bDisableOnStacked = false;
    switch( eDisableIfStacked )
    {
        // STATE_CHECK: Disable window, if stacked text is turned on or "don't know".
        case STATE_CHECK:   bDisableOnStacked = (mrCbStacked.GetState() != STATE_NOCHECK);  break;
        // STATE_NOCHECK: Disable window, if stacked text is turned off or "don't know".
        case STATE_NOCHECK: bDisableOnStacked = (mrCbStacked.GetState() != STATE_CHECK);    break;
        default: ;//prevent warning
    }
    rWindow.Enable( mbEnabled && !bDisableOnStacked );
}

void OrientationHelper_Impl::ShowDependentWindows()
{
    for( WindowVec::iterator aIt = maWinVec.begin(), aEnd = maWinVec.end(); aIt != aEnd; ++aIt )
        aIt->first->Show( mbVisible );
}

IMPL_LINK( OrientationHelper_Impl, ClickHdl, void*, EMPTYARG )
{
    EnableDependentWindows();
    return 0L;
}

// ============================================================================

OrientationHelper::OrientationHelper( DialControl& rCtrlDial, CheckBox& rCbStacked ) :
    mpImpl( new OrientationHelper_Impl( rCtrlDial, rCbStacked ) )
{
    mpImpl->EnableDependentWindows();
    mpImpl->ShowDependentWindows();
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

void OrientationHelper::AddDependentWindow( Window& rWindow, TriState eDisableIfStacked )
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

