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

#include <VectorContent.hxx>

#include <CellGeometry.hxx>
#include <CellMarkPrimitive2D.hxx>
#include <CellPositionJsonProcessor.hxx>
#include <CellPrimitiveBuilder.hxx>
#include <CellTextPrimitive2D.hxx>
#include <GridCoverPrimitive2D.hxx>
#include <SparklinePrimitive2D.hxx>
#include <sc_primitivetypes2d.hxx>

#include <brdcst.hxx>
#include <bulkdatahint.hxx>
#include <cellvalue.hxx>
#include <columnspanset.hxx>
#include <docsh.hxx>
#include <document.hxx>
#include <drawview.hxx>
#include <drwlayer.hxx>
#include <global.hxx>
#include <hints.hxx>
#include <scmod.hxx>
#include <tabvwsh.hxx>
#include <uiitems.hxx>
#include <userdat.hxx>
#include <viewdata.hxx>
#include <viewopti.hxx>
#include <gridwin.hxx>

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <comphelper/kit.hxx>
#include <comphelper/scopeguard.hxx>
#include <cppuhelper/implbase.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/Primitive2DContainer.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/processor2d/Primitive2dJsonProcessor.hxx>
#include <editeng/StripPortionsHelper.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <o3tl/string_view.hxx>
#include <o3tl/unit_conversion.hxx>
#include <sfx2/kit/helper.hxx>
#include <sfx2/viewsh.hxx>
#include <svl/listener.hxx>
#include <svtools/colorcfg.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/svdmodel.hxx>
#include <svx/VectorObjectContent.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <svx/unoapi.hxx>
#include <tools/json_writer.hxx>

#include <algorithm>
#include <functional>
#include <set>
#include <string_view>

using namespace drawinglayer;

namespace
{
/// Twips per 1/100 mm, the unit of the drawing layer and of the EditEngine of Calc.
constexpr double constTwipsPerMm100 = o3tl::convert(1.0, o3tl::Length::mm100, o3tl::Length::twip);

/// The size of one pixel in twips at 100% zoom, for the marks that keep a size in pixels.
constexpr double constPixelTwips = o3tl::convert(1.0, o3tl::Length::px, o3tl::Length::twip);

OString colorToHex(const Color& rColor) { return "#" + rColor.AsRGBHexString().toUtf8(); }

std::string_view horizontalName(SvxCellHorJustify eHorizontal)
{
    switch (eHorizontal)
    {
        case SvxCellHorJustify::Center:
            return "center";
        case SvxCellHorJustify::Right:
            return "right";
        case SvxCellHorJustify::Block:
            return "block";
        default:
            return "left";
    }
}

std::string_view verticalName(SvxCellVerJustify eVertical)
{
    switch (eVertical)
    {
        case SvxCellVerJustify::Top:
            return "top";
        case SvxCellVerJustify::Center:
            return "center";
        case SvxCellVerJustify::Block:
            return "block";
        default:
            return "bottom";
    }
}

std::string_view markKindName(sc::CellMarkKind eKind)
{
    switch (eKind)
    {
        case sc::CellMarkKind::Formula:
            return "formula";
        case sc::CellMarkKind::FormulaError:
            return "formulaError";
        case sc::CellMarkKind::ClipLeft:
            return "clipLeft";
        case sc::CellMarkKind::ClipRight:
            return "clipRight";
        case sc::CellMarkKind::AutoFilter:
            return "autoFilter";
        case sc::CellMarkKind::PivotButton:
            return "pivotButton";
    }
    return "unknown";
}

/// Write the type and fields of a Calc cell primitive, with its ranges as positions relative to
/// the cells. Its decomposition follows as children.
bool writeCalcPrimitive(const primitive2d::BasePrimitive2D& rPrimitive, tools::JsonWriter& rWriter,
                        sc::CellPositionJsonProcessor& rCells)
{
    auto writeCalcRange = [&rWriter, &rCells](std::string_view aName,
                                              const basegfx::B2DRange& rRange)
    { rCells.writeCellRange(rWriter, aName, rRange); };
    switch (rPrimitive.getPrimitive2DID())
    {
        case PRIMITIVE2D_ID_SC_CELLTEXTPRIMITIVE2D:
        {
            const auto& rText = static_cast<const sc::CellTextPrimitive2D&>(rPrimitive);
            rWriter.put("type", "calcCellText");
            // The text portions are written with the fill of the cell, which a client chooses an
            // automatic font color against.
            rCells.setChildTextSurface(rText.getSurface());
            rWriter.put("column", sal_Int32(rText.getColumn()));
            rWriter.put("row", sal_Int32(rText.getRow()));
            if (rText.isClip())
                writeCalcRange("clip", rText.getClipRange());
            // Left and bottom alignment are left out, and so are empty flags.
            const std::string_view aHorizontal = horizontalName(rText.getHorizontal());
            if (aHorizontal != "left")
                rWriter.put("horizontal", aHorizontal);
            const std::string_view aVertical = verticalName(rText.getVertical());
            if (aVertical != "bottom")
                rWriter.put("vertical", aVertical);

            const sc::CellTextFlags eFlags = rText.getFlags();
            if (eFlags == sc::CellTextFlags::NONE)
                return true;
            auto aFlags = rWriter.startArray("flags");
            const std::pair<sc::CellTextFlags, std::string_view> aNames[]
                = { { sc::CellTextFlags::EditEngine, "editEngine" },
                    { sc::CellTextFlags::HashFill, "hashFill" },
                    { sc::CellTextFlags::Shrink, "shrink" },
                    { sc::CellTextFlags::Repeat, "repeat" },
                    { sc::CellTextFlags::ClippedLeft, "clippedLeft" },
                    { sc::CellTextFlags::ClippedRight, "clippedRight" },
                    { sc::CellTextFlags::ClippedBottom, "clippedBottom" },
                    { sc::CellTextFlags::Wrap, "wrap" } };
            for (const auto& [eFlag, aName] : aNames)
            {
                if (eFlags & eFlag)
                    rWriter.putSimpleValue(OUString::fromUtf8(aName));
            }
            return true;
        }
        case PRIMITIVE2D_ID_SC_CELLMARKPRIMITIVE2D:
        {
            const auto& rMark = static_cast<const sc::CellMarkPrimitive2D&>(rPrimitive);
            rWriter.put("type", "calcCellMark");
            rWriter.put("kind", markKindName(rMark.getKind()));
            rWriter.put("column", sal_Int32(rMark.getColumn()));
            rWriter.put("row", sal_Int32(rMark.getRow()));
            writeCalcRange("cell", rMark.getCellRange());
            rWriter.put("color", colorToHex(rMark.getColor()));
            if (rMark.isActive())
                rWriter.put("active", true);
            return true;
        }
        case PRIMITIVE2D_ID_SC_GRIDCOVERPRIMITIVE2D:
        {
            const auto& rCover = static_cast<const sc::GridCoverPrimitive2D&>(rPrimitive);
            rWriter.put("type", "calcGridCover");
            auto aAreas = rWriter.startArray("areas");
            for (const sc::GridCoverPrimitive2D::Area& rArea : rCover.getAreas())
            {
                auto aArea = rWriter.startStruct();
                rWriter.put("startColumn", sal_Int32(rArea.mnStartColumn));
                rWriter.put("startRow", sal_Int32(rArea.mnStartRow));
                rWriter.put("endColumn", sal_Int32(rArea.mnEndColumn));
                rWriter.put("endRow", sal_Int32(rArea.mnEndRow));
                writeCalcRange("rect", rArea.maRange);
            }
            return true;
        }
        case PRIMITIVE2D_ID_SC_SPARKLINEPRIMITIVE2D:
        {
            const auto& rSparkline = static_cast<const sc::SparklinePrimitive2D&>(rPrimitive);
            rWriter.put("type", "calcSparkline");
            rWriter.put("column", sal_Int32(rSparkline.getColumn()));
            rWriter.put("row", sal_Int32(rSparkline.getRow()));
            writeCalcRange("cell", rSparkline.getCellRange());
            rWriter.put("group", rSparkline.getGroupId());
            return true;
        }
        default:
            return false;
    }
}

std::string_view layerName(SdrLayerID nLayer)
{
    if (nLayer == SC_LAYER_BACK)
        return "back";
    if (nLayer == SC_LAYER_INTERN)
        return "internal";
    return "front";
}

/// The object and the objects inside it, in the order they paint.
void collectObjects(SdrObject& rObject, std::vector<SdrObject*>& rObjects)
{
    rObjects.push_back(&rObject);
    if (const SdrObjList* pChildren = rObject.GetSubList())
    {
        for (const rtl::Reference<SdrObject>& pChild : *pChildren)
            collectObjects(*pChild, rObjects);
    }
}

/// Whether the object or one of its members, at any depth, has an id in rIds.
bool anyMemberNamed(const SdrObject& rObject, const std::set<sal_uInt64>& rIds)
{
    if (rIds.contains(rObject.GetUniqueID()))
        return true;
    if (const SdrObjList* pChildren = rObject.GetSubList())
    {
        for (const rtl::Reference<SdrObject>& pChild : *pChildren)
        {
            if (anyMemberNamed(*pChild, rIds))
                return true;
        }
    }
    return false;
}

/// Whether the object, a group it sits in, or one of its members has an id in rIds. A group draws
/// its members, and the handles of a group follow its members.
bool touchesAny(const SdrObject& rObject, const std::set<sal_uInt64>& rIds)
{
    if (anyMemberNamed(rObject, rIds))
        return true;
    for (const SdrObject* pParent = rObject.getParentSdrObjectFromSdrObject(); pParent;
         pParent = pParent->getParentSdrObjectFromSdrObject())
    {
        if (rIds.contains(pParent->GetUniqueID()))
            return true;
    }
    return false;
}

/// The chart object with this model on the page, or nullptr.
const SdrObject* chartObjectOf(const SdrPage& rPage,
                               const cpo::uno::Reference<cpo::uno::XInterface>& rChart)
{
    for (const rtl::Reference<SdrObject>& pObject : rPage)
    {
        auto* pOle = dynamic_cast<SdrOle2Obj*>(pObject.get());
        if (pOle && pOle->IsChart() && pOle->getXModel() == rChart)
            return pOle;
    }
    return nullptr;
}

/// Move every position of the content by the given twips. The glue points and handle rails are
/// on the object's own square and stay. The primitives are in 1/100 mm and move inside one
/// transformation.
void moveContent(svx::vector::ObjectContent& rContent, const Point& rByTwips)
{
    rContent.maPaintedBox.Move(rByTwips.X(), rByTwips.Y());
    const basegfx::B2DHomMatrix aMove
        = basegfx::utils::createTranslateB2DHomMatrix(rByTwips.X(), rByTwips.Y());
    rContent.maTransformation = aMove * rContent.maTransformation;
    rContent.maPath.transform(aMove);
    for (svx::vector::ObjectContent::Handle& rHandle : rContent.maHandles)
        rHandle.maPosition += rByTwips;
    if (rContent.moRouting)
    {
        const basegfx::B2DPoint aBy(rByTwips.X(), rByTwips.Y());
        for (svx::vector::ObjectContent::ConnectorTie* pTie :
             { &rContent.moRouting->maTieAtStart, &rContent.moRouting->maTieAtEnd })
        {
            if (pTie->moAt)
                *pTie->moAt += aBy;
        }
    }

    const basegfx::B2DHomMatrix aMoveMm100 = basegfx::utils::createTranslateB2DHomMatrix(
        o3tl::convert(double(rByTwips.X()), o3tl::Length::twip, o3tl::Length::mm100),
        o3tl::convert(double(rByTwips.Y()), o3tl::Length::twip, o3tl::Length::mm100));
    for (drawinglayer::primitive2d::Primitive2DContainer* pPrimitives :
         { &rContent.maPrimitives, &rContent.maAids })
    {
        if (pPrimitives->empty())
            continue;
        drawinglayer::primitive2d::Primitive2DContainer aMoved;
        aMoved.push_back(new drawinglayer::primitive2d::TransformPrimitive2D(
            aMoveMm100, std::move(*pPrimitives)));
        *pPrimitives = std::move(aMoved);
    }
}

/// The sheet that holds the chart with this model, or nothing when no sheet holds it.
std::optional<SCTAB> sheetOfChart(const ScDrawLayer& rDrawLayer,
                                  const cpo::uno::Reference<cpo::uno::XInterface>& rChart)
{
    for (sal_uInt16 nPage = 0; nPage < rDrawLayer.GetPageCount(); ++nPage)
    {
        const SdrPage* pPage = rDrawLayer.GetPage(nPage);
        if (!pPage)
            continue;
        for (const rtl::Reference<SdrObject>& pObject : *pPage)
        {
            auto* pOle = dynamic_cast<SdrOle2Obj*>(pObject.get());
            if (pOle && pOle->IsChart() && pOle->getXModel() == rChart)
                return SCTAB(nPage);
        }
    }
    return std::nullopt;
}

}

