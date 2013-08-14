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

#include <svx/dialogs.hrc>

#define DELAY_TIMEOUT           300

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
    bIgnoreStatusUpdate( sal_False ),
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

//========================================================================

SvxFillToolBoxControl::~SvxFillToolBoxControl()
{
    delete pStyleItem;
    delete pColorItem;
    delete pGradientItem;
    delete pHatchItem;
    delete pBitmapItem;
}

//========================================================================

void SvxFillToolBoxControl::StateChanged(

    sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )

{

    if ( bIgnoreStatusUpdate )
        return;

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
            // leerer oder uneindeutiger Status
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

//========================================================================

void SvxFillToolBoxControl::IgnoreStatusUpdate( sal_Bool bSet )
{
    bIgnoreStatusUpdate = bSet;
}

//========================================================================

void SvxFillToolBoxControl::Update( const SfxPoolItem* pState )
{
    if ( pStyleItem && pState && bUpdate )
    {
        bUpdate = sal_False;

        XFillStyle eXFS = (XFillStyle)pStyleItem->GetValue();

        // Pruefen, ob Fuellstil schon vorher aktiv war
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
                    String aString( pColorItem->GetName() );
                    ::Color aColor = pColorItem->GetColorValue();

                    pFillAttrLB->SelectEntry( aString );

                    if ( pFillAttrLB->GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND ||
                         pFillAttrLB->GetSelectEntryColor() != aColor )
                        pFillAttrLB->SelectEntry( aColor );

                    // NEU
                    // Pruefen, ob Eintrag nicht in der Liste ist
                    if( pFillAttrLB->GetSelectEntryPos() ==
                        LISTBOX_ENTRY_NOTFOUND ||
                        pFillAttrLB->GetSelectEntryColor() != aColor )
                    {
                        sal_uInt16 nCount = pFillAttrLB->GetEntryCount();
                        String aTmpStr;
                        if( nCount > 0 )
                        {
                            //Letzter Eintrag wird auf temporaere Farbe geprueft
                            aTmpStr = pFillAttrLB->GetEntry( nCount - 1 );
                            if(  aTmpStr.GetChar(0) == TMP_STR_BEGIN &&
                                 aTmpStr.GetChar(aTmpStr.Len()-1) == TMP_STR_END )
                            {
                                pFillAttrLB->RemoveEntry( nCount - 1 );
                            }
                        }
                        aTmpStr = TMP_STR_BEGIN;
                        aTmpStr += aString;
                        aTmpStr += TMP_STR_END;

                        //pFillAttrLB->SetUpdateMode( sal_False );
                        sal_uInt16 nPos = pFillAttrLB->InsertEntry( aColor, aTmpStr );
                        //pFillAttrLB->SetUpdateMode( sal_True );
                        pFillAttrLB->SelectEntryPos( nPos );
                    }
                    // NEU
                }
                else
                    pFillAttrLB->SetNoSelection();
            }
            break;

            case XFILL_GRADIENT:
            {
                if ( pGradientItem )
                {
                    String aString( pGradientItem->GetName() );
                    pFillAttrLB->SelectEntry( aString );
                    // NEU
                    // Pruefen, ob Eintrag nicht in der Liste ist
                    if( pFillAttrLB->GetSelectEntry() != aString )
                    {
                        sal_uInt16 nCount = pFillAttrLB->GetEntryCount();
                        String aTmpStr;
                        if( nCount > 0 )
                        {
                            //Letzter Eintrag wird auf temporaeren Eintrag geprueft
                            aTmpStr = pFillAttrLB->GetEntry( nCount - 1 );
                            if(  aTmpStr.GetChar(0) == TMP_STR_BEGIN &&
                                 aTmpStr.GetChar(aTmpStr.Len()-1) == TMP_STR_END )
                            {
                                pFillAttrLB->RemoveEntry( nCount - 1 );
                            }
                        }
                        aTmpStr = TMP_STR_BEGIN;
                        aTmpStr += aString;
                        aTmpStr += TMP_STR_END;

                        XGradientEntry* pEntry = new XGradientEntry( pGradientItem->GetGradientValue(), aTmpStr );
                        String aEmptyString = String();
                         XGradientList aGradientList( aEmptyString );
                        aGradientList.Insert( pEntry );
                        aGradientList.SetDirty( sal_False );
                        const Bitmap aBmp = aGradientList.GetUiBitmap( 0 );

                        if( !aBmp.IsEmpty() )
                        {
                            ( (ListBox*)pFillAttrLB )->InsertEntry( pEntry->GetName(), aBmp );
                            pFillAttrLB->SelectEntryPos( pFillAttrLB->GetEntryCount() - 1 );
                            //delete pBmp;
                        }

                        aGradientList.Remove( 0 );
                        delete pEntry;
                    }
                    // NEU
                }
                else
                    pFillAttrLB->SetNoSelection();
            }
            break;

            case XFILL_HATCH:
            {
                if ( pHatchItem )
                {
                    String aString( pHatchItem->GetName() );
                    pFillAttrLB->SelectEntry( aString );
                    // NEU
                    // Pruefen, ob Eintrag nicht in der Liste ist
                    if( pFillAttrLB->GetSelectEntry() != aString )
                    {
                        sal_uInt16 nCount = pFillAttrLB->GetEntryCount();
                        String aTmpStr;
                        if( nCount > 0 )
                        {
                            //Letzter Eintrag wird auf temporaeren Eintrag geprueft
                            aTmpStr = pFillAttrLB->GetEntry( nCount - 1 );
                            if(  aTmpStr.GetChar(0) == TMP_STR_BEGIN &&
                                 aTmpStr.GetChar(aTmpStr.Len()-1) == TMP_STR_END )
                            {
                                pFillAttrLB->RemoveEntry( nCount - 1 );
                            }
                        }
                        aTmpStr = TMP_STR_BEGIN;
                        aTmpStr += aString;
                        aTmpStr += TMP_STR_END;

                        XHatchEntry* pEntry = new XHatchEntry( pHatchItem->GetHatchValue(), aTmpStr );
                        String aEmptyString = String();
                        XHatchList aHatchList( aEmptyString );
                        aHatchList.Insert( pEntry );
                        aHatchList.SetDirty( sal_False );
                        const Bitmap aBmp = aHatchList.GetUiBitmap( 0 );

                        if( !aBmp.IsEmpty() )
                        {
                            ( (ListBox*)pFillAttrLB )->InsertEntry( pEntry->GetName(), aBmp );
                            pFillAttrLB->SelectEntryPos( pFillAttrLB->GetEntryCount() - 1 );
                            //delete pBmp;
                        }

                        aHatchList.Remove( 0 );
                        delete pEntry;
                    }
                    // NEU
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
                    String aString( pBitmapItem->GetName() );
                    // Bitmap aBitmap( pBitmapItem->GetValue() );

                    // SvxBitmapListItem aItem( *(const SvxBitmapListItem*)(
                    //  SfxObjectShell::Current()->GetItem( SID_BITMAP_LIST ) ) );
                    pFillAttrLB->SelectEntry( aString );
                    // NEU
                    // Pruefen, ob Eintrag nicht in der Liste ist
                    if( pFillAttrLB->GetSelectEntry() != aString )
                    {
                        sal_uInt16 nCount = pFillAttrLB->GetEntryCount();
                        String aTmpStr;
                        if( nCount > 0 )
                        {
                            //Letzter Eintrag wird auf temporaeren Eintrag geprueft
                            aTmpStr = pFillAttrLB->GetEntry( nCount - 1 );
                            if(  aTmpStr.GetChar(0) == TMP_STR_BEGIN &&
                                 aTmpStr.GetChar(aTmpStr.Len()-1) == TMP_STR_END )
                            {
                                pFillAttrLB->RemoveEntry( nCount - 1 );
                            }
                        }
                        aTmpStr = TMP_STR_BEGIN;
                        aTmpStr += aString;
                        aTmpStr += TMP_STR_END;

                        XBitmapEntry* pEntry = new XBitmapEntry(pBitmapItem->GetGraphicObject(), aTmpStr);
                        XBitmapListRef xBitmapList =
                            XPropertyList::CreatePropertyList(XBITMAP_LIST,
                            OUString("TmpList"))->AsBitmapList();
                        xBitmapList->Insert( pEntry );
                        xBitmapList->SetDirty( sal_False );
                        pFillAttrLB->Fill( xBitmapList );
                        pFillAttrLB->SelectEntryPos( pFillAttrLB->GetEntryCount() - 1 );
                        xBitmapList->Remove( 0 );
                        delete pEntry;
                    }
                    // NEU
                }
                else
                    pFillAttrLB->SetNoSelection();
            }
            break;

            default:
                OSL_FAIL( "Nicht unterstuetzter Flaechentyp" );
            break;
        }
    }

    if( pState && pStyleItem )
    {
        XFillStyle eXFS = (XFillStyle) pStyleItem->GetValue();

        // Die Listen haben sich geaendert ?
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
            String aString( pFillAttrLB->GetSelectEntry() );
            pFillAttrLB->Clear();
            pFillAttrLB->Fill( ( (SvxGradientListItem*)pState )->GetGradientList() );
            pFillAttrLB->SelectEntry( aString );
        }
        if( pState->ISA( SvxHatchListItem ) &&
            eXFS == XFILL_HATCH )
        {
            String aString( pFillAttrLB->GetSelectEntry() );
            pFillAttrLB->Clear();
            pFillAttrLB->Fill( ( (SvxHatchListItem*)pState )->GetHatchList() );
            pFillAttrLB->SelectEntry( aString );
        }
        if( pState->ISA( SvxBitmapListItem ) &&
            eXFS == XFILL_BITMAP )
        {
            String aString( pFillAttrLB->GetSelectEntry() );
            pFillAttrLB->Clear();
            pFillAttrLB->Fill( ( (SvxBitmapListItem*)pState )->GetBitmapList() );
            pFillAttrLB->SelectEntry( aString );
        }
    }
}

