/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <memory>
#include "AccessibleText.hxx"
#include "AccessibleCell.hxx"
#include "tabvwsh.hxx"
#include "editutil.hxx"
#include "document.hxx"
#include "scmod.hxx"
#include "prevwsh.hxx"
#include "docsh.hxx"
#include "prevloc.hxx"
#include "patattr.hxx"
#include "inputwin.hxx"
#include <editeng/unofored.hxx>
#include <editeng/editview.hxx>
#include <editeng/unoedhlp.hxx>
#include <vcl/virdev.hxx>
#include <editeng/editobj.hxx>
#include <editeng/adjitem.hxx>
#include <editeng/justifyitem.hxx>
#include <svx/svdmodel.hxx>
#include <svx/algitem.hxx>
#include <vcl/svapp.hxx>


// ============================================================================

class ScViewForwarder : public SvxViewForwarder
{
    ScTabViewShell*     mpViewShell;
    ScAddress           maCellPos;
    ScSplitPos          meSplitPos;
public:
                        ScViewForwarder(ScTabViewShell* pViewShell, ScSplitPos eSplitPos, const ScAddress& rCell);
    virtual             ~ScViewForwarder();

    virtual sal_Bool        IsValid() const;
    virtual Rectangle   GetVisArea() const;
    virtual Point       LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const;
    virtual Point       PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const;

    void                SetInvalid();
};

ScViewForwarder::ScViewForwarder(ScTabViewShell* pViewShell, ScSplitPos eSplitPos, const ScAddress& rCell)
    :
    mpViewShell(pViewShell),
    maCellPos(rCell),
    meSplitPos(eSplitPos)
{
}

ScViewForwarder::~ScViewForwarder()
{
}

sal_Bool ScViewForwarder::IsValid() const
{
    return mpViewShell != NULL;
}

Rectangle ScViewForwarder::GetVisArea() const
{
    Rectangle aVisArea;
    if (mpViewShell)
    {
        Window* pWindow = mpViewShell->GetWindowByPos(meSplitPos);
        if (pWindow)
        {
            aVisArea.SetSize(pWindow->GetSizePixel());

            ScHSplitPos eWhichH = ((meSplitPos == SC_SPLIT_TOPLEFT) || (meSplitPos == SC_SPLIT_BOTTOMLEFT)) ?
                                    SC_SPLIT_LEFT : SC_SPLIT_RIGHT;
            ScVSplitPos eWhichV = ((meSplitPos == SC_SPLIT_TOPLEFT) || (meSplitPos == SC_SPLIT_TOPRIGHT)) ?
                                    SC_SPLIT_TOP : SC_SPLIT_BOTTOM;

            Point aBaseCellPos(mpViewShell->GetViewData()->GetScrPos(mpViewShell->GetViewData()->GetPosX(eWhichH),
                mpViewShell->GetViewData()->GetPosY(eWhichV), meSplitPos, sal_True));
            Point aCellPos(mpViewShell->GetViewData()->GetScrPos(maCellPos.Col(), maCellPos.Row(), meSplitPos, sal_True));
            aVisArea.SetPos(aCellPos - aBaseCellPos);
        }
    }
    else
    {
        OSL_FAIL("this ViewForwarder is not valid");
    }
    return aVisArea;
}

Point ScViewForwarder::LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const
{
    if (mpViewShell)
    {
        Window* pWindow = mpViewShell->GetWindowByPos(meSplitPos);
        if (pWindow)
            return pWindow->LogicToPixel( rPoint, rMapMode );
    }
    else
    {
        OSL_FAIL("this ViewForwarder is not valid");
    }
    return Point();
}

Point ScViewForwarder::PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const
{
    if (mpViewShell)
    {
        Window* pWindow = mpViewShell->GetWindowByPos(meSplitPos);
        if (pWindow)
            return pWindow->PixelToLogic( rPoint, rMapMode );
    }
    else
    {
        OSL_FAIL("this ViewForwarder is not valid");
    }
    return Point();
}

void ScViewForwarder::SetInvalid()
{
    mpViewShell = NULL;
}

// ============================================================================

class ScEditObjectViewForwarder : public SvxViewForwarder
{
    Window*             mpWindow;
    // #i49561# EditView needed for access to its visible area.
    const EditView* mpEditView;
public:
                        ScEditObjectViewForwarder( Window* pWindow,
                                                   const EditView* _pEditView);
    virtual             ~ScEditObjectViewForwarder();

    virtual sal_Bool        IsValid() const;
    virtual Rectangle   GetVisArea() const;
    virtual Point       LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const;
    virtual Point       PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const;

    void                SetInvalid();
};

ScEditObjectViewForwarder::ScEditObjectViewForwarder( Window* pWindow,
                                                      const EditView* _pEditView )
    :
    mpWindow(pWindow),
    mpEditView( _pEditView )
{
}

ScEditObjectViewForwarder::~ScEditObjectViewForwarder()
{
}

sal_Bool ScEditObjectViewForwarder::IsValid() const
{
    return (mpWindow != NULL);
}

Rectangle ScEditObjectViewForwarder::GetVisArea() const
{
    Rectangle aVisArea;
    if (mpWindow)
    {
        Rectangle aVisRect(mpWindow->GetWindowExtentsRelative(mpWindow->GetAccessibleParentWindow()));

        aVisRect.SetPos(Point(0, 0));

        aVisArea = aVisRect;
    }
    else
    {
        OSL_FAIL("this ViewForwarder is not valid");
    }
    return aVisArea;
}

Point ScEditObjectViewForwarder::LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const
{
    if (mpWindow)
    {
        // #i49561# - consider offset of the visible area
        // of the EditView before converting point to pixel.
        Point aPoint( rPoint );
        if ( mpEditView )
        {
            Rectangle aEditViewVisArea( mpEditView->GetVisArea() );
            aPoint += aEditViewVisArea.TopLeft();
        }
        return mpWindow->LogicToPixel( aPoint, rMapMode );
    }
    else
    {
        OSL_FAIL("this ViewForwarder is not valid");
    }
    return Point();
}

Point ScEditObjectViewForwarder::PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const
{
    if (mpWindow)
    {
        // #i49561# - consider offset of the visible area
        // of the EditView after converting point to logic.
        Point aPoint( mpWindow->PixelToLogic( rPoint, rMapMode ) );
        if ( mpEditView )
        {
            Rectangle aEditViewVisArea( mpEditView->GetVisArea() );
            aPoint -= aEditViewVisArea.TopLeft();
        }
        return aPoint;
    }
    else
    {
        OSL_FAIL("this ViewForwarder is not valid");
    }
    return Point();
}

void ScEditObjectViewForwarder::SetInvalid()
{
    mpWindow = NULL;
}

class ScPreviewViewForwarder : public SvxViewForwarder
{
protected:
    ScPreviewShell*     mpViewShell;
    mutable ScPreviewTableInfo* mpTableInfo;
public:
                        ScPreviewViewForwarder(ScPreviewShell* pViewShell);
    virtual             ~ScPreviewViewForwarder();

    virtual sal_Bool        IsValid() const;
    virtual Rectangle   GetVisArea() const;
    virtual Point       LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const;
    virtual Point       PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const;

    void                SetInvalid();

    Rectangle GetVisRect() const;

    // clips the VisArea and calculates with the negativ coordinates
    Rectangle CorrectVisArea(const Rectangle& rVisArea) const;
};

