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

#include <sfx2/app.hxx>
#include <svx/EnhancedCustomShape2d.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdview.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdoashp.hxx>
#include <svx/sderitm.hxx>
#include <svx/dialogs.hrc>
#include <svx/transfrmhelper.hxx>
#include <editeng/sizeitem.hxx>

#include <transfrm.hxx>
#include <svx/dlgutil.hxx>
#include <editeng/svxenum.hxx>
#include <svx/anchorid.hxx>
#include <sfx2/module.hxx>
#include <svl/rectitem.hxx>
#include <svl/aeitem.hxx>
#include <swpossizetabpage.hxx>

// static ----------------------------------------------------------------

const sal_uInt16 SvxPositionSizeTabPage::pPosSizeRanges[] =
{
    SID_ATTR_TRANSFORM_POS_X,
    SID_ATTR_TRANSFORM_POS_Y,
    SID_ATTR_TRANSFORM_PROTECT_POS,
    SID_ATTR_TRANSFORM_PROTECT_POS,
    SID_ATTR_TRANSFORM_INTERN,
    SID_ATTR_TRANSFORM_INTERN,
    SID_ATTR_TRANSFORM_ANCHOR,
    SID_ATTR_TRANSFORM_VERT_ORIENT,
    SID_ATTR_TRANSFORM_WIDTH,
    SID_ATTR_TRANSFORM_SIZE_POINT,
    SID_ATTR_TRANSFORM_PROTECT_POS,
    SID_ATTR_TRANSFORM_INTERN,
    SID_ATTR_TRANSFORM_AUTOWIDTH,
    SID_ATTR_TRANSFORM_AUTOHEIGHT,
    0
};

const sal_uInt16 SvxAngleTabPage::pAngleRanges[] =
{
    SID_ATTR_TRANSFORM_ROT_X,
    SID_ATTR_TRANSFORM_ANGLE,
    SID_ATTR_TRANSFORM_INTERN,
    SID_ATTR_TRANSFORM_INTERN,
    0
};

const sal_uInt16 SvxSlantTabPage::pSlantRanges[] =
{
    SDRATTR_ECKENRADIUS,
    SDRATTR_ECKENRADIUS,
    SID_ATTR_TRANSFORM_SHEAR,
    SID_ATTR_TRANSFORM_SHEAR_VERTICAL,
    SID_ATTR_TRANSFORM_INTERN,
    SID_ATTR_TRANSFORM_INTERN,
    0
};

/*************************************************************************
|*
|* constructor of the tab dialog: adds the pages to the dialog
|*
\************************************************************************/

SvxTransformTabDialog::SvxTransformTabDialog(weld::Window* pParent, const SfxItemSet* pAttr,
                                             const SdrView* pSdrView, SvxAnchorIds nAnchorTypes)
    : SfxTabDialogController(pParent, "cui/ui/positionsizedialog.ui", "PositionAndSizeDialog", pAttr)
    , pView(pSdrView)
    , nAnchorCtrls(nAnchorTypes)
{
    DBG_ASSERT(pView, "no valid view (!)");

    //different positioning page in Writer
    if(nAnchorCtrls & (SvxAnchorIds::Paragraph | SvxAnchorIds::Character | SvxAnchorIds::Page | SvxAnchorIds::Fly))
    {
        AddTabPage("RID_SVXPAGE_SWPOSSIZE", SvxSwPosSizeTabPage::Create, SvxSwPosSizeTabPage::GetRanges);
        RemoveTabPage("RID_SVXPAGE_POSITION_SIZE");
    }
    else
    {
        AddTabPage("RID_SVXPAGE_POSITION_SIZE", SvxPositionSizeTabPage::Create, SvxPositionSizeTabPage::GetRanges);
        RemoveTabPage("RID_SVXPAGE_SWPOSSIZE");
    }

    AddTabPage("RID_SVXPAGE_ANGLE", SvxAngleTabPage::Create, SvxAngleTabPage::GetRanges);
    AddTabPage("RID_SVXPAGE_SLANT", SvxSlantTabPage::Create, SvxSlantTabPage::GetRanges);
}


void SvxTransformTabDialog::PageCreated(const OString& rId, SfxTabPage &rPage)
{
    if (rId == "RID_SVXPAGE_POSITION_SIZE")
    {
        SvxPositionSizeTabPage& rSvxPos =  static_cast<SvxPositionSizeTabPage&>(rPage);
        rSvxPos.SetView(pView);
        rSvxPos.Construct();

        if(nAnchorCtrls & SvxAnchorIds::NoResize)
        {
            rSvxPos.DisableResize();
        }

        if(nAnchorCtrls & SvxAnchorIds::NoProtect)
        {
            rSvxPos.DisableProtect();
            rSvxPos.UpdateControlStates();
        }
    }
    else if (rId == "RID_SVXPAGE_SWPOSSIZE")
    {
        SvxSwPosSizeTabPage& rSwPos =  static_cast<SvxSwPosSizeTabPage&>(rPage);

        rSwPos.EnableAnchorTypes(nAnchorCtrls);
        rSwPos.SetValidateFramePosLink(aValidateLink);
        rSwPos.SetView(pView);
    }
    else if (rId == "RID_SVXPAGE_ANGLE")
    {
        SvxAngleTabPage& rSvxAng =  static_cast<SvxAngleTabPage&>(rPage);

        rSvxAng.SetView( pView );
        rSvxAng.Construct();
    }
    else if (rId == "RID_SVXPAGE_SLANT")
    {
        SvxSlantTabPage& rSvxSlnt =  static_cast<SvxSlantTabPage&>(rPage);

        rSvxSlnt.SetView( pView );
        rSvxSlnt.Construct();
    }
}

void SvxTransformTabDialog::SetValidateFramePosLink(const Link<SvxSwFrameValidation&,void>& rLink)
{
    aValidateLink = rLink;
}

/*************************************************************************
|*
|*      dialog for changing the positions of the rotation
|*      angle and the rotation angle of the graphic objects
|*
\************************************************************************/
SvxAngleTabPage::SvxAngleTabPage(TabPageParent pParent, const SfxItemSet& rInAttrs)
    : SvxTabPage(pParent, "cui/ui/rotationtabpage.ui", "Rotation", rInAttrs)
    , rOutAttrs(rInAttrs)
    , pView(nullptr)
    , eDlgUnit(FUNIT_NONE)
    , m_aCtlRect(this)
    , m_xFlPosition(m_xBuilder->weld_widget("FL_POSITION"))
    , m_xMtrPosX(m_xBuilder->weld_metric_spin_button("MTR_FLD_POS_X", FUNIT_CM))
    , m_xMtrPosY(m_xBuilder->weld_metric_spin_button("MTR_FLD_POS_Y", FUNIT_CM))
    , m_xCtlRect(new weld::CustomWeld(*m_xBuilder, "CTL_RECT", m_aCtlRect))
    , m_xFlAngle(m_xBuilder->weld_widget("FL_ANGLE"))
    , m_xNfAngle(m_xBuilder->weld_spin_button("NF_ANGLE"))
    , m_xCtlAngle(new weld::CustomWeld(*m_xBuilder, "CTL_ANGLE", m_aCtlAngle))
{
    // calculate PoolUnit
    SfxItemPool* pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "no pool (!)" );
    ePoolUnit = pPool->GetMetric(SID_ATTR_TRANSFORM_POS_X);

    m_aCtlAngle.SetLinkedField(m_xNfAngle.get(), 2);
}

SvxAngleTabPage::~SvxAngleTabPage()
{
}

