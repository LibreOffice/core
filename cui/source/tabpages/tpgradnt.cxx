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

#include <memory>
#include <vcl/wrkwin.hxx>
#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dialoghelper.hxx>
#include <sfx2/filedlghelper.hxx>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>

#include <strings.hrc>
#include <helpids.h>
#include <svx/colorbox.hxx>
#include <svx/xattr.hxx>
#include <svx/xtable.hxx>
#include <svx/xpool.hxx>
#include <svx/drawitem.hxx>
#include <cuitabarea.hxx>
#include <defdlgname.hxx>
#include <dlgname.hxx>
#include <svx/svxdlg.hxx>
#include <dialmgr.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include <svx/strings.hrc>

#include <o3tl/make_unique.hxx>

#define DEFAULT_GRADIENTSTEP 64

using namespace com::sun::star;

SvxGradientTabPage::SvxGradientTabPage
(
    vcl::Window* pParent,
    const SfxItemSet& rInAttrs
) :
    SfxTabPage          ( pParent, "GradientPage", "cui/ui/gradientpage.ui", &rInAttrs ),

    m_rOutAttrs           ( rInAttrs ),

    m_pnGradientListState ( nullptr ),
    m_pnColorListState    ( nullptr ),

    m_aXGradientItem      ( OUString(), XGradient( COL_BLACK, COL_WHITE ) ),
    m_aXFillAttr          ( rInAttrs.GetPool() ),
    m_rXFSet              ( m_aXFillAttr.GetItemSet() )
{
    get(m_pCbIncrement,    "autoincrement");
    get(m_pMtrIncrement,   "incrementmtr");
    get(m_pSliderIncrement,"incrementslider");
    get(m_pLbGradientType, "gradienttypelb");
    get(m_pFtCenter,       "centerft");
    get(m_pMtrCenterX,     "centerxmtr");
    get(m_pMtrCenterY,     "centerymtr");
    get(m_pFtAngle,        "angleft");
    get(m_pMtrAngle,       "anglemtr");
    get(m_pMtrBorder,      "bordermtr");
    get(m_pSliderBorder,   "borderslider");
    get(m_pLbColorFrom,    "colorfromlb");
    get(m_pMtrColorFrom,   "colorfrommtr");
    get(m_pLbColorTo,      "colortolb");
    get(m_pMtrColorTo,     "colortomtr");
    get(m_pGradientLB,     "gradientpresetlist");
    get(m_pCtlPreview,     "previewctl");
    Size aSize = getDrawPreviewOptimalSize(this);
    m_pGradientLB->set_width_request(aSize.Width());
    m_pGradientLB->set_height_request(aSize.Height());
    m_pCtlPreview->set_width_request(aSize.Width());
    m_pCtlPreview->set_height_request(aSize.Height());
    get(m_pBtnAdd,         "add");
    get(m_pBtnModify,      "modify");
    // this page needs ExchangeSupport
    SetExchangeSupport();

    // as long as NOT supported by the item

    m_pSliderIncrement->SetRange(Range(3,256));
    m_pMtrColorTo->SetValue( 100 );
    m_pMtrColorFrom->SetValue( 100 );
    m_pSliderBorder->SetRange(Range(0,100));

    // setting the output device
    m_rXFSet.Put( XFillStyleItem(drawing::FillStyle_GRADIENT) );
    m_rXFSet.Put( m_aXGradientItem );
    m_pCtlPreview->SetAttributes( m_aXFillAttr.GetItemSet() );

    // set handler
    m_pGradientLB->SetSelectHdl( LINK( this, SvxGradientTabPage, ChangeGradientHdl ) );
    m_pGradientLB->SetRenameHdl( LINK( this, SvxGradientTabPage, ClickRenameHdl_Impl ) );
    m_pGradientLB->SetDeleteHdl( LINK( this, SvxGradientTabPage, ClickDeleteHdl_Impl ) );
    m_pBtnAdd->SetClickHdl( LINK( this, SvxGradientTabPage, ClickAddHdl_Impl ) );
    m_pBtnModify->SetClickHdl(
        LINK( this, SvxGradientTabPage, ClickModifyHdl_Impl ) );

    Link<Edit&,void> aLink = LINK( this, SvxGradientTabPage, ModifiedEditHdl_Impl );
    Link<ListBox&,void> aLink2 = LINK( this, SvxGradientTabPage, ModifiedListBoxHdl_Impl );
    m_pLbGradientType->SetSelectHdl( aLink2 );
    m_pCbIncrement->SetToggleHdl( LINK( this, SvxGradientTabPage, ChangeAutoStepHdl_Impl ) );
    m_pMtrIncrement->SetModifyHdl( aLink );
    m_pSliderIncrement->SetSlideHdl( LINK( this, SvxGradientTabPage, ModifiedSliderHdl_Impl ) );
    m_pMtrCenterX->SetModifyHdl( aLink );
    m_pMtrCenterY->SetModifyHdl( aLink );
    m_pMtrAngle->SetModifyHdl( aLink );
    m_pMtrBorder->SetModifyHdl( aLink );
    m_pSliderBorder->SetSlideHdl( LINK( this, SvxGradientTabPage, ModifiedSliderHdl_Impl ) );
    m_pMtrColorFrom->SetModifyHdl( aLink );
    Link<SvxColorListBox&,void> aLink3 = LINK( this, SvxGradientTabPage, ModifiedColorListBoxHdl_Impl );
    m_pLbColorFrom->SetSelectHdl( aLink3 );
    m_pMtrColorTo->SetModifyHdl( aLink );
    m_pLbColorTo->SetSelectHdl( aLink3 );

    // #i76307# always paint the preview in LTR, because this is what the document does
    m_pCtlPreview->EnableRTL( false );

    setPreviewsToSamePlace(pParent, this);
}

