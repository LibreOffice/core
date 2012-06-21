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
package com.sun.star.wizards.web.data;

import com.sun.star.wizards.common.*;
import org.w3c.dom.Node;

public class CGDesign extends ConfigGroup implements XMLProvider
{

    public String cp_Layout;
    public String cp_Style;
    public String cp_BackgroundImage;
    public String cp_IconSet;
    //public int cp_OptimizeDisplay;
    public boolean cp_DisplayTitle;
    public boolean cp_DisplayDescription;
    public boolean cp_DisplayAuthor;
    public boolean cp_DisplayCreateDate;
    public boolean cp_DisplayUpdateDate;
    public boolean cp_DisplayFilename;
    public boolean cp_DisplayFileFormat;
    public boolean cp_DisplayFormatIcon;
    public boolean cp_DisplayPages;
    public boolean cp_DisplaySize;
    public int cp_OptimizeDisplaySize;
    private final String TRUE = "true";
    private final String FALSE = null;
    private String styleName;

    public Node createDOM(Node parent)
    {
        return XMLHelper.addElement(parent, "design", new String[0],
                new String[0]);

    }
    private static final Short SHORT_TRUE = new Short((short) 1);
    private static final Short SHORT_FALSE = new Short((short) 0);

    public short[] getStyle()
    {
        CGStyle style = ((CGStyle) ((CGSettings) root).cp_Styles.getElement(cp_Style));
        return new short[]
                {
                    (short) ((CGSettings) root).cp_Styles.getIndexOf(style)
                };
    }

    public void setStyle(short[] newStyle)
    {
        Object o = ((CGSettings) root).cp_Styles.getElementAt(newStyle[0]);
        cp_Style = (String) ((CGSettings) root).cp_Styles.getKey(o);
    }

    public int getLayout()
    {
        CGLayout layout = ((CGLayout) ((CGSettings) root).cp_Layouts.getElement(cp_Layout));
        return layout.cp_Index;
    }

    public void setLayout(int layoutIndex)
    {
        CGLayout layout = (CGLayout) ((CGSettings) root).cp_Layouts.getElementAt(layoutIndex);
        cp_Layout = (String) ((CGSettings) root).cp_Layouts.getKey(layout);
    }
}

