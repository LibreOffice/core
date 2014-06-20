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

package org.apache.openoffice.ooxml.viewer.tokenview;

import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics2D;

import javax.swing.UIManager;

public class Style
{
    public Style ()
    {
        maForegroundColor = Color.BLACK;
        if (DefaultFont == null)
            DefaultFont = UIManager.getDefaults().getFont("TextField.font");
        maFont = DefaultFont;
    }




    public void Set(Graphics2D aG2)
    {
        aG2.setColor(maForegroundColor);
        aG2.setFont(maFont);
    }




    public Style SetForegroundColor (final Color aColor)
    {
        maForegroundColor = aColor;
        return this;
    }




    public Color GetForegroundColor()
    {
        return maForegroundColor;
    }




    public Style SetBackgroundColor (final Color aColor)
    {
        maBackgroundColor = aColor;
        return this;
    }




    public Color GetBackgroundColor()
    {
        return maBackgroundColor;
    }




    public Style SetBold()
    {
        maFont = maFont.deriveFont(Font.BOLD);
        return this;
    }




    public Font GetFont()
    {
        return maFont;
    }




    public static final Style DefaultStyle = new Style();
    public static Font DefaultFont = null;

    private Color maBackgroundColor;
    private Color maForegroundColor;
    private Font maFont;
}
