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

#ifndef SC_OUTPUT_HXX
#define SC_OUTPUT_HXX

#include "address.hxx"
#include <tools/list.hxx>
#include <tools/color.hxx>
#include <tools/fract.hxx>
#include <com/sun/star/embed/XEmbeddedObject.hpp>

class Rectangle;
class Font;
class OutputDevice;
class Window;
class EditEngine;
class ScDocument;
class ScBaseCell;
class ScPatternAttr;
class SvxMarginItem;
class SdrObject;
class SdrOle2Obj;
struct RowInfo;
struct ScTableInfo;
class ScTabViewShell;
class ScPageBreakData;
class FmFormView;

// #i74769# SdrPaintWindow predefine
class SdrPaintWindow;

// ---------------------------------------------------------------------------

#define SC_SCENARIO_HSPACE      60
#define SC_SCENARIO_VSPACE      50

// ---------------------------------------------------------------------------

#define SC_OBJECTS_NONE         0
#define SC_OBJECTS_DRAWING      1
#define SC_OBJECTS_OLE          2
#define SC_OBJECTS_CHARTS       4
#define SC_OBJECTS_ALL          ( SC_OBJECTS_DRAWING | SC_OBJECTS_OLE | SC_OBJECTS_CHARTS )

enum ScOutputType { OUTTYPE_WINDOW, OUTTYPE_PRINTER };

class ScOutputData
{
friend class ScDrawStringsVars;
private:
    struct OutputAreaParam
    {
        Rectangle   maAlignRect;
        Rectangle   maClipRect;
        long        mnColWidth;
        bool        mbLeftClip;
        bool        mbRightClip;
    };

    OutputDevice* pDev;         // Device
    OutputDevice* pRefDevice;   // printer if used for preview
    OutputDevice* pFmtDevice;   // reference for text formatting
    ScTableInfo& mrTabInfo;
    RowInfo* pRowInfo;          // Info-Block
    SCSIZE nArrCount;           // belegte Zeilen im Info-Block
    ScDocument* pDoc;           // Dokument
    SCTAB nTab;                 // Tabelle
    long nScrX;                 // Ausgabe Startpos. (Pixel)
    long nScrY;
    long nScrW;                 // Ausgabe Groesse (Pixel)
    long nScrH;
    long nMirrorW;              // Visible output width for mirroring (default: nScrW)
    SCCOL nX1;                  // Start-/Endkoordinaten
    SCROW nY1;                  //  ( incl. versteckte )
    SCCOL nX2;
    SCROW nY2;
    SCCOL nVisX1;               // Start-/Endkoordinaten
    SCROW nVisY1;               //  ( sichtbarer Bereich )
    SCCOL nVisX2;
    SCROW nVisY2;
    ScOutputType eType;         // Bildschirm/Drucker ...
    double nPPTX;               // Pixel per Twips
    double nPPTY;
//  sal_uInt16 nZoom;               // Zoom-Faktor (Prozent) - fuer GetFont
    Fraction aZoomX;
    Fraction aZoomY;

    SdrObject* pEditObj;        // beim Painten auslassen

    ScTabViewShell* pViewShell; // zum Connecten von sichtbaren Plug-Ins

    // #114135#
    FmFormView* pDrawView;      // SdrView to paint to

    sal_Bool bEditMode;             // InPlace editierte Zelle - nicht ausgeben
    SCCOL nEditCol;
    SCROW nEditRow;

    sal_Bool bMetaFile;             // Ausgabe auf Metafile (nicht in Pixeln!)
    sal_Bool bSingleGrid;           // beim Gitter bChanged auswerten

    sal_Bool bPagebreakMode;        // Seitenumbruch-Vorschau
    sal_Bool bSolidBackground;      // weiss statt transparent

    sal_Bool bUseStyleColor;
    sal_Bool bForceAutoColor;

    sal_Bool bSyntaxMode;           // Syntax-Highlighting
    Color* pValueColor;
    Color* pTextColor;
    Color* pFormulaColor;

    Color   aGridColor;

    sal_Bool    bShowNullValues;
    sal_Bool    bShowFormulas;
    sal_Bool    bShowSpellErrors;   // Spell-Errors in EditObjekten anzeigen
    sal_Bool    bMarkClipped;

    sal_Bool    bSnapPixel;

    sal_Bool    bAnyRotated;        // intern
    sal_Bool    bAnyClipped;        // intern
    sal_Bool    bTabProtected;
    sal_uInt8   nTabTextDirection;  // EEHorizontalTextDirection values
    sal_Bool    bLayoutRTL;

    // #i74769# use SdrPaintWindow direct, remember it during BeginDrawLayers/EndDrawLayers
    SdrPaintWindow*     mpTargetPaintWindow;

                            // private methods

    sal_Bool            GetMergeOrigin( SCCOL nX, SCROW nY, SCSIZE nArrY,
                                    SCCOL& rOverX, SCROW& rOverY, sal_Bool bVisRowChanged );
    sal_Bool            IsEmptyCellText( RowInfo* pThisRowInfo, SCCOL nX, SCROW nY );
    void            GetVisibleCell( SCCOL nCol, SCROW nRow, SCTAB nTab, ScBaseCell*& rpCell );

    sal_Bool            IsAvailable( SCCOL nX, SCROW nY );

