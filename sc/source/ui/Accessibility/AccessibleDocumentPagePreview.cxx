/*************************************************************************
 *
 *  $RCSfile: AccessibleDocumentPagePreview.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: hr $ $Date: 2003-07-17 11:30:42 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#ifndef _SC_ACCESSIBLEDOCUMENTPAGEPREVIEW_HXX
#include "AccessibleDocumentPagePreview.hxx"
#endif
#ifndef _SC_ACCESSIBLEPREVIEWTABLE_HXX
#include "AccessiblePreviewTable.hxx"
#endif
#ifndef _SC_ACCESSIBLEPAGEHEADER_HXX
#include "AccessiblePageHeader.hxx"
#endif
#ifndef SC_ACCESSIBILITYHINTS_HXX
#include "AccessibilityHints.hxx"
#endif
#ifndef _SC_ACCESSIBLETEXT_HXX
#include "AccessibleText.hxx"
#endif
#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_PREVWSH_HXX
#include "prevwsh.hxx"
#endif
#ifndef SC_PREVLOC_HXX
#include "prevloc.hxx"
#endif
#ifndef SC_UNOGUARD_HXX
#include "unoguard.hxx"
#endif
#ifndef SC_DRWLAYER_HXX
#include "drwlayer.hxx"
#endif
#ifndef SC_EDITSRC_HXX
#include "editsrc.hxx"
#endif
#ifndef SC_SCRESID_HXX
#include "scresid.hxx"
#endif
#ifndef SC_SC_HRC
#include "sc.hrc"
#endif
#ifndef _SC_DRAWMODELBROADCASTER_HXX
#include "DrawModelBroadcaster.hxx"
#endif
#ifndef SC_DOCSHELL_HXX
#include "docsh.hxx"
#endif
#ifndef SC_DRAWVIEW_HXX
#include "drawview.hxx"
#endif
#ifndef SC_PREVIEW_HXX
#include "preview.hxx"
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLERELATIONTYPE_HPP_
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#endif

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif
#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif
#ifndef _SVX_ACCESSILE_TEXT_HELPER_HXX_
#include <svx/AccessibleTextHelper.hxx>
#endif
#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_HXX
#include <svx/AccessibleShape.hxx>
#endif
#ifndef _SVX_ACCESSIBILITY_SHAPE_TYPE_HANDLER_HXX
#include <svx/ShapeTypeHandler.hxx>
#endif
#ifndef _TOOLKIT_HELPER_CONVERT_HXX_
#include <toolkit/helper/convert.hxx>
#endif
#ifndef _SVX_UNOSHAPE_HXX
#include <svx/unoshape.hxx>
#endif
#ifndef _UTL_ACCESSIBLERELATIONSETHELPER_HXX_
#include <unotools/accessiblerelationsethelper.hxx>
#endif

#include<vector>
#include<list>
#include<algorithm>
#include<memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

//=========================================================================

typedef std::list< uno::Reference< XAccessible > > ScXAccList;


struct ScAccNote
{
    String      maNoteText;
    Rectangle   maRect;
    ScAddress   maNoteCell;
    ::accessibility::AccessibleTextHelper* mpTextHelper;
    sal_Int32   mnParaCount;
    sal_Bool    mbMarkNote;

                ScAccNote() : mpTextHelper(NULL), mnParaCount(0) {}
};

class ScNotesChilds
{
public:
    ScNotesChilds(ScPreviewShell* pViewShell, ScAccessibleDocumentPagePreview* pAccDoc);
    ~ScNotesChilds();
    void Init(const Rectangle& rVisRect, sal_Int32 nOffset);

    sal_Int32 GetChildsCount() const;
    uno::Reference<XAccessible> GetChild(sal_Int32 nIndex) const;
    uno::Reference<XAccessible> GetAt(const awt::Point& rPoint) const;

    void DataChanged(const Rectangle& rVisRect);
    void SetOffset(sal_Int32 nNewOffset);
private:
    ScPreviewShell*         mpViewShell;
    ScAccessibleDocumentPagePreview* mpAccDoc;
    typedef std::vector<ScAccNote> ScAccNotes;
    mutable ScAccNotes      maNotes;
    mutable ScAccNotes      maMarks;
    sal_Int32               mnParagraphs;
    sal_Int32               mnOffset;

    ::accessibility::AccessibleTextHelper* CreateTextHelper(const String& rString, const Rectangle& rVisRect, const ScAddress& aCellPos, sal_Bool bMarkNote, sal_Int32 nChildOffset) const;
    sal_Int32 AddNotes(const ScPreviewLocationData& rData, const Rectangle& rVisRect, sal_Bool bMark, ScAccNotes& rNotes);

    sal_Int8 CompareCell(const ScAddress& aCell1, const ScAddress& aCell2);
    void CollectChilds(const ScAccNote& rNote, ScXAccList& rList);
    sal_Int32 CheckChanges(const ScPreviewLocationData& rData, const Rectangle& rVisRect,
        sal_Bool bMark, ScAccNotes& rOldNotes, ScAccNotes& rNewNotes,
        ScXAccList& rOldParas, ScXAccList& rNewParas);

    inline ScDocument* GetDocument() const;
};

ScNotesChilds::ScNotesChilds(ScPreviewShell* pViewShell, ScAccessibleDocumentPagePreview* pAccDoc)
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

ScNotesChilds::~ScNotesChilds()
{
    std::for_each(maNotes.begin(), maNotes.end(), DeleteAccNote());
    std::for_each(maMarks.begin(), maMarks.end(), DeleteAccNote());
}

::accessibility::AccessibleTextHelper* ScNotesChilds::CreateTextHelper(const String& rString, const Rectangle& rVisRect, const ScAddress& aCellPos, sal_Bool bMarkNote, sal_Int32 nChildOffset) const
{
    ::accessibility::AccessibleTextHelper* pTextHelper = NULL;

    ::std::auto_ptr < ScAccessibleTextData > pAccessiblePreviewHeaderCellTextData
        (new ScAccessibleNoteTextData(mpViewShell, rString, aCellPos, bMarkNote));
    ::std::auto_ptr< SvxEditSource > pEditSource (new ScAccessibilityEditSource(pAccessiblePreviewHeaderCellTextData));

    pTextHelper = new ::accessibility::AccessibleTextHelper(pEditSource);

    if (pTextHelper)
    {
        pTextHelper->SetEventSource(mpAccDoc);
        pTextHelper->SetStartIndex(nChildOffset);
        pTextHelper->SetOffset(rVisRect.TopLeft());
    }

    return pTextHelper;
}

sal_Int32 ScNotesChilds::AddNotes(const ScPreviewLocationData& rData, const Rectangle& rVisRect, sal_Bool bMark, ScAccNotes& rNotes)
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
                    aNote.maNoteCell.Format( aNote.maNoteText, SCA_VALID, NULL );
                }
                else
                {
                    ScPostIt aPostIt;
                    pDoc->GetNote(aNote.maNoteCell.Col(), aNote.maNoteCell.Row(), aNote.maNoteCell.Tab(), aPostIt);
                    aNote.maNoteText = aPostIt.GetText();
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

void ScNotesChilds::Init(const Rectangle& rVisRect, sal_Int32 nOffset)
{
    if (mpViewShell && !mnParagraphs)
    {
        mnOffset = nOffset;
        const ScPreviewLocationData& rData = mpViewShell->GetLocationData();

        mnParagraphs = AddNotes(rData, rVisRect, sal_False, maMarks);
        mnParagraphs += AddNotes(rData, rVisRect, sal_True, maNotes);
    }
}

sal_Int32 ScNotesChilds::GetChildsCount() const
{
    return mnParagraphs;
}

struct ScParaFound
{
    sal_Int32 mnIndex;
    ScParaFound(sal_Int32 nIndex) : mnIndex(nIndex) {}
    sal_Bool operator() (const ScAccNote& rNote)
    {
        sal_Bool bResult(sal_False);
        if (rNote.mnParaCount > mnIndex)
            bResult = sal_True;
        else
            mnIndex -= rNote.mnParaCount;
        return bResult;
    }
};

uno::Reference<XAccessible> ScNotesChilds::GetChild(sal_Int32 nIndex) const
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
                DBG_ASSERT((aItr->maNoteCell == maMarks[nIndex].maNoteCell) && (aItr->mbMarkNote == maMarks[nIndex].mbMarkNote), "wrong note found");
            }
            else
                DBG_ERRORFILE("wrong note found");
            if (!aItr->mpTextHelper)
                aItr->mpTextHelper = CreateTextHelper(maMarks[nIndex].maNoteText, maMarks[nIndex].maRect, maMarks[nIndex].maNoteCell, maMarks[nIndex].mbMarkNote, nIndex + mnOffset); // the marks are the first and every mark has only one paragraph
            xAccessible = aItr->mpTextHelper->GetChild(aParaFound.mnIndex + aItr->mpTextHelper->GetStartIndex());
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
    Rectangle maPoint;
    sal_Int32 mnParagraphs;
    ScPointFound(const Point& rPoint) : maPoint(rPoint, Size(0, 0)), mnParagraphs(0) {}
    sal_Bool operator() (const ScAccNote& rNote)
    {
        sal_Bool bResult(sal_False);
        if (maPoint.IsInside(rNote.maRect))
            bResult = sal_True;
        else
            mnParagraphs += rNote.mnParaCount;
        return bResult;
    }
};

uno::Reference<XAccessible> ScNotesChilds::GetAt(const awt::Point& rPoint) const
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

sal_Int8 ScNotesChilds::CompareCell(const ScAddress& aCell1, const ScAddress& aCell2)
{
    DBG_ASSERT(aCell1.Tab() == aCell2.Tab(), "the notes should be on the same table")
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

void ScNotesChilds::CollectChilds(const ScAccNote& rNote, ScXAccList& rList)
{
    if (rNote.mpTextHelper)
        for (sal_Int32 i = 0; i < rNote.mnParaCount; ++i)
            rList.push_back(rNote.mpTextHelper->GetChild(i + rNote.mpTextHelper->GetStartIndex()));
}

sal_Int32 ScNotesChilds::CheckChanges(const ScPreviewLocationData& rData,
            const Rectangle& rVisRect, sal_Bool bMark, ScAccNotes& rOldNotes,
            ScAccNotes& rNewNotes, ScXAccList& rOldParas, ScXAccList& rNewParas)
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
        sal_Bool bAddNote(sal_False);
        for (sal_Int32 nIndex = 0; nIndex < nCount; ++nIndex)
        {
            if (rData.GetNoteInRange(rVisRect, nIndex, bMark, aNote.maNoteCell, aNote.maRect))
            {
                if (bMark)
                {
                    // Document not needed, because only the cell address, but not the tablename is needed
                    aNote.maNoteCell.Format( aNote.maNoteText, SCA_VALID, NULL );
                }
                else
                {
                    ScPostIt aPostIt;
                    pDoc->GetNote(aNote.maNoteCell.Col(), aNote.maNoteCell.Row(), aNote.maNoteCell.Tab(), aPostIt);
                    aNote.maNoteText = aPostIt.GetText();
                }

                sal_Int8 nCompare(-1); // if there are no more old childs it is always a new one
                if (aItr != aEndItr)
                    nCompare = CompareCell(aNote.maNoteCell, aItr->maNoteCell);
                if (nCompare == 0)
                {
                    if (aNote.maNoteText == aItr->maNoteText)
                    {
                        aNote.mpTextHelper = aItr->mpTextHelper;
                        if (aNote.maRect != aItr->maRect)  //neue VisArea setzen
                        {
                            aNote.mpTextHelper->SetOffset(aNote.maRect.TopLeft());
                            aNote.mpTextHelper->UpdateChildren();
                            //DBG_ASSERT(aItr->maRect.GetSize() == aNote.maRect.GetSize(), "size should be the same, because the text is not changed");
                            // could be changed, because only a part of the note is visible
                        }
                    }
                    else
                    {
                        aNote.mpTextHelper = CreateTextHelper(aNote.maNoteText, aNote.maRect, aNote.maNoteCell, aNote.mbMarkNote, nParagraphs + mnOffset);
                        if (aNote.mpTextHelper)
                            aNote.mnParaCount = aNote.mpTextHelper->GetChildCount();
                        // collect removed childs
                        CollectChilds(*aItr, rOldParas);
                        DELETEZ(aItr->mpTextHelper);
                        // collect new childs
                        CollectChilds(aNote, rNewParas);
                    }
                    bAddNote = sal_True;
                    // not necessary, because this branch should not be reached if it is the end
                    //if (aItr != aEndItr)
                    ++aItr;
                }
                else if (nCompare < 0)
                {
                    aNote.mpTextHelper = CreateTextHelper(aNote.maNoteText, aNote.maRect, aNote.maNoteCell, aNote.mbMarkNote, nParagraphs + mnOffset);
                    if (aNote.mpTextHelper)
                        aNote.mnParaCount = aNote.mpTextHelper->GetChildCount();
                    // collect new childs
                    CollectChilds(aNote, rNewParas);
                    bAddNote = sal_True;
                }
                else
                {
                    // collect removed childs
                    CollectChilds(*aItr, rOldParas);
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
                    bAddNote = sal_False;
                }
            }
        }
    }
    return nParagraphs;
}

struct ScChildGone
{
    ScAccessibleDocumentPagePreview* mpAccDoc;
    ScChildGone(ScAccessibleDocumentPagePreview* pAccDoc) : mpAccDoc(pAccDoc) {}
    void operator() (const uno::Reference<XAccessible>& xAccessible) const
    {
        if (mpAccDoc)
        {
            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::CHILD;
            aEvent.Source = uno::Reference< XAccessible >(mpAccDoc);
            aEvent.OldValue <<= xAccessible;

            mpAccDoc->CommitChange(aEvent); // gone child - event
        }
    }
};

struct ScChildNew
{
    ScAccessibleDocumentPagePreview* mpAccDoc;
    ScChildNew(ScAccessibleDocumentPagePreview* pAccDoc) : mpAccDoc(pAccDoc) {}
    void operator() (const uno::Reference<XAccessible>& xAccessible) const
    {
        if (mpAccDoc)
        {
            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::CHILD;
            aEvent.Source = uno::Reference< XAccessible >(mpAccDoc);
            aEvent.NewValue <<= xAccessible;

            mpAccDoc->CommitChange(aEvent); // new child - event
        }
    }
};

void ScNotesChilds::DataChanged(const Rectangle& rVisRect)
{
    if (mpViewShell && mpAccDoc)
    {
        ScXAccList aNewParas;
        ScXAccList aOldParas;
        ScAccNotes aNewMarks;
        mnParagraphs = CheckChanges(mpViewShell->GetLocationData(), rVisRect, sal_True, maMarks, aNewMarks, aOldParas, aNewParas);
        maMarks = aNewMarks;
        ScAccNotes aNewNotes;
        mnParagraphs += CheckChanges(mpViewShell->GetLocationData(), rVisRect, sal_False, maNotes, aNewNotes, aOldParas, aNewParas);
        maNotes = aNewNotes;

        std::for_each(aOldParas.begin(), aOldParas.end(), ScChildGone(mpAccDoc));
        std::for_each(aNewParas.begin(), aNewParas.end(), ScChildNew(mpAccDoc));
    }
}

struct ScChangeOffset
{
    sal_Int32 mnDiff;
    ScChangeOffset(sal_Int32 nDiff) : mnDiff(nDiff) {}
    void operator() (const ScAccNote& rNote)
    {
        if (rNote.mpTextHelper)
            rNote.mpTextHelper->SetStartIndex(rNote.mpTextHelper->GetStartIndex() + mnDiff);
    }
};

void ScNotesChilds::SetOffset(sal_Int32 nNewOffset)
{
    sal_Int32 nDiff(nNewOffset - mnOffset);
    if (nDiff != 0)
    {
        std::for_each(maMarks.begin(), maMarks.end(), ScChangeOffset(nDiff));
        std::for_each(maNotes.begin(), maNotes.end(), ScChangeOffset(nDiff));
        mnOffset = nNewOffset;
    }
}

inline ScDocument* ScNotesChilds::GetDocument() const
{
    ScDocument* pDoc = NULL;
    if (mpViewShell)
        pDoc = mpViewShell->GetDocument();
    return pDoc;
}

class ScIAccessibleViewForwarder : public ::accessibility::IAccessibleViewForwarder
{
public:
    ScIAccessibleViewForwarder();
    ScIAccessibleViewForwarder(ScPreviewShell* pViewShell,
                                ScAccessibleDocumentPagePreview* pAccDoc,
                                const MapMode& aMapMode);
    ~ScIAccessibleViewForwarder();

    ///=====  IAccessibleViewForwarder  ========================================

    virtual sal_Bool IsValid (void) const;
    virtual Rectangle GetVisibleArea() const;
    virtual Point LogicToPixel (const Point& rPoint) const;
    virtual Size LogicToPixel (const Size& rSize) const;
    virtual Point PixelToLogic (const Point& rPoint) const;
    virtual Size PixelToLogic (const Size& rSize) const;

private:
    ScPreviewShell*                     mpViewShell;
    ScAccessibleDocumentPagePreview*    mpAccDoc;
    MapMode                             maMapMode;
    sal_Bool                            mbValid;
};

ScIAccessibleViewForwarder::ScIAccessibleViewForwarder()
    : mbValid(sal_False)
{
}

ScIAccessibleViewForwarder::ScIAccessibleViewForwarder(ScPreviewShell* pViewShell,
                                ScAccessibleDocumentPagePreview* pAccDoc,
                                const MapMode& aMapMode)
    : mpViewShell(pViewShell),
    mpAccDoc(pAccDoc),
    maMapMode(aMapMode),
    mbValid(sal_True)
{
}

ScIAccessibleViewForwarder::~ScIAccessibleViewForwarder()
{
}

///=====  IAccessibleViewForwarder  ========================================

sal_Bool ScIAccessibleViewForwarder::IsValid (void) const
{
    ScUnoGuard aGuard;
    return mbValid;
}

Rectangle ScIAccessibleViewForwarder::GetVisibleArea() const
{
    ScUnoGuard aGuard;
    Rectangle aVisRect;
    Window* pWin = mpViewShell->GetWindow();
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
    ScUnoGuard aGuard;
    Point aPoint;
    Window* pWin = mpViewShell->GetWindow();
    if (pWin && mpAccDoc)
    {
        Rectangle aRect(mpAccDoc->GetBoundingBoxOnScreen());
        aPoint = pWin->LogicToPixel(rPoint, maMapMode) + aRect.TopLeft();
    }

    return aPoint;
}

Size ScIAccessibleViewForwarder::LogicToPixel (const Size& rSize) const
{
    ScUnoGuard aGuard;
    Size aSize;
    Window* pWin = mpViewShell->GetWindow();
    if (pWin)
        aSize = pWin->LogicToPixel(rSize, maMapMode);
    return aSize;
}

Point ScIAccessibleViewForwarder::PixelToLogic (const Point& rPoint) const
{
    ScUnoGuard aGuard;
    Point aPoint;
    Window* pWin = mpViewShell->GetWindow();
    if (pWin && mpAccDoc)
    {
        Rectangle aRect(mpAccDoc->GetBoundingBoxOnScreen());
        aPoint = pWin->PixelToLogic(rPoint - aRect.TopLeft(), maMapMode);
    }
    return aPoint;
}

Size ScIAccessibleViewForwarder::PixelToLogic (const Size& rSize) const
{
    ScUnoGuard aGuard;
    Size aSize;
    Window* pWin = mpViewShell->GetWindow();
    if (pWin)
        aSize = pWin->PixelToLogic(rSize, maMapMode);
    return aSize;
}

struct ScShapeChild
{
    ScShapeChild() : mpAccShape(NULL) {}
    ScShapeChild(const ScShapeChild& rOld);
    ~ScShapeChild();
    mutable ::accessibility::AccessibleShape* mpAccShape;
    com::sun::star::uno::Reference< com::sun::star::drawing::XShape > mxShape;
    sal_Int32 mnRangeId;
};

ScShapeChild::ScShapeChild(const ScShapeChild& rOld)
:
mpAccShape(rOld.mpAccShape),
mxShape(rOld.mxShape),
mnRangeId(rOld.mnRangeId)
{
    if (mpAccShape)
        mpAccShape->acquire();
}

ScShapeChild::~ScShapeChild()
{
    if (mpAccShape)
    {
        mpAccShape->dispose();
        mpAccShape->release();
    }
}

struct ScShapeChildLess
{
    sal_Bool operator()(const ScShapeChild& rChild1, const ScShapeChild& rChild2) const
    {
      sal_Bool bResult(sal_False);
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
    Rectangle       maPixelRect;
    MapMode         maMapMode;
    ScIAccessibleViewForwarder maViewForwarder;
};

typedef std::vector<ScShapeRange> ScShapeRangeVec;

class ScShapeChilds : public SfxListener,
        public ::accessibility::IAccessibleParent
{
public:
    ScShapeChilds(ScPreviewShell* pViewShell, ScAccessibleDocumentPagePreview* pAccDoc);
    ~ScShapeChilds();

    ///=====  SfxListener  =====================================================

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    ///=====  IAccessibleParent  ==============================================

    virtual sal_Bool ReplaceChild (
        ::accessibility::AccessibleShape* pCurrentChild,
        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& _rxShape,
        const long _nIndex,
        const ::accessibility::AccessibleShapeTreeInfo& _rShapeTreeInfo
    )   throw (::com::sun::star::uno::RuntimeException);

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
    ScAccessibleDocumentPagePreview* mpAccDoc;
    ScPreviewShell* mpViewShell;
    ScShapeRangeVec maShapeRanges;

    void FindChanged(ScShapeChildVec& aOld, ScShapeChildVec& aNew) const;
    void FindChanged(ScShapeRange& aOld, ScShapeRange& aNew) const;
    ::accessibility::AccessibleShape* GetAccShape(const ScShapeChild& rShape) const;
    ::accessibility::AccessibleShape* GetAccShape(const ScShapeChildVec& rShapes, sal_Int32 nIndex) const;
    void FillShapes(const Rectangle& aPixelPaintRect, const MapMode& aMapMode, sal_uInt8 nRangeId);
    sal_Bool FindShape(ScShapeChildVec& rShapes, const uno::Reference <drawing::XShape>& xShape, ScShapeChildVec::iterator& rItr) const;
//    void AddShape(const uno::Reference<drawing::XShape>& xShape, SdrLayerID aLayerID);
//    void RemoveShape(const uno::Reference<drawing::XShape>& xShape, SdrLayerID aLayerID);
    SdrPage* GetDrawPage() const;
};

ScShapeChilds::ScShapeChilds(ScPreviewShell* pViewShell, ScAccessibleDocumentPagePreview* pAccDoc)
    :
    mpAccDoc(pAccDoc),
    mpViewShell(pViewShell),
    maShapeRanges(SC_PREVIEW_MAXRANGES)
{
    if (pViewShell)
    {
        SfxBroadcaster* pDrawBC = pViewShell->GetDocument()->GetDrawBroadcaster();
        if (pDrawBC)
            StartListening(*pDrawBC);
    }
}

ScShapeChilds::~ScShapeChilds()
{
    if (mpViewShell)
    {
        SfxBroadcaster* pDrawBC = mpViewShell->GetDocument()->GetDrawBroadcaster();
        if (pDrawBC)
            EndListening(*pDrawBC);
    }
}

void ScShapeChilds::SetDrawBroadcaster()
{
    if (mpViewShell)
    {
        SfxBroadcaster* pDrawBC = mpViewShell->GetDocument()->GetDrawBroadcaster();
        if (pDrawBC)
            StartListening(*pDrawBC, TRUE);
    }
}

void ScShapeChilds::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    if ( rHint.ISA( SdrHint ) )
    {
        const SdrHint* pSdrHint = PTR_CAST( SdrHint, &rHint );
        if (pSdrHint)
        {
            SdrObject* pObj = const_cast<SdrObject*>(pSdrHint->GetObject());
            if (pObj && (pObj->GetPage() == GetDrawPage()))
            {
                switch (pSdrHint->GetKind())
                {
                    case HINT_OBJCHG :         // Objekt geaendert
                    {
                    }
                    break;
                    // no longer necessary
/*                    case HINT_OBJINSERTED :    // Neues Zeichenobjekt eingefuegt
                    {
                        uno::Reference<drawing::XShape> xShape (pObj->getUnoShape(), uno::UNO_QUERY);
                        if (xShape.is())
                            AddShape(xShape, pObj->GetLayer());
                    }
                    break;
                    case HINT_OBJREMOVED :     // Zeichenobjekt aus Liste entfernt
                    {
                        uno::Reference<drawing::XShape> xShape (pObj->getUnoShape(), uno::UNO_QUERY);
                        if (xShape.is())
                            RemoveShape(xShape, pObj->GetLayer());
                    }
                    break;*/
                    default :
                    {
                        // other events are not interesting
                    }
                    break;
                }
            }
        }
    }
}

