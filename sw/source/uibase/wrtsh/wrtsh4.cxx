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
#include <crsskip.hxx>

// Private methods, which move the cursor over search.
// The removal of the selection must be made on the level above.

// The beginning of a word is the follow of a
// non-delimiter to delimiter. Furthermore, the follow of
// non-sentence separators on sentence separator.
// The begin of paragraph is also the word beginning.

bool SwWrtShell::_SttWrd()
{
    if ( IsSttPara() )
        return true;
        // Create temporary cursor without selection.
    Push();
    ClearMark();
    if( !GoStartWord() )
            // not found --> go to the beginning of the paragraph.
        SwCursorShell::MovePara( fnParaCurr, fnParaStart );
    ClearMark();
        // If Mark was previously set, summarize.
    Combine();
    return true;
}

// The end of a word is the follow of separator to nonseparator.
// The end of a word is also the sequence of wordseparators to
// punctuation marks.
// The end of a paragraph is also the end of a word.

bool SwWrtShell::_EndWrd()
{
    if ( IsEndWrd() )
        return true;
        // Create temporary cursor without selection.
    Push();
    ClearMark();
    if( !GoEndWord() )
            // not found --> go to the end of the paragraph.
        SwCursorShell::MovePara(fnParaCurr, fnParaEnd);
    ClearMark();
        // If Mark was previously set, summarize.
    Combine();
    return true;
}

bool SwWrtShell::_NxtWrd()
{
    bool bRet = false;
    while( IsEndPara() )               // If already at the end, then the next???
    {
        if(!SwCursorShell::Right(1,CRSR_SKIP_CHARS))  // Document - end ??
        {
            Pop( false );
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
            if( (!IsEndPara() && !SwCursorShell::MovePara( fnParaCurr, fnParaEnd ) )
                || !SwCursorShell::Right(1,CRSR_SKIP_CHARS) )
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

bool SwWrtShell::_PrvWrd()
{
    bool bRet = false;
    while( IsSttPara() )
    {                            // if already at the beginning, then the next???
        if(!SwCursorShell::Left(1,CRSR_SKIP_CHARS))
        {                        // Document - beginning ??
            Pop( false );
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
            if( (!IsSttPara() && !SwCursorShell::MovePara( fnParaCurr, fnParaStart ) )
                || !SwCursorShell::Left(1,CRSR_SKIP_CHARS) )
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
// method code of <SwWrtShell::_NxtWrd()> before fix for issue i72162
bool SwWrtShell::_NxtWrdForDelete()
{
    if ( IsEndPara() )
    {
        if ( !SwCursorShell::Right(1,CRSR_SKIP_CHARS) )
        {
            Pop( false );
            return false;
        }
        return true;
    }
    Push();
    ClearMark();
    if ( !GoNextWord() )
    {
        SwCursorShell::MovePara( fnParaCurr, fnParaEnd );
    }
    ClearMark();
    Combine();
    return true;
}

// method code of <SwWrtShell::_PrvWrd()> before fix for issue i72162
bool SwWrtShell::_PrvWrdForDelete()
{
    if ( IsSttPara() )
    {
        if ( !SwCursorShell::Left(1,CRSR_SKIP_CHARS) )
        {
            Pop( false );
            return false;
        }
        return true;
    }
    Push();
    ClearMark();
    if( !GoPrevWord() )
    {
        SwCursorShell::MovePara( fnParaCurr, fnParaStart );
    }
    ClearMark();
    Combine();
    return true;
}

bool SwWrtShell::_FwdSentence()
{
    Push();
    ClearMark();
    if(!SwCursorShell::Right(1,CRSR_SKIP_CHARS))
    {
        Pop(false);
        return false;
    }
    if( !GoNextSentence() && !IsEndPara() )
        SwCursorShell::MovePara(fnParaCurr, fnParaEnd);

    ClearMark();
    Combine();
    return true;
}

bool SwWrtShell::_BwdSentence()
{
    Push();
    ClearMark();
    if(!SwCursorShell::Left(1,CRSR_SKIP_CHARS))
    {
        Pop(false);
        return false;
    }
    if( !GoStartSentence()  && !IsSttPara() )
            // not found --> go to the beginning of the paragraph
        SwCursorShell::MovePara( fnParaCurr, fnParaStart );
    ClearMark();
    Combine();
    return true;
}

bool SwWrtShell::_FwdPara()
{
    Push();
    ClearMark();
    bool bRet = SwCursorShell::MovePara(fnParaNext, fnParaStart);

    ClearMark();
    Combine();
    return bRet;
}

bool SwWrtShell::_BwdPara()
{
    Push();
    ClearMark();

    bool bRet = SwCursorShell::MovePara(fnParaPrev, fnParaStart);
    if ( !bRet && !IsSttOfPara() )
    {
        SttPara();
    }

    ClearMark();
    Combine();
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
