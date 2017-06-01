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
#include <unotools/pathoptions.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <svtools/colrdlg.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/filedlghelper.hxx>
#include <svx/ofaitem.hxx>
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"

#include <cuires.hrc>
#include <strings.hrc>
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
#include <cuitabline.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include <svx/strings.hrc>
#include <osl/file.hxx>
#include <svx/Palette.hxx>
#include <cppu/unotype.hxx>
#include <officecfg/Office/Common.hxx>

using namespace com::sun::star;

SvxColorTabPage::SvxColorTabPage(vcl::Window* pParent, const SfxItemSet& rInAttrs)
    : SfxTabPage(pParent, "ColorPage", "cui/ui/colorpage.ui", &rInAttrs)
    , meType( XPropertyListType::Color )
    , mpTopDlg( GetParentDialog() )
    , rOutAttrs           ( rInAttrs )
    // All the horrific pointers we store and should not
    , pnColorListState( nullptr )
    , aXFStyleItem( drawing::FillStyle_SOLID )
    , aXFillColorItem( OUString(), Color( COL_BLACK ) )
    , aXFillAttr( rInAttrs.GetPool() )
    , rXFSet( aXFillAttr.GetItemSet() )
    , eCM( ColorModel::RGB )
    , m_context(comphelper::getProcessComponentContext())
{
    get(m_pSelectPalette, "paletteselector");
    get(m_pValSetColorList, "colorset");
    Size aSize = LogicToPixel(Size(100 , 120), MapUnit::MapAppFont);
    m_pValSetColorList->set_width_request(aSize.Width());
    m_pValSetColorList->set_height_request(aSize.Height());
    get(m_pValSetRecentList, "recentcolorset");
    get(m_pCtlPreviewOld, "oldpreview");
    get(m_pCtlPreviewNew, "newpreview");
    aSize = LogicToPixel(Size(34 , 25), MapUnit::MapAppFont);
    m_pCtlPreviewOld->set_width_request(aSize.Width());
    m_pCtlPreviewOld->set_height_request(aSize.Height());
    m_pCtlPreviewNew->set_width_request(aSize.Width());
    m_pCtlPreviewNew->set_height_request(aSize.Height());
    get(m_pRbRGB, "RGB");
    get(m_pRbCMYK, "CMYK");
    get(m_pRGBcustom, "rgbcustom");
    get(m_pRGBpreset, "rgbpreset");
    get(m_pRcustom, "R_custom");
    get(m_pRpreset, "R_preset-nospin");
    get(m_pGcustom, "G_custom");
    get(m_pGpreset, "G_preset-nospin");
    get(m_pBcustom, "B_custom");
    get(m_pBpreset, "B_preset-nospin");
    get(m_pHexpreset, "hex_preset");
    get(m_pHexcustom, "hex_custom");
    get(m_pCMYKcustom, "cmykcustom");
    get(m_pCMYKpreset,  "cmykpreset");
    get(m_pCcustom, "C_custom");
    get(m_pCpreset, "C_preset-nospin");
    get(m_pYcustom, "Y_custom");
    get(m_pYpreset, "Y_preset-nospin");
    get(m_pMcustom, "M_custom");
    get(m_pMpreset, "M_preset-nospin");
    get(m_pKcustom, "K_custom");
    get(m_pKpreset, "K_preset-nospin");
    get(m_pBtnAdd, "add");
    get(m_pBtnDelete, "delete");
    get(m_pBtnWorkOn, "edit");

    // this page needs ExchangeSupport
    SetExchangeSupport();

    // setting the output device
    rXFSet.Put( aXFStyleItem );
    rXFSet.Put( aXFillColorItem );
    m_pCtlPreviewOld->SetAttributes( aXFillAttr.GetItemSet() );
    m_pCtlPreviewNew->SetAttributes( aXFillAttr.GetItemSet() );

    // set handler
    m_pSelectPalette->SetSelectHdl( LINK(this, SvxColorTabPage, SelectPaletteLBHdl) );
    Link<ValueSet*, void> aValSelectLink = LINK( this, SvxColorTabPage, SelectValSetHdl_Impl );
    m_pValSetColorList->SetSelectHdl( aValSelectLink );
    m_pValSetRecentList->SetSelectHdl( aValSelectLink );

    Link<Edit&,void> aLink = LINK( this, SvxColorTabPage, ModifiedHdl_Impl );
    m_pRcustom->SetModifyHdl( aLink );
    m_pGcustom->SetModifyHdl( aLink );
    m_pBcustom->SetModifyHdl( aLink );
    m_pHexpreset->SetModifyHdl( aLink );
    m_pHexcustom->SetModifyHdl( aLink );
    m_pCcustom->SetModifyHdl( aLink );
    m_pYcustom->SetModifyHdl( aLink );
    m_pMcustom->SetModifyHdl( aLink );
    m_pKcustom->SetModifyHdl( aLink );

    Link<RadioButton&,void> aLink2 = LINK( this, SvxColorTabPage, SelectColorModeHdl_Impl );
    m_pRbRGB->SetToggleHdl( aLink2 );
    m_pRbCMYK->SetToggleHdl( aLink2 );
    SetColorModel( eCM );
    ChangeColorModel();

    m_pBtnAdd->SetClickHdl( LINK( this, SvxColorTabPage, ClickAddHdl_Impl ) );
    m_pBtnWorkOn->SetClickHdl( LINK( this, SvxColorTabPage, ClickWorkOnHdl_Impl ) );
    m_pBtnDelete->SetClickHdl( LINK( this, SvxColorTabPage, ClickDeleteHdl_Impl ) );
    // disable modify buttons
    // Color palettes can't be modified
    m_pBtnDelete->Disable();

    // disable preset color values
    m_pRGBpreset->Disable();
    m_pCMYKpreset->Disable();

    // ValueSet
    m_pValSetColorList->SetStyle( m_pValSetColorList->GetStyle() | WB_ITEMBORDER );
    m_pValSetColorList->Show();

    m_pValSetRecentList->SetStyle( m_pValSetColorList->GetStyle() | WB_ITEMBORDER );
    m_pValSetRecentList->Show();
    maPaletteManager.ReloadRecentColorSet( *m_pValSetRecentList );
    aSize = m_pValSetRecentList->layoutAllVisible(maPaletteManager.GetRecentColorCount());
    m_pValSetRecentList->set_height_request(aSize.Height());
    m_pValSetRecentList->set_width_request(aSize.Width());
}

