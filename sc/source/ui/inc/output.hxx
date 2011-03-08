/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
//  USHORT nZoom;               // Zoom-Faktor (Prozent) - fuer GetFont
    Fraction aZoomX;
    Fraction aZoomY;

    SdrObject* pEditObj;        // beim Painten auslassen

    ScTabViewShell* pViewShell; // zum Connecten von sichtbaren Plug-Ins

    // #114135#
    FmFormView* pDrawView;      // SdrView to paint to

    BOOL bEditMode;             // InPlace editierte Zelle - nicht ausgeben
    SCCOL nEditCol;
    SCROW nEditRow;

    BOOL bMetaFile;             // Ausgabe auf Metafile (nicht in Pixeln!)
    BOOL bSingleGrid;           // beim Gitter bChanged auswerten

    BOOL bPagebreakMode;        // Seitenumbruch-Vorschau
    BOOL bSolidBackground;      // weiss statt transparent

    BOOL bUseStyleColor;
    BOOL bForceAutoColor;

    BOOL bSyntaxMode;           // Syntax-Highlighting
    Color* pValueColor;
    Color* pTextColor;
    Color* pFormulaColor;

    Color   aGridColor;

    BOOL    bShowNullValues;
    BOOL    bShowFormulas;
    BOOL    bShowSpellErrors;   // Spell-Errors in EditObjekten anzeigen
    BOOL    bMarkClipped;

    BOOL    bSnapPixel;

    BOOL    bAnyRotated;        // intern
    BOOL    bAnyClipped;        // intern
    BOOL    bTabProtected;
    BYTE    nTabTextDirection;  // EEHorizontalTextDirection values
    BOOL    bLayoutRTL;

    // #i74769# use SdrPaintWindow direct, remember it during BeginDrawLayers/EndDrawLayers
    SdrPaintWindow*     mpTargetPaintWindow;

                            // private methods

    BOOL            GetMergeOrigin( SCCOL nX, SCROW nY, SCSIZE nArrY,
                                    SCCOL& rOverX, SCROW& rOverY, BOOL bVisRowChanged );
    BOOL            IsEmptyCellText( RowInfo* pThisRowInfo, SCCOL nX, SCROW nY );
    void            GetVisibleCell( SCCOL nCol, SCROW nRow, SCTAB nTab, ScBaseCell*& rpCell );

    BOOL            IsAvailable( SCCOL nX, SCROW nY );

    void            GetOutputArea( SCCOL nX, SCSIZE nArrY, long nPosX, long nPosY,
                                   SCCOL nCellX, SCROW nCellY, long nNeeded,
                                   const ScPatternAttr& rPattern,
                                   USHORT nHorJustify, bool bCellIsValue,
                                   bool bBreak, bool bOverwrite,
                                   OutputAreaParam& rParam );

    void            ShrinkEditEngine( EditEngine& rEngine, const Rectangle& rAlignRect,
                                    long nLeftM, long nTopM, long nRightM, long nBottomM,
                                    BOOL bWidth, USHORT nOrient, long nAttrRotate, BOOL bPixelToLogic,
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

    void    SetSolidBackground( BOOL bSet )     { bSolidBackground = bSet; }
    void    SetUseStyleColor( BOOL bSet )       { bUseStyleColor = bSet; }

    void    SetEditCell( SCCOL nCol, SCROW nRow );
    void    SetSyntaxMode( BOOL bNewMode );
    void    SetMetaFileMode( BOOL bNewMode );
    void    SetSingleGrid( BOOL bNewMode );
    void    SetGridColor( const Color& rColor );
    void    SetMarkClipped( BOOL bSet );
    void    SetShowNullValues ( BOOL bSet = TRUE );
    void    SetShowFormulas   ( BOOL bSet = TRUE );
    void    SetShowSpellErrors( BOOL bSet = TRUE );
    void    SetMirrorWidth( long nNew );
    long    GetScrW() const     { return nScrW; }
    long    GetScrH() const     { return nScrH; }

    void    SetSnapPixel( BOOL bSet = TRUE );

    void    DrawGrid( BOOL bGrid, BOOL bPage );
    void    DrawStrings( BOOL bPixelToLogic = FALSE );
    void    DrawDocumentBackground();
    void    DrawBackground();
    void    DrawShadow();
    void    DrawExtraShadow(BOOL bLeft, BOOL bTop, BOOL bRight, BOOL bBottom);
    void    DrawFrame();

                    // with logic MapMode set!
    void    DrawEdit(BOOL bPixelToLogic);

    void    FindRotated();
    void    DrawRotated(BOOL bPixelToLogic);        // logisch

    void    DrawClear();

    // #i72502# printer only command set
    Point PrePrintDrawingLayer(long nLogStX, long nLogStY );
    void PostPrintDrawingLayer(const Point& rMMOffset); // #i74768# need offset for FormLayer
    void PrintDrawingLayer(const sal_uInt16 nLayer, const Point& rMMOffset);

    // nur Bildschirm:
    void    DrawingSingle(const sal_uInt16 nLayer);
    void    DrawSelectiveObjects(const sal_uInt16 nLayer);

    BOOL    SetChangedClip();       // FALSE = nix
    PolyPolygon GetChangedArea();

    void    FindChanged();
    void    SetPagebreakMode( ScPageBreakData* pPageData );
    void    DrawRefMark( SCCOL nRefStartX, SCROW nRefStartY,
                         SCCOL nRefEndX, SCROW nRefEndY,
                         const Color& rColor, BOOL bHandle );
    void    DrawOneChange( SCCOL nRefStartX, SCROW nRefStartY,
                            SCCOL nRefEndX, SCROW nRefEndY,
                            const Color& rColor, USHORT nType );
    void    DrawChangeTrack();
    void    DrawClipMarks();

    void    DrawNoteMarks();
    void    AddPDFNotes();
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
