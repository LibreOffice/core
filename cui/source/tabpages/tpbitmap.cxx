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
#include <vcl/settings.hxx>
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
#include <boost/scoped_ptr.hpp>

using namespace com::sun::star;

SvxBitmapTabPage::SvxBitmapTabPage(  vcl::Window* pParent, const SfxItemSet& rInAttrs) :

    SvxTabPage          ( pParent,
                          "BitmapTabPage",
                          "cui/ui/bitmaptabpage.ui",
                          rInAttrs ),
    rOutAttrs           ( rInAttrs ),

    pnBitmapListState   ( 0 ),
    pnColorListState    ( 0 ),
    pPageType           ( 0 ),
    nDlgType            ( 0 ),
    pPos                ( 0 ),
    pbAreaTP            ( 0 ),

    bBmpChanged         ( false ),

    pXPool              ( static_cast<XOutdevItemPool*>( rInAttrs.GetPool() )),
    aXFStyleItem        ( drawing::FillStyle_BITMAP ),
    aXBitmapItem        ( OUString(), Graphic() ),
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
    disposeOnce();
}

void SvxBitmapTabPage::dispose()
{
    delete m_pBitmapCtl;
    m_pBitmapCtl = NULL;
    m_pBxPixelEditor.clear();
    m_pCtlPixel.clear();
    m_pLbColor.clear();
    m_pLbBackgroundColor.clear();
    m_pLbBitmapsHidden.clear();
    m_pLbBitmaps.clear();
    m_pCtlPreview.clear();
    m_pBtnAdd.clear();
    m_pBtnModify.clear();
    m_pBtnImport.clear();
    m_pBtnDelete.clear();
    m_pBtnLoad.clear();
    m_pBtnSave.clear();
    SvxTabPage::dispose();
}

void SvxBitmapTabPage::Construct()
{
    m_pLbColor->Fill( pColorList );
    m_pLbBackgroundColor->CopyEntries( *m_pLbColor );

    m_pLbBitmaps->Fill( pBitmapList );
}



