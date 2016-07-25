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

#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/settings.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <sfx2/filedlghelper.hxx>
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"

#include <cuires.hrc>
#include "helpid.hrc"

#include "svx/xattr.hxx"
#include <svx/xpool.hxx>
#include <svx/xtable.hxx>

#include "svx/drawitem.hxx"
#include "cuitabline.hxx"
#include "dlgname.hxx"
#include "defdlgname.hxx"
#include <svx/svxdlg.hxx>
#include <dialmgr.hxx>
#include "svx/dlgutil.hxx"
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>

#define XOUT_WIDTH    150

using namespace com::sun::star;


SvxLineDefTabPage::SvxLineDefTabPage
(
    vcl::Window* pParent,
    const SfxItemSet& rInAttrs
) :

    SfxTabPage( pParent
              , "LineStylePage"
              , "cui/ui/linestyletabpage.ui"
              , &rInAttrs ),
    rOutAttrs       ( rInAttrs ),
    bObjSelected    ( false ),

    aXLStyle            ( drawing::LineStyle_DASH ),
    aXWidth             ( XOUT_WIDTH ),
    aXDash              ( OUString(), XDash( css::drawing::DashStyle_RECT, 3, 7, 2, 40, 15 ) ),
    aXColor             ( OUString(), COL_BLACK ),
    aXLineAttr          ( rInAttrs.GetPool() ),
    rXLSet              ( aXLineAttr.GetItemSet() ),
    pnDashListState(nullptr),
    pPageType(nullptr),
    nDlgType(0),
    pPosDashLb(nullptr)
{

   get(m_pLbLineStyles   ,"LB_LINESTYLES");
   get(m_pLbType1        ,"LB_TYPE_1");
   get(m_pLbType2        ,"LB_TYPE_2");
   get(m_pNumFldNumber1  ,"NUM_FLD_1");
   get(m_pNumFldNumber2  ,"NUM_FLD_2");
   get(m_pMtrLength1     ,"MTR_FLD_LENGTH_1");
   m_pMtrLength1->SetCustomUnitText("%");
   get(m_pMtrLength2     ,"MTR_FLD_LENGTH_2");
   m_pMtrLength2->SetCustomUnitText("%");
   get(m_pMtrDistance    ,"MTR_FLD_DISTANCE");
   m_pMtrDistance->SetCustomUnitText("%");
   get(m_pCbxSynchronize ,"CBX_SYNCHRONIZE");
   get(m_pBtnAdd         ,"BTN_ADD");
   get(m_pBtnModify      ,"BTN_MODIFY");
   get(m_pBtnDelete      ,"BTN_DELETE");
   get(m_pBtnLoad        ,"BTN_LOAD");
   get(m_pBtnSave        ,"BTN_SAVE");
   get(m_pCtlPreview     ,"CTL_PREVIEW");

    // this page needs ExchangeSupport
    SetExchangeSupport();

    // adjust metric
    eFUnit = GetModuleFieldUnit( rInAttrs );

    switch ( eFUnit )
    {
        case FUNIT_M:
        case FUNIT_KM:
            eFUnit = FUNIT_MM;
            break;
        default: ; //prevent warning
    }
    SetFieldUnit( *m_pMtrDistance, eFUnit );
    SetFieldUnit( *m_pMtrLength1, eFUnit );
    SetFieldUnit( *m_pMtrLength2, eFUnit );

    // determine PoolUnit
    SfxItemPool* pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool?" );
    ePoolUnit = pPool->GetMetric( SID_ATTR_LINE_WIDTH );

    rXLSet.Put( aXLStyle );
    rXLSet.Put( aXWidth );
    rXLSet.Put( aXDash );
    rXLSet.Put( aXColor );

    // #i34740#
    m_pCtlPreview->SetLineAttributes(aXLineAttr.GetItemSet());

    m_pBtnAdd->SetClickHdl( LINK( this, SvxLineDefTabPage, ClickAddHdl_Impl ) );
    m_pBtnModify->SetClickHdl( LINK( this, SvxLineDefTabPage, ClickModifyHdl_Impl ) );
    m_pBtnDelete->SetClickHdl( LINK( this, SvxLineDefTabPage, ClickDeleteHdl_Impl ) );
    m_pBtnLoad->SetClickHdl( LINK( this, SvxLineDefTabPage, ClickLoadHdl_Impl ) );
    m_pBtnSave->SetClickHdl( LINK( this, SvxLineDefTabPage, ClickSaveHdl_Impl ) );

    m_pNumFldNumber1->SetModifyHdl( LINK( this, SvxLineDefTabPage, ChangeNumber1Hdl_Impl ) );
    m_pNumFldNumber2->SetModifyHdl( LINK( this, SvxLineDefTabPage, ChangeNumber2Hdl_Impl ) );
    m_pLbLineStyles->SetSelectHdl( LINK( this, SvxLineDefTabPage, SelectLinestyleListBoxHdl_Impl ) );

    // #i122042# switch off default adding of 'none' and 'solid' entries
    // for this ListBox; we want to select only editable/dashed styles
    m_pLbLineStyles->setAddStandardFields(false);

    // absolute (in mm) or relative (in %)
    m_pCbxSynchronize->SetClickHdl(  LINK( this, SvxLineDefTabPage, ChangeMetricHdl_Impl ) );

    // preview must be updated when there's something changed
    Link<ListBox&, void> aLink = LINK( this, SvxLineDefTabPage, SelectTypeListBoxHdl_Impl );
    m_pLbType1->SetSelectHdl( aLink );
    m_pLbType2->SetSelectHdl( aLink );
    Link<Edit&,void> aLink2 = LINK( this, SvxLineDefTabPage, ChangePreviewHdl_Impl );
    m_pMtrLength1->SetModifyHdl( aLink2 );
    m_pMtrLength2->SetModifyHdl( aLink2 );
    m_pMtrDistance->SetModifyHdl( aLink2 );

    pDashList = nullptr;
}

