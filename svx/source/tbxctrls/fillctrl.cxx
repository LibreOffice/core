/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <string> // HACK: prevent conflict between STLPORT and Workshop headers
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <rtl/ustring.hxx>
#include <svx/dialogs.hrc>

#define TMP_STR_BEGIN   '['
#define TMP_STR_END     ']'

#include "svx/drawitem.hxx"
#include "svx/xattr.hxx"
#include <svx/xtable.hxx>
#include <svx/fillctrl.hxx>
#include <svx/itemwin.hxx>
#include <svx/dialmgr.hxx>
#include "helpid.hrc"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;

SFX_IMPL_TOOLBOX_CONTROL( SvxFillToolBoxControl, XFillStyleItem );

/*************************************************************************
|*
|* SvxFillToolBoxControl
|*
\************************************************************************/

SvxFillToolBoxControl::SvxFillToolBoxControl(
    sal_uInt16 nSlotId,
    sal_uInt16 nId,
    ToolBox& rTbx )
:   SfxToolBoxControl( nSlotId, nId, rTbx ),
    mpStyleItem(0),
    mpColorItem(0),
    mpGradientItem(0),
    mpHatchItem(0),
    mpBitmapItem(0),
    mpFillControl(0),
    mpFillTypeLB(0),
    mpFillAttrLB(0),
    meLastXFS(XFILL_NONE),
    mbUpdate(false)
{
    addStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FillColor" )));
    addStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FillGradient" )));
    addStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FillHatch" )));
    addStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FillBitmap" )));
    addStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ColorTableState" )));
    addStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:GradientListState" )));
    addStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:HatchListState" )));
    addStatusListener( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:BitmapListState" )));
}

//========================================================================

SvxFillToolBoxControl::~SvxFillToolBoxControl()
{
    delete mpStyleItem;
    delete mpColorItem;
    delete mpGradientItem;
    delete mpHatchItem;
    delete mpBitmapItem;
}

//========================================================================

void SvxFillToolBoxControl::StateChanged(
    sal_uInt16 nSID,
    SfxItemState eState,
    const SfxPoolItem* pState)
{
    bool bEnableControls(false);

    if(eState == SFX_ITEM_DISABLED)
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
    else if(SFX_ITEM_AVAILABLE == eState)
    {
        // slot available state
        if(nSID == SID_ATTR_FILL_STYLE)
        {
            delete mpStyleItem;
            mpStyleItem = static_cast< XFillStyleItem* >(pState->Clone());
            mpFillTypeLB->Enable();
        }
        else if(mpStyleItem)
        {
            const XFillStyle eXFS(static_cast< XFillStyle >(mpStyleItem->GetValue()));

            if(nSID == SID_ATTR_FILL_COLOR)
            {
                delete mpColorItem;
                mpColorItem = static_cast< XFillColorItem* >(pState->Clone());

                if(eXFS == XFILL_SOLID)
                {
                    bEnableControls = true;
                }
            }
            else if(nSID == SID_ATTR_FILL_GRADIENT)
            {
                delete mpGradientItem;
                mpGradientItem = static_cast< XFillGradientItem* >(pState->Clone());

                if(eXFS == XFILL_GRADIENT)
                {
                    bEnableControls = true;
                }
            }
            else if(nSID == SID_ATTR_FILL_HATCH)
            {
                delete mpHatchItem;
                mpHatchItem = static_cast< XFillHatchItem* >(pState->Clone());

                if(eXFS == XFILL_HATCH)
                {
                    bEnableControls = true;
                }
            }
            else if(nSID == SID_ATTR_FILL_BITMAP)
            {
                delete mpBitmapItem;
                mpBitmapItem = static_cast< XFillBitmapItem* >(pState->Clone());

                if(eXFS == XFILL_BITMAP)
                {
                    bEnableControls = true;
                }
            }
        }

        if(mpStyleItem)
        {
            // ensure that the correct entry is selected in mpFillTypeLB
            XFillStyle eXFS(static_cast< XFillStyle >(mpStyleItem->GetValue()));
            const bool bFillTypeChangedByUser(mpFillControl->mbFillTypeChanged);

            if(bFillTypeChangedByUser)
            {
                meLastXFS = static_cast< XFillStyle >(mpFillControl->mnLastFillTypeControlSelectEntryPos);
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
            XFillStyle eXFS(XFILL_NONE);

            if(mpStyleItem)
            {
                eXFS = static_cast< XFillStyle >(mpStyleItem->GetValue());
            }

            if(!mpStyleItem ||
                (nSID == SID_ATTR_FILL_COLOR && eXFS == XFILL_SOLID) ||
                (nSID == SID_ATTR_FILL_GRADIENT && eXFS == XFILL_GRADIENT) ||
                (nSID == SID_ATTR_FILL_HATCH && eXFS == XFILL_HATCH) ||
                (nSID == SID_ATTR_FILL_BITMAP && eXFS == XFILL_BITMAP))
            {
                mpFillAttrLB->SetNoSelection();
            }
        }
    }
}

//========================================================================

void SvxFillToolBoxControl::Update(const SfxPoolItem* pState)
{
    if(mpStyleItem && pState && mbUpdate)
    {
        mbUpdate = false;
        const XFillStyle eXFS(static_cast< XFillStyle >(mpStyleItem->GetValue()));

        // Pruefen, ob Fuellstil schon vorher aktiv war
        if(meLastXFS != eXFS)
        {
            // update mnLastFillTypeControlSelectEntryPos and fill style list
            mpFillControl->updateLastFillTypeControlSelectEntryPos();
            mpFillControl->InitializeFillStyleAccordingToGivenFillType(eXFS);
            meLastXFS = eXFS;
        }

        switch(eXFS)
        {
            case XFILL_NONE:
            {
                break;
            }

            case XFILL_SOLID:
            {
                if(mpColorItem)
                {
                    String aString(mpColorItem->GetName());
                    ::Color aColor = mpColorItem->GetColorValue();

                    mpFillAttrLB->SelectEntry(aString);

                    if(mpFillAttrLB->GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND || mpFillAttrLB->GetSelectEntryColor() != aColor)
                    {
                        mpFillAttrLB->SelectEntry(aColor);
                    }

                    // Pruefen, ob Eintrag nicht in der Liste ist
                    if(mpFillAttrLB->GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND || mpFillAttrLB->GetSelectEntryColor() != aColor)
                    {
                        sal_uInt16 nCount = mpFillAttrLB->GetEntryCount();
                        String aTmpStr;
                        if(nCount > 0)
                        {
                            //Letzter Eintrag wird auf temporaere Farbe geprueft
                            aTmpStr = mpFillAttrLB->GetEntry(nCount - 1);

                            if(aTmpStr.GetChar(0) == TMP_STR_BEGIN && aTmpStr.GetChar(aTmpStr.Len() - 1) == TMP_STR_END)
                            {
                                mpFillAttrLB->RemoveEntry(nCount - 1);
                            }
                        }

                        aTmpStr = TMP_STR_BEGIN;
                        aTmpStr += aString;
                        aTmpStr += TMP_STR_END;

                        sal_uInt16 nPos = mpFillAttrLB->InsertEntry(aColor,aTmpStr);
                        mpFillAttrLB->SelectEntryPos(nPos);
                    }
                }
                else
                {
                    mpFillAttrLB->SetNoSelection();
                }
                break;
            }

            case XFILL_GRADIENT:
            {
                if(mpGradientItem)
                {
                    String aString(mpGradientItem->GetName());
                    mpFillAttrLB->SelectEntry(aString);

                    // Pruefen, ob Eintrag nicht in der Liste ist
                    if(mpFillAttrLB->GetSelectEntry() != aString)
                    {
                        sal_uInt16 nCount = mpFillAttrLB->GetEntryCount();
                        String aTmpStr;

                        if(nCount > 0)
                        {
                            //Letzter Eintrag wird auf temporaeren Eintrag geprueft
                            aTmpStr = mpFillAttrLB->GetEntry(nCount - 1);

                            if(aTmpStr.GetChar(0) == TMP_STR_BEGIN && aTmpStr.GetChar(aTmpStr.Len() - 1) == TMP_STR_END)
                            {
                                mpFillAttrLB->RemoveEntry(nCount - 1);
                            }
                        }

                        aTmpStr = TMP_STR_BEGIN;
                        aTmpStr += aString;
                        aTmpStr += TMP_STR_END;

                        XGradientEntry* pEntry = new XGradientEntry(mpGradientItem->GetGradientValue(),aTmpStr);
                        XGradientListSharedPtr aGradientList(XPropertyListFactory::CreateSharedXGradientList(String::CreateFromAscii("TmpList")));

                        aGradientList->Insert(pEntry);
                        aGradientList->SetDirty(false);
                        const Bitmap aBmp = aGradientList->GetUiBitmap(0);

                        if(!aBmp.IsEmpty())
                        {
                            ((ListBox*)mpFillAttrLB)->InsertEntry(pEntry->GetName(),aBmp);
                            mpFillAttrLB->SelectEntryPos(mpFillAttrLB->GetEntryCount() - 1);
                        }
                    }
                }
                else
                {
                    mpFillAttrLB->SetNoSelection();
                }
                break;
            }

            case XFILL_HATCH:
            {
                if(mpHatchItem)
                {
                    String aString(mpHatchItem->GetName());
                    mpFillAttrLB->SelectEntry(aString);

                    // Pruefen, ob Eintrag nicht in der Liste ist
                    if(mpFillAttrLB->GetSelectEntry() != aString)
                    {
                        sal_uInt16 nCount = mpFillAttrLB->GetEntryCount();
                        String aTmpStr;
                        if(nCount > 0)
                        {
                            //Letzter Eintrag wird auf temporaeren Eintrag geprueft
                            aTmpStr = mpFillAttrLB->GetEntry(nCount - 1);
                            if(aTmpStr.GetChar(0) == TMP_STR_BEGIN &&
                                aTmpStr.GetChar(aTmpStr.Len() - 1) == TMP_STR_END)
                            {
                                mpFillAttrLB->RemoveEntry(nCount - 1);
                            }
                        }

                        aTmpStr = TMP_STR_BEGIN;
                        aTmpStr += aString;
                        aTmpStr += TMP_STR_END;

                        XHatchEntry* pEntry = new XHatchEntry(mpHatchItem->GetHatchValue(),aTmpStr);
                        XHatchListSharedPtr aHatchList(XPropertyListFactory::CreateSharedXHatchList(String::CreateFromAscii("TmpList")));

                        aHatchList->Insert(pEntry);
                        aHatchList->SetDirty(sal_False);
                        const Bitmap aBmp = aHatchList->GetUiBitmap(0);

                        if(!aBmp.IsEmpty())
                        {
                            ((ListBox*)mpFillAttrLB)->InsertEntry(pEntry->GetName(),aBmp);
                            mpFillAttrLB->SelectEntryPos(mpFillAttrLB->GetEntryCount() - 1);
                        }
                    }
                }
                else
                {
                    mpFillAttrLB->SetNoSelection();
                }
                break;
            }

            case XFILL_BITMAP:
            {
                if(mpBitmapItem)
                {
                    String aString(mpBitmapItem->GetName());
                    mpFillAttrLB->SelectEntry(aString);

                    // Pruefen, ob Eintrag nicht in der Liste ist
                    if(mpFillAttrLB->GetSelectEntry() != aString)
                    {
                        sal_uInt16 nCount = mpFillAttrLB->GetEntryCount();
                        String aTmpStr;

                        if(nCount > 0)
                        {
                            //Letzter Eintrag wird auf temporaeren Eintrag geprueft
                            aTmpStr = mpFillAttrLB->GetEntry(nCount - 1);

                            if(aTmpStr.GetChar(0) == TMP_STR_BEGIN && aTmpStr.GetChar(aTmpStr.Len() - 1) == TMP_STR_END)
                            {
                                mpFillAttrLB->RemoveEntry(nCount - 1);
                            }
                        }

                        aTmpStr = TMP_STR_BEGIN;
                        aTmpStr += aString;
                        aTmpStr += TMP_STR_END;

                        XBitmapListSharedPtr aNew(XPropertyListFactory::CreateSharedXBitmapList(String::CreateFromAscii("TmpList")));
                        aNew->Insert(new XBitmapEntry(mpBitmapItem->GetGraphicObject(),aTmpStr));
                        aNew->SetDirty(false);

                        mpFillAttrLB->Fill(aNew);
                        mpFillAttrLB->SelectEntryPos(mpFillAttrLB->GetEntryCount() - 1);
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
                DBG_ERROR("Nicht unterstuetzter Flaechentyp");
                break;
            }
        }

        // update mnLastFillAttrControlSelectEntryPos
        mpFillControl->updateLastFillAttrControlSelectEntryPos();
    }

    if(pState && mpStyleItem)
    {
        XFillStyle eXFS = static_cast< XFillStyle >(mpStyleItem->GetValue());

        // Die Listen haben sich geaendert ?
        switch(eXFS)
        {
            case XFILL_SOLID:
            {
                const SvxColorTableItem* pItem = dynamic_cast< const SvxColorTableItem* >(pState);

                if(pItem)
                {
                    ::Color aTmpColor(mpFillAttrLB->GetSelectEntryColor());
                    mpFillAttrLB->Clear();
                    mpFillAttrLB->Fill(pItem->GetColorTable());
                    mpFillAttrLB->SelectEntry(aTmpColor);
                }
                break;
            }
            case XFILL_GRADIENT:
            {
                const SvxGradientListItem* pItem = dynamic_cast< const SvxGradientListItem* >(pState);

                if(pItem)
                {
                    String aString(mpFillAttrLB->GetSelectEntry());
                    mpFillAttrLB->Clear();
                    mpFillAttrLB->Fill(pItem->GetGradientList());
                    mpFillAttrLB->SelectEntry(aString);
                }
                break;
            }
            case XFILL_HATCH:
            {
                const SvxHatchListItem* pItem = dynamic_cast< const SvxHatchListItem* >(pState);

                if(pItem)
                {
                    String aString(mpFillAttrLB->GetSelectEntry());
                    mpFillAttrLB->Clear();
                    mpFillAttrLB->Fill(pItem->GetHatchList());
                    mpFillAttrLB->SelectEntry(aString);
                }
                break;
            }
            case XFILL_BITMAP:
            {
                const SvxBitmapListItem* pItem = dynamic_cast< const SvxBitmapListItem* >(pState);

                if(pItem)
                {
                    String aString(mpFillAttrLB->GetSelectEntry());
                    mpFillAttrLB->Clear();
                    mpFillAttrLB->Fill(pItem->GetBitmapList());
                    mpFillAttrLB->SelectEntry(aString);
                }
                break;
            }
            default: // XFILL_NONE
            {
                break;
            }
        }
    }
}

//========================================================================

Window* SvxFillToolBoxControl::CreateItemWindow(Window *pParent)
{
    if(GetSlotId() == SID_ATTR_FILL_STYLE)
    {
        mpFillControl = new FillControl(pParent);
        // Damit dem FillControl das SvxFillToolBoxControl bekannt ist
        // (und um kompatibel zu bleiben)
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
            // XFILL_NONE do the trick
            mpStyleItem = new XFillStyleItem(XFILL_SOLID);
        }

        return mpFillControl;
    }
    return NULL;
}

/*************************************************************************
|*
|* FillControl
|*
\************************************************************************/

FillControl::FillControl(Window* pParent,WinBits nStyle)
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
        Max(aAttrSize.Height(),aTypeSize.Height())));

    mpLbFillType->SetSelectHdl(LINK(this,FillControl,SelectFillTypeHdl));
    mpLbFillAttr->SetSelectHdl(LINK(this,FillControl,SelectFillAttrHdl));
}

