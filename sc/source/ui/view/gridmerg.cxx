/*************************************************************************
 *
 *  $RCSfile: gridmerg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:08 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include <vcl/outdev.hxx>

#include "gridmerg.hxx"

//------------------------------------------------------------------

ScGridMerger::ScGridMerger( OutputDevice* pOutDev, long nOnePixelX, long nOnePixelY ) :
    pDev( pOutDev ),
    nOneX( nOnePixelX ),
    nOneY( nOnePixelY ),
    bVertical( FALSE ),
    nCount( 0 )
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
            bVertical = FALSE;
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
            bVertical = TRUE;
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



