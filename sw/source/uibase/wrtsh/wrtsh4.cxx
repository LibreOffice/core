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

#include <wrtsh.hxx>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>

// Private methods, which move the cursor over search.
// The removal of the selection must be made on the level above.

// The beginning of a word is the follow of a
// non-delimiter to delimiter. Furthermore, the follow of
// non-sentence separators on sentence separator.
// The begin of paragraph is also the word beginning.

void SwWrtShell::SttWrd()
{
    if ( IsSttPara() )
        return;
        // Create temporary cursor without selection.
    Push();
    ClearMark();
    if( !GoStartWord() )
            // not found --> go to the beginning of the paragraph.
        SwCursorShell::MovePara( GoCurrPara, fnParaStart );
    ClearMark();
        // If Mark was previously set, summarize.
    Combine();
}

// The end of a word is the follow of separator to nonseparator.
// The end of a word is also the sequence of word separators to
// punctuation marks.
// The end of a paragraph is also the end of a word.

void SwWrtShell::EndWrd()
{
    if ( IsEndWrd() )
        return;
        // Create temporary cursor without selection.
    Push();
    ClearMark();
    if( !GoEndWord() )
            // not found --> go to the end of the paragraph.
        SwCursorShell::MovePara(GoCurrPara, fnParaEnd);
    ClearMark();
        // If Mark was previously set, summarize.
    Combine();
}

bool SwWrtShell::NxtWrd_()
{
    bool bRet = false;
    while( IsEndPara() )               // If already at the end, then the next???
    {
        if(!SwCursorShell::Right(1,SwCursorSkipMode::Chars))  // Document - end ??
        {
            Pop(SwCursorShell::PopMode::DeleteCurrent);
            return bRet;
        }
        bRet = IsStartWord();
    }
    Push();
    ClearMark();
    while( !bRet )
    {
        if( !GoNextWord() )
        {
            if( (!IsEndPara() && !SwCursorShell::MovePara( GoCurrPara, fnParaEnd ) )
                || !SwCursorShell::Right(1,SwCursorSkipMode::Chars) )
                break;
            bRet = IsStartWord();
        }
        else
            bRet = true;
    }
    ClearMark();
    Combine();
    return bRet;
}

bool SwWrtShell::PrvWrd_()
{
    bool bRet = false;
    while( IsSttPara() )
    {                            // if already at the beginning, then the next???
        if(!SwCursorShell::Left(1,SwCursorSkipMode::Chars))
        {                        // Document - beginning ??
            Pop(SwCursorShell::PopMode::DeleteCurrent);
            return bRet;
        }
        bRet = IsStartWord() || IsEndPara();
    }
    Push();
    ClearMark();
    while( !bRet )
    {
        if( !GoPrevWord() )
        {
            if( (!IsSttPara() && !SwCursorShell::MovePara( GoCurrPara, fnParaStart ) )
                || !SwCursorShell::Left(1,SwCursorSkipMode::Chars) )
                break;
            bRet = IsStartWord();
        }
        else
            bRet = true;
    }
    ClearMark();
    Combine();
    return bRet;
}

// #i92468#
// method code of <SwWrtShell::NxtWrd_()> before fix for issue i72162
bool SwWrtShell::NxtWrdForDelete()
{
    if ( IsEndPara() )
    {
        if ( !SwCursorShell::Right(1,SwCursorSkipMode::Chars) )
        {
            Pop(SwCursorShell::PopMode::DeleteCurrent);
            return false;
        }
        return true;
    }
    Push();
    ClearMark();
    if ( !GoNextWord() )
    {
        SwCursorShell::MovePara( GoCurrPara, fnParaEnd );
    }
    ClearMark();
    Combine();
    return true;
}

// method code of <SwWrtShell::PrvWrd_()> before fix for issue i72162
bool SwWrtShell::PrvWrdForDelete()
{
    if ( IsSttPara() )
    {
        if ( !SwCursorShell::Left(1,SwCursorSkipMode::Chars) )
        {
            Pop(SwCursorShell::PopMode::DeleteCurrent);
            return false;
        }
        return true;
    }
    Push();
    ClearMark();
    if( !GoPrevWord() )
    {
        SwCursorShell::MovePara( GoCurrPara, fnParaStart );
    }
    ClearMark();
    Combine();
    return true;
}

bool SwWrtShell::FwdSentence_()
{
    Push();
    ClearMark();
    if(!SwCursorShell::Right(1,SwCursorSkipMode::Chars))
    {
        Pop(SwCursorShell::PopMode::DeleteCurrent);
        return false;
    }
    if( !GoNextSentence() && !IsEndPara() )
        SwCursorShell::MovePara(GoCurrPara, fnParaEnd);

    ClearMark();
    Combine();
    return true;
}

bool SwWrtShell::EndSentence_()
{
    Push();
    ClearMark();
    if(!SwCursorShell::Right(1,SwCursorSkipMode::Chars))
    {
        Pop(SwCursorShell::PopMode::DeleteCurrent);
        return false;
    }
    if( !GoEndSentence() && !IsEndPara() )
        SwCursorShell::MovePara(GoCurrPara, fnParaEnd);

    ClearMark();
    Combine();
    return true;
}

bool SwWrtShell::BwdSentence_()
{
    Push();
    ClearMark();
    if(!SwCursorShell::Left(1,SwCursorSkipMode::Chars))
    {
        Pop(SwCursorShell::PopMode::DeleteCurrent);
        return false;
    }
    if( !GoStartSentence()  && !IsSttPara() )
            // not found --> go to the beginning of the paragraph
        SwCursorShell::MovePara( GoCurrPara, fnParaStart );
    ClearMark();
    Combine();
    return true;
}

bool SwWrtShell::FwdPara_()
{
    Push();
    ClearMark();
    bool bRet = SwCursorShell::MovePara(GoNextPara, fnParaStart);

    ClearMark();
    Combine();
    return bRet;
}

bool SwWrtShell::BwdPara_()
{
    Push();
    ClearMark();

    bool bRet = SwCursorShell::MovePara(GoPrevPara, fnParaStart);
    if ( !bRet && !IsSttOfPara() )
    {
        SttPara();
    }

    ClearMark();
    Combine();
    return bRet;
}

std::optional<OString> SwWrtShell::getLOKPayload(int nType, int nViewId) const
{
    switch(nType)
    {
        case LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR:
        case LOK_CALLBACK_INVALIDATE_VIEW_CURSOR:
            return GetVisibleCursor()->getLOKPayload(nType, nViewId);
        case LOK_CALLBACK_TEXT_SELECTION:
        case LOK_CALLBACK_TEXT_SELECTION_START:
        case LOK_CALLBACK_TEXT_SELECTION_END:
        case LOK_CALLBACK_TEXT_VIEW_SELECTION:
            return GetCursor_()->getLOKPayload(nType, nViewId);
    }
    abort();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
