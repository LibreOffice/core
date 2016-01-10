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

#include <com/sun/star/embed/EmbedMisc.hpp>

#include "scitems.hxx"
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/editdata.hxx>
#include <svtools/colorcfg.hxx>
#include <svx/rotmodit.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/svxfont.hxx>
#include <svx/svdoole2.hxx>
#include <tools/poly.hxx>
#include <vcl/svapp.hxx>
#include <vcl/pdfextoutdevdata.hxx>
#include <svtools/accessibilityoptions.hxx>
#include <svx/framelinkarray.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/processor2d/processorfromoutputdevice.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/gradient.hxx>
#include <vcl/settings.hxx>
#include <svx/unoapi.hxx>

#include "output.hxx"
#include "document.hxx"
#include "drwlayer.hxx"
#include "formulacell.hxx"
#include "attrib.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "tabvwsh.hxx"
#include "progress.hxx"
#include "pagedata.hxx"
#include "chgtrack.hxx"
#include "chgviset.hxx"
#include "viewutil.hxx"
#include "gridmerg.hxx"
#include "invmerge.hxx"
#include "fillinfo.hxx"
#include "scmod.hxx"
#include "appoptio.hxx"
#include "postit.hxx"

#include "scresid.hxx"
#include "colorscale.hxx"

#include <math.h>
#include <iostream>
#include <map>
#include <memory>
#include <utility>

using namespace com::sun::star;

// Static Data

// color for ChangeTracking "by author" as in the writer (swmodul1.cxx)

#define SC_AUTHORCOLORCOUNT     9

static const ColorData nAuthorColor[ SC_AUTHORCOLORCOUNT ] = {
                    COL_LIGHTRED,       COL_LIGHTBLUE,      COL_LIGHTMAGENTA,
                    COL_GREEN,          COL_RED,            COL_BLUE,
                    COL_BROWN,          COL_MAGENTA,        COL_CYAN };

// Helper class for color assignment to avoid repeated lookups for the same user

ScActionColorChanger::ScActionColorChanger( const ScChangeTrack& rTrack ) :
    rOpt( SC_MOD()->GetAppOptions() ),
    rUsers( rTrack.GetUserCollection() ),
    nLastUserIndex( 0 ),
    nColor( COL_BLACK )
{
}

void ScActionColorChanger::Update( const ScChangeAction& rAction )
{
    ColorData nSetColor;
    switch (rAction.GetType())
    {
        case SC_CAT_INSERT_COLS:
        case SC_CAT_INSERT_ROWS:
        case SC_CAT_INSERT_TABS:
            nSetColor = rOpt.GetTrackInsertColor();
            break;
        case SC_CAT_DELETE_COLS:
        case SC_CAT_DELETE_ROWS:
        case SC_CAT_DELETE_TABS:
            nSetColor = rOpt.GetTrackDeleteColor();
            break;
        case SC_CAT_MOVE:
            nSetColor = rOpt.GetTrackMoveColor();
            break;
        default:
            nSetColor = rOpt.GetTrackContentColor();
            break;
    }
    if ( nSetColor != COL_TRANSPARENT )     // color assigned
        nColor = nSetColor;
    else                                    // by author
    {
        if (!aLastUserName.equals(rAction.GetUser()))
        {
            aLastUserName = rAction.GetUser();
            std::set<OUString>::const_iterator it = rUsers.find(aLastUserName);
            if (it == rUsers.end())
            {
                // empty string is possible if a name wasn't found while saving a 5.0 file
                SAL_INFO_IF( aLastUserName.isEmpty(), "sc.ui", "Author not found" );
                nLastUserIndex = 0;
            }
            else
            {
                size_t nPos = std::distance(rUsers.begin(), it);
                nLastUserIndex = nPos % SC_AUTHORCOLORCOUNT;
            }
        }
        nColor = nAuthorColor[nLastUserIndex];
    }
}

ScOutputData::ScOutputData( OutputDevice* pNewDev, ScOutputType eNewType,
                            ScTableInfo& rTabInfo, ScDocument* pNewDoc,
                            SCTAB nNewTab, long nNewScrX, long nNewScrY,
                            SCCOL nNewX1, SCROW nNewY1, SCCOL nNewX2, SCROW nNewY2,
                            double nPixelPerTwipsX, double nPixelPerTwipsY,
                            const Fraction* pZoomX, const Fraction* pZoomY ) :
    mpDev( pNewDev ),
    mpRefDevice( pNewDev ),      // default is output device
    pFmtDevice( pNewDev ),      // default is output device
    mrTabInfo( rTabInfo ),
    pRowInfo( rTabInfo.mpRowInfo ),
    nArrCount( rTabInfo.mnArrCount ),
    mpDoc( pNewDoc ),
    nTab( nNewTab ),
    nScrX( nNewScrX ),
    nScrY( nNewScrY ),
    nX1( nNewX1 ),
    nY1( nNewY1 ),
    nX2( nNewX2 ),
    nY2( nNewY2 ),
    eType( eNewType ),
    mnPPTX( nPixelPerTwipsX ),
    mnPPTY( nPixelPerTwipsY ),
    pEditObj( nullptr ),
    pViewShell( nullptr ),
    pDrawView( nullptr ),
    bEditMode( false ),
    nEditCol( 0 ),
    nEditRow( 0 ),
    bMetaFile( false ),
    bSingleGrid( false ),
    bPagebreakMode( false ),
    bSolidBackground( false ),
    mbUseStyleColor( false ),
    mbForceAutoColor( SC_MOD()->GetAccessOptions().GetIsAutomaticFontColor() ),
    mbSyntaxMode( false ),
    pValueColor( nullptr ),
    pTextColor( nullptr ),
    pFormulaColor( nullptr ),
    aGridColor( COL_BLACK ),
    mbShowNullValues( true ),
    mbShowFormulas( false ),
    bShowSpellErrors( false ),
    bMarkClipped( false ), // sal_False for printer/metafile etc.
    bSnapPixel( false ),
    bAnyRotated( false ),
    bAnyClipped( false ),
    mpTargetPaintWindow(nullptr), // #i74769# use SdrPaintWindow direct
    mpSpellCheckCxt(nullptr)
{
    if (pZoomX)
        aZoomX = *pZoomX;
    else
        aZoomX = Fraction(1,1);
    if (pZoomY)
        aZoomY = *pZoomY;
    else
        aZoomY = Fraction(1,1);

    nVisX1 = nX1;
    nVisY1 = nY1;
    nVisX2 = nX2;
    nVisY2 = nY2;
    mpDoc->StripHidden( nVisX1, nVisY1, nVisX2, nVisY2, nTab );

    nScrW = 0;
    for (SCCOL nX=nVisX1; nX<=nVisX2; nX++)
        nScrW += pRowInfo[0].pCellInfo[nX+1].nWidth;

    nMirrorW = nScrW;

    nScrH = 0;
    for (SCSIZE nArrY=1; nArrY+1<nArrCount; nArrY++)
        nScrH += pRowInfo[nArrY].nHeight;

    bTabProtected = mpDoc->IsTabProtected( nTab );
    nTabTextDirection = mpDoc->GetEditTextDirection( nTab );
    bLayoutRTL = mpDoc->IsLayoutRTL( nTab );
}

ScOutputData::~ScOutputData()
{
    delete pValueColor;
    delete pTextColor;
    delete pFormulaColor;
}

void ScOutputData::SetSpellCheckContext( const sc::SpellCheckContext* pCxt )
{
    mpSpellCheckCxt = pCxt;
}

void ScOutputData::SetContentDevice( OutputDevice* pContentDev )
{
    // use pContentDev instead of pDev where used

    if ( mpRefDevice == mpDev )
        mpRefDevice = pContentDev;
    if ( pFmtDevice == mpDev )
        pFmtDevice = pContentDev;
    mpDev = pContentDev;
}

void ScOutputData::SetMirrorWidth( long nNew )
{
    nMirrorW = nNew;
}

void ScOutputData::SetGridColor( const Color& rColor )
{
    aGridColor = rColor;
}

void ScOutputData::SetMarkClipped( bool bSet )
{
    bMarkClipped = bSet;
}

void ScOutputData::SetShowNullValues( bool bSet )
{
    mbShowNullValues = bSet;
}

void ScOutputData::SetShowFormulas( bool bSet )
{
    mbShowFormulas = bSet;
}

void ScOutputData::SetShowSpellErrors( bool bSet )
{
    bShowSpellErrors = bSet;
}

void ScOutputData::SetSnapPixel( bool bSet )
{
    bSnapPixel = bSet;
}

void ScOutputData::SetEditCell( SCCOL nCol, SCROW nRow )
{
    nEditCol = nCol;
    nEditRow = nRow;
    bEditMode = true;
}

void ScOutputData::SetMetaFileMode( bool bNewMode )
{
    bMetaFile = bNewMode;
}

void ScOutputData::SetSingleGrid( bool bNewMode )
{
    bSingleGrid = bNewMode;
}

void ScOutputData::SetSyntaxMode( bool bNewMode )
{
    mbSyntaxMode = bNewMode;
    if (bNewMode)
        if (!pValueColor)
        {
            pValueColor = new Color( COL_LIGHTBLUE );
            pTextColor = new Color( COL_BLACK );
            pFormulaColor = new Color( COL_GREEN );
        }
}

