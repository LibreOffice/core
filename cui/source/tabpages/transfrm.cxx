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
#include <svx/sderitm.hxx>
#include <svx/dialogs.hrc>
#include <svx/transfrmhelper.hxx>
#include <cuires.hrc>
#include <editeng/sizeitem.hxx>

#include "transfrm.hxx"
#include <dialmgr.hxx>
#include "svx/dlgutil.hxx"
#include <editeng/svxenum.hxx>
#include "svx/anchorid.hxx"
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

SvxTransformTabDialog::SvxTransformTabDialog( vcl::Window* pParent, const SfxItemSet* pAttr,
                                const SdrView* pSdrView, sal_uInt16 nAnchorTypes )
    : SfxTabDialog( pParent
                  ,"PositionAndSizeDialog"
                  ,"cui/ui/positionsizedialog.ui"
                  , pAttr )
    , nPosSize(0)
    , nSWPosSize(0)
    , pView(pSdrView)
    , nAnchorCtrls(nAnchorTypes)
{
    DBG_ASSERT(pView, "no valid view (!)");

    //different positioning page in Writer
    if(nAnchorCtrls & 0x00ff)
    {
        nSWPosSize = AddTabPage("RID_SVXPAGE_SWPOSSIZE", SvxSwPosSizeTabPage::Create, SvxSwPosSizeTabPage::GetRanges);
        RemoveTabPage("RID_SVXPAGE_POSITION_SIZE");
    }
    else
    {
        nPosSize = AddTabPage("RID_SVXPAGE_POSITION_SIZE", SvxPositionSizeTabPage::Create, SvxPositionSizeTabPage::GetRanges);
        RemoveTabPage("RID_SVXPAGE_SWPOSSIZE");
    }

    nRotation = AddTabPage("RID_SVXPAGE_ANGLE", SvxAngleTabPage::Create, SvxAngleTabPage::GetRanges);
    nSlant = AddTabPage("RID_SVXPAGE_SLANT", SvxSlantTabPage::Create, SvxSlantTabPage::GetRanges);
}


void SvxTransformTabDialog::PageCreated(sal_uInt16 nId, SfxTabPage &rPage)
{
    if (nId==nPosSize)
        {
            SvxPositionSizeTabPage& rSvxPos =  static_cast<SvxPositionSizeTabPage&>(rPage);
            rSvxPos.SetView(pView);
            rSvxPos.Construct();

            if(nAnchorCtrls & SVX_OBJ_NORESIZE)
            {
                rSvxPos.DisableResize();
            }

            if(nAnchorCtrls & SVX_OBJ_NOPROTECT)
            {
                rSvxPos.DisableProtect();
                rSvxPos.UpdateControlStates();
            }
        }
    else if (nId == nSWPosSize)
        {
            SvxSwPosSizeTabPage& rSwPos =  static_cast<SvxSwPosSizeTabPage&>(rPage);

            rSwPos.EnableAnchorTypes(nAnchorCtrls);
            rSwPos.SetValidateFramePosLink(aValidateLink);
            rSwPos.SetView(pView);
        }

    else if( nId == nRotation)
        {
            SvxAngleTabPage& rSvxAng =  static_cast<SvxAngleTabPage&>(rPage);

            rSvxAng.SetView( pView );
            rSvxAng.Construct();
        }

    else if (nId == nSlant)
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
SvxAngleTabPage::SvxAngleTabPage(vcl::Window* pParent, const SfxItemSet& rInAttrs)
    : SvxTabPage( pParent,"Rotation","cui/ui/rotationtabpage.ui", rInAttrs)
    , rOutAttrs(rInAttrs)
    , pView(nullptr)
    , eDlgUnit(FUNIT_NONE)
{
    get(m_pFlPosition, "FL_POSITION");
    get(m_pMtrPosX, "MTR_FLD_POS_X");
    get(m_pMtrPosY, "MTR_FLD_POS_Y");
    get(m_pCtlRect, "CTL_RECT");

    get(m_pFlAngle, "FL_ANGLE");
    get(m_pNfAngle, "NF_ANGLE");
    get(m_pCtlAngle, "CTL_ANGLE");

    // calculate PoolUnit
    SfxItemPool* pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "no pool (!)" );
    ePoolUnit = pPool->GetMetric(SID_ATTR_TRANSFORM_POS_X);

    m_pCtlAngle->SetLinkedField( m_pNfAngle, 2 );
}

SvxAngleTabPage::~SvxAngleTabPage()
{
    disposeOnce();
}

void SvxAngleTabPage::dispose()
{
    m_pFlPosition.clear();
    m_pMtrPosX.clear();
    m_pMtrPosY.clear();
    m_pCtlRect.clear();
    m_pFlAngle.clear();
    m_pNfAngle.clear();
    m_pCtlAngle.clear();
    SvxTabPage::dispose();
}

void SvxAngleTabPage::Construct()
{
    DBG_ASSERT(pView, "No valid view (!)");
    eDlgUnit = GetModuleFieldUnit(GetItemSet());
    SetFieldUnit(*m_pMtrPosX, eDlgUnit, true);
    SetFieldUnit(*m_pMtrPosY, eDlgUnit, true);

    if(FUNIT_MILE == eDlgUnit || FUNIT_KM == eDlgUnit)
    {
        m_pMtrPosX->SetDecimalDigits( 3 );
        m_pMtrPosY->SetDecimalDigits( 3 );
    }

    { // #i75273#
        Rectangle aTempRect(pView->GetAllMarkedRect());
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
    sal_uInt16 nDigits(m_pMtrPosX->GetDecimalDigits());
    TransfrmHelper::ConvertRect(maRange, nDigits, (MapUnit)ePoolUnit, eDlgUnit);

    if(!pView->IsRotateAllowed())
    {
        m_pFlPosition->Disable();
        m_pFlAngle->Disable();
    }
}

bool SvxAngleTabPage::FillItemSet(SfxItemSet* rSet)
{
    bool bModified = false;

    if(m_pCtlAngle->IsValueModified() || m_pMtrPosX->IsValueModified() || m_pMtrPosY->IsValueModified())
    {
        const double fUIScale(double(pView->GetModel()->GetUIScale()));
        const double fTmpX((GetCoreValue(*m_pMtrPosX, ePoolUnit) + maAnchor.getX()) * fUIScale);
        const double fTmpY((GetCoreValue(*m_pMtrPosY, ePoolUnit) + maAnchor.getY()) * fUIScale);

        rSet->Put(SfxInt32Item(GetWhich(SID_ATTR_TRANSFORM_ANGLE), m_pCtlAngle->GetRotation()));
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
        const double fTmp(((double)static_cast<const SfxInt32Item*>(pItem)->GetValue() - maAnchor.getX()) / fUIScale);
        SetMetricValue(*m_pMtrPosX, basegfx::fround(fTmp), ePoolUnit);
    }
    else
    {
        m_pMtrPosX->SetText( OUString() );
    }

    pItem = GetItem(*rAttrs, SID_ATTR_TRANSFORM_ROT_Y);
    if(pItem)
    {
        const double fTmp(((double)static_cast<const SfxInt32Item*>(pItem)->GetValue() - maAnchor.getY()) / fUIScale);
        SetMetricValue(*m_pMtrPosY, basegfx::fround(fTmp), ePoolUnit);
    }
    else
    {
        m_pMtrPosY->SetText( OUString() );
    }

    pItem = GetItem( *rAttrs, SID_ATTR_TRANSFORM_ANGLE );
    if(pItem)
    {
        m_pCtlAngle->SetRotation(static_cast<const SfxInt32Item*>(pItem)->GetValue());
    }
    else
    {
        m_pCtlAngle->SetRotation(0);
    }
    m_pCtlAngle->SaveValue();
}


VclPtr<SfxTabPage> SvxAngleTabPage::Create( vcl::Window* pWindow, const SfxItemSet* rSet)
{
    return VclPtr<SvxAngleTabPage>::Create(pWindow, *rSet);
}


void SvxAngleTabPage::ActivatePage(const SfxItemSet& /*rSet*/)
{
}


SfxTabPage::sfxpg SvxAngleTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if(_pSet)
    {
        FillItemSet(_pSet);
    }

    return LEAVE_PAGE;
}


