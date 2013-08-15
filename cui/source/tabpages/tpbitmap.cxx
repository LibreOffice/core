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
#include <sfx2/dialoghelper.hxx>
#include <sfx2/filedlghelper.hxx>
#include <unotools/localfilehelper.hxx>
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#include <svx/dialmgr.hxx>
#include <vcl/bmpacc.hxx>
#include <svx/dialogs.hrc>

#include <cuires.hrc>
#include "helpid.hrc"
#include "svx/xattr.hxx"
#include <svx/xpool.hxx>
#include <svx/xtable.hxx>
#include "svx/xoutbmp.hxx"
#include "svx/drawitem.hxx"
#include "cuitabarea.hxx"
#include "defdlgname.hxx"
#include "dlgname.hxx"
#include <svx/svxdlg.hxx>
#include <dialmgr.hxx>
#include "sfx2/opengrf.hxx"
#include "paragrph.hrc"

SvxBitmapTabPage::SvxBitmapTabPage(  Window* pParent, const SfxItemSet& rInAttrs) :

    SvxTabPage          ( pParent,
                          "BitmapTabPage",
                          "cui/ui/bitmaptabpage.ui",
                          rInAttrs ),
    rOutAttrs           ( rInAttrs ),

    pnBitmapListState   ( 0 ),
    pnColorListState    ( 0 ),
    pPageType           ( 0 ),
    pDlgType            ( 0 ),
    pbAreaTP            ( 0 ),

    bBmpChanged         ( false ),

    pXPool              ( (XOutdevItemPool*) rInAttrs.GetPool() ),
    aXFStyleItem        ( XFILL_BITMAP ),
    aXBitmapItem        ( String(), Graphic() ),
    aXFillAttr          ( pXPool ),
    rXFSet              ( aXFillAttr.GetItemSet() )
{
    get(m_pBxPixelEditor,"maingrid");
    get(m_pCtlPixel,"CTL_PIXEL");
    get(m_pLbColor,"LB_COLOR");
    get(m_pLbBackgroundColor,"LB_BACKGROUND_COLOR");
    get(m_pCtlPreview,"CTL_PREVIEW");
    get(m_pLbBitmaps,"LB_BITMAPS");
    get(m_pLbBitmapsHidden,"FT_BITMAPS_HIDDEN");
    get(m_pBtnAdd,"BTN_ADD");
    get(m_pBtnModify,"BTN_MODIFY");
    get(m_pBtnImport,"BTN_IMPORT");
    get(m_pBtnDelete,"BTN_DELETE");
    get(m_pBtnLoad,"BTN_LOAD");
    get(m_pBtnSave,"BTN_SAVE");

    // size of the bitmap listbox
    Size aSize = getDrawListBoxOptimalSize(this);
    m_pLbBitmaps->set_width_request(aSize.Width());
    m_pLbBitmaps->set_height_request(aSize.Height());

    // size of the bitmap display
    Size aSize2 = getDrawPreviewOptimalSize(this);
    m_pCtlPreview->set_width_request(aSize2.Width());
    m_pCtlPreview->set_height_request(aSize2.Height());

    m_pBitmapCtl = new SvxBitmapCtl(this, aSize2);

    // this page needs ExchangeSupport
    SetExchangeSupport();

    // setting the output device
    rXFSet.Put( aXFStyleItem );
    rXFSet.Put( aXBitmapItem );

    m_pBtnAdd->SetClickHdl( LINK( this, SvxBitmapTabPage, ClickAddHdl_Impl ) );
    m_pBtnImport->SetClickHdl( LINK( this, SvxBitmapTabPage, ClickImportHdl_Impl ) );
    m_pBtnModify->SetClickHdl( LINK( this, SvxBitmapTabPage, ClickModifyHdl_Impl ) );
    m_pBtnDelete->SetClickHdl( LINK( this, SvxBitmapTabPage, ClickDeleteHdl_Impl ) );
    m_pBtnLoad->SetClickHdl( LINK( this, SvxBitmapTabPage, ClickLoadHdl_Impl ) );
    m_pBtnSave->SetClickHdl( LINK( this, SvxBitmapTabPage, ClickSaveHdl_Impl ) );

    m_pLbBitmaps->SetSelectHdl( LINK( this, SvxBitmapTabPage, ChangeBitmapHdl_Impl ) );
    m_pLbColor->SetSelectHdl( LINK( this, SvxBitmapTabPage, ChangePixelColorHdl_Impl ) );
    m_pLbBackgroundColor->SetSelectHdl( LINK( this, SvxBitmapTabPage, ChangeBackgrndColorHdl_Impl ) );

    setPreviewsToSamePlace(pParent, this);
}

