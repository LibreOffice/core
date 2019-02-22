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

#include <AccessibleDocumentPagePreview.hxx>
#include <AccessiblePreviewTable.hxx>
#include <AccessiblePageHeader.hxx>
#include <AccessibilityHints.hxx>
#include <AccessibleText.hxx>
#include <document.hxx>
#include <prevwsh.hxx>
#include <prevloc.hxx>
#include <drwlayer.hxx>
#include <editsrc.hxx>
#include <scresid.hxx>
#include <strings.hrc>
#include <strings.hxx>
#include <preview.hxx>
#include <postit.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>

#include <unotools/accessiblestatesethelper.hxx>
#include <tools/gen.hxx>
#include <svx/fmview.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdobj.hxx>
#include <svx/AccessibleTextHelper.hxx>
#include <svx/AccessibleShape.hxx>
#include <svx/IAccessibleParent.hxx>
#include <svx/IAccessibleViewForwarder.hxx>
#include <svx/ShapeTypeHandler.hxx>
#include <toolkit/helper/convert.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/docfile.hxx>

#include <vector>
#include <algorithm>
#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

typedef std::vector< uno::Reference< XAccessible > > ScXAccVector;

struct ScAccNote
{
    OUString    maNoteText;
    tools::Rectangle   maRect;
    ScAddress   maNoteCell;
    ::accessibility::AccessibleTextHelper* mpTextHelper;
    sal_Int32   mnParaCount;
    bool    mbMarkNote;

    ScAccNote()
        : mpTextHelper(nullptr)
        , mnParaCount(0)
        , mbMarkNote(false)
    {
    }
};

class ScNotesChildren
{
public:
    ScNotesChildren(ScPreviewShell* pViewShell, ScAccessibleDocumentPagePreview* pAccDoc);
    ~ScNotesChildren();
    void Init(const tools::Rectangle& rVisRect, sal_Int32 nOffset);

    sal_Int32 GetChildrenCount() const { return mnParagraphs;}
    uno::Reference<XAccessible> GetChild(sal_Int32 nIndex) const;
    uno::Reference<XAccessible> GetAt(const awt::Point& rPoint) const;

    void DataChanged(const tools::Rectangle& rVisRect);

private:
    ScPreviewShell*         mpViewShell;
    ScAccessibleDocumentPagePreview* const mpAccDoc;
    typedef std::vector<ScAccNote> ScAccNotes;
    mutable ScAccNotes      maNotes;
    mutable ScAccNotes      maMarks;
    sal_Int32               mnParagraphs;
    sal_Int32               mnOffset;

    ::accessibility::AccessibleTextHelper* CreateTextHelper(const OUString& rString, const tools::Rectangle& rVisRect, const ScAddress& aCellPos, bool bMarkNote, sal_Int32 nChildOffset) const;
    sal_Int32 AddNotes(const ScPreviewLocationData& rData, const tools::Rectangle& rVisRect, bool bMark, ScAccNotes& rNotes);

    static sal_Int8 CompareCell(const ScAddress& aCell1, const ScAddress& aCell2);
    static void CollectChildren(const ScAccNote& rNote, ScXAccVector& rVector);
    sal_Int32 CheckChanges(const ScPreviewLocationData& rData, const tools::Rectangle& rVisRect,
        bool bMark, ScAccNotes& rOldNotes, ScAccNotes& rNewNotes,
        ScXAccVector& rOldParas, ScXAccVector& rNewParas);

    inline ScDocument* GetDocument() const;
};

ScNotesChildren::ScNotesChildren(ScPreviewShell* pViewShell, ScAccessibleDocumentPagePreview* pAccDoc)
    : mpViewShell(pViewShell),
    mpAccDoc(pAccDoc),
    mnParagraphs(0),
    mnOffset(0)
{
}

struct DeleteAccNote
{
    void operator()(ScAccNote& rNote)
    {
        if (rNote.mpTextHelper)
            DELETEZ( rNote.mpTextHelper);
    }
};

ScNotesChildren::~ScNotesChildren()
{
    std::for_each(maNotes.begin(), maNotes.end(), DeleteAccNote());
    std::for_each(maMarks.begin(), maMarks.end(), DeleteAccNote());
}

::accessibility::AccessibleTextHelper* ScNotesChildren::CreateTextHelper(const OUString& rString, const tools::Rectangle& rVisRect, const ScAddress& aCellPos, bool bMarkNote, sal_Int32 nChildOffset) const
{
    ::accessibility::AccessibleTextHelper* pTextHelper = new ::accessibility::AccessibleTextHelper(std::make_unique<ScAccessibilityEditSource>(std::make_unique<ScAccessibleNoteTextData>(mpViewShell, rString, aCellPos, bMarkNote)));
    pTextHelper->SetEventSource(mpAccDoc);
    pTextHelper->SetStartIndex(nChildOffset);
    pTextHelper->SetOffset(rVisRect.TopLeft());

    return pTextHelper;
}

sal_Int32 ScNotesChildren::AddNotes(const ScPreviewLocationData& rData, const tools::Rectangle& rVisRect, bool bMark, ScAccNotes& rNotes)
{
    sal_Int32 nCount = rData.GetNoteCountInRange(rVisRect, bMark);

    rNotes.reserve(nCount);

    sal_Int32 nParagraphs(0);
    ScDocument* pDoc = GetDocument();
    if (pDoc)
    {
        ScAccNote aNote;
        aNote.mbMarkNote = bMark;
        if (bMark)
            aNote.mnParaCount = 1;
        for (sal_Int32 nIndex = 0; nIndex < nCount; ++nIndex)
        {
            if (rData.GetNoteInRange(rVisRect, nIndex, bMark, aNote.maNoteCell, aNote.maRect))
            {
                if (bMark)
                {
                    // Document not needed, because only the cell address, but not the tablename is needed
                    aNote.maNoteText = aNote.maNoteCell.Format(ScRefFlags::VALID);
                }
                else
                {
                    if( ScPostIt* pNote = pDoc->GetNote( aNote.maNoteCell ) )
                        aNote.maNoteText = pNote->GetText();
                    aNote.mpTextHelper = CreateTextHelper(aNote.maNoteText, aNote.maRect, aNote.maNoteCell, aNote.mbMarkNote, nParagraphs + mnOffset);
                    if (aNote.mpTextHelper)
                        aNote.mnParaCount = aNote.mpTextHelper->GetChildCount();
                }
                nParagraphs += aNote.mnParaCount;
                rNotes.push_back(aNote);
            }
        }
    }
    return nParagraphs;
}

void ScNotesChildren::Init(const tools::Rectangle& rVisRect, sal_Int32 nOffset)
{
    if (mpViewShell && !mnParagraphs)
    {
        mnOffset = nOffset;
        const ScPreviewLocationData& rData = mpViewShell->GetLocationData();

        mnParagraphs = AddNotes(rData, rVisRect, false, maMarks);
        mnParagraphs += AddNotes(rData, rVisRect, true, maNotes);
    }
}

struct ScParaFound
{
    sal_Int32 mnIndex;
    explicit ScParaFound(sal_Int32 nIndex) : mnIndex(nIndex) {}
    bool operator() (const ScAccNote& rNote)
    {
        bool bResult(false);
        if (rNote.mnParaCount > mnIndex)
            bResult = true;
        else
            mnIndex -= rNote.mnParaCount;
        return bResult;
    }
};

