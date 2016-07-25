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
#include <vcl/bitmapaccess.hxx>
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

using namespace com::sun::star;

SvxPatternTabPage::SvxPatternTabPage(  vcl::Window* pParent, const SfxItemSet& rInAttrs) :

    SvxTabPage          ( pParent,
                          "PatternTabPage",
                          "cui/ui/patterntabpage.ui",
                          rInAttrs ),
    m_rOutAttrs           ( rInAttrs ),

    m_pnPatternListState  ( nullptr ),
    m_pnColorListState    ( nullptr ),
    m_pPageType           ( nullptr ),
    m_nDlgType            ( 0 ),
    m_pPos                ( nullptr ),
    m_pbAreaTP            ( nullptr ),

    m_bPtrnChanged        ( false ),

    m_aXFStyleItem        ( drawing::FillStyle_BITMAP ),
    m_aXPatternItem       ( OUString(), Graphic() ),
    m_aXFillAttr          ( rInAttrs.GetPool() ),
    m_rXFSet              ( m_aXFillAttr.GetItemSet() )
{
    get(m_pBxPixelEditor,"maingrid");
    get(m_pCtlPixel,"CTL_PIXEL");
    get(m_pLbColor,"LB_COLOR");
    get(m_pLbBackgroundColor,"LB_BACKGROUND_COLOR");
    get(m_pCtlPreview,"CTL_PREVIEW");
    get(m_pLbPatterns,"LB_PATTERN");
    get(m_pBtnAdd,"BTN_ADD");
    get(m_pBtnModify,"BTN_MODIFY");
    get(m_pBtnDelete,"BTN_DELETE");
    get(m_pBtnLoad,"BTN_LOAD");
    get(m_pBtnSave,"BTN_SAVE");

    // size of the bitmap listbox
    Size aSize = getDrawListBoxOptimalSize(this);
    m_pLbPatterns->set_width_request(aSize.Width());
    m_pLbPatterns->set_height_request(aSize.Height());

    // size of the bitmap display
    Size aSize2 = getDrawPreviewOptimalSize(this);
    m_pCtlPreview->set_width_request(aSize2.Width());
    m_pCtlPreview->set_height_request(aSize2.Height());

    m_pBitmapCtl = new SvxBitmapCtl;

    // this page needs ExchangeSupport
    SetExchangeSupport();

    // setting the output device
    m_rXFSet.Put( m_aXFStyleItem );
    m_rXFSet.Put( m_aXPatternItem );

    m_pBtnAdd->SetClickHdl( LINK( this, SvxPatternTabPage, ClickAddHdl_Impl ) );
    m_pBtnModify->SetClickHdl( LINK( this, SvxPatternTabPage, ClickModifyHdl_Impl ) );
    m_pBtnDelete->SetClickHdl( LINK( this, SvxPatternTabPage, ClickDeleteHdl_Impl ) );
    m_pBtnLoad->SetClickHdl( LINK( this, SvxPatternTabPage, ClickLoadHdl_Impl ) );
    m_pBtnSave->SetClickHdl( LINK( this, SvxPatternTabPage, ClickSaveHdl_Impl ) );

    m_pLbPatterns->SetSelectHdl( LINK( this, SvxPatternTabPage, ChangePatternHdl_Impl ) );
    m_pLbColor->SetSelectHdl( LINK( this, SvxPatternTabPage, ChangeColorHdl_Impl ) );
    m_pLbBackgroundColor->SetSelectHdl( LINK( this, SvxPatternTabPage, ChangeColorHdl_Impl ) );

    setPreviewsToSamePlace(pParent, this);
}

SvxPatternTabPage::~SvxPatternTabPage()
{
    disposeOnce();
}

void SvxPatternTabPage::dispose()
{
    delete m_pBitmapCtl;
    m_pBitmapCtl = nullptr;
    m_pBxPixelEditor.clear();
    m_pCtlPixel.clear();
    m_pLbColor.clear();
    m_pLbBackgroundColor.clear();
    m_pLbPatterns.clear();
    m_pCtlPreview.clear();
    m_pBtnAdd.clear();
    m_pBtnModify.clear();
    m_pBtnDelete.clear();
    m_pBtnLoad.clear();
    m_pBtnSave.clear();
    SvxTabPage::dispose();
}

