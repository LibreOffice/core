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
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <sfx2/dialoghelper.hxx>
#include <sfx2/objsh.hxx>

#include <strings.hrc>
#include <svx/drawitem.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xflclit.hxx>
#include <svx/colorbox.hxx>
#include <svx/xtable.hxx>
#include <svx/xflbckit.hxx>
#include <cuitabarea.hxx>
#include <svx/svxdlg.hxx>
#include <dialmgr.hxx>
#include <svx/dlgutil.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/svxids.hrc>
#include <sal/log.hxx>
#include <svtools/unitconv.hxx>
#include <comphelper/lok.hxx>

using namespace com::sun::star;

SvxHatchTabPage::SvxHatchTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs)
    : SfxTabPage(pPage, pController, u"cui/ui/hatchpage.ui"_ustr, u"HatchPage"_ustr, &rInAttrs)
    , m_rOutAttrs(rInAttrs)
    , m_nHatchingListState(ChangeType::NONE)
    , m_pnColorListState(nullptr)
    , m_aXFillAttr(rInAttrs.GetPool())
    , m_rXFSet(m_aXFillAttr.GetItemSet())
    , aIconSize(60, 64)
    , m_xMtrDistance(m_xBuilder->weld_metric_spin_button(u"distancemtr"_ustr, FieldUnit::MM))
    , m_xMtrAngle(m_xBuilder->weld_metric_spin_button(u"anglemtr"_ustr, FieldUnit::DEGREE))
    , m_xSliderAngle(m_xBuilder->weld_scale(u"angleslider"_ustr))
    , m_xLbLineType(m_xBuilder->weld_combo_box(u"linetypelb"_ustr))
    , m_xLbLineColor(new ColorListBox(m_xBuilder->weld_menu_button(u"linecolorlb"_ustr),
                [this]{ return GetDialogController()->getDialog(); }))
    , m_xCbBackgroundColor(m_xBuilder->weld_check_button(u"backgroundcolor"_ustr))
    , m_xLbBackgroundColor(new ColorListBox(m_xBuilder->weld_menu_button(u"backgroundcolorlb"_ustr),
                [this]{ return GetDialogController()->getDialog(); }))
    , m_xHatchLB(m_xBuilder->weld_icon_view(u"hatchpresetlist"_ustr))
    , m_xBtnAdd(m_xBuilder->weld_button(u"add"_ustr))
    , m_xBtnModify(m_xBuilder->weld_button(u"modify"_ustr))
    , m_xCtlPreview(new weld::CustomWeld(*m_xBuilder, u"previewctl"_ustr, m_aCtlPreview))
{
    Size aSize = getDrawPreviewOptimalSize(m_aCtlPreview.GetDrawingArea()->get_ref_device());
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
    m_xHatchLB->connect_selection_changed(LINK(this, SvxHatchTabPage, ChangeHatchHdl));
    m_xHatchLB->connect_mouse_press(LINK(this, SvxHatchTabPage, MousePressHdl));
    m_xHatchLB->connect_query_tooltip(LINK(this, SvxHatchTabPage, QueryTooltipHdl));

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

    m_aCtlPreview.SetDrawMode(Application::GetSettings().GetStyleSettings().GetHighContrastMode() ? OUTPUT_DRAWMODE_CONTRAST : OUTPUT_DRAWMODE_COLOR);
}

SvxHatchTabPage::~SvxHatchTabPage()
{
    m_xCtlPreview.reset();
    m_xHatchLB.reset();
    m_xLbBackgroundColor.reset();
    m_xLbLineColor.reset();

    if (m_nHatchingListState & ChangeType::MODIFIED)
    {
        m_pHatchingList->SetPath(AreaTabHelper::GetPalettePath());
        m_pHatchingList->Save();

        // ToolBoxControls are informed:
        SfxObjectShell* pShell = SfxObjectShell::Current();
        if (pShell)
            pShell->PutItem(SvxHatchListItem(m_pHatchingList, SID_HATCH_LIST));
    }
}

void SvxHatchTabPage::Construct()
{
    FillPresetListBox();
}


