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

#include <sal/config.h>

#include <tools/helpers.hxx>
#include <svl/eitem.hxx>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/module.hxx>
#include <unotools/localedatawrapper.hxx>

#include <svx/svxids.hrc>
#include <svx/contdlg.hxx>
#include "contimp.hxx"
#include "contwnd.hxx"
#include <svx/svdopath.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/virdev.hxx>
#include "dlgunit.hxx"
#include <vcl/weld.hxx>

SFX_IMPL_MODELESSDIALOGCONTOLLER_WITHID(SvxContourDlgChildWindow, SID_CONTOUR_DLG);

SvxContourDlgItem::SvxContourDlgItem( SvxSuperContourDlg& rContourDlg, SfxBindings& rBindings ) :
            SfxControllerItem   ( SID_CONTOUR_EXEC, rBindings ),
            rDlg                ( rContourDlg )
{
}

void SvxContourDlgItem::StateChangedAtToolBoxControl( sal_uInt16 nSID, SfxItemState /*eState*/, const SfxPoolItem* pItem )
{
    if ( pItem && ( SID_CONTOUR_EXEC == nSID ) )
    {
        const SfxBoolItem* pStateItem = dynamic_cast<const SfxBoolItem*>( pItem  );
        assert(pStateItem); //SfxBoolItem expected
        if (pStateItem)
            rDlg.SetExecState(!pStateItem->GetValue());
    }
}

SvxContourDlgChildWindow::SvxContourDlgChildWindow(vcl::Window* _pParent, sal_uInt16 nId,
                                                   SfxBindings* pBindings, SfxChildWinInfo const * pInfo)
    : SfxChildWindow( _pParent, nId )
{
    SetController(std::make_shared<SvxContourDlg>(pBindings, this, _pParent->GetFrameWeld()));
    SvxContourDlg* pDlg = static_cast<SvxContourDlg*>(GetController().get());
    pDlg->Initialize( pInfo );
}

SvxContourDlg::SvxContourDlg(SfxBindings* _pBindings, SfxChildWindow* pCW,
                             weld::Window* _pParent)
    : SfxModelessDialogController(_pBindings, pCW, _pParent, "svx/ui/floatingcontour.ui", "FloatingContour")
    , m_xImpl(std::make_unique<SvxSuperContourDlg>(*m_xBuilder, *m_xDialog, _pBindings))
{
}

SvxContourDlg::~SvxContourDlg()
{
}

