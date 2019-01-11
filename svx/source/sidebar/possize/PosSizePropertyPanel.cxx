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

#include <algorithm>

#include <sfx2/sidebar/ControlFactory.hxx>
#include "PosSizePropertyPanel.hxx"
#include <svx/sidebar/SidebarDialControl.hxx>
#include <svx/svxids.hrc>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>
#include <svx/dlgutil.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/viewoptions.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/toolbox.hxx>
#include <svl/aeitem.hxx>
#include <svx/svdview.hxx>
#include <svx/transfrmhelper.hxx>
#include <comphelper/lok.hxx>

using namespace css;
using namespace css::uno;

const char USERITEM_NAME[]      = "FitItem";

namespace {

bool lcl_twipsNeeded(const SdrView* pView)
{
    const bool bTiledRendering = comphelper::LibreOfficeKit::isActive();
    if (bTiledRendering)
    {
        // We gets the position in twips
        if (OutputDevice* pOutputDevice = pView->GetFirstOutputDevice())
        {
            if (pOutputDevice->GetMapMode().GetMapUnit() == MapUnit::Map100thMM)
            {
                return true;
            }
        }
    }
    return false;
}

} // anonymouus ns

namespace svx { namespace sidebar {

PosSizePropertyPanel::PosSizePropertyPanel(
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings,
    const css::uno::Reference<css::ui::XSidebar>& rxSidebar)
:   PanelLayout(pParent, "PosSizePropertyPanel", "svx/ui/sidebarpossize.ui", rxFrame),
    maRect(),
    mpView(nullptr),
    mlOldWidth(1),
    mlOldHeight(1),
    mlRotX(0),
    mlRotY(0),
    maUIScale(),
    mePoolUnit(),
    meDlgUnit(FieldUnit::INCH), // #i124409# init with fallback default
    maTransfPosXControl(SID_ATTR_TRANSFORM_POS_X, *pBindings, *this),
    maTransfPosYControl(SID_ATTR_TRANSFORM_POS_Y, *pBindings, *this),
    maTransfWidthControl(SID_ATTR_TRANSFORM_WIDTH, *pBindings, *this),
    maTransfHeightControl(SID_ATTR_TRANSFORM_HEIGHT, *pBindings, *this),
    maSvxAngleControl( SID_ATTR_TRANSFORM_ANGLE, *pBindings, *this),
    maRotXControl(SID_ATTR_TRANSFORM_ROT_X, *pBindings, *this),
    maRotYControl(SID_ATTR_TRANSFORM_ROT_Y, *pBindings, *this),
    maProPosControl(SID_ATTR_TRANSFORM_PROTECT_POS, *pBindings, *this),
    maProSizeControl(SID_ATTR_TRANSFORM_PROTECT_SIZE, *pBindings, *this),
    maAutoWidthControl(SID_ATTR_TRANSFORM_AUTOWIDTH, *pBindings, *this),
    maAutoHeightControl(SID_ATTR_TRANSFORM_AUTOHEIGHT, *pBindings, *this),
    m_aMetricCtl(SID_ATTR_METRIC, *pBindings, *this),
    maContext(),
    mpBindings(pBindings),
    mbSizeProtected(false),
    mbPositionProtected(false),
    mbAutoWidth(false),
    mbAutoHeight(false),
    mbAdjustEnabled(false),
    mxSidebar(rxSidebar)
{
    get( mpFtPosX,    "horizontallabel" );
    get( mpMtrPosX,   "horizontalpos" );
    get( mpFtPosY,    "verticallabel" );
    get( mpMtrPosY,   "verticalpos" );
    get( mpFtWidth,   "widthlabel" );
    get( mpMtrWidth,  "selectwidth" );
    get( mpFtHeight,  "heightlabel" );
    get( mpMtrHeight, "selectheight" );
    get( mpCbxScale,  "ratio" );
    get( mpFtAngle,   "rotationlabel" );
    get( mpMtrAngle,  "rotation" );
    get( mpDial,      "orientationcontrol" );
    get( mpFtFlip,    "fliplabel" );
    get( mpFlipTbx,   "selectrotationtype" );
    Initialize();

    mpBindings->Update( SID_ATTR_TRANSFORM_WIDTH );
    mpBindings->Update( SID_ATTR_TRANSFORM_HEIGHT );
    mpBindings->Update( SID_ATTR_TRANSFORM_PROTECT_SIZE );
    mpBindings->Update( SID_ATTR_METRIC );
}

PosSizePropertyPanel::~PosSizePropertyPanel()
{
    disposeOnce();
}

void PosSizePropertyPanel::dispose()
{
    mpFtPosX.clear();
    mpMtrPosX.clear();
    mpFtPosY.clear();
    mpMtrPosY.clear();
    mpFtWidth.clear();
    mpMtrWidth.clear();
    mpFtHeight.clear();
    mpMtrHeight.clear();
    mpCbxScale.clear();
    mpFtAngle.clear();
    mpMtrAngle.clear();
    mpDial.clear();
    mpFtFlip.clear();
    mpFlipTbx.clear();

    maTransfPosXControl.dispose();
    maTransfPosYControl.dispose();
    maTransfWidthControl.dispose();
    maTransfHeightControl.dispose();

    maSvxAngleControl.dispose();
    maRotXControl.dispose();
    maRotYControl.dispose();
    maProPosControl.dispose();
    maProSizeControl.dispose();
    maAutoWidthControl.dispose();
    maAutoHeightControl.dispose();
    m_aMetricCtl.dispose();

    PanelLayout::dispose();
}


namespace
{
    bool hasText(const SdrView& rSdrView)
    {
        const SdrMarkList& rMarkList = rSdrView.GetMarkedObjectList();

        if(1 == rMarkList.GetMarkCount())
        {
            const SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
            const SdrObjKind eKind(static_cast<SdrObjKind>(pObj->GetObjIdentifier()));

            if((pObj->GetObjInventor() == SdrInventor::Default) && (OBJ_TEXT == eKind || OBJ_TITLETEXT == eKind || OBJ_OUTLINETEXT == eKind))
            {
                const SdrTextObj* pSdrTextObj = dynamic_cast< const SdrTextObj* >(pObj);

                if(pSdrTextObj && pSdrTextObj->HasText())
                {
                    return true;
                }
            }
        }

        return false;
    }
} // end of anonymous namespace


void PosSizePropertyPanel::Initialize()
{
    //Position : Horizontal / Vertical
    mpMtrPosX->SetModifyHdl( LINK( this, PosSizePropertyPanel, ChangePosXHdl ) );
    mpMtrPosY->SetModifyHdl( LINK( this, PosSizePropertyPanel, ChangePosYHdl ) );

    //Size : Width / Height
    mpMtrWidth->SetModifyHdl( LINK( this, PosSizePropertyPanel, ChangeWidthHdl ) );
    mpMtrHeight->SetModifyHdl( LINK( this, PosSizePropertyPanel, ChangeHeightHdl ) );

    //Size : Keep ratio
    mpCbxScale->SetClickHdl( LINK( this, PosSizePropertyPanel, ClickAutoHdl ) );

    //rotation:
    mpMtrAngle->SetModifyHdl(LINK( this, PosSizePropertyPanel, AngleModifiedHdl));
    mpMtrAngle->EnableAutocomplete( false );

    //rotation control
    mpDial->SetModifyHdl(LINK( this, PosSizePropertyPanel, RotationHdl));

    //flip:
    mpFlipTbx->SetSelectHdl( LINK( this, PosSizePropertyPanel, FlipHdl) );

    mpMtrAngle->InsertValue(0, FieldUnit::CUSTOM);
    mpMtrAngle->InsertValue(4500, FieldUnit::CUSTOM);
    mpMtrAngle->InsertValue(9000, FieldUnit::CUSTOM);
    mpMtrAngle->InsertValue(13500, FieldUnit::CUSTOM);
    mpMtrAngle->InsertValue(18000, FieldUnit::CUSTOM);
    mpMtrAngle->InsertValue(22500, FieldUnit::CUSTOM);
    mpMtrAngle->InsertValue(27000, FieldUnit::CUSTOM);
    mpMtrAngle->InsertValue(31500, FieldUnit::CUSTOM);
    mpMtrAngle->AdaptDropDownLineCountToMaximum();

    SfxViewShell* pCurSh = SfxViewShell::Current();
    if ( pCurSh )
        mpView = pCurSh->GetDrawView();
    else
        mpView = nullptr;

    if ( mpView != nullptr )
    {
        maUIScale = mpView->GetModel()->GetUIScale();
        mbAdjustEnabled = hasText(*mpView);
    }

    mePoolUnit = maTransfWidthControl.GetCoreMetric();
}

VclPtr<vcl::Window> PosSizePropertyPanel::Create (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings,
    const css::uno::Reference<css::ui::XSidebar>& rxSidebar)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to PosSizePropertyPanel::Create", nullptr, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to PosSizePropertyPanel::Create", nullptr, 1);
    if (pBindings == nullptr)
        throw lang::IllegalArgumentException("no SfxBindings given to PosSizePropertyPanel::Create", nullptr, 2);

