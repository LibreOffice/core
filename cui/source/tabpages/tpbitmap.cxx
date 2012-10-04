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

#include <vcl/wrkwin.hxx>
#include <tools/shl.hxx>
#include <vcl/msgbox.hxx>
#include <tools/urlobj.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/app.hxx>
#include <sfx2/filedlghelper.hxx>
#include <unotools/localfilehelper.hxx>
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#include <svx/dialmgr.hxx>
#include <vcl/bmpacc.hxx>
#include <svx/dialogs.hrc>

#define _SVX_TPBITMAP_CXX

#include <cuires.hrc>
#include "helpid.hrc"
#include "svx/xattr.hxx"
#include <svx/xpool.hxx>
#include <svx/xtable.hxx>
#include "svx/xoutbmp.hxx"
#include "svx/drawitem.hxx"
#include "cuitabarea.hxx"
#include "tabarea.hrc"
#include "defdlgname.hxx"
#include <svx/svxdlg.hxx>
#include <dialmgr.hxx>
#include "sfx2/opengrf.hxx"
#include "paragrph.hrc"

SvxBitmapTabPage::SvxBitmapTabPage
(
    Window* pParent,
    const SfxItemSet& rInAttrs
) :

    SvxTabPage          ( pParent, CUI_RES( RID_SVXPAGE_BITMAP ), rInAttrs ),

    aCtlPixel           ( this, CUI_RES( CTL_PIXEL ) ),
    aFtPixelEdit        ( this, CUI_RES( FT_PIXEL_EDIT ) ),
    aFtColor            ( this, CUI_RES( FT_COLOR ) ),
    aLbColor            ( this, CUI_RES( LB_COLOR ) ),
    aFtBackgroundColor  ( this, CUI_RES( FT_BACKGROUND_COLOR ) ),
    aLbBackgroundColor  ( this, CUI_RES( LB_BACKGROUND_COLOR ) ),
    // This fix text is used only to provide the name for the following
    // bitmap list box.  The fixed text is not displayed.
    aLbBitmapsHidden    ( this, CUI_RES( FT_BITMAPS_HIDDEN ) ),
    aLbBitmaps          ( this, CUI_RES( LB_BITMAPS ) ),
    aFlProp             ( this, CUI_RES( FL_PROP ) ),
    aCtlPreview         ( this, CUI_RES( CTL_PREVIEW ) ),
    aBtnAdd             ( this, CUI_RES( BTN_ADD ) ),
    aBtnModify          ( this, CUI_RES( BTN_MODIFY ) ),
    aBtnImport          ( this, CUI_RES( BTN_IMPORT ) ),
    aBtnDelete          ( this, CUI_RES( BTN_DELETE ) ),
    aBtnLoad            ( this, CUI_RES( BTN_LOAD ) ),
    aBtnSave            ( this, CUI_RES( BTN_SAVE ) ),

    aBitmapCtl          ( this, aCtlPreview.GetSizePixel() ),
    rOutAttrs           ( rInAttrs ),

    pXPool              ( (XOutdevItemPool*) rInAttrs.GetPool() ),
    aXFStyleItem        ( XFILL_BITMAP ),
    aXBitmapItem        ( String(), XOBitmap() ),
    aXFillAttr          ( pXPool ),
    rXFSet              ( aXFillAttr.GetItemSet() )
{
    FreeResource();

    // this page needs ExchangeSupport
    SetExchangeSupport();

    // setting the output device
    rXFSet.Put( aXFStyleItem );
    rXFSet.Put( aXBitmapItem );

    aBtnAdd.SetClickHdl( LINK( this, SvxBitmapTabPage, ClickAddHdl_Impl ) );
    aBtnImport.SetClickHdl(
        LINK( this, SvxBitmapTabPage, ClickImportHdl_Impl ) );
    aBtnModify.SetClickHdl(
        LINK( this, SvxBitmapTabPage, ClickModifyHdl_Impl ) );
    aBtnDelete.SetClickHdl(
        LINK( this, SvxBitmapTabPage, ClickDeleteHdl_Impl ) );
    aBtnLoad.SetClickHdl( LINK( this, SvxBitmapTabPage, ClickLoadHdl_Impl ) );
    aBtnSave.SetClickHdl( LINK( this, SvxBitmapTabPage, ClickSaveHdl_Impl ) );

    aLbBitmaps.SetSelectHdl(
        LINK( this, SvxBitmapTabPage, ChangeBitmapHdl_Impl ) );
    aLbColor.SetSelectHdl(
        LINK( this, SvxBitmapTabPage, ChangePixelColorHdl_Impl ) );
    aLbBackgroundColor.SetSelectHdl(
        LINK( this, SvxBitmapTabPage, ChangeBackgrndColorHdl_Impl ) );

    String accName = String(CUI_RES(STR_EXAMPLE));
    aCtlPreview.SetAccessibleName(accName);
    aCtlPixel.SetAccessibleRelationMemberOf( &aFlProp );
    aCtlPixel.SetAccessibleRelationLabeledBy( &aFtPixelEdit );
    aLbBitmaps.SetAccessibleRelationLabeledBy(&aLbBitmaps);
    aBtnAdd.SetAccessibleRelationMemberOf( &aFlProp );
    aBtnModify.SetAccessibleRelationMemberOf( &aFlProp );
    aBtnImport.SetAccessibleRelationMemberOf( &aFlProp );
    aBtnDelete.SetAccessibleRelationMemberOf( &aFlProp );

}

