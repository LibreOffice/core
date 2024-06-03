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
#include <svl/intitem.hxx>
#include <svx/colorbox.hxx>
#include <svx/drawitem.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/itemwin.hxx>
#include <svx/SvxNumOptionsTabPageHelper.hxx>
#include "PageStylesPanel.hxx"
#include <sfx2/sidebar/Panel.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/objsh.hxx>
#include <cmdid.h>

using namespace ::com::sun::star;

namespace sw::sidebar{

namespace {

enum eFillStyle
{
    NONE,
    SOLID,
    GRADIENT,
    HATCH,
    BITMAP,
    PATTERN
};

}

const SvxPageUsage aArr[] =
{
    SvxPageUsage::All,
    SvxPageUsage::Mirror,
    SvxPageUsage::Right,
    SvxPageUsage::Left
};


static sal_uInt16 PageUsageToPos_Impl( SvxPageUsage nUsage )
{
    for ( size_t i = 0; i < SAL_N_ELEMENTS(aArr); ++i )
        if ( aArr[i] == nUsage )
            return i;
    return 3;
}

static SvxPageUsage PosToPageUsage_Impl( sal_uInt16 nPos )
{
    if ( nPos >= SAL_N_ELEMENTS(aArr) )
        return SvxPageUsage::NONE;
    return aArr[nPos];
}

std::unique_ptr<PanelLayout> PageStylesPanel::Create(weld::Widget* pParent, SfxBindings* pBindings)
{
    if( pParent == nullptr )
        throw ::com::sun::star::lang::IllegalArgumentException(u"no parent window given to PageStylesPanel::Create"_ustr, nullptr, 0);
    if( pBindings == nullptr )
        throw ::com::sun::star::lang::IllegalArgumentException(u"no SfxBindings given to PageStylesPanel::Create"_ustr, nullptr, 0);

    return std::make_unique<PageStylesPanel>(pParent, pBindings);
}

PageStylesPanel::PageStylesPanel(
    weld::Widget* pParent,
    SfxBindings* pBindings
    ) :
    PanelLayout(pParent, u"PageStylesPanel"_ustr, u"modules/swriter/ui/pagestylespanel.ui"_ustr),
    mpBindings( pBindings ),
    mpPageColumnItem( new SfxInt16Item(SID_ATTR_PAGE_COLUMN) ),
    mpPageItem( new SvxPageItem(SID_ATTR_PAGE) ),
    maPageColumnControl(SID_ATTR_PAGE_COLUMN, *pBindings, *this),
    maPageNumFormatControl( SID_ATTR_PAGE, *pBindings, *this ),
    maBgColorControl( SID_ATTR_PAGE_COLOR, *pBindings, *this ),
    maBgHatchingControl( SID_ATTR_PAGE_HATCH, *pBindings, *this ),
    maBgGradientControl( SID_ATTR_PAGE_GRADIENT, *pBindings, *this ),
    maBgBitmapControl( SID_ATTR_PAGE_BITMAP, *pBindings, *this ),
    maBgFillStyleControl(SID_ATTR_PAGE_FILLSTYLE, *pBindings, *this),
    mxBgColorLB(new ColorListBox(m_xBuilder->weld_menu_button(u"lbcolor"_ustr), [this]{ return GetFrameWeld(); })),
    mxBgHatchingLB(m_xBuilder->weld_combo_box(u"lbhatching"_ustr)),
    mxBgGradientLB(new ColorListBox(m_xBuilder->weld_menu_button(u"lbgradient"_ustr), [this]{ return GetFrameWeld(); })),
    mxBgBitmapLB(m_xBuilder->weld_combo_box(u"lbbitmap"_ustr)),
    mxLayoutSelectLB(m_xBuilder->weld_combo_box(u"layoutbox"_ustr)),
    mxColumnCount(m_xBuilder->weld_combo_box(u"columnbox"_ustr)),
    mxNumberSelectLB(new SvxPageNumberListBox(m_xBuilder->weld_combo_box(u"numberbox"_ustr))),
    mxBgFillType(m_xBuilder->weld_combo_box(u"bgselect"_ustr)),
    mxCustomEntry(m_xBuilder->weld_label(u"customlabel"_ustr))
{
    Initialize();
}

PageStylesPanel::~PageStylesPanel()
{
    mxColumnCount.reset();
    mxNumberSelectLB.reset();
    mxBgFillType.reset();
    mxBgColorLB.reset();
    mxBgHatchingLB.reset();
    mxBgGradientLB.reset();
    mxBgBitmapLB.reset();
    mxLayoutSelectLB.reset();
    mxCustomEntry.reset();

    maBgBitmapControl.dispose();
    maBgColorControl.dispose();
    maBgFillStyleControl.dispose();
    maBgGradientControl.dispose();
    maBgHatchingControl.dispose();
    maPageColumnControl.dispose();
    maPageNumFormatControl.dispose();
}

void PageStylesPanel::Initialize()
{
    SvxFillTypeBox::Fill(*mxBgFillType);

    m_aCustomEntry = mxCustomEntry->get_label();
    mpBindings->Invalidate(SID_ATTR_PAGE_COLUMN);
    mpBindings->Invalidate(SID_ATTR_PAGE);
    mpBindings->Invalidate(SID_ATTR_PAGE_FILLSTYLE);
    Update();

    mxColumnCount->connect_changed( LINK(this, PageStylesPanel, ModifyColumnCountHdl) );
    SvxNumOptionsTabPageHelper::GetI18nNumbering(mxNumberSelectLB->get_widget(), ::std::numeric_limits<sal_uInt16>::max());
    mxNumberSelectLB->connect_changed( LINK(this, PageStylesPanel, ModifyNumberingHdl) );
    mxLayoutSelectLB->connect_changed( LINK(this, PageStylesPanel, ModifyLayoutHdl) );
    mxBgFillType->connect_changed( LINK(this, PageStylesPanel, ModifyFillStyleHdl));
    mxBgColorLB->SetSelectHdl( LINK(this, PageStylesPanel, ModifyFillColorListHdl));
    mxBgGradientLB->SetSelectHdl( LINK(this, PageStylesPanel, ModifyFillColorListHdl));
    mxBgHatchingLB->connect_changed( LINK(this, PageStylesPanel, ModifyFillColorHdl));
    mxBgBitmapLB->connect_changed( LINK(this, PageStylesPanel, ModifyFillColorHdl));
}

void PageStylesPanel::Update()
{
    const eFillStyle eXFS = static_cast<eFillStyle>(mxBgFillType->get_active());
    SfxObjectShell* pSh = SfxObjectShell::Current();
    if (!pSh)
    {
        SAL_WARN("sw.ui", "PageStylesPanel::Update(): no SfxObjectShell found");
        return;
    }
    switch(eXFS)
    {
        case NONE:
        {
            mxBgColorLB->hide();
            mxBgHatchingLB->hide();
            mxBgGradientLB->hide();
            mxBgBitmapLB->hide();
        }
        break;
        case SOLID:
        {
            mxBgBitmapLB->hide();
            mxBgGradientLB->hide();
            mxBgHatchingLB->hide();
            mxBgColorLB->show();
            const Color aColor = GetColorSetOrDefault();
            mxBgColorLB->SelectEntry(aColor);
        }
        break;
        case GRADIENT:
        {
            mxBgBitmapLB->hide();
            mxBgHatchingLB->hide();
            mxBgColorLB->show();
            mxBgGradientLB->show();

            const basegfx::BGradient aBGradient = GetGradientSetOrDefault();
            const Color aStartColor(aBGradient.GetColorStops().front().getStopColor());
            mxBgColorLB->SelectEntry(aStartColor);
            const Color aEndColor(aBGradient.GetColorStops().back().getStopColor());
            mxBgGradientLB->SelectEntry(aEndColor);
        }
        break;

        case HATCH:
        {
            mxBgColorLB->hide();
            mxBgGradientLB->hide();
            mxBgBitmapLB->hide();
            mxBgHatchingLB->show();
            mxBgHatchingLB->clear();
            SvxFillAttrBox::Fill(*mxBgHatchingLB, pSh->GetItem(SID_HATCH_LIST)->GetHatchList());

            const OUString aHatchName = GetHatchingSetOrDefault();
            mxBgHatchingLB->set_active_text( aHatchName );
        }
        break;

        case BITMAP:
        case PATTERN:
        {
            mxBgColorLB->hide();
            mxBgGradientLB->hide();
            mxBgHatchingLB->hide();
            mxBgBitmapLB->show();
            mxBgBitmapLB->clear();
            OUString aBitmapName;

            if( eXFS == BITMAP )
            {
                SvxFillAttrBox::Fill(*mxBgBitmapLB, pSh->GetItem(SID_BITMAP_LIST)->GetBitmapList());
                aBitmapName = GetBitmapSetOrDefault();
            }
            else
            {
                SvxFillAttrBox::Fill(*mxBgBitmapLB, pSh->GetItem(SID_PATTERN_LIST)->GetPatternList());
                aBitmapName = GetPatternSetOrDefault();
            }

            mxBgBitmapLB->set_active_text( aBitmapName );
        }
        break;

        default:
            break;
    }

    // Need to do a relayouting, otherwise the panel size is not updated after show / hide controls
    if (m_pPanel)
        m_pPanel->TriggerDeckLayouting();
}

Color const & PageStylesPanel::GetColorSetOrDefault()
{
   if ( !mpBgColorItem )
        mpBgColorItem.reset( new XFillColorItem( OUString(), Color(0x72, 0x9f, 0xcf) ) );

   return mpBgColorItem->GetColorValue();
}

basegfx::BGradient const & PageStylesPanel::GetGradientSetOrDefault()
{
    if( !mpBgGradientItem )
    {
        basegfx::BGradient aGradient;
        OUString aGradientName;
        if (SfxObjectShell* pSh = SfxObjectShell::Current())
        {
            const SvxGradientListItem* pGradListItem = pSh->GetItem(SID_GRADIENT_LIST);
            aGradient = pGradListItem->GetGradientList()->GetGradient(0)->GetGradient();
            aGradientName = pGradListItem->GetGradientList()->GetGradient(0)->GetName();
        }
        mpBgGradientItem.reset( new XFillGradientItem( aGradientName, aGradient ) );
    }

    return mpBgGradientItem->GetGradientValue();
}

OUString const & PageStylesPanel::GetHatchingSetOrDefault()
{
    if( !mpBgHatchItem )
    {
        XHatch aHatch;
        OUString aHatchName;
        if (SfxObjectShell* pSh = SfxObjectShell::Current())
        {
            const SvxHatchListItem * pHatchListItem = pSh->GetItem(SID_HATCH_LIST);
            aHatch = pHatchListItem->GetHatchList()->GetHatch(0)->GetHatch();
            aHatchName = pHatchListItem->GetHatchList()->GetHatch(0)->GetName();
        }
        mpBgHatchItem.reset( new XFillHatchItem( aHatchName, aHatch ) );
    }

    return mpBgHatchItem->GetName();
}

OUString const & PageStylesPanel::GetBitmapSetOrDefault()
{
    if( !mpBgBitmapItem || mpBgBitmapItem->isPattern() )
    {
        GraphicObject aGraphObj;
        OUString aBmpName;
        if (SfxObjectShell* pSh = SfxObjectShell::Current())
        {
            const SvxBitmapListItem * pBmpListItem = pSh->GetItem(SID_BITMAP_LIST);
            aGraphObj = pBmpListItem->GetBitmapList()->GetBitmap(0)->GetGraphicObject();
            aBmpName = pBmpListItem->GetBitmapList()->GetBitmap(0)->GetName();
        }
        mpBgBitmapItem.reset( new XFillBitmapItem( aBmpName, aGraphObj ) );
    }

    return mpBgBitmapItem->GetName();
}

OUString const & PageStylesPanel::GetPatternSetOrDefault()
{
    if( !mpBgBitmapItem || !mpBgBitmapItem->isPattern() )
    {
        GraphicObject aGraphObj;
        OUString aPatternName;
        if (SfxObjectShell* pSh = SfxObjectShell::Current())
        {
            const SvxPatternListItem * pPatternListItem = pSh->GetItem(SID_PATTERN_LIST);
            aGraphObj = pPatternListItem->GetPatternList()->GetBitmap(0)->GetGraphicObject();
            aPatternName = pPatternListItem->GetPatternList()->GetBitmap(0)->GetName();
        }
        mpBgBitmapItem.reset( new XFillBitmapItem( aPatternName, aGraphObj ) );
    }

    return mpBgBitmapItem->GetName();
}

void PageStylesPanel::NotifyItemUpdate(
    const sal_uInt16 nSid,
    const SfxItemState eState,
    const SfxPoolItem* pState)
{
    if (!mxColumnCount) //disposed
        return;

    switch(nSid)
    {
        case SID_ATTR_PAGE_COLUMN:
        {
            if ( eState >= SfxItemState::DEFAULT &&
                 dynamic_cast< const SfxInt16Item *>( pState ) )
            {
                mpPageColumnItem.reset( static_cast<SfxInt16Item*>(pState->Clone()) );
                if(mpPageColumnItem->GetValue() <= 5)
                {
                    mxColumnCount->set_active(mpPageColumnItem->GetValue() - 1);
                    int nIndex = mxColumnCount->find_text(m_aCustomEntry);
                    if (nIndex != -1)
                        mxColumnCount->remove(nIndex);
                }
                else
                {
                    if (mxColumnCount->find_text(m_aCustomEntry) == -1)
                        mxColumnCount->append_text(m_aCustomEntry);
                    mxColumnCount->set_active_text(m_aCustomEntry);
                }
            }
        }
        break;

        case SID_ATTR_PAGE:
        {
            if( eState >= SfxItemState::DEFAULT &&
                dynamic_cast< const SvxPageItem*>( pState ) )
            {
                mpPageItem.reset( static_cast<SvxPageItem*>(pState->Clone()) );
                SvxNumType eNumType = mpPageItem->GetNumType();
                mxNumberSelectLB->set_active_id(eNumType);

                SvxPageUsage nUse = mpPageItem->GetPageUsage();
                mxLayoutSelectLB->set_active( PageUsageToPos_Impl( nUse ) );
            }
        }
        break;

        case SID_ATTR_PAGE_COLOR:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mxBgFillType->set_active( static_cast<sal_Int32>(SOLID) );
                mpBgColorItem.reset(pState ? static_cast< XFillColorItem* >(pState->Clone()) : nullptr);
                Update();
            }
        }
        break;

