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

import java.awt.Graphics2D;
import java.util.Iterator;
import java.util.Vector;

public class Line<TokenType>

    implements Iterable<Run<TokenType>>
{
    Line ()
    {
        maRuns = new Vector<Run<TokenType>>();
        mnWidth = -1;
        mnHeight = -1;
        mnY = -1;
        mnStartOffset = -1;
        mnEndOffset = -1;
    }




    public void AddRun (final Run<TokenType> aRun)
    {
        maRuns.add(aRun);
        mnWidth += aRun.GetWidth();
        if (aRun.GetHeight() > mnHeight)
            mnHeight = aRun.GetHeight();
        aRun.SetLine(this);

        if (aRun.GetStreamOffset() >= 0)
        {
            if (mnStartOffset < 0)
                mnStartOffset = aRun.GetStreamOffset();

            if (mnEndOffset < aRun.GetStreamEndOffset())
                mnEndOffset = aRun.GetStreamEndOffset();
        }
    }




    public void Format (
        final Graphics2D aG2,
        final int nY)
    {
        mnY = nY;

        mnWidth = 0;
        mnHeight = 0;
        for (final Run<TokenType> aRun : maRuns)
        {
            aRun.Format(aG2);
            mnWidth += aRun.GetWidth();
            if (mnHeight < aRun.GetHeight())
                mnHeight = aRun.GetHeight();
        }
    }




    public int GetWidth ()
    {
        return mnWidth;
    }




    public int GetHeight ()
    {
        return mnHeight;
    }




    public int GetTop ()
    {
        return mnY;
    }




    public int GetBottom ()
    {
        return mnY + mnHeight;
    }




    public boolean Overlaps (
        final double nTop,
        final double nBottom)
    {
        return mnY<=nBottom && mnY+mnHeight>nTop;
    }




    public boolean Contains (final int nY)
    {
        return nY>=mnY && nY<mnY+mnHeight;
    }




    @Override
    public Iterator<Run<TokenType>> iterator()
    {
        return maRuns.iterator();
    }




    public Run<TokenType> GetRunForX (final int nX)
    {
        int nRunX = 0;
        for (final Run<TokenType> aRun : maRuns)
        {
            final int nRunWidth = aRun.GetWidth();
            final int nRight = nRunX + nRunWidth;
            if (nX>=nRunX && nX<nRight)
                return aRun;
            nRunX = nRight;
        }
        return null;
    }




    public Run<TokenType> GetRunForOffset (int nOffset)
    {
        for (int nIndex=0; nIndex<maRuns.size(); ++nIndex)
        {
            final Run<TokenType> aRun = maRuns.get(nIndex);
            final int nRunOffset = aRun.GetStreamOffset();
            if (nRunOffset >= 0)
                if (nRunOffset<=nOffset && nOffset<=aRun.GetStreamEndOffset())
                    return aRun;
        }
        return null;
    }




    public Iterable<Run<TokenType>> GetRunsForOffsets (
        final int nStartOffset,
        final int nEndOffset)
    {
        final Vector<Run<TokenType>> aRuns = new Vector<>();

        for (final Run<TokenType> aRun : maRuns)
        {
            if (aRun.GetStreamOffset() >= nEndOffset)
                break;
            else if (aRun.GetStreamEndOffset()<nStartOffset)
                continue;
            else
                aRuns.add(aRun);
        }

        return aRuns;
    }




    public int GetStartOffset()
    {
        return mnStartOffset;
    }




    public int GetEndOffset ()
    {
        return mnEndOffset;
    }




    public boolean ContainsOffset (final int nOffset)
    {
        return mnStartOffset<=nOffset && nOffset<mnEndOffset;
    }




    @Override
    public String toString ()
    {
        return String.format("line of %d runs: %s", maRuns.size(), maRuns.toString());
    }




    private final Vector<Run<TokenType>> maRuns;
    private int mnY;
    private int mnWidth;
    private int mnHeight;
    private int mnStartOffset;
    private int mnEndOffset;
}
