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

#include "NumberFormatPropertyPanel.hxx"
#include <sc.hrc>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

using namespace css;
using namespace css::uno;

namespace sc::sidebar {

NumberFormatPropertyPanel::NumberFormatPropertyPanel(
    weld::Widget* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
    : PanelLayout(pParent,"NumberFormatPropertyPanel", "modules/scalc/ui/sidebarnumberformat.ui")
    , mxLbCategory(m_xBuilder->weld_combo_box("numberformatcombobox"))
    , mxTBCategory(m_xBuilder->weld_toolbar("numberformat"))
    , mxCategoryDispatch(new ToolbarUnoDispatcher(*mxTBCategory, *m_xBuilder, rxFrame))
    , mxFtDecimals(m_xBuilder->weld_label("decimalplaceslabel"))
    , mxEdDecimals(m_xBuilder->weld_spin_button("decimalplaces"))
    , mxFtDenominator(m_xBuilder->weld_label("denominatorplaceslabel"))
    , mxEdDenominator(m_xBuilder->weld_spin_button("denominatorplaces"))
    , mxFtLeadZeroes(m_xBuilder->weld_label("leadingzeroeslabel"))
    , mxEdLeadZeroes(m_xBuilder->weld_spin_button("leadingzeroes"))
    , mxBtnNegRed(m_xBuilder->weld_check_button("negativenumbersred"))
    , mxBtnThousand(m_xBuilder->weld_check_button("thousandseparator"))
    , mxBtnEngineering(m_xBuilder->weld_check_button("engineeringnotation"))
    , maNumFormatControl(SID_NUMBER_TYPE_FORMAT, *pBindings, *this)
    , maFormatControl(SID_NUMBER_FORMAT, *pBindings, *this)
    , mnCategorySelected(0)
    , maContext()
    , mpBindings(pBindings)
{
    Initialize();
}

NumberFormatPropertyPanel::~NumberFormatPropertyPanel()
{
    mxLbCategory.reset();
    mxCategoryDispatch.reset();
    mxTBCategory.reset();
    mxFtDecimals.reset();
    mxEdDecimals.reset();
    mxFtDenominator.reset();
    mxEdDenominator.reset();
    mxFtLeadZeroes.reset();
    mxEdLeadZeroes.reset();
    mxBtnNegRed.reset();
    mxBtnThousand.reset();
    mxBtnEngineering.reset();

    maNumFormatControl.dispose();
    maFormatControl.dispose();
}

void NumberFormatPropertyPanel::Initialize()
{
    mxLbCategory->connect_changed( LINK(this, NumberFormatPropertyPanel, NumFormatSelectHdl) );
    mxLbCategory->set_active(0);

    Link<weld::SpinButton&,void> aLink = LINK(this, NumberFormatPropertyPanel, NumFormatValueHdl);

    mxEdDecimals->connect_value_changed( aLink );
    mxEdDenominator->connect_value_changed( aLink );
    mxEdLeadZeroes->connect_value_changed( aLink );

    mxBtnNegRed->connect_toggled( LINK(this, NumberFormatPropertyPanel, NumFormatValueClickHdl) );
    mxBtnThousand->connect_toggled( LINK(this, NumberFormatPropertyPanel, NumFormatValueClickHdl) );
    mxBtnEngineering->connect_toggled( LINK(this, NumberFormatPropertyPanel, NumFormatValueClickHdl) );
}

IMPL_LINK( NumberFormatPropertyPanel, NumFormatSelectHdl, weld::ComboBox&, rBox, void )
{
    const sal_Int32 nVal = rBox.get_active();
    if( nVal != mnCategorySelected )
    {
        SfxUInt16Item aItem( SID_NUMBER_TYPE_FORMAT,  nVal );
        GetBindings()->GetDispatcher()->ExecuteList(SID_NUMBER_TYPE_FORMAT,
                SfxCallMode::RECORD, { &aItem });
        mnCategorySelected = nVal;
    }
}

IMPL_LINK_NOARG( NumberFormatPropertyPanel, NumFormatValueClickHdl, weld::ToggleButton&, void )
{
    NumFormatValueHdl(*mxEdDecimals);
}

IMPL_LINK_NOARG( NumberFormatPropertyPanel, NumFormatValueHdl, weld::SpinButton&, void )
{
    OUString    aFormat;
    OUString    sBreak = ",";
    bool        bThousand   = ( mxBtnThousand->get_visible() && mxBtnThousand->get_sensitive() && mxBtnThousand->get_active() )
                           || ( mxBtnEngineering->get_visible() && mxBtnEngineering->get_sensitive() && mxBtnEngineering->get_active() );
    bool        bNegRed     =  mxBtnNegRed->get_sensitive() && mxBtnNegRed->get_active();
    sal_uInt16  nPrecision  = (mxEdDecimals->get_sensitive() && mxEdDecimals->get_visible())
                            ? static_cast<sal_uInt16>(mxEdDecimals->get_value())
                            : (mxEdDenominator->get_sensitive() && mxEdDenominator->get_visible())
                                ? static_cast<sal_uInt16>(mxEdDenominator->get_value())
                                : sal_uInt16(0);
    sal_uInt16  nLeadZeroes = (mxEdLeadZeroes->get_sensitive())
                            ? static_cast<sal_uInt16>(mxEdLeadZeroes->get_value())
                            : sal_uInt16(0);

    OUString sThousand = OUString::number(static_cast<sal_Int32>(bThousand));
    OUString sNegRed = OUString::number(static_cast<sal_Int32>(bNegRed));
    OUString sPrecision = OUString::number(nPrecision);
    OUString sLeadZeroes = OUString::number(nLeadZeroes);

    aFormat += sThousand +
        sBreak +
        sNegRed +
        sBreak +
        sPrecision +
        sBreak +
        sLeadZeroes +
        sBreak;

    SfxStringItem aItem( SID_NUMBER_FORMAT,  aFormat );
    GetBindings()->GetDispatcher()->ExecuteList(SID_NUMBER_FORMAT,
            SfxCallMode::RECORD, { &aItem });
}

std::unique_ptr<PanelLayout> NumberFormatPropertyPanel::Create (
    weld::Widget* pParent,
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
{
    if (pParent == nullptr)
        throw lang::IllegalArgumentException("no parent Window given to NumberFormatPropertyPanel::Create", nullptr, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException("no XFrame given to NumberFormatPropertyPanel::Create", nullptr, 1);
    if (pBindings == nullptr)
        throw lang::IllegalArgumentException("no SfxBindings given to NumberFormatPropertyPanel::Create", nullptr, 2);

    return std::make_unique<NumberFormatPropertyPanel>(pParent, rxFrame, pBindings);
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
    const SfxPoolItem* pState)
{
    switch(nSID)
    {
    case SID_NUMBER_TYPE_FORMAT:
        {
            if( eState >= SfxItemState::DEFAULT)
            {
                const SfxInt16Item* pItem = static_cast<const SfxInt16Item*>(pState);
                sal_uInt16 nVal = pItem->GetValue();
                mnCategorySelected = nVal;
                mxLbCategory->set_active(nVal);
                if (nVal < 8 &&  // General, Number, Percent, Currency, Time, Scientific, Fraction
                    nVal != 4 )  // not Date
                {
                    bool bIsScientific ( nVal == 6 );// For scientific, Thousand separator is replaced by Engineering notation
                    bool bIsFraction ( nVal == 7 );  // For fraction, Decimal places is replaced by Denominator places
                    bool bIsTime ( nVal == 5 );  // For Time, Decimal places and NegRed available
                    mxBtnThousand->set_visible( !bIsScientific );
                    mxBtnThousand->set_sensitive( !bIsScientific && !bIsTime );
                    mxBtnThousand->set_active(false);
                    mxBtnEngineering->set_visible(bIsScientific);
                    mxBtnEngineering->set_sensitive(bIsScientific);
                    mxBtnEngineering->set_active(false);
                    mxBtnNegRed->set_sensitive(true);
                    mxFtDenominator->set_visible(bIsFraction);
                    mxEdDenominator->set_visible(bIsFraction);
                    mxFtDenominator->set_sensitive(bIsFraction);
                    mxEdDenominator->set_sensitive(bIsFraction);
                    mxFtDecimals->set_visible(!bIsFraction);
                    mxEdDecimals->set_visible(!bIsFraction);
                    mxFtDecimals->set_sensitive(!bIsFraction);
                    mxEdDecimals->set_sensitive(!bIsFraction);
                    mxFtLeadZeroes->set_sensitive( !bIsTime );
                    mxEdLeadZeroes->set_sensitive( !bIsTime );
                }
                else
                    DisableControls();
            }
            else
            {
                DisableControls();
                mxLbCategory->set_active(-1);
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
                const OUString& aCode = pItem->GetValue();
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
            if ( mxBtnThousand->get_visible() )
                mxBtnThousand->set_active(bThousand);
            else if ( mxBtnEngineering->get_visible() )
                mxBtnEngineering->set_active(bThousand);
            mxBtnNegRed->set_active(bNegRed);
            if ( mxLbCategory->get_active() == 0 )
                mxEdDecimals->set_text(""); // tdf#44399
            else if ( mxEdDecimals->get_visible() )
                mxEdDecimals->set_value(nPrecision);
            else if ( mxEdDenominator->get_visible() )
                mxEdDenominator->set_value(nPrecision);
            mxEdLeadZeroes->set_value(nLeadZeroes);
        }
        break;
    default:
        break;
    }
}

void NumberFormatPropertyPanel::DisableControls()
{
    mxBtnEngineering->hide();
    mxBtnThousand->show();
    mxBtnThousand->set_sensitive(false);
    mxBtnNegRed->set_sensitive(false);
    mxFtDenominator->hide();
    mxEdDenominator->hide();
    mxFtDecimals->show();
    mxEdDecimals->show();
    mxFtDecimals->set_sensitive(false);
    mxEdDecimals->set_sensitive(false);
    mxFtLeadZeroes->set_sensitive(false);
    mxEdLeadZeroes->set_sensitive(false);
}

} // end of namespace ::sc::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
