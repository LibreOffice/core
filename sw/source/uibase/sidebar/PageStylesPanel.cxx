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
#include <swtypes.hxx>
#include <svl/intitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/paperinf.hxx>
#include <svx/svxids.hrc>
#include <svx/colorbox.hxx>
#include <svx/dlgutil.hxx>
#include <svx/rulritem.hxx>
#include <svx/drawitem.hxx>
#include <svx/SvxNumOptionsTabPageHelper.hxx>
#include "PageStylesPanel.hxx"
#include <sfx2/sidebar/ControlFactory.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/objsh.hxx>
#include <cmdid.h>

using namespace ::com::sun::star;

namespace sw { namespace sidebar{

enum eFillStyle
{
    NONE,
    SOLID,
    GRADIENT,
    HATCH,
    BITMAP,
    PATTERN
};

const SvxPageUsage aArr[] =
{
    SvxPageUsage::All,
    SvxPageUsage::Mirror,
    SvxPageUsage::Right,
    SvxPageUsage::Left
};


sal_uInt16 PageUsageToPos_Impl( SvxPageUsage nUsage )
{
    for ( sal_uInt16 i = 0; i < SAL_N_ELEMENTS(aArr); ++i )
        if ( aArr[i] == nUsage )
            return i;
    return 3;
}


SvxPageUsage PosToPageUsage_Impl( sal_uInt16 nPos )
{
    if ( nPos >= SAL_N_ELEMENTS(aArr) )
        return SvxPageUsage::NONE;
    return aArr[nPos];
}

VclPtr<vcl::Window> PageStylesPanel::Create(
    vcl::Window* pParent,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
    SfxBindings* pBindings)
{
    if( pParent == nullptr )
        throw ::com::sun::star::lang::IllegalArgumentException("no parent window given to PageStylesPanel::Create", nullptr, 0);
    if( !rxFrame.is() )
        throw ::com::sun::star::lang::IllegalArgumentException("no XFrame given to PageStylesPanel::Create", nullptr, 0);
    if( pBindings == nullptr )
        throw ::com::sun::star::lang::IllegalArgumentException("no SfxBindings given to PageStylesPanel::Create", nullptr, 0);

    return VclPtr<PageStylesPanel>::Create(pParent, rxFrame, pBindings);
}

PageStylesPanel::PageStylesPanel(
    vcl::Window* pParent,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
    SfxBindings* pBindings
    ) :
    PanelLayout(pParent, "PageStylesPanel", "modules/swriter/ui/pagestylespanel.ui", rxFrame),
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
    aCustomEntry()
{
    get(mpColumnCount, "columnbox");
    get(mpNumberSelectLB, "numberbox");
    get(mpBgFillType, "bgselect");
    get(mpBgColorLB, "lbcolor");
    get(mpBgHatchingLB, "lbhatching");
    get(mpBgGradientLB, "lbgradient");
    get(mpBgBitmapLB, "lbbitmap");
    get(mpLayoutSelectLB, "layoutbox");
    get(mpCustomEntry, "customlabel");

    Initialize();
}

PageStylesPanel::~PageStylesPanel()
{
    disposeOnce();
}

void PageStylesPanel::dispose()
{
    mpColumnCount.disposeAndClear();
    mpNumberSelectLB.disposeAndClear();
    mpBgFillType.disposeAndClear();
    mpBgColorLB.disposeAndClear();
    mpBgHatchingLB.disposeAndClear();
    mpBgGradientLB.disposeAndClear();
    mpBgBitmapLB.disposeAndClear();
    mpLayoutSelectLB.disposeAndClear();

    maBgBitmapControl.dispose();
    maBgColorControl.dispose();
    maBgFillStyleControl.dispose();
    maBgGradientControl.dispose();
    maBgHatchingControl.dispose();
    maPageColumnControl.dispose();
    maPageNumFormatControl.dispose();
    mpCustomEntry.clear();
    PanelLayout::dispose();
}

void PageStylesPanel::Initialize()
{
    aCustomEntry = mpCustomEntry->GetText();
    mpBindings->Invalidate(SID_ATTR_PAGE_COLUMN);
    mpBindings->Invalidate(SID_ATTR_PAGE);
    mpBindings->Invalidate(SID_ATTR_PAGE_FILLSTYLE);
    Update();

    mpColumnCount->SetSelectHdl( LINK(this, PageStylesPanel, ModifyColumnCountHdl) );
    SvxNumOptionsTabPageHelper::GetI18nNumbering( *mpNumberSelectLB, ::std::numeric_limits<sal_uInt16>::max());
    mpNumberSelectLB->SetSelectHdl( LINK(this, PageStylesPanel, ModifyNumberingHdl) );
    mpLayoutSelectLB->SetSelectHdl( LINK(this, PageStylesPanel, ModifyLayoutHdl) );
    mpBgFillType->SetSelectHdl( LINK(this, PageStylesPanel, ModifyFillStyleHdl));
    mpBgColorLB->SetSelectHdl( LINK(this, PageStylesPanel, ModifyFillColorListHdl));
    mpBgGradientLB->SetSelectHdl( LINK(this, PageStylesPanel, ModifyFillColorListHdl));
    mpBgHatchingLB->SetSelectHdl( LINK(this, PageStylesPanel, ModifyFillColorHdl));
    mpBgBitmapLB->SetSelectHdl( LINK(this, PageStylesPanel, ModifyFillColorHdl));
}

void PageStylesPanel::Update()
{
    const eFillStyle eXFS = static_cast<eFillStyle>(mpBgFillType->GetSelectedEntryPos());
    SfxObjectShell* pSh = SfxObjectShell::Current();
    switch(eXFS)
    {
        case NONE:
        {
            mpBgColorLB->Hide();
            mpBgHatchingLB->Hide();
            mpBgGradientLB->Hide();
            mpBgBitmapLB->Hide();
        }
        break;
        case SOLID:
        {
            mpBgBitmapLB->Hide();
            mpBgGradientLB->Hide();
            mpBgHatchingLB->Hide();
            mpBgColorLB->Show();
            const Color aColor = GetColorSetOrDefault();
            mpBgColorLB->SelectEntry(aColor);
        }
        break;
        case GRADIENT:
        {
            mpBgBitmapLB->Hide();
            mpBgHatchingLB->Hide();
            mpBgColorLB->Show();
            mpBgGradientLB->Show();

            const XGradient xGradient = GetGradientSetOrDefault();
            const Color aStartColor = xGradient.GetStartColor();
            mpBgColorLB->SelectEntry(aStartColor);
            const Color aEndColor = xGradient.GetEndColor();
            mpBgGradientLB->SelectEntry(aEndColor);
        }
        break;

        case HATCH:
        {
            mpBgColorLB->Hide();
            mpBgGradientLB->Hide();
            mpBgBitmapLB->Hide();
            mpBgHatchingLB->Show();
            mpBgHatchingLB->Clear();
            mpBgHatchingLB->Fill(pSh->GetItem(SID_HATCH_LIST)->GetHatchList());

            const OUString aHatchName = GetHatchingSetOrDefault();
            mpBgHatchingLB->SelectEntry( aHatchName );
        }
        break;

        case BITMAP:
        case PATTERN:
        {
            mpBgColorLB->Hide();
            mpBgGradientLB->Hide();
            mpBgHatchingLB->Hide();
            mpBgBitmapLB->Show();
            mpBgBitmapLB->Clear();
            OUString aBitmapName;

            if( eXFS == BITMAP )
            {
                mpBgBitmapLB->Fill(pSh->GetItem(SID_BITMAP_LIST)->GetBitmapList());
                aBitmapName = GetBitmapSetOrDefault();
            }
            else
            {
                mpBgBitmapLB->Fill(pSh->GetItem(SID_PATTERN_LIST)->GetPatternList());
                aBitmapName = GetPatternSetOrDefault();
            }

            mpBgBitmapLB->SelectEntry( aBitmapName );
        }
        break;

        default:
            break;
    }
}

Color const & PageStylesPanel::GetColorSetOrDefault()
{
   if ( !mpBgColorItem )
        mpBgColorItem.reset( new XFillColorItem( OUString(), Color(0x72, 0x9f, 0xcf) ) );

   return mpBgColorItem->GetColorValue();
}

XGradient const & PageStylesPanel::GetGradientSetOrDefault()
{
    if( !mpBgGradientItem )
    {
        SfxObjectShell* pSh = SfxObjectShell::Current();
        const SvxGradientListItem * pGradListItem = pSh->GetItem(SID_GRADIENT_LIST);
        const XGradient aGradient = pGradListItem->GetGradientList()->GetGradient(0)->GetGradient();
        const OUString aGradientName = pGradListItem->GetGradientList()->GetGradient(0)->GetName();

        mpBgGradientItem.reset( new XFillGradientItem( aGradientName, aGradient ) );
    }

    return mpBgGradientItem->GetGradientValue();
}

OUString const & PageStylesPanel::GetHatchingSetOrDefault()
{
    if( !mpBgHatchItem )
    {
        SfxObjectShell* pSh = SfxObjectShell::Current();
        const SvxHatchListItem * pHatchListItem = pSh->GetItem(SID_HATCH_LIST);
        const XHatch aHatch = pHatchListItem->GetHatchList()->GetHatch(0)->GetHatch();
        const OUString aHatchName = pHatchListItem->GetHatchList()->GetHatch(0)->GetName();

        mpBgHatchItem.reset( new XFillHatchItem( aHatchName, aHatch ) );
    }

    return mpBgHatchItem->GetName();
}

OUString const & PageStylesPanel::GetBitmapSetOrDefault()
{
    if( !mpBgBitmapItem || mpBgBitmapItem->isPattern() )
    {
        SfxObjectShell* pSh = SfxObjectShell::Current();
        const SvxBitmapListItem * pBmpListItem = pSh->GetItem(SID_BITMAP_LIST);
        const GraphicObject aGraphObj = pBmpListItem->GetBitmapList()->GetBitmap(0)->GetGraphicObject();
        const OUString aBmpName = pBmpListItem->GetBitmapList()->GetBitmap(0)->GetName();

        mpBgBitmapItem.reset( new XFillBitmapItem( aBmpName, aGraphObj ) );
    }

    return mpBgBitmapItem->GetName();
}

OUString const & PageStylesPanel::GetPatternSetOrDefault()
{
    if( !mpBgBitmapItem || !mpBgBitmapItem->isPattern() )
    {
        SfxObjectShell* pSh = SfxObjectShell::Current();
        const SvxPatternListItem * pPatternListItem = pSh->GetItem(SID_PATTERN_LIST);
        const GraphicObject aGraphObj = pPatternListItem->GetPatternList()->GetBitmap(0)->GetGraphicObject();
        const OUString aPatternName = pPatternListItem->GetPatternList()->GetBitmap(0)->GetName();

        mpBgBitmapItem.reset( new XFillBitmapItem( aPatternName, aGraphObj ) );
    }

    return mpBgBitmapItem->GetName();
}

void PageStylesPanel::NotifyItemUpdate(
    const sal_uInt16 nSid,
    const SfxItemState eState,
    const SfxPoolItem* pState,
    const bool)
{
    if(IsDisposed())
        return;

    switch(nSid)
    {
        case SID_ATTR_PAGE_COLUMN:
        {
            if ( eState >= SfxItemState::DEFAULT &&
                 pState && dynamic_cast< const SfxInt16Item *>( pState ) !=  nullptr )
            {
                mpPageColumnItem.reset( static_cast<SfxInt16Item*>(pState->Clone()) );
                if(mpPageColumnItem->GetValue() <= 5)
                {
                    mpColumnCount->SelectEntryPos(mpPageColumnItem->GetValue() - 1);
                    mpColumnCount->RemoveEntry(aCustomEntry);
                }
                else
                {
                    if(mpColumnCount->GetEntryPos(aCustomEntry) == LISTBOX_ENTRY_NOTFOUND)
                        mpColumnCount->InsertEntry(aCustomEntry);
                    mpColumnCount->SelectEntry(aCustomEntry);
                }
            }
        }
        break;

        case SID_ATTR_PAGE:
        {
            if( eState >= SfxItemState::DEFAULT &&
                pState && dynamic_cast< const SvxPageItem*>( pState ) !=  nullptr )
            {
                mpPageItem.reset( static_cast<SvxPageItem*>(pState->Clone()) );
                SvxNumType eNumType = mpPageItem->GetNumType();
                mpNumberSelectLB->SetSelection(eNumType);

                SvxPageUsage nUse = mpPageItem->GetPageUsage();
                mpLayoutSelectLB->SelectEntryPos( PageUsageToPos_Impl( nUse ) );
            }
        }
        break;

        case SID_ATTR_PAGE_COLOR:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mpBgFillType->SelectEntryPos( static_cast<sal_Int32>(SOLID) );
                mpBgColorItem.reset(pState ? static_cast< XFillColorItem* >(pState->Clone()) : nullptr);
                Update();
            }
        }
        break;