SvxColorTabPage::~SvxColorTabPage()
{
    disposeOnce();
}

void SvxColorTabPage::dispose()
{
    mpTopDlg.clear();
    m_pSelectPalette.clear();
    m_pValSetColorList.clear();
    m_pValSetRecentList.clear();
    m_pCtlPreviewOld.clear();
    m_pCtlPreviewNew.clear();
    m_pRbRGB.clear();
    m_pRbCMYK.clear();
    m_pRGBcustom.clear();
    m_pRGBpreset.clear();
    m_pRcustom.clear();
    m_pRpreset.clear();
    m_pGcustom.clear();
    m_pGpreset.clear();
    m_pBcustom.clear();
    m_pBpreset.clear();
    m_pHexpreset.clear();
    m_pHexcustom.clear();
    m_pCMYKcustom.clear();
    m_pCMYKpreset.clear();
    m_pCcustom.clear();
    m_pCpreset.clear();
    m_pYcustom.clear();
    m_pYpreset.clear();
    m_pMcustom.clear();
    m_pMpreset.clear();
    m_pKcustom.clear();
    m_pKpreset.clear();
    m_pBtnAdd.clear();
    m_pBtnDelete.clear();
    m_pBtnWorkOn.clear();
    SfxTabPage::dispose();
}

void SvxColorTabPage::ImpColorCountChanged()
{
    if (!pColorList.is())
        return;
    m_pValSetColorList->SetColCount(SvxColorValueSet::getColumnCount());
    m_pValSetRecentList->SetColCount(SvxColorValueSet::getColumnCount());
}