SvxBitmapTabPage::~SvxBitmapTabPage()
{
    delete m_pBitmapCtl;
}

// -----------------------------------------------------------------------

void SvxBitmapTabPage::Construct()
{
    m_pLbColor->Fill( pColorList );
    m_pLbBackgroundColor->CopyEntries( *m_pLbColor );

    m_pLbBitmaps->Fill( pBitmapList );
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
                nPos = m_pLbColor->GetSelectEntryPos();
                m_pLbColor->Clear();
                m_pLbColor->Fill( pColorList );
                nCount = m_pLbColor->GetEntryCount();
                if( nCount == 0 )
                    ; // this case should not occur
                else if( nCount <= nPos )
                    m_pLbColor->SelectEntryPos( 0 );
                else
                    m_pLbColor->SelectEntryPos( nPos );

                // LbColorBackground
                nPos = m_pLbBackgroundColor->GetSelectEntryPos();
                m_pLbBackgroundColor->Clear();
                m_pLbBackgroundColor->CopyEntries( *m_pLbColor );
                nCount = m_pLbBackgroundColor->GetEntryCount();
                if( nCount == 0 )
                    ; // this case should not occur
                else if( nCount <= nPos )
                    m_pLbBackgroundColor->SelectEntryPos( 0 );
                else
                    m_pLbBackgroundColor->SelectEntryPos( nPos );

                ChangePixelColorHdl_Impl( this );
                ChangeBackgrndColorHdl_Impl( this );
            }

            // determining (possibly cutting) the name and
            // displaying it in the GroupBox
            OUString        aString( CUI_RES( RID_SVXSTR_TABLE ) );
            aString         += ": ";
            INetURLObject   aURL( pBitmapList->GetPath() );

            aURL.Append( pBitmapList->GetName() );
            DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

            if( aURL.getBase().getLength() > 18 )
            {
                aString += aURL.getBase().copy( 0, 15 );
                aString += "...";
            }
            else
                aString += aURL.getBase();

            if( *pPageType == PT_BITMAP && *pPos != LISTBOX_ENTRY_NOTFOUND )
            {
                m_pLbBitmaps->SelectEntryPos( *pPos );
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
        if(PT_BITMAP == *pPageType)
        {
            _rOutAttrs.Put(XFillStyleItem(XFILL_BITMAP));
            sal_uInt16 nPos = m_pLbBitmaps->GetSelectEntryPos();
            if(LISTBOX_ENTRY_NOTFOUND != nPos)
            {
                const XBitmapEntry* pXBitmapEntry = pBitmapList->GetBitmap(nPos);
                const String aString(m_pLbBitmaps->GetSelectEntry());

                _rOutAttrs.Put(XFillBitmapItem(aString, pXBitmapEntry->GetGraphicObject()));
            }
            else
            {
                const BitmapEx aBitmapEx(m_pBitmapCtl->GetBitmapEx());

                _rOutAttrs.Put(XFillBitmapItem(String(), Graphic(aBitmapEx)));
            }
        }
    }

    return sal_True;
}

// -----------------------------------------------------------------------