SvxGradientTabPage::~SvxGradientTabPage()
{
    disposeOnce();
}

void SvxGradientTabPage::dispose()
{
    m_pCbIncrement.clear();
    m_pMtrIncrement.clear();
    m_pSliderIncrement.clear();
    m_pLbGradientType.clear();
    m_pFtCenter.clear();
    m_pMtrCenterX.clear();
    m_pMtrCenterY.clear();
    m_pFtAngle.clear();
    m_pMtrAngle.clear();
    m_pMtrBorder.clear();
    m_pSliderBorder.clear();
    m_pLbColorFrom.clear();
    m_pMtrColorFrom.clear();
    m_pLbColorTo.clear();
    m_pMtrColorTo.clear();
    m_pGradientLB.clear();
    m_pCtlPreview.clear();
    m_pBtnAdd.clear();
    m_pBtnModify.clear();
    SfxTabPage::dispose();
}

void SvxGradientTabPage::Construct()
{
    m_pGradientLB->FillPresetListBox( *m_pGradientList );
}

void SvxGradientTabPage::ActivatePage( const SfxItemSet& rSet )
{
    if( m_pColorList.is() )
    {
        // ColorList
        if( *m_pnColorListState & ChangeType::CHANGED ||
            *m_pnColorListState & ChangeType::MODIFIED )
        {
            SvxAreaTabDialog* pArea = (*m_pnColorListState & ChangeType::CHANGED) ?
                dynamic_cast<SvxAreaTabDialog*>(GetParentDialog()) : nullptr;
            if (pArea)
                m_pColorList = pArea->GetNewColorList();

            ModifiedHdl_Impl( this );
        }

        // determining (and possibly cutting) the name and
        // displaying it in the GroupBox
        OUString        aString( CuiResId( RID_SVXSTR_TABLE ) );
        aString         += ": ";
        INetURLObject   aURL( m_pGradientList->GetPath() );

        aURL.Append( m_pGradientList->GetName() );
        SAL_WARN_IF( aURL.GetProtocol() == INetProtocol::NotValid, "cui.tabpages", "invalid URL" );

        if ( aURL.getBase().getLength() > 18 )
        {
            aString += aURL.getBase().copy( 0, 15 ) + "...";
        }
        else
            aString += aURL.getBase();

        sal_Int32 nPos = SearchGradientList( rSet.Get(XATTR_FILLGRADIENT).GetName() );
        if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            sal_uInt16 nId = m_pGradientLB->GetItemId( static_cast<size_t>( nPos ) );
            m_pGradientLB->SelectItem( nId );
        }
        // colors could have been deleted
        ChangeGradientHdl_Impl();
    }
}