void SvxBitmapTabPage::ActivatePage( const SfxItemSet&  )
{
    sal_Int32 nPos;
    sal_Int32 nCount;

    if( nDlgType == 0 ) // area dialog
    {
        *pbAreaTP = false;

        if( pColorList.is() )
        {
            // ColorList
            if( *pnColorListState & ChangeType::CHANGED ||
                *pnColorListState & ChangeType::MODIFIED )
            {
                if( *pnColorListState & ChangeType::CHANGED )
                    pColorList = static_cast<SvxAreaTabDialog*>( GetParentDialog() )->GetNewColorList();

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
            DBG_ASSERT( aURL.GetProtocol() != INetProtocol::NotValid, "invalid URL" );

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



SfxTabPage::sfxpg SvxBitmapTabPage::DeactivatePage( SfxItemSet* _pSet)
{
    if ( CheckChanges_Impl() == -1L )
        return KEEP_PAGE;

    if( _pSet )
        FillItemSet( _pSet );

    return LEAVE_PAGE;
}



bool SvxBitmapTabPage::FillItemSet( SfxItemSet* _rOutAttrs )
{
    if( nDlgType == 0 && !*pbAreaTP ) // area dialog
    {
        if(PT_BITMAP == *pPageType)
        {
            _rOutAttrs->Put(XFillStyleItem(drawing::FillStyle_BITMAP));
            sal_Int32 nPos = m_pLbBitmaps->GetSelectEntryPos();
            if(LISTBOX_ENTRY_NOTFOUND != nPos)
            {
                const XBitmapEntry* pXBitmapEntry = pBitmapList->GetBitmap(nPos);
                const OUString aString(m_pLbBitmaps->GetSelectEntry());

                _rOutAttrs->Put(XFillBitmapItem(aString, pXBitmapEntry->GetGraphicObject()));
            }
            else
            {
                const BitmapEx aBitmapEx(m_pBitmapCtl->GetBitmapEx());

                _rOutAttrs->Put(XFillBitmapItem(OUString(), Graphic(aBitmapEx)));
            }
        }
    }

    return true;
}



void SvxBitmapTabPage::Reset( const SfxItemSet*  )
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



VclPtr<SfxTabPage> SvxBitmapTabPage::Create( vcl::Window* pWindow,
                                             const SfxItemSet* rSet )
{
    return VclPtr<SvxBitmapTabPage>::Create( pWindow, *rSet );
}



IMPL_LINK_NOARG(SvxBitmapTabPage, ChangeBitmapHdl_Impl)
{
    boost::scoped_ptr<GraphicObject> pGraphicObject;
    int nPos(m_pLbBitmaps->GetSelectEntryPos());

    if(LISTBOX_ENTRY_NOTFOUND != nPos)
    {
        pGraphicObject.reset(new GraphicObject(pBitmapList->GetBitmap(nPos)->GetGraphicObject()));
    }
    else
    {
        const SfxPoolItem* pPoolItem = 0;

        if(SfxItemState::SET == rOutAttrs.GetItemState(GetWhich(XATTR_FILLSTYLE), true, &pPoolItem))
        {
            const drawing::FillStyle eXFS((drawing::FillStyle)static_cast<const XFillStyleItem*>(pPoolItem)->GetValue());

            if((drawing::FillStyle_BITMAP == eXFS) && (SfxItemState::SET == rOutAttrs.GetItemState(GetWhich(XATTR_FILLBITMAP), true, &pPoolItem)))
            {
                pGraphicObject.reset(new GraphicObject(static_cast<const XFillBitmapItem*>(pPoolItem)->GetGraphicObject()));
            }
        }

        if(!pGraphicObject)
        {
            m_pLbBitmaps->SelectEntryPos(0);
            nPos = m_pLbBitmaps->GetSelectEntryPos();

            if(LISTBOX_ENTRY_NOTFOUND != nPos)
            {
                pGraphicObject.reset(new GraphicObject(pBitmapList->GetBitmap(nPos)->GetGraphicObject()));
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
            m_pCtlPixel->SetPaintable( true );
            m_pBxPixelEditor->Enable();
            m_pBtnModify->Enable();
            m_pBtnAdd->Enable();

            // setting the pixel control

            m_pCtlPixel->SetXBitmap(pGraphicObject->GetGraphic().GetBitmapEx());

            Color aPixelColor = aFront;
            Color aBackColor = aBack;

            // #i123564# This causes the wrong color to be selected
            // as foreground color when the 1st bitmap in the bitmap
            // list is selected. I see no reason why this is done,
            // thus I will take it out

            //if( 0 == m_pLbBitmaps->GetSelectEntryPos() )
            //{
            //  m_pLbColor->SelectEntry( Color( COL_BLACK ) );
            //  ChangePixelColorHdl_Impl( this );
            //}
            //else

            m_pLbColor->SelectEntry( aPixelColor );

            if( m_pLbColor->GetSelectEntryCount() == 0 )
            {
                m_pLbColor->InsertEntry( aPixelColor, OUString() );
                m_pLbColor->SelectEntry( aPixelColor );
            }

            m_pLbBackgroundColor->SelectEntry( aBackColor );

            if( m_pLbBackgroundColor->GetSelectEntryCount() == 0 )
            {
                m_pLbBackgroundColor->InsertEntry( aBackColor, OUString() );
                m_pLbBackgroundColor->SelectEntry( aBackColor );
            }

            // update m_pBitmapCtl, rXFSet and m_pCtlPreview
            m_pBitmapCtl->SetPixelColor( aPixelColor );
            m_pBitmapCtl->SetBackgroundColor( aBackColor );
            rXFSet.Put(XFillBitmapItem(OUString(), Graphic(m_pBitmapCtl->GetBitmapEx())));
            m_pCtlPreview->SetAttributes( aXFillAttr.GetItemSet() );
            m_pCtlPreview->Invalidate();
        }
        else
        {
            m_pCtlPixel->Reset();
            m_pCtlPixel->SetPaintable( false );
            m_pBxPixelEditor->Disable();
            m_pBtnModify->Disable();
            m_pBtnAdd->Disable();
        }

        m_pCtlPixel->Invalidate();

        // display bitmap
        const XFillBitmapItem aXBmpItem(OUString(), *pGraphicObject);
        rXFSet.Put( aXBmpItem );

        m_pCtlPreview->SetAttributes( aXFillAttr.GetItemSet() );
        m_pCtlPreview->Invalidate();

        bBmpChanged = false;
    }

    return 0;
}



long SvxBitmapTabPage::CheckChanges_Impl()
{
    sal_Int32 nPos = m_pLbBitmaps->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        if( bBmpChanged )
        {
            ResMgr& rMgr = CUI_MGR();
            Image aWarningBoxImage = WarningBox::GetStandardImage();
            ScopedVclPtrInstance<SvxMessDialog> aMessDlg( GetParentDialog(),
                                                          SVX_RES( RID_SVXSTR_BITMAP ),
                                                          CUI_RES( RID_SVXSTR_ASK_CHANGE_BITMAP ),
                                                          &aWarningBoxImage );
            DBG_ASSERT(aMessDlg, "Dialog creation failed!");
            aMessDlg->SetButtonText( MESS_BTN_1, ResId( RID_SVXSTR_CHANGE, rMgr ) );
            aMessDlg->SetButtonText( MESS_BTN_2, ResId( RID_SVXSTR_ADD, rMgr ) );

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
                }
                break;

                case RET_CANCEL:
                break;
            }
        }
    }
    nPos = m_pLbBitmaps->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
        *pPos = nPos;
    return 0L;
}



IMPL_LINK_NOARG(SvxBitmapTabPage, ClickAddHdl_Impl)
{

    OUString aNewName( SVX_RES( RID_SVXSTR_BITMAP ) );
    OUString aDesc( CUI_RES( RID_SVXSTR_DESC_NEW_BITMAP ) );
    OUString aName;

    long nCount = pBitmapList->Count();
    long j = 1;
    bool bDifferent = false;

    while( !bDifferent )
    {
        aName  = aNewName;
        aName += " ";
        aName += OUString::number( j++ );
        bDifferent = true;

        for( long i = 0; i < nCount && bDifferent; i++ )
            if( aName == pBitmapList->GetBitmap( i )->GetName() )
                bDifferent = false;
    }

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialog creation failed!");
    boost::scoped_ptr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc ));
    DBG_ASSERT(pDlg, "Dialog creation failed!");
    ScopedVclPtr<MessageDialog> pWarnBox;
    sal_uInt16         nError(1);

    while( pDlg->Execute() == RET_OK )
    {
        pDlg->GetName( aName );

        bDifferent = true;

        for( long i = 0; i < nCount && bDifferent; i++ )
            if( aName == pBitmapList->GetBitmap( i )->GetName() )
                bDifferent = false;

        if( bDifferent ) {
            nError = 0;
            break;
        }

        if( !pWarnBox )
        {
            pWarnBox.reset(VclPtr<MessageDialog>::Create( GetParentDialog()
                                        ,"DuplicateNameDialog"
                                        ,"cui/ui/queryduplicatedialog.ui"));
        }

        if( pWarnBox->Execute() != RET_OK )
            break;
    }

    pDlg.reset();
    pWarnBox.reset();

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

            if(SfxItemState::SET == rOutAttrs.GetItemState(XATTR_FILLBITMAP, true, &pPoolItem))
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

            *pnBitmapListState |= ChangeType::MODIFIED;

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




IMPL_LINK_NOARG(SvxBitmapTabPage, ClickImportHdl_Impl)
{
    ResMgr& rMgr = CUI_MGR();
    SvxOpenGraphicDialog aDlg( OUString("Import") );
    aDlg.EnableLink(false);

    if( !aDlg.Execute() )
    {
        Graphic         aGraphic;

        EnterWait();
        int nError = aDlg.GetGraphic( aGraphic );
        LeaveWait();

        if( !nError )
        {
            OUString aDesc( ResId(RID_SVXSTR_DESC_EXT_BITMAP, rMgr) );
            ScopedVclPtr<MessageDialog> pWarnBox;

            // convert file URL to UI name
            OUString        aName;
            INetURLObject   aURL( aDlg.GetPath() );
            SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
            DBG_ASSERT(pFact, "Dialog creation failed!");
            boost::scoped_ptr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog( GetParentDialog(), aURL.GetName().getToken( 0, '.' ), aDesc ));
            DBG_ASSERT(pDlg, "Dialog creation failed!");
            nError = 1;

            while( pDlg->Execute() == RET_OK )
            {
                pDlg->GetName( aName );

                bool bDifferent = true;
                long nCount     = pBitmapList->Count();

                for( long i = 0; i < nCount && bDifferent; i++ )
                    if( aName == pBitmapList->GetBitmap( i )->GetName() )
                        bDifferent = false;

                if( bDifferent ) {
                    nError = 0;
                    break;
                }

                if( !pWarnBox )
                {
                    pWarnBox.reset(VclPtr<MessageDialog>::Create( GetParentDialog()
                                                 ,"DuplicateNameDialog"
                                                 ,"cui/ui/queryduplicatedialog.ui"));
                }

                if( pWarnBox->Execute() != RET_OK )
                    break;
            }

            pDlg.reset();
            pWarnBox.reset();

            if( !nError )
            {
                XBitmapEntry* pEntry = new XBitmapEntry( aGraphic, aName );
                pBitmapList->Insert( pEntry );

                const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
                m_pLbBitmaps->Append(rStyleSettings.GetListBoxPreviewDefaultPixelSize(), *pEntry );
                m_pLbBitmaps->SelectEntryPos( m_pLbBitmaps->GetEntryCount() - 1 );

                *pnBitmapListState |= ChangeType::MODIFIED;

                ChangeBitmapHdl_Impl( this );
            }
        }
        else
            // graphic couldn't be loaded
            ScopedVclPtrInstance<MessageDialog>::Create( GetParentDialog()
                          ,"NoLoadedFileDialog"
                          ,"cui/ui/querynoloadedfiledialog.ui")->Execute();
    }

    return 0L;
}



