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

#include <tools/PresentationLint.hxx>

#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

#include <editeng/editobj.hxx>
#include <editeng/outlobj.hxx>
#include <o3tl/sorted_vector.hxx>
#include <o3tl/string_view.hxx>
#include <o3tl/unit_conversion.hxx>
#include <rtl/ref.hxx>
#include <svx/sdgcpitm.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdundo.hxx>
#include <unotools/weakref.hxx>
#include <vcl/GraphicAttributes.hxx>
#include <vcl/GraphicObject.hxx>
#include <vcl/gfxlink.hxx>
#include <vcl/graph.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/outdev.hxx>
#include <vcl/svapp.hxx>

#include <DrawViewShell.hxx>
#include <ModelTraverser.hxx>
#include <View.hxx>
#include <ViewShellBase.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <sdresid.hxx>
#include <strings.hrc>
#include <tools/LintImageCompressor.hxx>
#include <tools/LintMeasureCache.hxx>

namespace sd::lint
{
namespace
{
/** An image counts as too big only once it carries half again the pixels the largest drawing of it
    needs, so that an image a little over the target does not earn a row of its own. */
constexpr double gfPixelTolerance = 1.5;

/** An image counts as heavily cropped once a crop hides at least this share of its area, so that
    trimming off a thin edge does not earn a row of its own. */
constexpr double gfCroppedAreaThreshold = 0.10;

/** Zero-based slide number of every standard page and of every notes page of the document. Master
    pages are not in it. */
using SlideIndexMap = std::unordered_map<const SdrPage*, sal_Int32>;

/** Numbers the pages of the document, so that a rule walking many objects looks a page up instead
    of searching the document again for each of them. */
SlideIndexMap buildSlideIndexMap(SdDrawDocument& rDoc)
{
    SlideIndexMap aSlideIndexOfPage;

    const sal_uInt16 nPageCount = rDoc.GetSdPageCount(PageKind::Standard);
    for (sal_uInt16 nPage = 0; nPage < nPageCount; ++nPage)
    {
        // A notes page counts as the slide it belongs to, so an object pasted into the speaker
        // notes is reported under that slide's number.
        for (PageKind eKind : { PageKind::Standard, PageKind::Notes })
        {
            if (const SdPage* pPage = rDoc.GetSdPage(nPage, eKind))
                aSlideIndexOfPage.emplace(pPage, sal_Int32(nPage));
        }
    }

    return aSlideIndexOfPage;
}

/** Zero-based number of the slide the page belongs to, or -1 for a master page and for a page the
    map does not know. */
sal_Int32 lookUpSlideIndex(const SlideIndexMap& rSlideIndexOfPage, const SdrPage* pPage)
{
    if (!pPage)
        return -1;

    auto aFound = rSlideIndexOfPage.find(pPage);
    return aFound == rSlideIndexOfPage.end() ? -1 : aFound->second;
}

/** True when the object still sits in an object list of a page the document holds, so it can be
    replaced there. */
bool isObjectOnPage(const SdrObject* pObject)
{
    if (!pObject || !pObject->IsInserted() || !pObject->getParentSdrObjListFromSdrObject())
        return false;

    // Taking a page out of the document leaves the objects on it marked as inserted, so the page
    // itself is asked too. A page that is out of the document takes its objects with it, and a
    // change to one of them would reach nothing the reader can see.
    const SdrPage* pPage = pObject->getSdrPageFromSdrObject();
    return pPage && pPage->IsInserted();
}

/** Shows the standard page with the given zero-based number and marks one object on it. An object
    that is no longer on a page is left unmarked. */
void showSlide(ViewShellBase& rViewShellBase, sal_Int32 nSlideIndex, SdrObject* pObject)
{
    if (nSlideIndex < 0)
        return;

    std::shared_ptr<ViewShell> pViewShell = rViewShellBase.GetMainViewShell();
    auto* pDrawViewShell = dynamic_cast<DrawViewShell*>(pViewShell.get());
    if (!pDrawViewShell)
        return;

    // A page number is looked up in the page list of the edit mode the view is in, so the view is
    // put into the normal mode first and the number then names the slide the reader asked for.
    if (pDrawViewShell->GetEditMode() != EditMode::Page)
        pDrawViewShell->ChangeEditMode(EditMode::Page, false);

    pDrawViewShell->SwitchPage(sal_uInt16(nSlideIndex));

    ::sd::View* pView = pDrawViewShell->GetView();
    if (!pView || !isObjectOnPage(pObject))
        return;

    SdrPageView* pPageView = pView->GetSdrPageView();
    if (!pPageView)
        return;

    pView->UnmarkAll();
    pView->MarkObj(pObject, pPageView);
}

/** Closes a text edit that is open in the main view, so a page or an object can be taken out from
    under it. */
void endTextEdit(ViewShellBase& rViewShellBase)
{
    std::shared_ptr<ViewShell> pViewShell = rViewShellBase.GetMainViewShell();
    auto* pDrawViewShell = dynamic_cast<DrawViewShell*>(pViewShell.get());
    if (!pDrawViewShell)
        return;

    ::sd::View* pView = pDrawViewShell->GetView();
    if (pView)
        pView->SdrEndTextEdit();
}

/** True when the page is still one of the master pages of the document. */
bool isMasterPageOfDocument(SdDrawDocument& rDoc, const SdPage* pMasterPage)
{
    const sal_uInt16 nMasterCount = rDoc.GetMasterSdPageCount(PageKind::Standard);
    for (sal_uInt16 nMaster = 0; nMaster < nMasterCount; ++nMaster)
    {
        if (rDoc.GetMasterSdPage(nMaster, PageKind::Standard) == pMasterPage)
            return true;
    }

    return false;
}

/** One entry in the undo list that carries the precious mark of a master slide. A master that is
    marked precious stays in the document when unused masters are dropped, and the cleanup takes
    that mark off before it lets the master go. Taking the cleanup back puts the mark back at the
    value the master carried, and making it again takes the mark off. The page comes and goes with
    the entry that holds it, and this holds a reference of its own, so the same page is marked
    either way. */
class LintUndoMasterPrecious final : public SdrUndoAction
{
public:
    LintUndoMasterPrecious(SdrModel& rModel, SdPage& rMasterPage, bool bWasPrecious)
        : SdrUndoAction(rModel)
        , mxMasterPage(&rMasterPage)
        , mbWasPrecious(bWasPrecious)
    {
    }