uno::Reference<XAccessible> ScNotesChildren::GetChild(sal_Int32 nIndex) const
{
    uno::Reference<XAccessible> xAccessible;

    if (nIndex < mnParagraphs)
    {
        if (nIndex < static_cast<sal_Int32>(maMarks.size()))
        {
            ScAccNotes::iterator aEndItr = maMarks.end();
            ScParaFound aParaFound(nIndex);
            ScAccNotes::iterator aItr = std::find_if(maMarks.begin(), aEndItr, aParaFound);
            if (aItr != aEndItr)
            {
                OSL_ENSURE((aItr->maNoteCell == maMarks[nIndex].maNoteCell) && (aItr->mbMarkNote == maMarks[nIndex].mbMarkNote), "wrong note found");
                if (!aItr->mpTextHelper)
                    aItr->mpTextHelper = CreateTextHelper(maMarks[nIndex].maNoteText, maMarks[nIndex].maRect, maMarks[nIndex].maNoteCell, maMarks[nIndex].mbMarkNote, nIndex + mnOffset); // the marks are the first and every mark has only one paragraph
                xAccessible = aItr->mpTextHelper->GetChild(aParaFound.mnIndex + aItr->mpTextHelper->GetStartIndex());
            }
            else
            {
                OSL_FAIL("wrong note found");
            }
        }
        else
        {
            nIndex -= maMarks.size();
            ScAccNotes::iterator aEndItr = maNotes.end();
            ScParaFound aParaFound(nIndex);
            ScAccNotes::iterator aItr = std::find_if(maNotes.begin(), aEndItr, aParaFound);
            if (aEndItr != aItr)
            {
                if (!aItr->mpTextHelper)
                    aItr->mpTextHelper = CreateTextHelper(aItr->maNoteText, aItr->maRect, aItr->maNoteCell, aItr->mbMarkNote, (nIndex - aParaFound.mnIndex) + mnOffset + maMarks.size());
                xAccessible = aItr->mpTextHelper->GetChild(aParaFound.mnIndex + aItr->mpTextHelper->GetStartIndex());
            }
        }
    }

    return xAccessible;
}

struct ScPointFound
{
    tools::Rectangle const maPoint;
    sal_Int32 mnParagraphs;
    explicit ScPointFound(const Point& rPoint) : maPoint(rPoint, Size(0, 0)), mnParagraphs(0) {}
    bool operator() (const ScAccNote& rNote)
    {
        bool bResult(false);
        if (maPoint.IsInside(rNote.maRect))
            bResult = true;
        else
            mnParagraphs += rNote.mnParaCount;
        return bResult;
    }
};

uno::Reference<XAccessible> ScNotesChildren::GetAt(const awt::Point& rPoint) const
{
    uno::Reference<XAccessible> xAccessible;

    ScPointFound aPointFound(Point(rPoint.X, rPoint.Y));

    ScAccNotes::iterator aEndItr = maMarks.end();
    ScAccNotes::iterator aItr = std::find_if(maMarks.begin(), aEndItr, aPointFound);
    if (aEndItr == aItr)
    {
        aEndItr = maNotes.end();
        aItr = std::find_if(maNotes.begin(), aEndItr, aPointFound);
    }
    if (aEndItr != aItr)
    {
        if (!aItr->mpTextHelper)
            aItr->mpTextHelper = CreateTextHelper(aItr->maNoteText, aItr->maRect, aItr->maNoteCell, aItr->mbMarkNote, aPointFound.mnParagraphs + mnOffset);
        xAccessible = aItr->mpTextHelper->GetAt(rPoint);
    }

    return xAccessible;
}

sal_Int8 ScNotesChildren::CompareCell(const ScAddress& aCell1, const ScAddress& aCell2)
{
    OSL_ENSURE(aCell1.Tab() == aCell2.Tab(), "the notes should be on the same table");
    sal_Int8 nResult(0);
    if (aCell1 != aCell2)
    {
        if (aCell1.Row() == aCell2.Row())
            nResult = (aCell1.Col() < aCell2.Col()) ? -1 : 1;
        else
            nResult = (aCell1.Row() < aCell2.Row()) ? -1 : 1;
    }
    return nResult;
}

void ScNotesChildren::CollectChildren(const ScAccNote& rNote, ScXAccVector& rVector)
{
    if (rNote.mpTextHelper)
        for (sal_Int32 i = 0; i < rNote.mnParaCount; ++i)
            rVector.push_back(rNote.mpTextHelper->GetChild(i + rNote.mpTextHelper->GetStartIndex()));
}

sal_Int32 ScNotesChildren::CheckChanges(const ScPreviewLocationData& rData,
            const tools::Rectangle& rVisRect, bool bMark, ScAccNotes& rOldNotes,
            ScAccNotes& rNewNotes, ScXAccVector& rOldParas, ScXAccVector& rNewParas)
{
    sal_Int32 nCount = rData.GetNoteCountInRange(rVisRect, bMark);

    rNewNotes.reserve(nCount);

    sal_Int32 nParagraphs(0);
    ScDocument* pDoc = GetDocument();
    if (pDoc)
    {
        ScAccNote aNote;
        aNote.mbMarkNote = bMark;
        if (bMark)
            aNote.mnParaCount = 1;
        ScAccNotes::iterator aItr = rOldNotes.begin();
        ScAccNotes::iterator aEndItr = rOldNotes.end();
        bool bAddNote(false);
        for (sal_Int32 nIndex = 0; nIndex < nCount; ++nIndex)
        {
            if (rData.GetNoteInRange(rVisRect, nIndex, bMark, aNote.maNoteCell, aNote.maRect))
            {
                if (bMark)
                {
                    // Document not needed, because only the cell address, but not the tablename is needed
                    aNote.maNoteText = aNote.maNoteCell.Format(ScRefFlags::VALID);
                }
                else
                {
                    if( ScPostIt* pNote = pDoc->GetNote( aNote.maNoteCell ) )
                        aNote.maNoteText = pNote->GetText();
                }

                sal_Int8 nCompare(-1); // if there are no more old children it is always a new one
                if (aItr != aEndItr)
                    nCompare = CompareCell(aNote.maNoteCell, aItr->maNoteCell);
                if (nCompare == 0)
                {
                    if (aNote.maNoteText == aItr->maNoteText)
                    {
                        aNote.mpTextHelper = aItr->mpTextHelper;
                        if (aNote.maRect != aItr->maRect)  // set new VisArea
                        {
                            aNote.mpTextHelper->SetOffset(aNote.maRect.TopLeft());
                            aNote.mpTextHelper->UpdateChildren();
                            //OSL_ENSURE(aItr->maRect.GetSize() == aNote.maRect.GetSize(), "size should be the same, because the text is not changed");
                            // could be changed, because only a part of the note is visible
                        }
                    }
                    else
                    {
                        aNote.mpTextHelper = CreateTextHelper(aNote.maNoteText, aNote.maRect, aNote.maNoteCell, aNote.mbMarkNote, nParagraphs + mnOffset);
                        if (aNote.mpTextHelper)
                            aNote.mnParaCount = aNote.mpTextHelper->GetChildCount();
                        // collect removed children
                        CollectChildren(*aItr, rOldParas);
                        DELETEZ(aItr->mpTextHelper);
                        // collect new children
                        CollectChildren(aNote, rNewParas);
                    }
                    bAddNote = true;
                    // not necessary, because this branch should not be reached if it is the end
                    //if (aItr != aEndItr)
                    ++aItr;
                }
                else if (nCompare < 0)
                {
                    aNote.mpTextHelper = CreateTextHelper(aNote.maNoteText, aNote.maRect, aNote.maNoteCell, aNote.mbMarkNote, nParagraphs + mnOffset);
                    if (aNote.mpTextHelper)
                        aNote.mnParaCount = aNote.mpTextHelper->GetChildCount();
                    // collect new children
                    CollectChildren(aNote, rNewParas);
                    bAddNote = true;
                }
                else
                {
                    // collect removed children
                    CollectChildren(*aItr, rOldParas);
                    DELETEZ(aItr->mpTextHelper);

                    // no note to add
                    // not necessary, because this branch should not be reached if it is the end
                    //if (aItr != aEndItr)
                    ++aItr;
                }
                if (bAddNote)
                {
                    nParagraphs += aNote.mnParaCount;
                    rNewNotes.push_back(aNote);
                    bAddNote = false;
                }
            }
        }
    }
    return nParagraphs;
}

