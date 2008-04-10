/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PanelLabel.java,v $
 * $Revision: 1.4 $
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

package org.openoffice.setup.PanelHelper;

import java.awt.Color;
import java.awt.Font;
import javax.swing.JTextArea;
import javax.swing.UIManager;

public class PanelLabel extends JTextArea {

    static private Color BackgroundColor;
    static private Color TextColor;
    static private Font  TextFont;

    public PanelLabel() {
    }

    public PanelLabel(String text, int rows, int columns) {
        super(text, rows, columns);
        init(true);
    }

    public PanelLabel(String text, boolean multiline) {
        super(text);
        init(multiline);
    }

    public PanelLabel(String text) {
        super(text);
        init(false);
    }

    private void init(boolean multiline) {
        setEditable(false);
        setBackground(BackgroundColor);
        setForeground(TextColor);
        setFont(TextFont);
        setFocusable(false);

        if (multiline) {
            setLineWrap(true);
            setWrapStyleWord(true);
        }
    }

    static {
        BackgroundColor = (Color)UIManager.get("Label.background");
        TextColor       = (Color)UIManager.get("Label.foreground");
        TextFont        = ((Font)UIManager.get("Label.font")).deriveFont(Font.PLAIN);
    }
}
