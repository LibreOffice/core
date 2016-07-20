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
#include <sfx2/dialoghelper.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/module.hxx>
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"

#include <cuires.hrc>
#include "helpid.hrc"
#include "svx/xattr.hxx"
#include <svx/xpool.hxx>
#include <svx/xtable.hxx>
#include "svx/drawitem.hxx"
#include "cuitabarea.hxx"
#include "defdlgname.hxx"
#include "dlgname.hxx"
#include <svx/svxdlg.hxx>
#include <dialmgr.hxx>
#include "svx/dlgutil.hxx"
#include <svx/dialmgr.hxx>
#include "paragrph.hrc"
#include <svx/dialogs.hrc>

using namespace com::sun::star;

SvxHatchTabPage::SvxHatchTabPage
(
    vcl::Window* pParent,
    const SfxItemSet& rInAttrs
) :

    SvxTabPage          ( pParent, "HatchPage", "cui/ui/hatchpage.ui", rInAttrs ),

    m_rOutAttrs           ( rInAttrs ),

    m_pnHatchingListState ( nullptr ),
    m_pnColorListState    ( nullptr ),
    m_pPageType           ( nullptr ),
    m_nDlgType            ( 0 ),
    m_pPos                ( nullptr ),
    m_pbAreaTP            ( nullptr ),

    m_aXFStyleItem        ( drawing::FillStyle_HATCH ),
    m_aXHatchItem         ( OUString(), XHatch() ),
    m_aXFillAttr          ( rInAttrs.GetPool() ),
    m_rXFSet              ( m_aXFillAttr.GetItemSet() )

{
    get(m_pMtrDistance, "distancemtr");
    get(m_pMtrAngle, "anglemtr");
    get(m_pCtlAngle, "anglectl");
    m_pCtlAngle->SetCS(CS_ANGLE);
    get(m_pLbLineType, "linetypelb");
    get(m_pLbLineColor, "linecolorlb");
    get(m_pLbHatchings, "hatchingslb");
    Size aSize = getDrawListBoxOptimalSize(this);
    m_pLbHatchings->set_width_request(aSize.Width());
    m_pLbHatchings->set_height_request(aSize.Height());
    get(m_pCtlPreview, "previewctl");
    aSize = getDrawPreviewOptimalSize(this);
    m_pCtlPreview->set_width_request(aSize.Width());
    m_pCtlPreview->set_height_request(aSize.Height());
    get(m_pBtnAdd, "add");
    get(m_pBtnModify, "modify");
    get(m_pBtnDelete, "delete");
    get(m_pBtnLoad, "load");
    get(m_pBtnSave, "save");

    // this page needs ExchangeSupport
    SetExchangeSupport();

    // adjust metric
    FieldUnit eFUnit = GetModuleFieldUnit( rInAttrs );

    switch ( eFUnit )
    {
        case FUNIT_M:
        case FUNIT_KM:
            eFUnit = FUNIT_MM;
            break;
        default: ;//prevent warning
    }
    SetFieldUnit( *m_pMtrDistance, eFUnit );

    // determine PoolUnit
    SfxItemPool* pPool = m_rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool?" );
    m_ePoolUnit = pPool->GetMetric( SID_ATTR_FILL_HATCH );

    // setting the output device
    m_rXFSet.Put( m_aXFStyleItem );
    m_rXFSet.Put( m_aXHatchItem );
    m_pCtlPreview->SetAttributes( m_aXFillAttr.GetItemSet() );

    m_pLbHatchings->SetSelectHdl( LINK( this, SvxHatchTabPage, ChangeHatchHdl_Impl ) );

    Link<Edit&,void> aLink = LINK( this, SvxHatchTabPage, ModifiedEditHdl_Impl );
    Link<ListBox&,void> aLink2 = LINK( this, SvxHatchTabPage, ModifiedListBoxHdl_Impl );
    m_pMtrDistance->SetModifyHdl( aLink );
    m_pMtrAngle->SetModifyHdl( aLink );
    m_pLbLineType->SetSelectHdl( aLink2 );
    m_pLbLineColor->SetSelectHdl( aLink2 );

    m_pBtnAdd->SetClickHdl( LINK( this, SvxHatchTabPage, ClickAddHdl_Impl ) );
    m_pBtnModify->SetClickHdl(
        LINK( this, SvxHatchTabPage, ClickModifyHdl_Impl ) );
    m_pBtnDelete->SetClickHdl(
        LINK( this, SvxHatchTabPage, ClickDeleteHdl_Impl ) );
    m_pBtnLoad->SetClickHdl( LINK( this, SvxHatchTabPage, ClickLoadHdl_Impl ) );
    m_pBtnSave->SetClickHdl( LINK( this, SvxHatchTabPage, ClickSaveHdl_Impl ) );

    m_pCtlPreview->SetDrawMode( GetSettings().GetStyleSettings().GetHighContrastMode() ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR );

    setPreviewsToSamePlace(pParent, this);
}

