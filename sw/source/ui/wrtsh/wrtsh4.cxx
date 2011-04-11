/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <wrtsh.hxx>
#include <crsskip.hxx>


/*
* private Methoden, die den Cursor ueber Suchen bewegen. Das
* Aufheben der Selektion muss auf der Ebene darueber erfolgen.
*/

/*
* Der Anfang eines Wortes ist das Folgen eines nicht-
* Trennzeichens auf Trennzeichen. Ferner das Folgen von
* nicht-Satztrennern auf Satztrenner. Der Absatzanfang ist
* ebenfalls Wortanfang.
*/


sal_Bool SwWrtShell::_SttWrd()
{
    if ( IsSttPara() )
        return 1;
        /*
            * temporaeren Cursor ohne Selektion erzeugen
            */
    Push();
    ClearMark();
    if( !GoStartWord() )
            // nicht gefunden --> an den Absatzanfang
        SwCrsrShell::MovePara( fnParaCurr, fnParaStart );
    ClearMark();
        // falls vorher Mark gesetzt war, zusammenfassen
    Combine();
    return 1;
}
/*
 * Das Ende eines Wortes ist das Folgen von Trennzeichen auf
 * nicht-Trennzeichen.  Unter dem Ende eines Wortes wird
 * ebenfalls die Folge von Worttrennzeichen auf Interpunktions-
 * zeichen verstanden. Das Absatzende ist ebenfalls Wortende.
 */



sal_Bool SwWrtShell::_EndWrd()
{
    if ( IsEndWrd() )
        return 1;
        // temporaeren Cursor ohne Selektion erzeugen
    Push();
    ClearMark();
    if( !GoEndWord() )
            // nicht gefunden --> an das Absatz Ende
        SwCrsrShell::MovePara(fnParaCurr, fnParaEnd);
    ClearMark();
        // falls vorher Mark gesetzt war, zusammenfassen
    Combine();
    return 1;
}



sal_Bool SwWrtShell::_NxtWrd()
{
    sal_Bool bRet = sal_False;
    while( IsEndPara() )                // wenn schon am Ende, dann naechsten ???
    {
        if(!SwCrsrShell::Right(1,CRSR_SKIP_CHARS))  // Document - Ende ??
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
    {                               // wenn schon am Anfang, dann naechsten ???
        if(!SwCrsrShell::Left(1,CRSR_SKIP_CHARS))
        {                           // Document - Anfang ??
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

// --> OD 2008-08-06 #i92468#
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
// <--


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
    if(IsSttPara())
    {
        Pop();
        return 1;
    }
    if( !GoPrevSentence()  && !IsSttPara() )
            // nicht gefunden --> an den Absatz Anfang
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
    // <--

    ClearMark();
    Combine();
    return bRet;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
