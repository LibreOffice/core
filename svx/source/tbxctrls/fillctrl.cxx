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

#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <rtl/ustring.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/virdev.hxx>
#include <svl/itemset.hxx>
#include <svx/svxids.hrc>
#include <tools/json_writer.hxx>

constexpr OUString TMP_STR_BEGIN = u"["_ustr;
constexpr OUString TMP_STR_END = u"]"_ustr;

#include <svx/drawitem.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xtable.hxx>
#include <svx/fillctrl.hxx>
#include <svx/itemwin.hxx>
#include <svx/xflclit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xbtmpit.hxx>
#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;

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

drawing::FillStyle toCssFillStyle( eFillStyle eXFS )
{
    if (eXFS == PATTERN)
    {
        return drawing::FillStyle_BITMAP;
    }

    return static_cast<drawing::FillStyle>(eXFS);
}

}

SFX_IMPL_TOOLBOX_CONTROL( SvxFillToolBoxControl, XFillStyleItem );

SvxFillToolBoxControl::SvxFillToolBoxControl(
    sal_uInt16 nSlotId,
    ToolBoxItemId nId,
    ToolBox& rTbx )
    : SfxToolBoxControl( nSlotId, nId, rTbx )
    , mxFillControl(nullptr)
    , mpLbFillType(nullptr)
    , mpToolBoxColor(nullptr)
    , mpLbFillAttr(nullptr)
    , mnLastXFS(-1)
    , mnLastPosGradient(0)
    , mnLastPosHatch(0)
    , mnLastPosBitmap(0)
    , mnLastPosPattern(0)
{
    addStatusListener( u".uno:FillColor"_ustr);
    addStatusListener( u".uno:FillGradient"_ustr);
    addStatusListener( u".uno:FillHatch"_ustr);
    addStatusListener( u".uno:FillBitmap"_ustr);
    addStatusListener( u".uno:ColorTableState"_ustr);
    addStatusListener( u".uno:GradientListState"_ustr);
    addStatusListener( u".uno:HatchListState"_ustr);
    addStatusListener( u".uno:BitmapListState"_ustr);
}

SvxFillToolBoxControl::~SvxFillToolBoxControl()
{
}