void SvxAngleTabPage::Construct()
{
    DBG_ASSERT(pView, "No valid view (!)");
    eDlgUnit = GetModuleFieldUnit(GetItemSet());
    SetFieldUnit(*m_xMtrPosX, eDlgUnit, true);
    SetFieldUnit(*m_xMtrPosY, eDlgUnit, true);

    if (FUNIT_MILE == eDlgUnit || FUNIT_KM == eDlgUnit)
    {
        m_xMtrPosX->set_digits(3);
        m_xMtrPosY->set_digits(3);
    }

    { // #i75273#
        ::tools::Rectangle aTempRect(pView->GetAllMarkedRect());
        pView->GetSdrPageView()->LogicToPagePos(aTempRect);
        maRange = basegfx::B2DRange(aTempRect.Left(), aTempRect.Top(), aTempRect.Right(), aTempRect.Bottom());
    }

    // Take anchor into account (Writer)
    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();

    if(rMarkList.GetMarkCount())
    {
        const SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        maAnchor = basegfx::B2DPoint(pObj->GetAnchorPos().X(), pObj->GetAnchorPos().Y());

        if(!maAnchor.equalZero()) // -> Writer
        {
            maRange = basegfx::B2DRange(maRange.getMinimum() - maAnchor, maRange.getMaximum() - maAnchor);
        }
    }

    // take scale into account
    const Fraction aUIScale(pView->GetModel()->GetUIScale());
    TransfrmHelper::ScaleRect(maRange, aUIScale);

    // take UI units into account
    sal_uInt16 nDigits(m_xMtrPosX->get_digits());
    TransfrmHelper::ConvertRect(maRange, nDigits, ePoolUnit, eDlgUnit);

    if(!pView->IsRotateAllowed())
    {
        m_xFlPosition->set_sensitive(false);
        m_xFlAngle->set_sensitive(false);
    }
}

bool SvxAngleTabPage::FillItemSet(SfxItemSet* rSet)
{
    bool bModified = false;

    if (m_aCtlAngle.IsValueModified() || m_xMtrPosX->get_value_changed_from_saved() || m_xMtrPosY->get_value_changed_from_saved())
    {
        const double fUIScale(double(pView->GetModel()->GetUIScale()));
        const double fTmpX((GetCoreValue(*m_xMtrPosX, ePoolUnit) + maAnchor.getX()) * fUIScale);
        const double fTmpY((GetCoreValue(*m_xMtrPosY, ePoolUnit) + maAnchor.getY()) * fUIScale);

        rSet->Put(SfxInt32Item(GetWhich(SID_ATTR_TRANSFORM_ANGLE), m_aCtlAngle.GetRotation()));
        rSet->Put(SfxInt32Item(GetWhich(SID_ATTR_TRANSFORM_ROT_X), basegfx::fround(fTmpX)));
        rSet->Put(SfxInt32Item(GetWhich(SID_ATTR_TRANSFORM_ROT_Y), basegfx::fround(fTmpY)));

        bModified = true;
    }

    return bModified;
}


void SvxAngleTabPage::Reset(const SfxItemSet* rAttrs)
{
    const double fUIScale(double(pView->GetModel()->GetUIScale()));

    const SfxPoolItem* pItem = GetItem( *rAttrs, SID_ATTR_TRANSFORM_ROT_X );
    if(pItem)
    {
        const double fTmp((static_cast<double>(static_cast<const SfxInt32Item*>(pItem)->GetValue()) - maAnchor.getX()) / fUIScale);
        SetMetricValue(*m_xMtrPosX, basegfx::fround(fTmp), ePoolUnit);
    }
    else
    {
        m_xMtrPosX->set_text(OUString());
    }

    pItem = GetItem(*rAttrs, SID_ATTR_TRANSFORM_ROT_Y);
    if(pItem)
    {
        const double fTmp((static_cast<double>(static_cast<const SfxInt32Item*>(pItem)->GetValue()) - maAnchor.getY()) / fUIScale);
        SetMetricValue(*m_xMtrPosY, basegfx::fround(fTmp), ePoolUnit);
    }
    else
    {
        m_xMtrPosY->set_text(OUString());
    }

    pItem = GetItem( *rAttrs, SID_ATTR_TRANSFORM_ANGLE );
    if(pItem)
    {
        m_aCtlAngle.SetRotation(static_cast<const SfxInt32Item*>(pItem)->GetValue());
    }
    else
    {
        m_aCtlAngle.SetRotation(0);
    }
    m_aCtlAngle.SaveValue();
    m_xMtrPosX->save_value();
    m_xMtrPosY->save_value();
}

VclPtr<SfxTabPage> SvxAngleTabPage::Create(TabPageParent pParent, const SfxItemSet* rSet)
{
    return VclPtr<SvxAngleTabPage>::Create(pParent, *rSet);
}

void SvxAngleTabPage::ActivatePage(const SfxItemSet& rSet)
{
    SfxBoolItem const * bPosProtect = nullptr;
    if(SfxItemState::SET == rSet.GetItemState( GetWhich(SID_ATTR_TRANSFORM_PROTECT_POS  ) , false, reinterpret_cast<SfxPoolItem const **>(&bPosProtect) ))
    {
        m_xFlPosition->set_sensitive(!bPosProtect->GetValue());
        m_xFlAngle->set_sensitive(!bPosProtect->GetValue());
    }
}

DeactivateRC SvxAngleTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if(_pSet)
    {
        FillItemSet(_pSet);
    }

    return DeactivateRC::LeavePage;
}

void SvxAngleTabPage::PointChanged(vcl::Window*, RectPoint)
{
    assert(false);
}

void SvxAngleTabPage::PointChanged(weld::DrawingArea* pDrawingArea, RectPoint eRP)
{
    if (pDrawingArea == m_aCtlRect.GetDrawingArea())
    {
        switch(eRP)
        {
            case RectPoint::LT:
            {
                m_xMtrPosX->set_value( basegfx::fround64(maRange.getMinX()), FUNIT_NONE );
                m_xMtrPosY->set_value( basegfx::fround64(maRange.getMinY()), FUNIT_NONE );
                break;
            }
            case RectPoint::MT:
            {
                m_xMtrPosX->set_value( basegfx::fround64(maRange.getCenter().getX()), FUNIT_NONE );
                m_xMtrPosY->set_value( basegfx::fround64(maRange.getMinY()), FUNIT_NONE );
                break;
            }
            case RectPoint::RT:
            {
                m_xMtrPosX->set_value( basegfx::fround64(maRange.getMaxX()), FUNIT_NONE );
                m_xMtrPosY->set_value( basegfx::fround64(maRange.getMinY()), FUNIT_NONE );
                break;
            }
            case RectPoint::LM:
            {
                m_xMtrPosX->set_value( basegfx::fround64(maRange.getMinX()), FUNIT_NONE );
                m_xMtrPosY->set_value( basegfx::fround64(maRange.getCenter().getY()), FUNIT_NONE );
                break;
            }
            case RectPoint::MM:
            {
                m_xMtrPosX->set_value( basegfx::fround64(maRange.getCenter().getX()), FUNIT_NONE );
                m_xMtrPosY->set_value( basegfx::fround64(maRange.getCenter().getY()), FUNIT_NONE );
                break;
            }
            case RectPoint::RM:
            {
                m_xMtrPosX->set_value( basegfx::fround64(maRange.getMaxX()), FUNIT_NONE );
                m_xMtrPosY->set_value( basegfx::fround64(maRange.getCenter().getY()), FUNIT_NONE );
                break;
            }
            case RectPoint::LB:
            {
                m_xMtrPosX->set_value( basegfx::fround64(maRange.getMinX()), FUNIT_NONE );
                m_xMtrPosY->set_value( basegfx::fround64(maRange.getMaxY()), FUNIT_NONE );
                break;
            }
            case RectPoint::MB:
            {
                m_xMtrPosX->set_value( basegfx::fround64(maRange.getCenter().getX()), FUNIT_NONE );
                m_xMtrPosY->set_value( basegfx::fround64(maRange.getMaxY()), FUNIT_NONE );
                break;
            }
            case RectPoint::RB:
            {
                m_xMtrPosX->set_value( basegfx::fround64(maRange.getMaxX()), FUNIT_NONE );
                m_xMtrPosY->set_value( basegfx::fround64(maRange.getMaxY()), FUNIT_NONE );
                break;
            }
        }
    }
}

