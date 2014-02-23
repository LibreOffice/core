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
#include <svx/dialogs.hrc>

#define DELAY_TIMEOUT           300

#define TMP_STR_BEGIN   "["
#define TMP_STR_END     "]"

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

SvxFillToolBoxControl::SvxFillToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx ),

    pStyleItem      ( NULL ),
    pColorItem      ( NULL ),
    pGradientItem   ( NULL ),
    pHatchItem      ( NULL ),
    pBitmapItem     ( NULL ),
    pFillControl    ( NULL ),
    pFillTypeLB     ( NULL ),
    pFillAttrLB     ( NULL ),
    bUpdate         ( sal_False ),
    eLastXFS        ( XFILL_NONE )
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
    delete pStyleItem;
    delete pColorItem;
    delete pGradientItem;
    delete pHatchItem;
    delete pBitmapItem;
}



void SvxFillToolBoxControl::StateChanged(

    sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )

{

    if( eState == SFX_ITEM_DISABLED )
    {
        if( nSID == SID_ATTR_FILL_STYLE )
        {
            pFillTypeLB->Disable();
            pFillTypeLB->SetNoSelection();
        }
        pFillAttrLB->Disable();
        pFillAttrLB->SetNoSelection();
    }
    else
    {
        if ( SFX_ITEM_AVAILABLE == eState )
        {
            bool bEnableControls = false;
            if( nSID == SID_ATTR_FILL_STYLE )
            {
                delete pStyleItem;
                pStyleItem = (XFillStyleItem*) pState->Clone();
                pFillTypeLB->Enable();
            }
            else if( pStyleItem )
            {
                XFillStyle eXFS = (XFillStyle)pStyleItem->GetValue();

                if( nSID == SID_ATTR_FILL_COLOR )
                {
                    delete pColorItem;
                    pColorItem = (XFillColorItem*) pState->Clone();

                    if( eXFS == XFILL_SOLID )
                        bEnableControls = true;
                }
                else if( nSID == SID_ATTR_FILL_GRADIENT )
                {
                    delete pGradientItem;
                    pGradientItem = (XFillGradientItem*) pState->Clone();

                    if( eXFS == XFILL_GRADIENT )
                        bEnableControls = true;
                }
                else if( nSID == SID_ATTR_FILL_HATCH )
                {
                    delete pHatchItem;
                    pHatchItem = (XFillHatchItem*) pState->Clone();

                    if( eXFS == XFILL_HATCH )
                        bEnableControls = true;
                }
                else if( nSID == SID_ATTR_FILL_BITMAP )
                {
                    delete pBitmapItem;
                    pBitmapItem = (XFillBitmapItem*) pState->Clone();

                    if( eXFS == XFILL_BITMAP )
                        bEnableControls = true;
                }
            }

            if( pStyleItem )
            {
                // ensure that the correct entry is selected in pFillTypeLB. It
                // might have been changed by nSID == SID_ATTR_FILL_STYLE, but
                // it might also be in an in-between state when user had started to
                // change fillstyle, but not yet changed fillvalue for new style
                // and when nSID == SID_ATTR_FILL_COLOR/SID_ATTR_FILL_GRADIENT/
                // SID_ATTR_FILL_HATCH/SID_ATTR_FILL_BITMAP value change is triggered
                eLastXFS = pFillTypeLB->GetSelectEntryPos();
                XFillStyle eXFS = (XFillStyle)pStyleItem->GetValue();

                if(eLastXFS != eXFS)
                {
                    bUpdate = sal_True;
                    pFillTypeLB->SelectEntryPos( sal::static_int_cast< sal_uInt16 >( eXFS ) );
                }

                pFillAttrLB->Enable();
            }

            if( bEnableControls )
            {
                //pFillTypeLB->Enable();
                pFillAttrLB->Enable();

                bUpdate = sal_True;
            }

            Update( pState );
        }
        else
        {
            // empty or ambiguous status
            if( nSID == SID_ATTR_FILL_STYLE )
            {
                pFillTypeLB->SetNoSelection();
                pFillAttrLB->Disable();
                pFillAttrLB->SetNoSelection();
                bUpdate = sal_False;
            }
            else
            {
                XFillStyle eXFS = XFILL_NONE;
                if( pStyleItem )
                    eXFS = (XFillStyle)pStyleItem->GetValue();
                if( !pStyleItem ||
                    ( nSID == SID_ATTR_FILL_COLOR    && eXFS == XFILL_SOLID ) ||
                    ( nSID == SID_ATTR_FILL_GRADIENT && eXFS == XFILL_GRADIENT ) ||
                    ( nSID == SID_ATTR_FILL_HATCH    && eXFS == XFILL_HATCH ) ||
                    ( nSID == SID_ATTR_FILL_BITMAP   && eXFS == XFILL_BITMAP ) )
                {
                    pFillAttrLB->SetNoSelection();
                    //bUpdate = sal_False;
                }
            }
        }
    }
}