ScPreviewViewForwarder::ScPreviewViewForwarder(ScPreviewShell* pViewShell)
    :
    mpViewShell(pViewShell),
    mpTableInfo(NULL)
{
}

ScPreviewViewForwarder::~ScPreviewViewForwarder()
{
    delete mpTableInfo;
}

sal_Bool ScPreviewViewForwarder::IsValid() const
{
    return mpViewShell != NULL;
}

Rectangle ScPreviewViewForwarder::GetVisArea() const
{
    Rectangle aVisArea;
    OSL_FAIL("should be implemented in an abrevated class");
    return aVisArea;
}

Point ScPreviewViewForwarder::LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const
{
    if (mpViewShell)
    {
        Window* pWindow = mpViewShell->GetWindow();
        if (pWindow)
        {
            MapMode aMapMode(pWindow->GetMapMode().GetMapUnit());
            Point aPoint2( OutputDevice::LogicToLogic( rPoint, rMapMode, aMapMode) );
            return pWindow->LogicToPixel(aPoint2);
        }
    }
    else
    {
        OSL_FAIL("this ViewForwarder is not valid");
    }
    return Point();
}

Point ScPreviewViewForwarder::PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const
{
    if (mpViewShell)
    {
        Window* pWindow = mpViewShell->GetWindow();
        if (pWindow)
        {
            MapMode aMapMode(pWindow->GetMapMode());
            aMapMode.SetOrigin(Point());
            Point aPoint1( pWindow->PixelToLogic( rPoint ) );
            Point aPoint2( OutputDevice::LogicToLogic( aPoint1,
                                                       aMapMode.GetMapUnit(),
                                                       rMapMode ) );
            return aPoint2;
        }
    }
    else
    {
        OSL_FAIL("this ViewForwarder is not valid");
    }
    return Point();
}

void ScPreviewViewForwarder::SetInvalid()
{
    mpViewShell = NULL;
}

Rectangle ScPreviewViewForwarder::GetVisRect() const
{
    if ( mpViewShell )
    {
        Size aOutputSize;
        Window* pWindow = mpViewShell->GetWindow();
        if ( pWindow )
            aOutputSize = pWindow->GetOutputSizePixel();
        Point aPoint;
        Rectangle aVisRect( aPoint, aOutputSize );
        return aVisRect;
    }
    return Rectangle();
}

Rectangle ScPreviewViewForwarder::CorrectVisArea(const Rectangle& rVisArea) const
{
    Rectangle aVisArea(rVisArea);
    Point aPos = aVisArea.TopLeft(); // get first the position to remember negative positions after clipping

    Window* pWin = mpViewShell->GetWindow();
    if (pWin)
        aVisArea = pWin->GetWindowExtentsRelative(pWin).GetIntersection(aVisArea);

    sal_Int32 nX(aPos.getX());
    sal_Int32 nY(aPos.getY());

    if (nX > 0)
        nX = 0;
    else if (nX < 0)
        nX = -nX;
    if (nY > 0)
        nY = 0;
    else if (nY < 0)
        nY = -nY;
    aVisArea.SetPos(Point(nX, nY));

    return aVisArea;
}

class ScPreviewHeaderFooterViewForwarder : public ScPreviewViewForwarder
{
    sal_Bool            mbHeader;
public:
                        ScPreviewHeaderFooterViewForwarder(ScPreviewShell* pViewShell, sal_Bool bHeader);
    virtual             ~ScPreviewHeaderFooterViewForwarder();

    virtual Rectangle   GetVisArea() const;
};

ScPreviewHeaderFooterViewForwarder::ScPreviewHeaderFooterViewForwarder(ScPreviewShell* pViewShell, sal_Bool bHeader)
    :
    ScPreviewViewForwarder(pViewShell),
    mbHeader(bHeader)
{
}

ScPreviewHeaderFooterViewForwarder::~ScPreviewHeaderFooterViewForwarder()
{
}

Rectangle ScPreviewHeaderFooterViewForwarder::GetVisArea() const
{
    Rectangle aVisArea;
    if (mpViewShell)
    {
        const ScPreviewLocationData& rData = mpViewShell->GetLocationData();
        if ( mbHeader )
            rData.GetHeaderPosition( aVisArea );
        else
            rData.GetFooterPosition( aVisArea );

        aVisArea = CorrectVisArea(aVisArea);
    }
    else
    {
        OSL_FAIL("this ViewForwarder is not valid");
    }
    return aVisArea;
}

class ScPreviewCellViewForwarder : public ScPreviewViewForwarder
{
    ScAddress           maCellPos;
public:
                        ScPreviewCellViewForwarder(ScPreviewShell* pViewShell,
                            ScAddress aCellPos);
    virtual             ~ScPreviewCellViewForwarder();

    virtual Rectangle   GetVisArea() const;
};

ScPreviewCellViewForwarder::ScPreviewCellViewForwarder(ScPreviewShell* pViewShell,
                                                       ScAddress aCellPos)
    :
    ScPreviewViewForwarder(pViewShell),
    maCellPos(aCellPos)
{
}

ScPreviewCellViewForwarder::~ScPreviewCellViewForwarder()
{
}

Rectangle ScPreviewCellViewForwarder::GetVisArea() const
{
    Rectangle aVisArea;
    if (mpViewShell)
    {
        const ScPreviewLocationData& rData = mpViewShell->GetLocationData();
        aVisArea = rData.GetCellOutputRect(maCellPos);

        aVisArea = CorrectVisArea(aVisArea);
    }
    else
    {
        OSL_FAIL("this ViewForwarder is not valid");
    }
    return aVisArea;
}

class ScPreviewHeaderCellViewForwarder : public ScPreviewViewForwarder
{
    ScAddress           maCellPos;
    sal_Bool            mbColHeader;
public:
                        ScPreviewHeaderCellViewForwarder(ScPreviewShell* pViewShell,
                            ScAddress aCellPos,
                            sal_Bool bColHeader);
    virtual             ~ScPreviewHeaderCellViewForwarder();

    virtual Rectangle   GetVisArea() const;
};

ScPreviewHeaderCellViewForwarder::ScPreviewHeaderCellViewForwarder(ScPreviewShell* pViewShell,
                                                                   ScAddress aCellPos,
                                                                   sal_Bool bColHeader)
    :
    ScPreviewViewForwarder(pViewShell),
    maCellPos(aCellPos),
    mbColHeader(bColHeader)
{
}

ScPreviewHeaderCellViewForwarder::~ScPreviewHeaderCellViewForwarder()
{
}

Rectangle ScPreviewHeaderCellViewForwarder::GetVisArea() const
{
    Rectangle aVisArea;
    if (mpViewShell)
    {
        const ScPreviewLocationData& rData = mpViewShell->GetLocationData();
        aVisArea = rData.GetHeaderCellOutputRect(GetVisRect(), maCellPos, mbColHeader);

        aVisArea = CorrectVisArea(aVisArea);
    }
    else
    {
        OSL_FAIL("this ViewForwarder is not valid");
    }
    return aVisArea;
}

class ScPreviewNoteViewForwarder : public ScPreviewViewForwarder
{
    ScAddress           maCellPos;
    sal_Bool            mbNoteMark;
public:
                        ScPreviewNoteViewForwarder(ScPreviewShell* pViewShell,
                            ScAddress aCellPos,
                            sal_Bool bNoteMark);
    virtual             ~ScPreviewNoteViewForwarder();

    virtual Rectangle   GetVisArea() const;
};