/*************************************************************************
|*
|*      dialog for changing slant and corner radius
|*
\************************************************************************/
SvxSlantTabPage::SvxSlantTabPage(TabPageParent pParent, const SfxItemSet& rInAttrs)
    : SvxTabPage(pParent, "cui/ui/slantcornertabpage.ui", "SlantAndCornerRadius", rInAttrs)
    , rOutAttrs(rInAttrs)
    , pView(nullptr)
    , eDlgUnit(FUNIT_NONE)
    , m_xFlRadius(m_xBuilder->weld_widget("FL_RADIUS"))
    , m_xMtrRadius(m_xBuilder->weld_metric_spin_button("MTR_FLD_RADIUS", FUNIT_CM))
    , m_xFlAngle(m_xBuilder->weld_widget("FL_SLANT"))
    , m_xMtrAngle(m_xBuilder->weld_metric_spin_button("MTR_FLD_ANGLE", FUNIT_DEGREE))
{
    for (int i = 0; i < 2; ++i)
    {
        m_aControlGroups[i].reset(m_xBuilder->weld_widget("controlgroups" + OString::number(i+1)));
        m_aControlGroupX[i].reset(m_xBuilder->weld_widget("controlgroupx" + OString::number(i+1)));
        m_aControlX[i].reset(m_xBuilder->weld_metric_spin_button("controlx" + OString::number(i+1), FUNIT_CM));
        m_aControlGroupY[i].reset(m_xBuilder->weld_widget("controlgroupy" + OString::number(i+1)));
        m_aControlY[i].reset(m_xBuilder->weld_metric_spin_button("controly" + OString::number(i+1), FUNIT_CM));
    }

    // this page needs ExchangeSupport
    SetExchangeSupport();

    // evaluate PoolUnit
    SfxItemPool* pPool = rOutAttrs.GetPool();
    assert(pPool && "no pool (!)");
    ePoolUnit = pPool->GetMetric( SID_ATTR_TRANSFORM_POS_X );
}

SvxSlantTabPage::~SvxSlantTabPage()
{
}

void SvxSlantTabPage::Construct()
{
    // get the range
    DBG_ASSERT(pView, "no valid view (!)");
    eDlgUnit = GetModuleFieldUnit(GetItemSet());
    SetFieldUnit(*m_xMtrRadius, eDlgUnit, true);

    { // #i75273#
        ::tools::Rectangle aTempRect(pView->GetAllMarkedRect());
        pView->GetSdrPageView()->LogicToPagePos(aTempRect);
        maRange = basegfx::B2DRange(aTempRect.Left(), aTempRect.Top(), aTempRect.Right(), aTempRect.Bottom());
    }
}

bool SvxSlantTabPage::FillItemSet(SfxItemSet* rAttrs)
{
    bool  bModified = false;

    if (m_xMtrRadius->get_value_changed_from_saved())
    {
        Fraction aUIScale = pView->GetModel()->GetUIScale();
        long nTmp = long(GetCoreValue(*m_xMtrRadius, ePoolUnit) * aUIScale);

        rAttrs->Put( makeSdrEckenradiusItem( nTmp ) );
        bModified = true;
    }

    if (m_xMtrAngle->get_value_changed_from_saved())
    {
        sal_Int32 nValue = static_cast<sal_Int32>(m_xMtrAngle->get_value(FUNIT_NONE));
        rAttrs->Put( SfxInt32Item( SID_ATTR_TRANSFORM_SHEAR, nValue ) );
        bModified = true;
    }

    if( bModified )
    {
        // set reference points
        ::tools::Rectangle aObjectRect(pView->GetAllMarkedRect());
        pView->GetSdrPageView()->LogicToPagePos(aObjectRect);
        Point aPt = aObjectRect.Center();

        rAttrs->Put(SfxInt32Item(SID_ATTR_TRANSFORM_SHEAR_X, aPt.X()));
        rAttrs->Put(SfxInt32Item(SID_ATTR_TRANSFORM_SHEAR_Y, aPt.Y()));
        rAttrs->Put( SfxBoolItem( SID_ATTR_TRANSFORM_SHEAR_VERTICAL, false ) );
    }

    bool bControlPointsChanged = false;
    for (int i = 0; i < 2; ++i)
    {
        bControlPointsChanged |= (m_aControlX[i]->get_value_changed_from_saved() ||
                                  m_aControlY[i]->get_value_changed_from_saved());
    }

    if (!bControlPointsChanged)
        return bModified;

    bool bSelectionIsSdrObjCustomShape(false);

    while(true)
    {
        if(nullptr == pView)
        {
            break;
        }

        if(0 == pView->GetMarkedObjectList().GetMarkCount())
        {
            break;
        }

        SdrObject* pCandidate(pView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj());

        if(nullptr == pCandidate)
        {
            break;
        }

        if(nullptr == dynamic_cast< SdrObjCustomShape* >(pCandidate))
        {
            break;
        }

        bSelectionIsSdrObjCustomShape = true;
        break;
    }

    if(bSelectionIsSdrObjCustomShape)
    {
        SdrObjCustomShape& rSdrObjCustomShape(
            static_cast< SdrObjCustomShape& >(
                *pView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj()));
        SdrModel& rModel(rSdrObjCustomShape.getSdrModelFromSdrObject());
        SdrUndoAction* pUndo(
            rModel.IsUndoEnabled()
                ? rModel.GetSdrUndoFactory().CreateUndoAttrObject(rSdrObjCustomShape)
                : nullptr);

        if(pUndo)
        {
            rModel.BegUndo(pUndo->GetComment());
        }

        EnhancedCustomShape2d aShape(rSdrObjCustomShape);
        ::tools::Rectangle aLogicRect = aShape.GetLogicRect();

        for (int i = 0; i < 2; ++i)
        {
            if (m_aControlX[i]->get_value_changed_from_saved() || m_aControlY[i]->get_value_changed_from_saved())
            {
                Point aNewPosition(GetCoreValue(*m_aControlX[i], ePoolUnit),
                                GetCoreValue(*m_aControlY[i], ePoolUnit));
                aNewPosition.Move(aLogicRect.Left(), aLogicRect.Top());

                css::awt::Point aPosition;
                aPosition.X = aNewPosition.X();
                aPosition.Y = aNewPosition.Y();

                aShape.SetHandleControllerPosition(i, aPosition);
            }
        }

        rSdrObjCustomShape.SetChanged();
        rSdrObjCustomShape.BroadcastObjectChange();
        bModified = true;

        if (pUndo)
        {
            rModel.AddUndo(pUndo);
            rModel.EndUndo();
        }
    }

    return bModified;
}