void SvxHatchTabPage::FillPresetListBox()
{
    m_xHatchLB->clear();

    m_xHatchLB->freeze();
    for (tools::Long nId = 0; nId < m_pHatchingList->Count(); nId++)
    {
        const OUString aString(m_pHatchingList->GetHatch(nId)->GetName());

        OUString sId = OUString::number(nId);
        BitmapEx aBitmap = m_pHatchingList->GetBitmapForPreview(nId, aIconSize);
        VclPtr<VirtualDevice> aVDev = GetVirtualDevice(aBitmap);

        if (!m_xHatchLB->get_id(nId).isEmpty())
        {
            m_xHatchLB->set_image(nId, aVDev);
            m_xHatchLB->set_id(nId, sId);
            m_xHatchLB->set_text(nId, aString);
        }
        else
        {
            m_xHatchLB->insert(-1, &aString, &sId, aVDev, nullptr);
        }
    }

    m_xHatchLB->thaw();
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
                dynamic_cast<SvxAreaTabDialog*>(GetDialogController()) : nullptr;
            if (pArea)
                m_pColorList = pArea->GetNewColorList();

            ModifiedHdl_Impl( this );
        }

        // determining (possibly cutting) the name
        // and displaying it in the GroupBox
        OUString        aString = CuiResId( RID_CUISTR_TABLE ) + ": ";
        INetURLObject   aURL( m_pHatchingList->GetPath() );

        aURL.Append( m_pHatchingList->GetName() );
        SAL_WARN_IF( aURL.GetProtocol() == INetProtocol::NotValid, "cui.tabpages", "invalid URL" );

        if ( aURL.getBase().getLength() > 18 )
        {
            aString += OUString::Concat(aURL.getBase().subView( 0, 15 )) + "...";
        }
        else
            aString += aURL.getBase();

        sal_Int32 nPos = SearchHatchList( rSet.Get(XATTR_FILLHATCH).GetName() );
        if( nPos != -1)
        {
            m_xHatchLB->select( nPos );
        }
        // colors could have been deleted
        ChangeHatchHdl_Impl();
    }

    const XFillBackgroundItem& aBckItem( rSet.Get(XATTR_FILLBACKGROUND));
    m_rXFSet.Put( aBckItem );

    if (aBckItem.GetValue())
    {
        m_xCbBackgroundColor->set_state(TRISTATE_TRUE);
        const XFillColorItem& aColorItem( rSet.Get(XATTR_FILLCOLOR) );
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

sal_Int32 SvxHatchTabPage::SearchHatchList(std::u16string_view rHatchName)
{
    tools::Long nCount = m_pHatchingList->Count();
    bool bValidHatchName = true;
    sal_Int32 nPos = -1;

    for(tools::Long i = 0;i < nCount && bValidHatchName;i++)
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
    OUString sId = m_xHatchLB->get_selected_id();
    sal_Int32 nPos = !sId.isEmpty() ? sId.toInt32() : -1;

    if( nPos != -1 )
    {
        pXHatch.reset(new XHatch( m_pHatchingList->GetHatch( static_cast<sal_uInt16>(nPos) )->GetHatch() ));
        aString = m_pHatchingList->GetHatch(nPos)->GetName();
    }
    // unidentified hatch has been passed
    else
    {
        pXHatch.reset(new XHatch( m_xLbLineColor->GetSelectEntryColor(),
                    static_cast<css::drawing::HatchStyle>(m_xLbLineType->get_active()),
                    GetCoreValue( *m_xMtrDistance, m_ePoolUnit ),
                    Degree10(static_cast<sal_Int16>(m_xMtrAngle->get_value(FieldUnit::NONE) * 10)) ));
    }
    assert( pXHatch && "XHatch couldn't be created" );
    rSet->Put( XFillStyleItem( drawing::FillStyle_HATCH ) );
    rSet->Put( XFillHatchItem( aString, *pXHatch ) );
    rSet->Put( XFillBackgroundItem( m_xCbBackgroundColor->get_active() ) );
    if (m_xCbBackgroundColor->get_active())
    {
        NamedColor aColor = m_xLbBackgroundColor->GetSelectedEntry();
        rSet->Put(XFillColorItem(aColor.m_aName, aColor.m_aColor));
    }
    return true;
}

void SvxHatchTabPage::Reset( const SfxItemSet* rSet )
{
    ChangeHatchHdl_Impl();

    const XFillColorItem& aColItem( rSet->Get(XATTR_FILLCOLOR) );
    m_xLbBackgroundColor->SelectEntry(aColItem.GetColorValue());
    m_rXFSet.Put( aColItem );

    const XFillBackgroundItem& aBckItem( rSet->Get(XATTR_FILLBACKGROUND) );
    if(aBckItem.GetValue())
        m_xCbBackgroundColor->set_state(TRISTATE_TRUE);
    else
        m_xCbBackgroundColor->set_state(TRISTATE_FALSE);
    m_rXFSet.Put( aBckItem );

    m_aCtlPreview.SetAttributes( m_aXFillAttr.GetItemSet() );
    m_aCtlPreview.Invalidate();
}

std::unique_ptr<SfxTabPage> SvxHatchTabPage::Create( weld::Container* pPage, weld::DialogController* pController,
                                            const SfxItemSet* rSet )
{
    return std::make_unique<SvxHatchTabPage>(pPage, pController, *rSet);
}

IMPL_LINK( SvxHatchTabPage, ModifiedListBoxHdl_Impl, weld::ComboBox&, rListBox, void )
{
    ModifiedHdl_Impl(&rListBox);
    // hatch params have changed, it is no longer one of the presets
}

IMPL_LINK( SvxHatchTabPage, ModifiedColorListBoxHdl_Impl, ColorListBox&, rListBox, void )
{
    ModifiedHdl_Impl(&rListBox);
}

IMPL_LINK_NOARG( SvxHatchTabPage, ToggleHatchBackgroundColor_Impl, weld::Toggleable&, void )
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
}

