/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wrtsh4.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:53:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _CRSSKIP_HXX
#include <crsskip.hxx>
#endif


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


BOOL SwWrtShell::_SttWrd()
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



BOOL SwWrtShell::_EndWrd()
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



BOOL SwWrtShell::_NxtWrd()
{
    if( IsEndPara() )               // wenn schon am Ende, dann naechsten ???
    {
        if(!SwCrsrShell::Right(1,CRSR_SKIP_CHARS))  // Document - Ende ??
        {
            Pop( FALSE );
            return 0L;
        }
        return 1;
    }
    Push();
    ClearMark();
    if( !GoNextWord() )
            // nicht gefunden --> das AbsatzEnde ist Ende vom Wort
        SwCrsrShell::MovePara( fnParaCurr, fnParaEnd );
    ClearMark();
    Combine();
    return 1;
}



BOOL SwWrtShell::_PrvWrd()
{
    if(IsSttPara())
    {                               // wenn schon am Anfang, dann naechsten ???
        if(!SwCrsrShell::Left(1,CRSR_SKIP_CHARS))
        {                           // Document - Anfang ??
            Pop( FALSE );
            return 0;
        }
        return 1;
    }
    Push();
    ClearMark();
    if( !GoPrevWord() )
            // nicht gefunden --> an den Absatz Anfang
        SwCrsrShell::MovePara( fnParaCurr, fnParaStart );
    ClearMark();
    Combine();
    return 1;
}



BOOL SwWrtShell::_FwdSentence()
{
    Push();
    ClearMark();
    if(!SwCrsrShell::Right(1,CRSR_SKIP_CHARS))
    {
        Pop(FALSE);
        return 0;
    }
    if( !GoNextSentence() && !IsEndPara() )
        SwCrsrShell::MovePara(fnParaCurr, fnParaEnd);

    ClearMark();
    Combine();
    return 1;
}



BOOL SwWrtShell::_BwdSentence()
{
    Push();
    ClearMark();
    if(!SwCrsrShell::Left(1,CRSR_SKIP_CHARS))
    {
        Pop(FALSE);
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


BOOL SwWrtShell::_FwdPara()
{
    Push();
    ClearMark();
    if(!SwCrsrShell::Right(1,CRSR_SKIP_CHARS))
    {
        Pop(FALSE);
        return 0;
    }
    SwCrsrShell::Left(1,CRSR_SKIP_CHARS);
    BOOL bRet = SwCrsrShell::MovePara(fnParaNext, fnParaStart);

    ClearMark();
    Combine();
    return bRet;
}


BOOL SwWrtShell::_BwdPara()
{
    Push();
    ClearMark();
    if(!SwCrsrShell::Left(1,CRSR_SKIP_CHARS))
    {
        Pop(FALSE);
        return 0;
    }
    SwCrsrShell::Right(1,CRSR_SKIP_CHARS);
    if(!IsSttOfPara())
        SttPara();
    BOOL bRet = SwCrsrShell::MovePara(fnParaPrev, fnParaStart);

    ClearMark();
    Combine();
    return bRet;
}