void SvxBitmapTabPage::Reset( const SfxItemSet&  )
{
    // aLbBitmaps.SelectEntryPos( 0 );

    m_pBitmapCtl->SetLines( m_pCtlPixel->GetLineCount() );
    m_pBitmapCtl->SetPixelColor( m_pLbColor->GetSelectEntryColor() );
    m_pBitmapCtl->SetBackgroundColor( m_pLbBackgroundColor->GetSelectEntryColor() );
    m_pBitmapCtl->SetBmpArray( m_pCtlPixel->GetBitmapPixelPtr() );

    // get bitmap and display it
    const XFillBitmapItem aBmpItem(OUString(), Graphic(m_pBitmapCtl->GetBitmapEx()));
    rXFSet.Put( aBmpItem );
    m_pCtlPreview->SetAttributes( aXFillAttr.GetItemSet() );
    m_pCtlPreview->Invalidate();

    ChangeBitmapHdl_Impl( this );

    // determine button state
    if( pBitmapList.is() && pBitmapList->Count() )
    {
        m_pBtnAdd->Enable();
        m_pBtnModify->Enable();
        m_pBtnDelete->Enable();
        m_pBtnSave->Enable();
    }
    else
    {
        m_pBtnModify->Disable();
        m_pBtnDelete->Disable();
        m_pBtnSave->Disable();
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
    GraphicObject* pGraphicObject = 0;
    int nPos(m_pLbBitmaps->GetSelectEntryPos());

    if(LISTBOX_ENTRY_NOTFOUND != nPos)
    {
        pGraphicObject = new GraphicObject(pBitmapList->GetBitmap(nPos)->GetGraphicObject());
    }
    else
    {
        const SfxPoolItem* pPoolItem = 0;

        if(SFX_ITEM_SET == rOutAttrs.GetItemState(GetWhich(XATTR_FILLSTYLE), true, &pPoolItem))
        {
            const XFillStyle eXFS((XFillStyle)((const XFillStyleItem*)pPoolItem)->GetValue());

            if((XFILL_BITMAP == eXFS) && (SFX_ITEM_SET == rOutAttrs.GetItemState(GetWhich(XATTR_FILLBITMAP), true, &pPoolItem)))
            {
                pGraphicObject = new GraphicObject(((const XFillBitmapItem*)pPoolItem)->GetGraphicObject());
            }
        }

        if(!pGraphicObject)
        {
            m_pLbBitmaps->SelectEntryPos(0);
            nPos = m_pLbBitmaps->GetSelectEntryPos();

            if(LISTBOX_ENTRY_NOTFOUND != nPos)
            {
                pGraphicObject = new GraphicObject(pBitmapList->GetBitmap(nPos)->GetGraphicObject());
            }
        }
    }

    if(pGraphicObject)
    {
        BitmapColor aBack;
        BitmapColor aFront;
        bool bIs8x8(isHistorical8x8(pGraphicObject->GetGraphic().GetBitmap(), aBack, aFront));

        m_pLbColor->SetNoSelection();
        m_pLbBackgroundColor->SetNoSelection();

        if(bIs8x8)
        {
            m_pCtlPixel->SetPaintable( sal_True );
            m_pBxPixelEditor->Enable();
            m_pBtnModify->Enable();
            m_pBtnAdd->Enable();

            // setting the pixel control

            m_pCtlPixel->SetXBitmap(pGraphicObject->GetGraphic().GetBitmapEx());

            Color aPixelColor = aFront;
            Color aBackColor = aBack;

            m_pBitmapCtl->SetPixelColor( aPixelColor );
            m_pBitmapCtl->SetBackgroundColor( aBackColor );

            // if the entry is not in the listbox,
            // the color is added temporarily
            if( 0 == m_pLbBitmaps->GetSelectEntryPos() )
            {
                m_pLbColor->SelectEntry( Color( COL_BLACK ) );
                ChangePixelColorHdl_Impl( this );
            }
            else
                m_pLbColor->SelectEntry( aPixelColor );
            if( m_pLbColor->GetSelectEntryCount() == 0 )
            {
                m_pLbColor->InsertEntry( aPixelColor, String() );
                m_pLbColor->SelectEntry( aPixelColor );
            }
            m_pLbBackgroundColor->SelectEntry( aBackColor );
            if( m_pLbBackgroundColor->GetSelectEntryCount() == 0 )
            {
                m_pLbBackgroundColor->InsertEntry( aBackColor, String() );
                m_pLbBackgroundColor->SelectEntry( aBackColor );
            }
        }
        else
        {
            m_pCtlPixel->Reset();
            m_pCtlPixel->SetPaintable( sal_False );
            m_pBxPixelEditor->Disable();
            m_pBtnModify->Disable();
            m_pBtnAdd->Disable();
        }

        m_pCtlPixel->Invalidate();

        // display bitmap
        const XFillBitmapItem aXBmpItem(String(), *pGraphicObject);
        rXFSet.Put( aXBmpItem );

        m_pCtlPreview->SetAttributes( aXFillAttr.GetItemSet() );
        m_pCtlPreview->Invalidate();

        bBmpChanged = sal_False;
        delete pGraphicObject;
    }

    return 0;
}

// -----------------------------------------------------------------------

long SvxBitmapTabPage::CheckChanges_Impl()
{
    sal_uInt16 nPos = m_pLbBitmaps->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        if( bBmpChanged )
        {
            ResMgr& rMgr = CUI_MGR();
            Image aWarningBoxImage = WarningBox::GetStandardImage();
            SvxMessDialog* aMessDlg = new SvxMessDialog(GetParentDialog(),
                                                        String( SVX_RES( RID_SVXSTR_BITMAP ) ),
                                                        String( CUI_RES( RID_SVXSTR_ASK_CHANGE_BITMAP ) ),
                                                        &aWarningBoxImage  );
            DBG_ASSERT(aMessDlg, "Dialogdiet fail!");
            aMessDlg->SetButtonText( MESS_BTN_1, String( ResId( RID_SVXSTR_CHANGE, rMgr ) ) );
            aMessDlg->SetButtonText( MESS_BTN_2, String( ResId( RID_SVXSTR_ADD, rMgr ) ) );

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
                    nPos = m_pLbBitmaps->GetSelectEntryPos();
                }
                break;

                case RET_CANCEL:
                break;
            }
            delete aMessDlg;
        }
    }
    nPos = m_pLbBitmaps->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
        *pPos = nPos;
    return 0L;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxBitmapTabPage, ClickAddHdl_Impl)
{

    String aNewName( SVX_RES( RID_SVXSTR_BITMAP ) );
    String aDesc( CUI_RES( RID_SVXSTR_DESC_NEW_BITMAP ) );
    OUString aName;

    long nCount = pBitmapList->Count();
    long j = 1;
    sal_Bool bDifferent = sal_False;

    while( !bDifferent )
    {
        aName  = aNewName;
        aName += " ";
        aName += OUString::number( j++ );
        bDifferent = sal_True;

        for( long i = 0; i < nCount && bDifferent; i++ )
            if( aName == pBitmapList->GetBitmap( i )->GetName() )
                bDifferent = sal_False;
    }

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialogdiet fail!");
    AbstractSvxNameDialog* pDlg = pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc );
    DBG_ASSERT(pDlg, "Dialogdiet fail!");
    MessageDialog*    pWarnBox = NULL;
    sal_uInt16         nError(1);

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
            pWarnBox = new MessageDialog( GetParentDialog()
                                        ,"DuplicateNameDialog"
                                        ,"cui/ui/queryduplicatedialog.ui");
        }

        if( pWarnBox->Execute() != RET_OK )
            break;
    }

    delete pDlg;
    delete pWarnBox;

    if( !nError )
    {
        XBitmapEntry* pEntry = 0;
        if( m_pCtlPixel->IsEnabled() )
        {
            const BitmapEx aBitmapEx(m_pBitmapCtl->GetBitmapEx());

            pEntry = new XBitmapEntry(Graphic(aBitmapEx), aName);
        }
        else // it must be a not existing imported bitmap
        {
            const SfxPoolItem* pPoolItem = 0;

            if(SFX_ITEM_SET == rOutAttrs.GetItemState(XATTR_FILLBITMAP, true, &pPoolItem))
            {
                pEntry = new XBitmapEntry(dynamic_cast< const XFillBitmapItem* >(pPoolItem)->GetGraphicObject(), aName);
            }
        }

        DBG_ASSERT( pEntry, "SvxBitmapTabPage::ClickAddHdl_Impl(), pEntry == 0 ?" );

        if( pEntry )
        {
            pBitmapList->Insert( pEntry );
            const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
            m_pLbBitmaps->Append(rStyleSettings.GetListBoxPreviewDefaultPixelSize(), *pEntry );
            m_pLbBitmaps->SelectEntryPos( m_pLbBitmaps->GetEntryCount() - 1 );

            *pnBitmapListState |= CT_MODIFIED;

            ChangeBitmapHdl_Impl( this );
        }
    }

    // determine button state
    if( pBitmapList->Count() )
    {
        m_pBtnModify->Enable();
        m_pBtnDelete->Enable();
        m_pBtnSave->Enable();
    }
    return 0L;
}


