/*************************************************************************
 *
 *  $RCSfile: AccessibleText.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 18:05:43 $
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

#include "scitems.hxx"
#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <memory>

#ifndef _SC_ACCESSIBLETEXT_HXX
#include "AccessibleText.hxx"
#endif

#ifndef SC_TABVWSH_HXX
#include "tabvwsh.hxx"
#endif
#ifndef SC_EDITUTIL_HXX
#include "editutil.hxx"
#endif
#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_SCMOD_HXX
#include "scmod.hxx"
#endif
#ifndef SC_PREVWSH_HXX
#include "prevwsh.hxx"
#endif
#ifndef SC_DOCSHELL_HXX
#include "docsh.hxx"
#endif
#ifndef SC_PREVLOC_HXX
#include "prevloc.hxx"
#endif
#ifndef SC_UNOGUARD_HXX
#include "unoguard.hxx"
#endif
#include "patattr.hxx"
#ifndef SC_INPUTWIN_HXX
#include "inputwin.hxx"
#endif

#ifndef _SVX_UNOFORED_HXX
#include <svx/unofored.hxx>
#endif
#ifndef _MyEDITVIEW_HXX
#include <svx/editview.hxx>
#endif
#ifndef _SVX_UNOEDHLP_HXX
#include <svx/unoedhlp.hxx>
#endif
#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif
#ifndef _EDITOBJ_HXX
#include <svx/editobj.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX
#include <svx/adjitem.hxx>
#endif
#ifndef _SVDMODEL_HXX
#include <svx/svdmodel.hxx>
#endif


// ============================================================================

class ScViewForwarder : public SvxViewForwarder
{
    ScTabViewShell*     mpViewShell;
    ScAddress           maCellPos;
    ScSplitPos          meSplitPos;
public:
                        ScViewForwarder(ScTabViewShell* pViewShell, ScSplitPos eSplitPos, const ScAddress& rCell);
    virtual             ~ScViewForwarder();

    virtual BOOL        IsValid() const;
    virtual Rectangle   GetVisArea() const;
    virtual Point       LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const;
    virtual Point       PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const;

    void                SetInvalid();
};

ScViewForwarder::ScViewForwarder(ScTabViewShell* pViewShell, ScSplitPos eSplitPos, const ScAddress& rCell)
    :
    mpViewShell(pViewShell),
    meSplitPos(eSplitPos),
    maCellPos(rCell)
{
}

ScViewForwarder::~ScViewForwarder()
{
}

BOOL ScViewForwarder::IsValid() const
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
        DBG_ERROR("this ViewForwarder is not valid");
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
        DBG_ERROR("this ViewForwarder is not valid");
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
        DBG_ERROR("this ViewForwarder is not valid");
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
public:
                        ScEditObjectViewForwarder(Window* pWindow);
    virtual             ~ScEditObjectViewForwarder();

    virtual BOOL        IsValid() const;
    virtual Rectangle   GetVisArea() const;
    virtual Point       LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const;
    virtual Point       PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const;

    void                SetInvalid();
};

ScEditObjectViewForwarder::ScEditObjectViewForwarder(Window* pWindow)
    :
    mpWindow(pWindow)
{
}

ScEditObjectViewForwarder::~ScEditObjectViewForwarder()
{
}

BOOL ScEditObjectViewForwarder::IsValid() const
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
        DBG_ERROR("this ViewForwarder is not valid");
    return aVisArea;
}

Point ScEditObjectViewForwarder::LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const
{
    if (mpWindow)
        return mpWindow->LogicToPixel( rPoint, rMapMode );
    else
        DBG_ERROR("this ViewForwarder is not valid");
    return Point();
}

Point ScEditObjectViewForwarder::PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const
{
    if (mpWindow)
        return mpWindow->PixelToLogic( rPoint, rMapMode );
    else
        DBG_ERROR("this ViewForwarder is not valid");
    return Point();
}

void ScEditObjectViewForwarder::SetInvalid()
{
    mpWindow = NULL;
}

// ============================================================================

class ScPreviewViewForwarder : public SvxViewForwarder
{
protected:
    ScPreviewShell*     mpViewShell;
    mutable ScPreviewTableInfo* mpTableInfo;
public:
                        ScPreviewViewForwarder(ScPreviewShell* pViewShell);
    virtual             ~ScPreviewViewForwarder();

    virtual BOOL        IsValid() const;
    virtual Rectangle   GetVisArea() const;
    virtual Point       LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const;
    virtual Point       PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const;

    void                SetInvalid();

    Rectangle GetVisRect() const;
    void FillTableInfo() const;

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

BOOL ScPreviewViewForwarder::IsValid() const
{
    return mpViewShell != NULL;
}

Rectangle ScPreviewViewForwarder::GetVisArea() const
{
    Rectangle aVisArea;
    DBG_ERROR("should be implemented in an abrevated class");
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
        DBG_ERROR("this ViewForwarder is not valid");
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
        DBG_ERROR("this ViewForwarder is not valid");
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

void ScPreviewViewForwarder::FillTableInfo() const
{
    if ( mpViewShell && !mpTableInfo )
    {
        mpTableInfo = new ScPreviewTableInfo;
        mpViewShell->GetLocationData().GetTableInfo( GetVisRect(), *mpTableInfo );
    }
}

Rectangle ScPreviewViewForwarder::CorrectVisArea(const Rectangle& rVisArea) const
{
    Rectangle aVisArea(rVisArea);
    Point aPos;
    aVisArea.getLocation(aPos); // get first the position to remember negative positions after clipping

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

// ============================================================================

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
        DBG_ERROR("this ViewForwarder is not valid");
    return aVisArea;
}

// ============================================================================

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
        DBG_ERROR("this ViewForwarder is not valid");
    return aVisArea;
}

// ============================================================================

class ScPreviewHeaderCellViewForwarder : public ScPreviewViewForwarder
{
    ScAddress           maCellPos;
    sal_Bool            mbColHeader;
    sal_Bool            mbRowHeader;
public:
                        ScPreviewHeaderCellViewForwarder(ScPreviewShell* pViewShell,
                            ScAddress aCellPos,
                            sal_Bool bColHeader, sal_Bool bRowHeader);
    virtual             ~ScPreviewHeaderCellViewForwarder();

    virtual Rectangle   GetVisArea() const;
};

ScPreviewHeaderCellViewForwarder::ScPreviewHeaderCellViewForwarder(ScPreviewShell* pViewShell,
                                                                   ScAddress aCellPos,
                                                                   sal_Bool bColHeader, sal_Bool bRowHeader)
    :
    ScPreviewViewForwarder(pViewShell),
    maCellPos(aCellPos),
    mbColHeader(bColHeader),
    mbRowHeader(bRowHeader)
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
        DBG_ERROR("this ViewForwarder is not valid");
    return aVisArea;
}

// ============================================================================

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
        DBG_ERROR("this ViewForwarder is not valid");
    return aVisArea;
}

// ============================================================================

class ScEditViewForwarder : public SvxEditViewForwarder
{
    EditView*           mpEditView;
    Window*             mpWindow;
public:
                        ScEditViewForwarder(EditView* pEditView, Window* pWin);
    virtual             ~ScEditViewForwarder();

    virtual BOOL        IsValid() const;
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

BOOL ScEditViewForwarder::IsValid() const
{
    sal_Bool bResult(sal_False);
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
        DBG_ERROR("this EditViewForwarder is no longer valid");
    return aVisArea;
}

Point ScEditViewForwarder::LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const
{
    if (mpWindow)
        return mpWindow->LogicToPixel( rPoint, rMapMode );
    else
        DBG_ERROR("this ViewForwarder is not valid");
    return Point();
}

Point ScEditViewForwarder::PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const
{
    if (mpWindow)
        return mpWindow->PixelToLogic( rPoint, rMapMode );
    else
        DBG_ERROR("this ViewForwarder is not valid");
    return Point();
}

sal_Bool ScEditViewForwarder::GetSelection( ESelection& rSelection ) const
{
    sal_Bool bResult(sal_False);
    if (IsValid())
    {
        rSelection = mpEditView->GetSelection();
        bResult = sal_True;
    }
    else
        DBG_ERROR("this ViewForwarder is not valid");
    return bResult;
}

sal_Bool ScEditViewForwarder::SetSelection( const ESelection& rSelection )
{
    sal_Bool bResult(sal_False);
    if (IsValid())
    {
        mpEditView->SetSelection(rSelection);
        bResult = sal_True;
    }
    else
        DBG_ERROR("this ViewForwarder is not valid");
    return bResult;
}

sal_Bool ScEditViewForwarder::Copy()
{
    sal_Bool bResult(sal_False);
    if (IsValid())
    {
        mpEditView->Copy();
        bResult = sal_True;
    }
    else
        DBG_ERROR("this ViewForwarder is not valid");
    return bResult;
}

sal_Bool ScEditViewForwarder::Cut()
{
    sal_Bool bResult(sal_False);
    if (IsValid())
    {
        mpEditView->Cut();
        bResult = sal_True;
    }
    else
        DBG_ERROR("this ViewForwarder is not valid");
    return bResult;
}

sal_Bool ScEditViewForwarder::Paste()
{
    sal_Bool bResult(sal_False);
    if (IsValid())
    {
        mpEditView->Paste();
        bResult = sal_True;
    }
    else
        DBG_ERROR("this ViewForwarder is not valid");
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

// ============================================================================

//  ScAccessibleCellTextData: shared data between sub objects of a accessible cell text object

ScAccessibleCellTextData::ScAccessibleCellTextData(ScTabViewShell* pViewShell,
                            const ScAddress& rP, ScSplitPos eSplitPos)
    : ScAccessibleCellBaseTextData(GetDocShell(pViewShell), rP),
    mpViewShell(pViewShell),
    meSplitPos(eSplitPos),
    mpViewForwarder(NULL),
    mpEditViewForwarder(NULL),
    mbViewEditEngine(sal_False)
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
        ULONG nId = ((const SfxSimpleHint&)rHint).GetId();
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
    return new ScAccessibleCellTextData(mpViewShell, aCellPos, meSplitPos);
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
/*  sal_Bool bHasForwarder(sal_False);
    if (mpViewShell && mpViewShell->GetViewData() &&
        (mpViewShell->GetViewData()->GetCurPos() == aCellPos) &&
        (mpViewShell->GetViewData()->HasEditView(meSplitPos)) &&
        (mpViewShell->GetViewData()->GetEditViewCol() == aCellPos.Col()) &&
        (mpViewShell->GetViewData()->GetEditViewRow() == aCellPos.Row()))
    {
        if (!mbViewEditEngine)
        {
            if (pForwarder)
                DELETEZ( pForwarder );
            if (pEditEngine)
                DELETEZ( pEditEngine );

            sal_uInt16 nCol, nRow;
            EditView* pEditView;
            mpViewShell->GetViewData()->GetEditView( meSplitPos, pEditView, nCol, nRow );
            if (pEditView)
            {
                pEditEngine = (ScFieldEditEngine*)pEditView->GetEditEngine();
                pForwarder = new SvxEditEngineForwarder(*pEditEngine);
                bHasForwarder = sal_True;
            }
        }
        else
            bHasForwarder = sal_True;
    }
    else if (mbViewEditEngine)
    {
        // remove Forwarder created with EditEngine from EditView
        if (pForwarder)
            DELETEZ( pForwarder );
        pEditEngine->SetNotifyHdl(Link());
        // don't delete, because it is the EditEngine of the EditView
        pEditEngine = NULL;
        mbViewEditEngine = sal_False;
    }

    if (!bHasForwarder)*/
        ScCellTextData::GetTextForwarder(); // creates Forwarder and EditEngine

    if (pEditEngine && mpViewShell)
    {
        sal_Int32 nSizeX, nSizeY;
        mpViewShell->GetViewData()->GetMergeSizePixel(
            aCellPos.Col(), aCellPos.Row(), nSizeX, nSizeY);

        Size aSize(nSizeX, nSizeY);

        Window* pWin = mpViewShell->GetWindowByPos(meSplitPos);
        if (pWin)
            aSize = pWin->PixelToLogic(aSize, pEditEngine->GetRefMapMode());

        pEditEngine->SetPaperSize(aSize);

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

SvxEditViewForwarder* ScAccessibleCellTextData::GetEditViewForwarder( sal_Bool bCreate )
{
    //#102219#; there should no EditViewForwarder be, because the cell is now readonly in this interface
/*  if (!mpEditViewForwarder)
    {
        sal_uInt16 nCol, nRow;
        EditView* pEditView;
        mpViewShell->GetViewData()->GetEditView( meSplitPos, pEditView, nCol, nRow );

        mpEditViewForwarder = new ScEditViewForwarder(pEditView, mpViewShell->GetWindowByPos(meSplitPos));
    }
    else if (bCreate)
        mpEditViewForwarder->GrabFocus();
    return mpEditViewForwarder;*/
    return NULL;
}

IMPL_LINK(ScAccessibleCellTextData, NotifyHdl, EENotify*, aNotify)
{
    if( aNotify )
    {
        ::std::auto_ptr< SfxHint > aHint = SvxEditSourceHelper::EENotification2Hint( aNotify );

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


// ============================================================================

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
        ULONG nId = ((const SfxSimpleHint&)rHint).GetId();
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
    if (!mpForwarder && mpEditView)
    {
        if (!mpEditEngine)
        {
            mpEditEngine = mpEditView->GetEditEngine();
            if (mpEditEngine)
                mpEditEngine->SetNotifyHdl( LINK(this, ScAccessibleEditObjectTextData, NotifyHdl) );
        }
        mpForwarder = new SvxEditEngineForwarder(*mpEditEngine);
    }
    return mpForwarder;
}

SvxViewForwarder* ScAccessibleEditObjectTextData::GetViewForwarder()
{
    if (!mpViewForwarder)
        mpViewForwarder = new ScEditObjectViewForwarder(mpWindow);
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
        ::std::auto_ptr< SfxHint > aHint = SvxEditSourceHelper::EENotification2Hint( aNotify );

        if( aHint.get() )
            GetBroadcaster().Broadcast( *aHint.get() );
    }

    return 0;
}


// ============================================================================

ScAccessibleEditLineTextData::ScAccessibleEditLineTextData(EditView* pEditView, Window* pWin)
    :
    ScAccessibleEditObjectTextData(pEditView, pWin),
    mbEditEngineCreated(sal_False)
{
    ScTextWnd* pTxtWnd = (ScTextWnd*)pWin;

    if (pTxtWnd)
        pTxtWnd->SetAccessibleTextData(this);
}

ScAccessibleEditLineTextData::~ScAccessibleEditLineTextData()
{
    ScTextWnd* pTxtWnd = (ScTextWnd*)mpWindow;

    if (pTxtWnd)
        pTxtWnd->SetAccessibleTextData(NULL);

    if (mbEditEngineCreated && mpEditEngine)
    {
        delete mpEditEngine;
        mpEditEngine = NULL;    // #103346# don't access in ScAccessibleEditObjectTextData dtor!
    }
    else if (pTxtWnd && pTxtWnd->GetEditView() && pTxtWnd->GetEditView()->GetEditEngine())
    {
        //  #103346# the NotifyHdl also has to be removed from the ScTextWnd's EditEngine
        //  (it's set in ScAccessibleEditLineTextData::GetTextForwarder, and mpEditEngine
        //  is reset there)
        pTxtWnd->GetEditView()->GetEditEngine()->SetNotifyHdl(Link());
    }
}

void ScAccessibleEditLineTextData::Dispose()
{
    ScTextWnd* pTxtWnd = (ScTextWnd*)mpWindow;

    if (pTxtWnd)
        pTxtWnd->SetAccessibleTextData(NULL);

    ResetEditMode();
    mpWindow = NULL;
}

ScAccessibleTextData* ScAccessibleEditLineTextData::Clone() const
{
    return new ScAccessibleEditLineTextData(mpEditView, mpWindow);
}

SvxTextForwarder* ScAccessibleEditLineTextData::GetTextForwarder()
{
    ScTextWnd* pTxtWnd = (ScTextWnd*)mpWindow;

    if (pTxtWnd)
    {
        mpEditView = pTxtWnd->GetEditView();
        if (mpEditView)
        {
            if (mbEditEngineCreated && mpEditEngine)
                ResetEditMode();
            mbEditEngineCreated = sal_False;

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
                mpEditEngine = new ScFieldEditEngine( pEnginePool, NULL, TRUE );
                mbEditEngineCreated = sal_True;
#if SUPD > 600
                //  currently, GetPortions doesn't work if UpdateMode is FALSE,
                //  this will be fixed (in EditEngine) by src600
        //      pEditEngine->SetUpdateMode( FALSE );
#endif
                mpEditEngine->EnableUndo( FALSE );
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
    ScTextWnd* pTxtWnd = (ScTextWnd*)mpWindow;

    if (pTxtWnd)
    {
        mpEditView = pTxtWnd->GetEditView();
        if (!mpEditView && bCreate)
        {
            if ( !pTxtWnd->IsActive() )
            {
                pTxtWnd->StartEditEngine();
                pTxtWnd->GrabFocus();
//              pTxtWnd->SetTextString( rText );
//              pTxtWnd->GetEditView()->SetSelection( rSel );

                mpEditView = pTxtWnd->GetEditView();
            }
        }
    }

    return ScAccessibleEditObjectTextData::GetEditViewForwarder(bCreate);
}

void ScAccessibleEditLineTextData::ResetEditMode()
{
    ScTextWnd* pTxtWnd = (ScTextWnd*)mpWindow;

    if (mbEditEngineCreated && mpEditEngine)
        delete mpEditEngine;
    else if (pTxtWnd && pTxtWnd->GetEditView() && pTxtWnd->GetEditView()->GetEditEngine())
        pTxtWnd->GetEditView()->GetEditEngine()->SetNotifyHdl(Link());
    mpEditEngine = NULL;

    DELETEZ(mpForwarder);
    DELETEZ(mpEditViewForwarder);
    mbEditEngineCreated = sal_False;
}

void ScAccessibleEditLineTextData::TextChanged()
{
    if (mbEditEngineCreated && mpEditEngine)
    {
        ScTextWnd* pTxtWnd = (ScTextWnd*)mpWindow;

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


// ============================================================================

//  ScAccessiblePreviewCellTextData: shared data between sub objects of a accessible cell text object

ScAccessiblePreviewCellTextData::ScAccessiblePreviewCellTextData(ScPreviewShell* pViewShell,
                            const ScAddress& rP)
    : ScAccessibleCellBaseTextData(GetDocShell(pViewShell), rP),
    mpViewShell(pViewShell),
    mpViewForwarder(NULL)
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
        ULONG nId = ((const SfxSimpleHint&)rHint).GetId();
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

IMPL_LINK(ScAccessiblePreviewCellTextData, NotifyHdl, EENotify*, aNotify)
{
    if( aNotify )
    {
        ::std::auto_ptr< SfxHint > aHint = SvxEditSourceHelper::EENotification2Hint( aNotify);

        if( aHint.get() )
            GetBroadcaster().Broadcast( *aHint.get() );
    }

    return 0;
}

ScDocShell* ScAccessiblePreviewCellTextData::GetDocShell(ScPreviewShell* pViewShell)
{
    ScDocShell* pDocSh = NULL;
    if (pViewShell && pViewShell->GetDocument())
        pDocSh = (ScDocShell*) pViewShell->GetDocument()->GetDocumentShell();
    return pDocSh;
}


// ============================================================================

//  ScAccessiblePreviewHeaderCellTextData: shared data between sub objects of a accessible cell text object

ScAccessiblePreviewHeaderCellTextData::ScAccessiblePreviewHeaderCellTextData(ScPreviewShell* pViewShell,
            const String& rText, const ScAddress& rP, sal_Bool bColHeader, sal_Bool bRowHeader)
    : ScAccessibleCellBaseTextData(GetDocShell(pViewShell), rP),
    mpViewShell(pViewShell),
    mpViewForwarder(NULL),
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
        ULONG nId = ((const SfxSimpleHint&)rHint).GetId();
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
            pEditEngine = new ScFieldEditEngine( pEnginePool, NULL, TRUE );
        }
#if SUPD > 600
        //  currently, GetPortions doesn't work if UpdateMode is FALSE,
        //  this will be fixed (in EditEngine) by src600
//      pEditEngine->SetUpdateMode( FALSE );
#endif
        pEditEngine->EnableUndo( FALSE );
        if (pDocShell)
            pEditEngine->SetRefDevice(pDocShell->GetVirtualDevice_100th_mm());
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

    bDataValid = TRUE;

    if (pEditEngine)
        pEditEngine->SetNotifyHdl( LINK(this, ScAccessibleCellTextData, NotifyHdl) );

    return pForwarder;
}

SvxViewForwarder* ScAccessiblePreviewHeaderCellTextData::GetViewForwarder()
{
    if (!mpViewForwarder)
        mpViewForwarder = new ScPreviewHeaderCellViewForwarder(mpViewShell, aCellPos, mbColHeader, mbRowHeader);
    return mpViewForwarder;
}

IMPL_LINK(ScAccessiblePreviewHeaderCellTextData, NotifyHdl, EENotify*, aNotify)
{
    if( aNotify )
    {
        ::std::auto_ptr< SfxHint > aHint = SvxEditSourceHelper::EENotification2Hint( aNotify);

        if( aHint.get() )
            GetBroadcaster().Broadcast( *aHint.get() );
    }

    return 0;
}

ScDocShell* ScAccessiblePreviewHeaderCellTextData::GetDocShell(ScPreviewShell* pViewShell)
{
    ScDocShell* pDocSh = NULL;
    if (pViewShell && pViewShell->GetDocument())
        pDocSh = (ScDocShell*) pViewShell->GetDocument()->GetDocumentShell();
    return pDocSh;
}


// ============================================================================

ScAccessibleHeaderTextData::ScAccessibleHeaderTextData(ScPreviewShell* pViewShell,
                            const EditTextObject* pEditObj, sal_Bool bHeader, SvxAdjust eAdjust)
    :
    mpViewShell(pViewShell),
    mpEditObj(pEditObj),
    mbHeader(bHeader),
    mpEditEngine(NULL),
    mpForwarder(NULL),
    mpDocSh(NULL),
    mpViewForwarder(NULL),
    mbDataValid(sal_False),
    meAdjust(eAdjust)
{
    if (pViewShell && pViewShell->GetDocument())
        mpDocSh = (ScDocShell*) pViewShell->GetDocument()->GetDocumentShell();
    if (mpDocSh)
        mpDocSh->GetDocument()->AddUnoObject(*this);
}

ScAccessibleHeaderTextData::~ScAccessibleHeaderTextData()
{
    ScUnoGuard aGuard;      //  needed for EditEngine dtor

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

void ScAccessibleHeaderTextData::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( SfxSimpleHint ) )
    {
        ULONG nId = ((const SfxSimpleHint&)rHint).GetId();
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
        ScHeaderEditEngine* pHdrEngine = new ScHeaderEditEngine( pEnginePool, TRUE );

        pHdrEngine->EnableUndo( FALSE );
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


// ============================================================================

ScAccessibleNoteTextData::ScAccessibleNoteTextData(ScPreviewShell* pViewShell,
                            const String& sText, const ScAddress& aCellPos, sal_Bool bMarkNote)
    :
    mpViewShell(pViewShell),
    msText(sText),
    mpEditEngine(NULL),
    mpForwarder(NULL),
    mpDocSh(NULL),
    mpViewForwarder(NULL),
    mbDataValid(sal_False),
    maCellPos(aCellPos),
    mbMarkNote(bMarkNote)
{
    if (pViewShell && pViewShell->GetDocument())
        mpDocSh = (ScDocShell*) pViewShell->GetDocument()->GetDocumentShell();
    if (mpDocSh)
        mpDocSh->GetDocument()->AddUnoObject(*this);
}

ScAccessibleNoteTextData::~ScAccessibleNoteTextData()
{
    ScUnoGuard aGuard;      //  needed for EditEngine dtor

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

void ScAccessibleNoteTextData::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( SfxSimpleHint ) )
    {
        ULONG nId = ((const SfxSimpleHint&)rHint).GetId();
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
            mpEditEngine = new ScFieldEditEngine( pEnginePool, NULL, TRUE );
        }
#if SUPD > 600
        //  currently, GetPortions doesn't work if UpdateMode is FALSE,
        //  this will be fixed (in EditEngine) by src600
//      pEditEngine->SetUpdateMode( FALSE );
#endif
        mpEditEngine->EnableUndo( FALSE );
        if (mpDocSh)
            mpEditEngine->SetRefDevice(mpDocSh->GetVirtualDevice_100th_mm());
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

    mbDataValid = TRUE;

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

    virtual BOOL                IsValid() const;
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

BOOL ScCsvViewForwarder::IsValid() const
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

// ----------------------------------------------------------------------------

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
        ULONG nId = ((const SfxSimpleHint&)rHint).GetId();
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

SvxEditViewForwarder* ScAccessibleCsvTextData::GetEditViewForwarder( sal_Bool bCreate )
{
    return NULL;
}


// ============================================================================

