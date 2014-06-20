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

import java.util.Stack;


public class DocumentFactory<TokenType>
{
    interface IRepaintTarget
    {
        void RequestRepaint();
    }




    DocumentFactory (
        final LineContainer<TokenType> aLines,
        final IRepaintTarget aRepaintTarget)
    {
        maLines = aLines;
        maRepaintTarget = aRepaintTarget;
        mbIsGroupBeginPending = false;
        maGroupStartStack = new Stack<Run<TokenType>>();
        maLastRun = null;
        maCurrentGroup = null;
        StartNewLine();
    }




    public void AddText (
        final String sText,
        final TokenType eTokenType,
        final Style aStyle,
        final int nOffset)
    {
        final Run<TokenType> aRun = new Run<TokenType>(sText, eTokenType, aStyle, nOffset);
        synchronized(maLines)
        {
            maCurrentLine.AddRun(aRun);
            if (sText.endsWith("\n"))
                StartNewLine();
        }

        if (mbIsGroupBeginPending)
        {
            maGroupStartStack.push(maCurrentGroup);
            aRun.SetGroupParent(maCurrentGroup);
            maCurrentGroup = aRun;
            mbIsGroupBeginPending = false;
        }
        else if (maCurrentGroup != null)
            aRun.SetGroupParent(maCurrentGroup);
        maLastRun = aRun;

        maRepaintTarget.RequestRepaint();
    }




    public void FinishText ()
    {
        StartNewLine();
    }




    public void BeginGroup()
    {
        mbIsGroupBeginPending = true;
    }




    public void EndGroup ()
    {
        maCurrentGroup.SetGroupEnd(maLastRun);
        maCurrentGroup = maGroupStartStack.pop();
    }





    private void StartNewLine ()
    {
        if (maCurrentLine != null)
            maLines.AddLine(maCurrentLine);
        maCurrentLine = new Line<TokenType>();
    }




    private final LineContainer<TokenType> maLines;
    private final IRepaintTarget maRepaintTarget;
    private Line<TokenType> maCurrentLine;
    private boolean mbIsGroupBeginPending;
    private Stack<Run<TokenType>> maGroupStartStack;
    private Run<TokenType> maLastRun;
    private Run<TokenType> maCurrentGroup;
}
