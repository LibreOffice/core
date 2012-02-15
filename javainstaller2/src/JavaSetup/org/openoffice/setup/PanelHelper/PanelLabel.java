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



package org.openoffice.setup.PanelHelper;

import java.awt.Color;
import java.awt.ComponentOrientation;
import java.awt.Font;
import javax.swing.JTextArea;
import javax.swing.UIManager;
import org.openoffice.setup.InstallData;

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

        InstallData data = InstallData.getInstance();
        if ( data.useRtl() ) { setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }

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