tools::PolyPolygon SvxContourDlg::CreateAutoContour( const Graphic& rGraphic,
                                                     const tools::Rectangle* pRect )
{
    Bitmap  aBmp;
    bool bContourEdgeDetect = false;

    if ( rGraphic.GetType() == GraphicType::Bitmap )
    {
        if( rGraphic.IsAnimated() )
        {
            ScopedVclPtrInstance< VirtualDevice > pVDev;
            MapMode             aTransMap;
            const Animation     aAnim( rGraphic.GetAnimation() );
            const Size&         rSizePix = aAnim.GetDisplaySizePixel();
            const sal_uInt16        nCount = aAnim.Count();

            if ( pVDev->SetOutputSizePixel( rSizePix ) )
            {
                pVDev->SetLineColor( COL_BLACK );
                pVDev->SetFillColor( COL_BLACK );

                for( sal_uInt16 i = 0; i < nCount; i++ )
                {
                    const AnimationFrame& rStepBmp = aAnim.Get( i );

                    // Push Polygon output to the right place; this is the
                    // offset of the sub-image within the total animation
                    aTransMap.SetOrigin( Point( rStepBmp.maPositionPixel.X(), rStepBmp.maPositionPixel.Y() ) );
                    pVDev->SetMapMode( aTransMap );
                    pVDev->DrawPolyPolygon( CreateAutoContour( rStepBmp.maBitmapEx, pRect ) );
                }

                aTransMap.SetOrigin( Point() );
                pVDev->SetMapMode( aTransMap );
                aBmp = pVDev->GetBitmap( Point(), rSizePix );
                aBmp.Convert( BmpConversion::N1BitThreshold );
            }
        }
        else if( rGraphic.IsTransparent() )
            aBmp = rGraphic.GetBitmapEx().GetAlphaMask().GetBitmap();
        else
        {
            aBmp = rGraphic.GetBitmapEx().GetBitmap();
            bContourEdgeDetect = true;
        }
    }
    else if( rGraphic.GetType() != GraphicType::NONE )
    {
        const Graphic   aTmpGrf( rGraphic.GetGDIMetaFile().GetMonochromeMtf( COL_BLACK ) );
        ScopedVclPtrInstance< VirtualDevice > pVDev;
        Size            aSizePix( pVDev->LogicToPixel( aTmpGrf.GetPrefSize(), aTmpGrf.GetPrefMapMode() ) );

        if( aSizePix.Width() && aSizePix.Height() && ( aSizePix.Width() > 512 || aSizePix.Height() > 512 ) )
        {
            double fWH = static_cast<double>(aSizePix.Width()) / aSizePix.Height();

            if( fWH <= 1.0 )
            {
                aSizePix.setHeight(512);
                aSizePix.setWidth(basegfx::fround<tools::Long>(aSizePix.Height() * fWH));
            }
            else
            {
                aSizePix.setWidth(512);
                aSizePix.setHeight(basegfx::fround<tools::Long>(aSizePix.Width() / fWH));
            }
        }

        if( pVDev->SetOutputSizePixel( aSizePix ) )
        {
            const Point aPt;
            aTmpGrf.Draw(*pVDev, aPt, aSizePix);
            aBmp = pVDev->GetBitmap( aPt, aSizePix );
        }

        bContourEdgeDetect = true;
    }

    aBmp.SetPrefSize( rGraphic.GetPrefSize() );
    aBmp.SetPrefMapMode( rGraphic.GetPrefMapMode() );

    return tools::PolyPolygon( BitmapEx(aBmp).GetContour( bContourEdgeDetect, pRect ) );
}

// Loop through to super class, no virtual Methods to not become incompatible
// due to IF changes

const Graphic& SvxContourDlg::GetGraphic() const
{
    return m_xImpl->GetGraphic();
}

bool SvxContourDlg::IsGraphicChanged() const
{
    return m_xImpl->IsGraphicChanged();
}

tools::PolyPolygon SvxContourDlg::GetPolyPolygon()
{
    return m_xImpl->GetPolyPolygon();
}

const void* SvxContourDlg::GetEditingObject() const
{
    return m_xImpl->GetEditingObject();
}

void SvxContourDlg::Update( const Graphic& rGraphic, bool bGraphicLinked,
                            const tools::PolyPolygon* pPolyPoly, void* pEditingObj )
{
    m_xImpl->UpdateGraphic( rGraphic, bGraphicLinked, pPolyPoly, pEditingObj );
}

