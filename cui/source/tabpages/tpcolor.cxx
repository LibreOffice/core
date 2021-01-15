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
#include <i18nutil/unicode.hxx>
#include <svtools/colrdlg.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

#include <strings.hrc>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xtable.hxx>
#include <cuitabarea.hxx>
#include <svx/svxdlg.hxx>
#include <dialmgr.hxx>
#include <cuitabline.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <officecfg/Office/Common.hxx>

using namespace com::sun::star;

SvxColorTabPage::SvxColorTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs)
    : SfxTabPage(pPage, pController, "cui/ui/colorpage.ui", "ColorPage", &rInAttrs)
    , rOutAttrs           ( rInAttrs )
    // All the horrific pointers we store and should not
    , pnColorListState( nullptr )
    , aXFillAttr( rInAttrs.GetPool() )
    , rXFSet( aXFillAttr.GetItemSet() )
    , eCM( ColorModel::RGB )
    , m_context(comphelper::getProcessComponentContext())
    , m_xValSetColorList(new SvxColorValueSet(m_xBuilder->weld_scrolled_window("colorsetwin", true)))
    , m_xValSetRecentList(new SvxColorValueSet(nullptr))
    , m_xSelectPalette(m_xBuilder->weld_combo_box("paletteselector"))
    , m_xRbRGB(m_xBuilder->weld_radio_button("RGB"))
    , m_xRbCMYK(m_xBuilder->weld_radio_button("CMYK"))
    , m_xRGBcustom(m_xBuilder->weld_widget("rgbcustom"))
    , m_xRGBpreset(m_xBuilder->weld_widget("rgbpreset"))
    , m_xRpreset(m_xBuilder->weld_entry("R_preset"))
    , m_xGpreset(m_xBuilder->weld_entry("G_preset"))
    , m_xBpreset(m_xBuilder->weld_entry("B_preset"))
    , m_xRcustom(m_xBuilder->weld_spin_button("R_custom"))
    , m_xGcustom(m_xBuilder->weld_spin_button("G_custom"))
    , m_xBcustom(m_xBuilder->weld_spin_button("B_custom"))
    , m_xHexpreset(new weld::HexColorControl(m_xBuilder->weld_entry("hex_preset")))
    , m_xHexcustom(new weld::HexColorControl(m_xBuilder->weld_entry("hex_custom")))
    , m_xCMYKcustom(m_xBuilder->weld_widget("cmykcustom"))
    , m_xCMYKpreset(m_xBuilder->weld_widget("cmykpreset"))
    , m_xCpreset(m_xBuilder->weld_entry("C_preset"))
    , m_xYpreset(m_xBuilder->weld_entry("Y_preset"))
    , m_xMpreset(m_xBuilder->weld_entry("M_preset"))
    , m_xKpreset(m_xBuilder->weld_entry("K_preset"))
    , m_xCcustom(m_xBuilder->weld_metric_spin_button("C_custom", FieldUnit::PERCENT))
    , m_xYcustom(m_xBuilder->weld_metric_spin_button("Y_custom", FieldUnit::PERCENT))
    , m_xMcustom(m_xBuilder->weld_metric_spin_button("M_custom", FieldUnit::PERCENT))
    , m_xKcustom(m_xBuilder->weld_metric_spin_button("K_custom", FieldUnit::PERCENT))
    , m_xBtnAdd(m_xBuilder->weld_button("add"))
    , m_xBtnDelete(m_xBuilder->weld_button("delete"))
    , m_xBtnWorkOn(m_xBuilder->weld_button("edit"))
    , m_xCtlPreviewOld(new weld::CustomWeld(*m_xBuilder, "oldpreview", m_aCtlPreviewOld))
    , m_xCtlPreviewNew(new weld::CustomWeld(*m_xBuilder, "newpreview", m_aCtlPreviewNew))
    , m_xValSetColorListWin(new weld::CustomWeld(*m_xBuilder, "colorset", *m_xValSetColorList))
    , m_xValSetRecentListWin(new weld::CustomWeld(*m_xBuilder, "recentcolorset", *m_xValSetRecentList))
{
    Size aSize(m_xBtnWorkOn->get_approximate_digit_width() * 25,
               m_xBtnWorkOn->get_text_height() * 10);
    m_xValSetColorList->set_size_request(aSize.Width(), aSize.Height());
    aSize = Size(m_xBtnWorkOn->get_approximate_digit_width() * 8,
                 m_xBtnWorkOn->get_text_height() * 3);
    m_aCtlPreviewOld.set_size_request(aSize.Width(), aSize.Height());
    m_aCtlPreviewNew.set_size_request(aSize.Width(), aSize.Height());
    // this page needs ExchangeSupport
    SetExchangeSupport();

    // setting the output device
    rXFSet.Put( XFillStyleItem(drawing::FillStyle_SOLID) );
    rXFSet.Put( XFillColorItem(OUString(), COL_BLACK) );
    m_aCtlPreviewOld.SetAttributes( aXFillAttr.GetItemSet() );
    m_aCtlPreviewNew.SetAttributes( aXFillAttr.GetItemSet() );

    // set handler
    m_xSelectPalette->connect_changed(LINK(this, SvxColorTabPage, SelectPaletteLBHdl));
    Link<ValueSet*, void> aValSelectLink = LINK(this, SvxColorTabPage, SelectValSetHdl_Impl);
    m_xValSetColorList->SetSelectHdl(aValSelectLink);
    m_xValSetRecentList->SetSelectHdl(aValSelectLink);

    Link<weld::SpinButton&,void> aSpinLink = LINK(this, SvxColorTabPage, SpinValueHdl_Impl);
    m_xRcustom->connect_value_changed(aSpinLink);
    m_xGcustom->connect_value_changed(aSpinLink);
    m_xBcustom->connect_value_changed(aSpinLink);
    Link<weld::Entry&,void> aEntryLink = LINK(this, SvxColorTabPage, ModifiedHdl_Impl);
    m_xHexcustom->connect_changed(aEntryLink);
    Link<weld::MetricSpinButton&,void> aMetricSpinLink = LINK(this, SvxColorTabPage, MetricSpinValueHdl_Impl);
    m_xCcustom->connect_value_changed(aMetricSpinLink);
    m_xYcustom->connect_value_changed(aMetricSpinLink);
    m_xMcustom->connect_value_changed(aMetricSpinLink);
    m_xKcustom->connect_value_changed(aMetricSpinLink);

    Link<weld::ToggleButton&,void> aLink2 = LINK( this, SvxColorTabPage, SelectColorModeHdl_Impl );
    m_xRbRGB->connect_toggled(aLink2);
    m_xRbCMYK->connect_toggled(aLink2);
    SetColorModel( eCM );
    ChangeColorModel();

    m_xBtnAdd->connect_clicked( LINK( this, SvxColorTabPage, ClickAddHdl_Impl ) );
    m_xBtnWorkOn->connect_clicked( LINK( this, SvxColorTabPage, ClickWorkOnHdl_Impl ) );
    m_xBtnDelete->connect_clicked( LINK( this, SvxColorTabPage, ClickDeleteHdl_Impl ) );
    // disable modify buttons
    // Color palettes can't be modified
    m_xBtnDelete->set_sensitive(false);
    m_xBtnDelete->set_tooltip_text( SvxResId(RID_SVXSTR_DELETEUSERCOLOR1) );

    // disable preset color values
    m_xRGBpreset->set_sensitive(false);
    m_xCMYKpreset->set_sensitive(false);

    // ValueSet
    m_xValSetColorList->SetStyle(m_xValSetColorList->GetStyle() | WB_ITEMBORDER);
    m_xValSetColorList->Show();

    m_xValSetRecentList->SetStyle(m_xValSetRecentList->GetStyle() | WB_ITEMBORDER);
    m_xValSetRecentList->Show();

    maPaletteManager.ReloadRecentColorSet(*m_xValSetRecentList);
    aSize = m_xValSetRecentList->layoutAllVisible(maPaletteManager.GetRecentColorCount());
    m_xValSetRecentList->set_size_request(aSize.Width(), aSize.Height());
}