void ScShapeChilds::FindChanged(ScShapeChildVec& rOld, ScShapeChildVec& rNew) const
{
    ScShapeChildVec::iterator aOldItr = rOld.begin();
    ScShapeChildVec::iterator aOldEnd = rOld.end();
    ScShapeChildVec::const_iterator aNewItr = rNew.begin();
    ScShapeChildVec::const_iterator aNewEnd = rNew.begin();
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
            aEvent.EventId = AccessibleEventId::CHILD;
            aEvent.NewValue <<= xAcc;
            mpAccDoc->CommitChange(aEvent);
            ++aNewItr;
        }
        else
        {
            xAcc = GetAccShape(*aOldItr);
            AccessibleEventObject aEvent;
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
        aEvent.EventId = AccessibleEventId::CHILD;
        aEvent.OldValue <<= xAcc;
        mpAccDoc->CommitChange(aEvent);
        ++aOldItr;
    }
    while (aNewItr != aNewEnd)
    {
        xAcc = GetAccShape(*aNewItr);
        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::CHILD;
        aEvent.NewValue <<= xAcc;
        mpAccDoc->CommitChange(aEvent);
        ++aNewItr;
    }
}

void ScShapeChilds::FindChanged(ScShapeRange& rOld, ScShapeRange& rNew) const
{
    FindChanged(rOld.maBackShapes, rNew.maBackShapes);
    FindChanged(rOld.maForeShapes, rNew.maForeShapes);
    FindChanged(rOld.maControls, rNew.maControls);
}

