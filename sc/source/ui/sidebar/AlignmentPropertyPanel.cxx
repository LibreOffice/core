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

#include "AlignmentPropertyPanel.hxx"
#include <editeng/justifyitem.hxx>
#include "sc.hrc"
#include "scresid.hxx"
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/algitem.hxx>
#include <svx/dlgutil.hxx>
#include <vcl/toolbox.hxx>
#include <svx/sidebar/SidebarDialControl.hxx>

using namespace css;
using namespace css::uno;

// namespace open

namespace sc { namespace sidebar {

AlignmentPropertyPanel::AlignmentPropertyPanel(
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
    : PanelLayout(pParent, "AlignmentPropertyPanel", "modules/scalc/ui/sidebaralignment.ui", rxFrame),
      maAlignHorControl(SID_H_ALIGNCELL, *pBindings, *this),
      maLeftIndentControl(SID_ATTR_ALIGN_INDENT, *pBindings, *this),
      maMergeCellControl(FID_MERGE_TOGGLE, *pBindings, *this),
      maWrapTextControl(SID_ATTR_ALIGN_LINEBREAK, *pBindings, *this),
      maAngleControl(SID_ATTR_ALIGN_DEGREES, *pBindings, *this),
      maStackControl(SID_ATTR_ALIGN_STACKED, *pBindings, *this),
      mbMultiDisable(false),
      mxFrame(rxFrame),
      maContext(),
      mpBindings(pBindings)
{
    get(mpFTLeftIndent, "leftindentlabel");
    get(mpMFLeftIndent, "leftindent");
    get(mpCBXWrapText, "wraptext");
    get(mpCBXMergeCell, "mergecells");
    get(mpFtRotate, "orientationlabel");
    get(mpCtrlDial, "orientationcontrol");
    get(mpMtrAngle, "orientationdegrees");
    get(mpCbStacked, "verticallystacked");

    Initialize();

    mpFTLeftIndent->SetBackground(Wallpaper());
    mpFtRotate->SetBackground(Wallpaper());
}

AlignmentPropertyPanel::~AlignmentPropertyPanel()
{
    disposeOnce();
}

void AlignmentPropertyPanel::dispose()
{
    mpFTLeftIndent.clear();
    mpMFLeftIndent.clear();
    mpCBXWrapText.clear();
    mpCBXMergeCell.clear();
    mpFtRotate.clear();
    mpCtrlDial.clear();
    mpMtrAngle.clear();
    mpCbStacked.clear();

    maAlignHorControl.dispose();
    maLeftIndentControl.dispose();
    maMergeCellControl.dispose();
    maWrapTextControl.dispose();
    maAngleControl.dispose();
    maStackControl.dispose();

    PanelLayout::dispose();
}

void AlignmentPropertyPanel::Initialize()
{
    mpFTLeftIndent->Disable();
    mpMFLeftIndent->Disable();
    mpMFLeftIndent->SetAccessibleName(OUString( "Left Indent"));    //wj acc
    Link<> aLink = LINK(this, AlignmentPropertyPanel, MFLeftIndentMdyHdl);
    mpMFLeftIndent->SetModifyHdl ( aLink );

    aLink = LINK(this, AlignmentPropertyPanel, CBOXMergnCellClkHdl);
    mpCBXMergeCell->SetClickHdl ( aLink );

    aLink = LINK(this, AlignmentPropertyPanel, CBOXWrapTextClkHdl);
    mpCBXWrapText->SetClickHdl ( aLink );

    //rotation control
    mpCtrlDial->SetAccessibleName(OUString( "Text Orientation"));   //wj acc
    mpCtrlDial->SetModifyHdl(LINK( this, AlignmentPropertyPanel, RotationHdl));

    //rotation
    mpMtrAngle->SetAccessibleName(OUString( "Text Orientation"));   //wj acc
    mpMtrAngle->SetModifyHdl(LINK( this, AlignmentPropertyPanel, AngleModifiedHdl));
    mpMtrAngle->EnableAutocomplete( false );

    //Vertical stacked
    mpCbStacked->SetClickHdl( LINK( this, AlignmentPropertyPanel, ClickStackHdl ) );

    mpMtrAngle->InsertValue(0, FUNIT_CUSTOM);
    mpMtrAngle->InsertValue(45, FUNIT_CUSTOM);
    mpMtrAngle->InsertValue(90, FUNIT_CUSTOM);
    mpMtrAngle->InsertValue(135, FUNIT_CUSTOM);
    mpMtrAngle->InsertValue(180, FUNIT_CUSTOM);
    mpMtrAngle->InsertValue(225, FUNIT_CUSTOM);
    mpMtrAngle->InsertValue(270, FUNIT_CUSTOM);
    mpMtrAngle->InsertValue(315, FUNIT_CUSTOM);
    mpMtrAngle->SetDropDownLineCount(mpMtrAngle->GetEntryCount());

    mpMFLeftIndent->SetAccessibleRelationLabeledBy(mpFTLeftIndent);
    mpMtrAngle->SetAccessibleRelationLabeledBy(mpFtRotate);
}

IMPL_LINK_NOARG( AlignmentPropertyPanel, AngleModifiedHdl )
{
    OUString sTmp = mpMtrAngle->GetText();
    if (sTmp.isEmpty())
        return 0;
    sal_Unicode nChar = sTmp[0];
    if( nChar == '-' )
    {
        if (sTmp.getLength() < 2)
            return 0;
        nChar = sTmp[1];
    }

    if( (nChar < '0') || (nChar > '9') )
        return 0;

    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
    const sal_Unicode cSep = rLocaleWrapper.getNumDecimalSep()[0];

    // Do not check that the entire string was parsed up to its end, there may
    // be a degree symbol following the number. Note that this also means that
    // the number recognized just stops at any non-matching character.
    /* TODO: we could check for the degree symbol stop if there are no other
     * cases with different symbol characters in any language? */
    rtl_math_ConversionStatus eStatus;
    double fTmp = rtl::math::stringToDouble( sTmp, cSep, 0, &eStatus);
    if (eStatus != rtl_math_ConversionStatus_Ok)
        return 0;

    FormatDegrees(fTmp);

    sal_Int64 nTmp = (sal_Int64)fTmp*100;
    SfxInt32Item aAngleItem( SID_ATTR_ALIGN_DEGREES,(sal_uInt32) nTmp);

    GetBindings()->GetDispatcher()->Execute(
        SID_ATTR_ALIGN_DEGREES, SfxCallMode::RECORD, &aAngleItem, 0L );
    return 0;
}

IMPL_LINK_NOARG( AlignmentPropertyPanel, RotationHdl )
{
    sal_Int32 nTmp = mpCtrlDial->GetRotation();
    SfxInt32Item aAngleItem( SID_ATTR_ALIGN_DEGREES,(sal_uInt32) nTmp);

    GetBindings()->GetDispatcher()->Execute(
        SID_ATTR_ALIGN_DEGREES, SfxCallMode::RECORD, &aAngleItem, 0L );

    return 0;
}

IMPL_LINK_NOARG( AlignmentPropertyPanel, ClickStackHdl )
{
    bool bVertical = mpCbStacked->IsChecked();
    SfxBoolItem  aStackItem( SID_ATTR_ALIGN_STACKED, bVertical );
    GetBindings()->GetDispatcher()->Execute(
        SID_ATTR_ALIGN_STACKED, SfxCallMode::RECORD, &aStackItem, 0L );
    return 0;
}

IMPL_LINK_NOARG(AlignmentPropertyPanel, MFLeftIndentMdyHdl)
{
    mpCBXWrapText->EnableTriState(false);
    sal_uInt16 nVal = (sal_uInt16)mpMFLeftIndent->GetValue();
    SfxUInt16Item aItem( SID_ATTR_ALIGN_INDENT,  (sal_uInt16)CalcToUnit( nVal,  SFX_MAPUNIT_TWIP ) );

    GetBindings()->GetDispatcher()->Execute(SID_ATTR_ALIGN_INDENT, SfxCallMode::RECORD, &aItem, 0L);
    return 0L;
}

IMPL_LINK_NOARG(AlignmentPropertyPanel, CBOXMergnCellClkHdl)
{
    bool bState = mpCBXMergeCell->IsChecked();

    //Modified
    //SfxBoolItem aItem( FID_MERGE_TOGGLE , bState);
    //GetBindings()->GetDispatcher()->Execute(FID_MERGE_TOGGLE, SfxCallMode::RECORD, &aItem, false, 0L);
    if(bState)
        GetBindings()->GetDispatcher()->Execute(FID_MERGE_ON, SfxCallMode::RECORD);
    else
        GetBindings()->GetDispatcher()->Execute(FID_MERGE_OFF, SfxCallMode::RECORD);
    GetBindings()->Invalidate(FID_MERGE_TOGGLE,true,false);
    //modified end

    return 0;
}

IMPL_LINK_NOARG(AlignmentPropertyPanel, CBOXWrapTextClkHdl)
{
    bool bState = mpCBXWrapText->IsChecked();
    SfxBoolItem aItem( SID_ATTR_ALIGN_LINEBREAK , bState);
    GetBindings()->GetDispatcher()->Execute(SID_ATTR_ALIGN_LINEBREAK, SfxCallMode::RECORD, &aItem, 0L);
    return 0;
}

VclPtr<vcl::Window> AlignmentPropertyPanel::Create (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
{
    if (pParent == NULL)
        throw lang::IllegalArgumentException("no parent Window given to AlignmentPropertyPanel::Create", NULL, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to AlignmentPropertyPanel::Create", NULL, 1);
    if (pBindings == NULL)
        throw lang::IllegalArgumentException("no SfxBindings given to AlignmentPropertyPanel::Create", NULL, 2);

    return  VclPtr<AlignmentPropertyPanel>::Create(
                        pParent, rxFrame, pBindings);
}

void AlignmentPropertyPanel::DataChanged(
    const DataChangedEvent& rEvent)
{
    (void)rEvent;
}

void AlignmentPropertyPanel::HandleContextChange(
    const ::sfx2::sidebar::EnumContext& rContext)
{
    if (maContext == rContext)
    {
        // Nothing to do.
        return;
    }

    maContext = rContext;
}

void AlignmentPropertyPanel::NotifyItemUpdate(
    sal_uInt16 nSID,
    SfxItemState eState,
    const SfxPoolItem* pState,
    const bool bIsEnabled)
{
    (void)bIsEnabled;

    switch(nSID)
    {
    case SID_H_ALIGNCELL:
        {
            SvxCellHorJustify meHorAlignState = SVX_HOR_JUSTIFY_STANDARD;
            if(eState >= SfxItemState::DEFAULT && pState && pState->ISA(SvxHorJustifyItem) )
            {
                const SvxHorJustifyItem* pItem = static_cast<const SvxHorJustifyItem*>(pState);
                meHorAlignState = (SvxCellHorJustify)pItem->GetValue();
            }

            if( meHorAlignState == SVX_HOR_JUSTIFY_REPEAT )
            {
                mpFtRotate->Disable();
                mpCtrlDial->Disable();
                mpMtrAngle->Disable();
            }
            else
            {
                mpFtRotate->Enable(!mbMultiDisable);
                mpCtrlDial->Enable(!mbMultiDisable);
                mpMtrAngle->Enable(!mbMultiDisable);
            }

            mpCbStacked->Enable( meHorAlignState != SVX_HOR_JUSTIFY_REPEAT );
            mpFTLeftIndent->Enable( meHorAlignState == SVX_HOR_JUSTIFY_LEFT );
            mpMFLeftIndent->Enable( meHorAlignState == SVX_HOR_JUSTIFY_LEFT );
        }
        break;
    case SID_ATTR_ALIGN_INDENT:
        if(eState >= SfxItemState::DEFAULT && pState && pState->ISA(SfxUInt16Item) )
        {
                const SfxUInt16Item* pItem = static_cast<const SfxUInt16Item*>(pState);
                sal_uInt16 nVal = pItem->GetValue();
                mpMFLeftIndent->SetValue( CalcToPoint(nVal, SFX_MAPUNIT_TWIP, 1) );
        }
        else
        {
            mpMFLeftIndent->SetValue(0);
            mpMFLeftIndent->SetText(OUString());
        }
        break;
    case FID_MERGE_TOGGLE:
        if(eState >= SfxItemState::DEFAULT && pState && pState->ISA(SfxBoolItem) )
        {
            mpCBXMergeCell->Enable();
            const SfxBoolItem* pItem = static_cast<const SfxBoolItem*>(pState);
            mpCBXMergeCell->Check(pItem->GetValue());
        }
        else
        {
            mpCBXMergeCell->Check(false);
            mpCBXMergeCell->Disable();
        }
        break;

    case SID_ATTR_ALIGN_LINEBREAK:
        if(eState == SfxItemState::DISABLED)
        {
            mpCBXWrapText->EnableTriState(false);
            mpCBXWrapText->Check(false);
            mpCBXWrapText->Disable();
        }
        else
        {
            mpCBXWrapText->Enable();
            if(eState >= SfxItemState::DEFAULT && pState && pState->ISA(SfxBoolItem) )
            {
                mpCBXWrapText->EnableTriState(false);
                const SfxBoolItem* pItem = static_cast<const SfxBoolItem*>(pState);
                mpCBXWrapText->Check(pItem->GetValue());
            }
            else if(eState == SfxItemState::DONTCARE)
            {
                mpCBXWrapText->EnableTriState(true);
                mpCBXWrapText->SetState(TRISTATE_INDET);
            }
        }
        break;
    case SID_ATTR_ALIGN_DEGREES:
        if (eState >= SfxItemState::DEFAULT)
        {
            long nTmp = static_cast<const SfxInt32Item*>(pState)->GetValue();
            mpMtrAngle->SetValue( nTmp / 100);  //wj
            mpCtrlDial->SetRotation( nTmp );
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
                case 31500:
                    mpMtrAngle->SelectEntryPos(7);
            }
        }
        else
        {
            mpMtrAngle->SetText( OUString() );
            mpCtrlDial->SetRotation( 0 );
        }
        break;
    case SID_ATTR_ALIGN_STACKED:
        if (eState >= SfxItemState::DEFAULT)
        {
            mpCbStacked->EnableTriState(false);
            const SfxBoolItem* aStackItem = static_cast<const SfxBoolItem*>(pState);
            mbMultiDisable = aStackItem->GetValue();
            mpCbStacked->Check(mbMultiDisable);
            mpFtRotate->Enable(!mbMultiDisable);
            mpMtrAngle->Enable(!mbMultiDisable);
            mpCtrlDial->Enable(!mbMultiDisable);
        }
        else
        {
            mbMultiDisable = true;
            mpFtRotate->Disable();
            mpMtrAngle->Disable();
            mpCtrlDial->Disable();
            mpCbStacked->EnableTriState(true);
            mpCbStacked->SetState(TRISTATE_INDET);
        }
    }
}

void AlignmentPropertyPanel::FormatDegrees(double& dTmp)
{
    while(dTmp<0)
        dTmp += 360;
    while (dTmp > 359)  //modify
        dTmp = 359;
}

// namespace close

}} // end of namespace ::sc::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