IMPL_LINK( SvxHatchTabPage, ModifiedSliderHdl_Impl, weld::Scale&, rSlider, void )
{
    ModifiedHdl_Impl(&rSlider);
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
                    Degree10(static_cast<sal_Int16>(m_xMtrAngle->get_value(FieldUnit::NONE) * 10)) );

    m_rXFSet.Put( XFillHatchItem( OUString(), aXHatch ) );

    m_aCtlPreview.SetAttributes( m_aXFillAttr.GetItemSet() );
    m_aCtlPreview.Invalidate();
}

IMPL_LINK_NOARG(SvxHatchTabPage, ChangeHatchHdl, weld::IconView&, void)
{
    ChangeHatchHdl_Impl();
}

void SvxHatchTabPage::ChangeHatchHdl_Impl()
{
    std::unique_ptr<XHatch> pHatch;
    OUString sId = m_xHatchLB->get_selected_id();
    sal_Int32 nPos = !sId.isEmpty() ? sId.toInt32() : -1;

    if( nPos != -1 )
        pHatch.reset(new XHatch( m_pHatchingList->GetHatch( static_cast<sal_uInt16>(nPos) )->GetHatch() ));
    else
    {
        if( const XFillStyleItem* pFillStyleItem = m_rOutAttrs.GetItemIfSet( GetWhich( XATTR_FILLSTYLE ) ) )
        {
            const XFillHatchItem* pFillHatchItem;
            if( ( drawing::FillStyle_HATCH == pFillStyleItem->GetValue() ) &&
                ( pFillHatchItem = m_rOutAttrs.GetItemIfSet( GetWhich( XATTR_FILLHATCH ) ) ) )
            {
                pHatch.reset(new XHatch( pFillHatchItem->GetHatchValue() ));
            }
        }
        if(!pHatch && m_xHatchLB->n_children() > 0)
        {
            m_xHatchLB->select(0);
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
        tools::Long nHatchAngle = pHatch->GetAngle().get() / 10;
        m_xMtrAngle->set_value(nHatchAngle, FieldUnit::NONE);
        m_xSliderAngle->set_value(nHatchAngle);

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
    OUString aDesc( CuiResId( RID_CUISTR_DESC_HATCH ) );
    OUString aName;

    tools::Long nCount = m_pHatchingList->Count();
    tools::Long j = 1;
    bool bValidHatchName = false;

    while( !bValidHatchName )
    {
        aName  = aNewName + " " + OUString::number( j++ );
        bValidHatchName = (SearchHatchList(aName) == -1);
    }

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    ScopedVclPtr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog(GetFrameWeld(), aName, aDesc));
    sal_uInt16         nError   = 1;

    while( pDlg->Execute() == RET_OK )
    {
        aName = pDlg->GetName();

        bValidHatchName = (SearchHatchList(aName) == -1);
        if( bValidHatchName )
        {
            nError = 0;
            break;
        }

        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), u"cui/ui/queryduplicatedialog.ui"_ustr));
        std::unique_ptr<weld::MessageDialog> xWarnBox(xBuilder->weld_message_dialog(u"DuplicateNameDialog"_ustr));
        if (xWarnBox->run() != RET_OK)
            break;
    }
    pDlg.disposeAndClear();

    if( nError )
        return;

    XHatch aXHatch( m_xLbLineColor->GetSelectEntryColor(),
                    static_cast<css::drawing::HatchStyle>(m_xLbLineType->get_active()),
                    GetCoreValue( *m_xMtrDistance, m_ePoolUnit ),
                    Degree10(static_cast<sal_Int16>(m_xMtrAngle->get_value(FieldUnit::NONE) * 10)) );

    m_pHatchingList->Insert(std::make_unique<XHatchEntry>(aXHatch, aName), nCount);

    OUString sId = nCount > 0 ? m_xHatchLB->get_id( nCount - 1 ) : OUString();
    sal_Int32 nId = !sId.isEmpty() ? sId.toInt32() : -1;
    BitmapEx aBitmap = m_pHatchingList->GetBitmapForPreview( nCount, aIconSize );
    VclPtr<VirtualDevice> pVDev = GetVirtualDevice(aBitmap);

    m_xHatchLB->insert( nId + 1, &aName, &sId, pVDev, nullptr);
    FillPresetListBox();
    m_xHatchLB->select( nId + 1 );

    m_nHatchingListState |= ChangeType::MODIFIED;

    ChangeHatchHdl_Impl();
}