SvxColorTabPage::~SvxColorTabPage()
{
    m_xValSetRecentListWin.reset();
    m_xValSetRecentList.reset();
    m_xValSetColorListWin.reset();
    m_xValSetColorList.reset();
}

void SvxColorTabPage::ImpColorCountChanged()
{
    if (!pColorList.is())
        return;
    m_xValSetColorList->SetColCount(SvxColorValueSet::getColumnCount());
    m_xValSetRecentList->SetColCount(SvxColorValueSet::getColumnCount());
}

void SvxColorTabPage::FillPaletteLB()
{
    m_xSelectPalette->clear();
    std::vector<OUString> aPaletteList = maPaletteManager.GetPaletteList();
    for (auto const& palette : aPaletteList)
    {
        m_xSelectPalette->append_text(palette);
    }
    OUString aPaletteName( officecfg::Office::Common::UserColors::PaletteName::get() );
    m_xSelectPalette->set_active_text(aPaletteName);
    if (m_xSelectPalette->get_active() != -1)
    {
        SelectPaletteLBHdl(*m_xSelectPalette);
    }
}

void SvxColorTabPage::Construct()
{
    if (pColorList.is())
    {
        FillPaletteLB();
        ImpColorCountChanged();
    }
}

void SvxColorTabPage::ActivatePage( const SfxItemSet& )
{
    if( !pColorList.is() )
        return;

    const SfxPoolItem* pPoolItem = nullptr;
    if( SfxItemState::SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLCOLOR ), true, &pPoolItem ) )
    {
        SetColorModel( ColorModel::RGB );
        ChangeColorModel();

        const Color aColor = static_cast<const XFillColorItem*>(pPoolItem)->GetColorValue();
        ChangeColor( aColor );
        sal_Int32 nPos = FindInPalette( aColor );

        if ( nPos != -1 )
            m_xValSetColorList->SelectItem(m_xValSetColorList->GetItemId(nPos));
        // else search in other palettes?

    }

    m_aCtlPreviewOld.SetAttributes(aXFillAttr.GetItemSet());
    m_aCtlPreviewOld.Invalidate();

    SelectValSetHdl_Impl(m_xValSetColorList.get());
}