void SvxFillToolBoxControl::Update( const SfxPoolItem* pState )
{
    if ( pStyleItem && pState && bUpdate )
    {
        bUpdate = sal_False;

        XFillStyle eXFS = (XFillStyle)pStyleItem->GetValue();

        // Check if the fill style was already active
        //if( eTmpXFS != eXFS )
        if( (XFillStyle) eLastXFS != eXFS )
            pFillControl->SelectFillTypeHdl( NULL );
            //eLastXFS = eXFS;

        switch( eXFS )
        {
            case XFILL_NONE:
            break;

            case XFILL_SOLID:
            {
                if ( pColorItem )
                {
                    OUString aString( pColorItem->GetName() );
                    ::Color aColor = pColorItem->GetColorValue();

                    pFillAttrLB->SelectEntry( aString );

                    if ( pFillAttrLB->GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND ||
                         pFillAttrLB->GetSelectEntryColor() != aColor )
                        pFillAttrLB->SelectEntry( aColor );

                    // Check if the entry is not in the list
                    if( pFillAttrLB->GetSelectEntryPos() ==
                        LISTBOX_ENTRY_NOTFOUND ||
                        pFillAttrLB->GetSelectEntryColor() != aColor )
                    {
                        sal_uInt16 nCount = pFillAttrLB->GetEntryCount();
                        OUString aTmpStr;
                        if( nCount > 0 )
                        {
                            // Last entry gets tested against temporary color
                            aTmpStr = pFillAttrLB->GetEntry( nCount - 1 );
                            if(  aTmpStr.startsWith(TMP_STR_BEGIN) &&
                                 aTmpStr.endsWith(TMP_STR_END) )
                            {
                                pFillAttrLB->RemoveEntry( nCount - 1 );
                            }
                        }
                        aTmpStr = TMP_STR_BEGIN + aString + TMP_STR_END;

                        //pFillAttrLB->SetUpdateMode( sal_False );
                        sal_uInt16 nPos = pFillAttrLB->InsertEntry( aColor, aTmpStr );
                        //pFillAttrLB->SetUpdateMode( sal_True );
                        pFillAttrLB->SelectEntryPos( nPos );
                    }
                }
                else
                    pFillAttrLB->SetNoSelection();
            }
            break;

            case XFILL_GRADIENT:
            {
                if ( pGradientItem )
                {
                    OUString aString( pGradientItem->GetName() );
                    pFillAttrLB->SelectEntry( aString );
                    // Check if the entry is not in the list
                    if( pFillAttrLB->GetSelectEntry() != aString )
                    {
                        sal_uInt16 nCount = pFillAttrLB->GetEntryCount();
                        OUString aTmpStr;
                        if( nCount > 0 )
                        {
                            // Last entry gets tested against temporary entry
                            aTmpStr = pFillAttrLB->GetEntry( nCount - 1 );
                            if(  aTmpStr.startsWith(TMP_STR_BEGIN) &&
                                 aTmpStr.endsWith(TMP_STR_END) )
                            {
                                pFillAttrLB->RemoveEntry( nCount - 1 );
                            }
                        }
                        aTmpStr = TMP_STR_BEGIN + aString + TMP_STR_END;

                        XGradientEntry* pEntry = new XGradientEntry( pGradientItem->GetGradientValue(), aTmpStr );
                        XGradientList aGradientList( "", ""/*TODO?*/ );
                        aGradientList.Insert( pEntry );
                        aGradientList.SetDirty( false );
                        const Bitmap aBmp = aGradientList.GetUiBitmap( 0 );

                        if( !aBmp.IsEmpty() )
                        {
                            pFillAttrLB->InsertEntry(pEntry->GetName(), Image(aBmp));
                            pFillAttrLB->SelectEntryPos( pFillAttrLB->GetEntryCount() - 1 );
                            //delete pBmp;
                        }

                        aGradientList.Remove( 0 );
                        delete pEntry;
                    }
                }
                else
                    pFillAttrLB->SetNoSelection();
            }
            break;

            case XFILL_HATCH:
            {
                if ( pHatchItem )
                {
                    OUString aString( pHatchItem->GetName() );
                    pFillAttrLB->SelectEntry( aString );
                    // Check if the entry is not in the list
                    if( pFillAttrLB->GetSelectEntry() != aString )
                    {
                        sal_uInt16 nCount = pFillAttrLB->GetEntryCount();
                        OUString aTmpStr;
                        if( nCount > 0 )
                        {
                            // Last entry gets tested against temporary entry
                            aTmpStr = pFillAttrLB->GetEntry( nCount - 1 );
                            if(  aTmpStr.startsWith(TMP_STR_BEGIN) &&
                                 aTmpStr.endsWith(TMP_STR_END) )
                            {
                                pFillAttrLB->RemoveEntry( nCount - 1 );
                            }
                        }
                        aTmpStr = TMP_STR_BEGIN + aString + TMP_STR_END;

                        XHatchEntry* pEntry = new XHatchEntry( pHatchItem->GetHatchValue(), aTmpStr );
                        XHatchList aHatchList( "", ""/*TODO?*/ );
                        aHatchList.Insert( pEntry );
                        aHatchList.SetDirty( false );
                        const Bitmap aBmp = aHatchList.GetUiBitmap( 0 );

                        if( !aBmp.IsEmpty() )
                        {
                            pFillAttrLB->InsertEntry(pEntry->GetName(), Image(aBmp));
                            pFillAttrLB->SelectEntryPos( pFillAttrLB->GetEntryCount() - 1 );
                            //delete pBmp;
                        }

                        aHatchList.Remove( 0 );
                        delete pEntry;
                    }
                }
                else
                    pFillAttrLB->SetNoSelection();
            }
            break;

            case XFILL_BITMAP:
            {
                if ( pBitmapItem )
                    // &&
                    // SfxObjectShell::Current()    &&
                    // SfxObjectShell::Current()->GetItem( SID_BITMAP_LIST ) )
                {
                    OUString aString( pBitmapItem->GetName() );
                    // Bitmap aBitmap( pBitmapItem->GetValue() );

                    // SvxBitmapListItem aItem( *(const SvxBitmapListItem*)(
                    //  SfxObjectShell::Current()->GetItem( SID_BITMAP_LIST ) ) );
                    pFillAttrLB->SelectEntry( aString );
                    // Check if the entry is not in the list
                    if( pFillAttrLB->GetSelectEntry() != aString )
                    {
                        sal_uInt16 nCount = pFillAttrLB->GetEntryCount();
                        OUString aTmpStr;
                        if( nCount > 0 )
                        {
                            // Last entry gets tested against temporary entry
                            aTmpStr = pFillAttrLB->GetEntry( nCount - 1 );
                            if(  aTmpStr.startsWith(TMP_STR_BEGIN) &&
                                 aTmpStr.endsWith(TMP_STR_END) )
                            {
                                pFillAttrLB->RemoveEntry( nCount - 1 );
                            }
                        }
                        aTmpStr = TMP_STR_BEGIN + aString + TMP_STR_END;

                        XBitmapEntry* pEntry = new XBitmapEntry(pBitmapItem->GetGraphicObject(), aTmpStr);
                        XBitmapListRef xBitmapList =
                            XPropertyList::CreatePropertyList(XBITMAP_LIST,
                            OUString("TmpList"), ""/*TODO?*/)->AsBitmapList();
                        xBitmapList->Insert( pEntry );
                        xBitmapList->SetDirty( false );
                        pFillAttrLB->Fill( xBitmapList );
                        pFillAttrLB->SelectEntryPos( pFillAttrLB->GetEntryCount() - 1 );
                        xBitmapList->Remove( 0 );
                        delete pEntry;
                    }
                }
                else
                    pFillAttrLB->SetNoSelection();
            }
            break;

            default:
                OSL_FAIL( "Unsupported fill type" );
            break;
        }
    }

    if( pState && pStyleItem )
    {
        XFillStyle eXFS = (XFillStyle) pStyleItem->GetValue();

        // Does the lists have changed?
        if( pState->ISA( SvxColorListItem ) &&
            eXFS == XFILL_SOLID )
        {
            ::Color aTmpColor( pFillAttrLB->GetSelectEntryColor() );
            pFillAttrLB->Clear();
            pFillAttrLB->Fill( ( (SvxColorListItem*)pState )->GetColorList() );
            pFillAttrLB->SelectEntry( aTmpColor );
        }
        if( pState->ISA( SvxGradientListItem ) &&
            eXFS == XFILL_GRADIENT )
        {
            OUString aString( pFillAttrLB->GetSelectEntry() );
            pFillAttrLB->Clear();
            pFillAttrLB->Fill( ( (SvxGradientListItem*)pState )->GetGradientList() );
            pFillAttrLB->SelectEntry( aString );
        }
        if( pState->ISA( SvxHatchListItem ) &&
            eXFS == XFILL_HATCH )
        {
            OUString aString( pFillAttrLB->GetSelectEntry() );
            pFillAttrLB->Clear();
            pFillAttrLB->Fill( ( (SvxHatchListItem*)pState )->GetHatchList() );
            pFillAttrLB->SelectEntry( aString );
        }
        if( pState->ISA( SvxBitmapListItem ) &&
            eXFS == XFILL_BITMAP )
        {
            OUString aString( pFillAttrLB->GetSelectEntry() );
            pFillAttrLB->Clear();
            pFillAttrLB->Fill( ( (SvxBitmapListItem*)pState )->GetBitmapList() );
            pFillAttrLB->SelectEntry( aString );
        }
    }
}