struct ScChildGone
{
    ScAccessibleDocumentPagePreview* const mpAccDoc;
    explicit ScChildGone(ScAccessibleDocumentPagePreview* pAccDoc) : mpAccDoc(pAccDoc) {}
    void operator() (const uno::Reference<XAccessible>& xAccessible) const
    {
        if (mpAccDoc)
        {
            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::CHILD;
            aEvent.Source = uno::Reference< XAccessibleContext >(mpAccDoc);
            aEvent.OldValue <<= xAccessible;

            mpAccDoc->CommitChange(aEvent); // gone child - event
        }
    }
};

struct ScChildNew
{
    ScAccessibleDocumentPagePreview* const mpAccDoc;
    explicit ScChildNew(ScAccessibleDocumentPagePreview* pAccDoc) : mpAccDoc(pAccDoc) {}
    void operator() (const uno::Reference<XAccessible>& xAccessible) const
    {
        if (mpAccDoc)
        {
            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::CHILD;
            aEvent.Source = uno::Reference< XAccessibleContext >(mpAccDoc);
            aEvent.NewValue <<= xAccessible;

            mpAccDoc->CommitChange(aEvent); // new child - event
        }
    }
};

void ScNotesChildren::DataChanged(const tools::Rectangle& rVisRect)
{
    if (mpViewShell && mpAccDoc)
    {
        ScXAccVector aNewParas;
        ScXAccVector aOldParas;
        ScAccNotes aNewMarks;
        mnParagraphs = CheckChanges(mpViewShell->GetLocationData(), rVisRect, true, maMarks, aNewMarks, aOldParas, aNewParas);
        maMarks = aNewMarks;
        ScAccNotes aNewNotes;
        mnParagraphs += CheckChanges(mpViewShell->GetLocationData(), rVisRect, false, maNotes, aNewNotes, aOldParas, aNewParas);
        maNotes = aNewNotes;

        std::for_each(aOldParas.begin(), aOldParas.end(), ScChildGone(mpAccDoc));
        std::for_each(aNewParas.begin(), aNewParas.end(), ScChildNew(mpAccDoc));
    }
}

inline ScDocument* ScNotesChildren::GetDocument() const
{
    ScDocument* pDoc = nullptr;
    if (mpViewShell)
        pDoc = &mpViewShell->GetDocument();
    return pDoc;
}

class ScIAccessibleViewForwarder : public ::accessibility::IAccessibleViewForwarder
{
public:
    ScIAccessibleViewForwarder();
    ScIAccessibleViewForwarder(ScPreviewShell* pViewShell,
                                ScAccessibleDocumentPagePreview* pAccDoc,
                                const MapMode& aMapMode);

    ///=====  IAccessibleViewForwarder  ========================================

    virtual tools::Rectangle GetVisibleArea() const override;
    virtual Point LogicToPixel (const Point& rPoint) const override;
    virtual Size LogicToPixel (const Size& rSize) const override;

private:
    ScPreviewShell*                     mpViewShell;
    ScAccessibleDocumentPagePreview*    mpAccDoc;
    MapMode                             maMapMode;
};

ScIAccessibleViewForwarder::ScIAccessibleViewForwarder()
    : mpViewShell(nullptr), mpAccDoc(nullptr)
{
}

ScIAccessibleViewForwarder::ScIAccessibleViewForwarder(ScPreviewShell* pViewShell,
                                ScAccessibleDocumentPagePreview* pAccDoc,
                                const MapMode& aMapMode)
    : mpViewShell(pViewShell),
    mpAccDoc(pAccDoc),
    maMapMode(aMapMode)
{
}

///=====  IAccessibleViewForwarder  ========================================

tools::Rectangle ScIAccessibleViewForwarder::GetVisibleArea() const
{
    SolarMutexGuard aGuard;
    tools::Rectangle aVisRect;
    vcl::Window* pWin = mpViewShell->GetWindow();
    if (pWin)
    {
        aVisRect.SetSize(pWin->GetOutputSizePixel());
        aVisRect.SetPos(Point(0, 0));

        aVisRect = pWin->PixelToLogic(aVisRect, maMapMode);
    }

    return aVisRect;
}

Point ScIAccessibleViewForwarder::LogicToPixel (const Point& rPoint) const
{
    SolarMutexGuard aGuard;
    Point aPoint;
    vcl::Window* pWin = mpViewShell->GetWindow();
    if (pWin && mpAccDoc)
    {
        tools::Rectangle aRect(mpAccDoc->GetBoundingBoxOnScreen());
        aPoint = pWin->LogicToPixel(rPoint, maMapMode) + aRect.TopLeft();
    }

    return aPoint;
}

Size ScIAccessibleViewForwarder::LogicToPixel (const Size& rSize) const
{
    SolarMutexGuard aGuard;
    Size aSize;
    vcl::Window* pWin = mpViewShell->GetWindow();
    if (pWin)
        aSize = pWin->LogicToPixel(rSize, maMapMode);
    return aSize;
}

struct ScShapeChild
{
    ScShapeChild()
        : mnRangeId(0)
    {
    }
    ~ScShapeChild();
    mutable rtl::Reference< ::accessibility::AccessibleShape > mpAccShape;
    css::uno::Reference< css::drawing::XShape > mxShape;
    sal_Int32 mnRangeId;
};

ScShapeChild::~ScShapeChild()
{
    if (mpAccShape.is())
    {
        mpAccShape->dispose();
    }
}

struct ScShapeChildLess
{
    bool operator()(const ScShapeChild& rChild1, const ScShapeChild& rChild2) const
    {
      bool bResult(false);
      if (rChild1.mxShape.is() && rChild2.mxShape.is())
          bResult = (rChild1.mxShape.get() < rChild2.mxShape.get());
      return bResult;
    }
};

typedef std::vector<ScShapeChild> ScShapeChildVec;

struct ScShapeRange
{
    ScShapeChildVec maBackShapes;
    ScShapeChildVec maForeShapes; // inclusive internal shapes
    ScShapeChildVec maControls;
    ScIAccessibleViewForwarder maViewForwarder;
};

typedef std::vector<ScShapeRange> ScShapeRangeVec;

