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

#ifndef INCLUDED_SC_SOURCE_UI_INC_OUTPUT_HXX
#define INCLUDED_SC_SOURCE_UI_INC_OUTPUT_HXX

#include <address.hxx>
#include <cellvalue.hxx>
#include <tools/color.hxx>
#include <tools/fract.hxx>
#include <tools/gen.hxx>
#include <editeng/svxenum.hxx>
#include <vcl/outdev.hxx>
#include <o3tl/deleter.hxx>

namespace sc {
    struct SpellCheckContext;
}

namespace editeng {
    struct MisspellRanges;
}
namespace drawinglayer { namespace processor2d { class BaseProcessor2D; } }

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

class ScOutputData
{
friend class ScDrawStringsVars;
friend class ScGridWindow;
private:
    struct OutputAreaParam
    {
        tools::Rectangle   maAlignRect;
        tools::Rectangle   maClipRect;
        long        mnColWidth;
        long        mnLeftClipLength; /// length of the string getting cut off on the left.
        long        mnRightClipLength; /// length of the string getting cut off on the right.
        bool        mbLeftClip;
        bool        mbRightClip;
    };

    class DrawEditParam
    {
    public:
        SvxCellHorJustify const       meHorJustAttr;      ///< alignment attribute
        SvxCellHorJustify       meHorJustContext;   ///< context depending on attribute, content and direction
        SvxCellHorJustify       meHorJustResult;    ///< result for EditEngine
        SvxCellVerJustify       meVerJust;
        SvxCellJustifyMethod const    meHorJustMethod;
        SvxCellJustifyMethod const    meVerJustMethod;
        SvxCellOrientation      meOrient;
        SCSIZE                  mnArrY;
        SCCOL                   mnX;
        SCCOL                   mnCellX;
        SCROW                   mnCellY;
        long                    mnPosX;
        long                    mnPosY;
        long                    mnInitPosX;
        bool const              mbBreak:1;
        bool const              mbCellIsValue:1;
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
        const RowInfo*          mpThisRowInfo;
        const std::vector<editeng::MisspellRanges>* mpMisspellRanges;

        explicit DrawEditParam(const ScPatternAttr* pPattern, const SfxItemSet* pCondSet, bool bCellIsValue);

        bool readCellContent(const ScDocument* pDoc, bool bShowNullValues, bool bShowFormulas, bool bSyntaxMode, bool bUseStyleColor, bool bForceAutoColor, bool& rWrapFields);
        void setPatternToEngine(bool bUseStyleColor);
        void calcMargins(long& rTop, long& rLeft, long& rBottom, long& rRight, double nPPTX, double nPPTY) const;
        void calcPaperSize(Size& rPaperSize, const tools::Rectangle& rAlignRect, double nPPTX, double nPPTY) const;
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
        void calcStartPosForVertical(Point& rLogicStart, long nCellWidth, long nEngineWidth, long nTopM, const OutputDevice* pRefDevice);

        void setAlignmentToEngine();
        bool adjustHorAlignment(ScFieldEditEngine* pEngine);
        void adjustForHyperlinkInPDF(Point aURLStart, const OutputDevice* pDev);
    };

    VclPtr<OutputDevice> mpDev;        // Device
    VclPtr<OutputDevice> mpRefDevice;  // printer if used for preview
    VclPtr<OutputDevice> pFmtDevice;   // reference for text formatting
    ScTableInfo& mrTabInfo;
    RowInfo* pRowInfo;          // Info block
    SCSIZE const nArrCount;           // occupied lines in info block
    ScDocument* mpDoc;          // Document
    SCTAB nTab;                 // sheet
    long const nScrX;                 // Output Startpos. (Pixel)
    long const nScrY;
    long nScrW;                 // Output size (Pixel)
    long nScrH;
    long nMirrorW;              // Visible output width for mirroring (default: nScrW)
    SCCOL const nX1;                  // Start-/End coordinates
    SCROW const nY1;                  //  ( incl. hidden )
    SCCOL const nX2;
    SCROW const nY2;
    SCCOL nVisX1;               // Start-/End coordinates
    SCROW nVisY1;               //  ( visible range )
    SCCOL nVisX2;
    SCROW nVisY2;
    ScOutputType const eType;         // Screen/Printer ...
    double const mnPPTX;              // Pixel per Twips
    double const mnPPTY;
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
    bool const mbForceAutoColor;

    bool mbSyntaxMode;          // Syntax highlighting
    std::unique_ptr<Color> pValueColor;
    std::unique_ptr<Color> pTextColor;
    std::unique_ptr<Color> pFormulaColor;

    Color   aGridColor;

    bool    mbShowNullValues;
    bool    mbShowFormulas;
    bool    bShowSpellErrors;   // Show spelling errors in EditObjects
    bool    bMarkClipped;

    bool    bSnapPixel;

    bool    bAnyClipped;        // internal
    bool    bTabProtected;
    bool    bLayoutRTL;

    // #i74769# use SdrPaintWindow direct, remember it during BeginDrawLayers/EndDrawLayers
    SdrPaintWindow*     mpTargetPaintWindow;
    const sc::SpellCheckContext* mpSpellCheckCxt;

                            // private methods

    bool            GetMergeOrigin( SCCOL nX, SCROW nY, SCSIZE nArrY,
                                    SCCOL& rOverX, SCROW& rOverY, bool bVisRowChanged );
    bool IsEmptyCellText( const RowInfo* pThisRowInfo, SCCOL nX, SCROW nY );
    void GetVisibleCell( SCCOL nCol, SCROW nRow, SCTAB nTab, ScRefCellValue& rCell );