void ScOutputData::DrawGrid(vcl::RenderContext& rRenderContext, bool bGrid, bool bPage)
{
    SCCOL nX;
    SCROW nY;
    long nPosX;
    long nPosY;
    SCSIZE nArrY;
    ScBreakType nBreak    = BREAK_NONE;
    ScBreakType nBreakOld = BREAK_NONE;

    bool bSingle;
    Color aPageColor;
    Color aManualColor;

    if (bPagebreakMode)
        bPage = false;          // no "normal" breaks over the whole width/height

    // It is a big mess to distinguish when we are using pixels and when logic
    // units for drawing.  Ultimately we want to work only in the logic units,
    // but until that happens, we need to special-case:
    //
    //   * metafile
    //   * drawing to the screen - everything is internally counted in pixels there
    //
    // 'Internally' in the above means the pCellInfo[...].nWidth and
    // pRowInfo[...]->nHeight:
    //
    //   * when bWorksInPixels is true: these are in pixels
    //   * when bWorksInPixels is false: these are in the logic units
    //
    // This is where all the confusion comes from, ultimately we want them
    // always in the logic units (100th of millimiters), but we need to get
    // there gradually (get rid of setting MAP_PIXEL first), otherwise we'd
    // break all the drawing by one change.
    // So until that happens, we need to special case.
    bool bWorksInPixels = bMetaFile;

    if ( eType == OUTTYPE_WINDOW )
    {
        bWorksInPixels = true;
        const svtools::ColorConfig& rColorCfg = SC_MOD()->GetColorConfig();
        aPageColor.SetColor( rColorCfg.GetColorValue(svtools::CALCPAGEBREAKAUTOMATIC).nColor );
        aManualColor.SetColor( rColorCfg.GetColorValue(svtools::CALCPAGEBREAKMANUAL).nColor );
    }
    else
    {
        aPageColor = aGridColor;
        aManualColor = aGridColor;
    }

    long nOneX = 1;
    long nOneY = 1;
    if (!bWorksInPixels)
    {
        Size aOnePixel = rRenderContext.PixelToLogic(Size(1,1));
        nOneX = aOnePixel.Width();
        nOneY = aOnePixel.Height();
    }

    long nLayoutSign = bLayoutRTL ? -1 : 1;
    long nSignedOneX = nOneX * nLayoutSign;

    rRenderContext.SetLineColor(aGridColor);
    ScGridMerger aGrid(&rRenderContext, nOneX, nOneY);

    // vertical lines

    nPosX = nScrX;
    if ( bLayoutRTL )
        nPosX += nMirrorW - nOneX;

    for (nX=nX1; nX<=nX2; nX++)
    {
        SCCOL nXplus1 = nX+1;
        SCCOL nXplus2 = nX+2;
        sal_uInt16 nWidth = pRowInfo[0].pCellInfo[nXplus1].nWidth;
        if (nWidth)
        {
            nPosX += nWidth * nLayoutSign;

            if ( bPage )
            {
                // Search also in hidden part for page breaks
                SCCOL nCol = nXplus1;
                while (nCol <= MAXCOL)
                {
                    nBreak = mpDoc->HasColBreak(nCol, nTab);
                    bool bHidden = mpDoc->ColHidden(nCol, nTab);

                    if ( nBreak || !bHidden )
                        break;
                    ++nCol;
                }

                if (nBreak != nBreakOld)
                {
                    aGrid.Flush();
                    rRenderContext.SetLineColor( (nBreak & BREAK_MANUAL) ? aManualColor :
                                        nBreak ? aPageColor : aGridColor );
                    nBreakOld = nBreak;
                }
            }

            bool bDraw = bGrid || nBreakOld; // simple grid only if set that way

            sal_uInt16 nWidthXplus2 = pRowInfo[0].pCellInfo[nXplus2].nWidth;
            bSingle = bSingleGrid; //! get into Fillinfo !!!!!
            if ( nX<MAXCOL && !bSingle )
            {
                bSingle = ( nWidthXplus2 == 0 );
                for (nArrY=1; nArrY+1<nArrCount && !bSingle; nArrY++)
                {
                    if (pRowInfo[nArrY].pCellInfo[nXplus2].bHOverlapped)
                        bSingle = true;
                    if (pRowInfo[nArrY].pCellInfo[nXplus1].bHideGrid)
                        bSingle = true;
                }
            }

            if (bDraw)
            {
                if ( nX<MAXCOL && bSingle )
                {
                    SCCOL nVisX = nXplus1;
                    while ( nVisX < MAXCOL && !mpDoc->GetColWidth(nVisX,nTab) )
                        ++nVisX;

                    nPosY = nScrY;
                    for (nArrY=1; nArrY+1<nArrCount; nArrY++)
                    {
                        RowInfo* pThisRowInfo = &pRowInfo[nArrY];
                        const long nNextY = nPosY + pThisRowInfo->nHeight;

                        bool bHOver = pThisRowInfo->pCellInfo[nXplus1].bHideGrid;
                        if (!bHOver)
                        {
                            if (nWidthXplus2)
                                bHOver = pThisRowInfo->pCellInfo[nXplus2].bHOverlapped;
                            else
                            {
                                if (nVisX <= nX2)
                                    bHOver = pThisRowInfo->pCellInfo[nVisX+1].bHOverlapped;
                                else
                                    bHOver = static_cast<const ScMergeFlagAttr*>(mpDoc->GetAttr(
                                                nVisX,pThisRowInfo->nRowNo,nTab,ATTR_MERGE_FLAG))
                                                ->IsHorOverlapped();
                                if (bHOver)
                                    bHOver = static_cast<const ScMergeFlagAttr*>(mpDoc->GetAttr(
                                                nXplus1,pThisRowInfo->nRowNo,nTab,ATTR_MERGE_FLAG))
                                                ->IsHorOverlapped();
                            }
                        }

                        if (pThisRowInfo->bChanged && !bHOver)
                        {
                            aGrid.AddVerLine(bWorksInPixels, nPosX-nSignedOneX, nPosY, nNextY-nOneY);
                        }
                        nPosY = nNextY;
                    }
                }
                else
                {
                    aGrid.AddVerLine(bWorksInPixels, nPosX-nSignedOneX, nScrY, nScrY+nScrH-nOneY);
                }
            }
        }
    }

    // horizontal lines

    bool bHiddenRow = true;
    SCROW nHiddenEndRow = -1;
    nPosY = nScrY;
    for (nArrY=1; nArrY+1<nArrCount; nArrY++)
    {
        SCSIZE nArrYplus1 = nArrY+1;
        nY = pRowInfo[nArrY].nRowNo;
        SCROW nYplus1 = nY+1;
        nPosY += pRowInfo[nArrY].nHeight;

        if (pRowInfo[nArrY].bChanged)
        {
            if ( bPage )
            {
                for (SCROW i = nYplus1; i <= MAXROW; ++i)
                {
                    if (i > nHiddenEndRow)
                        bHiddenRow = mpDoc->RowHidden(i, nTab, nullptr, &nHiddenEndRow);
                    /* TODO: optimize the row break thing for large hidden
                     * segments where HasRowBreak() has to be called
                     * nevertheless for each row, as a row break is drawn also
                     * for hidden rows, above them. This needed to be done only
                     * once per hidden segment, maybe giving manual breaks
                     * priority. Something like GetNextRowBreak() and
                     * GetNextManualRowBreak(). */
                    nBreak = mpDoc->HasRowBreak(i, nTab);
                    if (!bHiddenRow || nBreak)
                        break;
                }

                if (nBreakOld != nBreak)
                {
                    aGrid.Flush();
                    rRenderContext.SetLineColor( (nBreak & BREAK_MANUAL) ? aManualColor :
                                        (nBreak) ? aPageColor : aGridColor );
                    nBreakOld = nBreak;
                }
            }

            bool bDraw = bGrid || nBreakOld;    // simple grid only if set so

            bool bNextYisNextRow = (pRowInfo[nArrYplus1].nRowNo == nYplus1);
            bSingle = !bNextYisNextRow;             // Hidden
            for (SCCOL i=nX1; i<=nX2 && !bSingle; i++)
            {
                if (pRowInfo[nArrYplus1].pCellInfo[i+1].bVOverlapped)
                    bSingle = true;
            }

            if (bDraw)
            {
                if ( bSingle && nY<MAXROW )
                {
                    SCROW nVisY = pRowInfo[nArrYplus1].nRowNo;

                    nPosX = nScrX;
                    if ( bLayoutRTL )
                        nPosX += nMirrorW - nOneX;

                    for (SCCOL i=nX1; i<=nX2; i++)
                    {
                        const long nNextX = nPosX + pRowInfo[0].pCellInfo[i+1].nWidth * nLayoutSign;
                        if (nNextX != nPosX)                                // visible
                        {
                            bool bVOver;
                            if ( bNextYisNextRow )
                                bVOver = pRowInfo[nArrYplus1].pCellInfo[i+1].bVOverlapped;
                            else
                            {
                                bVOver = static_cast<const ScMergeFlagAttr*>(mpDoc->GetAttr(
                                            i,nYplus1,nTab,ATTR_MERGE_FLAG))
                                            ->IsVerOverlapped()
                                    &&   static_cast<const ScMergeFlagAttr*>(mpDoc->GetAttr(
                                            i,nVisY,nTab,ATTR_MERGE_FLAG))
                                            ->IsVerOverlapped();
                                    //! nVisY from Array ??
                            }
                            if (!bVOver)
                            {
                                aGrid.AddHorLine(bWorksInPixels, nPosX, nNextX-nSignedOneX, nPosY-nOneY);
                            }
                        }
                        nPosX = nNextX;
                    }
                }
                else
                {
                    aGrid.AddHorLine(bWorksInPixels, nScrX, nScrX+nScrW-nOneX, nPosY-nOneY);
                }
            }
        }
    }
}

void ScOutputData::SetPagebreakMode( ScPageBreakData* pPageData )
{
    bPagebreakMode = true;
    if (!pPageData)
        return;                     // not yet initialized -> everything "not printed"

    // mark printed range
    // (everything in FillInfo is already initialized to sal_False)

    sal_uInt16 nRangeCount = sal::static_int_cast<sal_uInt16>(pPageData->GetCount());
    for (sal_uInt16 nPos=0; nPos<nRangeCount; nPos++)
    {
        ScRange aRange = pPageData->GetData( nPos ).GetPrintRange();

        SCCOL nStartX = std::max( aRange.aStart.Col(), nX1 );
        SCCOL nEndX   = std::min( aRange.aEnd.Col(),   nX2 );
        SCROW nStartY = std::max( aRange.aStart.Row(), nY1 );
        SCROW nEndY   = std::min( aRange.aEnd.Row(),   nY2 );

        for (SCSIZE nArrY=1; nArrY+1<nArrCount; nArrY++)
        {
            RowInfo* pThisRowInfo = &pRowInfo[nArrY];
            if ( pThisRowInfo->bChanged && pThisRowInfo->nRowNo >= nStartY &&
                                           pThisRowInfo->nRowNo <= nEndY )
            {
                for (SCCOL nX=nStartX; nX<=nEndX; nX++)
                    pThisRowInfo->pCellInfo[nX+1].bPrinted = true;
            }
        }
    }
}

void ScOutputData::FindRotated()
{
    //! save nRotMax
    SCCOL nRotMax = nX2;
    for (SCSIZE nRotY=0; nRotY<nArrCount; nRotY++)
        if (pRowInfo[nRotY].nRotMaxCol != SC_ROTMAX_NONE && pRowInfo[nRotY].nRotMaxCol > nRotMax)
            nRotMax = pRowInfo[nRotY].nRotMaxCol;

    for (SCSIZE nArrY=1; nArrY<nArrCount; nArrY++)
    {
        RowInfo* pThisRowInfo = &pRowInfo[nArrY];
        if ( pThisRowInfo->nRotMaxCol != SC_ROTMAX_NONE &&
             ( pThisRowInfo->bChanged || pRowInfo[nArrY-1].bChanged ||
               ( nArrY+1<nArrCount && pRowInfo[nArrY+1].bChanged ) ) )
        {
            SCROW nY = pThisRowInfo->nRowNo;

            for (SCCOL nX=0; nX<=nRotMax; nX++)
            {
                CellInfo* pInfo = &pThisRowInfo->pCellInfo[nX+1];
                const ScPatternAttr* pPattern = pInfo->pPatternAttr;
                const SfxItemSet* pCondSet = pInfo->pConditionSet;

                if ( !pPattern && !mpDoc->ColHidden(nX, nTab) )
                {
                    pPattern = mpDoc->GetPattern( nX, nY, nTab );
                    pCondSet = mpDoc->GetCondResult( nX, nY, nTab );
                }

                if ( pPattern )     // column isn't hidden
                {
                    sal_uInt8 nDir = pPattern->GetRotateDir( pCondSet );
                    if (nDir != SC_ROTDIR_NONE)
                    {
                        pInfo->nRotateDir = nDir;
                        bAnyRotated = true;
                    }
                }
            }
        }
    }
}

static sal_uInt16 lcl_GetRotateDir( ScDocument* pDoc, SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    const ScPatternAttr* pPattern = pDoc->GetPattern( nCol, nRow, nTab );
    const SfxItemSet* pCondSet = pDoc->GetCondResult( nCol, nRow, nTab );

    sal_uInt16 nRet = SC_ROTDIR_NONE;

    long nAttrRotate = pPattern->GetRotateVal( pCondSet );
    if ( nAttrRotate )
    {
        SvxRotateMode eRotMode = (SvxRotateMode)static_cast<const SvxRotateModeItem&>(
                    pPattern->GetItem(ATTR_ROTATE_MODE, pCondSet)).GetValue();

        if ( eRotMode == SVX_ROTATE_MODE_STANDARD )
            nRet = SC_ROTDIR_STANDARD;
        else if ( eRotMode == SVX_ROTATE_MODE_CENTER )
            nRet = SC_ROTDIR_CENTER;
        else if ( eRotMode == SVX_ROTATE_MODE_TOP || eRotMode == SVX_ROTATE_MODE_BOTTOM )
        {
            long nRot180 = nAttrRotate % 18000;     // 1/100 degree
            if ( nRot180 == 9000 )
                nRet = SC_ROTDIR_CENTER;
            else if ( ( eRotMode == SVX_ROTATE_MODE_TOP && nRot180 < 9000 ) ||
                      ( eRotMode == SVX_ROTATE_MODE_BOTTOM && nRot180 > 9000 ) )
                nRet = SC_ROTDIR_LEFT;
            else
                nRet = SC_ROTDIR_RIGHT;
        }
    }

    return nRet;
}

static const SvxBrushItem* lcl_FindBackground( ScDocument* pDoc, SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    const ScPatternAttr* pPattern = pDoc->GetPattern( nCol, nRow, nTab );
    const SfxItemSet* pCondSet = pDoc->GetCondResult( nCol, nRow, nTab );
    const SvxBrushItem* pBackground = static_cast<const SvxBrushItem*>(
                            &pPattern->GetItem( ATTR_BACKGROUND, pCondSet ));

    sal_uInt16 nDir = lcl_GetRotateDir( pDoc, nCol, nRow, nTab );

    // treat CENTER like RIGHT
    if ( nDir == SC_ROTDIR_RIGHT || nDir == SC_ROTDIR_CENTER )
    {
        // text goes to the right -> take background from the left
        while ( nCol > 0 && lcl_GetRotateDir( pDoc, nCol, nRow, nTab ) == nDir &&
                            pBackground->GetColor().GetTransparency() != 255 )
        {
            --nCol;
            pPattern = pDoc->GetPattern( nCol, nRow, nTab );
            pCondSet = pDoc->GetCondResult( nCol, nRow, nTab );
            pBackground = static_cast<const SvxBrushItem*>(&pPattern->GetItem( ATTR_BACKGROUND, pCondSet ));
        }
    }
    else if ( nDir == SC_ROTDIR_LEFT )
    {
        // text goes to the left -> take background from the right
        while ( nCol < MAXCOL && lcl_GetRotateDir( pDoc, nCol, nRow, nTab ) == nDir &&
                            pBackground->GetColor().GetTransparency() != 255 )
        {
            ++nCol;
            pPattern = pDoc->GetPattern( nCol, nRow, nTab );
            pCondSet = pDoc->GetCondResult( nCol, nRow, nTab );
            pBackground = static_cast<const SvxBrushItem*>(&pPattern->GetItem( ATTR_BACKGROUND, pCondSet ));
        }
    }

    return pBackground;
}