    return VclPtr<PosSizePropertyPanel>::Create(
                        pParent,
                        rxFrame,
                        pBindings,
                        rxSidebar);
}


void PosSizePropertyPanel::DataChanged(
    const DataChangedEvent& /*rEvent*/)
{
}

void PosSizePropertyPanel::HandleContextChange(
    const vcl::EnumContext& rContext)
{
    if (maContext == rContext)
    {
        // Nothing to do.
        return;
    }

    maContext = rContext;

    bool bShowPosition = false;
    bool bShowAngle = false;
    bool bShowFlip = false;

    switch (maContext.GetCombinedContext_DI())
    {
        case CombinedEnumContext(Application::WriterVariants, Context::Draw):
            bShowAngle = true;
            bShowFlip = true;
            break;

        case CombinedEnumContext(Application::WriterVariants, Context::Graphic):
            bShowFlip = true;
            bShowAngle = true; // RotGrfFlyFrame: Writer FlyFrames for Graphics now support angle
            break;

        case CombinedEnumContext(Application::Calc, Context::Draw):
        case CombinedEnumContext(Application::Calc, Context::DrawLine):
        case CombinedEnumContext(Application::Calc, Context::Graphic):
        case CombinedEnumContext(Application::DrawImpress, Context::Draw):
        case CombinedEnumContext(Application::DrawImpress, Context::DrawLine):
        case CombinedEnumContext(Application::DrawImpress, Context::TextObject):
        case CombinedEnumContext(Application::DrawImpress, Context::Graphic):
            bShowPosition = true;
            bShowAngle = true;
            bShowFlip = true;
            break;

        case CombinedEnumContext(Application::Calc, Context::Chart):
        case CombinedEnumContext(Application::Calc, Context::Form):
        case CombinedEnumContext(Application::Calc, Context::Media):
        case CombinedEnumContext(Application::Calc, Context::OLE):
        case CombinedEnumContext(Application::Calc, Context::MultiObject):
        case CombinedEnumContext(Application::DrawImpress, Context::Media):
        case CombinedEnumContext(Application::DrawImpress, Context::Form):
        case CombinedEnumContext(Application::DrawImpress, Context::OLE):
        case CombinedEnumContext(Application::DrawImpress, Context::ThreeDObject):
        case CombinedEnumContext(Application::DrawImpress, Context::MultiObject):
            bShowPosition = true;
            break;
    }

    // Position
    mpFtPosX->Show(bShowPosition);
    mpMtrPosX->Show(bShowPosition);
    mpFtPosY->Show(bShowPosition);
    mpMtrPosY->Show(bShowPosition);

    // Rotation
    mpFtAngle->Show(bShowAngle);
    mpMtrAngle->Show(bShowAngle);
    mpDial->Show(bShowAngle);

    // Flip
    mpFtFlip->Show(bShowFlip);
    mpFlipTbx->Show(bShowFlip);

    if (mxSidebar.is())
        mxSidebar->requestLayout();
}


