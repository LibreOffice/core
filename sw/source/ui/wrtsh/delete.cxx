/*************************************************************************
 *
 *  $RCSfile: delete.cxx,v $
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

#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif


inline void SwWrtShell::OpenMark()
{
    StartAllAction();
    ResetCursorStack();
    KillPams();
    SetMark();
}

inline void SwWrtShell::CloseMark( BOOL bOkFlag )
{
    if( bOkFlag )
        UpdateAttr();
    else
        SwapPam();

    ClearMark();
    EndAllAction();
}


/*------------------------------------------------------------------------
 Beschreibung:  Zeile loeschen
------------------------------------------------------------------------*/



long SwWrtShell::DelLine()
{
    ACT_KONTEXT(this);
    ResetCursorStack();
        // alten Cursor merken
    Push();
    ClearMark();
    SwCrsrShell::LeftMargin();
    SetMark();
    SwCrsrShell::RightMargin();
//Warum soll hier noch ein Zeichen in der naechsten Zeile geloescht werden?
//  if(!IsEndOfPara())
//      SwCrsrShell::Right();
    long nRet = Delete();
    Pop(FALSE);
    if( nRet )
        UpdateAttr();
    return nRet;
}



long SwWrtShell::DelToStartOfLine()
{
    OpenMark();
    SwCrsrShell::LeftMargin();
    long nRet = Delete();
    CloseMark( 0 != nRet );
    return nRet;
}



long SwWrtShell::DelToEndOfLine()
{
    OpenMark();
    SwCrsrShell::RightMargin();
    long nRet = Delete();
    CloseMark( 0 != nRet );
    return 1;
}



long SwWrtShell::DelLeft()
{
    // wenns denn ein Fly ist, wech damit
    int nSelType = GetSelectionType();
    const int nCmp = SEL_FRM | SEL_GRF | SEL_OLE | SEL_DRW;
    if( nCmp & nSelType )
    {
        DelSelectedObj();
        LeaveSelFrmMode();
        UnSelectFrm();
        nSelType = GetSelectionType();
        if ( nCmp & nSelType )
        {
            EnterSelFrmMode();
            GotoNextFly();
        }
        return 1L;
    }

    // wenn eine Selektion existiert, diese loeschen.
    if ( IsSelection() )
    {
         //OS: wieder einmal Basic: ACT_KONTEXT muss vor
        //EnterStdMode verlassen werden!
        {
            ACT_KONTEXT(this);
            ResetCursorStack();
            Delete();
            UpdateAttr();
        }
        EnterStdMode();
        return 1L;
    }

    // JP 29.06.95: nie eine davor stehende Tabelle loeschen.
    BOOL bSwap = FALSE;
    if( SwCrsrShell::IsSttPara() && !SwCrsrShell::IsCrsrInTbl() )
    {
        if( !SwCrsrShell::Left() )
            return 0;
        if( SwCrsrShell::IsCrsrInTbl() )
        {
            SwCrsrShell::Right();
            return 0;
        }
        OpenMark();
        SwCrsrShell::Right();
        SwCrsrShell::SwapPam();
        bSwap = TRUE;
    }
    else
    {
        OpenMark();
        SwCrsrShell::Left();
    }
    long nRet = Delete();
    if( !nRet && bSwap )
        SwCrsrShell::SwapPam();
    CloseMark( 0 != nRet );
    return nRet;
}



