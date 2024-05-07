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
#include <osl/diagnose.h>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/propertyvalue.hxx>

using namespace com::sun::star;

SvxColorTabPage::SvxColorTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs)
    : SfxTabPage(pPage, pController, u"cui/ui/colorpage.ui"_ustr, u"ColorPage"_ustr, &rInAttrs)
    , rOutAttrs           ( rInAttrs )
    // All the horrific pointers we store and should not
    , pnColorListState( nullptr )
    , aXFillAttr( rInAttrs.GetPool() )
    , rXFSet( aXFillAttr.GetItemSet() )
    , eCM( ColorModel::RGB )
    , m_xValSetColorList(new SvxColorValueSet(m_xBuilder->weld_scrolled_window(u"colorsetwin"_ustr, true)))
    , m_xValSetRecentList(new SvxColorValueSet(nullptr))
    , m_xSelectPalette(m_xBuilder->weld_combo_box(u"paletteselector"_ustr))
    , m_xRbRGB(m_xBuilder->weld_radio_button(u"RGB"_ustr))
    , m_xRbCMYK(m_xBuilder->weld_radio_button(u"CMYK"_ustr))
    , m_xRGBcustom(m_xBuilder->weld_widget(u"rgbcustom"_ustr))
    , m_xRGBpreset(m_xBuilder->weld_widget(u"rgbpreset"_ustr))
    , m_xRpreset(m_xBuilder->weld_entry(u"R_preset"_ustr))
    , m_xGpreset(m_xBuilder->weld_entry(u"G_preset"_ustr))
    , m_xBpreset(m_xBuilder->weld_entry(u"B_preset"_ustr))
    , m_xRcustom(m_xBuilder->weld_spin_button(u"R_custom"_ustr))
    , m_xGcustom(m_xBuilder->weld_spin_button(u"G_custom"_ustr))
    , m_xBcustom(m_xBuilder->weld_spin_button(u"B_custom"_ustr))
    , m_xHexpreset(new weld::HexColorControl(m_xBuilder->weld_entry(u"hex_preset"_ustr)))
    , m_xHexcustom(new weld::HexColorControl(m_xBuilder->weld_entry(u"hex_custom"_ustr)))
    , m_xCMYKcustom(m_xBuilder->weld_widget(u"cmykcustom"_ustr))
    , m_xCMYKpreset(m_xBuilder->weld_widget(u"cmykpreset"_ustr))
    , m_xCpreset(m_xBuilder->weld_entry(u"C_preset"_ustr))
    , m_xYpreset(m_xBuilder->weld_entry(u"Y_preset"_ustr))
    , m_xMpreset(m_xBuilder->weld_entry(u"M_preset"_ustr))
    , m_xKpreset(m_xBuilder->weld_entry(u"K_preset"_ustr))
    , m_xCcustom(m_xBuilder->weld_metric_spin_button(u"C_custom"_ustr, FieldUnit::PERCENT))
    , m_xYcustom(m_xBuilder->weld_metric_spin_button(u"Y_custom"_ustr, FieldUnit::PERCENT))
    , m_xMcustom(m_xBuilder->weld_metric_spin_button(u"M_custom"_ustr, FieldUnit::PERCENT))
    , m_xKcustom(m_xBuilder->weld_metric_spin_button(u"K_custom"_ustr, FieldUnit::PERCENT))
    , m_xBtnAdd(m_xBuilder->weld_button(u"add"_ustr))
    , m_xBtnDelete(m_xBuilder->weld_button(u"delete"_ustr))
    , m_xBtnWorkOn(m_xBuilder->weld_button(u"edit"_ustr))
    , m_xMoreColors(m_xBuilder->weld_button(u"btnMoreColors"_ustr))
    , m_xCtlPreviewOld(new weld::CustomWeld(*m_xBuilder, u"oldpreview"_ustr, m_aCtlPreviewOld))
    , m_xCtlPreviewNew(new weld::CustomWeld(*m_xBuilder, u"newpreview"_ustr, m_aCtlPreviewNew))
    , m_xValSetColorListWin(new weld::CustomWeld(*m_xBuilder, u"colorset"_ustr, *m_xValSetColorList))
    , m_xValSetRecentListWin(new weld::CustomWeld(*m_xBuilder, u"recentcolorset"_ustr, *m_xValSetRecentList))
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

    Link<weld::Toggleable&,void> aLink2 = LINK( this, SvxColorTabPage, SelectColorModeHdl_Impl );
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
    m_xBtnDelete->set_tooltip_text( CuiResId(RID_CUISTR_DELETEUSERCOLOR1) );

    m_xMoreColors->connect_clicked(LINK(this, SvxColorTabPage, OnMoreColorsClick));

    // disable preset color values
    m_xRGBpreset->set_sensitive(false);
    m_xCMYKpreset->set_sensitive(false);

    // ValueSet
    m_xValSetColorList->SetStyle(m_xValSetColorList->GetStyle() |
            WB_FLATVALUESET | WB_ITEMBORDER | WB_NO_DIRECTSELECT | WB_TABSTOP);
    m_xValSetColorList->Show();

    m_xValSetRecentList->SetStyle(m_xValSetRecentList->GetStyle() |
            WB_FLATVALUESET | WB_ITEMBORDER | WB_NO_DIRECTSELECT | WB_TABSTOP);
    m_xValSetRecentList->Show();

    maPaletteManager.ReloadRecentColorSet(*m_xValSetRecentList);
    aSize = m_xValSetRecentList->layoutAllVisible(maPaletteManager.GetRecentColorCount());
    m_xValSetRecentList->set_size_request(aSize.Width(), aSize.Height());

    // it is not possible to install color palette extensions in Online or mobile apps
    if(comphelper::LibreOfficeKit::isActive())
    {
        m_xMoreColors->hide();
    }
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

    if( const XFillColorItem* pFillColorItem = rOutAttrs.GetItemIfSet( GetWhich( XATTR_FILLCOLOR ) ) )
    {
        SetColorModel( ColorModel::RGB );
        ChangeColorModel();

        const Color aColor = pFillColorItem->GetColorValue();
        NamedColor aNamedColor;
        aNamedColor.m_aColor = aColor;
        ChangeColor(aNamedColor);
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
    if (m_aCurrentColor.m_aColor == aColor)
       sColorName = m_xValSetColorList->GetItemText( m_xValSetColorList->GetSelectedItemId() );
    else
       sColorName = "#" + m_aCurrentColor.m_aColor.AsRGBHexString().toAsciiUpperCase();

    maPaletteManager.AddRecentColor(m_aCurrentColor.m_aColor, sColorName);
    XFillColorItem aColorItem(sColorName, m_aCurrentColor.m_aColor);
    aColorItem.setComplexColor(m_aCurrentColor.getComplexColor());
    rSet->Put(aColorItem);
    rSet->Put(XFillStyleItem(drawing::FillStyle_SOLID));
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
        m_aPreviousColor = aColorItem.GetColorValue();
        aNewColor = aColorItem.GetColorValue();
    }

    // set color model
    OUString aStr = GetUserData();
    eCM = static_cast<ColorModel>(aStr.toInt32());
    SetColorModel( eCM );
    ChangeColorModel();

    NamedColor aColor;
    aColor.m_aColor = aNewColor;
    ChangeColor(aColor);

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
    m_aCurrentColor.m_aColor = Color(static_cast<sal_uInt8>(PercentToColor_Impl(m_xRcustom->get_value())),
                          static_cast<sal_uInt8>(PercentToColor_Impl(m_xGcustom->get_value())),
                          static_cast<sal_uInt8>(PercentToColor_Impl(m_xBcustom->get_value())));
    UpdateColorValues();

    rXFSet.Put( XFillColorItem( OUString(), m_aCurrentColor.m_aColor ) );
    m_aCtlPreviewNew.SetAttributes( aXFillAttr.GetItemSet() );

    m_aCtlPreviewNew.Invalidate();
}

