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
#include <svx/pageitem.hxx>
#include <svx/sidebar/ValueSetWithTextControl.hxx>

#include <unotools/localedatawrapper.hxx>
#include <rtl/character.hxx>
#include <editeng/paperinf.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/module.hxx>
#include <sfx2/viewfrm.hxx>

#include <vcl/settings.hxx>
#include <svl/itempool.hxx>
#include <svl/intitem.hxx>
#include <svtools/unitconv.hxx>
#include <editeng/sizeitem.hxx>

#include <PageSizePopup.hxx>

namespace
{
    FieldUnit lcl_GetFieldUnit()
    {
        if (SfxViewFrame* pViewFrm = SfxViewFrame::Current())
        {
            SfxPoolItemHolder aResult;
            const SfxItemState eState(pViewFrm->GetBindings().GetDispatcher()->QueryState(SID_ATTR_METRIC, aResult));
            const SfxUInt16Item* pItem(static_cast<const SfxUInt16Item*>(aResult.getItem()));

            if (pItem && eState >= SfxItemState::DEFAULT)
                return static_cast<FieldUnit>(pItem->GetValue());
        }
        return SfxModule::GetCurrentFieldUnit();
    }

    MapUnit lcl_GetUnit()
    {
        SfxItemPool &rPool = SfxGetpApp()->GetPool();
        sal_uInt16 nWhich = rPool.GetWhich( SID_ATTR_PAGE_SIZE );
        return rPool.GetMetric( nWhich );
    }
}

