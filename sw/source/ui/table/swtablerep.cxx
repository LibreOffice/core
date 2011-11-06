/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <hintids.hxx>
#include <tools/list.hxx>
#include <vcl/msgbox.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svx/htmlmode.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/brkitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svl/ctloptions.hxx>
#include <swmodule.hxx>
#include <fmtornt.hxx>
#include <fmtpdsc.hxx>
#include <fmtlsplt.hxx>

#include <svtools/htmlcfg.hxx>
#include <fmtrowsplt.hxx>
#include <svx/htmlmode.hxx>

#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#include <wrtsh.hxx>
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#include <viewopt.hxx>
#include <uitool.hxx>
#include <frmatr.hxx>

#ifndef _TABLEPG_HXX
#include <tablepg.hxx>
#endif
#include <tablemgr.hxx>
#include <pagedesc.hxx>
#include <poolfmt.hxx>
#include <SwStyleNameMapper.hxx>

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _TABLEDLG_HRC
#include <tabledlg.hrc>
#endif
#ifndef _TABLE_HRC
#include <table.hrc>
#endif
#include "swtablerep.hxx"

#ifdef DEBUG_TBLDLG

void DbgTColumn(TColumn* pTColumn, sal_uInt16 nCount)
{
    for(sal_uInt16 i = 0; i < nCount; i++)
    {
        String sMsg(i);
        sMsg += pTColumn[i].bVisible ? " v " : " h ";
        sMsg += pTColumn[i].nWidth;
        DBG_ERROR(sMsg)
    }
}
#endif


/*-----------------20.08.96 09.43-------------------
--------------------------------------------------*/
SwTableRep::SwTableRep( const SwTabCols& rTabCol, sal_Bool bCplx )
    :
    nTblWidth(0),
    nSpace(0),
    nLeftSpace(0),
    nRightSpace(0),
    nAlign(0),
    nWidthPercent(0),
    bComplex(bCplx),
    bLineSelected(sal_False),
    bWidthChanged(sal_False),
    bColsChanged(sal_False)
{
    nAllCols = nColCount = rTabCol.Count();
    pTColumns = new TColumn[ nColCount + 1 ];
    SwTwips nStart = 0,
            nEnd;
    for( sal_uInt16 i = 0; i < nAllCols; ++i )
    {
        nEnd  = rTabCol[ i ] - rTabCol.GetLeft();
        pTColumns[ i ].nWidth = nEnd - nStart;
        pTColumns[ i ].bVisible = !rTabCol.IsHidden(i);
        if(!pTColumns[ i ].bVisible)
            nColCount --;
        nStart = nEnd;
    }
    pTColumns[ nAllCols ].nWidth = rTabCol.GetRight() - rTabCol.GetLeft() - nStart;
    pTColumns[ nAllCols ].bVisible = sal_True;
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
sal_Bool SwTableRep::FillTabCols( SwTabCols& rTabCols ) const
{
    long nOldLeft = rTabCols.GetLeft(),
         nOldRight = rTabCols.GetRight();

    sal_Bool bSingleLine = sal_False;
    sal_uInt16 i;

    for ( i = 0; i < rTabCols.Count(); ++i )
        if(!pTColumns[i].bVisible)
        {
            bSingleLine = sal_True;
            break;
        }

#ifdef DEBUG_TBLDLG
#define DbgTColumn(pTColumns, nAllCols);
#endif

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
        for(i = 0; i < nAllCols - 1; i++)
        {
            nEnd  = rTabCols[i] - rTabCols.GetLeft();
            pOldTColumns[i].nWidth = nEnd - nStart;
            pOldTColumns[i].bVisible = !rTabCols.IsHidden(i);
            nStart = nEnd;
        }
        pOldTColumns[nAllCols - 1].nWidth = rTabCols.GetRight() - rTabCols.GetLeft() - nStart;
        pOldTColumns[nAllCols - 1].bVisible = sal_True;

#ifdef DEBUG_TBLDLG
#define DbgTColumn(pOldTColumns, nAllCols);
#endif

        sal_uInt16 nOldPos = 0;
        sal_uInt16 nNewPos = 0;
        SwTwips nOld = 0;
        SwTwips nNew = 0;
        sal_Bool bOld = sal_False;
        sal_Bool bFirst = sal_True;
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
            bFirst = sal_False;
            // sie muessen sortiert eingefuegt werden
            bOld = nOld < nNew;
            nPos = sal_uInt16(bOld ? nOld : nNew);
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