SvxSuperContourDlg::SvxSuperContourDlg(weld::Builder& rBuilder,
    weld::Dialog& rDialog, SfxBindings* pBindings)
    : aUpdateIdle( "SvxSuperContourDlg UpdateIdle" )
    , aCreateIdle( "SvxSuperContourDlg CreateIdle" )
    , mpBindings(pBindings)
    , pUpdateEditingObject( nullptr )
    , pCheckObj( nullptr )
    , aContourItem( *this, *pBindings )
    , mnGrfChanged( 0 )
    , bExecState( false )
    , bUpdateGraphicLinked( false )
    , bGraphicLinked( false )
    , m_rDialog(rDialog)
    , m_xContourWnd(new ContourWindow(&rDialog))
    , m_xStbStatusColor(new StatusColor(*m_xContourWnd))
    , m_xTbx1(rBuilder.weld_toolbar("toolbar"))
    , m_xMtfTolerance(rBuilder.weld_metric_spin_button("spinbutton", FieldUnit::PERCENT))
    , m_xStbStatus2(rBuilder.weld_label("statuspos"))
    , m_xStbStatus3(rBuilder.weld_label("statussize"))
    , m_xCancelBtn(rBuilder.weld_button("cancel"))
    , m_xStbStatusColorWeld(new weld::CustomWeld(rBuilder, "statuscolor", *m_xStbStatusColor))
    , m_xContourWndWeld(new weld::CustomWeld(rBuilder, "container", *m_xContourWnd))
{
    m_xCancelBtn->connect_clicked(LINK(this, SvxSuperContourDlg, CancelHdl));

    m_xContourWnd->SetMousePosLink( LINK( this, SvxSuperContourDlg, MousePosHdl ) );
    m_xContourWnd->SetGraphSizeLink( LINK( this, SvxSuperContourDlg, GraphSizeHdl ) );
    m_xContourWnd->SetUpdateLink( LINK( this, SvxSuperContourDlg, StateHdl ) );
    m_xContourWnd->SetPipetteHdl( LINK( this, SvxSuperContourDlg, PipetteHdl ) );
    m_xContourWnd->SetPipetteClickHdl( LINK( this, SvxSuperContourDlg, PipetteClickHdl ) );
    m_xContourWnd->SetWorkplaceClickHdl( LINK( this, SvxSuperContourDlg, WorkplaceClickHdl ) );

    m_xTbx1->connect_clicked( LINK( this, SvxSuperContourDlg, Tbx1ClickHdl ) );

    m_xMtfTolerance->set_value(10, FieldUnit::PERCENT);

    aUpdateIdle.SetInvokeHandler( LINK( this, SvxSuperContourDlg, UpdateHdl ) );

    aCreateIdle.SetPriority( TaskPriority::RESIZE );
    aCreateIdle.SetInvokeHandler( LINK( this, SvxSuperContourDlg, CreateHdl ) );
}

SvxSuperContourDlg::~SvxSuperContourDlg()
{
    m_xContourWnd->SetUpdateLink( Link<GraphCtrl*,void>() );
    m_xContourWnd.reset();
}

IMPL_LINK_NOARG(SvxSuperContourDlg, CancelHdl, weld::Button&, void)
{
    bool bRet = true;

    if (m_xTbx1->get_item_sensitive("TBI_APPLY"))
    {
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(&m_rDialog, "svx/ui/querysavecontchangesdialog.ui"));
        std::unique_ptr<weld::MessageDialog> xQBox(xBuilder->weld_message_dialog("QuerySaveContourChangesDialog"));
        const short nRet = xQBox->run();

        if ( nRet == RET_YES )
        {
            SfxBoolItem aBoolItem( SID_CONTOUR_EXEC, true );
            GetBindings().GetDispatcher()->ExecuteList(
                SID_CONTOUR_EXEC, SfxCallMode::SYNCHRON | SfxCallMode::RECORD,
                { &aBoolItem });
        }
        else if ( nRet == RET_CANCEL )
            bRet = false;
    }

    if (bRet)
        m_rDialog.response(RET_CANCEL);
}

// Enabled or disabled all Controls

void SvxSuperContourDlg::SetExecState( bool bEnable )
{
    bExecState = bEnable;
}

void SvxSuperContourDlg::SetGraphic( const Graphic& rGraphic )
{
    aUndoGraphic = aRedoGraphic = Graphic();
    aGraphic = rGraphic;
    mnGrfChanged = 0;
    m_xContourWnd->SetGraphic( aGraphic );
}

void SvxSuperContourDlg::SetPolyPolygon( const tools::PolyPolygon& rPolyPoly )
{
    DBG_ASSERT(  m_xContourWnd->GetGraphic().GetType() != GraphicType::NONE, "Graphic must've been set first!" );

    tools::PolyPolygon aPolyPoly( rPolyPoly );
    const MapMode   aMap100( MapUnit::Map100thMM );
    const MapMode   aGrfMap( aGraphic.GetPrefMapMode() );
    OutputDevice*   pOutDev = Application::GetDefaultDevice();
    bool            bPixelMap = aGrfMap.GetMapUnit() == MapUnit::MapPixel;

    for ( sal_uInt16 j = 0, nPolyCount = aPolyPoly.Count(); j < nPolyCount; j++ )
    {
        tools::Polygon& rPoly = aPolyPoly[ j ];

        for ( sal_uInt16 i = 0, nCount = rPoly.GetSize(); i < nCount; i++ )
        {
            Point& rPt = rPoly[ i ];

            if ( !bPixelMap )
                rPt = pOutDev->LogicToPixel( rPt, aGrfMap );

            rPt = pOutDev->PixelToLogic( rPt, aMap100 );
        }
    }

    m_xContourWnd->SetPolyPolygon( aPolyPoly );
    m_xContourWnd->GetSdrModel()->SetChanged();
}