ScPreviewNoteViewForwarder::ScPreviewNoteViewForwarder(ScPreviewShell* pViewShell,
                                                                   ScAddress aCellPos,
                                                                   sal_Bool bNoteMark)
    :
    ScPreviewViewForwarder(pViewShell),
    maCellPos(aCellPos),
    mbNoteMark(bNoteMark)
{
}

ScPreviewNoteViewForwarder::~ScPreviewNoteViewForwarder()
{
}

Rectangle ScPreviewNoteViewForwarder::GetVisArea() const
{
    Rectangle aVisArea;
    if (mpViewShell)
    {
        const ScPreviewLocationData& rData = mpViewShell->GetLocationData();
        aVisArea = rData.GetNoteInRangeOutputRect(GetVisRect(), mbNoteMark, maCellPos);

        aVisArea = CorrectVisArea(aVisArea);
    }
    else
    {
        OSL_FAIL("this ViewForwarder is not valid");
    }
    return aVisArea;
}

class ScEditViewForwarder : public SvxEditViewForwarder
{
    EditView*           mpEditView;
    Window*             mpWindow;
public:
                        ScEditViewForwarder(EditView* pEditView, Window* pWin);
    virtual             ~ScEditViewForwarder();

    virtual sal_Bool        IsValid() const;
    virtual Rectangle   GetVisArea() const;
    virtual Point       LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const;
    virtual Point       PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const;
    virtual sal_Bool    GetSelection( ESelection& rSelection ) const;
    virtual sal_Bool    SetSelection( const ESelection& rSelection );
    virtual sal_Bool    Copy();
    virtual sal_Bool    Cut();
    virtual sal_Bool    Paste();

    void                GrabFocus();

    void                SetInvalid();
};

ScEditViewForwarder::ScEditViewForwarder(EditView* pEditView, Window* pWin)
    : mpEditView(pEditView),
    mpWindow(pWin)
{
    GrabFocus();
}

ScEditViewForwarder::~ScEditViewForwarder()
{
}

sal_Bool ScEditViewForwarder::IsValid() const
{
    sal_Bool bResult(false);
    if (mpWindow && mpEditView)
    {
        bResult = sal_True;
    }
    return bResult;
}

Rectangle ScEditViewForwarder::GetVisArea() const
{
    Rectangle aVisArea;
    if (IsValid() && mpEditView->GetEditEngine())
    {
        MapMode aMapMode(mpEditView->GetEditEngine()->GetRefMapMode());

        aVisArea = mpWindow->LogicToPixel( mpEditView->GetVisArea(), aMapMode );
    }
    else
    {
        OSL_FAIL("this EditViewForwarder is no longer valid");
    }
    return aVisArea;
}

Point ScEditViewForwarder::LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const
{
    if (mpWindow)
        return mpWindow->LogicToPixel( rPoint, rMapMode );
    else
    {
        OSL_FAIL("this ViewForwarder is not valid");
    }
    return Point();
}

Point ScEditViewForwarder::PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const
{
    if (mpWindow)
        return mpWindow->PixelToLogic( rPoint, rMapMode );
    else
    {
        OSL_FAIL("this ViewForwarder is not valid");
    }
    return Point();
}

sal_Bool ScEditViewForwarder::GetSelection( ESelection& rSelection ) const
{
    sal_Bool bResult(false);
    if (IsValid())
    {
        rSelection = mpEditView->GetSelection();
        bResult = sal_True;
    }
    else
    {
        OSL_FAIL("this ViewForwarder is not valid");
    }
    return bResult;
}

sal_Bool ScEditViewForwarder::SetSelection( const ESelection& rSelection )
{
    sal_Bool bResult(false);
    if (IsValid())
    {
        mpEditView->SetSelection(rSelection);
        bResult = sal_True;
    }
    else
    {
        OSL_FAIL("this ViewForwarder is not valid");
    }
    return bResult;
}

sal_Bool ScEditViewForwarder::Copy()
{
    sal_Bool bResult(false);
    if (IsValid())
    {
        mpEditView->Copy();
        bResult = sal_True;
    }
    else
    {
        OSL_FAIL("this ViewForwarder is not valid");
    }
    return bResult;
}

sal_Bool ScEditViewForwarder::Cut()
{
    sal_Bool bResult(false);
    if (IsValid())
    {
        mpEditView->Cut();
        bResult = sal_True;
    }
    else
    {
        OSL_FAIL("this ViewForwarder is not valid");
    }
    return bResult;
}

sal_Bool ScEditViewForwarder::Paste()
{
    sal_Bool bResult(false);
    if (IsValid())
    {
        mpEditView->Paste();
        bResult = sal_True;
    }
    else
    {
        OSL_FAIL("this ViewForwarder is not valid");
    }
    return bResult;
}

void ScEditViewForwarder::GrabFocus()
{
}

void ScEditViewForwarder::SetInvalid()
{
    mpWindow = NULL;
    mpEditView = NULL;
}

//  ScAccessibleCellTextData: shared data between sub objects of a accessible cell text object

ScAccessibleCellTextData::ScAccessibleCellTextData(ScTabViewShell* pViewShell,
        const ScAddress& rP, ScSplitPos eSplitPos, ScAccessibleCell* pAccCell)
    : ScAccessibleCellBaseTextData(GetDocShell(pViewShell), rP),
    mpViewForwarder(NULL),
    mpEditViewForwarder(NULL),
    mpViewShell(pViewShell),
    meSplitPos(eSplitPos),
    mpAccessibleCell( pAccCell )
{
}

ScAccessibleCellTextData::~ScAccessibleCellTextData()
{
    if (pEditEngine)
        pEditEngine->SetNotifyHdl(Link());
    if (mpViewForwarder)
        delete mpViewForwarder;
    if (mpEditViewForwarder)
        delete mpEditViewForwarder;
}

void ScAccessibleCellTextData::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( SfxSimpleHint ) )
    {
        sal_uLong nId = ((const SfxSimpleHint&)rHint).GetId();
        if ( nId == SFX_HINT_DYING )
        {
            mpViewShell = NULL;                     // invalid now
            if (mpViewForwarder)
                mpViewForwarder->SetInvalid();
            if (mpEditViewForwarder)
                mpEditViewForwarder->SetInvalid();
        }
    }
    ScAccessibleCellBaseTextData::Notify(rBC, rHint);
}

ScAccessibleTextData* ScAccessibleCellTextData::Clone() const
{
    return new ScAccessibleCellTextData( mpViewShell, aCellPos, meSplitPos, mpAccessibleCell );
}

void ScAccessibleCellTextData::GetCellText(const ScAddress& rCellPos, String& rText)
{
//  #104893#; don't use the input string
//    ScCellTextData::GetCellText(rCellPos, rText);
    ScDocument* pDoc = pDocShell->GetDocument();
    if (pDoc)
    {
        //  #104893#; use the displayed string
        pDoc->GetString(rCellPos.Col(), rCellPos.Row(), rCellPos.Tab(), rText);
        if (mpViewShell)
        {
            const ScViewOptions& aOptions = mpViewShell->GetViewData()->GetOptions();
            CellType aCellType;
            pDoc->GetCellType(rCellPos.Col(), rCellPos.Row(), rCellPos.Tab(), aCellType);
            if (aCellType == CELLTYPE_FORMULA && aOptions.GetOption( VOPT_FORMULAS ))
            {
                pDoc->GetFormula( rCellPos.Col(), rCellPos.Row(), rCellPos.Tab(), rText);
            }
            else if (!aOptions.GetOption( VOPT_NULLVALS ))
            {
                if ((aCellType == CELLTYPE_VALUE || aCellType == CELLTYPE_FORMULA) && pDoc->GetValue(rCellPos) == 0.0)
                    rText.Erase();
            }
        }
    }
}