IMPL_LINK_NOARG( PosSizePropertyPanel, ChangeWidthHdl, Edit&, void )
{
    if( mpCbxScale->IsChecked() &&
        mpCbxScale->IsEnabled() )
    {
        long nHeight = static_cast<long>( (static_cast<double>(mlOldHeight) * static_cast<double>(mpMtrWidth->GetValue())) / static_cast<double>(mlOldWidth) );
        if( nHeight <= mpMtrHeight->GetMax( FieldUnit::NONE ) )
        {
            mpMtrHeight->SetUserValue( nHeight, FieldUnit::NONE );
        }
        else
        {
            nHeight = static_cast<long>(mpMtrHeight->GetMax( FieldUnit::NONE ));
            mpMtrHeight->SetUserValue( nHeight );
            const long nWidth = static_cast<long>( (static_cast<double>(mlOldWidth) * static_cast<double>(nHeight)) / static_cast<double>(mlOldHeight) );
            mpMtrWidth->SetUserValue( nWidth, FieldUnit::NONE );
        }
    }
    executeSize();
}


IMPL_LINK_NOARG( PosSizePropertyPanel, ChangeHeightHdl, Edit&, void )
{
    if( mpCbxScale->IsChecked() &&
        mpCbxScale->IsEnabled() )
    {
        long nWidth = static_cast<long>( (static_cast<double>(mlOldWidth) * static_cast<double>(mpMtrHeight->GetValue())) / static_cast<double>(mlOldHeight) );
        if( nWidth <= mpMtrWidth->GetMax( FieldUnit::NONE ) )
        {
            mpMtrWidth->SetUserValue( nWidth, FieldUnit::NONE );
        }
        else
        {
            nWidth = static_cast<long>(mpMtrWidth->GetMax( FieldUnit::NONE ));
            mpMtrWidth->SetUserValue( nWidth );
            const long nHeight = static_cast<long>( (static_cast<double>(mlOldHeight) * static_cast<double>(nWidth)) / static_cast<double>(mlOldWidth) );
            mpMtrHeight->SetUserValue( nHeight, FieldUnit::NONE );
        }
    }
    executeSize();
}


IMPL_LINK_NOARG( PosSizePropertyPanel, ChangePosXHdl, Edit&, void )
{
    if ( mpMtrPosX->IsValueModified())
    {
        long lX = GetCoreValue( *mpMtrPosX, mePoolUnit );

        Fraction aUIScale = mpView->GetModel()->GetUIScale();
        lX = long( lX * aUIScale );

        if (lcl_twipsNeeded(mpView))
        {
            lX = OutputDevice::LogicToLogic(lX, MapUnit::Map100thMM, MapUnit::MapTwip);
        }

        SfxInt32Item aPosXItem( SID_ATTR_TRANSFORM_POS_X,static_cast<sal_uInt32>(lX));

        GetBindings()->GetDispatcher()->ExecuteList(
            SID_ATTR_TRANSFORM, SfxCallMode::RECORD, { &aPosXItem });
    }
}


IMPL_LINK_NOARG( PosSizePropertyPanel, ChangePosYHdl, Edit&, void )
{
    if ( mpMtrPosY->IsValueModified() )
    {
        long lY = GetCoreValue( *mpMtrPosY, mePoolUnit );

        Fraction aUIScale = mpView->GetModel()->GetUIScale();
        lY = long( lY * aUIScale );

        if (lcl_twipsNeeded(mpView))
        {
            lY = OutputDevice::LogicToLogic(lY, MapUnit::Map100thMM, MapUnit::MapTwip);
        }

        SfxInt32Item aPosYItem( SID_ATTR_TRANSFORM_POS_Y,static_cast<sal_uInt32>(lY));

        GetBindings()->GetDispatcher()->ExecuteList(
            SID_ATTR_TRANSFORM, SfxCallMode::RECORD, { &aPosYItem });
    }
}


