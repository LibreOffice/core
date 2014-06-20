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

package org.apache.openoffice.ooxml.viewer.xml;

import java.awt.Color;
import java.util.HashMap;
import java.util.Map;

import org.apache.openoffice.ooxml.viewer.tokenview.DocumentFactory;
import org.apache.openoffice.ooxml.viewer.tokenview.Style;
import org.apache.openoffice.ooxml.viewer.tokenview.TokenView;

public class DocumentTokenFormatter
{
    DocumentTokenFormatter (
        final XMLScanner aScanner,
        final TokenView<TokenType> aView)
    {
        maScanner = aScanner;
        maView = aView.GetDocumentFactory();

        maTagStartEndStyle = new Style().SetForegroundColor(new Color(0x87CEFA));  // Light Sky Blue
        maNamespaceNameStyle = new Style().SetForegroundColor(new Color(0x7B68EE)); // Medium Slate Blue
        maIdentifierStyle = new Style()
            .SetForegroundColor(new Color(0x7B68EE))  // Medium Slate Blue
            .SetBold();
        maTextStyle = new Style().SetForegroundColor(new Color(0xF08080)); // Light Coral
        maAttributeValueStyle = new Style().SetForegroundColor(new Color(0xFFA07A)); // Light Salmon

        msIndentation = "";

        maNamespaceMap = new HashMap<String,String>();
    }




    public void Parse ()
    {
        try
        {
            ParseIntro();
            AppendText("\n", TokenType.WHITESPACE, null, -1);
            while (true)
            {
                switch (maScanner.Peek().Type)
                {
                    case EOF:
                        return;

                    case TAG_START:
                    case END_TAG_START:
                        ParseTag();
                        AppendText("\n", TokenType.WHITESPACE, null, -1);
                        break;

                    default:
                        ParseText();
                }
            }
        }
        catch(final Exception aException)
        {
            aException.printStackTrace();
        }
        maView.FinishText();
    }




    Map<String,String> GetNamespaceMap ()
    {
        return maNamespaceMap;
    }




    private void ParseIntro ()
    {
        final Token aStartToken = maScanner.Next();
        ExpectToken(aStartToken, TokenType.INTRO_START);
        ShowToken(aStartToken);

        ParseTagContent();

        final Token aEndToken = maScanner.Next();
        ExpectToken(aEndToken, TokenType.INTRO_END);
        ShowToken(aEndToken);
    }




    private void ParseTag ()
    {
        final Token aStartToken = maScanner.Next();
        ExpectToken(aStartToken, TokenType.TAG_START, TokenType.END_TAG_START);
        if (aStartToken.Type == TokenType.TAG_START)
            maView.BeginGroup();
        if (aStartToken.Type == TokenType.END_TAG_START)
            DecreaseIndentation();
        ShowToken(aStartToken);

        ParseTagContent();

        final Token aEndToken = maScanner.Next();
        if (aStartToken.Type == TokenType.TAG_START)
            ExpectToken(aEndToken, TokenType.TAG_END, TokenType.ELEMENT_END);
        else
            ExpectToken(aEndToken, TokenType.TAG_END);
        ShowToken(aEndToken);

        if (aStartToken.Type != TokenType.END_TAG_START
            && aEndToken.Type != TokenType.ELEMENT_END)
        {
            IncreaseIndentation();
        }
        else
        {
            maView.EndGroup();
        }
    }




    private void ParseTagContent ()
    {
        ParseQualifiedName();

        if (maScanner.Peek().Type != TokenType.IDENTIFIER)
            return;

        IncreaseIndentation();
        while (true)
        {
            final Token aToken = maScanner.Peek();
            if (aToken.Type != TokenType.IDENTIFIER)
                break;

            if (mbStartNewLineBeforeEachAttribute
                || mbStartNewLineBeforeNamespaceDefinition && aToken.Text.startsWith("xmlns"))
            {
                AppendText("\n", TokenType.WHITESPACE, null, -1);
                AppendText("  ", TokenType.WHITESPACE, null, -1);
            }
            else
            {
                AppendText(" ", TokenType.WHITESPACE, null, -1);
            }

            ParseQualifiedName();
            final Token aAssignToken = maScanner.Next();
            ExpectToken(aAssignToken, TokenType.ATTRIBUTE_DEFINE);
            ShowToken(aAssignToken);

            final Token aValueToken = maScanner.Next();
            ExpectToken(aValueToken, TokenType.ATTRIBUTE_VALUE);
            ShowToken(aValueToken, maAttributeValueStyle);

            if (msLastNamespaceName.equals("xmlns"))
                SaveNamespaceDefinition(msLastName, StripValueQuotes(aValueToken.Text));
        }
        DecreaseIndentation();
    }