tools::PolyPolygon SvxSuperContourDlg::GetPolyPolygon()
{
    tools::PolyPolygon aRetPolyPoly( m_xContourWnd->GetPolyPolygon() );

    const MapMode   aMap100( MapUnit::Map100thMM );
    const MapMode   aGrfMap( aGraphic.GetPrefMapMode() );
    OutputDevice*   pOutDev = Application::GetDefaultDevice();
    bool            bPixelMap = aGrfMap.GetMapUnit() == MapUnit::MapPixel;

    for ( sal_uInt16 j = 0, nPolyCount = aRetPolyPoly.Count(); j < nPolyCount; j++ )
    {
        tools::Polygon& rPoly = aRetPolyPoly[ j ];

        for ( sal_uInt16 i = 0, nCount = rPoly.GetSize(); i < nCount; i++ )
        {
            Point& rPt = rPoly[ i ];

            rPt = pOutDev->LogicToPixel( rPt, aMap100  );

            if ( !bPixelMap )
                rPt = pOutDev->PixelToLogic( rPt, aGrfMap  );
        }
    }

    return aRetPolyPoly;
}

void SvxSuperContourDlg::UpdateGraphic( const Graphic& rGraphic, bool _bGraphicLinked,
                                 const tools::PolyPolygon* pPolyPoly, void* pEditingObj )
{
    aUpdateGraphic = rGraphic;
    bUpdateGraphicLinked = _bGraphicLinked;
    pUpdateEditingObject = pEditingObj;

    if ( pPolyPoly )
        aUpdatePolyPoly = *pPolyPoly;
    else
        aUpdatePolyPoly = tools::PolyPolygon();

    aUpdateIdle.Start();
}

// Click handler for ToolBox

