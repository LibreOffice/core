/*************************************************************************
 *
 *  $RCSfile: output.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:06:15 $
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

#include "scitems.hxx"
#include <svx/boxitem.hxx>
#include <svx/brshitem.hxx>
#include <svx/editdata.hxx>
#include <svtools/colorcfg.hxx>
#include <svx/rotmodit.hxx>
#include <svx/shaditem.hxx>
#include <svx/svxfont.hxx>
#include <so3/ipobj.hxx>
#include <tools/poly.hxx>
#include <vcl/svapp.hxx>
#include <svtools/accessibilityoptions.hxx>

#ifndef SVX_FRAMELINKARRAY_HXX
#include <svx/framelinkarray.hxx>
#endif

#include <math.h>

#include "output.hxx"
#include "document.hxx"
#include "cell.hxx"
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

// STATIC DATA -----------------------------------------------------------

//  Farben fuer ChangeTracking "nach Autor" wie im Writer (swmodul1.cxx)

#define SC_AUTHORCOLORCOUNT     9

static ColorData nAuthorColor[ SC_AUTHORCOLORCOUNT ] = {
                    COL_LIGHTRED,       COL_LIGHTBLUE,      COL_LIGHTMAGENTA,
                    COL_GREEN,          COL_RED,            COL_BLUE,
                    COL_BROWN,          COL_MAGENTA,        COL_CYAN };

//  Hilfsklasse, fuer die Farbzuordnung,
//  um nicht mehrfach hintereinander denselben User aus der Liste zu suchen

class ScActionColorChanger
{
private:
    const ScAppOptions&     rOpt;
    const StrCollection&    rUsers;
    String                  aLastUserName;
    USHORT                  nLastUserIndex;
    ColorData               nColor;

public:
                ScActionColorChanger( const ScChangeTrack& rTrack );
                ~ScActionColorChanger() {}

    void        Update( const ScChangeAction& rAction );
    ColorData   GetColor() const    { return nColor; }
};

//------------------------------------------------------------------

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
    if ( nSetColor != COL_TRANSPARENT )     // Farbe eingestellt
        nColor = nSetColor;
    else                                    // nach Autor
    {
        if ( rAction.GetUser() != aLastUserName )
        {
            aLastUserName = rAction.GetUser();
            StrData aData(aLastUserName);
            USHORT nIndex;
            if (!rUsers.Search(&aData, nIndex))
            {
                // empty string is possible if a name wasn't found while saving a 5.0 file
                DBG_ASSERT( aLastUserName.Len() == 0, "Author not found" );
                nIndex = 0;
            }
            nLastUserIndex = nIndex % SC_AUTHORCOLORCOUNT;
        }
        nColor = nAuthorColor[nLastUserIndex];
    }
}

//==================================================================

ScOutputData::ScOutputData( OutputDevice* pNewDev, ScOutputType eNewType,
                            ScTableInfo& rTabInfo, ScDocument* pNewDoc,
                            SCTAB nNewTab, long nNewScrX, long nNewScrY,
                            SCCOL nNewX1, SCROW nNewY1, SCCOL nNewX2, SCROW nNewY2,
                            double nPixelPerTwipsX, double nPixelPerTwipsY,
                            const Fraction* pZoomX, const Fraction* pZoomY ) :
    pDev( pNewDev ),
    pRefDevice( pNewDev ),      // default is output device
    pFmtDevice( pNewDev ),      // default is output device
    eType( eNewType ),
    mrTabInfo( rTabInfo ),
    pRowInfo( rTabInfo.mpRowInfo ),
    nArrCount( rTabInfo.mnArrCount ),
    pDoc( pNewDoc ),
    nTab( nNewTab ),
    nScrX( nNewScrX ),
    nScrY( nNewScrY ),
    nX1( nNewX1 ),
    nY1( nNewY1 ),
    nX2( nNewX2 ),
    nY2( nNewY2 ),
    nPPTX( nPixelPerTwipsX ),
    nPPTY( nPixelPerTwipsY ),
    bEditMode( FALSE ),
    bMetaFile( FALSE ),
    bPagebreakMode( FALSE ),
    bSolidBackground( FALSE ),
    bUseStyleColor( FALSE ),
    bForceAutoColor( SC_MOD()->GetAccessOptions().GetIsAutomaticFontColor() ),
    bSyntaxMode( FALSE ),
    pValueColor( NULL ),
    pTextColor( NULL ),
    pFormulaColor( NULL ),
    bSingleGrid( FALSE ),
    aGridColor( COL_BLACK ),
    bMarkClipped( FALSE ),          // FALSE fuer Drucker/Metafile etc.
    bShowNullValues( TRUE ),
    bShowFormulas( FALSE ),
    bSnapPixel( FALSE ),
    bShowSpellErrors( FALSE ),
    pEditObj( NULL ),
    pViewShell( NULL ),
    pDrawView( NULL ), // #114135#
    bAnyRotated( FALSE ),
    bAnyClipped( FALSE )
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
    pDoc->StripHidden( nVisX1, nVisY1, nVisX2, nVisY2, nTab );

    nScrW = 0;
    for (SCCOL nX=nVisX1; nX<=nVisX2; nX++)
        nScrW += pRowInfo[0].pCellInfo[nX+1].nWidth;

    nMirrorW = nScrW;

    nScrH = 0;
    for (SCSIZE nArrY=1; nArrY+1<nArrCount; nArrY++)
        nScrH += pRowInfo[nArrY].nHeight;

    bTabProtected = pDoc->IsTabProtected( nTab );
    nTabTextDirection = pDoc->GetEditTextDirection( nTab );
    bLayoutRTL = pDoc->IsLayoutRTL( nTab );
}

ScOutputData::~ScOutputData()
{
    delete pValueColor;
    delete pTextColor;
    delete pFormulaColor;
}

void ScOutputData::SetMirrorWidth( long nNew )
{
    nMirrorW = nNew;
}

void ScOutputData::SetGridColor( const Color& rColor )
{
    aGridColor = rColor;
}

void ScOutputData::SetMarkClipped( BOOL bSet )
{
    bMarkClipped = bSet;
}

void ScOutputData::SetShowNullValues( BOOL bSet )
{
    bShowNullValues = bSet;
}

void ScOutputData::SetShowFormulas( BOOL bSet )
{
    bShowFormulas = bSet;
}

void ScOutputData::SetShowSpellErrors( BOOL bSet )
{
    bShowSpellErrors = bSet;
}

void ScOutputData::SetSnapPixel( BOOL bSet )
{
    bSnapPixel = bSet;
}

void ScOutputData::SetEditCell( SCCOL nCol, SCROW nRow )
{
    nEditCol = nCol;
    nEditRow = nRow;
    bEditMode = TRUE;
}

void ScOutputData::SetMetaFileMode( BOOL bNewMode )
{
    bMetaFile = bNewMode;
}

void ScOutputData::SetSingleGrid( BOOL bNewMode )
{
    bSingleGrid = bNewMode;
}

void ScOutputData::SetSyntaxMode( BOOL bNewMode )
{
    bSyntaxMode = bNewMode;
    if (bNewMode)
        if (!pValueColor)
        {
            pValueColor = new Color( COL_LIGHTBLUE );
            pTextColor = new Color( COL_BLACK );
            pFormulaColor = new Color( COL_GREEN );
        }
}

void ScOutputData::DrawGrid( BOOL bGrid, BOOL bPage )
{
    SCCOL nX;
    SCROW nY;
    long nPosX;
    long nPosY;
    SCSIZE nArrY;
    BYTE nOldFlags = 0;
    BYTE nFlags;
    BOOL bSingle;
    Color aPageColor;
    Color aManualColor;

    if (bPagebreakMode)
        bPage = FALSE;          // keine "normalen" Umbrueche ueber volle Breite/Hoehe

    //! um den einen Pixel sieht das Metafile (oder die Druck-Ausgabe) anders aus
    //! als die Bildschirmdarstellung, aber wenigstens passen Druck und Metafile zusammen

    Size aOnePixel = pDev->PixelToLogic(Size(1,1));
    long nOneX = aOnePixel.Width();
    long nOneY = aOnePixel.Height();
    if (bMetaFile)
        nOneX = nOneY = 1;

    long nLayoutSign = bLayoutRTL ? -1 : 1;
    long nSignedOneX = nOneX * nLayoutSign;

    if ( eType == OUTTYPE_WINDOW )
    {
        const svtools::ColorConfig& rColorCfg = SC_MOD()->GetColorConfig();
        aPageColor.SetColor( rColorCfg.GetColorValue(svtools::CALCPAGEBREAKAUTOMATIC).nColor );
        aManualColor.SetColor( rColorCfg.GetColorValue(svtools::CALCPAGEBREAKMANUAL).nColor );
    }
    else
    {
        aPageColor = aGridColor;
        aManualColor = aGridColor;
    }

    pDev->SetLineColor( aGridColor );
    ScGridMerger aGrid( pDev, nOneX, nOneY );

                                        //
                                        //  Vertikale Linien
                                        //

    nPosX = nScrX;
    if ( bLayoutRTL )
        nPosX += nMirrorW - nOneX;

    for (nX=nX1; nX<=nX2; nX++)
    {
        SCCOL nXplus1 = nX+1;
        SCCOL nXplus2 = nX+2;
        USHORT nWidth = pRowInfo[0].pCellInfo[nXplus1].nWidth;
        if (nWidth)
        {
            nPosX += nWidth * nLayoutSign;

            if ( bPage )
            {
                //  Seitenumbrueche auch in ausgeblendeten suchen
                nFlags = 0;
                SCCOL nCol = nXplus1;
                while (nCol <= MAXCOL)
                {
                    BYTE nDocFl = pDoc->GetColFlags( nCol, nTab );
                    nFlags = nDocFl & ( CR_PAGEBREAK | CR_MANUALBREAK );
                    if ( nFlags || !(nDocFl & CR_HIDDEN) )
                        break;
                    ++nCol;
                }

                if (nFlags != nOldFlags)
                {
                    aGrid.Flush();
                    pDev->SetLineColor( (nFlags & CR_MANUALBREAK) ? aManualColor :
                                     (nFlags) ? aPageColor : aGridColor );
                    nOldFlags = nFlags;
                }
            }

            BOOL bDraw = bGrid || nOldFlags;    // einfaches Gitter nur wenn eingestellt

            //! Mit dieser Abfrage wird zuviel weggelassen, wenn ein automatischer
            //! Umbruch mitten in den Wiederholungsspalten liegt.
            //! Dann lieber den aeusseren Rahmen zweimal ausgeben...
#if 0
            //  auf dem Drucker die Aussen-Linien weglassen (werden getrennt ausgegeben)
            if ( eType == OUTTYPE_PRINTER && !bMetaFile )
            {
                if ( nX == MAXCOL )
                    bDraw = FALSE;
                else if (pDoc->GetColFlags(nXplus1,nTab) & ( CR_PAGEBREAK | CR_MANUALBREAK ))
                    bDraw = FALSE;
            }
#endif

            USHORT nWidthXplus2 = pRowInfo[0].pCellInfo[nXplus2].nWidth;
            bSingle = bSingleGrid;                                  //! in Fillinfo holen !!!!!
            if ( nX<MAXCOL && !bSingle )
            {
                bSingle = ( nWidthXplus2 == 0 );
                for (nArrY=1; nArrY+1<nArrCount && !bSingle; nArrY++)
                {
                    if (pRowInfo[nArrY].pCellInfo[nXplus2].bHOverlapped)
                        bSingle = TRUE;
                    if (pRowInfo[nArrY].pCellInfo[nXplus1].bHideGrid)
                        bSingle = TRUE;
                }
            }

            if (bDraw)
            {
                if ( nX<MAXCOL && bSingle )
                {
                    SCCOL nVisX = nXplus1;
                    while ( nVisX < MAXCOL && !pDoc->GetColWidth(nVisX,nTab) )
                        ++nVisX;

                    nPosY = nScrY;
                    long nNextY;
                    for (nArrY=1; nArrY+1<nArrCount; nArrY++)
                    {
                        RowInfo* pThisRowInfo = &pRowInfo[nArrY];
                        nNextY = nPosY + pThisRowInfo->nHeight;

                        BOOL bHOver = pThisRowInfo->pCellInfo[nXplus1].bHideGrid;
                        if (!bHOver)
                        {
                            if (nWidthXplus2)
                                bHOver = pThisRowInfo->pCellInfo[nXplus2].bHOverlapped;
                            else
                            {
                                if (nVisX <= nX2)
                                    bHOver = pThisRowInfo->pCellInfo[nVisX+1].bHOverlapped;
                                else
                                    bHOver = ((ScMergeFlagAttr*)pDoc->GetAttr(
                                                nVisX,pThisRowInfo->nRowNo,nTab,ATTR_MERGE_FLAG))
                                                ->IsHorOverlapped();
                                if (bHOver)
                                    bHOver = ((ScMergeFlagAttr*)pDoc->GetAttr(
                                                nXplus1,pThisRowInfo->nRowNo,nTab,ATTR_MERGE_FLAG))
                                                ->IsHorOverlapped();
                            }
                        }

                        if (pThisRowInfo->bChanged && !bHOver)
                        {
                            //Point aStart( nPosX-nSignedOneX, nPosY );
                            //Point aEnd( nPosX-nSignedOneX, nNextY-nOneY );
                            //pDev->DrawLine( aStart, aEnd );
                            aGrid.AddVerLine( nPosX-nSignedOneX, nPosY, nNextY-nOneY );
                        }
                        nPosY = nNextY;
                    }
                }
                else
                {
                    //Point aStart( nPosX-nSignedOneX, nScrY );
                    //Point aEnd( nPosX-nSignedOneX, nScrY+nScrH-nOneY );
                    //pDev->DrawLine( aStart, aEnd );
                    aGrid.AddVerLine( nPosX-nSignedOneX, nScrY, nScrY+nScrH-nOneY );
                }
            }
        }
    }

                                        //
                                        //  Horizontale Linien
                                        //

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
                //  Seitenumbrueche auch in ausgeblendeten suchen
                nFlags = 0;
                SCROW nRow = nYplus1;
                while (nRow <= MAXROW)
                {
                    BYTE nDocFl = pDoc->GetRowFlags( nRow, nTab );
                    nFlags = nDocFl & ( CR_PAGEBREAK | CR_MANUALBREAK );
                    if ( nFlags || !(nDocFl & CR_HIDDEN) )
                        break;
                    ++nRow;
                }

                if (nFlags != nOldFlags)
                {
                    aGrid.Flush();
                    pDev->SetLineColor( (nFlags & CR_MANUALBREAK) ? aManualColor :
                                     (nFlags) ? aPageColor : aGridColor );
                    nOldFlags = nFlags;
                }
            }

            BOOL bDraw = bGrid || nOldFlags;    // einfaches Gitter nur wenn eingestellt

            //! Mit dieser Abfrage wird zuviel weggelassen, wenn ein automatischer
            //! Umbruch mitten in den Wiederholungszeilen liegt.
            //! Dann lieber den aeusseren Rahmen zweimal ausgeben...
#if 0
            //  auf dem Drucker die Aussen-Linien weglassen (werden getrennt ausgegeben)
            if ( eType == OUTTYPE_PRINTER && !bMetaFile )
            {
                if ( nY == MAXROW )
                    bDraw = FALSE;
                else if (pDoc->GetRowFlags(nYplus1,nTab) & ( CR_PAGEBREAK | CR_MANUALBREAK ))
                    bDraw = FALSE;
            }
#endif

            BOOL bNextYisNextRow = (pRowInfo[nArrYplus1].nRowNo == nYplus1);
            bSingle = !bNextYisNextRow;             // Hidden
            for (SCCOL i=nX1; i<=nX2 && !bSingle; i++)
            {
                if (pRowInfo[nArrYplus1].pCellInfo[i+1].bVOverlapped)
                    bSingle = TRUE;
            }

            if (bDraw)
            {
                if ( bSingle && nY<MAXROW )
                {
                    SCROW nVisY = pRowInfo[nArrYplus1].nRowNo;

                    nPosX = nScrX;
                    if ( bLayoutRTL )
                        nPosX += nMirrorW - nOneX;

                    long nNextX;
                    for (SCCOL i=nX1; i<=nX2; i++)
                    {
                        nNextX = nPosX + pRowInfo[0].pCellInfo[i+1].nWidth * nLayoutSign;
                        if (nNextX != nPosX)                                // sichtbar
                        {
                            BOOL bVOver;
                            if ( bNextYisNextRow )
                                bVOver = pRowInfo[nArrYplus1].pCellInfo[i+1].bVOverlapped;
                            else
                            {
                                bVOver = ((ScMergeFlagAttr*)pDoc->GetAttr(
                                            i,nYplus1,nTab,ATTR_MERGE_FLAG))
                                            ->IsVerOverlapped()
                                    &&   ((ScMergeFlagAttr*)pDoc->GetAttr(
                                            i,nVisY,nTab,ATTR_MERGE_FLAG))
                                            ->IsVerOverlapped();
                                    //! nVisY aus Array ??
                            }
                            if (!bVOver)
                            {
                                //Point aStart( nPosX, nPosY-nOneY );
                                //Point aEnd( nNextX-nSignedOneX, nPosY-nOneY );
                                //pDev->DrawLine( aStart, aEnd );
                                aGrid.AddHorLine( nPosX, nNextX-nSignedOneX, nPosY-nOneY );
                            }
                        }
                        nPosX = nNextX;
                    }
                }
                else
                {
                    //Point aStart( nScrX, nPosY-nOneY );
                    //Point aEnd( nScrX+nScrW-nOneX, nPosY-nOneY );
                    //pDev->DrawLine( aStart, aEnd );
                    aGrid.AddHorLine( nScrX, nScrX+nScrW-nOneX, nPosY-nOneY );
                }
            }
        }
    }
}

//  ----------------------------------------------------------------------------

void ScOutputData::SetPagebreakMode( ScPageBreakData* pPageData )
{
    bPagebreakMode = TRUE;
    if (!pPageData)
        return;                     // noch nicht initialisiert -> alles "nicht gedruckt"

    //  gedruckten Bereich markieren
    //  (in FillInfo ist schon alles auf FALSE initialisiert)

    USHORT nRangeCount = pPageData->GetCount();
    for (USHORT nPos=0; nPos<nRangeCount; nPos++)
    {
        ScRange aRange = pPageData->GetData( nPos ).GetPrintRange();

        SCCOL nStartX = Max( aRange.aStart.Col(), nX1 );
        SCCOL nEndX   = Min( aRange.aEnd.Col(),   nX2 );
        SCROW nStartY = Max( aRange.aStart.Row(), nY1 );
        SCROW nEndY   = Min( aRange.aEnd.Row(),   nY2 );

        for (SCSIZE nArrY=1; nArrY+1<nArrCount; nArrY++)
        {
            RowInfo* pThisRowInfo = &pRowInfo[nArrY];
            if ( pThisRowInfo->bChanged && pThisRowInfo->nRowNo >= nStartY &&
                                           pThisRowInfo->nRowNo <= nEndY )
            {
                for (SCCOL nX=nStartX; nX<=nEndX; nX++)
                    pThisRowInfo->pCellInfo[nX+1].bPrinted = TRUE;
            }
        }
    }
}

void ScOutputData::FindRotated()
{
    //! nRotMax speichern
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

                if ( !pPattern && (pDoc->GetColFlags(nX,nTab) & CR_HIDDEN) == 0 )
                {
                    pPattern = pDoc->GetPattern( nX, nY, nTab );
                    pCondSet = pDoc->GetCondResult( nX, nY, nTab );
                }

                if ( pPattern )     // Spalte nicht ausgeblendet
                {
                    BYTE nDir = pPattern->GetRotateDir( pCondSet );
                    if (nDir != SC_ROTDIR_NONE)
                    {
                        pInfo->nRotateDir = nDir;
                        bAnyRotated = TRUE;
                    }
                }
            }
        }
    }
}

//  ----------------------------------------------------------------------------

USHORT lcl_GetRotateDir( ScDocument* pDoc, SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    const ScPatternAttr* pPattern = pDoc->GetPattern( nCol, nRow, nTab );
    const SfxItemSet* pCondSet = pDoc->GetCondResult( nCol, nRow, nTab );

    USHORT nRet = SC_ROTDIR_NONE;

    long nAttrRotate = pPattern->GetRotateVal( pCondSet );
    if ( nAttrRotate )
    {
        SvxRotateMode eRotMode = (SvxRotateMode)((const SvxRotateModeItem&)
                    pPattern->GetItem(ATTR_ROTATE_MODE, pCondSet)).GetValue();

        if ( eRotMode == SVX_ROTATE_MODE_STANDARD )
            nRet = SC_ROTDIR_STANDARD;
        else if ( eRotMode == SVX_ROTATE_MODE_CENTER )
            nRet = SC_ROTDIR_CENTER;
        else if ( eRotMode == SVX_ROTATE_MODE_TOP || eRotMode == SVX_ROTATE_MODE_BOTTOM )
        {
            long nRot180 = nAttrRotate % 18000;     // 1/100 Grad
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

const SvxBrushItem* lcl_FindBackground( ScDocument* pDoc, SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    const ScPatternAttr* pPattern = pDoc->GetPattern( nCol, nRow, nTab );
    const SfxItemSet* pCondSet = pDoc->GetCondResult( nCol, nRow, nTab );
    const SvxBrushItem* pBackground = (const SvxBrushItem*)
                            &pPattern->GetItem( ATTR_BACKGROUND, pCondSet );

    USHORT nDir = lcl_GetRotateDir( pDoc, nCol, nRow, nTab );

    //  CENTER wird wie RIGHT behandelt...
    if ( nDir == SC_ROTDIR_RIGHT || nDir == SC_ROTDIR_CENTER )
    {
        //  Text geht nach rechts -> Hintergrund von links nehmen
        while ( nCol > 0 && lcl_GetRotateDir( pDoc, nCol, nRow, nTab ) == nDir &&
                            pBackground->GetColor().GetTransparency() != 255 )
        {
            --nCol;
            pPattern = pDoc->GetPattern( nCol, nRow, nTab );
            pCondSet = pDoc->GetCondResult( nCol, nRow, nTab );
            pBackground = (const SvxBrushItem*)&pPattern->GetItem( ATTR_BACKGROUND, pCondSet );
        }
    }
    else if ( nDir == SC_ROTDIR_LEFT )
    {
        //  Text geht nach links -> Hintergrund von rechts nehmen
        while ( nCol < MAXCOL && lcl_GetRotateDir( pDoc, nCol, nRow, nTab ) == nDir &&
                            pBackground->GetColor().GetTransparency() != 255 )
        {
            ++nCol;
            pPattern = pDoc->GetPattern( nCol, nRow, nTab );
            pCondSet = pDoc->GetCondResult( nCol, nRow, nTab );
            pBackground = (const SvxBrushItem*)&pPattern->GetItem( ATTR_BACKGROUND, pCondSet );
        }
    }

    return pBackground;
}

//  ----------------------------------------------------------------------------

BOOL lcl_EqualBack( const RowInfo& rFirst, const RowInfo& rOther,
                    SCCOL nX1, SCCOL nX2, BOOL bShowProt, BOOL bPagebreakMode )
{
    if ( rFirst.bChanged   != rOther.bChanged ||
         rFirst.bEmptyBack != rOther.bEmptyBack )
        return FALSE;

    SCCOL nX;
    if ( bShowProt )
    {
        for ( nX=nX1; nX<=nX2; nX++ )
        {
            const ScPatternAttr* pPat1 = rFirst.pCellInfo[nX+1].pPatternAttr;
            const ScPatternAttr* pPat2 = rOther.pCellInfo[nX+1].pPatternAttr;
            if ( !pPat1 || !pPat2 ||
                    &pPat1->GetItem(ATTR_PROTECTION) != &pPat2->GetItem(ATTR_PROTECTION) )
                return FALSE;
        }
    }
    else
    {
        for ( nX=nX1; nX<=nX2; nX++ )
            if ( rFirst.pCellInfo[nX+1].pBackground != rOther.pCellInfo[nX+1].pBackground )
                return FALSE;
    }

    if ( rFirst.nRotMaxCol != SC_ROTMAX_NONE || rOther.nRotMaxCol != SC_ROTMAX_NONE )
        for ( nX=nX1; nX<=nX2; nX++ )
            if ( rFirst.pCellInfo[nX+1].nRotateDir != rOther.pCellInfo[nX+1].nRotateDir )
                return FALSE;

    if ( bPagebreakMode )
        for ( nX=nX1; nX<=nX2; nX++ )
            if ( rFirst.pCellInfo[nX+1].bPrinted != rOther.pCellInfo[nX+1].bPrinted )
                return FALSE;

    return TRUE;
}

void ScOutputData::DrawBackground()
{
    FindRotated();              //! von aussen ?

    ScModule* pScMod = SC_MOD();

    // used only if bSolidBackground is set (only for ScGridWindow):
    Color aBgColor( pScMod->GetColorConfig().GetColorValue(svtools::DOCCOLOR).nColor );

    Rectangle aRect;
    Size aOnePixel = pDev->PixelToLogic(Size(1,1));
    long nOneX = aOnePixel.Width();
    long nOneY = aOnePixel.Height();

    if (bMetaFile)
        nOneX = nOneY = 0;

    long nLayoutSign = bLayoutRTL ? -1 : 1;
    long nSignedOneX = nOneX * nLayoutSign;

    pDev->SetLineColor();

    BOOL bShowProt = bSyntaxMode && pDoc->IsTabProtected(nTab);
    BOOL bDoAll = bShowProt || bPagebreakMode || bSolidBackground;

    //  #105733# SvtAccessibilityOptions::GetIsForBorders is no longer used (always assumed TRUE)
    BOOL bCellContrast = bUseStyleColor &&
            Application::GetSettings().GetStyleSettings().GetHighContrastMode();

    long nPosY = nScrY;
    for (SCSIZE nArrY=1; nArrY+1<nArrCount; nArrY++)
    {
        RowInfo* pThisRowInfo = &pRowInfo[nArrY];
        long nRowHeight = pThisRowInfo->nHeight;

        if ( pThisRowInfo->bChanged )
        {
            if ( ( ( pThisRowInfo->bEmptyBack ) || bSyntaxMode ) && !bDoAll )
            {
                //  nichts
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
                aRect = Rectangle( nPosX,nPosY, nPosX,nPosY+nRowHeight-nOneY );

                const SvxBrushItem* pOldBackground = NULL;
                const SvxBrushItem* pBackground;
                for (SCCOL nX=nX1; nX<=nX2; nX++)
                {
                    CellInfo* pInfo = &pThisRowInfo->pCellInfo[nX+1];

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
                            const ScProtectionAttr& rProt = (const ScProtectionAttr&)
                                                                pP->GetItem(ATTR_PROTECTION);
                            if (rProt.GetProtection() || rProt.GetHideCell())
                                pBackground = ScGlobal::GetProtectedBrushItem();
                            else
                                pBackground = ScGlobal::GetEmptyBrushItem();
                        }
                        else
                            pBackground = NULL;
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
                        pBackground = lcl_FindBackground( pDoc, nX, nY, nTab );
                    }

                    if ( pBackground != pOldBackground )
                    {
                        aRect.Right() = nPosX-nSignedOneX;
                        if (pOldBackground)             // ==0 if hidden
                        {
                            Color aBackCol = pOldBackground->GetColor();
                            if ( bSolidBackground && aBackCol.GetTransparency() )
                                aBackCol = aBgColor;
                            if ( !aBackCol.GetTransparency() )      //! partial transparency?
                            {
                                pDev->SetFillColor( aBackCol );
                                pDev->DrawRect( aRect );
                            }
                        }
                        aRect.Left() = nPosX;
                        pOldBackground = pBackground;
                    }
                    nPosX += pRowInfo[0].pCellInfo[nX+1].nWidth * nLayoutSign;
                }
                aRect.Right() = nPosX-nSignedOneX;
                if (pOldBackground)
                {
                    Color aBackCol = pOldBackground->GetColor();
                    if ( bSolidBackground && aBackCol.GetTransparency() )
                        aBackCol = aBgColor;
                    if ( !aBackCol.GetTransparency() )      //! partial transparency?
                    {
                        pDev->SetFillColor( aBackCol );
                        pDev->DrawRect( aRect );
                    }
                }

                nArrY += nSkip;
            }
        }
        nPosY += nRowHeight;
    }
}

void ScOutputData::DrawShadow()
{
    DrawExtraShadow( FALSE, FALSE, FALSE, FALSE );
}

void ScOutputData::DrawExtraShadow(BOOL bLeft, BOOL bTop, BOOL bRight, BOOL bBottom)
{
    pDev->SetLineColor();

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    //  #105733# SvtAccessibilityOptions::GetIsForBorders is no longer used (always assumed TRUE)
    BOOL bCellContrast = bUseStyleColor && rStyleSettings.GetHighContrastMode();
    Color aAutoTextColor;
    if ( bCellContrast )
        aAutoTextColor.SetColor( SC_MOD()->GetColorConfig().GetColorValue(svtools::FONTCOLOR).nColor );

    long nInitPosX = nScrX;
    if ( bLayoutRTL )
    {
        Size aOnePixel = pDev->PixelToLogic(Size(1,1));
        long nOneX = aOnePixel.Width();
        nInitPosX += nMirrorW - nOneX;
    }
    long nLayoutSign = bLayoutRTL ? -1 : 1;

    long nPosY = nScrY - pRowInfo[0].nHeight;
    for (SCSIZE nArrY=0; nArrY<nArrCount; nArrY++)
    {
        BOOL bCornerY = ( nArrY == 0 ) || ( nArrY+1 == nArrCount );
        BOOL bSkipY = ( nArrY==0 && !bTop ) || ( nArrY+1 == nArrCount && !bBottom );

        RowInfo* pThisRowInfo = &pRowInfo[nArrY];
        long nRowHeight = pThisRowInfo->nHeight;

        if ( pThisRowInfo->bChanged && !bSkipY )
        {
            long nPosX = nInitPosX - pRowInfo[0].pCellInfo[nX1].nWidth * nLayoutSign;
            for (SCCOL nArrX=nX1; nArrX<=nX2+2; nArrX++)
            {
                BOOL bCornerX = ( nArrX==nX1 || nArrX==nX2+2 );
                BOOL bSkipX = ( nArrX==nX1 && !bLeft ) || ( nArrX==nX2+2 && !bRight );

                for (USHORT nPass=0; nPass<2; nPass++)          // horizontal / vertikal
                {
                    const SvxShadowItem* pAttr = nPass ?
                            pThisRowInfo->pCellInfo[nArrX].pVShadowOrigin :
                            pThisRowInfo->pCellInfo[nArrX].pHShadowOrigin;
                    if ( pAttr && !bSkipX )
                    {
                        ScShadowPart ePart = nPass ?
                                pThisRowInfo->pCellInfo[nArrX].eVShadowPart :
                                pThisRowInfo->pCellInfo[nArrX].eHShadowPart;

                        BOOL bDo = TRUE;
                        if ( (nPass==0 && bCornerX) || (nPass==1 && bCornerY) )
                            if ( ePart != SC_SHADOW_CORNER )
                                bDo = FALSE;

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

//                          Rectangle aRect( Point(nPosX,nPosY),
//                                           Size( pRowInfo[0].pCellInfo[nArrX].nWidth,
//                                                  pRowInfo[nArrY].nHeight ) );

                            // rectangle is in logical orientation
                            Rectangle aRect( nPosX, nPosY,
                                             nPosX + ( nThisWidth - 1 ) * nLayoutSign,
                                             nPosY + pRowInfo[nArrY].nHeight - 1 );

                            long nSize = pAttr->GetWidth();
                            long nSizeX = (long)(nSize*nPPTX);
                            if (nSizeX >= nMaxWidth) nSizeX = nMaxWidth-1;
                            long nSizeY = (long)(nSize*nPPTY);
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
                            pDev->SetFillColor( bCellContrast ? aAutoTextColor : pAttr->GetColor() );
                            pDev->DrawRect( aRect );
                        }
                    }
                }

                nPosX += pRowInfo[0].pCellInfo[nArrX].nWidth * nLayoutSign;
            }
        }
        nPosY += nRowHeight;
    }
}

//
//  Loeschen
//

void ScOutputData::DrawClear()
{
    Rectangle aRect;
    Size aOnePixel = pDev->PixelToLogic(Size(1,1));
    long nOneX = aOnePixel.Width();
    long nOneY = aOnePixel.Height();

    // (called only for ScGridWindow)
    Color aBgColor( SC_MOD()->GetColorConfig().GetColorValue(svtools::DOCCOLOR).nColor );

    if (bMetaFile)
        nOneX = nOneY = 0;

    pDev->SetLineColor();

    pDev->SetFillColor( aBgColor );

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
            pDev->DrawRect( aRect );

            nArrY += nSkip;
        }
        nPosY += nRowHeight;
    }
}


//
//  Linien
//

long lclGetSnappedX( OutputDevice& rDev, long nPosX, bool bSnapPixel )
{
    return (bSnapPixel && nPosX) ? rDev.PixelToLogic( rDev.LogicToPixel( Size( nPosX, 0 ) ) ).Width() : nPosX;
}

long lclGetSnappedY( OutputDevice& rDev, long nPosY, bool bSnapPixel )
{
    return (bSnapPixel && nPosY) ? rDev.PixelToLogic( rDev.LogicToPixel( Size( 0, nPosY ) ) ).Height() : nPosY;
}

size_t lclGetArrayColFromCellInfoX( USHORT nCellInfoX, USHORT nCellInfoFirstX, USHORT nCellInfoLastX, bool bRTL )
{
    return static_cast< size_t >( bRTL ? (nCellInfoLastX + 2 - nCellInfoX) : (nCellInfoX - nCellInfoFirstX) );
}

USHORT lclGetCellInfoXFromArrayCol( size_t nCol, USHORT nCellInfoFirstX, USHORT nCellInfoLastX, bool bRTL )
{
    return static_cast< USHORT >( bRTL ? (nCellInfoLastX + 2 - nCol) : (nCol + nCellInfoFirstX) );
}

void ScOutputData::DrawFrame()
{
    ULONG nOldDrawMode = pDev->GetDrawMode();

    Color aSingleColor;
    BOOL bUseSingleColor = FALSE;
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    //  #105733# SvtAccessibilityOptions::GetIsForBorders is no longer used (always assumed TRUE)
    BOOL bCellContrast = bUseStyleColor && rStyleSettings.GetHighContrastMode();

    //  #107519# if a Calc OLE object is embedded in Draw/Impress, the VCL DrawMode is used
    //  for display mode / B&W printing. The VCL DrawMode handling doesn't work for lines
    //  that are drawn with DrawRect, so if the line/background bits are set, the DrawMode
    //  must be reset and the border colors handled here.
    //  (Similar to fix for #72796# in SdrObject::ImpDrawLineGeometry)

    if ( ( nOldDrawMode & DRAWMODE_WHITEFILL ) && ( nOldDrawMode & DRAWMODE_BLACKLINE ) )
    {
        pDev->SetDrawMode( nOldDrawMode & (~DRAWMODE_WHITEFILL) );
        aSingleColor.SetColor( COL_BLACK );
        bUseSingleColor = TRUE;
    }
    else if ( ( nOldDrawMode & DRAWMODE_SETTINGSFILL ) && ( nOldDrawMode & DRAWMODE_SETTINGSLINE ) )
    {
        pDev->SetDrawMode( nOldDrawMode & (~DRAWMODE_SETTINGSFILL) );
        aSingleColor = rStyleSettings.GetWindowTextColor();     // same as used in VCL for DRAWMODE_SETTINGSLINE
        bUseSingleColor = TRUE;
    }
    else if ( bCellContrast )
    {
        aSingleColor.SetColor( SC_MOD()->GetColorConfig().GetColorValue(svtools::FONTCOLOR).nColor );
        bUseSingleColor = TRUE;
    }

    const Color* pForceColor = bUseSingleColor ? &aSingleColor : 0;

    if (bAnyRotated)
        DrawRotatedFrame( pForceColor );        // removes the lines that must not be painted here

    long nInitPosX = nScrX;
    if ( bLayoutRTL )
    {
        Size aOnePixel = pDev->PixelToLogic(Size(1,1));
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
    long nOldSnapY = lclGetSnappedY( *pDev, nOldPosY, bSnapPixel );
    rArray.SetYOffset( nOldSnapY );
    for( size_t nRow = 0; nRow < nRowCount; ++nRow )
    {
        long nNewPosY = nOldPosY + pRowInfo[ nRow ].nHeight;
        long nNewSnapY = lclGetSnappedY( *pDev, nNewPosY, bSnapPixel );
        rArray.SetRowHeight( nRow, nNewSnapY - nOldSnapY );
        nOldPosY = nNewPosY;
        nOldSnapY = nNewSnapY;
    }

    // column widths

    // column nX1 is not visible (dummy for borders from left) - subtract its width from initial position
    // subtract 1 unit more, because position 0 is first *in* cell, grid line is one unit above
    long nOldPosX = nInitPosX - nLayoutSign * (1 + pRowInfo[ 0 ].pCellInfo[ nX1 ].nWidth);
    long nOldSnapX = lclGetSnappedX( *pDev, nOldPosX, bSnapPixel );
    // set X offset for left-to-right sheets; for right-to-left sheets this is done after for() loop
    if( !bLayoutRTL )
        rArray.SetXOffset( nOldSnapX );
    for( USHORT nInfoIdx = nX1; nInfoIdx <= nX2 + 2; ++nInfoIdx )
    {
        size_t nCol = lclGetArrayColFromCellInfoX( nInfoIdx, nX1, nX2, bLayoutRTL );
        long nNewPosX = nOldPosX + pRowInfo[ 0 ].pCellInfo[ nInfoIdx ].nWidth * nLayoutSign;
        long nNewSnapX = lclGetSnappedX( *pDev, nNewPosX, bSnapPixel );
        rArray.SetColWidth( nCol, Abs( nNewSnapX - nOldSnapX ) );
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
    while( nRow1 <= nLastRow )
    {
        while( (nRow1 <= nLastRow) && !pRowInfo[ nRow1 ].bChanged ) ++nRow1;
        if( nRow1 <= nLastRow )
        {
            size_t nRow2 = nRow1;
            while( (nRow2 + 1 <= nLastRow) && pRowInfo[ nRow2 + 1 ].bChanged ) ++nRow2;
            rArray.DrawRange( *pDev, nFirstCol, nRow1, nLastCol, nRow2, pForceColor );
            nRow1 = nRow2 + 1;
        }
    }

    pDev->SetDrawMode(nOldDrawMode);
}

//  -------------------------------------------------------------------------

//  Linie unter der Zelle

const SvxBorderLine* lcl_FindHorLine( ScDocument* pDoc,
                        SCCOL nCol, SCROW nRow, SCTAB nTab, USHORT nRotDir,
                        BOOL bTopLine )
{
    if ( nRotDir != SC_ROTDIR_LEFT && nRotDir != SC_ROTDIR_RIGHT )
        return NULL;

    BOOL bFound = FALSE;
    while (!bFound)
    {
        if ( nRotDir == SC_ROTDIR_LEFT )
        {
            //  Text nach links -> Linie von rechts
            if ( nCol < MAXCOL )
                ++nCol;
            else
                return NULL;                // war nix
        }
        else
        {
            //  Text nach rechts -> Linie von links
            if ( nCol > 0 )
                --nCol;
            else
                return NULL;                // war nix
        }
        const ScPatternAttr* pPattern = pDoc->GetPattern( nCol, nRow, nTab );
        const SfxItemSet* pCondSet = pDoc->GetCondResult( nCol, nRow, nTab );
        if ( !pPattern->GetRotateVal( pCondSet ) ||
                ((const SvxRotateModeItem&)pPattern->GetItem(
                    ATTR_ROTATE_MODE, pCondSet)).GetValue() == SVX_ROTATE_MODE_STANDARD )
            bFound = TRUE;
    }

    if (bTopLine)
        --nRow;
    const SvxBorderLine* pThisBottom;
    if ( ValidRow(nRow) )
        pThisBottom = ((const SvxBoxItem*)pDoc->GetAttr( nCol, nRow, nTab, ATTR_BORDER ))->GetBottom();
    else
        pThisBottom = NULL;
    const SvxBorderLine* pNextTop;
    if ( nRow < MAXROW )
        pNextTop = ((const SvxBoxItem*)pDoc->GetAttr( nCol, nRow+1, nTab, ATTR_BORDER ))->GetTop();
    else
        pNextTop = NULL;

    if ( HasPriority( pThisBottom, pNextTop ) )
        return pThisBottom;
    else
        return pNextTop;
}

// lcl_HorizLine muss genau zu normal ausgegebenen Linien passen!

void lcl_HorizLine( OutputDevice& rDev, const Point& rLeft, const Point& rRight,
                    const svx::frame::Style& rLine, const Color* pForceColor )
{
    svx::frame::DrawHorFrameBorder( rDev, rLeft, rRight, rLine, pForceColor );
}

void lcl_VertLineEnds( OutputDevice& rDev, const Point& rTop, const Point& rBottom,
        const Color& rColor, long nXOffs, long nWidth,
        const svx::frame::Style& rTopLine, const svx::frame::Style& rBottomLine )
{
    rDev.SetLineColor(rColor);              // PEN_NULL ???
    rDev.SetFillColor(rColor);

    //  Position oben/unten muss unabhaengig von der Liniendicke sein,
    //  damit der Winkel stimmt (oder X-Position auch anpassen)
    long nTopPos = rTop.Y();
    long nBotPos = rBottom.Y();

    long nTopLeft = rTop.X() + nXOffs;
    long nTopRight = nTopLeft + nWidth - 1;

    long nBotLeft = rBottom.X() + nXOffs;
    long nBotRight = nBotLeft + nWidth - 1;

    //  oben abschliessen

    if ( rTopLine.Prim() )
    {
        long nLineW = rTopLine.GetWidth();
        if (nLineW >= 2)
        {
            Point aTriangle[3];
            aTriangle[0] = Point( nTopLeft, nTopPos );      // wie aPoints[0]
            aTriangle[1] = Point( nTopRight, nTopPos );     // wie aPoints[1]
            aTriangle[2] = Point( rTop.X(), nTopPos - (nLineW - 1) / 2 );
            Polygon aTriPoly( 3, aTriangle );
            rDev.DrawPolygon( aTriPoly );
        }
    }

    //  unten abschliessen

    if ( rBottomLine.Prim() )
    {
        long nLineW = rBottomLine.GetWidth();
        if (nLineW >= 2)
        {
            Point aTriangle[3];
            aTriangle[0] = Point( nBotLeft, nBotPos );      // wie aPoints[3]
            aTriangle[1] = Point( nBotRight, nBotPos );     // wie aPoints[2]
            aTriangle[2] = Point( rBottom.X(), nBotPos - (nLineW - 1) / 2 + nLineW - 1 );
            Polygon aTriPoly( 3, aTriangle );
            rDev.DrawPolygon( aTriPoly );
        }
    }
}

void lcl_VertLine( OutputDevice& rDev, const Point& rTop, const Point& rBottom,
                    const svx::frame::Style& rLine,
                    const svx::frame::Style& rTopLine, const svx::frame::Style& rBottomLine,
                    const Color* pForceColor )
{
    if( rLine.Prim() )
    {
        svx::frame::DrawVerFrameBorderSlanted( rDev, rTop, rBottom, rLine, pForceColor );

        svx::frame::Style aScaled( rLine );
        aScaled.ScaleSelf( 1.0 / cos( svx::frame::GetVerDiagAngle( rTop, rBottom ) ) );
        if( pForceColor )
            aScaled.SetColor( *pForceColor );

        long nXOffs = (aScaled.GetWidth() - 1) / -2L;

        lcl_VertLineEnds( rDev, rTop, rBottom, aScaled.GetColor(),
            nXOffs, aScaled.Prim(), rTopLine, rBottomLine );

        if( aScaled.Secn() )
            lcl_VertLineEnds( rDev, rTop, rBottom, aScaled.GetColor(),
                nXOffs + aScaled.Prim() + aScaled.Dist(), aScaled.Secn(), rTopLine, rBottomLine );
    }
}

void ScOutputData::DrawRotatedFrame( const Color* pForceColor )
{
    //! nRotMax speichern
    SCCOL nRotMax = nX2;
    for (SCSIZE nRotY=0; nRotY<nArrCount; nRotY++)
        if (pRowInfo[nRotY].nRotMaxCol != SC_ROTMAX_NONE && pRowInfo[nRotY].nRotMaxCol > nRotMax)
            nRotMax = pRowInfo[nRotY].nRotMaxCol;

    const ScPatternAttr* pPattern;
    const SfxItemSet*    pCondSet;
    const ScPatternAttr* pOldPattern = NULL;
    const SfxItemSet*    pOldCondSet = NULL;

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    //  #105733# SvtAccessibilityOptions::GetIsForBorders is no longer used (always assumed TRUE)
    BOOL bCellContrast = bUseStyleColor && rStyleSettings.GetHighContrastMode();

    //  color (pForceColor) is determined externally, including DrawMode changes

    long nInitPosX = nScrX;
    if ( bLayoutRTL )
    {
        Size aOnePixel = pDev->PixelToLogic(Size(1,1));
        long nOneX = aOnePixel.Width();
        nInitPosX += nMirrorW - nOneX;
    }
    long nLayoutSign = bLayoutRTL ? -1 : 1;

    Rectangle aClipRect( Point(nScrX, nScrY), Size(nScrW, nScrH) );
    if (bMetaFile)
    {
        pDev->Push();
        pDev->IntersectClipRegion( aClipRect );
    }
    else
        pDev->SetClipRegion( Region( aClipRect ) );

    svx::frame::Array& rArray = mrTabInfo.maArray;

    long nPosY = nScrY;
    for (SCSIZE nArrY=1; nArrY<nArrCount; nArrY++)
    {
        //  Rotated wird auch 1 Zeile ueber/unter Changed gezeichnet, falls Teile
        //  in die Zeile hineinragen...

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

                USHORT nArrX = nX + 1;

                CellInfo* pInfo = &rThisRowInfo.pCellInfo[nArrX];
                long nColWidth = pRowInfo[0].pCellInfo[nArrX].nWidth;
                if ( pInfo->nRotateDir > SC_ROTDIR_STANDARD &&
                        !pInfo->bHOverlapped && !pInfo->bVOverlapped )
                {
                    pPattern = pInfo->pPatternAttr;
                    pCondSet = pInfo->pConditionSet;
                    if (!pPattern)
                    {
                        pPattern = pDoc->GetPattern( nX, nY, nTab );
                        pInfo->pPatternAttr = pPattern;
                        pCondSet = pDoc->GetCondResult( nX, nY, nTab );
                        pInfo->pConditionSet = pCondSet;
                    }

                    //! LastPattern etc.

                    long nAttrRotate = pPattern->GetRotateVal( pCondSet );
                    SvxRotateMode eRotMode = (SvxRotateMode)((const SvxRotateModeItem&)
                                    pPattern->GetItem(ATTR_ROTATE_MODE, pCondSet)).GetValue();

                    if ( nAttrRotate )
                    {
                        if (nX<nX1)         // negative Position berechnen
                        {
                            nPosX = nInitPosX;
                            SCCOL nCol = nX1;
                            while (nCol > nX)
                            {
                                --nCol;
                                nPosX -= nLayoutSign * (long) pRowInfo[0].pCellInfo[nCol+1].nWidth;
                            }
                        }

                        //  Startposition minus 1, damit auch schraege Hintergruende
                        //  zur Umrandung passen (Umrandung ist auf dem Gitter)

                        long nTop = nPosY - 1;
                        long nBottom = nPosY + nRowHeight - 1;
                        long nTopLeft = nPosX - nLayoutSign;
                        long nTopRight = nPosX + ( nColWidth - 1 ) * nLayoutSign;
                        long nBotLeft = nTopLeft;
                        long nBotRight = nTopRight;

                        //  inclusion of the sign here hasn't been decided yet
                        //  (if not, the extension of the non-rotated background must also be changed)
                        double nRealOrient = nLayoutSign * nAttrRotate * F_PI18000;     // 1/100th degrees
                        double nCos = cos( nRealOrient );
                        double nSin = sin( nRealOrient );
                        //! begrenzen !!!
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
                        }

                        Point aPoints[4];
                        aPoints[0] = Point( nTopLeft, nTop );
                        aPoints[1] = Point( nTopRight, nTop );
                        aPoints[2] = Point( nBotRight, nBottom );
                        aPoints[3] = Point( nBotLeft, nBottom );

                        const SvxBrushItem* pBackground = pInfo->pBackground;
                        if (!pBackground)
                            pBackground = (const SvxBrushItem*) &pPattern->GetItem(
                                                ATTR_BACKGROUND, pCondSet );
                        if (bCellContrast)
                        {
                            //  high contrast for cell borders and backgrounds -> empty background
                            pBackground = ScGlobal::GetEmptyBrushItem();
                        }
                        const Color& rColor = pBackground->GetColor();
                        if ( rColor.GetTransparency() != 255 )
                        {
                            //  #95879# draw background only for the changed row itself
                            //  (background doesn't extend into other cells).
                            //  For the borders (rotated and normal), clipping should be
                            //  set if the row isn't changed, but at least the borders
                            //  don't cover the cell contents.
                            if ( rThisRowInfo.bChanged )
                            {
                                Polygon aPoly( 4, aPoints );

                                //  ohne Pen wird bei DrawPolygon rechts und unten
                                //  ein Pixel weggelassen...
                                if ( rColor.GetTransparency() == 0 )
                                    pDev->SetLineColor(rColor);
                                else
                                    pDev->SetLineColor();
                                pDev->SetFillColor(rColor);
                                pDev->DrawPolygon( aPoly );
                            }
                        }

                        svx::frame::Style aTopLine, aBottomLine, aLeftLine, aRightLine;

                        if ( nX < nX1 || nX > nX2 )     // Attribute in FillInfo nicht gesetzt
                        {
                            //! Seitengrenzen fuer Druck beruecksichtigen !!!!!
                            const SvxBorderLine* pLeftLine;
                            const SvxBorderLine* pTopLine;
                            const SvxBorderLine* pRightLine;
                            const SvxBorderLine* pBottomLine;
                            pDoc->GetBorderLines( nX, nY, nTab,
                                    &pLeftLine, &pTopLine, &pRightLine, &pBottomLine );
                            aTopLine.Set( pTopLine, nPPTY );
                            aBottomLine.Set( pBottomLine, nPPTY );
                            aLeftLine.Set( pLeftLine, nPPTX );
                            aRightLine.Set( pRightLine, nPPTX );
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

                        lcl_HorizLine( *pDev, aPoints[bLayoutRTL?1:0], aPoints[bLayoutRTL?0:1], aTopLine, pForceColor );
                        lcl_HorizLine( *pDev, aPoints[bLayoutRTL?2:3], aPoints[bLayoutRTL?3:2], aBottomLine, pForceColor );

                        lcl_VertLine( *pDev, aPoints[0], aPoints[3], aLeftLine, aTopLine, aBottomLine, pForceColor );
                        lcl_VertLine( *pDev, aPoints[1], aPoints[2], aRightLine, aTopLine, aBottomLine, pForceColor );
                    }
                }
                nPosX += nColWidth * nLayoutSign;
            }

            //  erst hinterher im zweiten Schritt die Linien fuer normale Ausgabe loeschen

            nX = nX1 > 0 ? (nX1-1) : static_cast<SCCOL>(0);
            for (; nX<=nX2+1; nX++)         // sichtbarer Teil +- 1
            {
                USHORT nArrX = nX + 1;
                CellInfo& rInfo = rThisRowInfo.pCellInfo[nArrX];
                if ( rInfo.nRotateDir > SC_ROTDIR_STANDARD &&
                        !rInfo.bHOverlapped && !rInfo.bVOverlapped )
                {
                    pPattern = rInfo.pPatternAttr;
                    pCondSet = rInfo.pConditionSet;
                    SvxRotateMode eRotMode = (SvxRotateMode)((const SvxRotateModeItem&)
                                    pPattern->GetItem(ATTR_ROTATE_MODE, pCondSet)).GetValue();

                    size_t nCol = lclGetArrayColFromCellInfoX( nArrX, nX1, nX2, bLayoutRTL );

                    //  horizontal: angrenzende Linie verlaengern
                    //  (nur, wenn die gedrehte Zelle eine Umrandung hat)
                    USHORT nDir = rInfo.nRotateDir;
                    if ( rArray.GetCellStyleTop( nCol, nRow ).Prim() && eRotMode != SVX_ROTATE_MODE_TOP )
                    {
                        svx::frame::Style aStyle( lcl_FindHorLine( pDoc, nX, nY, nTab, nDir, TRUE ), nPPTY );
                        rArray.SetCellStyleTop( nCol, nRow, aStyle );
                        if( nRow > 0 )
                            rArray.SetCellStyleBottom( nCol, nRow - 1, aStyle );
                    }
                    if ( rArray.GetCellStyleBottom( nCol, nRow ).Prim() && eRotMode != SVX_ROTATE_MODE_BOTTOM )
                    {
                        svx::frame::Style aStyle( lcl_FindHorLine( pDoc, nX, nY, nTab, nDir, FALSE ), nPPTY );
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

    if (bMetaFile)
        pDev->Pop();
    else
        pDev->SetClipRegion();
}

//  Drucker

void ScOutputData::DrawPageBorder( SCCOL nStartX, SCROW nStartY, SCCOL nEndX, SCROW nEndY )
{
    PutInOrder( nStartX, nEndX );
    PutInOrder( nStartY, nEndY );

    if ( nStartX <= nX2 && nEndX >= nX1 &&
         nStartY <= nY2 && nEndY >= nY1 )
    {
        long nMinX = nScrX;
        long nMinY = nScrY;
        long nMaxX = nScrX+nScrW-1;
        long nMaxY = nScrY+nScrH-1;
        BOOL bTop    = FALSE;
        BOOL bBottom = FALSE;
        BOOL bLeft   = FALSE;
        BOOL bRight  = FALSE;

        long nPosY = nScrY;
        for (SCSIZE nArrY=1; nArrY+1<nArrCount; nArrY++)
        {
            SCROW nY = pRowInfo[nArrY].nRowNo;

            if ( nY==nStartY )
            {
                nMinY = nPosY;
                bTop = TRUE;
            }

            if ( nY==nEndY )
            {
//              nMaxY = nPosY + pRowInfo[nArrY].nHeight - 2;
                nMaxY = nPosY + pRowInfo[nArrY].nHeight;
                bBottom = TRUE;
            }

            nPosY += pRowInfo[nArrY].nHeight;
        }

        RowInfo* pThisRowInfo = &pRowInfo[0];
        long nPosX = nScrX;
        for (SCCOL nX=nX1; nX<=nX2; nX++)
        {
            if ( nX==nStartX )
            {
                nMinX = nPosX;
                bLeft = TRUE;
            }
            if ( nX==nEndX )
            {
//              nMaxX = nPosX + pRowInfo[0].pCellInfo[nX+1].nWidth - 2;
                nMaxX = nPosX + pRowInfo[0].pCellInfo[nX+1].nWidth;
                bRight = TRUE;
            }
            nPosX += pRowInfo[0].pCellInfo[nX+1].nWidth;
        }

        pDev->SetLineColor( COL_BLACK );
        if (bTop && bBottom && bLeft && bRight)
        {
            pDev->SetFillColor();
            pDev->DrawRect( Rectangle( nMinX, nMinY, nMaxX, nMaxY ) );
        }
        else
        {
            if (bTop)
                pDev->DrawLine( Point( nMinX,nMinY ), Point( nMaxX,nMinY ) );
            if (bBottom)
                pDev->DrawLine( Point( nMinX,nMaxY ), Point( nMaxX,nMaxY ) );
            if (bLeft)
                pDev->DrawLine( Point( nMinX,nMinY ), Point( nMinX,nMaxY ) );
            if (bRight)
                pDev->DrawLine( Point( nMaxX,nMinY ), Point( nMaxX,nMaxY ) );
        }
    }
}

BOOL ScOutputData::SetChangedClip()
{
    PolyPolygon aPoly;

    Rectangle aDrawingRect;
    aDrawingRect.Left() = nScrX;
    aDrawingRect.Right() = nScrX+nScrW-1;

    BOOL    bHad    = FALSE;
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
                bHad = TRUE;
            }
            aDrawingRect.Bottom() = nPosY + pRowInfo[nArrY].nHeight - 1;
        }
        else if (bHad)
        {
            aPoly.Insert( Polygon( pDev->PixelToLogic(aDrawingRect) ) );
            bHad = FALSE;
        }
        nPosY += pRowInfo[nArrY].nHeight;
    }

    if (bHad)
        aPoly.Insert( Polygon( pDev->PixelToLogic(aDrawingRect) ) );

    BOOL bRet = (aPoly.Count() != 0);
    if (bRet)
        pDev->SetClipRegion(Region(aPoly));
    return bRet;
}

void ScOutputData::FindChanged()
{
    SCCOL   nX;
    SCSIZE  nArrY;

    BOOL bWasIdleDisabled = pDoc->IsIdleDisabled();
    pDoc->DisableIdle( TRUE );
    for (nArrY=0; nArrY<nArrCount; nArrY++)
        pRowInfo[nArrY].bChanged = FALSE;

    BOOL bProgress = FALSE;
    for (nArrY=0; nArrY<nArrCount; nArrY++)
    {
        RowInfo* pThisRowInfo = &pRowInfo[nArrY];
        for (nX=nX1; nX<=nX2; nX++)
        {
            ScBaseCell* pCell = pThisRowInfo->pCellInfo[nX+1].pCell;
            if (pCell)
                if (pCell->GetCellType() == CELLTYPE_FORMULA)
                {
                    ScFormulaCell* pFCell = (ScFormulaCell*)pCell;
                    if ( !bProgress && pFCell->GetDirty() )
                    {
                        ScProgress::CreateInterpretProgress( pDoc, TRUE );
                        bProgress = TRUE;
                    }
                    if (!pFCell->IsRunning())
                    {
                        double aVal = pFCell->GetValue();
                        if (pFCell->IsChanged())
                        {
                            pThisRowInfo->bChanged = TRUE;
                            if ( pThisRowInfo->pCellInfo[nX+1].bMerged )
                            {
                                SCSIZE nOverY = nArrY + 1;
                                while ( nOverY<nArrCount &&
                                        pRowInfo[nOverY].pCellInfo[nX+1].bVOverlapped )
                                {
                                    pRowInfo[nOverY].bChanged = TRUE;
                                    ++nOverY;
                                }
                            }
                        }
                    }
                }
        }
    }
    if ( bProgress )
        ScProgress::DeleteInterpretProgress();
    pDoc->DisableIdle( bWasIdleDisabled );
}

void ScOutputData::DrawMark( Window* pWin )
{
    Rectangle aRect;
    ScInvertMerger aInvert( pWin );
    //! additional method AddLineRect for ScInvertMerger?

    long nPosY = nScrY;
    for (SCSIZE nArrY=1; nArrY+1<nArrCount; nArrY++)
    {
        RowInfo* pThisRowInfo = &pRowInfo[nArrY];
        if ( pThisRowInfo->bChanged )
        {
            long nPosX = nScrX;
            if ( bLayoutRTL )
                nPosX += nMirrorW - 1;      // always in pixels

            aRect = Rectangle( Point( nPosX,nPosY ), Size(1, pThisRowInfo->nHeight) );
            if ( bLayoutRTL )
                aRect.Left() = aRect.Right() + 1;
            else
                aRect.Right() = aRect.Left() - 1;

            BOOL bOldMarked = FALSE;
            for (SCCOL nX=nX1; nX<=nX2; nX++)
            {
                if ( pThisRowInfo->pCellInfo[nX+1].bMarked != bOldMarked )
                {
                    if (bOldMarked && aRect.Right() >= aRect.Left())
                        aInvert.AddRect( aRect );

                    if ( bLayoutRTL )
                        aRect.Right() = nPosX;
                    else
                        aRect.Left() = nPosX;

                    bOldMarked = pThisRowInfo->pCellInfo[nX+1].bMarked;
                }

                if ( bLayoutRTL )
                {
                    nPosX -= pRowInfo[0].pCellInfo[nX+1].nWidth;
                    aRect.Left() = nPosX+1;
                }
                else
                {
                    nPosX += pRowInfo[0].pCellInfo[nX+1].nWidth;
                    aRect.Right() = nPosX-1;
                }
            }
            if (bOldMarked && aRect.Right() >= aRect.Left())
                aInvert.AddRect( aRect );
        }
        nPosY += pThisRowInfo->nHeight;
    }
}

void ScOutputData::DrawRefMark( SCCOL nRefStartX, SCROW nRefStartY,
                                SCCOL nRefEndX, SCROW nRefEndY,
                                const Color& rColor, BOOL bHandle )
{
    PutInOrder( nRefStartX, nRefEndX );
    PutInOrder( nRefStartY, nRefEndY );

    if ( nRefStartX == nRefEndX && nRefStartY == nRefEndY )
        pDoc->ExtendMerge( nRefStartX, nRefStartY, nRefEndX, nRefEndY, nTab );

    if ( nRefStartX <= nVisX2 && nRefEndX >= nVisX1 &&
         nRefStartY <= nVisY2 && nRefEndY >= nVisY1 )
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

        BOOL bTop    = FALSE;
        BOOL bBottom = FALSE;
        BOOL bLeft   = FALSE;
        BOOL bRight  = FALSE;

        long nPosY = nScrY;
        BOOL bNoStartY = ( nY1 < nRefStartY );
        BOOL bNoEndY   = FALSE;
        for (SCSIZE nArrY=1; nArrY<nArrCount; nArrY++)      // loop to end for bNoEndY check
        {
            SCROW nY = pRowInfo[nArrY].nRowNo;

            if ( nY==nRefStartY || (nY>nRefStartY && bNoStartY) )
            {
                nMinY = nPosY;
                bTop = TRUE;
            }
            if ( nY==nRefEndY )
            {
                nMaxY = nPosY + pRowInfo[nArrY].nHeight - 2;
                bBottom = TRUE;
            }
            if ( nY>nRefEndY && bNoEndY )
            {
                nMaxY = nPosY-2;
                bBottom = TRUE;
            }
            bNoStartY = ( nY < nRefStartY );
            bNoEndY   = ( nY < nRefEndY );
            nPosY += pRowInfo[nArrY].nHeight;
        }

        RowInfo* pThisRowInfo = &pRowInfo[0];
        long nPosX = nScrX;
        if ( bLayoutRTL )
            nPosX += nMirrorW - 1;      // always in pixels

        for (SCCOL nX=nX1; nX<=nX2; nX++)
        {
            if ( nX==nRefStartX )
            {
                nMinX = nPosX;
                bLeft = TRUE;
            }
            if ( nX==nRefEndX )
            {
                nMaxX = nPosX + ( pRowInfo[0].pCellInfo[nX+1].nWidth - 2 ) * nLayoutSign;
                bRight = TRUE;
            }
            nPosX += pRowInfo[0].pCellInfo[nX+1].nWidth * nLayoutSign;
        }

        if ( nMaxX * nLayoutSign >= nMinX * nLayoutSign &&
             nMaxY >= nMinY )
        {
            pDev->SetLineColor( rColor );
            if (bTop && bBottom && bLeft && bRight)
            {
                pDev->SetFillColor();
                pDev->DrawRect( Rectangle( nMinX, nMinY, nMaxX, nMaxY ) );
            }
            else
            {
                if (bTop)
                    pDev->DrawLine( Point( nMinX,nMinY ), Point( nMaxX,nMinY ) );
                if (bBottom)
                    pDev->DrawLine( Point( nMinX,nMaxY ), Point( nMaxX,nMaxY ) );
                if (bLeft)
                    pDev->DrawLine( Point( nMinX,nMinY ), Point( nMinX,nMaxY ) );
                if (bRight)
                    pDev->DrawLine( Point( nMaxX,nMinY ), Point( nMaxX,nMaxY ) );
            }
            if ( bHandle && bRight && bBottom )
            {
                pDev->SetLineColor();
                pDev->SetFillColor( rColor );
                pDev->DrawRect( Rectangle( nMaxX-3*nLayoutSign, nMaxY-3, nMaxX+nLayoutSign, nMaxY+1 ) );
            }
        }
    }
}

void ScOutputData::DrawOneChange( SCCOL nRefStartX, SCROW nRefStartY,
                                SCCOL nRefEndX, SCROW nRefEndY,
                                const Color& rColor, USHORT nType )
{
    PutInOrder( nRefStartX, nRefEndX );
    PutInOrder( nRefStartY, nRefEndY );

    if ( nRefStartX == nRefEndX && nRefStartY == nRefEndY )
        pDoc->ExtendMerge( nRefStartX, nRefStartY, nRefEndX, nRefEndY, nTab );

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

        BOOL bTop    = FALSE;
        BOOL bBottom = FALSE;
        BOOL bLeft   = FALSE;
        BOOL bRight  = FALSE;

        long nPosY = nScrY;
        BOOL bNoStartY = ( nY1 < nRefStartY );
        BOOL bNoEndY   = FALSE;
        for (SCSIZE nArrY=1; nArrY<nArrCount; nArrY++)      // loop to end for bNoEndY check
        {
            SCROW nY = pRowInfo[nArrY].nRowNo;

            if ( nY==nRefStartY || (nY>nRefStartY && bNoStartY) )
            {
                nMinY = nPosY - 1;
                bTop = TRUE;
            }
            if ( nY==nRefEndY )
            {
                nMaxY = nPosY + pRowInfo[nArrY].nHeight - 1;
                bBottom = TRUE;
            }
            if ( nY>nRefEndY && bNoEndY )
            {
                nMaxY = nPosY - 1;
                bBottom = TRUE;
            }
            bNoStartY = ( nY < nRefStartY );
            bNoEndY   = ( nY < nRefEndY );
            nPosY += pRowInfo[nArrY].nHeight;
        }

        RowInfo* pThisRowInfo = &pRowInfo[0];
        long nPosX = nScrX;
        if ( bLayoutRTL )
            nPosX += nMirrorW - 1;      // always in pixels

        for (SCCOL nX=nX1; nX<=nX2+1; nX++)
        {
            if ( nX==nRefStartX )
            {
                nMinX = nPosX - nLayoutSign;
                bLeft = TRUE;
            }
            if ( nX==nRefEndX )
            {
                nMaxX = nPosX + ( pRowInfo[0].pCellInfo[nX+1].nWidth - 1 ) * nLayoutSign;
                bRight = TRUE;
            }
            nPosX += pRowInfo[0].pCellInfo[nX+1].nWidth * nLayoutSign;
        }

        if ( nMaxX * nLayoutSign >= nMinX * nLayoutSign &&
             nMaxY >= nMinY )
        {
            if ( nType == SC_CAT_DELETE_ROWS )
                bLeft = bRight = bBottom = FALSE;       //! dicke Linie ???
            else if ( nType == SC_CAT_DELETE_COLS )
                bTop = bBottom = bRight = FALSE;        //! dicke Linie ???

            pDev->SetLineColor( rColor );
            if (bTop && bBottom && bLeft && bRight)
            {
                pDev->SetFillColor();
                pDev->DrawRect( Rectangle( nMinX, nMinY, nMaxX, nMaxY ) );
            }
            else
            {
                if (bTop)
                {
                    pDev->DrawLine( Point( nMinX,nMinY ), Point( nMaxX,nMinY ) );
                    if ( nType == SC_CAT_DELETE_ROWS )
                        pDev->DrawLine( Point( nMinX,nMinY+1 ), Point( nMaxX,nMinY+1 ) );
                }
                if (bBottom)
                    pDev->DrawLine( Point( nMinX,nMaxY ), Point( nMaxX,nMaxY ) );
                if (bLeft)
                {
                    pDev->DrawLine( Point( nMinX,nMinY ), Point( nMinX,nMaxY ) );
                    if ( nType == SC_CAT_DELETE_COLS )
                        pDev->DrawLine( Point( nMinX+nLayoutSign,nMinY ), Point( nMinX+nLayoutSign,nMaxY ) );
                }
                if (bRight)
                    pDev->DrawLine( Point( nMaxX,nMinY ), Point( nMaxX,nMaxY ) );
            }
            if ( bLeft && bTop )
            {
                pDev->SetLineColor();
                pDev->SetFillColor( rColor );
                pDev->DrawRect( Rectangle( nMinX+nLayoutSign, nMinY+1, nMinX+3*nLayoutSign, nMinY+3 ) );
            }
        }
    }
}

void ScOutputData::DrawChangeTrack()
{
    ScChangeTrack* pTrack = pDoc->GetChangeTrack();
    ScChangeViewSettings* pSettings = pDoc->GetChangeViewSettings();
    if ( !pTrack || !pTrack->GetFirst() || !pSettings || !pSettings->ShowChanges() )
        return;         // nix da oder abgeschaltet

    ScActionColorChanger aColorChanger(*pTrack);

    //  Clipping passiert von aussen
    //! ohne Clipping, nur betroffene Zeilen painten ??!??!?

    SCCOL nEndX = nX2;
    SCROW nEndY = nY2;
    if ( nEndX < MAXCOL ) ++nEndX;      // auch noch von der naechsten Zelle, weil die Markierung
    if ( nEndY < MAXROW ) ++nEndY;      // in die jeweils vorhergehende Zelle hineinragt
    ScRange aViewRange( nX1, nY1, nTab, nEndX, nEndY, nTab );
    const ScChangeAction* pAction = pTrack->GetFirst();
    while (pAction)
    {
        ScChangeActionType eType;
        if ( pAction->IsVisible() )
        {
            eType = pAction->GetType();
            const ScBigRange& rBig = pAction->GetBigRange();
            if ( rBig.aStart.Tab() == nTab )
            {
                ScRange aRange = rBig.MakeRange();

                if ( eType == SC_CAT_DELETE_ROWS )
                    aRange.aEnd.SetRow( aRange.aStart.Row() );
                else if ( eType == SC_CAT_DELETE_COLS )
                    aRange.aEnd.SetCol( aRange.aStart.Col() );

                if ( aRange.Intersects( aViewRange ) &&
                     ScViewUtil::IsActionShown( *pAction, *pSettings, *pDoc ) )
                {
                    aColorChanger.Update( *pAction );
                    Color aColor( aColorChanger.GetColor() );
                    DrawOneChange( aRange.aStart.Col(), aRange.aStart.Row(),
                                    aRange.aEnd.Col(), aRange.aEnd.Row(), aColor, eType );

                }
            }
            if ( eType == SC_CAT_MOVE &&
                    ((const ScChangeActionMove*)pAction)->
                        GetFromRange().aStart.Tab() == nTab )
            {
                ScRange aRange = ((const ScChangeActionMove*)pAction)->
                        GetFromRange().MakeRange();
                if ( aRange.Intersects( aViewRange ) &&
                     ScViewUtil::IsActionShown( *pAction, *pSettings, *pDoc ) )
                {
                    aColorChanger.Update( *pAction );
                    Color aColor( aColorChanger.GetColor() );
                    DrawOneChange( aRange.aStart.Col(), aRange.aStart.Row(),
                                    aRange.aEnd.Col(), aRange.aEnd.Row(), aColor, eType );
                }
            }
        }

        pAction = pAction->GetNext();
    }
}

void ScOutputData::DrawNoteMarks()
{
    BOOL bFirst = TRUE;

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
                ScBaseCell* pCell = pInfo->pCell;
                BOOL bIsMerged = FALSE;

                if ( nX==nX1 && pInfo->bHOverlapped && !pInfo->bVOverlapped )
                {
                    // find start of merged cell
                    bIsMerged = TRUE;
                    SCROW nY = pRowInfo[nArrY].nRowNo;
                    SCCOL nMergeX = nX;
                    SCROW nMergeY = nY;
                    pDoc->ExtendOverlapped( nMergeX, nMergeY, nX, nY, nTab );
                    pCell = pDoc->GetCell( ScAddress(nMergeX,nMergeY,nTab) );
                    // use origin's pCell for NotePtr test below
                }

                if ( pCell && pCell->GetNotePtr() && ( bIsMerged ||
                        ( !pInfo->bHOverlapped && !pInfo->bVOverlapped ) ) )
                {
                    if (bFirst)
                    {
                        pDev->SetLineColor();

                        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
                        if ( bUseStyleColor && rStyleSettings.GetHighContrastMode() )
                            pDev->SetFillColor( SC_MOD()->GetColorConfig().GetColorValue(svtools::FONTCOLOR).nColor );
                        else
                            pDev->SetFillColor(COL_LIGHTRED);

                        bFirst = FALSE;
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
                        pDev->DrawRect( Rectangle( nMarkX,nPosY,nMarkX+2*nLayoutSign,nPosY+2 ) );
                }

                nPosX += pRowInfo[0].pCellInfo[nX+1].nWidth * nLayoutSign;
            }
        }
        nPosY += pThisRowInfo->nHeight;
    }
}

long lcl_FindInList( const List& rPosList, const ScAddress &rPos )
{
    long nCount = rPosList.Count();
    for (long i=0; i<nCount; i++)
        if (*(ScAddress*)rPosList.GetObject(i) == rPos)
            return i+1;

    return 0;
}

void ScOutputData::PrintNoteMarks( const List& rPosList )
{
    Font aFont;
    ScAutoFontColorMode eColorMode = bUseStyleColor ?
                                        ( bForceAutoColor ? SC_AUTOCOL_IGNOREFONT : SC_AUTOCOL_DISPLAY ) :
                                        SC_AUTOCOL_PRINT;
    ((const ScPatternAttr&)pDoc->GetPool()->GetDefaultItem(ATTR_PATTERN)).GetFont(aFont, eColorMode);
    aFont.SetSize( Size( 0, (long) ( 120 * nPPTY ) ) );         // 6 pt
    pDev->SetFont( aFont );

    String aStr;

    long nPosY = nScrY;
    for (SCSIZE nArrY=1; nArrY+1<nArrCount; nArrY++)
    {
        RowInfo* pThisRowInfo = &pRowInfo[nArrY];
        if ( pThisRowInfo->bChanged )
        {
            long nPosX = nScrX;
            for (SCCOL nX=nX1; nX<=nX2; nX++)
            {
                CellInfo* pInfo = &pThisRowInfo->pCellInfo[nX+1];
                ScBaseCell* pCell = pInfo->pCell;
                if ( pCell && pCell->GetNotePtr() )     // auch verdeckte wegen der Numerierung
                {
                    aStr = String::CreateFromInt32( lcl_FindInList( rPosList,
                                ScAddress( nX, pThisRowInfo->nRowNo, nTab)));
                    long nMarkX = nPosX + pRowInfo[0].pCellInfo[nX+1].nWidth - 2 -
                                    pDev->GetTextWidth(aStr);
                    pDev->DrawText( Point( nMarkX,nPosY ), aStr );
                }

                nPosX += pRowInfo[0].pCellInfo[nX+1].nWidth;
            }
        }
        nPosY += pThisRowInfo->nHeight;
    }
}

void ScOutputData::ConnectObject( const SvInPlaceObjectRef& rRef, SdrOle2Obj* pOleObj )
{
    if (rRef.Is())
    {
        ULONG nMisc = rRef->GetMiscStatus();
        if ( nMisc & SVOBJ_MISCSTATUS_ACTIVATEWHENVISIBLE )
            pViewShell->ConnectObject( pOleObj );
    }
}

void ScOutputData::DrawClipMarks()
{
    if (!bAnyClipped)
        return;

    Color aArrowFillCol( COL_LIGHTRED );

    ULONG nOldDrawMode = pDev->GetDrawMode();
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    if ( bUseStyleColor && rStyleSettings.GetHighContrastMode() )
    {
        //  use DrawMode to change the arrow's outline color
        pDev->SetDrawMode( nOldDrawMode | DRAWMODE_SETTINGSLINE );
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

                        while ( nOverX > 0 && ( ((const ScMergeFlagAttr*)pDoc->GetAttr(
                                nOverX, nOverY, nTab, ATTR_MERGE_FLAG ))->GetValue() & SC_MF_HOR ) )
                        {
                            --nOverX;
                            nStartPosX -= nLayoutSign * (long) ( pDoc->GetColWidth(nOverX,nTab) * nPPTX );
                        }

                        while ( nOverY > 0 && ( ((const ScMergeFlagAttr*)pDoc->GetAttr(
                                nOverX, nOverY, nTab, ATTR_MERGE_FLAG ))->GetValue() & SC_MF_VER ) )
                        {
                            --nOverY;
                            nStartPosY -= nLayoutSign * (long) ( pDoc->GetRowHeight(nOverY,nTab) * nPPTY );
                        }

                        long nOutWidth = (long) ( pDoc->GetColWidth(nOverX,nTab) * nPPTX );
                        long nOutHeight = (long) ( pDoc->GetRowHeight(nOverY,nTab) * nPPTY );

                        const ScMergeAttr* pMerge = (const ScMergeAttr*)
                                    pDoc->GetAttr( nOverX, nOverY, nTab, ATTR_MERGE );
                        SCCOL nCountX = pMerge->GetColMerge();
                        for (SCCOL i=1; i<nCountX; i++)
                            nOutWidth += (long) ( pDoc->GetColWidth(nOverX+i,nTab) * nPPTX );
                        SCROW nCountY = pMerge->GetRowMerge();
                        for (SCROW j=1; j<nCountY; j++)
                            nOutHeight += (long) ( pDoc->GetRowHeight(nOverY+j,nTab) * nPPTY );

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
                                    (ScMergeAttr*)&pInfo->pPatternAttr->GetItem(ATTR_MERGE);
                            SCCOL nCountX = pMerge->GetColMerge();
                            for (SCCOL i=1; i<nCountX; i++)
                                nOutWidth += (long) ( pDoc->GetColWidth(nOverX+i,nTab) * nPPTX );
                            SCROW nCountY = pMerge->GetRowMerge();
                            for (SCROW j=1; j<nCountY; j++)
                                nOutHeight += (long) ( pDoc->GetRowHeight(nOverY+j,nTab) * nPPTY );
                        }

                        long nStartPosX = nPosX;
                        if ( bLayoutRTL )
                            nStartPosX -= nOutWidth - 1;
                        aCellRect = Rectangle( Point( nStartPosX, nPosY ), Size( nOutWidth, nOutHeight ) );
                    }

                    aCellRect.Bottom() -= 1;    // don't paint over the cell grid
                    if ( bLayoutRTL )
                        aCellRect.Left() += 1;
                    else
                        aCellRect.Right() -= 1;

                    long nMarkPixel = (long)( SC_CLIPMARK_SIZE * nPPTX );
                    Size aMarkSize( nMarkPixel, (nMarkPixel-1)*2 );

                    if ( pInfo->nClipMark & ( bLayoutRTL ? SC_CLIPMARK_RIGHT : SC_CLIPMARK_LEFT ) )
                    {
                        //  visually left
                        Rectangle aMarkRect = aCellRect;
                        aMarkRect.Right() = aCellRect.Left()+nMarkPixel-1;
#if 0
                        //! Test
                        pDev->SetLineColor(); pDev->SetFillColor(COL_YELLOW);
                        pDev->DrawRect(aMarkRect);
                        //! Test
#endif
                        SvxFont::DrawArrow( *pDev, aMarkRect, aMarkSize, aArrowFillCol, TRUE );
                    }
                    if ( pInfo->nClipMark & ( bLayoutRTL ? SC_CLIPMARK_LEFT : SC_CLIPMARK_RIGHT ) )
                    {
                        //  visually right
                        Rectangle aMarkRect = aCellRect;
                        aMarkRect.Left() = aCellRect.Right()-nMarkPixel+1;
#if 0
                        //! Test
                        pDev->SetLineColor(); pDev->SetFillColor(COL_LIGHTGREEN);
                        pDev->DrawRect(aMarkRect);
                        //! Test
#endif
                        SvxFont::DrawArrow( *pDev, aMarkRect, aMarkSize, aArrowFillCol, FALSE );
                    }
                }
                nPosX += pRowInfo[0].pCellInfo[nX+1].nWidth * nLayoutSign;
            }
        }
        nPosY += pThisRowInfo->nHeight;
    }

    pDev->SetDrawMode(nOldDrawMode);
}



