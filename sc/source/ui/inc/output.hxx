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

#pragma once

#include <address.hxx>
#include <cellvalue.hxx>
#include <tools/color.hxx>
#include <tools/fract.hxx>
#include <tools/gen.hxx>
#include <editeng/svxenum.hxx>
#include <vcl/outdev.hxx>
#include <vcl/pdfwriter.hxx>
#include <tools/degree.hxx>
#include <o3tl/deleter.hxx>
#include <optional>

struct ScCellInfo;

namespace sc {
    class SpellCheckContext;
}

namespace editeng {
    struct MisspellRanges;
}
namespace drawinglayer::processor2d { class BaseProcessor2D; }

namespace vcl { class Font; }
class EditEngine;
class ScDocument;
class ScPatternAttr;
struct RowInfo;
struct ScTableInfo;
class ScTabViewShell;
class ScPageBreakData;
class FmFormView;
class ScFieldEditEngine;
class SdrPaintWindow;

#define SC_SCENARIO_HSPACE      60
#define SC_SCENARIO_VSPACE      50

enum ScOutputType { OUTTYPE_WINDOW, OUTTYPE_PRINTER };

class ClearableClipRegion;
typedef std::unique_ptr<ClearableClipRegion, o3tl::default_delete<ClearableClipRegion>> ClearableClipRegionPtr;

typedef std::map<SCROW, sal_Int32> TableRowIdMap;
typedef std::map<std::pair<SCROW, SCCOL>, sal_Int32> TableDataIdMap;
struct ScEnhancedPDFState
{
    sal_Int32 m_WorksheetId = -1;
    sal_Int32 m_TableId = -1;
    TableRowIdMap m_TableRowMap;
    TableDataIdMap m_TableDataMap;
    ScEnhancedPDFState(){};
};

/// Describes reference mark to be drawn, position & size in TWIPs
struct ReferenceMark {
    tools::Long nX;
    tools::Long nY;
    tools::Long nWidth;
    tools::Long nHeight;
    tools::Long nTab;
    Color aColor;

    ReferenceMark()
        : nX( 0 )
        , nY( 0 )
        , nWidth( 0 )
        , nHeight( 0 )
        , nTab( 0 )
        , aColor( COL_AUTO ) {}

    ReferenceMark( tools::Long aX,
                   tools::Long aY,
                   tools::Long aWidth,
                   tools::Long aHeight,
                   tools::Long aTab,
                   const Color& rColor )
        : nX( aX )
        , nY( aY )
        , nWidth( aWidth )
        , nHeight( aHeight )
        , nTab( aTab )
        , aColor( rColor ) {}

    bool Is() const { return ( nWidth > 0 && nHeight > 0 ); }
};

class ScOutputData
{
friend class ScDrawStringsVars;
friend class ScGridWindow;
private:
    struct OutputAreaParam
    {
        tools::Rectangle   maAlignRect;
        tools::Rectangle   maClipRect;
        tools::Long        mnColWidth;
        tools::Long        mnLeftClipLength; /// length of the string getting cut off on the left.
        tools::Long        mnRightClipLength; /// length of the string getting cut off on the right.
        bool        mbLeftClip;
        bool        mbRightClip;
    };

    class DrawEditParam
    {
    public:
        SvxCellHorJustify       meHorJustAttr;      ///< alignment attribute
        SvxCellHorJustify       meHorJustContext;   ///< context depending on attribute, content and direction
        SvxCellHorJustify       meHorJustResult;    ///< result for EditEngine
        SvxCellVerJustify       meVerJust;
        SvxCellJustifyMethod    meHorJustMethod;
        SvxCellJustifyMethod    meVerJustMethod;
        SvxCellOrientation      meOrient;
        SCSIZE                  mnArrY;
        SCCOL                   mnX;
        SCCOL                   mnCellX;
        SCROW                   mnCellY;
        tools::Long                    mnPosX;
        tools::Long                    mnPosY;
        tools::Long                    mnInitPosX;
        bool                    mbBreak:1;
        bool                    mbCellIsValue:1;
        bool                    mbAsianVertical:1;
        bool                    mbPixelToLogic:1;
        bool                    mbHyphenatorSet:1;
        ScFieldEditEngine*      mpEngine;
        ScRefCellValue          maCell;
        const ScPatternAttr*    mpPattern;
        const SfxItemSet*       mpCondSet;
        const SfxItemSet*       mpPreviewFontSet;
        const ScPatternAttr*    mpOldPattern;
        const SfxItemSet*       mpOldCondSet;
        const SfxItemSet*       mpOldPreviewFontSet;
        RowInfo*                mpThisRowInfo;
        const std::vector<editeng::MisspellRanges>* mpMisspellRanges;