SvxLineDefTabPage::~SvxLineDefTabPage()
{
    disposeOnce();
}

void SvxLineDefTabPage::dispose()
{
    m_pLbLineStyles.clear();
    m_pLbType1.clear();
    m_pLbType2.clear();
    m_pNumFldNumber1.clear();
    m_pNumFldNumber2.clear();
    m_pMtrLength1.clear();
    m_pMtrLength2.clear();
    m_pMtrDistance.clear();
    m_pCbxSynchronize.clear();
    m_pBtnAdd.clear();
    m_pBtnModify.clear();
    m_pBtnDelete.clear();
    m_pBtnLoad.clear();
    m_pBtnSave.clear();
    m_pCtlPreview.clear();
    SfxTabPage::dispose();
}

void SvxLineDefTabPage::Construct()
{
    // Line style fill; do *not* add default fields here
    m_pLbLineStyles->Fill( pDashList );
}


void SvxLineDefTabPage::ActivatePage( const SfxItemSet& )
{
    if( nDlgType == 0 ) // area dialog
    {
        // ActivatePage() is called before the dialog receives PageCreated() !!!
        if( pDashList.is() )
        {
            if( *pPageType == 1 &&
                *pPosDashLb != LISTBOX_ENTRY_NOTFOUND )
            {
                m_pLbLineStyles->SelectEntryPos( *pPosDashLb );
            }
            // so that a possibly existing line style is discarded
            SelectLinestyleHdl_Impl( nullptr );

            // determining (and possibly cutting) the name
            // and displaying it in the GroupBox
//             OUString        aString( CUI_RES( RID_SVXSTR_TABLE ) );
//             aString         += ": ";
            INetURLObject   aURL( pDashList->GetPath() );

            aURL.Append( pDashList->GetName() );
            DBG_ASSERT( aURL.GetProtocol() != INetProtocol::NotValid, "invalid URL" );

            *pPageType = 0; // 2
            *pPosDashLb = LISTBOX_ENTRY_NOTFOUND;
        }
    }
}


