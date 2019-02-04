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

#include <string>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/sidebar/SidebarToolBox.hxx>
#include <rtl/ustring.hxx>
#include <vcl/settings.hxx>
#include <vcl/toolbox.hxx>
#include <svx/svxids.hrc>

#define TMP_STR_BEGIN   "["
#define TMP_STR_END     "]"

#include <svx/drawitem.hxx>
#include <svx/xattr.hxx>
#include <svx/xtable.hxx>
#include <svx/fillctrl.hxx>
#include <svx/itemwin.hxx>
#include <memory>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

SFX_IMPL_TOOLBOX_CONTROL( SvxFillToolBoxControl, XFillStyleItem );

SvxFillToolBoxControl::SvxFillToolBoxControl(
    sal_uInt16 nSlotId,
    sal_uInt16 nId,
    ToolBox& rTbx )
    : SfxToolBoxControl( nSlotId, nId, rTbx )
    , mpStyleItem()
    , mpColorItem()
    , mpFillGradientItem()
    , mpHatchItem()
    , mpBitmapItem()
    , mpFillControl(nullptr)
    , mpLbFillType(nullptr)
    , mpLbFillAttr(nullptr)
    , meLastXFS(static_cast<drawing::FillStyle>(-1))
    , mnLastPosGradient(0)
    , mnLastPosHatch(0)
    , mnLastPosBitmap(0)
{
    addStatusListener( ".uno:FillColor");
    addStatusListener( ".uno:FillGradient");
    addStatusListener( ".uno:FillHatch");
    addStatusListener( ".uno:FillBitmap");
    addStatusListener( ".uno:ColorTableState");
    addStatusListener( ".uno:GradientListState");
    addStatusListener( ".uno:HatchListState");
    addStatusListener( ".uno:BitmapListState");
}

SvxFillToolBoxControl::~SvxFillToolBoxControl()
{
}

void SvxFillToolBoxControl::StateChanged(
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
                mpLbFillType->Disable();
                mpLbFillType->SetNoSelection();
                mpLbFillAttr->Show();
                mpLbFillAttr->Disable();
                mpLbFillAttr->SetNoSelection();
                mpToolBoxColor->Hide();
                meLastXFS = static_cast<drawing::FillStyle>(-1);
                mpStyleItem.reset();
            }

            if(eState >= SfxItemState::DEFAULT)
            {
                const XFillStyleItem* pItem = dynamic_cast< const XFillStyleItem* >(pState);

                if(pItem)
                {
                    mpStyleItem.reset(dynamic_cast< XFillStyleItem* >(pItem->Clone()));
                    mpLbFillType->Enable();
                    drawing::FillStyle eXFS = mpStyleItem->GetValue();
                    meLastXFS = eXFS;
                    mpLbFillType->SelectEntryPos(sal::static_int_cast< sal_Int32 >(eXFS));

                    if(drawing::FillStyle_NONE == eXFS)
                    {
                        mpLbFillAttr->SetNoSelection();
                        mpLbFillAttr->Disable();
                    }

                    Update();
                    break;
                }
            }

            mpLbFillType->SetNoSelection();
            mpLbFillAttr->Show();
            mpLbFillAttr->Disable();
            mpLbFillAttr->SetNoSelection();
            mpToolBoxColor->Hide();
            meLastXFS = static_cast<drawing::FillStyle>(-1);
            mpStyleItem.reset();
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
                mpLbFillAttr->Hide();
                mpToolBoxColor->Show();

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
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();

                if(SfxItemState::DEFAULT == eState)
                {
                    mpLbFillAttr->Enable();
                    Update();
                }
                else if(SfxItemState::DISABLED == eState )
                {
                    mpLbFillAttr->Disable();
                    mpLbFillAttr->SetNoSelection();
                }
                else
                {
                    mpLbFillAttr->SetNoSelection();
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
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();

                if(SfxItemState::DEFAULT == eState)
                {
                    mpLbFillAttr->Enable();
                    Update();
                }
                else if(SfxItemState::DISABLED == eState )
                {
                    mpLbFillAttr->Disable();
                    mpLbFillAttr->SetNoSelection();
                }
                else
                {
                    mpLbFillAttr->SetNoSelection();
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
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();

                if(SfxItemState::DEFAULT == eState)
                {
                    mpLbFillAttr->Enable();
                    Update();
                }
                else if(SfxItemState::DISABLED == eState )
                {
                    mpLbFillAttr->Disable();
                    mpLbFillAttr->SetNoSelection();
                }
                else
                {
                    mpLbFillAttr->SetNoSelection();
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
                        const SfxObjectShell* pSh = SfxObjectShell::Current();

                        mpLbFillAttr->Clear();
                        mpLbFillAttr->Enable();
                        mpLbFillAttr->Fill(pSh->GetItem(SID_GRADIENT_LIST)->GetGradientList());
                        mpLbFillAttr->SelectEntry(aString);
                    }
                    else
                    {
                        mpLbFillAttr->SetNoSelection();
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
                        const SfxObjectShell* pSh = SfxObjectShell::Current();

                        mpLbFillAttr->Clear();
                        mpLbFillAttr->Enable();
                        mpLbFillAttr->Fill(pSh->GetItem(SID_HATCH_LIST)->GetHatchList());
                        mpLbFillAttr->SelectEntry(aString);
                    }
                    else
                    {
                        mpLbFillAttr->SetNoSelection();
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
                        const SfxObjectShell* pSh = SfxObjectShell::Current();

                        mpLbFillAttr->Clear();
                        mpLbFillAttr->Enable();
                        mpLbFillAttr->Fill(pSh->GetItem(SID_BITMAP_LIST)->GetBitmapList());
                        mpLbFillAttr->SelectEntry(aString);
                    }
                    else
                    {
                        mpLbFillAttr->SetNoSelection();
                    }
                }
            }
            break;
        }
    }
}