long SwWrtShell::DelRight(BOOL bDelFrm)
{
        // werden verodert, wenn Tabellenselektion vorliegt;
        // wird hier auf SEL_TBL umgesetzt.
    long nRet = 0;
    int nSelection = GetSelectionType();
    if(nSelection & SwWrtShell::SEL_TBL_CELLS)
        nSelection = SwWrtShell::SEL_TBL;
    if(nSelection & SwWrtShell::SEL_TXT)
        nSelection = SwWrtShell::SEL_TXT;
    switch( nSelection & ~(SEL_BEZ) )
    {
    case SEL_TXT:
    case SEL_TBL:
    case SEL_NUM:
            //  wenn eine Selektion existiert, diese loeschen.
        if( IsSelection() )
        {
            //OS: wieder einmal Basic: ACT_KONTEXT muss vor
            //EnterStdMode verlassen werden!
            {
                ACT_KONTEXT(this);
                ResetCursorStack();
                Delete();
                UpdateAttr();
            }
            EnterStdMode();
            nRet = 1L;
            break;
        }

        if( SEL_TXT & nSelection && SwCrsrShell::IsSttPara() &&
            SwCrsrShell::IsEndPara() && !IsCrsrInTbl() &&
            SwCrsrShell::Right() )
        {
            BOOL bDelFull = 0 != IsCrsrInTbl();
            SwCrsrShell::Left();

            if( bDelFull )
            {
                DelFullPara();
                UpdateAttr();
                break;
            }
        }

        OpenMark();
        SwCrsrShell::Right();
        nRet = Delete();
        CloseMark( 0 != nRet );
        break;

    case SEL_FRM:
    case SEL_GRF:
    case SEL_OLE:
    case SEL_DRW:
    case SEL_DRW_TXT:
    case SEL_DRW_FORM:
        DelSelectedObj();
        LeaveSelFrmMode();
        UnSelectFrm();

        if( IsFrmSelected() )
        {
            nSelection = GetSelectionType();
            if ( SEL_FRM & nSelection || SEL_GRF & nSelection ||
                SEL_OLE & nSelection || SEL_DRW & nSelection )
            {
                EnterSelFrmMode();
                GotoNextFly();
            }
        }
        nRet = 1;
        break;
    }
    return nRet;
}



long SwWrtShell::DelToEndOfPara()
{
    ACT_KONTEXT(this);
    ResetCursorStack();
    Push();
    SetMark();
    if( !MovePara(fnParaCurr,fnParaEnd))
    {
        Pop(FALSE);
        return 0;
    }
    long nRet = Delete();
    Pop(FALSE);
    if( nRet )
        UpdateAttr();
    return nRet;
}



long SwWrtShell::DelToStartOfPara()
{
    ACT_KONTEXT(this);
    ResetCursorStack();
    Push();
    SetMark();
    if( !MovePara(fnParaCurr,fnParaStart))
    {
        Pop(FALSE);
        return 0;
    }
    long nRet = Delete();
    Pop(FALSE);
    if( nRet )
        UpdateAttr();
    return nRet;
}
/*
 * alle Loeschoperationen sollten mit Find statt mit
 * Nxt-/PrvDelim arbeiten, da letzteren mit Wrap Around arbeiten
 * -- das ist wohl nicht gewuenscht.
 */



long SwWrtShell::DelToStartOfSentence()
{
    if(IsStartOfDoc())
        return 0;
    OpenMark();
    long nRet = _BwdSentence() ? Delete() : 0;
    CloseMark( 0 != nRet );
    return nRet;
}



long SwWrtShell::DelToEndOfSentence()
{
    if(IsEndOfDoc())
        return 0;
    OpenMark();
    long nRet = _FwdSentence() ? Delete() : 0;
    CloseMark( 0 != nRet );
    return nRet;
}



long SwWrtShell::DelNxtWord()
{
    if(IsEndOfDoc())
        return 0;
    ACT_KONTEXT(this);
    ResetCursorStack();
    EnterStdMode();
    SetMark();
    if(IsEndWrd())
        _NxtWrd();
    if(IsSttWrd() || IsEndPara())
        _NxtWrd();
    else
        _EndWrd();

    long nRet = Delete();
    if( nRet )
        UpdateAttr();
    else
        SwapPam();
    ClearMark();
    return nRet;
}