void SvxSlantTabPage::Reset(const SfxItemSet* rAttrs)
{
    // if the view has selected objects, items with SfxItemState::DEFAULT need to be disabled
    const SfxPoolItem* pItem;

    // corner radius
    if(!pView->IsEdgeRadiusAllowed())
    {
        m_xMtrRadius->set_text("");
        m_xFlRadius->set_sensitive(false);
    }
    else
    {
        pItem = GetItem( *rAttrs, SDRATTR_ECKENRADIUS );

        if( pItem )
        {
            const double fUIScale(double(pView->GetModel()->GetUIScale()));
            const double fTmp(static_cast<double>(static_cast<const SdrMetricItem*>(pItem)->GetValue()) / fUIScale);
            SetMetricValue(*m_xMtrRadius, basegfx::fround(fTmp), ePoolUnit);
        }
        else
        {
            m_xMtrRadius->set_text("");
        }
    }

    m_xMtrRadius->save_value();

    // slant: angle
    if( !pView->IsShearAllowed() )
    {
        m_xMtrAngle->set_text( "" );
        m_xFlAngle->set_sensitive(false);
    }
    else
    {
        pItem = GetItem( *rAttrs, SID_ATTR_TRANSFORM_SHEAR );

        if( pItem )
        {
            m_xMtrAngle->set_value(static_cast<const SfxInt32Item*>(pItem)->GetValue(), FUNIT_NONE);
        }
        else
        {
            m_xMtrAngle->set_text("");
        }
    }

    m_xMtrAngle->save_value();

    bool bSelectionIsSdrObjCustomShape(false);

    while(true)
    {
        if(1 != pView->GetMarkedObjectList().GetMarkCount())
        {
            break;
        }

        SdrObject* pCandidate(pView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj());

        if(nullptr == pCandidate)
        {
            break;
        }

        if(nullptr == dynamic_cast< SdrObjCustomShape* >(pCandidate))
        {
            break;
        }

        bSelectionIsSdrObjCustomShape = true;
        break;
    }

    if(bSelectionIsSdrObjCustomShape)
    {
        SdrObjCustomShape& rSdrObjCustomShape(
            static_cast< SdrObjCustomShape& >(
                *pView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj()));

        //save geometry
        SdrCustomShapeGeometryItem aInitialGeometry(rSdrObjCustomShape.GetMergedItem(SDRATTR_CUSTOMSHAPE_GEOMETRY));
        EnhancedCustomShape2d aShape(rSdrObjCustomShape);

        for (int i = 0; i < 2; ++i)
        {
            Point aInitialPosition;
            if (!aShape.GetHandlePosition(i, aInitialPosition))
                break;
            m_aControlGroups[i]->set_sensitive(true);
            css::awt::Point aPosition;

            aPosition.X = SAL_MAX_INT32/2;
            aPosition.Y = SAL_MAX_INT32/2;
            aShape.SetHandleControllerPosition(i, aPosition);
            Point aMaxPosition;
            aShape.GetHandlePosition(i, aMaxPosition);

            aPosition.X = SAL_MIN_INT32/2;
            aPosition.Y = SAL_MIN_INT32/2;
            aShape.SetHandleControllerPosition(i, aPosition);
            Point aMinPosition;
            aShape.GetHandlePosition(i, aMinPosition);

            ::tools::Rectangle aLogicRect = aShape.GetLogicRect();
            aInitialPosition.Move(-aLogicRect.Left(), -aLogicRect.Top());
            aMaxPosition.Move(-aLogicRect.Left(), -aLogicRect.Top());
            aMinPosition.Move(-aLogicRect.Left(), -aLogicRect.Top());

            SetMetricValue(*m_aControlX[i], aInitialPosition.X(), ePoolUnit);
            SetMetricValue(*m_aControlY[i], aInitialPosition.Y(), ePoolUnit);

            if (aMaxPosition.X() == aMinPosition.X())
                m_aControlGroupX[i]->set_sensitive(false);
            else
                m_aControlX[i]->set_range(aMinPosition.X(), aMaxPosition.X(), FUNIT_MM);
            if (aMaxPosition.Y() == aMinPosition.Y())
                m_aControlGroupY[i]->set_sensitive(false);
            else
                m_aControlY[i]->set_range(aMinPosition.Y(), aMaxPosition.Y(), FUNIT_MM);
        }

        //restore geometry
        rSdrObjCustomShape.SetMergedItem(aInitialGeometry);
    }

    for (int i = 0; i < 2; ++i)
    {
        m_aControlX[i]->save_value();
        m_aControlY[i]->save_value();
    }
}

VclPtr<SfxTabPage> SvxSlantTabPage::Create(TabPageParent pParent, const SfxItemSet* rOutAttrs)
{
    return VclPtr<SvxSlantTabPage>::Create(pParent, *rOutAttrs);
}

void SvxSlantTabPage::ActivatePage( const SfxItemSet& rSet )
{
    SfxRectangleItem const * pRectItem = nullptr;

    if( SfxItemState::SET == rSet.GetItemState( GetWhich( SID_ATTR_TRANSFORM_INTERN ) , false, reinterpret_cast<SfxPoolItem const **>(&pRectItem) ) )
    {
        const ::tools::Rectangle aTempRect(pRectItem->GetValue());
        maRange = basegfx::B2DRange(aTempRect.Left(), aTempRect.Top(), aTempRect.Right(), aTempRect.Bottom());
    }

    SfxBoolItem const * bPosProtect = nullptr;
    if(SfxItemState::SET == rSet.GetItemState( GetWhich(SID_ATTR_TRANSFORM_PROTECT_POS  ) , false, reinterpret_cast<SfxPoolItem const **>(&bPosProtect) ))
    {
        m_xFlAngle->set_sensitive(!bPosProtect->GetValue());
    }
    SfxBoolItem const * bSizeProtect = nullptr;
    if(SfxItemState::SET == rSet.GetItemState( GetWhich(SID_ATTR_TRANSFORM_PROTECT_SIZE ) , false, reinterpret_cast<SfxPoolItem const **>(&bSizeProtect) ))
    {
        m_xFlAngle->set_sensitive(!bSizeProtect->GetValue());
    }

}

DeactivateRC SvxSlantTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if(_pSet)
    {
        FillItemSet(_pSet);
    }

    return DeactivateRC::LeavePage;
}


void SvxSlantTabPage::PointChanged( vcl::Window*, RectPoint )
{
}

void SvxSlantTabPage::PointChanged( weld::DrawingArea*, RectPoint )
{
}

/*************************************************************************
|*
|*      Dialog for changing position and size of graphic objects
|*
\************************************************************************/
SvxPositionSizeTabPage::SvxPositionSizeTabPage(TabPageParent pParent, const SfxItemSet& rInAttrs)
    : SvxTabPage(pParent, "cui/ui/possizetabpage.ui", "PositionAndSize", rInAttrs)
    , mrOutAttrs(rInAttrs)
    , mpView(nullptr)
    , meDlgUnit(FUNIT_NONE)
    , mnProtectSizeState(TRISTATE_FALSE)
    , mbPageDisabled(false)
    , mbProtectDisabled(false)
    , mbSizeDisabled(false)
    , mbAdjustDisabled(true)
    , mbIgnoreAutoGrowWidth(true)
    , mbIgnoreAutoGrowHeight(true)
    , mfOldWidth(0.0)
    , mfOldHeight(0.0)
    , m_aCtlPos(this)
    , m_aCtlSize(this)
    , m_xFlPosition(m_xBuilder->weld_widget("FL_POSITION"))
    , m_xMtrPosX(m_xBuilder->weld_metric_spin_button("MTR_FLD_POS_X", FUNIT_CM))
    , m_xMtrPosY(m_xBuilder->weld_metric_spin_button("MTR_FLD_POS_Y", FUNIT_CM))
    , m_xCtlPos(new weld::CustomWeld(*m_xBuilder, "CTL_POSRECT", m_aCtlPos))
    , m_xFlSize(m_xBuilder->weld_widget("FL_SIZE"))
    , m_xFtWidth(m_xBuilder->weld_label("FT_WIDTH"))
    , m_xMtrWidth(m_xBuilder->weld_metric_spin_button("MTR_FLD_WIDTH", FUNIT_CM))
    , m_xFtHeight(m_xBuilder->weld_label("FT_HEIGHT"))
    , m_xMtrHeight(m_xBuilder->weld_metric_spin_button("MTR_FLD_HEIGHT", FUNIT_CM))
    , m_xCbxScale(m_xBuilder->weld_check_button("CBX_SCALE"))
    , m_xCtlSize(new weld::CustomWeld(*m_xBuilder, "CTL_SIZERECT", m_aCtlSize))
    , m_xFlProtect(m_xBuilder->weld_widget("FL_PROTECT"))
    , m_xTsbPosProtect(m_xBuilder->weld_check_button("TSB_POSPROTECT"))
    , m_xTsbSizeProtect(m_xBuilder->weld_check_button("TSB_SIZEPROTECT"))
    , m_xFlAdjust(m_xBuilder->weld_widget("FL_ADJUST"))
    , m_xTsbAutoGrowWidth(m_xBuilder->weld_check_button("TSB_AUTOGROW_WIDTH"))
    , m_xTsbAutoGrowHeight(m_xBuilder->weld_check_button("TSB_AUTOGROW_HEIGHT"))
{
    // this page needs ExchangeSupport
    SetExchangeSupport();

    // evaluate PoolUnit
    SfxItemPool* pPool = mrOutAttrs.GetPool();
    DBG_ASSERT( pPool, "no pool (!)" );
    mePoolUnit = pPool->GetMetric( SID_ATTR_TRANSFORM_POS_X );

    m_aCtlPos.SetActualRP(RectPoint::LT);
    m_aCtlSize.SetActualRP(RectPoint::LT);
    meRP = RectPoint::LT; // see above

    m_xMtrWidth->connect_value_changed( LINK( this, SvxPositionSizeTabPage, ChangeWidthHdl ) );
    m_xMtrHeight->connect_value_changed( LINK( this, SvxPositionSizeTabPage, ChangeHeightHdl ) );
    m_xCbxScale->connect_toggled( LINK( this, SvxPositionSizeTabPage, ClickAutoHdl ) );

    m_xTsbAutoGrowWidth->set_sensitive(false);
    m_xTsbAutoGrowHeight->set_sensitive(false);
    m_xFlAdjust->set_sensitive(false);

    // #i2379# disable controls when protected
    m_xTsbPosProtect->connect_toggled( LINK( this, SvxPositionSizeTabPage, ChangePosProtectHdl ) );
    m_xTsbSizeProtect->connect_toggled( LINK( this, SvxPositionSizeTabPage, ChangeSizeProtectHdl ) );
}