void SvxAngleTabPage::PointChanged(vcl::Window* pWindow, RECT_POINT eRP)
{
    if(pWindow == m_pCtlRect)
    {
        switch(eRP)
        {
            case RP_LT:
            {
                m_pMtrPosX->SetUserValue( basegfx::fround64(maRange.getMinX()), FUNIT_NONE );
                m_pMtrPosY->SetUserValue( basegfx::fround64(maRange.getMinY()), FUNIT_NONE );
                break;
            }
            case RP_MT:
            {
                m_pMtrPosX->SetUserValue( basegfx::fround64(maRange.getCenter().getX()), FUNIT_NONE );
                m_pMtrPosY->SetUserValue( basegfx::fround64(maRange.getMinY()), FUNIT_NONE );
                break;
            }
            case RP_RT:
            {
                m_pMtrPosX->SetUserValue( basegfx::fround64(maRange.getMaxX()), FUNIT_NONE );
                m_pMtrPosY->SetUserValue( basegfx::fround64(maRange.getMinY()), FUNIT_NONE );
                break;
            }
            case RP_LM:
            {
                m_pMtrPosX->SetUserValue( basegfx::fround64(maRange.getMinX()), FUNIT_NONE );
                m_pMtrPosY->SetUserValue( basegfx::fround64(maRange.getCenter().getY()), FUNIT_NONE );
                break;
            }
            case RP_MM:
            {
                m_pMtrPosX->SetUserValue( basegfx::fround64(maRange.getCenter().getX()), FUNIT_NONE );
                m_pMtrPosY->SetUserValue( basegfx::fround64(maRange.getCenter().getY()), FUNIT_NONE );
                break;
            }
            case RP_RM:
            {
                m_pMtrPosX->SetUserValue( basegfx::fround64(maRange.getMaxX()), FUNIT_NONE );
                m_pMtrPosY->SetUserValue( basegfx::fround64(maRange.getCenter().getY()), FUNIT_NONE );
                break;
            }
            case RP_LB:
            {
                m_pMtrPosX->SetUserValue( basegfx::fround64(maRange.getMinX()), FUNIT_NONE );
                m_pMtrPosY->SetUserValue( basegfx::fround64(maRange.getMaxY()), FUNIT_NONE );
                break;
            }
            case RP_MB:
            {
                m_pMtrPosX->SetUserValue( basegfx::fround64(maRange.getCenter().getX()), FUNIT_NONE );
                m_pMtrPosY->SetUserValue( basegfx::fround64(maRange.getMaxY()), FUNIT_NONE );
                break;
            }
            case RP_RB:
            {
                m_pMtrPosX->SetUserValue( basegfx::fround64(maRange.getMaxX()), FUNIT_NONE );
                m_pMtrPosY->SetUserValue( basegfx::fround64(maRange.getMaxY()), FUNIT_NONE );
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
SvxSlantTabPage::SvxSlantTabPage(vcl::Window* pParent, const SfxItemSet& rInAttrs)
    : SvxTabPage( pParent,"SlantAndCornerRadius","cui/ui/slantcornertabpage.ui",
        rInAttrs)
    , rOutAttrs(rInAttrs)
    , pView(nullptr)
    , eDlgUnit(FUNIT_NONE)
{
    get(m_pFlRadius, "FL_RADIUS");
    get(m_pMtrRadius, "MTR_FLD_RADIUS");
    get(m_pFlAngle, "FL_SLANT");
    get(m_pMtrAngle, "MTR_FLD_ANGLE");

    for (int i = 0; i < 2; ++i)
    {
        get(m_aControlGroups[i], "controlgroups" + OString::number(i+1));
        get(m_aControlGroupX[i], "controlgroupx" + OString::number(i+1));
        get(m_aControlX[i], "controlx" + OString::number(i+1));
        get(m_aControlGroupY[i], "controlgroupy" + OString::number(i+1));
        get(m_aControlY[i], "controly" + OString::number(i+1));
    }

    // this page needs ExchangeSupport
    SetExchangeSupport();

    // evaluate PoolUnit
    SfxItemPool* pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "no pool (!)" );
    ePoolUnit = pPool->GetMetric( SID_ATTR_TRANSFORM_POS_X );
}

SvxSlantTabPage::~SvxSlantTabPage()
{
    disposeOnce();
}

void SvxSlantTabPage::dispose()
{
    m_pFlRadius.clear();
    m_pMtrRadius.clear();
    m_pFlAngle.clear();
    m_pMtrAngle.clear();
    for (int i = 0; i < 2; ++i)
    {
        m_aControlGroups[i].clear();
        m_aControlGroupX[i].clear();
        m_aControlX[i].clear();
        m_aControlGroupY[i].clear();
        m_aControlY[i].clear();
    }
    SvxTabPage::dispose();
}

void SvxSlantTabPage::Construct()
{
    // get the range
    DBG_ASSERT(pView, "no valid view (!)");
    eDlgUnit = GetModuleFieldUnit(GetItemSet());
    SetFieldUnit(*m_pMtrRadius, eDlgUnit, true);

    { // #i75273#
        Rectangle aTempRect(pView->GetAllMarkedRect());
        pView->GetSdrPageView()->LogicToPagePos(aTempRect);
        maRange = basegfx::B2DRange(aTempRect.Left(), aTempRect.Top(), aTempRect.Right(), aTempRect.Bottom());
    }
}


bool SvxSlantTabPage::FillItemSet(SfxItemSet* rAttrs)
{
    bool  bModified = false;
    OUString aStr = m_pMtrRadius->GetText();

    if( m_pMtrRadius->IsValueChangedFromSaved() )
    {
        Fraction aUIScale = pView->GetModel()->GetUIScale();
        long nTmp = GetCoreValue( *m_pMtrRadius, ePoolUnit );
        nTmp = Fraction( nTmp ) * aUIScale;

        rAttrs->Put( makeSdrEckenradiusItem( nTmp ) );
        bModified = true;
    }

    aStr = m_pMtrAngle->GetText();

    if( m_pMtrAngle->IsValueChangedFromSaved() )
    {
        sal_Int32 nValue = static_cast<sal_Int32>(m_pMtrAngle->GetValue());
        rAttrs->Put( SfxInt32Item( SID_ATTR_TRANSFORM_SHEAR, nValue ) );
        bModified = true;
    }

    if( bModified )
    {
        // set reference points
        Rectangle aObjectRect(pView->GetAllMarkedRect());
        pView->GetSdrPageView()->LogicToPagePos(aObjectRect);
        Point aPt = aObjectRect.Center();

        rAttrs->Put(SfxInt32Item(SID_ATTR_TRANSFORM_SHEAR_X, aPt.X()));
        rAttrs->Put(SfxInt32Item(SID_ATTR_TRANSFORM_SHEAR_Y, aPt.Y()));
        rAttrs->Put( SfxBoolItem( SID_ATTR_TRANSFORM_SHEAR_VERTICAL, false ) );
    }

    bool bControlPointsChanged = false;
    for (int i = 0; i < 2; ++i)
    {
        bControlPointsChanged |= (m_aControlX[i]->IsValueChangedFromSaved() ||
                                  m_aControlY[i]->IsValueChangedFromSaved());
    }

    if (!bControlPointsChanged)
        return bModified;

    SdrObject* pObj = pView->GetMarkedObjectList().GetMark(0)->GetMarkedSdrObj();
    SdrModel* pModel = pObj->GetModel();
    SdrUndoAction* pUndo = pModel->IsUndoEnabled() ?
                pModel->GetSdrUndoFactory().CreateUndoAttrObject(*pObj) :
                nullptr;

    if (pUndo)
        pModel->BegUndo(pUndo->GetComment());

    EnhancedCustomShape2d aShape(pObj);
    Rectangle aLogicRect = aShape.GetLogicRect();

    for (int i = 0; i < 2; ++i)
    {
        if (m_aControlX[i]->IsValueChangedFromSaved() || m_aControlY[i]->IsValueChangedFromSaved())
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

    pObj->SetChanged();
    pObj->BroadcastObjectChange();
    bModified = true;

    if (pUndo)
    {
        pModel->AddUndo(pUndo);
        pModel->EndUndo();
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
        m_pMtrRadius->SetText( "" );
        m_pFlRadius->Disable();
    }
    else
    {
        pItem = GetItem( *rAttrs, SDRATTR_ECKENRADIUS );

        if( pItem )
        {
            const double fUIScale(double(pView->GetModel()->GetUIScale()));
            const double fTmp((double)static_cast<const SdrMetricItem*>(pItem)->GetValue() / fUIScale);
            SetMetricValue(*m_pMtrRadius, basegfx::fround(fTmp), ePoolUnit);
        }
        else
        {
            m_pMtrRadius->SetText( "" );
        }
    }

    m_pMtrRadius->SaveValue();

    // slant: angle
    if( !pView->IsShearAllowed() )
    {
        m_pMtrAngle->SetText( "" );
        m_pFlAngle->Disable();
    }
    else
    {
        pItem = GetItem( *rAttrs, SID_ATTR_TRANSFORM_SHEAR );

        if( pItem )
        {
            m_pMtrAngle->SetValue( static_cast<const SfxInt32Item*>(pItem)->GetValue() );
        }
        else
        {
            m_pMtrAngle->SetText( "" );
        }
    }

    m_pMtrAngle->SaveValue();

    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    if (rMarkList.GetMarkCount() == 1)
    {
        SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        SdrObjKind eKind = (SdrObjKind) pObj->GetObjIdentifier();
        if (eKind == OBJ_CUSTOMSHAPE)
        {
            //save geometry
            SdrCustomShapeGeometryItem aInitialGeometry =
                static_cast<const SdrCustomShapeGeometryItem&>(pObj->GetMergedItem(SDRATTR_CUSTOMSHAPE_GEOMETRY));

            EnhancedCustomShape2d aShape(pObj);

            for (int i = 0; i < 2; ++i)
            {
                Point aInitialPosition;
                if (!aShape.GetHandlePosition(i, aInitialPosition))
                    break;
                m_aControlGroups[i]->Enable();
                css::awt::Point aPosition;

                aPosition.X = SAL_MAX_INT32;
                aPosition.Y = SAL_MAX_INT32;
                aShape.SetHandleControllerPosition(i, aPosition);
                Point aMaxPosition;
                aShape.GetHandlePosition(i, aMaxPosition);

                aPosition.X = SAL_MIN_INT32;
                aPosition.Y = SAL_MIN_INT32;
                aShape.SetHandleControllerPosition(i, aPosition);
                Point aMinPosition;
                aShape.GetHandlePosition(i, aMinPosition);

                Rectangle aLogicRect = aShape.GetLogicRect();
                aInitialPosition.Move(-aLogicRect.Left(), -aLogicRect.Top());
                aMaxPosition.Move(-aLogicRect.Left(), -aLogicRect.Top());
                aMinPosition.Move(-aLogicRect.Left(), -aLogicRect.Top());

                SetMetricValue(*m_aControlX[i], aInitialPosition.X(), ePoolUnit);
                SetMetricValue(*m_aControlY[i], aInitialPosition.Y(), ePoolUnit);

                if (aMaxPosition.X() == aMinPosition.X())
                    m_aControlGroupX[i]->Disable();
                else
                {
                    m_aControlX[i]->SetMin(aMinPosition.X(), FUNIT_MM);
                    m_aControlX[i]->SetMax(aMaxPosition.X(), FUNIT_MM);
                }
                if (aMaxPosition.Y() == aMinPosition.Y())
                    m_aControlGroupY[i]->Disable();
                else
                {
                    m_aControlY[i]->SetMin(aMinPosition.Y(), FUNIT_MM);
                    m_aControlY[i]->SetMax(aMaxPosition.Y(), FUNIT_MM);
                }
            }

            //restore geometry
            pObj->SetMergedItem(aInitialGeometry);
        }
    }
    for (int i = 0; i < 2; ++i)
    {
        m_aControlX[i]->SaveValue();
        m_aControlY[i]->SaveValue();
    }
}

VclPtr<SfxTabPage> SvxSlantTabPage::Create( vcl::Window* pWindow, const SfxItemSet* rOutAttrs )
{
    return VclPtr<SvxSlantTabPage>::Create( pWindow, *rOutAttrs );
}

void SvxSlantTabPage::ActivatePage( const SfxItemSet& rSet )
{
    SfxRectangleItem const * pRectItem = nullptr;

    if( SfxItemState::SET == rSet.GetItemState( GetWhich( SID_ATTR_TRANSFORM_INTERN ) , false, reinterpret_cast<SfxPoolItem const **>(&pRectItem) ) )
    {
        const Rectangle aTempRect(pRectItem->GetValue());
        maRange = basegfx::B2DRange(aTempRect.Left(), aTempRect.Top(), aTempRect.Right(), aTempRect.Bottom());
    }
}


SfxTabPage::sfxpg SvxSlantTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if(_pSet)
    {
        FillItemSet(_pSet);
    }

    return LEAVE_PAGE;
}


void SvxSlantTabPage::PointChanged( vcl::Window* , RECT_POINT  )
{
}

/*************************************************************************
|*
|*      Dialog for changing position and size of graphic objects
|*
\************************************************************************/
SvxPositionSizeTabPage::SvxPositionSizeTabPage(vcl::Window* pParent, const SfxItemSet& rInAttrs)
    : SvxTabPage(pParent,"PositionAndSize","cui/ui/possizetabpage.ui", rInAttrs)
    , mrOutAttrs(rInAttrs)
    , mpView(nullptr)
    , meDlgUnit(FUNIT_NONE)
    , mnProtectSizeState(TRISTATE_FALSE)
    , mbPageDisabled(false)
    , mbProtectDisabled(false)
    , mbSizeDisabled(false)
    , mbAdjustDisabled(true)
    , mfOldWidth(0.0)
    , mfOldHeight(0.0)
{
    get(m_pFlPosition, "FL_POSITION");
    get(m_pMtrPosX, "MTR_FLD_POS_X");
    get(m_pMtrPosY, "MTR_FLD_POS_Y");
    get(m_pCtlPos, "CTL_POSRECT" );

    get(m_pFlSize, "FL_SIZE");
    get(m_pFtWidth, "FT_WIDTH");
    get(m_pMtrWidth, "MTR_FLD_WIDTH");
    get(m_pFtHeight, "FT_HEIGHT");
    get(m_pMtrHeight, "MTR_FLD_HEIGHT");
    get(m_pCbxScale, "CBX_SCALE");
    get(m_pCtlSize, "CTL_SIZERECT" );

    get(m_pFlProtect, "FL_PROTECT");
    get(m_pTsbPosProtect, "TSB_POSPROTECT");
    get(m_pTsbSizeProtect, "TSB_SIZEPROTECT");

    get(m_pFlAdjust, "FL_ADJUST");
    get(m_pTsbAutoGrowWidth, "TSB_AUTOGROW_WIDTH");
    get(m_pTsbAutoGrowHeight, "TSB_AUTOGROW_HEIGHT");

    // this page needs ExchangeSupport
    SetExchangeSupport();

    // evaluate PoolUnit
    SfxItemPool* pPool = mrOutAttrs.GetPool();
    DBG_ASSERT( pPool, "no pool (!)" );
    mePoolUnit = pPool->GetMetric( SID_ATTR_TRANSFORM_POS_X );

    m_pCtlPos->SetActualRP(RP_LT);
    m_pCtlSize->SetActualRP(RP_LT);
    meRP = RP_LT; // see above

    m_pMtrWidth->SetModifyHdl( LINK( this, SvxPositionSizeTabPage, ChangeWidthHdl ) );
    m_pMtrHeight->SetModifyHdl( LINK( this, SvxPositionSizeTabPage, ChangeHeightHdl ) );
    m_pCbxScale->SetClickHdl( LINK( this, SvxPositionSizeTabPage, ClickAutoHdl ) );

    m_pTsbAutoGrowWidth->Disable();
    m_pTsbAutoGrowHeight->Disable();
    m_pFlAdjust->Disable();

    // #i2379# disable controls when protected
    m_pTsbPosProtect->SetClickHdl( LINK( this, SvxPositionSizeTabPage, ChangePosProtectHdl ) );
    m_pTsbSizeProtect->SetClickHdl( LINK( this, SvxPositionSizeTabPage, ChangeSizeProtectHdl ) );

}

SvxPositionSizeTabPage::~SvxPositionSizeTabPage()
{
    disposeOnce();
}

void SvxPositionSizeTabPage::dispose()
{
    m_pFlPosition.clear();
    m_pMtrPosX.clear();
    m_pMtrPosY.clear();
    m_pCtlPos.clear();
    m_pFlSize.clear();
    m_pFtWidth.clear();
    m_pMtrWidth.clear();
    m_pFtHeight.clear();
    m_pMtrHeight.clear();
    m_pCbxScale.clear();
    m_pCtlSize.clear();
    m_pFlProtect.clear();
    m_pTsbPosProtect.clear();
    m_pTsbSizeProtect.clear();
    m_pFlAdjust.clear();
    m_pTsbAutoGrowWidth.clear();
    m_pTsbAutoGrowHeight.clear();
    SvxTabPage::dispose();
}

void SvxPositionSizeTabPage::Construct()
{
    // get range and work area
    DBG_ASSERT( mpView, "no valid view (!)" );
    meDlgUnit = GetModuleFieldUnit( GetItemSet() );
    SetFieldUnit( *m_pMtrPosX, meDlgUnit, true );
    SetFieldUnit( *m_pMtrPosY, meDlgUnit, true );
    SetFieldUnit( *m_pMtrWidth, meDlgUnit, true );
    SetFieldUnit( *m_pMtrHeight, meDlgUnit, true );

    if(FUNIT_MILE == meDlgUnit || FUNIT_KM == meDlgUnit)
    {
        m_pMtrPosX->SetDecimalDigits( 3 );
        m_pMtrPosY->SetDecimalDigits( 3 );
        m_pMtrWidth->SetDecimalDigits( 3 );
        m_pMtrHeight->SetDecimalDigits( 3 );
    }

    { // #i75273#
        Rectangle aTempRect(mpView->GetAllMarkedRect());
        mpView->GetSdrPageView()->LogicToPagePos(aTempRect);
        maRange = basegfx::B2DRange(aTempRect.Left(), aTempRect.Top(), aTempRect.Right(), aTempRect.Bottom());
    }

    { // #i75273#
        Rectangle aTempRect(mpView->GetWorkArea());
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
                    m_pMtrPosX->SetText( "" );
                    m_pMtrPosY->SetText( "" );
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
        const SdrObjKind eKind((SdrObjKind)pObj->GetObjIdentifier());

        if((pObj->GetObjInventor() == SdrInventor) &&
            (OBJ_TEXT == eKind || OBJ_TITLETEXT == eKind || OBJ_OUTLINETEXT == eKind) &&
            pObj->HasText())
        {
            mbAdjustDisabled = false;

            m_pFlAdjust->Enable();

            m_pTsbAutoGrowWidth->SetClickHdl( LINK( this, SvxPositionSizeTabPage, ClickSizeProtectHdl ) );
            m_pTsbAutoGrowHeight->SetClickHdl( LINK( this, SvxPositionSizeTabPage, ClickSizeProtectHdl ) );

            // is used as flag to evaluate if its selectable
            m_pTsbAutoGrowWidth->EnableTriState( false );
            m_pTsbAutoGrowHeight->EnableTriState( false );
        }
    }

    // take scale into account
    const Fraction aUIScale(mpView->GetModel()->GetUIScale());
    TransfrmHelper::ScaleRect( maWorkRange, aUIScale );
    TransfrmHelper::ScaleRect( maRange, aUIScale );

    // take UI units into account
    const sal_uInt16 nDigits(m_pMtrPosX->GetDecimalDigits());
    TransfrmHelper::ConvertRect( maWorkRange, nDigits, (MapUnit) mePoolUnit, meDlgUnit );
    TransfrmHelper::ConvertRect( maRange, nDigits, (MapUnit) mePoolUnit, meDlgUnit );

    SetMinMaxPosition();
}


bool SvxPositionSizeTabPage::FillItemSet( SfxItemSet* rOutAttrs )
{
    bool bModified(false);

    if ( m_pMtrWidth->HasFocus() )
    {
        ChangeWidthHdl( *m_pMtrWidth );
    }

    if ( m_pMtrHeight->HasFocus() )
    {
        ChangeHeightHdl( *m_pMtrHeight );
    }

    if( !mbPageDisabled )
    {
        if ( m_pMtrPosX->IsValueModified() || m_pMtrPosY->IsValueModified() )
        {
            const double fUIScale(double(mpView->GetModel()->GetUIScale()));
            double fX((GetCoreValue( *m_pMtrPosX, mePoolUnit ) + maAnchor.getX()) * fUIScale);
            double fY((GetCoreValue( *m_pMtrPosY, mePoolUnit ) + maAnchor.getY()) * fUIScale);

            { // #i75273#
                Rectangle aTempRect(mpView->GetAllMarkedRect());
                mpView->GetSdrPageView()->LogicToPagePos(aTempRect);
                maRange = basegfx::B2DRange(aTempRect.Left(), aTempRect.Top(), aTempRect.Right(), aTempRect.Bottom());
            }

            // #101581# GetTopLeftPosition(...) needs coordinates after UI scaling, in real PagePositions
            GetTopLeftPosition(fX, fY, maRange);

            rOutAttrs->Put(SfxInt32Item(GetWhich(SID_ATTR_TRANSFORM_POS_X), basegfx::fround(fX)));
            rOutAttrs->Put(SfxInt32Item(GetWhich(SID_ATTR_TRANSFORM_POS_Y), basegfx::fround(fY)));

            bModified = true;
        }

        if ( m_pTsbPosProtect->IsValueChangedFromSaved() )
        {
            if( m_pTsbPosProtect->GetState() == TRISTATE_INDET )
            {
                rOutAttrs->InvalidateItem( SID_ATTR_TRANSFORM_PROTECT_POS );
            }
            else
            {
                rOutAttrs->Put(
                    SfxBoolItem( GetWhich( SID_ATTR_TRANSFORM_PROTECT_POS ),
                    m_pTsbPosProtect->GetState() == TRISTATE_TRUE ) );
            }

            bModified = true;
        }
    }

    if ( m_pMtrWidth->IsValueModified() || m_pMtrHeight->IsValueModified() )
    {
        Fraction aUIScale = mpView->GetModel()->GetUIScale();

        // get Width
        double nWidth = static_cast<double>(m_pMtrWidth->GetValue( meDlgUnit ));
        nWidth = MetricField::ConvertDoubleValue( nWidth, m_pMtrWidth->GetBaseValue(), m_pMtrWidth->GetDecimalDigits(), meDlgUnit, FUNIT_100TH_MM );
        long lWidth = long(nWidth * (double)aUIScale);
        lWidth = OutputDevice::LogicToLogic( lWidth, MAP_100TH_MM, (MapUnit)mePoolUnit );
        lWidth = static_cast<long>(m_pMtrWidth->Denormalize( lWidth ));

        // get Height
        double nHeight = static_cast<double>(m_pMtrHeight->GetValue( meDlgUnit ));
        nHeight = MetricField::ConvertDoubleValue( nHeight, m_pMtrHeight->GetBaseValue(), m_pMtrHeight->GetDecimalDigits(), meDlgUnit, FUNIT_100TH_MM );
        long lHeight = long(nHeight * (double)aUIScale);
        lHeight = OutputDevice::LogicToLogic( lHeight, MAP_100TH_MM, (MapUnit)mePoolUnit );
        lHeight = static_cast<long>(m_pMtrHeight->Denormalize( lHeight ));

        // put Width & Height to itemset
        rOutAttrs->Put( SfxUInt32Item( GetWhich( SID_ATTR_TRANSFORM_WIDTH ), (sal_uInt32) lWidth ) );
        rOutAttrs->Put( SfxUInt32Item( GetWhich( SID_ATTR_TRANSFORM_HEIGHT ), (sal_uInt32) lHeight ) );
        rOutAttrs->Put( SfxAllEnumItem( GetWhich( SID_ATTR_TRANSFORM_SIZE_POINT ), sal::static_int_cast< sal_uInt16 >( meRP ) ) );
        bModified = true;
    }

    if ( m_pTsbSizeProtect->IsValueChangedFromSaved() )
    {
        if ( m_pTsbSizeProtect->GetState() == TRISTATE_INDET )
            rOutAttrs->InvalidateItem( SID_ATTR_TRANSFORM_PROTECT_SIZE );
        else
            rOutAttrs->Put(
                SfxBoolItem( GetWhich( SID_ATTR_TRANSFORM_PROTECT_SIZE ),
                m_pTsbSizeProtect->GetState() == TRISTATE_TRUE ) );
        bModified = true;
    }

    if ( m_pTsbAutoGrowWidth->IsValueChangedFromSaved() )
    {
        if ( !m_pTsbAutoGrowWidth->IsTriStateEnabled() )
        {
            if( m_pTsbAutoGrowWidth->GetState() == TRISTATE_INDET )
                rOutAttrs->InvalidateItem( SID_ATTR_TRANSFORM_AUTOWIDTH );
            else
                rOutAttrs->Put(
                    SfxBoolItem( GetWhich( SID_ATTR_TRANSFORM_AUTOWIDTH ),
                    m_pTsbAutoGrowWidth->GetState() == TRISTATE_TRUE ) );
        }
        bModified = true;
    }

    if ( m_pTsbAutoGrowHeight->IsValueChangedFromSaved() )
    {
        if ( !m_pTsbAutoGrowHeight->IsTriStateEnabled() )
        {
            if( m_pTsbAutoGrowHeight->GetState() == TRISTATE_INDET )
                rOutAttrs->InvalidateItem( SID_ATTR_TRANSFORM_AUTOHEIGHT );
            else
                rOutAttrs->Put(
                    SfxBoolItem( GetWhich( SID_ATTR_TRANSFORM_AUTOHEIGHT ),
                    m_pTsbAutoGrowHeight->GetState() == TRISTATE_TRUE ) );
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
            SetMetricValue(*m_pMtrPosX, basegfx::fround(fTmp), mePoolUnit);
        }

        pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_POS_Y );
        if ( pItem )
        {
            const double fTmp((static_cast<const SfxInt32Item*>(pItem)->GetValue() - maAnchor.getY()) / fUIScale);
            SetMetricValue(*m_pMtrPosY, basegfx::fround(fTmp), mePoolUnit);
        }

        pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_PROTECT_POS );
        if ( pItem )
        {
            bool bProtected = static_cast<const SfxBoolItem*>( pItem )->GetValue();
            m_pTsbPosProtect->SetState( bProtected ? TRISTATE_TRUE : TRISTATE_FALSE );
            m_pTsbPosProtect->EnableTriState( false );
        }
        else
        {
            m_pTsbPosProtect->SetState( TRISTATE_INDET );
        }

        m_pTsbPosProtect->SaveValue();
        m_pCtlPos->Reset();

        // #i2379# Disable controls for protected objects
        ChangePosProtectHdl( nullptr );
    }

    { // #i75273# set width
        pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_WIDTH );
        mfOldWidth = std::max( pItem ? (double)static_cast<const SfxUInt32Item*>(pItem)->GetValue() : 0.0, 1.0 );
        double fTmpWidth((OutputDevice::LogicToLogic(static_cast<sal_Int32>(mfOldWidth), (MapUnit)mePoolUnit, MAP_100TH_MM)) / fUIScale);

        if(m_pMtrWidth->GetDecimalDigits())
            fTmpWidth *= pow(10.0, m_pMtrWidth->GetDecimalDigits());

        fTmpWidth = MetricField::ConvertDoubleValue(fTmpWidth, m_pMtrWidth->GetBaseValue(), m_pMtrWidth->GetDecimalDigits(), FUNIT_100TH_MM, meDlgUnit);
        m_pMtrWidth->SetValue(static_cast<sal_Int64>(fTmpWidth), meDlgUnit);
    }

    { // #i75273# set height
        pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_HEIGHT );
        mfOldHeight = std::max( pItem ? (double)static_cast<const SfxUInt32Item*>(pItem)->GetValue() : 0.0, 1.0 );
        double fTmpHeight((OutputDevice::LogicToLogic(static_cast<sal_Int32>(mfOldHeight), (MapUnit)mePoolUnit, MAP_100TH_MM)) / fUIScale);

        if(m_pMtrHeight->GetDecimalDigits())
            fTmpHeight *= pow(10.0, m_pMtrHeight->GetDecimalDigits());

        fTmpHeight = MetricField::ConvertDoubleValue(fTmpHeight, m_pMtrHeight->GetBaseValue(), m_pMtrHeight->GetDecimalDigits(), FUNIT_100TH_MM, meDlgUnit);
        m_pMtrHeight->SetValue(static_cast<sal_Int64>(fTmpHeight), meDlgUnit);
    }

    pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_PROTECT_SIZE );
    if ( pItem )
    {
        m_pTsbSizeProtect->SetState( static_cast<const SfxBoolItem*>(pItem)->GetValue()
                              ? TRISTATE_TRUE : TRISTATE_FALSE );
        m_pTsbSizeProtect->EnableTriState( false );
    }
    else
        m_pTsbSizeProtect->SetState( TRISTATE_INDET );

    pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_AUTOWIDTH );
    if ( pItem )
    {
        m_pTsbAutoGrowWidth->SetState( static_cast<const SfxBoolItem*>( pItem )->GetValue()
                           ? TRISTATE_TRUE : TRISTATE_FALSE );
    }
    else
        m_pTsbAutoGrowWidth->SetState( TRISTATE_INDET );

    pItem = GetItem( mrOutAttrs, SID_ATTR_TRANSFORM_AUTOHEIGHT );
    if ( pItem )
    {
        m_pTsbAutoGrowHeight->SetState( static_cast<const SfxBoolItem*>( pItem )->GetValue()
                           ? TRISTATE_TRUE : TRISTATE_FALSE );
    }
    else
        m_pTsbAutoGrowHeight->SetState( TRISTATE_INDET );

    // Is matching set?
    OUString aStr = GetUserData();
    m_pCbxScale->Check( aStr.toInt32() != 0 );

    m_pTsbSizeProtect->SaveValue();
    m_pTsbAutoGrowWidth->SaveValue();
    m_pTsbAutoGrowHeight->SaveValue();
    ClickSizeProtectHdl( nullptr );

    // #i2379# Disable controls for protected objects
    ChangeSizeProtectHdl( nullptr );
}


