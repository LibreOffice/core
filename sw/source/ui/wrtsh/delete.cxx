/*************************************************************************
 *
 *  $RCSfile: delete.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 16:04:30 $
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


#pragma hdrstop

#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _CRSSKIP_HXX
#include <crsskip.hxx>
#endif
#ifndef _SWCRSR_HXX
#include <swcrsr.hxx>
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
    const SwTableNode * pWasInTblNd = SwCrsrShell::IsCrsrInTbl();
    if( SwCrsrShell::IsSttPara())
    {
        /* If the cursor is at the beginning of a paragraph, try to step
           backwards. On failure we are done. */
        if( !SwCrsrShell::Left(1,CRSR_SKIP_CHARS) )
            return 0;

        /* If the cursor entered or left a table (or both) we are done. No step
           back. */
        if( SwCrsrShell::IsCrsrInTbl() != pWasInTblNd )
            return 0;

        OpenMark();
        SwCrsrShell::Right(1,CRSR_SKIP_CHARS);
        SwCrsrShell::SwapPam();
        bSwap = TRUE;
    }
    else
    {
        OpenMark();
        SwCrsrShell::Left(1,CRSR_SKIP_CHARS);
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

    const SwTableNode * pWasInTblNd = NULL;

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

        pWasInTblNd = IsCrsrInTbl();

        if( SEL_TXT & nSelection && SwCrsrShell::IsSttPara() &&
            SwCrsrShell::IsEndPara() && !IsCrsrInTbl() &&
            SwCrsrShell::Right(1,CRSR_SKIP_CHARS) )
        {
            BOOL bDelFull = 0 != IsCrsrInTbl();
            SwCrsrShell::Left(1,CRSR_SKIP_CHARS);

            if( bDelFull )
            {
                DelFullPara();
                UpdateAttr();
                break;
            }
        }
        else
        {
            /* #108049# Save the startnode of the current cell */
            const SwStartNode * pSNdOld;
            pSNdOld = GetSwCrsr()->GetNode()->
                FindTableBoxStartNode();

            if (SwCrsrShell::IsEndPara() &&
                SwCrsrShell::Right(1, CRSR_SKIP_CHARS))
            {
                if (IsCrsrInTbl() || (pWasInTblNd != IsCrsrInTbl()))
                {
                    /* #108049# Save the startnode of the current
                        cell. May be different to pSNdOld as we have
                        moved. */
                    const SwStartNode * pSNdNew = GetSwCrsr()
                        ->GetNode()->FindTableBoxStartNode();

                    /* #108049# Only move instead of deleting if we
                        have moved to a different cell */
                    if (pSNdOld != pSNdNew)
                        break;
                }

                SwCrsrShell::Left(1, CRSR_SKIP_CHARS);

            }
        }

        OpenMark();
        SwCrsrShell::Right(1,CRSR_SKIP_CELLS);
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