SvxTextForwarder* ScAccessibleCellTextData::GetTextForwarder()
{
    ScCellTextData::GetTextForwarder(); // creates Forwarder and EditEngine

    ScDocument* pDoc = ( pDocShell ? pDocShell->GetDocument() : NULL );
    if ( pDoc && pEditEngine && mpViewShell )
    {
        long nSizeX, nSizeY;
        mpViewShell->GetViewData()->GetMergeSizePixel(
            aCellPos.Col(), aCellPos.Row(), nSizeX, nSizeY);

        Size aSize(nSizeX, nSizeY);

        // #i92143# text getRangeExtents reports incorrect 'x' values for spreadsheet cells
        long nIndent = 0;
        const SvxHorJustifyItem* pHorJustifyItem = static_cast< const SvxHorJustifyItem* >(
            pDoc->GetAttr( aCellPos.Col(), aCellPos.Row(), aCellPos.Tab(), ATTR_HOR_JUSTIFY ) );
        SvxCellHorJustify eHorJust = ( pHorJustifyItem ? static_cast< SvxCellHorJustify >( pHorJustifyItem->GetValue() ) : SVX_HOR_JUSTIFY_STANDARD );
        if ( eHorJust == SVX_HOR_JUSTIFY_LEFT )
        {
            const SfxUInt16Item* pIndentItem = static_cast< const SfxUInt16Item* >(
                pDoc->GetAttr( aCellPos.Col(), aCellPos.Row(), aCellPos.Tab(), ATTR_INDENT ) );
            if ( pIndentItem )
            {
                nIndent = static_cast< long >( pIndentItem->GetValue() );
            }
        }

        const SvxMarginItem* pMarginItem = static_cast< const SvxMarginItem* >(
            pDoc->GetAttr( aCellPos.Col(), aCellPos.Row(), aCellPos.Tab(), ATTR_MARGIN ) );
        ScViewData* pViewData = mpViewShell->GetViewData();
        double nPPTX = ( pViewData ? pViewData->GetPPTX() : 0 );
        double nPPTY = ( pViewData ? pViewData->GetPPTY() : 0 );
        long nLeftM = ( pMarginItem ? static_cast< long >( ( pMarginItem->GetLeftMargin() + nIndent ) * nPPTX ) : 0 );
        long nTopM = ( pMarginItem ? static_cast< long >( pMarginItem->GetTopMargin() * nPPTY ) : 0 );
        long nRightM = ( pMarginItem ? static_cast< long >( pMarginItem->GetRightMargin() * nPPTX ) : 0 );
        long nBottomM = ( pMarginItem ? static_cast< long >( pMarginItem->GetBottomMargin() * nPPTY ) : 0 );
        long nWidth = aSize.getWidth() - nLeftM - nRightM;
        aSize.setWidth( nWidth );
        aSize.setHeight( aSize.getHeight() - nTopM - nBottomM );

        Window* pWin = mpViewShell->GetWindowByPos( meSplitPos );
        if ( pWin )
        {
            aSize = pWin->PixelToLogic( aSize, pEditEngine->GetRefMapMode() );
        }

        /*  #i19430# Gnopernicus reads text partly if it sticks out of the cell
            boundaries. This leads to wrong results in cases where the cell text
            is rotated, because rotation is not taken into account when calcu-
            lating the visible part of the text. In these cases we will expand
            the cell size passed as paper size to the edit engine. The function
            accessibility::AccessibleStaticTextBase::GetParagraphBoundingBox()
            (see svx/source/accessibility/AccessibleStaticTextBase.cxx) will
            return the size of the complete text then, which is used to expand
            the cell bounding box in ScAccessibleCell::GetBoundingBox()
            (see sc/source/ui/Accessibility/AccessibleCell.cxx). */
        const SfxInt32Item* pItem = static_cast< const SfxInt32Item* >(
            pDoc->GetAttr( aCellPos.Col(), aCellPos.Row(), aCellPos.Tab(), ATTR_ROTATE_VALUE ) );
        if( pItem && (pItem->GetValue() != 0) )
        {
            pEditEngine->SetPaperSize( Size( LONG_MAX, aSize.getHeight() ) );
            long nTxtWidth = static_cast< long >( pEditEngine->CalcTextWidth() );
            aSize.setWidth( std::max( aSize.getWidth(), nTxtWidth + 2 ) );
        }
        else
        {
            // #i92143# text getRangeExtents reports incorrect 'x' values for spreadsheet cells
            const SfxBoolItem* pLineBreakItem = static_cast< const SfxBoolItem* >(
                pDoc->GetAttr( aCellPos.Col(), aCellPos.Row(), aCellPos.Tab(), ATTR_LINEBREAK ) );
            bool bLineBreak = ( pLineBreakItem && pLineBreakItem->GetValue() );
            if ( !bLineBreak )
            {
                long nTxtWidth = static_cast< long >( pEditEngine->CalcTextWidth() );
                aSize.setWidth( ::std::max( aSize.getWidth(), nTxtWidth ) );
            }
        }

        pEditEngine->SetPaperSize( aSize );

        // #i92143# text getRangeExtents reports incorrect 'x' values for spreadsheet cells
        if ( eHorJust == SVX_HOR_JUSTIFY_STANDARD && pDoc->HasValueData( aCellPos.Col(), aCellPos.Row(), aCellPos.Tab() ) )
        {
            pEditEngine->SetDefaultItem( SvxAdjustItem( SVX_ADJUST_RIGHT, EE_PARA_JUST ) );
        }

        Size aTextSize;
        if ( pWin )
        {
            aTextSize = pWin->LogicToPixel( Size( pEditEngine->CalcTextWidth(), pEditEngine->GetTextHeight() ), pEditEngine->GetRefMapMode() );
        }
        long nTextWidth = aTextSize.Width();
        long nTextHeight = aTextSize.Height();

        long nOffsetX = nLeftM;
        long nDiffX = nTextWidth - nWidth;
        if ( nDiffX > 0 )
        {
            switch ( eHorJust )
            {
                case SVX_HOR_JUSTIFY_RIGHT:
                    {
                        nOffsetX -= nDiffX;
                    }
                    break;
                case SVX_HOR_JUSTIFY_CENTER:
                    {
                        nOffsetX -= nDiffX / 2;
                    }
                    break;
                default:
                    {
                    }
                    break;
            }
        }

        long nOffsetY = 0;
        const SvxVerJustifyItem* pVerJustifyItem = static_cast< const SvxVerJustifyItem* >(
            pDoc->GetAttr( aCellPos.Col(), aCellPos.Row(), aCellPos.Tab(), ATTR_VER_JUSTIFY ) );
        SvxCellVerJustify eVerJust = ( pVerJustifyItem ? static_cast< SvxCellVerJustify >( pVerJustifyItem->GetValue() ) : SVX_VER_JUSTIFY_STANDARD );
        switch ( eVerJust )
        {
            case SVX_VER_JUSTIFY_STANDARD:
            case SVX_VER_JUSTIFY_BOTTOM:
                {
                    nOffsetY = nSizeY - nBottomM - nTextHeight;
                }
                break;
            case SVX_VER_JUSTIFY_CENTER:
                {
                    nOffsetY = ( nSizeY - nTopM - nBottomM - nTextHeight ) / 2 + nTopM;
                }
                break;
            default:
                {
                    nOffsetY = nTopM;
                }
                break;
        }

        if ( mpAccessibleCell )
        {
            mpAccessibleCell->SetOffset( Point( nOffsetX, nOffsetY ) );
        }

        pEditEngine->SetNotifyHdl( LINK(this, ScAccessibleCellTextData, NotifyHdl) );
    }

    return pForwarder;
}