class ScShapeChildren : public SfxListener,
        public ::accessibility::IAccessibleParent
{
public:
    ScShapeChildren(ScPreviewShell* pViewShell, ScAccessibleDocumentPagePreview* pAccDoc);
    virtual ~ScShapeChildren() override;

    ///=====  SfxListener  =====================================================

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    ///=====  IAccessibleParent  ==============================================

    virtual bool ReplaceChild (
        ::accessibility::AccessibleShape* pCurrentChild,
        const css::uno::Reference< css::drawing::XShape >& _rxShape,
        const long _nIndex,
        const ::accessibility::AccessibleShapeTreeInfo& _rShapeTreeInfo
    ) override;

    ///=====  Internal  ========================================================

    void Init();

    sal_Int32 GetBackShapeCount() const;
    uno::Reference<XAccessible> GetBackShape(sal_Int32 nIndex) const;
    sal_Int32 GetForeShapeCount() const;
    uno::Reference<XAccessible> GetForeShape(sal_Int32 nIndex) const;
    sal_Int32 GetControlCount() const;
    uno::Reference<XAccessible> GetControl(sal_Int32 nIndex) const;
    uno::Reference<XAccessible> GetForegroundShapeAt(const awt::Point& rPoint) const; // inclusive controls
    uno::Reference<XAccessible> GetBackgroundShapeAt(const awt::Point& rPoint) const;

    void DataChanged();
    void VisAreaChanged() const;

    void SetDrawBroadcaster();
private:
    ScAccessibleDocumentPagePreview* const mpAccDoc;
    ScPreviewShell* mpViewShell;
    ScShapeRangeVec maShapeRanges;

    void FindChanged(ScShapeChildVec& aOld, ScShapeChildVec& aNew) const;
    void FindChanged(ScShapeRange& aOld, ScShapeRange& aNew) const;
    ::accessibility::AccessibleShape* GetAccShape(const ScShapeChild& rShape) const;
    ::accessibility::AccessibleShape* GetAccShape(const ScShapeChildVec& rShapes, sal_Int32 nIndex) const;
    void FillShapes(const tools::Rectangle& aPixelPaintRect, const MapMode& aMapMode, sal_uInt8 nRangeId);

//    void AddShape(const uno::Reference<drawing::XShape>& xShape, SdrLayerID aLayerID);
//    void RemoveShape(const uno::Reference<drawing::XShape>& xShape, SdrLayerID aLayerID);
    SdrPage* GetDrawPage() const;
};

ScShapeChildren::ScShapeChildren(ScPreviewShell* pViewShell, ScAccessibleDocumentPagePreview* pAccDoc)
    :
    mpAccDoc(pAccDoc),
    mpViewShell(pViewShell),
    maShapeRanges(SC_PREVIEW_MAXRANGES)
{
    if (pViewShell)
    {
        SfxBroadcaster* pDrawBC = pViewShell->GetDocument().GetDrawBroadcaster();
        if (pDrawBC)
            StartListening(*pDrawBC);
    }
}

ScShapeChildren::~ScShapeChildren()
{
    if (mpViewShell)
    {
        SfxBroadcaster* pDrawBC = mpViewShell->GetDocument().GetDrawBroadcaster();
        if (pDrawBC)
            EndListening(*pDrawBC);
    }
}

void ScShapeChildren::SetDrawBroadcaster()
{
    if (mpViewShell)
    {
        SfxBroadcaster* pDrawBC = mpViewShell->GetDocument().GetDrawBroadcaster();
        if (pDrawBC)
            StartListening(*pDrawBC, DuplicateHandling::Prevent);
    }
}

void ScShapeChildren::Notify(SfxBroadcaster&, const SfxHint& rHint)
{
    const SdrHint* pSdrHint = dynamic_cast<const SdrHint*>( &rHint );
    if (pSdrHint)
    {
        SdrObject* pObj = const_cast<SdrObject*>(pSdrHint->GetObject());
        if (pObj && (pObj->getSdrPageFromSdrObject() == GetDrawPage()))
        {
            switch (pSdrHint->GetKind())
            {
                case SdrHintKind::ObjectChange :
                {
                }
                break;
                default :
                {
                    // other events are not interesting
                }
                break;
            }
        }
    }
}

void ScShapeChildren::FindChanged(ScShapeChildVec& rOld, ScShapeChildVec& rNew) const
{
    ScShapeChildVec::iterator aOldItr = rOld.begin();
    ScShapeChildVec::iterator aOldEnd = rOld.end();
    ScShapeChildVec::const_iterator aNewItr = rNew.begin();
    ScShapeChildVec::const_iterator aNewEnd = rNew.end();
    uno::Reference<XAccessible> xAcc;
    while ((aNewItr != aNewEnd) && (aOldItr != aOldEnd))
    {
        if (aNewItr->mxShape.get() == aOldItr->mxShape.get())
        {
            ++aOldItr;
            ++aNewItr;
        }
        else if (aNewItr->mxShape.get() < aOldItr->mxShape.get())
        {
            xAcc = GetAccShape(*aNewItr);
            AccessibleEventObject aEvent;
            aEvent.Source = uno::Reference<XAccessibleContext> (mpAccDoc);
            aEvent.EventId = AccessibleEventId::CHILD;
            aEvent.NewValue <<= xAcc;
            mpAccDoc->CommitChange(aEvent);
            ++aNewItr;
        }
        else
        {
            xAcc = GetAccShape(*aOldItr);
            AccessibleEventObject aEvent;
            aEvent.Source = uno::Reference<XAccessibleContext> (mpAccDoc);
            aEvent.EventId = AccessibleEventId::CHILD;
            aEvent.OldValue <<= xAcc;
            mpAccDoc->CommitChange(aEvent);
            ++aOldItr;
        }
    }
    while (aOldItr != aOldEnd)
    {
        xAcc = GetAccShape(*aOldItr);
        AccessibleEventObject aEvent;
        aEvent.Source = uno::Reference<XAccessibleContext> (mpAccDoc);
        aEvent.EventId = AccessibleEventId::CHILD;
        aEvent.OldValue <<= xAcc;
        mpAccDoc->CommitChange(aEvent);
        ++aOldItr;
    }
    while (aNewItr != aNewEnd)
    {
        xAcc = GetAccShape(*aNewItr);
        AccessibleEventObject aEvent;
        aEvent.Source = uno::Reference<XAccessibleContext> (mpAccDoc);
        aEvent.EventId = AccessibleEventId::CHILD;
        aEvent.NewValue <<= xAcc;
        mpAccDoc->CommitChange(aEvent);
        ++aNewItr;
    }
}

void ScShapeChildren::FindChanged(ScShapeRange& rOld, ScShapeRange& rNew) const
{
    FindChanged(rOld.maBackShapes, rNew.maBackShapes);
    FindChanged(rOld.maForeShapes, rNew.maForeShapes);
    FindChanged(rOld.maControls, rNew.maControls);
}

void ScShapeChildren::DataChanged()
{
    ScShapeRangeVec aOldShapeRanges(maShapeRanges);
    maShapeRanges.clear();
    maShapeRanges.resize(SC_PREVIEW_MAXRANGES);
    Init();
    for (sal_Int32 i = 0; i < SC_PREVIEW_MAXRANGES; ++i)
    {
        FindChanged(aOldShapeRanges[i], maShapeRanges[i]);
    }
}