void SvxPatternTabPage::Construct()
{
    m_pLbColor->Fill( m_pColorList );
    m_pLbBackgroundColor->CopyEntries( *m_pLbColor );
    m_pLbPatterns->Fill( m_pPatternList );
}


void SvxPatternTabPage::ActivatePage( const SfxItemSet&  )
{
    sal_Int32 nPos;
    sal_Int32 nCount;

    if( m_nDlgType == 0 ) // area dialog
    {
        *m_pbAreaTP = false;

        if( m_pColorList.is() )
        {
            // ColorList
            if( *m_pnColorListState & ChangeType::CHANGED ||
                *m_pnColorListState & ChangeType::MODIFIED )
            {
                if( *m_pnColorListState & ChangeType::CHANGED )
                    m_pColorList = static_cast<SvxAreaTabDialog*>( GetParentDialog() )->GetNewColorList();

                // LbColor
                nPos = m_pLbColor->GetSelectEntryPos();
                m_pLbColor->Clear();
                m_pLbColor->Fill( m_pColorList );
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
                ChangeColor_Impl();
            }

            // determining (possibly cutting) the name and
            // displaying it in the GroupBox
            OUString        aString( CUI_RES( RID_SVXSTR_TABLE ) );
            aString         += ": ";
            INetURLObject   aURL( m_pPatternList->GetPath() );

            aURL.Append( m_pPatternList->GetName() );
            SAL_WARN_IF( aURL.GetProtocol() == INetProtocol::NotValid, "cui.tabpages", "invalid URL" );

            if( aURL.getBase().getLength() > 18 )
            {
                aString += aURL.getBase().copy( 0, 15 ) + "...";
            }
            else
                aString += aURL.getBase();

            if( *m_pPageType == PT_BITMAP && *m_pPos != LISTBOX_ENTRY_NOTFOUND )
            {
                m_pLbPatterns->SelectEntryPos( *m_pPos );
            }
            // colors could have been deleted
            ChangePatternHdl_Impl( *m_pLbPatterns );

            *m_pPageType = PT_BITMAP;
            *m_pPos = LISTBOX_ENTRY_NOTFOUND;
        }
    }
}


DeactivateRC SvxPatternTabPage::DeactivatePage( SfxItemSet* _pSet)
{
    if ( CheckChanges_Impl() == -1L )
        return DeactivateRC::KeepPage;

    if( _pSet )
        FillItemSet( _pSet );

    return DeactivateRC::LeavePage;
}