SvxHatchTabPage::~SvxHatchTabPage()
{
    disposeOnce();
}

void SvxHatchTabPage::dispose()
{
    m_pMtrDistance.clear();
    m_pMtrAngle.clear();
    m_pCtlAngle.clear();
    m_pLbLineType.clear();
    m_pLbLineColor.clear();
    m_pLbHatchings.clear();
    m_pCtlPreview.clear();
    m_pBtnAdd.clear();
    m_pBtnModify.clear();
    m_pBtnDelete.clear();
    m_pBtnLoad.clear();
    m_pBtnSave.clear();
    SvxTabPage::dispose();
}


void SvxHatchTabPage::Construct()
{
    m_pLbLineColor->Fill( m_pColorList );
    m_pLbHatchings->Fill( m_pHatchingList );
}


void SvxHatchTabPage::ActivatePage( const SfxItemSet& rSet )
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

                // LbLineColor
                nPos = m_pLbLineColor->GetSelectEntryPos();
                m_pLbLineColor->Clear();
                m_pLbLineColor->Fill( m_pColorList );
                nCount = m_pLbLineColor->GetEntryCount();
                if( nCount == 0 )
                    ; // this case should not occur
                else if( nCount <= nPos )
                    m_pLbLineColor->SelectEntryPos( 0 );
                else
                    m_pLbLineColor->SelectEntryPos( nPos );

                ModifiedHdl_Impl( this );
            }

            // determining (possibly cutting) the name
            // and displaying it in the GroupBox
            OUString        aString( CUI_RES( RID_SVXSTR_TABLE ) );
            aString         += ": ";
            INetURLObject   aURL( m_pHatchingList->GetPath() );

            aURL.Append( m_pHatchingList->GetName() );
            DBG_ASSERT( aURL.GetProtocol() != INetProtocol::NotValid, "invalid URL" );

            if ( aURL.getBase().getLength() > 18 )
            {
                aString += aURL.getBase().copy( 0, 15 );
                aString += "...";
            }
            else
                aString += aURL.getBase();

            if( *m_pPageType == PT_HATCH && *m_pPos != LISTBOX_ENTRY_NOTFOUND )
            {
                m_pLbHatchings->SelectEntryPos( *m_pPos );
            }
            // colors could have been deleted
            ChangeHatchHdl_Impl( *m_pLbHatchings );

            *m_pPageType = PT_HATCH;
            *m_pPos = LISTBOX_ENTRY_NOTFOUND;
        }
    }

    m_rXFSet.Put( static_cast<const XFillColorItem&>(     rSet.Get(XATTR_FILLCOLOR)) );
    m_rXFSet.Put( static_cast<const XFillBackgroundItem&>(rSet.Get(XATTR_FILLBACKGROUND)) );
    m_pCtlPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pCtlPreview->Invalidate();
}


SfxTabPage::sfxpg SvxHatchTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( CheckChanges_Impl() == -1L )
        return KEEP_PAGE;

    if( _pSet )
        FillItemSet( _pSet );

    return LEAVE_PAGE;
}


