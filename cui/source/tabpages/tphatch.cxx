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
#include <vcl/settings.hxx>
#include <vcl/weld.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dialoghelper.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/module.hxx>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>

#include <strings.hrc>
#include <svx/colorbox.hxx>
#include <svx/xattr.hxx>
#include <svx/xpool.hxx>
#include <svx/xtable.hxx>
#include <svx/drawitem.hxx>
#include <cuitabarea.hxx>
#include <defdlgname.hxx>
#include <dlgname.hxx>
#include <svx/svxdlg.hxx>
#include <dialmgr.hxx>
#include <svx/dlgutil.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include <svx/strings.hrc>
#include <sal/log.hxx>

using namespace com::sun::star;

SvxHatchTabPage::SvxHatchTabPage(TabPageParent pParent, const SfxItemSet& rInAttrs)
    : SfxTabPage(pParent, "cui/ui/hatchpage.ui", "HatchPage", &rInAttrs)
    , m_rOutAttrs(rInAttrs)
    , m_pnHatchingListState(nullptr)
    , m_pnColorListState(nullptr)
    , m_aXFillAttr(rInAttrs.GetPool())
    , m_rXFSet(m_aXFillAttr.GetItemSet())
    , m_xMtrDistance(m_xBuilder->weld_metric_spin_button("distancemtr", FieldUnit::MM))
    , m_xMtrAngle(m_xBuilder->weld_metric_spin_button("anglemtr", FieldUnit::DEGREE))
    , m_xSliderAngle(m_xBuilder->weld_scale("angleslider"))
    , m_xLbLineType(m_xBuilder->weld_combo_box("linetypelb"))
    , m_xLbLineColor(new ColorListBox(m_xBuilder->weld_menu_button("linecolorlb"), pParent.GetFrameWeld()))
    , m_xCbBackgroundColor(m_xBuilder->weld_check_button("backgroundcolor"))
    , m_xLbBackgroundColor(new ColorListBox(m_xBuilder->weld_menu_button("backgroundcolorlb"), pParent.GetFrameWeld()))
    , m_xHatchLB(new SvxPresetListBox(m_xBuilder->weld_scrolled_window("hatchpresetlistwin")))
    , m_xBtnAdd(m_xBuilder->weld_button("add"))
    , m_xBtnModify(m_xBuilder->weld_button("modify"))
    , m_xHatchLBWin(new weld::CustomWeld(*m_xBuilder, "hatchpresetlist", *m_xHatchLB))
    , m_xCtlPreview(new weld::CustomWeld(*m_xBuilder, "previewctl", m_aCtlPreview))
{
    Size aSize = getDrawPreviewOptimalSize(this);
    m_xHatchLBWin->set_size_request(aSize.Width(), aSize.Height());
    m_xCtlPreview->set_size_request(aSize.Width(), aSize.Height());

    // this page needs ExchangeSupport
    SetExchangeSupport();

    // adjust metric
    FieldUnit eFUnit = GetModuleFieldUnit( rInAttrs );

    switch ( eFUnit )
    {
        case FieldUnit::M:
        case FieldUnit::KM:
            eFUnit = FieldUnit::MM;
            break;
        default: ;//prevent warning
    }
    SetFieldUnit( *m_xMtrDistance, eFUnit );

    // determine PoolUnit
    SfxItemPool* pPool = m_rOutAttrs.GetPool();
    assert( pPool && "Where is the pool?" );
    m_ePoolUnit = pPool->GetMetric( SID_ATTR_FILL_HATCH );

    // setting the output device
    m_rXFSet.Put( XFillStyleItem(drawing::FillStyle_HATCH) );
    m_rXFSet.Put( XFillHatchItem(OUString(), XHatch()) );
    m_aCtlPreview.SetAttributes( m_aXFillAttr.GetItemSet() );
    m_xHatchLB->SetSelectHdl( LINK( this, SvxHatchTabPage, ChangeHatchHdl ) );
    m_xHatchLB->SetRenameHdl( LINK( this, SvxHatchTabPage, ClickRenameHdl_Impl ) );
    m_xHatchLB->SetDeleteHdl( LINK( this, SvxHatchTabPage, ClickDeleteHdl_Impl ) );

    Link<weld::MetricSpinButton&,void> aLink = LINK( this, SvxHatchTabPage, ModifiedEditHdl_Impl );
    Link<weld::ComboBox&,void> aLink2 = LINK( this, SvxHatchTabPage, ModifiedListBoxHdl_Impl );
    m_xMtrDistance->connect_value_changed( aLink );
    m_xMtrAngle->connect_value_changed( aLink );
    m_xSliderAngle->connect_value_changed(LINK(this, SvxHatchTabPage, ModifiedSliderHdl_Impl));
    m_xLbLineType->connect_changed( aLink2 );
    Link<ColorListBox&,void> aLink3 = LINK( this, SvxHatchTabPage, ModifiedColorListBoxHdl_Impl );
    m_xLbLineColor->SetSelectHdl( aLink3 );
    m_xCbBackgroundColor->connect_toggled( LINK( this, SvxHatchTabPage, ToggleHatchBackgroundColor_Impl ) );
    m_xLbBackgroundColor->SetSelectHdl( LINK( this, SvxHatchTabPage, ModifiedBackgroundHdl_Impl ) );

    m_xBtnAdd->connect_clicked( LINK( this, SvxHatchTabPage, ClickAddHdl_Impl ) );
    m_xBtnModify->connect_clicked( LINK( this, SvxHatchTabPage, ClickModifyHdl_Impl ) );

    m_aCtlPreview.SetDrawMode( GetSettings().GetStyleSettings().GetHighContrastMode() ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR );
}

