/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: KeyHandler.java,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:28:36 $
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