SvxPositionSizeTabPage::~SvxPositionSizeTabPage()
{
}

void SvxPositionSizeTabPage::Construct()
{
    // get range and work area
    DBG_ASSERT( mpView, "no valid view (!)" );
    meDlgUnit = GetModuleFieldUnit( GetItemSet() );
    SetFieldUnit( *m_xMtrPosX, meDlgUnit, true );
    SetFieldUnit( *m_xMtrPosY, meDlgUnit, true );
    SetFieldUnit( *m_xMtrWidth, meDlgUnit, true );
    SetFieldUnit( *m_xMtrHeight, meDlgUnit, true );

    if(FUNIT_MILE == meDlgUnit || FUNIT_KM == meDlgUnit)
    {
        m_xMtrPosX->set_digits( 3 );
        m_xMtrPosY->set_digits( 3 );
        m_xMtrWidth->set_digits( 3 );
        m_xMtrHeight->set_digits( 3 );
    }

    { // #i75273#
        ::tools::Rectangle aTempRect(mpView->GetAllMarkedRect());
        mpView->GetSdrPageView()->LogicToPagePos(aTempRect);
        maRange = basegfx::B2DRange(aTempRect.Left(), aTempRect.Top(), aTempRect.Right(), aTempRect.Bottom());
    }

    { // #i75273#
        ::tools::Rectangle aTempRect(mpView->GetWorkArea());
        mpView->GetSdrPageView()->LogicToPagePos(aTempRect);
        maWorkRange = basegfx::B2DRange(aTempRect.Left(), aTempRect.Top(), aTempRect.Right(), aTempRect.Bottom());
    }

    // take anchor into account (Writer)
    const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();

    if(rMarkList.GetMarkCount())
    {
        const SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        maAnchor = basegfx::B2DPoint(pObj->GetAnchorPos().X(), pObj->GetAnchorPos().Y());

        if(!maAnchor.equalZero()) // -> Writer
        {
            for(size_t i = 1; i < rMarkList.GetMarkCount(); ++i)
            {
                pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();

                if(maAnchor != basegfx::B2DPoint(pObj->GetAnchorPos().X(), pObj->GetAnchorPos().Y()))
                {
                    // different anchor positions
                    m_xMtrPosX->set_text("");
                    m_xMtrPosY->set_text("");
                    mbPageDisabled = true;
                    return;
                }
            }

            // translate ranges about anchor
            maRange = basegfx::B2DRange(maRange.getMinimum() - maAnchor, maRange.getMaximum() - maAnchor);
            maWorkRange = basegfx::B2DRange(maWorkRange.getMinimum() - maAnchor, maWorkRange.getMaximum() - maAnchor);
        }
    }

    // this should happen via SID_ATTR_TRANSFORM_AUTOSIZE
    if(1 == rMarkList.GetMarkCount())
    {
        const SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        const SdrObjKind eKind(static_cast<SdrObjKind>(pObj->GetObjIdentifier()));

        if((pObj->GetObjInventor() == SdrInventor::Default) &&
            (OBJ_TEXT == eKind || OBJ_TITLETEXT == eKind || OBJ_OUTLINETEXT == eKind) &&
            pObj->HasText())
        {
            mbAdjustDisabled = false;

            m_xFlAdjust->set_sensitive(true);

            m_xTsbAutoGrowWidth->connect_toggled( LINK( this, SvxPositionSizeTabPage, ClickSizeProtectHdl ) );
            m_xTsbAutoGrowHeight->connect_toggled( LINK( this, SvxPositionSizeTabPage, ClickSizeProtectHdl ) );

            // is used as flag to evaluate if its selectable
            mbIgnoreAutoGrowWidth = false;
            mbIgnoreAutoGrowHeight = false;
        }
    }

    // take scale into account
    const Fraction aUIScale(mpView->GetModel()->GetUIScale());
    TransfrmHelper::ScaleRect( maWorkRange, aUIScale );
    TransfrmHelper::ScaleRect( maRange, aUIScale );

    // take UI units into account
    const sal_uInt16 nDigits(m_xMtrPosX->get_digits());
    TransfrmHelper::ConvertRect( maWorkRange, nDigits, mePoolUnit, meDlgUnit );
    TransfrmHelper::ConvertRect( maRange, nDigits, mePoolUnit, meDlgUnit );

    SetMinMaxPosition();
}


bool SvxPositionSizeTabPage::FillItemSet( SfxItemSet* rOutAttrs )
{
    bool bModified(false);

    if ( m_xMtrWidth->has_focus() )
    {
        ChangeWidthHdl( *m_xMtrWidth );
    }

    if ( m_xMtrHeight->has_focus() )
    {
        ChangeHeightHdl( *m_xMtrHeight );
    }

    if( !mbPageDisabled )
    {
        if (m_xMtrPosX->get_value_changed_from_saved() || m_xMtrPosY->get_value_changed_from_saved())
        {
            const double fUIScale(double(mpView->GetModel()->GetUIScale()));
            double fX((GetCoreValue( *m_xMtrPosX, mePoolUnit ) + maAnchor.getX()) * fUIScale);
            double fY((GetCoreValue( *m_xMtrPosY, mePoolUnit ) + maAnchor.getY()) * fUIScale);

            { // #i75273#
                ::tools::Rectangle aTempRect(mpView->GetAllMarkedRect());
                mpView->GetSdrPageView()->LogicToPagePos(aTempRect);
                maRange = basegfx::B2DRange(aTempRect.Left(), aTempRect.Top(), aTempRect.Right(), aTempRect.Bottom());
            }

            // #101581# GetTopLeftPosition(...) needs coordinates after UI scaling, in real PagePositions
            GetTopLeftPosition(fX, fY, maRange);

            rOutAttrs->Put(SfxInt32Item(GetWhich(SID_ATTR_TRANSFORM_POS_X), basegfx::fround(fX)));
            rOutAttrs->Put(SfxInt32Item(GetWhich(SID_ATTR_TRANSFORM_POS_Y), basegfx::fround(fY)));

            bModified = true;
        }

        if (m_xTsbPosProtect->get_state_changed_from_saved())
        {
            if( m_xTsbPosProtect->get_inconsistent() )
            {
                rOutAttrs->InvalidateItem( SID_ATTR_TRANSFORM_PROTECT_POS );
            }
            else
            {
                rOutAttrs->Put(
                    SfxBoolItem( GetWhich( SID_ATTR_TRANSFORM_PROTECT_POS ),
                    m_xTsbPosProtect->get_active() ) );
            }

            bModified = true;
        }
    }

    if (m_xMtrWidth->get_value_changed_from_saved() || m_xMtrHeight->get_value_changed_from_saved())
    {
        Fraction aUIScale = mpView->GetModel()->GetUIScale();

        // get Width
        double nWidth = static_cast<double>(m_xMtrWidth->get_value(FUNIT_100TH_MM));
        long lWidth = long(nWidth * static_cast<double>(aUIScale));
        lWidth = OutputDevice::LogicToLogic( lWidth, MapUnit::Map100thMM, mePoolUnit );
        lWidth = static_cast<long>(m_xMtrWidth->denormalize( lWidth ));

        // get Height
        double nHeight = static_cast<double>(m_xMtrHeight->get_value(FUNIT_100TH_MM));
        long lHeight = long(nHeight * static_cast<double>(aUIScale));
        lHeight = OutputDevice::LogicToLogic( lHeight, MapUnit::Map100thMM, mePoolUnit );
        lHeight = static_cast<long>(m_xMtrHeight->denormalize( lHeight ));

        // put Width & Height to itemset
        rOutAttrs->Put( SfxUInt32Item( GetWhich( SID_ATTR_TRANSFORM_WIDTH ), static_cast<sal_uInt32>(lWidth) ) );
        rOutAttrs->Put( SfxUInt32Item( GetWhich( SID_ATTR_TRANSFORM_HEIGHT ), static_cast<sal_uInt32>(lHeight) ) );
        rOutAttrs->Put( SfxAllEnumItem( GetWhich( SID_ATTR_TRANSFORM_SIZE_POINT ), sal::static_int_cast< sal_uInt16 >( meRP ) ) );
        bModified = true;
    }

    if (m_xTsbSizeProtect->get_state_changed_from_saved())
    {
        if ( m_xTsbSizeProtect->get_inconsistent() )
            rOutAttrs->InvalidateItem( SID_ATTR_TRANSFORM_PROTECT_SIZE );
        else
            rOutAttrs->Put(
                SfxBoolItem( GetWhich( SID_ATTR_TRANSFORM_PROTECT_SIZE ),
                m_xTsbSizeProtect->get_active() ) );
        bModified = true;
    }

    if (m_xTsbAutoGrowWidth->get_state_changed_from_saved())
    {
        if (!mbIgnoreAutoGrowWidth)
        {
            if( m_xTsbAutoGrowWidth->get_inconsistent() )
                rOutAttrs->InvalidateItem( SID_ATTR_TRANSFORM_AUTOWIDTH );
            else
                rOutAttrs->Put(
                    SfxBoolItem( GetWhich( SID_ATTR_TRANSFORM_AUTOWIDTH ),
                    m_xTsbAutoGrowWidth->get_active() ) );
        }
        bModified = true;
    }

    if (m_xTsbAutoGrowHeight->get_state_changed_from_saved())
    {
        if (!mbIgnoreAutoGrowHeight)
        {
            if (m_xTsbAutoGrowHeight->get_inconsistent())
            {
                rOutAttrs->InvalidateItem( SID_ATTR_TRANSFORM_AUTOHEIGHT );
            }
            else
            {
                rOutAttrs->Put(
                    SfxBoolItem( GetWhich( SID_ATTR_TRANSFORM_AUTOHEIGHT ),
                    m_xTsbAutoGrowHeight->get_active() ) );
            }
        }
        bModified = true;
    }

    return bModified;
}

