/*************************************************************************
 *
 *  $RCSfile: AccessibleKeyBinding.java,v $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package org.openoffice.java.accessibility;

import drafts.com.sun.star.accessibility.*;
import drafts.com.sun.star.awt.*;

/**
 *
 */
public class AccessibleKeyBinding extends Object implements javax.accessibility.AccessibleKeyBinding {

    XAccessibleKeyBinding unoAccessibleKeybinding;

    public AccessibleKeyBinding(XAccessibleKeyBinding unoKB) {
        unoAccessibleKeybinding = unoKB;
    }

    public static int convertModifiers(short s) {
        int modifiers = 0;
        if ((s & com.sun.star.awt.KeyModifier.SHIFT) != 0) {
            modifiers = modifiers | java.awt.event.KeyEvent.SHIFT_DOWN_MASK;
        }

        if ((s & com.sun.star.awt.KeyModifier.MOD1) != 0) {
            modifiers = modifiers | java.awt.event.KeyEvent.CTRL_DOWN_MASK;
        }

        if ((s & com.sun.star.awt.KeyModifier.MOD2) != 0) {
            modifiers = modifiers | java.awt.event.KeyEvent.ALT_DOWN_MASK;
        }
        return modifiers;
    }

    public static int convertKeyCode(short s) {
        int keycode = java.awt.event.KeyEvent.VK_UNDEFINED;

        switch(s) {
            case com.sun.star.awt.Key.NUM0:
                keycode = java.awt.event.KeyEvent.VK_NUMPAD0;
                break;
            case com.sun.star.awt.Key.NUM1:
                keycode = java.awt.event.KeyEvent.VK_NUMPAD1;
                break;
            case com.sun.star.awt.Key.NUM2:
                keycode = java.awt.event.KeyEvent.VK_NUMPAD2;
                break;
            case com.sun.star.awt.Key.NUM3:
                keycode = java.awt.event.KeyEvent.VK_NUMPAD3;
                break;
            case com.sun.star.awt.Key.NUM4:
                keycode = java.awt.event.KeyEvent.VK_NUMPAD4;
                break;
            case com.sun.star.awt.Key.NUM5:
                keycode = java.awt.event.KeyEvent.VK_NUMPAD5;
                break;
            case com.sun.star.awt.Key.NUM6:
                keycode = java.awt.event.KeyEvent.VK_NUMPAD6;
                break;
            case com.sun.star.awt.Key.NUM7:
                keycode = java.awt.event.KeyEvent.VK_NUMPAD7;
                break;
            case com.sun.star.awt.Key.NUM8:
                keycode = java.awt.event.KeyEvent.VK_NUMPAD8;
                break;
            case com.sun.star.awt.Key.NUM9:
                keycode = java.awt.event.KeyEvent.VK_NUMPAD9;
                break;
            case com.sun.star.awt.Key.A:
                keycode = java.awt.event.KeyEvent.VK_A;
                break;
            case com.sun.star.awt.Key.B:
                keycode = java.awt.event.KeyEvent.VK_B;
                break;
            case com.sun.star.awt.Key.C:
                keycode = java.awt.event.KeyEvent.VK_C;
                break;
            case com.sun.star.awt.Key.D:
                keycode = java.awt.event.KeyEvent.VK_D;
                break;
            case com.sun.star.awt.Key.E:
                keycode = java.awt.event.KeyEvent.VK_E;
                break;
            case com.sun.star.awt.Key.F:
                keycode = java.awt.event.KeyEvent.VK_F;
                break;
            case com.sun.star.awt.Key.G:
                keycode = java.awt.event.KeyEvent.VK_G;
                break;
            case com.sun.star.awt.Key.H:
                keycode = java.awt.event.KeyEvent.VK_H;
                break;
            case com.sun.star.awt.Key.I:
                keycode = java.awt.event.KeyEvent.VK_I;
                break;
            case com.sun.star.awt.Key.J:
                keycode = java.awt.event.KeyEvent.VK_J;
                break;
            case com.sun.star.awt.Key.K:
                keycode = java.awt.event.KeyEvent.VK_K;
                break;
            case com.sun.star.awt.Key.L:
                keycode = java.awt.event.KeyEvent.VK_L;
                break;
            case com.sun.star.awt.Key.M:
                keycode = java.awt.event.KeyEvent.VK_M;
                break;
            case com.sun.star.awt.Key.N:
                keycode = java.awt.event.KeyEvent.VK_N;
                break;
            case com.sun.star.awt.Key.O:
                keycode = java.awt.event.KeyEvent.VK_O;
                break;
            case com.sun.star.awt.Key.P:
                keycode = java.awt.event.KeyEvent.VK_P;
                break;
            case com.sun.star.awt.Key.Q:
                keycode = java.awt.event.KeyEvent.VK_Q;
                break;
            case com.sun.star.awt.Key.R:
                keycode = java.awt.event.KeyEvent.VK_R;
                break;
            case com.sun.star.awt.Key.S:
                keycode = java.awt.event.KeyEvent.VK_S;
                break;
            case com.sun.star.awt.Key.T:
                keycode = java.awt.event.KeyEvent.VK_T;
                break;
            case com.sun.star.awt.Key.U:
                keycode = java.awt.event.KeyEvent.VK_U;
                break;
            case com.sun.star.awt.Key.V:
                keycode = java.awt.event.KeyEvent.VK_V;
                break;
            case com.sun.star.awt.Key.W:
                keycode = java.awt.event.KeyEvent.VK_W;
                break;
            case com.sun.star.awt.Key.X:
                keycode = java.awt.event.KeyEvent.VK_X;
                break;
            case com.sun.star.awt.Key.Y:
                keycode = java.awt.event.KeyEvent.VK_Y;
                break;
            case com.sun.star.awt.Key.Z:
                keycode = java.awt.event.KeyEvent.VK_Z;
                break;
            case com.sun.star.awt.Key.F1:
                keycode = java.awt.event.KeyEvent.VK_F1;
                break;
            case com.sun.star.awt.Key.F2:
                keycode = java.awt.event.KeyEvent.VK_F2;
                break;
            case com.sun.star.awt.Key.F3:
                keycode = java.awt.event.KeyEvent.VK_F3;
                break;
            case com.sun.star.awt.Key.F4:
                keycode = java.awt.event.KeyEvent.VK_F4;
                break;
            case com.sun.star.awt.Key.F5:
                keycode = java.awt.event.KeyEvent.VK_F5;
                break;
            case com.sun.star.awt.Key.F6:
                keycode = java.awt.event.KeyEvent.VK_F6;
                break;
            case com.sun.star.awt.Key.F7:
                keycode = java.awt.event.KeyEvent.VK_F7;
                break;
            case com.sun.star.awt.Key.F8:
                keycode = java.awt.event.KeyEvent.VK_F8;
                break;
            case com.sun.star.awt.Key.F9:
                keycode = java.awt.event.KeyEvent.VK_F9;
                break;
            case com.sun.star.awt.Key.F10:
                keycode = java.awt.event.KeyEvent.VK_F10;
                break;
            case com.sun.star.awt.Key.F11:
                keycode = java.awt.event.KeyEvent.VK_F11;
                break;
            case com.sun.star.awt.Key.F12:
                keycode = java.awt.event.KeyEvent.VK_F12;
                break;
            case com.sun.star.awt.Key.F13:
                keycode = java.awt.event.KeyEvent.VK_F13;
                break;
            case com.sun.star.awt.Key.F14:
                keycode = java.awt.event.KeyEvent.VK_F14;
                break;
            case com.sun.star.awt.Key.F15:
                keycode = java.awt.event.KeyEvent.VK_F15;
                break;
            case com.sun.star.awt.Key.F16:
                keycode = java.awt.event.KeyEvent.VK_F16;
                break;
            case com.sun.star.awt.Key.F17:
                keycode = java.awt.event.KeyEvent.VK_F17;
                break;
            case com.sun.star.awt.Key.F18:
                keycode = java.awt.event.KeyEvent.VK_F18;
                break;
            case com.sun.star.awt.Key.F19:
                keycode = java.awt.event.KeyEvent.VK_F19;
                break;
            case com.sun.star.awt.Key.F20:
                keycode = java.awt.event.KeyEvent.VK_F20;
                break;
            case com.sun.star.awt.Key.F21:
                keycode = java.awt.event.KeyEvent.VK_F21;
                break;
            case com.sun.star.awt.Key.F22:
                keycode = java.awt.event.KeyEvent.VK_F22;
                break;
            case com.sun.star.awt.Key.F23:
                keycode = java.awt.event.KeyEvent.VK_F23;
                break;
            case com.sun.star.awt.Key.F24:
                keycode = java.awt.event.KeyEvent.VK_F24;
                break;
            case com.sun.star.awt.Key.DOWN:
                keycode = java.awt.event.KeyEvent.VK_DOWN;
                break;
            case com.sun.star.awt.Key.UP:
                keycode = java.awt.event.KeyEvent.VK_UP;
                break;
            case com.sun.star.awt.Key.LEFT:
                keycode = java.awt.event.KeyEvent.VK_LEFT;
                break;
            case com.sun.star.awt.Key.RIGHT:
                keycode = java.awt.event.KeyEvent.VK_RIGHT;
                break;
            case com.sun.star.awt.Key.HOME:
                keycode = java.awt.event.KeyEvent.VK_HOME;
                break;
            case com.sun.star.awt.Key.END:
                keycode = java.awt.event.KeyEvent.VK_END;
                break;
            case com.sun.star.awt.Key.PAGEUP:
                keycode = java.awt.event.KeyEvent.VK_PAGE_UP;
                break;
            case com.sun.star.awt.Key.PAGEDOWN:
                keycode = java.awt.event.KeyEvent.VK_PAGE_DOWN;
                break;
            case com.sun.star.awt.Key.RETURN:
                keycode = java.awt.event.KeyEvent.VK_ENTER;
                break;
            case com.sun.star.awt.Key.ESCAPE:
                keycode = java.awt.event.KeyEvent.VK_ESCAPE;
                break;
            case com.sun.star.awt.Key.TAB:
                keycode = java.awt.event.KeyEvent.VK_TAB;
                break;
            case com.sun.star.awt.Key.BACKSPACE:
                keycode = java.awt.event.KeyEvent.VK_BACK_SPACE;
                break;
            case com.sun.star.awt.Key.SPACE:
                keycode = java.awt.event.KeyEvent.VK_SPACE;
                break;
            case com.sun.star.awt.Key.INSERT:
                keycode = java.awt.event.KeyEvent.VK_INSERT;
                break;
            case com.sun.star.awt.Key.DELETE:
                keycode = java.awt.event.KeyEvent.VK_DELETE;
                break;
            case com.sun.star.awt.Key.ADD:
                keycode = java.awt.event.KeyEvent.VK_ADD;
                break;
            case com.sun.star.awt.Key.SUBTRACT:
                keycode = java.awt.event.KeyEvent.VK_SUBTRACT;
                break;
            case com.sun.star.awt.Key.MULTIPLY:
                keycode = java.awt.event.KeyEvent.VK_MULTIPLY;
                break;
            case com.sun.star.awt.Key.DIVIDE:
                keycode = java.awt.event.KeyEvent.VK_DIVIDE;
                break;
            case com.sun.star.awt.Key.POINT:
                keycode = java.awt.event.KeyEvent.VK_PERIOD;
                break;
            case com.sun.star.awt.Key.COMMA:
                keycode = java.awt.event.KeyEvent.VK_COMMA;
                break;
            case com.sun.star.awt.Key.LESS:
                keycode = java.awt.event.KeyEvent.VK_LESS;
                break;
            case com.sun.star.awt.Key.GREATER:
                keycode = java.awt.event.KeyEvent.VK_GREATER;
                break;
            case com.sun.star.awt.Key.EQUAL:
                keycode = java.awt.event.KeyEvent.VK_EQUALS;
                break;
            case com.sun.star.awt.Key.CUT:
                keycode = java.awt.event.KeyEvent.VK_CUT;
                break;
            case com.sun.star.awt.Key.COPY:
                keycode = java.awt.event.KeyEvent.VK_COPY;
                break;
            case com.sun.star.awt.Key.PASTE:
                keycode = java.awt.event.KeyEvent.VK_PASTE;
                break;
            case com.sun.star.awt.Key.UNDO:
                keycode = java.awt.event.KeyEvent.VK_UNDO;
                break;
            case com.sun.star.awt.Key.FIND:
                keycode = java.awt.event.KeyEvent.VK_FIND;
                break;
            case com.sun.star.awt.Key.PROPERTIES:
                keycode = java.awt.event.KeyEvent.VK_PROPS;
                break;
            case com.sun.star.awt.Key.HELP:
                keycode = java.awt.event.KeyEvent.VK_HELP;
                break;
            default:
                ;
        }
        return keycode;
    }

    /*
    * AccessibleKeyBinding
    */

    /** Returns a key binding for this object */
    public Object getAccessibleKeyBinding(int i) {
        try {
            KeyStroke[] keys = unoAccessibleKeybinding.getAccessibleKeyBinding(i);
            javax.swing.KeyStroke[] data = new javax.swing.KeyStroke[keys.length];
            for (int j=0; j < keys.length; j++) {
                int keyCode = convertKeyCode(keys[j].KeyCode);
                if (keyCode != java.awt.event.KeyEvent.VK_UNDEFINED) {
                    data[j] = javax.swing.KeyStroke.getKeyStroke(keyCode, convertModifiers(keys[j].Modifiers));
                } else {
                    data[j] = null;
                }
            }

            if (keys.length == 1) {
                return data[0];
            } else {
                return data;
            }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            return null;
        } catch (com.sun.star.uno.RuntimeException e) {
            return null;
        }
    }

    /** Returns the number of key bindings for this object */
        public int getAccessibleKeyBindingCount() {
        try {
            return unoAccessibleKeybinding.getAccessibleKeyBindingCount();
        } catch (com.sun.star.uno.RuntimeException e) {
            return 0;
        }
    }
}