void SvxColorTabPage::FillPaletteLB()
{
    m_pSelectPalette->Clear();
    std::vector<OUString> aPaletteList = maPaletteManager.GetPaletteList();
    for( std::vector<OUString>::iterator it = aPaletteList.begin(); it != aPaletteList.end(); ++it )
    {
        m_pSelectPalette->InsertEntry( *it );
    }
    OUString aPaletteName( officecfg::Office::Common::UserColors::PaletteName::get() );
    m_pSelectPalette->SelectEntry(aPaletteName);
    if (m_pSelectPalette->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND)
    {
        SelectPaletteLBHdl( *m_pSelectPalette );
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
    if( pColorList.is() )
    {
        const SfxPoolItem* pPoolItem = nullptr;
        if( SfxItemState::SET == rOutAttrs.GetItemState( GetWhich( XATTR_FILLCOLOR ), true, &pPoolItem ) )
        {
            SetColorModel( ColorModel::RGB );
            ChangeColorModel();

            const Color aColor = static_cast<const XFillColorItem*>(pPoolItem)->GetColorValue();
            ChangeColor( aColor );
            sal_Int32 nPos = FindInPalette( aColor );

            if ( nPos != -1 )
                m_pValSetColorList->SelectItem( m_pValSetColorList->GetItemId( nPos ) );
            // else search in other palettes?

        }

        m_pCtlPreviewOld->SetAttributes( aXFillAttr.GetItemSet() );
        m_pCtlPreviewOld->Invalidate();

        SelectValSetHdl_Impl( m_pValSetColorList );
    }
}

DeactivateRC SvxColorTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( _pSet )
        FillItemSet( _pSet );

    return DeactivateRC::LeavePage;
}


bool SvxColorTabPage::FillItemSet( SfxItemSet* rSet )
{
    maPaletteManager.AddRecentColor( aCurrentColor, OUString() );
    rSet->Put( XFillColorItem( OUString(), aCurrentColor ) );
    rSet->Put( XFillStyleItem( drawing::FillStyle_SOLID ) );
    return true;
}

void SvxColorTabPage::UpdateModified()
{
    bool bEnable = pColorList.is() && pColorList->Count();
    m_pBtnWorkOn->Enable( bEnable );
}

void SvxColorTabPage::Reset( const SfxItemSet* rSet )
{
    SfxItemState nState = rSet->GetItemState( XATTR_FILLCOLOR );

    Color aNewColor;

    if ( nState >= SfxItemState::DEFAULT )
    {
        XFillColorItem aColorItem( static_cast<const XFillColorItem&>(rSet->Get( XATTR_FILLCOLOR )) );
        aPreviousColor = aColorItem.GetColorValue();
        aNewColor = aColorItem.GetColorValue();
    }

    // set color model
    OUString aStr = GetUserData();
    eCM = (ColorModel)aStr.toInt32();
    SetColorModel( eCM );
    ChangeColorModel();

    ChangeColor(aNewColor);

    UpdateModified();
}


VclPtr<SfxTabPage> SvxColorTabPage::Create( vcl::Window* pWindow,
                                            const SfxItemSet* rOutAttrs )
{
    return VclPtr<SvxColorTabPage>::Create( pWindow, *rOutAttrs );
}

// is called when the content of the MtrFields is changed for color values
IMPL_LINK(SvxColorTabPage, ModifiedHdl_Impl, Edit&, rEdit, void)
{
    if (eCM == ColorModel::RGB)
    {
        // read current MtrFields, if cmyk, then k-value as transparency
        if(&rEdit == m_pHexcustom)
            aCurrentColor = Color(m_pHexcustom->GetColor());
        else
        {
            aCurrentColor.SetColor ( Color( (sal_uInt8)PercentToColor_Impl( (sal_uInt16) m_pRcustom->GetValue() ),
                                            (sal_uInt8)PercentToColor_Impl( (sal_uInt16) m_pGcustom->GetValue() ),
                                            (sal_uInt8)PercentToColor_Impl( (sal_uInt16) m_pBcustom->GetValue() ) ).GetColor() );

        }
        UpdateColorValues();
    }
    else
    {
        // read current MtrFields, if cmyk, then k-value as transparency
        aCurrentColor.SetColor ( Color( (sal_uInt8)PercentToColor_Impl( (sal_uInt16) m_pKcustom->GetValue() ),
                                        (sal_uInt8)PercentToColor_Impl( (sal_uInt16) m_pCcustom->GetValue() ),
                                        (sal_uInt8)PercentToColor_Impl( (sal_uInt16) m_pYcustom->GetValue() ),
                                        (sal_uInt8)PercentToColor_Impl( (sal_uInt16) m_pMcustom->GetValue() ) ).GetColor() );
        ConvertColorValues (aCurrentColor, ColorModel::RGB);
    }

    rXFSet.Put( XFillColorItem( OUString(), aCurrentColor ) );
    m_pCtlPreviewNew->SetAttributes( aXFillAttr.GetItemSet() );

    m_pCtlPreviewNew->Invalidate();
}