IMPL_LINK_NOARG(SvxHatchTabPage, ClickModifyHdl_Impl, weld::Button&, void)
{
    OUString sId = m_xHatchLB->get_selected_id();
    sal_Int32 nPos = !sId.isEmpty() ? sId.toInt32() : -1;

    if ( nPos == -1 )
        return;

    OUString aName( m_pHatchingList->GetHatch( static_cast<sal_uInt16>(nPos) )->GetName() );

    XHatch aXHatch( m_xLbLineColor->GetSelectEntryColor(),
                    static_cast<css::drawing::HatchStyle>(m_xLbLineType->get_active()),
                     GetCoreValue( *m_xMtrDistance, m_ePoolUnit ),
                    Degree10(static_cast<sal_Int16>(m_xMtrAngle->get_value(FieldUnit::NONE) * 10)) );

    m_pHatchingList->Replace(std::make_unique<XHatchEntry>(aXHatch, aName), nPos);

    BitmapEx aBitmap = m_pHatchingList->GetBitmapForPreview( static_cast<sal_uInt16>(nPos), aIconSize );
    VclPtr<VirtualDevice> pVDev = GetVirtualDevice(aBitmap);

    m_xHatchLB->remove( nPos );
    m_xHatchLB->insert( nPos, &aName, &sId, pVDev, nullptr);
    FillPresetListBox();
    m_xHatchLB->select( nPos );

    // save values for changes recognition (-> method)
    m_xMtrDistance->save_value();
    m_xMtrAngle->save_value();
    m_xLbLineType->save_value();
    m_xLbLineColor->SaveValue();
    m_xLbBackgroundColor->SaveValue();

    m_nHatchingListState |= ChangeType::MODIFIED;
}

VclPtr<VirtualDevice> SvxHatchTabPage::GetVirtualDevice(BitmapEx aBitmap)
{
    VclPtr<VirtualDevice> pVDev = VclPtr<VirtualDevice>::Create();
    const Point aNull(0, 0);
    if (pVDev->GetDPIScaleFactor() > 1)
        aBitmap.Scale(pVDev->GetDPIScaleFactor(), pVDev->GetDPIScaleFactor());
    const Size aSize(aBitmap.GetSizePixel());
    pVDev->SetOutputSizePixel(aSize);
    pVDev->DrawBitmapEx(aNull, aBitmap);

    return pVDev;
}

IMPL_LINK(SvxHatchTabPage, QueryTooltipHdl, const weld::TreeIter&, rIter, OUString)
{
    OUString sId = m_xHatchLB->get_id(rIter);
    sal_Int32 nId = !sId.isEmpty() ? sId.toInt32() : -1;

    if (nId >= 0)
    {
        return m_pHatchingList->GetHatch(nId)->GetName();
    }
    return OUString();
}