VclPtr<SfxTabPage> SvxPositionSizeTabPage::Create( vcl::Window* pWindow, const SfxItemSet* rOutAttrs )
{
    return VclPtr<SvxPositionSizeTabPage>::Create( pWindow, *rOutAttrs );
}


void SvxPositionSizeTabPage::ActivatePage( const SfxItemSet& rSet )
{
    SfxRectangleItem const * pRectItem = nullptr;

    if( SfxItemState::SET == rSet.GetItemState( GetWhich( SID_ATTR_TRANSFORM_INTERN ) , false, reinterpret_cast<SfxPoolItem const **>(&pRectItem) ) )
    {
        { // #i75273#
            const Rectangle aTempRect(pRectItem->GetValue());
            maRange = basegfx::B2DRange(aTempRect.Left(), aTempRect.Top(), aTempRect.Right(), aTempRect.Bottom());
        }

        SetMinMaxPosition();
    }
}


SfxTabPage::sfxpg SvxPositionSizeTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( _pSet )
    {
        double fX((double)m_pMtrPosX->GetValue());
        double fY((double)m_pMtrPosY->GetValue());

        GetTopLeftPosition(fX, fY, maRange);
        const Rectangle aOutRectangle(
            basegfx::fround(fX), basegfx::fround(fY),
            basegfx::fround(fX + maRange.getWidth()), basegfx::fround(fY + maRange.getHeight()));
        _pSet->Put(SfxRectangleItem(SID_ATTR_TRANSFORM_INTERN, aOutRectangle));

        FillItemSet(_pSet);
    }

    return LEAVE_PAGE;
}