namespace sc
{
/// Reports chart changes. A chart changes with its cells, without a hint of the drawing layer.
class ChartModifyListener final : public cppu::WeakImplHelper<css::util::XModifyListener>
{
public:
    explicit ChartModifyListener(VectorContent& rContent)
        : mpContent(&rContent)
    {
    }

    /// Listen to the chart on sheet nTab. A chart that is listened to already moves to nTab.
    void listenTo(const cpo::uno::Reference<css::util::XModifyBroadcaster>& rChart, SCTAB nTab)
    {
        for (auto& [rListened, rTab] : maCharts)
        {
            if (rListened == rChart)
            {
                rTab = nTab;
                return;
            }
        }
        rChart->addModifyListener(this);
        maCharts.emplace_back(rChart, nTab);
    }

    /// Keep listening to the charts in rCharts on sheet nTab, and stop for the other charts on it.
    void keepOnly(SCTAB nTab,
                  const std::vector<cpo::uno::Reference<css::util::XModifyBroadcaster>>& rCharts)
    {
        std::erase_if(maCharts,
                      [nTab, &rCharts, this](const auto& rEntry)
                      {
                          if (rEntry.second != nTab)
                              return false;
                          if (std::find(rCharts.begin(), rCharts.end(), rEntry.first)
                              != rCharts.end())
                              return false;
                          rEntry.first->removeModifyListener(this);
                          return true;
                      });
    }

    /// Record again which sheet holds each chart, and stop listening to charts no sheet holds.
    void relocate(const ScDrawLayer& rDrawLayer)
    {
        std::erase_if(maCharts,
                      [&rDrawLayer, this](auto& rEntry)
                      {
                          const std::optional<SCTAB> oTab = sheetOfChart(
                              rDrawLayer,
                              cpo::uno::Reference<cpo::uno::XInterface>(rEntry.first.get()));
                          if (oTab)
                          {
                              rEntry.second = *oTab;
                              return false;
                          }
                          rEntry.first->removeModifyListener(this);
                          return true;
                      });
    }

    /// Stop listening to every chart. The content is not told of anything after that.
    void stop()
    {
        mpContent = nullptr;
        for (auto& [rChart, nTab] : maCharts)
            rChart->removeModifyListener(this);
        maCharts.clear();
    }

    void modified(const css::lang::EventObject& rEvent) override
    {
        if (!mpContent)
            return;
        for (const auto& [rChart, nTab] : maCharts)
        {
            if (rChart == rEvent.Source)
            {
                mpContent->chartChanged(nTab, rEvent.Source);
                return;
            }
        }
    }

    void disposing(const css::lang::EventObject& rEvent) override
    {
        std::erase_if(maCharts, [&rEvent](const auto& rEntry)
                      { return rEntry.first == rEvent.Source; });
    }

private:
    VectorContent* mpContent;
    std::vector<std::pair<cpo::uno::Reference<css::util::XModifyBroadcaster>, SCTAB>> maCharts;
};

namespace
{
/// Marks the cells of each span of a bulk broadcast as changed.
class ChangedSpans final : public ColumnSpanSet::Action
{
public:
    explicit ChangedSpans(VectorContent& rContent)
        : mrContent(rContent)
    {
    }

    void execute(const ScAddress& rPosition, SCROW nLength, bool bValue) override
    {
        if (!bValue)
            return;
        mrContent.cellsChanged(ScRange(
            rPosition, ScAddress(rPosition.Col(), rPosition.Row() + nLength - 1, rPosition.Tab())));
    }

private:
    VectorContent& mrContent;
};
}

/** Hears which cells of one sheet got new results, or get them when next read, by their address.

    The area listened to is the whole sheet. The document moves the area along when rows, columns
    or sheets are inserted or removed, and names the new range, so the area is found again when
    listening ends. Group listening gets the changed ranges of a bulk broadcast in one hint, where
    plain listening would hear of the first changed cell only.
 */
class SheetListener final : public SvtListener
{
public:
    SheetListener(VectorContent& rContent, ScDocument& rDocument, SCTAB nTab)
        : mrContent(rContent)
        , mrDocument(rDocument)
        , maRange(0, 0, nTab, rDocument.MaxCol(), rDocument.MaxRow(), nTab)
    {
        mrDocument.StartListeningArea(maRange, /*bGroupListening*/ true, this);
    }

    virtual ~SheetListener() override
    {
        if (mbAreaAlive)
            mrDocument.EndListeningArea(maRange, /*bGroupListening*/ true, this);
    }