namespace
{
    struct ScVisAreaChanged
    {
        void operator() (const ScShapeChild& rAccShapeData) const
        {
            if (rAccShapeData.mpAccShape.is())
            {
                rAccShapeData.mpAccShape->ViewForwarderChanged();
            }
        }
    };
}

void ScShapeChildren::VisAreaChanged() const
{
    for (auto const& shape : maShapeRanges)
    {
        ScVisAreaChanged aVisAreaChanged;
        std::for_each(shape.maBackShapes.begin(), shape.maBackShapes.end(), aVisAreaChanged);
        std::for_each(shape.maControls.begin(), shape.maControls.end(), aVisAreaChanged);
        std::for_each(shape.maForeShapes.begin(), shape.maForeShapes.end(), aVisAreaChanged);
    }
}

    ///=====  IAccessibleParent  ==============================================

bool ScShapeChildren::ReplaceChild (::accessibility::AccessibleShape* /* pCurrentChild */,
    const css::uno::Reference< css::drawing::XShape >& /* _rxShape */,
        const long /* _nIndex */, const ::accessibility::AccessibleShapeTreeInfo& /* _rShapeTreeInfo */)
{
    OSL_FAIL("should not be called in the page preview");
    return false;
}

    ///=====  Internal  ========================================================

void ScShapeChildren::Init()
{
    if(mpViewShell)
    {
        const ScPreviewLocationData& rData = mpViewShell->GetLocationData();
        MapMode aMapMode;
        tools::Rectangle aPixelPaintRect;
        sal_uInt8 nRangeId;
        sal_uInt16 nCount(rData.GetDrawRanges());
        for (sal_uInt16 i = 0; i < nCount; ++i)
        {
            rData.GetDrawRange(i, aPixelPaintRect, aMapMode, nRangeId);
            FillShapes(aPixelPaintRect, aMapMode, nRangeId);
        }
    }
}

sal_Int32 ScShapeChildren::GetBackShapeCount() const
{
    sal_Int32 nCount(0);
    for (auto const& shape : maShapeRanges)
        nCount += shape.maBackShapes.size();
    return nCount;
}

uno::Reference<XAccessible> ScShapeChildren::GetBackShape(sal_Int32 nIndex) const
{
    uno::Reference<XAccessible> xAccessible;
    for (const auto& rShapeRange : maShapeRanges)
    {
        sal_Int32 nCount(rShapeRange.maBackShapes.size());
        if(nIndex < nCount)
            xAccessible = GetAccShape(rShapeRange.maBackShapes, nIndex);
        nIndex -= nCount;
        if (xAccessible.is())
            break;
    }

    if (nIndex >= 0)
        throw lang::IndexOutOfBoundsException();

    return xAccessible;
}

sal_Int32 ScShapeChildren::GetForeShapeCount() const
{
    sal_Int32 nCount(0);
    for (auto const& shape : maShapeRanges)
        nCount += shape.maForeShapes.size();
    return nCount;
}

uno::Reference<XAccessible> ScShapeChildren::GetForeShape(sal_Int32 nIndex) const
{
    uno::Reference<XAccessible> xAccessible;
    for (const auto& rShapeRange : maShapeRanges)
    {
        sal_Int32 nCount(rShapeRange.maForeShapes.size());
        if(nIndex < nCount)
            xAccessible = GetAccShape(rShapeRange.maForeShapes, nIndex);
        nIndex -= nCount;
        if (xAccessible.is())
            break;
    }

    if (nIndex >= 0)
        throw lang::IndexOutOfBoundsException();

    return xAccessible;
}

sal_Int32 ScShapeChildren::GetControlCount() const
{
    sal_Int32 nCount(0);
    for (auto const& shape : maShapeRanges)
        nCount += shape.maControls.size();
    return nCount;
}

uno::Reference<XAccessible> ScShapeChildren::GetControl(sal_Int32 nIndex) const
{
    uno::Reference<XAccessible> xAccessible;
    for (const auto& rShapeRange : maShapeRanges)
    {
        sal_Int32 nCount(rShapeRange.maControls.size());
        if(nIndex < nCount)
            xAccessible = GetAccShape(rShapeRange.maControls, nIndex);
        nIndex -= nCount;
        if (xAccessible.is())
            break;
    }

    if (nIndex >= 0)
        throw lang::IndexOutOfBoundsException();

    return xAccessible;
}

struct ScShapePointFound
{
    Point const maPoint;
    explicit ScShapePointFound(const awt::Point& rPoint) : maPoint(VCLPoint(rPoint)) {}
    bool operator() (const ScShapeChild& rShape)
    {
        bool bResult(false);
        if (VCLRectangle(rShape.mpAccShape->getBounds()).IsInside(maPoint))
            bResult = true;
        return bResult;
    }
};

uno::Reference<XAccessible> ScShapeChildren::GetForegroundShapeAt(const awt::Point& rPoint) const //inclusive Controls
{
    uno::Reference<XAccessible> xAcc;

    for(const auto& rShapeRange : maShapeRanges)
    {
        ScShapeChildVec::const_iterator aFindItr = std::find_if(rShapeRange.maForeShapes.begin(), rShapeRange.maForeShapes.end(), ScShapePointFound(rPoint));
        if (aFindItr != rShapeRange.maForeShapes.end())
            xAcc = GetAccShape(*aFindItr);
        else
        {
            ScShapeChildVec::const_iterator aCtrlItr = std::find_if(rShapeRange.maControls.begin(), rShapeRange.maControls.end(), ScShapePointFound(rPoint));
            if (aCtrlItr != rShapeRange.maControls.end())
                xAcc = GetAccShape(*aCtrlItr);
        }

        if (xAcc.is())
            break;
    }

    return xAcc;
}

uno::Reference<XAccessible> ScShapeChildren::GetBackgroundShapeAt(const awt::Point& rPoint) const
{
    uno::Reference<XAccessible> xAcc;

    for(const auto& rShapeRange : maShapeRanges)
    {
        ScShapeChildVec::const_iterator aFindItr = std::find_if(rShapeRange.maBackShapes.begin(), rShapeRange.maBackShapes.end(), ScShapePointFound(rPoint));
        if (aFindItr != rShapeRange.maBackShapes.end())
            xAcc = GetAccShape(*aFindItr);
        if (xAcc.is())
            break;
    }

    return xAcc;
}

::accessibility::AccessibleShape* ScShapeChildren::GetAccShape(const ScShapeChild& rShape) const
{
    if (!rShape.mpAccShape.is())
    {
        ::accessibility::ShapeTypeHandler& rShapeHandler = ::accessibility::ShapeTypeHandler::Instance();
        ::accessibility::AccessibleShapeInfo aShapeInfo(rShape.mxShape, mpAccDoc);

        if (mpViewShell)
        {
            ::accessibility::AccessibleShapeTreeInfo aShapeTreeInfo;
            aShapeTreeInfo.SetSdrView(mpViewShell->GetPreview()->GetDrawView());
            aShapeTreeInfo.SetController(nullptr);
            aShapeTreeInfo.SetWindow(mpViewShell->GetWindow());
            aShapeTreeInfo.SetViewForwarder(&(maShapeRanges[rShape.mnRangeId].maViewForwarder));
            rShape.mpAccShape = rShapeHandler.CreateAccessibleObject(aShapeInfo, aShapeTreeInfo);
            if (rShape.mpAccShape.is())
            {
                rShape.mpAccShape->Init();
            }
        }
    }
    return rShape.mpAccShape.get();
}

