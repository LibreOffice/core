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

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.UnsupportedEncodingException;
import java.util.Arrays;
import java.util.Vector;

public class XMLScanner
{
    XMLScanner (final InputStream aIn)
    {
        Reader aReader = null;
        try
        {
            aReader = new InputStreamReader(aIn, "UTF8");
        }
        catch (UnsupportedEncodingException e)
        {
            e.printStackTrace();
        }
        maIn = aReader;
        mnNextCharacter = 0;
        maTokens = new Vector<Token>();
        mnTokensReadIndex = 0;
        mbIsInsideTag = false;
        maTextBuffer = new int[1024];
    }




    public Token Next ()
    {
        while (maTokens.isEmpty())
            ProvideToken();

        final Token aToken = maTokens.get(mnTokensReadIndex);
        ++mnTokensReadIndex;
        if (mnTokensReadIndex >= maTokens.size())
        {
            maTokens.clear();
            mnTokensReadIndex = 0;
        }
        return aToken;
    }




    public Token Peek()
    {
        while (maTokens.isEmpty())
            ProvideToken();

        return maTokens.get(mnTokensReadIndex);
    }




    private void ProvideToken ()
    {
        final int nC = PeekCharacter();
        if (nC == -1)
        {
            AddToken(TokenType.EOF, "", mnOffset);
        }
        else if (mbIsInsideTag)
        {
            switch (Character.getType(nC))
            {
                case Character.DIRECTIONALITY_WHITESPACE:
                case Character.DIRECTIONALITY_LEFT_TO_RIGHT_OVERRIDE:
                    ScanWhitespace();
                    break;

                default:
                    switch(nC)
                    {
                        case '?':
                        case '/':
                        case '>':
                        case '=':
                        case ':':
                        case '-':
                            switch(ScanSymbol())
                            {
                                case TAG_END:
                                case INTRO_END:
                                case ELEMENT_END:
                                    mbIsInsideTag = false;
                                    break;
                                default:
                                    break;
                            }
                            break;

                        case '"':
                            ScanQuotedValue();
                            break;

                        default:
                            ScanIdentifier();
                            break;
                    }
            }
        }
        else
        {
            switch (Character.getType(PeekCharacter()))
            {
                case Character.DIRECTIONALITY_WHITESPACE:
                case Character.DIRECTIONALITY_LEFT_TO_RIGHT_OVERRIDE:
                    ScanWhitespace();
                    break;

                default:
                    if (nC == '<')
                    {
                        mbIsInsideTag = true;
                        ScanSymbol();
                    }
                    else
                    {
                        ScanText();
                    }
                    break;
            }
        }
    }




    Token NextNonWhitespaceToken ()
    {
        while(true)
        {
            final Token aToken = Next();
            if (aToken.Type != TokenType.WHITESPACE)
                return aToken;
        }
    }




    private TokenType ScanSymbol ()
    {
        final int nStartOffset = mnOffset;

        switch (PeekCharacter())
        {
            case -1:
                AddToken(TokenType.EOF, "", nStartOffset);
                return TokenType.EOF;

            case '<':
                GetNextCharacter();
                switch(PeekCharacter())
                {
                    case '/':
                        GetNextCharacter();
                        AddToken(TokenType.END_TAG_START, "</", nStartOffset);
                        break;

                    case '?':
                        GetNextCharacter();
                        AddToken(TokenType.INTRO_START, "<?", nStartOffset);
                        break;

                    case '!':
                        GetNextCharacter();
                        if (GetNextCharacter() != '-')
                            throw new RuntimeException("expected '-' after '<!'");
                        if (GetNextCharacter() != '-')
                            throw new RuntimeException("expected '-' after '<!-'");
                        AddToken(TokenType.COMMENT_START, "<!--", nStartOffset);
                        break;

                    default:
                        AddToken(TokenType.TAG_START, "<", nStartOffset);
                        break;
                }
                return maTokens.lastElement().Type;

            case '>':
                GetNextCharacter();
                AddToken(TokenType.TAG_END, ">", nStartOffset);
                return TokenType.TAG_END;

            case '/':
                GetNextCharacter();
                if (GetNextCharacter() != '>')
                    throw new RuntimeException("expected '>' after '/'");
                AddToken(TokenType.ELEMENT_END, "/>", nStartOffset);
                return TokenType.ELEMENT_END;

            case '?':
                GetNextCharacter();
                if (GetNextCharacter() != '>')
                    throw new RuntimeException("expected '>' after '?'");
                AddToken(TokenType.INTRO_END, "?>", nStartOffset);
                return TokenType.INTRO_END;

            case '-':
                GetNextCharacter();
                if (GetNextCharacter() != '-')
                    throw new RuntimeException("expected '-' after '-'");
                if (GetNextCharacter() != '>')
                    throw new RuntimeException("expected '>' after '--'");
                AddToken(TokenType.COMMENT_END, "-->", nStartOffset);
                return TokenType.COMMENT_END;

            case '=':
                GetNextCharacter();
                AddToken(TokenType.ATTRIBUTE_DEFINE, "=", nStartOffset);
                return TokenType.ATTRIBUTE_DEFINE;

            case ':':
                GetNextCharacter();
                AddToken(TokenType.COLON, ":", nStartOffset);
                return TokenType.COLON;

            default:
                    throw new RuntimeException(String.format(
                            "unexpected character '%c' of type %d",
                            PeekCharacter(),
                            Character.getType(PeekCharacter())));
        }
    }




