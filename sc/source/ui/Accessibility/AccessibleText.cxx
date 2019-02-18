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

#include <scitems.hxx>
#include <editeng/eeitem.hxx>

#include <memory>
#include <AccessibleText.hxx>
#include <AccessibleCell.hxx>
#include <tabvwsh.hxx>
#include <editutil.hxx>
#include <document.hxx>
#include <scmod.hxx>
#include <prevwsh.hxx>
#include <docsh.hxx>
#include <prevloc.hxx>
#include <patattr.hxx>
#include <inputwin.hxx>
#include <editeng/unofored.hxx>
#include <editeng/editview.hxx>
#include <editeng/unoedhlp.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/justifyitem.hxx>
#include <svx/svdmodel.hxx>
#include <svx/algitem.hxx>
#include <vcl/svapp.hxx>

class ScViewForwarder : public SvxViewForwarder
{
    ScTabViewShell*     mpViewShell;
    ScSplitPos          meSplitPos;
public:
                        ScViewForwarder(ScTabViewShell* pViewShell, ScSplitPos eSplitPos);

    virtual bool        IsValid() const override;
    virtual Point       LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const override;
    virtual Point       PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const override;

    void                SetInvalid();
};

ScViewForwarder::ScViewForwarder(ScTabViewShell* pViewShell, ScSplitPos eSplitPos)
    :
    mpViewShell(pViewShell),
    meSplitPos(eSplitPos)
{
}

bool ScViewForwarder::IsValid() const
{
    return mpViewShell != nullptr;
}