    bool IsAvailable( SCCOL nX, SCROW nY );

    void            GetOutputArea( SCCOL nX, SCSIZE nArrY, long nPosX, long nPosY,
                                   SCCOL nCellX, SCROW nCellY, long nNeeded,
                                   const ScPatternAttr& rPattern,
                                   sal_uInt16 nHorJustify, bool bCellIsValue,
                                   bool bBreak, bool bOverwrite,
                                   OutputAreaParam& rParam );

    void            ShrinkEditEngine( EditEngine& rEngine, const tools::Rectangle& rAlignRect,
                                    long nLeftM, long nTopM, long nRightM, long nBottomM,
                                    bool bWidth, SvxCellOrientation nOrient, long nAttrRotate, bool bPixelToLogic,
                                    long& rEngineWidth, long& rEngineHeight, long& rNeededPixel,
                                    bool& rLeftClip, bool& rRightClip );

    void SetSyntaxColor( vcl::Font* pFont, const ScRefCellValue& rCell );
    void SetEditSyntaxColor( EditEngine& rEngine, const ScRefCellValue& rCell );

    double          GetStretch();

    void            DrawRotatedFrame(vcl::RenderContext& rRenderContext);       // pixel

    std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> CreateProcessor2D( );

    void DrawEditStandard(DrawEditParam& rParam);
    void DrawEditBottomTop(DrawEditParam& rParam);
    void DrawEditTopBottom(DrawEditParam& rParam);
    void DrawEditStacked(DrawEditParam& rParam);
    void DrawEditAsianVertical(DrawEditParam& rParam);

    ScFieldEditEngine* CreateOutputEditEngine();

    void ShowClipMarks( DrawEditParam& rParam, long nEngineHeight, const Size& aCellSize,
                        bool bMerged, OutputAreaParam& aAreaParam );

    ClearableClipRegionPtr Clip(DrawEditParam& rParam, const Size& aCellSize, OutputAreaParam& aAreaParam,
                                long nEngineHeight, bool bWrapFields);

    bool AdjustAreaParamClipRect(OutputAreaParam& rAreaParam);
    long SetEngineTextAndGetWidth( DrawEditParam& rParam, const OUString& rSetString,
                                   long& rNeededPixel, long nAddWidthPixels );

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
                                    SCTAB nNewTab, long nNewScrX, long nNewScrY,
                                    SCCOL nNewX1, SCROW nNewY1, SCCOL nNewX2, SCROW nNewY2,
                                    double nPixelPerTwipsX, double nPixelPerTwipsY,
                                    const Fraction* pZoomX = nullptr,
                                    const Fraction* pZoomY = nullptr );

                    ~ScOutputData();

    void    SetSpellCheckContext( const sc::SpellCheckContext* pCxt );
    void    SetContentDevice( OutputDevice* pContentDev );

    void    SetRefDevice( OutputDevice* pRDev ) { mpRefDevice = pFmtDevice = pRDev; }
    void    SetFmtDevice( OutputDevice* pRDev ) { pFmtDevice = pRDev; }
    void    SetViewShell( ScTabViewShell* pSh ) { pViewShell = pSh; }

    void    SetDrawView( FmFormView* pNew )     { pDrawView = pNew; }

    void    SetSolidBackground( bool bSet )     { bSolidBackground = bSet; }
    void    SetUseStyleColor( bool bSet )       { mbUseStyleColor = bSet; }

    void    SetEditCell( SCCOL nCol, SCROW nRow );
    void    SetSyntaxMode( bool bNewMode );
    void    SetMetaFileMode( bool bNewMode );
    void    SetGridColor( const Color& rColor );
    void    SetMarkClipped( bool bSet );
    void    SetShowNullValues ( bool bSet );
    void    SetShowFormulas   ( bool bSet );
    void    SetShowSpellErrors( bool bSet );
    void    SetMirrorWidth( long nNew );
    long    GetScrW() const     { return nScrW; }
    long    GetScrH() const     { return nScrH; }

    void    SetSnapPixel();

    void    DrawGrid(vcl::RenderContext& rRenderContext, bool bGrid, bool bPage);
    void    DrawStrings( bool bPixelToLogic = false );

    /// Draw all strings, or provide Rectangle where the text (defined by rAddress) would be drawn.
    tools::Rectangle LayoutStrings(bool bPixelToLogic, bool bPaint = true, const ScAddress &rAddress = ScAddress());

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
    Point   PrePrintDrawingLayer(long nLogStX, long nLogStY );
    void    PostPrintDrawingLayer(const Point& rMMOffset); // #i74768# need offset for FormLayer
    void    PrintDrawingLayer(SdrLayerID nLayer, const Point& rMMOffset);

    // only screen:
    void    DrawSelectiveObjects(SdrLayerID nLayer);

    bool    SetChangedClip();       // sal_False = not
    vcl::Region  GetChangedAreaRegion();

    void    FindChanged();
    void    SetPagebreakMode( ScPageBreakData* pPageData );
    void    DrawRefMark( SCCOL nRefStartX, SCROW nRefStartY,
                         SCCOL nRefEndX, SCROW nRefEndY,
                         const Color& rColor, bool bHandle );
    void    DrawOneChange( SCCOL nRefStartX, SCROW nRefStartY,
                            SCCOL nRefEndX, SCROW nRefEndY,
                            const Color& rColor, sal_uInt16 nType );
    void    DrawChangeTrack();
    void    DrawClipMarks();

    void    DrawNoteMarks(vcl::RenderContext& rRenderContext);
    void    AddPDFNotes();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