IMPL_LINK_NOARG(SvxColorTabPage, MetricSpinValueHdl_Impl, weld::MetricSpinButton&, void)
{
    // read current MtrFields, if cmyk, then k-value as transparency
    m_aCurrentColor.m_aColor = Color(ColorTransparency, static_cast<sal_uInt8>(PercentToColor_Impl(m_xKcustom->get_value(FieldUnit::NONE))),
                          static_cast<sal_uInt8>(PercentToColor_Impl(m_xCcustom->get_value(FieldUnit::NONE))),
                          static_cast<sal_uInt8>(PercentToColor_Impl(m_xYcustom->get_value(FieldUnit::NONE))),
                          static_cast<sal_uInt8>(PercentToColor_Impl(m_xMcustom->get_value(FieldUnit::NONE))));
    ConvertColorValues (m_aCurrentColor.m_aColor, ColorModel::RGB);

    rXFSet.Put( XFillColorItem( OUString(), m_aCurrentColor.m_aColor ) );
    m_aCtlPreviewNew.SetAttributes( aXFillAttr.GetItemSet() );

    m_aCtlPreviewNew.Invalidate();
}

IMPL_LINK_NOARG(SvxColorTabPage, ModifiedHdl_Impl, weld::Entry&, void)
{
    m_aCurrentColor.m_aColor = m_xHexcustom->GetColor();
    UpdateColorValues();

    rXFSet.Put( XFillColorItem( OUString(), m_aCurrentColor.m_aColor ) );
    m_aCtlPreviewNew.SetAttributes( aXFillAttr.GetItemSet() );

    m_aCtlPreviewNew.Invalidate();
}