        case SID_ATTR_PAGE_HATCH:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mpBgFillType->SelectEntryPos( static_cast<sal_Int32>(HATCH) );
                mpBgHatchItem.reset(pState ? static_cast < XFillHatchItem* >(pState->Clone()) : nullptr);
                Update();
            }
        }
        break;

        case SID_ATTR_PAGE_GRADIENT:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                mpBgFillType->SelectEntryPos( static_cast<sal_Int32>(GRADIENT) );
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
                        mpBgFillType->SelectEntryPos( static_cast<sal_Int32>(PATTERN) );
                    else
                        mpBgFillType->SelectEntryPos( static_cast<sal_Int32>(BITMAP) );
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
                        mpBgFillType->SelectEntryPos( static_cast<sal_Int32>(NONE) );
                        break;
                    case drawing::FillStyle_SOLID:
                        mpBgFillType->SelectEntryPos( static_cast<sal_Int32>(SOLID) );
                        break;
                    case drawing::FillStyle_GRADIENT:
                        mpBgFillType->SelectEntryPos( static_cast<sal_Int32>(GRADIENT) );
                        break;
                    case drawing::FillStyle_HATCH:
                        mpBgFillType->SelectEntryPos( static_cast<sal_Int32>(HATCH) );
                        break;
                    case drawing::FillStyle_BITMAP:
                        if (mpBgBitmapItem->isPattern())
                            mpBgFillType->SelectEntryPos( static_cast<sal_Int32>(PATTERN) );
                        else
                            mpBgFillType->SelectEntryPos( static_cast<sal_Int32>(BITMAP) );
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