    virtual void Undo() override { mxMasterPage->SetPrecious(mbWasPrecious); }
    virtual void Redo() override { mxMasterPage->SetPrecious(false); }

    /** The name of the whole entry is the one the cleanup was given, and this part of it adds
        nothing to it. */
    virtual OUString GetComment() const override { return OUString(); }

private:
    rtl::Reference<SdPage> mxMasterPage;

    /** The precious mark the master carried before the cleanup took it off. */
    bool mbWasPrecious;
};

/** Puts a new object in the place of an old one and records the swap on the undo stack, so a single
    undo brings the old object back. */
void replaceObject(SdrObject& rOldObject, SdrObject& rNewObject)
{
    SdrModel& rModel = rOldObject.getSdrModelFromSdrObject();
    if (rModel.IsUndoEnabled())
        rModel.AddUndo(rModel.GetSdrUndoFactory().CreateUndoReplaceObject(rOldObject, rNewObject));

    rOldObject.getParentSdrObjListFromSdrObject()->ReplaceObject(&rNewObject,
                                                                 rOldObject.GetOrdNum());
}

/** Everything the scan learned about one distinct bitmap and the objects that draw it. The scan
    fills this in for every bitmap it meets, and only afterwards is it decided which of them are
    stored at a higher resolution than the deck needs. */
struct LargeImageCandidate
{
    /** Every object drawing the bitmap, whatever resolution each of them draws it at. */
    std::vector<SdrObject*> maObjects;

    /** The bitmap itself. */
    Graphic maGraphic;

    /** Checksum of the bitmap, which is what tells one bitmap from another. */
    BitmapChecksum mnChecksum = 0;

    /** The largest drawing size any object gives the bitmap, taken separately for each dimension,
        in hundredths of a millimetre. */
    Size maMaxLogicSize{ 0, 0 };

    /** The highest resolution in dots per inch any object draws the bitmap at. */
    sal_Int32 mnMaxEffectiveDPI = 0;

    /** Bytes the document spends on the encoded bitmap, or zero when it kept no encoded data. */
    sal_uInt64 mnCurrentBytes = 0;

    /** The object that draws the bitmap at the highest resolution. */
    SdrObject* mpWorstObject = nullptr;

    /** Zero-based number of the slide that object sits on, or -1 for a master slide. */
    sal_Int32 mnWorstSlideIndex = -1;

    /** Size of the whole bitmap in hundredths of a millimetre, which is the size the crop values
        are measured against. */
    Size maBitmapLogicSize{ 0, 0 };

    /** How much of the bitmap the objects cut off at each edge, in hundredths of a millimetre. All
        four are zero for a bitmap every object shows whole. */
    tools::Long mnCropLeft = 0;
    tools::Long mnCropTop = 0;
    tools::Long mnCropRight = 0;
    tools::Long mnCropBottom = 0;

    /** True when the objects disagree about which part of the bitmap they show, or when one of them
        carries a negative crop value, which pads the picture out rather than trimming it. */
    bool mbMixedCrop = false;

    /** True when at least one object is drawn at a size that gives no resolution to work from. */
    bool mbUnmeasured = false;
};

/** Size of the whole bitmap in hundredths of a millimetre, which is the unit the crop values are
    written in. A bitmap that knows itself in pixels is measured on the default device, and one that
    knows itself in a logic unit is converted from that unit. */
Size getBitmapLogicSize(const Graphic& rGraphic)
{
    const MapMode aMap100(MapUnit::Map100thMM);
    const MapMode aPrefMapMode = rGraphic.GetPrefMapMode();
    const Size aPrefSize = rGraphic.GetPrefSize();

    if (aPrefMapMode.GetMapUnit() == MapUnit::MapPixel)
        return Application::GetDefaultDevice()->PixelToLogic(aPrefSize, aMap100);

    return OutputDevice::LogicToLogic(aPrefSize, aPrefMapMode, aMap100);
}

/** The share of one dimension of the bitmap the crop leaves showing, from just above zero up to
    one. A crop of nothing leaves the whole of it, and a crop that would leave nothing is held just
    above zero so that the figures worked out from it stay usable. */
double getVisibleFraction(tools::Long nLogicLength, tools::Long nStartCrop, tools::Long nEndCrop)
{
    if (nLogicLength <= 0)
        return 1.0;

    const double fVisible = double(nLogicLength - nStartCrop - nEndCrop) / double(nLogicLength);
    return std::clamp(fVisible, 0.001, 1.0);
}

double getVisibleWidthFraction(const LargeImageCandidate& rCandidate)
{
    return getVisibleFraction(rCandidate.maBitmapLogicSize.Width(), rCandidate.mnCropLeft,
                              rCandidate.mnCropRight);
}

double getVisibleHeightFraction(const LargeImageCandidate& rCandidate)
{
    return getVisibleFraction(rCandidate.maBitmapLogicSize.Height(), rCandidate.mnCropTop,
                              rCandidate.mnCropBottom);
}

/** The share of the area of the bitmap the crop hides, from zero for a bitmap that is shown whole
    up to just under one. */
double getHiddenAreaShare(const LargeImageCandidate& rCandidate)
{
    return 1.0 - getVisibleWidthFraction(rCandidate) * getVisibleHeightFraction(rCandidate);
}

/** Takes in the crop of one more object that draws the bitmap. The first object says which part of
    the bitmap the entry is about, and an object that shows a different part, or one that pads the
    picture out with a negative value, marks the entry as one to leave alone. */
void recordCrop(LargeImageCandidate& rCandidate, const SdrGrafCropItem& rCrop, bool bFirstObject)
{
    if (rCrop.GetLeft() < 0 || rCrop.GetTop() < 0 || rCrop.GetRight() < 0 || rCrop.GetBottom() < 0)
    {
        rCandidate.mbMixedCrop = true;
        return;
    }

    if (bFirstObject)
    {
        rCandidate.mnCropLeft = rCrop.GetLeft();
        rCandidate.mnCropTop = rCrop.GetTop();
        rCandidate.mnCropRight = rCrop.GetRight();
        rCandidate.mnCropBottom = rCrop.GetBottom();
        return;
    }

    if (rCandidate.mnCropLeft != rCrop.GetLeft() || rCandidate.mnCropTop != rCrop.GetTop()
        || rCandidate.mnCropRight != rCrop.GetRight()
        || rCandidate.mnCropBottom != rCrop.GetBottom())
        rCandidate.mbMixedCrop = true;
}

/** True when the part of the bitmap that is shown holds more pixels than the largest drawing of it
    asks for at the target resolution, by enough of a margin to be worth a row. The pixels a crop
    hides are not drawn at all, so they are left out of the comparison. */
bool isOverTarget(const LargeImageCandidate& rCandidate, sal_Int32 nTargetDPI)
{
    const tools::Long nTargetWidth
        = getTargetPixelCount(rCandidate.maMaxLogicSize.Width(), nTargetDPI);
    const tools::Long nTargetHeight
        = getTargetPixelCount(rCandidate.maMaxLogicSize.Height(), nTargetDPI);
    if (nTargetWidth <= 0 || nTargetHeight <= 0)
        return false;

    const Size aPixelSize = rCandidate.maGraphic.GetSizePixel();
    const double fVisibleWidth = double(aPixelSize.Width()) * getVisibleWidthFraction(rCandidate);
    const double fVisibleHeight
        = double(aPixelSize.Height()) * getVisibleHeightFraction(rCandidate);

    return fVisibleWidth > gfPixelTolerance * double(nTargetWidth)
           || fVisibleHeight > gfPixelTolerance * double(nTargetHeight);
}

/** A finding about one standard page as a whole. */
class SlideFinding : public LintFinding
{
public:
    SlideFinding(SdDrawDocument& rDoc, LintCategory eCategory, sal_Int32 nSlideIndex)
        : LintFinding(rDoc, eCategory, 0, nSlideIndex)
    {
    }