        case SID_ATTR_PAGE_HATCH:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mxBgFillType->set_active( static_cast<sal_Int32>(HATCH) );
                mpBgHatchItem.reset(pState ? static_cast < XFillHatchItem* >(pState->Clone()) : nullptr);
                Update();
            }
        }
        break;

        case SID_ATTR_PAGE_GRADIENT:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mxBgFillType->set_active( static_cast<sal_Int32>(GRADIENT) );
                mpBgGradientItem.reset(pState ? static_cast< XFillGradientItem* >(pState->Clone()) : nullptr);
                Update();
            }
        }
        break;
        case SID_ATTR_PAGE_BITMAP:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mpBgBitmapItem.reset(pState ? static_cast< XFillBitmapItem* >(pState->Clone()) : nullptr);
                if (mpBgBitmapItem)
                {
                    if (mpBgBitmapItem->isPattern())
                        mxBgFillType->set_active( static_cast<sal_Int32>(PATTERN) );
                    else
                        mxBgFillType->set_active( static_cast<sal_Int32>(BITMAP) );
                }
                Update();
            }
        }
        break;

        case SID_ATTR_PAGE_FILLSTYLE:
        {
            const XFillStyleItem* pFillStyleItem = nullptr;
            if (eState >= SfxItemState::DEFAULT)
                pFillStyleItem = dynamic_cast< const XFillStyleItem* >(pState);
            if (pFillStyleItem)
            {
                css::drawing::FillStyle eXFS = pFillStyleItem->GetValue();
                switch(eXFS)
                {
                    case drawing::FillStyle_NONE:
                        mxBgFillType->set_active( static_cast<sal_Int32>(NONE) );
                        break;
                    case drawing::FillStyle_SOLID:
                        mxBgFillType->set_active( static_cast<sal_Int32>(SOLID) );
                        break;
                    case drawing::FillStyle_GRADIENT:
                        mxBgFillType->set_active( static_cast<sal_Int32>(GRADIENT) );
                        break;
                    case drawing::FillStyle_HATCH:
                        mxBgFillType->set_active( static_cast<sal_Int32>(HATCH) );
                        break;
                    case drawing::FillStyle_BITMAP:
                        if (mpBgBitmapItem->isPattern())
                            mxBgFillType->set_active( static_cast<sal_Int32>(PATTERN) );
                        else
                            mxBgFillType->set_active( static_cast<sal_Int32>(BITMAP) );
                        break;
                    default:
                        break;
                }
                Update();
            }
        }
        break;

        default:
            break;
    }
}