namespace sw::sidebar {

PageSizeControl::PageSizeControl(PageSizePopup* pControl, weld::Widget* pParent)
    : WeldToolbarPopup(pControl->getFrameInterface(), pParent, "modules/swriter/ui/pagesizecontrol.ui", "PageSizeControl")
    , mxMoreButton(m_xBuilder->weld_button("moreoptions"))
    , mxWidthHeightField(m_xBuilder->weld_metric_spin_button("metric", FieldUnit::CM))
    , mxSizeValueSet(new svx::sidebar::ValueSetWithTextControl)
    , mxSizeValueSetWin(new weld::CustomWeld(*m_xBuilder, "pagesizevalueset", *mxSizeValueSet))
    , mxControl(pControl)
{
    mxWidthHeightField->set_unit(FieldUnit::CM);
    mxWidthHeightField->set_range(0, 9999, FieldUnit::NONE);
    mxWidthHeightField->set_digits(2);
    mxWidthHeightField->set_increments(10, 100, FieldUnit::NONE);
    SetFieldUnit( *mxWidthHeightField, lcl_GetFieldUnit() );

    maPaperList.push_back( PAPER_A3 );
    maPaperList.push_back( PAPER_A4 );
    maPaperList.push_back( PAPER_A5 );
    maPaperList.push_back( PAPER_B4_ISO );
    maPaperList.push_back( PAPER_B5_ISO );
    maPaperList.push_back( PAPER_ENV_C5 );
    maPaperList.push_back( PAPER_LETTER );
    maPaperList.push_back( PAPER_LEGAL );

    mxSizeValueSet->SetStyle( mxSizeValueSet->GetStyle() | WB_3DLOOK | WB_NO_DIRECTSELECT );
    mxSizeValueSet->SetColor( Application::GetSettings().GetStyleSettings().GetMenuColor() );

    sal_uInt16 nSelectedItem = 0;
    {
        OUString aMetricStr;
        {
            const OUString aText = mxWidthHeightField->get_text();
            for (short i = aText.getLength() - 1; i >= 0; i--)
            {
                sal_Unicode c = aText[i];
                if ( rtl::isAsciiAlpha(c) || (c == '\'') || (c == '\"') || (c == '%') )
                {
                    aMetricStr = OUStringChar(c) + aMetricStr;
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
        const SvxSizeItem* pSize(nullptr);
        if (SfxViewFrame* pViewFrm = SfxViewFrame::Current())
        {
            SfxPoolItemHolder aResult;
            pViewFrm->GetBindings().GetDispatcher()->QueryState(SID_ATTR_PAGE, aResult );
            bLandscape = static_cast<const SvxPageItem*>(aResult.getItem())->IsLandscape();
            pViewFrm->GetBindings().GetDispatcher()->QueryState(SID_ATTR_PAGE_SIZE, aResult);
            pSize = static_cast<const SvxSizeItem*>(aResult.getItem());
        }

        const LocaleDataWrapper& localeDataWrapper = Application::GetSettings().GetLocaleDataWrapper();
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

            mxWidthHeightField->set_value( mxWidthHeightField->normalize( aPaperSize.Width() ), FieldUnit::TWIP );
            aWidthStr = localeDataWrapper.getNum(
                mxWidthHeightField->get_value(FieldUnit::NONE),
                mxWidthHeightField->get_digits(),
                true,
                true );

            mxWidthHeightField->set_value( mxWidthHeightField->normalize( aPaperSize.Height() ), FieldUnit::TWIP);
            aHeightStr = localeDataWrapper.getNum(
                mxWidthHeightField->get_value(FieldUnit::NONE),
                mxWidthHeightField->get_digits(),
                true,
                true );

            aItemText2 = aWidthStr + " x " + aHeightStr + " " + aMetricStr;

            mxSizeValueSet->AddItem(
                SvxPaperInfo::GetName( maPaperList[ nPaperIdx ] ),
                aItemText2 );

            if ( pSize && aPaperSize == pSize->GetSize() )
            {
                nSelectedItem = nPaperIdx + 1;
            }
        }
    }
    mxSizeValueSet->SetNoSelection();
    mxSizeValueSet->SetSelectHdl( LINK(this, PageSizeControl, ImplSizeHdl ) );
    mxSizeValueSet->Show();
    mxSizeValueSet->Resize();

    mxSizeValueSet->SelectItem( nSelectedItem );
    mxSizeValueSet->SetFormat();
    mxSizeValueSet->Invalidate();

    mxMoreButton->connect_clicked( LINK( this, PageSizeControl, MoreButtonClickHdl_Impl ) );
    mxMoreButton->grab_focus();
}

void PageSizeControl::GrabFocus()
{
    mxSizeValueSet->GrabFocus();
}

PageSizeControl::~PageSizeControl()
{
}

void PageSizeControl::ExecuteSizeChange( const Paper ePaper )
{
    bool bLandscape = false;
    MapUnit eUnit = lcl_GetUnit();
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if (!pViewFrm)
        return;

    SfxPoolItemHolder aResult;
    pViewFrm->GetBindings().GetDispatcher()->QueryState(SID_ATTR_PAGE, aResult);
    const SvxPageItem* pItem(static_cast<const SvxPageItem*>(aResult.getItem()));
    bLandscape = pItem->IsLandscape();

    SvxSizeItem aPageSizeItem(SID_ATTR_PAGE_SIZE);
    Size aPageSize = SvxPaperInfo::GetPaperSize( ePaper, eUnit );
    if ( bLandscape )
    {
        Swap( aPageSize );
    }
    aPageSizeItem.SetSize( aPageSize );

    pViewFrm->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_SIZE,
        SfxCallMode::RECORD, { &aPageSizeItem });
}


IMPL_LINK_NOARG(PageSizeControl, ImplSizeHdl, ValueSet*, void)
{
    mxSizeValueSet->SetNoSelection();
    const sal_uInt16 nSelectedPaper = mxSizeValueSet->GetSelectedItemId();
    const Paper ePaper = maPaperList[nSelectedPaper - 1];
    ExecuteSizeChange( ePaper );

    mxControl->EndPopupMode();
}

IMPL_LINK_NOARG(PageSizeControl, MoreButtonClickHdl_Impl, weld::Button&, void)
{
    if (SfxViewFrame* pViewFrm = SfxViewFrame::Current())
        pViewFrm->GetDispatcher()->Execute( FN_FORMAT_PAGE_SETTING_DLG, SfxCallMode::ASYNCHRON );
    mxControl->EndPopupMode();
}

} // end of namespace sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
