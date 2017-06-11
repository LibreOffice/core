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
#include <strings.hrc>
#include "helpid.hrc"
#include <svx/colorbox.hxx>
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
#include <svx/dialogs.hrc>
#include <svx/strings.hrc>

#include <o3tl/make_unique.hxx>

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

    m_aXFStyleItem        ( drawing::FillStyle_HATCH ),
    m_aXHatchItem         ( OUString(), XHatch() ),
    m_aXFillAttr          ( rInAttrs.GetPool() ),
    m_rXFSet              ( m_aXFillAttr.GetItemSet() )

{
    get(m_pMtrDistance, "distancemtr");
    get(m_pMtrAngle, "anglemtr");
    get(m_pSliderAngle, "angleslider");
    get(m_pLbLineType, "linetypelb");
    get(m_pLbLineColor, "linecolorlb");
    get(m_pCbBackgroundColor, "backgroundcolor");
    get(m_pLbBackgroundColor, "backgroundcolorlb");
    get(m_pHatchLB , "hatchpresetlist");
    get(m_pCtlPreview, "previewctl");
    Size aSize = getDrawPreviewOptimalSize(this);
    m_pHatchLB->set_width_request(aSize.Width());
    m_pHatchLB->set_height_request(aSize.Height());
    m_pCtlPreview->set_width_request(aSize.Width());
    m_pCtlPreview->set_height_request(aSize.Height());
    get(m_pBtnAdd, "add");
    get(m_pBtnModify, "modify");

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
    assert( pPool && "Where is the pool?" );
    m_ePoolUnit = pPool->GetMetric( SID_ATTR_FILL_HATCH );

    // setting the output device
    m_rXFSet.Put( m_aXFStyleItem );
    m_rXFSet.Put( m_aXHatchItem );
    m_pCtlPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pHatchLB->SetSelectHdl( LINK( this, SvxHatchTabPage, ChangeHatchHdl ) );
    m_pHatchLB->SetRenameHdl( LINK( this, SvxHatchTabPage, ClickRenameHdl_Impl ) );
    m_pHatchLB->SetDeleteHdl( LINK( this, SvxHatchTabPage, ClickDeleteHdl_Impl ) );

    Link<Edit&,void> aLink = LINK( this, SvxHatchTabPage, ModifiedEditHdl_Impl );
    Link<ListBox&,void> aLink2 = LINK( this, SvxHatchTabPage, ModifiedListBoxHdl_Impl );
    m_pMtrDistance->SetModifyHdl( aLink );
    m_pMtrAngle->SetModifyHdl( aLink );
    m_pSliderAngle->SetSlideHdl( LINK( this, SvxHatchTabPage, ModifiedSliderHdl_Impl ) );
    m_pLbLineType->SetSelectHdl( aLink2 );
    Link<SvxColorListBox&,void> aLink3 = LINK( this, SvxHatchTabPage, ModifiedColorListBoxHdl_Impl );
    m_pLbLineColor->SetSelectHdl( aLink3 );
    m_pCbBackgroundColor->SetToggleHdl( LINK( this, SvxHatchTabPage, ToggleHatchBackgroundColor_Impl ) );
    m_pLbBackgroundColor->SetSelectHdl( LINK( this, SvxHatchTabPage, ModifiedBackgroundHdl_Impl ) );

    m_pBtnAdd->SetClickHdl( LINK( this, SvxHatchTabPage, ClickAddHdl_Impl ) );
    m_pBtnModify->SetClickHdl( LINK( this, SvxHatchTabPage, ClickModifyHdl_Impl ) );

    m_pCtlPreview->SetDrawMode( GetSettings().GetStyleSettings().GetHighContrastMode() ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR );

}

SvxHatchTabPage::~SvxHatchTabPage()
{
    disposeOnce();
}

void SvxHatchTabPage::dispose()
{
    m_pMtrDistance.clear();
    m_pMtrAngle.clear();
    m_pSliderAngle.clear();
    m_pLbLineType.clear();
    m_pLbLineColor.clear();
    m_pCbBackgroundColor.clear();
    m_pLbBackgroundColor.clear();
    m_pHatchLB.clear();
    m_pCtlPreview.clear();
    m_pBtnAdd.clear();
    m_pBtnModify.clear();
    SvxTabPage::dispose();
}

