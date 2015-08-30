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
#include <svx/dialogs.hrc>

#define TMP_STR_BEGIN   "["
#define TMP_STR_END     "]"

#include "svx/drawitem.hxx"
#include "svx/xattr.hxx"
#include <svx/xtable.hxx>
#include <svx/fillctrl.hxx>
#include <svx/itemwin.hxx>
#include <svx/dialmgr.hxx>
#include "helpid.hrc"
#include <boost/scoped_ptr.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
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
    , mpFillControl(0)
    , mpLbFillType(0)
    , mpLbFillAttr(0)
    , meLastXFS(static_cast<sal_uInt16>(-1))
    , mnLastPosGradient(0)
    , mnLastPosHatch(0)
    , mnLastPosBitmap(0)
{
    addStatusListener( OUString( ".uno:FillColor" ));
    addStatusListener( OUString( ".uno:FillGradient" ));
    addStatusListener( OUString( ".uno:FillHatch" ));
    addStatusListener( OUString( ".uno:FillBitmap" ));
    addStatusListener( OUString( ".uno:ColorTableState" ));
    addStatusListener( OUString( ".uno:GradientListState" ));
    addStatusListener( OUString( ".uno:HatchListState" ));
    addStatusListener( OUString( ".uno:BitmapListState" ));
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
                meLastXFS = static_cast<sal_uInt16>(-1);
                mpStyleItem.reset();
            }

            if(eState >= SfxItemState::DEFAULT)
            {
                const XFillStyleItem* pItem = dynamic_cast< const XFillStyleItem* >(pState);

                if(pItem)
                {
                    mpStyleItem.reset(dynamic_cast< XFillStyleItem* >(pItem->Clone()));
                    mpLbFillType->Enable();
                    drawing::FillStyle eXFS = (drawing::FillStyle)mpStyleItem->GetValue();
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
            meLastXFS = static_cast<sal_uInt16>(-1);
            mpStyleItem.reset();
            break;
        }
        case SID_ATTR_FILL_COLOR:
        {
            if(SfxItemState::DEFAULT == eState)
            {
                mpColorItem.reset(pState ? static_cast<XFillColorItem*>(pState->Clone()) : 0);
            }

            if(mpStyleItem && drawing::FillStyle_SOLID == (drawing::FillStyle)mpStyleItem->GetValue())
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
                mpFillGradientItem.reset(pState ? static_cast<XFillGradientItem*>(pState->Clone()) : 0);
            }

            if(mpStyleItem && drawing::FillStyle_GRADIENT == (drawing::FillStyle)mpStyleItem->GetValue())
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
                mpHatchItem.reset(pState ? static_cast<XFillHatchItem*>(pState->Clone()) : 0);
            }

            if(mpStyleItem && drawing::FillStyle_HATCH == (drawing::FillStyle)mpStyleItem->GetValue())
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
                mpBitmapItem.reset(pState ? static_cast<XFillBitmapItem*>(pState->Clone()) : 0);
            }

            if(mpStyleItem && drawing::FillStyle_BITMAP == (drawing::FillStyle)mpStyleItem->GetValue())
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
                if(mpStyleItem && drawing::FillStyle_GRADIENT == (drawing::FillStyle)mpStyleItem->GetValue())
                {
                    if(mpFillGradientItem)
                    {
                        const OUString aString( mpFillGradientItem->GetName() );
                        const SfxObjectShell* pSh = SfxObjectShell::Current();
                        const SvxGradientListItem aItem( *static_cast<const SvxGradientListItem*>(pSh->GetItem(SID_GRADIENT_LIST)));

                        mpLbFillAttr->Clear();
                        mpLbFillAttr->Enable();
                        mpLbFillAttr->Fill(aItem.GetGradientList());
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
                if(mpStyleItem && drawing::FillStyle_HATCH == (drawing::FillStyle)mpStyleItem->GetValue())
                {
                    if(mpHatchItem)
                    {
                        const OUString aString( mpHatchItem->GetName() );
                        const SfxObjectShell* pSh = SfxObjectShell::Current();
                        const SvxHatchListItem aItem(*static_cast<const SvxHatchListItem*>(pSh->GetItem(SID_HATCH_LIST)));

                        mpLbFillAttr->Clear();
                        mpLbFillAttr->Enable();
                        mpLbFillAttr->Fill(aItem.GetHatchList());
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
                if(mpStyleItem && drawing::FillStyle_BITMAP == (drawing::FillStyle)mpStyleItem->GetValue())
                {
                    if(mpBitmapItem)
                    {
                        const OUString aString( mpBitmapItem->GetName() );
                        const SfxObjectShell* pSh = SfxObjectShell::Current();
                        const SvxBitmapListItem aItem(*static_cast<const SvxBitmapListItem*>(pSh->GetItem(SID_BITMAP_LIST)));

                        mpLbFillAttr->Clear();
                        mpLbFillAttr->Enable();
                        mpLbFillAttr->Fill(aItem.GetBitmapList());
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
        const drawing::FillStyle eXFS = (drawing::FillStyle)mpStyleItem->GetValue();
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
                    const SvxGradientListItem aItem(*static_cast<const SvxGradientListItem*>(pSh->GetItem(SID_GRADIENT_LIST)));
                    mpLbFillAttr->Enable();
                    mpLbFillAttr->Clear();
                    mpLbFillAttr->Fill(aItem.GetGradientList());

                    if(mpFillGradientItem)
                    {
                        const OUString aString(mpFillGradientItem->GetName());

                        mpLbFillAttr->SelectEntry(aString);

                        // Check if the entry is not in the list
                        if (mpLbFillAttr->GetSelectEntry() != aString)
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

                            boost::scoped_ptr<XGradientEntry> pEntry(new XGradientEntry(mpFillGradientItem->GetGradientValue(), aTmpStr));
                            XGradientList aGradientList( "", ""/*TODO?*/ );
                            aGradientList.Insert( pEntry.get() );
                            aGradientList.SetDirty( false );
                            const Bitmap aBmp = aGradientList.GetUiBitmap( 0 );

                            if(!aBmp.IsEmpty())
                            {
                                mpLbFillAttr->InsertEntry(pEntry->GetName(), Image(aBmp));
                                mpLbFillAttr->SelectEntryPos(mpLbFillAttr->GetEntryCount() - 1);
                            }

                            aGradientList.Remove( 0 );
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
                    const SvxHatchListItem aItem(*static_cast<const SvxHatchListItem*>(pSh->GetItem(SID_HATCH_LIST)));
                    mpLbFillAttr->Enable();
                    mpLbFillAttr->Clear();
                    mpLbFillAttr->Fill(aItem.GetHatchList());

                    if(mpHatchItem)
                    {
                        const OUString aString(mpHatchItem->GetName());

                        mpLbFillAttr->SelectEntry( aString );

                        // Check if the entry is not in the list
                        if( mpLbFillAttr->GetSelectEntry() != aString )
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

                            XHatchEntry* pEntry = new XHatchEntry(mpHatchItem->GetHatchValue(), aTmpStr);
                            XHatchList aHatchList( "", ""/*TODO?*/ );
                            aHatchList.Insert( pEntry );
                            aHatchList.SetDirty( false );
                            const Bitmap aBmp = aHatchList.GetUiBitmap( 0 );

                            if( !aBmp.IsEmpty() )
                            {
                                mpLbFillAttr->InsertEntry(pEntry->GetName(), Image(aBmp));
                                mpLbFillAttr->SelectEntryPos( mpLbFillAttr->GetEntryCount() - 1 );
                                //delete pBmp;
                            }

                            aHatchList.Remove( 0 );
                            delete pEntry;
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
                    const SvxBitmapListItem aItem(*static_cast<const SvxBitmapListItem*>(pSh->GetItem(SID_BITMAP_LIST)));
                    mpLbFillAttr->Enable();
                    mpLbFillAttr->Clear();
                    mpLbFillAttr->Fill(aItem.GetBitmapList());

                    if(mpBitmapItem)
                    {
                        const OUString aString(mpBitmapItem->GetName());

                        mpLbFillAttr->SelectEntry(aString);

                        // Check if the entry is not in the list
                        if (mpLbFillAttr->GetSelectEntry() != aString)
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

                            boost::scoped_ptr<XBitmapEntry> pEntry(new XBitmapEntry(mpBitmapItem->GetGraphicObject(), aTmpStr));
                            XBitmapListRef xBitmapList =
                                XPropertyList::AsBitmapList(
                                    XPropertyList::CreatePropertyList(
                                        XBITMAP_LIST, "TmpList", ""/*TODO?*/));
                            xBitmapList->Insert( pEntry.get() );
                            xBitmapList->SetDirty( false );
                            mpLbFillAttr->Fill( xBitmapList );
                            mpLbFillAttr->SelectEntryPos(mpLbFillAttr->GetEntryCount() - 1);
                            xBitmapList->Remove( 0 );
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
        // Thus the FillControl is known by SvxFillToolBoxControl
        // (and in order to remain compatible)
        mpFillControl->SetData(this);

        mpLbFillType = mpFillControl->mpLbFillType;
        mpLbFillAttr = mpFillControl->mpLbFillAttr;
        mpToolBoxColor = mpFillControl->mpToolBoxColor;
        mpFillControl->Resize();
        mpToolBoxColor->InsertItem(".uno:FillColor", m_xFrame, ToolBoxItemBits::DROPDOWNONLY, Size(mpToolBoxColor->GetSizePixel().Width(), 0));

        mpLbFillAttr->SetUniqueId(HID_FILL_ATTR_LISTBOX);
        mpToolBoxColor->SetUniqueId(HID_FILL_ATTR_LISTBOX);
        mpLbFillType->SetUniqueId(HID_FILL_TYPE_LISTBOX);

        mpLbFillType->SetSelectHdl(LINK(this,SvxFillToolBoxControl,SelectFillTypeHdl));
        mpLbFillAttr->SetSelectHdl(LINK(this,SvxFillToolBoxControl,SelectFillAttrHdl));

        return mpFillControl.get();
    }
    return VclPtr<vcl::Window>();
}

FillControl::FillControl(vcl::Window* pParent,WinBits nStyle)
    : Window(pParent,nStyle | WB_DIALOGCONTROL)
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

IMPL_LINK(SvxFillToolBoxControl, SelectFillTypeHdl, ListBox *, pToolBox)
{
    const drawing::FillStyle eXFS = (drawing::FillStyle)mpLbFillType->GetSelectEntryPos();

    if((drawing::FillStyle)meLastXFS != eXFS)
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
                SfxViewFrame::Current()->GetDispatcher()->Execute(
                    SID_ATTR_FILL_STYLE, SfxCallMode::RECORD, &aXFillStyleItem, 0L);
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
                SfxViewFrame::Current()->GetDispatcher()->Execute(
                    SID_ATTR_FILL_COLOR, SfxCallMode::RECORD, &aXFillColorItem, &aXFillStyleItem, 0L);
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
                        const SvxGradientListItem aItem(*static_cast<const SvxGradientListItem*>(pSh->GetItem(SID_GRADIENT_LIST)));
                        mpLbFillAttr->Enable();
                        mpLbFillAttr->Clear();
                        mpLbFillAttr->Fill(aItem.GetGradientList());
                    }

                    mpLbFillAttr->AdaptDropDownLineCountToMaximum();

                    if(LISTBOX_ENTRY_NOTFOUND != mnLastPosGradient)
                    {
                        const SvxGradientListItem aItem(*static_cast<const SvxGradientListItem*>(pSh->GetItem(SID_GRADIENT_LIST)));

                        if(mnLastPosGradient < aItem.GetGradientList()->Count())
                        {
                            const XGradient aGradient = aItem.GetGradientList()->GetGradient(mnLastPosGradient)->GetGradient();
                            const XFillGradientItem aXFillGradientItem(mpLbFillAttr->GetEntry(mnLastPosGradient), aGradient);

                            // #i122676# change FillStyle and Gradient in one call
                            SfxViewFrame::Current()->GetDispatcher()->Execute(
                                SID_ATTR_FILL_GRADIENT, SfxCallMode::RECORD, &aXFillGradientItem, &aXFillStyleItem, 0L);
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
                        const SvxHatchListItem aItem( *static_cast<const SvxHatchListItem*>(pSh->GetItem(SID_HATCH_LIST)));
                        mpLbFillAttr->Enable();
                        mpLbFillAttr->Clear();
                        mpLbFillAttr->Fill(aItem.GetHatchList());
                    }

                    mpLbFillAttr->AdaptDropDownLineCountToMaximum();

                    if(LISTBOX_ENTRY_NOTFOUND != mnLastPosHatch)
                    {
                        const SvxHatchListItem aItem(*static_cast<const SvxHatchListItem*>(pSh->GetItem(SID_HATCH_LIST)));

                        if(mnLastPosHatch < aItem.GetHatchList()->Count())
                        {
                            const XHatch aHatch = aItem.GetHatchList()->GetHatch(mnLastPosHatch)->GetHatch();
                            const XFillHatchItem aXFillHatchItem(mpLbFillAttr->GetSelectEntry(), aHatch);

                            // #i122676# change FillStyle and Hatch in one call
                            SfxViewFrame::Current()->GetDispatcher()->Execute(
                                SID_ATTR_FILL_HATCH, SfxCallMode::RECORD, &aXFillHatchItem, &aXFillStyleItem, 0L);
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
                        const SvxBitmapListItem aItem( *static_cast<const SvxBitmapListItem*>(pSh->GetItem(SID_BITMAP_LIST)));
                        mpLbFillAttr->Enable();
                        mpLbFillAttr->Clear();
                        mpLbFillAttr->Fill(aItem.GetBitmapList());
                    }

                    mpLbFillAttr->AdaptDropDownLineCountToMaximum();

                    if(LISTBOX_ENTRY_NOTFOUND != mnLastPosBitmap)
                    {
                        const SvxBitmapListItem aItem(*static_cast<const SvxBitmapListItem*>(pSh->GetItem(SID_BITMAP_LIST)));

                        if(mnLastPosBitmap < aItem.GetBitmapList()->Count())
                        {
                            const XBitmapEntry* pXBitmapEntry = aItem.GetBitmapList()->GetBitmap(mnLastPosBitmap);
                            const XFillBitmapItem aXFillBitmapItem(mpLbFillAttr->GetSelectEntry(), pXBitmapEntry->GetGraphicObject());

                            // #i122676# change FillStyle and Bitmap in one call
                            SfxViewFrame::Current()->GetDispatcher()->Execute(
                                SID_ATTR_FILL_BITMAP, SfxCallMode::RECORD, &aXFillBitmapItem, &aXFillStyleItem, 0L);
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

        meLastXFS = (sal_uInt16)eXFS;

        if(drawing::FillStyle_NONE != eXFS)
        {
            if(pToolBox)
            {
                mpLbFillType->Selected();
            }
        }
    }

    return 0;
}

IMPL_LINK(SvxFillToolBoxControl, SelectFillAttrHdl, ListBox *, pToolBox)
{
    const drawing::FillStyle eXFS = (drawing::FillStyle)mpLbFillType->GetSelectEntryPos();
    const XFillStyleItem aXFillStyleItem(eXFS);
    SfxObjectShell* pSh = SfxObjectShell::Current();

    if(pToolBox)
    {
        // #i122676# dependent from bFillStyleChange, do execute a single or two
        // changes in one Execute call
        const bool bFillStyleChange((drawing::FillStyle) meLastXFS != eXFS);

        switch(eXFS)
        {
            case drawing::FillStyle_SOLID:
            {
                if(bFillStyleChange)
                {
                    // #i122676# Single FillStyle change call needed here
                    SfxViewFrame::Current()->GetDispatcher()->Execute(SID_ATTR_FILL_STYLE, SfxCallMode::RECORD, &aXFillStyleItem, 0L);
                }
                break;
            }
            case drawing::FillStyle_GRADIENT:
            {
                sal_Int32 nPos = mpLbFillAttr->GetSelectEntryPos();

                if(LISTBOX_ENTRY_NOTFOUND == nPos)
                {
                    nPos = mnLastPosGradient;
                }

                if(LISTBOX_ENTRY_NOTFOUND != nPos && pSh && pSh->GetItem(SID_GRADIENT_LIST))
                {
                    const SvxGradientListItem aItem(*static_cast<const SvxGradientListItem*>(pSh->GetItem(SID_GRADIENT_LIST)));

                    if(nPos < aItem.GetGradientList()->Count())
                    {
                        const XGradient aGradient = aItem.GetGradientList()->GetGradient(nPos)->GetGradient();
                        const XFillGradientItem aXFillGradientItem(mpLbFillAttr->GetSelectEntry(), aGradient);

                        // #i122676# Change FillStale and Gradinet in one call
                        SfxViewFrame::Current()->GetDispatcher()->Execute(
                            SID_ATTR_FILL_GRADIENT, SfxCallMode::RECORD, &aXFillGradientItem,
                            bFillStyleChange ? &aXFillStyleItem : 0L, 0L);
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
                sal_Int32 nPos = mpLbFillAttr->GetSelectEntryPos();

                if(LISTBOX_ENTRY_NOTFOUND == nPos)
                {
                    nPos = mnLastPosHatch;
                }

                if(LISTBOX_ENTRY_NOTFOUND != nPos && pSh && pSh->GetItem(SID_HATCH_LIST))
                {
                    const SvxHatchListItem aItem(*static_cast<const SvxHatchListItem*>(pSh->GetItem(SID_HATCH_LIST)));

                    if(nPos < aItem.GetHatchList()->Count())
                    {
                        const XHatch aHatch = aItem.GetHatchList()->GetHatch(nPos)->GetHatch();
                        const XFillHatchItem aXFillHatchItem( mpLbFillAttr->GetSelectEntry(), aHatch);

                        // #i122676# Change FillStale and Hatch in one call
                        SfxViewFrame::Current()->GetDispatcher()->Execute(
                            SID_ATTR_FILL_HATCH, SfxCallMode::RECORD, &aXFillHatchItem,
                            bFillStyleChange ? &aXFillStyleItem : 0L, 0L);
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
                sal_Int32 nPos = mpLbFillAttr->GetSelectEntryPos();

                if(LISTBOX_ENTRY_NOTFOUND == nPos)
                {
                    nPos = mnLastPosBitmap;
                }

                if(LISTBOX_ENTRY_NOTFOUND != nPos && pSh && pSh->GetItem(SID_BITMAP_LIST))
                {
                    const SvxBitmapListItem aItem(*static_cast<const SvxBitmapListItem*>(pSh->GetItem(SID_BITMAP_LIST)));

                    if(nPos < aItem.GetBitmapList()->Count())
                    {
                        const XBitmapEntry* pXBitmapEntry = aItem.GetBitmapList()->GetBitmap(nPos);
                        const XFillBitmapItem aXFillBitmapItem(mpLbFillAttr->GetSelectEntry(), pXBitmapEntry->GetGraphicObject());

                        // #i122676# Change FillStale and Bitmap in one call
                        SfxViewFrame::Current()->GetDispatcher()->Execute(
                            SID_ATTR_FILL_BITMAP, SfxCallMode::RECORD, &aXFillBitmapItem,
                            bFillStyleChange ? &aXFillStyleItem : 0L, 0L);
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

    return 0;
}

void FillControl::Resize()
{
    // Relative width of the two list boxes is 2/5 : 3/5
    Size aSize(GetOutputSizePixel());
    long nW = aSize.Width() / 5;
    long nH = aSize.Height();

    long nPrefHeight = mpLbFillType->get_preferred_size().Height();
    long nOffset = (nH - nPrefHeight)/2;
    mpLbFillType->SetPosSizePixel(Point(0, nOffset), Size(nW * 2, nPrefHeight));
    nPrefHeight = mpToolBoxColor->get_preferred_size().Height();
    nOffset = (nH - nPrefHeight)/2;
    mpToolBoxColor->SetPosSizePixel(Point(nW * 2, nOffset),Size(nW * 3, nPrefHeight));
    nPrefHeight = mpLbFillType->get_preferred_size().Height();
    nOffset = (nH - nPrefHeight)/2;
    mpLbFillAttr->SetPosSizePixel(Point(nW * 2, nOffset),Size(nW * 3, nPrefHeight));
}

void FillControl::SetOptimalSize()
{
    const Size aLogicalAttrSize(50,0);
    Size aSize(LogicToPixel(aLogicalAttrSize,MAP_APPFONT));

    Point aAttrPnt = mpLbFillAttr->GetPosPixel();

    aSize.Height() = std::max(aSize.Height(), mpLbFillType->get_preferred_size().Height());
    aSize.Height() = std::max(aSize.Height(), mpToolBoxColor->get_preferred_size().Height());
    aSize.Height() = std::max(aSize.Height(), mpLbFillAttr->get_preferred_size().Height());

    aSize.Width() = aAttrPnt.X() + aSize.Width();

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