SvxViewForwarder* ScAccessibleCellTextData::GetViewForwarder()
{
    if (!mpViewForwarder)
        mpViewForwarder = new ScViewForwarder(mpViewShell, meSplitPos, aCellPos);
    return mpViewForwarder;
}

SvxEditViewForwarder* ScAccessibleCellTextData::GetEditViewForwarder( sal_Bool /* bCreate */ )
{
    //#102219#; there should no EditViewForwarder be, because the cell is now readonly in this interface
    return NULL;
}

IMPL_LINK(ScAccessibleCellTextData, NotifyHdl, EENotify*, aNotify)
{
    if( aNotify )
    {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr< SfxHint > aHint = SvxEditSourceHelper::EENotification2Hint( aNotify );
        SAL_WNODEPRECATED_DECLARATIONS_POP

        if( aHint.get() )
            GetBroadcaster().Broadcast( *aHint.get() );
    }

    return 0;
}

ScDocShell* ScAccessibleCellTextData::GetDocShell(ScTabViewShell* pViewShell)
{
    ScDocShell* pDocSh = NULL;
    if (pViewShell)
        pDocSh = pViewShell->GetViewData()->GetDocShell();
    return pDocSh;
}


ScAccessibleEditObjectTextData::ScAccessibleEditObjectTextData(EditView* pEditView, Window* pWin)
    :
    mpViewForwarder(NULL),
    mpEditViewForwarder(NULL),
    mpEditView(pEditView),
    mpEditEngine(pEditView ? pEditView->GetEditEngine() : 0),
    mpForwarder(NULL),
    mpWindow(pWin)
{
    if (mpEditEngine)
        mpEditEngine->SetNotifyHdl( LINK(this, ScAccessibleEditObjectTextData, NotifyHdl) );
}

ScAccessibleEditObjectTextData::~ScAccessibleEditObjectTextData()
{
    if (mpEditEngine)
        mpEditEngine->SetNotifyHdl(Link());
    if (mpViewForwarder)
        delete mpViewForwarder;
    if (mpEditViewForwarder)
        delete mpEditViewForwarder;
    if (mpForwarder)
        delete mpForwarder;
}

void ScAccessibleEditObjectTextData::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( SfxSimpleHint ) )
    {
        sal_uLong nId = ((const SfxSimpleHint&)rHint).GetId();
        if ( nId == SFX_HINT_DYING )
        {
            mpWindow = NULL;
            mpEditView = NULL;
            mpEditEngine = NULL;
            DELETEZ(mpForwarder);
            if (mpViewForwarder)
                mpViewForwarder->SetInvalid();
            if (mpEditViewForwarder)
                mpEditViewForwarder->SetInvalid();
        }
    }
    ScAccessibleTextData::Notify(rBC, rHint);
}

ScAccessibleTextData* ScAccessibleEditObjectTextData::Clone() const
{
    return new ScAccessibleEditObjectTextData(mpEditView, mpWindow);
}

SvxTextForwarder* ScAccessibleEditObjectTextData::GetTextForwarder()
{
    if ((!mpForwarder && mpEditView) || (mpEditEngine && !mpEditEngine->GetNotifyHdl().IsSet()))
    {
        if (!mpEditEngine)
            mpEditEngine = mpEditView->GetEditEngine();
        if (mpEditEngine && !mpEditEngine->GetNotifyHdl().IsSet())
            mpEditEngine->SetNotifyHdl( LINK(this, ScAccessibleEditObjectTextData, NotifyHdl) );
        if(!mpForwarder)
            mpForwarder = new SvxEditEngineForwarder(*mpEditEngine);
    }
    return mpForwarder;
}

SvxViewForwarder* ScAccessibleEditObjectTextData::GetViewForwarder()
{
    if (!mpViewForwarder)
    {
        // i#49561 Get right-aligned cell content to be read by screenreader.
        mpViewForwarder = new ScEditObjectViewForwarder( mpWindow, mpEditView );
    }
    return mpViewForwarder;
}

SvxEditViewForwarder* ScAccessibleEditObjectTextData::GetEditViewForwarder( sal_Bool bCreate )
{
    if (!mpEditViewForwarder && mpEditView)
        mpEditViewForwarder = new ScEditViewForwarder(mpEditView, mpWindow);
    if (bCreate)
    {
        if (!mpEditView && mpEditViewForwarder)
        {
            DELETEZ(mpEditViewForwarder);
        }
        else if (mpEditViewForwarder)
            mpEditViewForwarder->GrabFocus();
    }
    return mpEditViewForwarder;
}

IMPL_LINK(ScAccessibleEditObjectTextData, NotifyHdl, EENotify*, aNotify)
{
    if( aNotify )
    {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr< SfxHint > aHint = SvxEditSourceHelper::EENotification2Hint( aNotify );
        SAL_WNODEPRECATED_DECLARATIONS_POP

        if( aHint.get() )
            GetBroadcaster().Broadcast( *aHint.get() );
    }

    return 0;
}

ScAccessibleEditLineTextData::ScAccessibleEditLineTextData(EditView* pEditView, Window* pWin)
    :
    ScAccessibleEditObjectTextData(pEditView, pWin),
    mbEditEngineCreated(false)
{
    ScTextWnd* pTxtWnd = dynamic_cast<ScTextWnd*>( pWin );

    if (pTxtWnd)
        pTxtWnd->InsertAccessibleTextData( *this );
}

ScAccessibleEditLineTextData::~ScAccessibleEditLineTextData()
{
    ScTextWnd* pTxtWnd = dynamic_cast< ScTextWnd* >(mpWindow);

    if (pTxtWnd)
        pTxtWnd->RemoveAccessibleTextData( *this );

    if (mbEditEngineCreated && mpEditEngine)
    {
        delete mpEditEngine;
        mpEditEngine = NULL;    // don't access in ScAccessibleEditObjectTextData dtor!
    }
    else if (pTxtWnd && pTxtWnd->GetEditView() && pTxtWnd->GetEditView()->GetEditEngine())
    {
        //  the NotifyHdl also has to be removed from the ScTextWnd's EditEngine
        //  (it's set in ScAccessibleEditLineTextData::GetTextForwarder, and mpEditEngine
        //  is reset there)
        pTxtWnd->GetEditView()->GetEditEngine()->SetNotifyHdl(Link());
    }
}

void ScAccessibleEditLineTextData::Dispose()
{
    ScTextWnd* pTxtWnd = dynamic_cast<ScTextWnd*>(mpWindow);

    if (pTxtWnd)
        pTxtWnd->RemoveAccessibleTextData( *this );

    ResetEditMode();
    mpWindow = NULL;
}

ScAccessibleTextData* ScAccessibleEditLineTextData::Clone() const
{
    return new ScAccessibleEditLineTextData(mpEditView, mpWindow);
}

