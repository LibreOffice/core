/*************************************************************************
 *
 *  $RCSfile: AccessibleText.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: sab $ $Date: 2002-05-31 08:06:59 $
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

class ScPreviewViewForwarder : public SvxViewForwarder
{
    ScPreviewShell*     mpViewShell;
    mutable ScPreviewTableInfo* mpTableInfo;
public:
                        ScPreviewViewForwarder(ScPreviewShell* pViewShell);
    virtual             ~ScPreviewViewForwarder();

    virtual BOOL        IsValid() const;
    virtual Rectangle   GetVisArea() const;
    virtual Point       LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const;
    virtual Point       PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const;

    void FillTableInfo() const;
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
    if (mpViewShell)
    {
        FillTableInfo();

        Rectangle aRect;
        if ( mpTableInfo )
        {
            USHORT nColumns = mpTableInfo->GetCols();
            USHORT nRows = mpTableInfo->GetRows();
            if ( nColumns > 0 && nRows > 0 )
            {
                const ScPreviewColRowInfo* pColInfo = mpTableInfo->GetColInfo();
                const ScPreviewColRowInfo* pRowInfo = mpTableInfo->GetRowInfo();

                aRect = Rectangle( pColInfo[0].nPixelStart,
                                   pRowInfo[0].nPixelStart,
                                   pColInfo[nColumns-1].nPixelEnd,
                                   pRowInfo[nRows-1].nPixelEnd );
            }
        }
    }
    else
        DBG_ERROR("this ViewForwarder is not valid");
    return aVisArea;
}

Point ScPreviewViewForwarder::LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const
{
    if (mpViewShell)
    {
        Window* pWindow = mpViewShell->GetWindow();
        if (pWindow)
            return pWindow->LogicToPixel( rPoint, rMapMode );
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
            return pWindow->PixelToLogic( rPoint, rMapMode );
    }
    else
        DBG_ERROR("this ViewForwarder is not valid");
    return Point();
}

void ScPreviewViewForwarder::FillTableInfo() const
{
    if ( mpViewShell && !mpTableInfo )
    {
        Size aOutputSize;
        Window* pWindow = mpViewShell->GetWindow();
        if ( pWindow )
            aOutputSize = pWindow->GetOutputSizePixel();
        Point aPoint;
        Rectangle aVisRect( aPoint, aOutputSize );

        mpTableInfo = new ScPreviewTableInfo;
        mpViewShell->GetLocationData().GetTableInfo( aVisRect, *mpTableInfo );
    }
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
        }
    }
    ScAccessibleCellBaseTextData::Notify(rBC, rHint);
}

ScAccessibleTextData* ScAccessibleCellTextData::Clone() const
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

    if (!bHasForwarder)
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
    if (bCreate)
    {
        if (!mpEditViewForwarder)
            mpEditViewForwarder = new ScEditViewForwarder(mpViewShell, meSplitPos, aCellPos);
        else
            mpEditViewForwarder->GrabFocus();
    }
    return mpEditViewForwarder;
}

IMPL_LINK(ScAccessibleCellTextData, NotifyHdl, EENotify*, aNotify)
{
    if( aNotify )
    {
        ::std::auto_ptr< SfxHint > aHint = SvxEditSourceHintTranslator::EENotification2Hint( aNotify );

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
        mpViewForwarder = new ScPreviewViewForwarder(mpViewShell);
    return mpViewForwarder;
}

IMPL_LINK(ScAccessiblePreviewCellTextData, NotifyHdl, EENotify*, aNotify)
{
    if( aNotify )
    {
        ::std::auto_ptr< SfxHint > aHint = SvxEditSourceHintTranslator::EENotification2Hint( aNotify);

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
        mpViewForwarder = new ScPreviewViewForwarder(mpViewShell);
    return mpViewForwarder;
}

IMPL_LINK(ScAccessiblePreviewHeaderCellTextData, NotifyHdl, EENotify*, aNotify)
{
    if( aNotify )
    {
        ::std::auto_ptr< SfxHint > aHint = SvxEditSourceHintTranslator::EENotification2Hint( aNotify);

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
        mpViewForwarder = new ScPreviewViewForwarder(mpViewShell);
    return mpViewForwarder;
}

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
        mpViewForwarder = new ScPreviewViewForwarder(mpViewShell);
    return mpViewForwarder;
}