IMPL_LINK_NOARG(SvxColorTabPage, ClickAddHdl_Impl, weld::Button&, void)
{
    OUString aNewName( SvxResId( RID_SVXSTR_COLOR ) );
    OUString aDesc( CuiResId( RID_CUISTR_DESC_COLOR ) );
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
        aName = pDlg->GetName();

        bValidColorName = (FindInCustomColors(aName) == -1);
        if (bValidColorName)
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

    if (!nError)
    {
        m_xSelectPalette->set_active(0);
        SelectPaletteLBHdl(*m_xSelectPalette);
        std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
        css::uno::Sequence< sal_Int32 > aCustomColorList(officecfg::Office::Common::UserColors::CustomColor::get());
        css::uno::Sequence< OUString > aCustomColorNameList(officecfg::Office::Common::UserColors::CustomColorName::get());
        sal_Int32 nSize = aCustomColorList.getLength();
        aCustomColorList.realloc( nSize + 1 );
        aCustomColorNameList.realloc( nSize + 1 );
        aCustomColorList.getArray()[nSize] = sal_Int32(m_aCurrentColor.m_aColor);
        aCustomColorNameList.getArray()[nSize] = aName;
        officecfg::Office::Common::UserColors::CustomColor::set(aCustomColorList, batch);
        officecfg::Office::Common::UserColors::CustomColorName::set(aCustomColorNameList, batch);
        batch->commit();
        sal_uInt16 nId = m_xValSetColorList->GetItemId(nSize - 1);
        m_xValSetColorList->InsertItem( nId + 1 , m_aCurrentColor.m_aColor, aName );
        m_xValSetColorList->SelectItem( nId + 1 );
        m_xBtnDelete->set_sensitive(false);
        m_xBtnDelete->set_tooltip_text( CuiResId(RID_CUISTR_DELETEUSERCOLOR2) );
        ImpColorCountChanged();
    }

    UpdateModified();
}