void ScShapeChilds::DataChanged()
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

struct ScVisAreaChanged
{
    const ScIAccessibleViewForwarder* mpViewForwarder;
    ScVisAreaChanged(const ScIAccessibleViewForwarder* pViewForwarder) : mpViewForwarder(pViewForwarder) {}
    void operator() (const ScShapeChild& rAccShapeData) const
    {
        if (rAccShapeData.mpAccShape)
        {
            rAccShapeData.mpAccShape->ViewForwarderChanged(::accessibility::IAccessibleViewForwarderListener::VISIBLE_AREA, mpViewForwarder);
        }
    }
};

void ScShapeChilds::VisAreaChanged() const
{
    ScShapeRangeVec::const_iterator aEndItr = maShapeRanges.end();
    ScShapeRangeVec::const_iterator aItr = maShapeRanges.begin();
    while (aItr != aEndItr)
    {
        ScVisAreaChanged aVisAreaChanged(&(aItr->maViewForwarder));
        std::for_each(aItr->maBackShapes.begin(), aItr->maBackShapes.end(), aVisAreaChanged);
        std::for_each(aItr->maControls.begin(), aItr->maControls.end(), aVisAreaChanged);
        std::for_each(aItr->maForeShapes.begin(), aItr->maForeShapes.end(), aVisAreaChanged);
        ++aItr;
    }
}

    ///=====  IAccessibleParent  ==============================================

