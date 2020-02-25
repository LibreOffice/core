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
#include <sc.hrc>
#include <scitems.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/intitem.hxx>
#include <svx/rotmodit.hxx>
#include <svtools/unitconv.hxx>
#include <unotools/localedatawrapper.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <sfx2/lokhelper.hxx>
#include <comphelper/lok.hxx>

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
      maVrtStackControl(SID_ATTR_ALIGN_STACKED, *pBindings, *this),
      maRefEdgeControl(SID_ATTR_ALIGN_LOCKPOS, *pBindings, *this),
      mbMultiDisable(false),
      maContext(),
      mpBindings(pBindings)
{
    get(mpFTLeftIndent, "leftindentlabel");
    get(mpMFLeftIndent, "leftindent");
    get(mpCBXWrapText, "wraptext");
    get(mpCBXMergeCell, "mergecells");
    get(mpFtRotate, "orientationlabel");
    get(mpMtrAngle, "orientationdegrees");
    get(mpRefEdgeBottom, "bottom");
    get(mpRefEdgeTop, "top");
    get(mpRefEdgeStd, "standard");
    get(mpCBStacked, "stacked");
    get(mpTextOrientBox , "textorientbox");

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
    mpMtrAngle.clear();
    mpCBStacked.clear();
    mpRefEdgeBottom.clear();
    mpRefEdgeTop.clear();
    mpRefEdgeStd.clear();
    mpTextOrientBox.clear();

    maAlignHorControl.dispose();
    maLeftIndentControl.dispose();
    maMergeCellControl.dispose();
    maWrapTextControl.dispose();
    maAngleControl.dispose();
    maVrtStackControl.dispose();
    maRefEdgeControl.dispose();

    PanelLayout::dispose();
}

void AlignmentPropertyPanel::Initialize()
{
    mpFTLeftIndent->Disable();
    mpMFLeftIndent->Disable();
    Link<Edit&,void> aLink = LINK(this, AlignmentPropertyPanel, MFLeftIndentMdyHdl);
    mpMFLeftIndent->SetModifyHdl ( aLink );

    mpCBXMergeCell->SetClickHdl ( LINK(this, AlignmentPropertyPanel, CBOXMergnCellClkHdl) );

    mpCBXWrapText->SetClickHdl ( LINK(this, AlignmentPropertyPanel, CBOXWrapTextClkHdl) );

    //rotation
    mpMtrAngle->SetModifyHdl(LINK( this, AlignmentPropertyPanel, AngleModifiedHdl));
    mpMtrAngle->EnableAutocomplete( false );
    mpCBStacked->SetClickHdl(LINK(this, AlignmentPropertyPanel, ClickStackHdl));

    Link<Button*,void> aLink2 = LINK(this, AlignmentPropertyPanel, ReferenceEdgeHdl);
    mpRefEdgeBottom->SetClickHdl(aLink2);
    mpRefEdgeTop->SetClickHdl(aLink2);
    mpRefEdgeStd->SetClickHdl(aLink2);

    mpMtrAngle->InsertValue(0, FieldUnit::CUSTOM);
    mpMtrAngle->InsertValue(45, FieldUnit::CUSTOM);
    mpMtrAngle->InsertValue(90, FieldUnit::CUSTOM);
    mpMtrAngle->InsertValue(135, FieldUnit::CUSTOM);
    mpMtrAngle->InsertValue(180, FieldUnit::CUSTOM);
    mpMtrAngle->InsertValue(225, FieldUnit::CUSTOM);
    mpMtrAngle->InsertValue(270, FieldUnit::CUSTOM);
    mpMtrAngle->InsertValue(315, FieldUnit::CUSTOM);
    mpMtrAngle->SetDropDownLineCount(mpMtrAngle->GetEntryCount());
}

namespace {

void eraseNode(boost::property_tree::ptree& pTree, const std::string& aValue)
{
    boost::optional<boost::property_tree::ptree&> pId;
    boost::optional<boost::property_tree::ptree&> pSubTree = pTree.get_child_optional("children");

    if (pSubTree)
    {
        boost::property_tree::ptree::iterator itFound = pSubTree.get().end();
        for (boost::property_tree::ptree::iterator it = pSubTree.get().begin(); it != pSubTree.get().end(); ++it)
        {
            pId = it->second.get_child_optional("id");
            if (pId && pId.get().get_value<std::string>("") == aValue)
            {
                itFound = it;
                break;
            }

            eraseNode(it->second, aValue);
        }

        if (itFound != pSubTree.get().end())
        {
            pSubTree.get().erase(itFound);
        }
    }
}

}