// -----------------------------------------------------------------------

void SvxBitmapTabPage::Construct()
{
    aLbColor.Fill( pColorList );
    aLbBackgroundColor.CopyEntries( aLbColor );

    aLbBitmaps.Fill( pBitmapList );
}

// -----------------------------------------------------------------------

void SvxBitmapTabPage::ActivatePage( const SfxItemSet&  )
{
    sal_uInt16 nPos;
    sal_uInt16 nCount;

    if( *pDlgType == 0 ) // area dialog
    {
        *pbAreaTP = sal_False;

        if( pColorList.is() )
        {
            // ColorList
            if( *pnColorListState & CT_CHANGED ||
                *pnColorListState & CT_MODIFIED )
            {
                if( *pnColorListState & CT_CHANGED )
                    pColorList = ( (SvxAreaTabDialog*) GetParentDialog() )->GetNewColorList();

                // LbColor
                nPos = aLbColor.GetSelectEntryPos();
                aLbColor.Clear();
                aLbColor.Fill( pColorList );
                nCount = aLbColor.GetEntryCount();
                if( nCount == 0 )
                    ; // this case should not occur
                else if( nCount <= nPos )
                    aLbColor.SelectEntryPos( 0 );
                else
                    aLbColor.SelectEntryPos( nPos );

                // LbColorBackground
                nPos = aLbBackgroundColor.GetSelectEntryPos();
                aLbBackgroundColor.Clear();
                aLbBackgroundColor.CopyEntries( aLbColor );
                nCount = aLbBackgroundColor.GetEntryCount();
                if( nCount == 0 )
                    ; // this case should not occur
                else if( nCount <= nPos )
                    aLbBackgroundColor.SelectEntryPos( 0 );
                else
                    aLbBackgroundColor.SelectEntryPos( nPos );

                ChangePixelColorHdl_Impl( this );
                ChangeBackgrndColorHdl_Impl( this );
            }

            // determining (possibly cutting) the name and
            // displaying it in the GroupBox
            String          aString( CUI_RES( RID_SVXSTR_TABLE ) ); aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );
            INetURLObject   aURL( pBitmapList->GetPath() );

            aURL.Append( pBitmapList->GetName() );
            DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

            if( aURL.getBase().getLength() > 18 )
            {
                aString += String(aURL.getBase()).Copy( 0, 15 );
                aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
            }
            else
                aString += String(aURL.getBase());

            if( *pPageType == PT_BITMAP && *pPos != LISTBOX_ENTRY_NOTFOUND )
            {
                aLbBitmaps.SelectEntryPos( *pPos );
            }
            // colors could have been deleted
            ChangeBitmapHdl_Impl( this );

            *pPageType = PT_BITMAP;
            *pPos = LISTBOX_ENTRY_NOTFOUND;
        }
    }
}

// -----------------------------------------------------------------------

