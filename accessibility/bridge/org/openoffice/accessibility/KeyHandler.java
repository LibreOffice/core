/*************************************************************************
 *
 *  $RCSfile: KeyHandler.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-18 15:48:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards this License Version 1.1
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
 *  Sun Industry Standards this License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  this License Version 1.1 (the "License"); You may not use this file
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


package org.openoffice.accessibility;

import com.sun.star.uno.UnoRuntime;
import drafts.com.sun.star.awt.XKeyHandler;
import org.openoffice.java.accessibility.AccessibleKeyBinding;
import org.openoffice.java.accessibility.Build;

import java.awt.*;
import java.awt.event.KeyEvent;
import javax.accessibility.*;

public class KeyHandler extends Component implements XKeyHandler, java.awt.KeyEventDispatcher {
    EventQueue eventQueue;

    public class VCLKeyEvent extends KeyEvent {
        com.sun.star.awt.KeyEvent data;
        XKeyHandler xKeyHandler;

        public VCLKeyEvent(Component c, int id, XKeyHandler handler, com.sun.star.awt.KeyEvent event) {
            super(c, id, System.currentTimeMillis(), AccessibleKeyBinding.convertModifiers(event.Modifiers),
                id == KeyEvent.KEY_TYPED ? KeyEvent.VK_UNDEFINED : AccessibleKeyBinding.convertKeyCode(event.KeyCode),
                event.KeyChar != 0 ? event.KeyChar : KeyEvent.CHAR_UNDEFINED
            );

            data = event;
            xKeyHandler = handler;
        }

        public void setData(com.sun.star.awt.KeyEvent event) {
            data = event;
        }

        public com.sun.star.awt.KeyEvent getData() {
            return data;
        }

        public XKeyHandler getKeyHandler() {
            return xKeyHandler;
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
        if (Build.DEBUG) {
            System.err.println("in dispatchKeyEvent()");
        }

        if ((e.getID() != KeyEvent.KEY_TYPED) && (e instanceof VCLKeyEvent)) {
            VCLKeyEvent event = (VCLKeyEvent) e;
            com.sun.star.awt.KeyEvent unoKeyEvent = event.getData();
            XKeyHandler xHandler = event.getKeyHandler();

            // Return unhandled key events to VCL
            if (e.getID() == KeyEvent.KEY_PRESSED) {
                xHandler.keyPressed(unoKeyEvent);
            } else if (e.getID() == KeyEvent.KEY_RELEASED) {
                xHandler.keyReleased(unoKeyEvent);
            } else if (Build.DEBUG) {
                System.err.println("*** WARNING ***: KeyEvent has unexspected id");
            }
        }
        return false;
    }

    /** Handler for KeyPressed events */
    public boolean keyPressed(com.sun.star.awt.KeyEvent event) {
        if (event.Source != null) {
            XKeyHandler xHandler = (XKeyHandler) UnoRuntime.queryInterface(XKeyHandler.class, event.Source);
            if (xHandler != null) {
                eventQueue.postEvent(new VCLKeyEvent(this, KeyEvent.KEY_PRESSED, xHandler, event));

                // Synthesize KEY_PRESSED event to emulate Java behavior
                if (event.KeyChar != 0) {
                    eventQueue.postEvent(new VCLKeyEvent(this, KeyEvent.KEY_TYPED, xHandler, event));
                }

//              return true;
                return false;
            } else if (Build.DEBUG) {
//              System.err.println("*** ERROR *** KeyEvent source does not implement XKeyHandler");
            }
        } else if (Build.DEBUG) {
//          System.err.println("*** ERROR *** KeyEvent source not valid");
        }
        return false;
    }

    /** Handler for KeyReleased events */
    public boolean keyReleased(com.sun.star.awt.KeyEvent event) {
        if (event.Source != null) {
            XKeyHandler xHandler = (XKeyHandler) UnoRuntime.queryInterface(XKeyHandler.class, event.Source);
            if (xHandler != null) {
                eventQueue.postEvent(new VCLKeyEvent(this, KeyEvent.KEY_RELEASED, xHandler, event));
//              return true;
                return false;
            } else if (Build.DEBUG) {
                System.err.println("*** ERROR *** KeyEvent source does not implement XKeyHandler");
            }
        } else if (Build.DEBUG) {
            System.err.println("*** ERROR *** KeyEvent source not valid");
        }
        return false;
    }

    public void disposing(com.sun.star.lang.EventObject event) {
        java.awt.KeyboardFocusManager.getCurrentKeyboardFocusManager().removeKeyEventDispatcher(this);
    }
};
