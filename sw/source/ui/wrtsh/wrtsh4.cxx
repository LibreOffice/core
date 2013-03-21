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
        SwCrsrShell::MovePara( fnParaCurr, fnParaStart );
    ClearMark();
        // If Mark was previously set, summarize.
    Combine();
    return true;
}
/*
 * Das Ende eines Wortes ist das Folgen von Trennzeichen auf
 * nicht-Trennzeichen.  Unter dem Ende eines Wortes wird
 * ebenfalls die Folge von Worttrennzeichen auf Interpunktions-
 * zeichen verstanden. Das Absatzende ist ebenfalls Wortende.
 */



bool SwWrtShell::_EndWrd()
{
    if ( IsEndWrd() )
        return true;
        // Create temporary cursor without selection.
    Push();
    ClearMark();
    if( !GoEndWord() )
            // not found --> go to the end of the paragraph.
        SwCrsrShell::MovePara(fnParaCurr, fnParaEnd);
    ClearMark();
        // If Mark was previously set, summarize.
    Combine();
    return true;
}



sal_Bool SwWrtShell::_NxtWrd()
{
    sal_Bool bRet = sal_False;
    while( IsEndPara() )               // If already at the end, then the next???
    {
        if(!SwCrsrShell::Right(1,CRSR_SKIP_CHARS))  // Document - end ??
        {
            Pop( sal_False );
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
            if( (!IsEndPara() && !SwCrsrShell::MovePara( fnParaCurr, fnParaEnd ) )
                || !SwCrsrShell::Right(1,CRSR_SKIP_CHARS) )
                break;
            bRet = IsStartWord();
        }
        else
            bRet = sal_True;
    }
    ClearMark();
    Combine();
    return bRet;
}

sal_Bool SwWrtShell::_PrvWrd()
{
    sal_Bool bRet = sal_False;
    while( IsSttPara() )
    {                            // if already at the beginning, then the next???
        if(!SwCrsrShell::Left(1,CRSR_SKIP_CHARS))
        {                        // Document - beginning ??
            Pop( sal_False );
            return bRet;
        }
        bRet = IsStartWord();
    }
    Push();
    ClearMark();
    while( !bRet )
    {
        if( !GoPrevWord() )
        {
            if( (!IsSttPara() && !SwCrsrShell::MovePara( fnParaCurr, fnParaStart ) )
                || !SwCrsrShell::Left(1,CRSR_SKIP_CHARS) )
                break;
            bRet = IsStartWord();
        }
        else
            bRet = sal_True;
    }
    ClearMark();
    Combine();
    return bRet;
}

// #i92468#
// method code of <SwWrtShell::_NxtWrd()> before fix for issue i72162
sal_Bool SwWrtShell::_NxtWrdForDelete()
{
    if ( IsEndPara() )
    {
        if ( !SwCrsrShell::Right(1,CRSR_SKIP_CHARS) )
        {
            Pop( sal_False );
            return sal_False;
        }
        return sal_True;
    }
    Push();
    ClearMark();
    if ( !GoNextWord() )
    {
        SwCrsrShell::MovePara( fnParaCurr, fnParaEnd );
    }
    ClearMark();
    Combine();
    return sal_True;
}

// method code of <SwWrtShell::_PrvWrd()> before fix for issue i72162
sal_Bool SwWrtShell::_PrvWrdForDelete()
{
    if ( IsSttPara() )
    {
        if ( !SwCrsrShell::Left(1,CRSR_SKIP_CHARS) )
        {
            Pop( sal_False );
            return sal_False;
        }
        return sal_True;
    }
    Push();
    ClearMark();
    if( !GoPrevWord() )
    {
        SwCrsrShell::MovePara( fnParaCurr, fnParaStart );
    }
    ClearMark();
    Combine();
    return sal_True;
}

sal_Bool SwWrtShell::_FwdSentence()
{
    Push();
    ClearMark();
    if(!SwCrsrShell::Right(1,CRSR_SKIP_CHARS))
    {
        Pop(sal_False);
        return 0;
    }
    if( !GoNextSentence() && !IsEndPara() )
        SwCrsrShell::MovePara(fnParaCurr, fnParaEnd);

    ClearMark();
    Combine();
    return 1;
}

sal_Bool SwWrtShell::_BwdSentence()
{
    Push();
    ClearMark();
    if(!SwCrsrShell::Left(1,CRSR_SKIP_CHARS))
    {
        Pop(sal_False);
        return 0;
    }
    if( !GoStartSentence()  && !IsSttPara() )
            // not found --> go to the beginning of the paragraph
        SwCrsrShell::MovePara( fnParaCurr, fnParaStart );
    ClearMark();
    Combine();
    return 1;
}

sal_Bool SwWrtShell::_FwdPara()
{
    Push();
    ClearMark();
    sal_Bool bRet = SwCrsrShell::MovePara(fnParaNext, fnParaStart);

    ClearMark();
    Combine();
    return bRet;
}

sal_Bool SwWrtShell::_BwdPara()
{
    Push();
    ClearMark();

    sal_Bool bRet = SwCrsrShell::MovePara(fnParaPrev, fnParaStart);
    if ( !bRet && !IsSttOfPara() )
    {
        SttPara();
    }

    ClearMark();
    Combine();
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