void SvxPositionSizeTabPage::Reset( const SfxItemSet*  )
{
    const SfxPoolItem* pItem;
    const double fUIScale(double(mpView->GetModel()->GetUIScale()));

    if ( !mbPageDisabled )
    {
        pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_POS_X );
        if ( pItem )
        {
            const double fTmp((static_cast<const SfxInt32Item*>(pItem)->GetValue() - maAnchor.getX()) / fUIScale);
            SetMetricValue(*m_xMtrPosX, basegfx::fround(fTmp), mePoolUnit);
        }

        pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_POS_Y );
        if ( pItem )
        {
            const double fTmp((static_cast<const SfxInt32Item*>(pItem)->GetValue() - maAnchor.getY()) / fUIScale);
            SetMetricValue(*m_xMtrPosY, basegfx::fround(fTmp), mePoolUnit);
        }

        pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_PROTECT_POS );
        if ( pItem )
        {
            bool bProtected = static_cast<const SfxBoolItem*>( pItem )->GetValue();
            m_xTsbPosProtect->set_active(bProtected);
        }
        else
        {
            m_xTsbPosProtect->set_inconsistent(true);
        }

        m_xTsbPosProtect->save_state();
        m_aCtlPos.Reset();

        // #i2379# Disable controls for protected objects
        ChangePosProtectHdl(*m_xTsbPosProtect);
    }

    { // #i75273# set width
        pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_WIDTH );
        mfOldWidth = std::max( pItem ? static_cast<double>(static_cast<const SfxUInt32Item*>(pItem)->GetValue()) : 0.0, 1.0 );
        double fTmpWidth((OutputDevice::LogicToLogic(static_cast<sal_Int32>(mfOldWidth), mePoolUnit, MapUnit::Map100thMM)) / fUIScale);
        if (m_xMtrWidth->get_digits())
            fTmpWidth *= pow(10.0, m_xMtrWidth->get_digits());
        m_xMtrWidth->set_value(static_cast<int>(fTmpWidth), FUNIT_100TH_MM);
    }

    { // #i75273# set height
        pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_HEIGHT );
        mfOldHeight = std::max( pItem ? static_cast<double>(static_cast<const SfxUInt32Item*>(pItem)->GetValue()) : 0.0, 1.0 );
        double fTmpHeight((OutputDevice::LogicToLogic(static_cast<sal_Int32>(mfOldHeight), mePoolUnit, MapUnit::Map100thMM)) / fUIScale);
        if (m_xMtrHeight->get_digits())
            fTmpHeight *= pow(10.0, m_xMtrHeight->get_digits());
        m_xMtrHeight->set_value(static_cast<int>(fTmpHeight), FUNIT_100TH_MM);
    }

    pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_PROTECT_SIZE );
    if ( pItem )
    {
        m_xTsbSizeProtect->set_active(static_cast<const SfxBoolItem*>(pItem)->GetValue());
    }
    else
        m_xTsbSizeProtect->set_inconsistent(true);

    pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_AUTOWIDTH );
    if ( pItem )
    {
        m_xTsbAutoGrowWidth->set_active(static_cast<const SfxBoolItem*>( pItem )->GetValue());
    }
    else
        m_xTsbAutoGrowWidth->set_inconsistent(true);

    pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_AUTOHEIGHT );
    if ( pItem )
    {
        m_xTsbAutoGrowHeight->set_active(static_cast<const SfxBoolItem*>( pItem )->GetValue());
    }
    else
        m_xTsbAutoGrowHeight->set_inconsistent(true);

    // Is matching set?
    OUString aStr = GetUserData();
    m_xCbxScale->set_active(aStr.toInt32() != 0);

    m_xTsbSizeProtect->save_state();
    m_xTsbAutoGrowWidth->save_state();
    m_xTsbAutoGrowHeight->save_state();
    ClickSizeProtectHdl(*m_xTsbAutoGrowHeight);

    // #i2379# Disable controls for protected objects
    ChangeSizeProtectHdl(*m_xTsbSizeProtect);
}

VclPtr<SfxTabPage> SvxPositionSizeTabPage::Create(TabPageParent pParent, const SfxItemSet* rOutAttrs)
{
    return VclPtr<SvxPositionSizeTabPage>::Create(pParent, *rOutAttrs);
}

void SvxPositionSizeTabPage::ActivatePage( const SfxItemSet& rSet )
{
    SfxRectangleItem const * pRectItem = nullptr;

    if( SfxItemState::SET == rSet.GetItemState( GetWhich( SID_ATTR_TRANSFORM_INTERN ) , false, reinterpret_cast<SfxPoolItem const **>(&pRectItem) ) )
    {
        { // #i75273#
            const ::tools::Rectangle aTempRect(pRectItem->GetValue());
            maRange = basegfx::B2DRange(aTempRect.Left(), aTempRect.Top(), aTempRect.Right(), aTempRect.Bottom());
        }

        SetMinMaxPosition();
    }
}


DeactivateRC SvxPositionSizeTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( _pSet )
    {
        double fX(static_cast<double>(m_xMtrPosX->get_value(FUNIT_NONE)));
        double fY(static_cast<double>(m_xMtrPosY->get_value(FUNIT_NONE)));

        GetTopLeftPosition(fX, fY, maRange);
        const ::tools::Rectangle aOutRectangle(
            basegfx::fround(fX), basegfx::fround(fY),
            basegfx::fround(fX + maRange.getWidth()), basegfx::fround(fY + maRange.getHeight()));
        _pSet->Put(SfxRectangleItem(SID_ATTR_TRANSFORM_INTERN, aOutRectangle));
        _pSet->Put(SfxBoolItem( GetWhich( SID_ATTR_TRANSFORM_PROTECT_POS ),
            m_xTsbPosProtect->get_state() == TRISTATE_TRUE ));
        _pSet->Put(SfxBoolItem( GetWhich( SID_ATTR_TRANSFORM_PROTECT_SIZE ),
            m_xTsbSizeProtect->get_state() == TRISTATE_TRUE ));
        FillItemSet(_pSet);
    }

    return DeactivateRC::LeavePage;
}