SvxHatchTabPage::~SvxHatchTabPage()
{
    disposeOnce();
}

void SvxHatchTabPage::dispose()
{
    m_xCtlPreview.reset();
    m_xHatchLBWin.reset();
    m_xHatchLB.reset();
    m_xLbBackgroundColor.reset();
    m_xLbLineColor.reset();
    SfxTabPage::dispose();
}

void SvxHatchTabPage::Construct()
{
    m_xHatchLB->FillPresetListBox(*m_pHatchingList);
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

        sal_Int32 nPos = SearchHatchList( rSet.Get(XATTR_FILLHATCH).GetName() );
        if( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            sal_uInt16 nId = m_xHatchLB->GetItemId( static_cast<size_t>( nPos ) );
            m_xHatchLB->SelectItem( nId );
        }
        // colors could have been deleted
        ChangeHatchHdl_Impl();
    }

    XFillBackgroundItem aBckItem( rSet.Get(XATTR_FILLBACKGROUND));
    m_rXFSet.Put( aBckItem );

    if (aBckItem.GetValue())
    {
        m_xCbBackgroundColor->set_state(TRISTATE_TRUE);
        XFillColorItem aColorItem( rSet.Get(XATTR_FILLCOLOR) );
        Color aColor(aColorItem.GetColorValue());
        m_xLbBackgroundColor->SelectEntry(aColor);
        m_xLbBackgroundColor->set_sensitive(true);
        m_rXFSet.Put( aColorItem );
    }
    else
    {
        m_xCbBackgroundColor->set_state(TRISTATE_FALSE);
        m_xLbBackgroundColor->SelectEntry(COL_AUTO);
        m_xLbBackgroundColor->set_sensitive(false);
    }

    m_aCtlPreview.SetAttributes( m_aXFillAttr.GetItemSet() );
    m_aCtlPreview.Invalidate();
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
    size_t nPos = m_xHatchLB->IsNoSelection() ? VALUESET_ITEM_NOTFOUND : m_xHatchLB->GetSelectItemPos();
    if( nPos != VALUESET_ITEM_NOTFOUND )
    {
        pXHatch.reset(new XHatch( m_pHatchingList->GetHatch( static_cast<sal_uInt16>(nPos) )->GetHatch() ));
        aString = m_xHatchLB->GetItemText( m_xHatchLB->GetSelectedItemId() );
    }
    // unidentified hatch has been passed
    else
    {
        pXHatch.reset(new XHatch( m_xLbLineColor->GetSelectEntryColor(),
                    static_cast<css::drawing::HatchStyle>(m_xLbLineType->get_active()),
                    GetCoreValue( *m_xMtrDistance, m_ePoolUnit ),
                    static_cast<long>(m_xMtrAngle->get_value(FieldUnit::NONE) * 10) ));
    }
    assert( pXHatch && "XHatch couldn't be created" );
    rSet->Put( XFillStyleItem( drawing::FillStyle_HATCH ) );
    rSet->Put( XFillHatchItem( aString, *pXHatch ) );
    rSet->Put( XFillBackgroundItem( m_xCbBackgroundColor->get_active() ) );
    if (m_xCbBackgroundColor->get_active())
    {
        NamedColor aColor = m_xLbBackgroundColor->GetSelectedEntry();
        rSet->Put(XFillColorItem(aColor.second, aColor.first));
    }
    return true;
}

