/*************************************************************************
 *
 *  $RCSfile: output3.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: nn $ $Date: 2002-05-06 16:53:04 $
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

#include <svx/eeitem.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdview.hxx>
#include <svx/xoutx.hxx>

#include "output.hxx"
#include "drwlayer.hxx"
#include "document.hxx"
#include "tabvwsh.hxx"


#ifdef IRIX
#ifndef _SBXCLASS_HXX
#include <svtools/sbx.hxx>
#endif
#endif

// STATIC DATA -----------------------------------------------------------

SdrObject* pSkipPaintObj = NULL;


//==================================================================

void ScOutputData::DrawingLayer( USHORT nLayer, USHORT nObjectFlags, long nLogStX, long nLogStY )
{
    if ( nObjectFlags == SC_OBJECTS_NONE || !pDoc->GetDrawLayer() )
        return;

    MapMode aOldMode = pDev->GetMapMode();

                            // Area auch fuer Metafiles
    USHORT nCol;
    USHORT nRow;

    Point aOffset;
    Rectangle aRect;

    for (nCol=0; nCol<nX1; nCol++)
        aOffset.X() -= pDoc->GetColWidth( nCol, nTab );
    for (nRow=0; nRow<nY1; nRow++)
        aOffset.Y() -= pDoc->GetRowHeight( nRow, nTab );

    aRect.Left() = aRect.Right()  = -aOffset.X();
    aRect.Top()  = aRect.Bottom() = -aOffset.Y();

    Point aMMOffset( aOffset );
    aMMOffset.X() = (long)(aMMOffset.X() * HMM_PER_TWIPS);
    aMMOffset.Y() = (long)(aMMOffset.Y() * HMM_PER_TWIPS);

    if (!bMetaFile)
        aMMOffset += Point( nLogStX, nLogStY );

    for (nCol=nX1; nCol<=nX2; nCol++)
        aRect.Right() += pDoc->GetColWidth( nCol, nTab );
    for (nRow=nY1; nRow<=nY2; nRow++)
        aRect.Bottom() += pDoc->GetRowHeight( nRow, nTab );

    aRect.Left()   = (long) (aRect.Left()   * HMM_PER_TWIPS);
    aRect.Top()    = (long) (aRect.Top()    * HMM_PER_TWIPS);
    aRect.Right()  = (long) (aRect.Right()  * HMM_PER_TWIPS);
    aRect.Bottom() = (long) (aRect.Bottom() * HMM_PER_TWIPS);

    //  HACK: Ausschnitt genau auf Ausgabe-Pixel anpassen
//! aRect = Rectangle( aRect.TopLeft(), pDev->PixelToLogic( Size( nScrW,nScrH ) ) );

    if (!bMetaFile)
        pDev->SetMapMode( MapMode( MAP_100TH_MM, aMMOffset, aOldMode.GetScaleX(), aOldMode.GetScaleY() ) );

    //! bMeta: um aMMOffset verschieben ( DrawView ) !!!!!

        // Layer zeichnen

    DrawSelectiveObjects( nLayer, aRect, nObjectFlags );

    if (!bMetaFile)
        pDev->SetMapMode( aOldMode );
}

void ScOutputData::DrawSelectiveObjects( USHORT nLayer, const Rectangle& rRect,
                                            USHORT nObjectFlags, USHORT nDummyFlags )
{
    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    if (!pModel)
        return;

    SdrOutliner& rOutl = pModel->GetDrawOutliner();
    rOutl.EnableAutoColor( bUseStyleColor );

    ExtOutputDevice* pXOut = new ExtOutputDevice( pDev );
    pXOut->SetOutDev( pDev );
    SdrPaintInfoRec aInfoRec;

    if ( pViewShell )
    {
        SdrView* pDrawView = pViewShell->GetSdrView();
        if (pDrawView)
            aInfoRec.pPV = pDrawView->GetPageViewPvNum(0);
    }

    BOOL bDidDummy = FALSE;

    SdrPage* pPage = pModel->GetPage(nTab);
    DBG_ASSERT(pPage,"Page nicht gefunden");
    if (!pPage)
        return;

    if ( aInfoRec.pPV && aInfoRec.pPV->GetObjList() == pPage )
        aInfoRec.bNotActive = FALSE;

    //  handle high contrast draw modes in addition to the group draw modes,
    //  aInfoRec.nOriginalDrawMode must include the high contrast bits if used
    ULONG nOldDrawMode = pDev->GetDrawMode();
    if ( bUseStyleColor && Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
    {
        pDev->SetDrawMode( nOldDrawMode | DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL |
                            DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT );
    }

    //  DrawMode handling copied from SdrObjList::Paint
    UINT32 nWasDrawMode = pDev->GetDrawMode();
    if(!aInfoRec.bOriginalDrawModeSet)
    {
        // Original-Paintmode retten
        aInfoRec.bOriginalDrawModeSet = TRUE;
        aInfoRec.nOriginalDrawMode = pDev->GetDrawMode();
    }
    if(aInfoRec.pPV && aInfoRec.bNotActive)
    {
        if(aInfoRec.pPV->GetView().DoVisualizeEnteredGroup())
        {
            // Darstellung schmal
            pDev->SetDrawMode(nWasDrawMode | (
                DRAWMODE_GHOSTEDLINE|DRAWMODE_GHOSTEDFILL|DRAWMODE_GHOSTEDTEXT|DRAWMODE_GHOSTEDBITMAP|DRAWMODE_GHOSTEDGRADIENT));
        }
    }
    else
    {
        // Darstellung normal
        pDev->SetDrawMode(aInfoRec.nOriginalDrawMode);
    }

    //  Paint ueber SdrObjListIter geht bei 3D-Objekten schief

    ULONG nObjCount = pPage->GetObjCount();
    for (ULONG nObjNum = 0; nObjNum<nObjCount; nObjNum++)
    {
        SdrObject* pObject = pPage->GetObj(nObjNum);

        if ( pObject->GetLayer()==(SdrLayerID) nLayer && pObject != pSkipPaintObj )
        {
            // #62107# nur das anfassen, was ueberhaupt gepainted werden koennte,
            // damit nicht unnoetig durch Cache-Miss nachgeladen wird

            Rectangle aObjRect = pObject->GetBoundRect();
            if ( aObjRect.IsOver( rRect ) )
            {
                BOOL bDraw;
                BOOL bDummy;
                UINT16 nSdrObjKind = pObject->GetObjIdentifier();
                if (nSdrObjKind == OBJ_OLE2)
                {
                    if ( pDoc->IsChart( pObject ) )
                    {
                        bDraw = (nObjectFlags & SC_OBJECTS_CHARTS) != 0;
                        bDummy = (nDummyFlags & SC_OBJECTS_CHARTS) != 0;
                    }
                    else
                    {
                        bDraw = (nObjectFlags & SC_OBJECTS_OLE) != 0;
                        bDummy = (nDummyFlags & SC_OBJECTS_OLE) != 0;
                    }
                }
                else if (nSdrObjKind == OBJ_GRAF)       // Grafiken zusammen mit Ole-Objekten
                {
                    bDraw = (nObjectFlags & SC_OBJECTS_OLE) != 0;
                    bDummy = (nDummyFlags & SC_OBJECTS_OLE) != 0;
                }
                else
                {
                    bDraw = (nObjectFlags & SC_OBJECTS_DRAWING) != 0;
                    bDummy = (nDummyFlags & SC_OBJECTS_DRAWING) != 0;
                }

                if (bDraw || bDummy)
                {
                    BOOL bClip = !rRect.IsInside( aObjRect );
                    BOOL bPush = FALSE;

                    if (bClip && bDraw && nSdrObjKind == OBJ_GRAF)
                    {
                        //  #80136# don't call GetGraphic here
                        //  (would swap the graphic in and reschedule)
                        //  GetGraphicType / IsAnimated don't swap in

                        SdrGrafObj* pGrafObj = (SdrGrafObj*)pObject;
                        GraphicType eType = pGrafObj->GetGraphicType();
                        if ( eType == GRAPHIC_GDIMETAFILE || pGrafObj->IsAnimated() )
                            bClip = FALSE;
                    }

                    //  #36427#/#37790#: VC-Objekte pfuschen mit den ClipRegions rum,
                    //  darum fuer Metafiles Push/Pop aussenherum.
                    //! Hier wieder raus, wenn die VC-Objekte richtig funktionieren!
//                  if ( bMetaFile && bDraw && !bClip && pObject->ISA(VCSbxDrawObject) )
//                      bPush = TRUE;

                    if (bClip)
                    {
                        if (bMetaFile)
                        {
                            pDev->Push();
                            pDev->IntersectClipRegion( rRect );
                        }
                        else
                        {
                            if ( nLayer != SC_LAYER_BACK )
                            {   // #29660# HACK:
                                // untere Gitterlinie blieb manchmal stehen,
                                // anscheinend Rundungsfehler in ClipRegion
                                // vom Grafiktreiber
                                Rectangle aPix = pDev->LogicToPixel(rRect);
                                aPix.Bottom() += 1;
                                Rectangle aNew = pDev->PixelToLogic(aPix);
                                pDev->SetClipRegion( aNew );
                            }
                            else
                                pDev->SetClipRegion( rRect );
                        }
                    }
                    else if (bPush)         // nur Push/Pop
                        pDev->Push();

                    if (bDraw)
                    {
                        //  set model's hyphenator on demand
                        if ( ((const SfxBoolItem&)pObject->GetItemSet().Get(EE_PARA_HYPHENATE)).GetValue() )
                            pModel->UseHyphenator();

                        if (pObject == pEditObj)
                        {
                            aInfoRec.nPaintMode|=SDRPAINTMODE_TEXTEDIT;
                            pObject->Paint( *pXOut, aInfoRec );
                            aInfoRec.nPaintMode&=~SDRPAINTMODE_TEXTEDIT;
                        }
                        else
                            pObject->Paint( *pXOut, aInfoRec );
                        bDidDummy = FALSE;

                            // Plugins connecten, wenn sichtbar:

                        if ( pViewShell && pObject->ISA(SdrOle2Obj) )
                        {
                            SdrOle2Obj* pOleObj = (SdrOle2Obj*)pObject;
                            ConnectObject( pOleObj->GetObjRef(), pOleObj );
                        }
                    }
                    else
                    {
                        if (!bDidDummy)
                        {
                            pDev->SetFillColor(COL_LIGHTGRAY);
                            pDev->SetLineColor(COL_BLACK);
                            bDidDummy = TRUE;
                        }
                        pDev->DrawRect( aObjRect );
                    }

                    if (bClip || bPush)     // bei bPush ist auch bMetaFile=TRUE
                    {
                        if (bMetaFile)
                            pDev->Pop();
                        else
                            pDev->SetClipRegion();
                    }
                }
            }
        }
    }

//  pDev->SetDrawMode(aInfoRec.nOriginalDrawMode);
    pDev->SetDrawMode(nOldDrawMode);

    delete pXOut;
}

//  Teile nur fuer Bildschirm

void ScOutputData::DrawingSingle( USHORT nLayer, USHORT nObjectFlags, USHORT nDummyFlags )
{
    Rectangle aDrawingRect;
    aDrawingRect.Left() = nScrX;
    aDrawingRect.Right() = nScrX+nScrW-1;

    BOOL    bHad    = FALSE;
    long    nPosY   = nScrY;
    USHORT  nArrY;
    for (nArrY=1; nArrY+1<nArrCount; nArrY++)
    {
        BOOL bChanged = FALSE;
        RowInfo* pThisRowInfo = &pRowInfo[nArrY];

        if ( pThisRowInfo->bChanged )
        {
            if (!bHad)
            {
                aDrawingRect.Top() = nPosY;
                bHad = TRUE;
            }
            aDrawingRect.Bottom() = nPosY + pRowInfo[nArrY].nHeight - 1;
        }
        else if (bHad)
        {
            DrawSelectiveObjects( nLayer, pDev->PixelToLogic(aDrawingRect), nObjectFlags, nDummyFlags );
            bHad = FALSE;
        }
        nPosY += pRowInfo[nArrY].nHeight;
    }

    if (bHad)
        DrawSelectiveObjects( nLayer, pDev->PixelToLogic(aDrawingRect), nObjectFlags, nDummyFlags );
}