void SvxFillToolBoxControl::StateChangedAtToolBoxControl(
    sal_uInt16 nSID,
    SfxItemState eState,
    const SfxPoolItem* pState)
{
    const bool bDisabled(SfxItemState::DISABLED == eState);

    switch(nSID)
    {
        case SID_ATTR_FILL_STYLE:
        {
            if(bDisabled)
            {
                mpLbFillType->set_sensitive(false);
                mpLbFillType->set_active(-1);
                mpLbFillAttr->show();
                mpLbFillAttr->set_sensitive(false);
                mpLbFillAttr->set_active(-1);
                mpToolBoxColor->hide();
                mnLastXFS = -1;
                mpStyleItem.reset();
            }

            if(eState >= SfxItemState::DEFAULT)
            {
                const XFillStyleItem* pItem = dynamic_cast< const XFillStyleItem* >(pState);

                if(pItem)
                {
                    mpStyleItem.reset(pItem->Clone());
                    mpLbFillType->set_sensitive(true);
                    drawing::FillStyle eXFS = mpStyleItem->GetValue();
                    mnLastXFS = sal::static_int_cast< sal_Int32 >(eXFS);
                    mpLbFillType->set_active(mnLastXFS);

                    if(drawing::FillStyle_NONE == eXFS)
                    {
                        mpLbFillAttr->set_active(-1);
                        mpLbFillAttr->set_sensitive(false);
                    }

                    Update();
                    break;
                }
            }

            mpLbFillType->set_active(-1);
            mpLbFillAttr->show();
            mpLbFillAttr->set_sensitive(false);
            mpLbFillAttr->set_active(-1);
            mpToolBoxColor->hide();
            mnLastXFS = -1;
            mpStyleItem.reset();
            mxFillControl->Resize();
            break;
        }
        case SID_ATTR_FILL_COLOR:
        {
            if(SfxItemState::DEFAULT == eState)
            {
                mpColorItem.reset(pState ? static_cast<XFillColorItem*>(pState->Clone()) : nullptr);
            }

            if(mpStyleItem && drawing::FillStyle_SOLID == mpStyleItem->GetValue())
            {
                mpLbFillAttr->hide();
                mpToolBoxColor->show();
                mxFillControl->Resize();

                Update();
            }
            break;
        }
        case SID_ATTR_FILL_GRADIENT:
        {
            if(SfxItemState::DEFAULT == eState)
            {
                mpFillGradientItem.reset(pState ? static_cast<XFillGradientItem*>(pState->Clone()) : nullptr);
            }

            if(mpStyleItem && drawing::FillStyle_GRADIENT == mpStyleItem->GetValue())
            {
                mpLbFillAttr->show();
                mpToolBoxColor->hide();
                mxFillControl->Resize();

                if(SfxItemState::DEFAULT == eState)
                {
                    mpLbFillAttr->set_sensitive(true);
                    Update();
                }
                else if(SfxItemState::DISABLED == eState )
                {
                    mpLbFillAttr->set_sensitive(false);
                    mpLbFillAttr->set_active(-1);
                }
                else
                {
                    mpLbFillAttr->set_active(-1);
                }
            }
            break;
        }
        case SID_ATTR_FILL_HATCH:
        {
            if(SfxItemState::DEFAULT == eState)
            {
                mpHatchItem.reset(pState ? static_cast<XFillHatchItem*>(pState->Clone()) : nullptr);
            }

            if(mpStyleItem && drawing::FillStyle_HATCH == mpStyleItem->GetValue())
            {
                mpLbFillAttr->show();
                mpToolBoxColor->hide();
                mxFillControl->Resize();

                if(SfxItemState::DEFAULT == eState)
                {
                    mpLbFillAttr->set_sensitive(true);
                    Update();
                }
                else if(SfxItemState::DISABLED == eState )
                {
                    mpLbFillAttr->set_sensitive(false);
                    mpLbFillAttr->set_active(-1);
                }
                else
                {
                    mpLbFillAttr->set_active(-1);
                }
            }
            break;
        }
        case SID_ATTR_FILL_BITMAP:
        {
            if(SfxItemState::DEFAULT == eState)
            {
                mpBitmapItem.reset(pState ? static_cast<XFillBitmapItem*>(pState->Clone()) : nullptr);
            }

            if(mpStyleItem && drawing::FillStyle_BITMAP == mpStyleItem->GetValue())
            {
                mpLbFillAttr->show();
                mpToolBoxColor->hide();
                mxFillControl->Resize();

                if(SfxItemState::DEFAULT == eState)
                {
                    mpLbFillAttr->set_sensitive(true);
                    Update();
                }
                else if(SfxItemState::DISABLED == eState )
                {
                    mpLbFillAttr->set_sensitive(false);
                    mpLbFillAttr->set_active(-1);
                }
                else
                {
                    mpLbFillAttr->set_active(-1);
                }
            }
            break;
        }
        case SID_GRADIENT_LIST:
        {
            if(SfxItemState::DEFAULT == eState)
            {
                if(mpStyleItem && drawing::FillStyle_GRADIENT == mpStyleItem->GetValue())
                {
                    if(mpFillGradientItem)
                    {
                        const OUString aString( mpFillGradientItem->GetName() );
                        mpLbFillAttr->clear();
                        if (const SfxObjectShell* pSh = SfxObjectShell::Current())
                        {
                            mpLbFillAttr->set_sensitive(true);
                            SvxFillAttrBox::Fill(*mpLbFillAttr, pSh->GetItem(SID_GRADIENT_LIST)->GetGradientList());
                        }
                        mpLbFillAttr->set_active_text(aString);
                    }
                    else
                    {
                        mpLbFillAttr->set_active(-1);
                    }
                }
            }
            break;
        }
        case SID_HATCH_LIST:
        {
            if(SfxItemState::DEFAULT == eState)
            {
                if(mpStyleItem && drawing::FillStyle_HATCH == mpStyleItem->GetValue())
                {
                    if(mpHatchItem)
                    {
                        const OUString aString( mpHatchItem->GetName() );
                        mpLbFillAttr->clear();
                        if (const SfxObjectShell* pSh = SfxObjectShell::Current())
                        {
                            mpLbFillAttr->set_sensitive(true);
                            SvxFillAttrBox::Fill(*mpLbFillAttr, pSh->GetItem(SID_HATCH_LIST)->GetHatchList());
                        }
                        mpLbFillAttr->set_active_text(aString);
                    }
                    else
                    {
                        mpLbFillAttr->set_active(-1);
                    }
                }
            }
            break;
        }
        case SID_BITMAP_LIST:
        {
            if(SfxItemState::DEFAULT == eState)
            {
                if(mpStyleItem && drawing::FillStyle_BITMAP == mpStyleItem->GetValue())
                {
                    if(mpBitmapItem)
                    {
                        const OUString aString( mpBitmapItem->GetName() );
                        mpLbFillAttr->clear();
                        if (const SfxObjectShell* pSh = SfxObjectShell::Current())
                        {
                            mpLbFillAttr->set_sensitive(true);
                            SvxFillAttrBox::Fill(*mpLbFillAttr, pSh->GetItem(SID_BITMAP_LIST)->GetBitmapList());
                        }
                        mpLbFillAttr->set_active_text(aString);
                    }
                    else
                    {
                        mpLbFillAttr->set_active(-1);
                    }
                }
            }
            break;
        }
    }
}