IMPL_LINK_NOARG( PageStylesPanel, ModifyColumnCountHdl, weld::ComboBox&, void )
{
    sal_uInt16 nColumnType = mxColumnCount->get_active() + 1;
    mpPageColumnItem->SetValue( nColumnType );
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_COLUMN,
            SfxCallMode::RECORD, { mpPageColumnItem.get() });
}

IMPL_LINK_NOARG( PageStylesPanel, ModifyNumberingHdl, weld::ComboBox&, void )
{
    SvxNumType nEntryData = mxNumberSelectLB->get_active_id();
    mpPageItem->SetNumType(nEntryData);
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_PAGE, SfxCallMode::RECORD, { mpPageItem.get() });
}

IMPL_LINK_NOARG( PageStylesPanel, ModifyLayoutHdl, weld::ComboBox&, void )
{
    sal_uInt16 nUse = mxLayoutSelectLB->get_active();
    mpPageItem->SetPageUsage(PosToPageUsage_Impl(nUse));
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_PAGE, SfxCallMode::RECORD, { mpPageItem.get() });
}

IMPL_LINK_NOARG(PageStylesPanel, ModifyFillStyleHdl, weld::ComboBox&, void)
{
    const eFillStyle eXFS = static_cast<eFillStyle>(mxBgFillType->get_active());
    Update();

    switch (eXFS)
    {
        case NONE:
        {
            const XFillStyleItem aXFillStyleItem(drawing::FillStyle_NONE);
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_FILLSTYLE, SfxCallMode::RECORD, { &aXFillStyleItem });
        }
        break;

        case SOLID:
        {
            XFillColorItem aItem( OUString(), mpBgColorItem->GetColorValue() );
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_COLOR, SfxCallMode::RECORD, { &aItem });
        }
        break;

        case GRADIENT:
        {
            XFillGradientItem aItem( mpBgGradientItem->GetName(), mpBgGradientItem->GetGradientValue() );
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_GRADIENT, SfxCallMode::RECORD, { &aItem });
        }
        break;

        case HATCH:
        {
            XFillHatchItem aItem( mpBgHatchItem->GetName(), mpBgHatchItem->GetHatchValue() );
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_HATCH, SfxCallMode::RECORD, { &aItem });
        }
        break;

        case BITMAP:
        case PATTERN:
        {
            XFillBitmapItem aItem( mpBgBitmapItem->GetName(), mpBgBitmapItem->GetGraphicObject() );
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_BITMAP, SfxCallMode::RECORD, { &aItem });
        }
        break;

        default:
        break;
    }
}