::accessibility::AccessibleShape* ScShapeChildren::GetAccShape(const ScShapeChildVec& rShapes, sal_Int32 nIndex) const
{
    return GetAccShape(rShapes[nIndex]);
}

void ScShapeChildren::FillShapes(const tools::Rectangle& aPixelPaintRect, const MapMode& aMapMode, sal_uInt8 nRangeId)
{
    OSL_ENSURE(nRangeId < maShapeRanges.size(), "this is not a valid range for draw objects");
    SdrPage* pPage = GetDrawPage();
    vcl::Window* pWin = mpViewShell->GetWindow();
    if (pPage && pWin)
    {
        bool bForeAdded(false);
        bool bBackAdded(false);
        bool bControlAdded(false);
        tools::Rectangle aClippedPixelPaintRect(aPixelPaintRect);
        if (mpAccDoc)
        {
            tools::Rectangle aRect2(Point(0,0), mpAccDoc->GetBoundingBoxOnScreen().GetSize());
            aClippedPixelPaintRect = aPixelPaintRect.GetIntersection(aRect2);
        }
        ScIAccessibleViewForwarder aViewForwarder(mpViewShell, mpAccDoc, aMapMode);
        maShapeRanges[nRangeId].maViewForwarder = aViewForwarder;
        const size_t nCount(pPage->GetObjCount());
        for (size_t i = 0; i < nCount; ++i)
        {
            SdrObject* pObj = pPage->GetObj(i);
            if (pObj)
            {
                uno::Reference< drawing::XShape > xShape(pObj->getUnoShape(), uno::UNO_QUERY);
                if (xShape.is())
                {
                    tools::Rectangle aRect(pWin->LogicToPixel(VCLPoint(xShape->getPosition()), aMapMode), pWin->LogicToPixel(VCLSize(xShape->getSize()), aMapMode));
                    if(!aClippedPixelPaintRect.GetIntersection(aRect).IsEmpty())
                    {
                        ScShapeChild aShape;
                        aShape.mxShape = xShape;
                        aShape.mnRangeId = nRangeId;
                        if (pObj->GetLayer().anyOf(SC_LAYER_INTERN, SC_LAYER_FRONT))
                        {
                            maShapeRanges[nRangeId].maForeShapes.push_back(aShape);
                            bForeAdded = true;
                        }
                        else if (pObj->GetLayer() == SC_LAYER_BACK)
                        {
                            maShapeRanges[nRangeId].maBackShapes.push_back(aShape);
                            bBackAdded = true;
                        }
                        else if (pObj->GetLayer() == SC_LAYER_CONTROLS)
                        {
                            maShapeRanges[nRangeId].maControls.push_back(aShape);
                            bControlAdded = true;
                        }
                        else
                        {
                            OSL_FAIL("I don't know this layer.");
                        }
                    }
                }
            }
        }
        if (bForeAdded)
            std::sort(maShapeRanges[nRangeId].maForeShapes.begin(), maShapeRanges[nRangeId].maForeShapes.end(),ScShapeChildLess());
        if (bBackAdded)
            std::sort(maShapeRanges[nRangeId].maBackShapes.begin(), maShapeRanges[nRangeId].maBackShapes.end(),ScShapeChildLess());
        if (bControlAdded)
            std::sort(maShapeRanges[nRangeId].maControls.begin(), maShapeRanges[nRangeId].maControls.end(),ScShapeChildLess());
    }
}

SdrPage* ScShapeChildren::GetDrawPage() const
{
    SCTAB nTab( mpViewShell->GetLocationData().GetPrintTab() );
    SdrPage* pDrawPage = nullptr;
    ScDocument& rDoc = mpViewShell->GetDocument();
    if (rDoc.GetDrawLayer())
    {
        ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
        if (pDrawLayer->HasObjects() && (pDrawLayer->GetPageCount() > nTab))
            pDrawPage = pDrawLayer->GetPage(static_cast<sal_uInt16>(static_cast<sal_Int16>(nTab)));
    }
    return pDrawPage;
}

struct ScPagePreviewCountData
{
    //  order is background shapes, header, table or notes, footer, foreground shapes, controls

    tools::Rectangle aVisRect;
    long nBackShapes;
    long nHeaders;
    long nTables;
    long nNoteParagraphs;
    long nFooters;
    long nForeShapes;
    long nControls;

    ScPagePreviewCountData( const ScPreviewLocationData& rData, const vcl::Window* pSizeWindow,
        const ScNotesChildren* pNotesChildren, const ScShapeChildren* pShapeChildren );

    long GetTotal() const
    {
        return nBackShapes + nHeaders + nTables + nNoteParagraphs + nFooters + nForeShapes + nControls;
    }
};

ScPagePreviewCountData::ScPagePreviewCountData( const ScPreviewLocationData& rData,
                                const vcl::Window* pSizeWindow, const ScNotesChildren* pNotesChildren,
                                const ScShapeChildren* pShapeChildren) :
    nBackShapes( 0 ),
    nHeaders( 0 ),
    nTables( 0 ),
    nNoteParagraphs( 0 ),
    nFooters( 0 ),
    nForeShapes( 0 ),
    nControls( 0 )
{
    Size aOutputSize;
    if ( pSizeWindow )
        aOutputSize = pSizeWindow->GetOutputSizePixel();
    aVisRect = tools::Rectangle( Point(), aOutputSize );

    tools::Rectangle aObjRect;

    if ( rData.GetHeaderPosition( aObjRect ) && aObjRect.IsOver( aVisRect ) )
        nHeaders = 1;

    if ( rData.GetFooterPosition( aObjRect ) && aObjRect.IsOver( aVisRect ) )
        nFooters = 1;

    if ( rData.HasCellsInRange( aVisRect ) )
        nTables = 1;

    //! shapes...
    nBackShapes = pShapeChildren->GetBackShapeCount();
    nForeShapes = pShapeChildren->GetForeShapeCount();
    nControls = pShapeChildren->GetControlCount();

    // there are only notes if there is no table
    if (nTables == 0)
        nNoteParagraphs = pNotesChildren->GetChildrenCount();
}

//=====  internal  ========================================================

ScAccessibleDocumentPagePreview::ScAccessibleDocumentPagePreview(
        const uno::Reference<XAccessible>& rxParent, ScPreviewShell* pViewShell ) :
    ScAccessibleDocumentBase(rxParent),
    mpViewShell(pViewShell)
{
    if (pViewShell)
        pViewShell->AddAccessibilityObject(*this);

}

ScAccessibleDocumentPagePreview::~ScAccessibleDocumentPagePreview()
{
    if (!ScAccessibleDocumentBase::IsDefunc() && !rBHelper.bInDispose)
    {
        // increment refcount to prevent double call off dtor
        osl_atomic_increment( &m_refCount );
        // call dispose to inform object which have a weak reference to this object
        dispose();
    }
}

