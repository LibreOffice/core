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

/*
 * WindowListener.java
 *
 * Created on 30. Juli 2002, 12:36
 */

package helper;

/**
 * An own implementation of a XWindowListener
 *
 */
public class WindowListener implements com.sun.star.awt.XWindowListener {

    // hidden called
    public boolean hiddenTrigger;
    // move called
    public boolean movedTrigger;
    // resize called
    public boolean resizedTrigger;
    // show called
    public boolean shownTrigger;
    // dispose called
    public boolean disposeTrigger;

    /**
     * Creates a new WindowListener
     */
    public WindowListener() {
        resetTrigger();
    }

    /**
     * The window hidden event
     */
    public void windowHidden(com.sun.star.lang.EventObject eventObject) {
        hiddenTrigger = true;
    }

    /**
     * The window move event
     */
    public void windowMoved(com.sun.star.awt.WindowEvent windowEvent) {
        movedTrigger = true;
    }

    /**
     * The window resize event
     */
    public void windowResized(com.sun.star.awt.WindowEvent windowEvent) {
        resizedTrigger = true;
    }

    /**
     * The window show event
     */
    public void windowShown(com.sun.star.lang.EventObject eventObject) {
        shownTrigger = true;
    }

    /**
     * The dispose event
     */
    public void disposing(com.sun.star.lang.EventObject eventObject) {
        disposeTrigger = true;
    }

    /**
     * Reset all triggers to "not fired".
     */
    public void resetTrigger() {
        hiddenTrigger = false;
        movedTrigger = false;
        resizedTrigger = false;
        shownTrigger = false;
        disposeTrigger = false;
    }
}
