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
#include "precompiled_sc.hxx"



#include <vcl/outdev.hxx>

#include "gridmerg.hxx"

//------------------------------------------------------------------

ScGridMerger::ScGridMerger( OutputDevice* pOutDev, long nOnePixelX, long nOnePixelY ) :
    pDev( pOutDev ),
    nOneX( nOnePixelX ),
    nOneY( nOnePixelY ),
    nCount( 0 ),
    bVertical( sal_False )
{
    //  optimize (DrawGrid) only for pixel MapMode,
    //  to avoid rounding errors

    bOptimize = ( pDev->GetMapMode().GetMapUnit() == MAP_PIXEL );
}

ScGridMerger::~ScGridMerger()
{
    Flush();
}

void ScGridMerger::AddLine( long nStart, long nEnd, long nPos )
{
    if ( nCount )
    {
        //  not first line - test fix position
        //  more than one previous line - test distance

        if ( nStart != nFixStart || nEnd != nFixEnd )
        {
            if ( nCount == 1 && nPos == nVarStart &&
                    ( nStart == nFixEnd ||
                        nStart == nFixEnd + ( bVertical ? nOneY : nOneX ) ) )
            {
                //  additional optimization: extend connected lines
                //  keep nCount at 1
                nFixEnd = nEnd;
            }
            else
                Flush();
        }
        else if ( nCount == 1 )
        {
            nVarDiff = nPos - nVarStart;
            ++nCount;
        }
        else if ( nPos != nVarStart + nCount * nVarDiff )       //! keep VarEnd?
            Flush();
        else
            ++nCount;
    }

    if ( !nCount )
    {
        //  first line (or flushed above) - just store

        nFixStart = nStart;
        nFixEnd   = nEnd;
        nVarStart = nPos;
        nVarDiff  = 0;
        nCount    = 1;
    }
}

void ScGridMerger::AddHorLine( long nX1, long nX2, long nY )
{
    if ( bOptimize )
    {
        if ( bVertical )
        {
            Flush();
            bVertical = sal_False;
        }
        AddLine( nX1, nX2, nY );
    }
    else
        pDev->DrawLine( Point( nX1, nY ), Point( nX2, nY ) );
}

void ScGridMerger::AddVerLine( long nX, long nY1, long nY2 )
{
    if ( bOptimize )
    {
        if ( !bVertical )
        {
            Flush();
            bVertical = sal_True;
        }
        AddLine( nY1, nY2, nX );
    }
    else
        pDev->DrawLine( Point( nX, nY1 ), Point( nX, nY2 ) );
}

void ScGridMerger::Flush()
{
    if (nCount)
    {
        if (bVertical)
        {
            if ( nCount == 1 )
                pDev->DrawLine( Point( nVarStart, nFixStart ), Point( nVarStart, nFixEnd ) );
            else
            {
                long nVarEnd = nVarStart + ( nCount - 1 ) * nVarDiff;
                if ( nVarDiff < 0 )
                {
                    //  nVarDiff is negative in RTL layout mode
                    //  Change the positions so DrawGrid is called with a positive distance
                    //  (nVarStart / nVarDiff can be modified, aren't used after Flush)

                    nVarDiff = -nVarDiff;
                    long nTemp = nVarStart;
                    nVarStart = nVarEnd;
                    nVarEnd = nTemp;
                }
                pDev->DrawGrid( Rectangle( nVarStart, nFixStart, nVarEnd, nFixEnd ),
                                Size( nVarDiff, nFixEnd - nFixStart ),
                                GRID_VERTLINES );
            }
        }
        else
        {
            if ( nCount == 1 )
                pDev->DrawLine( Point( nFixStart, nVarStart ), Point( nFixEnd, nVarStart ) );
            else
            {
                long nVarEnd = nVarStart + ( nCount - 1 ) * nVarDiff;
                pDev->DrawGrid( Rectangle( nFixStart, nVarStart, nFixEnd, nVarEnd ),
                                Size( nFixEnd - nFixStart, nVarDiff ),
                                GRID_HORZLINES );
            }
        }
        nCount = 0;
    }
}