    void Notify(const SfxHint& rHint) override
    {
        switch (rHint.GetId())
        {
            case SfxHintId::ScDataChanged:
            case SfxHintId::ScTableOpDirty:
                mrContent.cellsChanged(static_cast<const ScHint&>(rHint).GetRange());
                break;
            case SfxHintId::ScBulkData:
            {
                const ColumnSpanSet* pSpans = static_cast<const BulkDataHint&>(rHint).getSpans();
                if (!pSpans)
                    break;
                ChangedSpans aAction(mrContent);
                pSpans->executeAction(mrDocument, aAction);
                break;
            }
            case SfxHintId::ScAreaChanged:
                maRange = static_cast<const ScAreaChangedHint&>(rHint).GetRange();
                break;
            case SfxHintId::Dying:
                // The document drops the area, with this listener, when it goes away.
                mbAreaAlive = false;
                break;
            default:
                break;
        }
    }

private:
    VectorContent& mrContent;
    ScDocument& mrDocument;
    /// The range of the area as the document holds it now.
    ScRange maRange;
    bool mbAreaAlive = true;
};

std::optional<SCTAB> VectorContent::sheetOfPartId(std::u16string_view aPartId) const
{
    if (aPartId.empty())
        return std::nullopt;
    const ScDocument& rDocument = mrDocShell.GetDocument();
    const sal_Int32 nTab = o3tl::toInt32(aPartId);
    if (nTab < 0 || nTab >= rDocument.GetTableCount() || !rDocument.HasTable(SCTAB(nTab)))
        return std::nullopt;
    return SCTAB(nTab);
}

ScAddress VectorContent::anchorCellOf(const ScDocument& rDocument, SCTAB nTab, SdrObject& rObject)
{
    const ScDrawObjData* pData = ScDrawLayer::GetObjDataTab(&rObject, nTab);
    if (pData && pData->maStart.IsValid())
        return ScAddress(pData->maStart.Col(), pData->maStart.Row(), nTab);
    ScDrawObjData aData;
    ScDrawLayer::GetCellAnchorFromPosition(rObject.GetSnapRect(), aData, rDocument, nTab);
    if (!aData.maStart.IsValid())
        return ScAddress(0, 0, nTab);
    return ScAddress(aData.maStart.Col(), aData.maStart.Row(), nTab);
}

Point VectorContent::cellStartTwips(const ScDocument& rDocument, SCTAB nTab, const ScAddress& rCell)
{
    if (rDocument.IsNegativePage(nTab))
    {
        const tools::Rectangle aCell = ScDrawLayer::GetCellRect(rDocument, rCell, false);
        return o3tl::convert(aCell.TopLeft(), o3tl::Length::mm100, o3tl::Length::twip);
    }
    const tools::Long nX = rCell.Col() > 0 ? rDocument.GetColWidth(0, rCell.Col() - 1, nTab) : 0;
    const tools::Long nY = rCell.Row() > 0 ? rDocument.GetRowHeight(0, rCell.Row() - 1, nTab) : 0;
    return Point(nX, nY);
}

ScRange VectorContent::cellsOf(const ScDocument& rDocument, SCTAB nTab,
                               const tools::Rectangle& rBoxTwips, const ScAddress& rAnchor)
{
    if (rBoxTwips.IsEmpty())
        return ScRange(rAnchor);
    const tools::Rectangle aBox = o3tl::convert(rBoxTwips, o3tl::Length::twip, o3tl::Length::mm100);
    return rDocument.GetRange(nTab, aBox, /*bHiddenAsZero*/ true);
}

void VectorContent::writeLayers(const CellPrimitiveLayers& rLayers, SCTAB nTab,
                                SCCOL nOriginColumn, SCROW nOriginRow, OString& rContent,
                                std::vector<sal_Int32>& rUsedTextStyles)
{
    rContent = OString();
    rUsedTextStyles.clear();
    if (rLayers.isEmpty())
        return;

    tools::JsonWriter aWriter;
    CellPositionJsonProcessor aProcessor(aWriter, mrDocShell.GetDocument(), nTab, nOriginColumn,
                                         nOriginRow);
    aProcessor.setScaleFactor(1.0);
    maResources.attachTo(aProcessor);
    aProcessor.setExternalPrimitiveWriter(
        [&aProcessor](const primitive2d::BasePrimitive2D& rPrimitive, tools::JsonWriter& rWriter,
                      double /*fScale*/)
        { return writeCalcPrimitive(rPrimitive, rWriter, aProcessor); });
    aProcessor.setTextStyles(&maTextStyles);

    if (!rLayers.maInkRange.isEmpty())
        aProcessor.writeCellRange(aWriter, "ink", rLayers.maInkRange);
    {
        auto aLayersNode = aWriter.startNode("layers");
        const std::pair<std::string_view, const primitive2d::Primitive2DContainer*> aNamedLayers[]
            = { { "background", &rLayers.maBackground },
                { "borders", &rLayers.maBorders },
                { "text", &rLayers.maText },
                { "extras", &rLayers.maExtras },
                { "marks", &rLayers.maMarks },
                { "gridCover", &rLayers.maGridCover } };
        for (const auto& [aName, pLayer] : aNamedLayers)
        {
            if (pLayer->empty())
                continue;
            auto aArray = aWriter.startArray(aName);
            aProcessor.decomposeAndWrite(*pLayer);
        }
    }
    rContent = aWriter.finishAndGetAsOString();
    const std::set<sal_Int32>& rUsed = aProcessor.getUsedTextStyles();
    rUsedTextStyles.assign(rUsed.begin(), rUsed.end());
}

VectorContent::VectorContent(ScDocShell& rDocShell)
    : mrDocShell(rDocShell)
    , mxChartListener(new ChartModifyListener(*this))
{
    listenToDrawLayer();
}

void VectorContent::listenToSheet(SCTAB nTab)
{
    if (maSheetListeners.contains(nTab))
        return;
    maSheetListeners.emplace(
        nTab, std::make_unique<SheetListener>(*this, mrDocShell.GetDocument(), nTab));
}

void VectorContent::listenToDrawLayer()
{
    // The drawing layer appears with the first inserted object, so it is looked for on every write.
    // Objects inserted before listening started are read again.
    ScDrawLayer* pDrawLayer = mrDocShell.GetDocument().GetDrawLayer();
    if (!pDrawLayer || IsListening(*pDrawLayer))
        return;
    StartListening(*pDrawLayer);
    for (auto& [rSheetKey, rState] : maSheets)
        rState.mbObjectsDirty = true;
}

VectorContent::~VectorContent() { mxChartListener->stop(); }

void VectorContent::getCommandValues(tools::JsonWriter& rWriter, std::string_view rCommand)
{
    svx::vector::answerPartCommand(rWriter, rCommand, maResources,
                                   [this, &rWriter](const svx::vector::PartRequest& rRequest)
                                   { writeRequestedSheet(rWriter, rRequest); });
}

void VectorContent::writeRequestedSheet(tools::JsonWriter& rWriter,
                                        const svx::vector::PartRequest& rRequest)
{
    // The content has one form, so the mode is only named back to the client.
    const sal_Int32 nMode = rRequest.moMode.value_or(constCellPositionsMode);

    const ScDocument& rDocument = mrDocShell.GetDocument();
    const std::optional<SCTAB> oTab = sheetOfPartId(OUString::fromUtf8(rRequest.maPartId));
    if (!oTab)
    {
        // A sheet that is not there gets the header alone.
        svx::vector::writeAnswerHeader(rWriter, /*bDelta*/ false, nMode, rRequest.maPartId,
                                       maResources.getEpoch());
        rWriter.put("docType", "spreadsheet");
        if (rRequest.moRequestId)
            rWriter.put("requestId", *rRequest.moRequestId);
        return;
    }

    // Blocks are "row:column", or "row:column:version" when the client holds one, comma separated.
    std::vector<BlockRequest> aBlocks;
    auto aBlocksIterator = rRequest.maParameters.find(u"blocks"_ustr);
    if (aBlocksIterator != rRequest.maParameters.end())
    {
        const std::u16string_view aList(aBlocksIterator->second);
        sal_Int32 nIndex = 0;
        do
        {
            const std::u16string_view aBlock = o3tl::getToken(aList, 0, ',', nIndex);
            if (aBlock.empty())
                continue;
            sal_Int32 nPart = 0;
            const sal_Int32 nBlockRow = o3tl::toInt32(o3tl::getToken(aBlock, 0, ':', nPart));
            if (nPart < 0)
                continue;
            const sal_Int32 nBlockColumn = o3tl::toInt32(o3tl::getToken(aBlock, 0, ':', nPart));
            if (nBlockRow < 0 || nBlockColumn < 0 || nBlockRow > rDocument.MaxRow() / constBlockRows
                || nBlockColumn > rDocument.MaxCol() / constBlockColumns)
                continue;
            BlockRequest aRequest{ { nBlockRow, nBlockColumn }, std::nullopt };
            if (nPart >= 0)
                aRequest.moHeldVersion = OUString(o3tl::getToken(aBlock, 0, ':', nPart)).toUInt64();
            aBlocks.push_back(aRequest);
        } while (nIndex >= 0);
    }

    // A held block version of another epoch, or of no named epoch, says nothing about the sheet.
    if (!rRequest.mbSameEpoch)
    {
        for (BlockRequest& rBlock : aBlocks)
            rBlock.moHeldVersion.reset();
    }

    // A client can name its request, and the answer names it back.
    writeVectorPrimitives(rWriter, sheetState(*oTab), nMode, aBlocks, rRequest.mnSinceVersion,
                          /*bPush*/ false, rRequest.moRequestId);
}

void VectorContent::pushDelta(tools::JsonWriter& rWriter, std::string_view rPartId, sal_Int32 nMode)
{
    const std::optional<SCTAB> oTab = sheetOfPartId(OUString::fromUtf8(rPartId));
    if (!oTab)
        return;

    auto aIterator = maSheets.find(*oTab);
    if (aIterator == maSheets.end())
    {
        // A new epoch dropped the sheet state. Only the header is written, naming the new epoch.
        svx::vector::writeAnswerHeader(rWriter, /*bDelta*/ true, nMode, OString::number(*oTab),
                                       maResources.getEpoch());
        rWriter.put("docType", "spreadsheet");
        return;
    }
    // A sheet that nobody asked for has nothing to push.
    if (!aIterator->second.maVersions.isServed())
        return;

    writeVectorPrimitives(rWriter, aIterator->second, nMode, {}, /*nSinceVersion*/ -1,
                          /*bPush*/ true);
}

VectorContent::SheetState& VectorContent::sheetState(SCTAB nTab)
{
    SheetState& rState = maSheets[nTab];
    rState.mnTab = nTab;
    listenToSheet(nTab);
    return rState;
}

void VectorContent::writeVectorPrimitives(tools::JsonWriter& rWriter, SheetState& rState,
                                          sal_Int32 nMode,
                                          const std::vector<BlockRequest>& rBlocks,
                                          sal_Int64 nSinceVersion, bool bPush,
                                          std::optional<sal_Int64> oRequestId)
{
    // Drawing object text keeps automatic colors, and a running text edit ignores the measurement
    // made here for its selection.
    comphelper::COKit::setVectorRendering(true);
    comphelper::ScopeGuard aVectorRenderingGuard([]
                                                 { comphelper::COKit::setVectorRendering(false); });

    // A sheet change during the write resets the states after it, so the answer is built from one
    // state and carries its epoch.
    mbWriting = true;
    comphelper::ScopeGuard aWritingGuard(
        [this]
        {
            mbWriting = false;
            if (mbResetSheetsPending)
                resetSheets();
        });

    listenToDrawLayer();
    // A shape text edit then reports every keystroke, which the entry for the edit draws.
    if (ScDrawLayer* pDrawLayer = mrDocShell.GetDocument().GetDrawLayer())
        pDrawLayer->SetDrawnFromModel(true);
    const SCTAB nTab = rState.mnTab;
    nSinceVersion = rState.maVersions.stepFrom(nSinceVersion, bPush);
    const bool bDelta = nSinceVersion >= 0;

    // Hints only mark what may differ. Comparing the rewritten content decides on a new version.
    resolveDirtyBlocks(rState);
    for (const BlockRequest& rRequest : rBlocks)
        resolveBlock(rState, rRequest.maKey);
    // A whole answer compares every object again, which repairs changes that came without a hint.
    if (!bDelta && !bPush)
        rState.mbObjectsDirty = true;
    resolveObjects(rState);
    resolveSheet(rState);
    const OString aTextEdits = writeTextEdits(nTab);

    const sal_uInt64 nSince = bDelta ? sal_uInt64(nSinceVersion) : 0;
    auto isNew = [&](sal_uInt64 nVersion) { return !bDelta || nVersion > nSince; };
    // A push carries the blocks changed since the last one from their content, which a block
    // built by a request in between may have dropped since.
    if (bPush)
    {
        for (auto& [rKey, rBlock] : rState.maBlocks)
        {
            if (rBlock.mnVersion > std::max(nSince, rBlock.mnCreatedVersion)
                && !rBlock.mbContentHeld)
                resolveBlock(rState, rKey);
        }
    }

    // A push also names the blocks that became stale since the last one.
    const bool bNewStale = std::any_of(rState.maBlocks.begin(), rState.maBlocks.end(),
                                       [](const auto& rEntry)
                                       { return rEntry.second.mbDirty && !rEntry.second.mbStaleSent; });
    if (bPush && !rState.maVersions.isChangedSince(nSinceVersion)
        && aTextEdits == rState.maLastTextEdits && !rState.mbTextEditChanged && !bNewStale
        && rState.maShifts.empty())
        return;

    svx::vector::writeAnswerHeader(rWriter, bDelta, nMode, OString::number(nTab),
                                   maResources.getEpoch());
    rWriter.put("docType", "spreadsheet");
    if (oRequestId)
        rWriter.put("requestId", *oRequestId);
    if (!bPush)
    {
        // The blocks asked for, so the client knows which it has current even when it forgot the
        // request before the answer came.
        auto aAsked = rWriter.startArray("blocks");
        for (const BlockRequest& rRequest : rBlocks)
            rWriter.putSimpleValue(rtl::Concat2View(OUString::number(rRequest.maKey.first) + ":"
                                                    + OUString::number(rRequest.maKey.second)));
    }
    svx::vector::writeAnswerVersions(rWriter, nSinceVersion, rState.maVersions.getVersion());
    rWriter.put("blockRows", sal_Int32(constBlockRows));
    rWriter.put("blockColumns", sal_Int32(constBlockColumns));

    if (isNew(rState.maSheet.mnVersion))
        rWriter.putRaw(Concat2View("\"sheet\": " + rState.maSheet.maContent));

    // The text styles that the cell text written here names.
    std::set<sal_Int32> aTextStyles;
    {
        auto aObjects = rWriter.startArray("objects");
        if (bPush)
        {
            // A push carries each block with a part changed after the last push. A client holds a
            // block from its first write or later, so parts of that write are left out, and a block
            // built since the last push is not written.
            for (const auto& [rKey, rBlock] : rState.maBlocks)
            {
                const sal_uInt64 nFrom = std::max(nSince, rBlock.mnCreatedVersion);
                if (rBlock.mnVersion > nFrom)
                    writeBlockEntry(rWriter, rState, rKey, rBlock, nFrom, aTextStyles);
            }
        }
        else
        {
            // A pull carries the requested blocks, apart from those the client has up to date.
            for (const BlockRequest& rRequest : rBlocks)
            {
                auto aIterator = rState.maBlocks.find(rRequest.maKey);
                if (aIterator == rState.maBlocks.end())
                    continue;
                if (rRequest.moHeldVersion
                    && *rRequest.moHeldVersion >= aIterator->second.mnVersion)
                    continue;
                writeBlockEntry(rWriter, rState, rRequest.maKey, aIterator->second,
                                rRequest.moHeldVersion.value_or(0), aTextStyles);
            }
        }
    }

    if (!aTextStyles.empty())
    {
        auto aStyles = rWriter.startNode("textStyles");
        for (const sal_Int32 nStyle : aTextStyles)
            rWriter.putRaw(Concat2View("\"" + OString::number(nStyle)
                                       + "\": " + maTextStyles.fieldsOf(nStyle)));
    }

    // A push names the held blocks that were not built again since the last push, once. A client
    // asks for the ones it shows, and does not take the versions of the pushes for them.
    if (bPush)
    {
        writeStaleBlocks(rWriter, rState);
        for (auto& [rKey, rBlock] : rState.maBlocks)
        {
            if (rBlock.mbDirty)
                rBlock.mbStaleSent = true;
        }
    }

    // A push tells the rows and columns moved since the last one, once.
    if (bPush && !rState.maShifts.empty())
    {
        auto aShifts = rWriter.startArray("shifts");
        for (const Shift& rShift : rState.maShifts)
        {
            auto aShift = rWriter.startStruct();
            rWriter.put("axis", rShift.mbRows ? "rows" : "columns");
            rWriter.put("at", sal_Int32(rShift.mnAt));
            rWriter.put("by", sal_Int32(rShift.mnBy));
            auto aAcross = rWriter.startArray("across");
            rWriter.putSimpleValue(sal_Int32(rShift.mnAcrossFirst));
            rWriter.putSimpleValue(sal_Int32(rShift.mnAcrossLast));
        }
        rState.maShifts.clear();
    }

    if (isNew(rState.maObjectBlocks.mnVersion))
        rWriter.putRaw(Concat2View("\"objectBlocks\": " + rState.maObjectBlocks.maContent));

    if (isNew(rState.maVersions.getOrderVersion()))
    {
        auto aOrder = rWriter.startArray("drawOrder");
        for (const sal_uInt64 nId : rState.maObjectOrder)
            rWriter.putSimpleValue(sal_Int64(nId));
    }

    // The text edits are always sent whole, so they list only the edits that are open.
    rWriter.putRaw(Concat2View("\"textEdits\": " + aTextEdits));

    // The text edits the readers hold go with the mark. The first pull sets the version pushes
    // count from, and no client holds an older one.
    if (bPush || !rState.maVersions.isServed())
        rState.maLastTextEdits = aTextEdits;
    if (bPush)
        rState.mbTextEditChanged = false;
    rState.maVersions.noteWritten(bPush);
    dropOldContent(rState);
}

void VectorContent::writeBlockEntry(tools::JsonWriter& rWriter, const SheetState& rState,
                                    const BlockKey& rKey, const BlockState& rBlock,
                                    sal_uInt64 nFrom, std::set<sal_Int32>& rTextStyles)
{
    const sal_Int32 nStartRow = rKey.first * constBlockRows;
    auto aEntry = rWriter.startStruct();
    rWriter.put("kind", "cells");
    rWriter.put("blockRow", rKey.first);
    rWriter.put("blockColumn", rKey.second);
    rWriter.put("startRow", nStartRow);
    rWriter.put("startColumn", sal_Int32(rKey.second * constBlockColumns));
    rWriter.put("endRow", sal_Int32((rKey.first + 1) * constBlockRows - 1));
    rWriter.put("endColumn", sal_Int32((rKey.second + 1) * constBlockColumns - 1));
    rWriter.put("version", sal_Int64(rBlock.mnVersion));
    rWriter.put("from", sal_Int64(nFrom));
    rWriter.putRaw(Concat2View("\"bounds\": " + rBlock.maBounds));

    // Parts changed after nFrom are written. An emptied part is written empty, except at nFrom 0,
    // which names no earlier content of the block.
    auto isWritten = [nFrom](const EntryState& rPart)
    { return rPart.mnVersion > nFrom && (nFrom > 0 || !rPart.maContent.isEmpty()); };
    auto writePart = [&rWriter, &rTextStyles](const EntryState& rPart)
    {
        rTextStyles.insert(rPart.maTextStyles.begin(), rPart.maTextStyles.end());
        rWriter.put("version", sal_Int64(rPart.mnVersion));
        rWriter.putRaw(Concat2View("\"content\": "
                                   + (rPart.maContent.isEmpty() ? "{}"_ostr : rPart.maContent)));
    };

    if (isWritten(rBlock.maBorders))
    {
        auto aBorders = rWriter.startNode("borders");
        writePart(rBlock.maBorders);
    }
    {
        auto aRows = rWriter.startArray("rows");
        for (size_t nIndex = 0; nIndex < rBlock.maRows.size(); ++nIndex)
        {
            const EntryState& rRow = rBlock.maRows[nIndex];
            if (!isWritten(rRow))
                continue;
            auto aRow = rWriter.startStruct();
            rWriter.put("row", sal_Int32(nStartRow + nIndex));
            writePart(rRow);
        }
    }

    // The objects anchored in the block that changed after nFrom, in the draw order, and the
    // objects that left the block after it. From 0 nothing was held, so nothing has left.
    {
        auto aObjects = rWriter.startArray("objects");
        for (const sal_uInt64 nId : rState.maObjectOrder)
        {
            auto aIterator = rState.maObjects.find(nId);
            if (aIterator == rState.maObjects.end() || aIterator->second.maBlock != rKey
                || aIterator->second.maEntry.mnVersion <= nFrom)
                continue;
            writeObjectEntry(rWriter, nId, aIterator->second.maEntry);
        }
    }
    if (nFrom > 0)
    {
        auto aRemoved = rWriter.startArray("removed");
        for (auto aIterator = rState.maRemovedObjects.lower_bound({ rKey, 0 });
             aIterator != rState.maRemovedObjects.end() && aIterator->first.first == rKey;
             ++aIterator)
        {
            if (aIterator->second > nFrom)
                rWriter.putSimpleValue(sal_Int64(aIterator->first.second));
        }
    }
}

void VectorContent::writeObjectEntry(tools::JsonWriter& rWriter, sal_uInt64 nId,
                                     const EntryState& rEntry)
{
    auto aEntry = rWriter.startStruct();
    rWriter.put("kind", "drawing");
    rWriter.put("id", sal_Int64(nId));
    rWriter.put("version", sal_Int64(rEntry.mnVersion));
    rWriter.putRaw(Concat2View("\"content\": " + rEntry.maContent));
}

SCROW VectorContent::rowAt(const ScDocument& rDocument, SCTAB nTab, tools::Long nTwips)
{
    const SCROW nRow = rDocument.GetRowForHeight(nTab, nTwips);
    return nRow < 0 ? rDocument.MaxRow() : nRow;
}

SCCOL VectorContent::columnAt(const ScDocument& rDocument, SCTAB nTab, tools::Long nTwips)
{
    tools::Long nEnd = 0;
    for (SCCOL nColumn = 0; nColumn <= rDocument.MaxCol(); ++nColumn)
    {
        nEnd += rDocument.GetColWidth(nColumn, nTab, /*bHiddenAsZero*/ true);
        if (nEnd > nTwips)
            return nColumn;
    }
    return rDocument.MaxCol();
}

std::vector<VectorContent::BlockRange> VectorContent::shownBlocks(SCTAB nTab) const
{
    std::vector<BlockRange> aShown;
    const ScDocument& rDocument = mrDocShell.GetDocument();
    for (SfxViewShell* pShell = SfxViewShell::GetFirst(false); pShell;
         pShell = SfxViewShell::GetNext(*pShell, false))
    {
        if (pShell->GetObjectShell() != &mrDocShell)
            continue;
        const auto* pTabViewShell = dynamic_cast<const ScTabViewShell*>(pShell);
        if (!pTabViewShell)
            continue;
        const ScViewData& rViewData = pTabViewShell->GetViewData();
        const tools::Rectangle& rArea = rViewData.getKitVisibleArea();
        if (rViewData.CurrentTabForData() != nTab || rArea.IsEmpty())
            continue;
        aShown.push_back({ rowAt(rDocument, nTab, rArea.Top()) / constBlockRows,
                           rowAt(rDocument, nTab, rArea.Bottom()) / constBlockRows,
                           columnAt(rDocument, nTab, rArea.Left()) / constBlockColumns,
                           columnAt(rDocument, nTab, rArea.Right()) / constBlockColumns });
    }
    return aShown;
}

void VectorContent::resolveDirtyBlocks(SheetState& rState)
{
    // The blocks already named stale wait for the clients to ask for them, so they do not count
    // against the ones changed since.
    std::vector<BlockKey> aDirty;
    for (const auto& [rKey, rBlock] : rState.maBlocks)
    {
        if (rBlock.mbDirty && !rBlock.mbStaleSent)
            aDirty.push_back(rKey);
    }
    // Of many changed blocks only the ones a view shows are built. The others wait for the
    // clients, which ask for them as their views come to show them.
    if (aDirty.size() > constEagerBlocks)
    {
        const std::vector<BlockRange> aShown = shownBlocks(rState.mnTab);
        std::erase_if(aDirty,
                      [&aShown](const BlockKey& rKey)
                      {
                          return std::none_of(aShown.begin(), aShown.end(),
                                              [&rKey](const BlockRange& rRange)
                                              { return rRange.holds(rKey); });
                      });
    }
    for (const BlockKey& rKey : aDirty)
        resolveBlock(rState, rKey);
}

void VectorContent::writeStaleBlocks(tools::JsonWriter& rWriter, const SheetState& rState)
{
    auto aStale = rWriter.startArray("stale");
    for (const auto& [rKey, rBlock] : rState.maBlocks)
    {
        if (rBlock.mbDirty && !rBlock.mbStaleSent)
            rWriter.putSimpleValue(
                rtl::Concat2View(OUString::number(rKey.first) + ":" + OUString::number(rKey.second)));
    }
}

sal_uInt64 VectorContent::contentHash(std::string_view rContent)
{
    if (rContent.empty())
        return 0;
    return std::hash<std::string_view>{}(rContent);
}

void VectorContent::dropOldContent(SheetState& rState)
{
    std::vector<std::pair<sal_uInt64, BlockState*>> aHeld;
    for (auto& [rKey, rBlock] : rState.maBlocks)
    {
        if (rBlock.mbContentHeld)
            aHeld.emplace_back(rBlock.mnLastUse, &rBlock);
    }
    if (aHeld.size() <= constHeldContentBlocks)
        return;
    std::sort(aHeld.begin(), aHeld.end(),
              [](const auto& rFirst, const auto& rSecond) { return rFirst.first < rSecond.first; });
    for (size_t nIndex = 0; nIndex < aHeld.size() - constHeldContentBlocks; ++nIndex)
    {
        BlockState& rBlock = *aHeld[nIndex].second;
        for (EntryState& rRow : rBlock.maRows)
            rRow.maContent = OString();
        rBlock.maBorders.maContent = OString();
        rBlock.mbContentHeld = false;
    }
}

void VectorContent::resolveBlock(SheetState& rState, const BlockKey& rKey)
{
    auto aIterator = rState.maBlocks.find(rKey);
    if (aIterator != rState.maBlocks.end() && !aIterator->second.mbDirty
        && aIterator->second.mbContentHeld)
    {
        aIterator->second.mnLastUse = ++rState.mnUseCount;
        return;
    }

    // Clear the mark before the build, so a hint during the write marks the block again. A block
    // written before is built again in the rows that may have changed, a new one in all, and so
    // is a block whose content was dropped, to hold it again.
    BlockState& rBlock = rState.maBlocks[rKey];
    const std::optional<std::pair<SCROW, SCROW>> oRows
        = rBlock.mnVersion != 0 && rBlock.mbContentHeld ? rBlock.moDirtyRows : std::nullopt;
    rBlock.mbDirty = false;
    rBlock.moDirtyRows.reset();
    BlockContent aContent = writeBlockContent(rState, rKey, oRows);

    // The content tells whether a part changed, or its hash where the content was dropped.
    auto isChanged = [&rBlock](const EntryState& rOld, const EntryState& rNew)
    {
        if (rBlock.mbContentHeld)
            return rOld.maContent != rNew.maContent;
        return rOld.mnHash != rNew.mnHash;
    };
    rBlock.maRows.resize(aContent.maRows.size());
    std::vector<size_t> aChangedRows;
    for (size_t nIndex = aContent.maBuiltRows.first; nIndex < aContent.maBuiltRows.second; ++nIndex)
    {
        if (isChanged(rBlock.maRows[nIndex], aContent.maRows[nIndex]))
            aChangedRows.push_back(nIndex);
    }
    const bool bBordersChanged = isChanged(rBlock.maBorders, aContent.maBorders);

    // The built parts hold their content, with the versions they had.
    auto takeContent = [](EntryState& rPart, EntryState& rNew)
    {
        rPart.maContent = std::move(rNew.maContent);
        rPart.mnHash = rNew.mnHash;
        rPart.maTextStyles = std::move(rNew.maTextStyles);
    };
    for (size_t nIndex = aContent.maBuiltRows.first; nIndex < aContent.maBuiltRows.second; ++nIndex)
        takeContent(rBlock.maRows[nIndex], aContent.maRows[nIndex]);
    takeContent(rBlock.maBorders, aContent.maBorders);
    rBlock.mbContentHeld = true;
    rBlock.mnLastUse = ++rState.mnUseCount;

    // Changed parts take the next version. A new empty block takes it too, so a client can hold it.
    if (rBlock.mnVersion != 0 && aChangedRows.empty() && !bBordersChanged
        && rBlock.maBounds == aContent.maBounds)
        return;

    const sal_uInt64 nVersion = rState.maVersions.countUp();
    if (rBlock.mnVersion == 0)
        rBlock.mnCreatedVersion = nVersion;
    for (size_t nIndex : aChangedRows)
        rBlock.maRows[nIndex].mnVersion = nVersion;
    if (bBordersChanged)
        rBlock.maBorders.mnVersion = nVersion;
    rBlock.maBounds = std::move(aContent.maBounds);
    rBlock.mnVersion = nVersion;
}

VectorContent::BlockContent
VectorContent::writeBlockContent(const SheetState& rState, const BlockKey& rKey,
                                 const std::optional<std::pair<SCROW, SCROW>>& roRows)
{
    ScDocument& rDocument = mrDocShell.GetDocument();
    const SCTAB nTab = rState.mnTab;
    const SCROW nStartRow = rKey.first * constBlockRows;
    const SCCOL nStartColumn = rKey.second * constBlockColumns;
    const SCROW nEndRow = std::min<SCROW>(nStartRow + constBlockRows - 1, rDocument.MaxRow());
    const SCCOL nEndColumn
        = std::min<SCCOL>(nStartColumn + constBlockColumns - 1, rDocument.MaxCol());
    SCROW nFirstContentRow = nStartRow;
    SCROW nLastContentRow = nEndRow;
    if (roRows)
    {
        nFirstContentRow = std::clamp(roRows->first, nStartRow, nEndRow);
        nLastContentRow = std::clamp(roRows->second, nFirstContentRow, nEndRow);
    }

    const ScViewOptions& rOptions = rDocument.GetViewOptions();
    const svtools::ColorConfig& rColors = ScModule::get()->GetColorConfig();
    CellPrimitiveSettings aSettings;
    aSettings.mbShowFormulas = rOptions.GetOption(sc::ViewOption::FORMULAS);
    aSettings.mbShowNullValues = rOptions.GetOption(sc::ViewOption::NULLVALS);
    aSettings.mbFormulaMarks = rOptions.GetOption(sc::ViewOption::FORMULAS_MARKS);
    aSettings.mbFormulaErrorMarks = true;
    aSettings.mbClipMarks = rColors.GetColorValue(svtools::CALCTEXTOVERFLOW).bIsVisible;
    aSettings.mbButtons = true;
    // A block holds the text of its own cells, which can overflow into the blocks next to it.
    aSettings.mbTextFromOutside = false;
    // Every view draws the same content, so each chooses the automatic font colors for itself.
    aSettings.mbAutomaticColors = true;
    // The browser draws the content at any zoom, so a mark that keeps a size in pixels gets its
    // size at 100%.
    aSettings.mfPixelSize = constPixelTwips;
    aSettings.maDocumentColor = rColors.GetColorValue(svtools::DOCCOLOR).nColor;
    aSettings.maFontColor = rColors.GetColorValue(svtools::FONTCOLOR).nColor;
    aSettings.maClipMarkColor = rColors.GetColorValue(svtools::CALCTEXTOVERFLOW).nColor;

    const CellGeometry aGeometry(rDocument, nTab, nStartColumn, nStartRow);
    CellPrimitiveBuilder aBuilder(rDocument, nTab, aGeometry, std::move(aSettings));
    const CellPrimitiveRows aRows = aBuilder.buildRows(nStartColumn, nStartRow, nEndColumn, nEndRow,
                                                       nFirstContentRow, nLastContentRow);

    BlockContent aContent;
    // The block covers its cells, and its content may reach out of them.
    {
        tools::JsonWriter aWriter;
        const basegfx::B2DRange aBounds
            = aGeometry.getRange(nStartColumn, nStartRow, nEndColumn, nEndRow);
        sc::CellPositionJsonProcessor aCells(aWriter, rDocument, nTab, nStartColumn, nStartRow);
        aCells.writeCellRangeFields(aWriter, aBounds);
        aContent.maBounds = aWriter.finishAndGetAsOString();
    }
    // Positions relative to the cells start from the first cell of the block.
    writeLayers(aRows.maRange, nTab, nStartColumn, nStartRow, aContent.maBorders.maContent,
                aContent.maBorders.maTextStyles);
    aContent.maBorders.mnHash = contentHash(aContent.maBorders.maContent);
    aContent.maRows.resize(aRows.maRows.size());
    aContent.maBuiltRows
        = { size_t(nFirstContentRow - nStartRow), size_t(nLastContentRow - nStartRow + 1) };
    for (size_t nIndex = aContent.maBuiltRows.first; nIndex < aContent.maBuiltRows.second; ++nIndex)
    {
        EntryState& rRow = aContent.maRows[nIndex];
        writeLayers(aRows.maRows[nIndex], nTab, nStartColumn, nStartRow, rRow.maContent,
                    rRow.maTextStyles);
        rRow.mnHash = contentHash(rRow.maContent);
    }
    return aContent;
}

void VectorContent::resolveObjects(SheetState& rState)
{
    const SCTAB nTab = rState.mnTab;
    if (!rState.mbObjectsDirty && rState.maDirtyObjects.empty())
        return;
    // Clear the marks before the write, so a hint during it marks the objects again.
    const bool bAll = rState.mbObjectsDirty;
    const std::set<sal_uInt64> aDirty = std::move(rState.maDirtyObjects);
    rState.mbObjectsDirty = false;
    rState.maDirtyObjects.clear();

    struct Written
    {
        OString maContent;
        ScAddress maAnchor;
        BlockKey maBlock;
        ScRange maCells;
    };
    std::vector<sal_uInt64> aOrder;
    std::unordered_map<sal_uInt64, Written> aContents;
    std::vector<cpo::uno::Reference<css::util::XModifyBroadcaster>> aCharts;

    const ScDocument& rDocument = mrDocShell.GetDocument();
    ScDrawLayer* pDrawLayer = mrDocShell.GetDocument().GetDrawLayer();
    SdrPage* pPage = pDrawLayer ? pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab)) : nullptr;
    if (pPage)
    {
        for (const rtl::Reference<SdrObject>& pTop : *pPage)
        {
            const SdrLayerID nLayer = pTop->GetLayer();
            // Form controls are drawn by the client of their own, and hidden objects not at all.
            if (nLayer == SC_LAYER_HIDDEN || nLayer == SC_LAYER_CONTROLS || !pTop->IsVisible())
                continue;

            // Group members get their own entries after the group, naming it as their parent. A
            // group with members draws nothing itself. The members share the anchor of the group,
            // so they move together when a cell is rounded.
            const ScAddress aAnchor = anchorCellOf(rDocument, nTab, *pTop);
            const BlockKey aBlock = blockOf(aAnchor.Col(), aAnchor.Row());
            std::vector<SdrObject*> aObjects;
            collectObjects(*pTop, aObjects);

            for (SdrObject* pObject : aObjects)
            {
                const sal_uInt64 nId = pObject->GetUniqueID();
                aOrder.push_back(nId);
                // An object that is held, anchored as before and not marked keeps the content it
                // was written with.
                auto aHeld = rState.maObjects.find(nId);
                if (!bAll && aHeld != rState.maObjects.end() && aHeld->second.maAnchor == aAnchor
                    && !touchesAny(*pObject, aDirty))
                    aContents.emplace(nId, Written{ aHeld->second.maEntry.maContent, aAnchor,
                                                    aBlock, aHeld->second.maCells });
                else
                {
                    ScRange aCells;
                    OString aContent = writeObjectContent(*pObject, aAnchor, aCells);
                    aContents.emplace(nId, Written{ std::move(aContent), aAnchor, aBlock, aCells });
                }

                if (auto* pOle = dynamic_cast<SdrOle2Obj*>(pObject); pOle && pOle->IsChart())
                {
                    cpo::uno::Reference<css::util::XModifyBroadcaster> xChart(pOle->getXModel(),
                                                                              cpo::uno::UNO_QUERY);
                    if (xChart.is())
                    {
                        mxChartListener->listenTo(xChart, nTab);
                        aCharts.push_back(xChart);
                    }
                }
            }
        }
    }
    // A chart that left the sheet is no longer listened to.
    mxChartListener->keepOnly(nTab, aCharts);

    for (auto& [nId, rWritten] : aContents)
    {
        rState.maRemovedObjects.erase({ rWritten.maBlock, nId });
        auto aIterator = rState.maObjects.find(nId);
        if (aIterator != rState.maObjects.end()
            && aIterator->second.maEntry.maContent == rWritten.maContent
            && aIterator->second.maBlock == rWritten.maBlock)
        {
            // The cells painted over follow the row and column sizes, without a new version.
            aIterator->second.maCells = rWritten.maCells;
            continue;
        }
        const sal_uInt64 nVersion = rState.maVersions.countUp();
        if (aIterator != rState.maObjects.end() && aIterator->second.maBlock != rWritten.maBlock)
        {
            // The object moved to another block, and left the one it was in.
            rState.maRemovedObjects[{ aIterator->second.maBlock, nId }] = nVersion;
            bumpBlock(rState, aIterator->second.maBlock, nVersion);
        }
        ObjectState& rObject = rState.maObjects[nId];
        rObject.maEntry.maContent = std::move(rWritten.maContent);
        rObject.maEntry.mnVersion = nVersion;
        rObject.maAnchor = rWritten.maAnchor;
        rObject.maBlock = rWritten.maBlock;
        rObject.maCells = rWritten.maCells;
        bumpBlock(rState, rWritten.maBlock, nVersion);
    }

    for (auto aIterator = rState.maObjects.begin(); aIterator != rState.maObjects.end();)
    {
        if (aContents.find(aIterator->first) == aContents.end())
        {
            const sal_uInt64 nVersion = rState.maVersions.countUp();
            rState.maRemovedObjects[{ aIterator->second.maBlock, aIterator->first }] = nVersion;
            bumpBlock(rState, aIterator->second.maBlock, nVersion);
            aIterator = rState.maObjects.erase(aIterator);
        }
        else
            ++aIterator;
    }

    if (aOrder != rState.maObjectOrder || rState.maVersions.getOrderVersion() == 0)
    {
        rState.maObjectOrder = std::move(aOrder);
        rState.maVersions.countUpOrder();
    }

    // The blocks with objects, each with the cells all its objects paint over, so a client can
    // ask for the blocks whose objects reach into its view.
    std::map<BlockKey, ScRange> aBlockCells;
    for (const auto& [nId, rObject] : rState.maObjects)
    {
        auto [aIterator, bInserted] = aBlockCells.try_emplace(rObject.maBlock, rObject.maCells);
        if (!bInserted)
            aIterator->second.ExtendTo(rObject.maCells);
    }
    OString aBlocks;
    {
        tools::JsonWriter aWriter;
        {
            auto aArray = aWriter.startArray("blocks");
            for (const auto& [rKey, rCells] : aBlockCells)
            {
                auto aEntry = aWriter.startStruct();
                aWriter.put("blockRow", rKey.first);
                aWriter.put("blockColumn", rKey.second);
                aWriter.put("startColumn", sal_Int32(rCells.aStart.Col()));
                aWriter.put("startRow", sal_Int32(rCells.aStart.Row()));
                aWriter.put("endColumn", sal_Int32(rCells.aEnd.Col()));
                aWriter.put("endRow", sal_Int32(rCells.aEnd.Row()));
            }
        }
        // The array alone travels, as the value of "objectBlocks".
        const OString aDocument = aWriter.finishAndGetAsOString();
        const sal_Int32 nStart = aDocument.indexOf('[');
        const sal_Int32 nEnd = aDocument.lastIndexOf(']');
        aBlocks
            = nStart >= 0 && nEnd >= nStart ? aDocument.copy(nStart, nEnd - nStart + 1) : "[]"_ostr;
    }
    if (rState.maObjectBlocks.mnVersion == 0 || rState.maObjectBlocks.maContent != aBlocks)
    {
        rState.maObjectBlocks.maContent = std::move(aBlocks);
        rState.maObjectBlocks.mnVersion = rState.maVersions.countUp();
    }
}

