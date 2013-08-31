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

#ifndef SC_OUTPUT_HXX
#define SC_OUTPUT_HXX

#include "address.hxx"
#include "cellvalue.hxx"
#include <tools/color.hxx>
#include <tools/fract.hxx>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>

namespace sc {
    struct SpellCheckContext;
}

namespace editeng {
    struct MisspellRanges;
}

class Rectangle;
class Font;
class OutputDevice;
class EditEngine;
class ScDocument;
class ScPatternAttr;
class SdrObject;
struct RowInfo;
struct ScTableInfo;
class ScTabViewShell;
class ScPageBreakData;
class FmFormView;
class ScFieldEditEngine;
class SdrPaintWindow;

// ---------------------------------------------------------------------------

#define SC_SCENARIO_HSPACE      60
#define SC_SCENARIO_VSPACE      50

// ---------------------------------------------------------------------------

enum ScOutputType { OUTTYPE_WINDOW, OUTTYPE_PRINTER };

class ScFieldEditEngine;

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

    class DrawEditParam
    {
    public:
        SvxCellHorJustify       meHorJust;
        SvxCellVerJustify       meVerJust;
        SvxCellJustifyMethod    meHorJustMethod;
        SvxCellJustifyMethod    meVerJustMethod;
        SvxCellOrientation      meOrient;
        SCSIZE                  mnArrY;
        SCCOL                   mnX;
        SCROW                   mnY;
        SCCOL                   mnCellX;
        SCROW                   mnCellY;
        SCTAB                   mnTab;
        long                    mnPosX;
        long                    mnPosY;
        long                    mnInitPosX;
        bool                    mbBreak:1;
        bool                    mbCellIsValue:1;
        bool                    mbAsianVertical:1;
        bool                    mbPixelToLogic:1;
        bool                    mbHyphenatorSet:1;
        bool                    mbRTL:1;
        ScFieldEditEngine*      mpEngine;
        ScRefCellValue          maCell;
        const ScPatternAttr*    mpPattern;
        const SfxItemSet*       mpCondSet;
        const SfxItemSet*       mpPreviewFontSet;
        const ScPatternAttr*    mpOldPattern;
        const SfxItemSet*       mpOldCondSet;
        const SfxItemSet*       mpOldPreviewFontSet;
        const RowInfo*          mpThisRowInfo;
        const std::vector<editeng::MisspellRanges>* mpMisspellRanges;

        explicit DrawEditParam(const ScPatternAttr* pPattern, const SfxItemSet* pCondSet, bool bCellIsValue);

        bool readCellContent(ScDocument* pDoc, bool bShowNullValues, bool bShowFormulas, bool bSyntaxMode, bool bUseStyleColor, bool bForceAutoColor, bool& rWrapFields);
        void setPatternToEngine(bool bUseStyleColor);
        void calcMargins(long& rTop, long& rLeft, long& rBottom, long& rRight, double nPPTX, double nPPTY) const;
        void calcPaperSize(Size& rPaperSize, const Rectangle& rAlignRect, double nPPTX, double nPPTY) const;
        void getEngineSize(ScFieldEditEngine* pEngine, long& rWidth, long& rHeight) const;
        bool hasLineBreak() const;
        bool isHyperlinkCell() const;

        /**
         * When the text is vertically oriented, the text is either rotated 90
         * degrees to the right or 90 degrees to the left.   Note that this is
         * different from being vertically stacked.
         */
        bool isVerticallyOriented() const;

        /**
         * Calculate offset position for vertically oriented (either
         * top-bottom or bottom-top orientation) text.
         *
         * @param rLogicStart initial position in pixels.  When the call is
         *                    finished, this parameter will store the new
         *                    position.
         */
        void calcStartPosForVertical(Point& rLogicStart, long nCellWidth, long nEngineWidth, long nTopM, OutputDevice* pRefDevice);

        void setAlignmentToEngine();
        bool adjustHorAlignment(ScFieldEditEngine* pEngine);
        void adjustForRTL();
        void adjustForHyperlinkInPDF(Point aURLStart, OutputDevice* pDev);
    };

    OutputDevice* mpDev;        // Device
    OutputDevice* mpRefDevice;  // printer if used for preview
    OutputDevice* pFmtDevice;   // reference for text formatting
    ScTableInfo& mrTabInfo;
    RowInfo* pRowInfo;          // Info block
    SCSIZE nArrCount;           // occupied lines in info block
    ScDocument* mpDoc;          // Document
    SCTAB nTab;                 // sheet
    long nScrX;                 // Output Startpos. (Pixel)
    long nScrY;
    long nScrW;                 // Output size (Pixel)
    long nScrH;
    long nMirrorW;              // Visible output width for mirroring (default: nScrW)
    SCCOL nX1;                  // Start-/End coordinates
    SCROW nY1;                  //  ( incl. hidden )
    SCCOL nX2;
    SCROW nY2;
    SCCOL nVisX1;               // Start-/End coordinates
    SCROW nVisY1;               //  ( visable range )
    SCCOL nVisX2;
    SCROW nVisY2;
    ScOutputType eType;         // Screen/Printer ...
    double mnPPTX;              // Pixel per Twips
    double mnPPTY;
    Fraction aZoomX;
    Fraction aZoomY;

    SdrObject* pEditObj;        // Omit when painting

    ScTabViewShell* pViewShell; // for connect from visible plug-ins

    // #114135#
    FmFormView* pDrawView;      // SdrView to paint to

    sal_Bool bEditMode;             // InPlace edited cell - do not output
    SCCOL nEditCol;
    SCROW nEditRow;

    bool bMetaFile;             // Output to metafile (not pixels!)
    bool bSingleGrid;           // beim Gitter bChanged auswerten

    bool bPagebreakMode;        // Page break preview
    bool bSolidBackground;      // white instead of transparant

    bool mbUseStyleColor;
    bool mbForceAutoColor;

    sal_Bool mbSyntaxMode;          // Syntax highlighting
    Color* pValueColor;
    Color* pTextColor;
    Color* pFormulaColor;

    Color   aGridColor;

    bool    mbShowNullValues;
    bool    mbShowFormulas;
    bool    bShowSpellErrors;   // Show spelling errors in EditObjects
    bool    bMarkClipped;

    bool    bSnapPixel;

    bool    bAnyRotated;        // internal
    bool    bAnyClipped;        // internal
    bool    bTabProtected;
    sal_uInt8   nTabTextDirection;  // EEHorizontalTextDirection values
    bool    bLayoutRTL;

    // #i74769# use SdrPaintWindow direct, remember it during BeginDrawLayers/EndDrawLayers
    SdrPaintWindow*     mpTargetPaintWindow;
    const sc::SpellCheckContext* mpSpellCheckCxt;

                            // private methods

    sal_Bool            GetMergeOrigin( SCCOL nX, SCROW nY, SCSIZE nArrY,
                                    SCCOL& rOverX, SCROW& rOverY, sal_Bool bVisRowChanged );
    bool IsEmptyCellText( RowInfo* pThisRowInfo, SCCOL nX, SCROW nY );
    void GetVisibleCell( SCCOL nCol, SCROW nRow, SCTAB nTab, ScRefCellValue& rCell );

    bool IsAvailable( SCCOL nX, SCROW nY );

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

    void SetSyntaxColor( Font* pFont, const ScRefCellValue& rCell );
    void SetEditSyntaxColor( EditEngine& rEngine, ScRefCellValue& rCell );

    double          GetStretch();

    void            DrawRotatedFrame( const Color* pForceColor );       // pixel

    drawinglayer::processor2d::BaseProcessor2D*  CreateProcessor2D( );

    void DrawEditStandard(DrawEditParam& rParam);
    void DrawEditBottomTop(DrawEditParam& rParam);
    void DrawEditTopBottom(DrawEditParam& rParam);
    void DrawEditStacked(DrawEditParam& rParam);
    void DrawEditAsianVertical(DrawEditParam& rParam);

    ScFieldEditEngine* CreateOutputEditEngine();

    void ShowClipMarks( DrawEditParam& rParam, long nEngineHeight, const Size& aCellSize,
                        bool bMerged, OutputAreaParam& aAreaParam );

    bool Clip( DrawEditParam& rParam, const Size& aCellSize, OutputAreaParam& aAreaParam,
               long nEngineHeight, bool bWrapFields);