void SvxFillToolBoxControl::Update()
{
    if(mpStyleItem)
    {
        const drawing::FillStyle eXFS = mpStyleItem->GetValue();
        SfxObjectShell* pSh = SfxObjectShell::Current();

        switch( eXFS )
        {
            case drawing::FillStyle_NONE:
            {
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();
                break;
            }
            case drawing::FillStyle_SOLID:
            {
                if(mpColorItem)
                {
                    mpLbFillAttr->Hide();
                    mpToolBoxColor->Show();
                }
                break;
            }
            case drawing::FillStyle_GRADIENT:
            {
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();

                if(pSh && pSh->GetItem(SID_GRADIENT_LIST))
                {
                    mpLbFillAttr->Enable();
                    mpLbFillAttr->Clear();
                    mpLbFillAttr->Fill(pSh->GetItem(SID_GRADIENT_LIST)->GetGradientList());

                    if(mpFillGradientItem)
                    {
                        const OUString aString(mpFillGradientItem->GetName());

                        mpLbFillAttr->SelectEntry(aString);

                        // Check if the entry is not in the list
                        if (mpLbFillAttr->GetSelectedEntry() != aString)
                        {
                            sal_Int32 nCount = mpLbFillAttr->GetEntryCount();
                            OUString aTmpStr;
                            if( nCount > 0 )
                            {
                                // Last entry gets tested against temporary entry
                                aTmpStr = mpLbFillAttr->GetEntry( nCount - 1 );
                                if( aTmpStr.startsWith(TMP_STR_BEGIN) &&
                                    aTmpStr.endsWith(TMP_STR_END) )
                                {
                                    mpLbFillAttr->RemoveEntry(nCount - 1);
                                }
                            }
                            aTmpStr = TMP_STR_BEGIN + aString + TMP_STR_END;

                            XGradientList aGradientList( "", ""/*TODO?*/ );
                            aGradientList.Insert(std::make_unique<XGradientEntry>(mpFillGradientItem->GetGradientValue(), aTmpStr));
                            aGradientList.SetDirty( false );
                            const BitmapEx aBmp = aGradientList.GetUiBitmap( 0 );

                            if(!aBmp.IsEmpty())
                            {
                                mpLbFillAttr->InsertEntry(aGradientList.Get(0)->GetName(), Image(aBmp));
                                mpLbFillAttr->SelectEntryPos(mpLbFillAttr->GetEntryCount() - 1);
                            }
                        }

                    }
                    else
                    {
                        mpLbFillAttr->SetNoSelection();
                    }
                }
                else
                {
                    mpLbFillAttr->SetNoSelection();
                }
                break;
            }
            case drawing::FillStyle_HATCH:
            {
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();

                if(pSh && pSh->GetItem(SID_HATCH_LIST))
                {
                    mpLbFillAttr->Enable();
                    mpLbFillAttr->Clear();
                    mpLbFillAttr->Fill(pSh->GetItem(SID_HATCH_LIST)->GetHatchList());

                    if(mpHatchItem)
                    {
                        const OUString aString(mpHatchItem->GetName());

                        mpLbFillAttr->SelectEntry( aString );

                        // Check if the entry is not in the list
                        if( mpLbFillAttr->GetSelectedEntry() != aString )
                        {
                            const sal_Int32 nCount = mpLbFillAttr->GetEntryCount();
                            OUString aTmpStr;
                            if( nCount > 0 )
                            {
                                // Last entry gets tested against temporary entry
                                aTmpStr = mpLbFillAttr->GetEntry( nCount - 1 );
                                if(  aTmpStr.startsWith(TMP_STR_BEGIN) &&
                                     aTmpStr.endsWith(TMP_STR_END) )
                                {
                                    mpLbFillAttr->RemoveEntry( nCount - 1 );
                                }
                            }
                            aTmpStr = TMP_STR_BEGIN + aString + TMP_STR_END;

                            XHatchList aHatchList( "", ""/*TODO?*/ );
                            aHatchList.Insert(std::make_unique<XHatchEntry>(mpHatchItem->GetHatchValue(), aTmpStr));
                            aHatchList.SetDirty( false );
                            const BitmapEx & aBmp = aHatchList.GetUiBitmap( 0 );

                            if( !aBmp.IsEmpty() )
                            {
                                mpLbFillAttr->InsertEntry(aHatchList.GetHatch(0)->GetName(), Image(aBmp));
                                mpLbFillAttr->SelectEntryPos( mpLbFillAttr->GetEntryCount() - 1 );
                                //delete pBmp;
                            }
                        }
                    }
                    else
                    {
                        mpLbFillAttr->SetNoSelection();
                    }
                }
                else
                {
                    mpLbFillAttr->SetNoSelection();
                }
                break;
            }
            case drawing::FillStyle_BITMAP:
            {
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();

                if(pSh && pSh->GetItem(SID_BITMAP_LIST))
                {
                    mpLbFillAttr->Enable();
                    mpLbFillAttr->Clear();
                    mpLbFillAttr->Fill(pSh->GetItem(SID_BITMAP_LIST)->GetBitmapList());

                    if(mpBitmapItem)
                    {
                        const OUString aString(mpBitmapItem->GetName());

                        mpLbFillAttr->SelectEntry(aString);

                        // Check if the entry is not in the list
                        if (mpLbFillAttr->GetSelectedEntry() != aString)
                        {
                            sal_Int32 nCount = mpLbFillAttr->GetEntryCount();
                            OUString aTmpStr;
                            if( nCount > 0 )
                            {
                                // Last entry gets tested against temporary entry
                                aTmpStr = mpLbFillAttr->GetEntry(nCount - 1);
                                if( aTmpStr.startsWith(TMP_STR_BEGIN) &&
                                    aTmpStr.endsWith(TMP_STR_END) )
                                {
                                    mpLbFillAttr->RemoveEntry(nCount - 1);
                                }
                            }
                            aTmpStr = TMP_STR_BEGIN + aString + TMP_STR_END;

                            XBitmapListRef xBitmapList =
                                XPropertyList::AsBitmapList(
                                    XPropertyList::CreatePropertyList(
                                        XPropertyListType::Bitmap, "TmpList", ""/*TODO?*/));
                            xBitmapList->Insert(std::make_unique<XBitmapEntry>(mpBitmapItem->GetGraphicObject(), aTmpStr));
                            xBitmapList->SetDirty( false );
                            mpLbFillAttr->Fill( xBitmapList );
                            mpLbFillAttr->SelectEntryPos(mpLbFillAttr->GetEntryCount() - 1);
                        }

                    }
                    else
                    {
                        mpLbFillAttr->SetNoSelection();
                    }
                }
                else
                {
                    mpLbFillAttr->SetNoSelection();
                }
                break;
            }
            default:
                OSL_ENSURE(false, "Non supported FillType (!)");
            break;
        }
    }

}