void SvxHatchTabPage::Reset( const SfxItemSet* rSet )
{
    ChangeHatchHdl_Impl();

    XFillColorItem aColItem( rSet->Get(XATTR_FILLCOLOR) );
    m_xLbBackgroundColor->SelectEntry(aColItem.GetColorValue());
    m_rXFSet.Put( aColItem );

    XFillBackgroundItem aBckItem( rSet->Get(XATTR_FILLBACKGROUND) );
    if(aBckItem.GetValue())
        m_xCbBackgroundColor->set_state(TRISTATE_TRUE);
    else
        m_xCbBackgroundColor->set_state(TRISTATE_FALSE);
    m_rXFSet.Put( aBckItem );

    m_aCtlPreview.SetAttributes( m_aXFillAttr.GetItemSet() );
    m_aCtlPreview.Invalidate();
}

VclPtr<SfxTabPage> SvxHatchTabPage::Create( TabPageParent pWindow,
                                            const SfxItemSet* rSet )
{
    return VclPtr<SvxHatchTabPage>::Create(pWindow, *rSet);
}

IMPL_LINK( SvxHatchTabPage, ModifiedListBoxHdl_Impl, weld::ComboBox&, rListBox, void )
{
    ModifiedHdl_Impl(&rListBox);
    // hatch params have changed, it is no longer one of the presets
    m_xHatchLB->SetNoSelection();
}

IMPL_LINK( SvxHatchTabPage, ModifiedColorListBoxHdl_Impl, ColorListBox&, rListBox, void )
{
    ModifiedHdl_Impl(&rListBox);
    m_xHatchLB->SetNoSelection();
}

IMPL_LINK_NOARG( SvxHatchTabPage, ToggleHatchBackgroundColor_Impl, weld::ToggleButton&, void )
{
    if (m_xCbBackgroundColor->get_active())
        m_xLbBackgroundColor->set_sensitive(true);
    else
        m_xLbBackgroundColor->set_sensitive(false);
    m_rXFSet.Put( XFillBackgroundItem( m_xCbBackgroundColor->get_active() ) );
    ModifiedBackgroundHdl_Impl(*m_xLbBackgroundColor);
}

IMPL_LINK_NOARG( SvxHatchTabPage, ModifiedBackgroundHdl_Impl, ColorListBox&, void )
{
    Color aColor(COL_TRANSPARENT);
    if (m_xCbBackgroundColor->get_active())
    {
        aColor = m_xLbBackgroundColor->GetSelectEntryColor();
        m_aCtlPreview.SetAttributes( m_aXFillAttr.GetItemSet() );
        m_aCtlPreview.Invalidate();
    }
    m_rXFSet.Put(XFillColorItem( OUString(), aColor ));

    m_aCtlPreview.SetAttributes( m_aXFillAttr.GetItemSet() );
    m_aCtlPreview.Invalidate();
}

IMPL_LINK( SvxHatchTabPage, ModifiedEditHdl_Impl, weld::MetricSpinButton&, rEdit, void )
{
    ModifiedHdl_Impl(&rEdit);
    m_xHatchLB->SetNoSelection();
}

IMPL_LINK( SvxHatchTabPage, ModifiedSliderHdl_Impl, weld::Scale&, rSlider, void )
{
    ModifiedHdl_Impl(&rSlider);
    m_xHatchLB->SetNoSelection();
}

void SvxHatchTabPage::ModifiedHdl_Impl( void const * p )
{
    if (p == m_xMtrAngle.get())
        m_xSliderAngle->set_value(m_xMtrAngle->get_value(FieldUnit::NONE));

    if (p == m_xSliderAngle.get())
        m_xMtrAngle->set_value(m_xSliderAngle->get_value(), FieldUnit::NONE);

    XHatch aXHatch( m_xLbLineColor->GetSelectEntryColor(),
                    static_cast<css::drawing::HatchStyle>(m_xLbLineType->get_active()),
                    GetCoreValue( *m_xMtrDistance, m_ePoolUnit ),
                    static_cast<long>(m_xMtrAngle->get_value(FieldUnit::NONE) * 10) );

    m_rXFSet.Put( XFillHatchItem( OUString(), aXHatch ) );

    m_aCtlPreview.SetAttributes( m_aXFillAttr.GetItemSet() );
    m_aCtlPreview.Invalidate();
}