void VectorContent::bumpBlock(SheetState& rState, const BlockKey& rKey, sal_uInt64 nVersion)
{
    auto aIterator = rState.maBlocks.find(rKey);
    if (aIterator != rState.maBlocks.end())
        aIterator->second.mnVersion = std::max(aIterator->second.mnVersion, nVersion);
}

OString VectorContent::writeObjectContent(SdrObject& rObject, const ScAddress& rAnchor,
                                          ScRange& rCells)
{
    tools::JsonWriter aWriter;
    Primitive2dJsonProcessor aProcessor(aWriter);
    // The drawing layer works in 1/100 mm.
    aProcessor.setScaleFactor(constTwipsPerMm100);
    maResources.attachTo(aProcessor);

    geometry::ViewInformation2D aViewInformation;
    if (SdrPage* pPage = rObject.getSdrPageFromSdrObject())
    {
        aViewInformation.setVisualizedPage(GetXDrawPageForSdrPage(pPage));
        // The shape under a text edit leaves out its text, which the entry for the edit draws.
        aViewInformation.setTextEditActive(
            anyShapeTextEditOn(static_cast<SCTAB>(pPage->GetPageNum())));
    }
    aProcessor.setViewInformation2D(aViewInformation);

    svx::vector::ObjectContent aContent;
    svx::vector::describeObject(rObject, aContent, aViewInformation);

    // The content is written from the top-left corner of the anchor cell, so a row or column
    // that changes size above or left of the object leaves the content as it is.
    const ScDocument& rDocument = mrDocShell.GetDocument();
    const SCTAB nTab = rAnchor.Tab();
    rCells = cellsOf(rDocument, nTab, aContent.maPaintedBox, rAnchor);
    const Point aAnchorTwips = cellStartTwips(rDocument, nTab, rAnchor);
    moveContent(aContent, Point(-aAnchorTwips.X(), -aAnchorTwips.Y()));

    aWriter.put("layer", layerName(rObject.GetLayer()));
    aWriter.put("anchorColumn", sal_Int32(rAnchor.Col()));
    aWriter.put("anchorRow", sal_Int32(rAnchor.Row()));
    aWriter.put("name", aContent.maName);
    // The group the object sits in, 0 for an object directly on the sheet.
    aWriter.put("parent", sal_Int64(aContent.mnParentId));
    // The object shows none of its own text, and the entry of the edit draws what was typed.
    if (aContent.mbTextEdit)
        aWriter.put("textEdit", true);
    svx::vector::writeEntryGeometry(aWriter, aContent, aProcessor);
    return aWriter.finishAndGetAsOString();
}