bool SvxPatternTabPage::FillItemSet( SfxItemSet* _rOutAttrs )
{
    if( m_nDlgType == 0 && !*m_pbAreaTP ) // area dialog
    {
        if(PT_BITMAP == *m_pPageType)
        {
            _rOutAttrs->Put(XFillStyleItem(drawing::FillStyle_BITMAP));
            sal_Int32 nPos = m_pLbPatterns->GetSelectEntryPos();
            if(LISTBOX_ENTRY_NOTFOUND != nPos)
            {
                const XBitmapEntry* pXBitmapEntry = m_pPatternList->GetBitmap(nPos);
                const OUString aString(m_pLbPatterns->GetSelectEntry());

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


void SvxPatternTabPage::Reset( const SfxItemSet*  )
{
    // aLbBitmaps.SelectEntryPos( 0 );

    m_pBitmapCtl->SetLines( m_pCtlPixel->GetLineCount() );
    m_pBitmapCtl->SetPixelColor( m_pLbColor->GetSelectEntryColor() );
    m_pBitmapCtl->SetBackgroundColor( m_pLbBackgroundColor->GetSelectEntryColor() );
    m_pBitmapCtl->SetBmpArray( m_pCtlPixel->GetBitmapPixelPtr() );

    // get bitmap and display it
    const XFillBitmapItem aBmpItem(OUString(), Graphic(m_pBitmapCtl->GetBitmapEx()));
    m_rXFSet.Put( aBmpItem );
    m_pCtlPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pCtlPreview->Invalidate();

    ChangePatternHdl_Impl( *m_pLbPatterns );

    // determine button state
    if( m_pPatternList.is() && m_pPatternList->Count() )
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


VclPtr<SfxTabPage> SvxPatternTabPage::Create( vcl::Window* pWindow,
                                             const SfxItemSet* rSet )
{
    return VclPtr<SvxPatternTabPage>::Create( pWindow, *rSet );
}


IMPL_LINK_NOARG_TYPED(SvxPatternTabPage, ChangePatternHdl_Impl, ListBox&, void)
{
    std::unique_ptr<GraphicObject> pGraphicObject;
    int nPos(m_pLbPatterns->GetSelectEntryPos());

    if(LISTBOX_ENTRY_NOTFOUND != nPos)
    {
        pGraphicObject.reset(new GraphicObject(m_pPatternList->GetBitmap(nPos)->GetGraphicObject()));
    }
    else
    {
        const SfxPoolItem* pPoolItem = nullptr;

        if(SfxItemState::SET == m_rOutAttrs.GetItemState(GetWhich(XATTR_FILLSTYLE), true, &pPoolItem))
        {
            const drawing::FillStyle eXFS((drawing::FillStyle)static_cast<const XFillStyleItem*>(pPoolItem)->GetValue());

            if((drawing::FillStyle_BITMAP == eXFS) && (SfxItemState::SET == m_rOutAttrs.GetItemState(GetWhich(XATTR_FILLBITMAP), true, &pPoolItem)))
            {
                pGraphicObject.reset(new GraphicObject(static_cast<const XFillBitmapItem*>(pPoolItem)->GetGraphicObject()));
            }
        }

        if(!pGraphicObject)
        {
            m_pLbPatterns->SelectEntryPos(0);
            nPos = m_pLbPatterns->GetSelectEntryPos();

            if(LISTBOX_ENTRY_NOTFOUND != nPos)
            {
                pGraphicObject.reset(new GraphicObject(m_pPatternList->GetBitmap(nPos)->GetGraphicObject()));
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
            m_rXFSet.Put(XFillBitmapItem(OUString(), Graphic(m_pBitmapCtl->GetBitmapEx())));
            m_pCtlPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
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

        m_bPtrnChanged = false;
    }
}


long SvxPatternTabPage::CheckChanges_Impl()
{
    sal_Int32 nPos = m_pLbPatterns->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        if( m_bPtrnChanged )
        {
            ResMgr& rMgr = CUI_MGR();
            Image aWarningBoxImage = WarningBox::GetStandardImage();
            ScopedVclPtrInstance<SvxMessDialog> aMessDlg( GetParentDialog(),
                                                          SVX_RES( RID_SVXSTR_PATTERN ),
                                                          CUI_RES( RID_SVXSTR_ASK_CHANGE_PATTERN ),
                                                          &aWarningBoxImage );
            assert(aMessDlg && "Dialog creation failed!");
            aMessDlg->SetButtonText( SvxMessDialogButton::N1, ResId( RID_SVXSTR_CHANGE, rMgr ) );
            aMessDlg->SetButtonText( SvxMessDialogButton::N2, ResId( RID_SVXSTR_ADD, rMgr ) );

            short nRet = aMessDlg->Execute();

            switch( nRet )
            {
                case RET_BTN_1:
                {
                    ClickModifyHdl_Impl( nullptr );
                }
                break;

                case RET_BTN_2:
                {
                    ClickAddHdl_Impl( nullptr );
                }
                break;

                case RET_CANCEL:
                break;
            }
        }
    }
    nPos = m_pLbPatterns->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
        *m_pPos = nPos;
    return 0L;
}


IMPL_LINK_NOARG_TYPED(SvxPatternTabPage, ClickAddHdl_Impl, Button*, void)
{

    OUString aNewName( SVX_RES( RID_SVXSTR_PATTERN_UNTITLED ) );
    OUString aDesc( CUI_RES( RID_SVXSTR_DESC_NEW_PATTERN ) );
    OUString aName;

    long nCount = m_pPatternList->Count();
    long j = 1;
    bool bDifferent = false;

    while( !bDifferent )
    {
        aName  = aNewName + " " + OUString::number( j++ );
        bDifferent = true;

        for( long i = 0; i < nCount && bDifferent; i++ )
            if( aName == m_pPatternList->GetBitmap( i )->GetName() )
                bDifferent = false;
    }

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    assert(pFact && "Dialog creation failed!");
    std::unique_ptr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc ));
    assert(pDlg && "Dialog creation failed!");
    ScopedVclPtr<MessageDialog> pWarnBox;
    sal_uInt16         nError(1);

    while( pDlg->Execute() == RET_OK )
    {
        pDlg->GetName( aName );

        bDifferent = true;

        for( long i = 0; i < nCount && bDifferent; i++ )
            if( aName == m_pPatternList->GetBitmap( i )->GetName() )
                bDifferent = false;

        if( bDifferent ) {
            nError = 0;
            break;
        }

        if( !pWarnBox )
        {
            pWarnBox.disposeAndReset(VclPtr<MessageDialog>::Create( GetParentDialog()
                                        ,"DuplicateNameDialog"
                                        ,"cui/ui/queryduplicatedialog.ui"));
        }

        if( pWarnBox->Execute() != RET_OK )
            break;
    }

    pDlg.reset();
    pWarnBox.disposeAndClear();

    if( !nError )
    {
        XBitmapEntry* pEntry = nullptr;
        if( m_pCtlPixel->IsEnabled() )
        {
            const BitmapEx aBitmapEx(m_pBitmapCtl->GetBitmapEx());

            pEntry = new XBitmapEntry(Graphic(aBitmapEx), aName);
        }
        else // it must be a not existing imported bitmap
        {
            const SfxPoolItem* pPoolItem = nullptr;

            if(SfxItemState::SET == m_rOutAttrs.GetItemState(XATTR_FILLBITMAP, true, &pPoolItem))
            {
                pEntry = new XBitmapEntry(dynamic_cast< const XFillBitmapItem* >(pPoolItem)->GetGraphicObject(), aName);
            }
        }

        assert( pEntry && "SvxPatternTabPage::ClickAddHdl_Impl(), pEntry == 0 ?" );

        if( pEntry )
        {
            m_pPatternList->Insert( pEntry );
            const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
            m_pLbPatterns->Append(rStyleSettings.GetListBoxPreviewDefaultPixelSize(), *pEntry );
            m_pLbPatterns->SelectEntryPos( m_pLbPatterns->GetEntryCount() - 1 );

            *m_pnPatternListState |= ChangeType::MODIFIED;

            ChangePatternHdl_Impl( *m_pLbPatterns );
        }
    }

    // determine button state
    if( m_pPatternList->Count() )
    {
        m_pBtnModify->Enable();
        m_pBtnDelete->Enable();
        m_pBtnSave->Enable();
    }
}


/******************************************************************************/
/******************************************************************************/


IMPL_LINK_NOARG_TYPED(SvxPatternTabPage, ClickModifyHdl_Impl, Button*, void)
{
    sal_Int32 nPos = m_pLbPatterns->GetSelectEntryPos();

    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        ResMgr& rMgr = CUI_MGR();
        OUString aDesc( ResId( RID_SVXSTR_DESC_NEW_PATTERN, rMgr ) );
        OUString aName( m_pPatternList->GetBitmap( nPos )->GetName() );
        OUString aOldName = aName;

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        assert(pFact && "Dialog creation failed!");
        std::unique_ptr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc ));
        assert(pDlg && "Dialog creation failed!");

        long nCount = m_pPatternList->Count();
        bool bLoop = true;
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

        while( bLoop && pDlg->Execute() == RET_OK )
        {
            pDlg->GetName( aName );
            bool bDifferent = true;

            for( long i = 0; i < nCount && bDifferent; i++ )
            {
                if( aName == m_pPatternList->GetBitmap( i )->GetName() &&
                    aName != aOldName )
                    bDifferent = false;
            }

            if( bDifferent )
            {
                bLoop = false;

                const BitmapEx aBitmapEx(m_pBitmapCtl->GetBitmapEx());

                // #i123497# Need to replace the existing entry with a new one (old returned needs to be deleted)
                XBitmapEntry* pEntry = new XBitmapEntry(Graphic(aBitmapEx), aName);
                delete m_pPatternList->Replace(pEntry, nPos);

                m_pLbPatterns->Modify( rStyleSettings.GetListBoxPreviewDefaultPixelSize(), *pEntry, nPos );
                m_pLbPatterns->SelectEntryPos( nPos );

                *m_pnPatternListState |= ChangeType::MODIFIED;

                m_bPtrnChanged = false;
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
}


IMPL_LINK_NOARG_TYPED(SvxPatternTabPage, ClickDeleteHdl_Impl, Button*, void)
{
    sal_Int32 nPos = m_pLbPatterns->GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        ScopedVclPtrInstance< MessageDialog > aQueryBox( GetParentDialog(),"AskDelBitmapDialog","cui/ui/querydeletebitmapdialog.ui" );

        if( aQueryBox->Execute() == RET_YES )
        {
            delete m_pPatternList->Remove( nPos );
            m_pLbPatterns->RemoveEntry( nPos );
            m_pLbPatterns->SelectEntryPos( 0 );

            m_pCtlPreview->Invalidate();
            m_pCtlPixel->Invalidate();

            ChangePatternHdl_Impl( *m_pLbPatterns );

            *m_pnPatternListState |= ChangeType::MODIFIED;
        }
    }
    // determine button state
    if( !m_pPatternList->Count() )
    {
        m_pBtnModify->Disable();
        m_pBtnDelete->Disable();
        m_pBtnSave->Disable();
    }
}


IMPL_LINK_NOARG_TYPED(SvxPatternTabPage, ClickLoadHdl_Impl, Button*, void)
{
    sal_uInt16 nReturn = RET_YES;
    ResMgr& rMgr = CUI_MGR();

    if ( *m_pnPatternListState & ChangeType::MODIFIED )
    {
        nReturn = ScopedVclPtrInstance<MessageDialog>::Create( GetParentDialog()
                                ,"AskSaveList"
                                ,"cui/ui/querysavelistdialog.ui")->Execute();

        if ( nReturn == RET_YES )
            m_pPatternList->Save();
    }

    if ( nReturn != RET_CANCEL )
    {
        ::sfx2::FileDialogHelper aDlg( css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE );
        OUString aStrFilterType( "*.sop" );
        aDlg.AddFilter( aStrFilterType, aStrFilterType );
        OUString aPalettePath(SvtPathOptions().GetPalettePath());
        OUString aLastDir;
        sal_Int32 nIndex = 0;
        do
        {
            aLastDir = aPalettePath.getToken(0, ';', nIndex);
        }
        while (nIndex >= 0);

        INetURLObject aFile(aLastDir);
        aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );

        if ( aDlg.Execute() == ERRCODE_NONE )
        {
            EnterWait();
            INetURLObject aURL( aDlg.GetPath() );
            INetURLObject aPathURL( aURL );

            aPathURL.removeSegment();
            aPathURL.removeFinalSlash();

            // save table
            XPatternListRef pBmpList = XPropertyList::AsPatternList(
                XPropertyList::CreatePropertyList(
                    XPATTERN_LIST, aPathURL.GetMainURL(INetURLObject::NO_DECODE),
                    ""));
            pBmpList->SetName( aURL.getName() );
            if( pBmpList->Load() )
            {
                m_pPatternList = pBmpList;
                static_cast<SvxAreaTabDialog*>( GetParentDialog() )->SetNewPatternList( m_pPatternList );

                m_pLbPatterns->Clear();
                m_pLbPatterns->Fill( m_pPatternList );
                Reset( &m_rOutAttrs );

                m_pPatternList->SetName( aURL.getName() );

                // determining (possibly cutting) the name
                // displaying it in the GroupBox
                OUString aString( ResId( RID_SVXSTR_TABLE, rMgr ) );
                aString += ": ";

                if ( aURL.getBase().getLength() > 18 )
                {
                    aString += aURL.getBase().copy( 0, 15 ) + "...";
                }
                else
                    aString += aURL.getBase();

                *m_pnPatternListState |= ChangeType::CHANGED;
                *m_pnPatternListState &= ~ChangeType::MODIFIED;
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
    if( m_pPatternList->Count() )
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
}


IMPL_LINK_NOARG_TYPED(SvxPatternTabPage, ClickSaveHdl_Impl, Button*, void)
{
    ::sfx2::FileDialogHelper aDlg( css::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE );
    OUString aStrFilterType( "*.sop" );
    aDlg.AddFilter( aStrFilterType, aStrFilterType );

    OUString aPalettePath(SvtPathOptions().GetPalettePath());
    OUString aLastDir;
    sal_Int32 nIndex = 0;
    do
    {
        aLastDir = aPalettePath.getToken(0, ';', nIndex);
    }
    while (nIndex >= 0);

    INetURLObject aFile(aLastDir);
    SAL_WARN_IF( aFile.GetProtocol() == INetProtocol::NotValid, "cui.tabpages", "invalid URL" );

    if( !m_pPatternList->GetName().isEmpty() )
    {
        aFile.Append( m_pPatternList->GetName() );

        if( aFile.getExtension().isEmpty() )
            aFile.SetExtension( "sop" );
    }

    aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );
    if ( aDlg.Execute() == ERRCODE_NONE )
    {
        INetURLObject   aURL( aDlg.GetPath() );
        INetURLObject   aPathURL( aURL );

        aPathURL.removeSegment();
        aPathURL.removeFinalSlash();

        m_pPatternList->SetName( aURL.getName() );
        m_pPatternList->SetPath( aPathURL.GetMainURL( INetURLObject::NO_DECODE ) );

        if( m_pPatternList->Save() )
        {
            // determining (possibly cutting) the name
            // displaying it in the GroupBox
            OUString aString( CUI_RES( RID_SVXSTR_TABLE ) );
            aString += ": ";

            if ( aURL.getBase().getLength() > 18 )
            {
                aString += aURL.getBase().copy( 0, 15 ) + "...";
            }
            else
                aString += aURL.getBase();

            *m_pnPatternListState |= ChangeType::SAVED;
            *m_pnPatternListState &= ~ChangeType::MODIFIED;
        }
        else
        {
            ScopedVclPtrInstance<MessageDialog>::Create( GetParentDialog()
                          ,"NoSaveFileDialog"
                          ,"cui/ui/querynosavefiledialog.ui")->Execute();
        }
    }
}


IMPL_LINK_NOARG_TYPED(SvxPatternTabPage, ChangeColorHdl_Impl, ListBox&, void)
{
    ChangeColor_Impl();
}

void SvxPatternTabPage::ChangeColor_Impl()
{
    m_pCtlPixel->SetPixelColor( m_pLbColor->GetSelectEntryColor() );
    m_pCtlPixel->SetBackgroundColor( m_pLbBackgroundColor->GetSelectEntryColor() );
    m_pCtlPixel->Invalidate();

    m_pBitmapCtl->SetPixelColor( m_pLbColor->GetSelectEntryColor() );
    m_pBitmapCtl->SetBackgroundColor( m_pLbBackgroundColor->GetSelectEntryColor() );

    // get bitmap and display it
    m_rXFSet.Put(XFillBitmapItem(OUString(), Graphic(m_pBitmapCtl->GetBitmapEx())));
    m_pCtlPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pCtlPreview->Invalidate();

    m_bPtrnChanged = true;
}


void SvxPatternTabPage::PointChanged( vcl::Window* pWindow, RECT_POINT )
{
    if( pWindow == m_pCtlPixel )
    {
        m_pBitmapCtl->SetBmpArray( m_pCtlPixel->GetBitmapPixelPtr() );

        // get bitmap and display it
        m_rXFSet.Put(XFillBitmapItem(OUString(), Graphic(m_pBitmapCtl->GetBitmapEx())));
        m_pCtlPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
        m_pCtlPreview->Invalidate();

        m_bPtrnChanged = true;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