SvxTextForwarder* ScAccessibleEditLineTextData::GetTextForwarder()
{
    ScTextWnd* pTxtWnd = dynamic_cast<ScTextWnd*>(mpWindow);

    if (pTxtWnd)
    {
        mpEditView = pTxtWnd->GetEditView();
        if (mpEditView)
        {
            if (mbEditEngineCreated && mpEditEngine)
                ResetEditMode();
            mbEditEngineCreated = false;

            mpEditView = pTxtWnd->GetEditView();
            ScAccessibleEditObjectTextData::GetTextForwarder(); // fill the mpForwarder
            mpEditEngine = NULL;
        }
        else
        {
            if (mpEditEngine && !mbEditEngineCreated)
                ResetEditMode();
            if (!mpEditEngine)
            {
                SfxItemPool* pEnginePool = EditEngine::CreatePool();
                pEnginePool->FreezeIdRanges();
                mpEditEngine = new ScFieldEditEngine(NULL, pEnginePool, NULL, true);
                mbEditEngineCreated = sal_True;
                mpEditEngine->EnableUndo( false );
                mpEditEngine->SetRefMapMode( MAP_100TH_MM );
                mpForwarder = new SvxEditEngineForwarder(*mpEditEngine);

                mpEditEngine->SetText(pTxtWnd->GetTextString());

                Size aSize(pTxtWnd->GetSizePixel());

                aSize = pTxtWnd->PixelToLogic(aSize, mpEditEngine->GetRefMapMode());

                mpEditEngine->SetPaperSize(aSize);

                mpEditEngine->SetNotifyHdl( LINK(this, ScAccessibleEditObjectTextData, NotifyHdl) );
            }
        }
    }
    return mpForwarder;
}

SvxEditViewForwarder* ScAccessibleEditLineTextData::GetEditViewForwarder( sal_Bool bCreate )
{
    ScTextWnd* pTxtWnd = dynamic_cast<ScTextWnd*>(mpWindow);

    if (pTxtWnd)
    {
        mpEditView = pTxtWnd->GetEditView();
        if (!mpEditView && bCreate)
        {
            if ( !pTxtWnd->IsInputActive() )
            {
                pTxtWnd->StartEditEngine();
                pTxtWnd->GrabFocus();

                mpEditView = pTxtWnd->GetEditView();
            }
        }
    }

    return ScAccessibleEditObjectTextData::GetEditViewForwarder(bCreate);
}

void ScAccessibleEditLineTextData::ResetEditMode()
{
    ScTextWnd* pTxtWnd = dynamic_cast<ScTextWnd*>(mpWindow);

    if (mbEditEngineCreated && mpEditEngine)
        delete mpEditEngine;
    else if (pTxtWnd && pTxtWnd->GetEditView() && pTxtWnd->GetEditView()->GetEditEngine())
        pTxtWnd->GetEditView()->GetEditEngine()->SetNotifyHdl(Link());
    mpEditEngine = NULL;

    DELETEZ(mpForwarder);
    DELETEZ(mpEditViewForwarder);
    DELETEZ(mpViewForwarder);
    mbEditEngineCreated = false;
}

void ScAccessibleEditLineTextData::TextChanged()
{
    if (mbEditEngineCreated && mpEditEngine)
    {
        ScTextWnd* pTxtWnd = dynamic_cast<ScTextWnd*>(mpWindow);

        if (pTxtWnd)
            mpEditEngine->SetText(pTxtWnd->GetTextString());
    }
}

void ScAccessibleEditLineTextData::StartEdit()
{
    ResetEditMode();
    mpEditView = NULL;

    // send HINT_BEGEDIT
    SdrHint aHint(HINT_BEGEDIT);
    GetBroadcaster().Broadcast( aHint );
}

void ScAccessibleEditLineTextData::EndEdit()
{
    // send HINT_ENDEDIT
    SdrHint aHint(HINT_ENDEDIT);
    GetBroadcaster().Broadcast( aHint );

    ResetEditMode();
    mpEditView = NULL;
}


//  ScAccessiblePreviewCellTextData: shared data between sub objects of a accessible cell text object

ScAccessiblePreviewCellTextData::ScAccessiblePreviewCellTextData(ScPreviewShell* pViewShell,
                            const ScAddress& rP)
    : ScAccessibleCellBaseTextData(GetDocShell(pViewShell), rP),
    mpViewForwarder(NULL),
    mpViewShell(pViewShell)
{
}

ScAccessiblePreviewCellTextData::~ScAccessiblePreviewCellTextData()
{
    if (pEditEngine)
        pEditEngine->SetNotifyHdl(Link());
    if (mpViewForwarder)
        delete mpViewForwarder;
}

void ScAccessiblePreviewCellTextData::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( SfxSimpleHint ) )
    {
        sal_uLong nId = ((const SfxSimpleHint&)rHint).GetId();
        if ( nId == SFX_HINT_DYING )
        {
            mpViewShell = NULL;                     // invalid now
            if (mpViewForwarder)
                mpViewForwarder->SetInvalid();
        }
    }
    ScAccessibleCellBaseTextData::Notify(rBC, rHint);
}

ScAccessibleTextData* ScAccessiblePreviewCellTextData::Clone() const
{
    return new ScAccessiblePreviewCellTextData(mpViewShell, aCellPos);
}

SvxTextForwarder* ScAccessiblePreviewCellTextData::GetTextForwarder()
{
    sal_Bool bEditEngineBefore(pEditEngine != NULL);

    ScCellTextData::GetTextForwarder(); // creates Forwarder and EditEngine

    if (!bEditEngineBefore && pEditEngine)
    {
        Size aSize(mpViewShell->GetLocationData().GetCellOutputRect(aCellPos).GetSize());
        Window* pWin = mpViewShell->GetWindow();
        if (pWin)
            aSize = pWin->PixelToLogic(aSize, pEditEngine->GetRefMapMode());
        pEditEngine->SetPaperSize(aSize);
    }

    if (pEditEngine)
        pEditEngine->SetNotifyHdl( LINK(this, ScAccessibleCellTextData, NotifyHdl) );

    return pForwarder;
}

SvxViewForwarder* ScAccessiblePreviewCellTextData::GetViewForwarder()
{
    if (!mpViewForwarder)
        mpViewForwarder = new ScPreviewCellViewForwarder(mpViewShell, aCellPos);
    return mpViewForwarder;
}

ScDocShell* ScAccessiblePreviewCellTextData::GetDocShell(ScPreviewShell* pViewShell)
{
    ScDocShell* pDocSh = NULL;
    if (pViewShell && pViewShell->GetDocument())
        pDocSh = (ScDocShell*) pViewShell->GetDocument()->GetDocumentShell();
    return pDocSh;
}


//  ScAccessiblePreviewHeaderCellTextData: shared data between sub objects of a accessible cell text object

ScAccessiblePreviewHeaderCellTextData::ScAccessiblePreviewHeaderCellTextData(ScPreviewShell* pViewShell,
            const String& rText, const ScAddress& rP, sal_Bool bColHeader, sal_Bool bRowHeader)
    : ScAccessibleCellBaseTextData(GetDocShell(pViewShell), rP),
    mpViewForwarder(NULL),
    mpViewShell(pViewShell),
    maText(rText),
    mbColHeader(bColHeader),
    mbRowHeader(bRowHeader)
{
}

ScAccessiblePreviewHeaderCellTextData::~ScAccessiblePreviewHeaderCellTextData()
{
    if (pEditEngine)
        pEditEngine->SetNotifyHdl(Link());
    if (mpViewForwarder)
        delete mpViewForwarder;
}

