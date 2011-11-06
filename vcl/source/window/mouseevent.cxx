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
#include "precompiled_vcl.hxx"

#include <com/sun/star/awt/MouseEvent.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/awt/MouseButton.hpp>
#include <tools/debug.hxx>
#include <vcl/event.hxx>

/** inits this vcl KeyEvent with all settings from the given awt event **/
MouseEvent::MouseEvent( const ::com::sun::star::awt::MouseEvent& rEvent )
: maPos( rEvent.X, rEvent.Y )
, mnMode( 0 )
, mnClicks( static_cast< sal_uInt16 >( rEvent.ClickCount ) )
, mnCode( 0 )
{
    if( rEvent.Modifiers )
    {
        if( (rEvent.Modifiers & ::com::sun::star::awt::KeyModifier::SHIFT) != 0 )
            mnCode |= KEY_SHIFT;
        if( (rEvent.Modifiers & ::com::sun::star::awt::KeyModifier::MOD1) != 0 )
            mnCode |= KEY_MOD1;
        if( (rEvent.Modifiers & ::com::sun::star::awt::KeyModifier::MOD2) != 0 )
            mnCode |= KEY_MOD2;
                if( (rEvent.Modifiers & ::com::sun::star::awt::KeyModifier::MOD3) != 0 )
                        mnCode |= KEY_MOD3;
    }

    if( rEvent.Buttons )
    {
        if( (rEvent.Buttons & ::com::sun::star::awt::MouseButton::LEFT) != 0 )
            mnCode |= MOUSE_LEFT;
        if( (rEvent.Buttons & ::com::sun::star::awt::MouseButton::RIGHT) != 0 )
            mnCode |= MOUSE_RIGHT;
        if( (rEvent.Buttons & ::com::sun::star::awt::MouseButton::MIDDLE) != 0 )
            mnCode |= MOUSE_MIDDLE;
    }
}

/** fills out the given awt KeyEvent with all settings from this vcl event **/
void MouseEvent::InitMouseEvent( ::com::sun::star::awt::MouseEvent& rEvent ) const
{
    rEvent.Modifiers = 0;
    if ( IsShift() )
        rEvent.Modifiers |= ::com::sun::star::awt::KeyModifier::SHIFT;
    if ( IsMod1() )
        rEvent.Modifiers |= ::com::sun::star::awt::KeyModifier::MOD1;
    if ( IsMod2() )
        rEvent.Modifiers |= ::com::sun::star::awt::KeyModifier::MOD2;
        if ( IsMod3() )
                rEvent.Modifiers |= ::com::sun::star::awt::KeyModifier::MOD3;

    rEvent.Buttons = 0;
    if ( IsLeft() )
        rEvent.Buttons |= ::com::sun::star::awt::MouseButton::LEFT;
    if ( IsRight() )
        rEvent.Buttons |= ::com::sun::star::awt::MouseButton::RIGHT;
    if ( IsMiddle() )
        rEvent.Buttons |= ::com::sun::star::awt::MouseButton::MIDDLE;

    rEvent.X = GetPosPixel().X();
    rEvent.Y = GetPosPixel().Y();
    rEvent.ClickCount = GetClicks();
    rEvent.PopupTrigger = sal_False;
}
