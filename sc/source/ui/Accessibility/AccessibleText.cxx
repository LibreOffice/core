/*************************************************************************
 *
 *  $RCSfile: AccessibleText.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: thb $ $Date: 2002-03-07 15:37:28 $
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

#include "AccessibleText.hxx"
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

#ifndef _SVX_UNOFORED_HXX
#include <svx/unofored.hxx>
#endif
#ifndef _MyEDITVIEW_HXX
#include <svx/editview.hxx>
#endif
#ifndef _SVX_UNOEDHLP_HXX
#include <svx/unoedhlp.hxx>
#endif

class ScViewForwarder : public SvxViewForwarder
{
    ScTabViewShell*     mpViewShell;
    ScSplitPos          meSplitPos;
public:
                        ScViewForwarder(ScTabViewShell* pViewShell, ScSplitPos eSplitPos);
    virtual             ~ScViewForwarder();

    virtual BOOL        IsValid() const;
    virtual Rectangle   GetVisArea() const;
    virtual Point       LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const;
    virtual Point       PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const;
};

ScViewForwarder::ScViewForwarder(ScTabViewShell* pViewShell, ScSplitPos eSplitPos)
    :
    mpViewShell(pViewShell),
    meSplitPos(eSplitPos)
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
            aVisArea.SetSize(pWindow->GetSizePixel());
        if(mpViewShell->GetViewData())
            aVisArea.SetPos(Point(mpViewShell->GetViewData()->GetPixPos(meSplitPos)));
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

class ScEditViewForwarder : public SvxEditViewForwarder
{
    ScTabViewShell*     mpViewShell;
    const ScAddress&    mrCell;
    ScSplitPos          meSplitPos;
public:
                        ScEditViewForwarder(ScTabViewShell* pViewShell, ScSplitPos eSplitPos, const ScAddress& rCell);
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
};

ScEditViewForwarder::ScEditViewForwarder(ScTabViewShell* pViewShell, ScSplitPos eSplitPos, const ScAddress& rCell)
    : mpViewShell(pViewShell),
    meSplitPos(eSplitPos),
    mrCell(rCell)
{
    GrabFocus();
}

ScEditViewForwarder::~ScEditViewForwarder()
{
}

BOOL ScEditViewForwarder::IsValid() const
{
    sal_Bool bResult(sal_False);
    if (mpViewShell && mpViewShell->GetViewData() &&
        (mpViewShell->GetViewData()->GetCurPos() == mrCell) &&
        (mpViewShell->GetViewData()->HasEditView(meSplitPos)) &&
        (mpViewShell->GetViewData()->GetEditViewCol() == mrCell.Col()) &&
        (mpViewShell->GetViewData()->GetEditViewRow() == mrCell.Row()))
    {
        bResult = sal_True;
    }
    return bResult;
}

Rectangle ScEditViewForwarder::GetVisArea() const
{
    Rectangle aVisArea;
    if (IsValid())
    {
        sal_uInt16 nCol, nRow;
        EditView* pEditView;
        mpViewShell->GetViewData()->GetEditView( meSplitPos, pEditView, nCol, nRow );
        Window* pWindow = mpViewShell->GetWindowByPos(meSplitPos);
        if (pEditView && pWindow && pEditView->GetEditEngine())
        {
            MapMode aMapMode(pEditView->GetEditEngine()->GetRefMapMode());
            Point aLTPoint = pWindow->LogicToPixel( pEditView->GetVisArea().TopLeft(), aMapMode );
            Point aBRPoint = pWindow->LogicToPixel( pEditView->GetVisArea().BottomRight(), aMapMode );
            aVisArea = Rectangle(aLTPoint, aBRPoint);
        }
    }
    else
        DBG_ERROR("this EditViewForwarder is no longer valid");
    return aVisArea;
}

Point ScEditViewForwarder::LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const
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

Point ScEditViewForwarder::PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const
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

sal_Bool ScEditViewForwarder::GetSelection( ESelection& rSelection ) const
{
    sal_Bool bResult(sal_False);
    if (IsValid())
    {
        sal_uInt16 nCol, nRow;
        EditView* pEditView;
        mpViewShell->GetViewData()->GetEditView( meSplitPos, pEditView, nCol, nRow );
        if (pEditView)
        {
            rSelection = pEditView->GetSelection();
            bResult = sal_True;
        }
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
        sal_uInt16 nCol, nRow;
        EditView* pEditView;
        mpViewShell->GetViewData()->GetEditView( meSplitPos, pEditView, nCol, nRow );
        if (pEditView)
        {
            pEditView->SetSelection(rSelection);
            bResult = sal_True;
        }
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
        sal_uInt16 nCol, nRow;
        EditView* pEditView;
        mpViewShell->GetViewData()->GetEditView( meSplitPos, pEditView, nCol, nRow );
        if (pEditView)
        {
            pEditView->Copy();
            bResult = sal_True;
        }
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
        sal_uInt16 nCol, nRow;
        EditView* pEditView;
        mpViewShell->GetViewData()->GetEditView( meSplitPos, pEditView, nCol, nRow );
        if (pEditView)
        {
            pEditView->Cut();
            bResult = sal_True;
        }
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
        sal_uInt16 nCol, nRow;
        EditView* pEditView;
        mpViewShell->GetViewData()->GetEditView( meSplitPos, pEditView, nCol, nRow );
        if (pEditView)
        {
            pEditView->Paste();
            bResult = sal_True;
        }
    }
    else
        DBG_ERROR("this ViewForwarder is not valid");
    return bResult;
}

void ScEditViewForwarder::GrabFocus()
{
    if (!IsValid() && mpViewShell && mpViewShell->GetViewData() &&
        mpViewShell->GetViewData()->GetTabNo() == mrCell.Tab())
    {
        if ((mpViewShell->GetViewData()->GetActivePart() != meSplitPos) &&
            mpViewShell->GetWindowByPos(meSplitPos)->IsVisible())
        {
            mpViewShell->ActivatePart(meSplitPos);
        }
        mpViewShell->SetCursor(mrCell.Col(), mrCell.Row());
        ScModule* pScMod = SC_MOD();
        pScMod->SetInputMode( SC_INPUT_TABLE );
    }
}

//  ScAccessibleCellTextData: shared data between sub objects of a accessible cell text object

ScAccessibleCellTextData::ScAccessibleCellTextData(ScTabViewShell* pViewShell,
                            const ScAddress& rP, ScSplitPos eSplitPos)
    : ScCellTextData(GetDocShell(pViewShell), rP),
    mpViewShell(pViewShell),
    meSplitPos(eSplitPos),
    mpViewForwarder(NULL),
    mpEditViewForwarder(NULL),
    mbViewEditEngine(sal_False)
{
}

ScAccessibleCellTextData::~ScAccessibleCellTextData()
{
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
        }
    }
    ScCellTextData::Notify(rBC, rHint);
}

ScAccessibleCellTextData* ScAccessibleCellTextData::Clone() const
{
    return new ScAccessibleCellTextData(mpViewShell, aCellPos, meSplitPos);
}

SvxTextForwarder* ScAccessibleCellTextData::GetTextForwarder()
{
    sal_Bool bHasForwarder(sal_False);
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
                pForwarder = new SvxEditEngineForwarder(*(pEditView->GetEditEngine()));
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
        // don't delete, because it is the EditEngine of the EditView
        pEditEngine = NULL;
    }

    if (!bHasForwarder)
        ScCellTextData::GetTextForwarder(); // creates Forwarder and EditEngine

    if (pEditEngine)
        pEditEngine->SetNotifyHdl( LINK(this, ScAccessibleCellTextData, NotifyHdl) );

    return pForwarder;
}

SvxViewForwarder* ScAccessibleCellTextData::GetViewForwarder()
{
    if (!mpViewForwarder)
        mpViewForwarder = new ScViewForwarder(mpViewShell, meSplitPos);
    return mpViewForwarder;
}

SvxEditViewForwarder* ScAccessibleCellTextData::GetEditViewForwarder( sal_Bool bCreate )
{
    if (bCreate)
    {
        if (!mpEditViewForwarder)
            mpEditViewForwarder = new ScEditViewForwarder(mpViewShell, meSplitPos, aCellPos);
        else
            mpEditViewForwarder->GrabFocus();
    }
    return mpEditViewForwarder;
}

SfxBroadcaster& ScAccessibleCellTextData::GetBroadcaster() const
{
    return maBroadcaster;
}

IMPL_LINK(ScAccessibleCellTextData, NotifyHdl, EENotify*, aNotify)
{
    if( aNotify )
        maBroadcaster.Broadcast( SvxEditSourceHintTranslator::EENotification2Hint( aNotify) );

    return 0;
}

ScDocShell* ScAccessibleCellTextData::GetDocShell(ScTabViewShell* pViewShell)
{
    ScDocShell* pDocSh = NULL;
    if (pViewShell && pViewShell->GetViewData() && pViewShell->GetViewData()->GetDocument())
        pDocSh = (ScDocShell*) pViewShell->GetViewData()->GetDocument()->GetDocumentShell();
    return pDocSh;
}