int SvxBitmapTabPage::DeactivatePage( SfxItemSet* _pSet)
{
    if ( CheckChanges_Impl() == -1L )
        return KEEP_PAGE;

    if( _pSet )
        FillItemSet( *_pSet );

    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

sal_Bool SvxBitmapTabPage::FillItemSet( SfxItemSet& _rOutAttrs )
{
    if( *pDlgType == 0 && *pbAreaTP == sal_False ) // area dialog
    {
        if( *pPageType == PT_BITMAP )
        {
            // CheckChanges_Impl(); <-- duplicate inquiry?

            XOBitmap aXOBitmap;
            String aString;
            sal_uInt16 nPos = aLbBitmaps.GetSelectEntryPos();
            if( nPos != LISTBOX_ENTRY_NOTFOUND )
            {
                aXOBitmap = pBitmapList->GetBitmap( nPos )->GetXBitmap();
                aString = aLbBitmaps.GetSelectEntry();

            }
            else
            {
                aXOBitmap = aBitmapCtl.GetXBitmap();

                // #85339# if it's an array, force conversion to bitmap before using it.
                if(aXOBitmap.GetBitmapType() == XBITMAP_8X8)
                    aXOBitmap.GetBitmap();

            }
            _rOutAttrs.Put( XFillStyleItem( XFILL_BITMAP ) );
            _rOutAttrs.Put( XFillBitmapItem( aString, aXOBitmap ) );
        }
    }
    return sal_True;
}

// -----------------------------------------------------------------------

void SvxBitmapTabPage::Reset( const SfxItemSet&  )
{
    // aLbBitmaps.SelectEntryPos( 0 );

    aBitmapCtl.SetLines( aCtlPixel.GetLineCount() );
    aBitmapCtl.SetPixelColor( aLbColor.GetSelectEntryColor() );
    aBitmapCtl.SetBackgroundColor( aLbBackgroundColor.GetSelectEntryColor() );
    aBitmapCtl.SetBmpArray( aCtlPixel.GetBitmapPixelPtr() );

    // get bitmap and display it
    XFillBitmapItem aBmpItem( (const String &) String(), aBitmapCtl.GetXBitmap() );
    rXFSet.Put( aBmpItem );
    aCtlPreview.SetAttributes( aXFillAttr.GetItemSet() );
    aCtlPreview.Invalidate();

    ChangeBitmapHdl_Impl( this );

    // determine button state
    if( pBitmapList->Count() )
    {
        aBtnAdd.Enable();
        aBtnModify.Enable();
        aBtnDelete.Enable();
        aBtnSave.Enable();
    }
    else
    {
        aBtnModify.Disable();
        aBtnDelete.Disable();
        aBtnSave.Disable();
    }
}

// -----------------------------------------------------------------------

SfxTabPage* SvxBitmapTabPage::Create( Window* pWindow,
                const SfxItemSet& rSet )
{
    return new SvxBitmapTabPage( pWindow, rSet );
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxBitmapTabPage, ChangeBitmapHdl_Impl)
{
    XOBitmap* pXOBitmap = NULL;
    int nPos = aLbBitmaps.GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
        pXOBitmap = new XOBitmap( ( (XBitmapEntry*) pBitmapList->GetBitmap( nPos ) )->GetXBitmap() );
    else
    {
        const SfxPoolItem* pPoolItem = NULL;
        if( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLSTYLE ), sal_True, &pPoolItem ) )
        {
            XFillStyle eXFS = (XFillStyle) ( ( const XFillStyleItem* ) pPoolItem )->GetValue();
            if( ( XFILL_BITMAP == eXFS ) &&
                ( SFX_ITEM_SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLBITMAP ), sal_True, &pPoolItem ) ) )
            {
                pXOBitmap = new XOBitmap( ( ( const XFillBitmapItem* ) pPoolItem )->GetBitmapValue() );
            }
        }
        if( !pXOBitmap )
        {
            aLbBitmaps.SelectEntryPos( 0 );
            nPos = aLbBitmaps.GetSelectEntryPos();
            if( nPos != LISTBOX_ENTRY_NOTFOUND )
                pXOBitmap = new XOBitmap( ( (XBitmapEntry*) pBitmapList->GetBitmap( nPos ) )->GetXBitmap() );
        }
    }
    if( pXOBitmap )
    {
        // #85339# try to convert bitmapped item to array item.
        if(pXOBitmap->GetBitmapType() == XBITMAP_IMPORT)
        {
            Bitmap aBitmap(pXOBitmap->GetBitmap());
            Size aSizePixel(aBitmap.GetSizePixel());

            if(8 == aSizePixel.Width() && 8 == aSizePixel.Height())
            {
                sal_uInt16* pPixelArray = new sal_uInt16[64];
                sal_uInt32 nCol1(0xffffffff); // background
                sal_uInt32 nCol2(0xffffffff); // pixel
                BitmapReadAccess* pAccess = aBitmap.AcquireReadAccess();
                sal_Bool bValid(sal_True);

                if(pAccess)
                {
                    for(sal_uInt32 a(0); bValid && a < 64; a++)
                    {
                        const BitmapColor aBmCol = pAccess->GetColor(a>>3, a%8);
                        Color aRgbCol(aBmCol.GetRed(), aBmCol.GetGreen(), aBmCol.GetBlue());
                        sal_uInt32 nColVal = aRgbCol.GetRGBColor();

                        // test with nCol1
                        if(nCol1 != nColVal)
                        {
                            if(0xffffffff == nCol1)
                            {
                                // nCol1 is used first time
                                nCol1 = nColVal;
                                pPixelArray[a] = 0;
                            }
                            else
                            {
                                // test with nCol2
                                if(nCol2 != nColVal)
                                {
                                    if(0xffffffff == nCol2)
                                    {
                                        // nCol2 used first time
                                        nCol2 = nColVal;
                                        pPixelArray[a] = 1;
                                    }
                                    else
                                    {
                                        // Third color detected
                                        bValid = sal_False;
                                    }
                                }
                                else
                                {
                                    // color is pixel color
                                    pPixelArray[a] = 1;
                                }
                            }
                        }
                        else
                        {
                            // color is background color
                            pPixelArray[a] = 0;
                        }
                    }

                    // release ReadAccess
                    aBitmap.ReleaseAccess(pAccess);
                }
                else
                {
                    // no access -> no success
                    bValid = sal_False;
                }

                if(bValid)
                {
                    Color aCol1(nCol1);
                    Color aCol2(nCol2);

                    // no pixel color found? Use opposite od background color.
                    if(0xffffffff == nCol2)
                    {
                        aCol2 = Color(
                            0xff - aCol1.GetRed(),
                            0xff - aCol1.GetGreen(),
                            0xff - aCol1.GetBlue());
                    }

                    // transformation did work, create a new Item
                    delete pXOBitmap;
                    pXOBitmap = new XOBitmap(pPixelArray, aCol2, aCol1);
                }

                // cleanup
                delete[] pPixelArray;
            }
        }

        aLbColor.SetNoSelection();
        aLbBackgroundColor.SetNoSelection();

        if( pXOBitmap->GetBitmapType() == XBITMAP_IMPORT )
        {
            aCtlPixel.Reset();
            aCtlPixel.SetPaintable( sal_False );
            aCtlPixel.Disable();
            aFtPixelEdit.Disable();
            aFtColor.Disable();
            aLbColor.Disable();
            aFtBackgroundColor.Disable();
            aLbBackgroundColor.Disable();
            aBtnModify.Disable();
            aBtnAdd.Disable();
        }
        else if( pXOBitmap->GetBitmapType() == XBITMAP_8X8 )
        {
            aCtlPixel.SetPaintable( sal_True );
            aCtlPixel.Enable();
            aFtPixelEdit.Enable();
            aFtColor.Enable();
            aLbColor.Enable();
            aFtBackgroundColor.Enable();
            aLbBackgroundColor.Enable();
            aBtnModify.Enable();
            aBtnAdd.Enable();

            // setting the pixel control
            aCtlPixel.SetXBitmap( *pXOBitmap );

            Color aPixelColor = pXOBitmap->GetPixelColor();
            Color aBackColor  = pXOBitmap->GetBackgroundColor();

            aBitmapCtl.SetPixelColor( aPixelColor );
            aBitmapCtl.SetBackgroundColor( aBackColor );

            // if the entry is not in the listbox,
            // the color is added temporarily
            if( 0 == aLbBitmaps.GetSelectEntryPos() )
            {
                aLbColor.SelectEntry( Color( COL_BLACK ) );
                ChangePixelColorHdl_Impl( this );
            }
            else
                aLbColor.SelectEntry( aPixelColor );
            if( aLbColor.GetSelectEntryCount() == 0 )
            {
                aLbColor.InsertEntry( aPixelColor, String() );
                aLbColor.SelectEntry( aPixelColor );
            }
            aLbBackgroundColor.SelectEntry( aBackColor );
            if( aLbBackgroundColor.GetSelectEntryCount() == 0 )
            {
                aLbBackgroundColor.InsertEntry( aBackColor, String() );
                aLbBackgroundColor.SelectEntry( aBackColor );
            }
        }
        aCtlPixel.Invalidate();

        // display bitmap
        XFillBitmapItem aXBmpItem( (const String &) String(), *pXOBitmap );
        rXFSet.Put( aXBmpItem );

        aCtlPreview.SetAttributes( aXFillAttr.GetItemSet() );
        aCtlPreview.Invalidate();

        bBmpChanged = sal_False;
        delete pXOBitmap;
    }
    return 0L;
}