void SvxFillToolBoxControl::Update()
{
    if(!mpStyleItem)
        return;

    const drawing::FillStyle eXFS = mpStyleItem->GetValue();
    SfxObjectShell* pSh = SfxObjectShell::Current();

    switch( eXFS )
    {
        case drawing::FillStyle_NONE:
        {
            mpLbFillAttr->show();
            mpToolBoxColor->hide();
            mxFillControl->Resize();
            break;
        }
        case drawing::FillStyle_SOLID:
        {
            if(mpColorItem)
            {
                mpLbFillAttr->hide();
                mpToolBoxColor->show();
                mxFillControl->Resize();
            }
            break;
        }
        case drawing::FillStyle_GRADIENT:
        {
            mpLbFillAttr->show();
            mpToolBoxColor->hide();
            mxFillControl->Resize();

            if(pSh && pSh->GetItem(SID_GRADIENT_LIST))
            {
                mpLbFillAttr->set_sensitive(true);
                mpLbFillAttr->clear();
                SvxFillAttrBox::Fill(*mpLbFillAttr, pSh->GetItem(SID_GRADIENT_LIST)->GetGradientList());

                if(mpFillGradientItem)
                {
                    const OUString aString(mpFillGradientItem->GetName());

                    mpLbFillAttr->set_active_text(aString);

                    // Check if the entry is not in the list
                    if (mpLbFillAttr->get_active_text() != aString)
                    {
                        sal_Int32 nCount = mpLbFillAttr->get_count();
                        OUString aTmpStr;
                        if( nCount > 0 )
                        {
                            // Last entry gets tested against temporary entry
                            aTmpStr = mpLbFillAttr->get_text( nCount - 1 );
                            if( aTmpStr.startsWith(TMP_STR_BEGIN) &&
                                aTmpStr.endsWith(TMP_STR_END) )
                            {
                                mpLbFillAttr->remove(nCount - 1);
                            }
                        }
                        aTmpStr = TMP_STR_BEGIN + aString + TMP_STR_END;

                        rtl::Reference<XGradientList> xGradientList = new XGradientList( u""_ustr, u""_ustr/*TODO?*/ );
                        xGradientList->Insert(std::make_unique<XGradientEntry>(mpFillGradientItem->GetGradientValue(), aTmpStr));
                        xGradientList->SetDirty( false );
                        const BitmapEx aBmp = xGradientList->GetUiBitmap( 0 );

                        if (!aBmp.IsEmpty())
                        {
                            ScopedVclPtrInstance< VirtualDevice > pVD;
                            const Size aBmpSize(aBmp.GetSizePixel());
                            pVD->SetOutputSizePixel(aBmpSize, false);
                            pVD->DrawBitmapEx(Point(), aBmp);
                            mpLbFillAttr->append(u""_ustr, xGradientList->Get(0)->GetName(), *pVD);
                            mpLbFillAttr->set_active(mpLbFillAttr->get_count() - 1);
                        }
                    }

                }
                else
                {
                    mpLbFillAttr->set_active(-1);
                }
            }
            else
            {
                mpLbFillAttr->set_active(-1);
            }
            break;
        }
        case drawing::FillStyle_HATCH:
        {
            mpLbFillAttr->show();
            mpToolBoxColor->hide();
            mxFillControl->Resize();

            if(pSh && pSh->GetItem(SID_HATCH_LIST))
            {
                mpLbFillAttr->set_sensitive(true);
                mpLbFillAttr->clear();
                SvxFillAttrBox::Fill(*mpLbFillAttr, pSh->GetItem(SID_HATCH_LIST)->GetHatchList());

                if(mpHatchItem)
                {
                    const OUString aString(mpHatchItem->GetName());

                    mpLbFillAttr->set_active_text( aString );

                    // Check if the entry is not in the list
                    if( mpLbFillAttr->get_active_text() != aString )
                    {
                        const sal_Int32 nCount = mpLbFillAttr->get_count();
                        OUString aTmpStr;
                        if( nCount > 0 )
                        {
                            // Last entry gets tested against temporary entry
                            aTmpStr = mpLbFillAttr->get_text( nCount - 1 );
                            if(  aTmpStr.startsWith(TMP_STR_BEGIN) &&
                                 aTmpStr.endsWith(TMP_STR_END) )
                            {
                                mpLbFillAttr->remove( nCount - 1 );
                            }
                        }
                        aTmpStr = TMP_STR_BEGIN + aString + TMP_STR_END;

                        rtl::Reference<XHatchList> xHatchList = new XHatchList( u""_ustr, u""_ustr/*TODO?*/ );
                        xHatchList->Insert(std::make_unique<XHatchEntry>(mpHatchItem->GetHatchValue(), aTmpStr));
                        xHatchList->SetDirty( false );
                        const BitmapEx & aBmp = xHatchList->GetUiBitmap( 0 );

                        if( !aBmp.IsEmpty() )
                        {
                            ScopedVclPtrInstance< VirtualDevice > pVD;
                            const Size aBmpSize(aBmp.GetSizePixel());
                            pVD->SetOutputSizePixel(aBmpSize, false);
                            pVD->DrawBitmapEx(Point(), aBmp);
                            mpLbFillAttr->append(u""_ustr, xHatchList->GetHatch(0)->GetName(), *pVD);
                            mpLbFillAttr->set_active(mpLbFillAttr->get_count() - 1);
                        }
                    }
                }
                else
                {
                    mpLbFillAttr->set_active(-1);
                }
            }
            else
            {
                mpLbFillAttr->set_active(-1);
            }
            break;
        }
        case drawing::FillStyle_BITMAP:
        {
            mpLbFillAttr->show();
            mpToolBoxColor->hide();
            mxFillControl->Resize();

            if(pSh)
            {
                mpLbFillAttr->set_sensitive(true);
                mpLbFillAttr->clear();

                if(mpBitmapItem && !mpBitmapItem->isPattern() && pSh->GetItem(SID_BITMAP_LIST))
                {
                    SvxFillAttrBox::Fill(*mpLbFillAttr, pSh->GetItem(SID_BITMAP_LIST)->GetBitmapList());

                    const OUString aString(mpBitmapItem->GetName());

                    mpLbFillAttr->set_active_text(aString);

                    // Check if the entry is not in the list
                    if (mpLbFillAttr->get_active_text() != aString)
                    {
                        sal_Int32 nCount = mpLbFillAttr->get_count();
                        OUString aTmpStr;
                        if( nCount > 0 )
                        {
                            // Last entry gets tested against temporary entry
                            aTmpStr = mpLbFillAttr->get_text(nCount - 1);
                            if( aTmpStr.startsWith(TMP_STR_BEGIN) &&
                                aTmpStr.endsWith(TMP_STR_END) )
                            {
                                mpLbFillAttr->remove(nCount - 1);
                            }
                        }
                        aTmpStr = TMP_STR_BEGIN + aString + TMP_STR_END;

                        XBitmapListRef xBitmapList =
                            XPropertyList::AsBitmapList(
                                XPropertyList::CreatePropertyList(
                                    XPropertyListType::Bitmap, u"TmpList"_ustr, u""_ustr/*TODO?*/));
                        xBitmapList->Insert(std::make_unique<XBitmapEntry>(mpBitmapItem->GetGraphicObject(), aTmpStr));
                        xBitmapList->SetDirty( false );
                        SvxFillAttrBox::Fill(*mpLbFillAttr, xBitmapList);
                        mpLbFillAttr->set_active(mpLbFillAttr->get_count() - 1);
                    }

                }
                else if (mpBitmapItem && mpBitmapItem->isPattern() && pSh->GetItem(SID_PATTERN_LIST))
                {
                    mnLastXFS = sal::static_int_cast<sal_Int32>(PATTERN);
                    mpLbFillType->set_active(mnLastXFS);

                    SvxFillAttrBox::Fill(*mpLbFillAttr, pSh->GetItem(SID_PATTERN_LIST)->GetPatternList());
                    const OUString aString(mpBitmapItem->GetName());

                    mpLbFillAttr->set_active_text(aString);
                }
                else
                {
                    mpLbFillAttr->set_active(-1);
                }
            }
            else
            {
                mpLbFillAttr->set_active(-1);
            }
            break;
        }
        default:
            OSL_ENSURE(false, "Non supported FillType (!)");
        break;
    }

}