void ScAccessiblePreviewHeaderCellTextData::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( SfxSimpleHint ) )
    {
        sal_uLong nId = ((const SfxSimpleHint&)rHint).GetId();
        if ( nId == SFX_HINT_DYING )
        {
            mpViewShell = NULL;                     // invalid now
            if (mpViewForwarder)
                mpViewForwarder->SetInvalid();
        }
    }
    ScAccessibleCellBaseTextData::Notify(rBC, rHint);
}

ScAccessibleTextData* ScAccessiblePreviewHeaderCellTextData::Clone() const
{
    return new ScAccessiblePreviewHeaderCellTextData(mpViewShell, maText, aCellPos, mbColHeader, mbRowHeader);
}

SvxTextForwarder* ScAccessiblePreviewHeaderCellTextData::GetTextForwarder()
{
    if (!pEditEngine)
    {
        if ( pDocShell )
        {
            ScDocument* pDoc = pDocShell->GetDocument();
            pEditEngine = pDoc->CreateFieldEditEngine();
        }
        else
        {
            SfxItemPool* pEnginePool = EditEngine::CreatePool();
            pEnginePool->FreezeIdRanges();
            pEditEngine = new ScFieldEditEngine(NULL, pEnginePool, NULL, true);
        }
        pEditEngine->EnableUndo( false );
        if (pDocShell)
            pEditEngine->SetRefDevice(pDocShell->GetRefDevice());
        else
            pEditEngine->SetRefMapMode( MAP_100TH_MM );
        pForwarder = new SvxEditEngineForwarder(*pEditEngine);
    }

    if (bDataValid)
        return pForwarder;

    if (maText.Len() && pEditEngine)
    {

        if ( mpViewShell  )
        {
            Size aOutputSize;
            Window* pWindow = mpViewShell->GetWindow();
            if ( pWindow )
                aOutputSize = pWindow->GetOutputSizePixel();
            Point aPoint;
            Rectangle aVisRect( aPoint, aOutputSize );
            Size aSize(mpViewShell->GetLocationData().GetHeaderCellOutputRect(aVisRect, aCellPos, mbColHeader).GetSize());
            if (pWindow)
                aSize = pWindow->PixelToLogic(aSize, pEditEngine->GetRefMapMode());
            pEditEngine->SetPaperSize(aSize);
        }
        pEditEngine->SetText( maText );
    }

    bDataValid = true;

    if (pEditEngine)
        pEditEngine->SetNotifyHdl( LINK(this, ScAccessibleCellTextData, NotifyHdl) );

    return pForwarder;
}

SvxViewForwarder* ScAccessiblePreviewHeaderCellTextData::GetViewForwarder()
{
    if (!mpViewForwarder)
        mpViewForwarder = new ScPreviewHeaderCellViewForwarder(mpViewShell, aCellPos, mbColHeader);
    return mpViewForwarder;
}

ScDocShell* ScAccessiblePreviewHeaderCellTextData::GetDocShell(ScPreviewShell* pViewShell)
{
    ScDocShell* pDocSh = NULL;
    if (pViewShell && pViewShell->GetDocument())
        pDocSh = (ScDocShell*) pViewShell->GetDocument()->GetDocumentShell();
    return pDocSh;
}

ScAccessibleHeaderTextData::ScAccessibleHeaderTextData(ScPreviewShell* pViewShell,
                            const EditTextObject* pEditObj, sal_Bool bHeader, SvxAdjust eAdjust)
    :
    mpViewForwarder(NULL),
    mpViewShell(pViewShell),
    mpEditEngine(NULL),
    mpForwarder(NULL),
    mpDocSh(NULL),
    mpEditObj(pEditObj),
    mbHeader(bHeader),
    mbDataValid(false),
    meAdjust(eAdjust)
{
    if (pViewShell && pViewShell->GetDocument())
        mpDocSh = (ScDocShell*) pViewShell->GetDocument()->GetDocumentShell();
    if (mpDocSh)
        mpDocSh->GetDocument()->AddUnoObject(*this);
}

ScAccessibleHeaderTextData::~ScAccessibleHeaderTextData()
{
    SolarMutexGuard aGuard;     //  needed for EditEngine dtor

    if (mpDocSh)
        mpDocSh->GetDocument()->RemoveUnoObject(*this);
    if (mpEditEngine)
        mpEditEngine->SetNotifyHdl(Link());
    delete mpEditEngine;
    delete mpForwarder;
}

ScAccessibleTextData* ScAccessibleHeaderTextData::Clone() const
{
    return new ScAccessibleHeaderTextData(mpViewShell, mpEditObj, mbHeader, meAdjust);
}

void ScAccessibleHeaderTextData::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.ISA( SfxSimpleHint ) )
    {
        sal_uLong nId = ((const SfxSimpleHint&)rHint).GetId();
        if ( nId == SFX_HINT_DYING )
        {
            mpViewShell = NULL;// invalid now
            mpDocSh = NULL;
            if (mpViewForwarder)
                mpViewForwarder->SetInvalid();
        }
    }
}

SvxTextForwarder* ScAccessibleHeaderTextData::GetTextForwarder()
{
    if (!mpEditEngine)
    {
        SfxItemPool* pEnginePool = EditEngine::CreatePool();
        pEnginePool->FreezeIdRanges();
        ScHeaderEditEngine* pHdrEngine = new ScHeaderEditEngine( pEnginePool, sal_True );

        pHdrEngine->EnableUndo( false );
        pHdrEngine->SetRefMapMode( MAP_TWIP );

        //  default font must be set, independently of document
        //  -> use global pool from module

        SfxItemSet aDefaults( pHdrEngine->GetEmptyItemSet() );
        const ScPatternAttr& rPattern = (const ScPatternAttr&)SC_MOD()->GetPool().GetDefaultItem(ATTR_PATTERN);
        rPattern.FillEditItemSet( &aDefaults );
        //  FillEditItemSet adjusts font height to 1/100th mm,
        //  but for header/footer twips is needed, as in the PatternAttr:
        aDefaults.Put( rPattern.GetItem(ATTR_FONT_HEIGHT), EE_CHAR_FONTHEIGHT );
        aDefaults.Put( rPattern.GetItem(ATTR_CJK_FONT_HEIGHT), EE_CHAR_FONTHEIGHT_CJK );
        aDefaults.Put( rPattern.GetItem(ATTR_CTL_FONT_HEIGHT), EE_CHAR_FONTHEIGHT_CTL );
        aDefaults.Put( SvxAdjustItem( meAdjust, EE_PARA_JUST ) );
        pHdrEngine->SetDefaults( aDefaults );

        ScHeaderFieldData aData;
        if (mpViewShell)
            mpViewShell->FillFieldData(aData);
        else
            ScHeaderFooterTextObj::FillDummyFieldData( aData );
        pHdrEngine->SetData( aData );

        mpEditEngine = pHdrEngine;
        mpForwarder = new SvxEditEngineForwarder(*mpEditEngine);
    }

    if (mbDataValid)
        return mpForwarder;

    if ( mpViewShell  )
    {
        Rectangle aVisRect;
        mpViewShell->GetLocationData().GetHeaderPosition(aVisRect);
        Size aSize(aVisRect.GetSize());
        Window* pWin = mpViewShell->GetWindow();
        if (pWin)
            aSize = pWin->PixelToLogic(aSize, mpEditEngine->GetRefMapMode());
        mpEditEngine->SetPaperSize(aSize);
    }
    if (mpEditObj)
        mpEditEngine->SetText(*mpEditObj);

    mbDataValid = sal_True;
    return mpForwarder;
}