// -----------------------------------------------------------------------

long SvxBitmapTabPage::CheckChanges_Impl()
{
    sal_uInt16 nPos = aLbBitmaps.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        String aString = aLbBitmaps.GetSelectEntry();

        if( bBmpChanged )
        {
            ResMgr& rMgr = CUI_MGR();
            Image aWarningBoxImage = WarningBox::GetStandardImage();
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            DBG_ASSERT(pFact, "Dialogdiet fail!");
            AbstractSvxMessDialog* aMessDlg = pFact->CreateSvxMessDialog( GetParentDialog(), RID_SVXDLG_MESSBOX,
                                                        String( SVX_RES( RID_SVXSTR_BITMAP ) ),
                                                        String( CUI_RES( RID_SVXSTR_ASK_CHANGE_BITMAP ) ),
                                                        &aWarningBoxImage  );
            DBG_ASSERT(aMessDlg, "Dialogdiet fail!");
            aMessDlg->SetButtonText( MESS_BTN_1,
                                    String( ResId( RID_SVXSTR_CHANGE, rMgr ) ) );
            aMessDlg->SetButtonText( MESS_BTN_2,
                                    String( ResId( RID_SVXSTR_ADD, rMgr ) ) );

            short nRet = aMessDlg->Execute();

            switch( nRet )
            {
                case RET_BTN_1:
                {
                    ClickModifyHdl_Impl( this );
                }
                break;

                case RET_BTN_2:
                {
                    ClickAddHdl_Impl( this );
                    nPos = aLbBitmaps.GetSelectEntryPos();
                }
                break;

                case RET_CANCEL:
                break;
            }
            delete aMessDlg;
        }
    }
    nPos = aLbBitmaps.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
        *pPos = nPos;
    return 0L;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxBitmapTabPage, ClickAddHdl_Impl)
{
    ResMgr& rMgr = CUI_MGR();
    String aNewName( SVX_RES( RID_SVXSTR_BITMAP ) );
    String aDesc( CUI_RES( RID_SVXSTR_DESC_NEW_BITMAP ) );
    String aName;

    long nCount = pBitmapList->Count();
    long j = 1;
    sal_Bool bDifferent = sal_False;

    while( !bDifferent )
    {
        aName  = aNewName;
        aName += sal_Unicode(' ');
        aName += UniString::CreateFromInt32( j++ );
        bDifferent = sal_True;

        for( long i = 0; i < nCount && bDifferent; i++ )
            if( aName == pBitmapList->GetBitmap( i )->GetName() )
                bDifferent = sal_False;
    }

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialogdiet fail!");
    AbstractSvxNameDialog* pDlg = pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc );
    DBG_ASSERT(pDlg, "Dialogdiet fail!");
    WarningBox*    pWarnBox = NULL;
    sal_uInt16         nError   = RID_SVXSTR_WARN_NAME_DUPLICATE;

    while( pDlg->Execute() == RET_OK )
    {
        pDlg->GetName( aName );

        bDifferent = sal_True;

        for( long i = 0; i < nCount && bDifferent; i++ )
            if( aName == pBitmapList->GetBitmap( i )->GetName() )
                bDifferent = sal_False;

        if( bDifferent ) {
            nError = 0;
            break;
        }

        if( !pWarnBox )
        {
            pWarnBox = new WarningBox( GetParentDialog(),
                                       WinBits( WB_OK_CANCEL ),
                                       String( ResId( nError, rMgr ) ) );
            pWarnBox->SetHelpId( HID_WARN_NAME_DUPLICATE );
        }

        if( pWarnBox->Execute() != RET_OK )
            break;
    }
    delete pDlg;
    delete pWarnBox;

    if( !nError )
    {
        XBitmapEntry* pEntry = 0;
        if( aCtlPixel.IsEnabled() )
        {
            XOBitmap aXOBitmap = aBitmapCtl.GetXBitmap();

            // #85339# if it's an array, force conversion to bitmap before using it.
            if(aXOBitmap.GetBitmapType() == XBITMAP_8X8)
                aXOBitmap.GetBitmap();

            pEntry = new XBitmapEntry( aXOBitmap, aName );
        }
        else // it must be a not existing imported bitmap
        {
            const SfxPoolItem* pPoolItem = NULL;
            if( SFX_ITEM_SET == rOutAttrs.GetItemState( XATTR_FILLBITMAP, sal_True, &pPoolItem ) )
            {
                XOBitmap aXOBitmap( ( ( const XFillBitmapItem* ) pPoolItem )->GetBitmapValue() );
                pEntry = new XBitmapEntry( aXOBitmap, aName );
            }
        }

        DBG_ASSERT( pEntry, "SvxBitmapTabPage::ClickAddHdl_Impl(), pEntry == 0 ?" );

        if( pEntry )
        {
            pBitmapList->Insert( pEntry );

            aLbBitmaps.Append( pEntry );
            aLbBitmaps.SelectEntryPos( aLbBitmaps.GetEntryCount() - 1 );

#ifdef WNT
            // hack: #31355# W.P.
            Rectangle aRect( aLbBitmaps.GetPosPixel(), aLbBitmaps.GetSizePixel() );
            if( sal_True ) {                // ??? overlapped with pDlg
                                        // and srolling
                Invalidate( aRect );
                //aLbBitmaps.Invalidate();
            }
#endif

            *pnBitmapListState |= CT_MODIFIED;

            ChangeBitmapHdl_Impl( this );
        }
    }

    // determine button state
    if( pBitmapList->Count() )
    {
        aBtnModify.Enable();
        aBtnDelete.Enable();
        aBtnSave.Enable();
    }
    return 0L;
}