VclPtr<InterimItemWindow> SvxFillToolBoxControl::CreateItemWindow(vcl::Window *pParent)
{
    if(GetSlotId() == SID_ATTR_FILL_STYLE)
    {
        mxFillControl.reset(VclPtr<FillControl>::Create(pParent, m_xFrame));

        mpLbFillType = mxFillControl->mxLbFillType.get();
        mpLbFillAttr = mxFillControl->mxLbFillAttr.get();
        mpToolBoxColor = mxFillControl->mxToolBoxColor.get();

        mpLbFillType->connect_changed(LINK(this,SvxFillToolBoxControl,SelectFillTypeHdl));
        mpLbFillAttr->connect_changed(LINK(this,SvxFillToolBoxControl,SelectFillAttrHdl));


        return mxFillControl;
    }
    return VclPtr<InterimItemWindow>();
}

FillControl::FillControl(vcl::Window* pParent, const css::uno::Reference<css::frame::XFrame>& rFrame)
    : InterimItemWindow(pParent, u"svx/ui/fillctrlbox.ui"_ustr, u"FillCtrlBox"_ustr)
    , mxLbFillType(m_xBuilder->weld_combo_box(u"type"_ustr))
    , mxToolBoxColor(m_xBuilder->weld_toolbar(u"color"_ustr))
    , mxColorDispatch(new ToolbarUnoDispatcher(*mxToolBoxColor, *m_xBuilder, rFrame))
    , mxLbFillAttr(m_xBuilder->weld_combo_box(u"attr"_ustr))
    , mnTypeCurPos(0)
    , mnAttrCurPos(0)
{
    InitControlBase(mxLbFillType.get());

    mxLbFillAttr->connect_key_press(LINK(this, FillControl, AttrKeyInputHdl));
    mxLbFillType->connect_key_press(LINK(this, FillControl, TypeKeyInputHdl));
    mxToolBoxColor->connect_key_press(LINK(this, FillControl, ColorKeyInputHdl));

    mxLbFillType->connect_get_property_tree(LINK(this, FillControl, DumpAsPropertyTreeHdl));

    mxLbFillType->connect_focus_in(LINK(this, FillControl, TypeFocusHdl));
    mxLbFillAttr->connect_focus_in(LINK(this, FillControl, AttrFocusHdl));

    SvxFillTypeBox::Fill(*mxLbFillType);

    SetOptimalSize();
}