DeactivateRC SvxGradientTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( _pSet )
        FillItemSet( _pSet );

    return DeactivateRC::LeavePage;
}

bool SvxGradientTabPage::FillItemSet( SfxItemSet* rSet )
{
    std::unique_ptr<XGradient> pXGradient;
    OUString      aString;
    size_t nPos = m_pGradientLB->IsNoSelection() ? VALUESET_ITEM_NOTFOUND : m_pGradientLB->GetSelectItemPos();
    if( nPos != VALUESET_ITEM_NOTFOUND )
    {
        pXGradient.reset(new XGradient( m_pGradientList->GetGradient( static_cast<sal_uInt16>(nPos) )->GetGradient() ));
        aString = m_pGradientLB->GetItemText( m_pGradientLB->GetSelectItemId() );
    }
    else
    // gradient was passed (unidentified)
    {
        aString = "gradient";
        pXGradient.reset(new XGradient( m_pLbColorFrom->GetSelectEntryColor(),
                    m_pLbColorTo->GetSelectEntryColor(),
                    (css::awt::GradientStyle) m_pLbGradientType->GetSelectedEntryPos(),
                    static_cast<long>(m_pMtrAngle->GetValue() * 10), // should be changed in resource
                    (sal_uInt16) m_pMtrCenterX->GetValue(),
                    (sal_uInt16) m_pMtrCenterY->GetValue(),
                    (sal_uInt16) m_pMtrBorder->GetValue(),
                    (sal_uInt16) m_pMtrColorFrom->GetValue(),
                    (sal_uInt16) m_pMtrColorTo->GetValue(),
                    (sal_uInt16) m_pMtrIncrement->GetValue() ));
    }

    sal_uInt16 nValue = 0;
    if( !m_pCbIncrement->IsChecked() )
        nValue = m_pMtrIncrement->GetValue();

    assert( pXGradient && "XGradient could not be created" );
    rSet->Put( XFillStyleItem( drawing::FillStyle_GRADIENT ) );
    rSet->Put( XFillGradientItem( aString, *pXGradient ) );
    rSet->Put( XGradientStepCountItem( nValue ) );
    return true;
}


void SvxGradientTabPage::Reset( const SfxItemSet* )
{
    m_pMtrIncrement->SetValue(DEFAULT_GRADIENTSTEP);
    m_pSliderIncrement->SetThumbPos(DEFAULT_GRADIENTSTEP);
    ChangeGradientHdl_Impl();

    // determine state of the buttons
    if( m_pGradientList->Count() )
        m_pBtnModify->Enable();
    else
        m_pBtnModify->Disable();
}


VclPtr<SfxTabPage> SvxGradientTabPage::Create( vcl::Window* pWindow,
                                               const SfxItemSet* rOutAttrs )
{
    return VclPtr<SvxGradientTabPage>::Create( pWindow, *rOutAttrs );
}

IMPL_LINK( SvxGradientTabPage, ModifiedListBoxHdl_Impl, ListBox&, rListBox, void )
{
    ModifiedHdl_Impl(&rListBox);
    // gradient params changed, it is no longer one of the presets
    m_pGradientLB->SetNoSelection();
}

IMPL_LINK( SvxGradientTabPage, ModifiedColorListBoxHdl_Impl, SvxColorListBox&, rListBox, void )
{
    ModifiedHdl_Impl(&rListBox);
    m_pGradientLB->SetNoSelection();
}