    private void ParseQualifiedName ()
    {
        final Token aNameToken = maScanner.Next();
        ExpectToken(aNameToken, TokenType.IDENTIFIER);
        if (maScanner.Peek().Type == TokenType.COLON)
        {
            final Token aSeparatorToken = maScanner.Next();
            final Token aSecondNameToken = maScanner.Next();
            ExpectToken(aSecondNameToken, TokenType.IDENTIFIER);
            ShowToken(aNameToken, maNamespaceNameStyle);
            ShowToken(aSeparatorToken);
            ShowToken(aSecondNameToken, maIdentifierStyle);

            msLastNamespaceName = aNameToken.Text;
            msLastName = aSecondNameToken.Text;
        }
        else
        {
            ShowToken(aNameToken, maIdentifierStyle);

            msLastNamespaceName = "";
            msLastName = aNameToken.Text;
        }
    }




    private void ParseText ()
    {
        final Token aTextToken = maScanner.Next();
        ExpectToken(aTextToken, TokenType.TEXT);
        ShowToken(aTextToken, maTextStyle);
        AppendText("\n", TokenType.WHITESPACE, null, -1);
    }




    private TokenType ExpectToken (final Token aToken, final TokenType ... aExcpectedTypes)
    {
        for (final TokenType eType : aExcpectedTypes)
            if (aToken.Type == eType)
                return eType;

        if (aExcpectedTypes.length == 1)
        {
            throw new RuntimeException(
                    String.format(
                            "expected '%s' but got %s",
                            aExcpectedTypes[0].toString(),
                            aToken.toString()));
        }
        else
        {
            String sList = null;
            for (final TokenType eType : aExcpectedTypes)
            {
                if (sList != null)
                    sList += String.format(", '%s'", eType.toString());
                else
                    sList = String.format("'%s'", eType.toString());
            }
                    throw new RuntimeException(
                    String.format(
                            "expected one of %s but got %s",
                            sList,
                            aToken.toString()));
        }
    }




    private void ShowToken (final Token aToken)
    {
        AppendText(aToken.Text, aToken.Type, GetStyle(aToken.Type), aToken.Offset);
    }




    private void ShowToken (
        final Token aToken,
        final Style aStyle)
    {
        AppendText(aToken.Text, aToken.Type, aStyle, aToken.Offset);
    }




    private void AppendText (
        final String sText,
        final TokenType eTokenType,
        final Style aStyle,
        final int nOffset)
    {
        try
        {
            if (mbIsAtBeginningOfLine)
            {
                AddText(msIndentation, TokenType.WHITESPACE, aStyle, -1);
                mbIsAtBeginningOfLine = false;
            }
            AddText(sText, eTokenType, aStyle, nOffset);
            mbIsAtBeginningOfLine = sText.endsWith("\n");
        }
        catch (RuntimeException e)
        {
            e.printStackTrace();
        }
    }




    private void AddText (
        final String sText,
        final TokenType eTokenType,
        final Style aStyle,
        final int nOffset)
    {
        maView.AddText(sText, eTokenType, aStyle, nOffset);
    }




    private void IncreaseIndentation ()
    {
        msIndentation += "    ";
    }




    private void DecreaseIndentation ()
    {
        if ( ! msIndentation.isEmpty())
            msIndentation = msIndentation.substring(4);
    }




    private Style GetStyle (final TokenType eType)
    {
        switch(eType)
        {
            case TAG_START:
            case TAG_END:
            case END_TAG_START:
            case INTRO_START:
            case INTRO_END:
            case ELEMENT_END:
                return maTagStartEndStyle;

            case IDENTIFIER:
                return maIdentifierStyle;

            case TEXT:
                return maTextStyle;

            case ATTRIBUTE_VALUE:
                return maAttributeValueStyle;

            default:
                return null;
        }
    }




    private String StripValueQuotes (final String sQuotedValue)
    {
        final String sValue = sQuotedValue.substring(1, sQuotedValue.length()-1);
        return sValue;
    }




    private void SaveNamespaceDefinition (final String sShortName, final String sLongName)
    {
        maNamespaceMap.put(sShortName, sLongName);
    }




    private final XMLScanner maScanner;
    private final DocumentFactory<TokenType> maView;
    private final Style maTagStartEndStyle;
    private final Style maNamespaceNameStyle;
    private final Style maIdentifierStyle;
    private final Style maTextStyle;
    private final Style maAttributeValueStyle;
    private String msIndentation;
    private boolean mbIsAtBeginningOfLine;

    private String msLastNamespaceName;
    private String msLastName;
    private Map<String,String> maNamespaceMap;

    private final boolean mbStartNewLineBeforeEachAttribute = false;
    private final boolean mbStartNewLineBeforeNamespaceDefinition = true;
}
