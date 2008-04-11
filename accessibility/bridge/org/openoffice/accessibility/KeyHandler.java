/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: KeyHandler.java,v $
 * $Revision: 1.10 $
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


package org.openoffice.accessibility;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.awt.XKeyHandler;
import org.openoffice.java.accessibility.AccessibleKeyBinding;
import org.openoffice.java.accessibility.Build;

import java.awt.*;
import java.awt.event.KeyEvent;
import javax.accessibility.*;

public class KeyHandler extends Component implements XKeyHandler, java.awt.KeyEventDispatcher {
    EventQueue eventQueue;

    public class VCLKeyEvent extends KeyEvent implements Runnable {
        boolean consumed = true;

        public VCLKeyEvent(Component c, int id, int modifiers, int keyCode, char keyChar) {
            super(c, id, System.currentTimeMillis(), modifiers, keyCode, keyChar);
        }

        public void run() {
            // This is a no-op ..
        }

        public void setConsumed(boolean b) {
            consumed = b;
        }

        public boolean isConsumed() {
            return consumed;
        }
    }

    public KeyHandler() {
        eventQueue = Toolkit.getDefaultToolkit().getSystemEventQueue();
        java.awt.KeyboardFocusManager.getCurrentKeyboardFocusManager().addKeyEventDispatcher(this);
    }

    /** This method is called by the current KeyboardFocusManager requesting that this KeyEventDispatcher
    * dispatch the specified event on its behalf
    */
    public boolean dispatchKeyEvent(java.awt.event.KeyEvent e) {
        if (e instanceof VCLKeyEvent) {
            VCLKeyEvent event = (VCLKeyEvent) e;
            event.setConsumed(false);
            return true;
        }
        return false;
    }

    /** Handler for KeyPressed events */
    public boolean keyPressed(com.sun.star.awt.KeyEvent event) {
//      try {
            VCLKeyEvent vke = new VCLKeyEvent(this, KeyEvent.KEY_PRESSED,
                AccessibleKeyBinding.convertModifiers(event.Modifiers),
                AccessibleKeyBinding.convertKeyCode(event.KeyCode),
                event.KeyChar != 0 ? event.KeyChar : KeyEvent.CHAR_UNDEFINED);

            eventQueue.postEvent(vke);

            // VCL events for TABs have empty KeyChar
            if (event.KeyCode == com.sun.star.awt.Key.TAB ) {
                event.KeyChar = '\t';
            }

            // Synthesize KEY_TYPED event to emulate Java behavior
            if (event.KeyChar != 0) {
                eventQueue.postEvent(new VCLKeyEvent(this,
                    KeyEvent.KEY_TYPED,
                    AccessibleKeyBinding.convertModifiers(event.Modifiers),
                    KeyEvent.VK_UNDEFINED,
                    event.KeyChar));
            }

            // Wait until the key event is processed
            return false;
//          eventQueue.invokeAndWait(vke);
//          return vke.isConsumed();
//      } catch(java.lang.InterruptedException e) {
//          return false;
//      } catch(java.lang.reflect.InvocationTargetException e) {
//          return false;
//      }
    }

    /** Handler for KeyReleased events */
    public boolean keyReleased(com.sun.star.awt.KeyEvent event) {
//      try {
            VCLKeyEvent vke = new VCLKeyEvent(this, KeyEvent.KEY_RELEASED,
                AccessibleKeyBinding.convertModifiers(event.Modifiers),
                AccessibleKeyBinding.convertKeyCode(event.KeyCode),
                event.KeyChar != 0 ? event.KeyChar : KeyEvent.CHAR_UNDEFINED);
            eventQueue.postEvent(vke);

            // Wait until the key event is processed
            return false;
//          eventQueue.invokeAndWait(vke);
//          return vke.isConsumed();
//      } catch(java.lang.InterruptedException e) {
//          return false;
//      } catch(java.lang.reflect.InvocationTargetException e) {
//          return false;
//      }
    }

    public void disposing(com.sun.star.lang.EventObject event) {
        java.awt.KeyboardFocusManager.getCurrentKeyboardFocusManager().removeKeyEventDispatcher(this);
    }
};
