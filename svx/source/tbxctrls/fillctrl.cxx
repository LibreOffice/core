/*************************************************************************
 *
 *  $RCSfile: fillctrl.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 17:05:13 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// include ---------------------------------------------------------------

#include <string> // HACK: prevent conflict between STLPORT and Workshop headers

#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif
#ifndef _SFXVIEWSH_HXX
#include <sfx2/viewsh.hxx>
#endif
#include <rtl/ustring.hxx>

#pragma hdrstop

#include "dialogs.hrc"

#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE
#define ITEMID_GRADIENT_LIST    SID_GRADIENT_LIST
#define ITEMID_HATCH_LIST       SID_HATCH_LIST
#define ITEMID_BITMAP_LIST      SID_BITMAP_LIST

#define DELAY_TIMEOUT           300

#define TMP_STR_BEGIN   '['
#define TMP_STR_END     ']'

#include "drawitem.hxx"
#include "xattr.hxx"
#include "xtable.hxx"
#include "fillctrl.hxx"
#include "itemwin.hxx"
#include "dialmgr.hxx"
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

SvxFillToolBoxControl::SvxFillToolBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx ),

    pStyleItem      ( NULL ),
    pColorItem      ( NULL ),
    pGradientItem   ( NULL ),
    pHatchItem      ( NULL ),
    pBitmapItem     ( NULL ),
    pFillControl    ( NULL ),
    pFillAttrLB     ( NULL ),
    pFillTypeLB     ( NULL ),
    bUpdate         ( FALSE ),
    eLastXFS        ( XFILL_NONE ),
    bIgnoreStatusUpdate( FALSE )
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

    USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )

{
    FASTBOOL bEnableControls = FALSE;

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

                eLastXFS = (XFillStyle) pFillTypeLB->GetSelectEntryPos();
                bUpdate = TRUE;

                XFillStyle eXFS = (XFillStyle)pStyleItem->GetValue();
                pFillTypeLB->SelectEntryPos( eXFS );
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
                        bEnableControls = TRUE;
                }
                else if( nSID == SID_ATTR_FILL_GRADIENT )
                {
                    delete pGradientItem;
                    pGradientItem = (XFillGradientItem*) pState->Clone();

                    if( eXFS == XFILL_GRADIENT )
                        bEnableControls = TRUE;
                }
                else if( nSID == SID_ATTR_FILL_HATCH )
                {
                    delete pHatchItem;
                    pHatchItem = (XFillHatchItem*) pState->Clone();

                    if( eXFS == XFILL_HATCH )
                        bEnableControls = TRUE;
                }
                else if( nSID == SID_ATTR_FILL_BITMAP )
                {
                    delete pBitmapItem;
                    pBitmapItem = (XFillBitmapItem*) pState->Clone();

                    if( eXFS == XFILL_BITMAP )
                        bEnableControls = TRUE;
                }
            }
            if( bEnableControls )
            {
                //pFillTypeLB->Enable();
                pFillAttrLB->Enable();

                bUpdate = TRUE;
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
                bUpdate = FALSE;
            }
            else
            {
                XFillStyle eXFS;
                if( pStyleItem )
                    eXFS = (XFillStyle)pStyleItem->GetValue();
                if( !pStyleItem ||
                    ( nSID == SID_ATTR_FILL_COLOR    && eXFS == XFILL_SOLID ) ||
                    ( nSID == SID_ATTR_FILL_GRADIENT && eXFS == XFILL_GRADIENT ) ||
                    ( nSID == SID_ATTR_FILL_HATCH    && eXFS == XFILL_HATCH ) ||
                    ( nSID == SID_ATTR_FILL_BITMAP   && eXFS == XFILL_BITMAP ) )
                {
                    pFillAttrLB->SetNoSelection();
                    //bUpdate = FALSE;
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
        bUpdate = FALSE;

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
                    Color aColor = pColorItem->GetValue();

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
                        long nCount = pFillAttrLB->GetEntryCount();
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

                        //pFillAttrLB->SetUpdateMode( FALSE );
                        USHORT nPos = pFillAttrLB->InsertEntry( aColor, aTmpStr );
                        //pFillAttrLB->SetUpdateMode( TRUE );
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
                        long nCount = pFillAttrLB->GetEntryCount();
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

                        XGradientEntry* pEntry = new XGradientEntry( pGradientItem->GetValue(), aTmpStr );
                        String aEmptyString = String();
                         XGradientList aGradientList( aEmptyString );
                        aGradientList.Insert( pEntry );
                        aGradientList.SetDirty( FALSE );
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
                        long nCount = pFillAttrLB->GetEntryCount();
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

                        XHatchEntry* pEntry = new XHatchEntry( pHatchItem->GetValue(), aTmpStr );
                        String aEmptyString = String();
                        XHatchList aHatchList( aEmptyString );
                        aHatchList.Insert( pEntry );
                        aHatchList.SetDirty( FALSE );
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
                        long nCount = pFillAttrLB->GetEntryCount();
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

                        XBitmapEntry* pEntry = new XBitmapEntry( pBitmapItem->GetValue(), aTmpStr );
                        XBitmapList aBitmapList( String::CreateFromAscii("TmpList") );
                        aBitmapList.Insert( pEntry );
                        aBitmapList.SetDirty( FALSE );
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
                DBG_ERROR( "Nicht unterstuetzter Flaechentyp" );
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
            Color aTmpColor( pFillAttrLB->GetSelectEntryColor() );
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

IMPL_LINK_INLINE_START( FillControl, DelayHdl, Timer *, pTimer )
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
            if ( pBox && pLbFillType->IsRelease() && SfxViewShell::Current()->GetWindow() )
                SfxViewShell::Current()->GetWindow()->GrabFocus();
        }
    }
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( FillControl, SelectFillAttrHdl, ListBox *, pBox )
{
    XFillStyle eXFS = (XFillStyle)pLbFillType->GetSelectEntryPos();
    XFillStyleItem aXFillStyleItem( eXFS );
    BOOL bAction = pBox && !pLbFillAttr->IsTravelSelect();

    SfxObjectShell* pSh = SfxObjectShell::Current();
//  SfxDispatcher* pDisp = ( (SvxFillToolBoxControl*)GetData() )->GetBindings().GetDispatcher();
//  DBG_ASSERT( pDisp, "invalid Dispatcher" );
    if ( bAction )
    {
        Any a;
        Sequence< PropertyValue > aArgs( 1 );

        // First set the style
        aArgs[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FillStyle" ));
        aXFillStyleItem.QueryValue(  a );
        aArgs[0].Value = a;
        ( (SvxFillToolBoxControl*)GetData() )->IgnoreStatusUpdate( TRUE );
        ((SvxFillToolBoxControl*)GetData())->Dispatch(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FillStyle" )), aArgs );
        ( (SvxFillToolBoxControl*)GetData() )->IgnoreStatusUpdate( FALSE );

        switch( eXFS )
        {
            case XFILL_NONE:
            {
/*
                aArgs[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FillStyle" ));
                aXFillStyleItem.QueryValue( a );
                aArgs[0].Value = a;
                ((SvxFillToolBoxControl*)GetData())->Dispatch( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FillStyle" )),
                                                                aArgs );
*/
//                pDisp->Execute( SID_ATTR_FILL_STYLE, SFX_CALLMODE_RECORD, &aXFillStyleItem, 0L );
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
                // NEU