/******************************************************************************/
/******************************************************************************/


//------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma optimize ( "", off )
#endif

IMPL_LINK_NOARG(SvxBitmapTabPage, ClickImportHdl_Impl)
{
    ResMgr& rMgr = CUI_MGR();
    SvxOpenGraphicDialog aDlg(rtl::OUString("Import"));
    aDlg.EnableLink(sal_False);

    if( !aDlg.Execute() )
    {
        Graphic         aGraphic;
        int             nError = 1;

        EnterWait();
        nError = aDlg.GetGraphic( aGraphic );
        LeaveWait();

        if( !nError )
        {
            String aDesc( ResId(RID_SVXSTR_DESC_EXT_BITMAP, rMgr) );
            WarningBox*    pWarnBox = NULL;

            // convert file URL to UI name
            String          aName;
            INetURLObject   aURL( aDlg.GetPath() );
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            DBG_ASSERT(pFact, "Dialogdiet fail!");
            AbstractSvxNameDialog* pDlg = pFact->CreateSvxNameDialog( GetParentDialog(), String(aURL.GetName()).GetToken( 0, '.' ), aDesc );
            DBG_ASSERT(pDlg, "Dialogdiet fail!");
            nError = RID_SVXSTR_WARN_NAME_DUPLICATE;

            while( pDlg->Execute() == RET_OK )
            {
                pDlg->GetName( aName );

                sal_Bool bDifferent = sal_True;
                long nCount     = pBitmapList->Count();

                for( long i = 0; i < nCount && bDifferent; i++ )
                    if( aName == pBitmapList->GetBitmap( i )->GetName() )
                        bDifferent = sal_False;

                if( bDifferent ) {
                    nError = 0;
                    break;
                }

                if( !pWarnBox )
                {
                    pWarnBox = new WarningBox( GetParentDialog(),
                                               WinBits( WB_OK_CANCEL ),
                                               String( ResId( nError, rMgr ) ) );
                    pWarnBox->SetHelpId( HID_WARN_NAME_DUPLICATE );
                }


                if( pWarnBox->Execute() != RET_OK )
                    break;
            }
            delete pDlg;
            delete pWarnBox;

            if( !nError )
            {
                Bitmap aBmp( aGraphic.GetBitmap() );
                XBitmapEntry* pEntry =
                    new XBitmapEntry( XOBitmap( aBmp ), aName );
                pBitmapList->Insert( pEntry );

                aLbBitmaps.Append( pEntry );
                aLbBitmaps.SelectEntryPos( aLbBitmaps.GetEntryCount() - 1 );

#ifdef WNT
                // hack: #31355# W.P.
                Rectangle aRect( aLbBitmaps.GetPosPixel(), aLbBitmaps.GetSizePixel() );
                if( sal_True ) {                // ??? overlapped with pDlg
                                            // and srolling
                    Invalidate( aRect );
                }
#endif

                *pnBitmapListState |= CT_MODIFIED;

                ChangeBitmapHdl_Impl( this );
            }
        }
        else
            // graphic couldn't be loaded
            ErrorBox( GetParentDialog(),
                      WinBits( WB_OK ),
                      String( ResId( RID_SVXSTR_READ_DATA_ERROR, rMgr ) ) ).Execute();
    }

    return 0L;
}