void VectorContent::resolveSheet(SheetState& rState)
{
    const SCTAB nTab = rState.mnTab;
    const ScDocument& rDocument = mrDocShell.GetDocument();
    const svtools::ColorConfig& rColors = ScModule::get()->GetColorConfig();

    tools::JsonWriter aWriter;
    aWriter.put("kind", "sheet");
    OUString aName;
    rDocument.GetName(nTab, aName);
    aWriter.put("name", aName);
    // A sheet laid out right to left is built left to right, and the client mirrors it.
    aWriter.put("rtl", rDocument.IsLayoutRTL(nTab));
    aWriter.put("backdrop", colorToHex(rColors.GetColorValue(svtools::DOCCOLOR).nColor));
    const Color aTabColor = rDocument.GetTabBgColor(nTab);
    if (aTabColor != COL_AUTO)
        aWriter.put("tabColor", colorToHex(aTabColor));
    OString aContent = aWriter.finishAndGetAsOString();

    if (rState.maSheet.mnVersion != 0 && rState.maSheet.maContent == aContent)
        return;
    rState.maSheet.maContent = std::move(aContent);
    rState.maSheet.mnVersion = rState.maVersions.countUp();
}

const SdrObject* VectorContent::shapeTextEditOn(ScTabViewShell& rViewShell, SCTAB nTab)
{
    const ScDrawView* pDrawView = rViewShell.GetScDrawView();
    if (!pDrawView || !pDrawView->IsTextEdit())
        return nullptr;
    const SdrObject* pEdited = pDrawView->GetTextEditObject();
    const SdrPage* pPage = pEdited ? pEdited->getSdrPageFromSdrObject() : nullptr;
    if (!pPage || pPage->GetPageNum() != nTab)
        return nullptr;
    return pEdited;
}