DeactivateRC SvxLineDefTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    CheckChanges_Impl();

    if( _pSet )
        FillItemSet( _pSet );

    return DeactivateRC::LeavePage;
}


void SvxLineDefTabPage::CheckChanges_Impl()
{
    // is here used to NOT lose changes
    //css::drawing::DashStyle eXDS;

    if( m_pNumFldNumber1->IsValueChangedFromSaved() ||
        m_pMtrLength1->IsValueChangedFromSaved() ||
        m_pLbType1->IsValueChangedFromSaved() ||
        m_pNumFldNumber2->IsValueChangedFromSaved() ||
        m_pMtrLength2->IsValueChangedFromSaved() ||
        m_pLbType2->IsValueChangedFromSaved() ||
        m_pMtrDistance->IsValueChangedFromSaved() )
    {
        ResMgr& rMgr = CUI_MGR();
        Image aWarningBoxImage = WarningBox::GetStandardImage();
        ScopedVclPtrInstance<SvxMessDialog> aMessDlg( GetParentDialog(),
                                                      SVX_RESSTR( RID_SVXSTR_LINESTYLE ),
                                                      OUString( ResId( RID_SVXSTR_ASK_CHANGE_LINESTYLE, rMgr ) ),
                                                      &aWarningBoxImage );
        DBG_ASSERT(aMessDlg, "Dialog creation failed!");
        aMessDlg->SetButtonText( SvxMessDialogButton::N1, OUString( ResId( RID_SVXSTR_CHANGE, rMgr ) ) );
        aMessDlg->SetButtonText( SvxMessDialogButton::N2, OUString( ResId( RID_SVXSTR_ADD, rMgr ) ) );

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


    sal_Int32 nPos = m_pLbLineStyles->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        *pPosDashLb = nPos;
    }
}


bool SvxLineDefTabPage::FillItemSet( SfxItemSet* rAttrs )
{
    if( nDlgType == 0 ) // line dialog
    {
        if( *pPageType == 2 )
        {
            FillDash_Impl();

            OUString aString( m_pLbLineStyles->GetSelectEntry() );
            rAttrs->Put( XLineStyleItem( drawing::LineStyle_DASH ) );
            rAttrs->Put( XLineDashItem( aString, aDash ) );
        }
    }
    return true;
}


