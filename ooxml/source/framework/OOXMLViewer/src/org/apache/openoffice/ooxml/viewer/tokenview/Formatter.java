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

import java.awt.Dimension;
import java.awt.Graphics2D;
import java.util.Vector;

public class Formatter<TokenType>
{
    public Formatter ()
    {
        mnLastFormattedLine = -1;
        mnWidth = 0;
        mnHeight = 0;
    }




    public FormatState<TokenType> FormatText (
        final Graphics2D aG2,
        final LineContainer<TokenType> aLines)
    {
        FormatLines(aG2, aLines);

        final double nTop = aG2.getClipBounds().getMinY();
        final double nBottom = aG2.getClipBounds().getMaxY();
        final Vector<Line<TokenType>> aVisibleLines = new Vector<>();
        for (final Line<TokenType> aLine : aLines.GetLines())
        {
            if (aLine.Overlaps(nTop, nBottom))
            {
                // Line is (partially) visible.
                aVisibleLines.add(aLine);
            }
        }

        return new FormatState<TokenType>(
            new Dimension(mnWidth,mnHeight),
            aVisibleLines);
    }




    private void FormatLines (
        final Graphics2D aG2,
        final LineContainer<TokenType> aLines)
    {
        for (int nIndex=mnLastFormattedLine+1,nCount=aLines.GetLineCount(); nIndex<nCount; ++nIndex)
        {
            final Line<TokenType> aLine = aLines.GetLine(nIndex);
            final int nY;
            if (nIndex > 0)
                nY = aLines.GetLine(nIndex-1).GetBottom();
            else
                nY = 0;
            aLine.Format(aG2, nY);
            if (aLine.GetWidth() > mnWidth)
                mnWidth = aLine.GetWidth();
            if (aLine.GetBottom() > mnHeight)
                mnHeight = aLine.GetBottom();
        }
        mnLastFormattedLine = aLines.GetLineCount()-1;
    }




    private int mnLastFormattedLine;
    private int mnWidth;
    private int mnHeight;
}