    bool canGoto() const override { return mnSlideIndex >= 0; }

    void goTo(ViewShellBase& rViewShellBase) override
    {
        showSlide(rViewShellBase, mnSlideIndex, nullptr);
    }
};

/** A hidden slide, which the fix takes out of the deck along with its notes page. */
class HiddenSlideFinding final : public SlideFinding
{
public:
    HiddenSlideFinding(SdDrawDocument& rDoc, sal_Int32 nSlideIndex, SdPage* pPage)
        : SlideFinding(rDoc, LintCategory::HiddenSlide, nSlideIndex)
        , mxPage(pPage)
    {
    }

    void fix(ViewShellBase& rViewShellBase) override
    {
        // A deck has to keep one slide, and the slide has to be one the document still holds,
        // which the slide says of itself. Only then is it asked whether it is still the hidden one
        // the scan reported. This is the order the findings next to this one go in.
        const rtl::Reference<SdPage> xPage = mxPage.get();
        if (mrDoc.GetSdPageCount(PageKind::Standard) < 2 || !xPage)
            return;

        if (!xPage->IsInserted() || !xPage->IsExcluded())
            return;

        endTextEdit(rViewShellBase);

        std::shared_ptr<ViewShell> pViewShell = rViewShellBase.GetMainViewShell();
        auto* pDrawViewShell = dynamic_cast<DrawViewShell*>(pViewShell.get());
        const bool bWasCurrentPage
            = pDrawViewShell && pDrawViewShell->getCurrentPage() == xPage.get();

        // The number of the slide tells the view where to go once the slide is gone, and it is read
        // off here, while the slide is still in the deck.
        const sal_Int32 nSlideIndex
            = bWasCurrentPage ? detail::getSlideIndexOfPage(mrDoc, xPage.get()) : -1;

        // Numbering here is over every page of the model, standard and notes together, not over the
        // slides alone.
        const sal_uInt16 nPageNum = xPage->GetPageNum();
        auto* pNotesPage = static_cast<SdPage*>(mrDoc.GetPage(nPageNum + 1));
        if (pNotesPage && pNotesPage->GetPageKind() != PageKind::Notes)
            pNotesPage = nullptr;

        const bool bUndo = mrDoc.IsUndoEnabled();
        if (bUndo)
        {
            // The notes page has to be recorded before the slide it belongs to, so that undo puts
            // the slide back first and the notes page finds its place after it.
            mrDoc.BegUndo(SdResId(STR_UNDO_DELETEPAGES));
            if (pNotesPage)
                mrDoc.AddUndo(SdrUndoFactory::CreateUndoDeletePage(*pNotesPage));
            mrDoc.AddUndo(SdrUndoFactory::CreateUndoDeletePage(*xPage));
        }

        // Taking the slide out moves its notes page down into the number the slide had, so the same
        // number is removed a second time.
        mrDoc.RemovePage(nPageNum);
        if (pNotesPage)
            mrDoc.RemovePage(nPageNum);

        if (bUndo)
            mrDoc.EndUndo();

        mxPage.clear();

        if (!bWasCurrentPage)
            return;

        // The slides that followed the deleted one each moved down a place, so the number it
        // had now names the slide that followed it, and the last slide is shown when it was the
        // last one.
        const sal_uInt16 nPageCount = mrDoc.GetSdPageCount(PageKind::Standard);
        if (nPageCount > 0)
            pDrawViewShell->SwitchPage(
                std::min(sal_uInt16(nSlideIndex), sal_uInt16(nPageCount - 1)));
    }

private:
    /** The hidden slide. Empty once the fix has taken it out of the document, and empty once the
        document has let go of the page for good. */
    unotools::WeakReference<SdPage> mxPage;
};

/** Speaker notes on a slide, which the fix empties out. */
class NotesFinding final : public SlideFinding
{
public:
    NotesFinding(SdDrawDocument& rDoc, sal_Int32 nSlideIndex, SdPage* pNotesPage)
        : SlideFinding(rDoc, LintCategory::NotesContent, nSlideIndex)
        , mxNotesPage(pNotesPage)
    {
    }

