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
#include <sfx2/sidebar/ControlFactory.hxx>
#include "NumberFormatPropertyPanel.hxx"
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

namespace sc { namespace sidebar {

NumberFormatPropertyPanel::NumberFormatPropertyPanel(
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
  : PanelLayout(pParent,"NumberFormatPropertyPanel", "modules/scalc/ui/sidebarnumberformat.ui", rxFrame),
    maNumFormatControl(SID_NUMBER_TYPE_FORMAT, *pBindings, *this),
    maFormatControl(SID_NUMBER_FORMAT, *pBindings, *this),

    mnCategorySelected(0),
    maContext(),
    mpBindings(pBindings)
{
    get(mpLbCategory,     "category");
    get(mpTBCategory,     "numberformat");
    get(mpFtDecimals,     "decimalplaceslabel");
    get(mpEdDecimals,     "decimalplaces");
    get(mpFtDenominator,  "denominatorplaceslabel");
    get(mpEdDenominator,  "denominatorplaces");
    get(mpEdLeadZeroes,   "leadingzeroes");
    get(mpBtnNegRed,      "negativenumbersred");
    get(mpBtnThousand,    "thousandseparator");
    get(mpBtnEngineering, "engineeringnotation");

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
    mpEdDenominator.clear();
    mpEdLeadZeroes.clear();
    mpBtnNegRed.clear();
    mpBtnThousand.clear();
    mpBtnEngineering.clear();

    maNumFormatControl.dispose();
    maFormatControl.dispose();

    PanelLayout::dispose();
}

void NumberFormatPropertyPanel::Initialize()
{
    mpLbCategory->SetSelectHdl ( LINK(this, NumberFormatPropertyPanel, NumFormatSelectHdl) );
    mpLbCategory->SelectEntryPos(0);
    mpLbCategory->SetDropDownLineCount(mpLbCategory->GetEntryCount());

    Link<Edit&,void> aLink = LINK(this, NumberFormatPropertyPanel, NumFormatValueHdl);

    mpEdDecimals->SetModifyHdl( aLink );
    mpEdDenominator->SetModifyHdl( aLink );
    mpEdLeadZeroes->SetModifyHdl( aLink );

    mpBtnNegRed->SetClickHdl( LINK(this, NumberFormatPropertyPanel, NumFormatValueClickHdl) );
    mpBtnThousand->SetClickHdl( LINK(this, NumberFormatPropertyPanel, NumFormatValueClickHdl) );
    mpBtnEngineering->SetClickHdl( LINK(this, NumberFormatPropertyPanel, NumFormatValueClickHdl) );
}

IMPL_LINK_TYPED( NumberFormatPropertyPanel, NumFormatSelectHdl, ListBox&, rBox, void )
{
    const sal_Int32 nVal = rBox.GetSelectEntryPos();
    if( nVal != mnCategorySelected )
    {
        SfxUInt16Item aItem( SID_NUMBER_TYPE_FORMAT,  nVal );
        GetBindings()->GetDispatcher()->ExecuteList(SID_NUMBER_TYPE_FORMAT,
                SfxCallMode::RECORD, { &aItem });
        mnCategorySelected = nVal;
    }
}

IMPL_LINK_NOARG_TYPED( NumberFormatPropertyPanel, NumFormatValueClickHdl, Button*, void )
{
    NumFormatValueHdl(*mpEdDecimals);
}
IMPL_LINK_NOARG_TYPED( NumberFormatPropertyPanel, NumFormatValueHdl, Edit&, void )
{
    OUString    aFormat;
    OUString    sBreak = ",";
    bool        bThousand   = ( mpBtnThousand->IsVisible() && mpBtnThousand->IsEnabled() && mpBtnThousand->IsChecked() )
                           || ( mpBtnEngineering->IsVisible() && mpBtnEngineering->IsEnabled() && mpBtnEngineering->IsChecked() );
    bool        bNegRed     =  mpBtnNegRed->IsEnabled() && mpBtnNegRed->IsChecked();
    sal_uInt16  nPrecision  = (mpEdDecimals->IsEnabled() && mpEdDecimals->IsVisible())
                            ? (sal_uInt16)mpEdDecimals->GetValue()
                            : (mpEdDenominator->IsEnabled() && mpEdDenominator->IsVisible())
                                ? (sal_uInt16)mpEdDenominator->GetValue()
                                : (sal_uInt16)0;
    sal_uInt16  nLeadZeroes = (mpEdLeadZeroes->IsEnabled())
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
    GetBindings()->GetDispatcher()->ExecuteList(SID_NUMBER_FORMAT,
            SfxCallMode::RECORD, { &aItem });
}

VclPtr<vcl::Window> NumberFormatPropertyPanel::Create (
    vcl::Window* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to NumberFormatPropertyPanel::Create", nullptr, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to NumberFormatPropertyPanel::Create", nullptr, 1);
    if (pBindings == nullptr)
        throw lang::IllegalArgumentException("no SfxBindings given to NumberFormatPropertyPanel::Create", nullptr, 2);

    return  VclPtr<NumberFormatPropertyPanel>::Create(
                        pParent, rxFrame, pBindings);
}

void NumberFormatPropertyPanel::DataChanged(
    const DataChangedEvent& rEvent)
{
    (void)rEvent;
}

void NumberFormatPropertyPanel::HandleContextChange(
    const vcl::EnumContext& rContext)
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
                    nVal == 6 || // scientific also
                    nVal == 7 )  // fraction
                {
                    bool bIsScientific ( nVal == 6 );// For scientific, Thousand separator is replaced by Engineering notation
                    bool bIsFraction ( nVal == 7 );  // For fraction, Decimal places is replaced by Denominator places
                    mpBtnThousand->Show(!bIsScientific);
                    mpBtnThousand->Enable(!bIsScientific && !bIsFraction);
                    mpBtnEngineering->Show(bIsScientific);
                    mpBtnEngineering->Enable(bIsScientific);
                    mpBtnNegRed->Enable();
                    mpFtDenominator->Show(bIsFraction);
                    mpEdDenominator->Show(bIsFraction);
                    mpFtDenominator->Enable(bIsFraction);
                    mpEdDenominator->Enable(bIsFraction);
                    mpFtDecimals->Show(!bIsFraction);
                    mpEdDecimals->Show(!bIsFraction);
                    mpFtDecimals->Enable(!bIsFraction);
                    mpEdDecimals->Enable(!bIsFraction);
                    mpEdLeadZeroes->Enable();
                }
                else
                    DisableControls();
            }
            else
            {
                DisableControls();
                mpLbCategory->SetNoSelection();
                mnCategorySelected = 0;
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
                sal_Int32 nIndex = 0;
                sal_Int32 aFormat[4] = {0};
                for (sal_Int32 & rn : aFormat)
                {
                    rn = aCode.getToken(0, ',', nIndex).toInt32();
                    if (nIndex<0)
                        break;
                }
                bThousand   = static_cast<bool>(aFormat[0]);
                bNegRed     = static_cast<bool>(aFormat[1]);
                nPrecision  = static_cast<sal_uInt16>(aFormat[2]);
                nLeadZeroes = static_cast<sal_uInt16>(aFormat[3]);
            }
            else
            {
                bThousand   =    false;
                bNegRed     =    false;
                nPrecision  =    0;
                nLeadZeroes =    1;
            }
            if ( mpBtnThousand->IsVisible() )
                mpBtnThousand->Check(bThousand);
            else if ( mpBtnEngineering->IsVisible() )
                mpBtnEngineering->Check(bThousand);
            mpBtnNegRed->Check(bNegRed);
            if ( mpLbCategory->GetSelectEntryPos() == 0 )
                mpEdDecimals->SetText(""); // tdf#44399
            else if ( mpEdDecimals->IsVisible() )
                mpEdDecimals->SetValue(nPrecision);
            else if ( mpEdDenominator->IsVisible() )
                mpEdDenominator->SetValue(nPrecision);
            mpEdLeadZeroes->SetValue(nLeadZeroes);
        }
        break;
    default:
        break;
    }
}

void NumberFormatPropertyPanel::DisableControls()
{
    mpBtnEngineering->Hide();
    mpBtnThousand->Show();
    mpBtnThousand->Disable();
    mpBtnNegRed->Disable();
    mpFtDenominator->Hide();
    mpEdDenominator->Hide();
    mpFtDecimals->Show();
    mpEdDecimals->Show();
    mpFtDecimals->Disable();
    mpEdDecimals->Disable();
    mpEdLeadZeroes->Disable();
}

}} // end of namespace ::sc::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