VclPtr<vcl::Window> SvxFillToolBoxControl::CreateItemWindow(vcl::Window *pParent)
{
    if(GetSlotId() == SID_ATTR_FILL_STYLE)
    {
        mpFillControl.reset(VclPtr<FillControl>::Create(pParent));

        mpLbFillType = mpFillControl->mpLbFillType;
        mpLbFillAttr = mpFillControl->mpLbFillAttr;
        mpToolBoxColor = mpFillControl->mpToolBoxColor;
        mpFillControl->SetBackground();
        mpFillControl->SetPaintTransparent( true );
        mpFillControl->Resize();
        mpToolBoxColor->InsertItem(".uno:FillColor", m_xFrame, ToolBoxItemBits::DROPDOWNONLY, Size(mpToolBoxColor->GetSizePixel().Width(), 0));

        mpLbFillType->SetSelectHdl(LINK(this,SvxFillToolBoxControl,SelectFillTypeHdl));
        mpLbFillAttr->SetSelectHdl(LINK(this,SvxFillToolBoxControl,SelectFillAttrHdl));

        return mpFillControl.get();
    }
    return VclPtr<vcl::Window>();
}

FillControl::FillControl(vcl::Window* pParent)
    : Window(pParent, WB_DIALOGCONTROL)
    , mpLbFillType(VclPtr<SvxFillTypeBox>::Create(this))
    , mpToolBoxColor(VclPtr<sfx2::sidebar::SidebarToolBox>::Create(this))
    , mpLbFillAttr(VclPtr<SvxFillAttrBox>::Create(this))
{
    SetOptimalSize();
}

