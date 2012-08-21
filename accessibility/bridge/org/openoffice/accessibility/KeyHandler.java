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
}