//                pDisp->Execute(
//                    SID_ATTR_FILL_COLOR, SFX_CALLMODE_RECORD, &aXFillColorItem, &aXFillStyleItem, 0L );
            }
            break;
            case XFILL_GRADIENT:
            {
                USHORT nPos = pLbFillAttr->GetSelectEntryPos();

                if ( nPos != LISTBOX_ENTRY_NOTFOUND && pSh && pSh->GetItem( SID_GRADIENT_LIST ) )
                {
                    SvxGradientListItem aItem(
                        *(const SvxGradientListItem*)( pSh->GetItem( SID_GRADIENT_LIST ) ) );

                    if ( nPos < aItem.GetGradientList()->Count() )  // kein temp. Eintrag ?
                    {
                        XGradient aGradient = aItem.GetGradientList()->Get( nPos )->GetGradient();
                        XFillGradientItem aXFillGradientItem( pLbFillAttr->GetSelectEntry(), aGradient );

                        aArgs[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FillGradient" ));
                        aXFillGradientItem.QueryValue( a );
                        aArgs[0].Value = a;
                        ((SvxFillToolBoxControl*)GetData())->Dispatch( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FillGradient" )),
                                                                       aArgs );
//                        pDisp->Execute( SID_ATTR_FILL_GRADIENT, SFX_CALLMODE_RECORD,
//                                        &aXFillGradientItem, &aXFillStyleItem, 0L );
                    }
                }
            }
            break;

            case XFILL_HATCH:
            {
                USHORT nPos = pLbFillAttr->GetSelectEntryPos();

                if ( nPos != LISTBOX_ENTRY_NOTFOUND && pSh && pSh->GetItem( SID_HATCH_LIST ) )
                {
                    SvxHatchListItem aItem( *(const SvxHatchListItem*)( pSh->GetItem( SID_HATCH_LIST ) ) );

                    if ( nPos < aItem.GetHatchList()->Count() )  // kein temp. Eintrag ?
                    {
                        XHatch aHatch = aItem.GetHatchList()->Get( nPos )->GetHatch();
                        XFillHatchItem aXFillHatchItem( pLbFillAttr->GetSelectEntry(), aHatch );

                        aArgs[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FillHatch" ));
                        aXFillHatchItem.QueryValue( a );
                        aArgs[0].Value = a;
                        ((SvxFillToolBoxControl*)GetData())->Dispatch( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FillHatch" )),
                                                                       aArgs );
//                        pDisp->Execute( SID_ATTR_FILL_HATCH, SFX_CALLMODE_RECORD,
//                                        &aXFillHatchItem, &aXFillStyleItem, 0L );
                    }
                }
            }
            break;

            case XFILL_BITMAP:
            {
                USHORT nPos = pLbFillAttr->GetSelectEntryPos();

                if ( nPos != LISTBOX_ENTRY_NOTFOUND && pSh && pSh->GetItem( SID_BITMAP_LIST ) )
                {
                    SvxBitmapListItem aItem(
                        *(const SvxBitmapListItem*)( pSh->GetItem( SID_BITMAP_LIST ) ) );

                    if ( nPos < aItem.GetBitmapList()->Count() )  // kein temp. Eintrag ?
                    {
                        XOBitmap aXOBitmap = aItem.GetBitmapList()->Get( nPos )->GetXBitmap();
                        XFillBitmapItem aXFillBitmapItem( pLbFillAttr->GetSelectEntry(), aXOBitmap );

                        aArgs[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FillBitmap" ));
                        aXFillBitmapItem.QueryValue( a );
                        aArgs[0].Value = a;
                        ((SvxFillToolBoxControl*)GetData())->Dispatch( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FillBitmap" )),
                                                                       aArgs );
//                        pDisp->Execute( SID_ATTR_FILL_BITMAP, SFX_CALLMODE_RECORD,
//                                        &aXFillBitmapItem, &aXFillStyleItem, 0L );
                    }
                }
            }
            break;
        }

        // release focus
        if ( pLbFillAttr->IsRelease()  && pBox && SfxViewShell::Current()->GetWindow() )
            SfxViewShell::Current()->GetWindow()->GrabFocus();
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
/* -----------------------------08.03.2002 15:04------------------------------

 ---------------------------------------------------------------------------*/

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