bool VectorContent::anyShapeTextEditOn(SCTAB nTab) const
{
    for (SfxViewShell* pShell = SfxViewShell::GetFirst(false); pShell;
         pShell = SfxViewShell::GetNext(*pShell, false))
    {
        if (pShell->GetObjectShell() != &mrDocShell)
            continue;
        if (auto* pTabViewShell = dynamic_cast<ScTabViewShell*>(pShell))
        {
            if (shapeTextEditOn(*pTabViewShell, nTab))
                return true;
        }
    }
    return false;
}

OString VectorContent::writeTextEdits(SCTAB nTab)
{
    ScDocument& rDocument = mrDocShell.GetDocument();
    tools::JsonWriter aWriter;
    {
        auto aArray = aWriter.startArray("edits");
        for (SfxViewShell* pShell = SfxViewShell::GetFirst(false); pShell;
             pShell = SfxViewShell::GetNext(*pShell, false))
        {
            if (pShell->GetObjectShell() != &mrDocShell)
                continue;
            auto* pTabViewShell = dynamic_cast<ScTabViewShell*>(pShell);
            if (!pTabViewShell)
                continue;

            // The shape hides its text while the view edits it, so this entry shows the typing.
            if (shapeTextEditOn(*pTabViewShell, nTab))
            {
                auto aEntry = aWriter.startStruct();
                Primitive2dJsonProcessor aProcessor(aWriter);
                // The drawing layer works in 1/100 mm.
                aProcessor.setScaleFactor(constTwipsPerMm100);
                maResources.attachTo(aProcessor);
                svx::vector::writeTextEditEntry(
                    aWriter, sal_Int32(pShell->GetViewShellId().get()),
                    svx::vector::textEditContentOf(*pTabViewShell->GetScDrawView(),
                                                   drawinglayer::geometry::ViewInformation2D()),
                    aProcessor);
                continue;
            }

            ScViewData& rViewData = pTabViewShell->GetViewData();
            const ScSplitPos eWhich = rViewData.GetEditActivePart();
            if (!rViewData.HasEditView(eWhich) || rViewData.GetRefTabNo() != nTab)
                continue;
            EditView* pEditView = rViewData.GetEditView(eWhich);
            ScGridWindow* pWindow = rViewData.GetActiveWin();
            if (!pEditView || !pWindow)
                continue;

            const SCCOL nStartColumn = rViewData.GetEditStartCol();
            const SCROW nStartRow = rViewData.GetEditStartRow();
            const SCCOL nEndColumn = rViewData.GetEditEndCol();
            const SCROW nEndRow = rViewData.GetEditEndRow();

            TextHierarchyBreakup aHelper;
            pEditView->getEditEngine().StripPortions(aHelper);
            primitive2d::Primitive2DContainer aContent(aHelper.getTextPortionPrimitives());

            // The edit view lays out in 1/100 mm, the logic units of its window, and its output
            // area is in the logic mode with the origin at zero. Placing the text at the output
            // area relative to the cell corner keeps it in the cell however cells are rounded.
            const MapMode& rLogicMode = rViewData.GetLogicMode();
            const Point aCellPixel = rViewData.GetScrPos(nStartColumn, nStartRow, eWhich, true);
            const Point aCellLogic = pWindow->PixelToLogic(aCellPixel, rLogicMode);
            const tools::Rectangle aOutputArea = pEditView->GetOutputArea();
            const tools::Rectangle aVisibleArea = pEditView->GetVisArea();
            const double fOffsetX = aOutputArea.Left() - aCellLogic.X() - aVisibleArea.Left();
            const double fOffsetY = aOutputArea.Top() - aCellLogic.Y() - aVisibleArea.Top();

            primitive2d::Primitive2DContainer aPlaced;
            if (!aContent.empty())
            {
                basegfx::B2DHomMatrix aTransform
                    = basegfx::utils::createTranslateB2DHomMatrix(fOffsetX, fOffsetY);
                aTransform *= basegfx::utils::createScaleB2DHomMatrix(constTwipsPerMm100,
                                                                      constTwipsPerMm100);
                aPlaced.push_back(
                    new primitive2d::TransformPrimitive2D(aTransform, std::move(aContent)));
            }

            Color aBackground = rDocument.GetCellBackgroundColor(nStartColumn, nStartRow, nTab);
            if (aBackground.IsTransparent())
                aBackground = pTabViewShell->GetViewRenderingData().GetDocColor();

            auto aEntry = aWriter.startStruct();
            aWriter.put("kind", "texteditoverlay");
            aWriter.put("viewId", sal_Int32(pShell->GetViewShellId().get()));
            aWriter.put("column", sal_Int32(nStartColumn));
            aWriter.put("row", sal_Int32(nStartRow));
            aWriter.put("endColumn", sal_Int32(nEndColumn));
            aWriter.put("endRow", sal_Int32(nEndRow));
            aWriter.put("background", colorToHex(aBackground));

            Primitive2dJsonProcessor aProcessor(aWriter);
            aProcessor.setScaleFactor(1.0);
            maResources.attachTo(aProcessor);
            auto aPrimitives = aWriter.startArray("primitives");
            aProcessor.decomposeAndWrite(aPlaced);
        }
    }
    const OString aDocument = aWriter.finishAndGetAsOString();
    // The array alone travels, as the value of "textEdits".
    const sal_Int32 nStart = aDocument.indexOf('[');
    const sal_Int32 nEnd = aDocument.lastIndexOf(']');
    if (nStart < 0 || nEnd < nStart)
        return "[]"_ostr;
    return aDocument.copy(nStart, nEnd - nStart + 1);
}

