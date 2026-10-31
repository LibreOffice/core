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

#include <IDocumentDrawModelAccess.hxx>
#include <anchoredobject.hxx>
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

#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/svdobj.hxx>

#include <algorithm>
#include <cassert>

namespace sw::vector
{
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
    // Above the frame ids and the ids of drawing objects, which stay far below 2^40.
    return (sal_uInt64(1) << 40) + rTable.GetFrameId();
}

sal_uInt64 bordersEntryIdOf(const SwFrame& rFrame)
{
    // From 2 * 2^40 up, above the ids of table border entries, which stay below 2^40 + 2^32.
    return (sal_uInt64(2) << 40) + rFrame.GetFrameId();
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
}

void Collector::open(sal_uInt64 nId, EntryKind eKind, const SwRect& rBox)
{
    Entry aEntry;
    aEntry.mnId = nId;
    aEntry.meKind = eKind;
    aEntry.maBox = rBox;
    if (!maOpen.empty())
        aEntry.moParent = maEntries[maOpen.back()].mnId;

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
    std::vector<Entry> aEntries;
    aEntries.swap(maEntries);
    return aEntries;
}

void Collector::openBorderLayer() { maBorderLayers.push_back(maBorders.size()); }

void Collector::deferBorders(const SwFrame& rFrame,
                             drawinglayer::primitive2d::Primitive2DContainer&& rBorders)
{
    if (rBorders.empty())
        return;

    Entry aEntry;
    aEntry.mnId = bordersEntryIdOf(rFrame);
    aEntry.moParent = current().mnId;
    aEntry.meKind = EntryKind::Borders;
    aEntry.maBox = current().maBox;
    aEntry.maPrimitives = std::move(rBorders);
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

std::optional<sal_uInt64> anchorOf(const SwAnchoredObject& rObject)
{
    const SwFrame* pAnchor = rObject.GetAnchorFrame();
    if (!pAnchor || pAnchor->IsPageFrame())
        return std::nullopt;
    return pAnchor->GetFrameId();
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

    const SwRect aBox(pObject->GetCurrentBoundRect());
    if (!aBox.HasArea())
        return;

    drawinglayer::primitive2d::Primitive2DContainer aPrimitives;
    pObject->GetViewContact().getViewIndependentPrimitive2DContainer(aPrimitives);
    if (aPrimitives.empty())
        return;

    rCollector.open(entryIdOf(*pObject), EntryKind::Shape, aBox);
    rCollector.current().moAnchor = anchorOf(rObject);
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

/// Whether the paint draws rObject, judged from the layout and the document alone. The object
/// has to be visible and on a visible layer, and set to print unless bForView is set, since a
/// view also shows what is not printed. A fly has to overlap its page, a drawing object needs a
/// valid position of the frame it is anchored at, and an object anchored in a fly is drawn only
/// when that fly is.
bool isDrawn(const SwAnchoredObject& rObject, const IDocumentDrawModelAccess& rAccess,
             bool bForView)
{
    const SdrObject* pDrawObject = rObject.GetDrawObj();
    if (!pDrawObject || !rAccess.IsVisibleLayerId(pDrawObject->GetLayer())
        || !pDrawObject->IsVisible())
        return false;
    if (!bForView && !rObject.GetFrameFormat()->GetPrint().GetValue())
        return false;

    const SwFrame* pAnchor = rObject.GetAnchorFrame();
    if (const SwFlyFrame* pFly = rObject.DynCastFlyFrame())
    {
        const SwPageFrame* pPage = pFly->FindPageFrame();
        if (!pPage || !pPage->getFrameArea().Overlaps(pFly->getFrameArea()))
            return false;
    }
    else if (!pAnchor || !pAnchor->isFrameAreaPositionValid())
        return false;

    if (const SwFlyFrame* pAnchorFly = pAnchor ? pAnchor->FindFlyFrame() : nullptr)
        return isDrawn(*pAnchorFly, rAccess, bForView);
    return true;
}

/// Adds the entries of the objects in rObjects that are drawn at ePlace, in paint order. With
/// bAsCharacter set, only objects anchored as a character are added.
void addObjects(Collector& rCollector, const SwSortedObjs& rObjects,
                const IDocumentDrawModelAccess& rAccess, ObjectPlace ePlace, bool bAsCharacter)
{
    std::vector<const SwAnchoredObject*> aObjects;
    for (const SwAnchoredObject* pObject : rObjects)
    {
        if (!isDrawn(*pObject, rAccess, /*bForView=*/false))
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
        if (const SwFlyFrame* pFly = pObject->DynCastFlyFrame())
        {
            pFly->CreatePrimitives(rCollector);
        }
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

std::vector<Entry> decomposePage(const SwPageFrame& rPage)
{
    Collector aCollector;
    rPage.CreatePrimitives(aCollector);
    return aCollector.take();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