IMPL_LINK_NOARG(SvxColorTabPage, ClickAddHdl_Impl, Button*, void)
{
    OUString aNewName( SvxResId( RID_SVXSTR_COLOR ) );
    OUString aDesc( CuiResId( RID_SVXSTR_DESC_COLOR ) );
    OUString aName;

    long j = 1;
    bool bValidColorName = false;
    // check if name is already existing
    while (!bValidColorName)
    {
        aName = aNewName + " " + OUString::number( j++ );
        bValidColorName = (FindInCustomColors(aName) == LISTBOX_ENTRY_NOTFOUND);
    }

    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    ScopedVclPtr<AbstractSvxNameDialog> pDlg(pFact->CreateSvxNameDialog( GetParentDialog(), aName, aDesc ));
    ScopedVclPtr<MessageDialog> pWarnBox;
    sal_uInt16 nError = 1;

    while (pDlg->Execute() == RET_OK)
    {
        pDlg->GetName( aName );

        bValidColorName = (FindInCustomColors(aName) == LISTBOX_ENTRY_NOTFOUND);
        if (bValidColorName)
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

    if (!nError)
    {
        m_pSelectPalette->SelectEntryPos(0);
        SelectPaletteLBHdl( *m_pSelectPalette );
        std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create(m_context));
        css::uno::Sequence< sal_Int32 > aCustomColorList(officecfg::Office::Common::UserColors::CustomColor::get());
        css::uno::Sequence< OUString > aCustomColorNameList(officecfg::Office::Common::UserColors::CustomColorName::get());
        sal_Int32 nSize = aCustomColorList.getLength();
        aCustomColorList.realloc( nSize + 1 );
        aCustomColorNameList.realloc( nSize + 1 );
        aCustomColorList[nSize] = aCurrentColor.GetColor();
        aCustomColorNameList[nSize] = aName;
        officecfg::Office::Common::UserColors::CustomColor::set(aCustomColorList, batch);
        officecfg::Office::Common::UserColors::CustomColorName::set(aCustomColorNameList, batch);
        batch->commit();
        sal_uInt16 nId = m_pValSetColorList->GetItemId(nSize - 1);
        m_pValSetColorList->InsertItem( nId + 1 , aCurrentColor, aName );
        m_pValSetColorList->SelectItem( nId + 1 );
        m_pBtnDelete->Enable();
        ImpColorCountChanged();
    }

    UpdateModified();
}

IMPL_LINK_NOARG(SvxColorTabPage, ClickWorkOnHdl_Impl, Button*, void)
{
    std::unique_ptr<SvColorDialog> pColorDlg(new SvColorDialog( GetParentDialog() ));

    pColorDlg->SetColor (aCurrentColor);
    pColorDlg->SetMode( svtools::ColorPickerMode_MODIFY );

    if( pColorDlg->Execute() == RET_OK )
    {
        Color aPreviewColor = pColorDlg->GetColor();
        aCurrentColor = aPreviewColor;
        UpdateColorValues( false );
        // fill ItemSet and pass it on to XOut
        rXFSet.Put( XFillColorItem( OUString(), aPreviewColor ) );
        //m_pCtlPreviewOld->SetAttributes( aXFillAttr );
        m_pCtlPreviewNew->SetAttributes( aXFillAttr.GetItemSet() );

        m_pCtlPreviewNew->Invalidate();
    }
}

IMPL_LINK_NOARG(SvxColorTabPage, ClickDeleteHdl_Impl, Button*, void)
{
    sal_uInt16 nId = m_pValSetColorList->GetSelectItemId();
    size_t nPos = m_pValSetColorList->GetSelectItemPos();
    if(m_pSelectPalette->GetSelectEntryPos() == 0 && nPos != VALUESET_ITEM_NOTFOUND )
    {
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
        m_pValSetColorList->RemoveItem(nId);
        if(m_pValSetColorList->GetItemCount() != 0)
        {
            nId = m_pValSetColorList->GetItemId(0);
            m_pValSetColorList->SelectItem(nId);
            SelectValSetHdl_Impl( m_pValSetColorList );
        }
        else
            m_pBtnDelete->Disable();
    }
}