void SvxLineDefTabPage::Reset( const SfxItemSet* rAttrs )
{
    if( rAttrs->GetItemState( GetWhich( XATTR_LINESTYLE ) ) != SfxItemState::DONTCARE )
    {
        drawing::LineStyle eXLS = (drawing::LineStyle) static_cast<const XLineStyleItem&>( rAttrs->Get( GetWhich( XATTR_LINESTYLE ) ) ).GetValue();

        switch( eXLS )
        {
            case drawing::LineStyle_NONE:
            case drawing::LineStyle_SOLID:
                m_pLbLineStyles->SelectEntryPos( 0 );
                break;

            case drawing::LineStyle_DASH:
            {
                const XLineDashItem& rDashItem = static_cast<const XLineDashItem&>( rAttrs->Get( XATTR_LINEDASH ) );
                aDash = rDashItem.GetDashValue();

                m_pLbLineStyles->SetNoSelection();
                m_pLbLineStyles->SelectEntry( rDashItem.GetName() );
            }
                break;

            default:
                break;
        }
    }
    SelectLinestyleHdl_Impl( nullptr );

    // determine button state
    if( pDashList->Count() )
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


VclPtr<SfxTabPage> SvxLineDefTabPage::Create( vcl::Window* pWindow, const SfxItemSet* rOutAttrs )
{
    return VclPtr<SvxLineDefTabPage>::Create( pWindow, *rOutAttrs );
}


IMPL_LINK_TYPED( SvxLineDefTabPage, SelectLinestyleListBoxHdl_Impl, ListBox&, rListBox, void )
{
    SelectLinestyleHdl_Impl(&rListBox);
}

void SvxLineDefTabPage::SelectLinestyleHdl_Impl(ListBox* p)
{
    if(pDashList->Count())
    {
        int nTmp = m_pLbLineStyles->GetSelectEntryPos();

        if(LISTBOX_ENTRY_NOTFOUND == nTmp)
        {
            OSL_ENSURE(false, "OOps, non-existent LineDash selected (!)");
            nTmp = 1;
        }

        aDash = pDashList->GetDash( nTmp )->GetDash();

        FillDialog_Impl();

        rXLSet.Put( XLineDashItem( OUString(), aDash ) );

        // #i34740#
        m_pCtlPreview->SetLineAttributes(aXLineAttr.GetItemSet());

        m_pCtlPreview->Invalidate();

        // Is not set before, in order to take the new style
        // only if there was an entry selected in the ListBox.
        // If it was called via Reset(), then p is == NULL
        if( p )
            *pPageType = 2;
    }
}


IMPL_LINK_NOARG_TYPED(SvxLineDefTabPage, ChangePreviewHdl_Impl, Edit&, void)
{
    FillDash_Impl();
    m_pCtlPreview->Invalidate();
}

IMPL_LINK_NOARG_TYPED(SvxLineDefTabPage, ChangeNumber1Hdl_Impl, Edit&, void)
{
    if( m_pNumFldNumber1->GetValue() == 0L )
    {
        m_pNumFldNumber2->SetMin( 1L );
        m_pNumFldNumber2->SetFirst( 1L );
    }
    else
    {
        m_pNumFldNumber2->SetMin( 0L );
        m_pNumFldNumber2->SetFirst( 0L );
    }

    ChangePreviewHdl_Impl( *m_pMtrLength1 );
}


IMPL_LINK_NOARG_TYPED(SvxLineDefTabPage, ChangeNumber2Hdl_Impl, Edit&, void)
{
    if( m_pNumFldNumber2->GetValue() == 0L )
    {
        m_pNumFldNumber1->SetMin( 1L );
        m_pNumFldNumber1->SetFirst( 1L );
    }
    else
    {
        m_pNumFldNumber1->SetMin( 0L );
        m_pNumFldNumber1->SetFirst( 0L );
    }

    ChangePreviewHdl_Impl( *m_pMtrLength1 );
}


IMPL_LINK_TYPED( SvxLineDefTabPage, ChangeMetricHdl_Impl, Button*, p, void )
{
    if( !m_pCbxSynchronize->IsChecked() && m_pMtrLength1->GetUnit() != eFUnit )
    {
        long nTmp1, nTmp2, nTmp3;

        // was changed with Control
        if( p )
        {
            nTmp1 = GetCoreValue( *m_pMtrLength1, ePoolUnit ) * XOUT_WIDTH / 100;
            nTmp2 = GetCoreValue( *m_pMtrLength2, ePoolUnit ) * XOUT_WIDTH / 100;
            nTmp3 = GetCoreValue( *m_pMtrDistance, ePoolUnit ) * XOUT_WIDTH / 100;
        }
        else
        {
            nTmp1 = GetCoreValue( *m_pMtrLength1, ePoolUnit );
            nTmp2 = GetCoreValue( *m_pMtrLength2, ePoolUnit );
            nTmp3 = GetCoreValue( *m_pMtrDistance, ePoolUnit );
        }
        m_pMtrLength1->SetDecimalDigits( 2 );
        m_pMtrLength2->SetDecimalDigits( 2 );
        m_pMtrDistance->SetDecimalDigits( 2 );

        // adjust metric
        m_pMtrLength1->SetUnit( eFUnit );
        m_pMtrLength2->SetUnit( eFUnit );
        m_pMtrDistance->SetUnit( eFUnit );

        SetMetricValue( *m_pMtrLength1, nTmp1, ePoolUnit );
        SetMetricValue( *m_pMtrLength2, nTmp2, ePoolUnit );
        SetMetricValue( *m_pMtrDistance, nTmp3, ePoolUnit );
    }
    else if( m_pCbxSynchronize->IsChecked() && m_pMtrLength1->GetUnit() != FUNIT_PERCENT )
    {
        long nTmp1, nTmp2, nTmp3;

        // was changed with Control
        if( p )
        {
            nTmp1 = GetCoreValue( *m_pMtrLength1, ePoolUnit ) * 100 / XOUT_WIDTH;
            nTmp2 = GetCoreValue( *m_pMtrLength2, ePoolUnit ) * 100 / XOUT_WIDTH;
            nTmp3 = GetCoreValue( *m_pMtrDistance, ePoolUnit ) * 100 / XOUT_WIDTH;
        }
        else
        {
            nTmp1 = GetCoreValue( *m_pMtrLength1, ePoolUnit );
            nTmp2 = GetCoreValue( *m_pMtrLength2, ePoolUnit );
            nTmp3 = GetCoreValue( *m_pMtrDistance, ePoolUnit );
        }

        m_pMtrLength1->SetDecimalDigits( 0 );
        m_pMtrLength2->SetDecimalDigits( 0 );
        m_pMtrDistance->SetDecimalDigits( 0 );

        m_pMtrLength1->SetUnit( FUNIT_PERCENT );
        m_pMtrLength2->SetUnit( FUNIT_PERCENT );
        m_pMtrDistance->SetUnit( FUNIT_PERCENT );

        SetMetricValue( *m_pMtrLength1, nTmp1, ePoolUnit );
        SetMetricValue( *m_pMtrLength2, nTmp2, ePoolUnit );
        SetMetricValue( *m_pMtrDistance, nTmp3, ePoolUnit );

    }
    SelectTypeHdl_Impl( nullptr );
}


IMPL_LINK_TYPED( SvxLineDefTabPage, SelectTypeListBoxHdl_Impl, ListBox&, rListBox, void )
{
    SelectTypeHdl_Impl(&rListBox);
}

void  SvxLineDefTabPage::SelectTypeHdl_Impl(ListBox* p)
{
    if ( p == m_pLbType1 || !p )
    {
        if ( m_pLbType1->GetSelectEntryPos() == 0 )
        {
            m_pMtrLength1->Disable();
            m_pMtrLength1->SetText( "" );
        }
        else if ( !m_pMtrLength1->IsEnabled() )
        {
            m_pMtrLength1->Enable();
            m_pMtrLength1->Reformat();
        }
    }

    if ( p == m_pLbType2 || !p )
    {
        if ( m_pLbType2->GetSelectEntryPos() == 0 )
        {
            m_pMtrLength2->Disable();
            m_pMtrLength2->SetText( "" );
        }
        else if ( !m_pMtrLength2->IsEnabled() )
        {
            m_pMtrLength2->Enable();
            m_pMtrLength2->Reformat();
        }
    }
    ChangePreviewHdl_Impl( *m_pMtrLength1 );
}


IMPL_LINK_NOARG_TYPED(SvxLineDefTabPage, ClickAddHdl_Impl, Button*, void)
{
    ResMgr& rMgr = CUI_MGR();
    OUString aNewName( SVX_RES( RID_SVXSTR_LINESTYLE ) );
    OUString aDesc( ResId( RID_SVXSTR_DESC_LINESTYLE, rMgr ) );
    OUString aName;
    XDashEntry* pEntry;

    long nCount = pDashList->Count();
    long j = 1;
    bool bDifferent = false;

    while ( !bDifferent )
    {
        aName = aNewName + " " + OUString::number( j++ );
        bDifferent = true;

        for ( long i = 0; i < nCount && bDifferent; i++ )
            if ( aName == pDashList->GetDash( i )->GetName() )
                bDifferent = false;
    }

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialog creation failed!");
    std::unique_ptr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc ));
    DBG_ASSERT(pDlg, "Dialog creation failed!");
    bool bLoop = true;

    while ( bLoop && pDlg->Execute() == RET_OK )
    {
        pDlg->GetName( aName );
        bDifferent = true;

        for( long i = 0; i < nCount && bDifferent; i++ )
        {
            if( aName == pDashList->GetDash( i )->GetName() )
                bDifferent = false;
        }

        if( bDifferent )
        {
            bLoop = false;
            FillDash_Impl();

            pEntry = new XDashEntry( aDash, aName );

            long nDashCount = pDashList->Count();
            pDashList->Insert( pEntry, nDashCount );
            const Bitmap aBitmap = pDashList->GetUiBitmap( nDashCount );
            m_pLbLineStyles->Append( *pEntry, pDashList->GetUiBitmap( nDashCount ) );

            m_pLbLineStyles->SelectEntryPos( m_pLbLineStyles->GetEntryCount() - 1 );

            *pnDashListState |= ChangeType::MODIFIED;

            *pPageType = 2;

            // save values for changes recognition (-> method)
            m_pNumFldNumber1->SaveValue();
            m_pMtrLength1->SaveValue();
            m_pLbType1->SaveValue();
            m_pNumFldNumber2->SaveValue();
            m_pMtrLength2->SaveValue();
            m_pLbType2->SaveValue();
            m_pMtrDistance->SaveValue();
        }
        else
        {
            ScopedVclPtrInstance<MessageDialog> aBox( GetParentDialog()
                                                      ,"DuplicateNameDialog"
                                                      ,"cui/ui/queryduplicatedialog.ui" );
            aBox->Execute();
        }
    }
    pDlg.reset();

    // determine button state
    if ( pDashList->Count() )
    {
        m_pBtnModify->Enable();
        m_pBtnDelete->Enable();
        m_pBtnSave->Enable();
    }
}