FillControl::~FillControl()
{
    disposeOnce();
}

void FillControl::dispose()
{
    mpLbFillType.disposeAndClear();
    mpToolBoxColor.disposeAndClear();
    mpLbFillAttr.disposeAndClear();
    vcl::Window::dispose();
}

IMPL_LINK_NOARG(SvxFillToolBoxControl, SelectFillTypeHdl, ListBox&, void)
{
    const drawing::FillStyle eXFS = static_cast<drawing::FillStyle>(mpLbFillType->GetSelectedEntryPos());

    if(meLastXFS != eXFS)
    {
        mpLbFillAttr->Clear();
        SfxObjectShell* pSh = SfxObjectShell::Current();
        const XFillStyleItem aXFillStyleItem(eXFS);

        // #i122676# Do no longer trigger two Execute calls, one for SID_ATTR_FILL_STYLE
        // and one for setting the fill attribute itself, but add two SfxPoolItems to the
        // call to get just one action at the SdrObject and to create only one Undo action, too.
        // Checked that this works in all apps.
        switch( eXFS )
        {
            default:
            case drawing::FillStyle_NONE:
            {
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();
                mpLbFillType->Selected();
                mpLbFillAttr->Disable();

                // #i122676# need to call a single SID_ATTR_FILL_STYLE change
                SfxViewFrame::Current()->GetDispatcher()->ExecuteList(
                    SID_ATTR_FILL_STYLE, SfxCallMode::RECORD,
                    { &aXFillStyleItem });
                break;
            }
            case drawing::FillStyle_SOLID:
            {
                mpLbFillAttr->Hide();
                mpToolBoxColor->Show();
                const OUString aTmpStr;
                const ::Color aColor = mpColorItem->GetColorValue();
                const XFillColorItem aXFillColorItem( aTmpStr, aColor );

                // #i122676# change FillStyle and Color in one call
                SfxViewFrame::Current()->GetDispatcher()->ExecuteList(
                    SID_ATTR_FILL_COLOR, SfxCallMode::RECORD,
                    { &aXFillColorItem, &aXFillStyleItem });
                break;
            }
            case drawing::FillStyle_GRADIENT:
            {
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();

                if(pSh && pSh->GetItem(SID_GRADIENT_LIST))
                {
                    if(!mpLbFillAttr->GetEntryCount())
                    {
                        mpLbFillAttr->Enable();
                        mpLbFillAttr->Clear();
                        mpLbFillAttr->Fill(pSh->GetItem(SID_GRADIENT_LIST)->GetGradientList());
                    }

                    mpLbFillAttr->AdaptDropDownLineCountToMaximum();

                    if(LISTBOX_ENTRY_NOTFOUND != mnLastPosGradient)
                    {
                        const SvxGradientListItem * pItem = pSh->GetItem(SID_GRADIENT_LIST);

                        if(mnLastPosGradient < pItem->GetGradientList()->Count())
                        {
                            const XGradient aGradient = pItem->GetGradientList()->GetGradient(mnLastPosGradient)->GetGradient();
                            const XFillGradientItem aXFillGradientItem(mpLbFillAttr->GetEntry(mnLastPosGradient), aGradient);

                            // #i122676# change FillStyle and Gradient in one call
                            SfxViewFrame::Current()->GetDispatcher()->ExecuteList(
                                SID_ATTR_FILL_GRADIENT, SfxCallMode::RECORD,
                                { &aXFillGradientItem, &aXFillStyleItem });
                            mpLbFillAttr->SelectEntryPos(mnLastPosGradient);
                        }
                    }
                }
                else
                {
                    mpLbFillAttr->Disable();
                }
                break;
            }
            case drawing::FillStyle_HATCH:
            {
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();

                if(pSh && pSh->GetItem(SID_HATCH_LIST))
                {
                    if(!mpLbFillAttr->GetEntryCount())
                    {
                        mpLbFillAttr->Enable();
                        mpLbFillAttr->Clear();
                        mpLbFillAttr->Fill(pSh->GetItem(SID_HATCH_LIST)->GetHatchList());
                    }

                    mpLbFillAttr->AdaptDropDownLineCountToMaximum();

                    if(LISTBOX_ENTRY_NOTFOUND != mnLastPosHatch)
                    {
                        const SvxHatchListItem * pItem = pSh->GetItem(SID_HATCH_LIST);

                        if(mnLastPosHatch < pItem->GetHatchList()->Count())
                        {
                            const XHatch aHatch = pItem->GetHatchList()->GetHatch(mnLastPosHatch)->GetHatch();
                            const XFillHatchItem aXFillHatchItem(mpLbFillAttr->GetSelectedEntry(), aHatch);

                            // #i122676# change FillStyle and Hatch in one call
                            SfxViewFrame::Current()->GetDispatcher()->ExecuteList(
                                SID_ATTR_FILL_HATCH, SfxCallMode::RECORD,
                                { &aXFillHatchItem, &aXFillStyleItem });
                            mpLbFillAttr->SelectEntryPos(mnLastPosHatch);
                        }
                    }
                }
                else
                {
                    mpLbFillAttr->Disable();
                }
                break;
            }
            case drawing::FillStyle_BITMAP:
            {
                mpLbFillAttr->Show();
                mpToolBoxColor->Hide();

                if(pSh && pSh->GetItem(SID_BITMAP_LIST))
                {
                    if(!mpLbFillAttr->GetEntryCount())
                    {
                        mpLbFillAttr->Enable();
                        mpLbFillAttr->Clear();
                        mpLbFillAttr->Fill(pSh->GetItem(SID_BITMAP_LIST)->GetBitmapList());
                    }

                    mpLbFillAttr->AdaptDropDownLineCountToMaximum();

                    if(LISTBOX_ENTRY_NOTFOUND != mnLastPosBitmap)
                    {
                        const SvxBitmapListItem * pItem = pSh->GetItem(SID_BITMAP_LIST);

                        if(mnLastPosBitmap < pItem->GetBitmapList()->Count())
                        {
                            const XBitmapEntry* pXBitmapEntry = pItem->GetBitmapList()->GetBitmap(mnLastPosBitmap);
                            const XFillBitmapItem aXFillBitmapItem(mpLbFillAttr->GetSelectedEntry(), pXBitmapEntry->GetGraphicObject());

                            // #i122676# change FillStyle and Bitmap in one call
                            SfxViewFrame::Current()->GetDispatcher()->ExecuteList(
                                SID_ATTR_FILL_BITMAP, SfxCallMode::RECORD,
                                { &aXFillBitmapItem, &aXFillStyleItem });
                            mpLbFillAttr->SelectEntryPos(mnLastPosBitmap);
                        }
                    }
                }
                else
                {
                    mpLbFillAttr->Disable();
                }
                break;
            }
        }

        meLastXFS = eXFS;

        if(drawing::FillStyle_NONE != eXFS)
        {
            mpLbFillType->Selected();
        }
    }
}