sal_Bool ScShapeChilds::ReplaceChild (::accessibility::AccessibleShape* pCurrentChild,
    const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& _rxShape,
        const long _nIndex, const ::accessibility::AccessibleShapeTreeInfo& _rShapeTreeInfo)
        throw (uno::RuntimeException)
{
    DBG_ERRORFILE("should not be called in the page preview");
    return sal_False;
}

    ///=====  Internal  ========================================================

void ScShapeChilds::Init()
{
    if(mpViewShell)
    {
        const ScPreviewLocationData& rData = mpViewShell->GetLocationData();
        MapMode aMapMode;
        Rectangle aPixelPaintRect;
        sal_uInt8 nRangeId;
        sal_uInt16 nCount(rData.GetDrawRanges());
        for (sal_uInt16 i = 0; i < nCount; ++i)
        {
            rData.GetDrawRange(i, aPixelPaintRect, aMapMode, nRangeId);
            FillShapes(aPixelPaintRect, aMapMode, nRangeId);
        }
    }
}

sal_Int32 ScShapeChilds::GetBackShapeCount() const
{
    sal_Int32 nCount(0);
    ScShapeRangeVec::const_iterator aEndItr = maShapeRanges.end();
    for ( ScShapeRangeVec::const_iterator aItr = maShapeRanges.begin(); aItr != aEndItr; ++aItr )
        nCount += aItr->maBackShapes.size();
    return nCount;
}

uno::Reference<XAccessible> ScShapeChilds::GetBackShape(sal_Int32 nIndex) const
{
    uno::Reference<XAccessible> xAccessible;
    ScShapeRangeVec::const_iterator aEndItr = maShapeRanges.end();
    ScShapeRangeVec::const_iterator aItr = maShapeRanges.begin();
    while ((aItr != aEndItr) && !xAccessible.is())
    {
        sal_Int32 nCount(aItr->maBackShapes.size());
        if(nIndex < nCount)
            xAccessible = GetAccShape(aItr->maBackShapes, nIndex);
        else
            ++aItr;
        nIndex -= nCount;
    }

    if (nIndex >= 0)
        throw lang::IndexOutOfBoundsException();

   return xAccessible;
}

sal_Int32 ScShapeChilds::GetForeShapeCount() const
{
    sal_Int32 nCount(0);
    ScShapeRangeVec::const_iterator aEndItr = maShapeRanges.end();
    for ( ScShapeRangeVec::const_iterator aItr = maShapeRanges.begin(); aItr != aEndItr; ++aItr )
        nCount += aItr->maForeShapes.size();
    return nCount;
}

uno::Reference<XAccessible> ScShapeChilds::GetForeShape(sal_Int32 nIndex) const
{
    uno::Reference<XAccessible> xAccessible;
    ScShapeRangeVec::const_iterator aEndItr = maShapeRanges.end();
    ScShapeRangeVec::const_iterator aItr = maShapeRanges.begin();
    while ((aItr != aEndItr) && !xAccessible.is())
    {
        sal_Int32 nCount(aItr->maForeShapes.size());
        if(nIndex < nCount)
            xAccessible = GetAccShape(aItr->maForeShapes, nIndex);
        else
            ++aItr;
        nIndex -= nCount;
    }

    if (nIndex >= 0)
        throw lang::IndexOutOfBoundsException();

   return xAccessible;
}

sal_Int32 ScShapeChilds::GetControlCount() const
{
    sal_Int32 nCount(0);
    ScShapeRangeVec::const_iterator aEndItr = maShapeRanges.end();
    for ( ScShapeRangeVec::const_iterator aItr = maShapeRanges.begin(); aItr != aEndItr; ++aItr )
        nCount += aItr->maControls.size();
    return nCount;
}

