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
    , mpStyleItem(0)
    , mpColorItem(0)
    , mpGradientItem(0)
    , mpHatchItem(0)
    , mpBitmapItem(0)
    , mpFillControl(0)
    , mpFillTypeLB(0)
    , mpFillAttrLB(0)
    , meLastXFS(drawing::FillStyle_NONE)
    , mbUpdate(false)
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
    delete mpStyleItem;
    delete mpColorItem;
    delete mpGradientItem;
    delete mpHatchItem;
    delete mpBitmapItem;
}



void SvxFillToolBoxControl::StateChanged(
    sal_uInt16 nSID,
    SfxItemState eState,
    const SfxPoolItem* pState)
{
    if(eState == SfxItemState::DISABLED)
    {
        // slot disable state
        if(nSID == SID_ATTR_FILL_STYLE)
        {
            mpFillTypeLB->Disable();
            mpFillTypeLB->SetNoSelection();
        }

        mpFillAttrLB->Disable();
        mpFillAttrLB->SetNoSelection();
    }
    else if(SfxItemState::DEFAULT == eState)
    {
        bool bEnableControls(false);

        // slot available state
        if(nSID == SID_ATTR_FILL_STYLE)
        {
            delete mpStyleItem;
            mpStyleItem = static_cast< XFillStyleItem* >(pState->Clone());
            mpFillTypeLB->Enable();
        }
        else if(mpStyleItem)
        {
            const drawing::FillStyle eXFS(static_cast< drawing::FillStyle >(mpStyleItem->GetValue()));

            if(nSID == SID_ATTR_FILL_COLOR)
            {
                delete mpColorItem;
                mpColorItem = static_cast< XFillColorItem* >(pState->Clone());

                if(eXFS == drawing::FillStyle_SOLID)
                {
                    bEnableControls = true;
                }
            }
            else if(nSID == SID_ATTR_FILL_GRADIENT)
            {
                delete mpGradientItem;
                mpGradientItem = static_cast< XFillGradientItem* >(pState->Clone());

                if(eXFS == drawing::FillStyle_GRADIENT)
                {
                    bEnableControls = true;
                }
            }
            else if(nSID == SID_ATTR_FILL_HATCH)
            {
                delete mpHatchItem;
                mpHatchItem = static_cast< XFillHatchItem* >(pState->Clone());

                if(eXFS == drawing::FillStyle_HATCH)
                {
                    bEnableControls = true;
                }
            }
            else if(nSID == SID_ATTR_FILL_BITMAP)
            {
                delete mpBitmapItem;
                mpBitmapItem = static_cast< XFillBitmapItem* >(pState->Clone());

                if(eXFS == drawing::FillStyle_BITMAP)
                {
                    bEnableControls = true;
                }
            }
        }

        if(mpStyleItem)
        {
            // ensure that the correct entry is selected in mpFillTypeLB
            drawing::FillStyle eXFS(static_cast< drawing::FillStyle >(mpStyleItem->GetValue()));
            const bool bFillTypeChangedByUser(mpFillControl->mbFillTypeChanged);

            if(bFillTypeChangedByUser)
            {
                meLastXFS = static_cast< drawing::FillStyle >(mpFillControl->mnLastFillTypeControlSelectEntryPos);
                mpFillControl->mbFillTypeChanged = false;
            }

            if(meLastXFS != eXFS)
            {
                mbUpdate = true;
                mpFillTypeLB->SelectEntryPos(sal::static_int_cast<sal_uInt16>(eXFS));
            }

            mpFillAttrLB->Enable();
        }

        if(bEnableControls)
        {
            mpFillAttrLB->Enable();
            mbUpdate = true;
        }

        Update(pState);
    }
    else
    {
        // slot empty or ambigous
        if(nSID == SID_ATTR_FILL_STYLE)
        {
            mpFillTypeLB->SetNoSelection();
            mpFillAttrLB->Disable();
            mpFillAttrLB->SetNoSelection();
            delete mpStyleItem;
            mpStyleItem = 0;
            mbUpdate = false;
        }
        else
        {
            drawing::FillStyle eXFS(drawing::FillStyle_NONE);

            if(mpStyleItem)
            {
                eXFS = static_cast< drawing::FillStyle >(mpStyleItem->GetValue());
            }

            if(!mpStyleItem ||
                (nSID == SID_ATTR_FILL_COLOR && eXFS == drawing::FillStyle_SOLID) ||
                (nSID == SID_ATTR_FILL_GRADIENT && eXFS == drawing::FillStyle_GRADIENT) ||
                (nSID == SID_ATTR_FILL_HATCH && eXFS == drawing::FillStyle_HATCH) ||
                (nSID == SID_ATTR_FILL_BITMAP && eXFS == drawing::FillStyle_BITMAP))
            {
                mpFillAttrLB->SetNoSelection();
            }
        }
    }
}