    /** The cleanup empties the text of one notes placeholder. The slides, the objects on them and
        the notes of every other slide are all left as they were. */
    bool invalidatesOtherFindings() const override { return false; }

    void fix(ViewShellBase& rViewShellBase) override
    {
        // A notes page the document no longer holds has nothing left to empty out.
        const rtl::Reference<SdPage> xNotesPage = mxNotesPage.get();
        if (!xNotesPage || !xNotesPage->IsInserted())
            return;

        SdrObject* pObject = xNotesPage->GetPresObj(PresObjKind::Notes);
        auto* pTextObject = dynamic_cast<SdrTextObj*>(pObject);
        if (!pTextObject)
            return;

        endTextEdit(rViewShellBase);

        if (mrDoc.IsUndoEnabled())
            mrDoc.AddUndo(mrDoc.GetSdrUndoFactory().CreateUndoObjectSetText(*pObject, 0));

        pTextObject->SetText(OUString());

        // An empty notes placeholder shows the words that invite the user to type into it again.
        pObject->SetEmptyPresObj(true);
    }

private:
    /** The notes page. Empty once the document has let go of the page for good. */
    unotools::WeakReference<SdPage> mxNotesPage;
};

/** A master slide no slide of the deck builds on, which the fix takes out of the document. */
class UnusedMasterFinding final : public LintFinding
{
public:
    UnusedMasterFinding(SdDrawDocument& rDoc, SdPage* pMasterPage)
        : LintFinding(rDoc, LintCategory::UnusedMaster, 0, -1)
        , mxMasterPage(pMasterPage)
    {
        maFacts.maName = pMasterPage ? pMasterPage->GetName() : OUString();
    }

    void fix(ViewShellBase&) override
    {
        const rtl::Reference<SdPage> xMasterPage = mxMasterPage.get();
        if (!xMasterPage || !isMasterPageOfDocument(mrDoc, xMasterPage.get())
            || mrDoc.GetMasterPageUserCount(xMasterPage.get()) != 0)
            return;

        // Every master a document is loaded with comes in marked precious, and a precious master is
        // one that is not to be dropped automatically. A click on this row is not automatic, so the
        // mark comes off before the master goes. A master that was made during the session carries
        // no mark, so the value it had is read off first and put back later.
        const bool bWasPrecious = xMasterPage->IsPrecious();
        xMasterPage->SetPrecious(false);

        // The document builds all the undo actions this needs, both for the pages it takes out and
        // for the layout stylesheets that go with them.
        mrDoc.RemoveUnnecessaryMasterPages(xMasterPage.get(), false, true);

        // The removal declines for a master that has no notes master of its own. The master is then
        // still in the document, so it gets the mark it had back and its row stands.
        if (isMasterPageOfDocument(mrDoc, xMasterPage.get()))
        {
            xMasterPage->SetPrecious(bWasPrecious);
            return;
        }

        // The mark goes back to the value it had when the cleanup is taken back, alongside the
        // pages the removal recorded. This action is recorded after them, so undo runs it first
        // and redo last.
        if (mrDoc.IsUndoEnabled())
            mrDoc.AddUndo(
                std::make_unique<LintUndoMasterPrecious>(mrDoc, *xMasterPage, bWasPrecious));

        mxMasterPage.clear();
    }

private:
    /** The master slide. Empty once the fix has taken it out of the document, and empty once the
        document has let go of the page for good. */
    unotools::WeakReference<SdPage> mxMasterPage;
};

/** A finding about one or more drawing objects that all share the same problem. The objects are
    held weakly, so one the document has released since the scan comes back as an empty reference
    rather than as a pointer to memory that is no longer an object. */
class ObjectFinding : public LintFinding
{
public:
    ObjectFinding(SdDrawDocument& rDoc, LintCategory eCategory, sal_uInt64 nCurrentBytes,
                  sal_Int32 nSlideIndex, SdrObject* pObject)
        : LintFinding(rDoc, eCategory, nCurrentBytes, nSlideIndex)
        , maObjects{ unotools::WeakReference<SdrObject>(pObject) }
    {
    }

    ObjectFinding(SdDrawDocument& rDoc, LintCategory eCategory, sal_uInt64 nCurrentBytes,
                  sal_Int32 nSlideIndex, const std::vector<SdrObject*>& rObjects)
        : LintFinding(rDoc, eCategory, nCurrentBytes, nSlideIndex)
        , maObjects(rObjects.begin(), rObjects.end())
    {
    }

    bool canGoto() const override { return mnSlideIndex >= 0 && !maObjects.empty(); }

    void goTo(ViewShellBase& rViewShellBase) override
    {
        showSlide(rViewShellBase, mnSlideIndex, maObjects.front().get().get());
    }

protected:
    /** Every object that draws the thing the finding is about. Emptied once a fix has replaced
        them. */
    std::vector<unotools::WeakReference<SdrObject>> maObjects;
};

/** An image the document keeps more of than it shows: either more pixels than it is drawn at, or
    pixels a crop hides. The fix encodes what is shown of it again at the target resolution. */
class ImageFinding final : public ObjectFinding
{
public:
    ImageFinding(SdDrawDocument& rDoc, LintCategory eCategory,
                 const LargeImageCandidate& rCandidate, const LintOptions& rOptions,
                 std::shared_ptr<LintMeasureCache> pMeasureCache)
        : ObjectFinding(rDoc, eCategory, rCandidate.mnCurrentBytes,
                        rCandidate.mnWorstSlideIndex, rCandidate.maObjects)
        , maGraphic(rCandidate.maGraphic)
        , maLogicSize(rCandidate.maMaxLogicSize)
        , mxWorstObject(rCandidate.mpWorstObject)
        , mnTargetDPI(rOptions.mnImageResolution)
        , mnJPEGQuality(rOptions.mnJPEGQuality)
        , mnChecksum(rCandidate.mnChecksum)
        , mnCropLeft(rCandidate.mnCropLeft)
        , mnCropTop(rCandidate.mnCropTop)
        , mnCropRight(rCandidate.mnCropRight)
        , mnCropBottom(rCandidate.mnCropBottom)
        , mpMeasureCache(std::move(pMeasureCache))
    {
        if (eCategory == LintCategory::CroppedImage)
            maFacts.mnHiddenPercent
                = sal_Int32(std::lround(getHiddenAreaShare(rCandidate) * 100.0));
        else
            maFacts.mnEffectiveDPI = rCandidate.mnMaxEffectiveDPI;
    }