public:
                    ScOutputData( OutputDevice* pNewDev, ScOutputType eNewType,
                                    ScTableInfo& rTabInfo, ScDocument* pNewDoc,
                                    SCTAB nNewTab, long nNewScrX, long nNewScrY,
                                    SCCOL nNewX1, SCROW nNewY1, SCCOL nNewX2, SCROW nNewY2,
                                    double nPixelPerTwipsX, double nPixelPerTwipsY,
                                    const Fraction* pZoomX = NULL,
                                    const Fraction* pZoomY = NULL );

                    ~ScOutputData();

    void SetSpellCheckContext( const sc::SpellCheckContext* pCxt );
    void    SetContentDevice( OutputDevice* pContentDev );

    void    SetRefDevice( OutputDevice* pRDev ) { mpRefDevice = pFmtDevice = pRDev; }
    void    SetFmtDevice( OutputDevice* pRDev ) { pFmtDevice = pRDev; }
    void    SetEditObject( SdrObject* pObj )    { pEditObj = pObj; }
    void    SetViewShell( ScTabViewShell* pSh ) { pViewShell = pSh; }

    // #114135#
    void    SetDrawView( FmFormView* pNew )     { pDrawView = pNew; }

    void    SetSolidBackground( sal_Bool bSet )     { bSolidBackground = bSet; }
    void    SetUseStyleColor( sal_Bool bSet )       { mbUseStyleColor = bSet; }

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
    void    DrawStrings( sal_Bool bPixelToLogic = false );
    void    DrawDocumentBackground();
    void    DrawBackground();
    void    DrawShadow();
    void    DrawExtraShadow(sal_Bool bLeft, sal_Bool bTop, sal_Bool bRight, sal_Bool bBottom);
    void    DrawFrame();

                    // with logic MapMode set!
    void    DrawEdit(sal_Bool bPixelToLogic);

    void    FindRotated();
    void    DrawRotated(sal_Bool bPixelToLogic);        // logical

    void    DrawClear();

    // #i72502# printer only command set
    Point PrePrintDrawingLayer(long nLogStX, long nLogStY );
    void PostPrintDrawingLayer(const Point& rMMOffset); // #i74768# need offset for FormLayer
    void PrintDrawingLayer(const sal_uInt16 nLayer, const Point& rMMOffset);

    // only screen:
    void    DrawingSingle(const sal_uInt16 nLayer);
    void    DrawSelectiveObjects(const sal_uInt16 nLayer);

    sal_Bool    SetChangedClip();       // sal_False = not
    Region      GetChangedAreaRegion();

    void    FindChanged();
    void    SetPagebreakMode( ScPageBreakData* pPageData );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