Point ScViewForwarder::LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const
{
    if (mpViewShell)
    {
        vcl::Window* pWindow = mpViewShell->GetWindowByPos(meSplitPos);
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
        vcl::Window* pWindow = mpViewShell->GetWindowByPos(meSplitPos);
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
    mpViewShell = nullptr;
}

class ScEditObjectViewForwarder : public SvxViewForwarder
{
    VclPtr<vcl::Window> mpWindow;
    // #i49561# EditView needed for access to its visible area.
    const EditView*     mpEditView;
public:
                        ScEditObjectViewForwarder( vcl::Window* pWindow,
                                                   const EditView* _pEditView);

    virtual bool        IsValid() const override;
    virtual Point       LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const override;
    virtual Point       PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const override;

    void                SetInvalid();
};

ScEditObjectViewForwarder::ScEditObjectViewForwarder( vcl::Window* pWindow,
                                                      const EditView* _pEditView )
    :
    mpWindow(pWindow),
    mpEditView( _pEditView )
{
}

bool ScEditObjectViewForwarder::IsValid() const
{
    return (mpWindow != nullptr);
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
            tools::Rectangle aEditViewVisArea( mpEditView->GetVisArea() );
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
            tools::Rectangle aEditViewVisArea( mpEditView->GetVisArea() );
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
    mpWindow = nullptr;
}

class ScPreviewViewForwarder : public SvxViewForwarder
{
protected:
    ScPreviewShell*     mpViewShell;
public:
    explicit            ScPreviewViewForwarder(ScPreviewShell* pViewShell);

    virtual bool        IsValid() const override;
    virtual Point       LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const override;
    virtual Point       PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const override;

    void                SetInvalid();
};

ScPreviewViewForwarder::ScPreviewViewForwarder(ScPreviewShell* pViewShell)
    : mpViewShell(pViewShell)
{
}

bool ScPreviewViewForwarder::IsValid() const
{
    return mpViewShell != nullptr;
}

Point ScPreviewViewForwarder::LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const
{
    if (mpViewShell)
    {
        vcl::Window* pWindow = mpViewShell->GetWindow();
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
        vcl::Window* pWindow = mpViewShell->GetWindow();
        if (pWindow)
        {
            MapMode aMapMode(pWindow->GetMapMode());
            aMapMode.SetOrigin(Point());
            Point aPoint1( pWindow->PixelToLogic( rPoint ) );
            Point aPoint2( OutputDevice::LogicToLogic( aPoint1,
                                                       MapMode(aMapMode.GetMapUnit()),
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
    mpViewShell = nullptr;
}

class ScPreviewHeaderFooterViewForwarder : public ScPreviewViewForwarder
{
public:
                        ScPreviewHeaderFooterViewForwarder(ScPreviewShell* pViewShell);
};

ScPreviewHeaderFooterViewForwarder::ScPreviewHeaderFooterViewForwarder(ScPreviewShell* pViewShell)
    :
    ScPreviewViewForwarder(pViewShell)
{
}

class ScPreviewCellViewForwarder : public ScPreviewViewForwarder
{
public:
                        ScPreviewCellViewForwarder(ScPreviewShell* pViewShell);
};

ScPreviewCellViewForwarder::ScPreviewCellViewForwarder(ScPreviewShell* pViewShell)
    :
    ScPreviewViewForwarder(pViewShell)
{
}

class ScPreviewHeaderCellViewForwarder : public ScPreviewViewForwarder
{
public:
                        ScPreviewHeaderCellViewForwarder(ScPreviewShell* pViewShell);
};

ScPreviewHeaderCellViewForwarder::ScPreviewHeaderCellViewForwarder(ScPreviewShell* pViewShell)
    :
    ScPreviewViewForwarder(pViewShell)
{
}

class ScPreviewNoteViewForwarder : public ScPreviewViewForwarder
{
public:
                        ScPreviewNoteViewForwarder(ScPreviewShell* pViewShell);
};

ScPreviewNoteViewForwarder::ScPreviewNoteViewForwarder(ScPreviewShell* pViewShell)
    :
    ScPreviewViewForwarder(pViewShell)
{
}

class ScEditViewForwarder : public SvxEditViewForwarder
{
    EditView*           mpEditView;
    VclPtr<vcl::Window> mpWindow;
public:
                        ScEditViewForwarder(EditView* pEditView, vcl::Window* pWin);

    virtual bool        IsValid() const override;
    virtual Point       LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const override;
    virtual Point       PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const override;
    virtual bool        GetSelection( ESelection& rSelection ) const override;
    virtual bool        SetSelection( const ESelection& rSelection ) override;
    virtual bool        Copy() override;
    virtual bool        Cut() override;
    virtual bool        Paste() override;

    void                SetInvalid();
};

ScEditViewForwarder::ScEditViewForwarder(EditView* pEditView, vcl::Window* pWin)
    : mpEditView(pEditView),
    mpWindow(pWin)
{
}

bool ScEditViewForwarder::IsValid() const
{
    return mpWindow && mpEditView;
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

bool ScEditViewForwarder::GetSelection( ESelection& rSelection ) const
{
    bool bResult(false);
    if (IsValid())
    {
        rSelection = mpEditView->GetSelection();
        bResult = true;
    }
    else
    {
        OSL_FAIL("this ViewForwarder is not valid");
    }
    return bResult;
}

bool ScEditViewForwarder::SetSelection( const ESelection& rSelection )
{
    bool bResult(false);
    if (IsValid())
    {
        mpEditView->SetSelection(rSelection);
        bResult = true;
    }
    else
    {
        OSL_FAIL("this ViewForwarder is not valid");
    }
    return bResult;
}

bool ScEditViewForwarder::Copy()
{
    bool bResult(false);
    if (IsValid())
    {
        mpEditView->Copy();
        bResult = true;
    }
    else
    {
        OSL_FAIL("this ViewForwarder is not valid");
    }
    return bResult;
}

bool ScEditViewForwarder::Cut()
{
    bool bResult(false);
    if (IsValid())
    {
        mpEditView->Cut();
        bResult = true;
    }
    else
    {
        OSL_FAIL("this ViewForwarder is not valid");
    }
    return bResult;
}

bool ScEditViewForwarder::Paste()
{
    bool bResult(false);
    if (IsValid())
    {
        mpEditView->Paste();
        bResult = true;
    }
    else
    {
        OSL_FAIL("this ViewForwarder is not valid");
    }
    return bResult;
}

void ScEditViewForwarder::SetInvalid()
{
    mpWindow = nullptr;
    mpEditView = nullptr;
}

//  ScAccessibleCellTextData: shared data between sub objects of a accessible cell text object

ScAccessibleCellTextData::ScAccessibleCellTextData(ScTabViewShell* pViewShell,
        const ScAddress& rP, ScSplitPos eSplitPos, ScAccessibleCell* pAccCell)
    : ScAccessibleCellBaseTextData(GetDocShell(pViewShell), rP),
    mpViewShell(pViewShell),
    meSplitPos(eSplitPos),
    mpAccessibleCell( pAccCell )
{
}

ScAccessibleCellTextData::~ScAccessibleCellTextData()
{
    if (pEditEngine)
        pEditEngine->SetNotifyHdl(Link<EENotify&,void>());
    mpViewForwarder.reset();
}

void ScAccessibleCellTextData::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.GetId() == SfxHintId::Dying )
    {
        mpViewShell = nullptr;                     // invalid now
        if (mpViewForwarder)
            mpViewForwarder->SetInvalid();
    }
    ScAccessibleCellBaseTextData::Notify(rBC, rHint);
}

ScAccessibleTextData* ScAccessibleCellTextData::Clone() const
{
    return new ScAccessibleCellTextData( mpViewShell, aCellPos, meSplitPos, mpAccessibleCell );
}

SvxTextForwarder* ScAccessibleCellTextData::GetTextForwarder()
{
    ScCellTextData::GetTextForwarder(); // creates Forwarder and EditEngine

    if ( pDocShell && pEditEngine && mpViewShell )
    {
        ScDocument& rDoc = pDocShell->GetDocument();
        long nSizeX, nSizeY;
        mpViewShell->GetViewData().GetMergeSizePixel(
            aCellPos.Col(), aCellPos.Row(), nSizeX, nSizeY);

        Size aSize(nSizeX, nSizeY);

        // #i92143# text getRangeExtents reports incorrect 'x' values for spreadsheet cells
        long nIndent = 0;
        const SvxHorJustifyItem* pHorJustifyItem = rDoc.GetAttr( aCellPos, ATTR_HOR_JUSTIFY );
        SvxCellHorJustify eHorJust = pHorJustifyItem ? pHorJustifyItem->GetValue() : SvxCellHorJustify::Standard;
        if ( eHorJust == SvxCellHorJustify::Left )
        {
            const SfxUInt16Item* pIndentItem = rDoc.GetAttr( aCellPos, ATTR_INDENT );
            if ( pIndentItem )
            {
                nIndent = static_cast< long >( pIndentItem->GetValue() );
            }
        }

        const SvxMarginItem* pMarginItem = rDoc.GetAttr( aCellPos, ATTR_MARGIN );
        ScViewData& rViewData = mpViewShell->GetViewData();
        double nPPTX = rViewData.GetPPTX();
        double nPPTY = rViewData.GetPPTY();
        long nLeftM = ( pMarginItem ? static_cast< long >( ( pMarginItem->GetLeftMargin() + nIndent ) * nPPTX ) : 0 );
        long nTopM = ( pMarginItem ? static_cast< long >( pMarginItem->GetTopMargin() * nPPTY ) : 0 );
        long nRightM = ( pMarginItem ? static_cast< long >( pMarginItem->GetRightMargin() * nPPTX ) : 0 );
        long nBottomM = ( pMarginItem ? static_cast< long >( pMarginItem->GetBottomMargin() * nPPTY ) : 0 );
        long nWidth = aSize.getWidth() - nLeftM - nRightM;
        aSize.setWidth( nWidth );
        aSize.setHeight( aSize.getHeight() - nTopM - nBottomM );

        vcl::Window* pWin = mpViewShell->GetWindowByPos( meSplitPos );
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
        const SfxInt32Item* pItem = rDoc.GetAttr( aCellPos, ATTR_ROTATE_VALUE );
        if( pItem && (pItem->GetValue() != 0) )
        {
            pEditEngine->SetPaperSize( Size( LONG_MAX, aSize.getHeight() ) );
            long nTxtWidth = static_cast< long >( pEditEngine->CalcTextWidth() );
            aSize.setWidth( std::max( aSize.getWidth(), nTxtWidth + 2 ) );
        }
        else
        {
            // #i92143# text getRangeExtents reports incorrect 'x' values for spreadsheet cells
            const SfxBoolItem* pLineBreakItem = rDoc.GetAttr( aCellPos, ATTR_LINEBREAK );
            bool bLineBreak = ( pLineBreakItem && pLineBreakItem->GetValue() );
            if ( !bLineBreak )
            {
                long nTxtWidth = static_cast< long >( pEditEngine->CalcTextWidth() );
                aSize.setWidth( ::std::max( aSize.getWidth(), nTxtWidth ) );
            }
        }

        pEditEngine->SetPaperSize( aSize );

        // #i92143# text getRangeExtents reports incorrect 'x' values for spreadsheet cells
        if ( eHorJust == SvxCellHorJustify::Standard && rDoc.HasValueData( aCellPos.Col(), aCellPos.Row(), aCellPos.Tab() ) )
        {
            pEditEngine->SetDefaultItem( SvxAdjustItem( SvxAdjust::Right, EE_PARA_JUST ) );
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
                case SvxCellHorJustify::Right:
                    {
                        nOffsetX -= nDiffX;
                    }
                    break;
                case SvxCellHorJustify::Center:
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
        const SvxVerJustifyItem* pVerJustifyItem = rDoc.GetAttr( aCellPos, ATTR_VER_JUSTIFY );
        SvxCellVerJustify eVerJust = ( pVerJustifyItem ? pVerJustifyItem->GetValue() : SvxCellVerJustify::Standard );
        switch ( eVerJust )
        {
            case SvxCellVerJustify::Standard:
            case SvxCellVerJustify::Bottom:
                {
                    nOffsetY = nSizeY - nBottomM - nTextHeight;
                }
                break;
            case SvxCellVerJustify::Center:
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

    return pForwarder.get();
}

SvxViewForwarder* ScAccessibleCellTextData::GetViewForwarder()
{
    if (!mpViewForwarder)
        mpViewForwarder.reset(new ScViewForwarder(mpViewShell, meSplitPos));
    return mpViewForwarder.get();
}

SvxEditViewForwarder* ScAccessibleCellTextData::GetEditViewForwarder( bool /* bCreate */ )
{
    //#102219#; there should no EditViewForwarder be, because the cell is now readonly in this interface
    return nullptr;
}

IMPL_LINK(ScAccessibleTextData, NotifyHdl, EENotify&, aNotify, void)
{
    ::std::unique_ptr< SfxHint > aHint = SvxEditSourceHelper::EENotification2Hint( &aNotify );

    if (aHint)
        GetBroadcaster().Broadcast(*aHint);
}

ScDocShell* ScAccessibleCellTextData::GetDocShell(ScTabViewShell* pViewShell)
{
    ScDocShell* pDocSh = nullptr;
    if (pViewShell)
        pDocSh = pViewShell->GetViewData().GetDocShell();
    return pDocSh;
}

ScAccessibleEditObjectTextData::ScAccessibleEditObjectTextData(EditView* pEditView, vcl::Window* pWin, bool isClone)
    :
    mpEditView(pEditView),
    mpEditEngine(pEditView ? pEditView->GetEditEngine() : nullptr),
    mpWindow(pWin)
{
    // If the object is cloned, do NOT add notify hdl.
    mbIsCloned = isClone;
    if (mpEditEngine && !mbIsCloned)
        mpEditEngine->SetNotifyHdl( LINK(this, ScAccessibleEditObjectTextData, NotifyHdl) );
}

ScAccessibleEditObjectTextData::~ScAccessibleEditObjectTextData()
{
    // If the object is cloned, do NOT set notify hdl.
    if (mpEditEngine && !mbIsCloned)
        mpEditEngine->SetNotifyHdl(Link<EENotify&,void>());
    mpViewForwarder.reset();
    mpEditViewForwarder.reset();
    mpForwarder.reset();
}

void ScAccessibleEditObjectTextData::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.GetId() == SfxHintId::Dying )
    {
        mpWindow = nullptr;
        mpEditView = nullptr;
        mpEditEngine = nullptr;
        mpForwarder.reset();
        if (mpViewForwarder)
            mpViewForwarder->SetInvalid();
        if (mpEditViewForwarder)
            mpEditViewForwarder->SetInvalid();
    }
    ScAccessibleTextData::Notify(rBC, rHint);
}

ScAccessibleTextData* ScAccessibleEditObjectTextData::Clone() const
{
    // Add para to indicate the object is cloned
    return new ScAccessibleEditObjectTextData(mpEditView, mpWindow, true);
}

SvxTextForwarder* ScAccessibleEditObjectTextData::GetTextForwarder()
{
    if ((!mpForwarder && mpEditView) || (mpEditEngine && !mpEditEngine->GetNotifyHdl().IsSet()))
    {
        if (!mpEditEngine)
            mpEditEngine = mpEditView->GetEditEngine();
        // If the object is cloned, do NOT add notify hdl.
        if (mpEditEngine && !mpEditEngine->GetNotifyHdl().IsSet()&&!mbIsCloned)
            mpEditEngine->SetNotifyHdl( LINK(this, ScAccessibleEditObjectTextData, NotifyHdl) );
        if(!mpForwarder)
            mpForwarder.reset(new SvxEditEngineForwarder(*mpEditEngine));
    }
    return mpForwarder.get();
}

SvxViewForwarder* ScAccessibleEditObjectTextData::GetViewForwarder()
{
    if (!mpViewForwarder)
    {
        // i#49561 Get right-aligned cell content to be read by screenreader.
        mpViewForwarder.reset(new ScEditObjectViewForwarder( mpWindow, mpEditView ));
    }
    return mpViewForwarder.get();
}

SvxEditViewForwarder* ScAccessibleEditObjectTextData::GetEditViewForwarder( bool bCreate )
{
    if (!mpEditViewForwarder && mpEditView)
        mpEditViewForwarder.reset(new ScEditViewForwarder(mpEditView, mpWindow));
    if (bCreate)
    {
        if (!mpEditView && mpEditViewForwarder)
        {
            mpEditViewForwarder.reset();
        }
    }
    return mpEditViewForwarder.get();
}

IMPL_LINK(ScAccessibleEditObjectTextData, NotifyHdl, EENotify&, rNotify, void)
{
    ::std::unique_ptr< SfxHint > aHint = SvxEditSourceHelper::EENotification2Hint( &rNotify );

    if (aHint)
        GetBroadcaster().Broadcast(*aHint);
}

ScAccessibleEditLineTextData::ScAccessibleEditLineTextData(EditView* pEditView, vcl::Window* pWin)
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
    ScTextWnd* pTxtWnd = dynamic_cast< ScTextWnd* >(mpWindow.get());

    if (pTxtWnd)
    {
        assert(!pTxtWnd->IsDisposed());
        pTxtWnd->RemoveAccessibleTextData( *this );
    }

    if (mbEditEngineCreated && mpEditEngine)
    {
        delete mpEditEngine;
        mpEditEngine = nullptr;    // don't access in ScAccessibleEditObjectTextData dtor!
    }
    else if (pTxtWnd && pTxtWnd->HasEditView() && pTxtWnd->GetEditView()->GetEditEngine())
    {
        //  the NotifyHdl also has to be removed from the ScTextWnd's EditEngine
        //  (it's set in ScAccessibleEditLineTextData::GetTextForwarder, and mpEditEngine
        //  is reset there)
        pTxtWnd->GetEditView()->GetEditEngine()->SetNotifyHdl(Link<EENotify&,void>());
    }
}

void ScAccessibleEditLineTextData::Dispose()
{
    ScTextWnd* pTxtWnd = dynamic_cast<ScTextWnd*>(mpWindow.get());

    if (pTxtWnd)
    {
        assert(!pTxtWnd->IsDisposed());
        pTxtWnd->RemoveAccessibleTextData( *this );
    }

    ResetEditMode();
    mpWindow = nullptr;
}

ScAccessibleTextData* ScAccessibleEditLineTextData::Clone() const
{
    return new ScAccessibleEditLineTextData(mpEditView, mpWindow);
}

SvxTextForwarder* ScAccessibleEditLineTextData::GetTextForwarder()
{
    ScTextWnd* pTxtWnd = dynamic_cast<ScTextWnd*>(mpWindow.get());

    if (pTxtWnd)
    {
        if (pTxtWnd->HasEditView())
        {
            mpEditView = pTxtWnd->GetEditView();

            if (mbEditEngineCreated && mpEditEngine)
                ResetEditMode();
            mbEditEngineCreated = false;

            mpEditView = pTxtWnd->GetEditView();
            ScAccessibleEditObjectTextData::GetTextForwarder(); // fill the mpForwarder
            mpEditEngine = nullptr;
        }
        else
        {
            mpEditView = nullptr;

            if (mpEditEngine && !mbEditEngineCreated)
                ResetEditMode();
            if (!mpEditEngine)
            {
                SfxItemPool* pEnginePool = EditEngine::CreatePool();
                pEnginePool->FreezeIdRanges();
                mpEditEngine = new ScFieldEditEngine(nullptr, pEnginePool, nullptr, true);
                mbEditEngineCreated = true;
                mpEditEngine->EnableUndo( false );
                mpEditEngine->SetRefMapMode(MapMode(MapUnit::Map100thMM));
                mpForwarder.reset(new SvxEditEngineForwarder(*mpEditEngine));

                mpEditEngine->SetText(pTxtWnd->GetTextString());

                Size aSize(pTxtWnd->GetSizePixel());

                aSize = pTxtWnd->PixelToLogic(aSize, mpEditEngine->GetRefMapMode());

                mpEditEngine->SetPaperSize(aSize);

                mpEditEngine->SetNotifyHdl( LINK(this, ScAccessibleEditObjectTextData, NotifyHdl) );
            }
        }
    }
    return mpForwarder.get();
}

SvxEditViewForwarder* ScAccessibleEditLineTextData::GetEditViewForwarder( bool bCreate )
{
    ScTextWnd* pTxtWnd = dynamic_cast<ScTextWnd*>(mpWindow.get());

    if (pTxtWnd)
    {
        if (!pTxtWnd->HasEditView() && bCreate)
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
    ScTextWnd* pTxtWnd = dynamic_cast<ScTextWnd*>(mpWindow.get());

    if (mbEditEngineCreated && mpEditEngine)
        delete mpEditEngine;
    else if (pTxtWnd && pTxtWnd->HasEditView() && pTxtWnd->GetEditView()->GetEditEngine())
        pTxtWnd->GetEditView()->GetEditEngine()->SetNotifyHdl(Link<EENotify&,void>());
    mpEditEngine = nullptr;

    mpForwarder.reset();
    mpEditViewForwarder.reset();
    mpViewForwarder.reset();
    mbEditEngineCreated = false;
}

void ScAccessibleEditLineTextData::TextChanged()
{
    if (mbEditEngineCreated && mpEditEngine)
    {
        ScTextWnd* pTxtWnd = dynamic_cast<ScTextWnd*>(mpWindow.get());

        if (pTxtWnd)
            mpEditEngine->SetText(pTxtWnd->GetTextString());
    }
}

void ScAccessibleEditLineTextData::StartEdit()
{
    ResetEditMode();
    mpEditView = nullptr;

    // send SdrHintKind::BeginEdit
    SdrHint aHint(SdrHintKind::BeginEdit);
    GetBroadcaster().Broadcast( aHint );
}

void ScAccessibleEditLineTextData::EndEdit()
{
    // send SdrHintKind::EndEdit
    SdrHint aHint(SdrHintKind::EndEdit);
    GetBroadcaster().Broadcast( aHint );

    ResetEditMode();
    mpEditView = nullptr;
}

//  ScAccessiblePreviewCellTextData: shared data between sub objects of a accessible cell text object

ScAccessiblePreviewCellTextData::ScAccessiblePreviewCellTextData(ScPreviewShell* pViewShell,
                            const ScAddress& rP)
    : ScAccessibleCellBaseTextData(GetDocShell(pViewShell), rP),
    mpViewShell(pViewShell)
{
}

ScAccessiblePreviewCellTextData::~ScAccessiblePreviewCellTextData()
{
    if (pEditEngine)
        pEditEngine->SetNotifyHdl(Link<EENotify&,void>());
    mpViewForwarder.reset();
}

void ScAccessiblePreviewCellTextData::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.GetId() == SfxHintId::Dying )
    {
        mpViewShell = nullptr;                     // invalid now
        if (mpViewForwarder)
            mpViewForwarder->SetInvalid();
    }
    ScAccessibleCellBaseTextData::Notify(rBC, rHint);
}

ScAccessibleTextData* ScAccessiblePreviewCellTextData::Clone() const
{
    return new ScAccessiblePreviewCellTextData(mpViewShell, aCellPos);
}

SvxTextForwarder* ScAccessiblePreviewCellTextData::GetTextForwarder()
{
    bool bEditEngineBefore(pEditEngine != nullptr);

    ScCellTextData::GetTextForwarder(); // creates Forwarder and EditEngine

    if (!bEditEngineBefore && pEditEngine)
    {
        Size aSize(mpViewShell->GetLocationData().GetCellOutputRect(aCellPos).GetSize());
        vcl::Window* pWin = mpViewShell->GetWindow();
        if (pWin)
            aSize = pWin->PixelToLogic(aSize, pEditEngine->GetRefMapMode());
        pEditEngine->SetPaperSize(aSize);
    }

    if (pEditEngine)
        pEditEngine->SetNotifyHdl( LINK(this, ScAccessiblePreviewCellTextData, NotifyHdl) );

    return pForwarder.get();
}

SvxViewForwarder* ScAccessiblePreviewCellTextData::GetViewForwarder()
{
    if (!mpViewForwarder)
        mpViewForwarder.reset(new ScPreviewCellViewForwarder(mpViewShell));
    return mpViewForwarder.get();
}

ScDocShell* ScAccessiblePreviewCellTextData::GetDocShell(ScPreviewShell* pViewShell)
{
    ScDocShell* pDocSh = nullptr;
    if (pViewShell)
        pDocSh = static_cast<ScDocShell*>( pViewShell->GetDocument().GetDocumentShell());
    return pDocSh;
}

//  ScAccessiblePreviewHeaderCellTextData: shared data between sub objects of a accessible cell text object

ScAccessiblePreviewHeaderCellTextData::ScAccessiblePreviewHeaderCellTextData(ScPreviewShell* pViewShell,
            const OUString& rText, const ScAddress& rP, bool bColHeader, bool bRowHeader)
    : ScAccessibleCellBaseTextData(GetDocShell(pViewShell), rP),
    mpViewShell(pViewShell),
    maText(rText),
    mbColHeader(bColHeader),
    mbRowHeader(bRowHeader)
{
}

ScAccessiblePreviewHeaderCellTextData::~ScAccessiblePreviewHeaderCellTextData()
{
    if (pEditEngine)
        pEditEngine->SetNotifyHdl(Link<EENotify&,void>());
    mpViewForwarder.reset();
}

void ScAccessiblePreviewHeaderCellTextData::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.GetId() == SfxHintId::Dying )
    {
        mpViewShell = nullptr;                     // invalid now
        if (mpViewForwarder)
            mpViewForwarder->SetInvalid();
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
            ScDocument& rDoc = pDocShell->GetDocument();
            pEditEngine = rDoc.CreateFieldEditEngine();
        }
        else
        {
            SfxItemPool* pEnginePool = EditEngine::CreatePool();
            pEnginePool->FreezeIdRanges();
            pEditEngine.reset( new ScFieldEditEngine(nullptr, pEnginePool, nullptr, true) );
        }
        pEditEngine->EnableUndo( false );
        if (pDocShell)
            pEditEngine->SetRefDevice(pDocShell->GetRefDevice());
        else
            pEditEngine->SetRefMapMode(MapMode(MapUnit::Map100thMM));
        pForwarder.reset( new SvxEditEngineForwarder(*pEditEngine) );
    }

    if (bDataValid)
        return pForwarder.get();

    if (!maText.isEmpty())
    {
        if ( mpViewShell  )
        {
            Size aOutputSize;
            vcl::Window* pWindow = mpViewShell->GetWindow();
            if ( pWindow )
                aOutputSize = pWindow->GetOutputSizePixel();
            tools::Rectangle aVisRect( Point(), aOutputSize );
            Size aSize(mpViewShell->GetLocationData().GetHeaderCellOutputRect(aVisRect, aCellPos, mbColHeader).GetSize());
            if (pWindow)
                aSize = pWindow->PixelToLogic(aSize, pEditEngine->GetRefMapMode());
            pEditEngine->SetPaperSize(aSize);
        }
        pEditEngine->SetText( maText );
    }

    bDataValid = true;

    pEditEngine->SetNotifyHdl( LINK(this, ScAccessiblePreviewHeaderCellTextData, NotifyHdl) );

    return pForwarder.get();
}