IMPL_LINK_NOARG_TYPED(SvxLineDefTabPage, ClickModifyHdl_Impl, Button*, void)
{
    sal_Int32 nPos = m_pLbLineStyles->GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        ResMgr& rMgr = CUI_MGR();
        OUString aDesc( ResId( RID_SVXSTR_DESC_LINESTYLE, rMgr ) );
        OUString aName( pDashList->GetDash( nPos )->GetName() );
        OUString aOldName = aName;

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialog creation failed!");
        std::unique_ptr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc ));
        DBG_ASSERT(pDlg, "Dialog creation failed!");

        long nCount = pDashList->Count();
        bool bLoop = true;

        while ( bLoop && pDlg->Execute() == RET_OK )
        {
            pDlg->GetName( aName );
            bool bDifferent = true;

            for( long i = 0; i < nCount && bDifferent; i++ )
            {
                if( aName == pDashList->GetDash( i )->GetName() &&
                    aName != aOldName )
                    bDifferent = false;
            }

            if ( bDifferent )
            {
                bLoop = false;
                FillDash_Impl();

                XDashEntry* pEntry = new XDashEntry( aDash, aName );

                delete pDashList->Replace( pEntry, nPos );
                m_pLbLineStyles->Modify( *pEntry, nPos, pDashList->GetUiBitmap( nPos ) );

                m_pLbLineStyles->SelectEntryPos( nPos );

                *pnDashListState |= ChangeType::MODIFIED;

                *pPageType = 2;

                // save values for changes recognition (-> method)
                m_pNumFldNumber1->SaveValue();
                m_pMtrLength1->SaveValue();
                m_pLbType1->SaveValue();
                m_pNumFldNumber2->SaveValue();
                m_pMtrLength2->SaveValue();
                m_pLbType2->SaveValue();
                m_pMtrDistance->SaveValue();
            }
            else
            {
                ScopedVclPtrInstance<MessageDialog> aBox( GetParentDialog()
                                                          ,"DuplicateNameDialog"
                                                          ,"cui/ui/queryduplicatedialog.ui" );
                aBox->Execute();
            }
        }
    }
}