DeactivateRC SvxColorTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( _pSet )
        FillItemSet( _pSet );

    return DeactivateRC::LeavePage;
}

bool SvxColorTabPage::FillItemSet( SfxItemSet* rSet )
{
    Color aColor = m_xValSetColorList->GetItemColor( m_xValSetColorList->GetSelectedItemId() );
    OUString sColorName;
    if ( aCurrentColor == aColor )
       sColorName = m_xValSetColorList->GetItemText( m_xValSetColorList->GetSelectedItemId() );
    else
       sColorName = "#" + aCurrentColor.AsRGBHexString().toAsciiUpperCase();
    maPaletteManager.AddRecentColor( aCurrentColor, sColorName );
    rSet->Put( XFillColorItem( sColorName, aCurrentColor ) );
    rSet->Put( XFillStyleItem( drawing::FillStyle_SOLID ) );
    return true;
}

void SvxColorTabPage::UpdateModified()
{
    bool bEnable = pColorList.is() && pColorList->Count();
    m_xBtnWorkOn->set_sensitive(bEnable);
}

void SvxColorTabPage::Reset( const SfxItemSet* rSet )
{
    SfxItemState nState = rSet->GetItemState( XATTR_FILLCOLOR );

    Color aNewColor;

    if ( nState >= SfxItemState::DEFAULT )
    {
        XFillColorItem aColorItem( rSet->Get( XATTR_FILLCOLOR ) );
        aPreviousColor = aColorItem.GetColorValue();
        aNewColor = aColorItem.GetColorValue();
    }

    // set color model
    OUString aStr = GetUserData();
    eCM = static_cast<ColorModel>(aStr.toInt32());
    SetColorModel( eCM );
    ChangeColorModel();

    ChangeColor(aNewColor);

    UpdateModified();
}