SvxViewForwarder* ScAccessiblePreviewHeaderCellTextData::GetViewForwarder()
{
    if (!mpViewForwarder)
        mpViewForwarder.reset(new ScPreviewHeaderCellViewForwarder(mpViewShell));
    return mpViewForwarder.get();
}

ScDocShell* ScAccessiblePreviewHeaderCellTextData::GetDocShell(ScPreviewShell* pViewShell)
{
    ScDocShell* pDocSh = nullptr;
    if (pViewShell)
        pDocSh = static_cast<ScDocShell*>(pViewShell->GetDocument().GetDocumentShell());
    return pDocSh;
}

ScAccessibleHeaderTextData::ScAccessibleHeaderTextData(ScPreviewShell* pViewShell,
                            const EditTextObject* pEditObj, SvxAdjust eAdjust)
    :
    mpViewForwarder(nullptr),
    mpViewShell(pViewShell),
    mpDocSh(nullptr),
    mpEditObj(pEditObj),
    mbDataValid(false),
    meAdjust(eAdjust)
{
    if (pViewShell)
        mpDocSh = static_cast<ScDocShell*>(pViewShell->GetDocument().GetDocumentShell());
    if (mpDocSh)
        mpDocSh->GetDocument().AddUnoObject(*this);
}

ScAccessibleHeaderTextData::~ScAccessibleHeaderTextData()
{
    SolarMutexGuard aGuard;     //  needed for EditEngine dtor

    if (mpDocSh)
        mpDocSh->GetDocument().RemoveUnoObject(*this);
    if (mpEditEngine)
        mpEditEngine->SetNotifyHdl(Link<EENotify&,void>());
    mpEditEngine.reset();
    mpForwarder.reset();
}

