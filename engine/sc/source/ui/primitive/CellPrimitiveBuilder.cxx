/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <CellPrimitiveBuilder.hxx>

#include <CellContent.hxx>
#include <CellGeometry.hxx>
#include <CellMarkPrimitive2D.hxx>
#include <CellTextAlignment.hxx>
#include <CellTextFit.hxx>
#include <CellTextPrimitive2D.hxx>
#include <GridCoverPrimitive2D.hxx>
#include <SparklinePrimitive2D.hxx>
#include <SparklineRenderer.hxx>

#include <attrib.hxx>
#include <cellform.hxx>
#include <colorscale.hxx>
#include <conditio.hxx>
#include <document.hxx>
#include <editutil.hxx>
#include <fillinfo.hxx>
#include <formulacell.hxx>
#include <global.hxx>
#include <patattr.hxx>
#include <scitems.hxx>
#include <scopetools.hxx>
#include <Sparkline.hxx>
#include <SparklineGroup.hxx>

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/utils/bgradient.hxx>
#include <comphelper/string.hxx>
#include <drawinglayer/attribute/fillgradientattribute.hxx>
#include <drawinglayer/attribute/fontattribute.hxx>
#include <drawinglayer/attribute/lineattribute.hxx>
#include <drawinglayer/attribute/strokeattribute.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolygonStrokePrimitive2D.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/textdecoratedprimitive2d.hxx>
#include <drawinglayer/primitive2d/textenumsprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <editeng/StripPortionsHelper.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editstat.hxx>
#include <editeng/justifyitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/udlnitem.hxx>
#include <svx/algitem.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <o3tl/unit_conversion.hxx>
#include <svl/numformat.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <tools/degree.hxx>
#include <unotools/localedatawrapper.hxx>
#include <vcl/font.hxx>

#include <algorithm>
#include <cmath>
#include <map>
#include <set>

using namespace drawinglayer;

namespace
{
/// The width of the drop-down button of an autofilter cell in pixels.
constexpr double constDropDownPixels = 18.0;
/// Twips per 1/100 mm. The EditEngine of Calc lays out in 1/100 mm.
constexpr double constTwipsPerMm100 = o3tl::convert(1.0, o3tl::Length::mm100, o3tl::Length::twip);
constexpr double constMm100PerTwip = o3tl::convert(1.0, o3tl::Length::twip, o3tl::Length::mm100);
/// Twips per pixel at 100% zoom.
constexpr double constPixelTwips = o3tl::convert(1.0, o3tl::Length::px, o3tl::Length::twip);
/// Formula marks are light blue, error marks red, as the window draws them.
constexpr Color constFormulaMarkColor = COL_LIGHTBLUE;
constexpr Color constFormulaErrorMarkColor(0xE0, 0x4F, 0x3E);

basegfx::B2DPolygon rectangle(const basegfx::B2DRange& rRange)
{
    return basegfx::utils::createPolygonFromRect(rRange);
}

primitive2d::TextEmphasisMark mapEmphasisMark(FontEmphasisMark eMark)
{
    const FontEmphasisMark eStyle = eMark & FontEmphasisMark::Style;
    switch (eStyle)
    {
        case FontEmphasisMark::Dot:
            return primitive2d::TEXT_FONT_EMPHASIS_MARK_DOT;
        case FontEmphasisMark::Circle:
            return primitive2d::TEXT_FONT_EMPHASIS_MARK_CIRCLE;
        case FontEmphasisMark::Disc:
            return primitive2d::TEXT_FONT_EMPHASIS_MARK_DISC;
        case FontEmphasisMark::Accent:
            return primitive2d::TEXT_FONT_EMPHASIS_MARK_ACCENT;
        default:
            return primitive2d::TEXT_FONT_EMPHASIS_MARK_NONE;
    }
}

primitive2d::TextRelief mapRelief(FontRelief eRelief)
{
    switch (eRelief)
    {
        case FontRelief::Embossed:
            return primitive2d::TEXT_RELIEF_EMBOSSED;
        case FontRelief::Engraved:
            return primitive2d::TEXT_RELIEF_ENGRAVED;
        default:
            return primitive2d::TEXT_RELIEF_NONE;
    }
}

/// The horizontal room a text takes and the cells it may use, in twips.
struct OutputArea
{
    /// The cell or the merged area, which the text is aligned in.
    basegfx::B2DRange maAlignRange;
    /// The cells the text may paint into.
    basegfx::B2DRange maClipRange;
    /// The width of the cell or the merged area.
    double mfColumnWidth = 0.0;
    bool mbLeftClip = false;
    bool mbRightClip = false;
    double mfLeftClipLength = 0.0;
    double mfRightClipLength = 0.0;
    /// The first and the last column the text paints into.
    SCCOL mnLeftColumn = 0;
    SCCOL mnRightColumn = 0;

    /// Takes the room of the arrows that mark cut text off the clip range.
    void reserveClipMarks(bool bLeft, bool bRight)
    {
        maClipRange = basegfx::B2DRange(maClipRange.getMinX() + (bLeft ? SC_CLIPMARK_SIZE : 0.0),
                                        maClipRange.getMinY(),
                                        maClipRange.getMaxX() - (bRight ? SC_CLIPMARK_SIZE : 0.0),
                                        maClipRange.getMaxY());
    }
};

/// What the content of one cell is drawn from.
struct CellSource
{
    SCCOL mnColumn = 0;
    SCROW mnRow = 0;
    ScRefCellValue maCell;
    CellAttributeHolder maPattern;
    const SfxItemSet* mpCondSet = nullptr;
    const SfxItemSet* mpTableSet = nullptr;
    /// The number of merged columns and rows, 1 for a cell that is not merged.
    SCCOL mnMergeColumns = 1;
    SCROW mnMergeRows = 1;

    const ScPatternAttr& getPattern() const { return *maPattern.getScPatternAttr(); }
};
}

namespace sc
{
bool CellPrimitiveLayers::isEmpty() const
{
    return maBackground.empty() && maBorders.empty() && maText.empty() && maExtras.empty()
           && maMarks.empty() && maGridCover.empty();
}

class CellPrimitiveBuilder::Impl
{
public:
    Impl(ScDocument& rDocument, SCTAB nTab, const CellGeometry& rGeometry,
         CellPrimitiveSettings aSettings)
        : mrDocument(rDocument)
        , mnTab(nTab)
        , mrGeometry(rGeometry)
        , maSettings(std::move(aSettings))
    {
    }

    CellPrimitiveLayers build(SCCOL nStartColumn, SCROW nStartRow, SCCOL nEndColumn, SCROW nEndRow,
                              ScTableInfo* pInfo);
    CellPrimitiveRows buildRows(SCCOL nStartColumn, SCROW nStartRow, SCCOL nEndColumn,
                                SCROW nEndRow, SCROW nFirstContentRow, SCROW nLastContentRow);

private:
    void collect(SCCOL nStartColumn, SCROW nStartRow, SCCOL nEndColumn, SCROW nEndRow,
                 SCROW nFirstContentRow, SCROW nLastContentRow, ScTableInfo* pInfo = nullptr);
    /// Whether the row gets the layers other than the shadows and borders.
    bool isContentRow(SCROW nRow) const
    {
        return nRow >= mnFirstContentRow && nRow <= mnLastContentRow;
    }
    CellPrimitiveLayers& rowLayers(SCROW nRow) { return maRowLayers[nRow]; }
    static void computeInkRange(CellPrimitiveLayers& rLayers);

    void collectBackground(const ScTableInfo& rInfo);
    void placeBackground();
    void addDataBar(const ScDataBarInfo& rDataBar, const basegfx::B2DRange& rCell, SCROW nRow);
    void addIconSet(const ScIconSetInfo& rIconSet, const basegfx::B2DRange& rCell, SCROW nRow);
    void buildShadows(const ScTableInfo& rInfo);
    void buildBorders(ScTableInfo& rInfo, SCCOL nStartColumn, SCCOL nEndColumn);
    void buildText(const ScTableInfo& rInfo);
    void buildMarks(const ScTableInfo& rInfo);
    void buildSparklines(const ScTableInfo& rInfo);
    void buildGridCover(const ScTableInfo& rInfo);

    bool isCellTextHidden(SCCOL nColumn, SCROW nRow) const;
    bool isAvailable(SCCOL nColumn, SCROW nRow) const;
    bool readSource(SCCOL nColumn, SCROW nRow, CellSource& rSource) const;
    /// Whether the data bar or the icon set from the conditional formats of the cell hides its
    /// value. As in FillInfo, the first format that gives a data bar decides on the data bar, and
    /// the first that gives an icon set on the icon set.
    bool isValueHiddenByConditionalFormat(const CellSource& rSource) const;
    void addCellText(SCCOL nColumn, SCROW nRow, const ScCellInfo* pInfo);
    void computeOutputArea(const CellSource& rSource, double fNeeded, SvxCellHorJustify eHorJust,
                           bool bNoOverflow, bool bBreak, OutputArea& rArea) const;
    bool addSimpleText(const CellSource& rSource);
    void addEditText(const CellSource& rSource);
    void addClipMarks(const CellSource& rSource, const OutputArea& rArea, bool bLeft, bool bRight);
    void addOverflowCover(const CellSource& rSource, const OutputArea& rArea);

    ScFieldEditEngine& getEditEngine();
    primitive2d::TextLayouterDevice& getTextLayouter();

    ScDocument& mrDocument;
    SCTAB mnTab;
    const CellGeometry& mrGeometry;
    CellPrimitiveSettings maSettings;

    SCCOL mnStartColumn = 0;
    SCROW mnStartRow = 0;
    SCCOL mnEndColumn = 0;
    SCROW mnEndRow = 0;
    /// The rows that get the layers other than the shadows and borders, both included.
    SCROW mnFirstContentRow = 0;
    SCROW mnLastContentRow = 0;

