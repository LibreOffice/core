/*************************************************************************
 *
 *  $RCSfile: notemark.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-09-29 10:19:55 $
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

// INCLUDE ---------------------------------------------------------------

#include <svx/svdoutl.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdobj.hxx>
#include <svx/xoutx.hxx>
#include <sfx2/printer.hxx>
#include <svtools/pathoptions.hxx>
#include <svtools/itempool.hxx>

#include "notemark.hxx"
#include "document.hxx"
#include "detfunc.hxx"

#define SC_NOTEMARK_TIME    800
#define SC_NOTEMARK_SHORT   70

// STATIC DATA -----------------------------------------------------------

// -----------------------------------------------------------------------

ScNoteMarker::ScNoteMarker( Window* pWin, Window* pRight, Window* pBottom, Window* pDiagonal,
                            ScDocument* pD, ScAddress aPos, const String& rUser,
                            const MapMode& rMap, BOOL bLeftEdge, BOOL bForce ) :
    pWindow( pWin ),
    pRightWin( pRight ),
    pBottomWin( pBottom ),
    pDiagWin( pDiagonal ),
    pDoc( pD ),
    aDocPos( aPos ),
    aUserText( rUser ),
    aMapMode( rMap ),
    bLeft( bLeftEdge ),
    bVisible( FALSE ),
    pModel( NULL ),
    pObject( NULL )
{
    aTimer.SetTimeoutHdl( LINK( this, ScNoteMarker, TimeHdl ) );
    aTimer.SetTimeout( bForce ? SC_NOTEMARK_SHORT : SC_NOTEMARK_TIME );
    aTimer.Start();
}

ScNoteMarker::~ScNoteMarker()
{
    InvalidateWin();

    delete pModel;
}

IMPL_LINK( ScNoteMarker, TimeHdl, Timer*, pTimer )
{
    if (!bVisible)
    {
        SvtPathOptions aPathOpt;
        String aPath = aPathOpt.GetPalettePath();
        pModel = new SdrModel(aPath);
        pModel->SetScaleUnit(MAP_100TH_MM);
        SfxItemPool& rPool = pModel->GetItemPool();
        rPool.SetDefaultMetric(SFX_MAPUNIT_100TH_MM);
        rPool.FreezeIdRanges();

        Printer* pPrinter = pDoc->GetPrinter();
        if (pPrinter)
        {
            //  Am Outliner des Draw-Model ist auch der Drucker als RefDevice gesetzt,
            //  und es soll einheitlich aussehen.
            Outliner& rOutliner = pModel->GetDrawOutliner();
            rOutliner.SetRefDevice(pPrinter);
        }

        SdrPage* pPage = pModel->AllocPage(FALSE);

        Size aSizePixel = pWindow->GetOutputSizePixel();
        Rectangle aVisPixel( Point(0,0), aSizePixel );
        Rectangle aVisible = pWindow->PixelToLogic( aVisPixel, aMapMode );

        USHORT nCol = aDocPos.Col();
        USHORT nRow = aDocPos.Row();
        USHORT nTab = aDocPos.Tab();
        pObject = ScDetectiveFunc( pDoc,nTab ).
                    ShowCommentUser( nCol, nRow, aUserText, aVisible, bLeft, FALSE, pPage );

        if (pObject)
            aRect = pObject->GetBoundRect();

        // #39351# Page einfuegen damit das Model sie kennt und auch deleted
        pModel->InsertPage( pPage );

        bVisible = TRUE;
    }

    Draw();
    return 0;
}

void lcl_DrawWin( SdrObject* pObject, Window* pWindow, const MapMode& rMap )
{
    MapMode aOld = pWindow->GetMapMode();
    pWindow->SetMapMode( rMap );

    ExtOutputDevice* pXOut = new ExtOutputDevice( pWindow );
    pXOut->SetOutDev( pWindow );
    SdrPaintInfoRec aInfoRec;
    pObject->Paint( *pXOut, aInfoRec );
    delete pXOut;

    pWindow->SetMapMode( aOld );
}

MapMode lcl_MoveMapMode( const MapMode& rMap, const Size& rMove )
{
    MapMode aNew = rMap;
    Point aOrigin = aNew.GetOrigin();
    aOrigin.X() -= rMove.Width();
    aOrigin.Y() -= rMove.Height();
    aNew.SetOrigin(aOrigin);
    return aNew;
}

void ScNoteMarker::Draw()
{
    if ( pObject && bVisible )
    {
        lcl_DrawWin( pObject, pWindow, aMapMode );

        if ( pRightWin || pBottomWin )
        {
            Size aWinSize = pWindow->PixelToLogic( pWindow->GetOutputSizePixel(), aMapMode );
            if ( pRightWin )
                lcl_DrawWin( pObject, pRightWin,
                                lcl_MoveMapMode( aMapMode, Size( aWinSize.Width(), 0 ) ) );
            if ( pBottomWin )
                lcl_DrawWin( pObject, pBottomWin,
                                lcl_MoveMapMode( aMapMode, Size( 0, aWinSize.Height() ) ) );
            if ( pDiagWin )
                lcl_DrawWin( pObject, pDiagWin, lcl_MoveMapMode( aMapMode, aWinSize ) );
        }
    }
}

void ScNoteMarker::InvalidateWin()
{
    if (bVisible)
    {
        pWindow->Invalidate( pWindow->LogicToLogic(aRect, aMapMode, pWindow->GetMapMode()) );

        if ( pRightWin || pBottomWin )
        {
            Size aWinSize = pWindow->PixelToLogic( pWindow->GetOutputSizePixel(), aMapMode );
            if ( pRightWin )
                pRightWin->Invalidate( pRightWin->LogicToLogic(aRect,
                                        lcl_MoveMapMode( aMapMode, Size( aWinSize.Width(), 0 ) ),
                                        pRightWin->GetMapMode()) );
            if ( pBottomWin )
                pBottomWin->Invalidate( pBottomWin->LogicToLogic(aRect,
                                        lcl_MoveMapMode( aMapMode, Size( 0, aWinSize.Height() ) ),
                                        pBottomWin->GetMapMode()) );
            if ( pDiagWin )
                pDiagWin->Invalidate( pDiagWin->LogicToLogic(aRect,
                                        lcl_MoveMapMode( aMapMode, aWinSize ),
                                        pDiagWin->GetMapMode()) );
        }
    }
}