std::unique_ptr<SfxTabPage> SvxColorTabPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rOutAttrs)
{
    return std::make_unique<SvxColorTabPage>(pPage, pController, *rOutAttrs);
}

// is called when the content of the MtrFields is changed for color values
IMPL_LINK_NOARG(SvxColorTabPage, SpinValueHdl_Impl, weld::SpinButton&, void)
{
    // read current MtrFields, if cmyk, then k-value as transparency
    aCurrentColor = Color(static_cast<sal_uInt8>(PercentToColor_Impl(m_xRcustom->get_value())),
                          static_cast<sal_uInt8>(PercentToColor_Impl(m_xGcustom->get_value())),
                          static_cast<sal_uInt8>(PercentToColor_Impl(m_xBcustom->get_value())));
    UpdateColorValues();

    rXFSet.Put( XFillColorItem( OUString(), aCurrentColor ) );
    m_aCtlPreviewNew.SetAttributes( aXFillAttr.GetItemSet() );

    m_aCtlPreviewNew.Invalidate();
}

IMPL_LINK_NOARG(SvxColorTabPage, MetricSpinValueHdl_Impl, weld::MetricSpinButton&, void)
{
    // read current MtrFields, if cmyk, then k-value as transparency
    aCurrentColor = Color(ColorTransparency, static_cast<sal_uInt8>(PercentToColor_Impl(m_xKcustom->get_value(FieldUnit::NONE))),
                          static_cast<sal_uInt8>(PercentToColor_Impl(m_xCcustom->get_value(FieldUnit::NONE))),
                          static_cast<sal_uInt8>(PercentToColor_Impl(m_xYcustom->get_value(FieldUnit::NONE))),
                          static_cast<sal_uInt8>(PercentToColor_Impl(m_xMcustom->get_value(FieldUnit::NONE))));
    ConvertColorValues (aCurrentColor, ColorModel::RGB);

    rXFSet.Put( XFillColorItem( OUString(), aCurrentColor ) );
    m_aCtlPreviewNew.SetAttributes( aXFillAttr.GetItemSet() );

    m_aCtlPreviewNew.Invalidate();
}

IMPL_LINK_NOARG(SvxColorTabPage, ModifiedHdl_Impl, weld::Entry&, void)
{
    aCurrentColor = m_xHexcustom->GetColor();
    UpdateColorValues();

    rXFSet.Put( XFillColorItem( OUString(), aCurrentColor ) );
    m_aCtlPreviewNew.SetAttributes( aXFillAttr.GetItemSet() );

    m_aCtlPreviewNew.Invalidate();
}

IMPL_LINK_NOARG(SvxColorTabPage, ClickAddHdl_Impl, weld::Button&, void)
{
    OUString aNewName( SvxResId( RID_SVXSTR_COLOR ) );
    OUString aDesc( CuiResId( RID_SVXSTR_DESC_COLOR ) );
    OUString aName;

    tools::Long j = 1;
    bool bValidColorName = false;
    // check if name is already existing
    while (!bValidColorName)
    {
        aName = aNewName + " " + OUString::number( j++ );
        bValidColorName = (FindInCustomColors(aName) == -1);
    }

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    ScopedVclPtr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog(GetFrameWeld(), aName, aDesc));
    sal_uInt16 nError = 1;

    while (pDlg->Execute() == RET_OK)
    {
        pDlg->GetName( aName );

        bValidColorName = (FindInCustomColors(aName) == -1);
        if (bValidColorName)
        {
            nError = 0;
            break;
        }

        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), "cui/ui/queryduplicatedialog.ui"));
        std::unique_ptr<weld::MessageDialog> xWarnBox(xBuilder->weld_message_dialog("DuplicateNameDialog"));
        if (xWarnBox->run() != RET_OK)
            break;
    }

    pDlg.disposeAndClear();

    if (!nError)
    {
        m_xSelectPalette->set_active(0);
        SelectPaletteLBHdl(*m_xSelectPalette);
        std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create(m_context));
        css::uno::Sequence< sal_Int32 > aCustomColorList(officecfg::Office::Common::UserColors::CustomColor::get());
        css::uno::Sequence< OUString > aCustomColorNameList(officecfg::Office::Common::UserColors::CustomColorName::get());
        sal_Int32 nSize = aCustomColorList.getLength();
        aCustomColorList.realloc( nSize + 1 );
        aCustomColorNameList.realloc( nSize + 1 );
        aCustomColorList[nSize] = sal_Int32(aCurrentColor);
        aCustomColorNameList[nSize] = aName;
        officecfg::Office::Common::UserColors::CustomColor::set(aCustomColorList, batch);
        officecfg::Office::Common::UserColors::CustomColorName::set(aCustomColorNameList, batch);
        batch->commit();
        sal_uInt16 nId = m_xValSetColorList->GetItemId(nSize - 1);
        m_xValSetColorList->InsertItem( nId + 1 , aCurrentColor, aName );
        m_xValSetColorList->SelectItem( nId + 1 );
        m_xBtnDelete->set_sensitive(false);
        m_xBtnDelete->set_tooltip_text( SvxResId(RID_SVXSTR_DELETEUSERCOLOR2) );
        ImpColorCountChanged();
    }

    UpdateModified();
}