        explicit DrawEditParam(const ScPatternAttr* pPattern, const SfxItemSet* pCondSet, bool bCellIsValue);

        bool readCellContent(const ScDocument* pDoc, bool bShowNullValues, bool bShowFormulas, bool bSyntaxMode, bool bUseStyleColor, bool bForceAutoColor, bool& rWrapFields);
        void setPatternToEngine(bool bUseStyleColor);
        void calcMargins(tools::Long& rTop, tools::Long& rLeft, tools::Long& rBottom, tools::Long& rRight, double nPPTX, double nPPTY) const;
        void calcPaperSize(Size& rPaperSize, const tools::Rectangle& rAlignRect, double nPPTX, double nPPTY) const;
        void getEngineSize(ScFieldEditEngine* pEngine, tools::Long& rWidth, tools::Long& rHeight) const;
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
        void calcStartPosForVertical(Point& rLogicStart, tools::Long nCellWidth, tools::Long nEngineWidth, tools::Long nTopM, const OutputDevice* pRefDevice);

        void setAlignmentToEngine();
        bool adjustHorAlignment(ScFieldEditEngine* pEngine);
        void adjustForHyperlinkInPDF(Point aURLStart, const OutputDevice* pDev);
    };

    VclPtr<OutputDevice> mpOriginalTargetDevice; // 'unpatched' TargetDevice
    VclPtr<OutputDevice> mpDev;        // Device
    VclPtr<OutputDevice> mpRefDevice;  // printer if used for preview
    VclPtr<OutputDevice> pFmtDevice;   // reference for text formatting
    ScTableInfo& mrTabInfo;
    RowInfo* pRowInfo;          // Info block
    SCSIZE nArrCount;           // occupied lines in info block
    ScDocument* mpDoc;          // Document
    SCTAB nTab;                 // sheet
    tools::Long nScrX;                 // Output Startpos. (Pixel)
    tools::Long nScrY;
    tools::Long nScrW;                 // Output size (Pixel)
    tools::Long nScrH;
    tools::Long nMirrorW;              // Visible output width for mirroring (default: nScrW)
    SCCOL nX1;                  // Start-/End coordinates
    SCROW nY1;                  //  ( incl. hidden )
    SCCOL nX2;
    SCROW nY2;
    SCCOL nVisX1;               // Start-/End coordinates
    SCROW nVisY1;               //  ( visible range )
    SCCOL nVisX2;
    SCROW nVisY2;
    ScOutputType eType;         // Screen/Printer ...
    double mnPPTX;              // Pixel per Twips
    double mnPPTY;
    Fraction aZoomX;
    Fraction aZoomY;

    ScTabViewShell* pViewShell; // for connect from visible plug-ins

    FmFormView* pDrawView;      // SdrView to paint to

    bool bEditMode;             // InPlace edited cell - do not output
    SCCOL nEditCol;
    SCROW nEditRow;

    bool bMetaFile;             // Output to metafile (not pixels!)

    bool bPagebreakMode;        // Page break preview
    bool bSolidBackground;      // white instead of transparent

    bool mbUseStyleColor;
    bool mbForceAutoColor;

    bool mbSyntaxMode;          // Syntax highlighting
    std::optional<Color> mxValueColor;
    std::optional<Color> mxTextColor;
    std::optional<Color> mxFormulaColor;

    Color   aGridColor;

    bool    mbShowNullValues;
    bool    mbShowFormulas;
    bool    bShowSpellErrors;   // Show spelling errors in EditObjects
    bool    bMarkClipped;