ScAccessibleTextData* ScAccessibleHeaderTextData::Clone() const
{
    return new ScAccessibleHeaderTextData(mpViewShell, mpEditObj, meAdjust);
}

void ScAccessibleHeaderTextData::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.GetId() == SfxHintId::Dying )
    {
        mpViewShell = nullptr;// invalid now
        mpDocSh = nullptr;
        if (mpViewForwarder)
            mpViewForwarder->SetInvalid();
    }
}

SvxTextForwarder* ScAccessibleHeaderTextData::GetTextForwarder()
{
    if (!mpEditEngine)
    {
        SfxItemPool* pEnginePool = EditEngine::CreatePool();
        pEnginePool->FreezeIdRanges();
        std::unique_ptr<ScHeaderEditEngine> pHdrEngine(new ScHeaderEditEngine( pEnginePool ));

        pHdrEngine->EnableUndo( false );
        pHdrEngine->SetRefMapMode(MapMode(MapUnit::MapTwip));

        //  default font must be set, independently of document
        //  -> use global pool from module

        SfxItemSet aDefaults( pHdrEngine->GetEmptyItemSet() );
        const ScPatternAttr& rPattern = SC_MOD()->GetPool().GetDefaultItem(ATTR_PATTERN);
        rPattern.FillEditItemSet( &aDefaults );
        //  FillEditItemSet adjusts font height to 1/100th mm,
        //  but for header/footer twips is needed, as in the PatternAttr:
        std::unique_ptr<SfxPoolItem> pNewItem(rPattern.GetItem(ATTR_FONT_HEIGHT).CloneSetWhich(EE_CHAR_FONTHEIGHT));
        aDefaults.Put( *pNewItem );
        pNewItem = rPattern.GetItem(ATTR_CJK_FONT_HEIGHT).CloneSetWhich(EE_CHAR_FONTHEIGHT_CJK);
        aDefaults.Put( *pNewItem );
        pNewItem = rPattern.GetItem(ATTR_CTL_FONT_HEIGHT).CloneSetWhich(EE_CHAR_FONTHEIGHT_CTL);
        aDefaults.Put( *pNewItem );
        aDefaults.Put( SvxAdjustItem( meAdjust, EE_PARA_JUST ) );
        pHdrEngine->SetDefaults( aDefaults );

        ScHeaderFieldData aData;
        if (mpViewShell)
            mpViewShell->FillFieldData(aData);
        else
            ScHeaderFooterTextObj::FillDummyFieldData( aData );
        pHdrEngine->SetData( aData );

        mpEditEngine = std::move(pHdrEngine);
        mpForwarder.reset(new SvxEditEngineForwarder(*mpEditEngine));
    }

    if (mbDataValid)
        return mpForwarder.get();

    if ( mpViewShell  )
    {
        tools::Rectangle aVisRect;
        mpViewShell->GetLocationData().GetHeaderPosition(aVisRect);
        Size aSize(aVisRect.GetSize());
        vcl::Window* pWin = mpViewShell->GetWindow();
        if (pWin)
            aSize = pWin->PixelToLogic(aSize, mpEditEngine->GetRefMapMode());
        mpEditEngine->SetPaperSize(aSize);
    }
    if (mpEditObj)
        mpEditEngine->SetText(*mpEditObj);

    mbDataValid = true;
    return mpForwarder.get();
}