IMPL_LINK_NOARG_TYPED(SvxPositionSizeTabPage, ChangePosProtectHdl, Button*, void)
{
    // #106572# Remember user's last choice
    m_pTsbSizeProtect->SetState( m_pTsbPosProtect->GetState() == TRISTATE_TRUE ?  TRISTATE_TRUE : mnProtectSizeState );
    UpdateControlStates();
}


void SvxPositionSizeTabPage::UpdateControlStates()
{
    const bool bPosProtect =  m_pTsbPosProtect->GetState() == TRISTATE_TRUE;
    const bool bSizeProtect = m_pTsbSizeProtect->GetState() == TRISTATE_TRUE;
    const bool bHeightChecked = !m_pTsbAutoGrowHeight->IsTriStateEnabled() && (m_pTsbAutoGrowHeight->GetState() == TRISTATE_TRUE);
    const bool bWidthChecked = !m_pTsbAutoGrowWidth->IsTriStateEnabled() && (m_pTsbAutoGrowWidth->GetState() == TRISTATE_TRUE);

    m_pFlPosition->Enable( !bPosProtect && !mbPageDisabled );

    m_pTsbPosProtect->Enable( !mbProtectDisabled && !mbPageDisabled );

    m_pFlSize->Enable( !mbSizeDisabled && !bSizeProtect );

    m_pFtWidth->Enable( !mbSizeDisabled && !bSizeProtect && !bWidthChecked );
    m_pMtrWidth->Enable( !mbSizeDisabled && !bSizeProtect && !bWidthChecked );

    m_pFtHeight->Enable( !mbSizeDisabled && !bSizeProtect && !bHeightChecked );
    m_pMtrHeight->Enable( !mbSizeDisabled && !bSizeProtect && !bHeightChecked );

    m_pCbxScale->Enable( !mbSizeDisabled && !bSizeProtect && !bHeightChecked && !bWidthChecked );
    m_pCtlSize->Enable( !mbSizeDisabled && !bSizeProtect && (!bHeightChecked || !bWidthChecked) );

    m_pFlProtect->Enable( !mbProtectDisabled );
    m_pTsbSizeProtect->Enable( !mbProtectDisabled && !bPosProtect );

    m_pFlAdjust->Enable( !mbSizeDisabled && !bSizeProtect && !mbAdjustDisabled );

    m_pCtlSize->Invalidate();
    m_pCtlPos->Invalidate();

}