uno::Reference<XAccessible> ScShapeChilds::GetControl(sal_Int32 nIndex) const
{
    uno::Reference<XAccessible> xAccessible;
    ScShapeRangeVec::const_iterator aEndItr = maShapeRanges.end();
    ScShapeRangeVec::const_iterator aItr = maShapeRanges.begin();
    while ((aItr != aEndItr) && !xAccessible.is())
    {
        sal_Int32 nCount(aItr->maControls.size());
        if(nIndex < nCount)
            xAccessible = GetAccShape(aItr->maControls, nIndex);
        else
            ++aItr;
        nIndex -= nCount;
    }

    if (nIndex >= 0)
        throw lang::IndexOutOfBoundsException();

   return xAccessible;
}

struct ScShapePointFound
{
    Point maPoint;
    ScShapePointFound(const awt::Point& rPoint) : maPoint(VCLPoint(rPoint)) {}
    sal_Bool operator() (const ScShapeChild& rShape)
    {
        sal_Bool bResult(sal_False);
        if ((VCLRectangle(rShape.mpAccShape->getBounds())).IsInside(maPoint))
            bResult = sal_True;
        return bResult;
    }
};

uno::Reference<XAccessible> ScShapeChilds::GetForegroundShapeAt(const awt::Point& rPoint) const //inclusive Controls
{
    uno::Reference<XAccessible> xAcc;

    ScShapeRangeVec::const_iterator aItr = maShapeRanges.begin();
    ScShapeRangeVec::const_iterator aEndItr = maShapeRanges.end();
    while((aItr != aEndItr) && !xAcc.is())
    {
        ScShapeChildVec::const_iterator aFindItr = std::find_if(aItr->maForeShapes.begin(), aItr->maForeShapes.end(), ScShapePointFound(rPoint));
        if (aFindItr != aItr->maForeShapes.end())
            xAcc = GetAccShape(*aFindItr);
        else
        {
            ScShapeChildVec::const_iterator aFindItr = std::find_if(aItr->maControls.begin(), aItr->maControls.end(), ScShapePointFound(rPoint));
            if (aFindItr != aItr->maControls.end())
                xAcc = GetAccShape(*aFindItr);
            else
                ++aItr;
        }
    }

    return xAcc;
}

uno::Reference<XAccessible> ScShapeChilds::GetBackgroundShapeAt(const awt::Point& rPoint) const
{
    uno::Reference<XAccessible> xAcc;

    ScShapeRangeVec::const_iterator aItr = maShapeRanges.begin();
    ScShapeRangeVec::const_iterator aEndItr = maShapeRanges.end();
    while((aItr != aEndItr) && !xAcc.is())
    {
        ScShapeChildVec::const_iterator aFindItr = std::find_if(aItr->maBackShapes.begin(), aItr->maBackShapes.end(), ScShapePointFound(rPoint));
        if (aFindItr != aItr->maBackShapes.end())
            xAcc = GetAccShape(*aFindItr);
        else
            ++aItr;
    }

    return xAcc;
}

::accessibility::AccessibleShape* ScShapeChilds::GetAccShape(const ScShapeChild& rShape) const
{
    if (!rShape.mpAccShape)
    {
        ::accessibility::ShapeTypeHandler& rShapeHandler = ::accessibility::ShapeTypeHandler::Instance();
        ::accessibility::AccessibleShapeInfo aShapeInfo(rShape.mxShape, mpAccDoc, const_cast<ScShapeChilds*>(this));

        if (mpViewShell)
        {
            ::accessibility::AccessibleShapeTreeInfo aShapeTreeInfo;
            aShapeTreeInfo.SetSdrView(mpViewShell->GetPreview()->GetDrawView());
            aShapeTreeInfo.SetController(NULL);
            aShapeTreeInfo.SetWindow(mpViewShell->GetWindow());
            aShapeTreeInfo.SetViewForwarder(&(maShapeRanges[rShape.mnRangeId].maViewForwarder));
            rShape.mpAccShape = rShapeHandler.CreateAccessibleObject(aShapeInfo, aShapeTreeInfo);
            if (rShape.mpAccShape)
            {
                rShape.mpAccShape->acquire();
                rShape.mpAccShape->Init();
            }
        }
    }
    return rShape.mpAccShape;
}

::accessibility::AccessibleShape* ScShapeChilds::GetAccShape(const ScShapeChildVec& rShapes, sal_Int32 nIndex) const
{
    return (GetAccShape(rShapes[nIndex]));
}