IMPL_LINK( SvxGradientTabPage, ModifiedEditHdl_Impl, Edit&, rBox, void )
{
    ModifiedHdl_Impl(&rBox);
    m_pGradientLB->SetNoSelection();
}

IMPL_LINK( SvxGradientTabPage, ModifiedSliderHdl_Impl, Slider*, rSlider, void )
{
    ModifiedHdl_Impl(rSlider);
    m_pGradientLB->SetNoSelection();
}

IMPL_LINK_NOARG( SvxGradientTabPage, ChangeAutoStepHdl_Impl, CheckBox&, void )
{
    if(m_pCbIncrement->IsChecked())
    {
        m_pSliderIncrement->Disable();
        m_pMtrIncrement->Disable();
    }
    else
    {
        m_pSliderIncrement->Enable();
        m_pMtrIncrement->Enable();
    }
    ModifiedHdl_Impl(m_pMtrIncrement);
}

void SvxGradientTabPage::ModifiedHdl_Impl( void const * pControl )
{
    if( pControl == m_pMtrBorder )
        m_pSliderBorder->SetThumbPos( m_pMtrBorder->GetValue() );
    if( pControl == m_pSliderBorder )
        m_pMtrBorder->SetValue( m_pSliderBorder->GetThumbPos() );
    if( pControl == m_pMtrIncrement )
        m_pSliderIncrement->SetThumbPos( m_pMtrIncrement->GetValue() );
    if(pControl == m_pSliderIncrement)
        m_pMtrIncrement->SetValue( m_pSliderIncrement->GetThumbPos() );

    css::awt::GradientStyle eXGS = (css::awt::GradientStyle) m_pLbGradientType->GetSelectedEntryPos();

    XGradient aXGradient( m_pLbColorFrom->GetSelectEntryColor(),
                          m_pLbColorTo->GetSelectEntryColor(),
                          eXGS,
                          static_cast<long>(m_pMtrAngle->GetValue() * 10), // should be changed in resource
                          (sal_uInt16) m_pMtrCenterX->GetValue(),
                          (sal_uInt16) m_pMtrCenterY->GetValue(),
                          (sal_uInt16) m_pMtrBorder->GetValue(),
                          (sal_uInt16) m_pMtrColorFrom->GetValue(),
                          (sal_uInt16) m_pMtrColorTo->GetValue(),
                          (sal_uInt16) m_pMtrIncrement->GetValue() );

    // enable/disable controls
    if( pControl == m_pLbGradientType || pControl == this )
        SetControlState_Impl( eXGS );

    sal_uInt16 nValue = 0;
    if(!m_pCbIncrement->IsChecked())
        nValue = (sal_uInt16)m_pMtrIncrement->GetValue();
    m_rXFSet.Put( XGradientStepCountItem( nValue ) );

    // displaying in XOutDev
    m_rXFSet.Put( XFillGradientItem( OUString(), aXGradient ) );
    m_pCtlPreview->SetAttributes( m_aXFillAttr.GetItemSet() );

    m_pCtlPreview->Invalidate();
}