IMPL_LINK(SvxSuperContourDlg, Tbx1ClickHdl, const OUString&, rId, void)
{
    if (rId == "TBI_APPLY")
    {
        SfxBoolItem aBoolItem( SID_CONTOUR_EXEC, true );
        GetBindings().GetDispatcher()->ExecuteList(
            SID_CONTOUR_EXEC, SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
            { &aBoolItem });
    }
    else if (rId == "TBI_WORKPLACE")
    {
        if (m_xTbx1->get_item_active("TBI_WORKPLACE"))
        {
            std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(&m_rDialog, "svx/ui/querydeletecontourdialog.ui"));
            std::unique_ptr<weld::MessageDialog> xQBox(xBuilder->weld_message_dialog("QueryDeleteContourDialog"));

            if (!m_xContourWnd->IsContourChanged() || (xQBox->run() == RET_YES))
                m_xContourWnd->SetWorkplaceMode( true );
            else
                m_xTbx1->set_item_active("TBI_WORKPLACE", false);
        }
        else
            m_xContourWnd->SetWorkplaceMode( false );
    }
    else if (rId == "TBI_SELECT")
    {
        SetActiveTool(rId);
        m_xContourWnd->SetEditMode( true );
    }
    else if (rId == "TBI_RECT")
    {
        SetActiveTool(rId);
        m_xContourWnd->SetObjKind( SdrObjKind::Rectangle );
    }
    else if (rId == "TBI_CIRCLE")
    {
        SetActiveTool(rId);
        m_xContourWnd->SetObjKind( SdrObjKind::CircleOrEllipse );
    }
    else if (rId == "TBI_POLY")
    {
        SetActiveTool(rId);
        m_xContourWnd->SetObjKind( SdrObjKind::Polygon );
    }
    else if (rId == "TBI_POLYEDIT")
    {
        m_xContourWnd->SetPolyEditMode(m_xTbx1->get_item_active("TBI_POLYEDIT") ? SID_BEZIER_MOVE : 0);
    }
    else if (rId == "TBI_POLYMOVE")
    {
        SetActivePoly(rId);
        m_xContourWnd->SetPolyEditMode( SID_BEZIER_MOVE );
    }
    else if (rId == "TBI_POLYINSERT")
    {
        SetActivePoly(rId);
        m_xContourWnd->SetPolyEditMode( SID_BEZIER_INSERT );
    }
    else if (rId == "TBI_POLYDELETE")
    {
        m_xContourWnd->GetSdrView()->DeleteMarkedPoints();
    }
    else if (rId == "TBI_UNDO")
    {
        mnGrfChanged = mnGrfChanged ? mnGrfChanged - 1 : 0;
        aRedoGraphic = aGraphic;
        aGraphic = aUndoGraphic;
        aUndoGraphic = Graphic();
        m_xContourWnd->SetGraphic( aGraphic, false );
    }
    else if (rId == "TBI_REDO")
    {
        mnGrfChanged++;
        aUndoGraphic = aGraphic;
        aGraphic = aRedoGraphic;
        aRedoGraphic = Graphic();
        m_xContourWnd->SetGraphic( aGraphic, false );
    }
    else if (rId == "TBI_AUTOCONTOUR")
    {
        aCreateIdle.Start();
    }
    else if (rId == "TBI_PIPETTE")
    {
        bool bPipette = m_xTbx1->get_item_active("TBI_PIPETTE");

        if ( !bPipette )
            m_xStbStatusColor->Invalidate();
        else if ( bGraphicLinked )
        {
            std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(&m_rDialog, "svx/ui/queryunlinkgraphicsdialog.ui"));
            std::unique_ptr<weld::MessageDialog> xQBox(xBuilder->weld_message_dialog("QueryUnlinkGraphicsDialog"));

            if (xQBox->run() != RET_YES)
            {
                bPipette = false;
                m_xTbx1->set_item_active("TBI_PIPETTE", bPipette);
                m_xStbStatusColor->Invalidate();
            }
        }

        m_xContourWnd->SetPipetteMode( bPipette );
    }
    m_xContourWnd->QueueIdleUpdate();
}

void SvxSuperContourDlg::SetActiveTool(std::u16string_view rId)
{
    m_xTbx1->set_item_active("TBI_SELECT", rId == u"TBI_SELECT");
    m_xTbx1->set_item_active("TBI_RECT", rId == u"TBI_RECT");
    m_xTbx1->set_item_active("TBI_CIRCLE", rId == u"TBI_CIRCLE");
    m_xTbx1->set_item_active("TBI_POLY", rId == u"TBI_POLY");
}

void SvxSuperContourDlg::SetActivePoly(std::u16string_view rId)
{
    m_xTbx1->set_item_active("TBI_POLYMOVE", rId == u"TBI_POLYMOVE");
    m_xTbx1->set_item_active("TBI_POLYINSERT", rId == u"TBI_POLYINSERT");
}

IMPL_LINK( SvxSuperContourDlg, MousePosHdl, GraphCtrl*, pWnd, void )
{
    OUString aStr;
    const FieldUnit eFieldUnit = GetBindings().GetDispatcher()->GetModule()->GetFieldUnit();
    const Point& rMousePos = pWnd->GetMousePos();
    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
    const sal_Unicode cSep = rLocaleWrapper.getNumDecimalSep()[0];

    aStr = GetUnitString( rMousePos.X(), eFieldUnit, cSep )
         + " / "
         + GetUnitString( rMousePos.Y(), eFieldUnit, cSep );

    m_xStbStatus2->set_label( aStr );
}

IMPL_LINK( SvxSuperContourDlg, GraphSizeHdl, GraphCtrl*, pWnd, void )
{
    OUString aStr;
    const FieldUnit eFieldUnit = GetBindings().GetDispatcher()->GetModule()->GetFieldUnit();
    const Size& rSize = pWnd->GetGraphicSize();
    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
    const sal_Unicode cSep = rLocaleWrapper.getNumDecimalSep()[0];

    aStr = GetUnitString( rSize.Width(), eFieldUnit, cSep )
         + " x "
         + GetUnitString( rSize.Height(), eFieldUnit, cSep );

    m_xStbStatus3->set_label( aStr );
}