long SwWrtShell::DelPrvWord()
{
    if(IsStartOfDoc())
        return 0;
    ACT_KONTEXT(this);
    ResetCursorStack();
    EnterStdMode();
    SetMark();
    if( !IsSttWrd() || !_PrvWrd() )
    {
        if( IsEndWrd() )
        {
            if( _PrvWrd() )
            {
                // skip over all-1 spaces
                short n = -1;
                while( ' ' == GetChar( FALSE, n ))
                    --n;

                if( ++n )
                    ExtendSelection( FALSE, -n );
            }
        }
        else if( IsSttPara())
            _PrvWrd();
        else
            _SttWrd();
    }
    long nRet = Delete();
    if( nRet )
        UpdateAttr();
    else
        SwapPam();
    ClearMark();
    return nRet;
}

/*************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.70  2000/09/18 16:06:26  willem.vandorp
      OpenOffice header added.

      Revision 1.69  2000/04/10 08:39:49  jp
      Bug #74881#: DelPrvWord

      Revision 1.68  1999/04/01 13:21:50  JP
      Bug #64298#: DelRight - auch Controls usw. entfernen


      Rev 1.67   01 Apr 1999 15:21:50   JP
   Bug #64298#: DelRight - auch Controls usw. entfernen

      Rev 1.66   05 Feb 1999 15:07:54   JP
   Bug #61386#: DelRight unnoetigen Befehl entfernt

      Rev 1.65   19 Jan 1999 22:53:46   JP
   Task #58677#: Crsr in Readonly Bereichen zulassen

      Rev 1.64   15 Jan 1999 15:43:28   JP
   Bug #60917#: DelPrevWord korrigiert

      Rev 1.63   09 Jun 1998 15:34:32   OM
   VC-Controls entfernt

      Rev 1.62   21 Apr 1998 17:36:28   OM
   #49630 Keine Rahmenselektion nach Loeschung von Zeichenobjekten

      Rev 1.61   24 Nov 1997 14:35:06   MA
   includes

      Rev 1.60   03 Nov 1997 14:02:54   MA
   precomp entfernt

      Rev 1.59   06 Mar 1997 09:22:58   OM
   sel_drw_ctrl ergaenzt

      Rev 1.58   05 Feb 1997 17:01:04   JP
   Bug #35638#: DelRight in Zellen

      Rev 1.57   03 Feb 1997 20:41:30   JP
   DelRight: leeren TextNode vor Tabellen loeschen

      Rev 1.56   22 Jan 1997 11:55:56   MA
   opt: bSelection entfernt

      Rev 1.55   29 Aug 1996 09:26:04   OS
   includes

      Rev 1.54   22 Jan 1996 16:28:50   OS
   DelRight/Left: ACT_KONTEXT muss vor EnterStdMode enden wg. Shellumschaltung aus Basic Bug #24529#

      Rev 1.53   24 Nov 1995 16:59:04   OM
   PCH->PRECOMPILED

      Rev 1.52   22 Sep 1995 11:45:48   JP
   DelLeft: & und nicht | abfragen

      Rev 1.51   10 Aug 1995 13:04:44   OS
   DelLine: nur bis zum Zeilenende markieren und loeschen

      Rev 1.50   05 Jul 1995 19:14:52   JP
   Delete: mit Backspace keine Tabelle loeschen

      Rev 1.49   29 Jun 1995 20:10:12   JP
   DelLeft: keine Tabellen loeschen mit BACKSPACE

      Rev 1.48   19 Jun 1995 09:53:44   MA
   fix(13180): Del fuer Zeichenobjekte.

      Rev 1.47   05 Apr 1995 19:47:38   OM
   Bug in NumShell mit Delete gefixt

      Rev 1.46   20 Dec 1994 11:53:44   MA
   SS Drawing.

      Rev 1.45   15 Dec 1994 20:32:04   SWG
   *ARR* Ersetzungen, svmem, __far_data etc.

*************************************************************************/