void ScShapeChilds::FillShapes(const Rectangle& aPixelPaintRect, const MapMode& aMapMode, sal_uInt8 nRangeId)
{
    DBG_ASSERT(nRangeId < maShapeRanges.size(), "this is not a valid range for draw objects");
    SdrPage* pPage = GetDrawPage();
    Window* pWin = mpViewShell->GetWindow();
    if (pPage && pWin)
    {
        sal_Bool bForeAdded(sal_False);
        sal_Bool bBackAdded(sal_False);
        sal_Bool bControlAdded(sal_False);
        Rectangle aClippedPixelPaintRect(aPixelPaintRect);
        if (mpAccDoc)
        {
            Rectangle aRect2(Point(0,0), mpAccDoc->GetBoundingBoxOnScreen().GetSize());
            aClippedPixelPaintRect = aPixelPaintRect.GetIntersection(aRect2);
        }
        maShapeRanges[nRangeId].maPixelRect = aClippedPixelPaintRect;
        maShapeRanges[nRangeId].maMapMode = aMapMode;
        ScIAccessibleViewForwarder aViewForwarder(mpViewShell, mpAccDoc, aMapMode);
        maShapeRanges[nRangeId].maViewForwarder = aViewForwarder;
        sal_uInt32 nCount(pPage->GetObjCount());
        for (sal_uInt32 i = 0; i < nCount; ++i)
        {
            SdrObject* pObj = pPage->GetObj(i);
            if (pObj)
            {
                uno::Reference< drawing::XShape > xShape(pObj->getUnoShape(), uno::UNO_QUERY);
                if (xShape.is())
                {
                    Rectangle aRect(pWin->LogicToPixel(VCLPoint(xShape->getPosition()), aMapMode), pWin->LogicToPixel(VCLSize(xShape->getSize()), aMapMode));
                    if(!aClippedPixelPaintRect.GetIntersection(aRect).IsEmpty())
                    {
                        ScShapeChild aShape;
                        aShape.mxShape = xShape;
                        aShape.mnRangeId = nRangeId;
                        switch (pObj->GetLayer())
                        {
                            case SC_LAYER_INTERN:
                            case SC_LAYER_FRONT:
                            {
                                maShapeRanges[nRangeId].maForeShapes.push_back(aShape);
                                bForeAdded = sal_True;
                            }
                            break;
                            case SC_LAYER_BACK:
                            {
                                maShapeRanges[nRangeId].maBackShapes.push_back(aShape);
                                bBackAdded = sal_True;
                            }
                            break;
                            case SC_LAYER_CONTROLS:
                            {
                                maShapeRanges[nRangeId].maControls.push_back(aShape);
                                bControlAdded = sal_True;
                            }
                            break;
                            default:
                            {
                                DBG_ERRORFILE("I don't know this layer.")
                            }
                            break;
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

sal_Bool ScShapeChilds::FindShape(ScShapeChildVec& rShapes, const uno::Reference <drawing::XShape>& xShape, ScShapeChildVec::iterator& rItr) const
{
    sal_Bool bResult(sal_False);
    ScShapeChild aShape;
    aShape.mxShape = xShape;
    rItr = std::lower_bound(rShapes.begin(), rShapes.end(), aShape, ScShapeChildLess());
    if (rItr->mxShape.get() == xShape.get())
        bResult = sal_True; // if the shape is found

/*#ifndef PRODUCT // test whether it finds truly the correct shape (perhaps it is not really sorted)
    ScShapeChildVec::iterator aDebugItr = std::find(rShapes.begin(), rShapes.end(), aShape);
    DBG_ASSERT(rItr == aDebugItr, "wrong Shape found");
#endif*/
    return bResult;
}

/*void ScShapeChilds::AddShape(const uno::Reference<drawing::XShape>& xShape, SdrLayerID aLayerID)
{
    uno::Reference < XAccessible > xNew;
    Window* pWin = mpViewShell->GetWindow();
    if (pWin)
    {
        ScShapeRangeVec::iterator aEndItr = maShapeRanges.end();
        ScShapeRangeVec::iterator aItr = maShapeRanges.begin();
        sal_Bool bNotify(sal_False);
        uno::Reference <XAccessible> xAcc;
        while (aItr != aEndItr)
        {
            Rectangle aLogicPaintRect(pWin->PixelToLogic(aItr->maPixelRect, aItr->maMapMode));
            Rectangle aRect(VCLPoint(xShape->getPosition()), VCLSize(xShape->getSize()));
            if(!aRect.GetIntersection(aLogicPaintRect).IsEmpty())
            {
                ScShapeChild aShape;
                aShape.mxShape = xShape;
                switch (aLayerID)
                {
                    case SC_LAYER_INTERN:
                    case SC_LAYER_FRONT:
                    {
                        SetAnchor(aShape);
                        aItr->maForeShapes.push_back(aShape);
                        std::sort(aItr->maForeShapes.begin(), aItr->maForeShapes.end(),ScShapeChildLess());

                    }
                    break;
                    case SC_LAYER_BACK:
                    {
                        aItr->maBackShapes.push_back(aShape);
                        std::sort(aItr->maBackShapes.begin(), aItr->maBackShapes.end(),ScShapeChildLess());
                    }
                    break;
                    case SC_LAYER_CONTROLS:
                    {
                        SetAnchor(aShape);
                        aItr->maControls.push_back(aShape);
                        std::sort(aItr->maControls.begin(), aItr->maControls.end(),ScShapeChildLess());
                    }
                    break;
                    default:
                    {
                        DBG_ERRORFILE("I don't know this layer.")
                    }
                    break;
                }
                if (bNotify)
                {
                    xAcc = GetAccShape(aShape);
                    AccessibleEventObject aEvent;
                    aEvent.EventId = AccessibleEventId::CHILD;
                    aEvent.NewValue <<= xAcc;
                    mpAccDoc->CommitChange(aEvent);
                    bNotify = sal_False;
                }
                xAcc = NULL;
            }
            ++aItr;
        }
    }
}*/

/*sal_Bool HaveToNotify(uno::Reference<XAccessible>& xAcc, ScShapeChildVec::iterator aItr)
{
    sal_Bool bResult(sal_False);
    if (aItr->mpAccShape)
    {
        bResult = sal_True;
        xAcc = aItr->mpAccShape;
    }
    else
        DBG_ERRORFILE("No Accessible object found. Don't know how to notify.");
    return bResult;
}*/

/*void ScShapeChilds::RemoveShape(const uno::Reference<drawing::XShape>& xShape, SdrLayerID aLayerID)
{
    ScShapeRangeVec::iterator aEndItr = maShapeRanges.end();
    ScShapeRangeVec::iterator aItr = maShapeRanges.begin();
    ScShapeChildVec::iterator aEraseItr;
    sal_Bool bNotify(sal_False);
    uno::Reference <XAccessible> xAcc;
    while (aItr != aEndItr)
    {
        switch (aLayerID)
        {
            case SC_LAYER_INTERN:
            case SC_LAYER_FRONT:
            {
                if (FindShape(aItr->maForeShapes, xShape, aEraseItr))
                {
                    bNotify = HaveToNotify(xAcc, aEraseItr);
                    aItr->maForeShapes.erase(aEraseItr);
                }
            }
            break;
            case SC_LAYER_BACK:
            {
                if (FindShape(aItr->maBackShapes, xShape, aEraseItr))
                {
                    bNotify = HaveToNotify(xAcc, aEraseItr);
                    aItr->maBackShapes.erase(aEraseItr);
                }
            }
            break;
            case SC_LAYER_CONTROLS:
            {
                if (FindShape(aItr->maControls, xShape, aEraseItr))
                {
                    bNotify = HaveToNotify(xAcc, aEraseItr);
                    aItr->maControls.erase(aEraseItr);
                }
            }
            break;
            default:
            {
                DBG_ERRORFILE("I don't know this layer.")
            }
            break;
        }
        if (bNotify)
        {
            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::CHILD;
            aEvent.OldValue <<= xAcc;
            mpAccDoc->CommitChange(aEvent);
            bNotify = sal_False;
        }
        xAcc = NULL;
        ++aItr;
    }
}*/

SdrPage* ScShapeChilds::GetDrawPage() const
{
    sal_uInt16 nTab( mpViewShell->GetLocationData().GetPrintTab() );
    SdrPage* pDrawPage = NULL;
    if (mpViewShell)
    {
        ScDocument* pDoc = mpViewShell->GetDocument();
        if (pDoc && pDoc->GetDrawLayer())
        {
            ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
            if (pDrawLayer->HasObjects() && (pDrawLayer->GetPageCount() > nTab))
                pDrawPage = pDrawLayer->GetPage(nTab);
        }
    }
    return pDrawPage;
}

struct ScPagePreviewCountData
{
    //  order is background shapes, header, table or notes, footer, foreground shapes, controls

    Rectangle aVisRect;
    long nBackShapes;
    long nHeaders;
    long nTables;
    long nNoteParagraphs;
    long nFooters;
    long nForeShapes;
    long nControls;

    ScPagePreviewCountData( const ScPreviewLocationData& rData, Window* pSizeWindow,
        ScNotesChilds* pNotesChilds, ScShapeChilds* pShapeChilds );

    long GetTotal() const
    {
        return nBackShapes + nHeaders + nTables + nNoteParagraphs + nFooters + nForeShapes + nControls;
    }
};

ScPagePreviewCountData::ScPagePreviewCountData( const ScPreviewLocationData& rData,
                                Window* pSizeWindow, ScNotesChilds* pNotesChilds,
                                ScShapeChilds* pShapeChilds) :
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
    Point aPoint;
    aVisRect = Rectangle( aPoint, aOutputSize );

    Rectangle aObjRect;

    if ( rData.GetHeaderPosition( aObjRect ) && aObjRect.IsOver( aVisRect ) )
        nHeaders = 1;

    if ( rData.GetFooterPosition( aObjRect ) && aObjRect.IsOver( aVisRect ) )
        nFooters = 1;

    if ( rData.HasCellsInRange( aVisRect ) )
        nTables = 1;

    //! shapes...
    nBackShapes = pShapeChilds->GetBackShapeCount();
    nForeShapes = pShapeChilds->GetForeShapeCount();
    nControls = pShapeChilds->GetControlCount();

    // there are only notes if there is no table
    if (nTables == 0)
        nNoteParagraphs = pNotesChilds->GetChildsCount();
}

//=====  internal  ========================================================

ScAccessibleDocumentPagePreview::ScAccessibleDocumentPagePreview(
        const uno::Reference<XAccessible>& rxParent, ScPreviewShell* pViewShell ) :
    ScAccessibleDocumentBase(rxParent),
    mpViewShell(pViewShell),
    mpNotesChilds(NULL),
    mpShapeChilds(NULL),
    mpTable(NULL),
    mpHeader(NULL),
    mpFooter(NULL)
{
    if (pViewShell)
        pViewShell->AddAccessibilityObject(*this);

//    GetNotesChilds(); not neccessary and reduces the creation performance
//    GetShapeChilds();
}

ScAccessibleDocumentPagePreview::~ScAccessibleDocumentPagePreview(void)
{
    if (!ScAccessibleDocumentBase::IsDefunc() && !rBHelper.bInDispose)
    {
        // increment refcount to prevent double call off dtor
        osl_incrementInterlockedCount( &m_refCount );
        // call dispose to inform object wich have a weak reference to this object
        dispose();
    }
}

void SAL_CALL ScAccessibleDocumentPagePreview::disposing()
{
    ScUnoGuard aGuard;
    if (mpTable)
    {
        mpTable->release();
        mpTable = NULL;
    }
    if (mpHeader)
    {
        mpHeader->release();
        mpHeader = NULL;
    }
    if (mpFooter)
    {
        mpFooter->release();
        mpFooter = NULL;
    }

    if (mpViewShell)
    {
        mpViewShell->RemoveAccessibilityObject(*this);
        mpViewShell = NULL;
    }

    // #100593# no need to Dispose the AccessibleTextHelper,
    // as long as mpNotesChilds are destructed here
    if (mpNotesChilds)
        DELETEZ(mpNotesChilds);

    if (mpShapeChilds)
        DELETEZ(mpShapeChilds);

    ScAccessibleDocumentBase::disposing();
}

//=====  SfxListener  =====================================================

void ScAccessibleDocumentPagePreview::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if (rHint.ISA( SfxSimpleHint ) )
    {
        const SfxSimpleHint& rRef = (const SfxSimpleHint&)rHint;
        // only notify if child exist, otherwise it is not necessary
        if ((rRef.GetId() == SC_HINT_DATACHANGED))
        {
            if (mpTable) // if there is no table there is nothing to notify, because no one recongnizes the change
            {
                {
                    uno::Reference<XAccessible> xAcc = mpTable;
                    AccessibleEventObject aEvent;
                    aEvent.EventId = AccessibleEventId::CHILD;
                    aEvent.Source = uno::Reference< XAccessible >(this);
                    aEvent.OldValue <<= xAcc;
                    CommitChange(aEvent);
                }

                mpTable->dispose();
                mpTable->release();
                mpTable = NULL;
            }

            Size aOutputSize;
            Window* pSizeWindow = mpViewShell->GetWindow();
            if ( pSizeWindow )
                aOutputSize = pSizeWindow->GetOutputSizePixel();
            Point aPoint;
            Rectangle aVisRect( aPoint, aOutputSize );
            GetNotesChilds()->DataChanged(aVisRect);

            GetShapeChilds()->DataChanged();

            const ScPreviewLocationData& rData = mpViewShell->GetLocationData();
            ScPagePreviewCountData aCount( rData, mpViewShell->GetWindow(), GetNotesChilds(), GetShapeChilds() );

            if (aCount.nTables > 0)
            {
                //! order is background shapes, header, table or notes, footer, foreground shapes, controls
                sal_Int32 nIndex (aCount.nBackShapes + aCount.nHeaders);

                mpTable = new ScAccessiblePreviewTable( this, mpViewShell, nIndex );
                mpTable->acquire();
                mpTable->Init();

                {
                    uno::Reference<XAccessible> xAcc = mpTable;
                    AccessibleEventObject aEvent;
                    aEvent.EventId = AccessibleEventId::CHILD;
                    aEvent.Source = uno::Reference< XAccessible >(this);
                    aEvent.NewValue <<= xAcc;
                    CommitChange(aEvent);
                }
            }
        }
        else if (rRef.GetId() == SC_HINT_ACC_MAKEDRAWLAYER)
        {
            GetShapeChilds()->SetDrawBroadcaster();
        }
        else if (rRef.GetId() == SC_HINT_ACC_VISAREACHANGED)
        {
            Size aOutputSize;
            Window* pSizeWindow = mpViewShell->GetWindow();
            if ( pSizeWindow )
                aOutputSize = pSizeWindow->GetOutputSizePixel();
            Point aPoint;
            Rectangle aVisRect( aPoint, aOutputSize );
            GetNotesChilds()->DataChanged(aVisRect);

            GetShapeChilds()->VisAreaChanged();

            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::VISIBLE_DATA_CHANGED;
            aEvent.Source = uno::Reference< XAccessible >(this);
            CommitChange(aEvent);
        }
    }
    else if ( rHint.ISA(ScAccWinFocusLostHint) )
    {
        CommitFocusLost();
    }
    else if ( rHint.ISA(ScAccWinFocusGotHint) )
    {
        CommitFocusGained();
    }
    ScAccessibleDocumentBase::Notify(rBC, rHint);
}

//=====  XAccessibleComponent  ============================================

uno::Reference< XAccessible > SAL_CALL ScAccessibleDocumentPagePreview::getAccessibleAtPoint( const awt::Point& rPoint )
                                throw (uno::RuntimeException)
{
    uno::Reference<XAccessible> xAccessible;
    if (containsPoint(rPoint))
    {
        ScUnoGuard aGuard;
        IsObjectValid();

        if ( mpViewShell )
        {
            xAccessible = GetShapeChilds()->GetForegroundShapeAt(rPoint);
            if (!xAccessible.is())
            {
                const ScPreviewLocationData& rData = mpViewShell->GetLocationData();
                ScPagePreviewCountData aCount( rData, mpViewShell->GetWindow(), GetNotesChilds(), GetShapeChilds() );

/*              if ( rData.HasCellsInRange( Rectangle( rPoint, rPoint ) ) )
                {
                    if ( !mpTable && (aCount.nTables > 0) )
                    {
                        //! order is background shapes, header, table or notes, footer, foreground shapes, controls
                        sal_Int32 nIndex (aCount.nBackShapes + aCount.nHeaders);

                        mpTable = new ScAccessiblePreviewTable( this, mpViewShell, nIndex );
                        mpTable->acquire();
                        mpTable->Init();
                    }
                    xAccessible = mpTable;
                }*/
                if ( !mpTable && (aCount.nTables > 0) )
                {
                    //! order is background shapes, header, table or notes, footer, foreground shapes, controls
                    sal_Int32 nIndex (aCount.nBackShapes + aCount.nHeaders);

                    mpTable = new ScAccessiblePreviewTable( this, mpViewShell, nIndex );
                    mpTable->acquire();
                    mpTable->Init();
                }
                if (mpTable && VCLRectangle(mpTable->getBounds()).IsInside(VCLPoint(rPoint)))
                    xAccessible = mpTable;
            }
            if (!xAccessible.is())
                xAccessible = GetNotesChilds()->GetAt(rPoint);
            if (!xAccessible.is())
            {
                if (!mpHeader || !mpFooter)
                {
                    const ScPreviewLocationData& rData = mpViewShell->GetLocationData();
                    ScPagePreviewCountData aCount( rData, mpViewShell->GetWindow(), GetNotesChilds(), GetShapeChilds() );

                    if (!mpHeader)
                    {
                        mpHeader = new ScAccessiblePageHeader( this, mpViewShell, sal_True, aCount.nBackShapes + aCount.nHeaders - 1);
                        mpHeader->acquire();
                    }
                    if (!mpFooter)
                    {
                        mpFooter = new ScAccessiblePageHeader( this, mpViewShell, sal_False, aCount.nBackShapes + aCount.nHeaders + aCount.nTables + aCount.nNoteParagraphs + aCount.nFooters - 1 );
                        mpFooter->acquire();
                    }
                }

                Point aPoint(VCLPoint(rPoint));

                if (VCLRectangle(mpHeader->getBounds()).IsInside(aPoint))
                    xAccessible = mpHeader;
                else if (VCLRectangle(mpFooter->getBounds()).IsInside(aPoint))
                    xAccessible = mpFooter;
            }
            if (!xAccessible.is())
                xAccessible = GetShapeChilds()->GetBackgroundShapeAt(rPoint);
        }
    }

    return xAccessible;
}

void SAL_CALL ScAccessibleDocumentPagePreview::grabFocus() throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
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

long SAL_CALL ScAccessibleDocumentPagePreview::getAccessibleChildCount(void) throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();

    long nRet = 0;
    if ( mpViewShell )
    {
        ScPagePreviewCountData aCount( mpViewShell->GetLocationData(), mpViewShell->GetWindow(), GetNotesChilds(), GetShapeChilds() );
        nRet = aCount.GetTotal();
    }

    return nRet;
}

uno::Reference<XAccessible> SAL_CALL ScAccessibleDocumentPagePreview::getAccessibleChild(long nIndex)
                throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    uno::Reference<XAccessible> xAccessible;

    if ( mpViewShell )
    {
        const ScPreviewLocationData& rData = mpViewShell->GetLocationData();
        ScPagePreviewCountData aCount( rData, mpViewShell->GetWindow(), GetNotesChilds(), GetShapeChilds() );

        if ( nIndex < aCount.nBackShapes )
        {
            xAccessible = GetShapeChilds()->GetBackShape(nIndex);
        }
        else if ( nIndex < aCount.nBackShapes + aCount.nHeaders )
        {
            if ( !mpHeader )
            {
                mpHeader = new ScAccessiblePageHeader( this, mpViewShell, sal_True, nIndex );
                mpHeader->acquire();
            }

            xAccessible = mpHeader;
        }
        else if ( nIndex < aCount.nBackShapes + aCount.nHeaders + aCount.nTables )
        {
            if ( !mpTable )
            {
                mpTable = new ScAccessiblePreviewTable( this, mpViewShell, nIndex );
                mpTable->acquire();
                mpTable->Init();
            }
            xAccessible = mpTable;
        }
        else if ( nIndex < aCount.nBackShapes + aCount.nHeaders + aCount.nNoteParagraphs )
        {
            xAccessible = GetNotesChilds()->GetChild(nIndex - aCount.nBackShapes - aCount.nHeaders);
        }
        else if ( (nIndex < aCount.nBackShapes + aCount.nHeaders + aCount.nTables + aCount.nNoteParagraphs + aCount.nFooters) )
        {
            if ( !mpFooter )
            {
                mpFooter = new ScAccessiblePageHeader( this, mpViewShell, sal_False, nIndex );
                mpFooter->acquire();
            }
            xAccessible = mpFooter;
        }
        else
        {
            sal_Int32 nIdx(nIndex - (aCount.nBackShapes + aCount.nHeaders + aCount.nTables + aCount.nNoteParagraphs + aCount.nFooters));
            if (nIdx < aCount.nForeShapes)
                xAccessible = GetShapeChilds()->GetForeShape(nIdx);
            else
                xAccessible = GetShapeChilds()->GetControl(nIdx - aCount.nForeShapes);
        }
    }

    if ( !xAccessible.is() )
        throw lang::IndexOutOfBoundsException();

    return xAccessible;
}

    /// Return the set of current states.
uno::Reference<XAccessibleStateSet> SAL_CALL ScAccessibleDocumentPagePreview::getAccessibleStateSet(void)
                        throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
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

::rtl::OUString SAL_CALL ScAccessibleDocumentPagePreview::getImplementationName(void)
                    throw (uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ScAccessibleDocumentPagePreview"));
}

uno::Sequence< ::rtl::OUString> SAL_CALL ScAccessibleDocumentPagePreview::getSupportedServiceNames(void)
                    throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aSequence = ScAccessibleContextBase::getSupportedServiceNames();
    sal_Int32 nOldSize(aSequence.getLength());
    aSequence.realloc(nOldSize + 1);
    ::rtl::OUString* pNames = aSequence.getArray();

    pNames[nOldSize] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.AccessibleSpreadsheetPageView"));

    return aSequence;
}

//=====  XTypeProvider  =======================================================

uno::Sequence<sal_Int8> SAL_CALL
    ScAccessibleDocumentPagePreview::getImplementationId(void)
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    static uno::Sequence<sal_Int8> aId;
    if (aId.getLength() == 0)
    {
        aId.realloc (16);
        rtl_createUuid (reinterpret_cast<sal_uInt8 *>(aId.getArray()), 0, sal_True);
    }
    return aId;
}

//=====  internal  ========================================================

::rtl::OUString SAL_CALL ScAccessibleDocumentPagePreview::createAccessibleDescription(void)
                    throw (uno::RuntimeException)
{
    rtl::OUString sDescription = String(ScResId(STR_ACC_PREVIEWDOC_DESCR));
    return sDescription;
}

::rtl::OUString SAL_CALL ScAccessibleDocumentPagePreview::createAccessibleName(void)
                    throw (uno::RuntimeException)
{
    rtl::OUString sName = String(ScResId(STR_ACC_PREVIEWDOC_NAME));
    return sName;
}

Rectangle ScAccessibleDocumentPagePreview::GetBoundingBoxOnScreen() const throw (uno::RuntimeException)
{
    Rectangle aRect;
    if (mpViewShell)
    {
        Window* pWindow = mpViewShell->GetWindow();
        if (pWindow)
            aRect = pWindow->GetWindowExtentsRelative(NULL);
    }
    return aRect;
}

Rectangle ScAccessibleDocumentPagePreview::GetBoundingBox() const throw (uno::RuntimeException)
{
    Rectangle aRect;
    if (mpViewShell)
    {
        Window* pWindow = mpViewShell->GetWindow();
        if (pWindow)
            aRect = pWindow->GetWindowExtentsRelative(pWindow->GetAccessibleParentWindow());
    }
    return aRect;
}

sal_Bool ScAccessibleDocumentPagePreview::IsDefunc(
    const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    return ScAccessibleContextBase::IsDefunc() || !getAccessibleParent().is() ||
        (rxParentStates.is() && rxParentStates->contains(AccessibleStateType::DEFUNC));
}

ScNotesChilds* ScAccessibleDocumentPagePreview::GetNotesChilds()
{
    if (!mpNotesChilds && mpViewShell)
    {
        mpNotesChilds = new ScNotesChilds(mpViewShell, this);

        const ScPreviewLocationData& rData = mpViewShell->GetLocationData();
        ScPagePreviewCountData aCount( rData, mpViewShell->GetWindow(), GetNotesChilds(), GetShapeChilds() );

        //! order is background shapes, header, table or notes, footer, foreground shapes, controls
        mpNotesChilds->Init(aCount.aVisRect, aCount.nBackShapes + aCount.nHeaders);
    }
    return mpNotesChilds;
}

ScShapeChilds* ScAccessibleDocumentPagePreview::GetShapeChilds()
{
    if (!mpShapeChilds && mpViewShell)
    {
        mpShapeChilds = new ScShapeChilds(mpViewShell, this);
        mpShapeChilds->Init();
    }

    return mpShapeChilds;
}

uno::Reference < XAccessible > ScAccessibleDocumentPagePreview::GetCurrentAccessibleTable()
{
    if (!mpTable)
    {
        if ( mpViewShell )
        {
            const ScPreviewLocationData& rData = mpViewShell->GetLocationData();
            ScPagePreviewCountData aCount( rData, mpViewShell->GetWindow(), GetNotesChilds(), GetShapeChilds() );
            //! order is background shapes, header, table or notes, footer, foreground shapes, controls
            sal_Int32 nIndex (aCount.nBackShapes + aCount.nHeaders);

            mpTable = new ScAccessiblePreviewTable( this, mpViewShell, nIndex );
            mpTable->acquire();
            mpTable->Init();
        }
    }
    return mpTable;
}

void ScAccessibleDocumentPagePreview::ChildCountChanged()
{
    if (mpViewShell)
    {
        const ScPreviewLocationData& rData = mpViewShell->GetLocationData();
        ScPagePreviewCountData aCount( rData, mpViewShell->GetWindow(), GetNotesChilds(), GetShapeChilds() );
        //! order is background shapes, header, table or notes, footer, foreground shapes, controls
        if(mpHeader)
            mpHeader->SetCurrentIndexInParent(aCount.nBackShapes);
        if (mpTable)
            mpTable->SetCurrentIndexInParent(aCount.nBackShapes + aCount.nHeaders);
        if (mpFooter)
            mpFooter->SetCurrentIndexInParent(aCount.nBackShapes + aCount.nHeaders + aCount.nTables + aCount.nNoteParagraphs);

        if (mpNotesChilds)
            mpNotesChilds->SetOffset(aCount.nBackShapes + aCount.nHeaders);
    }
}