long SvxHatchTabPage::CheckChanges_Impl()
{
    if( m_pMtrDistance->IsValueChangedFromSaved() ||
        m_pMtrAngle->IsValueChangedFromSaved() ||
        m_pLbLineType->IsValueChangedFromSaved()  ||
        m_pLbLineColor->IsValueChangedFromSaved() ||
        m_pLbHatchings->IsValueChangedFromSaved() )
    {
        ResMgr& rMgr = CUI_MGR();
        Image aWarningBoxImage = WarningBox::GetStandardImage();
        ScopedVclPtrInstance<SvxMessDialog> aMessDlg( GetParentDialog(),
                                                      SVX_RESSTR( RID_SVXSTR_HATCH ),
                                                      CUI_RESSTR( RID_SVXSTR_ASK_CHANGE_HATCH ),
                                                      &aWarningBoxImage );
        DBG_ASSERT(aMessDlg, "Dialog creation failed!");
        aMessDlg->SetButtonText( SvxMessDialogButton::N1,
                                OUString( ResId( RID_SVXSTR_CHANGE, rMgr ) ) );
        aMessDlg->SetButtonText( SvxMessDialogButton::N2,
                                OUString( ResId( RID_SVXSTR_ADD, rMgr ) ) );

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

    sal_Int32 nPos = m_pLbHatchings->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
        *m_pPos = nPos;
    return 0L;
}


bool SvxHatchTabPage::FillItemSet( SfxItemSet* rSet )
{
    if( m_nDlgType == 0 && !*m_pbAreaTP ) // area dialog
    {
        if( *m_pPageType == PT_HATCH )
        {
            // CheckChanges(); <-- duplicate inquiry ?

            std::unique_ptr<XHatch> pXHatch;
            OUString  aString;
            sal_Int32  nPos = m_pLbHatchings->GetSelectEntryPos();
            if( nPos != LISTBOX_ENTRY_NOTFOUND )
            {
                pXHatch.reset(new XHatch( m_pHatchingList->GetHatch( nPos )->GetHatch() ));
                aString = m_pLbHatchings->GetSelectEntry();
            }
            // gradient has been (unidentifiedly) passed
            else
            {
                pXHatch.reset(new XHatch( m_pLbLineColor->GetSelectEntryColor(),
                                 (css::drawing::HatchStyle) m_pLbLineType->GetSelectEntryPos(),
                                 GetCoreValue( *m_pMtrDistance, m_ePoolUnit ),
                                 static_cast<long>(m_pMtrAngle->GetValue() * 10) ));
            }
            DBG_ASSERT( pXHatch, "XHatch konnte nicht erzeugt werden" );
            rSet->Put( XFillStyleItem( drawing::FillStyle_HATCH ) );
            rSet->Put( XFillHatchItem( aString, *pXHatch ) );
        }
    }
    return true;
}


void SvxHatchTabPage::Reset( const SfxItemSet* rSet )
{
    ChangeHatchHdl_Impl( *m_pLbHatchings );

    // determine button state
    if( m_pHatchingList->Count() )
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

    m_rXFSet.Put( static_cast<const XFillColorItem&>(     rSet->Get(XATTR_FILLCOLOR)) );
    m_rXFSet.Put( static_cast<const XFillBackgroundItem&>(rSet->Get(XATTR_FILLBACKGROUND)) );
    m_pCtlPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pCtlPreview->Invalidate();
}


VclPtr<SfxTabPage> SvxHatchTabPage::Create( vcl::Window* pWindow,
                                            const SfxItemSet* rSet )
{
    return VclPtr<SvxHatchTabPage>::Create( pWindow, *rSet );
}


IMPL_LINK_TYPED( SvxHatchTabPage, ModifiedListBoxHdl_Impl, ListBox&, rListBox, void )
{
    ModifiedHdl_Impl(&rListBox);
}
IMPL_LINK_TYPED( SvxHatchTabPage, ModifiedEditHdl_Impl, Edit&, rEdit, void )
{
    ModifiedHdl_Impl(&rEdit);
}
void SvxHatchTabPage::ModifiedHdl_Impl( void* p )
{
    if( p == m_pMtrAngle )
    {
        switch( m_pMtrAngle->GetValue() )
        {
            case 135: m_pCtlAngle->SetActualRP( RP_LT ); break;
            case  90: m_pCtlAngle->SetActualRP( RP_MT ); break;
            case  45: m_pCtlAngle->SetActualRP( RP_RT ); break;
            case 180: m_pCtlAngle->SetActualRP( RP_LM ); break;
            case   0: m_pCtlAngle->SetActualRP( RP_RM ); break;
            case 225: m_pCtlAngle->SetActualRP( RP_LB ); break;
            case 270: m_pCtlAngle->SetActualRP( RP_MB ); break;
            case 315: m_pCtlAngle->SetActualRP( RP_RB ); break;
            default:  m_pCtlAngle->SetActualRP( RP_MM ); break;
        }
    }

    XHatch aXHatch( m_pLbLineColor->GetSelectEntryColor(),
                    (css::drawing::HatchStyle) m_pLbLineType->GetSelectEntryPos(),
                    GetCoreValue( *m_pMtrDistance, m_ePoolUnit ),
                    static_cast<long>(m_pMtrAngle->GetValue() * 10) );

    m_rXFSet.Put( XFillHatchItem( OUString(), aXHatch ) );
    m_pCtlPreview->SetAttributes( m_aXFillAttr.GetItemSet() );

    m_pCtlPreview->Invalidate();
}


IMPL_LINK_NOARG_TYPED(SvxHatchTabPage, ChangeHatchHdl_Impl, ListBox&, void)
{
    std::unique_ptr<XHatch> pHatch;
    int nPos = m_pLbHatchings->GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
        pHatch.reset(new XHatch( m_pHatchingList->GetHatch( nPos )->GetHatch() ));
    else
    {
        const SfxPoolItem* pPoolItem = nullptr;
        if( SfxItemState::SET == m_rOutAttrs.GetItemState( GetWhich( XATTR_FILLSTYLE ), true, &pPoolItem ) )
        {
            if( ( drawing::FillStyle_HATCH == (drawing::FillStyle) static_cast<const XFillStyleItem*>( pPoolItem )->GetValue() ) &&
                ( SfxItemState::SET == m_rOutAttrs.GetItemState( GetWhich( XATTR_FILLHATCH ), true, &pPoolItem ) ) )
            {
                pHatch.reset(new XHatch( static_cast<const XFillHatchItem*>( pPoolItem )->GetHatchValue() ));
            }
        }
        if( !pHatch )
        {
            m_pLbHatchings->SelectEntryPos( 0 );
            nPos = m_pLbHatchings->GetSelectEntryPos();
            if( nPos != LISTBOX_ENTRY_NOTFOUND )
                pHatch.reset(new XHatch( m_pHatchingList->GetHatch( nPos )->GetHatch() ));
        }
    }
    if( pHatch )
    {
        m_pLbLineType->SelectEntryPos(
            sal::static_int_cast< sal_Int32 >( pHatch->GetHatchStyle() ) );
        // if the entry is not in the listbox
        // the color is added temporarily
        m_pLbLineColor->SetNoSelection();
        m_pLbLineColor->SelectEntry( pHatch->GetColor() );
        if( m_pLbLineColor->GetSelectEntryCount() == 0 )
        {
            m_pLbLineColor->InsertEntry( pHatch->GetColor(), OUString() );
            m_pLbLineColor->SelectEntry( pHatch->GetColor() );
        }
        SetMetricValue( *m_pMtrDistance, pHatch->GetDistance(), m_ePoolUnit );
        m_pMtrAngle->SetValue( pHatch->GetAngle() / 10 );

        switch( m_pMtrAngle->GetValue() )
        {
            case 135: m_pCtlAngle->SetActualRP( RP_LT ); break;
            case  90: m_pCtlAngle->SetActualRP( RP_MT ); break;
            case  45: m_pCtlAngle->SetActualRP( RP_RT ); break;
            case 180: m_pCtlAngle->SetActualRP( RP_LM ); break;
            case   0: m_pCtlAngle->SetActualRP( RP_RM ); break;
            case 225: m_pCtlAngle->SetActualRP( RP_LB ); break;
            case 270: m_pCtlAngle->SetActualRP( RP_MB ); break;
            case 315: m_pCtlAngle->SetActualRP( RP_RB ); break;
            default:  m_pCtlAngle->SetActualRP( RP_MM ); break;
        }

        // fill ItemSet and pass it on to m_pCtlPreview
        m_rXFSet.Put( XFillHatchItem( OUString(), *pHatch ) );
        m_pCtlPreview->SetAttributes( m_aXFillAttr.GetItemSet() );

        m_pCtlPreview->Invalidate();
        pHatch.reset();
    }
    m_pMtrDistance->SaveValue();
    m_pMtrAngle->SaveValue();
    m_pLbLineType->SaveValue();
    m_pLbLineColor->SaveValue();
    m_pLbHatchings->SaveValue();
}


IMPL_LINK_NOARG_TYPED(SvxHatchTabPage, ClickAddHdl_Impl, Button*, void)
{
    OUString aNewName( SVX_RES( RID_SVXSTR_HATCH ) );
    OUString aDesc( CUI_RES( RID_SVXSTR_DESC_HATCH ) );
    OUString aName;

    long nCount = m_pHatchingList->Count();
    long j = 1;
    bool bDifferent = false;

    while( !bDifferent )
    {
        aName  = aNewName;
        aName += " ";
        aName += OUString::number( j++ );
        bDifferent = true;

        for( long i = 0; i < nCount && bDifferent; i++ )
            if( aName == m_pHatchingList->GetHatch( i )->GetName() )
                bDifferent = false;
    }

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialog creation failed!");
    std::unique_ptr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc ));
    DBG_ASSERT(pDlg, "Dialog creation failed!");
    ScopedVclPtr<MessageDialog> pWarnBox;
    sal_uInt16         nError   = 1;

    while( pDlg->Execute() == RET_OK )
    {
        pDlg->GetName( aName );

        bDifferent = true;

        for( long i = 0; i < nCount && bDifferent; i++ )
            if( aName == m_pHatchingList->GetHatch( i )->GetName() )
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
        XHatch aXHatch( m_pLbLineColor->GetSelectEntryColor(),
                        (css::drawing::HatchStyle) m_pLbLineType->GetSelectEntryPos(),
                        GetCoreValue( *m_pMtrDistance, m_ePoolUnit ),
                        static_cast<long>(m_pMtrAngle->GetValue() * 10) );
        XHatchEntry* pEntry = new XHatchEntry( aXHatch, aName );

        m_pHatchingList->Insert( pEntry, nCount );

        m_pLbHatchings->Append( *pEntry, m_pHatchingList->GetUiBitmap( nCount ) );

        m_pLbHatchings->SelectEntryPos( m_pLbHatchings->GetEntryCount() - 1 );

#ifdef _WIN32
        // hack: #31355# W.P.
        Rectangle aRect( m_pLbHatchings->GetPosPixel(), m_pLbHatchings->GetSizePixel() );
        if( sal_True ) {                // ??? overlapped with pDlg
                                    // and srolling
            Invalidate( aRect );
        }
#endif

        *m_pnHatchingListState |= ChangeType::MODIFIED;

        ChangeHatchHdl_Impl( *m_pLbHatchings );
    }

    // determine button state
    if( m_pHatchingList->Count() )
    {
        m_pBtnModify->Enable();
        m_pBtnDelete->Enable();
        m_pBtnSave->Enable();
    }
}