IMPL_LINK_NOARG(SvxSuperContourDlg, UpdateHdl, Timer *, void)
{
    aUpdateIdle.Stop();

    if ( pUpdateEditingObject != pCheckObj )
    {
        if( !GetEditingObject() )
            m_xContourWnd->GrabFocus();

        SetGraphic( aUpdateGraphic );
        SetPolyPolygon( aUpdatePolyPoly );
        pCheckObj = pUpdateEditingObject;
        bGraphicLinked = bUpdateGraphicLinked;

        aUpdateGraphic = Graphic();
        aUpdatePolyPoly = tools::PolyPolygon();
        bUpdateGraphicLinked = false;

        m_xContourWnd->GetSdrModel()->SetChanged( false );
    }

    GetBindings().Invalidate( SID_CONTOUR_EXEC );
    m_xContourWnd->QueueIdleUpdate();
}

IMPL_LINK_NOARG(SvxSuperContourDlg, CreateHdl, Timer *, void)
{
    aCreateIdle.Stop();

    const tools::Rectangle aWorkRect = m_xContourWnd->GetDrawingArea()->get_ref_device().LogicToPixel(
        m_xContourWnd->GetWorkRect(), MapMode( MapUnit::Map100thMM));

    const Graphic&  rGraphic = m_xContourWnd->GetGraphic();
    const bool      bValid = aWorkRect.Left() != aWorkRect.Right() && aWorkRect.Top() != aWorkRect.Bottom();

    weld::WaitObject aWaitObj(&m_rDialog);
    SetPolyPolygon( SvxContourDlg::CreateAutoContour( rGraphic, bValid ? &aWorkRect : nullptr ) );
}

IMPL_LINK( SvxSuperContourDlg, StateHdl, GraphCtrl*, pWnd, void )
{
    const SdrObject*    pObj = pWnd->GetSelectedSdrObject();
    const SdrView*      pView = pWnd->GetSdrView();
    const bool          bPolyEdit = ( pObj != nullptr ) && dynamic_cast<const SdrPathObj*>( pObj) !=  nullptr;
    const bool          bDrawEnabled = !(bPolyEdit && m_xTbx1->get_item_active("TBI_POLYEDIT"));
    const bool          bPipette = m_xTbx1->get_item_active("TBI_PIPETTE");
    const bool          bWorkplace = m_xTbx1->get_item_active("TBI_WORKPLACE");
    const bool          bDontHide = !( bPipette || bWorkplace );
    const bool          bBitmap = pWnd->GetGraphic().GetType() == GraphicType::Bitmap;

    m_xTbx1->set_item_sensitive("TBI_APPLY", bDontHide && bExecState && pWnd->IsChanged());

    m_xTbx1->set_item_sensitive("TBI_WORKPLACE", !bPipette && bDrawEnabled);

    m_xTbx1->set_item_sensitive("TBI_SELECT", bDontHide && bDrawEnabled);
    m_xTbx1->set_item_sensitive("TBI_RECT", bDontHide && bDrawEnabled);
    m_xTbx1->set_item_sensitive("TBI_CIRCLE", bDontHide && bDrawEnabled);
    m_xTbx1->set_item_sensitive("TBI_POLY", bDontHide && bDrawEnabled);

    m_xTbx1->set_item_sensitive("TBI_POLYEDIT", bDontHide && bPolyEdit);
    m_xTbx1->set_item_sensitive("TBI_POLYMOVE", bDontHide && !bDrawEnabled);
    m_xTbx1->set_item_sensitive("TBI_POLYINSERT", bDontHide && !bDrawEnabled);
    m_xTbx1->set_item_sensitive("TBI_POLYDELETE", bDontHide && !bDrawEnabled && pView->IsDeleteMarkedPointsPossible());

    m_xTbx1->set_item_sensitive("TBI_AUTOCONTOUR", bDontHide && bDrawEnabled);
    m_xTbx1->set_item_sensitive("TBI_PIPETTE", !bWorkplace && bDrawEnabled && bBitmap);

    m_xTbx1->set_item_sensitive("TBI_UNDO", bDontHide && aUndoGraphic.GetType() != GraphicType::NONE);
    m_xTbx1->set_item_sensitive("TBI_REDO", bDontHide && aRedoGraphic.GetType() != GraphicType::NONE);

    if ( bPolyEdit )
    {
        switch( pWnd->GetPolyEditMode() )
        {
            case SID_BEZIER_MOVE:
                SetActivePoly(u"TBI_POLYMOVE");
                break;
            case SID_BEZIER_INSERT:
                SetActivePoly(u"TBI_POLYINSERT");
                break;
            default:
                break;
        }
    }
    else
    {
        m_xTbx1->set_item_active("TBI_POLYEDIT", false);
        SetActivePoly(u"TBI_POLYMOVE");
        pWnd->SetPolyEditMode( 0 );
    }
}