IMPL_LINK_NOARG(SvxFillToolBoxControl, SelectFillAttrHdl, ListBox&, void)
{
    const drawing::FillStyle eXFS = static_cast<drawing::FillStyle>(mpLbFillType->GetSelectedEntryPos());
    const XFillStyleItem aXFillStyleItem(eXFS);
    SfxObjectShell* pSh = SfxObjectShell::Current();

    // #i122676# dependent from bFillStyleChange, do execute a single or two
    // changes in one Execute call
    const bool bFillStyleChange(meLastXFS != eXFS);

    switch(eXFS)
    {
        case drawing::FillStyle_SOLID:
        {
            if(bFillStyleChange)
            {
                // #i122676# Single FillStyle change call needed here
                SfxViewFrame::Current()->GetDispatcher()->ExecuteList(
                    SID_ATTR_FILL_STYLE, SfxCallMode::RECORD,
                    { &aXFillStyleItem });
            }
            break;
        }
        case drawing::FillStyle_GRADIENT:
        {
            sal_Int32 nPos = mpLbFillAttr->GetSelectedEntryPos();

            if(LISTBOX_ENTRY_NOTFOUND == nPos)
            {
                nPos = mnLastPosGradient;
            }

            if(LISTBOX_ENTRY_NOTFOUND != nPos && pSh && pSh->GetItem(SID_GRADIENT_LIST))
            {
                const SvxGradientListItem * pItem = pSh->GetItem(SID_GRADIENT_LIST);

                if(nPos < pItem->GetGradientList()->Count())
                {
                    const XGradient aGradient = pItem->GetGradientList()->GetGradient(nPos)->GetGradient();
                    const XFillGradientItem aXFillGradientItem(mpLbFillAttr->GetSelectedEntry(), aGradient);

                    // #i122676# Change FillStyle and Gradinet in one call
                    SfxViewFrame::Current()->GetDispatcher()->ExecuteList(
                        SID_ATTR_FILL_GRADIENT, SfxCallMode::RECORD,
                        bFillStyleChange
                            ? std::initializer_list<SfxPoolItem const*>{ &aXFillGradientItem, &aXFillStyleItem }
                            : std::initializer_list<SfxPoolItem const*>{ &aXFillGradientItem });
                }
            }

            if(LISTBOX_ENTRY_NOTFOUND != nPos)
            {
                mnLastPosGradient = nPos;
            }
            break;
        }
        case drawing::FillStyle_HATCH:
        {
            sal_Int32 nPos = mpLbFillAttr->GetSelectedEntryPos();

            if(LISTBOX_ENTRY_NOTFOUND == nPos)
            {
                nPos = mnLastPosHatch;
            }

            if(LISTBOX_ENTRY_NOTFOUND != nPos && pSh && pSh->GetItem(SID_HATCH_LIST))
            {
                const SvxHatchListItem * pItem = pSh->GetItem(SID_HATCH_LIST);

                if(nPos < pItem->GetHatchList()->Count())
                {
                    const XHatch aHatch = pItem->GetHatchList()->GetHatch(nPos)->GetHatch();
                    const XFillHatchItem aXFillHatchItem( mpLbFillAttr->GetSelectedEntry(), aHatch);

                    // #i122676# Change FillStyle and Hatch in one call
                    SfxViewFrame::Current()->GetDispatcher()->ExecuteList(
                        SID_ATTR_FILL_HATCH, SfxCallMode::RECORD,
                        bFillStyleChange
                            ? std::initializer_list<SfxPoolItem const*>{ &aXFillHatchItem, &aXFillStyleItem }
                            : std::initializer_list<SfxPoolItem const*>{ &aXFillHatchItem });
                }
            }

            if(LISTBOX_ENTRY_NOTFOUND != nPos)
            {
                mnLastPosHatch = nPos;
            }
            break;
        }
        case drawing::FillStyle_BITMAP:
        {
            sal_Int32 nPos = mpLbFillAttr->GetSelectedEntryPos();

            if(LISTBOX_ENTRY_NOTFOUND == nPos)
            {
                nPos = mnLastPosBitmap;
            }

            if(LISTBOX_ENTRY_NOTFOUND != nPos && pSh && pSh->GetItem(SID_BITMAP_LIST))
            {
                const SvxBitmapListItem * pItem = pSh->GetItem(SID_BITMAP_LIST);

                if(nPos < pItem->GetBitmapList()->Count())
                {
                    const XBitmapEntry* pXBitmapEntry = pItem->GetBitmapList()->GetBitmap(nPos);
                    const XFillBitmapItem aXFillBitmapItem(mpLbFillAttr->GetSelectedEntry(), pXBitmapEntry->GetGraphicObject());

                    // #i122676# Change FillStyle and Bitmap in one call
                    SfxViewFrame::Current()->GetDispatcher()->ExecuteList(
                        SID_ATTR_FILL_BITMAP, SfxCallMode::RECORD,
                        bFillStyleChange
                            ? std::initializer_list<SfxPoolItem const*>{ &aXFillBitmapItem, &aXFillStyleItem }
                            : std::initializer_list<SfxPoolItem const*>{ &aXFillBitmapItem });
                }
            }

            if(LISTBOX_ENTRY_NOTFOUND != nPos)
            {
                mnLastPosBitmap = nPos;
            }
            break;
        }
        default: break;
    }
}

