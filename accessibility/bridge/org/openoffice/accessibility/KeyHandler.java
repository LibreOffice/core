/*************************************************************************
 *
 *  $RCSfile: KeyHandler.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: obr $ $Date: 2002-08-23 09:32:26 $
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

public class KeyHandler extends Component implements XKeyHandler {
    EventQueue eventQueue;

    public class VCLKeyEvent extends KeyEvent {
        com.sun.star.awt.KeyEvent data;

        public VCLKeyEvent(int id, com.sun.star.awt.KeyEvent event) {
            super( KeyHandler.this,
                id,
                System.currentTimeMillis(),
                AccessibleKeyBinding.convertModifiers(event.Modifiers),
                id == KeyEvent.KEY_TYPED ? KeyEvent.VK_UNDEFINED : AccessibleKeyBinding.convertKeyCode(event.KeyCode),
                event.KeyChar != 0 ? event.KeyChar : KeyEvent.CHAR_UNDEFINED
            );

            data = event;
        }

        public void setData(com.sun.star.awt.KeyEvent event) {
            data = event;
        }

        public com.sun.star.awt.KeyEvent getData() {
            return data;
        }
    }

    public KeyHandler() {
        eventQueue = Toolkit.getDefaultToolkit().getSystemEventQueue();
        enableEvents(AWTEvent.KEY_EVENT_MASK);
    }

    /** Processes key events occurring on this component by dispatching them to any registered KeyListener objects */
    protected void processKeyEvent(KeyEvent e) {
        // FIXME: will have to return this event object to VCL
        System.err.println("in process key event\n");

        // Ignore key typed events, because VCL doesn't know about them
        if( e.getID() != KeyEvent.KEY_TYPED && e instanceof VCLKeyEvent ) {
            VCLKeyEvent event = (VCLKeyEvent) e;
            com.sun.star.awt.KeyEvent unoKeyEvent = event.getData();

            // Return unhandled key events to VCL
            if( unoKeyEvent.Source != null ) {
                XKeyHandler xHandler = (XKeyHandler) UnoRuntime.queryInterface(XKeyHandler.class, unoKeyEvent.Source);
                if( xHandler != null ) {
                    if( e.getID() == KeyEvent.KEY_PRESSED ) {
                        xHandler.keyPressed(unoKeyEvent);
                    } else if( e.getID() == KeyEvent.KEY_RELEASED ) {
                        xHandler.keyReleased(unoKeyEvent);
                    } else if( Build.DEBUG ) {
                        System.err.println("*** WARNING ***: KeyEvent has unexspected id");
                    }
                } else if( Build.DEBUG ) {
                    System.err.println("*** ERROR ***: KeyEvent source does not implement XKeyHandler");
                }
            } else if( Build.DEBUG ) {
                System.err.println("*** ERROR ***: KeyEvent has no valid source");
            }
        }
    }

    /** Handler for KeyPressed events */
    public boolean keyPressed(com.sun.star.awt.KeyEvent event) {

        if( Build.DEBUG ) {
            System.err.println("retrieved key pressed event: " + event.KeyChar );
        }

        eventQueue.postEvent(new VCLKeyEvent(KeyEvent.KEY_PRESSED, event));

        // Synthesize KEY_PRESSED event to emulate Java behavior
        if( event.KeyChar != 0 ) {
            eventQueue.postEvent(new VCLKeyEvent(KeyEvent.KEY_TYPED, event));
        }

//      return true;
        return false;
    }

    /** Handler for KeyReleased events */
    public boolean keyReleased(com.sun.star.awt.KeyEvent event) {
        if( Build.DEBUG ) {
            System.err.println("retrieved key released event\n");
        }

        eventQueue.postEvent(new VCLKeyEvent(KeyEvent.KEY_RELEASED, event));

//      return false;
        return true;
    }

    public void disposing(com.sun.star.lang.EventObject event) {
    }
};