static bool lcl_EqualBack( const RowInfo& rFirst, const RowInfo& rOther,
                    SCCOL nX1, SCCOL nX2, bool bShowProt, bool bPagebreakMode )
{
    if ( rFirst.bChanged   != rOther.bChanged ||
         rFirst.bEmptyBack != rOther.bEmptyBack )
        return false;

    SCCOL nX;
    if ( bShowProt )
    {
        for ( nX=nX1; nX<=nX2; nX++ )
        {
            const ScPatternAttr* pPat1 = rFirst.pCellInfo[nX+1].pPatternAttr;
            const ScPatternAttr* pPat2 = rOther.pCellInfo[nX+1].pPatternAttr;
            if ( !pPat1 || !pPat2 ||
                    &pPat1->GetItem(ATTR_PROTECTION) != &pPat2->GetItem(ATTR_PROTECTION) )
                return false;
        }
    }
    else
    {
        for ( nX=nX1; nX<=nX2; nX++ )
            if ( rFirst.pCellInfo[nX+1].pBackground != rOther.pCellInfo[nX+1].pBackground )
                return false;
    }

    if ( rFirst.nRotMaxCol != SC_ROTMAX_NONE || rOther.nRotMaxCol != SC_ROTMAX_NONE )
        for ( nX=nX1; nX<=nX2; nX++ )
            if ( rFirst.pCellInfo[nX+1].nRotateDir != rOther.pCellInfo[nX+1].nRotateDir )
                return false;

    if ( bPagebreakMode )
        for ( nX=nX1; nX<=nX2; nX++ )
            if ( rFirst.pCellInfo[nX+1].bPrinted != rOther.pCellInfo[nX+1].bPrinted )
                return false;

    for ( nX=nX1; nX<=nX2; nX++ )
    {
        const Color* pCol1 = rFirst.pCellInfo[nX+1].pColorScale.get();
        const Color* pCol2 = rOther.pCellInfo[nX+1].pColorScale.get();
        if( (pCol1 && !pCol2) || (!pCol1 && pCol2) )
            return false;

        if (pCol1 && (*pCol1 != *pCol2))
            return false;

        const ScDataBarInfo* pInfo1 = rFirst.pCellInfo[nX+1].pDataBar.get();
        const ScDataBarInfo* pInfo2 = rOther.pCellInfo[nX+1].pDataBar.get();

        if( (pInfo1 && !pInfo2) || (!pInfo1 && pInfo2) )
            return false;

        if (pInfo1 && (*pInfo1 != *pInfo2))
            return false;

        // each cell with an icon set should be painted the same way
        const ScIconSetInfo* pIconSet1 = rFirst.pCellInfo[nX+1].pIconSet.get();
        const ScIconSetInfo* pIconSet2 = rOther.pCellInfo[nX+1].pIconSet.get();

        if(pIconSet1 || pIconSet2)
            return false;
    }

    return true;
}

void ScOutputData::DrawDocumentBackground()
{
    if ( !bSolidBackground )
        return;

    Color aBgColor( SC_MOD()->GetColorConfig().GetColorValue(svtools::DOCCOLOR).nColor );
    mpDev->SetLineColor(aBgColor);
    mpDev->SetFillColor(aBgColor);

    Point aScreenPos  = mpDev->PixelToLogic(Point(nScrX, nScrY));
    Size  aScreenSize = mpDev->PixelToLogic(Size(nScrW - 1,nScrH - 1));

    mpDev->DrawRect(Rectangle(aScreenPos, aScreenSize));
}

namespace {

static const double lclCornerRectTransparency = 40.0;

void drawDataBars(vcl::RenderContext& rRenderContext, const ScDataBarInfo* pOldDataBarInfo, const Rectangle& rRect, long nOneX, long nOneY)
{
    long nPosZero = 0;
    Rectangle aPaintRect = rRect;
    aPaintRect.Top() += 2 * nOneY;
    aPaintRect.Bottom() -= 2 * nOneY;
    aPaintRect.Left() += 2 * nOneX;
    aPaintRect.Right() -= 2 * nOneX;
    if(pOldDataBarInfo->mnZero)
    {
        // need to calculate null point in cell
        long nLength = aPaintRect.Right() - aPaintRect.Left();
        nPosZero = static_cast<long>(aPaintRect.Left() + nLength*pOldDataBarInfo->mnZero/100.0);
    }
    else
    {
        nPosZero = aPaintRect.Left();
    }

    if(pOldDataBarInfo->mnLength < 0)
    {
        aPaintRect.Right() = nPosZero;
        long nLength = nPosZero - aPaintRect.Left();
        aPaintRect.Left() = nPosZero + static_cast<long>(nLength * pOldDataBarInfo->mnLength/100.0);
    }
    else if(pOldDataBarInfo->mnLength > 0)
    {
        aPaintRect.Left() = nPosZero;
        long nLength = aPaintRect.Right() - nPosZero;
        aPaintRect.Right() = nPosZero + static_cast<long>(nLength * pOldDataBarInfo->mnLength/100.0);
    }
    else
        return;

    if(pOldDataBarInfo->mbGradient)
    {
        rRenderContext.SetLineColor(pOldDataBarInfo->maColor);
        Gradient aGradient(GradientStyle_LINEAR, pOldDataBarInfo->maColor, COL_TRANSPARENT);

        if(pOldDataBarInfo->mnLength < 0)
            aGradient.SetAngle(2700);
        else
            aGradient.SetAngle(900);

        rRenderContext.DrawGradient(aPaintRect, aGradient);

        rRenderContext.SetLineColor();
    }
    else
    {
        rRenderContext.SetFillColor(pOldDataBarInfo->maColor);
        rRenderContext.DrawRect(aPaintRect);
    }

    //draw axis
    if(pOldDataBarInfo->mnZero && pOldDataBarInfo->mnZero != 100)
    {
        Point aPoint1(nPosZero, rRect.Top());
        Point aPoint2(nPosZero, rRect.Bottom());
        LineInfo aLineInfo(LINE_DASH, 1);
        aLineInfo.SetDashCount( 4 );
        aLineInfo.SetDistance( 3 );
        aLineInfo.SetDashLen( 3 );
        rRenderContext.SetFillColor(pOldDataBarInfo->maAxisColor);
        rRenderContext.SetLineColor(pOldDataBarInfo->maAxisColor);
        rRenderContext.DrawLine(aPoint1, aPoint2, aLineInfo);
        rRenderContext.SetLineColor();
        rRenderContext.SetFillColor();
    }
}

BitmapEx& getIcon(sc::IconSetBitmapMap & rIconSetBitmapMap, ScIconSetType eType, sal_Int32 nIndex)
{
    return ScIconSetFormat::getBitmap(rIconSetBitmapMap, eType, nIndex);
}

void drawIconSets(vcl::RenderContext& rRenderContext, const ScIconSetInfo* pOldIconSetInfo, const Rectangle& rRect, long nOneX, long nOneY,
        sc::IconSetBitmapMap & rIconSetBitmapMap)
{
    //long nSize = 16;
    ScIconSetType eType = pOldIconSetInfo->eIconSetType;
    sal_Int32 nIndex = pOldIconSetInfo->nIconIndex;
    BitmapEx& rIcon = getIcon(rIconSetBitmapMap, eType, nIndex);
    long aOrigSize = std::max<long>(0,std::min(rRect.GetSize().getWidth() - 4 * nOneX, rRect.GetSize().getHeight() -4 * nOneY));
    rRenderContext.DrawBitmapEx( Point( rRect.Left() + 2 * nOneX, rRect.Top() + 2 * nOneY), Size(aOrigSize, aOrigSize), rIcon );
}

void drawCells(vcl::RenderContext& rRenderContext, const Color* pColor, const SvxBrushItem* pBackground, const Color*& pOldColor, const SvxBrushItem*& pOldBackground,
        Rectangle& rRect, long nPosX, long nLayoutSign, long nOneX, long nOneY, const ScDataBarInfo* pDataBarInfo, const ScDataBarInfo*& pOldDataBarInfo,
        const ScIconSetInfo* pIconSetInfo, const ScIconSetInfo*& pOldIconSetInfo,
        sc::IconSetBitmapMap & rIconSetBitmapMap)
{
    long nSignedOneX = nOneX * nLayoutSign;
    // need to paint if old color scale has been used and now
    // we have a different color or a style based background
    // we can here fall back to pointer comparison
    if (pOldColor && (pBackground || pOldColor != pColor || pOldDataBarInfo || pDataBarInfo || pIconSetInfo || pOldIconSetInfo))
    {
        rRect.Right() = nPosX-nSignedOneX;
        if( !pOldColor->GetTransparency() )
        {
            rRenderContext.SetFillColor( *pOldColor );
            rRenderContext.DrawRect( rRect );
        }
        if( pOldDataBarInfo )
            drawDataBars(rRenderContext, pOldDataBarInfo, rRect, nOneX, nOneY);
        if( pOldIconSetInfo )
            drawIconSets(rRenderContext, pOldIconSetInfo, rRect, nOneX, nOneY, rIconSetBitmapMap);

        rRect.Left() = nPosX - nSignedOneX;
    }

    if ( pOldBackground && (pColor ||pBackground != pOldBackground || pOldDataBarInfo || pDataBarInfo || pIconSetInfo || pOldIconSetInfo) )
    {
        rRect.Right() = nPosX-nSignedOneX;
        if (pOldBackground)             // ==0 if hidden
        {
            Color aBackCol = pOldBackground->GetColor();
            if ( !aBackCol.GetTransparency() )      //! partial transparency?
            {
                rRenderContext.SetFillColor( aBackCol );
                rRenderContext.DrawRect( rRect );
            }
        }
        if( pOldDataBarInfo )
            drawDataBars(rRenderContext, pOldDataBarInfo, rRect, nOneX, nOneY);
        if( pOldIconSetInfo )
            drawIconSets(rRenderContext, pOldIconSetInfo, rRect, nOneX, nOneY, rIconSetBitmapMap);

        rRect.Left() = nPosX - nSignedOneX;
    }

    if (!pOldBackground && !pOldColor && (pDataBarInfo || pIconSetInfo))
    {
        rRect.Right() = nPosX -nSignedOneX;
        rRect.Left() = nPosX - nSignedOneX;
    }

    if(pColor)
    {
        // only update pOldColor if the colors changed
        if (!pOldColor || *pOldColor != *pColor)
            pOldColor = pColor;

        pOldBackground = nullptr;
    }
    else if(pBackground)
    {
        pOldBackground = pBackground;
        pOldColor = nullptr;
    }

    if(pDataBarInfo)
        pOldDataBarInfo = pDataBarInfo;
    else
        pOldDataBarInfo = nullptr;

    if(pIconSetInfo)
        pOldIconSetInfo = pIconSetInfo;
    else
        pOldIconSetInfo = nullptr;
}

}

void ScOutputData::DrawBackground(vcl::RenderContext& rRenderContext)
{
    FindRotated();              //! from the outside?

    Size aOnePixel = rRenderContext.PixelToLogic(Size(1,1));
    long nOneXLogic = aOnePixel.Width();
    long nOneYLogic = aOnePixel.Height();

    // See more about bWorksInPixels in ScOutputData::DrawGrid
    bool bWorksInPixels = false;
    if (eType == OUTTYPE_WINDOW)
        bWorksInPixels = true;

    long nOneX = 1;
    long nOneY = 1;
    if (!bWorksInPixels)
    {
        nOneX = nOneXLogic;
        nOneY = nOneYLogic;
    }

    Rectangle aRect;

    long nLayoutSign = bLayoutRTL ? -1 : 1;

    rRenderContext.SetLineColor();

    bool bShowProt = mbSyntaxMode && mpDoc->IsTabProtected(nTab);
    bool bDoAll = bShowProt || bPagebreakMode || bSolidBackground;

    bool bCellContrast = mbUseStyleColor &&
            Application::GetSettings().GetStyleSettings().GetHighContrastMode();

    long nPosY = nScrY;
    for (SCSIZE nArrY=1; nArrY+1<nArrCount; nArrY++)
    {
        RowInfo* pThisRowInfo = &pRowInfo[nArrY];
        long nRowHeight = pThisRowInfo->nHeight;

        if ( pThisRowInfo->bChanged )
        {
            if ( ( ( pThisRowInfo->bEmptyBack ) || mbSyntaxMode ) && !bDoAll )
            {
                // nothing
            }
            else
            {
                // scan for rows with the same background:
                SCSIZE nSkip = 0;
                while ( nArrY+nSkip+2<nArrCount &&
                        lcl_EqualBack( *pThisRowInfo, pRowInfo[nArrY+nSkip+1],
                                        nX1, nX2, bShowProt, bPagebreakMode ) )
                {
                    ++nSkip;
                    nRowHeight += pRowInfo[nArrY+nSkip].nHeight;    // after incrementing
                }

                long nPosX = nScrX;

                if ( bLayoutRTL )
                    nPosX += nMirrorW - nOneX;

                aRect = Rectangle(nPosX, nPosY - nOneY, nPosX, nPosY - nOneY + nRowHeight);
                if (bWorksInPixels)
                    aRect = rRenderContext.PixelToLogic(aRect); // internal data in pixels, but we'll be drawing in logic units

                const SvxBrushItem* pOldBackground = nullptr;
                const SvxBrushItem* pBackground;
                const Color* pOldColor = nullptr;
                const ScDataBarInfo* pOldDataBarInfo = nullptr;
                const ScIconSetInfo* pOldIconSetInfo = nullptr;
                SCCOL nMergedCells = 1;
                SCCOL nOldMerged = 0;

                for (SCCOL nX=nX1; nX + nMergedCells <= nX2 + 1; nX += nOldMerged)
                {
                    CellInfo* pInfo = &pThisRowInfo->pCellInfo[nX+nMergedCells];

                    nOldMerged = nMergedCells;

                    if (bCellContrast)
                    {
                        //  high contrast for cell borders and backgrounds -> empty background
                        pBackground = ScGlobal::GetEmptyBrushItem();
                    }
                    else if (bShowProt)         // show cell protection in syntax mode
                    {
                        const ScPatternAttr* pP = pInfo->pPatternAttr;
                        if (pP)
                        {
                            const ScProtectionAttr& rProt = static_cast<const ScProtectionAttr&>(
                                                                pP->GetItem(ATTR_PROTECTION));
                            if (rProt.GetProtection() || rProt.GetHideCell())
                                pBackground = ScGlobal::GetProtectedBrushItem();
                            else
                                pBackground = ScGlobal::GetEmptyBrushItem();
                        }
                        else
                            pBackground = nullptr;
                    }
                    else
                        pBackground = pInfo->pBackground;

                    if ( bPagebreakMode && !pInfo->bPrinted )
                        pBackground = ScGlobal::GetProtectedBrushItem();

                    if ( pInfo->nRotateDir > SC_ROTDIR_STANDARD &&
                            pBackground->GetColor().GetTransparency() != 255 &&
                            !bCellContrast )
                    {
                        SCROW nY = pRowInfo[nArrY].nRowNo;
                        pBackground = lcl_FindBackground( mpDoc, nX, nY, nTab );
                    }

                    const Color* pColor = pInfo->pColorScale.get();
                    const ScDataBarInfo* pDataBarInfo = pInfo->pDataBar.get();
                    const ScIconSetInfo* pIconSetInfo = pInfo->pIconSet.get();

                    long nPosXLogic = nPosX;
                    if (bWorksInPixels)
                        nPosXLogic = rRenderContext.PixelToLogic(Point(nPosX, 0)).X();

                    drawCells(rRenderContext, pColor, pBackground, pOldColor, pOldBackground, aRect, nPosXLogic, nLayoutSign, nOneXLogic, nOneYLogic, pDataBarInfo, pOldDataBarInfo, pIconSetInfo, pOldIconSetInfo, mpDoc->GetIconSetBitmapMap());

                    // extend for all merged cells
                    nMergedCells = 1;
                    if (pInfo->bMerged && pInfo->pPatternAttr)
                    {
                            const ScMergeAttr* pMerge =
                                    static_cast<const ScMergeAttr*>(&pInfo->pPatternAttr->GetItem(ATTR_MERGE));
                            nMergedCells = std::max<SCCOL>(1, pMerge->GetColMerge());
                    }

                    for (SCCOL nMerged = 0; nMerged < nMergedCells; ++nMerged)
                    {
                        SCCOL nCol = nX+nOldMerged+nMerged;
                        if (nCol > nX2+2)
                            break;
                        nPosX += pRowInfo[0].pCellInfo[nCol].nWidth * nLayoutSign;
                    }
                }

                long nPosXLogic = nPosX;
                if (bWorksInPixels)
                    nPosXLogic = rRenderContext.PixelToLogic(Point(nPosX, 0)).X();

                drawCells(rRenderContext, nullptr, nullptr, pOldColor, pOldBackground, aRect, nPosXLogic, nLayoutSign, nOneXLogic, nOneYLogic, nullptr, pOldDataBarInfo, nullptr, pOldIconSetInfo, mpDoc->GetIconSetBitmapMap());

                nArrY += nSkip;
            }
        }
        nPosY += nRowHeight;
    }
}