IMPL_LINK_NOARG( PageStylesPanel, ModifyColumnCountHdl, ListBox&, void )
{
    sal_uInt16 nColumnType = mpColumnCount->GetSelectedEntryPos() + 1;
    mpPageColumnItem->SetValue( nColumnType );
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_COLUMN,
            SfxCallMode::RECORD, { mpPageColumnItem.get() });
}

IMPL_LINK_NOARG( PageStylesPanel, ModifyNumberingHdl, ListBox&, void )
{
    SvxNumType nEntryData = static_cast<SvxNumType>(reinterpret_cast<sal_uLong>(mpNumberSelectLB->GetSelectedEntryData()));
    mpPageItem->SetNumType(nEntryData);
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_PAGE, SfxCallMode::RECORD, { mpPageItem.get() });
}

IMPL_LINK_NOARG( PageStylesPanel, ModifyLayoutHdl, ListBox&, void )
{
    sal_uInt16 nUse = mpLayoutSelectLB->GetSelectedEntryPos();
    mpPageItem->SetPageUsage(PosToPageUsage_Impl(nUse));
    mpBindings->GetDispatcher()->ExecuteList(SID_ATTR_PAGE, SfxCallMode::RECORD, { mpPageItem.get() });
}

IMPL_LINK_NOARG(PageStylesPanel, ModifyFillStyleHdl, ListBox&, void)
{
    const eFillStyle eXFS = static_cast<eFillStyle>(mpBgFillType->GetSelectedEntryPos());
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
    mpBgFillType->Selected();
}