SvxViewForwarder* ScAccessibleHeaderTextData::GetViewForwarder()
{
    if (!mpViewForwarder)
        mpViewForwarder = new ScPreviewHeaderFooterViewForwarder(mpViewShell);
    return mpViewForwarder;
}

ScAccessibleNoteTextData::ScAccessibleNoteTextData(ScPreviewShell* pViewShell,
                            const OUString& sText, const ScAddress& aCellPos, bool bMarkNote)
    :
    mpViewForwarder(nullptr),
    mpViewShell(pViewShell),
    mpDocSh(nullptr),
    msText(sText),
    maCellPos(aCellPos),
    mbMarkNote(bMarkNote),
    mbDataValid(false)
{
    if (pViewShell)
        mpDocSh = static_cast<ScDocShell*>(pViewShell->GetDocument().GetDocumentShell());
    if (mpDocSh)
        mpDocSh->GetDocument().AddUnoObject(*this);
}

ScAccessibleNoteTextData::~ScAccessibleNoteTextData()
{
    SolarMutexGuard aGuard;     //  needed for EditEngine dtor

    if (mpDocSh)
        mpDocSh->GetDocument().RemoveUnoObject(*this);
    if (mpEditEngine)
        mpEditEngine->SetNotifyHdl(Link<EENotify&,void>());
    mpEditEngine.reset();
    mpForwarder.reset();
}