Window* SvxFillToolBoxControl::CreateItemWindow( Window *pParent )
{
    if ( GetSlotId() == SID_ATTR_FILL_STYLE )
    {
        pFillControl = new FillControl( pParent );
        // Thus the FillControl is known by SvxFillToolBoxControl
        // (and in order to remain compatible)
        pFillControl->SetData( this );

        pFillAttrLB = (SvxFillAttrBox*)pFillControl->pLbFillAttr;
        pFillTypeLB = (SvxFillTypeBox*)pFillControl->pLbFillType;

        pFillAttrLB->SetUniqueId( HID_FILL_ATTR_LISTBOX );
        pFillTypeLB->SetUniqueId( HID_FILL_TYPE_LISTBOX );

        return pFillControl;
    }
    return NULL;
}

/*************************************************************************
|*
|* FillControl
|*
\************************************************************************/

FillControl::FillControl( Window* pParent, WinBits nStyle ) :
    Window( pParent, nStyle | WB_DIALOGCONTROL ),
    pLbFillType(new SvxFillTypeBox( this )),
    aLogicalFillSize(40,80),
    aLogicalAttrSize(50,80)
{
    pLbFillAttr = new SvxFillAttrBox( this );
    Size aTypeSize(LogicToPixel(aLogicalFillSize, MAP_APPFONT));
    Size aAttrSize(LogicToPixel(aLogicalAttrSize, MAP_APPFONT));
    pLbFillType->SetSizePixel(aTypeSize);
    pLbFillAttr->SetSizePixel(aAttrSize);
    //to get the base height
    aTypeSize = pLbFillType->GetSizePixel();
    aAttrSize = pLbFillAttr->GetSizePixel();
    Point aAttrPnt = pLbFillAttr->GetPosPixel();
    SetSizePixel(
        Size( aAttrPnt.X() + aAttrSize.Width(),
              std::max( aAttrSize.Height(), aTypeSize.Height() ) ) );

    pLbFillType->SetSelectHdl( LINK( this, FillControl, SelectFillTypeHdl ) );
    pLbFillAttr->SetSelectHdl( LINK( this, FillControl, SelectFillAttrHdl ) );

    aDelayTimer.SetTimeout( DELAY_TIMEOUT );
    aDelayTimer.SetTimeoutHdl( LINK( this, FillControl, DelayHdl ) );
    aDelayTimer.Start();
}