void SvxHatchTabPage::Construct()
{
    m_pHatchLB->FillPresetListBox(*m_pHatchingList);
}

void SvxHatchTabPage::ActivatePage( const SfxItemSet& rSet )
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

        // determining (possibly cutting) the name
        // and displaying it in the GroupBox
        OUString        aString( CuiResId( RID_SVXSTR_TABLE ) );
        aString         += ": ";
        INetURLObject   aURL( m_pHatchingList->GetPath() );

        aURL.Append( m_pHatchingList->GetName() );
        SAL_WARN_IF( aURL.GetProtocol() == INetProtocol::NotValid, "cui.tabpages", "invalid URL" );

        if ( aURL.getBase().getLength() > 18 )
        {
            aString += aURL.getBase().copy( 0, 15 ) + "...";
        }
        else
            aString += aURL.getBase();

        sal_Int32 nPos = SearchHatchList( ( &static_cast<const XFillHatchItem&>( rSet.Get(XATTR_FILLHATCH)) )->GetName() );
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            sal_uInt16 nId = m_pHatchLB->GetItemId( static_cast<size_t>( nPos ) );
            m_pHatchLB->SelectItem( nId );
        }
        // colors could have been deleted
        ChangeHatchHdl_Impl();
    }

    XFillBackgroundItem aBckItem( static_cast<const XFillBackgroundItem&>(rSet.Get(XATTR_FILLBACKGROUND)));
    m_rXFSet.Put( aBckItem );

    if(aBckItem.GetValue())
    {
        m_pCbBackgroundColor->SetState(TRISTATE_TRUE);
        XFillColorItem aColorItem( static_cast<const XFillColorItem&>(rSet.Get(XATTR_FILLCOLOR)) );
        Color aColor(aColorItem.GetColorValue());
        m_pLbBackgroundColor->Enable();
        m_pLbBackgroundColor->SelectEntry(aColor);
        m_rXFSet.Put( aColorItem );
    }
    else
    {
        m_pCbBackgroundColor->SetState(TRISTATE_FALSE);
        m_pLbBackgroundColor->Disable();
    }

    m_pCtlPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pCtlPreview->Invalidate();
}


DeactivateRC SvxHatchTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( _pSet )
        FillItemSet( _pSet );

    return DeactivateRC::LeavePage;
}


sal_Int32 SvxHatchTabPage::SearchHatchList(const OUString& rHatchName)
{
    long nCount = m_pHatchingList->Count();
    bool bValidHatchName = true;
    sal_Int32 nPos = LISTBOX_ENTRY_NOTFOUND;

    for(long i = 0;i < nCount && bValidHatchName;i++)
    {
        if(rHatchName == m_pHatchingList->GetHatch( i )->GetName())
        {
            nPos = i;
            bValidHatchName = false;
        }
    }
    return nPos;
}

bool SvxHatchTabPage::FillItemSet( SfxItemSet* rSet )
{
    std::unique_ptr<XHatch> pXHatch;
    OUString  aString;
    size_t nPos = m_pHatchLB->IsNoSelection() ? VALUESET_ITEM_NOTFOUND : m_pHatchLB->GetSelectItemPos();
    if( nPos != VALUESET_ITEM_NOTFOUND )
    {
        pXHatch.reset(new XHatch( m_pHatchingList->GetHatch( static_cast<sal_uInt16>(nPos) )->GetHatch() ));
        aString = m_pHatchLB->GetItemText( m_pHatchLB->GetSelectItemId() );
    }
    // unidentified hatch has been passed
    else
    {
        pXHatch.reset(new XHatch( m_pLbLineColor->GetSelectEntryColor(),
                    (css::drawing::HatchStyle) m_pLbLineType->GetSelectEntryPos(),
                    GetCoreValue( *m_pMtrDistance, m_ePoolUnit ),
                    static_cast<long>(m_pMtrAngle->GetValue() * 10) ));
    }
    assert( pXHatch && "XHatch couldn't be created" );
    rSet->Put( XFillStyleItem( drawing::FillStyle_HATCH ) );
    rSet->Put( XFillHatchItem( aString, *pXHatch ) );
    rSet->Put( XFillBackgroundItem( m_pCbBackgroundColor->IsChecked() ) );
    if (m_pCbBackgroundColor->IsChecked())
    {
        NamedColor aColor = m_pLbBackgroundColor->GetSelectEntry();
        rSet->Put(XFillColorItem(aColor.second, aColor.first));
    }
    return true;
}