void PageStylesPanel::ModifyFillColor()
{
    const eFillStyle eXFS = static_cast<eFillStyle>(mpBgFillType->GetSelectedEntryPos());
    SfxObjectShell* pSh = SfxObjectShell::Current();
    switch(eXFS)
    {
        case SOLID:
        {
            XFillColorItem aItem(OUString(), mpBgColorLB->GetSelectEntryColor());
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_COLOR, SfxCallMode::RECORD, { &aItem });
        }
        break;
        case GRADIENT:
        {
            XGradient aGradient;
            aGradient.SetStartColor(mpBgColorLB->GetSelectEntryColor());
            aGradient.SetEndColor(mpBgGradientLB->GetSelectEntryColor());

            XFillGradientItem aItem(aGradient);
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_GRADIENT, SfxCallMode::RECORD, { &aItem });
        }
        break;
        case HATCH:
        {
            const SvxHatchListItem * pHatchListItem = pSh->GetItem(SID_HATCH_LIST);
            sal_uInt16 nPos = mpBgHatchingLB->GetSelectedEntryPos();
            XHatch aHatch = pHatchListItem->GetHatchList()->GetHatch(nPos)->GetHatch();
            const OUString aHatchName = pHatchListItem->GetHatchList()->GetHatch(nPos)->GetName();

            XFillHatchItem aItem(aHatchName, aHatch);
            GetBindings()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_HATCH, SfxCallMode::RECORD, { &aItem });
        }
        break;
        case BITMAP:
        case PATTERN:
        {
            sal_Int16 nPos = mpBgBitmapLB->GetSelectedEntryPos();
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

IMPL_LINK_NOARG(PageStylesPanel, ModifyFillColorHdl, ListBox&, void)
{
    ModifyFillColor();
}

IMPL_LINK_NOARG(PageStylesPanel, ModifyFillColorListHdl, SvxColorListBox&, void)
{
    ModifyFillColor();
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