IMPL_LINK_NOARG(SvxColorTabPage, SelectPaletteLBHdl, ListBox&, void)
{
    m_pValSetColorList->Clear();
    sal_Int32 nPos = m_pSelectPalette->GetSelectEntryPos();
    maPaletteManager.SetPalette( nPos );
    maPaletteManager.ReloadColorSet( *m_pValSetColorList );

    if(nPos != maPaletteManager.GetPaletteCount() - 1 && nPos != 0)
    {
        XColorListRef pList = XPropertyList::AsColorList(
                                XPropertyList::CreatePropertyListFromURL(
                                meType, maPaletteManager.GetSelectedPalettePath()));
        pList->SetName(maPaletteManager.GetPaletteName());
        if(pList->Load())
        {
            SvxAreaTabDialog* pArea = dynamic_cast< SvxAreaTabDialog* >( mpTopDlg.get() );
            SvxLineTabDialog* pLine = dynamic_cast< SvxLineTabDialog* >( mpTopDlg.get() );
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
    if(nPos != 0)
        m_pBtnDelete->Disable();

    m_pValSetColorList->Resize();
}

IMPL_LINK(SvxColorTabPage, SelectValSetHdl_Impl, ValueSet*, pValSet, void)
{
    sal_Int32 nPos = pValSet->GetSelectItemId();
    if( nPos != 0 )
    {
        Color aColor = pValSet->GetItemColor( nPos );

        rXFSet.Put( XFillColorItem( OUString(), aColor ) );
        m_pCtlPreviewNew->SetAttributes( aXFillAttr.GetItemSet() );
        m_pCtlPreviewNew->Invalidate();
        ChangeColor(aColor, false);

        if(pValSet == m_pValSetColorList)
        {
            m_pValSetRecentList->SetNoSelection();
            if(m_pSelectPalette->GetSelectEntryPos() == 0 && m_pValSetColorList->GetSelectItemId() != 0)
                m_pBtnDelete->Enable();
            else
                m_pBtnDelete->Disable();
        }
        if(pValSet == m_pValSetRecentList)
        {
            m_pValSetColorList->SetNoSelection();
            m_pBtnDelete->Disable();
        }
    }
}

void SvxColorTabPage::ConvertColorValues (Color& rColor, ColorModel eModell)
{
    switch (eModell)
    {
        case ColorModel::RGB:
        {
            CmykToRgb_Impl (rColor, (sal_uInt16)rColor.GetTransparency() );
            rColor.SetTransparency ((sal_uInt8) 0);
        }
        break;

        case ColorModel::CMYK:
        {
            sal_uInt16 nK;
            RgbToCmyk_Impl (rColor, nK );
            rColor.SetTransparency ((sal_uInt8) nK);
        }
        break;
    }
}
IMPL_LINK(SvxColorTabPage, SelectColorModeHdl_Impl, RadioButton&, rRadioButton, void)
{
    if( &rRadioButton == m_pRbRGB )
        eCM = ColorModel::RGB;
    if( &rRadioButton == m_pRbCMYK )
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
    m_pCtlPreviewNew->SetAttributes( aXFillAttr.GetItemSet() );
    m_pCtlPreviewNew->Invalidate();
}

void SvxColorTabPage::SetColorModel( ColorModel eModel )
{
    m_pRbRGB->SetState(false);
    m_pRbCMYK->SetState(false);
    if(eModel == ColorModel::RGB)
        m_pRbRGB->SetState(true);
    if(eModel == ColorModel::CMYK)
        m_pRbCMYK->SetState(true);
}

void SvxColorTabPage::ChangeColorModel()
{
    switch( eCM )
    {
        case ColorModel::RGB:
        {
            m_pRGBcustom->Show();
            m_pRGBpreset->Show();
            m_pCMYKcustom->Hide();
            m_pCMYKpreset->Hide();
        }
        break;

        case ColorModel::CMYK:
        {
            m_pCMYKcustom->Show();
            m_pCMYKpreset->Show();
            m_pRGBcustom->Hide();
            m_pRGBpreset->Hide();
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

        m_pCcustom->SetValue( ColorToPercent_Impl( aCurrentColor.GetRed() ) );
        m_pMcustom->SetValue( ColorToPercent_Impl( aCurrentColor.GetBlue() ) );
        m_pYcustom->SetValue( ColorToPercent_Impl( aCurrentColor.GetGreen() ) );
        m_pKcustom->SetValue( ColorToPercent_Impl( aCurrentColor.GetTransparency() ) );

        if( bUpdatePreset )
        {
            m_pCpreset->SetValue( ColorToPercent_Impl( aPreviousColor.GetRed() ) );
            m_pMpreset->SetValue( ColorToPercent_Impl( aPreviousColor.GetBlue() ) );
            m_pYpreset->SetValue( ColorToPercent_Impl( aPreviousColor.GetGreen() ) );
            m_pKpreset->SetValue( ColorToPercent_Impl( aPreviousColor.GetTransparency() ) );
        }

        ConvertColorValues (aPreviousColor, ColorModel::RGB);
        ConvertColorValues (aCurrentColor, ColorModel::RGB);
    }
    else
    {
        m_pRcustom->SetValue( ColorToPercent_Impl( aCurrentColor.GetRed() ) );
        m_pGcustom->SetValue( ColorToPercent_Impl( aCurrentColor.GetGreen() ) );
        m_pBcustom->SetValue( ColorToPercent_Impl( aCurrentColor.GetBlue() ) );
        m_pHexcustom->SetColor( aCurrentColor.GetColor() );

        if( bUpdatePreset )
        {
            m_pRpreset->SetValue( ColorToPercent_Impl( aPreviousColor.GetRed() ) );
            m_pGpreset->SetValue( ColorToPercent_Impl( aPreviousColor.GetGreen() ) );
            m_pBpreset->SetValue( ColorToPercent_Impl( aPreviousColor.GetBlue() ) );
            m_pHexpreset->SetColor( aPreviousColor.GetColor() );
        }
    }
}

sal_Int32 SvxColorTabPage::FindInCustomColors(OUString const & aColorName)
{
    css::uno::Sequence< OUString > aCustomColorNameList(officecfg::Office::Common::UserColors::CustomColorName::get());
    long nCount = aCustomColorNameList.getLength();
    bool bValidColorName = true;
    sal_Int32 nPos = LISTBOX_ENTRY_NOTFOUND;

    for(long i = 0;i < nCount && bValidColorName;i++)
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
    sal_Int32 nPos = pColorList->GetIndexOfColor( rColor );

    return ( nPos == -1) ? LISTBOX_ENTRY_NOTFOUND : nPos;
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
    long lTemp;

    lTemp = 255 - ( rColor.GetRed() + nK );

    if( lTemp < 0L )
        lTemp = 0L;
    rColor.SetRed( (sal_uInt8)lTemp );

    lTemp = 255 - ( rColor.GetGreen() + nK );

    if( lTemp < 0L )
        lTemp = 0L;
    rColor.SetGreen( (sal_uInt8)lTemp );

    lTemp = 255 - ( rColor.GetBlue() + nK );

    if( lTemp < 0L )
        lTemp = 0L;
    rColor.SetBlue( (sal_uInt8)lTemp );
}


sal_uInt16 SvxColorTabPage::ColorToPercent_Impl( sal_uInt16 nColor )
{
    sal_uInt16 nWert = 0;

    switch (eCM)
    {
        case ColorModel::RGB :
            nWert = nColor;
            break;

        case ColorModel::CMYK:
            nWert = (sal_uInt16) ( (double) nColor * 100.0 / 255.0 + 0.5 );
            break;
    }

    return nWert;
}


sal_uInt16 SvxColorTabPage::PercentToColor_Impl( sal_uInt16 nPercent )
{
    sal_uInt16 nWert = 0;

    switch (eCM)
    {
        case ColorModel::RGB :
            nWert = nPercent;
            break;

        case ColorModel::CMYK:
            nWert = (sal_uInt16) ( (double) nPercent * 255.0 / 100.0 + 0.5 );
            break;
    }

    return nWert;
}


void SvxColorTabPage::FillUserData()
{
    // the color model is saved in the Ini-file
    SetUserData( OUString::number( (int)eCM ) );
}


void SvxColorTabPage::SetPropertyList( XPropertyListType t, const XPropertyListRef &xRef )
{
    (void) t;
    OSL_ASSERT( t == XPropertyListType::Color );
    pColorList = XColorListRef( static_cast<XColorList *>(xRef.get() ) );
}

void SvxColorTabPage::SetColorList( const XColorListRef& pColList )
{
    SetPropertyList( XPropertyListType::Color, XPropertyListRef( ( pColList.get() ) ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