    /** Carries the number that tells two images of one place apart. The figures the detail line
        is written from hold whichever of them the row stands for. */
    void setRowNumber(sal_Int32 nRowNumber) override { maFacts.mnImageNumber = nRowNumber; }

    bool canGoto() const override { return mnSlideIndex >= 0 && mxWorstObject.get().is(); }

    /** Shows the object that draws the image at the highest resolution, which is the one the row
        text names. */
    void goTo(ViewShellBase& rViewShellBase) override
    {
        showSlide(rViewShellBase, mnSlideIndex, mxWorstObject.get().get());
    }

    bool needsMeasuring() const override { return true; }

    /** One row stands for every object that draws the one bitmap, so the cleanup hands the new
        encoding to those objects and to nothing else. The pages, the masters and the order of the
        slides all stay as they were. */
    bool invalidatesOtherFindings() const override { return false; }

    bool measureSaving() override
    {
        // Without encoded data of its own the image has no size the document spends on it today,
        // and a saving is the difference from that size. The row stands as one the reader can still
        // act on, and it carries no figure.
        if (mnCurrentBytes == 0)
            return true;

        return ensureCompressed();
    }

    sal_uInt64 getSavingBytes() const override
    {
        if (!moCompressed || !moCompressed->mbSmaller
            || mnCurrentBytes <= moCompressed->getByteCount())
            return 0;

        return mnCurrentBytes - moCompressed->getByteCount();
    }

    sal_uInt64 getRealizedSavingBytes() const override { return mnRealizedSavingBytes; }

    /** Encodes the image again if that has not happened yet, and reads the new encoding back as
        the one graphic every object that draws the image is handed. Both halves are the slow part
        of the cleanup, and both work from the bytes this finding holds rather than from the
        document. */
    bool prepareFix() override
    {
        if (maObjects.empty() || !ensureCompressed())
            return false;

        if (maCompressedGraphic.IsNone())
            maCompressedGraphic = importCompressedImage(*moCompressed);

        return !maCompressedGraphic.IsNone();
    }

    void fix(ViewShellBase& rViewShellBase) override
    {
        if (!prepareFix())
            return;

        endTextEdit(rViewShellBase);

        // Every object drawing this image gets the one new encoding, so the document keeps a single
        // copy of the pixels.
        sal_Int32 nReplaced = 0;
        for (const unotools::WeakReference<SdrObject>& rxOldObject : maObjects)
        {
            const rtl::Reference<SdrObject> xOldObject = rxOldObject.get();
            if (!isObjectOnPage(xOldObject.get()))
                continue;

            SdrModel& rModel = xOldObject->getSdrModelFromSdrObject();
            rtl::Reference<SdrObject> pNewObject = xOldObject->CloneSdrObject(rModel);
            auto* pNewGraphicObject = dynamic_cast<SdrGrafObj*>(pNewObject.get());
            if (!pNewGraphicObject)
                continue;

            pNewGraphicObject->SetGraphic(maCompressedGraphic);

            // The new encoding holds only the pixels the crop left showing, so the object shows
            // the whole of what it is now given. Dropping the crop attribute leaves it at the
            // value a picture nothing is cropped off carries, which is zero on every edge. The
            // size the object takes on the slide is the size the crop already left it at, and
            // handing it a graphic does not change that.
            if (isCropped())
                pNewGraphicObject->ClearMergedItem(SDRATTR_GRAFCROP);

            replaceObject(*xOldObject, *pNewObject);
            ++nReplaced;
        }

        // The document holds one copy of the encoded image however many objects draw it, so the
        // space freed up counts once for the whole image rather than once per object.
        if (nReplaced > 0)
            mnRealizedSavingBytes = getSavingBytes();

        maObjects.clear();
        mxWorstObject.clear();

        // The pixels of the new encoding are held only for as long as there is an object left to
        // hand them to.
        maCompressedGraphic.Clear();
    }

private:
    /** Encodes the image again at the target resolution, once, and keeps the result. Answers
        whether that encoding came out smaller than the data the document holds. */
    bool ensureCompressed()
    {
        if (!moCompressed && !takeFromCache())
        {
            // The crop is baked in here rather than when the row was written, so an image whose
            // measurement the cache already holds never pays for it.
            moCompressed = compressGraphic(croppedGraphic(), getSourceBytes(), maLogicSize,
                                           mnTargetDPI, mnJPEGQuality);
            storeInCache();
        }

        // An encoding that comes out no smaller than the data the document already holds would cost
        // quality for nothing, so there is no saving to report and nothing worth doing.
        return moCompressed->mbSmaller;
    }

    /** True when a crop keeps part of the image off the slide. */
    bool isCropped() const
    {
        return mnCropLeft != 0 || mnCropTop != 0 || mnCropRight != 0 || mnCropBottom != 0;
    }

    /** The part of the image the slide shows, as a picture in its own right. An image nothing is
        cropped off comes back as it is. */
    Graphic croppedGraphic() const
    {
        if (!isCropped())
            return maGraphic;

        // The attribute carries the crop and nothing else, so the picture is trimmed to what is
        // shown and every pixel that is left keeps the colour it had.
        GraphicAttr aAttr;
        aAttr.SetCrop(mnCropLeft, mnCropTop, mnCropRight, mnCropBottom);

        return GraphicObject(maGraphic).GetTransformedGraphic(maLogicSize,
                                                              MapMode(MapUnit::Map100thMM), aAttr);
    }

    /** How many bytes the document spends on the image, which is the figure a new encoding is
        measured against. A byte count of zero means the document kept no encoded data for the
        image, and then the size of the raw bitmap is the only figure there is. It is used in place
        of the stored size, and it is larger than any encoding of the same pixels. */
    sal_uInt64 getSourceBytes() const
    {
        return mnCurrentBytes > 0 ? mnCurrentBytes : sal_uInt64(maGraphic.GetSizeBytes());
    }