/******************************************************************************/
/******************************************************************************/


//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxBitmapTabPage, ClickImportHdl_Impl)
{
    ResMgr& rMgr = CUI_MGR();
    SvxOpenGraphicDialog aDlg( OUString("Import") );
    aDlg.EnableLink(sal_False);

    if( !aDlg.Execute() )
    {
        Graphic         aGraphic;

        EnterWait();
        int nError = aDlg.GetGraphic( aGraphic );
        LeaveWait();

        if( !nError )
        {
            String aDesc( ResId(RID_SVXSTR_DESC_EXT_BITMAP, rMgr) );
            MessageDialog*    pWarnBox = NULL;

            // convert file URL to UI name
            OUString        aName;
            INetURLObject   aURL( aDlg.GetPath() );
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            DBG_ASSERT(pFact, "Dialogdiet fail!");
            AbstractSvxNameDialog* pDlg = pFact->CreateSvxNameDialog( GetParentDialog(), String(aURL.GetName()).GetToken( 0, '.' ), aDesc );
            DBG_ASSERT(pDlg, "Dialogdiet fail!");
            nError = 1;

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
                    pWarnBox = new MessageDialog( GetParentDialog()
                                                 ,"DuplicateNameDialog"
                                                 ,"cui/ui/queryduplicatedialog.ui");
                }

                if( pWarnBox->Execute() != RET_OK )
                    break;
            }

            delete pDlg;
            delete pWarnBox;

            if( !nError )
            {
                XBitmapEntry* pEntry = new XBitmapEntry( aGraphic, aName );
                pBitmapList->Insert( pEntry );

                const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
                m_pLbBitmaps->Append(rStyleSettings.GetListBoxPreviewDefaultPixelSize(), *pEntry );
                m_pLbBitmaps->SelectEntryPos( m_pLbBitmaps->GetEntryCount() - 1 );

                *pnBitmapListState |= CT_MODIFIED;

                ChangeBitmapHdl_Impl( this );
            }
        }
        else
            // graphic couldn't be loaded
            MessageDialog( GetParentDialog()
                          ,"NoLoadedFileDialog"
                          ,"cui/ui/querynoloadedfiledialog.ui").Execute();
    }

    return 0L;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxBitmapTabPage, ClickModifyHdl_Impl)
{
    sal_uInt16 nPos = m_pLbBitmaps->GetSelectEntryPos();

    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        ResMgr& rMgr = CUI_MGR();
        String aDesc( ResId( RID_SVXSTR_DESC_NEW_BITMAP, rMgr ) );
        OUString aName( pBitmapList->GetBitmap( nPos )->GetName() );
        OUString aOldName = aName;

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialogdiet fail!");
        AbstractSvxNameDialog* pDlg = pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc );
        DBG_ASSERT(pDlg, "Dialogdiet fail!");

        long nCount = pBitmapList->Count();
        sal_Bool bDifferent = sal_False;
        sal_Bool bLoop = sal_True;
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

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

                const BitmapEx aBitmapEx(m_pBitmapCtl->GetBitmapEx());
                const XBitmapEntry aEntry(Graphic(aBitmapEx), aName);

                m_pLbBitmaps->Modify( rStyleSettings.GetListBoxPreviewDefaultPixelSize(), aEntry, nPos );
                m_pLbBitmaps->SelectEntryPos( nPos );

                *pnBitmapListState |= CT_MODIFIED;

                bBmpChanged = sal_False;
            }
            else
            {
                MessageDialog aBox( GetParentDialog()
                                   ,"DuplicateNameDialog"
                                   ,"cui/ui/queryduplicatedialog.ui");
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
    sal_uInt16 nPos = m_pLbBitmaps->GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        QueryBox aQueryBox( GetParentDialog(), WinBits( WB_YES_NO | WB_DEF_NO ),
            String( CUI_RES( RID_SVXSTR_ASK_DEL_BITMAP ) ) );

        if( aQueryBox.Execute() == RET_YES )
        {
            delete pBitmapList->Remove( nPos );
            m_pLbBitmaps->RemoveEntry( nPos );
            m_pLbBitmaps->SelectEntryPos( 0 );

            m_pCtlPreview->Invalidate();
            m_pCtlPixel->Invalidate();

            ChangeBitmapHdl_Impl( this );

            *pnBitmapListState |= CT_MODIFIED;
        }
    }
    // determine button state
    if( !pBitmapList->Count() )
    {
        m_pBtnModify->Disable();
        m_pBtnDelete->Disable();
        m_pBtnSave->Disable();
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
        nReturn = MessageDialog( GetParentDialog()
                                ,"AskSaveList"
                                ,"cui/ui/querysavelistdialog.ui").Execute();

        if ( nReturn == RET_YES )
            pBitmapList->Save();
    }

    if ( nReturn != RET_CANCEL )
    {
        ::sfx2::FileDialogHelper aDlg( com::sun::star::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, 0 );
        String aStrFilterType( "*.sob" );
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
                XBITMAP_LIST, aPathURL.GetMainURL( INetURLObject::NO_DECODE ) )->AsBitmapList();
            pBmpList->SetName( aURL.getName() );
            if( pBmpList->Load() )
            {
                pBitmapList = pBmpList;
                ( (SvxAreaTabDialog*) GetParentDialog() )->SetNewBitmapList( pBitmapList );

                m_pLbBitmaps->Clear();
                m_pLbBitmaps->Fill( pBitmapList );
                Reset( rOutAttrs );

                pBitmapList->SetName( aURL.getName() );

                // determining (possibly cutting) the name
                // displaying it in the GroupBox
                OUString aString( ResId( RID_SVXSTR_TABLE, rMgr ) );
                aString += ": ";

                if ( aURL.getBase().getLength() > 18 )
                {
                    aString += aURL.getBase().copy( 0, 15 );
                    aString += "...";
                }
                else
                    aString += aURL.getBase();

                *pnBitmapListState |= CT_CHANGED;
                *pnBitmapListState &= ~CT_MODIFIED;
                LeaveWait();
            }
            else
            {
                LeaveWait();
                MessageDialog( GetParentDialog()
                              ,"NoLoadedFileDialog"
                              ,"cui/ui/querynoloadedfiledialog.ui").Execute();
            }
        }
    }

    // determine button state
    if( pBitmapList->Count() )
    {
        m_pBtnModify->Enable();
        m_pBtnDelete->Enable();
        m_pBtnSave->Enable();
    }
    else
    {
        m_pBtnModify->Disable();
        m_pBtnDelete->Disable();
        m_pBtnSave->Disable();
    }
    return 0L;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxBitmapTabPage, ClickSaveHdl_Impl)
{
       ::sfx2::FileDialogHelper aDlg( com::sun::star::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE, 0 );
    String aStrFilterType( "*.sob" );
    aDlg.AddFilter( aStrFilterType, aStrFilterType );

    INetURLObject aFile( SvtPathOptions().GetPalettePath() );
    DBG_ASSERT( aFile.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

    if( pBitmapList->GetName().Len() )
    {
        aFile.Append( pBitmapList->GetName() );

        if( aFile.getExtension().isEmpty() )
            aFile.SetExtension( OUString("sob") );
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
            OUString aString( CUI_RES( RID_SVXSTR_TABLE ) );
            aString += ": ";

            if ( aURL.getBase().getLength() > 18 )
            {
                aString += aURL.getBase().copy( 0, 15 );
                aString += "...";
            }
            else
                aString += aURL.getBase();

            *pnBitmapListState |= CT_SAVED;
            *pnBitmapListState &= ~CT_MODIFIED;
        }
        else
        {
            MessageDialog( GetParentDialog()
                          ,"NoSaveFileDialog"
                          ,"cui/ui/querynosavefiledialog.ui").Execute();
        }
    }

    return 0L;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxBitmapTabPage, ChangePixelColorHdl_Impl)
{
    m_pCtlPixel->SetPixelColor( m_pLbColor->GetSelectEntryColor() );
    m_pCtlPixel->Invalidate();

    m_pBitmapCtl->SetPixelColor( m_pLbColor->GetSelectEntryColor() );

    // get bitmap and display it
    rXFSet.Put(XFillBitmapItem(String(), Graphic(m_pBitmapCtl->GetBitmapEx())));
    m_pCtlPreview->SetAttributes( aXFillAttr.GetItemSet() );
    m_pCtlPreview->Invalidate();

    bBmpChanged = sal_True;

    return 0L;
}