IMPL_LINK_NOARG(SvxBitmapTabPage, ClickModifyHdl_Impl)
{
    sal_Int32 nPos = m_pLbBitmaps->GetSelectEntryPos();

    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        ResMgr& rMgr = CUI_MGR();
        OUString aDesc( ResId( RID_SVXSTR_DESC_NEW_BITMAP, rMgr ) );
        OUString aName( pBitmapList->GetBitmap( nPos )->GetName() );
        OUString aOldName = aName;

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialog creation failed!");
        boost::scoped_ptr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc ));
        DBG_ASSERT(pDlg, "Dialog creation failed!");

        long nCount = pBitmapList->Count();
        bool bLoop = true;
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

        while( bLoop && pDlg->Execute() == RET_OK )
        {
            pDlg->GetName( aName );
            bool bDifferent = true;

            for( long i = 0; i < nCount && bDifferent; i++ )
            {
                if( aName == pBitmapList->GetBitmap( i )->GetName() &&
                    aName != aOldName )
                    bDifferent = false;
            }

            if( bDifferent )
            {
                bLoop = false;

                const BitmapEx aBitmapEx(m_pBitmapCtl->GetBitmapEx());

                // #i123497# Need to replace the existing entry with a new one (old returned needs to be deleted)
                XBitmapEntry* pEntry = new XBitmapEntry(Graphic(aBitmapEx), aName);
                delete pBitmapList->Replace(pEntry, nPos);

                m_pLbBitmaps->Modify( rStyleSettings.GetListBoxPreviewDefaultPixelSize(), *pEntry, nPos );
                m_pLbBitmaps->SelectEntryPos( nPos );

                *pnBitmapListState |= ChangeType::MODIFIED;

                bBmpChanged = false;
            }
            else
            {
                ScopedVclPtrInstance<MessageDialog> aBox(
                                   GetParentDialog()
                                   ,"DuplicateNameDialog"
                                   ,"cui/ui/queryduplicatedialog.ui");
                aBox->Execute();
            }
        }
    }
    return 0L;
}