IMPL_LINK_NOARG(SvxPositionSizeTabPage, ChangePosProtectHdl, weld::ToggleButton&, void)
{
    // #106572# Remember user's last choice
    m_xTsbSizeProtect->set_state(m_xTsbPosProtect->get_state() == TRISTATE_TRUE ?  TRISTATE_TRUE : mnProtectSizeState);
    UpdateControlStates();
}


void SvxPositionSizeTabPage::UpdateControlStates()
{
    const bool bPosProtect =  m_xTsbPosProtect->get_state() == TRISTATE_TRUE;
    const bool bSizeProtect = m_xTsbSizeProtect->get_state() == TRISTATE_TRUE;
    const bool bHeightChecked = !mbIgnoreAutoGrowHeight && (m_xTsbAutoGrowHeight->get_active());
    const bool bWidthChecked = !mbIgnoreAutoGrowWidth && (m_xTsbAutoGrowWidth->get_active());

    m_xFlPosition->set_sensitive(!bPosProtect && !mbPageDisabled);

    m_xTsbPosProtect->set_sensitive( !mbProtectDisabled && !mbPageDisabled );

    m_xFlSize->set_sensitive( !mbSizeDisabled && !bSizeProtect );

    m_xFtWidth->set_sensitive( !mbSizeDisabled && !bSizeProtect && !bWidthChecked );
    m_xMtrWidth->set_sensitive( !mbSizeDisabled && !bSizeProtect && !bWidthChecked );

    m_xFtHeight->set_sensitive( !mbSizeDisabled && !bSizeProtect && !bHeightChecked );
    m_xMtrHeight->set_sensitive( !mbSizeDisabled && !bSizeProtect && !bHeightChecked );

    m_xCbxScale->set_sensitive( !mbSizeDisabled && !bSizeProtect && !bHeightChecked && !bWidthChecked );
    m_xCtlSize->set_sensitive( !mbSizeDisabled && !bSizeProtect && (!bHeightChecked || !bWidthChecked) );

    m_xFlProtect->set_sensitive( !mbProtectDisabled );
    m_xTsbSizeProtect->set_sensitive( !mbProtectDisabled && !bPosProtect );

    m_xFlAdjust->set_sensitive( !mbSizeDisabled && !bSizeProtect && !mbAdjustDisabled );

    m_aCtlSize.Invalidate();
    m_aCtlPos.Invalidate();
}

IMPL_LINK_NOARG(SvxPositionSizeTabPage, ChangeSizeProtectHdl, weld::ToggleButton&, void)
{
    if (m_xTsbSizeProtect->get_sensitive())
    {
        // #106572# Remember user's last choice

        // Note: this works only as long as the dialog is open.  When
        // the user closes the dialog, there is no way to remember
        // whether size was enabled or disabled before pos protect was
        // clicked. Thus, if pos protect is selected, the dialog is
        // closed and reopened again, unchecking pos protect will
        // always uncheck size protect, too. That's life.
        mnProtectSizeState = m_xTsbSizeProtect->get_state();
    }

    UpdateControlStates();
}

void SvxPositionSizeTabPage::SetMinMaxPosition()
{
    // position
    double fLeft(maWorkRange.getMinX());
    double fTop(maWorkRange.getMinY());
    double fRight(maWorkRange.getMaxX());
    double fBottom(maWorkRange.getMaxY());

    switch (m_aCtlPos.GetActualRP())
    {
        case RectPoint::LT:
        {
            fRight  -= maRange.getWidth();
            fBottom -= maRange.getHeight();
            break;
        }
        case RectPoint::MT:
        {
            fLeft   += maRange.getWidth() / 2.0;
            fRight  -= maRange.getWidth() / 2.0;
            fBottom -= maRange.getHeight();
            break;
        }
        case RectPoint::RT:
        {
            fLeft   += maRange.getWidth();
            fBottom -= maRange.getHeight();
            break;
        }
        case RectPoint::LM:
        {
            fRight  -= maRange.getWidth();
            fTop    += maRange.getHeight() / 2.0;
            fBottom -= maRange.getHeight() / 2.0;
            break;
        }
        case RectPoint::MM:
        {
            fLeft   += maRange.getWidth() / 2.0;
            fRight  -= maRange.getWidth() / 2.0;
            fTop    += maRange.getHeight() / 2.0;
            fBottom -= maRange.getHeight() / 2.0;
            break;
        }
        case RectPoint::RM:
        {
            fLeft   += maRange.getWidth();
            fTop    += maRange.getHeight() / 2.0;
            fBottom -= maRange.getHeight() / 2.0;
            break;
        }
        case RectPoint::LB:
        {
            fRight  -= maRange.getWidth();
            fTop    += maRange.getHeight();
            break;
        }
        case RectPoint::MB:
        {
            fLeft   += maRange.getWidth() / 2.0;
            fRight  -= maRange.getWidth() / 2.0;
            fTop    += maRange.getHeight();
            break;
        }
        case RectPoint::RB:
        {
            fLeft   += maRange.getWidth();
            fTop    += maRange.getHeight();
            break;
        }
    }

    const double fMaxLong(static_cast<double>(MetricField::ConvertValue( LONG_MAX, 0, MapUnit::Map100thMM, meDlgUnit ) - 1));
    fLeft = basegfx::clamp(fLeft, -fMaxLong, fMaxLong);
    fRight = basegfx::clamp(fRight, -fMaxLong, fMaxLong);
    fTop = basegfx::clamp(fTop, - fMaxLong, fMaxLong);
    fBottom = basegfx::clamp(fBottom, -fMaxLong, fMaxLong);

    // #i75273# normalizing when setting the min/max values was wrong, removed
    m_xMtrPosX->set_range(basegfx::fround64(fLeft), basegfx::fround64(fRight), FUNIT_NONE);
    m_xMtrPosY->set_range(basegfx::fround64(fTop), basegfx::fround64(fBottom), FUNIT_NONE);

    // size
    fLeft = maWorkRange.getMinX();
    fTop = maWorkRange.getMinY();
    fRight = maWorkRange.getMaxX();
    fBottom = maWorkRange.getMaxY();
    double fNewX(0);
    double fNewY(0);

    switch (m_aCtlSize.GetActualRP())
    {
        case RectPoint::LT:
        {
            fNewX = maWorkRange.getWidth() - ( maRange.getMinX() - fLeft );
            fNewY = maWorkRange.getHeight() - ( maRange.getMinY() - fTop );
            break;
        }
        case RectPoint::MT:
        {
            fNewX = std::min( maRange.getCenter().getX() - fLeft, fRight - maRange.getCenter().getX() ) * 2.0;
            fNewY = maWorkRange.getHeight() - ( maRange.getMinY() - fTop );
            break;
        }
        case RectPoint::RT:
        {
            fNewX = maWorkRange.getWidth() - ( fRight - maRange.getMaxX() );
            fNewY = maWorkRange.getHeight() - ( maRange.getMinY() - fTop );
            break;
        }
        case RectPoint::LM:
        {
            fNewX = maWorkRange.getWidth() - ( maRange.getMinX() - fLeft );
            fNewY = std::min( maRange.getCenter().getY() - fTop, fBottom - maRange.getCenter().getY() ) * 2.0;
            break;
        }
        case RectPoint::MM:
        {
            const double f1(maRange.getCenter().getX() - fLeft);
            const double f2(fRight - maRange.getCenter().getX());
            const double f3(std::min(f1, f2));
            const double f4(maRange.getCenter().getY() - fTop);
            const double f5(fBottom - maRange.getCenter().getY());
            const double f6(std::min(f4, f5));

            fNewX = f3 * 2.0;
            fNewY = f6 * 3.0;

            break;
        }
        case RectPoint::RM:
        {
            fNewX = maWorkRange.getWidth() - ( fRight - maRange.getMaxX() );
            fNewY = std::min( maRange.getCenter().getY() - fTop, fBottom - maRange.getCenter().getY() ) * 2.0;
            break;
        }
        case RectPoint::LB:
        {
            fNewX = maWorkRange.getWidth() - ( maRange.getMinX() - fLeft );
            fNewY = maWorkRange.getHeight() - ( fBottom - maRange.getMaxY() );
            break;
        }
        case RectPoint::MB:
        {
            fNewX = std::min( maRange.getCenter().getX() - fLeft, fRight - maRange.getCenter().getX() ) * 2.0;
            fNewY = maWorkRange.getHeight() - ( maRange.getMaxY() - fBottom );
            break;
        }
        case RectPoint::RB:
        {
            fNewX = maWorkRange.getWidth() - ( fRight - maRange.getMaxX() );
            fNewY = maWorkRange.getHeight() - ( fBottom - maRange.getMaxY() );
            break;
        }
    }

    // #i75273# normalizing when setting the min/max values was wrong, removed
    m_xMtrWidth->set_max(basegfx::fround64(fNewX), FUNIT_NONE);
    m_xMtrHeight->set_max(basegfx::fround64(fNewY), FUNIT_NONE);
}