    /// What the cells of the range show, by address, before the geometry places it.
    CellContent maContent;
    /// The shadows and borders, which are built for the whole range at once.
    CellPrimitiveLayers maRangeLayers;
    /// The other layers, for each row of the cells they belong to.
    std::map<SCROW, CellPrimitiveLayers> maRowLayers;
    /// The areas where grid lines are hidden, for each row of the cell that hides them.
    std::map<SCROW, std::vector<GridCoverPrimitive2D::Area>> maCoverAreas;
    /// Cells whose text was laid out already, so text reached from two sides is drawn once.
    std::set<std::pair<SCCOL, SCROW>> maTextDone;
    std::unique_ptr<ScFieldEditEngine> mpEditEngine;
    /// Lays out the text of the cells that need no EditEngine. It holds the font of the last cell.
    std::unique_ptr<primitive2d::TextLayouterDevice> mpTextLayouter;
};

CellPrimitiveLayers CellPrimitiveBuilder::Impl::build(SCCOL nStartColumn, SCROW nStartRow,
                                                      SCCOL nEndColumn, SCROW nEndRow,
                                                      ScTableInfo* pInfo)
{
    collect(nStartColumn, nStartRow, nEndColumn, nEndRow, nStartRow, nEndRow, pInfo);

    // The rows are joined in order, with one grid cover for the areas of all of them.
    CellPrimitiveLayers aLayers;
    aLayers.maBorders = std::move(maRangeLayers.maBorders);
    std::vector<GridCoverPrimitive2D::Area> aCoverAreas;
    for (auto& [nRow, rLayers] : maRowLayers)
    {
        aLayers.maBackground.append(std::move(rLayers.maBackground));
        aLayers.maText.append(std::move(rLayers.maText));
        aLayers.maExtras.append(std::move(rLayers.maExtras));
        aLayers.maMarks.append(std::move(rLayers.maMarks));
    }
    for (auto& [nRow, rAreas] : maCoverAreas)
        aCoverAreas.insert(aCoverAreas.end(), rAreas.begin(), rAreas.end());
    if (!aCoverAreas.empty())
        aLayers.maGridCover.push_back(new GridCoverPrimitive2D(std::move(aCoverAreas)));
    computeInkRange(aLayers);
    return aLayers;
}

CellPrimitiveRows CellPrimitiveBuilder::Impl::buildRows(SCCOL nStartColumn, SCROW nStartRow,
                                                        SCCOL nEndColumn, SCROW nEndRow,
                                                        SCROW nFirstContentRow,
                                                        SCROW nLastContentRow)
{
    collect(nStartColumn, nStartRow, nEndColumn, nEndRow, nFirstContentRow, nLastContentRow);

    CellPrimitiveRows aRows;
    aRows.maRange.maBorders = std::move(maRangeLayers.maBorders);
    computeInkRange(aRows.maRange);
    aRows.maRows.resize(nEndRow - nStartRow + 1);
    // Text that a cell outside the range reaches into it with is left to that cell's own range.
    for (auto& [nRow, rLayers] : maRowLayers)
    {
        if (nRow >= nStartRow && nRow <= nEndRow)
            aRows.maRows[nRow - nStartRow] = std::move(rLayers);
    }
    for (auto& [nRow, rAreas] : maCoverAreas)
    {
        if (nRow >= nStartRow && nRow <= nEndRow)
            aRows.maRows[nRow - nStartRow].maGridCover.push_back(
                new GridCoverPrimitive2D(std::move(rAreas)));
    }
    for (CellPrimitiveLayers& rLayers : aRows.maRows)
        computeInkRange(rLayers);
    return aRows;
}

void CellPrimitiveBuilder::Impl::computeInkRange(CellPrimitiveLayers& rLayers)
{
    const geometry::ViewInformation2D aViewInformation;
    for (const primitive2d::Primitive2DContainer* pLayer :
         { &rLayers.maBackground, &rLayers.maBorders, &rLayers.maText, &rLayers.maExtras,
           &rLayers.maMarks })
    {
        if (!pLayer->empty())
            rLayers.maInkRange.expand(pLayer->getB2DRange(aViewInformation));
    }
}

void CellPrimitiveBuilder::Impl::collect(SCCOL nStartColumn, SCROW nStartRow, SCCOL nEndColumn,
                                         SCROW nEndRow, SCROW nFirstContentRow,
                                         SCROW nLastContentRow, ScTableInfo* pInfo)
{
    mnStartColumn = nStartColumn;
    mnStartRow = nStartRow;
    mnEndColumn = nEndColumn;
    mnEndRow = nEndRow;
    mnFirstContentRow = nFirstContentRow;
    mnLastContentRow = nLastContentRow;
    maContent = CellContent();
    maRangeLayers = CellPrimitiveLayers();
    maRowLayers.clear();
    maCoverAreas.clear();
    maTextDone.clear();

    // Text needs the results. One calculation for the range is cheaper than one per cell.
    sc::IdleSwitch aIdleSwitch(mrDocument, false);
    mrDocument.InterpretCellsIfNeeded(
        ScRangeList(ScRange(nStartColumn, nStartRow, mnTab, nEndColumn, nEndRow, mnTab)));

    // The info of the caller serves, unless the sheet is right to left: its border array would
    // have to be mirrored, and the caller draws from it too.
    std::optional<ScTableInfo> oOwnInfo;
    if (!pInfo || mrDocument.IsLayoutRTL(mnTab))
    {
        // A scaled geometry fills the info in pixels, so border widths round like the cells.
        const double fScaleX = mrGeometry.isScaled() ? mrGeometry.getScaleX() : 1.0;
        const double fScaleY = mrGeometry.isScaled() ? mrGeometry.getScaleY() : 1.0;
        oOwnInfo.emplace(nStartRow, nEndRow, false);
        mrDocument.FillInfo(*oOwnInfo, nStartColumn, nStartRow, nEndColumn, nEndRow, mnTab,
                            fScaleX, fScaleY, false, maSettings.mbShowFormulas);

        // FillInfo mirrors the border array of a right-to-left sheet. Mirroring it again puts it
        // back in document order, so cells are built left to right on any sheet.
        if (mrDocument.IsLayoutRTL(mnTab))
            oOwnInfo->maArray.MirrorSelfX();
        pInfo = &*oOwnInfo;
    }
    ScTableInfo& aInfo = *pInfo;

    collectBackground(aInfo);
    placeBackground();
    buildShadows(aInfo);
    buildBorders(aInfo, nStartColumn, nEndColumn);
    buildText(aInfo);
    buildSparklines(aInfo);
    buildMarks(aInfo);
    buildGridCover(aInfo);
}

void CellPrimitiveBuilder::Impl::collectBackground(const ScTableInfo& rInfo)
{
    // Whether a column is hidden is looked up once for the range, not for every cell.
    std::vector<bool> aHidden(mnEndColumn - mnStartColumn + 1);
    for (SCCOL nColumn = mnStartColumn; nColumn <= mnEndColumn; ++nColumn)
        aHidden[nColumn - mnStartColumn] = mrDocument.ColHidden(nColumn, mnTab);

    const RowInfo* pRowInfo = rInfo.mpRowInfo.get();
    for (SCSIZE nArrY = 1; nArrY + 1 < rInfo.mnArrCount; ++nArrY)
    {
        const RowInfo& rRow = pRowInfo[nArrY];
        const SCROW nRow = rRow.nRowNo;
        if (rRow.bEmptyBack || !isContentRow(nRow))
            continue;

        // Neighbors with the same fill make one run. A hidden column neither ends a run nor
        // starts one.
        std::optional<Color> oRunColor;
        SCCOL nRunStart = 0;
        SCCOL nRunEnd = 0;
        auto flushRun = [&]()
        {
            if (oRunColor)
                maContent.maFills.push_back(CellFill{ nRow, nRunStart, nRunEnd, *oRunColor });
            oRunColor.reset();
        };

        for (SCCOL nColumn = mnStartColumn; nColumn <= mnEndColumn; ++nColumn)
        {
            if (aHidden[nColumn - mnStartColumn])
                continue;
            const ScCellInfo& rCell = rRow.cellInfo(nColumn);

            std::optional<Color> oColor;
            if (rCell.mxColorScale)
                oColor = *rCell.mxColorScale;
            else if (const auto* pBrush
                     = static_cast<const SvxBrushItem*>(rCell.maBackground.getItem()))
            {
                if (!pBrush->GetColor().IsTransparent())
                    oColor = pBrush->GetColor();
            }
            if (oColor && oColor->IsTransparent())
                oColor.reset();

            if (oColor != oRunColor)
            {
                flushRun();
                oRunColor = oColor;
                nRunStart = nColumn;
            }
            nRunEnd = nColumn;

            if (rCell.pDataBar)
                maContent.maDataBars.push_back(CellDataBar{ nColumn, nRow, *rCell.pDataBar });
            if (rCell.pIconSet)
                maContent.maIconSets.push_back(CellIconSet{ nColumn, nRow, *rCell.pIconSet });
        }
        flushRun();
    }
}

void CellPrimitiveBuilder::Impl::placeBackground()
{
    const std::vector<CellFill>& rFills = maContent.maFills;
    const std::vector<CellDataBar>& rBars = maContent.maDataBars;
    const std::vector<CellIconSet>& rIcons = maContent.maIconSets;
    size_t nFill = 0;
    size_t nBar = 0;
    size_t nIcon = 0;
    while (nFill < rFills.size() || nBar < rBars.size() || nIcon < rIcons.size())
    {
        SCROW nRow = mrDocument.MaxRow();
        if (nFill < rFills.size())
            nRow = std::min(nRow, rFills[nFill].mnRow);
        if (nBar < rBars.size())
            nRow = std::min(nRow, rBars[nBar].mnRow);
        if (nIcon < rIcons.size())
            nRow = std::min(nRow, rIcons[nIcon].mnRow);
        const double fTop = mrGeometry.getRowStart(nRow);
        const double fBottom = fTop + mrGeometry.getRowHeight(nRow);
        auto cellRange = [&](SCCOL nColumn)
        {
            const double fLeft = mrGeometry.getColumnStart(nColumn);
            return basegfx::B2DRange(fLeft, fTop, fLeft + mrGeometry.getColumnWidth(nColumn),
                                     fBottom);
        };

        // The data bars and icons of the row, cell by cell, the bar of a cell below its icon.
        for (;;)
        {
            const bool bBar = nBar < rBars.size() && rBars[nBar].mnRow == nRow;
            const bool bIcon = nIcon < rIcons.size() && rIcons[nIcon].mnRow == nRow;
            if (bBar && (!bIcon || rBars[nBar].mnColumn <= rIcons[nIcon].mnColumn))
            {
                addDataBar(rBars[nBar].maInfo, cellRange(rBars[nBar].mnColumn), nRow);
                ++nBar;
            }
            else if (bIcon)
            {
                addIconSet(rIcons[nIcon].maInfo, cellRange(rIcons[nIcon].mnColumn), nRow);
                ++nIcon;
            }
            else
                break;
        }

        // A row gets one fill primitive per color, made of all the rectangles of that color.
        std::map<Color, basegfx::B2DPolyPolygon> aFills;
        for (; nFill < rFills.size() && rFills[nFill].mnRow == nRow; ++nFill)
        {
            const CellFill& rFill = rFills[nFill];
            const double fLeft = mrGeometry.getColumnStart(rFill.mnStartColumn);
            const double fRight = mrGeometry.getColumnStart(rFill.mnEndColumn)
                                  + mrGeometry.getColumnWidth(rFill.mnEndColumn);
            if (fRight > fLeft)
                aFills[rFill.maColor].append(
                    rectangle(basegfx::B2DRange(fLeft, fTop, fRight, fBottom)));
        }

        // The fills go below the data bars and icons.
        if (aFills.empty())
            continue;
        CellPrimitiveLayers& rLayers = rowLayers(nRow);
        primitive2d::Primitive2DContainer aBackground;
        for (auto& [rColor, rPolyPolygon] : aFills)
            aBackground.push_back(new primitive2d::PolyPolygonColorPrimitive2D(
                std::move(rPolyPolygon), rColor.getBColor()));
        aBackground.append(std::move(rLayers.maBackground));
        rLayers.maBackground = std::move(aBackground);
    }
}

void CellPrimitiveBuilder::Impl::addDataBar(const ScDataBarInfo& rDataBar,
                                            const basegfx::B2DRange& rCell, SCROW nRow)
{
    const double fInset = 2 * maSettings.mfPixelSize;
    basegfx::B2DRange aPaint(rCell.getMinX() + fInset, rCell.getMinY() + fInset,
                             rCell.getMaxX() - fInset, rCell.getMaxY() - fInset);
    if (aPaint.isEmpty())
        return;

    // The bar starts at the zero point of the axis and grows to the right for a positive value and
    // to the left for a negative one, by its length in percent of the room on that side.
    double fZero = aPaint.getMinX();
    if (rDataBar.mnZero)
        fZero = aPaint.getMinX() + aPaint.getWidth() * rDataBar.mnZero / 100.0;

    double fBarStart;
    double fBarEnd;
    if (rDataBar.mnLength < 0)
    {
        fBarEnd = fZero;
        fBarStart = fZero + (fZero - aPaint.getMinX()) * rDataBar.mnLength / 100.0;
    }
    else if (rDataBar.mnLength > 0)
    {
        fBarStart = fZero;
        fBarEnd = fZero + (aPaint.getMaxX() - fZero) * rDataBar.mnLength / 100.0;
    }
    else
        return;

    const basegfx::B2DRange aBar(fBarStart, aPaint.getMinY(), fBarEnd, aPaint.getMaxY());
    if (rDataBar.mbGradient)
    {
        // The bar fades from its color at the axis to white at its end.
        const double fAngle = rDataBar.mnLength < 0 ? M_PI * 1.5 : M_PI * 0.5;
        const attribute::FillGradientAttribute aGradient(
            css::awt::GradientStyle_LINEAR, 0.0, 0.0, 0.0, fAngle,
            basegfx::BColorStops(rDataBar.maColor.getBColor(), COL_WHITE.getBColor()));
        rowLayers(nRow).maBackground.push_back(
            new primitive2d::FillGradientPrimitive2D(aBar, aGradient));
    }
    else
    {
        rowLayers(nRow).maBackground.push_back(new primitive2d::PolyPolygonColorPrimitive2D(
            basegfx::B2DPolyPolygon(rectangle(aBar)), rDataBar.maColor.getBColor()));
    }

    // An axis inside the cell is drawn as a dashed line at the zero point.
    if (!(rDataBar.mnZero && rDataBar.mnZero != 100))
        return;

    basegfx::B2DPolygon aAxis;
    aAxis.append(basegfx::B2DPoint(fZero, rCell.getMinY()));
    aAxis.append(basegfx::B2DPoint(fZero, rCell.getMaxY()));
    const double fDash = 3 * maSettings.mfPixelSize;
    rowLayers(nRow).maBackground.push_back(new primitive2d::PolygonStrokePrimitive2D(
        std::move(aAxis), attribute::LineAttribute(rDataBar.maAxisColor.getBColor()),
        attribute::StrokeAttribute(std::vector<double>{ fDash, fDash })));
}

void CellPrimitiveBuilder::Impl::addIconSet(const ScIconSetInfo& rIconSet,
                                            const basegfx::B2DRange& rCell, SCROW nRow)
{
    const Bitmap& rIcon = ScIconSetFormat::getBitmap(mrDocument.GetIconSetBitmapMap(),
                                                     rIconSet.eIconSetType, rIconSet.nIconIndex);
    const Size aPixelSize = rIcon.GetSizePixel();
    if (aPixelSize.IsEmpty())
        return;

    // The icon is as high as the font of the cell, 10pt when nothing says otherwise.
    double fHeight = rIconSet.mnHeight ? double(rIconSet.mnHeight)
                                       : o3tl::convert(10.0, o3tl::Length::pt, o3tl::Length::twip);
    const double fWidth = fHeight * aPixelSize.Width() / aPixelSize.Height();
    const double fInset = 2 * maSettings.mfPixelSize;
    const double fLeft = rCell.getMinX() + fInset;
    const double fTop = rCell.getMaxY() - fInset - fHeight;

    primitive2d::Primitive2DContainer aIcon{ new primitive2d::BitmapPrimitive2D(
        rIcon, basegfx::utils::createScaleTranslateB2DHomMatrix(fWidth, fHeight, fLeft, fTop)) };
    rowLayers(nRow).maBackground.push_back(new primitive2d::MaskPrimitive2D(
        basegfx::B2DPolyPolygon(rectangle(rCell)), std::move(aIcon)));
}

void CellPrimitiveBuilder::Impl::buildShadows(const ScTableInfo& rInfo)
{
    const RowInfo* pRowInfo = rInfo.mpRowInfo.get();
    for (SCSIZE nArrY = 1; nArrY + 1 < rInfo.mnArrCount; ++nArrY)
    {
        const RowInfo& rRow = pRowInfo[nArrY];
        const SCROW nRow = rRow.nRowNo;
        const double fTop = mrGeometry.getRowStart(nRow);
        const double fRowHeight = mrGeometry.getRowHeight(nRow);

        for (SCCOL nColumn = mnStartColumn; nColumn <= mnEndColumn; ++nColumn)
        {
            const double fWidth = mrGeometry.getColumnWidth(nColumn);
            if (fWidth <= 0.0)
                continue;
            const ScCellInfo& rCell = rRow.cellInfo(nColumn);

            for (int nPass = 0; nPass < 2; ++nPass) // horizontal, then vertical
            {
                const SvxShadowItem* pShadow = nPass ? rCell.pVShadowOrigin : rCell.pHShadowOrigin;
                if (!pShadow)
                    continue;
                const ScShadowPart ePart = nPass ? rCell.eVShadowPart : rCell.eHShadowPart;

                const double fLeft = mrGeometry.getColumnStart(nColumn);
                double fShadowLeft = fLeft;
                double fShadowRight = fLeft + fWidth;
                double fShadowTop = fTop;
                double fShadowBottom = fTop + fRowHeight;

                const double fSize = pShadow->GetWidth();
                const double fSizeX = std::min(fSize, fWidth);
                const double fSizeY = std::min(fSize, fRowHeight);
                const SvxShadowLocation eLocation = pShadow->GetLocation();
                const bool bTop = eLocation == SvxShadowLocation::TopLeft
                                  || eLocation == SvxShadowLocation::TopRight;
                const bool bLeftSide = eLocation == SvxShadowLocation::TopLeft
                                       || eLocation == SvxShadowLocation::BottomLeft;

                if (ePart == SC_SHADOW_HORIZ || ePart == SC_SHADOW_HSTART
                    || ePart == SC_SHADOW_CORNER)
                {
                    if (bTop)
                        fShadowTop = fShadowBottom - fSizeY;
                    else
                        fShadowBottom = fShadowTop + fSizeY;
                }
                if (ePart == SC_SHADOW_VERT || ePart == SC_SHADOW_VSTART
                    || ePart == SC_SHADOW_CORNER)
                {
                    if (bLeftSide)
                        fShadowLeft = fShadowRight - fSizeX;
                    else
                        fShadowRight = fShadowLeft + fSizeX;
                }
                if (ePart == SC_SHADOW_HSTART)
                {
                    if (bLeftSide)
                        fShadowRight -= fSizeX;
                    else
                        fShadowLeft += fSizeX;
                }
                if (ePart == SC_SHADOW_VSTART)
                {
                    if (bTop)
                        fShadowBottom -= fSizeY;
                    else
                        fShadowTop += fSizeY;
                }

                const basegfx::B2DRange aShadow(fShadowLeft, fShadowTop, fShadowRight,
                                                fShadowBottom);
                if (!aShadow.isEmpty())
                    maRangeLayers.maBorders.push_back(new primitive2d::PolyPolygonColorPrimitive2D(
                        basegfx::B2DPolyPolygon(rectangle(aShadow)),
                        pShadow->GetColor().getBColor()));
            }
        }
    }
}

void CellPrimitiveBuilder::Impl::buildBorders(ScTableInfo& rInfo, SCCOL nStartColumn,
                                              SCCOL nEndColumn)
{
    svx::frame::Array& rArray = rInfo.maArray;
    const size_t nColumnCount = rArray.GetColCount();
    const size_t nRowCount = rArray.GetRowCount();
    if (nColumnCount < 3 || nRowCount < 3)
        return;

    // The border array works in whole units, which are pixels for a scaled geometry and twips
    // otherwise. The primitives are scaled back to twips afterwards. The sizes are set before
    // each use and not kept: the array can be the one of the caller, which ScOutputData draws
    // from too, and ScOutputData::DrawFrame sets its own sizes before it draws.
    const double fScaleX = mrGeometry.isScaled() ? mrGeometry.getScaleX() : 1.0;
    const double fScaleY = mrGeometry.isScaled() ? mrGeometry.getScaleY() : 1.0;
    const RowInfo* pRowInfo = rInfo.mpRowInfo.get();

    auto rowHeight = [&](SCSIZE nArrY)
    {
        const SCROW nRow = pRowInfo[nArrY].nRowNo;
        if (mrDocument.ValidRow(nRow))
            return mrGeometry.getRowHeight(nRow);
        // The row above row 0 is a dummy that is never seen.
        return double(pRowInfo[nArrY].nHeight) / fScaleY;
    };

    const double fTop = mrGeometry.getRowStart(pRowInfo[1].nRowNo) - rowHeight(0);
    double fPosition = std::round(fTop * fScaleY);
    rArray.SetYOffset(sal_Int32(fPosition));
    double fEdge = fTop;
    for (size_t nRow = 0; nRow < nRowCount; ++nRow)
    {
        fEdge += rowHeight(nRow);
        const double fNewPosition = std::round(fEdge * fScaleY);
        rArray.SetRowHeight(nRow, sal_Int32(fNewPosition - fPosition));
        fPosition = fNewPosition;
    }

    const double fLeft
        = mrGeometry.getColumnStart(nStartColumn) - mrGeometry.getColumnWidth(nStartColumn - 1);
    fPosition = std::round(fLeft * fScaleX);
    rArray.SetXOffset(sal_Int32(fPosition));
    fEdge = fLeft;
    for (SCCOL nColumn = nStartColumn - 1; nColumn <= nEndColumn + 1; ++nColumn)
    {
        fEdge += mrGeometry.getColumnWidth(nColumn);
        const double fNewPosition = std::round(fEdge * fScaleX);
        rArray.SetColWidth(nColumn - (nStartColumn - 1), sal_Int32(fNewPosition - fPosition));
        fPosition = fNewPosition;
    }

    primitive2d::Primitive2DContainer aBorders
        = rArray.CreateB2DPrimitiveRange(1, 1, nColumnCount - 2, nRowCount - 2, nullptr);
    if (aBorders.empty())
        return;

    if (mrGeometry.isScaled())
    {
        maRangeLayers.maBorders.push_back(new primitive2d::TransformPrimitive2D(
            basegfx::utils::createScaleB2DHomMatrix(1.0 / fScaleX, 1.0 / fScaleY),
            std::move(aBorders)));
    }
    else
        maRangeLayers.maBorders.append(std::move(aBorders));
}

bool CellPrimitiveBuilder::Impl::isCellTextHidden(SCCOL nColumn, SCROW nRow) const
{
    return isCellContentHidden(mrDocument, ScAddress(nColumn, nRow, mnTab),
                               mrDocument.IsTabProtected(mnTab), maSettings.mbShowFormulas, false);
}

bool CellPrimitiveBuilder::Impl::isAvailable(SCCOL nColumn, SCROW nRow) const
{
    return isAvailableForOverflow(mrDocument, ScAddress(nColumn, nRow, mnTab),
                                  mrDocument.IsTabProtected(mnTab), maSettings.mbShowFormulas,
                                  false);
}

bool CellPrimitiveBuilder::Impl::readSource(SCCOL nColumn, SCROW nRow, CellSource& rSource) const
{
    rSource.mnColumn = nColumn;
    rSource.mnRow = nRow;
    rSource.maCell.assign(mrDocument, ScAddress(nColumn, nRow, mnTab));
    if (rSource.maCell.isEmpty() || isCellTextHidden(nColumn, nRow))
        return false;

    rSource.maPattern.setScPatternAttr(mrDocument.GetPattern(nColumn, nRow, mnTab));
    rSource.mpCondSet = mrDocument.GetCondResult(nColumn, nRow, mnTab);
    rSource.mpTableSet = mrDocument.GetTableFormatSet(nColumn, nRow, mnTab);

    const ScMergeAttr& rMerge = rSource.getPattern().GetItem(ATTR_MERGE);
    rSource.mnMergeColumns = std::max<SCCOL>(1, rMerge.GetColMerge());
    rSource.mnMergeRows = std::max<SCROW>(1, rMerge.GetRowMerge());
    return true;
}

bool CellPrimitiveBuilder::Impl::isValueHiddenByConditionalFormat(const CellSource& rSource) const
{
    const ScCondFormatIndexes& rKeys
        = rSource.getPattern().GetItem(ATTR_CONDITIONAL).GetCondFormatData();
    const ScConditionalFormatList* pList = mrDocument.GetCondFormList(mnTab);
    if (rKeys.empty() || !pList)
        return false;

    const ScAddress aAddress(rSource.mnColumn, rSource.mnRow, mnTab);
    bool bDataBarFound = false;
    bool bIconSetFound = false;
    for (sal_uInt32 nKey : rKeys)
    {
        const ScConditionalFormat* pFormat = pList->GetFormat(nKey);
        if (!pFormat)
            continue;
        const ScCondFormatData aData = pFormat->GetData(rSource.maCell, aAddress);
        if (aData.pDataBar && !bDataBarFound)
        {
            if (!aData.pDataBar->mbShowValue)
                return true;
            bDataBarFound = true;
        }
        if (aData.pIconSet && !bIconSetFound)
        {
            if (!aData.pIconSet->mbShowValue)
                return true;
            bIconSetFound = true;
        }
        if (bDataBarFound && bIconSetFound)
            break;
    }
    return false;
}

void CellPrimitiveBuilder::Impl::computeOutputArea(const CellSource& rSource, double fNeeded,
                                                   SvxCellHorJustify eHorJust, bool bNoOverflow,
                                                   bool bBreak, OutputArea& rArea) const
{
    const SCCOL nColumn = rSource.mnColumn;
    const SCROW nRow = rSource.mnRow;
    const bool bMerged = rSource.mnMergeColumns > 1 || rSource.mnMergeRows > 1;

    rArea.maAlignRange = mrGeometry.getRange(nColumn, nRow, nColumn + rSource.mnMergeColumns - 1,
                                             nRow + rSource.mnMergeRows - 1);
    rArea.maClipRange = rArea.maAlignRange;
    rArea.mfColumnWidth = rArea.maAlignRange.getWidth();
    rArea.mnLeftColumn = nColumn;
    rArea.mnRightColumn = nColumn + rSource.mnMergeColumns - 1;
    rArea.mbLeftClip = rArea.mbRightClip = false;
    rArea.mfLeftClipLength = rArea.mfRightClipLength = 0.0;

    const double fWidth = rArea.maAlignRange.getWidth();
    if (fNeeded <= fWidth)
    {
        // Text that also fits beside the autofilter button is aligned and clipped beside it, which
        // always holds for wrapped text.
        if (rSource.getPattern().GetItem(ATTR_MERGE_FLAG).GetValue()
            & (ScMF::Auto | ScMF::Button | ScMF::ButtonPopup))
        {
            const double fButton = constDropDownPixels * maSettings.mfPixelSize;
            if (fNeeded + fButton <= fWidth)
            {
                rArea.maAlignRange = basegfx::B2DRange(
                    rArea.maAlignRange.getMinX(), rArea.maAlignRange.getMinY(),
                    rArea.maAlignRange.getMaxX() - fButton, rArea.maAlignRange.getMaxY());
                rArea.maClipRange = rArea.maAlignRange;
            }
        }
        return;
    }

    double fMissing = fNeeded - fWidth;
    double fLeftMissing = 0.0;
    double fRightMissing = 0.0;
    switch (eHorJust)
    {
        case SvxCellHorJustify::Left:
            fRightMissing = fMissing;
            break;
        case SvxCellHorJustify::Right:
            fLeftMissing = fMissing;
            break;
        case SvxCellHorJustify::Center:
            fLeftMissing = fMissing / 2;
            fRightMissing = fMissing - fLeftMissing;
            break;
        default:
            break;
    }

    double fClipLeft = rArea.maClipRange.getMinX();
    double fClipRight = rArea.maClipRange.getMaxX();
    if (!bMerged && !bNoOverflow && !bBreak)
    {
        // The text overflows into empty neighbors as far as it needs.
        SCCOL nRightColumn = rArea.mnRightColumn;
        while (fRightMissing > 0 && nRightColumn < mrDocument.MaxCol()
               && isAvailable(nRightColumn + 1, nRow))
        {
            ++nRightColumn;
            const double fAdd = mrGeometry.getColumnWidth(nRightColumn);
            fRightMissing -= fAdd;
            fClipRight += fAdd;
        }
        SCCOL nLeftColumn = rArea.mnLeftColumn;
        while (fLeftMissing > 0 && nLeftColumn > 0 && isAvailable(nLeftColumn - 1, nRow))
        {
            --nLeftColumn;
            const double fAdd = mrGeometry.getColumnWidth(nLeftColumn);
            fLeftMissing -= fAdd;
            fClipLeft -= fAdd;
        }
        rArea.mnLeftColumn = nLeftColumn;
        rArea.mnRightColumn = nRightColumn;
    }

    // Text that is still cut off leaves room for the arrow that marks it.
    if (fRightMissing > 0 && maSettings.mbClipMarks && !bBreak && !bNoOverflow)
        fClipRight -= SC_CLIPMARK_SIZE;
    if (fLeftMissing > 0 && maSettings.mbClipMarks && !bBreak && !bNoOverflow)
        fClipLeft += SC_CLIPMARK_SIZE;

    rArea.maClipRange = basegfx::B2DRange(fClipLeft, rArea.maClipRange.getMinY(), fClipRight,
                                          rArea.maClipRange.getMaxY());
    rArea.mbLeftClip = fLeftMissing > 0;
    rArea.mbRightClip = fRightMissing > 0;
    rArea.mfLeftClipLength = std::max(0.0, fLeftMissing);
    rArea.mfRightClipLength = std::max(0.0, fRightMissing);
}

void CellPrimitiveBuilder::Impl::addClipMarks(const CellSource& rSource, const OutputArea& rArea,
                                              bool bLeft, bool bRight)
{
    if (!maSettings.mbClipMarks)
        return;

    const SCROW nRow = rSource.mnRow;
    const SCROW nEndRow = nRow + rSource.mnMergeRows - 1;
    if (bRight)
    {
        const SCCOL nColumn = rArea.mnRightColumn;
        rowLayers(nRow).maMarks.push_back(
            new CellMarkPrimitive2D(CellMarkKind::ClipRight, nColumn, nRow,
                                    mrGeometry.getRange(nColumn, nRow, nColumn, nEndRow),
                                    maSettings.maClipMarkColor, maSettings.mfPixelSize));
    }
    if (bLeft)
    {
        const SCCOL nColumn = rArea.mnLeftColumn;
        rowLayers(nRow).maMarks.push_back(
            new CellMarkPrimitive2D(CellMarkKind::ClipLeft, nColumn, nRow,
                                    mrGeometry.getRange(nColumn, nRow, nColumn, nEndRow),
                                    maSettings.maClipMarkColor, maSettings.mfPixelSize));
    }
}

void CellPrimitiveBuilder::Impl::addOverflowCover(const CellSource& rSource,
                                                  const OutputArea& rArea)
{
    if (rArea.mnLeftColumn == rSource.mnColumn
        && rArea.mnRightColumn == rSource.mnColumn + rSource.mnMergeColumns - 1)
        return;

    maCoverAreas[rSource.mnRow].push_back(
        { rArea.mnLeftColumn, rSource.mnRow, rArea.mnRightColumn, rSource.mnRow,
          mrGeometry.getRange(rArea.mnLeftColumn, rSource.mnRow, rArea.mnRightColumn,
                              rSource.mnRow) });
}

void CellPrimitiveBuilder::Impl::addCellText(SCCOL nColumn, SCROW nRow, const ScCellInfo* pInfo)
{
    if (!maTextDone.emplace(nColumn, nRow).second)
        return;

    if (maSettings.moSkipCell && maSettings.moSkipCell->Col() == nColumn
        && maSettings.moSkipCell->Row() == nRow)
        return;

    // A data bar or an icon set can hide the value it shows.
    if (pInfo
        && ((pInfo->pDataBar && !pInfo->pDataBar->mbShowValue)
            || (pInfo->pIconSet && !pInfo->pIconSet->mbShowValue)))
        return;

    CellSource aSource;
    if (!readSource(nColumn, nRow, aSource))
        return;

    // A cell outside the range has no info from FillInfo, so its conditional formats are read here.
    if (!pInfo && isValueHiddenByConditionalFormat(aSource))
        return;

    if (!addSimpleText(aSource))
        addEditText(aSource);
}

bool CellPrimitiveBuilder::Impl::addSimpleText(const CellSource& rSource)
{
    const ScRefCellValue& rCell = rSource.maCell;
    if (rCell.getType() == CELLTYPE_EDIT)
        return false;

    const ScPatternAttr& rPattern = rSource.getPattern();
    const SfxItemSet* pCondSet = rSource.mpCondSet;
    const SfxItemSet* pTableSet = rSource.mpTableSet;
    const SCCOL nColumn = rSource.mnColumn;
    const SCROW nRow = rSource.mnRow;

    SvNumberFormatter* pFormatter = mrDocument.GetFormatTable();
    const sal_uInt32 nFormat = rPattern.GetNumberFormat(pFormatter, pCondSet);

    SvtScriptType nScript
        = mrDocument.GetCellScriptType(ScAddress(nColumn, nRow, mnTab), nFormat, &rCell);
    if (nScript == SvtScriptType::NONE)
        nScript = ScGlobal::GetDefaultScriptType();

    const SvxCellOrientation eOrient = rPattern.GetCellOrientation(pCondSet);
    SvxCellHorJustify eHorJustAttr = rPattern.GetItem(ATTR_HOR_JUSTIFY, pCondSet).GetValue();
    SvxCellVerJustify eVerJust = rPattern.GetItem(ATTR_VER_JUSTIFY, pCondSet).GetValue();
    if (eVerJust == SvxCellVerJustify::Standard)
        eVerJust = SvxCellVerJustify::Bottom;
    bool bLineBreak = rPattern.GetItem(ATTR_LINEBREAK, pCondSet).GetValue();
    // A number in a plain number format never wraps. Currency, percent, date and time formats do.
    if (bLineBreak && rCell.hasNumeric() && pFormatter->GetType(nFormat) == SvNumFormatType::NUMBER)
        bLineBreak = false;
    const bool bRepeatAttr = eHorJustAttr == SvxCellHorJustify::Repeat;
    const bool bRotated
        = !bRepeatAttr && rPattern.GetItem(ATTR_ROTATE_VALUE, pCondSet).GetValue() != 0_deg100;

    // The EditEngine lays out rotated, stacked, wrapped, distributed and mixed-script text.
    if ((!bRepeatAttr && eOrient != SvxCellOrientation::Standard) || bRotated
        || isAmbiguousScript(nScript) || bLineBreak || eHorJustAttr == SvxCellHorJustify::Block)
        return false;

    const Color* pFormatColor = nullptr;
    OUString aString
        = ScCellFormat::GetString(rCell, nFormat, &pFormatColor, nullptr, mrDocument,
                                  maSettings.mbShowNullValues, maSettings.mbShowFormulas, true);

    // A format can repeat a character to fill the cell, marked by an escape character before it.
    sal_Int32 nRepeatPosition = -1;
    sal_Unicode cRepeat = 0;
    if (nFormat)
    {
        nRepeatPosition = aString.indexOf(0x1B);
        if (nRepeatPosition != -1)
        {
            if (nRepeatPosition + 1 == aString.getLength())
                nRepeatPosition = -1;
            else
            {
                cRepeat = aString[nRepeatPosition + 1];
                aString = aString.replaceAt(nRepeatPosition, 2, u"");
            }
        }
    }
    if (aString.getLength() > constMaxDrawTextLength)
        aString = aString.copy(0, constMaxDrawTextLength);

    bool bCellIsValue = rCell.getType() == CELLTYPE_VALUE;
    bool bCellIsCallable = false;
    if (rCell.getType() == CELLTYPE_FORMULA)
    {
        ScFormulaCell* pFormulaCell = rCell.getFormula();
        if (pFormulaCell->IsMultilineResult())
            return false;
        bCellIsValue = pFormulaCell->IsRunning() || pFormulaCell->IsValue();
        bCellIsCallable = pFormulaCell->IsCallable();
    }
    if (hasEditCharacters(aString, nRepeatPosition >= 0))
        return false;

    vcl::Font aFont;
    rPattern.fillFont(aFont, ScAutoFontColorMode::Display, nullptr, nullptr, pCondSet, pTableSet,
                      nScript, &maSettings.maDocumentColor, &maSettings.maFontColor);
    aFont.SetAlignment(ALIGN_BASELINE);
    aFont.SetKerning(FontKerning::NONE);
    if (pFormatColor)
        aFont.SetColor(*pFormatColor);
    const double fBaseFontHeight = aFont.GetFontSize().Height();

    primitive2d::TextLayouterDevice& rLayouter = getTextLayouter();
    rLayouter.setFont(aFont);
    // The width of a text is where its last advance ends. The advances of the last measured text
    // are kept for drawing it, which is valid because the font changes only before a measurement.
    OUString aMeasuredText;
    std::vector<double> aMeasuredAdvances;
    auto measure = [&rLayouter, &aMeasuredText, &aMeasuredAdvances](const OUString& rText)
    {
        aMeasuredText = rText;
        aMeasuredAdvances = rText.isEmpty() ? std::vector<double>()
                                            : rLayouter.getTextArray(rText, 0, rText.getLength());
        return aMeasuredAdvances.empty() ? 0.0 : aMeasuredAdvances.back();
    };

    const SvxMarginItem& rMargin = rPattern.GetItem(ATTR_MARGIN, pCondSet);
    sal_uInt16 nIndent = 0;
    if (eHorJustAttr == SvxCellHorJustify::Left || eHorJustAttr == SvxCellHorJustify::Right)
        nIndent = rPattern.GetItem(ATTR_INDENT, pCondSet).GetValue();
    const double fLeftTotal = rMargin.GetLeftMargin() + nIndent;
    const double fRightTotal = rMargin.GetRightMargin() + nIndent;
    const SvxCellHorJustify eHorJust = getAlignmentFromContext(
        eHorJustAttr, bCellIsValue, bCellIsCallable, aString, rPattern, pCondSet, mrDocument, mnTab,
        isNumberFormatText(mrDocument, nColumn, nRow, mnTab));

    const bool bRepeat = bRepeatAttr;
    const bool bShrink = !bRepeat && rPattern.GetItem(ATTR_SHRINKTOFIT, pCondSet).GetValue();
    const double fTotalMargin = rMargin.GetLeftMargin() + nIndent + rMargin.GetRightMargin();

    // A number, a repeated text and a text that shrinks to fit stay inside their cell.
    const bool bNoOverflow = bCellIsValue || bRepeat || bShrink;
    double fTextWidth = measure(aString);
    OutputArea aArea;
    computeOutputArea(rSource, fTextWidth + fTotalMargin, eHorJust, bNoOverflow, false, aArea);

    CellTextFlags eFlags = CellTextFlags::NONE;

    // Fill the room of the cell with the repeat character of the number format.
    if (std::optional<OUString> oFilled
        = fillWithRepeatCharacter(aString, nRepeatPosition, cRepeat,
                                  aArea.mfColumnWidth - fTotalMargin - fTextWidth, 1.0, measure))
    {
        aString = *oFilled;
        fTextWidth = measure(aString);
    }

    if (bShrink && (aArea.mbLeftClip || aArea.mbRightClip))
    {
        const double fAvailable = aArea.maAlignRange.getWidth() - fTotalMargin;
        if (fAvailable > 0 && fTextWidth > 0)
        {
            double fScale = fAvailable / fTextWidth;
            auto applyScale = [&]()
            {
                aFont.SetFontHeight(tools::Long(fBaseFontHeight * fScale));
                rLayouter.setFont(aFont);
                fTextWidth = measure(aString);
                return fTextWidth;
            };
            if (shrinkToFit(fAvailable, applyScale,
                            [&]()
                            {
                                fScale *= 0.9;
                                return applyScale();
                            }))
            {
                aArea.mbLeftClip = aArea.mbRightClip = false;
                aArea.mfLeftClipLength = aArea.mfRightClipLength = 0.0;
                eFlags |= CellTextFlags::Shrink;
            }
        }
    }

    if (bRepeat && !aArea.mbLeftClip && !aArea.mbRightClip)
    {
        if (std::optional<OUString> oRepeated = repeatToWidth(
                aString, fTextWidth, aArea.maAlignRange.getWidth() - fTotalMargin, 0.0, measure))
        {
            aString = *oRepeated;
            fTextWidth = measure(aString);
            eFlags |= CellTextFlags::Repeat;
        }
    }

    // Only text that may overflow gets the arrows that mark the cut.
    bool bClipLeftMark = aArea.mbLeftClip && !bNoOverflow;
    bool bClipRightMark = aArea.mbRightClip && !bNoOverflow;

    // A number that does not fit is shown with fewer decimals, or as "###".
    if (bCellIsValue && (aArea.mbLeftClip || aArea.mbRightClip))
    {
        const NumberWidths aWidths{
            [&]()
            {
                double fMaxDigit = 0.0;
                for (char cDigit = '0'; cDigit <= '9'; ++cDigit)
                    fMaxDigit = std::max(fMaxDigit, measure(OUString(sal_Unicode(cDigit))));
                return fMaxDigit;
            },
            [&]() { return measure(u"-"_ustr); },
            [&]() { return measure(ScGlobal::getLocaleData().getLocaleItem().decimalSeparator); },
            [&]() { return measure(u"E"_ustr); }
        };
        OUString aShort(aString);
        // With the formulas shown, every number that does not fit shows "###".
        const NumberFit eFit
            = maSettings.mbShowFormulas
                  ? NumberFit::Hash
                  : fitNumberToWidth(rCell, nFormat, false, mrDocument,
                                     aArea.mfColumnWidth - fTotalMargin, aWidths, measure, aShort);
        if (eFit == NumberFit::Fits || eFit == NumberFit::Failed)
            aString = aShort;
        const bool bHash = eFit == NumberFit::Hash;
        if (bHash)
        {
            aString = u"###"_ustr;
            eFlags |= CellTextFlags::HashFill;
            if (eHorJust == SvxCellHorJustify::Left)
                bClipRightMark = true;
            else if (eHorJust == SvxCellHorJustify::Right)
                bClipLeftMark = true;
            else
                bClipLeftMark = bClipRightMark = true;
            aArea.reserveClipMarks(bClipLeftMark, bClipRightMark);
        }
        fTextWidth = measure(aString);
        if (fTextWidth + fTotalMargin <= aArea.maClipRange.getWidth())
        {
            aArea.mbLeftClip = aArea.mbRightClip = false;
            aArea.mfLeftClipLength = aArea.mfRightClipLength = 0.0;
        }
    }

    const double fAscent = rLayouter.getFontAscent();
    const double fDescent = rLayouter.getFontDescent();
    const double fTextHeight = fAscent + fDescent;

    const basegfx::B2DRange& rAlign = aArea.maAlignRange;
    double fX = rAlign.getMinX();
    switch (eHorJust)
    {
        case SvxCellHorJustify::Left:
            fX += fLeftTotal;
            break;
        case SvxCellHorJustify::Right:
            fX += rAlign.getWidth() - fTextWidth - fRightTotal;
            break;
        case SvxCellHorJustify::Center:
            fX += (rAlign.getWidth() - fTextWidth + fLeftTotal - rMargin.GetRightMargin()) / 2;
            break;
        default:
            break;
    }

    double fY = rAlign.getMinY();
    double fNeededHeight = fTextHeight;
    switch (eVerJust)
    {
        case SvxCellVerJustify::Top:
        case SvxCellVerJustify::Block:
            fY += rMargin.GetTopMargin();
            fNeededHeight += rMargin.GetTopMargin();
            break;
        case SvxCellVerJustify::Center:
            fY += (rAlign.getHeight() + rMargin.GetTopMargin() - fTextHeight
                   - rMargin.GetBottomMargin())
                  / 2;
            fNeededHeight += std::abs(rMargin.GetTopMargin() - rMargin.GetBottomMargin());
            break;
        default:
            fY += rAlign.getHeight() - fTextHeight - rMargin.GetBottomMargin();
            fNeededHeight += rMargin.GetBottomMargin();
            break;
    }

    const bool bHorizontalClip = aArea.mbLeftClip || aArea.mbRightClip;
    const bool bVerticalClip = fNeededHeight > rAlign.getHeight();
    if (bVerticalClip)
        eFlags |= CellTextFlags::ClippedBottom;
    if (aArea.mbLeftClip)
        eFlags |= CellTextFlags::ClippedLeft;
    if (aArea.mbRightClip)
        eFlags |= CellTextFlags::ClippedRight;

    // Only the dimension that is cut off is clipped, so text that fits is drawn whole.
    const double fHuge = 1e9;
    const basegfx::B2DRange aClip(bHorizontalClip ? aArea.maClipRange.getMinX() : -fHuge,
                                  bVerticalClip ? aArea.maClipRange.getMinY() : -fHuge,
                                  bHorizontalClip ? aArea.maClipRange.getMaxX() : fHuge,
                                  bVerticalClip ? aArea.maClipRange.getMaxY() : fHuge);

    primitive2d::Primitive2DContainer aText;
    basegfx::B2DRange aTextRange;
    if (!aString.isEmpty())
    {
        basegfx::B2DVector aFontSize;
        const attribute::FontAttribute aFontAttribute
            = primitive2d::getFontAttributeFromVclFont(aFontSize, aFont, false, false);
        std::vector<double> aDXArray
            = aMeasuredText == aString ? std::move(aMeasuredAdvances)
                                       : rLayouter.getTextArray(aString, 0, aString.getLength());
        const basegfx::B2DHomMatrix aTransform(basegfx::utils::createScaleTranslateB2DHomMatrix(
            aFontSize.getX(), aFontSize.getY(), fX, fY + fAscent));
        const css::lang::Locale aLocale(LanguageTag::convertToLocale(aFont.GetLanguage()));
        const basegfx::BColor aFontColor(aFont.GetColor().getBColor());

        const bool bDecorated
            = aFont.GetUnderline() != LINESTYLE_NONE || aFont.GetOverline() != LINESTYLE_NONE
              || aFont.GetStrikeout() != STRIKEOUT_NONE
              || (aFont.GetEmphasisMark() & FontEmphasisMark::Style) != FontEmphasisMark::NONE
              || aFont.GetRelief() != FontRelief::NONE || aFont.IsShadow();
        if (bDecorated)
        {
            Color aUnderlineColor = rPattern.GetItem(ATTR_FONT_UNDERLINE, pCondSet).GetColor();
            if (aUnderlineColor == COL_AUTO)
                aUnderlineColor = aFont.GetColor();
            Color aOverlineColor = rPattern.GetItem(ATTR_FONT_OVERLINE, pCondSet).GetColor();
            if (aOverlineColor == COL_AUTO)
                aOverlineColor = aFont.GetColor();
            const FontEmphasisMark eEmphasis = aFont.GetEmphasisMark();
            aText.push_back(new primitive2d::TextDecoratedPortionPrimitive2D(
                aTransform, aString, 0, aString.getLength(), std::move(aDXArray), {},
                aFontAttribute, aLocale, aFontColor, COL_TRANSPARENT, 0, 100, 0, false,
                aOverlineColor.getBColor(), aUnderlineColor.getBColor(),
                primitive2d::mapFontLineStyleToTextLine(aFont.GetOverline()),
                primitive2d::mapFontLineStyleToTextLine(aFont.GetUnderline()), false,
                primitive2d::mapFontStrikeoutToTextStrikeout(aFont.GetStrikeout()),
                aFont.IsWordLineMode(), mapEmphasisMark(eEmphasis),
                bool(eEmphasis & FontEmphasisMark::PosAbove),
                bool(eEmphasis & FontEmphasisMark::PosBelow), mapRelief(aFont.GetRelief()),
                aFont.IsShadow()));
        }
        else
        {
            // The glyphs are measured in the font the text is laid out in, from its baseline.
            aTextRange = rLayouter.getTextBoundRect(aString, 0, aString.getLength());
            aTextRange.transform(basegfx::utils::createTranslateB2DHomMatrix(fX, fY + fAscent));
            aText.push_back(new primitive2d::TextSimplePortionPrimitive2D(
                aTransform, aString, 0, aString.getLength(), std::move(aDXArray), {},
                aFontAttribute, aLocale, aFontColor));
        }
    }

    if (!aText.empty())
    {
        CellPrimitiveLayers& rLayers = rowLayers(rSource.mnRow);
        rLayers.maText.push_back(new CellTextPrimitive2D(
            rSource.mnColumn, rSource.mnRow, aArea.maAlignRange, aClip,
            bHorizontalClip || bVerticalClip, eHorJust, eVerJust, eFlags, std::move(aText),
            aTextRange));
        addOverflowCover(rSource, aArea);
    }
    addClipMarks(rSource, aArea, bClipLeftMark, bClipRightMark);
    return true;
}

ScFieldEditEngine& CellPrimitiveBuilder::Impl::getEditEngine()
{
    if (!mpEditEngine)
    {
        mpEditEngine
            = std::make_unique<ScFieldEditEngine>(&mrDocument, mrDocument.GetEditEnginePool());
        mpEditEngine->SetUpdateLayout(false);
        mpEditEngine->EnableUndo(false);
        // The cell items are in 1/100 mm, so the engine lays out in 1/100 mm too.
        mpEditEngine->SetRefMapMode(MapMode(MapUnit::Map100thMM));
        EEControlBits nControl = mpEditEngine->GetControlWord();
        nControl &= ~EEControlBits::MARKURLFIELDS;
        nControl &= ~EEControlBits::ONLINESPELLING;
        mpEditEngine->SetControlWord(nControl);
        mpEditEngine->EnableAutoColor(true);
        mrDocument.ApplyAsianEditSettings(*mpEditEngine);
        mpEditEngine->SetDefaultHorizontalTextDirection(mrDocument.GetEditTextDirection(mnTab));
    }
    return *mpEditEngine;
}

primitive2d::TextLayouterDevice& CellPrimitiveBuilder::Impl::getTextLayouter()
{
    if (!mpTextLayouter)
        mpTextLayouter = std::make_unique<primitive2d::TextLayouterDevice>();
    return *mpTextLayouter;
}

void CellPrimitiveBuilder::Impl::addEditText(const CellSource& rSource)
{
    const ScRefCellValue& rCell = rSource.maCell;
    const ScPatternAttr& rPattern = rSource.getPattern();
    const SfxItemSet* pCondSet = rSource.mpCondSet;
    const SCCOL nColumn = rSource.mnColumn;
    const SCROW nRow = rSource.mnRow;

    ScFieldEditEngine& rEngine = getEditEngine();
    rEngine.SetUpdateLayout(false);
    rEngine.SetText(OUString());

    SvxCellOrientation eOrient = rPattern.GetCellOrientation(pCondSet);
    const SvxCellHorJustify eHorJustAttr = rPattern.GetItem(ATTR_HOR_JUSTIFY, pCondSet).GetValue();
    SvxCellVerJustify eVerJust = rPattern.GetItem(ATTR_VER_JUSTIFY, pCondSet).GetValue();
    const SvxCellJustifyMethod eHorJustMethod
        = rPattern.GetItem(ATTR_HOR_JUSTIFY_METHOD, pCondSet).GetValue();
    Degree100 nRotate = rPattern.GetItem(ATTR_ROTATE_VALUE, pCondSet).GetValue();
    bool bBreak = eHorJustAttr == SvxCellHorJustify::Block
                  || rPattern.GetItem(ATTR_LINEBREAK, pCondSet).GetValue();
    if (eHorJustAttr == SvxCellHorJustify::Repeat)
    {
        // "repeat" draws the text once and turns off orientation and rotation.
        eOrient = SvxCellOrientation::Standard;
        nRotate = 0_deg100;
    }
    const bool bStacked = eOrient == SvxCellOrientation::Stacked;
    const bool bVertical
        = eOrient == SvxCellOrientation::TopBottom || eOrient == SvxCellOrientation::BottomUp;
    if (bStacked)
        bBreak = true;

    bool bCellIsValue = false;
    switch (rCell.getType())
    {
        case CELLTYPE_VALUE:
            bCellIsValue = true;
            break;
        case CELLTYPE_FORMULA:
        {
            // A formula that is still being calculated counts as a number.
            ScFormulaCell* pFormulaCell = rCell.getFormula();
            bCellIsValue = pFormulaCell->IsRunning() || pFormulaCell->IsValue();
            break;
        }
        default:
            break;
    }
    // A number in a plain number format never wraps. Currency, percent, date and time formats do.
    SvNumberFormatter* pFormatter = mrDocument.GetFormatTable();
    const sal_uInt32 nFormat = rPattern.GetNumberFormat(pFormatter, pCondSet);
    if (bBreak && bCellIsValue && !bStacked
        && pFormatter->GetType(nFormat) == SvNumFormatType::NUMBER)
        bBreak = false;

    const OUString aCellString = mrDocument.GetString(nColumn, nRow, mnTab);
    const SvxCellHorJustify eHorJustContext = getAlignmentFromContext(
        eHorJustAttr, bCellIsValue, false, aCellString, rPattern, pCondSet, mrDocument, mnTab,
        isNumberFormatText(mrDocument, nColumn, nRow, mnTab));
    const SvxCellHorJustify eHorJust
        = eHorJustAttr == SvxCellHorJustify::Block ? SvxCellHorJustify::Block : eHorJustContext;

    // The cell gives the engine its defaults and the background for the automatic font color.
    SfxItemSet aSet(rEngine.GetEmptyItemSet());
    rPattern.FillEditItemSet(&aSet, pCondSet, rSource.mpTableSet);
    rEngine.SetDefaults(std::move(aSet));
    EEControlBits nControl = rEngine.GetControlWord();
    if (bStacked)
        nControl |= EEControlBits::ONECHARPERLINE;
    else
        nControl &= ~EEControlBits::ONECHARPERLINE;
    rEngine.SetControlWord(nControl);
    Color aBackColor = rPattern.GetItem(ATTR_BACKGROUND, pCondSet).GetColor();
    if (aBackColor.IsTransparent())
        aBackColor = maSettings.maDocumentColor;
    rEngine.SetBackgroundColor(aBackColor);

    SvxAdjust eAdjust = SvxAdjust::Left;
    if (bStacked)
        eAdjust = SvxAdjust::Center;
    else if (bBreak)
    {
        switch (eHorJust)
        {
            case SvxCellHorJustify::Center:
                eAdjust = SvxAdjust::Center;
                break;
            case SvxCellHorJustify::Right:
                eAdjust = SvxAdjust::Right;
                break;
            case SvxCellHorJustify::Block:
                eAdjust = SvxAdjust::Block;
                break;
            default:
                break;
        }
    }
    rEngine.SetDefaultItem(SvxAdjustItem(eAdjust, EE_PARA_JUST));
    rEngine.SetDefaultItem(SvxJustifyMethodItem(eHorJustMethod, EE_PARA_JUST_METHOD));
    if (eVerJust == SvxCellVerJustify::Block)
        rEngine.SetDefaultItem(SvxVerJustifyItem(SvxCellVerJustify::Block, EE_PARA_VER_JUST));
    rEngine.SetVertical(false);

    const SvxMarginItem& rMargin = rPattern.GetItem(ATTR_MARGIN, pCondSet);
    sal_uInt16 nIndent = 0;
    if (eHorJustAttr == SvxCellHorJustify::Left || eHorJustAttr == SvxCellHorJustify::Right)
        nIndent = rPattern.GetItem(ATTR_INDENT, pCondSet).GetValue();
    double fLeftMargin = rMargin.GetLeftMargin() + nIndent;
    double fRightMargin = rMargin.GetRightMargin();
    if (eHorJustAttr == SvxCellHorJustify::Right)
    {
        fLeftMargin = rMargin.GetLeftMargin();
        fRightMargin = rMargin.GetRightMargin() + nIndent;
    }
    const double fTopMargin = rMargin.GetTopMargin();
    const double fBottomMargin = rMargin.GetBottomMargin();

    // Wrapped text lays out at the cell width, other text on a page wide enough for any line.
    const basegfx::B2DRange aCellRange = mrGeometry.getRange(
        nColumn, nRow, nColumn + rSource.mnMergeColumns - 1, nRow + rSource.mnMergeRows - 1);
    const double fInnerWidth = aCellRange.getWidth() - fLeftMargin - fRightMargin;
    const double fInnerHeight = aCellRange.getHeight() - fTopMargin - fBottomMargin;
    Size aPaperSize(1000000, 1000000);
    // Vertical text breaks at the height of the cell.
    const double fLineLength = std::max(0.0, bVertical ? fInnerHeight : fInnerWidth);
    if (bBreak)
        aPaperSize.setWidth(tools::Long(fLineLength * constMm100PerTwip));
    rEngine.SetPaperSize(aPaperSize);

    bool bWrapFields = false;
    if (rCell.getType() == CELLTYPE_EDIT)
    {
        const EditTextObject* pData = rCell.getEditText();
        if (!pData)
            return;
        rEngine.SetTextCurrentDefaults(*pData);
        // Fields do not wrap, so a wrapped cell with a field is clipped to the cell.
        if (bBreak && pData->HasField())
            bWrapFields = true;
    }
    else
    {
        const Color* pColor = nullptr;
        const OUString aString
            = ScCellFormat::GetString(rCell, nFormat, &pColor, nullptr, mrDocument,
                                      maSettings.mbShowNullValues, maSettings.mbShowFormulas);
        rEngine.SetTextCurrentDefaults(aString);
        if (pColor)
        {
            // The text has no attributes, so the number format color goes into the defaults.
            rEngine.SetDefaultItem(SvxColorItem(*pColor, EE_CHAR_COLOR));
        }
    }

    rEngine.SetUpdateLayout(true);

    // Wrapped lines take the width of the page, other text the width of its longest line.
    auto engineWidth = [&]()
    {
        if (bBreak && !bStacked)
            return fLineLength;
        return double(rEngine.CalcTextWidth()) * constTwipsPerMm100;
    };
    auto engineHeight = [&]() { return double(rEngine.GetTextHeight()) * constTwipsPerMm100; };

    double fEngineWidth = engineWidth();
    double fEngineHeight = engineHeight();
    if (bStacked)
        fEngineWidth = fEngineWidth * 11 / 10;

    // The extent of the text in the cell, which is turned for vertical text.
    double fExtentX = bVertical ? fEngineHeight : fEngineWidth;
    double fExtentY = bVertical ? fEngineWidth : fEngineHeight;

    CellTextFlags eFlags = CellTextFlags::EditEngine;
    if (bBreak)
        eFlags |= CellTextFlags::Wrap;

    OutputArea aArea;
    const bool bShrink = !bBreak && rPattern.GetItem(ATTR_SHRINKTOFIT, pCondSet).GetValue()
                         && eHorJustAttr != SvxCellHorJustify::Repeat;
    // A number, a text that shrinks to fit and vertical or rotated text stay inside their cell.
    const bool bNoOverflow = bCellIsValue || bShrink || bVertical || nRotate != 0_deg100;
    // Wrapped lines take no width of their own, the way the engine size of the window counts them.
    const double fNeededWidth = (bBreak && !bStacked ? 0.0 : fExtentX) + fLeftMargin + fRightMargin;
    computeOutputArea(rSource, fNeededWidth, eHorJustContext, bNoOverflow, bBreak, aArea);

    if (bShrink && (aArea.mbLeftClip || aArea.mbRightClip || fExtentY > fInnerHeight))
    {
        // The fonts shrink until the text fits, after the first step at most 7 times by 10%.
        const double fAvailable = bVertical ? fInnerHeight : fInnerWidth;
        const double fNeeded = bVertical ? fExtentY : fExtentX;
        if (fAvailable > 0 && fNeeded > fAvailable)
        {
            double fScale = fAvailable / fNeeded;
            auto applyScale = [&]()
            {
                rEngine.setScalingParameters({ fScale, fScale, 1.0, 1.0 });
                fEngineWidth = engineWidth();
                fEngineHeight = engineHeight();
                fExtentX = bVertical ? fEngineHeight : fEngineWidth;
                fExtentY = bVertical ? fEngineWidth : fEngineHeight;
                return bVertical ? fExtentY : fExtentX;
            };
            if (shrinkToFit(fAvailable, applyScale,
                            [&]()
                            {
                                fScale *= 0.9;
                                return applyScale();
                            }))
            {
                aArea.mbLeftClip = aArea.mbRightClip = false;
                eFlags |= CellTextFlags::Shrink;
            }
        }
    }

    bool bClipLeftMark = false;
    bool bClipRightMark = false;
    if (bCellIsValue && (aArea.mbLeftClip || aArea.mbRightClip) && !bVertical
        && nRotate == 0_deg100)
    {
        rEngine.SetTextCurrentDefaults(u"###"_ustr);
        fEngineWidth = engineWidth();
        fEngineHeight = engineHeight();
        fExtentX = fEngineWidth;
        fExtentY = fEngineHeight;
        eFlags |= CellTextFlags::HashFill;
        bClipLeftMark = eHorJustContext != SvxCellHorJustify::Left;
        bClipRightMark = eHorJustContext != SvxCellHorJustify::Right;
        aArea.reserveClipMarks(bClipLeftMark, bClipRightMark);
        if (fExtentX + fLeftMargin + fRightMargin <= aArea.maClipRange.getWidth())
            aArea.mbLeftClip = aArea.mbRightClip = false;
    }
    else if (!bNoOverflow)
    {
        // Only text that may overflow gets the arrows that mark the cut.
        bClipLeftMark = aArea.mbLeftClip;
        bClipRightMark = aArea.mbRightClip;
    }

    if (!bBreak && !bVertical && !bStacked
        && (eHorJustContext == SvxCellHorJustify::Right
            || eHorJustContext == SvxCellHorJustify::Center))
    {
        // Several paragraphs align against each other within the width of the widest.
        rEngine.SetPaperSize(Size(tools::Long(fEngineWidth * constMm100PerTwip) + 1, 1000000));
        rEngine.SetDefaultItem(SvxAdjustItem(
            eHorJustContext == SvxCellHorJustify::Center ? SvxAdjust::Center : SvxAdjust::Right,
            EE_PARA_JUST));
        fEngineHeight = engineHeight();
        fExtentY = fEngineHeight;
    }

    const basegfx::B2DRange& rAlign = aArea.maAlignRange;
    const double fAlignInnerWidth = rAlign.getWidth() - fLeftMargin - fRightMargin;
    const double fAlignInnerHeight = rAlign.getHeight() - fTopMargin - fBottomMargin;

    // The box the text takes in the cell.
    double fBoxLeft = rAlign.getMinX() + fLeftMargin;
    if (!bBreak || bVertical)
    {
        if (eHorJustContext == SvxCellHorJustify::Right)
            fBoxLeft = rAlign.getMaxX() - fRightMargin - fExtentX;
        else if (eHorJustContext == SvxCellHorJustify::Center)
            fBoxLeft = rAlign.getMinX() + fLeftMargin + (fAlignInnerWidth - fExtentX) / 2;
    }
    double fBoxTop = rAlign.getMinY() + fTopMargin;
    if (eVerJust == SvxCellVerJustify::Bottom || eVerJust == SvxCellVerJustify::Standard)
        fBoxTop += fAlignInnerHeight - fExtentY;
    else if (eVerJust == SvxCellVerJustify::Center)
        fBoxTop += (fAlignInnerHeight - fExtentY) / 2;

    // Text that does not fit the height of a wrapped cell starts at the top.
    if (fExtentY > fAlignInnerHeight && bBreak && eVerJust == SvxCellVerJustify::Standard)
        fBoxTop = rAlign.getMinY() + fTopMargin;

    // The engine lays out from (0, 0) in 1/100 mm. The transformation scales to twips and rotates.
    basegfx::B2DHomMatrix aTransform
        = basegfx::utils::createScaleB2DHomMatrix(constTwipsPerMm100, constTwipsPerMm100);
    if (eOrient == SvxCellOrientation::BottomUp)
    {
        aTransform *= basegfx::utils::createRotateB2DHomMatrix(-M_PI / 2);
        aTransform *= basegfx::utils::createTranslateB2DHomMatrix(fBoxLeft, fBoxTop + fExtentY);
    }
    else if (eOrient == SvxCellOrientation::TopBottom)
    {
        aTransform *= basegfx::utils::createRotateB2DHomMatrix(M_PI / 2);
        aTransform *= basegfx::utils::createTranslateB2DHomMatrix(fBoxLeft + fExtentX, fBoxTop);
    }
    else if (nRotate != 0_deg100)
    {
        // Rising text starts at the bottom-left of the cell and falling text at the top-left. Both
        // overflow the cell like rotated text in the window.
        const double fAngle = toRadians(nRotate);
        aTransform *= basegfx::utils::createRotateB2DHomMatrix(-fAngle);
        const bool bRising = std::sin(fAngle) > 0;
        const double fStartY
            = bRising ? rAlign.getMaxY() - fBottomMargin - fEngineHeight * std::cos(fAngle)
                      : rAlign.getMinY() + fTopMargin;
        aTransform *= basegfx::utils::createTranslateB2DHomMatrix(
            rAlign.getMinX() + fLeftMargin + std::max(0.0, -std::sin(fAngle) * fEngineHeight),
            fStartY);
    }
    else
        aTransform *= basegfx::utils::createTranslateB2DHomMatrix(fBoxLeft, fBoxTop);

    TextHierarchyBreakup aHelper;
    rEngine.StripPortions(aHelper);
    rEngine.setScalingParameters({});
    primitive2d::Primitive2DContainer aContent(aHelper.getTextPortionPrimitives());
    if (aContent.empty())
        return;

    primitive2d::Primitive2DContainer aText{ new primitive2d::TransformPrimitive2D(
        aTransform, std::move(aContent)) };

    const bool bVerticalClip = fExtentY > fAlignInnerHeight + 1;
    const bool bHorizontalClip
        = aArea.mbLeftClip || aArea.mbRightClip || bWrapFields || (bBreak && !bVertical);
    const bool bRotatedOut = nRotate != 0_deg100;
    if (bVerticalClip)
        eFlags |= CellTextFlags::ClippedBottom;
    if (aArea.mbLeftClip)
        eFlags |= CellTextFlags::ClippedLeft;
    if (aArea.mbRightClip)
        eFlags |= CellTextFlags::ClippedRight;

    const double fHuge = 1e9;
    const bool bClip = !bRotatedOut && (bHorizontalClip || bVerticalClip);
    const basegfx::B2DRange aClip(bHorizontalClip ? aArea.maClipRange.getMinX() : -fHuge,
                                  bVerticalClip ? aArea.maClipRange.getMinY() : -fHuge,
                                  bHorizontalClip ? aArea.maClipRange.getMaxX() : fHuge,
                                  bVerticalClip ? aArea.maClipRange.getMaxY() : fHuge);

    rowLayers(nRow).maText.push_back(new CellTextPrimitive2D(nColumn, nRow, aArea.maAlignRange,
                                                             aClip, bClip, eHorJust, eVerJust,
                                                             eFlags, std::move(aText)));
    addOverflowCover(rSource, aArea);
    addClipMarks(rSource, aArea, bClipLeftMark, bClipRightMark);
}

void CellPrimitiveBuilder::Impl::buildText(const ScTableInfo& rInfo)
{
    const RowInfo* pRowInfo = rInfo.mpRowInfo.get();
    std::set<std::pair<SCCOL, SCROW>> aOutsideOrigins;

    SCCOL nLastContentColumn = mrDocument.MaxCol();
    if (maSettings.mbTextFromOutside)
    {
        SCROW nLastContentRow;
        mrDocument.GetCellArea(mnTab, nLastContentColumn, nLastContentRow);
    }

    for (SCSIZE nArrY = 1; nArrY + 1 < rInfo.mnArrCount; ++nArrY)
    {
        const RowInfo& rRow = pRowInfo[nArrY];
        const SCROW nRow = rRow.nRowNo;
        if (!isContentRow(nRow))
            continue;

        for (SCCOL nColumn = mnStartColumn; nColumn <= mnEndColumn; ++nColumn)
        {
            const ScCellInfo& rCell = rRow.cellInfo(nColumn);
            if (rCell.bHOverlapped || rCell.bVOverlapped)
            {
                // A merged area shows the text of its first cell, which can be outside the range.
                if (maSettings.mbTextFromOutside)
                {
                    SCCOL nOriginColumn = nColumn;
                    SCROW nOriginRow = nRow;
                    mrDocument.ExtendOverlapped(nOriginColumn, nOriginRow, nColumn, nRow, mnTab);
                    if (nOriginColumn < mnStartColumn || nOriginRow < mnStartRow)
                        aOutsideOrigins.emplace(nOriginColumn, nOriginRow);
                }
                continue;
            }
            if (mrGeometry.getColumnWidth(nColumn) <= 0.0)
                continue;
            if (rRow.basicCellInfo(nColumn).bEmptyCellText)
                continue;
            addCellText(nColumn, nRow, &rCell);
        }

        if (!maSettings.mbTextFromOutside)
            continue;

        // Text from outside can overflow into the range through empty edge cells, so the walk goes
        // outward from the edge to the first cell that is not empty.
        SCCOL nLeft = mnStartColumn;
        while (nLeft > 0 && isAvailable(nLeft, nRow))
            --nLeft;
        if (nLeft < mnStartColumn && !isAvailable(nLeft, nRow)
            && !mrDocument.HasAttrib(nLeft, nRow, mnTab, mnStartColumn, nRow, mnTab,
                                     HasAttrFlags::Merged | HasAttrFlags::Overlapped))
            addCellText(nLeft, nRow, nullptr);

        SCCOL nRight = mnEndColumn;
        while (nRight < nLastContentColumn && isAvailable(nRight, nRow))
            ++nRight;
        if (nRight > mnEndColumn && !isAvailable(nRight, nRow)
            && !mrDocument.HasAttrib(mnEndColumn, nRow, mnTab, nRight, nRow, mnTab,
                                     HasAttrFlags::Merged | HasAttrFlags::Overlapped))
            addCellText(nRight, nRow, nullptr);
    }

    for (const auto& [nColumn, nRow] : aOutsideOrigins)
        addCellText(nColumn, nRow, nullptr);
}

void CellPrimitiveBuilder::Impl::buildSparklines(const ScTableInfo& rInfo)
{
    SparklineRenderer aRenderer(mrDocument);
    const double fSparklineScale = constPixelTwips / maSettings.mfPixelSize;
    const RowInfo* pRowInfo = rInfo.mpRowInfo.get();
    for (SCSIZE nArrY = 1; nArrY + 1 < rInfo.mnArrCount; ++nArrY)
    {
        const RowInfo& rRow = pRowInfo[nArrY];
        const SCROW nRow = rRow.nRowNo;
        if (!isContentRow(nRow))
            continue;
        for (SCCOL nColumn = mnStartColumn; nColumn <= mnEndColumn; ++nColumn)
        {
            const ScCellInfo& rCell = rRow.cellInfo(nColumn);
            if (rCell.bHOverlapped || rCell.bVOverlapped
                || mrGeometry.getColumnWidth(nColumn) <= 0.0)
                continue;

            std::shared_ptr<sc::Sparkline> pSparkline
                = mrDocument.GetSparkline(ScAddress(nColumn, nRow, mnTab));
            if (!pSparkline)
                continue;

            SCCOL nEndColumn = nColumn;
            SCROW nEndRow = nRow;
            if (rCell.bMerged)
            {
                const ScMergeAttr& rMerge = mrDocument.GetAttr(nColumn, nRow, mnTab, ATTR_MERGE);
                nEndColumn += std::max<SCCOL>(1, rMerge.GetColMerge()) - 1;
                nEndRow += std::max<SCROW>(1, rMerge.GetRowMerge()) - 1;
            }
            const basegfx::B2DRange aRange
                = mrGeometry.getRange(nColumn, nRow, nEndColumn, nEndRow);

            // A line weight in points is as many pixels at 100% zoom, so lines and markers grow
            // with the zoom like in the window. The free border keeps its size in pixels.
            primitive2d::Primitive2DContainer aContent = aRenderer.createPrimitives(
                pSparkline, aRange, maSettings.mfPixelSize, maSettings.mfPixelSize, fSparklineScale,
                fSparklineScale);
            if (aContent.empty())
                continue;

            OString aGroupId;
            if (auto pGroup = pSparkline->getSparklineGroup())
                aGroupId = pGroup->getID().getString();
            rowLayers(nRow).maExtras.push_back(new SparklinePrimitive2D(
                nColumn, nRow, aRange, std::move(aGroupId), std::move(aContent)));
        }
    }
}

void CellPrimitiveBuilder::Impl::buildMarks(const ScTableInfo& rInfo)
{
    const RowInfo* pRowInfo = rInfo.mpRowInfo.get();
    for (SCSIZE nArrY = 1; nArrY + 1 < rInfo.mnArrCount; ++nArrY)
    {
        const RowInfo& rRow = pRowInfo[nArrY];
        const SCROW nRow = rRow.nRowNo;
        if (!isContentRow(nRow))
            continue;
        for (SCCOL nColumn = mnStartColumn; nColumn <= mnEndColumn; ++nColumn)
        {
            const ScCellInfo& rCell = rRow.cellInfo(nColumn);
            if (rCell.bHOverlapped || rCell.bVOverlapped
                || mrGeometry.getColumnWidth(nColumn) <= 0.0)
                continue;

            SCCOL nEndColumn = nColumn;
            SCROW nEndRow = nRow;
            if (rCell.bMerged)
            {
                const ScMergeAttr& rMerge = mrDocument.GetAttr(nColumn, nRow, mnTab, ATTR_MERGE);
                nEndColumn += std::max<SCCOL>(1, rMerge.GetColMerge()) - 1;
                nEndRow += std::max<SCROW>(1, rMerge.GetRowMerge()) - 1;
            }
            const basegfx::B2DRange aRange
                = mrGeometry.getRange(nColumn, nRow, nEndColumn, nEndRow);

            if (rCell.maCell.getType() == CELLTYPE_FORMULA)
            {
                if (maSettings.mbFormulaMarks)
                    rowLayers(nRow).maMarks.push_back(
                        new CellMarkPrimitive2D(CellMarkKind::Formula, nColumn, nRow, aRange,
                                                constFormulaMarkColor, maSettings.mfPixelSize));
                if (maSettings.mbFormulaErrorMarks
                    && rCell.maCell.getFormula()->GetRawError() != FormulaError::NONE)
                    rowLayers(nRow).maMarks.push_back(new CellMarkPrimitive2D(
                        CellMarkKind::FormulaError, nColumn, nRow, aRange,
                        constFormulaErrorMarkColor, maSettings.mfPixelSize));
            }

            if (maSettings.mbButtons)
            {
                if (rCell.bAutoFilter)
                    rowLayers(nRow).maMarks.push_back(new CellMarkPrimitive2D(
                        CellMarkKind::AutoFilter, nColumn, nRow, aRange, COL_LIGHTBLUE,
                        maSettings.mfPixelSize, rCell.bFilterActive));
                else if (rCell.bPivotButton || rCell.bPivotPopupButton
                         || rCell.bPivotPopupButtonMulti)
                    rowLayers(nRow).maMarks.push_back(new CellMarkPrimitive2D(
                        CellMarkKind::PivotButton, nColumn, nRow, aRange, COL_LIGHTBLUE,
                        maSettings.mfPixelSize, rCell.bFilterActive));
            }
        }
    }
}

void CellPrimitiveBuilder::Impl::buildGridCover(const ScTableInfo& rInfo)
{
    // Merged areas whose first cell is in the range hide the grid lines inside them.
    const RowInfo* pRowInfo = rInfo.mpRowInfo.get();
    for (SCSIZE nArrY = 1; nArrY + 1 < rInfo.mnArrCount; ++nArrY)
    {
        const RowInfo& rRow = pRowInfo[nArrY];
        const SCROW nRow = rRow.nRowNo;
        if (!isContentRow(nRow))
            continue;
        for (SCCOL nColumn = mnStartColumn; nColumn <= mnEndColumn; ++nColumn)
        {
            const ScCellInfo& rCell = rRow.cellInfo(nColumn);
            if (!rCell.bMerged)
                continue;
            const ScMergeAttr& rMerge = mrDocument.GetAttr(nColumn, nRow, mnTab, ATTR_MERGE);
            const SCCOL nEndColumn = nColumn + std::max<SCCOL>(1, rMerge.GetColMerge()) - 1;
            const SCROW nEndRow = nRow + std::max<SCROW>(1, rMerge.GetRowMerge()) - 1;
            if (nEndColumn == nColumn && nEndRow == nRow)
                continue;
            maCoverAreas[nRow].push_back(
                { nColumn, nRow, nEndColumn, nEndRow,
                  mrGeometry.getRange(nColumn, nRow, nEndColumn, nEndRow) });
        }
    }
}

CellPrimitiveBuilder::CellPrimitiveBuilder(ScDocument& rDocument, SCTAB nTab,
                                           const CellGeometry& rGeometry,
                                           CellPrimitiveSettings aSettings)
    : mpImpl(std::make_unique<Impl>(rDocument, nTab, rGeometry, std::move(aSettings)))
{
}

CellPrimitiveBuilder::~CellPrimitiveBuilder() = default;

CellPrimitiveLayers CellPrimitiveBuilder::build(SCCOL nStartColumn, SCROW nStartRow,
                                                SCCOL nEndColumn, SCROW nEndRow)
{
    return mpImpl->build(nStartColumn, nStartRow, nEndColumn, nEndRow, nullptr);
}

CellPrimitiveLayers CellPrimitiveBuilder::build(SCCOL nStartColumn, SCROW nStartRow,
                                                SCCOL nEndColumn, SCROW nEndRow, ScTableInfo& rInfo)
{
    return mpImpl->build(nStartColumn, nStartRow, nEndColumn, nEndRow, &rInfo);
}

CellPrimitiveRows CellPrimitiveBuilder::buildRows(SCCOL nStartColumn, SCROW nStartRow,
                                                  SCCOL nEndColumn, SCROW nEndRow)
{
    return mpImpl->buildRows(nStartColumn, nStartRow, nEndColumn, nEndRow, nStartRow, nEndRow);
}

CellPrimitiveRows CellPrimitiveBuilder::buildRows(SCCOL nStartColumn, SCROW nStartRow,
                                                  SCCOL nEndColumn, SCROW nEndRow,
                                                  SCROW nFirstContentRow, SCROW nLastContentRow)
{
    return mpImpl->buildRows(nStartColumn, nStartRow, nEndColumn, nEndRow, nFirstContentRow,
                             nLastContentRow);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