VectorContent::SheetState* VectorContent::stateOf(SCTAB nTab)
{
    auto aIterator = maSheets.find(nTab);
    return aIterator == maSheets.end() ? nullptr : &aIterator->second;
}

void VectorContent::markRangeDirty(const ScRange& rRange)
{
    // Overflowing text can reach many columns away, so every held block in the block rows of the
    // range is looked at again, in the rows of the range. Comparing the content keeps unchanged
    // parts at their version.
    const sal_Int32 nFirstBlockRow = VectorContent::blockOf(0, rRange.aStart.Row()).first;
    const sal_Int32 nLastBlockRow = VectorContent::blockOf(0, rRange.aEnd.Row()).first;

    for (SCTAB nTab = rRange.aStart.Tab(); nTab <= rRange.aEnd.Tab(); ++nTab)
    {
        if (SheetState* pState = stateOf(nTab))
        {
            for (auto aIterator = pState->maBlocks.lower_bound({ nFirstBlockRow, 0 });
                 aIterator != pState->maBlocks.end() && aIterator->first.first <= nLastBlockRow;
                 ++aIterator)
            {
                const SCROW nBlockStartRow = aIterator->first.first * constBlockRows;
                const SCROW nBlockEndRow = nBlockStartRow + constBlockRows - 1;
                const SCROW nStartRow = std::max(rRange.aStart.Row(), nBlockStartRow);
                const SCROW nEndRow = std::min(rRange.aEnd.Row(), nBlockEndRow);
                markRowsDirty(aIterator->second, nStartRow, nEndRow,
                              nStartRow == nBlockStartRow && nEndRow == nBlockEndRow);
            }
        }
    }
}