IMPL_LINK_NOARG(SvxColorTabPage, ClickWorkOnHdl_Impl, weld::Button&, void)
{
    SvColorDialog aColorDlg;

    aColorDlg.SetColor (aCurrentColor);
    aColorDlg.SetMode( svtools::ColorPickerMode::Modify );

    if (aColorDlg.Execute(GetFrameWeld()) == RET_OK)
    {
        Color aPreviewColor = aColorDlg.GetColor();
        aCurrentColor = aPreviewColor;
        UpdateColorValues( false );
        // fill ItemSet and pass it on to XOut
        rXFSet.Put( XFillColorItem( OUString(), aPreviewColor ) );
        //m_aCtlPreviewOld.SetAttributes( aXFillAttr );
        m_aCtlPreviewNew.SetAttributes( aXFillAttr.GetItemSet() );

        m_aCtlPreviewNew.Invalidate();
    }
}

IMPL_LINK_NOARG(SvxColorTabPage, ClickDeleteHdl_Impl, weld::Button&, void)
{
    sal_uInt16 nId = m_xValSetColorList->GetSelectedItemId();
    size_t nPos = m_xValSetColorList->GetSelectItemPos();
    if (m_xSelectPalette->get_active() != 0 || nPos == VALUESET_ITEM_NOTFOUND)
        return;

    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create(m_context));
    css::uno::Sequence< sal_Int32 > aCustomColorList(officecfg::Office::Common::UserColors::CustomColor::get());
    css::uno::Sequence< OUString > aCustomColorNameList(officecfg::Office::Common::UserColors::CustomColorName::get());
    sal_Int32 nSize = aCustomColorList.getLength() - 1;
    for(sal_Int32 nIndex = static_cast<sal_Int32>(nPos);nIndex < nSize;nIndex++)
    {
        aCustomColorList[nIndex] = aCustomColorList[nIndex+1];
        aCustomColorNameList[nIndex] = aCustomColorNameList[nIndex+1];
    }
    aCustomColorList.realloc(nSize);
    aCustomColorNameList.realloc(nSize);
    officecfg::Office::Common::UserColors::CustomColor::set(aCustomColorList, batch);
    officecfg::Office::Common::UserColors::CustomColorName::set(aCustomColorNameList, batch);
    batch->commit();
    m_xValSetColorList->RemoveItem(nId);
    if (m_xValSetColorList->GetItemCount() != 0)
    {
        nId = m_xValSetColorList->GetItemId(0);
        m_xValSetColorList->SelectItem(nId);
        SelectValSetHdl_Impl(m_xValSetColorList.get());
    }
    else
    {
        m_xBtnDelete->set_sensitive(false);
        m_xBtnDelete->set_tooltip_text( SvxResId(RID_SVXSTR_DELETEUSERCOLOR2) );
    }
}