FillControl::~FillControl()
{
    delete pLbFillType;
    delete pLbFillAttr;
}



IMPL_LINK_NOARG_INLINE_START(FillControl, DelayHdl)
{
    SelectFillTypeHdl( NULL );
    ( (SvxFillToolBoxControl*)GetData() )->updateStatus( OUString( ".uno:FillStyle" ));
//  ( (SvxFillToolBoxControl*)GetData() )->GetBindings().Invalidate( SID_ATTR_FILL_STYLE );
    return 0;
}
IMPL_LINK_INLINE_END( FillControl, DelayHdl, Timer *, pTimer )



IMPL_LINK( FillControl, SelectFillTypeHdl, ListBox *, pBox )
{
    XFillStyle  eXFS = (XFillStyle)pLbFillType->GetSelectEntryPos();

    // Later, an optimization should be accomplished,
    // that the lists or tables can only be deleted and rebuilt
    // when the lists, or tables have actually changed (in the LBs of course).

    if ( ( pBox && !pBox->IsTravelSelect() ) || !pBox )
    {
        // So that we can show a status in the following case:
        // One type was selected but no attribute.
        // The selection has exactly the same attributes as the previous one.
//      SvxFillToolBoxControl* pControlerItem = (SvxFillToolBoxControl*)GetData();
//      if( pControlerItem )
//          pControlerItem->ClearCache();

        pLbFillAttr->Clear();
        SfxObjectShell* pSh = SfxObjectShell::Current();

        switch( eXFS )
        {
            case XFILL_NONE:
            {
                pLbFillType->Selected();
                SelectFillAttrHdl( pBox );
                pLbFillAttr->Disable();
            }
            break;

            case XFILL_SOLID:
            {
                if ( pSh && pSh->GetItem( SID_COLOR_TABLE ) )
                {
                    SvxColorListItem aItem( *(const SvxColorListItem*)(
                        pSh->GetItem( SID_COLOR_TABLE ) ) );
                    pLbFillAttr->Enable();
                    pLbFillAttr->Fill( aItem.GetColorList() );
                }
                else
                    pLbFillAttr->Disable();
            }
            break;

            case XFILL_GRADIENT:
            {
                if ( pSh && pSh->GetItem( SID_GRADIENT_LIST ) )
                {
                    SvxGradientListItem aItem( *(const SvxGradientListItem*)(
                        pSh->GetItem( SID_GRADIENT_LIST ) ) );
                    pLbFillAttr->Enable();
                    pLbFillAttr->Fill( aItem.GetGradientList() );
                }
                else
                    pLbFillAttr->Disable();
            }
            break;

            case XFILL_HATCH:
            {
                if ( pSh && pSh->GetItem( SID_HATCH_LIST ) )
                {
                    SvxHatchListItem aItem( *(const SvxHatchListItem*)(
                        pSh->GetItem( SID_HATCH_LIST ) ) );
                    pLbFillAttr->Enable();
                    pLbFillAttr->Fill( aItem.GetHatchList() );
                }
                else
                    pLbFillAttr->Disable();
            }
            break;

            case XFILL_BITMAP:
            {
                if ( pSh && pSh->GetItem( SID_BITMAP_LIST ) )
                {
                    SvxBitmapListItem aItem( *(const SvxBitmapListItem*)(
                        pSh->GetItem( SID_BITMAP_LIST ) ) );
                    pLbFillAttr->Enable();
                    pLbFillAttr->Fill( aItem.GetBitmapList() );
                }
                else
                    pLbFillAttr->Disable();
            }
            break;
        }

        if( eXFS != XFILL_NONE ) // Has already been done
        {
            if ( pBox )
                pLbFillType->Selected();

            // release focus
            if ( pBox && pLbFillType->IsRelease() )
            {
                SfxViewShell* pViewShell = SfxViewShell::Current();
                if( pViewShell && pViewShell->GetWindow() )
                    pViewShell->GetWindow()->GrabFocus();
            }
        }
    }
    return 0;
}



