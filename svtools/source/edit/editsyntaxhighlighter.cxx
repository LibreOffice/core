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
#include <svtools/editsyntaxhighlighter.hxx>
#include <vcl/txtattr.hxx>


MultiLineEditSyntaxHighlight::MultiLineEditSyntaxHighlight( Window* pParent, WinBits nWinStyle,
    HighlighterLanguage aLanguage): MultiLineEdit(pParent,nWinStyle), mbDoBracketHilight(true)
{
    EnableUpdateData(300);
    aHighlighter.initialize( aLanguage );
}

MultiLineEditSyntaxHighlight::MultiLineEditSyntaxHighlight( Window* pParent, const ResId& rResId ,
    HighlighterLanguage aLanguage): MultiLineEdit(pParent,rResId), mbDoBracketHilight(true)
{
    EnableUpdateData(300);
    aHighlighter.initialize( aLanguage );
}

MultiLineEditSyntaxHighlight::~MultiLineEditSyntaxHighlight()
{
}

void MultiLineEditSyntaxHighlight::SetText(const OUString& rNewText)
{
    MultiLineEdit::SetText(rNewText);
    UpdateData();
}

void MultiLineEditSyntaxHighlight::DoBracketHilight(sal_uInt16 aKey)
{
    TextSelection aCurrentPos = GetTextView()->GetSelection();
    xub_StrLen aStartPos  = aCurrentPos.GetStart().GetIndex();
    sal_uLong nStartPara = aCurrentPos.GetStart().GetPara();
    sal_uInt16 aCount = 0;
    int aChar = -1;

    switch (aKey)
    {
        case '\'':  // no break
        case '"':
        {
            aChar = aKey;
            break;
        }
        case '}' :
        {
            aChar = '{';
            break;
        }
        case ')':
        {
            aChar = '(';
            break;
        }
        case ']':
        {
            aChar = '[';
            break;
        }
    }

    if (aChar != -1)
    {
        for (long aPara =nStartPara; aPara>=0;--aPara)
        {
            if ( aStartPos == 0 )
                continue;

            String aLine( GetTextEngine()->GetText( aPara ) );
            for (sal_uInt16 i = ((unsigned long)aPara==nStartPara) ? aStartPos-1 : (sal_uInt16)(aLine.Len()-1); i>0; --i)
            {
                if (aLine.GetChar(i)==aChar)
                {
                    if (!aCount)
                    {
                        GetTextEngine()->SetAttrib( TextAttribFontWeight( WEIGHT_ULTRABOLD ), aPara, i, i+1, sal_True );
                        GetTextEngine()->SetAttrib( TextAttribFontColor( Color(0,0,0) ), aPara, i, i+1, sal_True );
                        GetTextEngine()->SetAttrib( TextAttribFontWeight( WEIGHT_ULTRABOLD ), nStartPara, aStartPos, aStartPos, sal_True );
                        GetTextEngine()->SetAttrib( TextAttribFontColor( Color(0,0,0) ), nStartPara, aStartPos, aStartPos, sal_True );
                        return;
                    }
                    else
                        aCount--;
                }
                if (aLine.GetChar(i)==aKey)
                    aCount++;
            }
        }
    }
}

long MultiLineEditSyntaxHighlight::PreNotify( NotifyEvent& rNEvt )
{
    if ( mbDoBracketHilight && (rNEvt.GetType() == EVENT_KEYINPUT) )
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
    sal_Bool bTempModified = GetTextEngine()->IsModified();
    for (unsigned int nLine=0; nLine < GetTextEngine()->GetParagraphCount(); nLine++)
    {
        String aLine( GetTextEngine()->GetText( nLine ) );
        aHighlighter.notifyChange( nLine, 0, &aLine, 1 );

        GetTextEngine()->RemoveAttribs( nLine, sal_True );
        HighlightPortions aPortions;
        aHighlighter.getHighlightPortions( nLine, aLine, aPortions );
        for ( size_t i = 0; i < aPortions.size(); i++ )
        {
            HighlightPortion& r = aPortions[i];
            GetTextEngine()->SetAttrib( TextAttribFontColor( GetColorValue(r.tokenType) ), nLine, r.nBegin, r.nEnd, sal_True );
        }
    }
    GetTextView()->ShowCursor( false, true );
    GetTextEngine()->SetModified(bTempModified);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