IMPL_LINK_NOARG( PosSizePropertyPanel, ClickAutoHdl, Button*, void )
{
    if ( mpCbxScale->IsChecked() )
    {
        mlOldWidth  = std::max( GetCoreValue( *mpMtrWidth,  mePoolUnit ), 1L );
        mlOldHeight = std::max( GetCoreValue( *mpMtrHeight, mePoolUnit ), 1L );
    }

    // mpCbxScale must synchronized with that on Position and Size tabpage on Shape Properties dialog
    SvtViewOptions aPageOpt(EViewType::TabPage, "cui/ui/possizetabpage/PositionAndSize");
    aPageOpt.SetUserItem( USERITEM_NAME, css::uno::makeAny( OUString::number( int(mpCbxScale->IsChecked()) ) ) );
}


IMPL_LINK_NOARG( PosSizePropertyPanel, AngleModifiedHdl, Edit&, void )
{
    OUString sTmp = mpMtrAngle->GetText();
    if (sTmp.isEmpty())
        return;
    sal_Unicode nChar = sTmp[0];
    if( nChar == '-' )
    {
        if (sTmp.getLength() < 2)
            return;
        nChar = sTmp[1];
    }

    if( (nChar < '0') || (nChar > '9') )
        return;

    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );

    // Do not check that the entire string was parsed up to its end, there may
    // be a degree symbol following the number. Note that this also means that
    // the number recognized just stops at any non-matching character.
    /* TODO: we could check for the degree symbol stop if there are no other
     * cases with different symbol characters in any language? */
    rtl_math_ConversionStatus eStatus;
    double fTmp = rLocaleWrapper.stringToDouble( sTmp, false, &eStatus, nullptr);
    if (eStatus != rtl_math_ConversionStatus_Ok)
        return;

    while (fTmp < 0)
        fTmp += 360;

    sal_Int64 nTmp = fTmp*100;

    // #i123993# Need to take UIScale into account when executing rotations
    const double fUIScale(mpView && mpView->GetModel() ? double(mpView->GetModel()->GetUIScale()) : 1.0);
    long nRotateX = basegfx::fround(mlRotX * fUIScale);
    long nRotateY = basegfx::fround(mlRotY * fUIScale);

    if (lcl_twipsNeeded(mpView))
    {
        nRotateX = OutputDevice::LogicToLogic(nRotateX, MapUnit::Map100thMM, MapUnit::MapTwip);
        nRotateY = OutputDevice::LogicToLogic(nRotateY, MapUnit::Map100thMM, MapUnit::MapTwip);
    }

    SfxInt32Item aAngleItem( SID_ATTR_TRANSFORM_ANGLE,static_cast<sal_uInt32>(nTmp));
    SfxInt32Item aRotXItem( SID_ATTR_TRANSFORM_ROT_X, nRotateX);
    SfxInt32Item aRotYItem( SID_ATTR_TRANSFORM_ROT_Y, nRotateY);

    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_TRANSFORM,
            SfxCallMode::RECORD, { &aAngleItem, &aRotXItem, &aRotYItem });
}


IMPL_LINK_NOARG( PosSizePropertyPanel, RotationHdl, DialControl*, void )
{
    sal_Int32 nTmp = mpDial->GetRotation();

    // #i123993# Need to take UIScale into account when executing rotations
    const double fUIScale(mpView && mpView->GetModel() ? double(mpView->GetModel()->GetUIScale()) : 1.0);
    long nRotateX = basegfx::fround(mlRotX * fUIScale);
    long nRotateY = basegfx::fround(mlRotY * fUIScale);

    if (lcl_twipsNeeded(mpView))
    {
        nRotateX = OutputDevice::LogicToLogic(nRotateX, MapUnit::Map100thMM, MapUnit::MapTwip);
        nRotateY = OutputDevice::LogicToLogic(nRotateY, MapUnit::Map100thMM, MapUnit::MapTwip);
    }

    SfxInt32Item aAngleItem( SID_ATTR_TRANSFORM_ANGLE,static_cast<sal_uInt32>(nTmp));
    SfxInt32Item aRotXItem( SID_ATTR_TRANSFORM_ROT_X, nRotateX);
    SfxInt32Item aRotYItem( SID_ATTR_TRANSFORM_ROT_Y, nRotateY);

    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_TRANSFORM,
            SfxCallMode::RECORD, { &aAngleItem, &aRotXItem, &aRotYItem });
}


IMPL_LINK( PosSizePropertyPanel, FlipHdl, ToolBox*, pBox, void )
{
    const OUString aCommand(pBox->GetItemCommand(pBox->GetCurItemId()));

    if(aCommand == ".uno:FlipHorizontal")
    {
        SfxVoidItem aHoriItem(SID_FLIP_HORIZONTAL);
        GetBindings()->GetDispatcher()->ExecuteList(SID_FLIP_HORIZONTAL,
                SfxCallMode::RECORD, { &aHoriItem });
    }
    else if(aCommand == ".uno:FlipVertical")
    {
        SfxVoidItem aVertItem(SID_FLIP_VERTICAL);
        GetBindings()->GetDispatcher()->ExecuteList(SID_FLIP_VERTICAL,
                SfxCallMode::RECORD, { &aVertItem });
    }
}