void ScOutputData::DrawShadow()
{
    DrawExtraShadow( false, false, false, false );
}

void ScOutputData::DrawExtraShadow(bool bLeft, bool bTop, bool bRight, bool bBottom)
{
    mpDev->SetLineColor();

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    bool bCellContrast = mbUseStyleColor && rStyleSettings.GetHighContrastMode();
    Color aAutoTextColor;
    if ( bCellContrast )
        aAutoTextColor.SetColor( SC_MOD()->GetColorConfig().GetColorValue(svtools::FONTCOLOR).nColor );

    long nInitPosX = nScrX;
    if ( bLayoutRTL )
    {
        Size aOnePixel = mpDev->PixelToLogic(Size(1,1));
        long nOneX = aOnePixel.Width();
        nInitPosX += nMirrorW - nOneX;
    }
    long nLayoutSign = bLayoutRTL ? -1 : 1;

    long nPosY = nScrY - pRowInfo[0].nHeight;
    for (SCSIZE nArrY=0; nArrY<nArrCount; nArrY++)
    {
        bool bCornerY = ( nArrY == 0 ) || ( nArrY+1 == nArrCount );
        bool bSkipY = ( nArrY==0 && !bTop ) || ( nArrY+1 == nArrCount && !bBottom );

        RowInfo* pThisRowInfo = &pRowInfo[nArrY];
        long nRowHeight = pThisRowInfo->nHeight;

        if ( pThisRowInfo->bChanged && !bSkipY )
        {
            long nPosX = nInitPosX - pRowInfo[0].pCellInfo[nX1].nWidth * nLayoutSign;
            for (SCCOL nArrX=nX1; nArrX<=nX2+2; nArrX++)
            {
                bool bCornerX = ( nArrX==nX1 || nArrX==nX2+2 );
                bool bSkipX = ( nArrX==nX1 && !bLeft ) || ( nArrX==nX2+2 && !bRight );

                for (sal_uInt16 nPass=0; nPass<2; nPass++) // horizontal / vertical
                {
                    const SvxShadowItem* pAttr = nPass ?
                            pThisRowInfo->pCellInfo[nArrX].pVShadowOrigin :
                            pThisRowInfo->pCellInfo[nArrX].pHShadowOrigin;
                    if ( pAttr && !bSkipX )
                    {
                        ScShadowPart ePart = nPass ?
                                pThisRowInfo->pCellInfo[nArrX].eVShadowPart :
                                pThisRowInfo->pCellInfo[nArrX].eHShadowPart;

                        bool bDo = true;
                        if ( (nPass==0 && bCornerX) || (nPass==1 && bCornerY) )
                            if ( ePart != SC_SHADOW_CORNER )
                                bDo = false;

                        if (bDo)
                        {
                            long nThisWidth = pRowInfo[0].pCellInfo[nArrX].nWidth;
                            long nMaxWidth = nThisWidth;
                            if (!nMaxWidth)
                            {
                                //! direction must depend on shadow location
                                SCCOL nWx = nArrX;      // nX+1
                                while (nWx<nX2 && !pRowInfo[0].pCellInfo[nWx+1].nWidth)
                                    ++nWx;
                                nMaxWidth = pRowInfo[0].pCellInfo[nWx+1].nWidth;
                            }

                            // rectangle is in logical orientation
                            Rectangle aRect( nPosX, nPosY,
                                             nPosX + ( nThisWidth - 1 ) * nLayoutSign,
                                             nPosY + pRowInfo[nArrY].nHeight - 1 );

                            long nSize = pAttr->GetWidth();
                            long nSizeX = (long)(nSize*mnPPTX);
                            if (nSizeX >= nMaxWidth) nSizeX = nMaxWidth-1;
                            long nSizeY = (long)(nSize*mnPPTY);
                            if (nSizeY >= nRowHeight) nSizeY = nRowHeight-1;

                            nSizeX *= nLayoutSign;      // used only to add to rectangle values

                            SvxShadowLocation eLoc = pAttr->GetLocation();
                            if ( bLayoutRTL )
                            {
                                //  Shadow location is specified as "visual" (right is always right),
                                //  so the attribute's location value is mirrored here and in FillInfo.
                                switch (eLoc)
                                {
                                    case SVX_SHADOW_BOTTOMRIGHT: eLoc = SVX_SHADOW_BOTTOMLEFT;  break;
                                    case SVX_SHADOW_BOTTOMLEFT:  eLoc = SVX_SHADOW_BOTTOMRIGHT; break;
                                    case SVX_SHADOW_TOPRIGHT:    eLoc = SVX_SHADOW_TOPLEFT;     break;
                                    case SVX_SHADOW_TOPLEFT:     eLoc = SVX_SHADOW_TOPRIGHT;    break;
                                    default:
                                    {
                                        // added to avoid warnings
                                    }
                                }
                            }

                            if (ePart == SC_SHADOW_HORIZ || ePart == SC_SHADOW_HSTART ||
                                ePart == SC_SHADOW_CORNER)
                            {
                                if (eLoc == SVX_SHADOW_TOPLEFT || eLoc == SVX_SHADOW_TOPRIGHT)
                                    aRect.Top() = aRect.Bottom() - nSizeY;
                                else
                                    aRect.Bottom() = aRect.Top() + nSizeY;
                            }
                            if (ePart == SC_SHADOW_VERT || ePart == SC_SHADOW_VSTART ||
                                ePart == SC_SHADOW_CORNER)
                            {
                                if (eLoc == SVX_SHADOW_TOPLEFT || eLoc == SVX_SHADOW_BOTTOMLEFT)
                                    aRect.Left() = aRect.Right() - nSizeX;
                                else
                                    aRect.Right() = aRect.Left() + nSizeX;
                            }
                            if (ePart == SC_SHADOW_HSTART)
                            {
                                if (eLoc == SVX_SHADOW_TOPLEFT || eLoc == SVX_SHADOW_BOTTOMLEFT)
                                    aRect.Right() -= nSizeX;
                                else
                                    aRect.Left() += nSizeX;
                            }
                            if (ePart == SC_SHADOW_VSTART)
                            {
                                if (eLoc == SVX_SHADOW_TOPLEFT || eLoc == SVX_SHADOW_TOPRIGHT)
                                    aRect.Bottom() -= nSizeY;
                                else
                                    aRect.Top() += nSizeY;
                            }

                            //! merge rectangles?
                            mpDev->SetFillColor( bCellContrast ? aAutoTextColor : pAttr->GetColor() );
                            mpDev->DrawRect( aRect );
                        }
                    }
                }

                nPosX += pRowInfo[0].pCellInfo[nArrX].nWidth * nLayoutSign;
            }
        }
        nPosY += nRowHeight;
    }
}

void ScOutputData::DrawClear()
{
    Rectangle aRect;
    Size aOnePixel = mpDev->PixelToLogic(Size(1,1));
    long nOneX = aOnePixel.Width();
    long nOneY = aOnePixel.Height();

    // (called only for ScGridWindow)
    Color aBgColor( SC_MOD()->GetColorConfig().GetColorValue(svtools::DOCCOLOR).nColor );

    if (bMetaFile)
        nOneX = nOneY = 0;

    mpDev->SetLineColor();

    mpDev->SetFillColor( aBgColor );

    long nPosY = nScrY;
    for (SCSIZE nArrY=1; nArrY+1<nArrCount; nArrY++)
    {
        RowInfo* pThisRowInfo = &pRowInfo[nArrY];
        long nRowHeight = pThisRowInfo->nHeight;

        if ( pThisRowInfo->bChanged )
        {
            // scan for more rows which must be painted:
            SCSIZE nSkip = 0;
            while ( nArrY+nSkip+2<nArrCount && pRowInfo[nArrY+nSkip+1].bChanged )
            {
                ++nSkip;
                nRowHeight += pRowInfo[nArrY+nSkip].nHeight;    // after incrementing
            }

            aRect = Rectangle( Point( nScrX, nPosY ),
                    Size( nScrW+1-nOneX, nRowHeight+1-nOneY) );
            mpDev->DrawRect( aRect );

            nArrY += nSkip;
        }
        nPosY += nRowHeight;
    }
}

// Lines

long lclGetSnappedX( OutputDevice& rDev, long nPosX, bool bSnapPixel )
{
    return (bSnapPixel && nPosX) ? rDev.PixelToLogic( rDev.LogicToPixel( Size( nPosX, 0 ) ) ).Width() : nPosX;
}

long lclGetSnappedY( OutputDevice& rDev, long nPosY, bool bSnapPixel )
{
    return (bSnapPixel && nPosY) ? rDev.PixelToLogic( rDev.LogicToPixel( Size( 0, nPosY ) ) ).Height() : nPosY;
}

size_t lclGetArrayColFromCellInfoX( sal_uInt16 nCellInfoX, sal_uInt16 nCellInfoFirstX, sal_uInt16 nCellInfoLastX, bool bRTL )
{
    return static_cast< size_t >( bRTL ? (nCellInfoLastX + 2 - nCellInfoX) : (nCellInfoX - nCellInfoFirstX) );
}