    bool    bSnapPixel;

    bool    bAnyClipped;        // internal
    bool    bVertical;
    bool    bTabProtected;
    bool    bLayoutRTL;

    // #i74769# use SdrPaintWindow direct, remember it during BeginDrawLayers/EndDrawLayers
    SdrPaintWindow*     mpTargetPaintWindow;
    const sc::SpellCheckContext* mpSpellCheckCxt;
    std::unique_ptr<ScFieldEditEngine> mxOutputEditEngine;

                            // private methods

    bool            GetMergeOrigin( SCCOL nX, SCROW nY, SCSIZE nArrY,
                                    SCCOL& rOverX, SCROW& rOverY, bool bVisRowChanged );
    bool IsEmptyCellText( const RowInfo* pThisRowInfo, SCCOL nX, SCROW nY );
    void GetVisibleCell( SCCOL nCol, SCROW nRow, SCTAB nTab, ScRefCellValue& rCell );

    bool IsAvailable( SCCOL nX, SCROW nY );

    void            GetOutputArea( SCCOL nX, SCSIZE nArrY, tools::Long nPosX, tools::Long nPosY,
                                   SCCOL nCellX, SCROW nCellY, tools::Long nNeeded,
                                   const ScPatternAttr& rPattern,
                                   sal_uInt16 nHorJustify, bool bCellIsValue,
                                   bool bBreak, bool bOverwrite,
                                   OutputAreaParam& rParam );

    void            ShrinkEditEngine( EditEngine& rEngine, const tools::Rectangle& rAlignRect,
                                    tools::Long nLeftM, tools::Long nTopM, tools::Long nRightM, tools::Long nBottomM,
                                    bool bWidth, SvxCellOrientation nOrient, Degree100 nAttrRotate, bool bPixelToLogic,
                                    tools::Long& rEngineWidth, tools::Long& rEngineHeight, tools::Long& rNeededPixel,
                                    bool& rLeftClip, bool& rRightClip );

    void SetSyntaxColor( vcl::Font* pFont, const ScRefCellValue& rCell );
    void SetEditSyntaxColor( EditEngine& rEngine, const ScRefCellValue& rCell );

    double          GetStretch() const;

    void            DrawRotatedFrame(vcl::RenderContext& rRenderContext);       // pixel

    std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> CreateProcessor2D( );

    void DrawEditStandard(DrawEditParam& rParam);
    void DrawEditBottomTop(DrawEditParam& rParam);
    void DrawEditTopBottom(DrawEditParam& rParam);
    void DrawEditStacked(DrawEditParam& rParam);
    void DrawEditAsianVertical(DrawEditParam& rParam);

    void InitOutputEditEngine();

    void SetClipMarks( OutputAreaParam &aAreaParam, ScCellInfo* pClipMarkCell,
                       SvxCellHorJustify eOutHorJust, tools::Long nLayoutSign );

    void ShowClipMarks( DrawEditParam& rParam, tools::Long nEngineWidth, const Size& aCellSize,
                        bool bMerged, OutputAreaParam& aAreaParam, bool bTop );

    ClearableClipRegionPtr Clip(DrawEditParam& rParam, const Size& aCellSize, OutputAreaParam& aAreaParam,
                                tools::Long nEngineWidth, bool bWrapFields, bool bTop);

    bool AdjustAreaParamClipRect(OutputAreaParam& rAreaParam);
    tools::Long SetEngineTextAndGetWidth( DrawEditParam& rParam, const OUString& rSetString,
                                   tools::Long& rNeededPixel, tools::Long nAddWidthPixels );

    // Check for and set cell rotations at OutputData to have it available
    // in the svx tooling to render the borders. Moved to private section
    // and the single call to end of constructor to be sure this always happens
    void    SetCellRotations();

public:

    /**
     * @param nNewScrX: X-Offset in the output device for the table
     * @param nNewScrY: Y-Offset in the output device for the table
     *
     */
                    ScOutputData( OutputDevice* pNewDev, ScOutputType eNewType,
                                    ScTableInfo& rTabInfo, ScDocument* pNewDoc,
                                    SCTAB nNewTab, tools::Long nNewScrX, tools::Long nNewScrY,
                                    SCCOL nNewX1, SCROW nNewY1, SCCOL nNewX2, SCROW nNewY2,
                                    double nPixelPerTwipsX, double nPixelPerTwipsY,
                                    const Fraction* pZoomX = nullptr,
                                    const Fraction* pZoomY = nullptr );