void PosSizePropertyPanel::NotifyItemUpdate(
    sal_uInt16 nSID,
    SfxItemState eState,
    const SfxPoolItem* pState,
    const bool)
{
    mpFtAngle->Enable();
    mpMtrAngle->Enable();
    mpDial->Enable();
    mpFtFlip->Enable();
    mpFlipTbx->Enable();

    const SfxUInt32Item*    pWidthItem;
    const SfxUInt32Item*    pHeightItem;

    SfxViewShell* pCurSh = SfxViewShell::Current();
    if ( pCurSh )
        mpView = pCurSh->GetDrawView();
    else
        mpView = nullptr;

    if ( mpView == nullptr )
        return;

    mbAdjustEnabled = hasText(*mpView);

    // Pool unit and dialog unit may have changed, make sure that we
    // have the current values.
    mePoolUnit = maTransfWidthControl.GetCoreMetric();

    switch (nSID)
    {
        case SID_ATTR_TRANSFORM_WIDTH:
            if ( SfxItemState::DEFAULT == eState )
            {
                pWidthItem = dynamic_cast< const SfxUInt32Item* >(pState);

                if(pWidthItem)
                {
                    long lOldWidth1 = long( pWidthItem->GetValue() / maUIScale );
                    SetFieldUnit( *mpMtrWidth, meDlgUnit, true );
                    SetMetricValue( *mpMtrWidth, lOldWidth1, mePoolUnit );
                    mlOldWidth = lOldWidth1;
                    break;
                }
            }

            mpMtrWidth->SetText( "" );
            break;

        case SID_ATTR_TRANSFORM_HEIGHT:
            if ( SfxItemState::DEFAULT == eState )
            {
                pHeightItem = dynamic_cast< const SfxUInt32Item* >(pState);

                if(pHeightItem)
                {
                    long nTmp = long( pHeightItem->GetValue() / maUIScale);
                    SetFieldUnit( *mpMtrHeight, meDlgUnit, true );
                    SetMetricValue( *mpMtrHeight, nTmp, mePoolUnit );
                    mlOldHeight = nTmp;
                    break;
                }
            }

            mpMtrHeight->SetText( "");
            break;

        case SID_ATTR_TRANSFORM_POS_X:
            if(SfxItemState::DEFAULT == eState)
            {
                const SfxInt32Item* pItem = dynamic_cast< const SfxInt32Item* >(pState);

                if(pItem)
                {
                    long nTmp = long(pItem->GetValue() / maUIScale);
                    SetFieldUnit( *mpMtrPosX, meDlgUnit, true );
                    SetMetricValue( *mpMtrPosX, nTmp, mePoolUnit );
                    break;
                }
            }

            mpMtrPosX->SetText( "" );
            break;

        case SID_ATTR_TRANSFORM_POS_Y:
            if(SfxItemState::DEFAULT == eState)
            {
                const SfxInt32Item* pItem = dynamic_cast< const SfxInt32Item* >(pState);

                if(pItem)
                {
                    long nTmp = long(pItem->GetValue() / maUIScale);
                    SetFieldUnit( *mpMtrPosY, meDlgUnit, true );
                    SetMetricValue( *mpMtrPosY, nTmp, mePoolUnit );
                    break;
                }
            }

            mpMtrPosY->SetText( "" );
            break;

        case SID_ATTR_TRANSFORM_ROT_X:
            if (SfxItemState::DEFAULT == eState)
            {
                const SfxInt32Item* pItem = dynamic_cast< const SfxInt32Item* >(pState);

                if(pItem)
                {
                    mlRotX = pItem->GetValue();
                    mlRotX = long( mlRotX / maUIScale );
                }
            }
            break;

        case SID_ATTR_TRANSFORM_ROT_Y:
            if (SfxItemState::DEFAULT == eState)
            {
                const SfxInt32Item* pItem = dynamic_cast< const SfxInt32Item* >(pState);

                if(pItem)
                {
                    mlRotY = pItem->GetValue();
                    mlRotY = long( mlRotY / maUIScale );
                }
            }
            break;

        case SID_ATTR_TRANSFORM_PROTECT_POS:
            if(SfxItemState::DEFAULT == eState)
            {
                const SfxBoolItem* pItem = dynamic_cast< const SfxBoolItem* >(pState);

                if(pItem)
                {
                    // record the state of position protect
                    mbPositionProtected = pItem->GetValue();
                    break;
                }
            }

            mbPositionProtected = false;
            break;

        case SID_ATTR_TRANSFORM_PROTECT_SIZE:
            if(SfxItemState::DEFAULT == eState)
            {
                const SfxBoolItem* pItem = dynamic_cast< const SfxBoolItem* >(pState);

                if(pItem)
                {
                    // record the state of size protect
                    mbSizeProtected = pItem->GetValue();
                    break;
                }
            }

            mbSizeProtected = false;
            break;

        case SID_ATTR_TRANSFORM_AUTOWIDTH:
            if(SfxItemState::DEFAULT == eState)
            {
                const SfxBoolItem* pItem = dynamic_cast< const SfxBoolItem* >(pState);

                if(pItem)
                {
                    mbAutoWidth = pItem->GetValue();
                }
            }
            break;

        case SID_ATTR_TRANSFORM_AUTOHEIGHT:
            if(SfxItemState::DEFAULT == eState)
            {
                const SfxBoolItem* pItem = dynamic_cast< const SfxBoolItem* >(pState);

                if(pItem)
                {
                    mbAutoHeight = pItem->GetValue();
                }
            }
            break;

        case SID_ATTR_TRANSFORM_ANGLE:
            if (eState >= SfxItemState::DEFAULT)
            {
                const SfxInt32Item* pItem = dynamic_cast< const SfxInt32Item* >(pState);

                if(pItem)
                {
                    long nTmp = pItem->GetValue();
                    nTmp = nTmp < 0 ? 36000+nTmp : nTmp;

                    mpMtrAngle->SetValue( nTmp );
                    mpDial->SetRotation( nTmp );

                    switch(nTmp)
                    {
                        case 0:
                            mpMtrAngle->SelectEntryPos(0);
                            break;
                        case 4500:
                            mpMtrAngle->SelectEntryPos(1);
                            break;
                        case 9000:
                            mpMtrAngle->SelectEntryPos(2);
                            break;
                        case 13500:
                            mpMtrAngle->SelectEntryPos(3);
                            break;
                        case 18000:
                            mpMtrAngle->SelectEntryPos(4);
                            break;
                        case 22500:
                            mpMtrAngle->SelectEntryPos(5);
                            break;
                        case 27000:
                            mpMtrAngle->SelectEntryPos(6);
                            break;
                        case 315000:
                            mpMtrAngle->SelectEntryPos(7);
                            break;
                    }

                    break;
                }
            }

            mpMtrAngle->SetText( "" );
            mpDial->SetRotation( 0 );
            break;

        case SID_ATTR_METRIC:
            MetricState( eState, pState );
            UpdateUIScale();
            break;

        default:
            break;
    }

    const sal_Int32 nCombinedContext(maContext.GetCombinedContext_DI());
    const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();

    switch (rMarkList.GetMarkCount())
    {
        case 0:
            break;

        case 1:
        {
            const SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
            const SdrObjKind eKind(static_cast<SdrObjKind>(pObj->GetObjIdentifier()));

            if(((nCombinedContext == CombinedEnumContext(Application::DrawImpress, Context::Draw)
               || nCombinedContext == CombinedEnumContext(Application::DrawImpress, Context::TextObject)
                 ) && OBJ_EDGE == eKind)
               || OBJ_CAPTION == eKind)
            {
                mpFtAngle->Disable();
                mpMtrAngle->Disable();
                mpDial->Disable();
                mpFlipTbx->Disable();
                mpFtFlip->Disable();
            }
            break;
        }

        default:
        {
            sal_uInt16 nMarkObj = 0;
            bool isNoEdge = true;

            while(isNoEdge && rMarkList.GetMark(nMarkObj))
            {
                const SdrObject* pObj = rMarkList.GetMark(nMarkObj)->GetMarkedSdrObj();
                const SdrObjKind eKind(static_cast<SdrObjKind>(pObj->GetObjIdentifier()));

                if(((nCombinedContext == CombinedEnumContext(Application::DrawImpress, Context::Draw)
                  || nCombinedContext == CombinedEnumContext(Application::DrawImpress, Context::TextObject)
                     ) && OBJ_EDGE == eKind)
                  || OBJ_CAPTION == eKind)
                {
                    isNoEdge = false;
                    break;
                }
                nMarkObj++;
            }

            if(!isNoEdge)
            {
                mpFtAngle->Disable();
                mpMtrAngle->Disable();
                mpDial->Disable();
                mpFlipTbx->Disable();
                mpFtFlip->Disable();
            }
            break;
        }
    }

    if(nCombinedContext == CombinedEnumContext(Application::DrawImpress, Context::TextObject))
    {
        mpFlipTbx->Disable();
        mpFtFlip->Disable();
    }

    DisableControls();

    // mpCbxScale must synchronized with that on Position and Size tabpage on Shape Properties dialog
    SvtViewOptions aPageOpt(EViewType::TabPage, "cui/ui/possizetabpage/PositionAndSize");
    OUString  sUserData;
    css::uno::Any  aUserItem = aPageOpt.GetUserItem( USERITEM_NAME );
    OUString aTemp;
    if ( aUserItem >>= aTemp )
        sUserData = aTemp;
    mpCbxScale->Check( static_cast<bool>(sUserData.toInt32()) );
}