IMPL_LINK_NOARG_TYPED(SvxHatchTabPage, ClickModifyHdl_Impl, Button*, void)
{
    sal_Int32 nPos = m_pLbHatchings->GetSelectEntryPos();

    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        OUString aDesc( CUI_RES( RID_SVXSTR_DESC_HATCH ) );
        OUString aName( m_pHatchingList->GetHatch( nPos )->GetName() );
        OUString aOldName = aName;

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialog creation failed!");
        std::unique_ptr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc ));
        DBG_ASSERT(pDlg, "Dialog creation failed!");

        long nCount = m_pHatchingList->Count();
        bool bLoop = true;
        while( bLoop && pDlg->Execute() == RET_OK )
        {
            pDlg->GetName( aName );
            bool bDifferent = true;

            for( long i = 0; i < nCount && bDifferent; i++ )
            {
                if( aName == m_pHatchingList->GetHatch( i )->GetName() &&
                    aName != aOldName )
                    bDifferent = false;
            }

            if( bDifferent )
            {
                bLoop = false;
                XHatch aXHatch( m_pLbLineColor->GetSelectEntryColor(),
                                (css::drawing::HatchStyle) m_pLbLineType->GetSelectEntryPos(),
                                 GetCoreValue( *m_pMtrDistance, m_ePoolUnit ),
                                static_cast<long>(m_pMtrAngle->GetValue() * 10) );

                XHatchEntry* pEntry = new XHatchEntry( aXHatch, aName );

                delete m_pHatchingList->Replace( pEntry, nPos );

                m_pLbHatchings->Modify( *pEntry, nPos, m_pHatchingList->GetUiBitmap( nPos ) );

                m_pLbHatchings->SelectEntryPos( nPos );

                // save values for changes recognition (-> method)
                m_pMtrDistance->SaveValue();
                m_pMtrAngle->SaveValue();
                m_pLbLineType->SaveValue();
                m_pLbLineColor->SaveValue();
                m_pLbHatchings->SaveValue();

                *m_pnHatchingListState |= ChangeType::MODIFIED;
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


IMPL_LINK_NOARG_TYPED(SvxHatchTabPage, ClickDeleteHdl_Impl, Button*, void)
{
    sal_Int32 nPos = m_pLbHatchings->GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        ScopedVclPtrInstance< MessageDialog > aQueryBox( GetParentDialog(),"AskDelHatchDialog","cui/ui/querydeletehatchdialog.ui");

        if( aQueryBox->Execute() == RET_YES )
        {
            delete m_pHatchingList->Remove( nPos );
            m_pLbHatchings->RemoveEntry( nPos );
            m_pLbHatchings->SelectEntryPos( 0 );

            m_pCtlPreview->Invalidate();

            ChangeHatchHdl_Impl( *m_pLbHatchings );

            *m_pnHatchingListState |= ChangeType::MODIFIED;
        }
    }
    // determine button state
    if( !m_pHatchingList->Count() )
    {
        m_pBtnModify->Disable();
        m_pBtnDelete->Disable();
        m_pBtnSave->Disable();
    }
}


IMPL_LINK_NOARG_TYPED(SvxHatchTabPage, ClickLoadHdl_Impl, Button*, void)
{
    ResMgr& rMgr = CUI_MGR();
    sal_uInt16 nReturn = RET_YES;

    if ( *m_pnHatchingListState & ChangeType::MODIFIED )
    {
        nReturn = ScopedVclPtrInstance<MessageDialog>::Create( GetParentDialog()
                                ,"AskSaveList"
                                ,"cui/ui/querysavelistdialog.ui")->Execute();

        if ( nReturn == RET_YES )
            m_pHatchingList->Save();
    }

    if ( nReturn != RET_CANCEL )
    {
        ::sfx2::FileDialogHelper aDlg( css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE );
        OUString aStrFilterType( "*.soh" );
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

            XHatchListRef pHatchList = XPropertyList::AsHatchList(
                XPropertyList::CreatePropertyList(
                    XHATCH_LIST, aPathURL.GetMainURL( INetURLObject::NO_DECODE ), "" ));
            pHatchList->SetName( aURL.getName() );
            if( pHatchList->Load() )
            {
                m_pHatchingList = pHatchList;
                static_cast<SvxAreaTabDialog*>( GetParentDialog() )->SetNewHatchingList( m_pHatchingList );

                m_pLbHatchings->Clear();
                m_pLbHatchings->Fill( m_pHatchingList );
                Reset( &m_rOutAttrs );

                m_pHatchingList->SetName( aURL.getName() );

                // determining (and possibly cutting) the name
                // and displaying it in the GroupBox
                OUString aString( ResId( RID_SVXSTR_TABLE, rMgr ) );
                aString += ": ";

                if ( aURL.getBase().getLength() > 18 )
                {
                    aString += aURL.getBase().copy( 0, 15 );
                    aString += "...";
                }
                else
                    aString += aURL.getBase();

                *m_pnHatchingListState |= ChangeType::CHANGED;
                *m_pnHatchingListState &= ~ChangeType::MODIFIED;
            }
            else
                ScopedVclPtrInstance<MessageDialog>::Create( GetParentDialog()
                               ,"NoLoadedFileDialog"
                               ,"cui/ui/querynoloadedfiledialog.ui")->Execute();
        }
    }

    // determine button state
    if ( m_pHatchingList->Count() )
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


IMPL_LINK_NOARG_TYPED(SvxHatchTabPage, ClickSaveHdl_Impl, Button*, void)
{
    ::sfx2::FileDialogHelper aDlg( css::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE );
    OUString aStrFilterType( "*.soh" );
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

    if( !m_pHatchingList->GetName().isEmpty() )
    {
        aFile.Append( m_pHatchingList->GetName() );

        if( aFile.getExtension().isEmpty() )
            aFile.SetExtension( "soh" );
    }

    aDlg.SetDisplayDirectory( aFile.GetMainURL( INetURLObject::NO_DECODE ) );
    if ( aDlg.Execute() == ERRCODE_NONE )
    {
        INetURLObject aURL( aDlg.GetPath() );
        INetURLObject aPathURL( aURL );

        aPathURL.removeSegment();
        aPathURL.removeFinalSlash();

        m_pHatchingList->SetName( aURL.getName() );
        m_pHatchingList->SetPath( aPathURL.GetMainURL( INetURLObject::NO_DECODE ) );

        if( m_pHatchingList->Save() )
        {
            // determining (and possibly cutting) the name
            // and displaying it in the GroupBox
            OUString aString( CUI_RES( RID_SVXSTR_TABLE ) );
            aString += ": ";

            if ( aURL.getBase().getLength() > 18 )
            {
                aString += aURL.getBase().copy( 0, 15 );
                aString += "...";
            }
            else
                aString += aURL.getBase();

            *m_pnHatchingListState |= ChangeType::SAVED;
            *m_pnHatchingListState &= ~ChangeType::MODIFIED;
        }
        else
        {
            ScopedVclPtrInstance<MessageDialog>::Create( GetParentDialog()
                          ,"NoSaveFileDialog"
                          ,"cui/ui/querynosavefiledialog.ui")->Execute();
        }
    }
}


void SvxHatchTabPage::PointChanged( vcl::Window* pWindow, RECT_POINT eRcPt )
{
    if( pWindow == m_pCtlAngle )
    {
        switch( eRcPt )
        {
            case RP_LT: m_pMtrAngle->SetValue( 135 ); break;
            case RP_MT: m_pMtrAngle->SetValue( 90 );  break;
            case RP_RT: m_pMtrAngle->SetValue( 45 );  break;
            case RP_LM: m_pMtrAngle->SetValue( 180 ); break;
            case RP_RM: m_pMtrAngle->SetValue( 0 );   break;
            case RP_LB: m_pMtrAngle->SetValue( 225 ); break;
            case RP_MB: m_pMtrAngle->SetValue( 270 ); break;
            case RP_RB: m_pMtrAngle->SetValue( 315 ); break;
            case RP_MM: break;
        }
        ModifiedHdl_Impl( this );
    }
}


void SvxHatchTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( ( rDCEvt.GetType() == DataChangedEventType::SETTINGS ) && ( rDCEvt.GetFlags() & AllSettingsFlags::STYLE ) )
        m_pCtlPreview->SetDrawMode( GetSettings().GetStyleSettings().GetHighContrastMode() ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR );

    SvxTabPage::DataChanged( rDCEvt );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