void SAL_CALL ScAccessibleDocumentPagePreview::disposing()
{
    SolarMutexGuard aGuard;
    mpTable.clear();
    mpHeader.clear();
    mpFooter.clear();

    if (mpViewShell)
    {
        mpViewShell->RemoveAccessibilityObject(*this);
        mpViewShell = nullptr;
    }

    // no need to Dispose the AccessibleTextHelper,
    // as long as mpNotesChildren are destructed here
    mpNotesChildren.reset();

    mpShapeChildren.reset();

    ScAccessibleDocumentBase::disposing();
}

//=====  SfxListener  =====================================================

void ScAccessibleDocumentPagePreview::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( dynamic_cast<const ScAccWinFocusLostHint*>(&rHint) )
    {
        CommitFocusLost();
    }
    else if ( dynamic_cast<const ScAccWinFocusGotHint*>(&rHint) )
    {
        CommitFocusGained();
    }
    else
    {
        // only notify if child exist, otherwise it is not necessary
        if (rHint.GetId() == SfxHintId::ScDataChanged)
        {
            if (mpTable.is()) // if there is no table there is nothing to notify, because no one recongnizes the change
            {
                {
                    uno::Reference<XAccessible> xAcc = mpTable.get();
                    AccessibleEventObject aEvent;
                    aEvent.EventId = AccessibleEventId::CHILD;
                    aEvent.Source = uno::Reference< XAccessibleContext >(this);
                    aEvent.OldValue <<= xAcc;
                    CommitChange(aEvent);
                }

                mpTable->dispose();
                mpTable.clear();
            }

            Size aOutputSize;
            vcl::Window* pSizeWindow = mpViewShell->GetWindow();
            if ( pSizeWindow )
                aOutputSize = pSizeWindow->GetOutputSizePixel();
            tools::Rectangle aVisRect( Point(), aOutputSize );
            GetNotesChildren()->DataChanged(aVisRect);

            GetShapeChildren()->DataChanged();

            const ScPreviewLocationData& rData = mpViewShell->GetLocationData();
            ScPagePreviewCountData aCount( rData, mpViewShell->GetWindow(), GetNotesChildren(), GetShapeChildren() );

            if (aCount.nTables > 0)
            {
                //! order is background shapes, header, table or notes, footer, foreground shapes, controls
                sal_Int32 nIndex (aCount.nBackShapes + aCount.nHeaders);

                mpTable = new ScAccessiblePreviewTable( this, mpViewShell, nIndex );
                mpTable->Init();

                {
                    uno::Reference<XAccessible> xAcc = mpTable.get();
                    AccessibleEventObject aEvent;
                    aEvent.EventId = AccessibleEventId::CHILD;
                    aEvent.Source = uno::Reference< XAccessibleContext >(this);
                    aEvent.NewValue <<= xAcc;
                    CommitChange(aEvent);
                }
            }
        }
        else if (rHint.GetId() == SfxHintId::ScAccMakeDrawLayer)
        {
            GetShapeChildren()->SetDrawBroadcaster();
        }
        else if (rHint.GetId() == SfxHintId::ScAccVisAreaChanged)
        {
            Size aOutputSize;
            vcl::Window* pSizeWindow = mpViewShell->GetWindow();
            if ( pSizeWindow )
                aOutputSize = pSizeWindow->GetOutputSizePixel();
            tools::Rectangle aVisRect( Point(), aOutputSize );
            GetNotesChildren()->DataChanged(aVisRect);

            GetShapeChildren()->VisAreaChanged();

            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::VISIBLE_DATA_CHANGED;
            aEvent.Source = uno::Reference< XAccessibleContext >(this);
            CommitChange(aEvent);
        }
    }
    ScAccessibleDocumentBase::Notify(rBC, rHint);
}

//=====  XAccessibleComponent  ============================================

uno::Reference< XAccessible > SAL_CALL ScAccessibleDocumentPagePreview::getAccessibleAtPoint( const awt::Point& rPoint )
{
    uno::Reference<XAccessible> xAccessible;
    if (containsPoint(rPoint))
    {
        SolarMutexGuard aGuard;
        IsObjectValid();

        if ( mpViewShell )
        {
            xAccessible = GetShapeChildren()->GetForegroundShapeAt(rPoint);
            if (!xAccessible.is())
            {
                const ScPreviewLocationData& rData = mpViewShell->GetLocationData();
                ScPagePreviewCountData aCount( rData, mpViewShell->GetWindow(), GetNotesChildren(), GetShapeChildren() );

                if ( !mpTable.is() && (aCount.nTables > 0) )
                {
                    //! order is background shapes, header, table or notes, footer, foreground shapes, controls
                    sal_Int32 nIndex (aCount.nBackShapes + aCount.nHeaders);

                    mpTable = new ScAccessiblePreviewTable( this, mpViewShell, nIndex );
                    mpTable->Init();
                }
                if (mpTable.is() && VCLRectangle(mpTable->getBounds()).IsInside(VCLPoint(rPoint)))
                    xAccessible = mpTable.get();
            }
            if (!xAccessible.is())
                xAccessible = GetNotesChildren()->GetAt(rPoint);
            if (!xAccessible.is())
            {
                if (!mpHeader.is() || !mpFooter.is())
                {
                    const ScPreviewLocationData& rData = mpViewShell->GetLocationData();
                    ScPagePreviewCountData aCount( rData, mpViewShell->GetWindow(), GetNotesChildren(), GetShapeChildren() );

                    if (!mpHeader.is())
                    {
                        mpHeader = new ScAccessiblePageHeader( this, mpViewShell, true, aCount.nBackShapes + aCount.nHeaders - 1);
                    }
                    if (!mpFooter.is())
                    {
                        mpFooter = new ScAccessiblePageHeader( this, mpViewShell, false, aCount.nBackShapes + aCount.nHeaders + aCount.nTables + aCount.nNoteParagraphs + aCount.nFooters - 1 );
                    }
                }

                Point aPoint(VCLPoint(rPoint));

                if (VCLRectangle(mpHeader->getBounds()).IsInside(aPoint))
                    xAccessible = mpHeader.get();
                else if (VCLRectangle(mpFooter->getBounds()).IsInside(aPoint))
                    xAccessible = mpFooter.get();
            }
            if (!xAccessible.is())
                xAccessible = GetShapeChildren()->GetBackgroundShapeAt(rPoint);
        }
    }

    return xAccessible;
}

void SAL_CALL ScAccessibleDocumentPagePreview::grabFocus()
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    if (getAccessibleParent().is())
    {
        uno::Reference<XAccessibleComponent> xAccessibleComponent(getAccessibleParent()->getAccessibleContext(), uno::UNO_QUERY);
        if (xAccessibleComponent.is())
        {
            // just grab the focus for the window
            xAccessibleComponent->grabFocus();
        }
    }
}

//=====  XAccessibleContext  ==============================================

sal_Int32 SAL_CALL ScAccessibleDocumentPagePreview::getAccessibleChildCount()
{
    SolarMutexGuard aGuard;
    IsObjectValid();

    long nRet = 0;
    if ( mpViewShell )
    {
        ScPagePreviewCountData aCount( mpViewShell->GetLocationData(), mpViewShell->GetWindow(), GetNotesChildren(), GetShapeChildren() );
        nRet = aCount.GetTotal();
    }

    return nRet;
}