IMPL_STATIC_LINK(FillControl, DumpAsPropertyTreeHdl, tools::JsonWriter&, rJsonWriter, void)
{
    rJsonWriter.put("command", ".uno:FillStyle");
}

void FillControl::ReleaseFocus_Impl()
{
    SfxViewShell* pCurSh = SfxViewShell::Current();
    if (pCurSh)
    {
        vcl::Window* pShellWnd = pCurSh->GetWindow();
        if (pShellWnd)
            pShellWnd->GrabFocus();
    }
}

IMPL_LINK(FillControl, TypeKeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    const vcl::KeyCode& rKeyCode = rKEvt.GetKeyCode();
    sal_uInt16 nCode = rKeyCode.GetCode();

    if (nCode == KEY_ESCAPE)
    {
        mxLbFillType->set_active(mnTypeCurPos);
        ReleaseFocus_Impl();
        return true;
    }

    if (nCode != KEY_TAB)
        return false;
    if (rKeyCode.IsShift())
        return ChildKeyInput(rKEvt);
    if (mxLbFillAttr->get_visible() && !mxLbFillAttr->get_sensitive())
        return ChildKeyInput(rKEvt);
    return false;
}

IMPL_LINK_NOARG(FillControl, TypeFocusHdl, weld::Widget&, void)
{
    mnTypeCurPos = mxLbFillType->get_active();
}

IMPL_LINK_NOARG(FillControl, AttrFocusHdl, weld::Widget&, void)
{
    mnAttrCurPos = mxLbFillAttr->get_active();
}

IMPL_LINK(FillControl, AttrKeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    const vcl::KeyCode& rKeyCode = rKEvt.GetKeyCode();
    sal_uInt16 nCode = rKeyCode.GetCode();

    if (nCode == KEY_ESCAPE)
    {
        mxLbFillAttr->set_active(mnAttrCurPos);
        ReleaseFocus_Impl();
        return true;
    }

    return ChildKeyInput(rKEvt);
}

IMPL_LINK(FillControl, ColorKeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    return ChildKeyInput(rKEvt);
}

FillControl::~FillControl()
{
    disposeOnce();
}

void FillControl::dispose()
{
    mxLbFillAttr.reset();
    mxColorDispatch.reset();
    mxToolBoxColor.reset();
    mxLbFillType.reset();
    InterimItemWindow::dispose();
}