boost::property_tree::ptree AlignmentPropertyPanel::DumpAsPropertyTree()
{
    boost::property_tree::ptree aTree = PanelLayout::DumpAsPropertyTree();

    if (comphelper::LibreOfficeKit::isMobilePhone(SfxLokHelper::getView()))
    {
        eraseNode(aTree, "textorientbox");
    }

    return aTree;
}

IMPL_LINK( AlignmentPropertyPanel, ReferenceEdgeHdl, Button*, pControl, void )
{
    SvxRotateMode eMode;
    if(pControl == mpRefEdgeBottom)
        eMode = SVX_ROTATE_MODE_BOTTOM;
    else if(pControl == mpRefEdgeTop)
        eMode = SVX_ROTATE_MODE_TOP;
    else
        eMode = SVX_ROTATE_MODE_STANDARD;
    SvxRotateModeItem aItem(eMode,ATTR_ROTATE_MODE);
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_ALIGN_LOCKPOS,
            SfxCallMode::RECORD, { &aItem });
}

IMPL_LINK_NOARG( AlignmentPropertyPanel, AngleModifiedHdl, Edit&, void )
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

    FormatDegrees(fTmp);

    sal_Int64 nTmp = static_cast<sal_Int64>(fTmp)*100;
    SfxInt32Item aAngleItem( SID_ATTR_ALIGN_DEGREES,static_cast<sal_uInt32>(nTmp));

    GetBindings()->GetDispatcher()->ExecuteList(
        SID_ATTR_ALIGN_DEGREES, SfxCallMode::RECORD, { &aAngleItem });
}
IMPL_LINK_NOARG( AlignmentPropertyPanel, ClickStackHdl, Button*, void )
{
    bool bVertical = mpCBStacked->IsChecked();
    SfxBoolItem  aStackItem( SID_ATTR_ALIGN_STACKED, bVertical );
    GetBindings()->GetDispatcher()->ExecuteList(
        SID_ATTR_ALIGN_STACKED, SfxCallMode::RECORD, { &aStackItem });
}
IMPL_LINK_NOARG(AlignmentPropertyPanel, MFLeftIndentMdyHdl, Edit&, void)
{
    mpCBXWrapText->EnableTriState(false);
    sal_uInt16 nVal = static_cast<sal_uInt16>(mpMFLeftIndent->GetValue());
    SfxUInt16Item aItem( SID_ATTR_ALIGN_INDENT,  static_cast<sal_uInt16>(CalcToUnit( nVal,  MapUnit::MapTwip )) );

    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_ALIGN_INDENT,
            SfxCallMode::RECORD, { &aItem });
}

IMPL_LINK_NOARG(AlignmentPropertyPanel, CBOXMergnCellClkHdl, Button*, void)
{
    bool bState = mpCBXMergeCell->IsChecked();

    //Modified
    //SfxBoolItem aItem( FID_MERGE_TOGGLE , bState);
    //GetBindings()->GetDispatcher()->Execute(FID_MERGE_TOGGLE, SfxCallMode::RECORD, &aItem, false, 0L);
    if(bState)
        GetBindings()->GetDispatcher()->Execute(FID_MERGE_ON, SfxCallMode::RECORD);
    else
        GetBindings()->GetDispatcher()->Execute(FID_MERGE_OFF, SfxCallMode::RECORD);
    GetBindings()->Invalidate(FID_MERGE_TOGGLE,true);
    //modified end
}

IMPL_LINK_NOARG(AlignmentPropertyPanel, CBOXWrapTextClkHdl, Button*, void)
{
    bool bState = mpCBXWrapText->IsChecked();
    SfxBoolItem aItem( SID_ATTR_ALIGN_LINEBREAK , bState);
    GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_ALIGN_LINEBREAK,
            SfxCallMode::RECORD, { &aItem });
}

VclPtr<vcl::Window> AlignmentPropertyPanel::Create (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to AlignmentPropertyPanel::Create", nullptr, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to AlignmentPropertyPanel::Create", nullptr, 1);
    if (pBindings == nullptr)
        throw lang::IllegalArgumentException("no SfxBindings given to AlignmentPropertyPanel::Create", nullptr, 2);

    return  VclPtr<AlignmentPropertyPanel>::Create(
                        pParent, rxFrame, pBindings);
}