void PosSizePropertyPanel::executeSize()
{
    if ( !mpMtrWidth->IsValueModified() && !mpMtrHeight->IsValueModified())
        return;

    Fraction aUIScale = mpView->GetModel()->GetUIScale();

    // get Width
    double nWidth = static_cast<double>(mpMtrWidth->GetValue( meDlgUnit ));
    nWidth = MetricField::ConvertDoubleValue( nWidth, mpMtrWidth->GetBaseValue(), mpMtrWidth->GetDecimalDigits(), meDlgUnit, FieldUnit::MM_100TH );
    long lWidth = static_cast<long>(nWidth * static_cast<double>(aUIScale));
    lWidth = OutputDevice::LogicToLogic( lWidth, MapUnit::Map100thMM, mePoolUnit );
    lWidth = static_cast<long>(mpMtrWidth->Denormalize( lWidth ));

    // get Height
    double nHeight = static_cast<double>(mpMtrHeight->GetValue( meDlgUnit ));
    nHeight = MetricField::ConvertDoubleValue( nHeight, mpMtrHeight->GetBaseValue(), mpMtrHeight->GetDecimalDigits(), meDlgUnit, FieldUnit::MM_100TH );
    long lHeight = static_cast<long>(nHeight * static_cast<double>(aUIScale));
    lHeight = OutputDevice::LogicToLogic( lHeight, MapUnit::Map100thMM, mePoolUnit );
    lHeight = static_cast<long>(mpMtrWidth->Denormalize( lHeight ));

    if (lcl_twipsNeeded(mpView))
    {
        lWidth = OutputDevice::LogicToLogic(lWidth, MapUnit::Map100thMM, MapUnit::MapTwip);
        lHeight = OutputDevice::LogicToLogic(lHeight, MapUnit::Map100thMM, MapUnit::MapTwip);
    }

    // put Width & Height to itemset
    SfxUInt32Item aWidthItem( SID_ATTR_TRANSFORM_WIDTH, static_cast<sal_uInt32>(lWidth));
    SfxUInt32Item aHeightItem( SID_ATTR_TRANSFORM_HEIGHT, static_cast<sal_uInt32>(lHeight));
    SfxAllEnumItem aPointItem (SID_ATTR_TRANSFORM_SIZE_POINT, sal_uInt16(RectPoint::LT));
    const sal_Int32 nCombinedContext(maContext.GetCombinedContext_DI());

    if( nCombinedContext == CombinedEnumContext(Application::WriterVariants, Context::Graphic)
        || nCombinedContext == CombinedEnumContext(Application::WriterVariants, Context::OLE)
        )
    {
        GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_TRANSFORM,
            SfxCallMode::RECORD, { &aWidthItem, &aHeightItem, &aPointItem });
    }
    else
    {
        if ( (mpMtrWidth->IsValueModified()) && (mpMtrHeight->IsValueModified()))
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_TRANSFORM,
                SfxCallMode::RECORD, { &aWidthItem, &aHeightItem, &aPointItem });
        else if( mpMtrWidth->IsValueModified())
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_TRANSFORM,
                SfxCallMode::RECORD, { &aWidthItem, &aPointItem });
        else if ( mpMtrHeight->IsValueModified())
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_TRANSFORM,
                SfxCallMode::RECORD, { &aHeightItem, &aPointItem });
    }
}


