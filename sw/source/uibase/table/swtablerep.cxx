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

#include <hintids.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svl/ctloptions.hxx>
#include <swmodule.hxx>
#include <fmtornt.hxx>
#include <fmtpdsc.hxx>
#include <fmtlsplt.hxx>

#include <svtools/htmlcfg.hxx>
#include <fmtrowsplt.hxx>
#include <sfx2/htmlmode.hxx>

#include <docsh.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <viewopt.hxx>
#include <uitool.hxx>
#include <frmatr.hxx>

#include "tablepg.hxx"
#include <tablemgr.hxx>
#include <pagedesc.hxx>
#include <poolfmt.hxx>
#include <SwStyleNameMapper.hxx>

#include <cmdid.h>
#include <swtablerep.hxx>
#include <memory>

SwTableRep::SwTableRep( const SwTabCols& rTabCol )
    :
    m_nTableWidth(0),
    m_nSpace(0),
    m_nLeftSpace(0),
    m_nRightSpace(0),
    m_nAlign(0),
    m_nWidthPercent(0),
    m_bLineSelected(false),
    m_bWidthChanged(false),
    m_bColsChanged(false)
{
    m_nAllCols = m_nColCount = rTabCol.Count();
    m_pTColumns.reset( new TColumn[ m_nColCount + 1 ] );
    SwTwips nStart = 0,
            nEnd;
    for( sal_uInt16 i = 0; i < m_nAllCols; ++i )
    {
        nEnd  = rTabCol[ i ] - rTabCol.GetLeft();
        m_pTColumns[ i ].nWidth = nEnd - nStart;
        m_pTColumns[ i ].bVisible = !rTabCol.IsHidden(i);
        if(!m_pTColumns[ i ].bVisible)
            m_nColCount --;
        nStart = nEnd;
    }
    m_pTColumns[ m_nAllCols ].nWidth = rTabCol.GetRight() - rTabCol.GetLeft() - nStart;
    m_pTColumns[ m_nAllCols ].bVisible = true;
    m_nColCount++;
    m_nAllCols++;
}

SwTableRep::~SwTableRep()
{
}

bool SwTableRep::FillTabCols( SwTabCols& rTabCols ) const
{
    long nOldLeft = rTabCols.GetLeft(),
         nOldRight = rTabCols.GetRight();

    bool bSingleLine = false;

    for ( size_t i = 0; i < rTabCols.Count(); ++i )
        if(!m_pTColumns[i].bVisible)
        {
            bSingleLine = true;
            break;
        }

    SwTwips nPos = 0;
    const SwTwips nLeft = GetLeftSpace();
    rTabCols.SetLeft(nLeft);
    if(bSingleLine)
    {
        // The invisible separators are taken from the old TabCols,
        // the visible coming from pTColumns.
        std::unique_ptr<TColumn[]> pOldTColumns(new TColumn[m_nAllCols + 1]);
        SwTwips nStart = 0;
        for ( sal_uInt16 i = 0; i < m_nAllCols - 1; ++i )
        {
            const SwTwips nEnd = rTabCols[i] - rTabCols.GetLeft();
            pOldTColumns[i].nWidth = nEnd - nStart;
            pOldTColumns[i].bVisible = !rTabCols.IsHidden(i);
            nStart = nEnd;
        }
        pOldTColumns[m_nAllCols - 1].nWidth = rTabCols.GetRight() - rTabCols.GetLeft() - nStart;
        pOldTColumns[m_nAllCols - 1].bVisible = true;

        sal_uInt16 nOldPos = 0;
        sal_uInt16 nNewPos = 0;
        SwTwips nOld = 0;
        SwTwips nNew = 0;
        bool bOld = false;
        bool bFirst = true;

        for ( sal_uInt16 i = 0; i < m_nAllCols - 1; ++i )
        {
            while((bFirst || bOld ) && nOldPos < m_nAllCols )
            {
                nOld += pOldTColumns[nOldPos].nWidth;
                nOldPos++;
                if(!pOldTColumns[nOldPos - 1].bVisible)
                    break;
            }
            while((bFirst || !bOld ) && nNewPos < m_nAllCols )
            {
                nNew += m_pTColumns[nNewPos].nWidth;
                nNewPos++;
                if(pOldTColumns[nNewPos - 1].bVisible)
                    break;
            }
            bFirst = false;
            // They have to be inserted sorted.
            bOld = nOld < nNew;
            nPos = bOld ? nOld : nNew;
            rTabCols[i] = nPos + nLeft;
            rTabCols.SetHidden( i, bOld );
        }
        rTabCols.SetRight(nLeft + m_nTableWidth);
    }
    else
    {
        for ( sal_uInt16 i = 0; i < m_nAllCols - 1; ++i )
        {
            nPos += m_pTColumns[i].nWidth;
            rTabCols[i] = nPos + rTabCols.GetLeft();
            rTabCols.SetHidden( i, !m_pTColumns[i].bVisible );
            rTabCols.SetRight(nLeft + m_pTColumns[m_nAllCols - 1].nWidth + nPos);
        }
    }

    // intercept rounding errors
    if(std::abs(nOldLeft - rTabCols.GetLeft()) < 3)
        rTabCols.SetLeft(nOldLeft);

    if(std::abs(nOldRight - rTabCols.GetRight()) < 3)
        rTabCols.SetRight(nOldRight);

    if(GetRightSpace() >= 0 &&
            rTabCols.GetRight() > rTabCols.GetRightMax())
        rTabCols.SetRight(rTabCols.GetRightMax());
    return bSingleLine;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
