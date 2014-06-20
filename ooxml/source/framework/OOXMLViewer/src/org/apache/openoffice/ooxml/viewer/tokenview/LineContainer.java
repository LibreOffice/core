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

import java.util.Vector;

public class LineContainer<TokenType>
{
    LineContainer ()
    {
        maLines = new Vector<>();
    }




    boolean IsLineInGroup (
        final Line<TokenType> aLine,
        final Run<TokenType> aGroup)
    {
        if (aGroup == null)
            return false;

        final Line<TokenType> aStartLine = aGroup.GetLine();
        if (aLine.GetTop() < aStartLine.GetTop())
            return false;
        final Line<TokenType> aEndLine = aGroup.GetGroupEnd().GetLine();
        if (aLine.GetTop() > aEndLine.GetTop())
            return false;

        return true;
    }




    Line<TokenType> GetLineForY (final int nY)
    {
        int nMin = 0;
        int nMax = maLines.size()-1;
        if (nMin <= nMax)
        {
            while (nMin < nMax-1)
            {
                final int nMed = (nMax+nMin)/2;
                if (nY < maLines.get(nMed).GetTop())
                    nMax = nMed;
                else
                    nMin = nMed;
            }
            for (int nIndex=nMin; nIndex<=nMax; ++nIndex)
                if (maLines.get(nIndex).Contains(nY))
                    return maLines.get(nIndex);
        }
        return null;
    }




    public Line<TokenType> GetLineForOffset (final int nOffset)
    {
        final int nLineIndex = GetLineIndexForOffset(nOffset, 0);
        if (nLineIndex < 0)
            return null;
        else
            return maLines.get(nLineIndex);

    }




    public Iterable<Line<TokenType>> GetLinesForOffsets (
        final int nStartOffset,
        final int nEndOffset)
    {
        final Vector<Line<TokenType>> aLines = new Vector<>();

        final int nStartLineIndex = GetLineIndexForOffset(nStartOffset, -1);
        final int nEndLineIndex = GetLineIndexForOffset(nEndOffset, +1);
        if (nStartLineIndex >= 0)
        {
            if (nEndLineIndex < 0)
                aLines.add(maLines.get(nStartLineIndex));
            else
                for (int nIndex=nStartLineIndex; nIndex<=nEndLineIndex; ++nIndex)
                    aLines.add(maLines.get(nIndex));
        }
        return aLines;
    }




    public int GetLineCount()
    {
        return maLines.size();
    }




    public Line<TokenType> GetLine (final int nIndex)
    {
        return maLines.get(nIndex);
    }




    public Iterable<Line<TokenType>> GetLines ()
    {
        return maLines;
    }




    public void AddLine (final Line<TokenType> aLine)
    {
        maLines.add(aLine);
    }




    /** Return the index of the line that contains the given offset.
     *  When there is no line that contains the line that look at the bias
     *  to return the previous or next line.
     */
    private int GetLineIndexForOffset (
        final int nOffset,
        final int nBias)
    {
        int nMinIndex = 0;
        int nMaxIndex = maLines.size()-1;
        while (nMinIndex < nMaxIndex-1)
        {
            final int nMedIndex = (nMinIndex + nMaxIndex) / 2;
            if (maLines.get(nMedIndex).GetEndOffset() <= nOffset)
                nMinIndex = nMedIndex;
            else
                nMaxIndex = nMedIndex;
        }
        for (int nIndex=nMinIndex; nIndex<=nMaxIndex; ++nIndex)
        {
            if (maLines.get(nIndex).ContainsOffset(nOffset))
                return nIndex;
        }
        if (nBias < 0)
        {
            for (int nIndex=nMinIndex; nIndex<=nMaxIndex; ++nIndex)
            {
                if (maLines.get(nIndex).GetStartOffset() > nOffset)
                    return nIndex;
            }
        }
        else if (nBias > 0)
        {
            for (int nIndex=nMaxIndex; nIndex>=nMinIndex; ++nIndex)
            {
                if (maLines.get(nIndex).GetEndOffset() < nOffset)
                    return nIndex;
            }
        }

        return -1;
    }




    private final Vector<Line<TokenType>> maLines;
}