    /** What the measurement of this image is worked out from. */
    LintMeasureKey getMeasureKey() const
    {
        return { mnChecksum, maLogicSize.Width(), maLogicSize.Height(), mnTargetDPI, mnJPEGQuality,
                 mnCropLeft, mnCropTop,           mnCropRight,          mnCropBottom };
    }

    /** Takes up the encoding an earlier measurement of this bitmap at this size and these settings
        kept, and answers whether there was one. Whether it is smaller is worked out against the
        bytes this image takes, because the one bitmap may sit behind links of several sizes. */
    bool takeFromCache()
    {
        if (moCompressed || !mpMeasureCache || mnChecksum == 0)
            return false;

        moCompressed = mpMeasureCache->find(getMeasureKey(), getSourceBytes());
        return moCompressed.has_value();
    }

    /** Hands the encoding this measurement produced to the cache, for a later scan of the same
        bitmap at the same settings to read. */
    void storeInCache()
    {
        if (mpMeasureCache && mnChecksum != 0 && moCompressed)
            mpMeasureCache->store(getMeasureKey(), *moCompressed);
    }

    Graphic maGraphic;

    /** The largest size the image is drawn at, so the fix keeps the target resolution for the user
        that shows the most detail. */
    Size maLogicSize;

    /** The object that draws the image at the highest resolution, which is the one the row text
        names. Empty once the fix has replaced it, and empty once the document has let go of it. */
    unotools::WeakReference<SdrObject> mxWorstObject;

    sal_Int32 mnTargetDPI;
    sal_Int32 mnJPEGQuality;

    /** Checksum of the bitmap, which is what tells one bitmap from another. */
    BitmapChecksum mnChecksum;

    /** How much of the image the slide keeps off at each edge, in hundredths of a millimetre. All
        four are zero for an image that is shown whole. */
    tools::Long mnCropLeft;
    tools::Long mnCropTop;
    tools::Long mnCropRight;
    tools::Long mnCropBottom;

    /** Where the encodings of measurements are kept, or an empty pointer when they are not kept at
        all. */
    std::shared_ptr<LintMeasureCache> mpMeasureCache;

    /** The image encoded again at the target resolution, worked out on first use. */
    std::optional<LintCompressedImage> moCompressed;

    /** That new encoding read back, ready for the objects to be handed. Empty until the fix has
        been prepared. */
    Graphic maCompressedGraphic;

    sal_uInt64 mnRealizedSavingBytes = 0;
};

/** An embedded object, which the fix puts its own picture in the place of. */
class OleFinding final : public ObjectFinding
{
public:
    OleFinding(SdDrawDocument& rDoc, sal_Int32 nSlideIndex, SdrOle2Obj* pOleObject,
               bool bHasGraphic)
        : ObjectFinding(rDoc, LintCategory::OleObject, 0, nSlideIndex, pOleObject)
        , mbHasGraphic(bHasGraphic)
    {
    }

    bool canFix() const override { return mbHasGraphic; }

    void fix(ViewShellBase& rViewShellBase) override
    {
        if (maObjects.empty())
            return;

        const rtl::Reference<SdrObject> xObject = maObjects.front().get();
        auto* pOleObject = dynamic_cast<SdrOle2Obj*>(xObject.get());
        if (!isObjectOnPage(pOleObject))
            return;

        const Graphic* pGraphic = pOleObject->GetGraphic();
        if (!pGraphic)
            return;

        endTextEdit(rViewShellBase);

        SdrModel& rModel = pOleObject->getSdrModelFromSdrObject();
        rtl::Reference<SdrGrafObj> pNewObject
            = new SdrGrafObj(rModel, *pGraphic, pOleObject->GetLogicRect());
        pNewObject->SetLayer(pOleObject->GetLayer());
        pNewObject->SetName(pOleObject->GetName());

        replaceObject(*pOleObject, *pNewObject);
        maObjects.clear();
    }

private:
    bool mbHasGraphic;
};

/** Gathers the users of every bitmap in the document, one entry per distinct bitmap. It makes no
    judgement about which bitmaps are too large, because that can only be told once every user of a
    bitmap is known. */
class LargeImageHandler final : public ModelTraverseHandler
{
public:
    explicit LargeImageHandler(const SlideIndexMap& rSlideIndexOfPage)
        : mrSlideIndexOfPage(rSlideIndexOfPage)
    {
    }

    /** The bitmaps that were met, in the order they were first met. */
    const std::vector<LargeImageCandidate>& getCandidates() const { return maCandidates; }