IMPL_LINK_NOARG_TYPED(SvxPositionSizeTabPage, ChangeSizeProtectHdl, Button*, void)
{
    if( m_pTsbSizeProtect->IsEnabled() )
    {
        // #106572# Remember user's last choice

        // Note: this works only as long as the dialog is open.  When
        // the user closes the dialog, there is no way to remember
        // whether size was enabled or disabled before pos protect was
        // clicked. Thus, if pos protect is selected, the dialog is
        // closed and reopened again, unchecking pos protect will
        // always uncheck size protect, too. That's life.
        mnProtectSizeState = m_pTsbSizeProtect->GetState();
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

    switch ( m_pCtlPos->GetActualRP() )
    {
        case RP_LT:
        {
            fRight  -= maRange.getWidth();
            fBottom -= maRange.getHeight();
            break;
        }
        case RP_MT:
        {
            fLeft   += maRange.getWidth() / 2.0;
            fRight  -= maRange.getWidth() / 2.0;
            fBottom -= maRange.getHeight();
            break;
        }
        case RP_RT:
        {
            fLeft   += maRange.getWidth();
            fBottom -= maRange.getHeight();
            break;
        }
        case RP_LM:
        {
            fRight  -= maRange.getWidth();
            fTop    += maRange.getHeight() / 2.0;
            fBottom -= maRange.getHeight() / 2.0;
            break;
        }
        case RP_MM:
        {
            fLeft   += maRange.getWidth() / 2.0;
            fRight  -= maRange.getWidth() / 2.0;
            fTop    += maRange.getHeight() / 2.0;
            fBottom -= maRange.getHeight() / 2.0;
            break;
        }
        case RP_RM:
        {
            fLeft   += maRange.getWidth();
            fTop    += maRange.getHeight() / 2.0;
            fBottom -= maRange.getHeight() / 2.0;
            break;
        }
        case RP_LB:
        {
            fRight  -= maRange.getWidth();
            fTop    += maRange.getHeight();
            break;
        }
        case RP_MB:
        {
            fLeft   += maRange.getWidth() / 2.0;
            fRight  -= maRange.getWidth() / 2.0;
            fTop    += maRange.getHeight();
            break;
        }
        case RP_RB:
        {
            fLeft   += maRange.getWidth();
            fTop    += maRange.getHeight();
            break;
        }
    }

    const double fMaxLong((double)(MetricField::ConvertValue( LONG_MAX, 0, MAP_100TH_MM, meDlgUnit ) - 1L));
    fLeft = basegfx::clamp(fLeft, -fMaxLong, fMaxLong);
    fRight = basegfx::clamp(fRight, -fMaxLong, fMaxLong);
    fTop = basegfx::clamp(fTop, - fMaxLong, fMaxLong);
    fBottom = basegfx::clamp(fBottom, -fMaxLong, fMaxLong);

    // #i75273# normalizing when setting the min/max values was wrong, removed
    m_pMtrPosX->SetMin(basegfx::fround64(fLeft));
    m_pMtrPosX->SetFirst(basegfx::fround64(fLeft));
    m_pMtrPosX->SetMax(basegfx::fround64(fRight));
    m_pMtrPosX->SetLast(basegfx::fround64(fRight));
    m_pMtrPosY->SetMin(basegfx::fround64(fTop));
    m_pMtrPosY->SetFirst(basegfx::fround64(fTop));
    m_pMtrPosY->SetMax(basegfx::fround64(fBottom));
    m_pMtrPosY->SetLast(basegfx::fround64(fBottom));

    // size
    fLeft = maWorkRange.getMinX();
    fTop = maWorkRange.getMinY();
    fRight = maWorkRange.getMaxX();
    fBottom = maWorkRange.getMaxY();
    double fNewX(0);
    double fNewY(0);

    switch ( m_pCtlSize->GetActualRP() )
    {
        case RP_LT:
        {
            fNewX = maWorkRange.getWidth() - ( maRange.getMinX() - fLeft );
            fNewY = maWorkRange.getHeight() - ( maRange.getMinY() - fTop );
            break;
        }
        case RP_MT:
        {
            fNewX = std::min( maRange.getCenter().getX() - fLeft, fRight - maRange.getCenter().getX() ) * 2.0;
            fNewY = maWorkRange.getHeight() - ( maRange.getMinY() - fTop );
            break;
        }
        case RP_RT:
        {
            fNewX = maWorkRange.getWidth() - ( fRight - maRange.getMaxX() );
            fNewY = maWorkRange.getHeight() - ( maRange.getMinY() - fTop );
            break;
        }
        case RP_LM:
        {
            fNewX = maWorkRange.getWidth() - ( maRange.getMinX() - fLeft );
            fNewY = std::min( maRange.getCenter().getY() - fTop, fBottom - maRange.getCenter().getY() ) * 2.0;
            break;
        }
        case RP_MM:
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
        case RP_RM:
        {
            fNewX = maWorkRange.getWidth() - ( fRight - maRange.getMaxX() );
            fNewY = std::min( maRange.getCenter().getY() - fTop, fBottom - maRange.getCenter().getY() ) * 2.0;
            break;
        }
        case RP_LB:
        {
            fNewX = maWorkRange.getWidth() - ( maRange.getMinX() - fLeft );
            fNewY = maWorkRange.getHeight() - ( fBottom - maRange.getMaxY() );
            break;
        }
        case RP_MB:
        {
            fNewX = std::min( maRange.getCenter().getX() - fLeft, fRight - maRange.getCenter().getX() ) * 2.0;
            fNewY = maWorkRange.getHeight() - ( maRange.getMaxY() - fBottom );
            break;
        }
        case RP_RB:
        {
            fNewX = maWorkRange.getWidth() - ( fRight - maRange.getMaxX() );
            fNewY = maWorkRange.getHeight() - ( fBottom - maRange.getMaxY() );
            break;
        }
    }

    // #i75273# normalizing when setting the min/max values was wrong, removed
    m_pMtrWidth->SetMax(basegfx::fround64(fNewX));
    m_pMtrWidth->SetLast(basegfx::fround64(fNewX));
    m_pMtrHeight->SetMax(basegfx::fround64(fNewY));
    m_pMtrHeight->SetLast(basegfx::fround64(fNewY));
}


void SvxPositionSizeTabPage::GetTopLeftPosition(double& rfX, double& rfY, const basegfx::B2DRange& rRange)
{
    switch (m_pCtlPos->GetActualRP())
    {
        case RP_LT:
        {
            break;
        }
        case RP_MT:
        {
            rfX -= rRange.getCenter().getX() - rRange.getMinX();
            break;
        }
        case RP_RT:
        {
            rfX -= rRange.getWidth();
            break;
        }
        case RP_LM:
        {
            rfY -= rRange.getCenter().getY() - rRange.getMinY();
            break;
        }
        case RP_MM:
        {
            rfX -= rRange.getCenter().getX() - rRange.getMinX();
            rfY -= rRange.getCenter().getY() - rRange.getMinY();
            break;
        }
        case RP_RM:
        {
            rfX -= rRange.getWidth();
            rfY -= rRange.getCenter().getY() - rRange.getMinY();
            break;
        }
        case RP_LB:
        {
            rfY -= rRange.getHeight();
            break;
        }
        case RP_MB:
        {
            rfX -= rRange.getCenter().getX() - rRange.getMinX();
            rfY -= rRange.getHeight();
            break;
        }
        case RP_RB:
        {
            rfX -= rRange.getWidth();
            rfY -= rRange.getHeight();
            break;
        }
    }
}


void SvxPositionSizeTabPage::PointChanged( vcl::Window* pWindow, RECT_POINT eRP )
{
    if( pWindow == m_pCtlPos )
    {
        SetMinMaxPosition();
        switch( eRP )
        {
            case RP_LT:
            {
                m_pMtrPosX->SetValue( basegfx::fround64(maRange.getMinX()) );
                m_pMtrPosY->SetValue( basegfx::fround64(maRange.getMinY()) );
                break;
            }
            case RP_MT:
            {
                m_pMtrPosX->SetValue( basegfx::fround64(maRange.getCenter().getX()) );
                m_pMtrPosY->SetValue( basegfx::fround64(maRange.getMinY()) );
                break;
            }
            case RP_RT:
            {
                m_pMtrPosX->SetValue( basegfx::fround64(maRange.getMaxX()) );
                m_pMtrPosY->SetValue( basegfx::fround64(maRange.getMinY()) );
                break;
            }
            case RP_LM:
            {
                m_pMtrPosX->SetValue( basegfx::fround64(maRange.getMinX()) );
                m_pMtrPosY->SetValue( basegfx::fround64(maRange.getCenter().getY()) );
                break;
            }
            case RP_MM:
            {
                m_pMtrPosX->SetValue( basegfx::fround64(maRange.getCenter().getX()) );
                m_pMtrPosY->SetValue( basegfx::fround64(maRange.getCenter().getY()) );
                break;
            }
            case RP_RM:
            {
                m_pMtrPosX->SetValue( basegfx::fround64(maRange.getMaxX()) );
                m_pMtrPosY->SetValue( basegfx::fround64(maRange.getCenter().getY()) );
                break;
            }
            case RP_LB:
            {
                m_pMtrPosX->SetValue( basegfx::fround64(maRange.getMinX()) );
                m_pMtrPosY->SetValue( basegfx::fround64(maRange.getMaxY()) );
                break;
            }
            case RP_MB:
            {
                m_pMtrPosX->SetValue( basegfx::fround64(maRange.getCenter().getX()) );
                m_pMtrPosY->SetValue( basegfx::fround64(maRange.getMaxY()) );
                break;
            }
            case RP_RB:
            {
                m_pMtrPosX->SetValue( basegfx::fround64(maRange.getMaxX()) );
                m_pMtrPosY->SetValue( basegfx::fround64(maRange.getMaxY()) );
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


IMPL_LINK_NOARG_TYPED(SvxPositionSizeTabPage, ChangeWidthHdl, Edit&, void)
{
    if( m_pCbxScale->IsChecked() && m_pCbxScale->IsEnabled() )
    {
        sal_Int64 nHeight(basegfx::fround64((mfOldHeight * (double)m_pMtrWidth->GetValue()) / mfOldWidth));

        if(nHeight <= m_pMtrHeight->GetMax(FUNIT_NONE))
        {
            m_pMtrHeight->SetUserValue(nHeight, FUNIT_NONE);
        }
        else
        {
            nHeight = m_pMtrHeight->GetMax(FUNIT_NONE);
            m_pMtrHeight->SetUserValue(nHeight);

            const sal_Int64 nWidth(basegfx::fround64((mfOldWidth * (double)nHeight) / mfOldHeight));
            m_pMtrWidth->SetUserValue(nWidth, FUNIT_NONE);
        }
    }
}


IMPL_LINK_NOARG_TYPED(SvxPositionSizeTabPage, ChangeHeightHdl, Edit&, void)
{
    if( m_pCbxScale->IsChecked() && m_pCbxScale->IsEnabled() )
    {
        sal_Int64 nWidth(basegfx::fround64((mfOldWidth * (double)m_pMtrHeight->GetValue()) / mfOldHeight));

        if(nWidth <= m_pMtrWidth->GetMax(FUNIT_NONE))
        {
            m_pMtrWidth->SetUserValue(nWidth, FUNIT_NONE);
        }
        else
        {
            nWidth = m_pMtrWidth->GetMax(FUNIT_NONE);
            m_pMtrWidth->SetUserValue(nWidth);

            const sal_Int64 nHeight(basegfx::fround64((mfOldHeight * (double)nWidth) / mfOldWidth));
            m_pMtrHeight->SetUserValue(nHeight, FUNIT_NONE);
        }
    }
}


IMPL_LINK_NOARG_TYPED(SvxPositionSizeTabPage, ClickSizeProtectHdl, Button*, void)
{
    UpdateControlStates();
}


IMPL_LINK_NOARG_TYPED(SvxPositionSizeTabPage, ClickAutoHdl, Button*, void)
{
    if( m_pCbxScale->IsChecked() )
    {
        mfOldWidth  = std::max( (double)GetCoreValue( *m_pMtrWidth,  mePoolUnit ), 1.0 );
        mfOldHeight = std::max( (double)GetCoreValue( *m_pMtrHeight, mePoolUnit ), 1.0 );
    }
}


void SvxPositionSizeTabPage::FillUserData()
{
    // matching is saved in the Ini-file
    OUString aStr = m_pCbxScale->IsChecked() ? OUString("1") : OUString("0");
    SetUserData( aStr );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