void SvxHatchTabPage::Reset( const SfxItemSet* rSet )
{
    ChangeHatchHdl_Impl();

    XFillBackgroundItem aBckItem( static_cast<const XFillBackgroundItem&>(rSet->Get(XATTR_FILLBACKGROUND)) );
    if(aBckItem.GetValue())
        m_pCbBackgroundColor->SetState(TRISTATE_TRUE);
    else
        m_pCbBackgroundColor->SetState(TRISTATE_FALSE);
    m_rXFSet.Put( aBckItem );

    XFillColorItem aColItem( static_cast<const XFillColorItem&>(rSet->Get(XATTR_FILLCOLOR)) );
    m_pLbBackgroundColor->SelectEntry(aColItem.GetColorValue());
    m_rXFSet.Put( aColItem );

    m_pCtlPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pCtlPreview->Invalidate();
}


VclPtr<SfxTabPage> SvxHatchTabPage::Create( vcl::Window* pWindow,
                                            const SfxItemSet* rSet )
{
    return VclPtr<SvxHatchTabPage>::Create( pWindow, *rSet );
}

IMPL_LINK( SvxHatchTabPage, ModifiedListBoxHdl_Impl, ListBox&, rListBox, void )
{
    ModifiedHdl_Impl(&rListBox);
    // hatch params have changed, it is no longer one of the presets
    m_pHatchLB->SetNoSelection();
}

IMPL_LINK( SvxHatchTabPage, ModifiedColorListBoxHdl_Impl, SvxColorListBox&, rListBox, void )
{
    ModifiedHdl_Impl(&rListBox);
    m_pHatchLB->SetNoSelection();
}

IMPL_LINK_NOARG( SvxHatchTabPage, ToggleHatchBackgroundColor_Impl, CheckBox&, void )
{
    if(m_pCbBackgroundColor->IsChecked())
        m_pLbBackgroundColor->Enable();
    else
        m_pLbBackgroundColor->Disable();
    m_rXFSet.Put( XFillBackgroundItem( m_pCbBackgroundColor->IsChecked() ) );
    ModifiedBackgroundHdl_Impl(*m_pLbBackgroundColor);
}

IMPL_LINK_NOARG( SvxHatchTabPage, ModifiedBackgroundHdl_Impl, SvxColorListBox&, void )
{
    Color aColor(COL_TRANSPARENT);
    if(m_pCbBackgroundColor->IsChecked())
    {
        aColor = m_pLbBackgroundColor->GetSelectEntryColor();
        m_pCtlPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
        m_pCtlPreview->Invalidate();
    }
    m_rXFSet.Put(XFillColorItem( OUString(), m_pLbBackgroundColor->GetSelectEntryColor() ));

    m_pCtlPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pCtlPreview->Invalidate();
}

IMPL_LINK( SvxHatchTabPage, ModifiedEditHdl_Impl, Edit&, rEdit, void )
{
    ModifiedHdl_Impl(&rEdit);
    m_pHatchLB->SetNoSelection();
}

IMPL_LINK( SvxHatchTabPage, ModifiedSliderHdl_Impl, Slider*, rSlider, void )
{
    ModifiedHdl_Impl(rSlider);
    m_pHatchLB->SetNoSelection();
}
void SvxHatchTabPage::ModifiedHdl_Impl( void* p )
{
    if( p == m_pMtrAngle )
        m_pSliderAngle->SetThumbPos( m_pMtrAngle->GetValue() );

    if( p == m_pSliderAngle )
        m_pMtrAngle->SetValue( m_pSliderAngle->GetThumbPos() );

    XHatch aXHatch( m_pLbLineColor->GetSelectEntryColor(),
                    (css::drawing::HatchStyle) m_pLbLineType->GetSelectEntryPos(),
                    GetCoreValue( *m_pMtrDistance, m_ePoolUnit ),
                    static_cast<long>(m_pMtrAngle->GetValue() * 10) );

    m_rXFSet.Put( XFillHatchItem( OUString(), aXHatch ) );

    m_pCtlPreview->SetAttributes( m_aXFillAttr.GetItemSet() );
    m_pCtlPreview->Invalidate();
}