//------------------------------------------------------------------------

IMPL_LINK_NOARG(SvxBitmapTabPage, ChangeBackgrndColorHdl_Impl)
{
    m_pCtlPixel->SetBackgroundColor( m_pLbBackgroundColor->GetSelectEntryColor() );
    m_pCtlPixel->Invalidate();

    m_pBitmapCtl->SetBackgroundColor( m_pLbBackgroundColor->GetSelectEntryColor() );

    // get bitmap and display it
    rXFSet.Put(XFillBitmapItem(String(), Graphic(m_pBitmapCtl->GetBitmapEx())));
    m_pCtlPreview->SetAttributes( aXFillAttr.GetItemSet() );
    m_pCtlPreview->Invalidate();

    bBmpChanged = sal_True;

    return 0L;
}

//------------------------------------------------------------------------

void SvxBitmapTabPage::PointChanged( Window* pWindow, RECT_POINT )
{
    if( pWindow == m_pCtlPixel )
    {
        m_pBitmapCtl->SetBmpArray( m_pCtlPixel->GetBitmapPixelPtr() );

        // get bitmap and display it
        rXFSet.Put(XFillBitmapItem(String(), Graphic(m_pBitmapCtl->GetBitmapEx())));
        m_pCtlPreview->SetAttributes( aXFillAttr.GetItemSet() );
        m_pCtlPreview->Invalidate();

        bBmpChanged = sal_True;
    }
}




Window* SvxBitmapTabPage::GetParentLabeledBy( const Window* pLabeled ) const
{
    if (pLabeled == m_pLbBitmaps)
        return const_cast<FixedText*>(m_pLbBitmapsHidden);
    else
        return SvxTabPage::GetParentLabeledBy (pLabeled);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