IMPL_LINK_NOARG(SvxHatchTabPage, ChangeHatchHdl, SvtValueSet*, void)
{
    ChangeHatchHdl_Impl();
}

void SvxHatchTabPage::ChangeHatchHdl_Impl()
{
    std::unique_ptr<XHatch> pHatch;
    size_t nPos = m_xHatchLB->GetSelectItemPos();

    if( nPos != VALUESET_ITEM_NOTFOUND )
        pHatch.reset(new XHatch( m_pHatchingList->GetHatch( static_cast<sal_uInt16>(nPos) )->GetHatch() ));
    else
    {
        const SfxPoolItem* pPoolItem = nullptr;
        if( SfxItemState::SET == m_rOutAttrs.GetItemState( GetWhich( XATTR_FILLSTYLE ), true, &pPoolItem ) )
        {
            if( ( drawing::FillStyle_HATCH == static_cast<const XFillStyleItem*>( pPoolItem )->GetValue() ) &&
                ( SfxItemState::SET == m_rOutAttrs.GetItemState( GetWhich( XATTR_FILLHATCH ), true, &pPoolItem ) ) )
            {
                pHatch.reset(new XHatch( static_cast<const XFillHatchItem*>( pPoolItem )->GetHatchValue() ));
            }
        }
        if( !pHatch )
        {
            sal_uInt16 nPosition = m_xHatchLB->GetItemId( 0 );
            m_xHatchLB->SelectItem( nPosition );
            if( nPosition != 0 )
                pHatch.reset( new XHatch( m_pHatchingList->GetHatch( 0 )->GetHatch() ) );
        }
    }
    if( pHatch )
    {
        m_xLbLineType->set_active(
            sal::static_int_cast< sal_Int32 >( pHatch->GetHatchStyle() ) );
        m_xLbLineColor->SetNoSelection();
        m_xLbLineColor->SelectEntry( pHatch->GetColor() );
        SetMetricValue( *m_xMtrDistance, pHatch->GetDistance(), m_ePoolUnit );
        long mHatchAngle = pHatch->GetAngle() / 10;
        m_xMtrAngle->set_value(mHatchAngle, FieldUnit::NONE);
        m_xSliderAngle->set_value(mHatchAngle);

        // fill ItemSet and pass it on to m_aCtlPreview
        m_rXFSet.Put( XFillHatchItem( OUString(), *pHatch ) );
        m_aCtlPreview.SetAttributes( m_aXFillAttr.GetItemSet() );

        m_aCtlPreview.Invalidate();
        pHatch.reset();
    }
    m_xMtrDistance->save_value();
    m_xMtrAngle->save_value();
    m_xLbLineType->save_value();
    m_xLbLineColor->SaveValue();
    m_xLbBackgroundColor->SaveValue();
}

IMPL_LINK_NOARG(SvxHatchTabPage, ClickAddHdl_Impl, weld::Button&, void)
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
    ScopedVclPtr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog(GetDialogFrameWeld(), aName, aDesc));
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

        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetDialogFrameWeld(), "cui/ui/queryduplicatedialog.ui"));
        std::unique_ptr<weld::MessageDialog> xWarnBox(xBuilder->weld_message_dialog("DuplicateNameDialog"));
        if (xWarnBox->run() != RET_OK)
            break;
    }
    pDlg.disposeAndClear();

    if( !nError )
    {
        XHatch aXHatch( m_xLbLineColor->GetSelectEntryColor(),
                        static_cast<css::drawing::HatchStyle>(m_xLbLineType->get_active()),
                        GetCoreValue( *m_xMtrDistance, m_ePoolUnit ),
                        static_cast<long>(m_xMtrAngle->get_value(FieldUnit::NONE) * 10) );

        m_pHatchingList->Insert(std::make_unique<XHatchEntry>(aXHatch, aName), nCount);

        sal_Int32 nId = m_xHatchLB->GetItemId(nCount - 1); // calculate the last ID
        BitmapEx aBitmap = m_pHatchingList->GetBitmapForPreview( nCount, m_xHatchLB->GetIconSize() );
        // Insert the new entry at the next ID
        m_xHatchLB->InsertItem( nId + 1, Image(aBitmap), aName );
        m_xHatchLB->SelectItem( nId + 1 );
        m_xHatchLB->Resize();

        *m_pnHatchingListState |= ChangeType::MODIFIED;

        ChangeHatchHdl_Impl();
    }
}