IMPL_LINK_NOARG_TYPED(SvxLineDefTabPage, ClickDeleteHdl_Impl, Button*, void)
{
    sal_Int32 nPos = m_pLbLineStyles->GetSelectEntryPos();

    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        ScopedVclPtrInstance<MessageDialog> aQueryBox( GetParentDialog()
                                                       ,"AskDelLineStyleDialog"
                                                       ,"cui/ui/querydeletelinestyledialog.ui" );

        if ( aQueryBox->Execute() == RET_YES )
        {
            delete pDashList->Remove( nPos );
            m_pLbLineStyles->RemoveEntry( nPos );
            m_pLbLineStyles->SelectEntryPos( 0 );

            SelectLinestyleHdl_Impl( nullptr );
            *pPageType = 0; // style should not be taken

            *pnDashListState |= ChangeType::MODIFIED;

            ChangePreviewHdl_Impl( *m_pMtrLength1 );
        }
    }

    // determine button state
    if ( !pDashList->Count() )
    {
        m_pBtnModify->Disable();
        m_pBtnDelete->Disable();
        m_pBtnSave->Disable();
    }
}


IMPL_LINK_NOARG_TYPED(SvxLineDefTabPage, ClickLoadHdl_Impl, Button*, void)
{
    sal_uInt16 nReturn = RET_YES;

    if ( *pnDashListState & ChangeType::MODIFIED )
    {
        nReturn = ScopedVclPtrInstance<MessageDialog>::Create( GetParentDialog()
                                ,"AskSaveList"
                                ,"cui/ui/querysavelistdialog.ui")->Execute();

        if ( nReturn == RET_YES )
            pDashList->Save();
    }

    if ( nReturn != RET_CANCEL )
    {
        ::sfx2::FileDialogHelper aDlg( css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE );
        OUString aStrFilterType( "*.sod" );
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

        if( aDlg.Execute() == ERRCODE_NONE )
        {
            INetURLObject aURL( aDlg.GetPath() );
            INetURLObject aPathURL( aURL );

            aPathURL.removeSegment();
            aPathURL.removeFinalSlash();

            XDashListRef pDshLst = XPropertyList::AsDashList(XPropertyList::CreatePropertyList( XDASH_LIST, aPathURL.GetMainURL( INetURLObject::NO_DECODE ), "" ));
            pDshLst->SetName( aURL.getName() );

            if( pDshLst->Load() )
            {
                pDashList = pDshLst;
                static_cast<SvxLineTabDialog*>( GetParentDialog() )->SetNewDashList( pDashList );

                m_pLbLineStyles->Clear();
                m_pLbLineStyles->Fill( pDashList );
                Reset( &rOutAttrs );

                pDashList->SetName( aURL.getName() );

                *pnDashListState |= ChangeType::CHANGED;
                *pnDashListState &= ~ChangeType::MODIFIED;
            }
            else
                //aIStream.Close();
                ScopedVclPtrInstance<MessageDialog>::Create( GetParentDialog()
                              ,"NoLoadedFileDialog"
                              ,"cui/ui/querynoloadedfiledialog.ui")->Execute();
        }
    }

    // determine button state
    if ( pDashList->Count() )
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


IMPL_LINK_NOARG_TYPED(SvxLineDefTabPage, ClickSaveHdl_Impl, Button*, void)
{
    ::sfx2::FileDialogHelper aDlg( css::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE );
    OUString aStrFilterType( "*.sod" );
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
    DBG_ASSERT( aFile.GetProtocol() != INetProtocol::NotValid, "invalid URL" );

    if( !pDashList->GetName().isEmpty() )
    {
        aFile.Append( pDashList->GetName() );

        if( aFile.getExtension().isEmpty() )
            aFile.SetExtension( "sod" );
    }

    aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );
    if ( aDlg.Execute() == ERRCODE_NONE )
    {
        INetURLObject aURL( aDlg.GetPath() );
        INetURLObject aPathURL( aURL );

        aPathURL.removeSegment();
        aPathURL.removeFinalSlash();

        pDashList->SetName( aURL.getName() );
        pDashList->SetPath( aPathURL.GetMainURL( INetURLObject::NO_DECODE ) );

        if( pDashList->Save() )
        {
            *pnDashListState |= ChangeType::SAVED;
            *pnDashListState &= ~ChangeType::MODIFIED;
        }
        else
        {
            ScopedVclPtrInstance<MessageDialog>::Create( GetParentDialog()
                         ,"NoSaveFileDialog"
                         ,"cui/ui/querynosavefiledialog.ui")->Execute();
        }
    }
}