    void            GetOutputArea( SCCOL nX, SCSIZE nArrY, long nPosX, long nPosY,
                                   SCCOL nCellX, SCROW nCellY, long nNeeded,
                                   const ScPatternAttr& rPattern,
                                   sal_uInt16 nHorJustify, bool bCellIsValue,
                                   bool bBreak, bool bOverwrite,
                                   OutputAreaParam& rParam );

    void            ShrinkEditEngine( EditEngine& rEngine, const Rectangle& rAlignRect,
                                    long nLeftM, long nTopM, long nRightM, long nBottomM,
                                    sal_Bool bWidth, sal_uInt16 nOrient, long nAttrRotate, sal_Bool bPixelToLogic,
                                    long& rEngineWidth, long& rEngineHeight, long& rNeededPixel,
                                    bool& rLeftClip, bool& rRightClip );

    void            SetSyntaxColor( Font* pFont, ScBaseCell* pCell );
    void            SetEditSyntaxColor( EditEngine& rEngine, ScBaseCell* pCell );

    double          GetStretch();

    void            DrawRotatedFrame( const Color* pForceColor );       // pixel

public:
                    ScOutputData( OutputDevice* pNewDev, ScOutputType eNewType,
                                    ScTableInfo& rTabInfo, ScDocument* pNewDoc,
                                    SCTAB nNewTab, long nNewScrX, long nNewScrY,
                                    SCCOL nNewX1, SCROW nNewY1, SCCOL nNewX2, SCROW nNewY2,
                                    double nPixelPerTwipsX, double nPixelPerTwipsY,
                                    const Fraction* pZoomX = NULL,
                                    const Fraction* pZoomY = NULL );

                    ~ScOutputData();

    void    SetContentDevice( OutputDevice* pContentDev );

    void    SetRefDevice( OutputDevice* pRDev ) { pRefDevice = pFmtDevice = pRDev; }
    void    SetFmtDevice( OutputDevice* pRDev ) { pFmtDevice = pRDev; }
    void    SetEditObject( SdrObject* pObj )    { pEditObj = pObj; }
    void    SetViewShell( ScTabViewShell* pSh ) { pViewShell = pSh; }

    // #114135#
    void    SetDrawView( FmFormView* pNew )     { pDrawView = pNew; }

    void    SetSolidBackground( sal_Bool bSet )     { bSolidBackground = bSet; }
    void    SetUseStyleColor( sal_Bool bSet )       { bUseStyleColor = bSet; }

    void    SetEditCell( SCCOL nCol, SCROW nRow );
    void    SetSyntaxMode( sal_Bool bNewMode );
    void    SetMetaFileMode( sal_Bool bNewMode );
    void    SetSingleGrid( sal_Bool bNewMode );
    void    SetGridColor( const Color& rColor );
    void    SetMarkClipped( sal_Bool bSet );
    void    SetShowNullValues ( sal_Bool bSet = sal_True );
    void    SetShowFormulas   ( sal_Bool bSet = sal_True );
    void    SetShowSpellErrors( sal_Bool bSet = sal_True );
    void    SetMirrorWidth( long nNew );
    long    GetScrW() const     { return nScrW; }
    long    GetScrH() const     { return nScrH; }

    void    SetSnapPixel( sal_Bool bSet = sal_True );

    void    DrawGrid( sal_Bool bGrid, sal_Bool bPage );
    void    DrawStrings( sal_Bool bPixelToLogic = sal_False );
    void    DrawBackground();
    void    DrawShadow();
    void    DrawExtraShadow(sal_Bool bLeft, sal_Bool bTop, sal_Bool bRight, sal_Bool bBottom);
    void    DrawFrame();

                    // with logic MapMode set!
    void    DrawEdit(sal_Bool bPixelToLogic);

    void    FindRotated();
    void    DrawRotated(sal_Bool bPixelToLogic);        // logisch

    void    DrawClear();

    // #i72502# printer only command set
    Point PrePrintDrawingLayer(long nLogStX, long nLogStY );
    void PostPrintDrawingLayer(const Point& rMMOffset); // #i74768# need offset for FormLayer
    void PrintDrawingLayer(const sal_uInt16 nLayer, const Point& rMMOffset);

    // nur Bildschirm:
    void    DrawingSingle(const sal_uInt16 nLayer);
    void    DrawSelectiveObjects(const sal_uInt16 nLayer);

    sal_Bool    SetChangedClip();       // sal_False = nix
    PolyPolygon GetChangedArea();

    void    FindChanged();
    void    SetPagebreakMode( ScPageBreakData* pPageData );
#ifdef OLD_SELECTION_PAINT
    void    DrawMark( Window* pWin );
#endif
    void    DrawRefMark( SCCOL nRefStartX, SCROW nRefStartY,
                         SCCOL nRefEndX, SCROW nRefEndY,
                         const Color& rColor, sal_Bool bHandle );
    void    DrawOneChange( SCCOL nRefStartX, SCROW nRefStartY,
                            SCCOL nRefEndX, SCROW nRefEndY,
                            const Color& rColor, sal_uInt16 nType );
    void    DrawChangeTrack();
    void    DrawClipMarks();

    void    DrawNoteMarks();
    void    AddPDFNotes();
};



#endif