uno::Reference<XAccessible> SAL_CALL ScAccessibleDocumentPagePreview::getAccessibleChild(sal_Int32 nIndex)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    uno::Reference<XAccessible> xAccessible;

    if ( mpViewShell )
    {
        const ScPreviewLocationData& rData = mpViewShell->GetLocationData();
        ScPagePreviewCountData aCount( rData, mpViewShell->GetWindow(), GetNotesChildren(), GetShapeChildren() );

        if ( nIndex < aCount.nBackShapes )
        {
            xAccessible = GetShapeChildren()->GetBackShape(nIndex);
        }
        else if ( nIndex < aCount.nBackShapes + aCount.nHeaders )
        {
            if ( !mpHeader.is() )
            {
                mpHeader = new ScAccessiblePageHeader( this, mpViewShell, true, nIndex );
            }

            xAccessible = mpHeader.get();
        }
        else if ( nIndex < aCount.nBackShapes + aCount.nHeaders + aCount.nTables )
        {
            if ( !mpTable.is() )
            {
                mpTable = new ScAccessiblePreviewTable( this, mpViewShell, nIndex );
                mpTable->Init();
            }
            xAccessible = mpTable.get();
        }
        else if ( nIndex < aCount.nBackShapes + aCount.nHeaders + aCount.nNoteParagraphs )
        {
            xAccessible = GetNotesChildren()->GetChild(nIndex - aCount.nBackShapes - aCount.nHeaders);
        }
        else if ( nIndex < aCount.nBackShapes + aCount.nHeaders + aCount.nTables + aCount.nNoteParagraphs + aCount.nFooters )
        {
            if ( !mpFooter.is() )
            {
                mpFooter = new ScAccessiblePageHeader( this, mpViewShell, false, nIndex );
            }
            xAccessible = mpFooter.get();
        }
        else
        {
            sal_Int32 nIdx(nIndex - (aCount.nBackShapes + aCount.nHeaders + aCount.nTables + aCount.nNoteParagraphs + aCount.nFooters));
            if (nIdx < aCount.nForeShapes)
                xAccessible = GetShapeChildren()->GetForeShape(nIdx);
            else
                xAccessible = GetShapeChildren()->GetControl(nIdx - aCount.nForeShapes);
        }
    }

    if ( !xAccessible.is() )
        throw lang::IndexOutOfBoundsException();

    return xAccessible;
}

    /// Return the set of current states.
uno::Reference<XAccessibleStateSet> SAL_CALL ScAccessibleDocumentPagePreview::getAccessibleStateSet()
{
    SolarMutexGuard aGuard;
    uno::Reference<XAccessibleStateSet> xParentStates;
    if (getAccessibleParent().is())
    {
        uno::Reference<XAccessibleContext> xParentContext = getAccessibleParent()->getAccessibleContext();
        xParentStates = xParentContext->getAccessibleStateSet();
    }
    utl::AccessibleStateSetHelper* pStateSet = new utl::AccessibleStateSetHelper();
    if (IsDefunc(xParentStates))
        pStateSet->AddState(AccessibleStateType::DEFUNC);
    else
    {
        // never editable
        pStateSet->AddState(AccessibleStateType::ENABLED);
        pStateSet->AddState(AccessibleStateType::OPAQUE);
        if (isShowing())
            pStateSet->AddState(AccessibleStateType::SHOWING);
        if (isVisible())
            pStateSet->AddState(AccessibleStateType::VISIBLE);
    }
    return pStateSet;
}

    //=====  XServiceInfo  ====================================================

OUString SAL_CALL ScAccessibleDocumentPagePreview::getImplementationName()
{
    return OUString("ScAccessibleDocumentPagePreview");
}

uno::Sequence< OUString> SAL_CALL ScAccessibleDocumentPagePreview::getSupportedServiceNames()
{
    uno::Sequence< OUString > aSequence = ScAccessibleContextBase::getSupportedServiceNames();
    sal_Int32 nOldSize(aSequence.getLength());
    aSequence.realloc(nOldSize + 1);

    aSequence[nOldSize] = "com.sun.star.AccessibleSpreadsheetPageView";

    return aSequence;
}

//=====  XTypeProvider  =======================================================

uno::Sequence<sal_Int8> SAL_CALL
    ScAccessibleDocumentPagePreview::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

//=====  internal  ========================================================

OUString ScAccessibleDocumentPagePreview::createAccessibleDescription()
{
    OUString sDescription = STR_ACC_PREVIEWDOC_DESCR;
    return sDescription;
}

OUString ScAccessibleDocumentPagePreview::createAccessibleName()
{
    OUString sName = ScResId(STR_ACC_PREVIEWDOC_NAME);
    return sName;
}

tools::Rectangle ScAccessibleDocumentPagePreview::GetBoundingBoxOnScreen() const
{
    tools::Rectangle aRect;
    if (mpViewShell)
    {
        vcl::Window* pWindow = mpViewShell->GetWindow();
        if (pWindow)
            aRect = pWindow->GetWindowExtentsRelative(nullptr);
    }
    return aRect;
}

tools::Rectangle ScAccessibleDocumentPagePreview::GetBoundingBox() const
{
    tools::Rectangle aRect;
    if (mpViewShell)
    {
        vcl::Window* pWindow = mpViewShell->GetWindow();
        if (pWindow)
            aRect = pWindow->GetWindowExtentsRelative(pWindow->GetAccessibleParentWindow());
    }
    return aRect;
}

bool ScAccessibleDocumentPagePreview::IsDefunc(
    const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    return ScAccessibleContextBase::IsDefunc() || !getAccessibleParent().is() ||
        (rxParentStates.is() && rxParentStates->contains(AccessibleStateType::DEFUNC));
}

ScNotesChildren* ScAccessibleDocumentPagePreview::GetNotesChildren()
{
    if (!mpNotesChildren && mpViewShell)
    {
        mpNotesChildren.reset( new ScNotesChildren(mpViewShell, this) );

        const ScPreviewLocationData& rData = mpViewShell->GetLocationData();
        ScPagePreviewCountData aCount( rData, mpViewShell->GetWindow(), GetNotesChildren(), GetShapeChildren() );

        //! order is background shapes, header, table or notes, footer, foreground shapes, controls
        mpNotesChildren->Init(aCount.aVisRect, aCount.nBackShapes + aCount.nHeaders);
    }
    return mpNotesChildren.get();
}

ScShapeChildren* ScAccessibleDocumentPagePreview::GetShapeChildren()
{
    if (!mpShapeChildren && mpViewShell)
    {
        mpShapeChildren.reset( new ScShapeChildren(mpViewShell, this) );
        mpShapeChildren->Init();
    }

    return mpShapeChildren.get();
}

OUString ScAccessibleDocumentPagePreview::getAccessibleName()
{
    SolarMutexGuard g;

    OUString aName = ScResId(STR_ACC_DOC_SPREADSHEET);
    ScDocument& rScDoc = mpViewShell->GetDocument();

    SfxObjectShell* pObjSh = rScDoc.GetDocumentShell();
    if (!pObjSh)
        return aName;

    OUString aFileName;
    SfxMedium* pMed = pObjSh->GetMedium();
    if (pMed)
        aFileName = pMed->GetName();

    if (aFileName.isEmpty())
        aFileName = pObjSh->GetTitle(SFX_TITLE_APINAME);

    if (!aFileName.isEmpty())
    {
        aName = aFileName + " - " + aName;
        aName += ScResId(STR_ACC_DOC_PREVIEW_SUFFIX);

    }

    return aName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