void VectorContent::markRowsDirty(BlockState& rBlock, SCROW nStartRow, SCROW nEndRow,
                                  bool bWholeBlock)
{
    // A block marked in every row stays so.
    if (rBlock.mbDirty && !rBlock.moDirtyRows)
        return;
    // A block marked again after it was built is not known to the clients as stale yet.
    if (!rBlock.mbDirty)
        rBlock.mbStaleSent = false;
    if (bWholeBlock)
        rBlock.moDirtyRows.reset();
    else if (rBlock.mbDirty)
    {
        rBlock.moDirtyRows->first = std::min(rBlock.moDirtyRows->first, nStartRow);
        rBlock.moDirtyRows->second = std::max(rBlock.moDirtyRows->second, nEndRow);
    }
    else
        rBlock.moDirtyRows = std::make_pair(nStartRow, nEndRow);
    rBlock.mbDirty = true;
}

void VectorContent::cellsChanged(const ScRange& rRange)
{
    // The results are read when the block is written, so a cell still to be calculated is right
    // by then. The views hear of the sheet once the change that brought the cells is done.
    markRangeDirty(rRange);
    for (SCTAB nTab = rRange.aStart.Tab(); nTab <= rRange.aEnd.Tab(); ++nTab)
        maChangedTabs.insert(nTab);
}

void VectorContent::markObjectDirty(SCTAB nTab, sal_uInt64 nId)
{
    if (SheetState* pState = stateOf(nTab))
        pState->maDirtyObjects.insert(nId);
}

void VectorContent::notifyChangedSheets()
{
    const std::set<SCTAB> aTabs = std::move(maChangedTabs);
    maChangedTabs.clear();
    for (SCTAB nTab : aTabs)
        notifyViews(nTab);
}

void VectorContent::notifyViews(SCTAB nTab)
{
    if (!stateOf(nTab))
        return;
    KitHelper::notifyVectorPartChanged(&mrDocShell, OString::number(nTab),
                                       constCellPositionsMode);
}

void VectorContent::documentHint(const SfxHint& rHint)
{
    switch (rHint.GetId())
    {
        case SfxHintId::ScPaint:
        {
            const ScPaintHint& rPaint = static_cast<const ScPaintHint&>(rHint);
            const PaintPartFlags eParts = rPaint.GetParts();
            const ScDocument& rDocument = mrDocShell.GetDocument();
            ScRange aRange(rPaint.GetStartCol(), rPaint.GetStartRow(), rPaint.GetStartTab(),
                           rPaint.GetEndCol(), rPaint.GetEndRow(), rPaint.GetEndTab());

            // A change of size can change the rows of the range and, where rows were inserted or
            // removed, every row below them. The row above takes part in the borders of the first
            // row, and a merged area that reaches into it from above changes with it. Text
            // overflows into any column, so every column is looked at.
            if (eParts & (PaintPartFlags::Top | PaintPartFlags::Left | PaintPartFlags::Size))
            {
                const SCROW nAboveRow = std::max<SCROW>(aRange.aStart.Row() - 1, 0);
                SCROW nStartRow = nAboveRow;
                const SCTAB nLastTab
                    = std::min<SCTAB>(aRange.aEnd.Tab(), rDocument.GetTableCount() - 1);
                for (SCTAB nTab = aRange.aStart.Tab(); nStartRow > 0 && nTab <= nLastTab; ++nTab)
                {
                    SCCOL nOriginColumn = 0;
                    SCROW nOriginRow = nAboveRow;
                    rDocument.ExtendOverlapped(nOriginColumn, nOriginRow, rDocument.MaxCol(),
                                               nAboveRow, nTab);
                    nStartRow = std::min(nStartRow, nOriginRow);
                }
                aRange.aStart.SetCol(0);
                aRange.aStart.SetRow(nStartRow);
                aRange.aEnd.SetCol(rDocument.MaxCol());
                aRange.aEnd.SetRow(rDocument.MaxRow());
            }
            // An extras-only hint over all sheets comes from a change of sheet settings, such as a
            // name or tab color, that no block draws. The next write compares the sheet settings.
            // The hint ends at the last sheet of the document, whatever the paint was posted for.
            const bool bExtrasOfAllSheets
                = !(eParts
                    & (PaintPartFlags::Grid | PaintPartFlags::Top | PaintPartFlags::Left
                       | PaintPartFlags::Size))
                  && rPaint.GetStartTab() == 0
                  && rPaint.GetEndTab() >= rDocument.GetTableCount() - 1;
            if (!bExtrasOfAllSheets
                && (eParts
                    & (PaintPartFlags::Grid | PaintPartFlags::Extras | PaintPartFlags::Top
                       | PaintPartFlags::Left | PaintPartFlags::Size)))
                markRangeDirty(aRange);
            for (SCTAB nTab = aRange.aStart.Tab(); nTab <= aRange.aEnd.Tab(); ++nTab)
            {
                if (eParts & PaintPartFlags::Objects)
                {
                    if (SheetState* pState = stateOf(nTab))
                        pState->mbObjectsDirty = true;
                }
                notifyViews(nTab);
            }
            break;
        }
        case SfxHintId::ScDataChanged:
            // A change is done, and the sheet listeners marked the cells it gave new results.
            notifyChangedSheets();
            break;
        case SfxHintId::ScUpdateRef:
        {
            // Rows or columns inserted or removed move the cells after them. The paint of the
            // change marks the blocks; the next push tells the clients how the cells moved.
            const ScUpdateRefHint& rUpdate = static_cast<const ScUpdateRefHint&>(rHint);
            if (rUpdate.GetMode() != URM_INSDEL || rUpdate.GetDz() != 0
                || (rUpdate.GetDx() == 0 && rUpdate.GetDy() == 0))
                break;
            const ScRange& rRange = rUpdate.GetRange();
            const bool bRows = rUpdate.GetDy() != 0;
            const Shift aShift{ bRows,
                                bRows ? SCCOLROW(rRange.aStart.Row()) : SCCOLROW(rRange.aStart.Col()),
                                bRows ? SCCOLROW(rUpdate.GetDy()) : SCCOLROW(rUpdate.GetDx()),
                                bRows ? SCCOLROW(rRange.aStart.Col()) : SCCOLROW(rRange.aStart.Row()),
                                bRows ? SCCOLROW(rRange.aEnd.Col()) : SCCOLROW(rRange.aEnd.Row()) };
            for (SCTAB nTab = rRange.aStart.Tab(); nTab <= rRange.aEnd.Tab(); ++nTab)
            {
                if (SheetState* pState = stateOf(nTab))
                    pState->maShifts.push_back(aShift);
            }
            break;
        }
        case SfxHintId::ScTables:
            // Hiding a sheet renumbers nothing. The other sheet changes renumber the part ids,
            // so all clients start over in a new epoch.
            if (static_cast<const ScTablesHint&>(rHint).GetTablesHintId() == SC_TAB_HIDDEN)
                break;
            if (mbWriting)
                mbResetSheetsPending = true;
            else
                resetSheets();
            break;
        default:
            break;
    }
}

void VectorContent::resetSheets()
{
    mbResetSheetsPending = false;
    // The views are told for each served sheet, and their push writes the new epoch.
    const SCTAB nTabCount = mrDocShell.GetDocument().GetTableCount();
    std::vector<SCTAB> aServed;
    for (const auto& [nTab, rState] : maSheets)
        aServed.push_back(nTab);
    maSheets.clear();
    maSheetListeners.clear();
    maChangedTabs.clear();
    maResources.renewEpoch();
    // The charts moved with their sheets.
    if (const ScDrawLayer* pDrawLayer = mrDocShell.GetDocument().GetDrawLayer())
        mxChartListener->relocate(*pDrawLayer);

    for (const SCTAB nTab : aServed)
    {
        if (nTab < nTabCount)
            KitHelper::notifyVectorPartChanged(&mrDocShell, OString::number(nTab),
                                               constCellPositionsMode);
    }
}

void VectorContent::textEditChanged(SCTAB nTab)
{
    // The edit can read as before while what the client typed does not: completing a text the
    // client typed into keeps the text and moves only the selection.
    if (SheetState* pState = stateOf(nTab))
        pState->mbTextEditChanged = true;
    notifyViews(nTab);
}

void VectorContent::chartChanged(SCTAB nTab,
                                 const cpo::uno::Reference<cpo::uno::XInterface>& rChart)
{
    const ScDrawLayer* pDrawLayer = mrDocShell.GetDocument().GetDrawLayer();
    const SdrPage* pPage
        = pDrawLayer ? pDrawLayer->GetPage(static_cast<sal_uInt16>(nTab)) : nullptr;
    const SdrObject* pObject = pPage ? chartObjectOf(*pPage, rChart) : nullptr;
    if (pObject)
        markObjectDirty(nTab, pObject->GetUniqueID());
    else
    {
        if (SheetState* pState = stateOf(nTab))
            pState->mbObjectsDirty = true;
    }
    notifyViews(nTab);
}

void VectorContent::Notify(SfxBroadcaster& /*rBroadcaster*/, const SfxHint& rHint)
{
    if (rHint.GetId() == SfxHintId::Dying)
    {
        EndListeningAll();
        return;
    }
    if (rHint.GetId() != SfxHintId::ThisIsAnSdrHint)
        return;

    const SdrHint& rSdrHint = static_cast<const SdrHint&>(rHint);
    switch (rSdrHint.GetKind())
    {
        case SdrHintKind::ObjectChange:
        case SdrHintKind::ObjectInserted:
        case SdrHintKind::ObjectRemoved:
        // A shape text edit adds its entry and hides the text of the shape until it ends.
        case SdrHintKind::BeginEdit:
        case SdrHintKind::EndEdit:
        {
            const SdrPage* pPage = rSdrHint.GetPage();
            const SdrObject* pObject = rSdrHint.GetObject();
            if (!pPage && pObject)
                pPage = pObject->getSdrPageFromSdrObject();
            if (!pPage)
                return;
            const SCTAB nTab = static_cast<SCTAB>(pPage->GetPageNum());
            // A changed object is written again on its own. An inserted or removed object changes
            // the order and the group it belongs to, and a text edit changes what every shape
            // shows, so those mark every object.
            if (rSdrHint.GetKind() == SdrHintKind::ObjectChange && pObject)
                markObjectDirty(nTab, pObject->GetUniqueID());
            else
            {
                if (SheetState* pState = stateOf(nTab))
                    pState->mbObjectsDirty = true;
            }
            notifyViews(nTab);
            break;
        }
        default:
            break;
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