    void handleSdrObject(SdrObject* pObject) override
    {
        auto* pGraphicObject = dynamic_cast<SdrGrafObj*>(pObject);
        if (!pGraphicObject || pGraphicObject->GetGraphicType() != GraphicType::Bitmap)
            return;

        const Graphic& rGraphic = pGraphicObject->GetGraphicObject().GetGraphic();

        // A metafile or an SVG keeps its detail at every zoom level, so it is left as it is. This
        // holds for the graphic itself, so every user of it is skipped alike.
        if (rGraphic.isVectorGraphic())
            return;

        // An animation is a series of frames, and its size in pixels is the size of one frame. A
        // figure worked out from that one frame would promise a saving the whole animation cannot
        // give, and encoding it again would leave a still picture in the animation's place. So an
        // animated graphic is left as it is.
        if (rGraphic.IsAnimated())
            return;

        // Every object drawing the bitmap joins the entry, whether or not this one is drawn too
        // large, because the decision is about the bitmap and reaches all of its users at once.
        LargeImageCandidate& rCandidate = getCandidate(rGraphic);
        const bool bFirstObject = rCandidate.maObjects.empty();
        rCandidate.maObjects.push_back(pGraphicObject);

        // The crop of every object is taken in, including that of an object whose drawing size
        // gives nothing to work from, because the row stands for the bitmap and all of them.
        recordCrop(rCandidate, pGraphicObject->GetMergedItem(SDRATTR_GRAFCROP), bFirstObject);

        const Size aPixelSize = rGraphic.GetSizePixel();
        const Size aLogicSize = pGraphicObject->GetLogicRect().GetSize();
        if (aPixelSize.Width() <= 0 || aPixelSize.Height() <= 0 || aLogicSize.Width() <= 0
            || aLogicSize.Height() <= 0)
        {
            rCandidate.mbUnmeasured = true;
            return;
        }

        // The drawing size is in hundredths of a millimetre. Converting it to inches gives the
        // resolution the image is drawn at without asking any output device about itself. The
        // pixels a crop hides are not drawn, so the resolution counts the pixels that are.
        const double fWidthInInches
            = o3tl::convert(double(aLogicSize.Width()), o3tl::Length::mm100, o3tl::Length::in);
        const double fHeightInInches
            = o3tl::convert(double(aLogicSize.Height()), o3tl::Length::mm100, o3tl::Length::in);
        const double fVisibleWidth
            = double(aPixelSize.Width()) * getVisibleWidthFraction(rCandidate);
        const double fVisibleHeight
            = double(aPixelSize.Height()) * getVisibleHeightFraction(rCandidate);
        const sal_Int32 nEffectiveDPI = std::max(sal_Int32(fVisibleWidth / fWidthInInches),
                                                 sal_Int32(fVisibleHeight / fHeightInInches));

        rCandidate.maMaxLogicSize.setWidth(
            std::max(rCandidate.maMaxLogicSize.Width(), aLogicSize.Width()));
        rCandidate.maMaxLogicSize.setHeight(
            std::max(rCandidate.maMaxLogicSize.Height(), aLogicSize.Height()));

        if (nEffectiveDPI > rCandidate.mnMaxEffectiveDPI)
        {
            rCandidate.mnMaxEffectiveDPI = nEffectiveDPI;
            rCandidate.mpWorstObject = pGraphicObject;
            rCandidate.mnWorstSlideIndex
                = lookUpSlideIndex(mrSlideIndexOfPage, pObject->getSdrPageFromSdrObject());
        }
    }

private:
    const SlideIndexMap& mrSlideIndexOfPage;
    std::vector<LargeImageCandidate> maCandidates;
    std::unordered_map<BitmapChecksum, size_t> maIndexByChecksum;

    /** The entry for the given bitmap, started off with the bitmap and its stored size the first
        time that bitmap turns up. */
    LargeImageCandidate& getCandidate(const Graphic& rGraphic)
    {
        const BitmapChecksum nChecksum = rGraphic.GetChecksum();
        auto aExisting = maIndexByChecksum.find(nChecksum);
        if (aExisting != maIndexByChecksum.end())
            return maCandidates[aExisting->second];

        maIndexByChecksum.emplace(nChecksum, maCandidates.size());
        LargeImageCandidate& rCandidate = maCandidates.emplace_back();
        rCandidate.maGraphic = rGraphic;
        rCandidate.mnChecksum = nChecksum;
        rCandidate.maBitmapLogicSize = getBitmapLogicSize(rGraphic);

        // The document only knows how many bytes an image takes when it kept the encoded data it
        // was loaded from. Without that the row leaves the size out. Sharing the link reads the
        // byte count off the encoded data as it lies, so the count itself costs no decoding.
        const std::shared_ptr<GfxLink>& pGfxLink = rGraphic.GetSharedGfxLink();
        if (pGfxLink)
            rCandidate.mnCurrentBytes = pGfxLink->GetDataSize();

        return rCandidate;
    }
};

/** Collects one finding per embedded object. */
class OleHandler final : public ModelTraverseHandler
{
public:
    OleHandler(SdDrawDocument& rDoc, const SlideIndexMap& rSlideIndexOfPage)
        : mrDoc(rDoc)
        , mrSlideIndexOfPage(rSlideIndexOfPage)
    {
    }

    /** One finding per embedded object that was met, in the order they were met. */
    std::vector<std::shared_ptr<LintFinding>>& getFindings() { return maFindings; }