void SvxLineDefTabPage::FillDash_Impl()
{
    css::drawing::DashStyle eXDS;

    if( m_pCbxSynchronize->IsChecked() )
        eXDS = css::drawing::DashStyle_RECTRELATIVE;
    else
        eXDS = css::drawing::DashStyle_RECT;

    aDash.SetDashStyle( eXDS );
    aDash.SetDots( (sal_uInt8) m_pNumFldNumber1->GetValue() );
    aDash.SetDotLen( m_pLbType1->GetSelectEntryPos() == 0 ? 0 : GetCoreValue( *m_pMtrLength1, ePoolUnit ) );
    aDash.SetDashes( (sal_uInt8) m_pNumFldNumber2->GetValue() );
    aDash.SetDashLen( m_pLbType2->GetSelectEntryPos() == 0 ? 0 : GetCoreValue( *m_pMtrLength2, ePoolUnit ) );
    aDash.SetDistance( GetCoreValue( *m_pMtrDistance, ePoolUnit ) );

    rXLSet.Put( XLineDashItem( OUString(), aDash ) );

    // #i34740#
    m_pCtlPreview->SetLineAttributes(aXLineAttr.GetItemSet());
}


void SvxLineDefTabPage::FillDialog_Impl()
{
    css::drawing::DashStyle eXDS = aDash.GetDashStyle(); // css::drawing::DashStyle_RECT, css::drawing::DashStyle_ROUND
    if( eXDS == css::drawing::DashStyle_RECTRELATIVE )
        m_pCbxSynchronize->Check();
    else
        m_pCbxSynchronize->Check( false );

    m_pNumFldNumber1->SetValue( aDash.GetDots() );
    SetMetricValue( *m_pMtrLength1, aDash.GetDotLen(), ePoolUnit );
    m_pLbType1->SelectEntryPos( aDash.GetDotLen() == 0 ? 0 : 1 );
    m_pNumFldNumber2->SetValue( aDash.GetDashes() );
    SetMetricValue( *m_pMtrLength2, aDash.GetDashLen(), ePoolUnit );
    m_pLbType2->SelectEntryPos( aDash.GetDashLen() == 0 ? 0 : 1 );
    SetMetricValue( *m_pMtrDistance, aDash.GetDistance(), ePoolUnit );

    ChangeMetricHdl_Impl( nullptr );

    // save values for changes recognition (-> method)
    m_pNumFldNumber1->SaveValue();
    m_pMtrLength1->SaveValue();
    m_pLbType1->SaveValue();
    m_pNumFldNumber2->SaveValue();
    m_pMtrLength2->SaveValue();
    m_pLbType2->SaveValue();
    m_pMtrDistance->SaveValue();
}


void SvxLineDefTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxTabPage::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        sal_Int32 nOldSelect = m_pLbLineStyles->GetSelectEntryPos();
        m_pLbLineStyles->Clear();
        m_pLbLineStyles->Fill( pDashList );
        m_pLbLineStyles->SelectEntryPos( nOldSelect );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