void SvxFillToolBoxControl::Update(const SfxPoolItem* pState)
{
    if(mpStyleItem && pState && mbUpdate)
    {
        mbUpdate = false;
        const drawing::FillStyle eXFS(static_cast< drawing::FillStyle >(mpStyleItem->GetValue()));

        // Check if the fill style was already active
        if(meLastXFS != eXFS)
        {
            // update mnLastFillTypeControlSelectEntryPos and fill style list
            mpFillControl->updateLastFillTypeControlSelectEntryPos();
            mpFillControl->InitializeFillStyleAccordingToGivenFillType(eXFS);
            meLastXFS = eXFS;
        }

        switch(eXFS)
        {
            case drawing::FillStyle_NONE:
            {
                break;
            }

            case drawing::FillStyle_SOLID:
            {
                if(mpColorItem)
                {
                    OUString aString(mpColorItem->GetName());
                    ::Color aColor = mpColorItem->GetColorValue();

                    mpFillAttrLB->SelectEntry(aString);

                    if(mpFillAttrLB->GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND || mpFillAttrLB->GetSelectEntryColor() != aColor)
                    {
                        mpFillAttrLB->SelectEntry(aColor);
                    }

                    // Check if the entry is not in the list
                    if( mpFillAttrLB->GetSelectEntryPos() ==
                        LISTBOX_ENTRY_NOTFOUND ||
                        mpFillAttrLB->GetSelectEntryColor() != aColor )
                    {
                        sal_Int32 nCount = mpFillAttrLB->GetEntryCount();
                        OUString aTmpStr;
                        if( nCount > 0 )
                        {
                            // Last entry gets tested against temporary color
                            aTmpStr = mpFillAttrLB->GetEntry( nCount - 1 );
                            if( aTmpStr.startsWith(TMP_STR_BEGIN) &&
                                aTmpStr.endsWith(TMP_STR_END) )
                            {
                                mpFillAttrLB->RemoveEntry(nCount - 1);
                            }
                        }
                        aTmpStr = TMP_STR_BEGIN + aString + TMP_STR_END;

                        sal_Int32 nPos = mpFillAttrLB->InsertEntry(aColor, aTmpStr);
                        mpFillAttrLB->SelectEntryPos(nPos);
                    }
                }
                else
                {
                    mpFillAttrLB->SetNoSelection();
                }
                break;
            }

            case drawing::FillStyle_GRADIENT:
            {
                if(mpGradientItem)
                {
                    OUString aString(mpGradientItem->GetName());
                    mpFillAttrLB->SelectEntry( aString );
                    // Check if the entry is not in the list
                    if (mpFillAttrLB->GetSelectEntry() != aString)
                    {
                        sal_Int32 nCount = mpFillAttrLB->GetEntryCount();
                        OUString aTmpStr;
                        if( nCount > 0 )
                        {
                            // Last entry gets tested against temporary entry
                            aTmpStr = mpFillAttrLB->GetEntry( nCount - 1 );
                            if( aTmpStr.startsWith(TMP_STR_BEGIN) &&
                                aTmpStr.endsWith(TMP_STR_END) )
                            {
                                mpFillAttrLB->RemoveEntry(nCount - 1);
                            }
                        }
                        aTmpStr = TMP_STR_BEGIN + aString + TMP_STR_END;

                        boost::scoped_ptr<XGradientEntry> pEntry(new XGradientEntry(mpGradientItem->GetGradientValue(), aTmpStr));
                        XGradientList aGradientList( "", ""/*TODO?*/ );
                        aGradientList.Insert( pEntry.get() );
                        aGradientList.SetDirty( false );
                        const Bitmap aBmp = aGradientList.GetUiBitmap( 0 );

                        if(!aBmp.IsEmpty())
                        {
                            mpFillAttrLB->InsertEntry(pEntry->GetName(), Image(aBmp));
                            mpFillAttrLB->SelectEntryPos(mpFillAttrLB->GetEntryCount() - 1);
                        }

                        aGradientList.Remove( 0 );
                    }
                }
                else
                {
                    mpFillAttrLB->SetNoSelection();
                }
                break;
            }

            case drawing::FillStyle_HATCH:
            {
                if(mpHatchItem)
                {
                    OUString aString(mpHatchItem->GetName());
                    mpFillAttrLB->SelectEntry( aString );
                    // Check if the entry is not in the list
                    if (mpFillAttrLB->GetSelectEntry() != aString)
                    {
                        sal_Int32 nCount = mpFillAttrLB->GetEntryCount();
                        OUString aTmpStr;
                        if( nCount > 0 )
                        {
                            // Last entry gets tested against temporary entry
                            aTmpStr = mpFillAttrLB->GetEntry( nCount - 1 );
                            if( aTmpStr.startsWith(TMP_STR_BEGIN) &&
                                aTmpStr.endsWith(TMP_STR_END) )
                            {
                                mpFillAttrLB->RemoveEntry(nCount - 1);
                            }
                        }
                        aTmpStr = TMP_STR_BEGIN + aString + TMP_STR_END;

                        boost::scoped_ptr<XHatchEntry> pEntry(new XHatchEntry(mpHatchItem->GetHatchValue(), aTmpStr));
                        XHatchList aHatchList( "", ""/*TODO?*/ );
                        aHatchList.Insert( pEntry.get() );
                        aHatchList.SetDirty( false );
                        const Bitmap aBmp = aHatchList.GetUiBitmap( 0 );

                        if(!aBmp.IsEmpty())
                        {
                            mpFillAttrLB->InsertEntry(pEntry->GetName(), Image(aBmp));
                            mpFillAttrLB->SelectEntryPos(mpFillAttrLB->GetEntryCount() - 1);
                        }

                        aHatchList.Remove( 0 );
                    }
                }
                else
                {
                    mpFillAttrLB->SetNoSelection();
                }
                break;
            }

            case drawing::FillStyle_BITMAP:
            {
                if(mpBitmapItem)
                {
                    OUString aString(mpBitmapItem->GetName());
                    mpFillAttrLB->SelectEntry( aString );
                    // Check if the entry is not in the list
                    if (mpFillAttrLB->GetSelectEntry() != aString)
                    {
                        sal_Int32 nCount = mpFillAttrLB->GetEntryCount();
                        OUString aTmpStr;
                        if( nCount > 0 )
                        {
                            // Last entry gets tested against temporary entry
                            aTmpStr = mpFillAttrLB->GetEntry(nCount - 1);
                            if( aTmpStr.startsWith(TMP_STR_BEGIN) &&
                                aTmpStr.endsWith(TMP_STR_END) )
                            {
                                mpFillAttrLB->RemoveEntry(nCount - 1);
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
                        mpFillAttrLB->Fill( xBitmapList );
                        mpFillAttrLB->SelectEntryPos(mpFillAttrLB->GetEntryCount() - 1);
                        xBitmapList->Remove( 0 );
                    }
                }
                else
                {
                    mpFillAttrLB->SetNoSelection();
                }
                break;
            }

            default:
            {
                OSL_FAIL( "Unsupported fill type" );
                break;
            }
        }

        // update mnLastFillAttrControlSelectEntryPos
        mpFillControl->updateLastFillAttrControlSelectEntryPos();
    }

    if(pState && mpStyleItem)
    {
        drawing::FillStyle eXFS = static_cast< drawing::FillStyle >(mpStyleItem->GetValue());

        // Does the lists have changed?
        switch(eXFS)
        {
            case drawing::FillStyle_SOLID:
            {
                const SvxColorListItem* pItem = dynamic_cast< const SvxColorListItem* >(pState);

                if(pItem)
                {
                    ::Color aTmpColor(mpFillAttrLB->GetSelectEntryColor());
                    mpFillAttrLB->Clear();
                    mpFillAttrLB->Fill(pItem->GetColorList());
                    mpFillAttrLB->SelectEntry(aTmpColor);
                }
                break;
            }
            case drawing::FillStyle_GRADIENT:
            {
                const SvxGradientListItem* pItem = dynamic_cast< const SvxGradientListItem* >(pState);

                if(pItem)
                {
                    OUString aString(mpFillAttrLB->GetSelectEntry());
                    mpFillAttrLB->Clear();
                    mpFillAttrLB->Fill(pItem->GetGradientList());
                    mpFillAttrLB->SelectEntry(aString);
                }
                break;
            }
            case drawing::FillStyle_HATCH:
            {
                const SvxHatchListItem* pItem = dynamic_cast< const SvxHatchListItem* >(pState);

                if(pItem)
                {
                    OUString aString(mpFillAttrLB->GetSelectEntry());
                    mpFillAttrLB->Clear();
                    mpFillAttrLB->Fill(pItem->GetHatchList());
                    mpFillAttrLB->SelectEntry(aString);
                }
                break;
            }
            case drawing::FillStyle_BITMAP:
            {
                const SvxBitmapListItem* pItem = dynamic_cast< const SvxBitmapListItem* >(pState);

                if(pItem)
                {
                    OUString aString(mpFillAttrLB->GetSelectEntry());
                    mpFillAttrLB->Clear();
                    mpFillAttrLB->Fill(pItem->GetBitmapList());
                    mpFillAttrLB->SelectEntry(aString);
                }
                break;
            }
            default: // drawing::FillStyle_NONE
            {
                break;
            }
        }
    }
}

vcl::Window* SvxFillToolBoxControl::CreateItemWindow(vcl::Window *pParent)
{
    if(GetSlotId() == SID_ATTR_FILL_STYLE)
    {
        mpFillControl = new FillControl(pParent);
        // Thus the FillControl is known by SvxFillToolBoxControl
        // (and in order to remain compatible)
        mpFillControl->SetData(this);

        mpFillAttrLB = (SvxFillAttrBox*)mpFillControl->mpLbFillAttr;
        mpFillTypeLB = (SvxFillTypeBox*)mpFillControl->mpLbFillType;

        mpFillAttrLB->SetUniqueId(HID_FILL_ATTR_LISTBOX);
        mpFillTypeLB->SetUniqueId(HID_FILL_TYPE_LISTBOX);

        if(!mpStyleItem)
        {
            // for Writer and Calc it's not the same instance of
            // SvxFillToolBoxControl which gets used after deselecting
            // and selecting a DrawObject, thhus a useful initialization is
            // needed to get the FillType and the FillStyle List inited
            // correctly. This in combination with meLastXFS inited to
            // drawing::FillStyle_NONE do the trick
            mpStyleItem = new XFillStyleItem(drawing::FillStyle_SOLID);
        }

        return mpFillControl;
    }
    return NULL;
}

FillControl::FillControl(vcl::Window* pParent,WinBits nStyle)
:   Window(pParent,nStyle | WB_DIALOGCONTROL),
    mpLbFillType(new SvxFillTypeBox(this)),
    mpLbFillAttr(new SvxFillAttrBox(this)),
    maLogicalFillSize(40,80),
    maLogicalAttrSize(50,80),
    mnLastFillTypeControlSelectEntryPos(mpLbFillType->GetSelectEntryPos()),
    mnLastFillAttrControlSelectEntryPos(mpLbFillAttr->GetSelectEntryPos()),
    mbFillTypeChanged(false)
{
    Size aTypeSize(LogicToPixel(maLogicalFillSize,MAP_APPFONT));
    Size aAttrSize(LogicToPixel(maLogicalAttrSize,MAP_APPFONT));
    mpLbFillType->SetSizePixel(aTypeSize);
    mpLbFillAttr->SetSizePixel(aAttrSize);

    //to get the base height
    aTypeSize = mpLbFillType->GetSizePixel();
    aAttrSize = mpLbFillAttr->GetSizePixel();
    Point aAttrPnt = mpLbFillAttr->GetPosPixel();
    SetSizePixel(
        Size(aAttrPnt.X() + aAttrSize.Width(),
            std::max(aAttrSize.Height(),aTypeSize.Height())));

    mpLbFillType->SetSelectHdl(LINK(this,FillControl,SelectFillTypeHdl));
    mpLbFillAttr->SetSelectHdl(LINK(this,FillControl,SelectFillAttrHdl));
}

FillControl::~FillControl()
{
    delete mpLbFillType;
    delete mpLbFillAttr;
}

void FillControl::InitializeFillStyleAccordingToGivenFillType(drawing::FillStyle aFillStyle)
{
    SfxObjectShell* pSh = SfxObjectShell::Current();
    bool bDone(false);

    if(pSh)
    {
        // clear in all cases, else we would risk a mix of FillStyles in the Style list
        mpLbFillAttr->Clear();

        switch (aFillStyle)
        {
            case drawing::FillStyle_SOLID:
            {
                if(pSh->GetItem(SID_COLOR_TABLE))
                {
                    const SvxColorListItem* pItem = static_cast<const SvxColorListItem*>(pSh->GetItem(SID_COLOR_TABLE));
                    mpLbFillAttr->Enable();
                    mpLbFillAttr->Fill(pItem->GetColorList());
                    bDone = true;
                }
                break;
            }

            case drawing::FillStyle_GRADIENT:
            {
                if(pSh->GetItem(SID_GRADIENT_LIST))
                {
                    const SvxGradientListItem* pItem = static_cast< const SvxGradientListItem* >(pSh->GetItem(SID_GRADIENT_LIST));
                    mpLbFillAttr->Enable();
                    mpLbFillAttr->Fill(pItem->GetGradientList());
                    bDone = true;
                }
                break;
            }

            case drawing::FillStyle_HATCH:
            {
                if(pSh->GetItem(SID_HATCH_LIST))
                {
                    const SvxHatchListItem* pItem = static_cast< const SvxHatchListItem* >(pSh->GetItem(SID_HATCH_LIST));
                    mpLbFillAttr->Enable();
                    mpLbFillAttr->Fill(pItem->GetHatchList());
                    bDone = true;
                }
                break;
            }

            case drawing::FillStyle_BITMAP:
            {
                if(pSh->GetItem(SID_BITMAP_LIST))
                {
                    const SvxBitmapListItem* pItem = static_cast< const SvxBitmapListItem* >(pSh->GetItem(SID_BITMAP_LIST));
                    mpLbFillAttr->Enable();
                    mpLbFillAttr->Fill(pItem->GetBitmapList());
                    bDone = true;
                }
                break;
            }
            default: // drawing::FillStyle_NONE
            {
                // accept disable (no styles for drawing::FillStyle_NONE)
                break;
            }
        }
    }

    if (!bDone)
    {
        mpLbFillAttr->Disable();
    }
}

void FillControl::updateLastFillTypeControlSelectEntryPos()
{
    mnLastFillTypeControlSelectEntryPos = mpLbFillType->GetSelectEntryPos();
}

IMPL_LINK(FillControl,SelectFillTypeHdl,ListBox *,pBox)
{
    if(!pBox) // only work with real calls from ListBox, do not accept direct calls with zeros here
    {
        return 0;
    }

    const bool bAction(
           !mpLbFillType->IsTravelSelect() // keep TravelSelect, this means keyboard up/down in the list
        && mpLbFillType->GetSelectEntryCount()
        && mpLbFillType->GetSelectEntryPos() != mnLastFillTypeControlSelectEntryPos);

    updateLastFillTypeControlSelectEntryPos();
    drawing::FillStyle eXFS = static_cast< drawing::FillStyle >(mpLbFillType->GetSelectEntryPos());

    if(bAction && drawing::FillStyle_NONE != eXFS)
    {
        mbFillTypeChanged = true;
    }

    // update list of FillStyles in any case
    InitializeFillStyleAccordingToGivenFillType(eXFS);

    // for drawing::FillStyle_NONE do no longer call SelectFillAttrHdl (as done before),
    // trigger needed actions directly. This is the only action this handler
    // can trigger directly as the user action is finished in this case
    if(drawing::FillStyle_NONE == eXFS && bAction)
    {
        // for drawing::FillStyle_NONE do no longer call SelectFillAttrHdl,
        // trigger needed actions directly
        Any a;
        Sequence< PropertyValue > aArgsFillStyle(1);
        XFillStyleItem aXFillStyleItem(eXFS);

        aArgsFillStyle[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FillStyle"));
        aXFillStyleItem.QueryValue(a);
        aArgsFillStyle[0].Value = a;
        ((SvxFillToolBoxControl*)GetData())->Dispatch(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:FillStyle")), aArgsFillStyle);
    }

    mpLbFillType->Selected();

    // release focus. Needed to get focus automatically back to EditView
    if(mpLbFillType->IsRelease())
    {
        SfxViewShell* pViewShell = SfxViewShell::Current();

        if(pViewShell && pViewShell->GetWindow())
        {
            pViewShell->GetWindow()->GrabFocus();
        }
    }

    return 0;
}



void FillControl::updateLastFillAttrControlSelectEntryPos()
{
    mnLastFillAttrControlSelectEntryPos = mpLbFillAttr->GetSelectEntryPos();
}

IMPL_LINK(FillControl, SelectFillAttrHdl, ListBox *, pBox)
{
    if(!pBox) // only work with real calls from ListBox, do not accept direct calls with zeros here
    {
        return 0;
    }

    const bool bAction(
           !mpLbFillAttr->IsTravelSelect() // keep TravelSelect, this means keyboard up/down in the list
        && mpLbFillAttr->GetSelectEntryCount()
        && mpLbFillAttr->GetSelectEntryPos() != mnLastFillAttrControlSelectEntryPos);

    updateLastFillAttrControlSelectEntryPos();

    if(bAction)
    {
        SfxObjectShell* pSh = SfxObjectShell::Current();

        // Need to prepare the PropertyValue for the FillStyle dispatch action early,
        // else the call for FillType to Dispatch(".uno:FillStyle") will already destroy the current state
        // of selection in mpLbFillAttr again by calls to StateChanged which *will* set to no
        // selection again (e.g. when two objects, same fill style, but different fill attributes)
        Any a;
        Sequence< PropertyValue > aArgsFillAttr(1);
        OUString aFillAttrCommand;
        drawing::FillStyle eXFS(static_cast< drawing::FillStyle >(mpLbFillType->GetSelectEntryPos()));

        switch(eXFS)
        {
            default:
            case drawing::FillStyle_NONE:
            {
                // handled in SelectFillTypeHdl, nothing to do here
                break;
            }

            case drawing::FillStyle_SOLID:
            {
                // Entry gets tested against temporary color
                OUString aTmpStr = mpLbFillAttr->GetSelectEntry();
                if( aTmpStr.startsWith(TMP_STR_BEGIN) && aTmpStr.endsWith(TMP_STR_END) )
                {
                    aTmpStr = aTmpStr.copy(1, aTmpStr.getLength()-2);
                }

                XFillColorItem aXFillColorItem(aTmpStr, mpLbFillAttr->GetSelectEntryColor());
                aArgsFillAttr[0].Name = "FillColor";
                aXFillColorItem.QueryValue(a);
                aArgsFillAttr[0].Value = a;
                aFillAttrCommand = ".uno:FillColor";
                break;
            }
            case drawing::FillStyle_GRADIENT:
            {
                sal_Int32 nPos = mpLbFillAttr->GetSelectEntryPos();
                if (nPos != LISTBOX_ENTRY_NOTFOUND && pSh && pSh->GetItem(SID_GRADIENT_LIST))
                {
                    const SvxGradientListItem* pItem = static_cast< const SvxGradientListItem* >(pSh->GetItem(SID_GRADIENT_LIST));

                    if (nPos < pItem->GetGradientList()->Count())  // no temporary entry?
                    {
                        XGradient aGradient = pItem->GetGradientList()->GetGradient(nPos)->GetGradient();
                        XFillGradientItem aXFillGradientItem(mpLbFillAttr->GetSelectEntry(),aGradient);
                        aArgsFillAttr[0].Name = "FillGradient";
                        aXFillGradientItem.QueryValue(a);
                        aArgsFillAttr[0].Value = a;
                        aFillAttrCommand = ".uno:FillGradient";
                    }
                }
                break;
            }

            case drawing::FillStyle_HATCH:
            {
                sal_Int32 nPos = mpLbFillAttr->GetSelectEntryPos();
                if (nPos != LISTBOX_ENTRY_NOTFOUND && pSh && pSh->GetItem(SID_HATCH_LIST))
                {
                    const SvxHatchListItem* pItem = static_cast< const SvxHatchListItem* >(pSh->GetItem(SID_HATCH_LIST));

                    if (nPos < pItem->GetHatchList()->Count())  // no temporary entry?
                    {
                        XHatch aHatch = pItem->GetHatchList()->GetHatch(nPos)->GetHatch();
                        XFillHatchItem aXFillHatchItem(mpLbFillAttr->GetSelectEntry(), aHatch);

                        aArgsFillAttr[0].Name = "FillHatch";
                        aXFillHatchItem.QueryValue(a);
                        aArgsFillAttr[0].Value = a;
                        aFillAttrCommand = ".uno:FillHatch";
                    }
                }
                break;
            }

            case drawing::FillStyle_BITMAP:
            {
                sal_Int32 nPos = mpLbFillAttr->GetSelectEntryPos();
                if (nPos != LISTBOX_ENTRY_NOTFOUND && pSh && pSh->GetItem(SID_BITMAP_LIST))
                {
                    const SvxBitmapListItem* pItem = static_cast< const SvxBitmapListItem* >(pSh->GetItem(SID_BITMAP_LIST));

                    if (nPos < pItem->GetBitmapList()->Count())  // no temporary entry?
                    {
                        const XBitmapEntry* pXBitmapEntry = pItem->GetBitmapList()->GetBitmap(nPos);
                        const XFillBitmapItem aXFillBitmapItem(mpLbFillAttr->GetSelectEntry(),pXBitmapEntry->GetGraphicObject());

                        aArgsFillAttr[0].Name = "FillBitmap";
                        aXFillBitmapItem.QueryValue(a);
                        aArgsFillAttr[0].Value = a;
                        aFillAttrCommand = ".uno:FillBitmap";
                    }
                }
                break;
            }
        }

        // this is the place where evtl. a new slot action may be introduced to avoid the
        // two undo entries. Reason for this is that indeed two actions are executed, the fill style
        // and the fill attribute change. The sidebar already handles both separately, so
        // changing the fill style already changes the object and adds a default fill attribute for
        // the newly chosen fill style.
        // This control uses the older user's two-step action to select a fill style and a fill attribute. In
        // this case a lot of things may go wrong (e.g. the user stops that action and does something
        // different), thus the solution of the sidebar should be preferred from my POV in the future

        // first set the fill style if changed
        if(mbFillTypeChanged)
        {
            Sequence< PropertyValue > aArgsFillStyle(1);
            XFillStyleItem aXFillStyleItem(eXFS);

            aArgsFillStyle[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FillStyle"));
            aXFillStyleItem.QueryValue(a);
            aArgsFillStyle[0].Value = a;
            ((SvxFillToolBoxControl*)GetData())->Dispatch(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:FillStyle")), aArgsFillStyle);
            mbFillTypeChanged = false;
        }

        // second set fill attribute when a change was detected and prepared
        if(aFillAttrCommand.getLength())
        {
            ((SvxFillToolBoxControl*)GetData())->Dispatch(aFillAttrCommand, aArgsFillAttr);
        }

        // release focus. Needed to get focus automatically back to EditView
        if(mpLbFillAttr->IsRelease() && pBox)
        {
            SfxViewShell* pViewShell = SfxViewShell::Current();

            if(pViewShell && pViewShell->GetWindow())
            {
                pViewShell->GetWindow()->GrabFocus();
            }
        }
    }

    return 0;
}



void FillControl::Resize()
{
    // Width of the two list boxes not 1/2 : 1/2, but 2/5 : 3/5
    long nW = GetOutputSizePixel().Width() / 5;
    long nH = 180;
    long nSep = 0; // was previously 4

    mpLbFillType->SetSizePixel(Size(nW * 2 - nSep,nH));
    mpLbFillAttr->SetPosSizePixel(Point(nW * 2 + nSep,0),Size(nW * 3 - nSep,nH));
}

void FillControl::DataChanged(const DataChangedEvent& rDCEvt)
{
    if((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
        (rDCEvt.GetFlags() & AllSettingsFlags::STYLE))
    {
        Size aTypeSize(LogicToPixel(maLogicalFillSize,MAP_APPFONT));
        Size aAttrSize(LogicToPixel(maLogicalAttrSize,MAP_APPFONT));
        mpLbFillType->SetSizePixel(aTypeSize);
        mpLbFillAttr->SetSizePixel(aAttrSize);

        //to get the base height
        aTypeSize = mpLbFillType->GetSizePixel();
        aAttrSize = mpLbFillAttr->GetSizePixel();
        Point aAttrPnt = mpLbFillAttr->GetPosPixel();

        SetSizePixel(
            Size(aAttrPnt.X() + aAttrSize.Width(),
                std::max(aAttrSize.Height(), aTypeSize.Height())));
    }
    Window::DataChanged(rDCEvt);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
