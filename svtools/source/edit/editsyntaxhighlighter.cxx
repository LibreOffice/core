/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include <svtools/svmedit.hxx>
#include <vcl/event.hxx>
#include <vcl/xtextedt.hxx>
#include <vcl/textview.hxx>
#include <vcl/builderfactory.hxx>
#include <svtools/editsyntaxhighlighter.hxx>
#include <vcl/txtattr.hxx>


MultiLineEditSyntaxHighlight::MultiLineEditSyntaxHighlight( vcl::Window* pParent, WinBits nWinStyle,
    HighlighterLanguage aLanguage): MultiLineEdit(pParent,nWinStyle), aHighlighter(aLanguage)
{
    EnableUpdateData(300);
}

VCL_BUILDER_FACTORY(MultiLineEditSyntaxHighlight)

void MultiLineEditSyntaxHighlight::SetText(const OUString& rNewText)
{
    MultiLineEdit::SetText(rNewText);
    UpdateData();
}

void MultiLineEditSyntaxHighlight::DoBracketHilight(sal_uInt16 nKey)
{
    TextSelection aCurrentPos = GetTextView()->GetSelection();
    sal_Int32 nStartPos = aCurrentPos.GetStart().GetIndex();
    const sal_uInt32 nStartPara = aCurrentPos.GetStart().GetPara();
    sal_uInt16 nCount = 0;
    int nChar = -1;

    switch (nKey)
    {
        case '\'':  // no break
        case '"':
        {
            nChar = nKey;
            break;
        }
        case '}' :
        {
            nChar = '{';
            break;
        }
        case ')':
        {
            nChar = '(';
            break;
        }
        case ']':
        {
            nChar = '[';
            break;
        }
    }

    if (nChar == -1)
        return;

    sal_uInt32 nPara = nStartPara;
    do
    {
        if (nPara == nStartPara && nStartPos == 0)
            continue;

        OUString aLine( GetTextEngine()->GetText( nPara ) );

        if (aLine.isEmpty())
            continue;

        for (sal_Int32 i = (nPara==nStartPara) ? nStartPos-1 : aLine.getLength()-1; i>0; --i)
        {
            if (aLine[i] == nChar)
            {
                if (!nCount)
                {
                    GetTextEngine()->SetAttrib( TextAttribFontWeight( WEIGHT_ULTRABOLD ), nPara, i, i+1 );
                    GetTextEngine()->SetAttrib( TextAttribFontColor( Color(0,0,0) ), nPara, i, i+1 );
                    GetTextEngine()->SetAttrib( TextAttribFontWeight( WEIGHT_ULTRABOLD ), nStartPara, nStartPos, nStartPos );
                    GetTextEngine()->SetAttrib( TextAttribFontColor( Color(0,0,0) ), nStartPara, nStartPos, nStartPos );
                    return;
                }
                else
                    --nCount;
            }
            if (aLine[i] == nKey)
                ++nCount;
        }
    } while (nPara--);
}

bool MultiLineEditSyntaxHighlight::PreNotify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
        DoBracketHilight(rNEvt.GetKeyEvent()->GetCharCode());

    return MultiLineEdit::PreNotify(rNEvt);
}

Color MultiLineEditSyntaxHighlight::GetColorValue(TokenType aToken)
{
    Color aColor;
    switch (aHighlighter.GetLanguage())
    {
        case HighlighterLanguage::SQL:
        {
            switch (aToken)
            {
                case TokenType::Identifier: aColor = m_aColorConfig.GetColorValue(svtools::SQLIDENTIFIER).nColor; break;
                case TokenType::Number:     aColor = m_aColorConfig.GetColorValue(svtools::SQLNUMBER).nColor; break;
                case TokenType::String:     aColor = m_aColorConfig.GetColorValue(svtools::SQLSTRING).nColor; break;
                case TokenType::Operator:   aColor = m_aColorConfig.GetColorValue(svtools::SQLOPERATOR).nColor; break;
                case TokenType::Keywords:   aColor = m_aColorConfig.GetColorValue(svtools::SQLKEYWORD).nColor; break;
                case TokenType::Parameter:  aColor = m_aColorConfig.GetColorValue(svtools::SQLPARAMETER).nColor; break;
                case TokenType::Comment:    aColor = m_aColorConfig.GetColorValue(svtools::SQLCOMMENT).nColor; break;
                default:            aColor = Color(0,0,0);
            }
            break;
        }
        case HighlighterLanguage::Basic:
        {
            switch (aToken)
            {
                case TokenType::Identifier: aColor = Color(255,0,0); break;
                case TokenType::Comment:    aColor = Color(0,0,45); break;
                case TokenType::Number:     aColor = Color(204,102,204); break;
                case TokenType::String:     aColor = Color(0,255,45); break;
                case TokenType::Operator:   aColor = Color(0,0,100); break;
                case TokenType::Keywords:   aColor = Color(0,0,255); break;
                case TokenType::Error :     aColor = Color(0,255,255); break;
                default:            aColor = Color(0,0,0);
            }
            break;
        }
        default: aColor = Color(0,0,0);

    }
    return aColor;
}

void MultiLineEditSyntaxHighlight::UpdateData()
{
    // syntax highlighting
    // this must be possible improved by using notifychange correctly
    bool bTempModified = GetTextEngine()->IsModified();
    for (sal_uInt32 nLine=0; nLine < GetTextEngine()->GetParagraphCount(); ++nLine)
    {
        OUString aLine( GetTextEngine()->GetText( nLine ) );
        GetTextEngine()->RemoveAttribs( nLine );
        std::vector<HighlightPortion> aPortions;
        aHighlighter.getHighlightPortions( aLine, aPortions );
        for (auto const& portion : aPortions)
        {
            GetTextEngine()->SetAttrib( TextAttribFontColor( GetColorValue(portion.tokenType) ), nLine, portion.nBegin, portion.nEnd );
        }
    }
    GetTextView()->ShowCursor( false );
    GetTextEngine()->SetModified(bTempModified);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
