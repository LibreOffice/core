/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------

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
    bool bEnableControls = false;

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
            if( nSID == SID_ATTR_FILL_STYLE )
            {
                delete pStyleItem;
                pStyleItem = (XFillStyleItem*) pState->Clone();
                pFillTypeLB->Enable();

                eLastXFS = pFillTypeLB->GetSelectEntryPos();
                bUpdate = sal_True;

                XFillStyle eXFS = (XFillStyle)pStyleItem->GetValue();
                pFillTypeLB->SelectEntryPos(
                    sal::static_int_cast< sal_uInt16 >( eXFS ) );
                pFillAttrLB->Enable();
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
                        Bitmap* pBmp = aGradientList.CreateBitmapForUI( 0 );

                        if( pBmp )
                        {
                            ( (ListBox*)pFillAttrLB )->InsertEntry( pEntry->GetName(), *pBmp );
                            pFillAttrLB->SelectEntryPos( pFillAttrLB->GetEntryCount() - 1 );
                            delete pBmp;
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
                        Bitmap* pBmp = aHatchList.CreateBitmapForUI( 0 );

                        if( pBmp )
                        {
                            ( (ListBox*)pFillAttrLB )->InsertEntry( pEntry->GetName(), *pBmp );
                            pFillAttrLB->SelectEntryPos( pFillAttrLB->GetEntryCount() - 1 );
                            delete pBmp;
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

                        XBitmapEntry* pEntry = new XBitmapEntry( pBitmapItem->GetBitmapValue(), aTmpStr );
                        XBitmapList aBitmapList( String::CreateFromAscii("TmpList") );
                        aBitmapList.Insert( pEntry );
                        aBitmapList.SetDirty( sal_False );
                        //Bitmap* pBmp = aBitmapList.GetBitmap( 0 );
                        //( (ListBox*)pFillAttrLB )->InsertEntry( pEntry->GetName(), *pBmp );
                        pFillAttrLB->Fill( &aBitmapList );
                        pFillAttrLB->SelectEntryPos( pFillAttrLB->GetEntryCount() - 1 );
                        aBitmapList.Remove( 0 );
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
        if( pState->ISA( SvxColorTableItem ) &&
            eXFS == XFILL_SOLID )
        {
            ::Color aTmpColor( pFillAttrLB->GetSelectEntryColor() );
            pFillAttrLB->Clear();
            pFillAttrLB->Fill( ( (SvxColorTableItem*)pState )->GetColorTable() );
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
              Max( aAttrSize.Height(), aTypeSize.Height() ) ) );

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

IMPL_LINK_INLINE_START( FillControl, DelayHdl, Timer *, EMPTYARG )
{
    SelectFillTypeHdl( NULL );
    ( (SvxFillToolBoxControl*)GetData() )->updateStatus( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FillStyle" )));
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
                    SvxColorTableItem aItem( *(const SvxColorTableItem*)(
                        pSh->GetItem( SID_COLOR_TABLE ) ) );
                    pLbFillAttr->Enable();
                    pLbFillAttr->Fill( aItem.GetColorTable() );
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
    sal_Bool bAction = pBox && !pLbFillAttr->IsTravelSelect();

    SfxObjectShell* pSh = SfxObjectShell::Current();
    if ( bAction )
    {
        Any a;
        Sequence< PropertyValue > aArgs( 1 );

        // First set the style
        aArgs[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FillStyle" ));
        aXFillStyleItem.QueryValue(  a );
        aArgs[0].Value = a;
        ( (SvxFillToolBoxControl*)GetData() )->IgnoreStatusUpdate( sal_True );
        ((SvxFillToolBoxControl*)GetData())->Dispatch(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FillStyle" )), aArgs );
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

                aArgs[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FillColor" ));
                aXFillColorItem.QueryValue( a );
                aArgs[0].Value = a;
                ((SvxFillToolBoxControl*)GetData())->Dispatch( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FillColor" )),
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

                        aArgs[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FillGradient" ));
                        aXFillGradientItem.QueryValue( a );
                        aArgs[0].Value = a;
                        ((SvxFillToolBoxControl*)GetData())->Dispatch( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FillGradient" )),
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

                        aArgs[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FillHatch" ));
                        aXFillHatchItem.QueryValue( a );
                        aArgs[0].Value = a;
                        ((SvxFillToolBoxControl*)GetData())->Dispatch( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FillHatch" )),
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
                        XOBitmap aXOBitmap = aItem.GetBitmapList()->GetBitmap( nPos )->GetXBitmap();
                        XFillBitmapItem aXFillBitmapItem( pLbFillAttr->GetSelectEntry(), aXOBitmap );

                        aArgs[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FillBitmap" ));
                        aXFillBitmapItem.QueryValue( a );
                        aArgs[0].Value = a;
                        ((SvxFillToolBoxControl*)GetData())->Dispatch( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FillBitmap" )),
                                                                       aArgs );
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
                Max( aAttrSize.Height(), aTypeSize.Height() ) ) );
    }
    Window::DataChanged( rDCEvt );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