IMPL_LINK_NOARG(SvxHatchTabPage, ClickModifyHdl_Impl, weld::Button&, void)
{
    sal_uInt16 nId = m_xHatchLB->GetSelectedItemId();
    size_t nPos = m_xHatchLB->GetSelectItemPos();

    if( nPos != VALUESET_ITEM_NOTFOUND )
    {
        OUString aName( m_pHatchingList->GetHatch( static_cast<sal_uInt16>(nPos) )->GetName() );

        XHatch aXHatch( m_xLbLineColor->GetSelectEntryColor(),
                        static_cast<css::drawing::HatchStyle>(m_xLbLineType->get_active()),
                         GetCoreValue( *m_xMtrDistance, m_ePoolUnit ),
                        static_cast<long>(m_xMtrAngle->get_value(FieldUnit::NONE) * 10) );

        m_pHatchingList->Replace(std::make_unique<XHatchEntry>(aXHatch, aName), nPos);

        BitmapEx aBitmap = m_pHatchingList->GetBitmapForPreview( static_cast<sal_uInt16>(nPos), m_xHatchLB->GetIconSize() );
        m_xHatchLB->RemoveItem( nId );
        m_xHatchLB->InsertItem( nId, Image(aBitmap), aName, static_cast<sal_uInt16>(nPos) );
        m_xHatchLB->SelectItem( nId );

        // save values for changes recognition (-> method)
        m_xMtrDistance->save_value();
        m_xMtrAngle->save_value();
        m_xLbLineType->save_value();
        m_xLbLineColor->SaveValue();
        m_xLbBackgroundColor->SaveValue();

        *m_pnHatchingListState |= ChangeType::MODIFIED;
    }
}

IMPL_LINK_NOARG(SvxHatchTabPage, ClickDeleteHdl_Impl, SvxPresetListBox*, void)
{
    sal_uInt16 nId = m_xHatchLB->GetSelectedItemId();
    size_t nPos = m_xHatchLB->GetSelectItemPos();

    if( nPos != VALUESET_ITEM_NOTFOUND )
    {
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetDialogFrameWeld(), "cui/ui/querydeletehatchdialog.ui"));
        std::unique_ptr<weld::MessageDialog> xQueryBox(xBuilder->weld_message_dialog("AskDelHatchDialog"));
        if (xQueryBox->run() == RET_YES)
        {
            m_pHatchingList->Remove(nPos);
            m_xHatchLB->RemoveItem( nId );
            nId = m_xHatchLB->GetItemId(0);
            m_xHatchLB->SelectItem( nId );
            m_xHatchLB->Resize();

            m_aCtlPreview.Invalidate();

            ChangeHatchHdl_Impl();

            *m_pnHatchingListState |= ChangeType::MODIFIED;
        }
    }
}

IMPL_LINK_NOARG(SvxHatchTabPage, ClickRenameHdl_Impl, SvxPresetListBox*, void )
{
    sal_uInt16 nId = m_xHatchLB->GetSelectedItemId();
    size_t nPos = m_xHatchLB->GetSelectItemPos();

    if( nPos != VALUESET_ITEM_NOTFOUND )
    {
        OUString aDesc( CuiResId( RID_SVXSTR_DESC_HATCH ) );
        OUString aName( m_pHatchingList->GetHatch( nPos )->GetName() );

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        ScopedVclPtr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog(GetDialogFrameWeld(), aName, aDesc));

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

                m_xHatchLB->SetItemText(nId, aName);
                m_xHatchLB->SelectItem( nId );

                *m_pnHatchingListState |= ChangeType::MODIFIED;
            }
            else
            {
                std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetDialogFrameWeld(), "cui/ui/queryduplicatedialog.ui"));
                std::unique_ptr<weld::MessageDialog> xBox(xBuilder->weld_message_dialog("DuplicateNameDialog"));
                xBox->run();
            }
        }
    }

}

void SvxHatchTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( ( rDCEvt.GetType() == DataChangedEventType::SETTINGS ) && ( rDCEvt.GetFlags() & AllSettingsFlags::STYLE ) )
        m_aCtlPreview.SetDrawMode( GetSettings().GetStyleSettings().GetHighContrastMode() ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR );

    SfxTabPage::DataChanged( rDCEvt );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