                    ~ScOutputData();

    void    SetSpellCheckContext( const sc::SpellCheckContext* pCxt );
    void    SetContentDevice( OutputDevice* pContentDev );

    void    SetRefDevice( OutputDevice* pRDev );
    void    SetFmtDevice( OutputDevice* pRDev );
    void    SetViewShell( ScTabViewShell* pSh ) { pViewShell = pSh; }

    void    SetDrawView( FmFormView* pNew )     { pDrawView = pNew; }

    void    SetSolidBackground( bool bSet )     { bSolidBackground = bSet; }
    void    SetUseStyleColor( bool bSet );

    void    SetEditCell( SCCOL nCol, SCROW nRow );
    void    SetSyntaxMode( bool bNewMode );
    void    SetMetaFileMode( bool bNewMode );
    void    SetGridColor( const Color& rColor );
    void    SetMarkClipped( bool bSet );
    void    SetShowNullValues ( bool bSet );
    void    SetShowFormulas   ( bool bSet );
    void    SetShowSpellErrors( bool bSet );
    void    SetMirrorWidth( tools::Long nNew );
    tools::Long    GetScrW() const     { return nScrW; }
    tools::Long    GetScrH() const     { return nScrH; }

    void    SetSnapPixel();

    bool    ReopenPDFStructureElement(vcl::PDFWriter::StructElement aType, SCROW nRow = -1,
                                      SCCOL nCol = -1);

    void    DrawGrid(vcl::RenderContext& rRenderContext, bool bGrid, bool bPage, bool bMergeCover = false);
    void    DrawStrings( bool bPixelToLogic = false );

    /// Draw all strings
    void LayoutStrings(bool bPixelToLogic);

    void    DrawDocumentBackground();
    void    DrawBackground(vcl::RenderContext& rRenderContext);
    void    DrawShadow();
    void    DrawExtraShadow(bool bLeft, bool bTop, bool bRight, bool bBottom);
    void    DrawFrame(vcl::RenderContext& rRenderContext);

                    // with logic MapMode set!
    void    DrawEdit(bool bPixelToLogic);
    void    DrawRotated(bool bPixelToLogic);        // logical

    void    DrawClear();

    // #i72502# printer only command set
    Point   PrePrintDrawingLayer(tools::Long nLogStX, tools::Long nLogStY );
    void    PostPrintDrawingLayer(const Point& rMMOffset); // #i74768# need offset for FormLayer
    void    PrintDrawingLayer(SdrLayerID nLayer, const Point& rMMOffset);

    // only screen:
    void    DrawSelectiveObjects(SdrLayerID nLayer);

    bool    SetChangedClip();       // sal_False = not
    vcl::Region  GetChangedAreaRegion();

    void    FindChanged();
    void    SetPagebreakMode( ScPageBreakData* pPageData );
    /// Draws reference mark and returns its properties
    void    DrawRefMark( SCCOL nRefStartX, SCROW nRefStartY,
                         SCCOL nRefEndX, SCROW nRefEndY,
                         const Color& rColor, bool bHandle );
    ReferenceMark FillReferenceMark( SCCOL nRefStartX, SCROW nRefStartY,
                                    SCCOL nRefEndX, SCROW nRefEndY,
                                    const Color& rColor );
    void    DrawOneChange( SCCOL nRefStartX, SCROW nRefStartY,
                            SCCOL nRefEndX, SCROW nRefEndY,
                            const Color& rColor, sal_uInt16 nType );
    void    DrawChangeTrack();
    void    DrawClipMarks();

    void    DrawNoteMarks(vcl::RenderContext& rRenderContext);
    void    DrawFormulaMarks(vcl::RenderContext& rRenderContext);
    void    AddPDFNotes();
    void    DrawSparklines(vcl::RenderContext& rRenderContext);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