IMPL_LINK_NOARG(SvxColorTabPage, ClickWorkOnHdl_Impl, weld::Button&, void)
{
    SvColorDialog aColorDlg;

    aColorDlg.SetColor (m_aCurrentColor.m_aColor);
    aColorDlg.SetMode( svtools::ColorPickerMode::Modify );

    if (aColorDlg.Execute(GetFrameWeld()) == RET_OK)
    {
        Color aPreviewColor = aColorDlg.GetColor();
        m_aCurrentColor.m_aColor = aPreviewColor;
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

    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    css::uno::Sequence< sal_Int32 > aCustomColorList(officecfg::Office::Common::UserColors::CustomColor::get());
    auto aCustomColorListRange = asNonConstRange(aCustomColorList);
    css::uno::Sequence< OUString > aCustomColorNameList(officecfg::Office::Common::UserColors::CustomColorName::get());
    auto aCustomColorNameListRange = asNonConstRange(aCustomColorNameList);
    sal_Int32 nSize = aCustomColorList.getLength() - 1;
    for(sal_Int32 nIndex = static_cast<sal_Int32>(nPos);nIndex < nSize;nIndex++)
    {
        aCustomColorListRange[nIndex] = aCustomColorList[nIndex+1];
        aCustomColorNameListRange[nIndex] = aCustomColorNameList[nIndex+1];
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
        m_xBtnDelete->set_tooltip_text( CuiResId(RID_CUISTR_DELETEUSERCOLOR2) );
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
        m_xBtnDelete->set_tooltip_text( CuiResId(RID_CUISTR_DELETEUSERCOLOR1) );
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

    NamedColor aNamedColor;
    aNamedColor.m_aColor = aColor;

    if (pValSet == m_xValSetColorList.get() && maPaletteManager.IsThemePaletteSelected())
    {
        sal_uInt16 nThemeIndex;
        sal_uInt16 nEffectIndex;
        if (PaletteManager::GetThemeAndEffectIndex(nPos, nThemeIndex, nEffectIndex))
        {
            aNamedColor.m_nThemeIndex = nThemeIndex;
            maPaletteManager.GetLumModOff(nThemeIndex, nEffectIndex, aNamedColor.m_nLumMod, aNamedColor.m_nLumOff);
        }
    }

    ChangeColor(aNamedColor, false);

    if (pValSet == m_xValSetColorList.get())
    {
        m_xValSetRecentList->SetNoSelection();
        if (m_xSelectPalette->get_active() == 0 && m_xValSetColorList->GetSelectedItemId() != 0)
        {
            m_xBtnDelete->set_sensitive(true);
            m_xBtnDelete->set_tooltip_text(u""_ustr);
        }
        else
        {
            m_xBtnDelete->set_sensitive(false);
            m_xBtnDelete->set_tooltip_text( CuiResId(RID_CUISTR_DELETEUSERCOLOR1) );
        }
    }
    if (pValSet == m_xValSetRecentList.get())
    {
        m_xValSetColorList->SetNoSelection();
        m_xBtnDelete->set_sensitive(false);
        m_xBtnDelete->set_tooltip_text( CuiResId(RID_CUISTR_DELETEUSERCOLOR2) );
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

IMPL_LINK_NOARG(SvxColorTabPage, SelectColorModeHdl_Impl, weld::Toggleable&, void)
{
    if (m_xRbRGB->get_active())
        eCM = ColorModel::RGB;
    else if (m_xRbCMYK->get_active())
        eCM = ColorModel::CMYK;
    ChangeColorModel();
    UpdateColorValues();
}


IMPL_STATIC_LINK_NOARG(SvxColorTabPage, OnMoreColorsClick, weld::Button&, void)
{
    css::uno::Sequence<css::beans::PropertyValue> aArgs{ comphelper::makePropertyValue(
        u"AdditionsTag"_ustr, u"Color Palette"_ustr) };
    comphelper::dispatchCommand(u".uno:AdditionsDialog"_ustr, aArgs);
}

void SvxColorTabPage::ChangeColor(const NamedColor &rNewColor, bool bUpdatePreset )
{
    m_aPreviousColor = rNewColor.m_aColor;
    m_aCurrentColor = rNewColor;
    UpdateColorValues( bUpdatePreset );
    // fill ItemSet and pass it on to XOut
    XFillColorItem aItem(OUString(), m_aCurrentColor.m_aColor);
    aItem.setComplexColor(m_aCurrentColor.getComplexColor());
    rXFSet.Put(aItem);

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
        ConvertColorValues (m_aPreviousColor, eCM );
        ConvertColorValues (m_aCurrentColor.m_aColor, eCM);

        m_xCcustom->set_value( ColorToPercent_Impl( m_aCurrentColor.m_aColor.GetRed() ), FieldUnit::PERCENT );
        m_xMcustom->set_value( ColorToPercent_Impl( m_aCurrentColor.m_aColor.GetBlue() ), FieldUnit::PERCENT );
        m_xYcustom->set_value( ColorToPercent_Impl( m_aCurrentColor.m_aColor.GetGreen() ), FieldUnit::PERCENT );
        m_xKcustom->set_value( ColorToPercent_Impl( 255 - m_aCurrentColor.m_aColor.GetAlpha() ), FieldUnit::PERCENT );

        if( bUpdatePreset )
        {
            m_xCpreset->set_text(unicode::formatPercent(ColorToPercent_Impl(m_aPreviousColor.GetRed()),
                                                        Application::GetSettings().GetUILanguageTag()));
            m_xMpreset->set_text(unicode::formatPercent(ColorToPercent_Impl(m_aPreviousColor.GetBlue()),
                                                        Application::GetSettings().GetUILanguageTag()));
            m_xYpreset->set_text(unicode::formatPercent(ColorToPercent_Impl(m_aPreviousColor.GetGreen()),
                                                        Application::GetSettings().GetUILanguageTag()));
            m_xKpreset->set_text(unicode::formatPercent(ColorToPercent_Impl(255 - m_aPreviousColor.GetAlpha()),
                                                        Application::GetSettings().GetUILanguageTag()));
        }

        ConvertColorValues (m_aPreviousColor, ColorModel::RGB);
        ConvertColorValues (m_aCurrentColor.m_aColor, ColorModel::RGB);
    }
    else
    {
        m_xRcustom->set_value( ColorToPercent_Impl( m_aCurrentColor.m_aColor.GetRed() ) );
        m_xGcustom->set_value( ColorToPercent_Impl( m_aCurrentColor.m_aColor.GetGreen() ) );
        m_xBcustom->set_value( ColorToPercent_Impl( m_aCurrentColor.m_aColor.GetBlue() ) );
        m_xHexcustom->SetColor( m_aCurrentColor.m_aColor );

        if( bUpdatePreset )
        {
            m_xRpreset->set_text(OUString::number(ColorToPercent_Impl(m_aPreviousColor.GetRed())));
            m_xGpreset->set_text(OUString::number(ColorToPercent_Impl(m_aPreviousColor.GetGreen())));
            m_xBpreset->set_text(OUString::number(ColorToPercent_Impl(m_aPreviousColor.GetBlue())));
            m_xHexpreset->SetColor( m_aPreviousColor );
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
