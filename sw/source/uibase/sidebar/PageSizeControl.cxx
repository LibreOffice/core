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

#include <memory>
#include "PageSizeControl.hxx"

#include <cmdid.h>
#include <swtypes.hxx>
#include <svx/svxids.hrc>
#include <svx/pageitem.hxx>
#include <svx/sidebar/ValueSetWithTextControl.hxx>

#include <unotools/localedatawrapper.hxx>
#include <rtl/character.hxx>
#include <editeng/paperinf.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>

#include <vcl/settings.hxx>
#include <svl/itempool.hxx>
#include <svl/intitem.hxx>
#include <editeng/sizeitem.hxx>

namespace
{
    FieldUnit lcl_GetFieldUnit()
    {
        FieldUnit eUnit = FieldUnit::INCH;
        const SfxPoolItem* pItem = nullptr;
        SfxItemState eState = SfxViewFrame::Current()->GetBindings().GetDispatcher()->QueryState( SID_ATTR_METRIC, pItem );
        if ( pItem && eState >= SfxItemState::DEFAULT )
        {
            eUnit = static_cast<FieldUnit>(static_cast<const SfxUInt16Item*>(pItem)->GetValue());
        }
        else
        {
            return SfxModule::GetCurrentFieldUnit();
        }

        return eUnit;
    }

    MapUnit lcl_GetUnit()
    {
        SfxItemPool &rPool = SfxGetpApp()->GetPool();
        sal_uInt16 nWhich = rPool.GetWhich( SID_ATTR_PAGE_SIZE );
        return rPool.GetMetric( nWhich );
    }
}