IMPL_LINK(SvxHatchTabPage, MousePressHdl, const MouseEvent&, rMEvt, bool)
{
    if (!rMEvt.IsRight())
        return false;

    // Disable context menu for LibreOfficeKit mode
    if (comphelper::LibreOfficeKit::isActive())
        return false;

    const Point& pPos = rMEvt.GetPosPixel();
    for (int i = 0; i < m_xHatchLB->n_children(); i++)
    {
        const ::tools::Rectangle aRect = m_xHatchLB->get_rect(i);
        if (aRect.Contains(pPos))
        {
            ShowContextMenu(pPos);
            break;
        }
    }
    return false;
}

void SvxHatchTabPage::ShowContextMenu(const Point& pPos)
{
    ::tools::Rectangle aRect(pPos, Size(1, 1));
    std::unique_ptr<weld::Builder> xBuilder(
        Application::CreateBuilder(m_xHatchLB.get(), u"svx/ui/presetmenu.ui"_ustr));
    std::unique_ptr<weld::Menu> xMenu(xBuilder->weld_menu(u"menu"_ustr));

    xMenu->connect_activate(LINK(this, SvxHatchTabPage, OnPopupEnd));
    xMenu->popup_at_rect(m_xHatchLB.get(), aRect);
}

IMPL_LINK(SvxHatchTabPage, OnPopupEnd, const OUString&, sCommand, void)
{
    sLastItemIdent = sCommand;
    if (sLastItemIdent.isEmpty())
        return;

    Application::PostUserEvent(LINK(this, SvxHatchTabPage, MenuSelectAsyncHdl));
}

IMPL_LINK_NOARG(SvxHatchTabPage, MenuSelectAsyncHdl, void*, void)
{
    if (sLastItemIdent == u"rename")
    {
        ClickRenameHdl();
    }
    else if (sLastItemIdent == u"delete")
    {
        ClickDeleteHdl();
    }
}

void SvxHatchTabPage::ClickDeleteHdl()
{
    const OUString sId = m_xHatchLB->get_selected_id();
    const sal_Int32 nPos = !sId.isEmpty() ? sId.toInt32() : -1;

    if( nPos == -1 )
        return;

    std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), u"cui/ui/querydeletehatchdialog.ui"_ustr));
    std::unique_ptr<weld::MessageDialog> xQueryBox(xBuilder->weld_message_dialog(u"AskDelHatchDialog"_ustr));
    if (xQueryBox->run() != RET_YES)
        return;

    m_pHatchingList->Remove(nPos);
    m_xHatchLB->remove( nPos );

    FillPresetListBox();

    sal_Int32 nNextId = nPos;
    if (nPos >= m_xHatchLB->n_children())
        nNextId = m_xHatchLB->n_children() - 1;

    if(m_xHatchLB->n_children() > 0)
        m_xHatchLB->select(nNextId);
    m_aCtlPreview.Invalidate();

    ChangeHatchHdl_Impl();

    m_nHatchingListState |= ChangeType::MODIFIED;
}

void SvxHatchTabPage::ClickRenameHdl()
{
    const OUString sId = m_xHatchLB->get_selected_id();
    const sal_Int32 nPos = !sId.isEmpty() ? sId.toInt32() : -1;

    if ( nPos == -1 )
        return;

    OUString aDesc( CuiResId( RID_CUISTR_DESC_HATCH ) );
    OUString aName( m_pHatchingList->GetHatch( nPos )->GetName() );

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    ScopedVclPtr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog(GetFrameWeld(), aName, aDesc));

    bool bLoop = true;
    while( bLoop && pDlg->Execute() == RET_OK )
    {
        aName = pDlg->GetName();
        sal_Int32 nHatchPos = SearchHatchList( aName );
        bool bValidHatchName = (nHatchPos == nPos ) || (nHatchPos == -1);

        if(bValidHatchName)
        {
            bLoop = false;
            m_pHatchingList->GetHatch(nPos)->SetName(aName);

            m_xHatchLB->set_text(nPos, aName);

            m_nHatchingListState |= ChangeType::MODIFIED;
        }
        else
        {
            std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), u"cui/ui/queryduplicatedialog.ui"_ustr));
            std::unique_ptr<weld::MessageDialog> xBox(xBuilder->weld_message_dialog(u"DuplicateNameDialog"_ustr));
            xBox->run();
        }
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