//========================================================================

Window* SvxFillToolBoxControl::CreateItemWindow( Window *pParent )
{
    if ( GetSlotId() == SID_ATTR_FILL_STYLE )
    {
        pFillControl = new FillControl( pParent );
        // Damit dem FillControl das SvxFillToolBoxControl bekannt ist
        // (und um kompatibel zu bleiben)
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

//------------------------------------------------------------------------

FillControl::~FillControl()
{
    delete pLbFillType;
    delete pLbFillAttr;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG_INLINE_START(FillControl, DelayHdl)
{
    SelectFillTypeHdl( NULL );
    ( (SvxFillToolBoxControl*)GetData() )->updateStatus( OUString( ".uno:FillStyle" ));
//  ( (SvxFillToolBoxControl*)GetData() )->GetBindings().Invalidate( SID_ATTR_FILL_STYLE );
    return 0;
}
IMPL_LINK_INLINE_END( FillControl, DelayHdl, Timer *, pTimer )

//------------------------------------------------------------------------

IMPL_LINK( FillControl, SelectFillTypeHdl, ListBox *, pBox )
{
    XFillStyle  eXFS = (XFillStyle)pLbFillType->GetSelectEntryPos();

    // Spaeter sollte eine Optimierung derart erfolgen, dass die
    // Listen, bzw. Tables nur dann geloescht und wieder aufgebaut
    // werden, wenn sich die Listen, bzw. Tables tatsaechlich geaendert
    // haben (in den LBs natuerlich).

    if ( ( pBox && !pBox->IsTravelSelect() ) || !pBox )
    {
        // Damit wir in folgendem Fall einen Status anzeigen koennen:
        // Ein Typ wurde ausgewaehlt aber kein Attribut.
        // Die Selektion hat genau die gleichen Attribute wie die vorherige.
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

        if( eXFS != XFILL_NONE ) // Wurde schon erledigt
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

//------------------------------------------------------------------------

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
        aArgs[0].Name = OUString( "FillStyle" );
        aXFillStyleItem.QueryValue(  a );
        aArgs[0].Value = a;
        ( (SvxFillToolBoxControl*)GetData() )->IgnoreStatusUpdate( sal_True );
        ((SvxFillToolBoxControl*)GetData())->Dispatch(
            OUString( ".uno:FillStyle" ), aArgs );
        ( (SvxFillToolBoxControl*)GetData() )->IgnoreStatusUpdate( sal_False );

        switch( eXFS )
        {
            case XFILL_NONE:
            {
            }
            break;

            case XFILL_SOLID:
            {
                // NEU
                //Eintrag wird auf temporaere Farbe geprueft
                String aTmpStr = pLbFillAttr->GetSelectEntry();
                if( aTmpStr.GetChar(0) == TMP_STR_BEGIN && aTmpStr.GetChar(aTmpStr.Len()-1) == TMP_STR_END )
                {
                    aTmpStr.Erase( aTmpStr.Len()-1, 1 );
                    aTmpStr.Erase( 0, 1 );
                }

                XFillColorItem aXFillColorItem( aTmpStr, pLbFillAttr->GetSelectEntryColor() );

                aArgs[0].Name = OUString( "FillColor" );
                aXFillColorItem.QueryValue( a );
                aArgs[0].Value = a;
                ((SvxFillToolBoxControl*)GetData())->Dispatch( OUString( ".uno:FillColor" ),
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

                    if ( nPos < aItem.GetGradientList()->Count() )  // kein temp. Eintrag ?
                    {
                        XGradient aGradient = aItem.GetGradientList()->GetGradient( nPos )->GetGradient();
                        XFillGradientItem aXFillGradientItem( pLbFillAttr->GetSelectEntry(), aGradient );

                        aArgs[0].Name = OUString( "FillGradient" );
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

                    if ( nPos < aItem.GetHatchList()->Count() )  // kein temp. Eintrag ?
                    {
                        XHatch aHatch = aItem.GetHatchList()->GetHatch( nPos )->GetHatch();
                        XFillHatchItem aXFillHatchItem( pLbFillAttr->GetSelectEntry(), aHatch );

                        aArgs[0].Name = OUString( "FillHatch" );
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

                    if ( nPos < aItem.GetBitmapList()->Count() )  // kein temp. Eintrag ?
                    {
                        const XBitmapEntry* pXBitmapEntry = aItem.GetBitmapList()->GetBitmap(nPos);
                        const XFillBitmapItem aXFillBitmapItem(pLbFillAttr->GetSelectEntry(), pXBitmapEntry->GetGraphicObject());

                        aArgs[0].Name = OUString( "FillBitmap" );
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

//------------------------------------------------------------------------

void FillControl::Resize()
{
    // Breite der beiden ListBoxen nicht 1/2 : 1/2, sondern 2/5 : 3/5
    long nW = GetOutputSizePixel().Width() / 5;
    long nH = 180;
    long nSep = 0; // war vorher 4

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