IMPL_LINK_NOARG(SvxHatchTabPage, ChangeHatchHdl, ValueSet*, void)
{
    ChangeHatchHdl_Impl();
}

void SvxHatchTabPage::ChangeHatchHdl_Impl()
{
    std::unique_ptr<XHatch> pHatch;
    size_t nPos = m_pHatchLB->GetSelectItemPos();

    if( nPos != VALUESET_ITEM_NOTFOUND )
        pHatch.reset(new XHatch( m_pHatchingList->GetHatch( static_cast<sal_uInt16>(nPos) )->GetHatch() ));
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
            sal_uInt16 nPosition = m_pHatchLB->GetItemId( 0 );
            m_pHatchLB->SelectItem( nPosition );
            if( nPosition != 0 )
                pHatch.reset( new XHatch( m_pHatchingList->GetHatch( 0 )->GetHatch() ) );
        }
    }
    if( pHatch )
    {
        m_pLbLineType->SelectEntryPos(
            sal::static_int_cast< sal_Int32 >( pHatch->GetHatchStyle() ) );
        m_pLbLineColor->SetNoSelection();
        m_pLbLineColor->SelectEntry( pHatch->GetColor() );
        SetMetricValue( *m_pMtrDistance, pHatch->GetDistance(), m_ePoolUnit );
        long mHatchAngle = pHatch->GetAngle() / 10;
        m_pMtrAngle->SetValue( mHatchAngle );
        m_pSliderAngle->SetThumbPos( mHatchAngle );

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
    m_pLbBackgroundColor->SaveValue();
}

IMPL_LINK_NOARG(SvxHatchTabPage, ClickAddHdl_Impl, Button*, void)
{
    OUString aNewName( SvxResId( RID_SVXSTR_HATCH ) );
    OUString aDesc( CuiResId( RID_SVXSTR_DESC_HATCH ) );
    OUString aName;

    long nCount = m_pHatchingList->Count();
    long j = 1;
    bool bValidHatchName = false;

    while( !bValidHatchName )
    {
        aName  = aNewName + " " + OUString::number( j++ );
        bValidHatchName = (SearchHatchList(aName) == LISTBOX_ENTRY_NOTFOUND);
    }

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    assert(pFact && "Dialog creation failed!");
    ScopedVclPtr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc ));
    assert(pDlg && "Dialog creation failed!");
    ScopedVclPtr<MessageDialog> pWarnBox;
    sal_uInt16         nError   = 1;

    while( pDlg->Execute() == RET_OK )
    {
        pDlg->GetName( aName );

        bValidHatchName = (SearchHatchList(aName) == LISTBOX_ENTRY_NOTFOUND);
        if( bValidHatchName )
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
        XHatch aXHatch( m_pLbLineColor->GetSelectEntryColor(),
                        (css::drawing::HatchStyle) m_pLbLineType->GetSelectEntryPos(),
                        GetCoreValue( *m_pMtrDistance, m_ePoolUnit ),
                        static_cast<long>(m_pMtrAngle->GetValue() * 10) );

        m_pHatchingList->Insert(o3tl::make_unique<XHatchEntry>(aXHatch, aName), nCount);

        sal_Int32 nId = m_pHatchLB->GetItemId(nCount - 1); // calculate the last ID
        Bitmap aBitmap = m_pHatchingList->GetBitmapForPreview( nCount, m_pHatchLB->GetIconSize() );
        // Insert the new entry at the next ID
        m_pHatchLB->InsertItem( nId + 1, Image(aBitmap), aName );
        m_pHatchLB->SelectItem( nId + 1 );
        m_pHatchLB->Resize();

        *m_pnHatchingListState |= ChangeType::MODIFIED;

        ChangeHatchHdl_Impl();
    }
}