IMPL_LINK_NOARG(SvxBitmapTabPage, ClickDeleteHdl_Impl)
{
    sal_Int32 nPos = m_pLbBitmaps->GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        ScopedVclPtrInstance< MessageDialog > aQueryBox( GetParentDialog(),"AskDelBitmapDialog","cui/ui/querydeletebitmapdialog.ui" );

        if( aQueryBox->Execute() == RET_YES )
        {
            delete pBitmapList->Remove( nPos );
            m_pLbBitmaps->RemoveEntry( nPos );
            m_pLbBitmaps->SelectEntryPos( 0 );

            m_pCtlPreview->Invalidate();
            m_pCtlPixel->Invalidate();

            ChangeBitmapHdl_Impl( this );

            *pnBitmapListState |= ChangeType::MODIFIED;
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



IMPL_LINK_NOARG(SvxBitmapTabPage, ClickLoadHdl_Impl)
{
    sal_uInt16 nReturn = RET_YES;
    ResMgr& rMgr = CUI_MGR();

    if ( *pnBitmapListState & ChangeType::MODIFIED )
    {
        nReturn = ScopedVclPtrInstance<MessageDialog>::Create( GetParentDialog()
                                ,"AskSaveList"
                                ,"cui/ui/querysavelistdialog.ui")->Execute();

        if ( nReturn == RET_YES )
            pBitmapList->Save();
    }

    if ( nReturn != RET_CANCEL )
    {
        ::sfx2::FileDialogHelper aDlg( com::sun::star::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, 0 );
        OUString aStrFilterType( "*.sob" );
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
            XBitmapListRef pBmpList = XPropertyList::AsBitmapList(
                XPropertyList::CreatePropertyList(
                    XBITMAP_LIST, aPathURL.GetMainURL(INetURLObject::NO_DECODE),
                    ""));
            pBmpList->SetName( aURL.getName() );
            if( pBmpList->Load() )
            {
                pBitmapList = pBmpList;
                static_cast<SvxAreaTabDialog*>( GetParentDialog() )->SetNewBitmapList( pBitmapList );

                m_pLbBitmaps->Clear();
                m_pLbBitmaps->Fill( pBitmapList );
                Reset( &rOutAttrs );

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

                *pnBitmapListState |= ChangeType::CHANGED;
                *pnBitmapListState &= ~ChangeType::MODIFIED;
                LeaveWait();
            }
            else
            {
                LeaveWait();
                ScopedVclPtrInstance<MessageDialog>::Create( GetParentDialog()
                              ,"NoLoadedFileDialog"
                              ,"cui/ui/querynoloadedfiledialog.ui")->Execute();
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



IMPL_LINK_NOARG(SvxBitmapTabPage, ClickSaveHdl_Impl)
{
       ::sfx2::FileDialogHelper aDlg( com::sun::star::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE, 0 );
    OUString aStrFilterType( "*.sob" );
    aDlg.AddFilter( aStrFilterType, aStrFilterType );

    INetURLObject aFile( SvtPathOptions().GetPalettePath() );
    DBG_ASSERT( aFile.GetProtocol() != INetProtocol::NotValid, "invalid URL" );

    if( !pBitmapList->GetName().isEmpty() )
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

            *pnBitmapListState |= ChangeType::SAVED;
            *pnBitmapListState &= ~ChangeType::MODIFIED;
        }
        else
        {
            ScopedVclPtrInstance<MessageDialog>::Create( GetParentDialog()
                          ,"NoSaveFileDialog"
                          ,"cui/ui/querynosavefiledialog.ui")->Execute();
        }
    }

    return 0L;
}



IMPL_LINK_NOARG(SvxBitmapTabPage, ChangePixelColorHdl_Impl)
{
    m_pCtlPixel->SetPixelColor( m_pLbColor->GetSelectEntryColor() );
    m_pCtlPixel->Invalidate();

    m_pBitmapCtl->SetPixelColor( m_pLbColor->GetSelectEntryColor() );

    // get bitmap and display it
    rXFSet.Put(XFillBitmapItem(OUString(), Graphic(m_pBitmapCtl->GetBitmapEx())));
    m_pCtlPreview->SetAttributes( aXFillAttr.GetItemSet() );
    m_pCtlPreview->Invalidate();

    bBmpChanged = true;

    return 0L;
}



IMPL_LINK_NOARG(SvxBitmapTabPage, ChangeBackgrndColorHdl_Impl)
{
    m_pCtlPixel->SetBackgroundColor( m_pLbBackgroundColor->GetSelectEntryColor() );
    m_pCtlPixel->Invalidate();

    m_pBitmapCtl->SetBackgroundColor( m_pLbBackgroundColor->GetSelectEntryColor() );

    // get bitmap and display it
    rXFSet.Put(XFillBitmapItem(OUString(), Graphic(m_pBitmapCtl->GetBitmapEx())));
    m_pCtlPreview->SetAttributes( aXFillAttr.GetItemSet() );
    m_pCtlPreview->Invalidate();

    bBmpChanged = true;

    return 0L;
}



void SvxBitmapTabPage::PointChanged( vcl::Window* pWindow, RECT_POINT )
{
    if( pWindow == m_pCtlPixel )
    {
        m_pBitmapCtl->SetBmpArray( m_pCtlPixel->GetBitmapPixelPtr() );

        // get bitmap and display it
        rXFSet.Put(XFillBitmapItem(OUString(), Graphic(m_pBitmapCtl->GetBitmapEx())));
        m_pCtlPreview->SetAttributes( aXFillAttr.GetItemSet() );
        m_pCtlPreview->Invalidate();

        bBmpChanged = true;
    }
}




vcl::Window* SvxBitmapTabPage::GetParentLabeledBy( const vcl::Window* pLabeled ) const
{
    if (pLabeled == m_pLbBitmaps)
        return m_pLbBitmapsHidden.get();
    else
        return SvxTabPage::GetParentLabeledBy (pLabeled);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