namespace sw { namespace sidebar {

PageSizeControl::PageSizeControl( sal_uInt16 nId, vcl::Window* pParent )
    : SfxPopupWindow( nId, pParent, "PageSizeControl", "modules/swriter/ui/pagesizecontrol.ui" )
    , maPaperList()
{
    get(maMoreButton, "moreoptions");
    get(maContainer, "container");
    mpSizeValueSet = VclPtr<svx::sidebar::ValueSetWithTextControl>::Create( maContainer.get(), WB_BORDER );
    maWidthHeightField = VclPtr<MetricField>::Create( maContainer.get(), 0 );
    maWidthHeightField->Hide();
    maWidthHeightField->SetUnit(FieldUnit::CM);
    maWidthHeightField->SetMax(9999);
    maWidthHeightField->SetDecimalDigits(2);
    maWidthHeightField->SetSpinSize(10);
    maWidthHeightField->SetLast(9999);
    SetFieldUnit( *maWidthHeightField, lcl_GetFieldUnit() );

    maPaperList.push_back( PAPER_A3 );
    maPaperList.push_back( PAPER_A4 );
    maPaperList.push_back( PAPER_A5 );
    maPaperList.push_back( PAPER_B4_ISO );
    maPaperList.push_back( PAPER_B5_ISO );
    maPaperList.push_back( PAPER_ENV_C5 );
    maPaperList.push_back( PAPER_LETTER );
    maPaperList.push_back( PAPER_LEGAL );

    mpSizeValueSet->SetStyle( mpSizeValueSet->GetStyle() | WB_3DLOOK | WB_NO_DIRECTSELECT );
    mpSizeValueSet->SetColor( GetSettings().GetStyleSettings().GetMenuColor() );

    sal_uInt16 nSelectedItem = 0;
    {
        OUString aMetricStr;
        {
            const OUString aText = maWidthHeightField->GetText();
            for (short i = aText.getLength() - 1; i >= 0; i--)
            {
                sal_Unicode c = aText[i];
                if ( rtl::isAsciiAlpha(c) || (c == '\'') || (c == '\"') || (c == '%') )
                {
                    aMetricStr = OUStringLiteral1(c) + aMetricStr;
                }
                else
                {
                    if (!aMetricStr.isEmpty())
                    {
                        break;
                    }
                }
            }
        }

        bool bLandscape = false;
        const SfxPoolItem* pItem;
        const SvxSizeItem* pSize = nullptr;
        if ( SfxViewFrame::Current() )
        {
            SfxViewFrame::Current()->GetBindings().GetDispatcher()->QueryState( SID_ATTR_PAGE, pItem );
            bLandscape = static_cast<const SvxPageItem*>(pItem)->IsLandscape();
            SfxViewFrame::Current()->GetBindings().GetDispatcher()->QueryState( SID_ATTR_PAGE_SIZE, pItem );
            pSize = static_cast<const SvxSizeItem*>(pItem);
        }

        const LocaleDataWrapper& localeDataWrapper = maWidthHeightField->GetLocaleDataWrapper();
        OUString aWidthStr;
        OUString aHeightStr;
        OUString aItemText2;
        for ( std::vector< Paper >::size_type nPaperIdx = 0;
              nPaperIdx < maPaperList.size();
              ++nPaperIdx )
        {
            Size aPaperSize = SvxPaperInfo::GetPaperSize( maPaperList[ nPaperIdx ] );
            if ( bLandscape )
            {
                Swap( aPaperSize );
            }

            maWidthHeightField->SetValue( maWidthHeightField->Normalize( aPaperSize.Width() ), FieldUnit::TWIP );
            aWidthStr = localeDataWrapper.getNum(
                maWidthHeightField->GetValue(),
                maWidthHeightField->GetDecimalDigits(),
                maWidthHeightField->IsUseThousandSep(),
                maWidthHeightField->IsShowTrailingZeros() );

            maWidthHeightField->SetValue( maWidthHeightField->Normalize( aPaperSize.Height() ), FieldUnit::TWIP);
            aHeightStr = localeDataWrapper.getNum(
                maWidthHeightField->GetValue(),
                maWidthHeightField->GetDecimalDigits(),
                maWidthHeightField->IsUseThousandSep(),
                maWidthHeightField->IsShowTrailingZeros() );

            aItemText2 = aWidthStr + " x " + aHeightStr + " " + aMetricStr;

            mpSizeValueSet->AddItem(
                SvxPaperInfo::GetName( maPaperList[ nPaperIdx ] ),
                aItemText2 );

            if ( pSize && aPaperSize == pSize->GetSize() )
            {
                nSelectedItem = nPaperIdx + 1;
            }
        }
    }
    mpSizeValueSet->SetNoSelection();
    mpSizeValueSet->SetSelectHdl( LINK(this, PageSizeControl, ImplSizeHdl ) );
    mpSizeValueSet->Show();
    mpSizeValueSet->Resize();

    mpSizeValueSet->SelectItem( nSelectedItem );
    mpSizeValueSet->SetFormat();
    mpSizeValueSet->Invalidate();
    mpSizeValueSet->StartSelection();

    maMoreButton->SetClickHdl( LINK( this, PageSizeControl, MoreButtonClickHdl_Impl ) );
    maMoreButton->GrabFocus();
}

PageSizeControl::~PageSizeControl()
{
    disposeOnce();
}

void PageSizeControl::dispose()
{
    mpSizeValueSet.disposeAndClear();
    maMoreButton.disposeAndClear();
    maWidthHeightField.disposeAndClear();
    maContainer.disposeAndClear();
    SfxPopupWindow::dispose();
}

void PageSizeControl::ExecuteSizeChange( const Paper ePaper )
{
    bool bLandscape = false;
    const SfxPoolItem *pItem;
    MapUnit eUnit = lcl_GetUnit();
    if ( SfxViewFrame::Current() )
    {
        SfxViewFrame::Current()->GetBindings().GetDispatcher()->QueryState( SID_ATTR_PAGE, pItem );
        bLandscape = static_cast<const SvxPageItem*>(pItem)->IsLandscape();

        std::unique_ptr<SvxSizeItem> pPageSizeItem( new SvxSizeItem(SID_ATTR_PAGE_SIZE) );
        Size aPageSize = SvxPaperInfo::GetPaperSize( ePaper, eUnit );
        if ( bLandscape )
        {
            Swap( aPageSize );
        }
        pPageSizeItem->SetSize( aPageSize );

        SfxViewFrame::Current()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_SIZE,
            SfxCallMode::RECORD, { pPageSizeItem.get() });
    }
}


IMPL_LINK(PageSizeControl, ImplSizeHdl, ValueSet*, pControl, void)
{
    mpSizeValueSet->SetNoSelection();
    if ( pControl == mpSizeValueSet )
    {
        const sal_uInt16 nSelectedPaper = mpSizeValueSet->GetSelectedItemId();
        const Paper ePaper = maPaperList[nSelectedPaper - 1];
        ExecuteSizeChange( ePaper );
    }

    EndPopupMode();
}

IMPL_LINK_NOARG(PageSizeControl, MoreButtonClickHdl_Impl, Button*, void)
{
    if ( SfxViewFrame::Current() )
        SfxViewFrame::Current()->GetDispatcher()->Execute( FN_FORMAT_PAGE_SETTING_DLG, SfxCallMode::ASYNCHRON );
    EndPopupMode();
}

} } // end of namespace sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