void AlignmentPropertyPanel::DataChanged(
    const DataChangedEvent&)
{}

void AlignmentPropertyPanel::HandleContextChange(
    const vcl::EnumContext& rContext)
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
    const SfxPoolItem* pState)
{
    switch(nSID)
    {
    case SID_H_ALIGNCELL:
        {
            SvxCellHorJustify meHorAlignState = SvxCellHorJustify::Standard;
            if(eState >= SfxItemState::DEFAULT && dynamic_cast<const SvxHorJustifyItem*>( pState) )
            {
                const SvxHorJustifyItem* pItem = static_cast<const SvxHorJustifyItem*>(pState);
                meHorAlignState = pItem->GetValue();
            }

            if( meHorAlignState == SvxCellHorJustify::Repeat )
            {
                mpFtRotate->Disable();
                mpMtrAngle->Disable();
            }
            else
            {
                mpFtRotate->Enable(!mbMultiDisable);
                mpMtrAngle->Enable(!mbMultiDisable);
            }

            mpFTLeftIndent->Enable( meHorAlignState == SvxCellHorJustify::Left );
            mpMFLeftIndent->Enable( meHorAlignState == SvxCellHorJustify::Left );
        }
        break;
    case SID_ATTR_ALIGN_INDENT:
        if(eState >= SfxItemState::DEFAULT && dynamic_cast<const SfxUInt16Item*>( pState) )
        {
                const SfxUInt16Item* pItem = static_cast<const SfxUInt16Item*>(pState);
                sal_uInt16 nVal = pItem->GetValue();
                mpMFLeftIndent->SetValue( CalcToPoint(nVal, MapUnit::MapTwip, 1) );
        }
        else
        {
            mpMFLeftIndent->SetValue(0);
            mpMFLeftIndent->SetText(OUString());
        }
        break;
    case FID_MERGE_TOGGLE:
        if(eState >= SfxItemState::DEFAULT && dynamic_cast<const SfxBoolItem*>( pState) )
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
            if(eState >= SfxItemState::DEFAULT && dynamic_cast<const SfxBoolItem*>( pState) )
            {
                mpCBXWrapText->EnableTriState(false);
                const SfxBoolItem* pItem = static_cast<const SfxBoolItem*>(pState);
                mpCBXWrapText->Check(pItem->GetValue());
            }
            else if(eState == SfxItemState::DONTCARE)
            {
                mpCBXWrapText->EnableTriState();
                mpCBXWrapText->SetState(TRISTATE_INDET);
            }
        }
        break;
    case SID_ATTR_ALIGN_STACKED:
        if (eState >= SfxItemState::DEFAULT)
        {
            mpCBStacked->EnableTriState(false);
            const SfxBoolItem* aStackItem = static_cast<const SfxBoolItem*>(pState);
            mbMultiDisable = aStackItem->GetValue();
            mpCBStacked->Check(mbMultiDisable);
            mpTextOrientBox->Enable(!mbMultiDisable);
        }
        else
        {
            mbMultiDisable = true;
            mpTextOrientBox->Disable();
            mpCBStacked->EnableTriState();
            mpCBStacked->SetState(TRISTATE_INDET);
        }
        break;
    case SID_ATTR_ALIGN_LOCKPOS:
        if( eState >= SfxItemState::DEFAULT)
        {
            const SvxRotateModeItem* pItem = static_cast<const SvxRotateModeItem*>(pState);
            SvxRotateMode eMode = pItem->GetValue();
            if(eMode == SVX_ROTATE_MODE_BOTTOM)
            {
                mpRefEdgeBottom->SetState(true);
                mpRefEdgeTop->SetState(false);
                mpRefEdgeStd->SetState(false);
            }
            else if(eMode == SVX_ROTATE_MODE_TOP)
            {
                mpRefEdgeBottom->SetState(false);
                mpRefEdgeStd->SetState(false);
                mpRefEdgeTop->SetState(true);
            }
            else if(eMode == SVX_ROTATE_MODE_STANDARD)
            {
                mpRefEdgeBottom->SetState(false);
                mpRefEdgeTop->SetState(false);
                mpRefEdgeStd->SetState(true);
            }
        }
        break;
    case SID_ATTR_ALIGN_DEGREES:
        if (eState >= SfxItemState::DEFAULT)
        {
            long nTmp = static_cast<const SfxInt32Item*>(pState)->GetValue();
            mpMtrAngle->SetValue( nTmp / 100);  //wj
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
        }
        break;
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