void PosSizePropertyPanel::MetricState( SfxItemState eState, const SfxPoolItem* pState )
{
    bool bPosXBlank = false;
    bool bPosYBlank = false;
    bool bWidthBlank = false;
    bool bHeightBlank = false;

    // #i124409# use the given Item to get the correct UI unit and initialize it
    // and the Fields using it
    meDlgUnit = GetCurrentUnit(eState,pState);

    if (mpMtrPosX->GetText().isEmpty())
        bPosXBlank = true;
    SetFieldUnit( *mpMtrPosX, meDlgUnit, true );
    if(bPosXBlank)
        mpMtrPosX->SetText(OUString());

    if (mpMtrPosY->GetText().isEmpty())
        bPosYBlank = true;
    SetFieldUnit( *mpMtrPosY, meDlgUnit, true );
    if(bPosYBlank)
        mpMtrPosY->SetText(OUString());
    SetPosSizeMinMax();

    if (mpMtrWidth->GetText().isEmpty())
        bWidthBlank = true;
    SetFieldUnit( *mpMtrWidth, meDlgUnit, true );
    if(bWidthBlank)
        mpMtrWidth->SetText(OUString());

    if (mpMtrHeight->GetText().isEmpty())
        bHeightBlank = true;
    SetFieldUnit( *mpMtrHeight, meDlgUnit, true );
    if(bHeightBlank)
        mpMtrHeight->SetText(OUString());
}


FieldUnit PosSizePropertyPanel::GetCurrentUnit( SfxItemState eState, const SfxPoolItem* pState )
{
    FieldUnit eUnit = FieldUnit::NONE;

    if ( pState && eState >= SfxItemState::DEFAULT )
    {
        eUnit = static_cast<FieldUnit>(static_cast<const SfxUInt16Item*>(pState)->GetValue());
    }
    else
    {
        SfxViewFrame* pFrame = SfxViewFrame::Current();
        SfxObjectShell* pSh = nullptr;
        if ( pFrame )
            pSh = pFrame->GetObjectShell();
        if ( pSh )
        {
            SfxModule* pModule = pSh->GetModule();
            if ( pModule )
            {
                const SfxPoolItem* pItem = pModule->GetItem( SID_ATTR_METRIC );
                if ( pItem )
                    eUnit = static_cast<FieldUnit>(static_cast<const SfxUInt16Item*>(pItem)->GetValue());
            }
            else
            {
                SAL_WARN("svx.sidebar", "GetModuleFieldUnit(): no module found");
            }
        }
    }

    return eUnit;
}