ScAccessibleTextData* ScAccessibleNoteTextData::Clone() const
{
    return new ScAccessibleNoteTextData(mpViewShell, msText, maCellPos, mbMarkNote);
}

void ScAccessibleNoteTextData::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.GetId() == SfxHintId::Dying )
    {
        mpViewShell = nullptr;// invalid now
        mpDocSh = nullptr;
        if (mpViewForwarder)
            mpViewForwarder->SetInvalid();
    }
}

SvxTextForwarder* ScAccessibleNoteTextData::GetTextForwarder()
{
    if (!mpEditEngine)
    {
        if ( mpDocSh )
        {
            ScDocument& rDoc = mpDocSh->GetDocument();
            mpEditEngine = rDoc.CreateFieldEditEngine();
        }
        else
        {
            SfxItemPool* pEnginePool = EditEngine::CreatePool();
            pEnginePool->FreezeIdRanges();
            mpEditEngine.reset( new ScFieldEditEngine(nullptr, pEnginePool, nullptr, true) );
        }
        mpEditEngine->EnableUndo( false );
        if (mpDocSh)
            mpEditEngine->SetRefDevice(mpDocSh->GetRefDevice());
        else
            mpEditEngine->SetRefMapMode(MapMode(MapUnit::Map100thMM));
        mpForwarder.reset( new SvxEditEngineForwarder(*mpEditEngine) );
    }

    if (mbDataValid)
        return mpForwarder.get();

    if (!msText.isEmpty())
    {

        if ( mpViewShell  )
        {
            Size aOutputSize;
            vcl::Window* pWindow = mpViewShell->GetWindow();
            if ( pWindow )
                aOutputSize = pWindow->GetOutputSizePixel();
            tools::Rectangle aVisRect( Point(), aOutputSize );
            Size aSize(mpViewShell->GetLocationData().GetNoteInRangeOutputRect(aVisRect, mbMarkNote, maCellPos).GetSize());
            if (pWindow)
                aSize = pWindow->PixelToLogic(aSize, mpEditEngine->GetRefMapMode());
            mpEditEngine->SetPaperSize(aSize);
        }
        mpEditEngine->SetText( msText );
    }

    mbDataValid = true;

    mpEditEngine->SetNotifyHdl( LINK(this, ScAccessibleNoteTextData, NotifyHdl) );

    return mpForwarder.get();
}