IMPL_LINK_NOARG(SvxSuperContourDlg, PipetteHdl, ContourWindow&, void)
{
    m_xStbStatusColor->Invalidate();
}

void StatusColor::Paint(vcl::RenderContext& rDevice, const tools::Rectangle&)
{
    const Color& rOldLineColor = rDevice.GetLineColor();
    const Color& rOldFillColor = rDevice.GetFillColor();

    tools::Rectangle aRect(Point(), GetOutputSizePixel());
    const Color& rColor = m_rWnd.GetPipetteColor();

    rDevice.SetLineColor(rColor);
    rDevice.SetFillColor(rColor);

    aRect.AdjustLeft(4 );
    aRect.AdjustTop(4 );
    aRect.AdjustRight( -4 );
    aRect.AdjustBottom( -4 );

    rDevice.DrawRect( aRect );

    rDevice.SetLineColor(rOldLineColor);
    rDevice.SetFillColor(rOldFillColor);
}

IMPL_LINK( SvxSuperContourDlg, PipetteClickHdl, ContourWindow&, rWnd, void )
{
    if ( rWnd.IsClickValid() )
    {
        const Color&    rColor = rWnd.GetPipetteColor();

        weld::WaitObject aWaitObj(&m_rDialog);

        if( aGraphic.GetType() == GraphicType::Bitmap )
        {
            const tools::Long  nTol = static_cast<tools::Long>(m_xMtfTolerance->get_value(FieldUnit::PERCENT) * 255 / 100);

            AlphaMask aMask = aGraphic.GetBitmapEx().GetBitmap().CreateAlphaMask( rColor, nTol );

            if( aGraphic.IsTransparent() )
                aMask.AlphaCombineOr( aGraphic.GetBitmapEx().GetAlphaMask() );

            if( !aMask.IsEmpty() )
            {
                std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(&m_rDialog, "svx/ui/querynewcontourdialog.ui"));
                std::unique_ptr<weld::MessageDialog> xQBox(xBuilder->weld_message_dialog("QueryNewContourDialog"));

                bool        bNewContour;

                aRedoGraphic = Graphic();
                aUndoGraphic = aGraphic;
                Bitmap aBmp = aGraphic.GetBitmapEx().GetBitmap();
                aGraphic = Graphic( BitmapEx( aBmp, aMask ) );
                mnGrfChanged++;

                bNewContour = (xQBox->run() == RET_YES);
                rWnd.SetGraphic( aGraphic, bNewContour );

                if( bNewContour )
                    aCreateIdle.Start();
            }
        }
    }

    m_xTbx1->set_item_active("TBI_PIPETTE", false);
    rWnd.SetPipetteMode( false );
    m_xStbStatusColor->Invalidate();
}

IMPL_LINK( SvxSuperContourDlg, WorkplaceClickHdl, ContourWindow&, rWnd, void )
{
    m_xTbx1->set_item_active("TBI_WORKPLACE", false);
    m_xTbx1->set_item_active("TBI_SELECT", true);
    rWnd.SetWorkplaceMode( false );

    m_xContourWnd->QueueIdleUpdate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