void PosSizePropertyPanel::DisableControls()
{
    if( mbPositionProtected )
    {
        // the position is protected("Position protect" option in modal dialog is checked),
        // disable all the Position controls in sidebar
        mpFtPosX->Disable();
        mpMtrPosX->Disable();
        mpFtPosY->Disable();
        mpMtrPosY->Disable();
        mpFtAngle->Disable();
        mpMtrAngle->Disable();
        mpDial->Disable();
        mpFtFlip->Disable();
        mpFlipTbx->Disable();

        mpFtWidth->Disable();
        mpMtrWidth->Disable();
        mpFtHeight->Disable();
        mpMtrHeight->Disable();
        mpCbxScale->Disable();
    }
    else
    {
        mpFtPosX->Enable();
        mpMtrPosX->Enable();
        mpFtPosY->Enable();
        mpMtrPosY->Enable();

        //mpFtAngle->Enable();
        //mpMtrAngle->Enable();
        //mpDial->Enable();
        //mpFtFlip->Enable();
        //mpFlipTbx->Enable();

        if( mbSizeProtected )
        {
            mpFtWidth->Disable();
            mpMtrWidth->Disable();
            mpFtHeight->Disable();
            mpMtrHeight->Disable();
            mpCbxScale->Disable();
        }
        else
        {
            if( mbAdjustEnabled )
            {
                if( mbAutoWidth )
                {
                    mpFtWidth->Disable();
                    mpMtrWidth->Disable();
                    mpCbxScale->Disable();
                }
                else
                {
                    mpFtWidth->Enable();
                    mpMtrWidth->Enable();
                }
                if( mbAutoHeight )
                {
                    mpFtHeight->Disable();
                    mpMtrHeight->Disable();
                    mpCbxScale->Disable();
                }
                else
                {
                    mpFtHeight->Enable();
                    mpMtrHeight->Enable();
                }
                if( !mbAutoWidth && !mbAutoHeight )
                    mpCbxScale->Enable();
            }
            else
            {
                mpFtWidth->Enable();
                mpMtrWidth->Enable();
                mpFtHeight->Enable();
                mpMtrHeight->Enable();
                mpCbxScale->Enable();
            }
        }
    }
}

void PosSizePropertyPanel::SetPosSizeMinMax()
{
    SdrPageView* pPV = mpView->GetSdrPageView();
    if (!pPV)
        return;
    tools::Rectangle aTmpRect(mpView->GetAllMarkedRect());
    pPV->LogicToPagePos(aTmpRect);
    maRect = basegfx::B2DRange(aTmpRect.Left(), aTmpRect.Top(), aTmpRect.Right(), aTmpRect.Bottom());

    tools::Rectangle aTmpRect2(mpView->GetWorkArea());
    pPV->LogicToPagePos(aTmpRect2);
    maWorkArea = basegfx::B2DRange(aTmpRect2.Left(), aTmpRect2.Top(), aTmpRect2.Right(), aTmpRect2.Bottom());

    const Fraction aUIScale(mpView->GetModel()->GetUIScale());
    TransfrmHelper::ScaleRect( maWorkArea, aUIScale );
    TransfrmHelper::ScaleRect( maRect, aUIScale );

    const sal_uInt16 nDigits(mpMtrPosX->GetDecimalDigits());
    TransfrmHelper::ConvertRect( maWorkArea, nDigits, mePoolUnit, meDlgUnit );
    TransfrmHelper::ConvertRect( maRect, nDigits, mePoolUnit, meDlgUnit );

    double fLeft(maWorkArea.getMinX());
    double fTop(maWorkArea.getMinY());
    double fRight(maWorkArea.getMaxX());
    double fBottom(maWorkArea.getMaxY());

    // seems that sidebar defaults to top left reference point
    // and there's no way to set it to something else
    fRight  -= maRect.getWidth();
    fBottom -= maRect.getHeight();

    const double fMaxLong(static_cast<double>(MetricField::ConvertValue( LONG_MAX, 0, MapUnit::Map100thMM, meDlgUnit ) - 1));
    fLeft = std::clamp(fLeft, -fMaxLong, fMaxLong);
    fRight = std::clamp(fRight, -fMaxLong, fMaxLong);
    fTop = std::clamp(fTop, - fMaxLong, fMaxLong);
    fBottom = std::clamp(fBottom, -fMaxLong, fMaxLong);

    mpMtrPosX->SetMin(basegfx::fround64(fLeft));
    mpMtrPosX->SetFirst(basegfx::fround64(fLeft));
    mpMtrPosX->SetMax(basegfx::fround64(fRight));
    mpMtrPosX->SetLast(basegfx::fround64(fRight));
    mpMtrPosY->SetMin(basegfx::fround64(fTop));
    mpMtrPosY->SetFirst(basegfx::fround64(fTop));
    mpMtrPosY->SetMax(basegfx::fround64(fBottom));
    mpMtrPosY->SetLast(basegfx::fround64(fBottom));

    double fMaxWidth = maWorkArea.getWidth() - (maRect.getMinX() - fLeft);
    double fMaxHeight = maWorkArea.getHeight() - (maRect.getMinY() - fTop);
    mpMtrWidth->SetMax(basegfx::fround64(fMaxWidth));
    mpMtrWidth->SetLast(basegfx::fround64(fMaxWidth));
    mpMtrHeight->SetMax(basegfx::fround64(fMaxHeight));
    mpMtrHeight->SetLast(basegfx::fround64(fMaxHeight));
}

void PosSizePropertyPanel::UpdateUIScale()
{
    const Fraction aUIScale (mpView->GetModel()->GetUIScale());
    if (maUIScale != aUIScale)
    {
        // UI scale has changed.

        // Remember the new UI scale.
        maUIScale = aUIScale;

        // The content of the position and size boxes is only updated when item changes are notified.
        // Request such notifications without changing the actual item values.
        GetBindings()->Invalidate(SID_ATTR_TRANSFORM_POS_X, true);
        GetBindings()->Invalidate(SID_ATTR_TRANSFORM_POS_Y, true);
        GetBindings()->Invalidate(SID_ATTR_TRANSFORM_WIDTH, true);
        GetBindings()->Invalidate(SID_ATTR_TRANSFORM_HEIGHT, true);
    }
}


} } // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