#ifdef _MSC_VER
#pragma optimize ( "", on )
#endif

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxBitmapTabPage, ClickModifyHdl_Impl)
{
    sal_uInt16 nPos = aLbBitmaps.GetSelectEntryPos();

    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        ResMgr& rMgr = CUI_MGR();
        String aNewName( SVX_RES( RID_SVXSTR_BITMAP ) );
        String aDesc( ResId( RID_SVXSTR_DESC_NEW_BITMAP, rMgr ) );
        String aName( pBitmapList->GetBitmap( nPos )->GetName() );
        String aOldName = aName;

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialogdiet fail!");
        AbstractSvxNameDialog* pDlg = pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc );
        DBG_ASSERT(pDlg, "Dialogdiet fail!");

        long nCount = pBitmapList->Count();
        sal_Bool bDifferent = sal_False;
        sal_Bool bLoop = sal_True;

        while( bLoop && pDlg->Execute() == RET_OK )
        {
            pDlg->GetName( aName );
            bDifferent = sal_True;

            for( long i = 0; i < nCount && bDifferent; i++ )
            {
                if( aName == pBitmapList->GetBitmap( i )->GetName() &&
                    aName != aOldName )
                    bDifferent = sal_False;
            }

            if( bDifferent )
            {
                bLoop = sal_False;
                XBitmapEntry* pEntry = pBitmapList->GetBitmap( nPos );

                pEntry->SetName( aName );

                XOBitmap aXOBitmap = aBitmapCtl.GetXBitmap();

                // #85339# if it's an array, force conversion to bitmap before using it.
                if(aXOBitmap.GetBitmapType() == XBITMAP_8X8)
                    aXOBitmap.GetBitmap();

                pEntry->SetXBitmap( aXOBitmap );

                aLbBitmaps.Modify( pEntry, nPos );
                aLbBitmaps.SelectEntryPos( nPos );

                *pnBitmapListState |= CT_MODIFIED;

                bBmpChanged = sal_False;
            }
            else
            {
                WarningBox aBox( GetParentDialog(), WinBits( WB_OK ), String( ResId( RID_SVXSTR_WARN_NAME_DUPLICATE, rMgr ) ) );
                aBox.SetHelpId( HID_WARN_NAME_DUPLICATE );
                aBox.Execute();
            }
        }
        delete pDlg;
    }
    return 0L;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxBitmapTabPage, ClickDeleteHdl_Impl)
{
    sal_uInt16 nPos = aLbBitmaps.GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        QueryBox aQueryBox( GetParentDialog(), WinBits( WB_YES_NO | WB_DEF_NO ),
            String( CUI_RES( RID_SVXSTR_ASK_DEL_BITMAP ) ) );

        if( aQueryBox.Execute() == RET_YES )
        {
            delete pBitmapList->Remove( nPos );
            aLbBitmaps.RemoveEntry( nPos );
            aLbBitmaps.SelectEntryPos( 0 );

            aCtlPreview.Invalidate();
            aCtlPixel.Invalidate();

            ChangeBitmapHdl_Impl( this );

            *pnBitmapListState |= CT_MODIFIED;
        }
    }
    // determine button state
    if( !pBitmapList->Count() )
    {
        aBtnModify.Disable();
        aBtnDelete.Disable();
        aBtnSave.Disable();
    }
    return 0L;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxBitmapTabPage, ClickLoadHdl_Impl)
{
    sal_uInt16 nReturn = RET_YES;
    ResMgr& rMgr = CUI_MGR();

    if ( *pnBitmapListState & CT_MODIFIED )
    {
        nReturn = WarningBox( GetParentDialog(), WinBits( WB_YES_NO_CANCEL ),
            String( ResId( RID_SVXSTR_WARN_TABLE_OVERWRITE, rMgr ) ) ).Execute();

        if ( nReturn == RET_YES )
            pBitmapList->Save();
    }

    if ( nReturn != RET_CANCEL )
    {
        ::sfx2::FileDialogHelper aDlg(
            com::sun::star::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
            0 );
        String aStrFilterType( RTL_CONSTASCII_USTRINGPARAM( "*.sob" ) );
        aDlg.AddFilter( aStrFilterType, aStrFilterType );
        INetURLObject aFile( SvtPathOptions().GetPalettePath() );
        aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );

        if ( aDlg.Execute() == ERRCODE_NONE )
        {
            EnterWait();
            INetURLObject aURL( aDlg.GetPath() );
            INetURLObject aPathURL( aURL );

            aPathURL.removeSegment();
            aPathURL.removeFinalSlash();

            // save table
            XBitmapListRef pBmpList = XPropertyList::CreatePropertyList(
                XBITMAP_LIST, aPathURL.GetMainURL( INetURLObject::NO_DECODE ), pXPool )->AsBitmapList();
            pBmpList->SetName( aURL.getName() );
            if( pBmpList->Load() )
            {
                pBitmapList = pBmpList;
                ( (SvxAreaTabDialog*) GetParentDialog() )->SetNewBitmapList( pBitmapList );

                aLbBitmaps.Clear();
                aLbBitmaps.Fill( pBitmapList );
                Reset( rOutAttrs );

                pBitmapList->SetName( aURL.getName() );

                // determining (possibly cutting) the name
                // displaying it in the GroupBox
                String aString( ResId( RID_SVXSTR_TABLE, rMgr ) );
                aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );

                if ( aURL.getBase().getLength() > 18 )
                {
                    aString += String(aURL.getBase()).Copy( 0, 15 );
                    aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
                }
                else
                    aString += String(aURL.getBase());

                *pnBitmapListState |= CT_CHANGED;
                *pnBitmapListState &= ~CT_MODIFIED;
                LeaveWait();
            }
            else
            {
                LeaveWait();
                ErrorBox( GetParentDialog(), WinBits( WB_OK ),
                    String( ResId( RID_SVXSTR_READ_DATA_ERROR, rMgr ) ) ).Execute();
            }
        }
    }

    // determine button state
    if( pBitmapList->Count() )
    {
        aBtnModify.Enable();
        aBtnDelete.Enable();
        aBtnSave.Enable();
    }
    else
    {
        aBtnModify.Disable();
        aBtnDelete.Disable();
        aBtnSave.Disable();
    }
    return 0L;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxBitmapTabPage, ClickSaveHdl_Impl)
{
       ::sfx2::FileDialogHelper aDlg(
        com::sun::star::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE, 0 );
    String aStrFilterType( RTL_CONSTASCII_USTRINGPARAM( "*.sob" ) );
    aDlg.AddFilter( aStrFilterType, aStrFilterType );

    INetURLObject aFile( SvtPathOptions().GetPalettePath() );
    DBG_ASSERT( aFile.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    if( pBitmapList->GetName().Len() )
    {
        aFile.Append( pBitmapList->GetName() );

        if( aFile.getExtension().isEmpty() )
            aFile.SetExtension(rtl::OUString("sob"));
    }

    aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );
    if ( aDlg.Execute() == ERRCODE_NONE )
    {
        INetURLObject   aURL( aDlg.GetPath() );
        INetURLObject   aPathURL( aURL );

        aPathURL.removeSegment();
        aPathURL.removeFinalSlash();

        pBitmapList->SetName( aURL.getName() );
        pBitmapList->SetPath( aPathURL.GetMainURL( INetURLObject::NO_DECODE ) );

        if( pBitmapList->Save() )
        {
            // determining (possibly cutting) the name
            // displaying it in the GroupBox
            String aString( CUI_RES( RID_SVXSTR_TABLE ) );
            aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ": " ) );

            if ( aURL.getBase().getLength() > 18 )
            {
                aString += String(aURL.getBase()).Copy( 0, 15 );
                aString.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "..." ) );
            }
            else
                aString += String(aURL.getBase());

            *pnBitmapListState |= CT_SAVED;
            *pnBitmapListState &= ~CT_MODIFIED;
        }
        else
        {
            ErrorBox( GetParentDialog(), WinBits( WB_OK ),
                String( CUI_RES( RID_SVXSTR_WRITE_DATA_ERROR ) ) ).Execute();
        }
    }

    return 0L;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxBitmapTabPage, ChangePixelColorHdl_Impl)
{
    aCtlPixel.SetPixelColor( aLbColor.GetSelectEntryColor() );
    aCtlPixel.Invalidate();

    aBitmapCtl.SetPixelColor( aLbColor.GetSelectEntryColor() );

    // get bitmap and display it
    rXFSet.Put( XFillBitmapItem( String(), aBitmapCtl.GetXBitmap() ) );
    aCtlPreview.SetAttributes( aXFillAttr.GetItemSet() );
    aCtlPreview.Invalidate();

    bBmpChanged = sal_True;

    return 0L;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxBitmapTabPage, ChangeBackgrndColorHdl_Impl)
{
    aCtlPixel.SetBackgroundColor( aLbBackgroundColor.GetSelectEntryColor() );
    aCtlPixel.Invalidate();

    aBitmapCtl.SetBackgroundColor( aLbBackgroundColor.GetSelectEntryColor() );

    // get bitmap and display it
    rXFSet.Put( XFillBitmapItem( String(), aBitmapCtl.GetXBitmap() ) );
    aCtlPreview.SetAttributes( aXFillAttr.GetItemSet() );
    aCtlPreview.Invalidate();

    bBmpChanged = sal_True;

    return 0L;
}

//------------------------------------------------------------------------

void SvxBitmapTabPage::PointChanged( Window* pWindow, RECT_POINT )
{
    if( pWindow == &aCtlPixel )
    {
        aBitmapCtl.SetBmpArray( aCtlPixel.GetBitmapPixelPtr() );

        // get bitmap and display it
        rXFSet.Put( XFillBitmapItem( String(), aBitmapCtl.GetXBitmap() ) );
        aCtlPreview.SetAttributes( aXFillAttr.GetItemSet() );
        aCtlPreview.Invalidate();

        bBmpChanged = sal_True;
    }
}




Window* SvxBitmapTabPage::GetParentLabeledBy( const Window* pLabeled ) const
{
    if (pLabeled == &aLbBitmaps)
        return const_cast<FixedText*>(&aLbBitmapsHidden);
    else
        return SvxTabPage::GetParentLabeledBy (pLabeled);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