    private boolean ScanIdentifier ()
    {
        final int nStartOffset = mnOffset;
        int nBufferWriteIndex = 0;

        while (true)
        {
            switch(Character.getType(PeekCharacter()))
            {
                default:
                case -1:
                    if (nBufferWriteIndex == 0)
                        throw new RuntimeException(
                                String.format(
                                        "missing identifier, got '%c' of type %d",
                                        PeekCharacter(),
                                        Character.getType(PeekCharacter())));
                    AddToken(
                        TokenType.IDENTIFIER,
                        new String(maTextBuffer, 0, nBufferWriteIndex),
                        nStartOffset);
                    return true;

                case Character.LOWERCASE_LETTER:
                case Character.UPPERCASE_LETTER:
                case Character.DECIMAL_DIGIT_NUMBER:
                    if (nBufferWriteIndex >= maTextBuffer.length)
                        maTextBuffer = Arrays.copyOf(maTextBuffer, maTextBuffer.length*2);
                    maTextBuffer[nBufferWriteIndex] = GetNextCharacter();
                    ++nBufferWriteIndex;
                    break;
            }
        }
    }




    private void ScanWhitespace ()
    {
        final StringBuffer aBuffer = new StringBuffer();
        final int nStartOffset = mnOffset;

        while (true)
        {
            switch(Character.getType(PeekCharacter()))
            {
                default:
                    if (aBuffer.length() > 0)
                        AddToken(TokenType.WHITESPACE, aBuffer.toString(), nStartOffset);
                    return;

                case -1:
                    AddToken(TokenType.WHITESPACE, aBuffer.toString(), nStartOffset);
                    AddToken(TokenType.EOF, "", nStartOffset);
                    return;

                case Character.DIRECTIONALITY_WHITESPACE:
                case Character.DIRECTIONALITY_LEFT_TO_RIGHT_OVERRIDE:
                    aBuffer.append((char)GetNextCharacter());
                    break;
            }
        }
    }




    private void ScanQuotedValue ()
    {
        if (PeekCharacter() == '"')
        {
            final int nStartOffset = mnOffset;
            int nBufferWriteIndex = 0;
            maTextBuffer[nBufferWriteIndex++] = GetNextCharacter();

            while (PeekCharacter() != '"')
            {
                // Make sure that there is enough space for this character and the end quote.
                if (nBufferWriteIndex >= maTextBuffer.length-1)
                    maTextBuffer = Arrays.copyOf(maTextBuffer, maTextBuffer.length*2);
                maTextBuffer[nBufferWriteIndex++] = GetNextCharacter();
            }

            maTextBuffer[nBufferWriteIndex++] = GetNextCharacter();

            AddToken(TokenType.ATTRIBUTE_VALUE, new String(maTextBuffer, 0, nBufferWriteIndex), nStartOffset);
        }
    }




    private void ScanText ()
    {
        final int nStartOffset = mnOffset;
        int nBufferWriteIndex = 0;
        maTextBuffer[nBufferWriteIndex++] = GetNextCharacter();

        while (PeekCharacter() != '<')
        {
            if (nBufferWriteIndex >= maTextBuffer.length)
                maTextBuffer = Arrays.copyOf(maTextBuffer, maTextBuffer.length*2);
            maTextBuffer[nBufferWriteIndex++] = GetNextCharacter();
        }

        AddToken(TokenType.TEXT, new String(maTextBuffer, 0, nBufferWriteIndex), nStartOffset);
    }




    private int GetNextCharacter ()
    {
        final int nC;
        if (mnNextCharacter != 0)
        {
            nC = mnNextCharacter;
            mnNextCharacter = 0;
        }
        else
        {
            try
            {
                nC = maIn.read();
            }
            catch (Exception e)
            {
                e.printStackTrace();
                return -1;
            }
        }
        ++mnOffset;
        return nC;
    }




    private int PeekCharacter ()
    {
        if (mnNextCharacter == 0)
        {
            try
            {
                mnNextCharacter = maIn.read();
            }
            catch (IOException e)
            {
                e.printStackTrace();
                mnNextCharacter = -1;
            }
        }
        return mnNextCharacter;
    }




    private void AddToken (
            final TokenType eType,
            final String sText,
            final int nOffset)
    {
        if (eType != TokenType.WHITESPACE)
            maTokens.add(new Token(eType, sText, nOffset));
    }




    private final Reader maIn;
    private int mnNextCharacter;
    private Vector<Token> maTokens;
    private int mnTokensReadIndex;
    private boolean mbIsInsideTag;
    private int[] maTextBuffer;
    private int mnOffset;
}