IMPL_LINK_NOARG(SvxFillToolBoxControl, SelectFillTypeHdl, weld::ComboBox&, void)
{
    sal_Int32 nXFS = mpLbFillType->get_active();

    if(mnLastXFS == nXFS)
        return;

    eFillStyle eXFS = static_cast<eFillStyle>(nXFS);
    mpLbFillAttr->clear();
    SfxObjectShell* pSh = SfxObjectShell::Current();
    const XFillStyleItem aXFillStyleItem(toCssFillStyle(eXFS));

    // #i122676# Do no longer trigger two Execute calls, one for SID_ATTR_FILL_STYLE
    // and one for setting the fill attribute itself, but add two SfxPoolItems to the
    // call to get just one action at the SdrObject and to create only one Undo action, too.
    // Checked that this works in all apps.
    switch( eXFS )
    {
        default:
        case NONE:
        {
            mpLbFillAttr->show();
            mpToolBoxColor->hide();
            mpLbFillAttr->set_sensitive(false);
            if (pSh)
            {
                // #i122676# need to call a single SID_ATTR_FILL_STYLE change
                pSh->GetDispatcher()->ExecuteList(
                    SID_ATTR_FILL_STYLE, SfxCallMode::RECORD,
                    { &aXFillStyleItem });
            }
            break;
        }
        case SOLID:
        {
            mpLbFillAttr->hide();
            mpToolBoxColor->show();
            if (pSh)
            {
                const ::Color aColor = mpColorItem ? mpColorItem->GetColorValue() : COL_AUTO;
                const XFillColorItem aXFillColorItem( u""_ustr, aColor );

                // #i122676# change FillStyle and Color in one call
                pSh->GetDispatcher()->ExecuteList(
                    SID_ATTR_FILL_COLOR, SfxCallMode::RECORD,
                    { &aXFillColorItem, &aXFillStyleItem });
            }
            break;
        }
        case GRADIENT:
        {
            mpLbFillAttr->show();
            mpToolBoxColor->hide();

            if(pSh && pSh->GetItem(SID_GRADIENT_LIST))
            {
                if(!mpLbFillAttr->get_count())
                {
                    mpLbFillAttr->set_sensitive(true);
                    mpLbFillAttr->clear();
                    SvxFillAttrBox::Fill(*mpLbFillAttr, pSh->GetItem(SID_GRADIENT_LIST)->GetGradientList());
                }

                if (mnLastPosGradient != -1)
                {
                    const SvxGradientListItem * pItem = pSh->GetItem(SID_GRADIENT_LIST);

                    if(mnLastPosGradient < pItem->GetGradientList()->Count())
                    {
                        const basegfx::BGradient aGradient = pItem->GetGradientList()->GetGradient(mnLastPosGradient)->GetGradient();
                        const XFillGradientItem aXFillGradientItem(mpLbFillAttr->get_text(mnLastPosGradient), aGradient);

                        // #i122676# change FillStyle and Gradient in one call
                        pSh->GetDispatcher()->ExecuteList(
                            SID_ATTR_FILL_GRADIENT, SfxCallMode::RECORD,
                            { &aXFillGradientItem, &aXFillStyleItem });
                        mpLbFillAttr->set_active(mnLastPosGradient);
                    }
                }
            }
            else
            {
                mpLbFillAttr->set_sensitive(false);
            }
            break;
        }
        case HATCH:
        {
            mpLbFillAttr->show();
            mpToolBoxColor->hide();

            if(pSh && pSh->GetItem(SID_HATCH_LIST))
            {
                if(!mpLbFillAttr->get_count())
                {
                    mpLbFillAttr->set_sensitive(true);
                    mpLbFillAttr->clear();
                    SvxFillAttrBox::Fill(*mpLbFillAttr, pSh->GetItem(SID_HATCH_LIST)->GetHatchList());
                }

                if (mnLastPosHatch != -1)
                {
                    const SvxHatchListItem * pItem = pSh->GetItem(SID_HATCH_LIST);

                    if(mnLastPosHatch < pItem->GetHatchList()->Count())
                    {
                        const XHatch aHatch = pItem->GetHatchList()->GetHatch(mnLastPosHatch)->GetHatch();
                        const XFillHatchItem aXFillHatchItem(mpLbFillAttr->get_active_text(), aHatch);

                        // #i122676# change FillStyle and Hatch in one call
                        pSh->GetDispatcher()->ExecuteList(
                            SID_ATTR_FILL_HATCH, SfxCallMode::RECORD,
                            { &aXFillHatchItem, &aXFillStyleItem });
                        mpLbFillAttr->set_active(mnLastPosHatch);
                    }
                }
            }
            else
            {
                mpLbFillAttr->set_sensitive(false);
            }
            break;
        }
        case BITMAP:
        {
            mpLbFillAttr->show();
            mpToolBoxColor->hide();

            if(pSh && pSh->GetItem(SID_BITMAP_LIST))
            {
                if(!mpLbFillAttr->get_count())
                {
                    mpLbFillAttr->set_sensitive(true);
                    mpLbFillAttr->clear();
                    SvxFillAttrBox::Fill(*mpLbFillAttr, pSh->GetItem(SID_BITMAP_LIST)->GetBitmapList());
                }

                if (mnLastPosBitmap != -1)
                {
                    const SvxBitmapListItem * pItem = pSh->GetItem(SID_BITMAP_LIST);

                    if(mnLastPosBitmap < pItem->GetBitmapList()->Count())
                    {
                        const XBitmapEntry* pXBitmapEntry = pItem->GetBitmapList()->GetBitmap(mnLastPosBitmap);
                        const XFillBitmapItem aXFillBitmapItem(mpLbFillAttr->get_active_text(), pXBitmapEntry->GetGraphicObject());

                        // #i122676# change FillStyle and Bitmap in one call
                        pSh->GetDispatcher()->ExecuteList(
                            SID_ATTR_FILL_BITMAP, SfxCallMode::RECORD,
                            { &aXFillBitmapItem, &aXFillStyleItem });
                        mpLbFillAttr->set_active(mnLastPosBitmap);
                    }
                }
            }
            else
            {
                mpLbFillAttr->set_sensitive(false);
            }
            break;
        }
        case PATTERN:
        {
            mpLbFillAttr->show();
            mpToolBoxColor->hide();

            if(pSh && pSh->GetItem(SID_PATTERN_LIST))
            {
                if(!mpLbFillAttr->get_count())
                {
                    mpLbFillAttr->set_sensitive(true);
                    mpLbFillAttr->clear();
                    SvxFillAttrBox::Fill(*mpLbFillAttr, pSh->GetItem(SID_PATTERN_LIST)->GetPatternList());
                }

                if (mnLastPosPattern != -1)
                {
                    const SvxPatternListItem * pItem = pSh->GetItem(SID_PATTERN_LIST);

                    if(mnLastPosPattern < pItem->GetPatternList()->Count())
                    {
                        const XBitmapEntry* pXBitmapEntry = pItem->GetPatternList()->GetBitmap(mnLastPosPattern);
                        const XFillBitmapItem aXFillBitmapItem(mpLbFillAttr->get_active_text(), pXBitmapEntry->GetGraphicObject());

                        // #i122676# change FillStyle and Bitmap in one call
                        pSh->GetDispatcher()->ExecuteList(
                            SID_ATTR_FILL_BITMAP, SfxCallMode::RECORD,
                            { &aXFillBitmapItem, &aXFillStyleItem });
                        mpLbFillAttr->set_active(mnLastPosPattern);
                    }
                }
            }
            else
            {
                mpLbFillAttr->set_sensitive(false);
            }
            break;
        }

    }

    mnLastXFS = nXFS;

    mxFillControl->Resize();
}