IMPL_LINK_NOARG(SvxHatchTabPage, ClickModifyHdl_Impl, Button*, void)
{
    sal_uInt16 nId = m_pHatchLB->GetSelectItemId();
    size_t nPos = m_pHatchLB->GetSelectItemPos();

    if( nPos != VALUESET_ITEM_NOTFOUND )
    {
        OUString aName( m_pHatchingList->GetHatch( static_cast<sal_uInt16>(nPos) )->GetName() );

        XHatch aXHatch( m_pLbLineColor->GetSelectEntryColor(),
                        (css::drawing::HatchStyle) m_pLbLineType->GetSelectEntryPos(),
                         GetCoreValue( *m_pMtrDistance, m_ePoolUnit ),
                        static_cast<long>(m_pMtrAngle->GetValue() * 10) );

        m_pHatchingList->Replace(o3tl::make_unique<XHatchEntry>(aXHatch, aName), nPos);

        Bitmap aBitmap = m_pHatchingList->GetBitmapForPreview( static_cast<sal_uInt16>(nPos), m_pHatchLB->GetIconSize() );
        m_pHatchLB->RemoveItem( nId );
        m_pHatchLB->InsertItem( nId, Image(aBitmap), aName, static_cast<sal_uInt16>(nPos) );
        m_pHatchLB->SelectItem( nId );

        // save values for changes recognition (-> method)
        m_pMtrDistance->SaveValue();
        m_pMtrAngle->SaveValue();
        m_pLbLineType->SaveValue();
        m_pLbLineColor->SaveValue();
        m_pLbBackgroundColor->SaveValue();

        *m_pnHatchingListState |= ChangeType::MODIFIED;
    }
}

IMPL_LINK_NOARG(SvxHatchTabPage, ClickDeleteHdl_Impl, SvxPresetListBox*, void)
{
    sal_uInt16 nId = m_pHatchLB->GetSelectItemId();
    size_t nPos = m_pHatchLB->GetSelectItemPos();

    if( nPos != VALUESET_ITEM_NOTFOUND )
    {
        ScopedVclPtrInstance< MessageDialog > aQueryBox( GetParentDialog(),"AskDelHatchDialog","cui/ui/querydeletehatchdialog.ui");

        if( aQueryBox->Execute() == RET_YES )
        {
            m_pHatchingList->Remove(nPos);
            m_pHatchLB->RemoveItem( nId );
            nId = m_pHatchLB->GetItemId(0);
            m_pHatchLB->SelectItem( nId );
            m_pHatchLB->Resize();

            m_pCtlPreview->Invalidate();

            ChangeHatchHdl_Impl();

            *m_pnHatchingListState |= ChangeType::MODIFIED;
        }
    }
}

IMPL_LINK_NOARG(SvxHatchTabPage, ClickRenameHdl_Impl, SvxPresetListBox*, void )
{
    sal_uInt16 nId = m_pHatchLB->GetSelectItemId();
    size_t nPos = m_pHatchLB->GetSelectItemPos();

    if( nPos != VALUESET_ITEM_NOTFOUND )
    {
        OUString aDesc( CuiResId( RID_SVXSTR_DESC_HATCH ) );
        OUString aName( m_pHatchingList->GetHatch( nPos )->GetName() );

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        assert(pFact && "Dialog creation failed!");
        ScopedVclPtr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc ));
        assert(pDlg && "Dialog creation failed!");

        bool bLoop = true;
        while( bLoop && pDlg->Execute() == RET_OK )
        {
            pDlg->GetName( aName );
            sal_Int32 nHatchPos = SearchHatchList( aName );
            bool bValidHatchName = (nHatchPos == static_cast<sal_Int32>(nPos) ) || (nHatchPos == LISTBOX_ENTRY_NOTFOUND);

            if(bValidHatchName)
            {
                bLoop = false;
                m_pHatchingList->GetHatch(nPos)->SetName(aName);

                m_pHatchLB->SetItemText(nId, aName);
                m_pHatchLB->SelectItem( nId );

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

void SvxHatchTabPage::PointChanged( vcl::Window*, RectPoint )
{
}

void SvxHatchTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( ( rDCEvt.GetType() == DataChangedEventType::SETTINGS ) && ( rDCEvt.GetFlags() & AllSettingsFlags::STYLE ) )
        m_pCtlPreview->SetDrawMode( GetSettings().GetStyleSettings().GetHighContrastMode() ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR );

    SvxTabPage::DataChanged( rDCEvt );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