SvxViewForwarder* ScAccessibleNoteTextData::GetViewForwarder()
{
    if (!mpViewForwarder)
        mpViewForwarder = new ScPreviewNoteViewForwarder(mpViewShell);
    return mpViewForwarder;
}

// CSV import =================================================================

class ScCsvViewForwarder : public SvxViewForwarder
{
    VclPtr<vcl::Window>         mpWindow;

public:
    explicit                    ScCsvViewForwarder( vcl::Window* pWindow );

    virtual bool                IsValid() const override;
    virtual Point               LogicToPixel( const Point& rPoint, const MapMode& rMapMode ) const override;
    virtual Point               PixelToLogic( const Point& rPoint, const MapMode& rMapMode ) const override;

    void                        SetInvalid();
};

ScCsvViewForwarder::ScCsvViewForwarder( vcl::Window* pWindow ) :
    mpWindow( pWindow )
{
}

bool ScCsvViewForwarder::IsValid() const
{
    return mpWindow != nullptr;
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
    mpWindow = nullptr;
}

ScAccessibleCsvTextData::ScAccessibleCsvTextData(
        vcl::Window* pWindow, EditEngine* pEditEngine,
        const OUString& rCellText, const Size& rCellSize ) :
    mpWindow( pWindow ),
    mpEditEngine( pEditEngine ),
    maCellText( rCellText ),
    maCellSize( rCellSize )
{
}

ScAccessibleCsvTextData::~ScAccessibleCsvTextData()
{
}

void ScAccessibleCsvTextData::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.GetId() == SfxHintId::Dying )
    {
        mpWindow = nullptr;
        mpEditEngine = nullptr;
        if (mpViewForwarder.get())
            mpViewForwarder->SetInvalid();
    }
    ScAccessibleTextData::Notify( rBC, rHint );
}

ScAccessibleTextData* ScAccessibleCsvTextData::Clone() const
{
    return new ScAccessibleCsvTextData( mpWindow, mpEditEngine, maCellText, maCellSize );
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
        mpTextForwarder.reset();
    return mpTextForwarder.get();
}

SvxViewForwarder* ScAccessibleCsvTextData::GetViewForwarder()
{
    if( !mpViewForwarder.get() )
        mpViewForwarder.reset( new ScCsvViewForwarder( mpWindow ) );
    return mpViewForwarder.get();
}

SvxEditViewForwarder* ScAccessibleCsvTextData::GetEditViewForwarder( bool /* bCreate */ )
{
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
