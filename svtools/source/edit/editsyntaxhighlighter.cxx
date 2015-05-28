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
#include <vcl/xtextedt.hxx>
#include <vcl/builderfactory.hxx>
#include <svtools/editsyntaxhighlighter.hxx>
#include <vcl/txtattr.hxx>


MultiLineEditSyntaxHighlight::MultiLineEditSyntaxHighlight( vcl::Window* pParent, WinBits nWinStyle,
    HighlighterLanguage aLanguage): MultiLineEdit(pParent,nWinStyle), mbDoBracketHilight(true), aHighlighter(aLanguage)
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
    sal_Int32 nStartPara = aCurrentPos.GetStart().GetPara();
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

    if (nChar != -1)
    {
        for (sal_Int32 nPara = nStartPara; nPara >= 0; --nPara)
        {
            if (nStartPos == 0)
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
                        GetTextEngine()->SetAttrib( TextAttribFontWeight( WEIGHT_ULTRABOLD ), nPara, i, i+1, true );
                        GetTextEngine()->SetAttrib( TextAttribFontColor( Color(0,0,0) ), nPara, i, i+1, true );
                        GetTextEngine()->SetAttrib( TextAttribFontWeight( WEIGHT_ULTRABOLD ), nStartPara, nStartPos, nStartPos, true );
                        GetTextEngine()->SetAttrib( TextAttribFontColor( Color(0,0,0) ), nStartPara, nStartPos, nStartPos, true );
                        return;
                    }
                    else
                        --nCount;
                }
                if (aLine[i] == nKey)
                    ++nCount;
            }
        }
    }
}

bool MultiLineEditSyntaxHighlight::PreNotify( NotifyEvent& rNEvt )
{
    if ( mbDoBracketHilight && (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT) )
        DoBracketHilight(rNEvt.GetKeyEvent()->GetCharCode());

    return MultiLineEdit::PreNotify(rNEvt);
}

Color MultiLineEditSyntaxHighlight::GetColorValue(TokenTypes aToken)
{
    Color aColor;
    switch (aHighlighter.GetLanguage())
    {
        case HIGHLIGHT_SQL:
        {
            switch (aToken)
            {
                case TT_IDENTIFIER: aColor = (ColorData)m_aColorConfig.GetColorValue(svtools::SQLIDENTIFIER).nColor; break;
                case TT_NUMBER:     aColor = (ColorData)m_aColorConfig.GetColorValue(svtools::SQLNUMBER).nColor; break;
                case TT_STRING:     aColor = (ColorData)m_aColorConfig.GetColorValue(svtools::SQLSTRING).nColor; break;
                case TT_OPERATOR:   aColor = (ColorData)m_aColorConfig.GetColorValue(svtools::SQLOPERATOR).nColor; break;
                case TT_KEYWORDS:   aColor = (ColorData)m_aColorConfig.GetColorValue(svtools::SQLKEYWORD).nColor; break;
                case TT_PARAMETER:  aColor = (ColorData)m_aColorConfig.GetColorValue(svtools::SQLPARAMETER).nColor; break;
                case TT_COMMENT:    aColor = (ColorData)m_aColorConfig.GetColorValue(svtools::SQLCOMMENT).nColor; break;
                default:            aColor = Color(0,0,0);
            }
            break;
        }
        case HIGHLIGHT_BASIC:
        {
            switch (aToken)
            {
                case TT_IDENTIFIER: aColor = Color(255,0,0); break;
                case TT_COMMENT:    aColor = Color(0,0,45); break;
                case TT_NUMBER:     aColor = Color(204,102,204); break;
                case TT_STRING:     aColor = Color(0,255,45); break;
                case TT_OPERATOR:   aColor = Color(0,0,100); break;
                case TT_KEYWORDS:   aColor = Color(0,0,255); break;
                case TT_ERROR :     aColor = Color(0,255,255); break;
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
    for (sal_uLong nLine=0; nLine < GetTextEngine()->GetParagraphCount(); nLine++)
    {
        OUString aLine( GetTextEngine()->GetText( nLine ) );
        GetTextEngine()->RemoveAttribs( nLine, true );
        std::vector<HighlightPortion> aPortions;
        aHighlighter.getHighlightPortions( aLine, aPortions );
        for (std::vector<HighlightPortion>::iterator i(aPortions.begin());
             i != aPortions.end(); ++i)
        {
            GetTextEngine()->SetAttrib( TextAttribFontColor( GetColorValue(i->tokenType) ), nLine, i->nBegin, i->nEnd, true );
        }
    }
    GetTextView()->ShowCursor( false, true );
    GetTextEngine()->SetModified(bTempModified);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
