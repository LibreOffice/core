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
package com.sun.star.wizards.ui.event;

import com.sun.star.awt.*;
import com.sun.star.lang.EventObject;

public class CommonListener extends AbstractListener implements XActionListener, XItemListener, XTextListener, EventNames, XWindowListener, XMouseListener, XFocusListener, XKeyListener
{

    /** Creates a new instance of CommonListener */
    public CommonListener()
    {
    }

    /**
     * Implementation of com.sun.star.awt.XActionListener
     */
    public void actionPerformed(com.sun.star.awt.ActionEvent actionEvent)
    {
        invoke(getEventSourceName(actionEvent), EVENT_ACTION_PERFORMED, actionEvent);
    }

    public void disposing(com.sun.star.lang.EventObject eventObject)
    {
    }

    /**
     * Implementation of com.sun.star.awt.XItemListener
     */
    public void itemStateChanged(ItemEvent itemEvent)
    {
        invoke(getEventSourceName(itemEvent), EVENT_ITEM_CHANGED, itemEvent);
    }

    /**
     * Implementation of com.sun.star.awt.XTextListener
     */
    public void textChanged(TextEvent textEvent)
    {
        invoke(getEventSourceName(textEvent), EVENT_TEXT_CHANGED, textEvent);
    }

    /**
     * @see com.sun.star.awt.XWindowListener#windowResized(com.sun.star.awt.WindowEvent)
     */
    public void windowResized(WindowEvent event)
    {
        invoke(getEventSourceName(event), EVENT_WINDOW_RESIZED, event);
    }

    /**
     * @see com.sun.star.awt.XWindowListener#windowMoved(com.sun.star.awt.WindowEvent)
     */
    public void windowMoved(WindowEvent event)
    {
        invoke(getEventSourceName(event), EVENT_WINDOW_MOVED, event);
    }

    /**
     * @see com.sun.star.awt.XWindowListener#windowShown(com.sun.star.lang.EventObject)
     */
    public void windowShown(EventObject event)
    {
        invoke(getEventSourceName(event), EVENT_WINDOW_SHOWN, event);
    }

    /**
     * @see com.sun.star.awt.XWindowListener#windowHidden(com.sun.star.lang.EventObject)
     */
    public void windowHidden(EventObject event)
    {
        invoke(getEventSourceName(event), EVENT_WINDOW_HIDDEN, event);
    }

    /**
     * @see com.sun.star.awt.XMouseListener#mousePressed(com.sun.star.awt.MouseEvent)
     */
    public void mousePressed(MouseEvent event)
    {
        invoke(getEventSourceName(event), EVENT_MOUSE_PRESSED, event);
    }

    /**
     * @see com.sun.star.awt.XMouseListener#mouseReleased(com.sun.star.awt.MouseEvent)
     */
    public void mouseReleased(MouseEvent event)
    {
        invoke(getEventSourceName(event), EVENT_KEY_RELEASED, event);
    }

    /**
     * @see com.sun.star.awt.XMouseListener#mouseEntered(com.sun.star.awt.MouseEvent)
     */
    public void mouseEntered(MouseEvent event)
    {
        invoke(getEventSourceName(event), EVENT_MOUSE_ENTERED, event);
    }

    /**
     * @see com.sun.star.awt.XMouseListener#mouseExited(com.sun.star.awt.MouseEvent)
     */
    public void mouseExited(MouseEvent event)
    {
        invoke(getEventSourceName(event), EVENT_MOUSE_EXITED, event);
    }

    /**
     * @see com.sun.star.awt.XFocusListener#focusGained(com.sun.star.awt.FocusEvent)
     */
    public void focusGained(FocusEvent event)
    {
        invoke(getEventSourceName(event), EVENT_FOCUS_GAINED, event);
    }

    /**
     * @see com.sun.star.awt.XFocusListener#focusLost(com.sun.star.awt.FocusEvent)
     */
    public void focusLost(FocusEvent event)
    {
        invoke(getEventSourceName(event), EVENT_FOCUS_LOST, event);
    }

    /**
     * @see com.sun.star.awt.XKeyListener#keyPressed(com.sun.star.awt.KeyEvent)
     */
    public void keyPressed(KeyEvent event)
    {
        invoke(getEventSourceName(event), EVENT_KEY_PRESSED, event);
    }

    /**
     * @see com.sun.star.awt.XKeyListener#keyReleased(com.sun.star.awt.KeyEvent)
     */
    public void keyReleased(KeyEvent event)
    {
        invoke(getEventSourceName(event), EVENT_KEY_RELEASED, event);
    }
}