IMPL_LINK_NOARG(SvxGradientTabPage, ClickAddHdl_Impl, Button*, void)
{
    OUString aNewName( SvxResId( RID_SVXSTR_GRADIENT ) );
    OUString aDesc( CuiResId( RID_SVXSTR_DESC_GRADIENT ) );
    OUString aName;

    long nCount = m_pGradientList->Count();
    long j = 1;
    bool bValidGradientName = false;

    while( !bValidGradientName )
    {
        aName  = aNewName + " " + OUString::number( j++ );
        bValidGradientName = (SearchGradientList(aName) == LISTBOX_ENTRY_NOTFOUND);
    }

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    ScopedVclPtr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc ));
    ScopedVclPtr<MessageDialog> pWarnBox;
    sal_uInt16 nError   = 1;

    while (pDlg->Execute() == RET_OK)
    {
        pDlg->GetName( aName );

        bValidGradientName = (SearchGradientList(aName) == LISTBOX_ENTRY_NOTFOUND);

        if (bValidGradientName)
        {
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
    pDlg.disposeAndClear();
    pWarnBox.disposeAndClear();

    if( !nError )
    {
        XGradient aXGradient( m_pLbColorFrom->GetSelectEntryColor(),
                              m_pLbColorTo->GetSelectEntryColor(),
                              (css::awt::GradientStyle) m_pLbGradientType->GetSelectedEntryPos(),
                              static_cast<long>(m_pMtrAngle->GetValue() * 10), // should be changed in resource
                              (sal_uInt16) m_pMtrCenterX->GetValue(),
                              (sal_uInt16) m_pMtrCenterY->GetValue(),
                              (sal_uInt16) m_pMtrBorder->GetValue(),
                              (sal_uInt16) m_pMtrColorFrom->GetValue(),
                              (sal_uInt16) m_pMtrColorTo->GetValue(),
                              (sal_uInt16) m_pMtrIncrement->GetValue() );

        m_pGradientList->Insert(o3tl::make_unique<XGradientEntry>(aXGradient, aName), nCount);

        sal_Int32 nId = m_pGradientLB->GetItemId(nCount - 1); //calculate the last ID
        Bitmap aBitmap = m_pGradientList->GetBitmapForPreview( nCount, m_pGradientLB->GetIconSize() );
        m_pGradientLB->InsertItem( nId + 1, Image(aBitmap), aName );
        m_pGradientLB->SelectItem( nId + 1 );
        m_pGradientLB->Resize();

        *m_pnGradientListState |= ChangeType::MODIFIED;

        ChangeGradientHdl_Impl();
    }

    // determine button state
    if( m_pGradientList->Count() )
        m_pBtnModify->Enable();
}


IMPL_LINK_NOARG(SvxGradientTabPage, ClickModifyHdl_Impl, Button*, void)
{
    sal_uInt16 nId = m_pGradientLB->GetSelectItemId();
    size_t nPos = m_pGradientLB->GetSelectItemPos();

    if ( nPos != VALUESET_ITEM_NOTFOUND )
    {
        OUString aName( m_pGradientList->GetGradient( static_cast<sal_uInt16>(nPos) )->GetName() );

        XGradient aXGradient( m_pLbColorFrom->GetSelectEntryColor(),
                              m_pLbColorTo->GetSelectEntryColor(),
                              (css::awt::GradientStyle) m_pLbGradientType->GetSelectedEntryPos(),
                              static_cast<long>(m_pMtrAngle->GetValue() * 10), // should be changed in resource
                              (sal_uInt16) m_pMtrCenterX->GetValue(),
                              (sal_uInt16) m_pMtrCenterY->GetValue(),
                              (sal_uInt16) m_pMtrBorder->GetValue(),
                              (sal_uInt16) m_pMtrColorFrom->GetValue(),
                              (sal_uInt16) m_pMtrColorTo->GetValue(),
                              (sal_uInt16) m_pMtrIncrement->GetValue() );

        m_pGradientList->Replace(o3tl::make_unique<XGradientEntry>(aXGradient, aName), nPos);

        Bitmap aBitmap = m_pGradientList->GetBitmapForPreview( static_cast<sal_uInt16>(nPos), m_pGradientLB->GetIconSize() );
        m_pGradientLB->RemoveItem( nId );
        m_pGradientLB->InsertItem( nId, Image(aBitmap), aName, static_cast<sal_uInt16>(nPos) );
        m_pGradientLB->SelectItem( nId );

        *m_pnGradientListState |= ChangeType::MODIFIED;
    }
}

IMPL_LINK_NOARG(SvxGradientTabPage, ClickDeleteHdl_Impl, SvxPresetListBox*, void)
{
    sal_uInt16 nId = m_pGradientLB->GetSelectItemId();
    size_t nPos = m_pGradientLB->GetSelectItemPos();

    if( nPos != VALUESET_ITEM_NOTFOUND )
    {
        ScopedVclPtrInstance< MessageDialog > aQueryBox( GetParentDialog(),"AskDelGradientDialog","cui/ui/querydeletegradientdialog.ui");

        if ( aQueryBox->Execute() == RET_YES )
        {
            m_pGradientList->Remove(nPos);
            m_pGradientLB->RemoveItem( nId );
            nId = m_pGradientLB->GetItemId( 0 );
            m_pGradientLB->SelectItem( nId );
            m_pGradientLB->Resize();

            m_pCtlPreview->Invalidate();

            ChangeGradientHdl_Impl();

            *m_pnGradientListState |= ChangeType::MODIFIED;
        }
    }
    // determine button state
    if( !m_pGradientList->Count() )
        m_pBtnModify->Disable();
}

IMPL_LINK_NOARG(SvxGradientTabPage, ClickRenameHdl_Impl, SvxPresetListBox*, void)
{
    sal_uInt16 nId = m_pGradientLB->GetSelectItemId();
    size_t nPos = m_pGradientLB->GetSelectItemPos();

    if ( nPos != VALUESET_ITEM_NOTFOUND )
    {
        OUString aDesc( CuiResId( RID_SVXSTR_DESC_GRADIENT ) );
        OUString aName( m_pGradientList->GetGradient( nPos )->GetName() );

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        assert(pFact && "Dialog creation failed!");
        ScopedVclPtr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc ));
        assert(pDlg && "Dialog creation failed!");

        bool bLoop = true;
        while( bLoop && pDlg->Execute() == RET_OK )
        {
            pDlg->GetName( aName );
            sal_Int32 nGradientPos = SearchGradientList(aName);
            bool bValidGradientName = (nGradientPos == static_cast<sal_Int32>(nPos) ) || (nGradientPos == LISTBOX_ENTRY_NOTFOUND);

            if( bValidGradientName )
            {
                bLoop = false;
                m_pGradientList->GetGradient(nPos)->SetName(aName);

                m_pGradientLB->SetItemText( nId, aName );
                m_pGradientLB->SelectItem( nId );

                *m_pnGradientListState |= ChangeType::MODIFIED;
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

IMPL_LINK_NOARG(SvxGradientTabPage, ChangeGradientHdl, ValueSet*, void)
{
    ChangeGradientHdl_Impl();
}

void SvxGradientTabPage::ChangeGradientHdl_Impl()
{
    std::unique_ptr<XGradient> pGradient;
    size_t nPos = m_pGradientLB->GetSelectItemPos();

    if( nPos != VALUESET_ITEM_NOTFOUND )
        pGradient.reset(new XGradient( m_pGradientList->GetGradient( static_cast<sal_uInt16>( nPos ) )->GetGradient() ));
    else
    {
        const SfxPoolItem* pPoolItem = nullptr;
        if( SfxItemState::SET == m_rOutAttrs.GetItemState( GetWhich( XATTR_FILLSTYLE ), true, &pPoolItem ) )
        {
            if( ( drawing::FillStyle_GRADIENT == (drawing::FillStyle) static_cast<const XFillStyleItem*>( pPoolItem )->GetValue() ) &&
                ( SfxItemState::SET == m_rOutAttrs.GetItemState( GetWhich( XATTR_FILLGRADIENT ), true, &pPoolItem ) ) )
            {
                pGradient.reset(new XGradient( static_cast<const XFillGradientItem*>( pPoolItem )->GetGradientValue() ));
            }
        }
        if( !pGradient )
        {
            sal_uInt16 nPosition = m_pGradientLB->GetItemId(0);
            m_pGradientLB->SelectItem( nPosition );
            if( nPosition != 0 )
                pGradient.reset(new XGradient( m_pGradientList->GetGradient( 0 )->GetGradient() ));
        }
    }

    if( pGradient )
    {
        css::awt::GradientStyle eXGS = pGradient->GetGradientStyle();
        sal_uInt16 nValue = pGradient->GetSteps();
        if( nValue == 0 )
        {
            m_pCbIncrement->SetState(TRISTATE_TRUE);
            m_pMtrIncrement->Disable();
            m_pSliderIncrement->Disable();
        }
        else
        {
            m_pCbIncrement->SetState(TRISTATE_FALSE);
            m_pMtrIncrement->Enable();
            m_pMtrIncrement->SetValue( nValue );
            m_pSliderIncrement->Enable();
            m_pSliderIncrement->SetThumbPos( nValue );
        }
        m_pLbGradientType->SelectEntryPos(
            sal::static_int_cast< sal_Int32 >( eXGS ) );
        // if the entry is not in the listbox,
        // colors are added temporarily
        m_pLbColorFrom->SetNoSelection();
        m_pLbColorFrom->SelectEntry( pGradient->GetStartColor() );

        m_pLbColorTo->SetNoSelection();
        m_pLbColorTo->SelectEntry( pGradient->GetEndColor() );

        m_pMtrAngle->SetValue( pGradient->GetAngle() / 10 ); // should be changed in resource
        m_pMtrBorder->SetValue( pGradient->GetBorder() );
        m_pSliderBorder->SetThumbPos( pGradient->GetBorder() );
        m_pMtrCenterX->SetValue( pGradient->GetXOffset() );
        m_pMtrCenterY->SetValue( pGradient->GetYOffset() );
        m_pMtrColorFrom->SetValue( pGradient->GetStartIntens() );
        m_pMtrColorTo->SetValue( pGradient->GetEndIntens() );

        // disable/enable controls
        SetControlState_Impl( eXGS );

        // fill ItemSet and pass it on to aCtlPreview
        m_rXFSet.Put( XFillGradientItem( OUString(), *pGradient ) );
        m_rXFSet.Put( XGradientStepCountItem( nValue ) );
        m_pCtlPreview->SetAttributes( m_aXFillAttr.GetItemSet() );

        m_pCtlPreview->Invalidate();
    }
}


void SvxGradientTabPage::SetControlState_Impl( css::awt::GradientStyle eXGS )
{
    switch( eXGS )
    {
        case css::awt::GradientStyle_LINEAR:
        case css::awt::GradientStyle_AXIAL:
            m_pFtCenter->Disable();
            m_pMtrCenterX->Disable();
            m_pMtrCenterY->Disable();
            m_pFtAngle->Enable();
            m_pMtrAngle->Enable();
            break;

        case css::awt::GradientStyle_RADIAL:
            m_pFtCenter->Enable();
            m_pMtrCenterX->Enable();
            m_pMtrCenterY->Enable();
            m_pFtAngle->Disable();
            m_pMtrAngle->Disable();
            break;

        case css::awt::GradientStyle_ELLIPTICAL:
            m_pFtCenter->Enable();
            m_pMtrCenterX->Enable();
            m_pMtrCenterY->Enable();
            m_pFtAngle->Enable();
            m_pMtrAngle->Enable();
            break;

        case css::awt::GradientStyle_SQUARE:
        case css::awt::GradientStyle_RECT:
            m_pFtCenter->Enable();
            m_pMtrCenterX->Enable();
            m_pMtrCenterY->Enable();
            m_pFtAngle->Enable();
            m_pMtrAngle->Enable();
            break;
        default:
            break;
    }
}

sal_Int32 SvxGradientTabPage::SearchGradientList(const OUString& rGradientName)
{
    long nCount = m_pGradientList->Count();
    bool bValidGradientName = true;
    sal_Int32 nPos = LISTBOX_ENTRY_NOTFOUND;

    for(long i = 0;i < nCount && bValidGradientName;i++)
    {
        if(rGradientName == m_pGradientList->GetGradient( i )->GetName())
        {
            nPos = i;
            bValidGradientName = false;
        }
    }
    return nPos;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