SvxViewForwarder* ScAccessibleHeaderTextData::GetViewForwarder()
{
    if (!mpViewForwarder)
        mpViewForwarder = new ScPreviewHeaderFooterViewForwarder(mpViewShell, mbHeader);
    return mpViewForwarder;
}

ScAccessibleNoteTextData::ScAccessibleNoteTextData(ScPreviewShell* pViewShell,
                            const String& sText, const ScAddress& aCellPos, sal_Bool bMarkNote)
    :
    mpViewForwarder(NULL),
    mpViewShell(pViewShell),
    mpEditEngine(NULL),
    mpForwarder(NULL),
    mpDocSh(NULL),
    msText(sText),
    maCellPos(aCellPos),
    mbMarkNote(bMarkNote),
    mbDataValid(false)
{
    if (pViewShell && pViewShell->GetDocument())
        mpDocSh = (ScDocShell*) pViewShell->GetDocument()->GetDocumentShell();
    if (mpDocSh)
        mpDocSh->GetDocument()->AddUnoObject(*this);
}

ScAccessibleNoteTextData::~ScAccessibleNoteTextData()
{
    SolarMutexGuard aGuard;     //  needed for EditEngine dtor

    if (mpDocSh)
        mpDocSh->GetDocument()->RemoveUnoObject(*this);
    if (mpEditEngine)
        mpEditEngine->SetNotifyHdl(Link());
    delete mpEditEngine;
    delete mpForwarder;
}

ScAccessibleTextData* ScAccessibleNoteTextData::Clone() const
{
    return new ScAccessibleNoteTextData(mpViewShell, msText, maCellPos, mbMarkNote);
}

void ScAccessibleNoteTextData::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.ISA( SfxSimpleHint ) )
    {
        sal_uLong nId = ((const SfxSimpleHint&)rHint).GetId();
        if ( nId == SFX_HINT_DYING )
        {
            mpViewShell = NULL;// invalid now
            mpDocSh = NULL;
            if (mpViewForwarder)
                mpViewForwarder->SetInvalid();
        }
    }
}

SvxTextForwarder* ScAccessibleNoteTextData::GetTextForwarder()
{
    if (!mpEditEngine)
    {
        if ( mpDocSh )
        {
            ScDocument* pDoc = mpDocSh->GetDocument();
            mpEditEngine = pDoc->CreateFieldEditEngine();
        }
        else
        {
            SfxItemPool* pEnginePool = EditEngine::CreatePool();
            pEnginePool->FreezeIdRanges();
            mpEditEngine = new ScFieldEditEngine(NULL, pEnginePool, NULL, true);
        }
        mpEditEngine->EnableUndo( false );
        if (mpDocSh)
            mpEditEngine->SetRefDevice(mpDocSh->GetRefDevice());
        else
            mpEditEngine->SetRefMapMode( MAP_100TH_MM );
        mpForwarder = new SvxEditEngineForwarder(*mpEditEngine);
    }

    if (mbDataValid)
        return mpForwarder;

    if (msText.Len() && mpEditEngine)
    {

        if ( mpViewShell  )
        {
            Size aOutputSize;
            Window* pWindow = mpViewShell->GetWindow();
            if ( pWindow )
                aOutputSize = pWindow->GetOutputSizePixel();
            Point aPoint;
            Rectangle aVisRect( aPoint, aOutputSize );
            Size aSize(mpViewShell->GetLocationData().GetNoteInRangeOutputRect(aVisRect, mbMarkNote, maCellPos).GetSize());
            if (pWindow)
                aSize = pWindow->PixelToLogic(aSize, mpEditEngine->GetRefMapMode());
            mpEditEngine->SetPaperSize(aSize);
        }
        mpEditEngine->SetText( msText );
    }

    mbDataValid = sal_True;

    if (mpEditEngine)
        mpEditEngine->SetNotifyHdl( LINK(this, ScAccessibleCellTextData, NotifyHdl) );

    return mpForwarder;
}

SvxViewForwarder* ScAccessibleNoteTextData::GetViewForwarder()
{
    if (!mpViewForwarder)
        mpViewForwarder = new ScPreviewNoteViewForwarder(mpViewShell, maCellPos, mbMarkNote);
    return mpViewForwarder;
}


// CSV import =================================================================

class ScCsvViewForwarder : public SvxViewForwarder
{
    Rectangle                   maBoundBox;
    Window*                     mpWindow;

public:
    explicit                    ScCsvViewForwarder( Window* pWindow, const Rectangle& rBoundBox );

    virtual sal_Bool                IsValid() const;
    virtual Rectangle           GetVisArea() const;
    virtual Point               LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const;
    virtual Point               PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const;

    void                        SetInvalid();
};

ScCsvViewForwarder::ScCsvViewForwarder( Window* pWindow, const Rectangle& rBoundBox ) :
    maBoundBox( rBoundBox ),
    mpWindow( pWindow )
{
}

sal_Bool ScCsvViewForwarder::IsValid() const
{
    return mpWindow != NULL;
}

Rectangle ScCsvViewForwarder::GetVisArea() const
{
    return maBoundBox;
}

Point ScCsvViewForwarder::LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const
{
    if( !mpWindow ) return Point();
    return mpWindow->LogicToPixel( rPoint, rMapMode );
}

Point ScCsvViewForwarder::PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const
{
    if( !mpWindow ) return Point();
    return mpWindow->PixelToLogic( rPoint, rMapMode );
}

void ScCsvViewForwarder::SetInvalid()
{
    mpWindow = NULL;
}

ScAccessibleCsvTextData::ScAccessibleCsvTextData(
        Window* pWindow, EditEngine* pEditEngine,
        const String& rCellText, const Rectangle& rBoundBox, const Size& rCellSize ) :
    mpWindow( pWindow ),
    mpEditEngine( pEditEngine ),
    maCellText( rCellText ),
    maBoundBox( rBoundBox ),
    maCellSize( rCellSize )
{
}

ScAccessibleCsvTextData::~ScAccessibleCsvTextData()
{
}

void ScAccessibleCsvTextData::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( SfxSimpleHint ) )
    {
        sal_uLong nId = ((const SfxSimpleHint&)rHint).GetId();
        if( nId == SFX_HINT_DYING )
        {
            mpWindow = NULL;
            mpEditEngine = NULL;
            if (mpViewForwarder.get())
                mpViewForwarder->SetInvalid();
        }
    }
    ScAccessibleTextData::Notify( rBC, rHint );
}

ScAccessibleTextData* ScAccessibleCsvTextData::Clone() const
{
    return new ScAccessibleCsvTextData( mpWindow, mpEditEngine, maCellText, maBoundBox, maCellSize );
}

SvxTextForwarder* ScAccessibleCsvTextData::GetTextForwarder()
{
    if( mpEditEngine )
    {
        mpEditEngine->SetPaperSize( maCellSize );
        mpEditEngine->SetText( maCellText );
        if( !mpTextForwarder.get() )
            mpTextForwarder.reset( new SvxEditEngineForwarder( *mpEditEngine ) );
    }
    else
        mpTextForwarder.reset( NULL );
    return mpTextForwarder.get();
}

SvxViewForwarder* ScAccessibleCsvTextData::GetViewForwarder()
{
    if( !mpViewForwarder.get() )
        mpViewForwarder.reset( new ScCsvViewForwarder( mpWindow, maBoundBox ) );
    return mpViewForwarder.get();
}

SvxEditViewForwarder* ScAccessibleCsvTextData::GetEditViewForwarder( sal_Bool /* bCreate */ )
{
    return NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