IMPL_LINK_NOARG(SvxColorTabPage, SelectPaletteLBHdl, weld::ComboBox&, void)
{
    m_xValSetColorList->Clear();
    sal_Int32 nPos = m_xSelectPalette->get_active();
    maPaletteManager.SetPalette( nPos );
    maPaletteManager.ReloadColorSet(*m_xValSetColorList);

    if(nPos != maPaletteManager.GetPaletteCount() - 1 && nPos != 0)
    {
        XColorListRef pList = XPropertyList::AsColorList(
                                XPropertyList::CreatePropertyListFromURL(
                                XPropertyListType::Color, maPaletteManager.GetSelectedPalettePath()));
        pList->SetName(maPaletteManager.GetPaletteName());
        if(pList->Load())
        {
            SfxOkDialogController* pController = GetDialogController();
            SvxAreaTabDialog* pArea = dynamic_cast<SvxAreaTabDialog*>(pController);
            SvxLineTabDialog* pLine = dynamic_cast<SvxLineTabDialog*>(pController);
            pColorList = pList;
            if( pArea )
                pArea->SetNewColorList(pList);
            else if( pLine )
                pLine->SetNewColorList(pList);
            else
                SetColorList(pList);
            *pnColorListState |= ChangeType::CHANGED;
            *pnColorListState &= ~ChangeType::MODIFIED;
        }
    }
    if (nPos != 0)
    {
        m_xBtnDelete->set_sensitive(false);
        m_xBtnDelete->set_tooltip_text( SvxResId(RID_SVXSTR_DELETEUSERCOLOR1) );
    }

    m_xValSetColorList->Resize();
}

IMPL_LINK(SvxColorTabPage, SelectValSetHdl_Impl, ValueSet*, pValSet, void)
{
    sal_Int32 nPos = pValSet->GetSelectedItemId();
    if( nPos == 0 )
        return;

    Color aColor = pValSet->GetItemColor( nPos );

    rXFSet.Put( XFillColorItem( OUString(), aColor ) );
    m_aCtlPreviewNew.SetAttributes( aXFillAttr.GetItemSet() );
    m_aCtlPreviewNew.Invalidate();
    ChangeColor(aColor, false);

    if (pValSet == m_xValSetColorList.get())
    {
        m_xValSetRecentList->SetNoSelection();
        if (m_xSelectPalette->get_active() == 0 && m_xValSetColorList->GetSelectedItemId() != 0)
        {
            m_xBtnDelete->set_sensitive(true);
            m_xBtnDelete->set_tooltip_text("");
        }
        else
        {
            m_xBtnDelete->set_sensitive(false);
            m_xBtnDelete->set_tooltip_text( SvxResId(RID_SVXSTR_DELETEUSERCOLOR1) );
        }
    }
    if (pValSet == m_xValSetRecentList.get())
    {
        m_xValSetColorList->SetNoSelection();
        m_xBtnDelete->set_sensitive(false);
        m_xBtnDelete->set_tooltip_text( SvxResId(RID_SVXSTR_DELETEUSERCOLOR2) );
    }
}

void SvxColorTabPage::ConvertColorValues (Color& rColor, ColorModel eModell)
{
    switch (eModell)
    {
        case ColorModel::RGB:
        {
            CmykToRgb_Impl (rColor, static_cast<sal_uInt16>(255 - rColor.GetAlpha()) );
            rColor.SetAlpha (255);
        }
        break;

        case ColorModel::CMYK:
        {
            sal_uInt16 nK;
            RgbToCmyk_Impl (rColor, nK );
            rColor.SetAlpha (255 - static_cast<sal_uInt8>(nK));
        }
        break;
    }
}

IMPL_LINK_NOARG(SvxColorTabPage, SelectColorModeHdl_Impl, weld::ToggleButton&, void)
{
    if (m_xRbRGB->get_active())
        eCM = ColorModel::RGB;
    else if (m_xRbCMYK->get_active())
        eCM = ColorModel::CMYK;
    ChangeColorModel();
    UpdateColorValues();
}

