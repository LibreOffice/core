/*************************************************************************
 *
 *  $RCSfile: output3.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2000-11-21 18:05:22 $
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

//------------------------------------------------------------------

// TOOLS
#define _BIGINT_HXX
#define _SFXMULTISEL_HXX
#define _STACK_HXX
#define _QUEUE_HXX
#define _DYNARR_HXX
#define _TREELIST_HXX
#define _CACHESTR_HXX
#define _NEW_HXX
//#define _SHL_HXX
//#define _LINK_HXX
//#define _ERRCODE_HXX
//#define _GEN_HXX
//#define _FRACT_HXX
//#define _STRING_HXX
//#define _MTF_HXX
//#define _CONTNR_HXX
//#define _LIST_HXX
//#define _TABLE_HXX
#define _DYNARY_HXX
//#define _UNQIDX_HXX
//#define _UNQID_HXX
//#define _DEBUG_HXX
//#define _DATE_HXX
//#define _TIME_HXX
//#define _DATETIME_HXX
//#define _INTN_HXX
//#define _WLDCRD_HXX
//#define _FSYS_HXX
//#define _STREAM_HXX
#define _CACHESTR_HXX
#define _SV_MULTISEL_HXX

//SV
//#define _CLIP_HXX ***
#define _CONFIG_HXX
#define _CURSOR_HXX
#define _FONTDLG_HXX
#define _PRVWIN_HXX
//#define _COLOR_HXX
//#define _PAL_HXX
//#define _BITMAP_HXX
//#define _GDIOBJ_HXX
//#define _POINTR_HXX
//#define _ICON_HXX
//#define _IMAGE_HXX
//#define _KEYCOD_HXX
//#define _EVENT_HXX
#define _HELP_HXX
//#define _APP_HXX
//#define _MDIAPP_HXX
//#define _TIMER_HXX
//#define _METRIC_HXX
//#define _REGION_HXX
//#define _OUTDEV_HXX
//#define _SYSTEM_HXX
//#define _VIRDEV_HXX
//#define _JOBSET_HXX
//#define _PRINT_HXX
//#define _WINDOW_HXX
//#define _SYSWIN_HXX
//#define _WRKWIN_HXX
#define _MDIWIN_HXX
//#define _FLOATWIN_HXX
//#define _DOCKWIN_HXX
//#define _CTRL_HXX
//#define _SCRBAR_HXX
//#define _BUTTON_HXX
//#define _IMAGEBTN_HXX
//#define _FIXED_HXX
//#define _GROUP_HXX
//#define _EDIT_HXX
//#define _COMBOBOX_HXX
//#define _LSTBOX_HXX
//#define _SELENG_HXX
//#define _SPLIT_HXX
#define _SPIN_HXX
//#define _FIELD_HXX
//#define _MOREBTN_HXX
//#define _TOOLBOX_HXX
//#define _STATUS_HXX
//#define _DIALOG_HXX
//#define _MSGBOX_HXX
//#define _SYSDLG_HXX
//#define _FILDLG_HXX
//#define _PRNDLG_HXX
#define _COLDLG_HXX
//#define _TABDLG_HXX
//#define _MENU_HXX
//#define _GDIMTF_HXX
//#define _POLY_HXX
//#define _ACCEL_HXX
//#define _GRAPH_HXX
#define _SOUND_HXX

#if defined  WIN
#define _MENUBTN_HXX
#endif

//svtools
#define _SCRWIN_HXX
#define _RULER_HXX
//#define _TABBAR_HXX
//#define _VALUESET_HXX
#define _STDMENU_HXX
//#define _STDCTRL_HXX
//#define _CTRLBOX_HXX
#define _CTRLTOOL_HXX
#define _EXTATTR_HXX
#define _FRM3D_HXX
#define _EXTATTR_HXX

//SVTOOLS
//#define _SVTREELIST_HXX ***
#define _FILTER_HXX
//#define _SVLBOXITM_HXX ***
//#define _SVTREEBOX_HXX ***
#define _SVICNVW_HXX
#define _SVTABBX_HXX

//sfxcore.hxx
//#define _SFXINIMGR_HXX ***
//#define _SFXCFGITEM_HXX
//#define _SFX_PRINTER_HXX
#define _SFXGENLINK_HXX
#define _SFXHINTPOST_HXX
#define _SFXDOCINF_HXX
#define _SFXLINKHDL_HXX
//#define _SFX_PROGRESS_HXX

//sfxsh.hxx
//#define _SFX_SHELL_HXX
//#define _SFXAPP_HXX
//#define _SFXDISPATCH_HXX
#define _SFXMSG_HXX //*
#define _SFXOBJFACE_HXX
#define _SFXREQUEST_HXX
#define _SFXMACRO_HXX

// SFX
#define _SFXAPPWIN_HXX
#define _SFX_SAVEOPT_HXX
#define _SFXCTRLITEM_HXX
#define _SFXPRNMON_HXX
#define _INTRO_HXX
#define _SFXMSGDESCR_HXX
#define _SFXMSGPOOL_HXX
#define _SFXFILEDLG_HXX
#define _PASSWD_HXX
#define _SFXTBXCTRL_HXX
#define _SFXSTBITEM_HXX
#define _SFXMNUITEM_HXX
#define _SFXIMGMGR_HXX
#define _SFXTBXMGR_HXX
#define _SFXSTBMGR_HXX
#define _SFX_MINFITEM_HXX
#define _SFXEVENT_HXX

//sfxdoc.hxx
//#define _SFX_OBJSH_HXX
//#define _SFX_CLIENTSH_HXX
//#define _SFXDOCINF_HXX
//#define _SFX_OBJFAC_HXX
#define _SFX_DOCFILT_HXX
//#define _SFXDOCFILE_HXX ***
//define _VIEWFAC_HXX
//#define _SFXVIEWFRM_HXX
//#define _SFXVIEWSH_HXX
//#define _MDIFRM_HXX ***
#define _SFX_IPFRM_HXX
//#define _SFX_INTERNO_HXX

//sfxdlg.hxx
//#define _SFXTABDLG_HXX
//#define _BASEDLGS_HXX ***
#define _SFX_DINFDLG_HXX
#define _SFXDINFEDT_HXX
#define _SFX_MGETEMPL_HXX
#define _SFX_TPLPITEM_HXX
//#define _SFX_STYLEDLG_HXX
#define _NEWSTYLE_HXX
//#define _SFXDOCTEMPL_HXX ***
//#define _SFXDOCTDLG_HXX ***
//#define _SFX_TEMPLDLG_HXX ***
//#define _SFXNEW_HXX ***
#define _SFXDOCMAN_HXX
//#define _SFXDOCKWIN_HXX

//sfxitems.hxx
#define _SFX_WHMAP_HXX
#define _ARGS_HXX
//#define _SFXPOOLITEM_HXX
//#define _SFXINTITEM_HXX
//#define _SFXENUMITEM_HXX
#define _SFXFLAGITEM_HXX
//#define _SFXSTRITEM_HXX
#define _SFXPTITEM_HXX
#define _SFXRECTITEM_HXX
#define _SFXITEMITER_HXX
#define _SFX_WHITER_HXX
#define _SFXPOOLCACH_HXX
#define _AEITEM_HXX
#define _SFXRNGITEM_HXX
#define _SFXSLSTITM_HXX

//xout.hxx
//#define _XENUM_HXX
//#define _XPOLY_HXX
//#define _XATTR_HXX
//#define _XOUTX_HXX

//svdraw.hxx
#define _SDR_NOITEMS
#define _SDR_NOTOUCH
#define _SDR_NOTRANSFORM
//#define _SDR_NOVIEWS

#define SI_NOITEMS
#define _SI_NOSBXCONTROLS
#define _VCATTR_HXX
#define _VCONT_HXX
#define _SI_NOOTHERFORMS
#define _VCTRLS_HXX
#define _SI_NOCONTROL
#define _SETBRW_HXX
#define _VCBRW_HXX
#define _SI_NOSBXCONTROLS
#define _SIDLL_HXX

#define _SVX_DAILDLL_HXX
#define _SVX_HYPHEN_HXX
#define _SVX_IMPGRF_HXX
#define _SVX_OPTITEMS_HXX
#define _SVX_OPTGERL_HXX
#define _SVX_OPTSAVE_HXX
#define _SVX_OPTSPELL_HXX
#define _SVX_OPTPATH_HXX
#define _SVX_OPTLINGU_HXX
#define _SVX_RULER_HXX
#define _SVX_RULRITEM_HXX
#define _SVX_SPLWRAP_HXX
#define _SVX_SPLDLG_HXX
#define _SVX_THESDLG_HXX


// INCLUDE ---------------------------------------------------------------

#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
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

    pDev->SetDrawMode(aInfoRec.nOriginalDrawMode);

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