void ScOutputData::DrawFrame(vcl::RenderContext& rRenderContext)
{
    DrawModeFlags nOldDrawMode = rRenderContext.GetDrawMode();

    Color aSingleColor;
    bool bUseSingleColor = false;
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    bool bCellContrast = mbUseStyleColor && rStyleSettings.GetHighContrastMode();

    //  if a Calc OLE object is embedded in Draw/Impress, the VCL DrawMode is used
    //  for display mode / B&W printing. The VCL DrawMode handling doesn't work for lines
    //  that are drawn with DrawRect, so if the line/background bits are set, the DrawMode
    //  must be reset and the border colors handled here.

    if ( ( nOldDrawMode & DrawModeFlags::WhiteFill ) && ( nOldDrawMode & DrawModeFlags::BlackLine ) )
    {
        rRenderContext.SetDrawMode( nOldDrawMode & (~DrawModeFlags::WhiteFill) );
        aSingleColor.SetColor( COL_BLACK );
        bUseSingleColor = true;
    }
    else if ( ( nOldDrawMode & DrawModeFlags::SettingsFill ) && ( nOldDrawMode & DrawModeFlags::SettingsLine ) )
    {
        rRenderContext.SetDrawMode( nOldDrawMode & (~DrawModeFlags::SettingsFill) );
        aSingleColor = rStyleSettings.GetWindowTextColor();     // same as used in VCL for DrawModeFlags::SettingsLine
        bUseSingleColor = true;
    }
    else if ( bCellContrast )
    {
        aSingleColor.SetColor( SC_MOD()->GetColorConfig().GetColorValue(svtools::FONTCOLOR).nColor );
        bUseSingleColor = true;
    }

    const Color* pForceColor = bUseSingleColor ? &aSingleColor : nullptr;

    if (bAnyRotated)
        DrawRotatedFrame(rRenderContext, pForceColor);        // removes the lines that must not be painted here

    long nInitPosX = nScrX;
    if ( bLayoutRTL )
    {
        Size aOnePixel = rRenderContext.PixelToLogic(Size(1,1));
        long nOneX = aOnePixel.Width();
        nInitPosX += nMirrorW - nOneX;
    }
    long nLayoutSign = bLayoutRTL ? -1 : 1;

    // *** set column and row sizes of the frame border array ***

    svx::frame::Array& rArray = mrTabInfo.maArray;
    size_t nColCount = rArray.GetColCount();
    size_t nRowCount = rArray.GetRowCount();

    // row heights

    // row 0 is not visible (dummy for borders from top) - subtract its height from initial position
    // subtract 1 unit more, because position 0 is first *in* cell, grid line is one unit before
    long nOldPosY = nScrY - 1 - pRowInfo[ 0 ].nHeight;
    long nOldSnapY = lclGetSnappedY( rRenderContext, nOldPosY, bSnapPixel );
    rArray.SetYOffset( nOldSnapY );
    for( size_t nRow = 0; nRow < nRowCount; ++nRow )
    {
        long nNewPosY = nOldPosY + pRowInfo[ nRow ].nHeight;
        long nNewSnapY = lclGetSnappedY( rRenderContext, nNewPosY, bSnapPixel );
        rArray.SetRowHeight( nRow, nNewSnapY - nOldSnapY );
        nOldPosY = nNewPosY;
        nOldSnapY = nNewSnapY;
    }

    // column widths

    // column nX1 is not visible (dummy for borders from left) - subtract its width from initial position
    // subtract 1 unit more, because position 0 is first *in* cell, grid line is one unit above
    long nOldPosX = nInitPosX - nLayoutSign * (1 + pRowInfo[ 0 ].pCellInfo[ nX1 ].nWidth);
    long nOldSnapX = lclGetSnappedX( rRenderContext, nOldPosX, bSnapPixel );
    // set X offset for left-to-right sheets; for right-to-left sheets this is done after for() loop
    if( !bLayoutRTL )
        rArray.SetXOffset( nOldSnapX );
    for( sal_uInt16 nInfoIdx = nX1; nInfoIdx <= nX2 + 2; ++nInfoIdx )
    {
        size_t nCol = lclGetArrayColFromCellInfoX( nInfoIdx, nX1, nX2, bLayoutRTL );
        long nNewPosX = nOldPosX + pRowInfo[ 0 ].pCellInfo[ nInfoIdx ].nWidth * nLayoutSign;
        long nNewSnapX = lclGetSnappedX( rRenderContext, nNewPosX, bSnapPixel );
        rArray.SetColWidth( nCol, std::abs( nNewSnapX - nOldSnapX ) );
        nOldPosX = nNewPosX;
        nOldSnapX = nNewSnapX;
    }
    if( bLayoutRTL )
        rArray.SetXOffset( nOldSnapX );

    // *** draw the array ***

    size_t nFirstCol = 1;
    size_t nFirstRow = 1;
    size_t nLastCol = nColCount - 2;
    size_t nLastRow = nRowCount - 2;

    if( mrTabInfo.mbPageMode )
        rArray.SetClipRange( nFirstCol, nFirstRow, nLastCol, nLastRow );

    // draw only rows with set RowInfo::bChanged flag
    size_t nRow1 = nFirstRow;
    std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor(CreateProcessor2D());
    if (!pProcessor)
        return;

    while( nRow1 <= nLastRow )
    {
        while( (nRow1 <= nLastRow) && !pRowInfo[ nRow1 ].bChanged ) ++nRow1;
        if( nRow1 <= nLastRow )
        {
            size_t nRow2 = nRow1;
            while( (nRow2 + 1 <= nLastRow) && pRowInfo[ nRow2 + 1 ].bChanged ) ++nRow2;
            rArray.DrawRange( pProcessor.get(), nFirstCol, nRow1, nLastCol, nRow2, pForceColor );
            nRow1 = nRow2 + 1;
        }
    }
    pProcessor.reset();

    rRenderContext.SetDrawMode(nOldDrawMode);
}

// Line below the cell

static const ::editeng::SvxBorderLine* lcl_FindHorLine( ScDocument* pDoc,
                        SCCOL nCol, SCROW nRow, SCTAB nTab, sal_uInt16 nRotDir,
                        bool bTopLine )
{
    if ( nRotDir != SC_ROTDIR_LEFT && nRotDir != SC_ROTDIR_RIGHT )
        return nullptr;

    bool bFound = false;
    while (!bFound)
    {
        if ( nRotDir == SC_ROTDIR_LEFT )
        {
            // text to the left -> line from the right
            if ( nCol < MAXCOL )
                ++nCol;
            else
                return nullptr; // couldn't find it
        }
        else
        {
            // text to the right -> line from the left
            if ( nCol > 0 )
                --nCol;
            else
                return nullptr; // couldn't find it
        }
        const ScPatternAttr* pPattern = pDoc->GetPattern( nCol, nRow, nTab );
        const SfxItemSet* pCondSet = pDoc->GetCondResult( nCol, nRow, nTab );
        if ( !pPattern->GetRotateVal( pCondSet ) ||
                static_cast<const SvxRotateModeItem&>(pPattern->GetItem(
                    ATTR_ROTATE_MODE, pCondSet)).GetValue() == SVX_ROTATE_MODE_STANDARD )
            bFound = true;
    }

    if (bTopLine)
        --nRow;
    const ::editeng::SvxBorderLine* pThisBottom;
    if ( ValidRow(nRow) )
        pThisBottom = static_cast<const SvxBoxItem*>(pDoc->GetAttr( nCol, nRow, nTab, ATTR_BORDER ))->GetBottom();
    else
        pThisBottom = nullptr;
    const ::editeng::SvxBorderLine* pNextTop;
    if ( nRow < MAXROW )
        pNextTop = static_cast<const SvxBoxItem*>(pDoc->GetAttr( nCol, nRow+1, nTab, ATTR_BORDER ))->GetTop();
    else
        pNextTop = nullptr;

    if ( ScHasPriority( pThisBottom, pNextTop ) )
        return pThisBottom;
    else
        return pNextTop;
}

static long lcl_getRotate( ScDocument* pDoc, SCTAB nTab, SCCOL nX, SCROW nY )
{
    long nRotate = 0;

    const ScPatternAttr* pPattern = pDoc->GetPattern( nX, nY, nTab );
    const SfxItemSet* pCondSet = pDoc->GetCondResult( nX, nY, nTab );

    nRotate = pPattern->GetRotateVal( pCondSet );

    return nRotate;
}