void SvxColorTabPage::ChangeColor(const Color &rNewColor, bool bUpdatePreset )
{
    aPreviousColor = rNewColor;
    aCurrentColor = rNewColor;
    UpdateColorValues( bUpdatePreset );
    // fill ItemSet and pass it on to XOut
    rXFSet.Put( XFillColorItem( OUString(), aCurrentColor ) );
    m_aCtlPreviewNew.SetAttributes(aXFillAttr.GetItemSet());
    m_aCtlPreviewNew.Invalidate();
}

void SvxColorTabPage::SetColorModel( ColorModel eModel )
{
    if (eModel == ColorModel::RGB)
        m_xRbRGB->set_active(true);
    else if (eModel == ColorModel::CMYK)
        m_xRbCMYK->set_active(true);
}

void SvxColorTabPage::ChangeColorModel()
{
    switch( eCM )
    {
        case ColorModel::RGB:
        {
            m_xRGBcustom->show();
            m_xRGBpreset->show();
            m_xCMYKcustom->hide();
            m_xCMYKpreset->hide();
        }
        break;

        case ColorModel::CMYK:
        {
            m_xCMYKcustom->show();
            m_xCMYKpreset->show();
            m_xRGBcustom->hide();
            m_xRGBpreset->hide();
        }
        break;
    }
}

void SvxColorTabPage::UpdateColorValues( bool bUpdatePreset )
{
    if (eCM != ColorModel::RGB)
    {
        ConvertColorValues (aPreviousColor, eCM );
        ConvertColorValues (aCurrentColor, eCM);

        m_xCcustom->set_value( ColorToPercent_Impl( aCurrentColor.GetRed() ), FieldUnit::PERCENT );
        m_xMcustom->set_value( ColorToPercent_Impl( aCurrentColor.GetBlue() ), FieldUnit::PERCENT );
        m_xYcustom->set_value( ColorToPercent_Impl( aCurrentColor.GetGreen() ), FieldUnit::PERCENT );
        m_xKcustom->set_value( ColorToPercent_Impl( 255 - aCurrentColor.GetAlpha() ), FieldUnit::PERCENT );

        if( bUpdatePreset )
        {
            m_xCpreset->set_text(unicode::formatPercent(ColorToPercent_Impl(aPreviousColor.GetRed()),
                                                        Application::GetSettings().GetUILanguageTag()));
            m_xMpreset->set_text(unicode::formatPercent(ColorToPercent_Impl(aPreviousColor.GetBlue()),
                                                        Application::GetSettings().GetUILanguageTag()));
            m_xYpreset->set_text(unicode::formatPercent(ColorToPercent_Impl(aPreviousColor.GetGreen()),
                                                        Application::GetSettings().GetUILanguageTag()));
            m_xKpreset->set_text(unicode::formatPercent(ColorToPercent_Impl(255 - aPreviousColor.GetAlpha()),
                                                        Application::GetSettings().GetUILanguageTag()));
        }

        ConvertColorValues (aPreviousColor, ColorModel::RGB);
        ConvertColorValues (aCurrentColor, ColorModel::RGB);
    }
    else
    {
        m_xRcustom->set_value( ColorToPercent_Impl( aCurrentColor.GetRed() ) );
        m_xGcustom->set_value( ColorToPercent_Impl( aCurrentColor.GetGreen() ) );
        m_xBcustom->set_value( ColorToPercent_Impl( aCurrentColor.GetBlue() ) );
        m_xHexcustom->SetColor( aCurrentColor );

        if( bUpdatePreset )
        {
            m_xRpreset->set_text(OUString::number(ColorToPercent_Impl(aPreviousColor.GetRed())));
            m_xGpreset->set_text(OUString::number(ColorToPercent_Impl(aPreviousColor.GetGreen())));
            m_xBpreset->set_text(OUString::number(ColorToPercent_Impl(aPreviousColor.GetBlue())));
            m_xHexpreset->SetColor( aPreviousColor );
        }
    }
}