IMPL_LINK_NOARG(SvxFillToolBoxControl, SelectFillAttrHdl, weld::ComboBox&, void)
{
    sal_Int32 nXFS = mpLbFillType->get_active();
    eFillStyle eXFS = static_cast<eFillStyle>(nXFS);

    const XFillStyleItem aXFillStyleItem(toCssFillStyle(eXFS));
    SfxObjectShell* pSh = SfxObjectShell::Current();

    // #i122676# dependent from bFillStyleChange, do execute a single or two
    // changes in one Execute call
    const bool bFillStyleChange(mnLastXFS != nXFS);

    switch (eXFS)
    {
        case SOLID:
        {
            if (bFillStyleChange && pSh)
            {
                // #i122676# Single FillStyle change call needed here
                pSh->GetDispatcher()->ExecuteList(
                    SID_ATTR_FILL_STYLE, SfxCallMode::RECORD,
                    { &aXFillStyleItem });
            }
            break;
        }
        case GRADIENT:
        {
            sal_Int32 nPos = mpLbFillAttr->get_active();

            if (nPos == -1)
            {
                nPos = mnLastPosGradient;
            }

            if (nPos != -1 && pSh && pSh->GetItem(SID_GRADIENT_LIST))
            {
                const SvxGradientListItem * pItem = pSh->GetItem(SID_GRADIENT_LIST);

                if(nPos < pItem->GetGradientList()->Count())
                {
                    const basegfx::BGradient aGradient = pItem->GetGradientList()->GetGradient(nPos)->GetGradient();
                    const XFillGradientItem aXFillGradientItem(mpLbFillAttr->get_active_text(), aGradient);

                    // #i122676# Change FillStyle and Gradient in one call
                    pSh->GetDispatcher()->ExecuteList(
                        SID_ATTR_FILL_GRADIENT, SfxCallMode::RECORD,
                        bFillStyleChange
                            ? std::initializer_list<SfxPoolItem const*>{ &aXFillGradientItem, &aXFillStyleItem }
                            : std::initializer_list<SfxPoolItem const*>{ &aXFillGradientItem });
                }
            }

            if (nPos != -1)
            {
                mnLastPosGradient = nPos;
            }
            break;
        }
        case HATCH:
        {
            sal_Int32 nPos = mpLbFillAttr->get_active();

            if (nPos == -1)
            {
                nPos = mnLastPosHatch;
            }

            if (nPos != -1 && pSh && pSh->GetItem(SID_HATCH_LIST))
            {
                const SvxHatchListItem * pItem = pSh->GetItem(SID_HATCH_LIST);

                if(nPos < pItem->GetHatchList()->Count())
                {
                    const XHatch aHatch = pItem->GetHatchList()->GetHatch(nPos)->GetHatch();
                    const XFillHatchItem aXFillHatchItem( mpLbFillAttr->get_active_text(), aHatch);

                    // #i122676# Change FillStyle and Hatch in one call
                    pSh->GetDispatcher()->ExecuteList(
                        SID_ATTR_FILL_HATCH, SfxCallMode::RECORD,
                        bFillStyleChange
                            ? std::initializer_list<SfxPoolItem const*>{ &aXFillHatchItem, &aXFillStyleItem }
                            : std::initializer_list<SfxPoolItem const*>{ &aXFillHatchItem });
                }
            }

            if (nPos != -1)
            {
                mnLastPosHatch = nPos;
            }
            break;
        }
        case BITMAP:
        {
            sal_Int32 nPos = mpLbFillAttr->get_active();

            if (nPos == -1)
            {
                nPos = mnLastPosBitmap;
            }

            if (nPos != -1 && pSh && pSh->GetItem(SID_BITMAP_LIST))
            {
                const SvxBitmapListItem * pItem = pSh->GetItem(SID_BITMAP_LIST);

                if(nPos < pItem->GetBitmapList()->Count())
                {
                    const XBitmapEntry* pXBitmapEntry = pItem->GetBitmapList()->GetBitmap(nPos);
                    const XFillBitmapItem aXFillBitmapItem(mpLbFillAttr->get_active_text(), pXBitmapEntry->GetGraphicObject());

                    // #i122676# Change FillStyle and Bitmap in one call
                    pSh->GetDispatcher()->ExecuteList(
                        SID_ATTR_FILL_BITMAP, SfxCallMode::RECORD,
                        bFillStyleChange
                            ? std::initializer_list<SfxPoolItem const*>{ &aXFillBitmapItem, &aXFillStyleItem }
                            : std::initializer_list<SfxPoolItem const*>{ &aXFillBitmapItem });
                }
            }

            if (nPos != -1)
            {
                mnLastPosBitmap = nPos;
            }
            break;
        }
        case PATTERN:
        {
            sal_Int32 nPos = mpLbFillAttr->get_active();

            if (nPos == -1)
            {
                nPos = mnLastPosPattern;
            }

            if (nPos != -1 && pSh && pSh->GetItem(SID_PATTERN_LIST))
            {
                const SvxPatternListItem * pItem = pSh->GetItem(SID_PATTERN_LIST);

                if(nPos < pItem->GetPatternList()->Count())
                {
                    const XBitmapEntry* pXBitmapEntry = pItem->GetPatternList()->GetBitmap(nPos);
                    const XFillBitmapItem aXFillBitmapItem(mpLbFillAttr->get_active_text(), pXBitmapEntry->GetGraphicObject());

                    // #i122676# Change FillStyle and Bitmap in one call
                    pSh->GetDispatcher()->ExecuteList(
                        SID_ATTR_FILL_BITMAP, SfxCallMode::RECORD,
                        bFillStyleChange
                            ? std::initializer_list<SfxPoolItem const*>{ &aXFillBitmapItem, &aXFillStyleItem }
                            : std::initializer_list<SfxPoolItem const*>{ &aXFillBitmapItem });
                }
            }

            if (nPos != -1)
            {
                mnLastPosPattern = nPos;
            }
            break;
        }

        default: break;
    }
}

void FillControl::SetOptimalSize()
{
    Size aSize(mxLbFillType->get_preferred_size());
    Size aFirstSize(mxToolBoxColor->get_preferred_size());
    auto nWidth = std::max(aFirstSize.Width(), LogicToPixel(Size(55, 0), MapMode(MapUnit::MapAppFont)).Width());
    auto nHeight = std::max(aSize.Height(), aFirstSize.Height());
    mxToolBoxColor->set_size_request(nWidth, -1);
    mxLbFillAttr->set_size_request(42, -1); //something narrow so the toolbar sets the overall size of this column
    SetSizePixel(Size(m_xContainer->get_preferred_size().Width(), nHeight));
}

void FillControl::DataChanged(const DataChangedEvent& rDCEvt)
{
    if((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
        (rDCEvt.GetFlags() & AllSettingsFlags::STYLE))
    {
        SetOptimalSize();
    }
    InterimItemWindow::DataChanged(rDCEvt);
}

void FillControl::GetFocus()
{
    // tdf#148047 if the dropdown is active then leave the focus
    // there and don't grab back to a different widget
    if (mxToolBoxColor->get_menu_item_active(u".uno:FillColor"_ustr))
        return;
    InterimItemWindow::GetFocus();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
