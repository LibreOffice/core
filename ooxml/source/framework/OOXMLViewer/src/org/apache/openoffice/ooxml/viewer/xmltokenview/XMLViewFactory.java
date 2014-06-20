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

package org.apache.openoffice.ooxml.viewer.xmltokenview;

import java.util.Vector;

import javax.xml.stream.Location;

import org.apache.openoffice.ooxml.parser.ElementContext;
import org.apache.openoffice.ooxml.parser.ParseException;
import org.apache.openoffice.ooxml.parser.Parser;
import org.apache.openoffice.ooxml.parser.action.ActionTrigger;
import org.apache.openoffice.ooxml.parser.action.IAction;
import org.apache.openoffice.ooxml.viewer.tokenview.Run;
import org.apache.openoffice.ooxml.viewer.tokenview.RunRange;
import org.apache.openoffice.ooxml.viewer.tokenview.TokenView;
import org.apache.openoffice.ooxml.viewer.xml.TokenType;

public class XMLViewFactory
{
    public static void AddSemanticInformation(
        final TokenView<TokenType> aView,
        final Parser aParser,
        final Vector<String> aErrorsAndWarnings)
    {
        aParser.GetActionManager().AddElementStartAction(
            "*",
            new IAction()
            {
                public void Run (
                    final ActionTrigger eTrigger,
                    final ElementContext aContext,
                    final String sText,
                    final Location aStartLocation,
                    final Location aEndLocation)
                {
                    final RunRange<TokenType> aRuns = aView.GetRuns(
                        aStartLocation.getCharacterOffset(),
                        aEndLocation.getCharacterOffset());
                    if (aRuns.IsEmpty())
                        aView.GetRuns(
                            aStartLocation.getCharacterOffset(),
                            aEndLocation.getCharacterOffset());

                    // Search for the name (including namespace prefix) of the element.
                    int nIndex = aRuns.FindTokens(
                        TokenType.TAG_START,
                        TokenType.IDENTIFIER,
                        TokenType.COLON,
                        TokenType.IDENTIFIER);
                    if (nIndex < 0)
                        return;

                    aRuns.Get(nIndex+1).SetToolTipText(aContext.GetTypeName());
                    aRuns.Get(nIndex+2).SetToolTipText(aContext.GetTypeName());
                    aRuns.Get(nIndex+3).SetToolTipText(aContext.GetTypeName());
                    nIndex += 4;

                    // Process the attributes.
                    while (true)
                    {
                        final int nStartIndex = nIndex;
                        nIndex = aRuns.FindTokens(
                            nStartIndex,
                            TokenType.IDENTIFIER,
                            TokenType.COLON,
                            TokenType.IDENTIFIER,
                            TokenType.ATTRIBUTE_DEFINE);
                        if (nIndex >= 0)
                        {
                            final String sAttributeName = aRuns.Get(nIndex+2).GetText();
                            aRuns.Get(nIndex+0).SetToolTipText("attribute define of "+sAttributeName);
                            aRuns.Get(nIndex+1).SetToolTipText("attribute define of "+sAttributeName);
                            aRuns.Get(nIndex+2).SetToolTipText("attribute define of "+sAttributeName);
                            nIndex += 5;
                        }
                        else
                        {
                            // Try the variant without namespace.
                            nIndex = aRuns.FindTokens(
                                nStartIndex,
                                TokenType.IDENTIFIER,
                                TokenType.ATTRIBUTE_DEFINE);
                            if (nIndex >= 0)
                            {
                                final String sAttributeName = aRuns.Get(nIndex).GetText();
                                aRuns.Get(nIndex).SetToolTipText("attribute define of "+sAttributeName);
                                nIndex += 3;
                            }
                            else
                            {
                                // No more attributes.
                                break;
                            }
                        }
                    }
                }
            });
        try
        {
            aParser.Parse();
        }
        catch (final ParseException aException)
        {
            System.err.printf("caught exception when parsing %d,%d/%d\n",
                aException.Location.getLineNumber(),
                aException.Location.getColumnNumber(),
                aException.Location.getCharacterOffset());

            final Run<TokenType> aRun = aView.GetRun(aException.Location.getCharacterOffset());
            if (aRun != null)
            {
                aView.MarkError(aRun);
                aRun.SetToolTipText(
                    String.format(
                        "parse error at %d,%d/%d\n",
                        aException.Location.getLineNumber(),
                        aException.Location.getColumnNumber(),
                        aException.Location.getCharacterOffset()));
                aView.ShowRun(aRun);
            }
        }
    }
}
