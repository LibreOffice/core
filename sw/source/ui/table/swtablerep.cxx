/*************************************************************************
 *
 *  $RCSfile: swtablerep.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 16:38:20 $
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

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
//CHINA001 #ifndef _SVX_BORDER_HXX //autogen
//CHINA001 #include <svx/border.hxx>
//CHINA001 #endif
//CHINA001 #ifndef _SVX_BACKGRND_HXX //autogen
//CHINA001 #include <svx/backgrnd.hxx>
//CHINA001 #endif
#ifndef _SVX_HTMLMODE_HXX
#include <svx/htmlmode.hxx>
#endif
#ifndef _SVX_KEEPITEM_HXX //autogen
#include <svx/keepitem.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_FRMDIRITEM_HXX
#include <svx/frmdiritem.hxx>
#endif

#ifndef _SVTOOLS_CTLOPTIONS_HXX
#include <svtools/ctloptions.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif

#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTTSPLT_HXX //autogen
#include <fmtlsplt.hxx>
#endif

#include <svx/htmlcfg.hxx>

#ifndef _FMTROWSPLT_HXX //autogen
#include <fmtrowsplt.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX
#include <svx/htmlmode.hxx>
#endif

#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif

#ifndef _TABLEPG_HXX
#include <tablepg.hxx>
#endif
#ifndef _TABLEMGR_HXX
#include <tablemgr.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _UIITEMS_HXX
#include <uiitems.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _TABLEDLG_HRC
#include <tabledlg.hrc>
#endif
#ifndef _TABLE_HRC
#include <table.hrc>
#endif
#ifndef _SWTABLEREP_HXX
#include "swtablerep.hxx"
#endif

#ifdef DEBUG_TBLDLG

void DbgTColumn(TColumn* pTColumn, USHORT nCount)
{
    for(USHORT i = 0; i < nCount; i++)
    {
        String sMsg(i);
        sMsg += pTColumn[i].bVisible ? " v " : " h ";
        sMsg += pTColumn[i].nWidth;
        DBG_ERROR(sMsg)
    }
}
#endif


#ifdef DEBUG_TBLDLG
#define DEBUG_TBLDLG_TCOLUMN(pTColumn, nCount) DbgTColumn(pTColumn, nCount)
#else
#define DEBUG_TBLDLG_TCOLUMN
#endif
/*-----------------20.08.96 09.43-------------------
--------------------------------------------------*/
SwTableRep::SwTableRep( const SwTabCols& rTabCol, BOOL bCplx )
    : nTblWidth(0),
    nSpace(0),
    nLeftSpace(0),
    nRightSpace(0),
    nAlign(0),
    nWidthPercent(0),
    bLineSelected(FALSE),
    bComplex(bCplx),
    bWidthChanged(FALSE),
    bColsChanged(FALSE)
{
    nAllCols = nColCount = rTabCol.Count();
    pTColumns = new TColumn[ nColCount + 1 ];
    SwTwips nStart = 0,
            nEnd;
    for( USHORT i = 0; i < nAllCols; ++i )
    {
        nEnd  = rTabCol[ i ] - rTabCol.GetLeft();
        pTColumns[ i ].nWidth = nEnd - nStart;
        pTColumns[ i ].bVisible = !rTabCol.IsHidden(i);
        if(!pTColumns[ i ].bVisible)
            nColCount --;
        nStart = nEnd;
    }
    pTColumns[ nAllCols ].nWidth = rTabCol.GetRight() - rTabCol.GetLeft() - nStart;
    pTColumns[ nAllCols ].bVisible = TRUE;
    nColCount++;
    nAllCols++;
}

/*-----------------20.08.96 09.43-------------------
--------------------------------------------------*/
SwTableRep::~SwTableRep()
{
    delete[] pTColumns;
}

/*-----------------20.08.96 13.33-------------------
--------------------------------------------------*/
BOOL SwTableRep::FillTabCols( SwTabCols& rTabCols ) const
{
    long nOldLeft = rTabCols.GetLeft(),
         nOldRight = rTabCols.GetRight();

    BOOL bSingleLine = FALSE;
    USHORT i;

    for ( i = 0; i < rTabCols.Count(); ++i )
        if(!pTColumns[i].bVisible)
        {
            bSingleLine = TRUE;
            break;
        }

DEBUG_TBLDLG_TCOLUMN(pTColumns, nAllCols);

    SwTwips nPos = 0;
    SwTwips nLeft = GetLeftSpace();
    rTabCols.SetLeft(nLeft);
    if(bSingleLine)
    {
        // die unsichtbaren Trenner werden aus den alten TabCols genommen
        // die sichtbaren kommen aus pTColumns
        TColumn*    pOldTColumns = new TColumn[nAllCols + 1];
        SwTwips nStart = 0,
                nEnd;
        for(USHORT i = 0; i < nAllCols - 1; i++)
        {
            nEnd  = rTabCols[i] - rTabCols.GetLeft();
            pOldTColumns[i].nWidth = nEnd - nStart;
            pOldTColumns[i].bVisible = !rTabCols.IsHidden(i);
            nStart = nEnd;
        }
        pOldTColumns[nAllCols - 1].nWidth = rTabCols.GetRight() - rTabCols.GetLeft() - nStart;
        pOldTColumns[nAllCols - 1].bVisible = TRUE;

DEBUG_TBLDLG_TCOLUMN(pOldTColumns, nAllCols);

        USHORT nOldPos = 0;
        USHORT nNewPos = 0;
        SwTwips nOld = 0;
        SwTwips nNew = 0;
        BOOL bOld = FALSE;
        BOOL bFirst = TRUE;
        i = 0;

        while ( i < nAllCols -1 )
        {
            while((bFirst || bOld ) && nOldPos < nAllCols )
            {
                nOld += pOldTColumns[nOldPos].nWidth;
                nOldPos++;
                if(!pOldTColumns[nOldPos - 1].bVisible)
                    break;
            }
            while((bFirst || !bOld ) && nNewPos < nAllCols )
            {
                nNew += pTColumns[nNewPos].nWidth;
                nNewPos++;
                if(pOldTColumns[nNewPos - 1].bVisible)
                    break;
            }
            bFirst = FALSE;
            // sie muessen sortiert eingefuegt werden
            bOld = nOld < nNew;
            nPos = USHORT(bOld ? nOld : nNew);
            rTabCols[i] = nPos + nLeft;
            rTabCols.SetHidden( i, bOld );
            i++;
        }
        rTabCols.SetRight(nLeft + nTblWidth);

        delete[] pOldTColumns;
    }
    else
    {
        for ( i = 0; i < nAllCols - 1; ++i )
        {
            nPos += pTColumns[i].nWidth;
            rTabCols[i] = nPos + rTabCols.GetLeft();
            rTabCols.SetHidden( i, !pTColumns[i].bVisible );
            rTabCols.SetRight(nLeft + pTColumns[nAllCols - 1].nWidth + nPos);
        }
    }

// Rundungsfehler abfangen
    if(Abs((long)nOldLeft - (long)rTabCols.GetLeft()) < 3)
        rTabCols.SetLeft(nOldLeft);

    if(Abs((long)nOldRight - (long)rTabCols.GetRight()) < 3)
        rTabCols.SetRight(nOldRight);

    if(GetRightSpace() >= 0 &&
            rTabCols.GetRight() > rTabCols.GetRightMax())
        rTabCols.SetRight(rTabCols.GetRightMax());
    return bSingleLine;
}