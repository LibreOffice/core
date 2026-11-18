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

#include <VectorCollector.hxx>
#include <VectorText.hxx>

#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentSettingAccess.hxx>
#include <anchoredobject.hxx>
#include <dcontact.hxx>
#include <dflyobj.hxx>
#include <doc.hxx>
#include <flyfrm.hxx>
#include <fmtanchr.hxx>
#include <frmatr.hxx>
#include <frmfmt.hxx>
#include <ndtxt.hxx>
#include <layfrm.hxx>
#include <pagefrm.hxx>
#include <sortedobjs.hxx>
#include <tabfrm.hxx>
#include <txtfrm.hxx>
#include <viewimp.hxx>
#include <viewopt.hxx>
#include <viewsh.hxx>

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <rootfrm.hxx>
#include <tools/color.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/graphicprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/svdoattr.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdouno.hxx>
#include <svx/VectorObjectContent.hxx>
#include <vcl/GraphicObject.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/graph.hxx>
#include <vcl/virdev.hxx>

#include <algorithm>
#include <cassert>
#include <iterator>
#include <string_view>

namespace sw::vector
{
std::string_view nameOf(EntryKind eKind)
{
    switch (eKind)
    {
        case EntryKind::Page:
            return "page";
        case EntryKind::Header:
            return "header";
        case EntryKind::Footer:
            return "footer";
        case EntryKind::Body:
            return "body";
        case EntryKind::Column:
            return "column";
        case EntryKind::Section:
            return "section";
        case EntryKind::FootnoteContainer:
            return "footnotecontainer";
        case EntryKind::Footnote:
            return "footnote";
        case EntryKind::Table:
            return "table";
        case EntryKind::TableBorders:
            return "tableborders";
        case EntryKind::Borders:
            return "borders";
        case EntryKind::Background:
            return "background";
        case EntryKind::Row:
            return "row";
        case EntryKind::Cell:
            return "cell";
        case EntryKind::TextFrame:
            return "textframe";
        case EntryKind::NoTextFrame:
            return "notextframe";
        case EntryKind::Fly:
            return "fly";
        case EntryKind::Shape:
            return "shape";
    }
    return "body";
}

EntryKind kindOf(const SwFrame& rFrame)
{
    if (rFrame.IsPageFrame())
        return EntryKind::Page;
    if (rFrame.IsHeaderFrame())
        return EntryKind::Header;
    if (rFrame.IsFooterFrame())
        return EntryKind::Footer;
    if (rFrame.IsColumnFrame())
        return EntryKind::Column;
    if (rFrame.IsSctFrame())
        return EntryKind::Section;
    if (rFrame.IsFootnoteContFrame())
        return EntryKind::FootnoteContainer;
    if (rFrame.IsFootnoteFrame())
        return EntryKind::Footnote;
    if (rFrame.IsTabFrame())
        return EntryKind::Table;
    if (rFrame.IsRowFrame())
        return EntryKind::Row;
    if (rFrame.IsCellFrame())
        return EntryKind::Cell;
    if (rFrame.IsTextFrame())
        return EntryKind::TextFrame;
    if (rFrame.IsNoTextFrame())
        return EntryKind::NoTextFrame;
    if (rFrame.IsFlyFrame())
        return EntryKind::Fly;
    // The body and anything else that only holds other frames.
    return EntryKind::Body;
}

sal_uInt64 entryIdOf(const SdrObject& rObject)
{
    return (sal_uInt64(1) << 32) + rObject.GetUniqueID();
}

sal_uInt64 tableBordersEntryIdOf(const SwFrame& rTable)
{
    // Above the frame ids and the ids of drawing objects, which stay far below one step.
    return constEntryIdStep + rTable.GetFrameId();
}

sal_uInt64 bordersEntryIdOf(const SwFrame& rFrame)
{
    // From 2 steps up, above the ids of table border entries, which stay below 1 step + 2^32.
    return 2 * constEntryIdStep + rFrame.GetFrameId();
}

sal_uInt64 backgroundEntryIdOf(const SwFrame& rFrame)
{
    // The ids start at 4 steps. The ids of border entries stay below 2 steps + 2^32, and the
    // range from 3 steps up holds the keys the page entries are recorded under.
    return 4 * constEntryIdStep + rFrame.GetFrameId();
}

sal_uInt64 overContentEntryIdOf(const SwFrame& rFrame)
{
    // The ids start at 5 steps, above the ids of background entries, which stay below
    // 4 steps + 2^32.
    return 5 * constEntryIdStep + rFrame.GetFrameId();
}

void Collector::open(const SwFrame& rFrame)
{
    const EntryKind eKind = kindOf(rFrame);
    open(eKind == EntryKind::Page ? constPageEntryId : sal_uInt64(rFrame.GetFrameId()), eKind,
         rFrame.getFrameArea());

    if (rFrame.IsTextFrame())
    {
        const SwTextFrame& rTextFrame = static_cast<const SwTextFrame&>(rFrame);
        current().mbIsFollow = rTextFrame.IsFollow();
        current().mbHasFollow = rTextFrame.HasFollow();
    }
    else if (rFrame.IsFlyFrame() && !isPrinted(static_cast<const SwFlyFrame&>(rFrame)))
        current().mbPrinted = false;
}

void Collector::open(sal_uInt64 nId, EntryKind eKind, const SwRect& rBox)
{
    Entry aEntry;
    aEntry.mnId = nId;
    aEntry.meKind = eKind;
    aEntry.maBox = rBox;
    if (!maOpen.empty())
    {
        aEntry.moParent = maEntries[maOpen.back()].mnId;
        aEntry.mbPrinted = maEntries[maOpen.back()].mbPrinted;
    }

    maOpen.push_back(maEntries.size());
    maEntries.push_back(std::move(aEntry));
}

void Collector::close()
{
    assert(!maOpen.empty() && "Collector::close: no entry is open");
    maOpen.pop_back();
}

void Collector::add(const drawinglayer::primitive2d::Primitive2DReference& rPrimitive)
{
    if (rPrimitive)
        current().maPrimitives.push_back(rPrimitive);
}

void Collector::add(drawinglayer::primitive2d::Primitive2DContainer&& rPrimitives)
{
    if (!rPrimitives.empty())
        current().maPrimitives.append(std::move(rPrimitives));
}

Point Collector::origin() const
{
    assert(!maOpen.empty() && "Collector::origin: no entry is open");
    return maEntries[maOpen.back()].maBox.Pos();
}

Entry& Collector::current()
{
    assert(!maOpen.empty() && "Collector::current: no entry is open");
    return maEntries[maOpen.back()];
}

std::vector<Entry> Collector::take()
{
    assert(maOpen.empty() && "Collector::take: an entry is still open");
    assert(maBorderLayers.empty() && "Collector::take: a layer of border lines is still open");
    assert(maBackgroundLayers.empty() && "Collector::take: a layer of backgrounds is still open");
    std::vector<Entry> aEntries;
    aEntries.swap(maEntries);
    return aEntries;
}

void Collector::openBackgroundLayer()
{
    BackgroundLayer aLayer;
    aLayer.mnPosition = maEntries.size();
    aLayer.mnFirst = maBackgrounds.size();
    maBackgroundLayers.push_back(aLayer);
}

void Collector::addBackground(const SwFrame& rFrame,
                              drawinglayer::primitive2d::Primitive2DContainer&& rBackground)
{
    if (rBackground.empty())
        return;

    // The paint draws the backgrounds of paragraphs and table cells once more over the objects
    // behind the text when the document asks for that. Their own entries come after those
    // objects.
    const bool bOverObjects
        = (rFrame.IsTextFrame() || rFrame.IsCellFrame())
          && rFrame.getRootFrame()->GetFormat()->getIDocumentSettingAccess().get(
                 DocumentSettingId::BACKGROUND_PARA_OVER_DRAWINGS);
    if (maBackgroundLayers.empty() || rFrame.IsPageFrame() || rFrame.IsFlyFrame() || bOverObjects)
    {
        add(std::move(rBackground));
        return;
    }

    Entry aEntry;
    aEntry.mnId = backgroundEntryIdOf(rFrame);
    aEntry.moParent = current().mnId;
    aEntry.meKind = EntryKind::Background;
    aEntry.maBox = current().maBox;
    aEntry.mbPrinted = current().mbPrinted;
    aEntry.maPrimitives = std::move(rBackground);
    maBackgrounds.push_back(std::move(aEntry));
}

void Collector::closeBackgroundLayer()
{
    assert(!maBackgroundLayers.empty() && "Collector::closeBackgroundLayer: no layer is open");
    const BackgroundLayer aLayer = maBackgroundLayers.back();
    maBackgroundLayers.pop_back();
    // The entries open now were opened before the layer, so they keep their indexes.
    assert((maOpen.empty() || maOpen.back() < aLayer.mnPosition)
           && "Collector::closeBackgroundLayer: an entry opened inside the layer is still open");
    maEntries.insert(maEntries.begin() + aLayer.mnPosition,
                     std::make_move_iterator(maBackgrounds.begin() + aLayer.mnFirst),
                     std::make_move_iterator(maBackgrounds.end()));
    maBackgrounds.resize(aLayer.mnFirst);
}

void Collector::openBorderLayer() { maBorderLayers.push_back(maBorders.size()); }

void Collector::deferBorders(const SwFrame& rFrame,
                             drawinglayer::primitive2d::Primitive2DContainer&& rBorders)
{
    deferLines(bordersEntryIdOf(rFrame), std::move(rBorders));
}

void Collector::deferOverContent(const SwFrame& rFrame,
                                 drawinglayer::primitive2d::Primitive2DContainer&& rLines)
{
    deferLines(overContentEntryIdOf(rFrame), std::move(rLines));
}

void Collector::deferLines(sal_uInt64 nId, drawinglayer::primitive2d::Primitive2DContainer&& rLines)
{
    if (rLines.empty())
        return;

    Entry aEntry;
    aEntry.mnId = nId;
    aEntry.moParent = current().mnId;
    aEntry.meKind = EntryKind::Borders;
    aEntry.maBox = current().maBox;
    aEntry.mbPrinted = current().mbPrinted;
    aEntry.maPrimitives = std::move(rLines);
    // Without a layer around them the lines come right after what is gathered so far.
    if (maBorderLayers.empty())
        maEntries.push_back(std::move(aEntry));
    else
        maBorders.push_back(std::move(aEntry));
}

void Collector::closeBorderLayer()
{
    assert(!maBorderLayers.empty() && "Collector::closeBorderLayer: no layer is open");
    const size_t nStart = maBorderLayers.back();
    for (size_t i = nStart; i < maBorders.size(); ++i)
        maEntries.push_back(std::move(maBorders[i]));
    maBorders.resize(nStart);
    maBorderLayers.pop_back();
}

basegfx::B2DRange rangeRelativeTo(const SwRect& rArea, const Point& rOrigin)
{
    return basegfx::B2DRange(rArea.Left() - rOrigin.X(), rArea.Top() - rOrigin.Y(),
                             rArea.Left() + rArea.Width() - rOrigin.X(),
                             rArea.Top() + rArea.Height() - rOrigin.Y());
}

void addColorFill(drawinglayer::primitive2d::Primitive2DContainer& rTarget,
                  basegfx::B2DPolyPolygon&& rArea, const Color& rColor)
{
    if (!rArea.count() || rColor.IsFullyTransparent())
        return;

    drawinglayer::primitive2d::Primitive2DReference xFill(
        new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(std::move(rArea),
                                                                   rColor.getBColor()));
    if (rColor.IsTransparent())
        xFill = new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(
            drawinglayer::primitive2d::Primitive2DContainer{ xFill },
            1.0 - rColor.GetAlpha() / 255.0);
    rTarget.push_back(xFill);
}

std::optional<sal_uInt64> anchorOf(const SwAnchoredObject& rObject)
{
    const SwFrame* pAnchor = rObject.GetAnchorFrame();
    if (!pAnchor || pAnchor->IsPageFrame())
        return std::nullopt;
    return pAnchor->GetFrameId();
}

bool isPrinted(const SwAnchoredObject& rObject)
{
    if (!rObject.GetFrameFormat()->GetPrint().GetValue())
        return false;
    const SwFrame* pAnchor = rObject.GetAnchorFrame();
    const SwFlyFrame* pAnchorFly = pAnchor ? pAnchor->FindFlyFrame() : nullptr;
    return !pAnchorFly || isPrinted(*pAnchorFly);
}

namespace
{
/// Adds the entry of a drawing object that is not a frame of Writer: a shape, a control or a
/// group. Its primitives are in document twips, so they are moved onto the entry's origin.
void addShape(Collector& rCollector, const SwAnchoredObject& rObject)
{
    const SdrObject* pObject = rObject.GetDrawObj();
    if (!pObject)
        return;

    SwRect aBox(pObject->GetCurrentBoundRect());
    const std::optional<SwRect> oClip = getHorizontalRuleClip(pObject);
    if (oClip)
        aBox.Intersection(*oClip);
    if (!aBox.HasArea() || !rCollector.wants(aBox))
        return;

    drawinglayer::primitive2d::Primitive2DContainer aPrimitives;
    pObject->GetViewContact().getViewIndependentPrimitive2DContainer(aPrimitives);
    if (aPrimitives.empty())
        return;

    // A rule is cropped with the mask that the paint puts around it. The primitives of the object
    // are in document twips, as the mask is.
    if (oClip)
    {
        drawinglayer::primitive2d::Primitive2DReference xMask(
            createHorizontalRuleMask(*oClip, std::move(aPrimitives)));
        aPrimitives = drawinglayer::primitive2d::Primitive2DContainer{ xMask };
    }

    rCollector.open(entryIdOf(*pObject), EntryKind::Shape, aBox);
    rCollector.current().moAnchor = anchorOf(rObject);
    if (!isPrinted(rObject))
        rCollector.current().mbPrinted = false;
    // A view draws the entries right away and compares nothing, so it needs no decomposition.
    if (!rCollector.context().isForView())
        svx::vector::decomposeForComparison(aPrimitives, rCollector.current().maDrawn,
                                            drawinglayer::geometry::ViewInformation2D());
    rCollector.add(new drawinglayer::primitive2d::TransformPrimitive2D(
        basegfx::utils::createTranslateB2DHomMatrix(-aBox.Left(), -aBox.Top()),
        std::move(aPrimitives)));
    rCollector.close();
}

/// Where the paint draws the objects on the layer nLayer, against the text.
ObjectPlace placeOf(SdrLayerID nLayer, const IDocumentDrawModelAccess& rAccess)
{
    if (nLayer == rAccess.GetHellId())
        return ObjectPlace::BehindText;
    if (nLayer == rAccess.GetHeaderFooterHellId())
        return ObjectPlace::BehindBodyText;
    return ObjectPlace::InFrontOfText;
}

/// Whether the paint draws rObject on the screen, judged from the layout and the document alone.
/// The object has to be visible and on a visible layer. The object that stands for all the frames
/// of a fly format is never drawn, as each of those frames has an object of its own. Every object
/// needs the frame it is anchored at. A fly has to overlap its page, a drawing object needs a
/// valid position of the frame it is anchored at, and an object anchored in a fly is drawn only
/// when that fly is.
bool isDrawn(const SwAnchoredObject& rObject, const IDocumentDrawModelAccess& rAccess,
             const Context& rContext)
{
    const SdrObject* pDrawObject = rObject.GetDrawObj();
    if (!pDrawObject || !rAccess.IsVisibleLayerId(pDrawObject->GetLayer())
        || !pDrawObject->IsVisible())
        return false;
    if (dynamic_cast<const SwFlyDrawObj*>(pDrawObject))
        return false;

    const SwFrame* pAnchor = rObject.GetAnchorFrame();
    if (!pAnchor)
        return false;
    if (const SwFlyFrame* pFly = rObject.DynCastFlyFrame())
    {
        const SwPageFrame* pPage = pFly->FindPageFrame();
        if (!pPage || !pPage->getFrameArea().Overlaps(pFly->getFrameArea()))
            return false;
    }
    else if (!pAnchor->isFrameAreaPositionValid())
        return false;

    if (const SwFlyFrame* pAnchorFly = pAnchor->FindFlyFrame())
        return isDrawn(*pAnchorFly, rAccess, rContext);
    return true;
}

/// Whether the view that rContext is for hides rObject and shows a placeholder in its place. A
/// view can hide the drawing objects and the controls, and it can hide the pictures, which hides
/// every fly. Entries for any client show every object.
bool isReplacedInView(const SdrObject& rObject, const Context& rContext)
{
    if (!rContext.isForView() || !GetUserCall(&rObject))
        return false;

    const SwViewOption& rOptions = rContext.options();
    if (!rOptions.IsDraw()
        && (dynamic_cast<const SdrUnoObj*>(&rObject) || dynamic_cast<const SdrAttrObj*>(&rObject)))
        return true;
    return !rOptions.IsGraphic() && dynamic_cast<const SwVirtFlyDrawObj*>(&rObject);
}

/// Whether rObject is anchored inside a fly that the view of rContext shows a placeholder for.
/// The placeholder of that fly stands for all it holds.
bool isInReplacedFly(const SwAnchoredObject& rObject, const Context& rContext)
{
    const SwFrame* pAnchor = rObject.GetAnchorFrame();
    const SwFlyFrame* pFly = pAnchor ? pAnchor->FindFlyFrame() : nullptr;
    while (pFly)
    {
        if (isReplacedInView(*pFly->GetVirtDrawObj(), rContext))
            return true;
        pAnchor = pFly->GetAnchorFrame();
        pFly = pAnchor ? pAnchor->FindFlyFrame() : nullptr;
    }
    return false;
}

/// Adds the entry of the placeholder that the view of rCollector shows in place of rObject, which
/// it hides. The placeholder covers the area of the object with the space around it, and it is
/// what the paint of the view draws there, recorded as a picture.
void addReplacement(Collector& rCollector, const SwAnchoredObject& rObject)
{
    const SdrObject* pObject = rObject.GetDrawObj();
    const SwViewShell* pShell = rCollector.context().shell();
    const SwRect aBox(GetBoundRectOfAnchoredObj(pObject));
    if (!pShell || !aBox.HasArea() || !rCollector.wants(aBox))
        return;

    ScopedVclPtrInstance<VirtualDevice> pDevice;
    pDevice->SetMapMode(MapMode(MapUnit::MapTwip));
    GDIMetaFile aRecording;
    aRecording.Record(pDevice.get());
    vcl::Font aFont(pDevice->GetFont());
    Graphic::DrawEx(*pDevice, OUString(), aFont,
                    const_cast<SwViewShell*>(pShell)->GetReplacementBitmap(), Point(),
                    aBox.SSize());
    aRecording.Stop();
    aRecording.WindStart();
    aRecording.SetPrefMapMode(MapMode(MapUnit::MapTwip));
    aRecording.SetPrefSize(aBox.SSize());

    const SwFlyFrame* pFly = rObject.DynCastFlyFrame();
    if (pFly)
        rCollector.open(pFly->GetFrameId(), EntryKind::Fly, aBox);
    else
        rCollector.open(entryIdOf(*pObject), EntryKind::Shape, aBox);
    rCollector.current().moAnchor = anchorOf(rObject);
    if (!isPrinted(rObject))
        rCollector.current().mbPrinted = false;
    rCollector.add(new drawinglayer::primitive2d::GraphicPrimitive2D(
        basegfx::utils::createScaleB2DHomMatrix(aBox.Width(), aBox.Height()),
        GraphicObject(Graphic(aRecording))));
    rCollector.close();
}

/// Adds the entries of the objects in rObjects that are drawn at ePlace, in paint order. With
/// bAsCharacter set, only objects anchored as a character are added.
void addObjects(Collector& rCollector, const SwSortedObjs& rObjects,
                const IDocumentDrawModelAccess& rAccess, ObjectPlace ePlace, bool bAsCharacter)
{
    std::vector<const SwAnchoredObject*> aObjects;
    for (const SwAnchoredObject* pObject : rObjects)
    {
        if (!isDrawn(*pObject, rAccess, rCollector.context()))
            continue;
        if (bAsCharacter
            && pObject->GetFrameFormat()->GetAnchor().GetAnchorId() != RndStdIds::FLY_AS_CHAR)
            continue;
        if (placeOf(pObject->GetDrawObj()->GetLayer(), rAccess) == ePlace)
            aObjects.push_back(pObject);
    }

    // The controls are drawn after every other layer. Within a layer the order number is the
    // order the drawing layer paints in.
    const SdrLayerID nControlsId = rAccess.GetControlsId();
    std::stable_sort(aObjects.begin(), aObjects.end(),
                     [nControlsId](const SwAnchoredObject* pOne, const SwAnchoredObject* pOther) {
                         const bool bOneIsControl = pOne->GetDrawObj()->GetLayer() == nControlsId;
                         const bool bOtherIsControl
                             = pOther->GetDrawObj()->GetLayer() == nControlsId;
                         if (bOneIsControl != bOtherIsControl)
                             return bOtherIsControl;
                         return pOne->GetDrawObj()->GetOrdNum()
                                < pOther->GetDrawObj()->GetOrdNum();
                     });

    for (const SwAnchoredObject* pObject : aObjects)
    {
        // A view shows a placeholder for an object it hides, and nothing for what a hidden fly
        // holds.
        if (isReplacedInView(*pObject->GetDrawObj(), rCollector.context()))
            addReplacement(rCollector, *pObject);
        else if (isInReplacedFly(*pObject, rCollector.context()))
            continue;
        else if (const SwFlyFrame* pFly = pObject->DynCastFlyFrame())
            pFly->CreatePrimitives(rCollector);
        else
            addShape(rCollector, *pObject);
    }
}
}

void addPageObjects(Collector& rCollector, const SwPageFrame& rPage, ObjectPlace ePlace)
{
    if (const SwSortedObjs* pObjects = rPage.GetSortedObjs())
        addObjects(rCollector, *pObjects,
                   rPage.GetFormat()->GetDoc().getIDocumentDrawModelAccess(), ePlace,
                   /*bAsCharacter=*/false);
}

void addCharacterObjects(Collector& rCollector, const SwTextFrame& rFrame)
{
    const SwSortedObjs* pObjects = rFrame.GetDrawObjs();
    if (!pObjects)
        return;

    const IDocumentDrawModelAccess& rAccess
        = rFrame.GetTextNodeFirst()->GetDoc().getIDocumentDrawModelAccess();
    for (const ObjectPlace ePlace :
         { ObjectPlace::BehindText, ObjectPlace::BehindBodyText, ObjectPlace::InFrontOfText })
        addObjects(rCollector, *pObjects, rAccess, ePlace, /*bAsCharacter=*/true);
}

namespace
{
const SwFrame* frameIn(const SwFrame& rFrame, sal_uInt64 nId)
{
    if (rFrame.GetFrameId() == nId)
        return &rFrame;
    // Every fly is among the objects of the frame it is anchored at.
    if (const SwSortedObjs* pObjects = rFrame.GetDrawObjs())
    {
        for (const SwAnchoredObject* pObject : *pObjects)
        {
            const SwFlyFrame* pFly = pObject->DynCastFlyFrame();
            if (const SwFrame* pFound = pFly ? frameIn(*pFly, nId) : nullptr)
                return pFound;
        }
    }
    if (rFrame.IsLayoutFrame())
    {
        for (const SwFrame* pLower = static_cast<const SwLayoutFrame&>(rFrame).Lower(); pLower;
             pLower = pLower->GetNext())
        {
            if (const SwFrame* pFound = frameIn(*pLower, nId))
                return pFound;
        }
    }
    return nullptr;
}
}

const SwFrame* frameOf(const SwRootFrame& rLayout, sal_uInt64 nId, const SwPageFrame* pNear)
{
    if (!pNear || rLayout.GetFrameId() == nId)
        return frameIn(rLayout, nId);

    // The root frame holds the pages and nothing else, so the walk outward from pNear visits
    // every frame the whole walk visits.
    const SwFrame* pAfter = pNear;
    const SwFrame* pBefore = pNear->GetPrev();
    while (pAfter || pBefore)
    {
        if (pAfter)
        {
            if (const SwFrame* pFound = frameIn(*pAfter, nId))
                return pFound;
            pAfter = pAfter->GetNext();
        }
        if (pBefore)
        {
            if (const SwFrame* pFound = frameIn(*pBefore, nId))
                return pFound;
            pBefore = pBefore->GetPrev();
        }
    }
    return nullptr;
}

std::vector<Entry> decomposePage(const SwPageFrame& rPage)
{
    Collector aCollector;
    rPage.CreatePrimitives(aCollector);
    return aCollector.take();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
