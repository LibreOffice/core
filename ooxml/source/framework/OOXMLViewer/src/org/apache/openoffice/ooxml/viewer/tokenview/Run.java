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
import java.awt.FontMetrics;
import java.awt.Graphics2D;

public class Run<TokenType>
{
    Run (
        final String sText,
        final TokenType eTokenType,
        final Style aStyle,
        final int nOffset)
    {
        msText = sText;
        meTokenType = eTokenType;
        maStyle = aStyle!=null ? aStyle : Style.DefaultStyle;
        mnStreamOffset = nOffset;

        mnWidth = -1;
        mnHeight = -1;
        mnOffset = nOffset;
        maParent = null;
        maGroupEnd = null;
        maLine = null;
    }




    public void Format (
        final Graphics2D aG2)
    {
        final FontMetrics aMetrics = aG2.getFontMetrics(maStyle.GetFont());
        mnWidth = aMetrics.stringWidth(msText);
        mnHeight = aMetrics.getHeight();
        mnOffset = -aMetrics.getDescent();
    }




    public void Paint (
        final Graphics2D aG2,
        final int nX,
        final int nY,
        final Color aBackgroundColor)
    {
        maStyle.Set(aG2);

        if (mnWidth < 0)
        {
            mnWidth = aG2.getFontMetrics().stringWidth(msText);
            mnHeight = aG2.getFontMetrics().getHeight();
        }

        if (aBackgroundColor != null)
        {
            final Color aSavedColor = aG2.getColor();
            aG2.setColor(aBackgroundColor);
            aG2.fillRect(nX,nY-mnHeight, mnWidth, mnHeight);
            aG2.setColor(aSavedColor);
        }
        aG2.drawString(msText, nX, nY+mnOffset);

        if (msToolTipText != null)
        {
            aG2.drawLine(nX, nY-1, nX+mnWidth, nY-1);
        }
    }




    public String GetText()
    {
        return msText;
    }




    public Style GetStyle ()
    {
        return maStyle;
    }




    public int GetStreamOffset ()
    {
        return mnStreamOffset;
    }




    public int GetStreamEndOffset ()
    {
        return mnStreamOffset + msText.length();
    }




    public int GetWidth()
    {
        return mnWidth;
    }




    public int GetHeight ()
    {
        return mnHeight;
    }




    public void SetGroupParent (final Run<TokenType> aParent)
    {
        maParent = aParent;
    }




    public void SetGroupEnd (final Run<TokenType> aRun)
    {
        maGroupEnd = aRun;
    }




    public Run<TokenType> GetGroupEnd()
    {
        return maGroupEnd;
    }




    public boolean IsGroup ()
    {
        if (maGroupEnd == null)
            return false;
        else if (maLine == maGroupEnd.maLine)
            return true;
        else
            return true;
    }




    public Run<TokenType> GetParent ()
    {
        return maParent;
    }




    public Line<TokenType> GetLine ()
    {
        return maLine;
    }




    public void SetLine (final Line<TokenType> aLine)
    {
        maLine = aLine;
    }




    public void SetToolTipText (final String sText)
    {
        msToolTipText = sText;
    }




    public String GetToolTipText ()
    {
        return msToolTipText;
    }




    public TokenType GetTokenType ()
    {
        return meTokenType;
    }




    @Override
    public String toString ()
    {
        return "run '"+msText+"' @ "+mnOffset;
    }




    private final String msText;
    private final TokenType meTokenType;
    private final Style maStyle;
    private final int mnStreamOffset;
    private int mnWidth;
    private int mnHeight;
    private int mnOffset;
    private Run<TokenType> maParent;
    private Run<TokenType> maGroupEnd;
    private Line<TokenType> maLine;
    private String msToolTipText;
}
