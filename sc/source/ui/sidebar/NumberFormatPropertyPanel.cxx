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

#include <sfx2/sidebar/ResourceDefinitions.hrc>
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/sidebar/ControlFactory.hxx>
#include "NumberFormatPropertyPanel.hxx"
#include <NumberFormatPropertyPanel.hrc>
#include "sc.hrc"
#include "scresid.hxx"
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/imagemgr.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>
#include <vcl/toolbox.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>

using namespace css;
using namespace css::uno;
using ::sfx2::sidebar::Theme;

namespace sc { namespace sidebar {

NumberFormatPropertyPanel::NumberFormatPropertyPanel(
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
  : PanelLayout(pParent,"NumberFormatPropertyPanel", "modules/scalc/ui/sidebarnumberformat.ui", rxFrame),
    maThousandSeparator(ScResId(RID_SFX_STR_THOUSAND_SEP)),
    maEngineeringNotation(ScResId(RID_SFX_STR_ENGINEERING)),
    maNumFormatControl(SID_NUMBER_TYPE_FORMAT, *pBindings, *this),
    maFormatControl(SID_NUMBER_FORMAT, *pBindings, *this),

    mnCategorySelected(0),
    mxFrame(rxFrame),
    maContext(),
    mpBindings(pBindings)
{
    get(mpLbCategory,   "category");
    get(mpTBCategory,   "numberformat");
    get(mpEdDecimals,   "decimalplaces");
    get(mpEdLeadZeroes, "leadingzeroes");
    get(mpBtnNegRed,    "negativenumbersred");
    get(mpBtnThousand,  "thousandseparator");

    Initialize();
}

NumberFormatPropertyPanel::~NumberFormatPropertyPanel()
{
    disposeOnce();
}

void NumberFormatPropertyPanel::dispose()
{
    mpLbCategory.clear();
    mpTBCategory.clear();
    mpEdDecimals.clear();
    mpEdLeadZeroes.clear();
    mpBtnNegRed.clear();
    mpBtnThousand.clear();

    maNumFormatControl.dispose();
    maFormatControl.dispose();

    PanelLayout::dispose();
}

void NumberFormatPropertyPanel::Initialize()
{
    Link<> aLink = LINK(this, NumberFormatPropertyPanel, NumFormatSelectHdl);
    mpLbCategory->SetSelectHdl ( aLink );
    mpLbCategory->SelectEntryPos(0);
    mpLbCategory->SetAccessibleName(OUString( "Category"));
    mpLbCategory->SetDropDownLineCount(mpLbCategory->GetEntryCount());

    aLink = LINK(this, NumberFormatPropertyPanel, NumFormatValueHdl);

    mpEdDecimals->SetModifyHdl( aLink );
    mpEdLeadZeroes->SetModifyHdl( aLink );
    mpEdDecimals->SetAccessibleName(OUString( "Decimal Places"));
    mpEdLeadZeroes->SetAccessibleName(OUString( "Leading Zeroes"));

    mpBtnNegRed->SetClickHdl( LINK(this, NumberFormatPropertyPanel, NumFormatValueClickHdl) );
    mpBtnThousand->SetClickHdl( LINK(this, NumberFormatPropertyPanel, NumFormatValueClickHdl) );

    mpTBCategory->SetAccessibleRelationLabeledBy(mpTBCategory);
}

IMPL_LINK( NumberFormatPropertyPanel, NumFormatSelectHdl, ListBox*, pBox )
{
    const sal_Int32 nVal = pBox->GetSelectEntryPos();
    if( nVal != mnCategorySelected )
    {
        SfxUInt16Item aItem( SID_NUMBER_TYPE_FORMAT,  nVal );
        GetBindings()->GetDispatcher()->Execute(SID_NUMBER_TYPE_FORMAT, SfxCallMode::RECORD, &aItem, 0L);
        mnCategorySelected = nVal;
    }
    return 0L;
}

IMPL_LINK_NOARG_TYPED( NumberFormatPropertyPanel, NumFormatValueClickHdl, Button*, void )
{
    NumFormatValueHdl(nullptr);
}
IMPL_LINK_NOARG( NumberFormatPropertyPanel, NumFormatValueHdl )
{
    OUString      aFormat;
    OUString      sBreak = ",";
    bool          bThousand     =    mpBtnThousand->IsEnabled()
        && mpBtnThousand->IsChecked();
    bool          bNegRed       =    mpBtnNegRed->IsEnabled()
        && mpBtnNegRed->IsChecked();
    sal_uInt16        nPrecision    = (mpEdDecimals->IsEnabled())
        ? (sal_uInt16)mpEdDecimals->GetValue()
        : (sal_uInt16)0;
    sal_uInt16        nLeadZeroes   = (mpEdLeadZeroes->IsEnabled())
        ? (sal_uInt16)mpEdLeadZeroes->GetValue()
        : (sal_uInt16)0;

    OUString sThousand = OUString::number(static_cast<sal_Int32>(bThousand));
    OUString sNegRed = OUString::number(static_cast<sal_Int32>(bNegRed));
    OUString sPrecision = OUString::number(nPrecision);
    OUString sLeadZeroes = OUString::number(nLeadZeroes);

    aFormat += sThousand;
    aFormat += sBreak;
    aFormat += sNegRed;
    aFormat += sBreak;
    aFormat += sPrecision;
    aFormat += sBreak;
    aFormat += sLeadZeroes;
    aFormat += sBreak;

    SfxStringItem aItem( SID_NUMBER_FORMAT,  aFormat );
    GetBindings()->GetDispatcher()->Execute(SID_NUMBER_FORMAT, SfxCallMode::RECORD, &aItem, 0L);
    return 0L;
}

VclPtr<vcl::Window> NumberFormatPropertyPanel::Create (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
{
    if (pParent == NULL)
        throw lang::IllegalArgumentException("no parent Window given to NumberFormatPropertyPanel::Create", NULL, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to NumberFormatPropertyPanel::Create", NULL, 1);
    if (pBindings == NULL)
        throw lang::IllegalArgumentException("no SfxBindings given to NumberFormatPropertyPanel::Create", NULL, 2);

    return  VclPtr<NumberFormatPropertyPanel>::Create(
                        pParent, rxFrame, pBindings);
}

void NumberFormatPropertyPanel::DataChanged(
    const DataChangedEvent& rEvent)
{
    (void)rEvent;
}

void NumberFormatPropertyPanel::HandleContextChange(
    const ::sfx2::sidebar::EnumContext& rContext)
{
    if(maContext == rContext)
    {
        // Nothing to do.
        return;
    }

    maContext = rContext;
}

void NumberFormatPropertyPanel::NotifyItemUpdate(
    sal_uInt16 nSID,
    SfxItemState eState,
    const SfxPoolItem* pState,
    const bool bIsEnabled)
{
    (void)bIsEnabled;

    switch(nSID)
    {
    case SID_NUMBER_TYPE_FORMAT:
        {
            if( eState >= SfxItemState::DEFAULT)
            {
                const SfxInt16Item* pItem = static_cast<const SfxInt16Item*>(pState);
                sal_uInt16 nVal = pItem->GetValue();
                mnCategorySelected = nVal;
                mpLbCategory->SelectEntryPos(nVal);
                if( nVal < 4 ||  // General, Number, Percent and Currency
                    nVal == 6 )  // scientific also
                {
                    mpBtnThousand->Enable();
                    mpBtnNegRed->Enable();
                    mpEdDecimals->Enable();
                    mpEdLeadZeroes->Enable();
                }
                else
                {
                    mpBtnThousand->Disable();
                    mpBtnNegRed->Disable();
                    mpEdDecimals->Disable();
                    mpEdLeadZeroes->Disable();
                }
                if( nVal == 6 ) // For scientific, Thousand separator is replaced by Engineering notation
                    mpBtnThousand->SetText( maEngineeringNotation );
                else
                    mpBtnThousand->SetText( maThousandSeparator );
            }
            else
            {
                mpLbCategory->SetNoSelection();
                mnCategorySelected = 0;
                mpBtnThousand->Disable();
                mpBtnNegRed->Disable();
                mpEdDecimals->Disable();
                mpEdLeadZeroes->Disable();
            }
        }
        break;
    case SID_NUMBER_FORMAT:
        {
            bool          bThousand     =    false;
            bool          bNegRed       =    false;
            sal_uInt16        nPrecision    =    0;
            sal_uInt16        nLeadZeroes   =    0;
            if( eState >= SfxItemState::DEFAULT)
            {
                const SfxStringItem* pItem = static_cast<const SfxStringItem*>(pState);
                OUString aCode = pItem->GetValue();
                sal_uInt16 aLen = aCode.getLength();
                OUString* sFormat = new OUString[4];
                OUString  sTmpStr = "";
                sal_uInt16 nCount = 0;
                sal_uInt16 nStrCount = 0;
                while( nCount < aLen )
                {
                    sal_Unicode cChar = aCode[nCount];
                    if(cChar == ',')
                    {
                        sFormat[nStrCount] = sTmpStr;
                        sTmpStr.clear();
                        nStrCount++;
                    }
                    else
                    {
                        sTmpStr += OUString(cChar);
                    }
                    nCount++;
                }
                bThousand   =    sFormat[0].toInt32();
                bNegRed     =    sFormat[1].toInt32();
                nPrecision  =    (sal_uInt16)sFormat[2].toInt32();
                nLeadZeroes =    (sal_uInt16)sFormat[3].toInt32();
                delete[] sFormat;
            }
            else
            {
                bThousand   =    false;
                bNegRed     =    false;
                nPrecision  =    0;
                nLeadZeroes =    1;
            }
            mpBtnThousand->Check(bThousand);
            mpBtnNegRed->Check(bNegRed);
            if ( mpLbCategory->GetSelectEntryPos() == 0 )
                mpEdDecimals->SetText(""); // tdf#44399
            else
                mpEdDecimals->SetValue(nPrecision);
            mpEdLeadZeroes->SetValue(nLeadZeroes);
        }
    default:
        ;
    }
}

}} // end of namespace ::sc::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