void SvxPositionSizeTabPage::GetTopLeftPosition(double& rfX, double& rfY, const basegfx::B2DRange& rRange)
{
    switch (m_aCtlPos.GetActualRP())
    {
        case RectPoint::LT:
        {
            break;
        }
        case RectPoint::MT:
        {
            rfX -= rRange.getCenter().getX() - rRange.getMinX();
            break;
        }
        case RectPoint::RT:
        {
            rfX -= rRange.getWidth();
            break;
        }
        case RectPoint::LM:
        {
            rfY -= rRange.getCenter().getY() - rRange.getMinY();
            break;
        }
        case RectPoint::MM:
        {
            rfX -= rRange.getCenter().getX() - rRange.getMinX();
            rfY -= rRange.getCenter().getY() - rRange.getMinY();
            break;
        }
        case RectPoint::RM:
        {
            rfX -= rRange.getWidth();
            rfY -= rRange.getCenter().getY() - rRange.getMinY();
            break;
        }
        case RectPoint::LB:
        {
            rfY -= rRange.getHeight();
            break;
        }
        case RectPoint::MB:
        {
            rfX -= rRange.getCenter().getX() - rRange.getMinX();
            rfY -= rRange.getHeight();
            break;
        }
        case RectPoint::RB:
        {
            rfX -= rRange.getWidth();
            rfY -= rRange.getHeight();
            break;
        }
    }
}

void SvxPositionSizeTabPage::PointChanged(vcl::Window* /*pWindow*/, RectPoint /*eRP*/)
{
    assert(false);
}

void SvxPositionSizeTabPage::PointChanged(weld::DrawingArea* pDrawingArea, RectPoint eRP)
{
    if (pDrawingArea == m_aCtlPos.GetDrawingArea())
    {
        SetMinMaxPosition();
        switch( eRP )
        {
            case RectPoint::LT:
            {
                m_xMtrPosX->set_value( basegfx::fround64(maRange.getMinX()), FUNIT_NONE );
                m_xMtrPosY->set_value( basegfx::fround64(maRange.getMinY()), FUNIT_NONE );
                break;
            }
            case RectPoint::MT:
            {
                m_xMtrPosX->set_value( basegfx::fround64(maRange.getCenter().getX()), FUNIT_NONE );
                m_xMtrPosY->set_value( basegfx::fround64(maRange.getMinY()), FUNIT_NONE );
                break;
            }
            case RectPoint::RT:
            {
                m_xMtrPosX->set_value( basegfx::fround64(maRange.getMaxX()), FUNIT_NONE );
                m_xMtrPosY->set_value( basegfx::fround64(maRange.getMinY()), FUNIT_NONE );
                break;
            }
            case RectPoint::LM:
            {
                m_xMtrPosX->set_value( basegfx::fround64(maRange.getMinX()), FUNIT_NONE );
                m_xMtrPosY->set_value( basegfx::fround64(maRange.getCenter().getY()), FUNIT_NONE );
                break;
            }
            case RectPoint::MM:
            {
                m_xMtrPosX->set_value( basegfx::fround64(maRange.getCenter().getX()), FUNIT_NONE );
                m_xMtrPosY->set_value( basegfx::fround64(maRange.getCenter().getY()), FUNIT_NONE );
                break;
            }
            case RectPoint::RM:
            {
                m_xMtrPosX->set_value( basegfx::fround64(maRange.getMaxX()), FUNIT_NONE );
                m_xMtrPosY->set_value( basegfx::fround64(maRange.getCenter().getY()), FUNIT_NONE );
                break;
            }
            case RectPoint::LB:
            {
                m_xMtrPosX->set_value( basegfx::fround64(maRange.getMinX()), FUNIT_NONE );
                m_xMtrPosY->set_value( basegfx::fround64(maRange.getMaxY()), FUNIT_NONE );
                break;
            }
            case RectPoint::MB:
            {
                m_xMtrPosX->set_value( basegfx::fround64(maRange.getCenter().getX()), FUNIT_NONE );
                m_xMtrPosY->set_value( basegfx::fround64(maRange.getMaxY()), FUNIT_NONE );
                break;
            }
            case RectPoint::RB:
            {
                m_xMtrPosX->set_value( basegfx::fround64(maRange.getMaxX()), FUNIT_NONE );
                m_xMtrPosY->set_value( basegfx::fround64(maRange.getMaxY()), FUNIT_NONE );
                break;
            }
        }
    }
    else
    {
        meRP = eRP;
        SetMinMaxPosition();
    }
}

void SvxPositionSizeTabPage::DisableResize()
{
    mbSizeDisabled = true;
}


void SvxPositionSizeTabPage::DisableProtect()
{
    mbProtectDisabled = true;
}


IMPL_LINK_NOARG(SvxPositionSizeTabPage, ChangeWidthHdl, weld::MetricSpinButton&, void)
{
    if( m_xCbxScale->get_active() && m_xCbxScale->get_sensitive() )
    {
        sal_Int64 nHeight(basegfx::fround64((mfOldHeight * static_cast<double>(m_xMtrWidth->get_value(FUNIT_NONE))) / mfOldWidth));
        int nMin, nMax;
        m_xMtrHeight->get_range(nMin, nMax, FUNIT_NONE);

        if (nHeight <= nMax)
        {
            m_xMtrHeight->set_value(nHeight, FUNIT_NONE);
        }
        else
        {
            nHeight = nMax;
            m_xMtrHeight->set_value(nHeight, FUNIT_NONE);

            const sal_Int64 nWidth(basegfx::fround64((mfOldWidth * static_cast<double>(nHeight)) / mfOldHeight));
            m_xMtrWidth->set_value(nWidth, FUNIT_NONE);
        }
    }
}

IMPL_LINK_NOARG(SvxPositionSizeTabPage, ChangeHeightHdl, weld::MetricSpinButton&, void)
{
    if( m_xCbxScale->get_active() && m_xCbxScale->get_sensitive() )
    {
        sal_Int64 nWidth(basegfx::fround64((mfOldWidth * static_cast<double>(m_xMtrHeight->get_value(FUNIT_NONE))) / mfOldHeight));
        int nMin, nMax;
        m_xMtrWidth->get_range(nMin, nMax, FUNIT_NONE);

        if (nWidth <= nMax)
        {
            m_xMtrWidth->set_value(nWidth, FUNIT_NONE);
        }
        else
        {
            nWidth = nMax;
            m_xMtrWidth->set_value(nWidth, FUNIT_NONE);

            const sal_Int64 nHeight(basegfx::fround64((mfOldHeight * static_cast<double>(nWidth)) / mfOldWidth));
            m_xMtrHeight->set_value(nHeight, FUNIT_NONE);
        }
    }
}

IMPL_LINK_NOARG(SvxPositionSizeTabPage, ClickSizeProtectHdl, weld::ToggleButton&, void)
{
    UpdateControlStates();
}

IMPL_LINK_NOARG(SvxPositionSizeTabPage, ClickAutoHdl, weld::ToggleButton&, void)
{
    if (m_xCbxScale->get_active())
    {
        mfOldWidth  = std::max( static_cast<double>(GetCoreValue( *m_xMtrWidth,  mePoolUnit )), 1.0 );
        mfOldHeight = std::max( static_cast<double>(GetCoreValue( *m_xMtrHeight, mePoolUnit )), 1.0 );
    }
}

void SvxPositionSizeTabPage::FillUserData()
{
    // matching is saved in the Ini-file
    OUString aStr = m_xCbxScale->get_active() ? OUString("1") : OUString("0");
    SetUserData( aStr );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