sal_Int32 SvxColorTabPage::FindInCustomColors(std::u16string_view aColorName)
{
    css::uno::Sequence< OUString > aCustomColorNameList(officecfg::Office::Common::UserColors::CustomColorName::get());
    tools::Long nCount = aCustomColorNameList.getLength();
    bool bValidColorName = true;
    sal_Int32 nPos = -1;

    for(tools::Long i = 0;i < nCount && bValidColorName;i++)
    {
        if(aColorName == aCustomColorNameList[i])
        {
            nPos = i;
            bValidColorName = false;
        }
    }
    return nPos;
}

sal_Int32 SvxColorTabPage::FindInPalette( const Color& rColor )
{
    return pColorList->GetIndexOfColor(rColor);
}

// A RGB value is converted to a CMYK value - not in an ideal way as
// R is converted into C, G into M and B into Y. The K value is held in an
// extra variable. For further color models one should develop own
// classes which should contain the respective casts.

void SvxColorTabPage::RgbToCmyk_Impl( Color& rColor, sal_uInt16& rK )
{
    sal_uInt16 const nColor1 = 255 - rColor.GetRed();
    sal_uInt16 const nColor2 = 255 - rColor.GetGreen();
    sal_uInt16 const nColor3 = 255 - rColor.GetBlue();

    rK = std::min( std::min( nColor1, nColor2 ), nColor3 );

    rColor.SetRed( sal::static_int_cast< sal_uInt8 >( nColor1 - rK ) );
    rColor.SetGreen( sal::static_int_cast< sal_uInt8 >( nColor2 - rK ) );
    rColor.SetBlue( sal::static_int_cast< sal_uInt8 >( nColor3 - rK ) );
}


// reverse case to RgbToCmyk_Impl (see above)

void SvxColorTabPage::CmykToRgb_Impl( Color& rColor, const sal_uInt16 nK )
{
    tools::Long lTemp;

    lTemp = 255 - ( rColor.GetRed() + nK );

    if( lTemp < 0 )
        lTemp = 0;
    rColor.SetRed( static_cast<sal_uInt8>(lTemp) );

    lTemp = 255 - ( rColor.GetGreen() + nK );

    if( lTemp < 0 )
        lTemp = 0;
    rColor.SetGreen( static_cast<sal_uInt8>(lTemp) );

    lTemp = 255 - ( rColor.GetBlue() + nK );

    if( lTemp < 0 )
        lTemp = 0;
    rColor.SetBlue( static_cast<sal_uInt8>(lTemp) );
}


sal_uInt16 SvxColorTabPage::ColorToPercent_Impl( sal_uInt16 nColor )
{
    sal_uInt16 nValue = 0;

    switch (eCM)
    {
        case ColorModel::RGB :
            nValue = nColor;
            break;

        case ColorModel::CMYK:
            nValue = static_cast<sal_uInt16>( static_cast<double>(nColor) * 100.0 / 255.0 + 0.5 );
            break;
    }

    return nValue;
}


sal_uInt16 SvxColorTabPage::PercentToColor_Impl( sal_uInt16 nPercent )
{
    sal_uInt16 nValue = 0;

    switch (eCM)
    {
        case ColorModel::RGB :
            nValue = nPercent;
            break;

        case ColorModel::CMYK:
            nValue = static_cast<sal_uInt16>( static_cast<double>(nPercent) * 255.0 / 100.0 + 0.5 );
            break;
    }

    return nValue;
}


void SvxColorTabPage::FillUserData()
{
    // the color model is saved in the Ini-file
    SetUserData( OUString::number( static_cast<int>(eCM) ) );
}


void SvxColorTabPage::SetPropertyList( XPropertyListType t, const XPropertyListRef &xRef )
{
    OSL_ASSERT( t == XPropertyListType::Color );
    pColorList = XColorListRef( static_cast<XColorList *>(xRef.get() ) );
}

void SvxColorTabPage::SetColorList( const XColorListRef& pColList )
{
    SetPropertyList( XPropertyListType::Color, XPropertyListRef( ( pColList.get() ) ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