void ScOutputData::DrawRotatedFrame(vcl::RenderContext& rRenderContext, const Color* pForceColor)
{
    //! save nRotMax
    SCCOL nRotMax = nX2;
    for (SCSIZE nRotY=0; nRotY<nArrCount; nRotY++)
        if (pRowInfo[nRotY].nRotMaxCol != SC_ROTMAX_NONE && pRowInfo[nRotY].nRotMaxCol > nRotMax)
            nRotMax = pRowInfo[nRotY].nRotMaxCol;

    const ScPatternAttr* pPattern;
    const SfxItemSet*    pCondSet;

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    bool bCellContrast = mbUseStyleColor && rStyleSettings.GetHighContrastMode();

    // color (pForceColor) is determined externally, including DrawMode changes

    long nInitPosX = nScrX;
    if ( bLayoutRTL )
    {
        Size aOnePixel = rRenderContext.PixelToLogic(Size(1,1));
        long nOneX = aOnePixel.Width();
        nInitPosX += nMirrorW - nOneX;
    }
    long nLayoutSign = bLayoutRTL ? -1 : 1;

    Rectangle aClipRect( Point(nScrX, nScrY), Size(nScrW, nScrH) );
    if (bMetaFile)
    {
        rRenderContext.Push();
        rRenderContext.IntersectClipRegion( aClipRect );
    }
    else
        rRenderContext.SetClipRegion( vcl::Region( aClipRect ) );

    svx::frame::Array& rArray = mrTabInfo.maArray;
    std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor(CreateProcessor2D( ));

    long nPosY = nScrY;
    for (SCSIZE nArrY=1; nArrY<nArrCount; nArrY++)
    {
        // Rotated is also drawn one line above/below Changed if parts extend into the cell

        RowInfo& rPrevRowInfo = pRowInfo[nArrY-1];
        RowInfo& rThisRowInfo = pRowInfo[nArrY];
        RowInfo& rNextRowInfo = pRowInfo[nArrY+1];

        size_t nRow = static_cast< size_t >( nArrY );

        long nRowHeight = rThisRowInfo.nHeight;
        if ( rThisRowInfo.nRotMaxCol != SC_ROTMAX_NONE &&
             ( rThisRowInfo.bChanged || rPrevRowInfo.bChanged ||
               ( nArrY+1<nArrCount && rNextRowInfo.bChanged ) ) )
        {
            SCROW nY = rThisRowInfo.nRowNo;
            long nPosX = 0;
            SCCOL nX;
            for (nX=0; nX<=nRotMax; nX++)
            {
                if (nX==nX1) nPosX = nInitPosX;     // calculated individually for preceding positions

                sal_uInt16 nArrX = nX + 1;

                CellInfo* pInfo = &rThisRowInfo.pCellInfo[nArrX];
                long nColWidth = pRowInfo[0].pCellInfo[nArrX].nWidth;
                if ( pInfo->nRotateDir > SC_ROTDIR_STANDARD &&
                        !pInfo->bHOverlapped && !pInfo->bVOverlapped )
                {
                    pPattern = pInfo->pPatternAttr;
                    pCondSet = pInfo->pConditionSet;
                    if (!pPattern)
                    {
                        pPattern = mpDoc->GetPattern( nX, nY, nTab );
                        pInfo->pPatternAttr = pPattern;
                        pCondSet = mpDoc->GetCondResult( nX, nY, nTab );
                        pInfo->pConditionSet = pCondSet;
                    }

                    //! LastPattern etc.

                    long nAttrRotate = pPattern->GetRotateVal( pCondSet );
                    SvxRotateMode eRotMode = (SvxRotateMode)static_cast<const SvxRotateModeItem&>(
                                    pPattern->GetItem(ATTR_ROTATE_MODE, pCondSet)).GetValue();

                    if ( nAttrRotate )
                    {
                        if (nX<nX1)         // compute negative position
                        {
                            nPosX = nInitPosX;
                            SCCOL nCol = nX1;
                            while (nCol > nX)
                            {
                                --nCol;
                                nPosX -= nLayoutSign * (long) pRowInfo[0].pCellInfo[nCol+1].nWidth;
                            }
                        }

                        // start position minus 1 so rotated backgrounds suit the border
                        // (border is on the grid)

                        long nTop = nPosY - 1;
                        long nBottom = nPosY + nRowHeight - 1;
                        long nTopLeft = nPosX - nLayoutSign;
                        long nTopRight = nPosX + ( nColWidth - 1 ) * nLayoutSign;
                        long nBotLeft = nTopLeft;
                        long nBotRight = nTopRight;

                        // inclusion of the sign here hasn't been decided yet
                        // (if not, the extension of the non-rotated background must also be changed)
                        double nRealOrient = nLayoutSign * nAttrRotate * F_PI18000;     // 1/100th degrees
                        double nCos = cos( nRealOrient );
                        double nSin = sin( nRealOrient );
                        //! restrict !!!
                        long nSkew = (long) ( nRowHeight * nCos / nSin );

                        switch (eRotMode)
                        {
                            case SVX_ROTATE_MODE_BOTTOM:
                                nTopLeft += nSkew;
                                nTopRight += nSkew;
                                break;
                            case SVX_ROTATE_MODE_CENTER:
                                nSkew /= 2;
                                nTopLeft += nSkew;
                                nTopRight += nSkew;
                                nBotLeft -= nSkew;
                                nBotRight -= nSkew;
                                break;
                            case SVX_ROTATE_MODE_TOP:
                                nBotLeft -= nSkew;
                                nBotRight -= nSkew;
                                break;
                            default:
                            {
                                // added to avoid warnings
                            }
                        }

                        Point aPoints[4];
                        aPoints[0] = Point( nTopLeft, nTop );
                        aPoints[1] = Point( nTopRight, nTop );
                        aPoints[2] = Point( nBotRight, nBottom );
                        aPoints[3] = Point( nBotLeft, nBottom );

                        const SvxBrushItem* pBackground = pInfo->pBackground;
                        if (!pBackground)
                            pBackground = static_cast<const SvxBrushItem*>( &pPattern->GetItem(
                                                ATTR_BACKGROUND, pCondSet ));
                        if (bCellContrast)
                        {
                            //  high contrast for cell borders and backgrounds -> empty background
                            pBackground = ScGlobal::GetEmptyBrushItem();
                        }
                        if(!pInfo->pColorScale)
                        {
                            const Color& rColor = pBackground->GetColor();
                            if ( rColor.GetTransparency() != 255 )
                            {
                                //  draw background only for the changed row itself
                                //  (background doesn't extend into other cells).
                                //  For the borders (rotated and normal), clipping should be
                                //  set if the row isn't changed, but at least the borders
                                //  don't cover the cell contents.
                                if ( rThisRowInfo.bChanged )
                                {
                                    tools::Polygon aPoly( 4, aPoints );

                                    // for DrawPolygon, whitout Pen one pixel is left out
                                    // to the right and below...
                                    if ( rColor.GetTransparency() == 0 )
                                        rRenderContext.SetLineColor(rColor);
                                    else
                                        rRenderContext.SetLineColor();
                                    rRenderContext.SetFillColor(rColor);
                                    rRenderContext.DrawPolygon( aPoly );
                                }
                            }
                        }
                        else
                        {
                            tools::Polygon aPoly( 4, aPoints );
                            const Color* pColor = pInfo->pColorScale.get();

                            // for DrawPolygon, whitout Pen one pixel is left out
                            // to the right and below...
                            if ( pColor->GetTransparency() == 0 )
                                rRenderContext.SetLineColor(*pColor);
                            else
                                rRenderContext.SetLineColor();
                            rRenderContext.SetFillColor(*pColor);
                            rRenderContext.DrawPolygon( aPoly );

                        }

                        svx::frame::Style aTopLine, aBottomLine, aLeftLine, aRightLine;

                        if ( nX < nX1 || nX > nX2 )     // Attributes in FillInfo not set
                        {
                            //! consider page borders for printing !!!!!
                            const ::editeng::SvxBorderLine* pLeftLine;
                            const ::editeng::SvxBorderLine* pTopLine;
                            const ::editeng::SvxBorderLine* pRightLine;
                            const ::editeng::SvxBorderLine* pBottomLine;
                            mpDoc->GetBorderLines( nX, nY, nTab,
                                    &pLeftLine, &pTopLine, &pRightLine, &pBottomLine );
                            aTopLine.Set( pTopLine, mnPPTY );
                            aBottomLine.Set( pBottomLine, mnPPTY );
                            aLeftLine.Set( pLeftLine, mnPPTX );
                            aRightLine.Set( pRightLine, mnPPTX );
                        }
                        else
                        {
                            size_t nCol = lclGetArrayColFromCellInfoX( nArrX, nX1, nX2, bLayoutRTL );
                            aTopLine = rArray.GetCellStyleTop( nCol, nRow );
                            aBottomLine = rArray.GetCellStyleBottom( nCol, nRow );
                            aLeftLine = rArray.GetCellStyleLeft( nCol, nRow );
                            aRightLine = rArray.GetCellStyleRight( nCol, nRow );
                            // in RTL mode the array is already mirrored -> swap back left/right borders
                            if( bLayoutRTL )
                                std::swap( aLeftLine, aRightLine );
                        }

                        // Horizontal lines
                        if (aTopLine.Prim() || aTopLine.Secn())
                        {
                            long nUpperRotate = lcl_getRotate( mpDoc, nTab, nX, nY - 1 );
                            pProcessor->process( svx::frame::CreateBorderPrimitives(
                                        aPoints[bLayoutRTL?1:0], aPoints[bLayoutRTL?0:1], aTopLine,
                                        svx::frame::Style(),
                                        svx::frame::Style(),
                                        aLeftLine,
                                        svx::frame::Style(),
                                        svx::frame::Style(),
                                        aRightLine,
                                        pForceColor, nUpperRotate, nAttrRotate ) );
                        }

                        if (aBottomLine.Prim() || aBottomLine.Secn())
                        {
                            long nLowerRotate = lcl_getRotate( mpDoc, nTab, nX, nY + 1 );
                            pProcessor->process( svx::frame::CreateBorderPrimitives(
                                        aPoints[bLayoutRTL?2:3], aPoints[bLayoutRTL?3:2], aBottomLine,
                                        aLeftLine,
                                        svx::frame::Style(),
                                        svx::frame::Style(),
                                        aRightLine,
                                        svx::frame::Style(),
                                        svx::frame::Style(),
                                        pForceColor, 18000 - nAttrRotate, 18000 - nLowerRotate ) );
                        }

                        // Vertical slanted lines
                        if (aLeftLine.Prim() || aLeftLine.Secn())
                        {
                            long nLeftRotate = lcl_getRotate( mpDoc, nTab, nX - 1, nY );
                            pProcessor->process( svx::frame::CreateBorderPrimitives(
                                        aPoints[0], aPoints[3], aLeftLine,
                                        aTopLine,
                                        svx::frame::Style(),
                                        svx::frame::Style(),
                                        aBottomLine,
                                        svx::frame::Style(),
                                        svx::frame::Style(),
                                        pForceColor, nAttrRotate, nLeftRotate ) );
                        }

                        if (aRightLine.Prim() || aRightLine.Secn())
                        {
                            long nRightRotate = lcl_getRotate( mpDoc, nTab, nX + 1, nY );
                            pProcessor->process( svx::frame::CreateBorderPrimitives(
                                        aPoints[1], aPoints[2], aRightLine,
                                        svx::frame::Style(),
                                        svx::frame::Style(),
                                        aTopLine,
                                        svx::frame::Style(),
                                        svx::frame::Style(),
                                        aBottomLine,
                                        pForceColor, 18000 - nRightRotate, 18000 - nAttrRotate ) );
                        }
                    }
                }
                nPosX += nColWidth * nLayoutSign;
            }

            // delete the lines for normal output only afterwards in the second step

            nX = nX1 > 0 ? (nX1-1) : static_cast<SCCOL>(0);
            for (; nX<=nX2+1; nX++)         // visible part +- 1
            {
                sal_uInt16 nArrX = nX + 1;
                CellInfo& rInfo = rThisRowInfo.pCellInfo[nArrX];
                if ( rInfo.nRotateDir > SC_ROTDIR_STANDARD &&
                        !rInfo.bHOverlapped && !rInfo.bVOverlapped )
                {
                    size_t nCol = lclGetArrayColFromCellInfoX( nArrX, nX1, nX2, bLayoutRTL );

                    // horizontal: extend adjacent line
                    // (only when the rotated cell has a border)
                    sal_uInt16 nDir = rInfo.nRotateDir;
                    if ( rArray.GetCellStyleTop( nCol, nRow ).Prim() )
                    {
                        svx::frame::Style aStyle( lcl_FindHorLine( mpDoc, nX, nY, nTab, nDir, true ), mnPPTY );
                        rArray.SetCellStyleTop( nCol, nRow, aStyle );
                        if( nRow > 0 )
                            rArray.SetCellStyleBottom( nCol, nRow - 1, aStyle );
                    }
                    if ( rArray.GetCellStyleBottom( nCol, nRow ).Prim() )
                    {
                        svx::frame::Style aStyle( lcl_FindHorLine( mpDoc, nX, nY, nTab, nDir, false ), mnPPTY );
                        rArray.SetCellStyleBottom( nCol, nRow, aStyle );
                        if( nRow + 1 < rArray.GetRowCount() )
                            rArray.SetCellStyleTop( nCol, nRow + 1, aStyle );
                    }

                    // always remove vertical borders
                    if( !rArray.IsMergedOverlappedLeft( nCol, nRow ) )
                    {
                        rArray.SetCellStyleLeft( nCol, nRow, svx::frame::Style() );
                        if( nCol > 0 )
                            rArray.SetCellStyleRight( nCol - 1, nRow, svx::frame::Style() );
                    }
                    if( !rArray.IsMergedOverlappedRight( nCol, nRow ) )
                    {
                        rArray.SetCellStyleRight( nCol, nRow, svx::frame::Style() );
                        if( nCol + 1 < rArray.GetColCount() )
                            rArray.SetCellStyleLeft( nCol + 1, nRow, svx::frame::Style() );
                    }

                    // remove diagonal borders
                    rArray.SetCellStyleTLBR( nCol, nRow, svx::frame::Style() );
                    rArray.SetCellStyleBLTR( nCol, nRow, svx::frame::Style() );
                }
            }
        }
        nPosY += nRowHeight;
    }

    pProcessor.reset();

    if (bMetaFile)
        rRenderContext.Pop();
    else
        rRenderContext.SetClipRegion();
}

drawinglayer::processor2d::BaseProcessor2D* ScOutputData::CreateProcessor2D( )
{
    mpDoc->InitDrawLayer(mpDoc->GetDocumentShell());
    ScDrawLayer* pDrawLayer = mpDoc->GetDrawLayer();
    if (!pDrawLayer)
        return nullptr;

    basegfx::B2DRange aViewRange;
    SdrPage *pDrawPage = pDrawLayer->GetPage( static_cast< sal_uInt16 >( nTab ) );
    const drawinglayer::geometry::ViewInformation2D aNewViewInfos(
            basegfx::B2DHomMatrix(  ),
            mpDev->GetViewTransformation(),
            aViewRange,
            GetXDrawPageForSdrPage( pDrawPage ),
            0.0,
            uno::Sequence< beans::PropertyValue >() );

    return drawinglayer::processor2d::createBaseProcessor2DFromOutputDevice(
                    *mpDev, aNewViewInfos );
}

// Printer

vcl::Region ScOutputData::GetChangedAreaRegion()
{
    vcl::Region aRegion;
    Rectangle aDrawingRect;
    bool bHad(false);
    long nPosY = nScrY;
    SCSIZE nArrY;

    aDrawingRect.Left() = nScrX;
    aDrawingRect.Right() = nScrX+nScrW-1;

    for(nArrY=1; nArrY+1<nArrCount; nArrY++)
    {
        RowInfo* pThisRowInfo = &pRowInfo[nArrY];

        if(pThisRowInfo->bChanged)
        {
            if(!bHad)
            {
                aDrawingRect.Top() = nPosY;
                bHad = true;
            }

            aDrawingRect.Bottom() = nPosY + pRowInfo[nArrY].nHeight - 1;
        }
        else if(bHad)
        {
            aRegion.Union(mpDev->PixelToLogic(aDrawingRect));
            bHad = false;
        }

        nPosY += pRowInfo[nArrY].nHeight;
    }

    if(bHad)
    {
        aRegion.Union(mpDev->PixelToLogic(aDrawingRect));
    }

    return aRegion;
}

bool ScOutputData::SetChangedClip()
{
    tools::PolyPolygon aPoly;

    Rectangle aDrawingRect;
    aDrawingRect.Left() = nScrX;
    aDrawingRect.Right() = nScrX+nScrW-1;

    bool    bHad    = false;
    long    nPosY   = nScrY;
    SCSIZE  nArrY;
    for (nArrY=1; nArrY+1<nArrCount; nArrY++)
    {
        RowInfo* pThisRowInfo = &pRowInfo[nArrY];

        if ( pThisRowInfo->bChanged )
        {
            if (!bHad)
            {
                aDrawingRect.Top() = nPosY;
                bHad = true;
            }
            aDrawingRect.Bottom() = nPosY + pRowInfo[nArrY].nHeight - 1;
        }
        else if (bHad)
        {
            aPoly.Insert( tools::Polygon( mpDev->PixelToLogic(aDrawingRect) ) );
            bHad = false;
        }
        nPosY += pRowInfo[nArrY].nHeight;
    }

    if (bHad)
        aPoly.Insert( tools::Polygon( mpDev->PixelToLogic(aDrawingRect) ) );

    bool bRet = (aPoly.Count() != 0);
    if (bRet)
        mpDev->SetClipRegion(vcl::Region(aPoly));
    return bRet;
}

void ScOutputData::FindChanged()
{
    SCCOL   nX;
    SCSIZE  nArrY;

    bool bWasIdleEnabled = mpDoc->IsIdleEnabled();
    mpDoc->EnableIdle(false);
    for (nArrY=0; nArrY<nArrCount; nArrY++)
        pRowInfo[nArrY].bChanged = false;

    bool bProgress = false;
    for (nArrY=0; nArrY<nArrCount; nArrY++)
    {
        RowInfo* pThisRowInfo = &pRowInfo[nArrY];
        for (nX=nX1; nX<=nX2; nX++)
        {
            const ScRefCellValue& rCell = pThisRowInfo->pCellInfo[nX+1].maCell;

            if (rCell.meType != CELLTYPE_FORMULA)
                continue;

            ScFormulaCell* pFCell = rCell.mpFormula;
            if ( !bProgress && pFCell->GetDirty() )
            {
                ScProgress::CreateInterpretProgress(mpDoc);
                bProgress = true;
            }
            if (pFCell->IsRunning())
                // still being interpreted. Skip it.
                continue;

            (void)pFCell->GetValue();
            if (!pFCell->IsChanged())
                // the result hasn't changed. Skip it.
                continue;

            pThisRowInfo->bChanged = true;
            if ( pThisRowInfo->pCellInfo[nX+1].bMerged )
            {
                SCSIZE nOverY = nArrY + 1;
                while ( nOverY<nArrCount &&
                        pRowInfo[nOverY].pCellInfo[nX+1].bVOverlapped )
                {
                    pRowInfo[nOverY].bChanged = true;
                    ++nOverY;
                }
            }
        }
    }
    if ( bProgress )
        ScProgress::DeleteInterpretProgress();
    mpDoc->EnableIdle(bWasIdleEnabled);
}