IMPL_LINK( FillControl, SelectFillAttrHdl, ListBox *, pBox )
{
    XFillStyle eXFS = (XFillStyle)pLbFillType->GetSelectEntryPos();
    XFillStyleItem aXFillStyleItem( eXFS );
    bool bAction = pBox && !pLbFillAttr->IsTravelSelect();

    SfxObjectShell* pSh = SfxObjectShell::Current();
    if ( bAction )
    {
        Any a;
        Sequence< PropertyValue > aArgs( 1 );

        // First set the style
        aArgs[0].Name = "FillStyle";
        aXFillStyleItem.QueryValue(  a );
        aArgs[0].Value = a;
        ((SvxFillToolBoxControl*)GetData())->Dispatch(
            OUString( ".uno:FillStyle" ), aArgs );

        switch( eXFS )
        {
            case XFILL_NONE:
            {
            }
            break;

            case XFILL_SOLID:
            {
                // Entry gets tested against temporary color
                OUString aTmpStr = pLbFillAttr->GetSelectEntry();
                if( aTmpStr.startsWith(TMP_STR_BEGIN) && aTmpStr.endsWith(TMP_STR_END) )
                {
                    aTmpStr = aTmpStr.copy(1, aTmpStr.getLength()-2);
                }

                XFillColorItem aXFillColorItem( aTmpStr, pLbFillAttr->GetSelectEntryColor() );

                aArgs[0].Name = "FillColor";
                aXFillColorItem.QueryValue( a );
                aArgs[0].Value = a;
                ((SvxFillToolBoxControl*)GetData())->Dispatch( ".uno:FillColor",
                                                               aArgs );
            }
            break;
            case XFILL_GRADIENT:
            {
                sal_uInt16 nPos = pLbFillAttr->GetSelectEntryPos();

                if ( nPos != LISTBOX_ENTRY_NOTFOUND && pSh && pSh->GetItem( SID_GRADIENT_LIST ) )
                {
                    SvxGradientListItem aItem(
                        *(const SvxGradientListItem*)( pSh->GetItem( SID_GRADIENT_LIST ) ) );

                    if ( nPos < aItem.GetGradientList()->Count() )  // no temporary entry?
                    {
                        XGradient aGradient = aItem.GetGradientList()->GetGradient( nPos )->GetGradient();
                        XFillGradientItem aXFillGradientItem( pLbFillAttr->GetSelectEntry(), aGradient );

                        aArgs[0].Name = "FillGradient";
                        aXFillGradientItem.QueryValue( a );
                        aArgs[0].Value = a;
                        ((SvxFillToolBoxControl*)GetData())->Dispatch( OUString( ".uno:FillGradient" ),
                                                                       aArgs );
                    }
                }
            }
            break;

            case XFILL_HATCH:
            {
                sal_uInt16 nPos = pLbFillAttr->GetSelectEntryPos();

                if ( nPos != LISTBOX_ENTRY_NOTFOUND && pSh && pSh->GetItem( SID_HATCH_LIST ) )
                {
                    SvxHatchListItem aItem( *(const SvxHatchListItem*)( pSh->GetItem( SID_HATCH_LIST ) ) );

                    if ( nPos < aItem.GetHatchList()->Count() )  // no temporary entry?
                    {
                        XHatch aHatch = aItem.GetHatchList()->GetHatch( nPos )->GetHatch();
                        XFillHatchItem aXFillHatchItem( pLbFillAttr->GetSelectEntry(), aHatch );

                        aArgs[0].Name = "FillHatch";
                        aXFillHatchItem.QueryValue( a );
                        aArgs[0].Value = a;
                        ((SvxFillToolBoxControl*)GetData())->Dispatch( OUString( ".uno:FillHatch" ),
                                                                       aArgs );
                    }
                }
            }
            break;

            case XFILL_BITMAP:
            {
                sal_uInt16 nPos = pLbFillAttr->GetSelectEntryPos();

                if ( nPos != LISTBOX_ENTRY_NOTFOUND && pSh && pSh->GetItem( SID_BITMAP_LIST ) )
                {
                    SvxBitmapListItem aItem(
                        *(const SvxBitmapListItem*)( pSh->GetItem( SID_BITMAP_LIST ) ) );

                    if ( nPos < aItem.GetBitmapList()->Count() )  // no temporary entry?
                    {
                        const XBitmapEntry* pXBitmapEntry = aItem.GetBitmapList()->GetBitmap(nPos);
                        const XFillBitmapItem aXFillBitmapItem(pLbFillAttr->GetSelectEntry(), pXBitmapEntry->GetGraphicObject());

                        aArgs[0].Name = "FillBitmap";
                        aXFillBitmapItem.QueryValue( a );
                        aArgs[0].Value = a;
                        ((SvxFillToolBoxControl*)GetData())->Dispatch(OUString(".uno:FillBitmap"), aArgs);
                    }
                }
            }
            break;
        }

        // release focus
        if ( pLbFillAttr->IsRelease()  && pBox )
        {
            SfxViewShell* pViewShell = SfxViewShell::Current();
            if( pViewShell && pViewShell->GetWindow() )
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

    pLbFillType->SetSizePixel( Size( nW * 2 - nSep, nH ) );
    pLbFillAttr->SetPosSizePixel( Point( nW * 2 + nSep, 0 ), Size( nW * 3 - nSep, nH ) );
}

void FillControl::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        Size aTypeSize(LogicToPixel(aLogicalFillSize, MAP_APPFONT));
        Size aAttrSize(LogicToPixel(aLogicalAttrSize, MAP_APPFONT));
        pLbFillType->SetSizePixel(aTypeSize);
        pLbFillAttr->SetSizePixel(aAttrSize);
        //to get the base height
        aTypeSize = pLbFillType->GetSizePixel();
        aAttrSize = pLbFillAttr->GetSizePixel();
        Point aAttrPnt = pLbFillAttr->GetPosPixel();

        SetSizePixel(
            Size( aAttrPnt.X() + aAttrSize.Width(),
                std::max( aAttrSize.Height(), aTypeSize.Height() ) ) );
    }
    Window::DataChanged( rDCEvt );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