void FillControl::Resize()
{
    Size aSize(GetOutputSizePixel());
    long nH = aSize.Height();

    Size aTypeSize(mpLbFillType->get_preferred_size());
    long nPrefHeight = aTypeSize.Height();
    long nOffset = (nH - nPrefHeight)/2;
    mpLbFillType->SetPosSizePixel(Point(0, nOffset), Size(aTypeSize.Width(), nPrefHeight));
    nPrefHeight = mpToolBoxColor->get_preferred_size().Height();
    nOffset = (nH - nPrefHeight)/2;
    mpToolBoxColor->SetPosSizePixel(Point(aTypeSize.Width(), nOffset),Size(aSize.Width() - aTypeSize.Width(), nPrefHeight));
    nPrefHeight = mpLbFillType->get_preferred_size().Height();
    nOffset = (nH - nPrefHeight)/2;
    mpLbFillAttr->SetPosSizePixel(Point(aTypeSize.Width(), nOffset),Size(aSize.Width() - aTypeSize.Width(), nPrefHeight));
}

void FillControl::SetOptimalSize()
{
    Size aSize(mpLbFillType->get_preferred_size());
    Size aFirstSize(mpToolBoxColor->get_preferred_size());
    Size aSecondSize(mpLbFillAttr->get_preferred_size());
    aSize.setHeight(std::max({aSize.Height(), aFirstSize.Height(), aSecondSize.Height()}));
    aSize.setWidth(aSize.Width() + LogicToPixel(Size(55, 0), MapMode(MapUnit::MapAppFont)).Width());
    SetSizePixel(aSize);
}

void FillControl::DataChanged(const DataChangedEvent& rDCEvt)
{
    if((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
        (rDCEvt.GetFlags() & AllSettingsFlags::STYLE))
    {
        SetOptimalSize();
    }
    Window::DataChanged(rDCEvt);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