//------------------------------------------------------------------------

FillControl::~FillControl()
{
    delete mpLbFillType;
    delete mpLbFillAttr;
}

//------------------------------------------------------------------------

void FillControl::InitializeFillStyleAccordingToGivenFillType(XFillStyle aFillStyle)
{
    SfxObjectShell* pSh = SfxObjectShell::Current();
    bool bDone(false);

    if(pSh)
    {
        // clear in all cases, else we would risk a mix of FillStyles in the Style list
        mpLbFillAttr->Clear();

        switch(aFillStyle)
        {
            case XFILL_SOLID:
            {
                if(pSh->GetItem(SID_COLOR_TABLE))
                {
                    const SvxColorTableItem* pItem = static_cast< const SvxColorTableItem* >(pSh->GetItem(SID_COLOR_TABLE));
                    mpLbFillAttr->Enable();
                    mpLbFillAttr->Fill(pItem->GetColorTable());
                    bDone = true;
                }
                break;
            }

            case XFILL_GRADIENT:
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

            case XFILL_HATCH:
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

            case XFILL_BITMAP:
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
            default: // XFILL_NONE
            {
                // accept disable (no styles for XFILL_NONE)
                break;
            }
        }
    }

    if(!bDone)
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
    XFillStyle eXFS = static_cast< XFillStyle >(mpLbFillType->GetSelectEntryPos());

    if(bAction && XFILL_NONE != eXFS)
    {
        mbFillTypeChanged = true;
    }

    // update list of FillStyles in any case
    InitializeFillStyleAccordingToGivenFillType(eXFS);

    // for XFILL_NONE do no longer call SelectFillAttrHdl (as done before),
    // trigger needed actions directly. This is the only action this handler
    // can trigger directly as the user action is finished in this case
    if(XFILL_NONE == eXFS && bAction)
    {
        // for XFILL_NONE do no longer call SelectFillAttrHdl,
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

//------------------------------------------------------------------------

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
        ::rtl::OUString aFillAttrCommand;
        XFillStyle eXFS(static_cast< XFillStyle >(mpLbFillType->GetSelectEntryPos()));

        switch(eXFS)
        {
            case XFILL_NONE:
            {
                // handled in SelectFillTypeHdl, nothing to do here
                break;
            }

            case XFILL_SOLID:
            {
                //Eintrag wird auf temporaere Farbe geprueft
                String aTmpStr = mpLbFillAttr->GetSelectEntry();

                if(aTmpStr.GetChar(0) == TMP_STR_BEGIN && aTmpStr.GetChar(aTmpStr.Len() - 1) == TMP_STR_END)
                {
                    aTmpStr.Erase(aTmpStr.Len() - 1,1);
                    aTmpStr.Erase(0,1);
                }

                XFillColorItem aXFillColorItem(aTmpStr,mpLbFillAttr->GetSelectEntryColor());
                aArgsFillAttr[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FillColor"));
                aXFillColorItem.QueryValue(a);
                aArgsFillAttr[0].Value = a;
                aFillAttrCommand = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:FillColor"));
                break;
            }
            case XFILL_GRADIENT:
            {
                sal_uInt16 nPos = mpLbFillAttr->GetSelectEntryPos();

                if(nPos != LISTBOX_ENTRY_NOTFOUND && pSh && pSh->GetItem(SID_GRADIENT_LIST))
                {
                    const SvxGradientListItem* pItem = static_cast< const SvxGradientListItem* >(pSh->GetItem(SID_GRADIENT_LIST));

                    if(nPos < pItem->GetGradientList()->Count())  // kein temp. Eintrag ?
                    {
                        XGradient aGradient = pItem->GetGradientList()->GetGradient(nPos)->GetGradient();
                        XFillGradientItem aXFillGradientItem(mpLbFillAttr->GetSelectEntry(),aGradient);
                        aArgsFillAttr[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FillGradient"));
                        aXFillGradientItem.QueryValue(a);
                        aArgsFillAttr[0].Value = a;
                        aFillAttrCommand = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:FillGradient"));
                    }
                }
                break;
            }

            case XFILL_HATCH:
            {
                sal_uInt16 nPos = mpLbFillAttr->GetSelectEntryPos();

                if(nPos != LISTBOX_ENTRY_NOTFOUND && pSh && pSh->GetItem(SID_HATCH_LIST))
                {
                    const SvxHatchListItem* pItem = static_cast< const SvxHatchListItem* >(pSh->GetItem(SID_HATCH_LIST));

                    if(nPos < pItem->GetHatchList()->Count())  // kein temp. Eintrag ?
                    {
                        XHatch aHatch = pItem->GetHatchList()->GetHatch(nPos)->GetHatch();
                        XFillHatchItem aXFillHatchItem(mpLbFillAttr->GetSelectEntry(),aHatch);

                        aArgsFillAttr[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FillHatch"));
                        aXFillHatchItem.QueryValue(a);
                        aArgsFillAttr[0].Value = a;
                        aFillAttrCommand = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:FillHatch"));
                    }
                }
                break;
            }

            case XFILL_BITMAP:
            {
                sal_uInt16 nPos = mpLbFillAttr->GetSelectEntryPos();

                if(nPos != LISTBOX_ENTRY_NOTFOUND && pSh && pSh->GetItem(SID_BITMAP_LIST))
                {
                    const SvxBitmapListItem* pItem = static_cast< const SvxBitmapListItem* >(pSh->GetItem(SID_BITMAP_LIST));

                    if(nPos < pItem->GetBitmapList()->Count())  // kein temp. Eintrag ?
                    {
                        const XBitmapEntry* pXBitmapEntry = pItem->GetBitmapList()->GetBitmap(nPos);
                        const XFillBitmapItem aXFillBitmapItem(mpLbFillAttr->GetSelectEntry(),pXBitmapEntry->GetGraphicObject());

                        aArgsFillAttr[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FillBitmap"));
                        aXFillBitmapItem.QueryValue(a);
                        aArgsFillAttr[0].Value = a;
                        aFillAttrCommand = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:FillBitmap"));
                    }
                }
                break;
            }
        }

        // this is the place where evtl. a new slot action may be introduced to avoid the
        // two undo entries. Reason for this is that indeed two actions are executed, the fill style
        // and the fill attribute change. The sidebar already handles both separately, so
        // changing the fill style already changes the object and adds a default fill attribute for
        // the newly choosen fill style.
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

//------------------------------------------------------------------------

void FillControl::Resize()
{
    // Breite der beiden ListBoxen nicht 1/2 : 1/2, sondern 2/5 : 3/5
    long nW = GetOutputSizePixel().Width() / 5;
    long nH = 180;
    long nSep = 0; // war vorher 4

    mpLbFillType->SetSizePixel(Size(nW * 2 - nSep,nH));
    mpLbFillAttr->SetPosSizePixel(Point(nW * 2 + nSep,0),Size(nW * 3 - nSep,nH));
}

//------------------------------------------------------------------------

void FillControl::DataChanged(const DataChangedEvent& rDCEvt)
{
    if((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
        (rDCEvt.GetFlags() & SETTINGS_STYLE))
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
            Max(aAttrSize.Height(),aTypeSize.Height())));
    }
    Window::DataChanged(rDCEvt);
}

//------------------------------------------------------------------------
//eof