void ScOutputData::DrawRefMark( SCCOL nRefStartX, SCROW nRefStartY,
                                SCCOL nRefEndX, SCROW nRefEndY,
                                const Color& rColor, bool bHandle )
{
    PutInOrder( nRefStartX, nRefEndX );
    PutInOrder( nRefStartY, nRefEndY );

    if ( nRefStartX == nRefEndX && nRefStartY == nRefEndY )
        mpDoc->ExtendMerge( nRefStartX, nRefStartY, nRefEndX, nRefEndY, nTab );

    if ( nRefStartX <= nVisX2 && nRefEndX >= nVisX1 &&
         nRefStartY <= nVisY2 && nRefEndY >= nVisY1 )
    {
        long nMinX = nScrX;
        long nMinY = nScrY;
        long nMaxX = nScrX + nScrW - 1;
        long nMaxY = nScrY + nScrH - 1;
        if ( bLayoutRTL )
        {
            long nTemp = nMinX;
            nMinX = nMaxX;
            nMaxX = nTemp;
        }
        long nLayoutSign = bLayoutRTL ? -1 : 1;

        bool bTop    = false;
        bool bBottom = false;
        bool bLeft   = false;
        bool bRight  = false;

        long nPosY = nScrY;
        bool bNoStartY = ( nY1 < nRefStartY );
        bool bNoEndY   = false;
        for (SCSIZE nArrY=1; nArrY<nArrCount; nArrY++)      // loop to end for bNoEndY check
        {
            SCROW nY = pRowInfo[nArrY].nRowNo;

            if ( nY==nRefStartY || (nY>nRefStartY && bNoStartY) )
            {
                nMinY = nPosY;
                bTop = true;
            }
            if ( nY==nRefEndY )
            {
                nMaxY = nPosY + pRowInfo[nArrY].nHeight - 2;
                bBottom = true;
            }
            if ( nY>nRefEndY && bNoEndY )
            {
                nMaxY = nPosY-2;
                bBottom = true;
            }
            bNoStartY = ( nY < nRefStartY );
            bNoEndY   = ( nY < nRefEndY );
            nPosY += pRowInfo[nArrY].nHeight;
        }

        long nPosX = nScrX;
        if ( bLayoutRTL )
            nPosX += nMirrorW - 1;      // always in pixels

        for (SCCOL nX=nX1; nX<=nX2; nX++)
        {
            if ( nX==nRefStartX )
            {
                nMinX = nPosX;
                bLeft = true;
            }
            if ( nX==nRefEndX )
            {
                nMaxX = nPosX + ( pRowInfo[0].pCellInfo[nX+1].nWidth - 2 ) * nLayoutSign;
                bRight = true;
            }
            nPosX += pRowInfo[0].pCellInfo[nX+1].nWidth * nLayoutSign;
        }

        if ( nMaxX * nLayoutSign >= nMinX * nLayoutSign &&
             nMaxY >= nMinY )
        {
            mpDev->SetLineColor( rColor );
            if (bTop && bBottom && bLeft && bRight)
            {
                mpDev->SetFillColor();
                mpDev->DrawRect( Rectangle( nMinX, nMinY, nMaxX, nMaxY ) );
            }
            else
            {
                if (bTop)
                    mpDev->DrawLine( Point( nMinX, nMinY ), Point( nMaxX, nMinY ) );
                if (bBottom)
                    mpDev->DrawLine( Point( nMinX, nMaxY ), Point( nMaxX, nMaxY ) );
                if (bLeft)
                    mpDev->DrawLine( Point( nMinX, nMinY ), Point( nMinX, nMaxY ) );
                if (bRight)
                    mpDev->DrawLine( Point( nMaxX, nMinY ), Point( nMaxX, nMaxY ) );
            }
            if ( bHandle && bRight && bBottom )
            {
                mpDev->SetLineColor( rColor );
                mpDev->SetFillColor( rColor );

                const sal_Int32 aRadius = 4;

                sal_Int32 aRectMaxX1 = nMaxX - nLayoutSign * aRadius;
                sal_Int32 aRectMaxX2 = nMaxX + nLayoutSign;
                sal_Int32 aRectMinX1 = nMinX - nLayoutSign;
                sal_Int32 aRectMinX2 = nMinX + nLayoutSign * aRadius;

                sal_Int32 aRectMaxY1 = nMaxY - aRadius;
                sal_Int32 aRectMaxY2 = nMaxY + 1;
                sal_Int32 aRectMinY1 = nMinY - 1;
                sal_Int32 aRectMinY2 = nMinY + aRadius;

                // Draw corner rectangles
                Rectangle aLowerRight( aRectMaxX1, aRectMaxY1, aRectMaxX2, aRectMaxY2 );
                Rectangle aUpperLeft ( aRectMinX1, aRectMinY1, aRectMinX2, aRectMinY2 );
                Rectangle aLowerLeft ( aRectMinX1, aRectMaxY1, aRectMinX2, aRectMaxY2 );
                Rectangle aUpperRight( aRectMaxX1, aRectMinY1, aRectMaxX2, aRectMinY2 );

                mpDev->DrawTransparent( tools::PolyPolygon( tools::Polygon( aLowerRight ) ), lclCornerRectTransparency );
                mpDev->DrawTransparent( tools::PolyPolygon( tools::Polygon( aUpperLeft  ) ), lclCornerRectTransparency );
                mpDev->DrawTransparent( tools::PolyPolygon( tools::Polygon( aLowerLeft  ) ), lclCornerRectTransparency );
                mpDev->DrawTransparent( tools::PolyPolygon( tools::Polygon( aUpperRight ) ), lclCornerRectTransparency );
            }
        }
    }
}

void ScOutputData::DrawOneChange( SCCOL nRefStartX, SCROW nRefStartY,
                                SCCOL nRefEndX, SCROW nRefEndY,
                                const Color& rColor, sal_uInt16 nType )
{
    PutInOrder( nRefStartX, nRefEndX );
    PutInOrder( nRefStartY, nRefEndY );

    if ( nRefStartX == nRefEndX && nRefStartY == nRefEndY )
        mpDoc->ExtendMerge( nRefStartX, nRefStartY, nRefEndX, nRefEndY, nTab );

    if ( nRefStartX <= nVisX2 + 1 && nRefEndX >= nVisX1 &&
         nRefStartY <= nVisY2 + 1 && nRefEndY >= nVisY1 )       // +1 because it touches next cells left/top
    {
        long nMinX = nScrX;
        long nMinY = nScrY;
        long nMaxX = nScrX+nScrW-1;
        long nMaxY = nScrY+nScrH-1;
        if ( bLayoutRTL )
        {
            long nTemp = nMinX;
            nMinX = nMaxX;
            nMaxX = nTemp;
        }
        long nLayoutSign = bLayoutRTL ? -1 : 1;

        bool bTop    = false;
        bool bBottom = false;
        bool bLeft   = false;
        bool bRight  = false;

        long nPosY = nScrY;
        bool bNoStartY = ( nY1 < nRefStartY );
        bool bNoEndY   = false;
        for (SCSIZE nArrY=1; nArrY<nArrCount; nArrY++)      // loop to end for bNoEndY check
        {
            SCROW nY = pRowInfo[nArrY].nRowNo;

            if ( nY==nRefStartY || (nY>nRefStartY && bNoStartY) )
            {
                nMinY = nPosY - 1;
                bTop = true;
            }
            if ( nY==nRefEndY )
            {
                nMaxY = nPosY + pRowInfo[nArrY].nHeight - 1;
                bBottom = true;
            }
            if ( nY>nRefEndY && bNoEndY )
            {
                nMaxY = nPosY - 1;
                bBottom = true;
            }
            bNoStartY = ( nY < nRefStartY );
            bNoEndY   = ( nY < nRefEndY );
            nPosY += pRowInfo[nArrY].nHeight;
        }

        long nPosX = nScrX;
        if ( bLayoutRTL )
            nPosX += nMirrorW - 1;      // always in pixels

        for (SCCOL nX=nX1; nX<=nX2+1; nX++)
        {
            if ( nX==nRefStartX )
            {
                nMinX = nPosX - nLayoutSign;
                bLeft = true;
            }
            if ( nX==nRefEndX )
            {
                nMaxX = nPosX + ( pRowInfo[0].pCellInfo[nX+1].nWidth - 1 ) * nLayoutSign;
                bRight = true;
            }
            nPosX += pRowInfo[0].pCellInfo[nX+1].nWidth * nLayoutSign;
        }

        if ( nMaxX * nLayoutSign >= nMinX * nLayoutSign &&
             nMaxY >= nMinY )
        {
            if ( nType == SC_CAT_DELETE_ROWS )
                bLeft = bRight = bBottom = false;       //! thick lines???
            else if ( nType == SC_CAT_DELETE_COLS )
                bTop = bBottom = bRight = false;        //! thick lines???

            mpDev->SetLineColor( rColor );
            if (bTop && bBottom && bLeft && bRight)
            {
                mpDev->SetFillColor();
                mpDev->DrawRect( Rectangle( nMinX, nMinY, nMaxX, nMaxY ) );
            }
            else
            {
                if (bTop)
                {
                    mpDev->DrawLine( Point( nMinX,nMinY ), Point( nMaxX,nMinY ) );
                    if ( nType == SC_CAT_DELETE_ROWS )
                        mpDev->DrawLine( Point( nMinX,nMinY+1 ), Point( nMaxX,nMinY+1 ) );
                }
                if (bBottom)
                    mpDev->DrawLine( Point( nMinX,nMaxY ), Point( nMaxX,nMaxY ) );
                if (bLeft)
                {
                    mpDev->DrawLine( Point( nMinX,nMinY ), Point( nMinX,nMaxY ) );
                    if ( nType == SC_CAT_DELETE_COLS )
                        mpDev->DrawLine( Point( nMinX+nLayoutSign,nMinY ), Point( nMinX+nLayoutSign,nMaxY ) );
                }
                if (bRight)
                    mpDev->DrawLine( Point( nMaxX,nMinY ), Point( nMaxX,nMaxY ) );
            }
            if ( bLeft && bTop )
            {
                mpDev->SetLineColor();
                mpDev->SetFillColor( rColor );
                mpDev->DrawRect( Rectangle( nMinX+nLayoutSign, nMinY+1, nMinX+3*nLayoutSign, nMinY+3 ) );
            }
        }
    }
}

void ScOutputData::DrawChangeTrack()
{
    ScChangeTrack* pTrack = mpDoc->GetChangeTrack();
    ScChangeViewSettings* pSettings = mpDoc->GetChangeViewSettings();
    if ( !pTrack || !pTrack->GetFirst() || !pSettings || !pSettings->ShowChanges() )
        return;         // nothing there or hidden

    ScActionColorChanger aColorChanger(*pTrack);

    //  clipping happens from the outside
    //! without clipping, only paínt affected cells ??!??!?

    SCCOL nEndX = nX2;
    SCROW nEndY = nY2;
    if ( nEndX < MAXCOL ) ++nEndX;      // also from the next cell since the mark
    if ( nEndY < MAXROW ) ++nEndY;      // protrudes from the preceding cell
    ScRange aViewRange( nX1, nY1, nTab, nEndX, nEndY, nTab );
    const ScChangeAction* pAction = pTrack->GetFirst();
    while (pAction)
    {
        ScChangeActionType eActionType;
        if ( pAction->IsVisible() )
        {
            eActionType = pAction->GetType();
            const ScBigRange& rBig = pAction->GetBigRange();
            if ( rBig.aStart.Tab() == nTab )
            {
                ScRange aRange = rBig.MakeRange();

                if ( eActionType == SC_CAT_DELETE_ROWS )
                    aRange.aEnd.SetRow( aRange.aStart.Row() );
                else if ( eActionType == SC_CAT_DELETE_COLS )
                    aRange.aEnd.SetCol( aRange.aStart.Col() );

                if ( aRange.Intersects( aViewRange ) &&
                     ScViewUtil::IsActionShown( *pAction, *pSettings, *mpDoc ) )
                {
                    aColorChanger.Update( *pAction );
                    Color aColor( aColorChanger.GetColor() );
                    DrawOneChange( aRange.aStart.Col(), aRange.aStart.Row(),
                                    aRange.aEnd.Col(), aRange.aEnd.Row(), aColor, sal::static_int_cast<sal_uInt16>(eActionType) );

                }
            }
            if ( eActionType == SC_CAT_MOVE &&
                    static_cast<const ScChangeActionMove*>(pAction)->
                        GetFromRange().aStart.Tab() == nTab )
            {
                ScRange aRange = static_cast<const ScChangeActionMove*>(pAction)->
                        GetFromRange().MakeRange();
                if ( aRange.Intersects( aViewRange ) &&
                     ScViewUtil::IsActionShown( *pAction, *pSettings, *mpDoc ) )
                {
                    aColorChanger.Update( *pAction );
                    Color aColor( aColorChanger.GetColor() );
                    DrawOneChange( aRange.aStart.Col(), aRange.aStart.Row(),
                                    aRange.aEnd.Col(), aRange.aEnd.Row(), aColor, sal::static_int_cast<sal_uInt16>(eActionType) );
                }
            }
        }

        pAction = pAction->GetNext();
    }
}

