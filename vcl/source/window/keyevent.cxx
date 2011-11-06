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

#include <com/sun/star/awt/KeyEvent.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <tools/debug.hxx>
#include <vcl/event.hxx>

KeyEvent::KeyEvent (const KeyEvent& rKeyEvent) :
    maKeyCode (rKeyEvent.maKeyCode),
    mnRepeat  (rKeyEvent.mnRepeat),
    mnCharCode(rKeyEvent.mnCharCode)
{}

/** inits this vcl KeyEvent with all settings from the given awt event **/
KeyEvent::KeyEvent( const ::com::sun::star::awt::KeyEvent& rEvent )
{
    maKeyCode = KeyCode(
        rEvent.KeyCode,
        (rEvent.Modifiers & ::com::sun::star::awt::KeyModifier::SHIFT) != 0,
        (rEvent.Modifiers & ::com::sun::star::awt::KeyModifier::MOD1) != 0,
        (rEvent.Modifiers & ::com::sun::star::awt::KeyModifier::MOD2) != 0,
                (rEvent.Modifiers & ::com::sun::star::awt::KeyModifier::MOD3) != 0);
    mnRepeat = 0;
    mnCharCode = rEvent.KeyChar;
}

/** fills out the given awt KeyEvent with all settings from this vcl event **/
void KeyEvent::InitKeyEvent( ::com::sun::star::awt::KeyEvent& rEvent ) const
{
    rEvent.Modifiers = 0;
    if( GetKeyCode().IsShift() )
        rEvent.Modifiers |= ::com::sun::star::awt::KeyModifier::SHIFT;
    if( GetKeyCode().IsMod1() )
        rEvent.Modifiers |= ::com::sun::star::awt::KeyModifier::MOD1;
    if( GetKeyCode().IsMod2() )
        rEvent.Modifiers |= ::com::sun::star::awt::KeyModifier::MOD2;
        if( GetKeyCode().IsMod3() )
                rEvent.Modifiers |= ::com::sun::star::awt::KeyModifier::MOD3;

    rEvent.KeyCode = GetKeyCode().GetCode();
    rEvent.KeyChar = GetCharCode();
    rEvent.KeyFunc = sal::static_int_cast< sal_Int16 >(GetKeyCode().GetFunction());
}

KeyEvent KeyEvent::LogicalTextDirectionality (TextDirectionality eMode) const
{
    KeyEvent aClone(*this);

    sal_uInt16 nCode = maKeyCode.GetCode();
    sal_uInt16 nMod  = maKeyCode.GetAllModifier();

    switch (eMode)
    {
        case TextDirectionality_RightToLeft_TopToBottom:
            switch (nCode)
            {
                case KEY_LEFT:  aClone.maKeyCode = KeyCode(KEY_RIGHT, nMod); break;
                case KEY_RIGHT: aClone.maKeyCode = KeyCode(KEY_LEFT,  nMod); break;
            }
            break;

        case TextDirectionality_TopToBottom_RightToLeft:
            switch (nCode)
            {
                case KEY_DOWN:  aClone.maKeyCode = KeyCode(KEY_RIGHT, nMod); break;
                case KEY_UP:    aClone.maKeyCode = KeyCode(KEY_LEFT,  nMod); break;
                case KEY_LEFT:  aClone.maKeyCode = KeyCode(KEY_DOWN,  nMod); break;
                case KEY_RIGHT: aClone.maKeyCode = KeyCode(KEY_UP,    nMod); break;
            }
            break;

        case TextDirectionality_LeftToRight_TopToBottom:
            /* do nothing */
            break;
    }

    return aClone;
}


// -------------------------------------------------------

const Point&   HelpEvent::GetMousePosPixel() const
{
    //DBG_ASSERT( !mbKeyboardActivated, "Keyboard help has no mouse position !");
    return maPos;
}