    void handleSdrObject(SdrObject* pObject) override
    {
        auto* pOleObject = dynamic_cast<SdrOle2Obj*>(pObject);
        if (!pOleObject)
            return;

        // The rule speaks about the slides of the deck, and its row names the slide an object sits
        // on. An embedded object on a master slide, or on a page that is no slide of the deck, has
        // no such number and is left out.
        const SdrPage* pPage = pObject->getSdrPageFromSdrObject();
        if (!pPage || pPage->IsMasterPage())
            return;

        const sal_Int32 nSlideIndex = lookUpSlideIndex(mrSlideIndexOfPage, pPage);
        if (nSlideIndex < 0)
            return;

        // An embedded object without a replacement graphic has nothing to turn into a picture, so
        // it offers no cleanup, and the row the client writes says as much.
        const bool bHasGraphic = pOleObject->GetGraphic() != nullptr;

        maFindings.push_back(
            std::make_shared<OleFinding>(mrDoc, nSlideIndex, pOleObject, bHasGraphic));
    }

private:
    SdDrawDocument& mrDoc;
    const SlideIndexMap& mrSlideIndexOfPage;
    std::vector<std::shared_ptr<LintFinding>> maFindings;
};

/** Picks out the bitmaps the document keeps more of than it shows and writes a row for each of
    them. Which bitmaps those are can only be told once every user of every bitmap is known. */
void appendImageFindings(SdDrawDocument& rDoc, const LintOptions& rOptions,
                         const std::shared_ptr<LintMeasureCache>& rpMeasureCache,
                         const std::vector<LargeImageCandidate>& rCandidates,
                         std::vector<std::shared_ptr<LintFinding>>& rFindings)
{
    for (const LargeImageCandidate& rCandidate : rCandidates)
    {
        // The document keeps one encoding of a bitmap however many objects draw it, and one row
        // stands for all of them. So a row is written for a bitmap whose users agree about which
        // part of it is shown and each of whom is drawn at a size that gives a resolution to work
        // from. Users that disagree about the crop would each need an encoding of their own, which
        // can leave the document larger than it is now, so such a bitmap is left alone.
        if (rCandidate.mbMixedCrop || rCandidate.mbUnmeasured)
            continue;

        // A crop that hides a good part of the picture is worth a row on its own account, whatever
        // resolution what is left is stored at. Below that share the row is about the pixels alone,
        // and its cleanup bakes in the little that is cropped as it encodes the image again.
        const bool bHeavilyCropped = getHiddenAreaShare(rCandidate) >= gfCroppedAreaThreshold;
        if (!bHeavilyCropped && !isOverTarget(rCandidate, rOptions.mnImageResolution))
            continue;

        const LintCategory eCategory
            = bHeavilyCropped ? LintCategory::CroppedImage : LintCategory::LargeImage;

        rFindings.push_back(std::make_shared<ImageFinding>(rDoc, eCategory, rCandidate, rOptions,
                                                          rpMeasureCache));
    }
}

/** True when the object carries at least one paragraph with something other than spaces in it. */
bool hasVisibleText(SdrObject* pObject)
{
    if (!pObject || !pObject->IsSdrTextObj())
        return false;

    // An untouched placeholder still holds the words that invite the user to type into it, so the
    // text it carries only counts once the placeholder has been filled in.
    if (pObject->IsEmptyPresObj())
        return false;

    OutlinerParaObject* pParaObject = static_cast<SdrTextObj*>(pObject)->GetOutlinerParaObject();
    if (!pParaObject)
        return false;

    const EditTextObject& rTextObject = pParaObject->GetTextObject();
    for (sal_Int32 nParagraph = 0; nParagraph < rTextObject.GetParagraphCount(); ++nParagraph)
    {
        if (!o3tl::trim(rTextObject.GetText(nParagraph)).empty())
            return true;
    }

    return false;
}

} // end of anonymous namespace

namespace detail
{
void collectObjectFindings(SdDrawDocument& rDoc, const LintOptions& rOptions,
                           const std::shared_ptr<LintMeasureCache>& rpMeasureCache,
                           std::vector<std::shared_ptr<LintFinding>>& rFindings)
{
    const bool bWantLargeImages = isCategoryScanned(rOptions, LintCategory::LargeImage);
    const bool bWantOleObjects = isCategoryScanned(rOptions, LintCategory::OleObject);
    if (!bWantLargeImages && !bWantOleObjects)
        return;

    const SlideIndexMap aSlideIndexOfPage = buildSlideIndexMap(rDoc);

    // The two rules read the same drawing objects, so one walk over the document feeds them both.
    // The master slides are part of the walk for the sake of the images, which are reported
    // wherever they sit.
    ModelTraverser aModelTraverser(&rDoc, { .mbMasterPages = bWantLargeImages });

    std::shared_ptr<LargeImageHandler> pImageHandler;
    if (bWantLargeImages)
    {
        pImageHandler = std::make_shared<LargeImageHandler>(aSlideIndexOfPage);
        aModelTraverser.addNodeHandler(pImageHandler);
    }

    std::shared_ptr<OleHandler> pOleHandler;
    if (bWantOleObjects)
    {
        pOleHandler = std::make_shared<OleHandler>(rDoc, aSlideIndexOfPage);
        aModelTraverser.addNodeHandler(pOleHandler);
    }

    aModelTraverser.traverse();

    if (pImageHandler)
        appendImageFindings(rDoc, rOptions, rpMeasureCache, pImageHandler->getCandidates(),
                            rFindings);

    // The rows about the embedded objects follow the rows about the images.
    if (pOleHandler)
    {
        for (std::shared_ptr<LintFinding>& rpFinding : pOleHandler->getFindings())
            rFindings.push_back(std::move(rpFinding));
    }
}

void collectHiddenSlides(SdDrawDocument& rDoc, std::vector<std::shared_ptr<LintFinding>>& rFindings)
{
    const sal_uInt16 nPageCount = rDoc.GetSdPageCount(PageKind::Standard);

    // A one-slide deck has nothing to spare, so hiding that slide is not worth a row.
    if (nPageCount < 2)
        return;

    for (sal_uInt16 nPage = 0; nPage < nPageCount; ++nPage)
    {
        SdPage* pPage = rDoc.GetSdPage(nPage, PageKind::Standard);
        if (!pPage || !pPage->IsExcluded())
            continue;

        rFindings.push_back(
            std::make_shared<HiddenSlideFinding>(rDoc, sal_Int32(nPage), pPage));
    }
}

void collectUnusedMasters(SdDrawDocument& rDoc,
                          std::vector<std::shared_ptr<LintFinding>>& rFindings)
{
    // Every page names the one master it builds on, so a single walk over the pages of the document
    // gathers the masters that are in use, and each master is then looked up among them. Numbering
    // here is over every page of the model, standard and notes together.
    o3tl::sorted_vector<const SdrPage*> aUsedMasters;
    const sal_uInt16 nPageCount = rDoc.GetPageCount();
    for (sal_uInt16 nPage = 0; nPage < nPageCount; ++nPage)
    {
        const SdrPage* pPage = rDoc.GetPage(nPage);
        if (pPage && pPage->TRG_HasMasterPage())
            aUsedMasters.insert(&pPage->TRG_GetMasterPage());
    }

    const sal_uInt16 nMasterCount = rDoc.GetMasterSdPageCount(PageKind::Standard);
    for (sal_uInt16 nMaster = 0; nMaster < nMasterCount; ++nMaster)
    {
        SdPage* pMasterPage = rDoc.GetMasterSdPage(nMaster, PageKind::Standard);
        if (!pMasterPage || aUsedMasters.contains(pMasterPage))
            continue;

        rFindings.push_back(std::make_shared<UnusedMasterFinding>(rDoc, pMasterPage));
    }
}

void collectNotesContent(SdDrawDocument& rDoc, std::vector<std::shared_ptr<LintFinding>>& rFindings)
{
    const sal_uInt16 nPageCount = rDoc.GetSdPageCount(PageKind::Standard);
    for (sal_uInt16 nPage = 0; nPage < nPageCount; ++nPage)
    {
        SdPage* pNotesPage = rDoc.GetSdPage(nPage, PageKind::Notes);
        if (!pNotesPage || !hasVisibleText(pNotesPage->GetPresObj(PresObjKind::Notes)))
            continue;

        rFindings.push_back(std::make_shared<NotesFinding>(rDoc, sal_Int32(nPage), pNotesPage));
    }
}

} // end of namespace detail

} // end of namespace sd::lint

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
