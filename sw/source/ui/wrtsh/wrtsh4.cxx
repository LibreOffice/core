/*************************************************************************
 *
 *  $RCSfile: wrtsh4.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:53 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include "wrtsh.hxx"


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


FASTBOOL SwWrtShell::_SttWrd()
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



FASTBOOL SwWrtShell::_EndWrd()
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



FASTBOOL SwWrtShell::_NxtWrd()
{
    if( IsEndPara() )               // wenn schon am Ende, dann naechsten ???
    {
        if(!SwCrsrShell::Right())   // Document - Ende ??
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



FASTBOOL SwWrtShell::_PrvWrd()
{
    if(IsSttPara())
    {                               // wenn schon am Anfang, dann naechsten ???
        if(!SwCrsrShell::Left())
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



FASTBOOL SwWrtShell::_FwdSentence()
{
    Push();
    ClearMark();
    if(!SwCrsrShell::Right())
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



FASTBOOL SwWrtShell::_BwdSentence()
{
    Push();
    ClearMark();
    if(!SwCrsrShell::Left())
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


FASTBOOL SwWrtShell::_FwdPara()
{
    Push();
    ClearMark();
    if(!SwCrsrShell::Right())
    {
        Pop(FALSE);
        return 0;
    }
    SwCrsrShell::Left();
    SwCrsrShell::MovePara(fnParaNext, fnParaStart);

    ClearMark();
    Combine();
    return 1;
}


FASTBOOL SwWrtShell::_BwdPara()
{
    Push();
    ClearMark();
    if(!SwCrsrShell::Left())
    {
        Pop(FALSE);
        return 0;
    }
    SwCrsrShell::Right();
    if(!IsSttOfPara())
        SttPara();
    SwCrsrShell::MovePara(fnParaPrev, fnParaStart);

    ClearMark();
    Combine();
    return 1;
}
/*************************************************************************

   $Log: not supported by cvs2svn $
   Revision 1.8  2000/09/18 16:06:27  willem.vandorp
   OpenOffice header added.

   Revision 1.7  1998/07/13 08:58:02  JP
   Bug #52635#: _Fwd/_BwdSentence: End-/SttPara beachten


      Rev 1.6   13 Jul 1998 10:58:02   JP
   Bug #52635#: _Fwd/_BwdSentence: End-/SttPara beachten

      Rev 1.5   24 Nov 1997 14:35:04   MA
   includes

      Rev 1.4   03 Nov 1997 14:02:54   MA
   precomp entfernt

      Rev 1.3   29 Aug 1996 09:25:56   OS
   includes

      Rev 1.2   07 Jun 1996 09:14:06   OS
   Spruenge zum vorherigen/folgenden Absatz: auch bei leerem Absatz richtig springen Bug #28382#

      Rev 1.1   14 May 1996 13:35:32   HJS
   wer braucht schon includes...

      Rev 1.0   14 May 1996 13:29:20   HJS
   aus wrtsh1 w.g. clooks

      Rev 1.0   12 May 1996 12:41:56   HJS
   clooks

*************************************************************************/