//TODO: moggi Need to check if this can't be written simpler
void ScOutputData::DrawNoteMarks(vcl::RenderContext& rRenderContext)
{

    bool bFirst = true;

    long nInitPosX = nScrX;
    if ( bLayoutRTL )
        nInitPosX += nMirrorW - 1;              // always in pixels
    long nLayoutSign = bLayoutRTL ? -1 : 1;

    long nPosY = nScrY;
    for (SCSIZE nArrY=1; nArrY+1<nArrCount; nArrY++)
    {
        RowInfo* pThisRowInfo = &pRowInfo[nArrY];
        if ( pThisRowInfo->bChanged )
        {
            long nPosX = nInitPosX;
            for (SCCOL nX=nX1; nX<=nX2; nX++)
            {
                CellInfo* pInfo = &pThisRowInfo->pCellInfo[nX+1];
                bool bIsMerged = false;

                if ( nX==nX1 && pInfo->bHOverlapped && !pInfo->bVOverlapped )
                {
                    // find start of merged cell
                    bIsMerged = true;
                    SCROW nY = pRowInfo[nArrY].nRowNo;
                    SCCOL nMergeX = nX;
                    SCROW nMergeY = nY;
                    mpDoc->ExtendOverlapped( nMergeX, nMergeY, nX, nY, nTab );
                    // use origin's pCell for NotePtr test below
                }

                if ( mpDoc->GetNote(nX, pRowInfo[nArrY].nRowNo, nTab) && ( bIsMerged ||
                        ( !pInfo->bHOverlapped && !pInfo->bVOverlapped ) ) )
                {
                    if (bFirst)
                    {
                        rRenderContext.SetLineColor(COL_WHITE);

                        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
                        if ( mbUseStyleColor && rStyleSettings.GetHighContrastMode() )
                            rRenderContext.SetFillColor( SC_MOD()->GetColorConfig().GetColorValue(svtools::FONTCOLOR).nColor );
                        else
                            rRenderContext.SetFillColor(COL_LIGHTRED);

                        bFirst = false;
                    }

                    long nMarkX = nPosX + ( pRowInfo[0].pCellInfo[nX+1].nWidth - 4 ) * nLayoutSign;
                    if ( bIsMerged || pInfo->bMerged )
                    {
                        //  if merged, add widths of all cells
                        SCCOL nNextX = nX + 1;
                        while ( nNextX <= nX2 + 1 && pThisRowInfo->pCellInfo[nNextX+1].bHOverlapped )
                        {
                            nMarkX += pRowInfo[0].pCellInfo[nNextX+1].nWidth * nLayoutSign;
                            ++nNextX;
                        }
                    }
                    if ( bLayoutRTL ? ( nMarkX >= 0 ) : ( nMarkX < nScrX+nScrW ) )
                        rRenderContext.DrawRect( Rectangle( nMarkX-5*nLayoutSign,nPosY,nMarkX+1*nLayoutSign,nPosY+6 ) );
                }

                nPosX += pRowInfo[0].pCellInfo[nX+1].nWidth * nLayoutSign;
            }
        }
        nPosY += pThisRowInfo->nHeight;
    }
}

void ScOutputData::AddPDFNotes()
{
    vcl::PDFExtOutDevData* pPDFData = dynamic_cast< vcl::PDFExtOutDevData* >( mpDev->GetExtOutDevData() );
    if ( !pPDFData || !pPDFData->GetIsExportNotes() )
        return;

    long nInitPosX = nScrX;
    if ( bLayoutRTL )
    {
        Size aOnePixel = mpDev->PixelToLogic(Size(1,1));
        long nOneX = aOnePixel.Width();
        nInitPosX += nMirrorW - nOneX;
    }
    long nLayoutSign = bLayoutRTL ? -1 : 1;

    long nPosY = nScrY;
    for (SCSIZE nArrY=1; nArrY+1<nArrCount; nArrY++)
    {
        RowInfo* pThisRowInfo = &pRowInfo[nArrY];
        if ( pThisRowInfo->bChanged )
        {
            long nPosX = nInitPosX;
            for (SCCOL nX=nX1; nX<=nX2; nX++)
            {
                CellInfo* pInfo = &pThisRowInfo->pCellInfo[nX+1];
                bool bIsMerged = false;
                SCROW nY = pRowInfo[nArrY].nRowNo;
                SCCOL nMergeX = nX;
                SCROW nMergeY = nY;

                if ( nX==nX1 && pInfo->bHOverlapped && !pInfo->bVOverlapped )
                {
                    // find start of merged cell
                    bIsMerged = true;
                    mpDoc->ExtendOverlapped( nMergeX, nMergeY, nX, nY, nTab );
                    // use origin's pCell for NotePtr test below
                }

                if ( mpDoc->GetNote(nMergeX, nMergeY, nTab) && ( bIsMerged ||
                        ( !pInfo->bHOverlapped && !pInfo->bVOverlapped ) ) )
                {
                    long nNoteWidth = (long)( SC_CLIPMARK_SIZE * mnPPTX );
                    long nNoteHeight = (long)( SC_CLIPMARK_SIZE * mnPPTY );

                    long nMarkX = nPosX + ( pRowInfo[0].pCellInfo[nX+1].nWidth - nNoteWidth ) * nLayoutSign;
                    if ( bIsMerged || pInfo->bMerged )
                    {
                        //  if merged, add widths of all cells
                        SCCOL nNextX = nX + 1;
                        while ( nNextX <= nX2 + 1 && pThisRowInfo->pCellInfo[nNextX+1].bHOverlapped )
                        {
                            nMarkX += pRowInfo[0].pCellInfo[nNextX+1].nWidth * nLayoutSign;
                            ++nNextX;
                        }
                    }
                    if ( bLayoutRTL ? ( nMarkX >= 0 ) : ( nMarkX < nScrX+nScrW ) )
                    {
                        Rectangle aNoteRect( nMarkX, nPosY, nMarkX+nNoteWidth*nLayoutSign, nPosY+nNoteHeight );
                        const ScPostIt* pNote = mpDoc->GetNote(nMergeX, nMergeY, nTab);

                        // Note title is the cell address (as on printed note pages)
                        ScAddress aAddress( nMergeX, nMergeY, nTab );
                        OUString aTitle(aAddress.Format(SCA_VALID, mpDoc, mpDoc->GetAddressConvention()));

                        // Content has to be a simple string without line breaks
                        OUString aContent = pNote->GetText();
                        aContent = aContent.replaceAll("\n", " ");

                        vcl::PDFNote aNote;
                        aNote.Title = aTitle;
                        aNote.Contents = aContent;
                        pPDFData->CreateNote( aNoteRect, aNote );
                    }
                }

                nPosX += pRowInfo[0].pCellInfo[nX+1].nWidth * nLayoutSign;
            }
        }
        nPosY += pThisRowInfo->nHeight;
    }
}

void ScOutputData::DrawClipMarks()
{
    if (!bAnyClipped)
        return;

    Color aArrowFillCol( COL_LIGHTRED );

    DrawModeFlags nOldDrawMode = mpDev->GetDrawMode();
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    if ( mbUseStyleColor && rStyleSettings.GetHighContrastMode() )
    {
        //  use DrawMode to change the arrow's outline color
        mpDev->SetDrawMode( nOldDrawMode | DrawModeFlags::SettingsLine );
        //  use text color also for the fill color
        aArrowFillCol.SetColor( SC_MOD()->GetColorConfig().GetColorValue(svtools::FONTCOLOR).nColor );
    }

    long nInitPosX = nScrX;
    if ( bLayoutRTL )
        nInitPosX += nMirrorW - 1;              // always in pixels
    long nLayoutSign = bLayoutRTL ? -1 : 1;

    Rectangle aCellRect;
    long nPosY = nScrY;
    for (SCSIZE nArrY=1; nArrY+1<nArrCount; nArrY++)
    {
        RowInfo* pThisRowInfo = &pRowInfo[nArrY];
        if ( pThisRowInfo->bChanged )
        {
            SCROW nY = pThisRowInfo->nRowNo;
            long nPosX = nInitPosX;
            for (SCCOL nX=nX1; nX<=nX2; nX++)
            {
                CellInfo* pInfo = &pThisRowInfo->pCellInfo[nX+1];
                if (pInfo->nClipMark)
                {
                    if (pInfo->bHOverlapped || pInfo->bVOverlapped)
                    {
                        //  merge origin may be outside of visible area - use document functions

                        SCCOL nOverX = nX;
                        SCROW nOverY = nY;
                        long nStartPosX = nPosX;
                        long nStartPosY = nPosY;

                        while ( nOverX > 0 && ( static_cast<const ScMergeFlagAttr*>(mpDoc->GetAttr(
                                nOverX, nOverY, nTab, ATTR_MERGE_FLAG ))->GetValue() & SC_MF_HOR ) )
                        {
                            --nOverX;
                            nStartPosX -= nLayoutSign * (long) ( mpDoc->GetColWidth(nOverX,nTab) * mnPPTX );
                        }

                        while ( nOverY > 0 && ( static_cast<const ScMergeFlagAttr*>(mpDoc->GetAttr(
                                nOverX, nOverY, nTab, ATTR_MERGE_FLAG ))->GetValue() & SC_MF_VER ) )
                        {
                            --nOverY;
                            nStartPosY -= nLayoutSign * (long) ( mpDoc->GetRowHeight(nOverY,nTab) * mnPPTY );
                        }

                        long nOutWidth = (long) ( mpDoc->GetColWidth(nOverX,nTab) * mnPPTX );
                        long nOutHeight = (long) ( mpDoc->GetRowHeight(nOverY,nTab) * mnPPTY );

                        const ScMergeAttr* pMerge = static_cast<const ScMergeAttr*>(
                                    mpDoc->GetAttr( nOverX, nOverY, nTab, ATTR_MERGE ));
                        SCCOL nCountX = pMerge->GetColMerge();
                        for (SCCOL i=1; i<nCountX; i++)
                            nOutWidth += (long) ( mpDoc->GetColWidth(nOverX+i,nTab) * mnPPTX );
                        SCROW nCountY = pMerge->GetRowMerge();
                        nOutHeight += (long) mpDoc->GetScaledRowHeight( nOverY+1, nOverY+nCountY-1, nTab, mnPPTY);

                        if ( bLayoutRTL )
                            nStartPosX -= nOutWidth - 1;
                        aCellRect = Rectangle( Point( nStartPosX, nStartPosY ), Size( nOutWidth, nOutHeight ) );
                    }
                    else
                    {
                        long nOutWidth = pRowInfo[0].pCellInfo[nX+1].nWidth;
                        long nOutHeight = pThisRowInfo->nHeight;

                        if ( pInfo->bMerged && pInfo->pPatternAttr )
                        {
                            SCCOL nOverX = nX;
                            SCROW nOverY = nY;
                            const ScMergeAttr* pMerge =
                                    static_cast<const ScMergeAttr*>(&pInfo->pPatternAttr->GetItem(ATTR_MERGE));
                            SCCOL nCountX = pMerge->GetColMerge();
                            for (SCCOL i=1; i<nCountX; i++)
                                nOutWidth += (long) ( mpDoc->GetColWidth(nOverX+i,nTab) * mnPPTX );
                            SCROW nCountY = pMerge->GetRowMerge();
                            nOutHeight += (long) mpDoc->GetScaledRowHeight( nOverY+1, nOverY+nCountY-1, nTab, mnPPTY);
                        }

                        long nStartPosX = nPosX;
                        if ( bLayoutRTL )
                            nStartPosX -= nOutWidth - 1;
                        // #i80447# create aCellRect from two points in case nOutWidth is 0
                        aCellRect = Rectangle( Point( nStartPosX, nPosY ),
                                               Point( nStartPosX+nOutWidth-1, nPosY+nOutHeight-1 ) );
                    }

                    aCellRect.Bottom() -= 1;    // don't paint over the cell grid
                    if ( bLayoutRTL )
                        aCellRect.Left() += 1;
                    else
                        aCellRect.Right() -= 1;

                    long nMarkPixel = (long)( SC_CLIPMARK_SIZE * mnPPTX );
                    Size aMarkSize( nMarkPixel, (nMarkPixel-1)*2 );

                    if ( pInfo->nClipMark & ( bLayoutRTL ? SC_CLIPMARK_RIGHT : SC_CLIPMARK_LEFT ) )
                    {
                        //  visually left
                        Rectangle aMarkRect = aCellRect;
                        aMarkRect.Right() = aCellRect.Left()+nMarkPixel-1;
                        SvxFont::DrawArrow( *mpDev, aMarkRect, aMarkSize, aArrowFillCol, true );
                    }
                    if ( pInfo->nClipMark & ( bLayoutRTL ? SC_CLIPMARK_LEFT : SC_CLIPMARK_RIGHT ) )
                    {
                        //  visually right
                        Rectangle aMarkRect = aCellRect;
                        aMarkRect.Left() = aCellRect.Right()-nMarkPixel+1;
                        SvxFont::DrawArrow( *mpDev, aMarkRect, aMarkSize, aArrowFillCol, false );
                    }
                }
                nPosX += pRowInfo[0].pCellInfo[nX+1].nWidth * nLayoutSign;
            }
        }
        nPosY += pThisRowInfo->nHeight;
    }

    mpDev->SetDrawMode(nOldDrawMode);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