void PageStylesPanel::ModifyFillColor()
{
    const eFillStyle eXFS = static_cast<eFillStyle>(mxBgFillType->get_active());
    SfxObjectShell* pSh = SfxObjectShell::Current();
    switch(eXFS)
    {
        case SOLID:
        {
            auto aNamedColor =  mxBgColorLB->GetSelectedEntry();
            XFillColorItem aItem(OUString(), aNamedColor.m_aColor);
            aItem.setComplexColor(aNamedColor.getComplexColor());
            aItem.setComplexColor(mxBgColorLB->GetSelectedEntry().getComplexColor());
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_COLOR, SfxCallMode::RECORD, { &aItem });
        }
        break;
        case GRADIENT:
        {
            basegfx::BGradient aGradient(
                basegfx::BColorStops(
                    mxBgColorLB->GetSelectEntryColor().getBColor(),
                    mxBgGradientLB->GetSelectEntryColor().getBColor()));

            XFillGradientItem aItem(aGradient);
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_GRADIENT, SfxCallMode::RECORD, { &aItem });
        }
        break;
        case HATCH:
            if (pSh)
            {
                const SvxHatchListItem * pHatchListItem = pSh->GetItem(SID_HATCH_LIST);
                sal_uInt16 nPos = mxBgHatchingLB->get_active();
                XHatch aHatch = pHatchListItem->GetHatchList()->GetHatch(nPos)->GetHatch();
                const OUString aHatchName = pHatchListItem->GetHatchList()->GetHatch(nPos)->GetName();

                XFillHatchItem aItem(aHatchName, aHatch);
                GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_HATCH, SfxCallMode::RECORD, { &aItem });
            }
            break;
        case BITMAP:
        case PATTERN:
            if (pSh)
            {
                sal_Int16 nPos = mxBgBitmapLB->get_active();
                GraphicObject aBitmap;
                OUString aBitmapName;

                if ( eXFS == BITMAP )
                {
                    SvxBitmapListItem const * pBitmapListItem = pSh->GetItem(SID_BITMAP_LIST);
                    aBitmap = pBitmapListItem->GetBitmapList()->GetBitmap(nPos)->GetGraphicObject();
                    aBitmapName = pBitmapListItem->GetBitmapList()->GetBitmap(nPos)->GetName();
                }
                else
                {
                    SvxPatternListItem const * pPatternListItem = pSh->GetItem(SID_PATTERN_LIST);
                    aBitmap = pPatternListItem->GetPatternList()->GetBitmap(nPos)->GetGraphicObject();
                    aBitmapName = pPatternListItem->GetPatternList()->GetBitmap(nPos)->GetName();
                }

                XFillBitmapItem aItem(aBitmapName, aBitmap);
                GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_BITMAP, SfxCallMode::RECORD, { &aItem });
            }
            break;
        default:
            break;
    }
}

IMPL_LINK_NOARG(PageStylesPanel, ModifyFillColorHdl, weld::ComboBox&, void)
{
    ModifyFillColor();
}

IMPL_LINK_NOARG(PageStylesPanel, ModifyFillColorListHdl, ColorListBox&, void)
{
    ModifyFillColor();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
