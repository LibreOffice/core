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

#include <editeng/unolingu.hxx>
#include <vcl/svapp.hxx>
#include <vcl/idle.hxx>
#include <unotools/pathoptions.hxx>
#include <svtools/ctrltool.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/dialogs.hrc>
#include <svtools/unitconv.hxx>
#include <svl/languageoptions.hxx>
#include <svx/xtable.hxx>
#include <chardlg.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/blinkitem.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/autokernitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/colritem.hxx>
#include <svx/drawitem.hxx>
#include <svx/dlgutil.hxx>
#include <dialmgr.hxx>
#include <sfx2/htmlmode.hxx>
#include <cuicharmap.hxx>
#include "chardlg.h"
#include <editeng/emphasismarkitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/twolinesitem.hxx>
#include <editeng/charhiddenitem.hxx>
#include <svl/stritem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <officecfg/Office/Common.hxx>
#include <svx/svxdlg.hxx>
#include <strings.hrc>
#include <twolines.hrc>
#include <svl/intitem.hxx>
#include <sfx2/request.hxx>
#include <svx/flagsdef.hxx>
#include <FontFeaturesDialog.hxx>
#include <sal/log.hxx>

using namespace ::com::sun::star;

// static ----------------------------------------------------------------

const sal_uInt16 SvxCharNamePage::pNameRanges[] =
{
    SID_ATTR_CHAR_FONT,
    SID_ATTR_CHAR_WEIGHT,
    SID_ATTR_CHAR_FONTHEIGHT,
    SID_ATTR_CHAR_FONTHEIGHT,
    SID_ATTR_CHAR_COLOR,
    SID_ATTR_CHAR_COLOR,
    SID_ATTR_CHAR_LANGUAGE,
    SID_ATTR_CHAR_LANGUAGE,
    SID_ATTR_CHAR_CJK_FONT,
    SID_ATTR_CHAR_CJK_WEIGHT,
    SID_ATTR_CHAR_CTL_FONT,
    SID_ATTR_CHAR_CTL_WEIGHT,
    0
};

const sal_uInt16 SvxCharEffectsPage::pEffectsRanges[] =
{
    SID_ATTR_CHAR_SHADOWED,
    SID_ATTR_CHAR_UNDERLINE,
    SID_ATTR_CHAR_COLOR,
    SID_ATTR_CHAR_COLOR,
    SID_ATTR_CHAR_CASEMAP,
    SID_ATTR_CHAR_CASEMAP,
    SID_ATTR_FLASH,
    SID_ATTR_FLASH,
    SID_ATTR_CHAR_EMPHASISMARK,
    SID_ATTR_CHAR_EMPHASISMARK,
    SID_ATTR_CHAR_RELIEF,
    SID_ATTR_CHAR_RELIEF,
    SID_ATTR_CHAR_HIDDEN,
    SID_ATTR_CHAR_HIDDEN,
    SID_ATTR_CHAR_OVERLINE,
    SID_ATTR_CHAR_OVERLINE,
    0
};

const sal_uInt16 SvxCharPositionPage::pPositionRanges[] =
{
    SID_ATTR_CHAR_KERNING,
    SID_ATTR_CHAR_KERNING,
    SID_ATTR_CHAR_ESCAPEMENT,
    SID_ATTR_CHAR_ESCAPEMENT,
    SID_ATTR_CHAR_AUTOKERN,
    SID_ATTR_CHAR_AUTOKERN,
    SID_ATTR_CHAR_ROTATED,
    SID_ATTR_CHAR_SCALEWIDTH,
    SID_ATTR_CHAR_WIDTH_FIT_TO_LINE,
    SID_ATTR_CHAR_WIDTH_FIT_TO_LINE,
    0
};

const sal_uInt16 SvxCharTwoLinesPage::pTwoLinesRanges[] =
{
    SID_ATTR_CHAR_TWO_LINES,
    SID_ATTR_CHAR_TWO_LINES,
    0
};

// C-Function ------------------------------------------------------------

static bool StateToAttr( TriState aState )
{
    return ( TRISTATE_TRUE == aState );
}

namespace
{
    void setPrevFontEscapement(SvxFont& _rFont,sal_uInt8 nProp, sal_uInt8 nEscProp, short nEsc )
    {
        _rFont.SetPropr( nProp );
        _rFont.SetProprRel( nEscProp );
        _rFont.SetEscapement( nEsc );
    }
}

inline SvxFont& SvxCharBasePage::GetPreviewFont()
{
    return m_aPreviewWin.GetFont();
}

inline SvxFont& SvxCharBasePage::GetPreviewCJKFont()
{
    return m_aPreviewWin.GetCJKFont();
}

inline SvxFont& SvxCharBasePage::GetPreviewCTLFont()
{
    return m_aPreviewWin.GetCTLFont();
}

SvxCharBasePage::SvxCharBasePage(TabPageParent pParent, const OUString& rUIXMLDescription, const OString& rID, const SfxItemSet& rItemset)
    : SfxTabPage(pParent, rUIXMLDescription, rID, &rItemset)
    , m_bPreviewBackgroundToCharacter( false )
{
}

SvxCharBasePage::~SvxCharBasePage()
{
}

void SvxCharBasePage::ActivatePage(const SfxItemSet& rSet)
{
    m_aPreviewWin.SetFromItemSet(rSet, m_bPreviewBackgroundToCharacter);
}

void SvxCharBasePage::SetPrevFontWidthScale( const SfxItemSet& rSet )
{
    sal_uInt16 nWhich = GetWhich( SID_ATTR_CHAR_SCALEWIDTH );
    if (rSet.GetItemState(nWhich)>=SfxItemState::DEFAULT)
    {
        const SvxCharScaleWidthItem &rItem = static_cast<const SvxCharScaleWidthItem&>( rSet.Get( nWhich ) );
        m_aPreviewWin.SetFontWidthScale(rItem.GetValue());
    }
}

void SvxCharBasePage::SetPrevFontEscapement( sal_uInt8 nProp, sal_uInt8 nEscProp, short nEsc )
{
    setPrevFontEscapement(GetPreviewFont(),nProp,nEscProp,nEsc);
    setPrevFontEscapement(GetPreviewCJKFont(),nProp,nEscProp,nEsc);
    setPrevFontEscapement(GetPreviewCTLFont(),nProp,nEscProp,nEsc);
    m_aPreviewWin.Invalidate();
}


// SvxCharNamePage_Impl --------------------------------------------------

struct SvxCharNamePage_Impl
{
    Idle            m_aUpdateIdle;
    OUString        m_aNoStyleText;
    std::unique_ptr<FontList> m_pFontList;
    int             m_nExtraEntryPos;
    bool            m_bInSearchMode;

    SvxCharNamePage_Impl()
        : m_nExtraEntryPos(std::numeric_limits<int>::max())
        , m_bInSearchMode(false)

    {
        m_aUpdateIdle.SetPriority( TaskPriority::LOWEST );
    }
};

// class SvxCharNamePage -------------------------------------------------

SvxCharNamePage::SvxCharNamePage(TabPageParent pParent, const SfxItemSet& rInSet)
    : SvxCharBasePage(pParent, "cui/ui/charnamepage.ui", "CharNamePage", rInSet)
    , m_pImpl(new SvxCharNamePage_Impl)
    , m_xEastFrame(m_xBuilder->weld_widget("asian"))
    , m_xEastFontNameFT(m_xBuilder->weld_label("eastfontnameft"))
    , m_xEastFontNameLB(m_xBuilder->weld_combo_box("eastfontnamelb"))
    , m_xEastFontStyleFT(m_xBuilder->weld_label("eaststyleft"))
    , m_xEastFontStyleLB(new SvtFontStyleBox(m_xBuilder->weld_combo_box("eaststylelb")))
    , m_xEastFontSizeFT(m_xBuilder->weld_label("eastsizeft"))
    , m_xEastFontSizeLB(new SvtFontSizeBox(m_xBuilder->weld_combo_box("eastsizelb")))
    , m_xEastFontLanguageFT(m_xBuilder->weld_label("eastlangft"))
    , m_xEastFontLanguageLB(new LanguageBox(m_xBuilder->weld_combo_box("eastlanglb")))
    , m_xEastFontTypeFT(m_xBuilder->weld_label("eastfontinfo"))
    , m_xEastFontFeaturesButton(m_xBuilder->weld_button("east_features_button"))
    , m_xCTLFrame(m_xBuilder->weld_widget("ctl"))
    , m_xCTLFontNameFT(m_xBuilder->weld_label("ctlfontnameft"))
    , m_xCTLFontNameLB(m_xBuilder->weld_combo_box("ctlfontnamelb"))
    , m_xCTLFontStyleFT(m_xBuilder->weld_label("ctlstyleft"))
    , m_xCTLFontStyleLB(new SvtFontStyleBox(m_xBuilder->weld_combo_box("ctlstylelb")))
    , m_xCTLFontSizeFT(m_xBuilder->weld_label("ctlsizeft"))
    , m_xCTLFontSizeLB(new SvtFontSizeBox(m_xBuilder->weld_combo_box("ctlsizelb")))
    , m_xCTLFontLanguageFT(m_xBuilder->weld_label("ctllangft"))
    , m_xCTLFontLanguageLB(new LanguageBox(m_xBuilder->weld_combo_box("ctllanglb")))
    , m_xCTLFontTypeFT(m_xBuilder->weld_label("ctlfontinfo"))
    , m_xCTLFontFeaturesButton(m_xBuilder->weld_button("ctl_features_button"))
{
    m_xPreviewWin.reset(new weld::CustomWeld(*m_xBuilder, "preview", m_aPreviewWin));
#ifdef IOS
    m_xPreviewWin->hide();
#endif
    m_pImpl->m_aNoStyleText = CuiResId( RID_SVXSTR_CHARNAME_NOSTYLE );

    SvtLanguageOptions aLanguageOptions;
    bool bShowCJK = aLanguageOptions.IsCJKFontEnabled();
    bool bShowCTL = aLanguageOptions.IsCTLFontEnabled();
    bool bShowNonWestern = bShowCJK || bShowCTL;

    if (bShowNonWestern)
    {
        m_xWestFrame = m_xBuilder->weld_widget("western");
        m_xWestFontNameFT = m_xBuilder->weld_label("westfontnameft-cjk");
        m_xWestFontNameLB = m_xBuilder->weld_combo_box("westfontnamelb-cjk");
        m_xWestFontStyleFT = m_xBuilder->weld_label("weststyleft-cjk");
        m_xWestFontSizeFT = m_xBuilder->weld_label("westsizeft-cjk");

        m_xWestFontStyleLB.reset(new SvtFontStyleBox(m_xBuilder->weld_combo_box("weststylelb-cjk")));
        m_xWestFontSizeLB.reset(new SvtFontSizeBox(m_xBuilder->weld_combo_box("westsizelb-cjk")));

        m_xWestFontLanguageFT = m_xBuilder->weld_label("westlangft-cjk");
        m_xWestFontLanguageLB.reset(new LanguageBox(m_xBuilder->weld_combo_box("westlanglb-cjk")));
        m_xWestFontTypeFT = m_xBuilder->weld_label("westfontinfo-cjk");

        m_xWestFontFeaturesButton = m_xBuilder->weld_button("west_features_button-cjk");
    }
    else
    {
        m_xWestFrame = m_xBuilder->weld_widget("simple");
        m_xWestFontNameFT = m_xBuilder->weld_label("westfontnameft-nocjk");
        m_xWestFontStyleFT = m_xBuilder->weld_label("weststyleft-nocjk");
        m_xWestFontSizeFT = m_xBuilder->weld_label("westsizeft-nocjk");

        m_xWestFontLanguageFT = m_xBuilder->weld_label("westlangft-nocjk");
        m_xWestFontLanguageLB.reset(new LanguageBox(m_xBuilder->weld_combo_box("westlanglb-nocjk")));
        m_xWestFontTypeFT = m_xBuilder->weld_label("westfontinfo-nocjk");

        m_xWestFontFeaturesButton = m_xBuilder->weld_button("west_features_button-nocjk");

        std::unique_ptr<weld::EntryTreeView> xWestFontNameLB = m_xBuilder->weld_entry_tree_view("namegrid", "westfontname-nocjk", "westfontnamelb-nocjk");
        std::unique_ptr<weld::EntryTreeView> xWestFontStyleLB = m_xBuilder->weld_entry_tree_view("stylegrid", "weststyle-nocjk", "weststylelb-nocjk");
        std::unique_ptr<weld::EntryTreeView> xWestFontSizeLB = m_xBuilder->weld_entry_tree_view("sizegrid", "westsize-nocjk", "westsizelb-nocjk");

        // 7 lines in the treeview
        xWestFontNameLB->set_height_request_by_rows(7);
        xWestFontStyleLB->set_height_request_by_rows(7);
        xWestFontSizeLB->set_height_request_by_rows(7);

        m_xWestFontNameLB = std::move(xWestFontNameLB);
        m_xWestFontStyleLB.reset(new SvtFontStyleBox(std::move(xWestFontStyleLB)));
        m_xWestFontSizeLB.reset(new SvtFontSizeBox(std::move(xWestFontSizeLB)));
    }

    //In MacOSX the standard dialogs name font-name, font-style as
    //Family, Typeface
    //In GNOME the standard dialogs name font-name, font-style as
    //Family, Style
    //In Windows the standard dialogs name font-name, font-style as
    //Font, Style
#ifdef _WIN32
    OUString sFontFamilyString(CuiResId(RID_SVXSTR_CHARNAME_FONT));
#else
    OUString sFontFamilyString(CuiResId(RID_SVXSTR_CHARNAME_FAMILY));
#endif
    m_xWestFontNameFT->set_label(sFontFamilyString);
    m_xEastFontNameFT->set_label(sFontFamilyString);
    m_xCTLFontNameFT->set_label(sFontFamilyString);

#ifdef MACOSX
    OUString sFontStyleString(CuiResId(RID_SVXSTR_CHARNAME_TYPEFACE));
#else
    OUString sFontStyleString(CuiResId(RID_SVXSTR_CHARNAME_STYLE));
#endif
    m_xWestFontStyleFT->set_label(sFontStyleString);
    m_xEastFontStyleFT->set_label(sFontStyleString);
    m_xCTLFontStyleFT->set_label(sFontStyleString);

    m_xWestFrame->show();
    m_xEastFrame->set_visible(bShowCJK);
    m_xCTLFrame->set_visible(bShowCTL);

    m_xWestFontLanguageLB->SetLanguageList(SvxLanguageListFlags::WESTERN, true, false, true);
    m_xEastFontLanguageLB->SetLanguageList(SvxLanguageListFlags::CJK, true, false, true);
    m_xCTLFontLanguageLB->SetLanguageList(SvxLanguageListFlags::CTL, true, false, true);

    Initialize();
}

SvxCharNamePage::~SvxCharNamePage()
{
    disposeOnce();
}

void SvxCharNamePage::dispose()
{
    m_pImpl.reset();
    m_xCTLFontStyleLB.reset();
    m_xEastFontLanguageLB.reset();
    m_xWestFontStyleLB.reset();
    m_xCTLFontSizeLB.reset();
    m_xEastFontSizeLB.reset();
    m_xWestFontSizeLB.reset();
    m_xWestFontLanguageLB.reset();
    m_xPreviewWin.reset();
    m_xCTLFontLanguageLB.reset();
    m_xEastFontLanguageLB.reset();
    SvxCharBasePage::dispose();
}

void SvxCharNamePage::Initialize()
{
    // to handle the changes of the other pages
    SetExchangeSupport();

    Link<weld::ComboBox&,void> aLink = LINK(this, SvxCharNamePage, FontModifyComboBoxHdl_Impl);
    m_xWestFontNameLB->connect_changed(aLink);
    m_xWestFontStyleLB->connect_changed(aLink);
    m_xWestFontSizeLB->connect_changed(aLink);
    m_xWestFontLanguageLB->connect_changed(aLink);

    m_xWestFontFeaturesButton->connect_clicked(LINK(this, SvxCharNamePage, FontFeatureButtonClicked));

    m_xEastFontNameLB->connect_changed(aLink);
    m_xEastFontStyleLB->connect_changed(aLink);
    m_xEastFontSizeLB->connect_changed(aLink);
    m_xEastFontLanguageLB->connect_changed(aLink);
    m_xEastFontFeaturesButton->connect_clicked(LINK(this, SvxCharNamePage, FontFeatureButtonClicked));

    m_xCTLFontNameLB->connect_changed(aLink);
    m_xCTLFontStyleLB->connect_changed(aLink);
    m_xCTLFontSizeLB->connect_changed(aLink);
    m_xCTLFontLanguageLB->connect_changed(aLink);
    m_xCTLFontFeaturesButton->connect_clicked(LINK(this, SvxCharNamePage, FontFeatureButtonClicked));

    m_pImpl->m_aUpdateIdle.SetInvokeHandler( LINK( this, SvxCharNamePage, UpdateHdl_Impl ) );
}

const FontList* SvxCharNamePage::GetFontList() const
{
    if ( !m_pImpl->m_pFontList )
    {
        SfxObjectShell* pDocSh = SfxObjectShell::Current();

        /* #110771# SvxFontListItem::GetFontList can return NULL */
        if ( pDocSh )
        {
            const SfxPoolItem* pItem = pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST );
            if ( pItem != nullptr )
            {
                DBG_ASSERT(nullptr != static_cast<const SvxFontListItem*>(pItem)->GetFontList(),
                           "Where is the font list?");
                m_pImpl->m_pFontList = static_cast<const SvxFontListItem*>(pItem )->GetFontList()->Clone();
            }
        }
        if(!m_pImpl->m_pFontList)
        {
            m_pImpl->m_pFontList.reset(new FontList( Application::GetDefaultDevice() ));
        }
    }

    return m_pImpl->m_pFontList.get();
}


namespace
{
    FontMetric calcFontMetrics(  SvxFont& _rFont,
                    SvxCharNamePage const * _pPage,
                    const weld::ComboBox* _pFontNameLB,
                    const SvtFontStyleBox* _pFontStyleLB,
                    const SvtFontSizeBox* _pFontSizeLB,
                    const LanguageBox* _pLanguageLB,
                    const FontList* _pFontList,
                    sal_uInt16 _nFontWhich,
                    sal_uInt16 _nFontHeightWhich)
    {
        Size aSize = _rFont.GetFontSize();
        aSize.setWidth( 0 );
        FontMetric aFontMetrics;
        OUString sFontName(_pFontNameLB->get_active_text());
        bool bFontAvailable = _pFontList->IsAvailable( sFontName );
        if (bFontAvailable  || _pFontNameLB->get_value_changed_from_saved())
            aFontMetrics = _pFontList->Get(sFontName, _pFontStyleLB->get_active_text());
        else
        {
            //get the font from itemset
            SfxItemState eState = _pPage->GetItemSet().GetItemState( _nFontWhich );
            if ( eState >= SfxItemState::DEFAULT )
            {
                const SvxFontItem* pFontItem = static_cast<const SvxFontItem*>(&( _pPage->GetItemSet().Get( _nFontWhich ) ));
                aFontMetrics.SetFamilyName(pFontItem->GetFamilyName());
                aFontMetrics.SetStyleName(pFontItem->GetStyleName());
                aFontMetrics.SetFamily(pFontItem->GetFamily());
                aFontMetrics.SetPitch(pFontItem->GetPitch());
                aFontMetrics.SetCharSet(pFontItem->GetCharSet());
            }
        }
        if ( _pFontSizeLB->IsRelative() )
        {
            DBG_ASSERT( _pPage->GetItemSet().GetParent(), "No parent set" );
            const SvxFontHeightItem& rOldItem = static_cast<const SvxFontHeightItem&>(_pPage->GetItemSet().GetParent()->Get( _nFontHeightWhich ));

            // old value, scaled
            long nHeight;
            if ( _pFontSizeLB->IsPtRelative() )
                nHeight = rOldItem.GetHeight() + PointToTwips( static_cast<long>(_pFontSizeLB->get_value() / 10) );
            else
                nHeight = static_cast<long>(rOldItem.GetHeight() * _pFontSizeLB->get_value() / 100);

            // conversion twips for the example-window
            aSize.setHeight(
                ItemToControl( nHeight, _pPage->GetItemSet().GetPool()->GetMetric( _nFontHeightWhich ), FieldUnit::TWIP ) );
        }
        else if ( !_pFontSizeLB->get_active_text().isEmpty() )
            aSize.setHeight( PointToTwips( static_cast<long>(_pFontSizeLB->get_value() / 10) ) );
        else
            aSize.setHeight( 200 );   // default 10pt
        aFontMetrics.SetFontSize( aSize );

        _rFont.SetLanguage(_pLanguageLB->get_active_id());

        _rFont.SetFamily( aFontMetrics.GetFamilyType() );
        _rFont.SetFamilyName( aFontMetrics.GetFamilyName() );
        _rFont.SetStyleName( aFontMetrics.GetStyleName() );
        _rFont.SetPitch( aFontMetrics.GetPitch() );
        _rFont.SetCharSet( aFontMetrics.GetCharSet() );
        _rFont.SetWeight( aFontMetrics.GetWeight() );
        _rFont.SetItalic( aFontMetrics.GetItalic() );
        _rFont.SetFontSize( aFontMetrics.GetFontSize() );

        return aFontMetrics;
    }
}


void SvxCharNamePage::UpdatePreview_Impl()
{
    SvxFont& rFont = GetPreviewFont();
    SvxFont& rCJKFont = GetPreviewCJKFont();
    SvxFont& rCTLFont = GetPreviewCTLFont();
    // Font
    const FontList* pFontList = GetFontList();

    FontMetric aWestFontMetric = calcFontMetrics(rFont, this, m_xWestFontNameLB.get(),
        m_xWestFontStyleLB.get(), m_xWestFontSizeLB.get(), m_xWestFontLanguageLB.get(),
        pFontList, GetWhich(SID_ATTR_CHAR_FONT),
        GetWhich(SID_ATTR_CHAR_FONTHEIGHT));

    m_xWestFontTypeFT->set_label(pFontList->GetFontMapText(aWestFontMetric));

    FontMetric aEastFontMetric = calcFontMetrics(rCJKFont, this, m_xEastFontNameLB.get(),
        m_xEastFontStyleLB.get(), m_xEastFontSizeLB.get(), m_xEastFontLanguageLB.get(),
        pFontList, GetWhich(SID_ATTR_CHAR_CJK_FONT),
        GetWhich(SID_ATTR_CHAR_CJK_FONTHEIGHT));

    m_xEastFontTypeFT->set_label(pFontList->GetFontMapText(aEastFontMetric));

    FontMetric aCTLFontMetric = calcFontMetrics(rCTLFont,
        this, m_xCTLFontNameLB.get(), m_xCTLFontStyleLB.get(), m_xCTLFontSizeLB.get(),
        m_xCTLFontLanguageLB.get(), pFontList, GetWhich(SID_ATTR_CHAR_CTL_FONT),
        GetWhich(SID_ATTR_CHAR_CTL_FONTHEIGHT));

    m_xCTLFontTypeFT->set_label(pFontList->GetFontMapText(aCTLFontMetric));

    m_aPreviewWin.Invalidate();
}

void SvxCharNamePage::FillStyleBox_Impl(const weld::Widget& rNameBox)
{
    const FontList* pFontList = GetFontList();
    DBG_ASSERT( pFontList, "no fontlist" );

    SvtFontStyleBox* pStyleBox = nullptr;
    OUString sFontName;

    if (m_xWestFontNameLB.get() == &rNameBox)
    {
        pStyleBox = m_xWestFontStyleLB.get();
        sFontName = m_xWestFontNameLB->get_active_text();
    }
    else if (m_xEastFontNameLB.get() == &rNameBox)
    {
        pStyleBox = m_xEastFontStyleLB.get();
        sFontName = m_xEastFontStyleLB->get_active_text();
    }
    else if (m_xCTLFontNameLB.get() == &rNameBox)
    {
        pStyleBox = m_xCTLFontStyleLB.get();
        sFontName = m_xCTLFontNameLB->get_active_text();
    }
    else
    {
        SAL_WARN( "cui.tabpages", "invalid font name box" );
        return;
    }

    pStyleBox->Fill(sFontName, pFontList);

    if ( m_pImpl->m_bInSearchMode )
    {
        // additional entries for the search:
        // "not bold" and "not italic"
        OUString aEntry = m_pImpl->m_aNoStyleText;
        const sal_Char sS[] = "%1";
        aEntry = aEntry.replaceFirst( sS, pFontList->GetBoldStr() );
        m_pImpl->m_nExtraEntryPos = pStyleBox->get_count();
        pStyleBox->append_text( aEntry );
        aEntry = m_pImpl->m_aNoStyleText;
        aEntry = aEntry.replaceFirst( sS, pFontList->GetItalicStr() );
        pStyleBox->append_text(aEntry);
    }
}

void SvxCharNamePage::FillSizeBox_Impl(const weld::Widget& rNameBox)
{
    const FontList* pFontList = GetFontList();
    DBG_ASSERT( pFontList, "no fontlist" );

    SvtFontStyleBox* pStyleBox = nullptr;
    SvtFontSizeBox* pSizeBox = nullptr;
    OUString sFontName;

    if (m_xWestFontNameLB.get() == &rNameBox)
    {
        pStyleBox = m_xWestFontStyleLB.get();
        pSizeBox = m_xWestFontSizeLB.get();
        sFontName = m_xWestFontNameLB->get_active_text();
    }
    else if (m_xEastFontNameLB.get() == &rNameBox)
    {
        pStyleBox = m_xEastFontStyleLB.get();
        pSizeBox = m_xEastFontSizeLB.get();
        sFontName = m_xEastFontNameLB->get_active_text();
    }
    else if (m_xCTLFontNameLB.get() == &rNameBox)
    {
        pStyleBox = m_xCTLFontStyleLB.get();
        pSizeBox = m_xCTLFontSizeLB.get();
        sFontName = m_xCTLFontNameLB->get_active_text();
    }
    else
    {
        SAL_WARN( "cui.tabpages", "invalid font name box" );
        return;
    }

    FontMetric _aFontMetric(pFontList->Get(sFontName, pStyleBox->get_active_text()));
    pSizeBox->Fill( &_aFontMetric, pFontList );
}

namespace
{
    void FillFontNames(weld::ComboBox& rBox, const FontList& rList)
    {
        // insert fonts
        sal_uInt16 nFontCount = rList.GetFontNameCount();
        std::vector<weld::ComboBoxEntry> aVector;
        aVector.reserve(nFontCount);
        for (sal_uInt16 i = 0; i < nFontCount; ++i)
        {
            const FontMetric& rFontMetric = rList.GetFontName(i);
            aVector.emplace_back(rFontMetric.GetFamilyName());
        }
        rBox.insert_vector(aVector, false);
    }
}

void SvxCharNamePage::Reset_Impl( const SfxItemSet& rSet, LanguageGroup eLangGrp )
{
    weld::ComboBox* pNameBox = nullptr;
    weld::Label* pStyleLabel = nullptr;
    SvtFontStyleBox* pStyleBox = nullptr;
    weld::Label* pSizeLabel = nullptr;
    SvtFontSizeBox* pSizeBox = nullptr;
    weld::Label* pLangFT = nullptr;
    LanguageBox* pLangBox = nullptr;
    sal_uInt16 nWhich = 0;

    switch ( eLangGrp )
    {
        case Western :
            pNameBox = m_xWestFontNameLB.get();
            pStyleLabel = m_xWestFontStyleFT.get();
            pStyleBox = m_xWestFontStyleLB.get();
            pSizeLabel = m_xWestFontSizeFT.get();
            pSizeBox = m_xWestFontSizeLB.get();
            pLangFT = m_xWestFontLanguageFT.get();
            pLangBox = m_xWestFontLanguageLB.get();
            nWhich = GetWhich( SID_ATTR_CHAR_FONT );
            break;

        case Asian :
            pNameBox = m_xEastFontNameLB.get();
            pStyleLabel = m_xEastFontStyleFT.get();
            pStyleBox = m_xEastFontStyleLB.get();
            pSizeLabel = m_xEastFontSizeFT.get();
            pSizeBox = m_xEastFontSizeLB.get();
            pLangFT = m_xEastFontLanguageFT.get();
            pLangBox = m_xEastFontLanguageLB.get();
            nWhich = GetWhich( SID_ATTR_CHAR_CJK_FONT );
            break;

        case Ctl :
            pNameBox = m_xCTLFontNameLB.get();
            pStyleLabel = m_xCTLFontStyleFT.get();
            pStyleBox = m_xCTLFontStyleLB.get();
            pSizeLabel = m_xCTLFontSizeFT.get();
            pSizeBox = m_xCTLFontSizeLB.get();
            pLangFT = m_xCTLFontLanguageFT.get();
            pLangBox = m_xCTLFontLanguageLB.get();
            nWhich = GetWhich( SID_ATTR_CHAR_CTL_FONT );
            break;
    }

    const FontList* pFontList = GetFontList();
    FillFontNames(*pNameBox, *pFontList);

    const SvxFontItem* pFontItem = nullptr;
    SfxItemState eState = rSet.GetItemState( nWhich );

    if ( eState >= SfxItemState::DEFAULT )
    {
        pFontItem = static_cast<const SvxFontItem*>(&( rSet.Get( nWhich ) ));
        const OUString &rName = pFontItem->GetFamilyName();
        int nIndex = pNameBox->find_text(rName);
        pNameBox->set_active(nIndex);
        // tdf#122992 if it didn't exist in the list, set the entry text to it anyway
        if (nIndex == -1)
            pNameBox->set_entry_text(rName);
    }
    else
    {
        pNameBox->set_active_text( OUString() );
    }

    FillStyleBox_Impl(*pNameBox);

    bool bStyle = false;
    bool bStyleAvailable = true;
    FontItalic eItalic = ITALIC_NONE;
    FontWeight eWeight = WEIGHT_NORMAL;
    switch ( eLangGrp )
    {
        case Western : nWhich = GetWhich( SID_ATTR_CHAR_POSTURE ); break;
        case Asian : nWhich = GetWhich( SID_ATTR_CHAR_CJK_POSTURE ); break;
        case Ctl : nWhich = GetWhich( SID_ATTR_CHAR_CTL_POSTURE ); break;
    }
    eState = rSet.GetItemState( nWhich );

    if ( eState >= SfxItemState::DEFAULT )
    {
        const SvxPostureItem& rItem = static_cast<const SvxPostureItem&>(rSet.Get( nWhich ));
        eItalic = rItem.GetValue();
        bStyle = true;
    }
    bStyleAvailable = bStyleAvailable && (eState >= SfxItemState::DONTCARE);

    switch ( eLangGrp )
    {
        case Western : nWhich = GetWhich( SID_ATTR_CHAR_WEIGHT ); break;
        case Asian : nWhich = GetWhich( SID_ATTR_CHAR_CJK_WEIGHT ); break;
        case Ctl : nWhich = GetWhich( SID_ATTR_CHAR_CTL_WEIGHT ); break;
    }
    eState = rSet.GetItemState( nWhich );

    if ( eState >= SfxItemState::DEFAULT )
    {
        const SvxWeightItem& rItem = static_cast<const SvxWeightItem&>(rSet.Get( nWhich ));
        eWeight = rItem.GetValue();
    }
    else
        bStyle = false;
    bStyleAvailable = bStyleAvailable && (eState >= SfxItemState::DONTCARE);

    // currently chosen font
    if ( bStyle && pFontItem )
    {
        FontMetric aFontMetric = pFontList->Get( pFontItem->GetFamilyName(), eWeight, eItalic );
        pStyleBox->set_active_text( pFontList->GetStyleName( aFontMetric ) );
    }
    else if ( !m_pImpl->m_bInSearchMode || !bStyle )
    {
        pStyleBox->set_active_text( OUString() );
    }
    else if ( bStyle )
    {
        FontMetric aFontMetric = pFontList->Get( OUString(), eWeight, eItalic );
        pStyleBox->set_active_text( pFontList->GetStyleName( aFontMetric ) );
    }
    if (!bStyleAvailable)
    {
        pStyleBox->set_sensitive(false);
        pStyleLabel->set_sensitive(false);
    }

    FillSizeBox_Impl(*pNameBox);
    switch ( eLangGrp )
    {
        case Western : nWhich = GetWhich( SID_ATTR_CHAR_FONTHEIGHT ); break;
        case Asian : nWhich = GetWhich( SID_ATTR_CHAR_CJK_FONTHEIGHT ); break;
        case Ctl : nWhich = GetWhich( SID_ATTR_CHAR_CTL_FONTHEIGHT ); break;
    }
    eState = rSet.GetItemState( nWhich );

    if ( pSizeBox->IsRelativeMode() )
    {
        MapUnit eUnit = rSet.GetPool()->GetMetric( nWhich );
        const SvxFontHeightItem& rItem = static_cast<const SvxFontHeightItem&>(rSet.Get( nWhich ));

        if( rItem.GetProp() != 100 || MapUnit::MapRelative != rItem.GetPropUnit() )
        {
            bool bPtRel = MapUnit::MapPoint == rItem.GetPropUnit();
            pSizeBox->SetPtRelative( bPtRel );
            pSizeBox->set_value( bPtRel ? static_cast<short>(rItem.GetProp()) * 10 : rItem.GetProp() );
        }
        else
        {
            pSizeBox->SetRelative(false);
            pSizeBox->set_value( CalcToPoint( rItem.GetHeight(), eUnit, 10 ) );
        }
    }
    else if ( eState >= SfxItemState::DEFAULT )
    {
        MapUnit eUnit = rSet.GetPool()->GetMetric( nWhich );
        const SvxFontHeightItem& rItem = static_cast<const SvxFontHeightItem&>(rSet.Get( nWhich ));
        pSizeBox->set_value( CalcToPoint( rItem.GetHeight(), eUnit, 10 ) );
    }
    else
    {
        pSizeBox->set_active_text( OUString() );
        if ( eState <= SfxItemState::READONLY )
        {
            pSizeBox->set_sensitive(false);
            pSizeLabel->set_sensitive(false);
        }
    }

    switch ( eLangGrp )
    {
        case Western : nWhich = GetWhich( SID_ATTR_CHAR_LANGUAGE ); break;
        case Asian : nWhich = GetWhich( SID_ATTR_CHAR_CJK_LANGUAGE ); break;
        case Ctl : nWhich = GetWhich( SID_ATTR_CHAR_CTL_LANGUAGE ); break;
    }
    pLangBox->set_active(-1);
    eState = rSet.GetItemState( nWhich );

    switch ( eState )
    {
        case SfxItemState::UNKNOWN:
            pLangFT->hide();
            pLangBox->hide();
            break;

        case SfxItemState::DISABLED:
        case SfxItemState::READONLY:
            pLangFT->set_sensitive(false);
            pLangBox->set_sensitive(false);
            break;

        case SfxItemState::DEFAULT:
        case SfxItemState::SET:
        {
            const SvxLanguageItem& rItem = static_cast<const SvxLanguageItem&>(rSet.Get( nWhich ));
            LanguageType eLangType = rItem.GetValue();
            DBG_ASSERT( eLangType != LANGUAGE_SYSTEM, "LANGUAGE_SYSTEM not allowed" );
            if (eLangType != LANGUAGE_DONTKNOW)
                pLangBox->set_active_id(eLangType);
            break;
        }
        case SfxItemState::DONTCARE:
            break;
    }

    OUString sMapText(pFontList->GetFontMapText(
        pFontList->Get(pNameBox->get_active_text(), pStyleBox->get_active_text())));

    switch (eLangGrp)
    {
        case Western:
            m_xWestFontTypeFT->set_label(sMapText);
            break;
        case Asian:
            m_xEastFontTypeFT->set_label(sMapText);
            break;
        case Ctl:
            m_xCTLFontTypeFT->set_label(sMapText);
            break;
    }

    // save these settings
    pNameBox->save_value();
    pStyleBox->save_value();
    pSizeBox->save_value();
    pLangBox->save_active_id();
}

bool SvxCharNamePage::FillItemSet_Impl( SfxItemSet& rSet, LanguageGroup eLangGrp )
{
    bool bModified = false;

    weld::ComboBox* pNameBox = nullptr;
    SvtFontStyleBox* pStyleBox = nullptr;
    SvtFontSizeBox* pSizeBox = nullptr;
    LanguageBox* pLangBox = nullptr;
    sal_uInt16 nWhich = 0;
    sal_uInt16 nSlot = 0;

    switch ( eLangGrp )
    {
        case Western :
            pNameBox = m_xWestFontNameLB.get();
            pStyleBox = m_xWestFontStyleLB.get();
            pSizeBox = m_xWestFontSizeLB.get();
            pLangBox = m_xWestFontLanguageLB.get();
            nSlot = SID_ATTR_CHAR_FONT;
            break;

        case Asian :
            pNameBox = m_xEastFontNameLB.get();
            pStyleBox = m_xEastFontStyleLB.get();
            pSizeBox = m_xEastFontSizeLB.get();
            pLangBox = m_xEastFontLanguageLB.get();
            nSlot = SID_ATTR_CHAR_CJK_FONT;
            break;

        case Ctl :
            pNameBox = m_xCTLFontNameLB.get();
            pStyleBox = m_xCTLFontStyleLB.get();
            pSizeBox = m_xCTLFontSizeLB.get();
            pLangBox = m_xCTLFontLanguageLB.get();
            nSlot = SID_ATTR_CHAR_CTL_FONT;
            break;
    }

    nWhich = GetWhich( nSlot );
    const SfxPoolItem* pItem = nullptr;
    const SfxItemSet& rOldSet = GetItemSet();
    const SfxPoolItem* pOld = nullptr;

    const SfxItemSet* pExampleSet = GetDialogExampleSet();

    bool bChanged = true;
    const OUString& rFontName  = pNameBox->get_active_text();
    const FontList* pFontList = GetFontList();
    OUString aStyleBoxText = pStyleBox->get_active_text();
    int nEntryPos = pStyleBox->find_text(aStyleBoxText);
    if (nEntryPos >= m_pImpl->m_nExtraEntryPos)
        aStyleBoxText.clear();
    FontMetric aInfo( pFontList->Get( rFontName, aStyleBoxText ) );
    SvxFontItem aFontItem( aInfo.GetFamilyType(), aInfo.GetFamilyName(), aInfo.GetStyleName(),
                           aInfo.GetPitch(), aInfo.GetCharSet(), nWhich );
    pOld = GetOldItem( rSet, nSlot );

    if ( pOld )
    {
        const SvxFontItem& rItem = *static_cast<const SvxFontItem*>(pOld);

        if ( rItem.GetFamilyName() == aFontItem.GetFamilyName() )
            bChanged = false;
    }

    if ( !bChanged )
        bChanged = pNameBox->get_saved_value().isEmpty();

    if ( !bChanged && pExampleSet &&
         pExampleSet->GetItemState( nWhich, false, &pItem ) == SfxItemState::SET &&
         static_cast<const SvxFontItem*>(pItem)->GetFamilyName() != aFontItem.GetFamilyName() )
        bChanged = true;

    if ( bChanged && !rFontName.isEmpty() )
    {
        rSet.Put( aFontItem );
        bModified = true;
    }
    else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich, false ) )
        rSet.ClearItem( nWhich );


    bChanged = true;
    switch ( eLangGrp )
    {
        case Western : nSlot = SID_ATTR_CHAR_WEIGHT; break;
        case Asian : nSlot = SID_ATTR_CHAR_CJK_WEIGHT; break;
        case Ctl : nSlot = SID_ATTR_CHAR_CTL_WEIGHT; break;
    }
    nWhich = GetWhich( nSlot );
    FontWeight eWeight = aInfo.GetWeight();
    if ( nEntryPos >= m_pImpl->m_nExtraEntryPos )
        eWeight = WEIGHT_NORMAL;
    SvxWeightItem aWeightItem( eWeight, nWhich );
    pOld = GetOldItem( rSet, nSlot );

    if ( pOld )
    {
        const SvxWeightItem& rItem = *static_cast<const SvxWeightItem*>(pOld);

        if ( rItem.GetValue() == aWeightItem.GetValue() )
            bChanged = false;
    }

    if ( !bChanged )
    {
        bChanged = pStyleBox->get_saved_value().isEmpty();

        if ( m_pImpl->m_bInSearchMode && bChanged &&
             aInfo.GetWeight() == WEIGHT_NORMAL && aInfo.GetItalic() != ITALIC_NONE )
            bChanged = true;
    }

    if ( !bChanged && pExampleSet &&
         pExampleSet->GetItemState( nWhich, false, &pItem ) == SfxItemState::SET &&
         static_cast<const SvxWeightItem*>(pItem)->GetValue() != aWeightItem.GetValue() )
        bChanged = true;

    if ( nEntryPos >= m_pImpl->m_nExtraEntryPos )
        bChanged = ( nEntryPos == m_pImpl->m_nExtraEntryPos );

    OUString aText( pStyleBox->get_active_text() ); // Tristate, then text empty

    if ( bChanged && !aText.isEmpty() )
    {
        rSet.Put( aWeightItem );
        bModified = true;
    }
    else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich, false ) )
        rSet.InvalidateItem(nWhich);

    bChanged = true;
    switch ( eLangGrp )
    {
        case Western : nSlot = SID_ATTR_CHAR_POSTURE; break;
        case Asian : nSlot = SID_ATTR_CHAR_CJK_POSTURE; break;
        case Ctl : nSlot = SID_ATTR_CHAR_CTL_POSTURE; break;
    }
    nWhich = GetWhich( nSlot );
    FontItalic eItalic = aInfo.GetItalic();
    if ( nEntryPos >= m_pImpl->m_nExtraEntryPos )
        eItalic = ITALIC_NONE;
    SvxPostureItem aPostureItem( eItalic, nWhich );
    pOld = GetOldItem( rSet, nSlot );

    if ( pOld )
    {
        const SvxPostureItem& rItem = *static_cast<const SvxPostureItem*>(pOld);

        if ( rItem.GetValue() == aPostureItem.GetValue() )
            bChanged = false;
    }

    if ( !bChanged )
    {
        bChanged = pStyleBox->get_saved_value().isEmpty();

        if ( m_pImpl->m_bInSearchMode && bChanged &&
             aInfo.GetItalic() == ITALIC_NONE && aInfo.GetWeight() != WEIGHT_NORMAL )
            bChanged = false;
    }

    if ( !bChanged && pExampleSet &&
         pExampleSet->GetItemState( nWhich, false, &pItem ) == SfxItemState::SET &&
         static_cast<const SvxPostureItem*>(pItem)->GetValue() != aPostureItem.GetValue() )
        bChanged = true;

    if ( nEntryPos >= m_pImpl->m_nExtraEntryPos )
        bChanged = ( nEntryPos == ( m_pImpl->m_nExtraEntryPos + 1 ) );

    if ( bChanged && !aText.isEmpty() )
    {
        rSet.Put( aPostureItem );
        bModified = true;
    }
    else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich, false ) )
        rSet.InvalidateItem(nWhich);

    // FontSize
    long nSize = pSizeBox->get_value();

    if ( pSizeBox->get_active_text().isEmpty() )   // GetValue() returns the min-value
        nSize = 0;
    long nSavedSize = pSizeBox->get_saved_value();
    const bool bRel = pSizeBox->IsRelative();

    switch ( eLangGrp )
    {
        case Western : nSlot = SID_ATTR_CHAR_FONTHEIGHT; break;
        case Asian : nSlot = SID_ATTR_CHAR_CJK_FONTHEIGHT; break;
        case Ctl : nSlot = SID_ATTR_CHAR_CTL_FONTHEIGHT; break;
    }
    nWhich = GetWhich( nSlot );
    const SvxFontHeightItem* pOldHeight = static_cast<const SvxFontHeightItem*>(GetOldItem( rSet, nSlot ));
    bChanged = ( nSize != nSavedSize );

    if ( !bChanged && pExampleSet &&
         pExampleSet->GetItemState( nWhich, false, &pItem ) == SfxItemState::SET )
    {
        float fSize = static_cast<float>(nSize) / 10;
        long nVal = CalcToUnit( fSize, rSet.GetPool()->GetMetric( nWhich ) );
        if ( static_cast<const SvxFontHeightItem*>(pItem)->GetHeight() != static_cast<sal_uInt32>(nVal) )
            bChanged = true;
    }

    if ( bChanged || !pOldHeight ||
         bRel != ( MapUnit::MapRelative != pOldHeight->GetPropUnit() || 100 != pOldHeight->GetProp() ) )
    {
        MapUnit eUnit = rSet.GetPool()->GetMetric( nWhich );
        if ( pSizeBox->IsRelative() )
        {
            DBG_ASSERT( GetItemSet().GetParent(), "No parent set" );
            const SvxFontHeightItem& rOldItem =
                static_cast<const SvxFontHeightItem&>(GetItemSet().GetParent()->Get( nWhich ));

            SvxFontHeightItem aHeight( 240, 100, nWhich );
            if ( pSizeBox->IsPtRelative() )
                aHeight.SetHeight( rOldItem.GetHeight(), static_cast<sal_uInt16>( nSize / 10 ), MapUnit::MapPoint, eUnit );
            else
                aHeight.SetHeight( rOldItem.GetHeight(), static_cast<sal_uInt16>(nSize) );
            rSet.Put( aHeight );
        }
        else
        {
            float fSize = static_cast<float>(nSize) / 10;
            rSet.Put( SvxFontHeightItem( CalcToUnit( fSize, eUnit ), 100, nWhich ) );
        }
        bModified = true;
    }
    else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich, false ) )
        rSet.InvalidateItem(nWhich);

    bChanged = true;
    switch ( eLangGrp )
    {
        case Western : nSlot = SID_ATTR_CHAR_LANGUAGE; break;
        case Asian : nSlot = SID_ATTR_CHAR_CJK_LANGUAGE; break;
        case Ctl : nSlot = SID_ATTR_CHAR_CTL_LANGUAGE; break;
    }
    nWhich = GetWhich( nSlot );
    pOld = GetOldItem( rSet, nSlot );

    // For language list boxes acting as ComboBox, check for, add and select an
    // edited entry.
    if (pLangBox == m_xWestFontLanguageLB.get())
    {
        switch (pLangBox->GetEditedAndValid())
        {
            case LanguageBox::EditedAndValid::No:
                ;   // nothing to do
                break;
            case LanguageBox::EditedAndValid::Valid:
                {
                    const int nPos = pLangBox->SaveEditedAsEntry();
                    if (nPos != -1)
                        pLangBox->set_active(nPos);
                }
                break;
            case LanguageBox::EditedAndValid::Invalid:
                pLangBox->set_active_id(pLangBox->get_saved_active_id());
                break;
        }
    }

    int nLangPos = pLangBox->get_active();
    LanguageType eLangType = pLangBox->get_active_id();

    if (pOld)
    {
        const SvxLanguageItem& rItem = *static_cast<const SvxLanguageItem*>(pOld);
        if (nLangPos == -1 || eLangType == rItem.GetValue())
            bChanged = false;
    }

    if (!bChanged)
        bChanged = pLangBox->get_active_id_changed_from_saved();

    if (bChanged && nLangPos != -1)
    {
        rSet.Put(SvxLanguageItem(eLangType, nWhich));
        bModified = true;
    }
    else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich, false ) )
        rSet.InvalidateItem(nWhich);

    return bModified;
}

IMPL_LINK_NOARG(SvxCharNamePage, UpdateHdl_Impl, Timer *, void)
{
    UpdatePreview_Impl();
}

IMPL_LINK(SvxCharNamePage, FontModifyComboBoxHdl_Impl, weld::ComboBox&, rBox, void)
{
    FontModifyHdl_Impl(rBox);
}

IMPL_LINK(SvxCharNamePage, FontModifyEditHdl_Impl, weld::Entry&, rBox, void)
{
    FontModifyHdl_Impl(rBox);
}

IMPL_LINK(SvxCharNamePage, FontFeatureButtonClicked, weld::Button&, rButton, void)
{
    OUString sFontName;
    weld::ComboBox* pNameBox = nullptr;

    if (&rButton == m_xWestFontFeaturesButton.get())
    {
        pNameBox = m_xWestFontNameLB.get();
        sFontName = GetPreviewFont().GetFamilyName();
    }
    else if (&rButton == m_xEastFontFeaturesButton.get())
    {
        pNameBox = m_xEastFontNameLB.get();
        sFontName = GetPreviewCJKFont().GetFamilyName();
    }
    else if (&rButton == m_xCTLFontFeaturesButton.get())
    {
        pNameBox = m_xCTLFontNameLB.get();
        sFontName = GetPreviewCTLFont().GetFamilyName();
    }

    if (!sFontName.isEmpty() && pNameBox)
    {
        cui::FontFeaturesDialog aDialog(GetDialogFrameWeld(), sFontName);
        if (aDialog.run() == RET_OK)
        {
            pNameBox->set_entry_text(aDialog.getResultFontName());
            UpdatePreview_Impl();
        }
    }
}

void SvxCharNamePage::FontModifyHdl_Impl(const weld::Widget& rNameBox)
{
    m_pImpl->m_aUpdateIdle.Start();

    if (m_xWestFontNameLB.get() == &rNameBox || m_xEastFontNameLB.get() == &rNameBox || m_xCTLFontNameLB.get() == &rNameBox)
    {
        FillStyleBox_Impl(rNameBox);
        FillSizeBox_Impl(rNameBox);
    }
}

void SvxCharNamePage::ActivatePage( const SfxItemSet& rSet )
{
    SvxCharBasePage::ActivatePage( rSet );

    UpdatePreview_Impl();       // instead of asynchronous calling in ctor
}

DeactivateRC SvxCharNamePage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( _pSet );
    return DeactivateRC::LeavePage;
}

VclPtr<SfxTabPage> SvxCharNamePage::Create(TabPageParent pParent, const SfxItemSet* rSet)
{
    return VclPtr<SvxCharNamePage>::Create(pParent, *rSet );
}

void SvxCharNamePage::Reset( const SfxItemSet* rSet )
{
    Reset_Impl( *rSet, Western );
    Reset_Impl( *rSet, Asian );
    Reset_Impl( *rSet, Ctl );

    SetPrevFontWidthScale( *rSet );
    UpdatePreview_Impl();
}

void  SvxCharNamePage::ChangesApplied()
{
    m_xWestFontNameLB->save_value();
    m_xWestFontStyleLB->save_value();
    m_xWestFontSizeLB->save_value();
    m_xWestFontLanguageLB->save_active_id();
    m_xEastFontNameLB->save_value();
    m_xEastFontStyleLB->save_value();
    m_xEastFontSizeLB->save_value();
    m_xEastFontLanguageLB->save_active_id();
    m_xCTLFontNameLB->save_value();
    m_xCTLFontStyleLB->save_value();
    m_xCTLFontSizeLB->save_value();
    m_xCTLFontLanguageLB->save_active_id();
}

bool SvxCharNamePage::FillItemSet( SfxItemSet* rSet )
{
    bool bModified = FillItemSet_Impl( *rSet, Western );
    bModified |= FillItemSet_Impl( *rSet, Asian );
    bModified |= FillItemSet_Impl( *rSet, Ctl );
    return bModified;
}

void SvxCharNamePage::SetFontList( const SvxFontListItem& rItem )
{
    m_pImpl->m_pFontList = rItem.GetFontList()->Clone();
}

namespace
{
    void enableRelativeMode( SvxCharNamePage const * _pPage, SvtFontSizeBox* _pFontSizeLB, sal_uInt16 _nHeightWhich )
    {
        _pFontSizeLB->EnableRelativeMode( 5, 995 ); // min 5%, max 995%, step 5

        const SvxFontHeightItem& rHeightItem =
            static_cast<const SvxFontHeightItem&>(_pPage->GetItemSet().GetParent()->Get( _nHeightWhich ));
        MapUnit eUnit = _pPage->GetItemSet().GetPool()->GetMetric( _nHeightWhich );
        short nCurHeight =
            static_cast< short >( CalcToPoint( rHeightItem.GetHeight(), eUnit, 1 ) * 10 );

        // based on the current height:
        //      - negative until minimum of 2 pt
        //      - positive until maximum of 999 pt
        _pFontSizeLB->EnablePtRelativeMode( sal::static_int_cast< short >(-(nCurHeight - 20)), (9999 - nCurHeight) );
    }
}

void SvxCharNamePage::EnableRelativeMode()
{
    DBG_ASSERT( GetItemSet().GetParent(), "RelativeMode, but no ParentSet!" );
    enableRelativeMode(this,m_xWestFontSizeLB.get(),GetWhich( SID_ATTR_CHAR_FONTHEIGHT ));
    enableRelativeMode(this,m_xEastFontSizeLB.get(),GetWhich( SID_ATTR_CHAR_CJK_FONTHEIGHT ));
    enableRelativeMode(this,m_xCTLFontSizeLB.get(),GetWhich( SID_ATTR_CHAR_CTL_FONTHEIGHT ));
}

void SvxCharNamePage::EnableSearchMode()
{
    m_pImpl->m_bInSearchMode = true;
}

void SvxCharNamePage::DisableControls( sal_uInt16 nDisable )
{
    if ( DISABLE_LANGUAGE & nDisable )
    {
        if ( m_xWestFontLanguageFT ) m_xWestFontLanguageFT->set_sensitive(false);
        if ( m_xWestFontLanguageLB ) m_xWestFontLanguageLB->set_sensitive(false);
        if ( m_xEastFontLanguageFT ) m_xEastFontLanguageFT->set_sensitive(false);
        if ( m_xEastFontLanguageLB ) m_xEastFontLanguageLB->set_sensitive(false);
        if ( m_xCTLFontLanguageFT ) m_xCTLFontLanguageFT->set_sensitive(false);
        if ( m_xCTLFontLanguageLB ) m_xCTLFontLanguageLB->set_sensitive(false);
    }

    if ( DISABLE_HIDE_LANGUAGE & nDisable )
    {
        if ( m_xWestFontLanguageFT ) m_xWestFontLanguageFT->hide();
        if ( m_xWestFontLanguageLB ) m_xWestFontLanguageLB->hide();
        if ( m_xEastFontLanguageFT ) m_xEastFontLanguageFT->hide();
        if ( m_xEastFontLanguageLB ) m_xEastFontLanguageLB->hide();
        if ( m_xCTLFontLanguageFT ) m_xCTLFontLanguageFT->hide();
        if ( m_xCTLFontLanguageLB ) m_xCTLFontLanguageLB->hide();
    }
}

void SvxCharNamePage::PageCreated(const SfxAllItemSet& aSet)
{
    const SvxFontListItem* pFontListItem = aSet.GetItem<SvxFontListItem>(SID_ATTR_CHAR_FONTLIST, false);
    const SfxUInt32Item* pFlagItem = aSet.GetItem<SfxUInt32Item>(SID_FLAG_TYPE, false);
    const SfxUInt16Item* pDisalbeItem = aSet.GetItem<SfxUInt16Item>(SID_DISABLE_CTL, false);
    if (pFontListItem)
        SetFontList(*pFontListItem);

    if (pFlagItem)
    {
        sal_uInt32 nFlags=pFlagItem->GetValue();
        if ( ( nFlags & SVX_RELATIVE_MODE ) == SVX_RELATIVE_MODE )
            EnableRelativeMode();
        if ( ( nFlags & SVX_PREVIEW_CHARACTER ) == SVX_PREVIEW_CHARACTER )
            // the writer uses SID_ATTR_BRUSH as font background
            m_bPreviewBackgroundToCharacter = true;
    }
    if (pDisalbeItem)
        DisableControls(pDisalbeItem->GetValue());
}
// class SvxCharEffectsPage ----------------------------------------------

SvxCharEffectsPage::SvxCharEffectsPage(TabPageParent pParent, const SfxItemSet& rInSet)
    : SvxCharBasePage(pParent, "cui/ui/effectspage.ui", "EffectsPage", rInSet)
    , m_bOrigFontColor(false)
    , m_bNewFontColor(false)
    , m_bEnableNoneFontColor(false)
    , m_bUnderlineColorDisabled(false)
    , m_xFontColorFT(m_xBuilder->weld_label("fontcolorft"))
    , m_xFontColorLB(new ColorListBox(m_xBuilder->weld_menu_button("fontcolorlb"), pParent.GetFrameWeld()))
    , m_xEffectsFT(m_xBuilder->weld_label("effectsft"))
    , m_xEffectsLB(m_xBuilder->weld_combo_box("effectslb"))
    , m_xReliefFT(m_xBuilder->weld_label("reliefft"))
    , m_xReliefLB(m_xBuilder->weld_combo_box("relieflb"))
    , m_xOutlineBtn(m_xBuilder->weld_check_button("outlinecb"))
    , m_xShadowBtn(m_xBuilder->weld_check_button("shadowcb"))
    , m_xBlinkingBtn(m_xBuilder->weld_check_button("blinkingcb"))
    , m_xHiddenBtn(m_xBuilder->weld_check_button("hiddencb"))
    , m_xOverlineLB(m_xBuilder->weld_combo_box("overlinelb"))
    , m_xOverlineColorFT(m_xBuilder->weld_label("overlinecolorft"))
    , m_xOverlineColorLB(new ColorListBox(m_xBuilder->weld_menu_button("overlinecolorlb"), pParent.GetFrameWeld()))
    , m_xStrikeoutLB(m_xBuilder->weld_combo_box("strikeoutlb"))
    , m_xUnderlineLB(m_xBuilder->weld_combo_box("underlinelb"))
    , m_xUnderlineColorFT(m_xBuilder->weld_label("underlinecolorft"))
    , m_xUnderlineColorLB(new ColorListBox(m_xBuilder->weld_menu_button("underlinecolorlb"), pParent.GetFrameWeld()))
    , m_xIndividualWordsBtn(m_xBuilder->weld_check_button("individualwordscb"))
    , m_xEmphasisFT(m_xBuilder->weld_label("emphasisft"))
    , m_xEmphasisLB(m_xBuilder->weld_combo_box("emphasislb"))
    , m_xPositionFT(m_xBuilder->weld_label("positionft"))
    , m_xPositionLB(m_xBuilder->weld_combo_box("positionlb"))
    , m_xA11yWarningFT(m_xBuilder->weld_label("a11ywarning"))
{
    m_xPreviewWin.reset(new weld::CustomWeld(*m_xBuilder, "preview", m_aPreviewWin));
#ifdef IOS
    m_xPreviewWin->hide();
#endif
    m_xFontColorLB->SetSlotId(SID_ATTR_CHAR_COLOR);
    m_xOverlineColorLB->SetSlotId(SID_ATTR_CHAR_COLOR);
    m_xUnderlineColorLB->SetSlotId(SID_ATTR_CHAR_COLOR);
    Initialize();
}

void SvxCharEffectsPage::EnableNoneFontColor()
{
    m_xFontColorLB->SetSlotId(SID_ATTR_CHAR_COLOR, true);
    m_bEnableNoneFontColor = true;
}

Color SvxCharEffectsPage::GetPreviewFontColor(const Color& rColor) const
{
    if (rColor == COL_AUTO)
        return COL_BLACK;
    if (m_bEnableNoneFontColor && rColor == COL_NONE_COLOR)
        return COL_BLACK;
    return rColor;
}

SvxCharEffectsPage::~SvxCharEffectsPage()
{
    disposeOnce();
}

void SvxCharEffectsPage::dispose()
{
    m_xUnderlineColorLB.reset();
    m_xOverlineColorLB.reset();
    m_xFontColorLB.reset();
    SvxCharBasePage::dispose();
}

void SvxCharEffectsPage::Initialize()
{
    // to handle the changes of the other pages
    SetExchangeSupport();

    // HTML-Mode
    const SfxPoolItem* pItem;
    SfxObjectShell* pShell;
    if ( SfxItemState::SET == GetItemSet().GetItemState( SID_HTML_MODE, false, &pItem ) ||
         ( nullptr != ( pShell = SfxObjectShell::Current() ) &&
           nullptr != ( pItem = pShell->GetItem( SID_HTML_MODE ) ) ) )
    {
        m_nHtmlMode = static_cast<const SfxUInt16Item*>(pItem)->GetValue();
        if ( ( m_nHtmlMode & HTMLMODE_ON ) == HTMLMODE_ON )
        {
            //!!! hide some controls please
        }
    }

    m_xFontColorLB->SetSelectHdl(LINK(this, SvxCharEffectsPage, ColorBoxSelectHdl_Impl));

    // handler
    Link<weld::ComboBox&,void> aLink = LINK( this, SvxCharEffectsPage, SelectListBoxHdl_Impl );
    m_xUnderlineLB->connect_changed( aLink );
    m_xUnderlineColorLB->SetSelectHdl(LINK(this, SvxCharEffectsPage, ColorBoxSelectHdl_Impl));
    m_xOverlineLB->connect_changed( aLink );
    m_xOverlineColorLB->SetSelectHdl(LINK(this, SvxCharEffectsPage, ColorBoxSelectHdl_Impl));
    m_xStrikeoutLB->connect_changed( aLink );
    m_xEmphasisLB->connect_changed( aLink );
    m_xPositionLB->connect_changed( aLink );
    m_xEffectsLB->connect_changed( aLink );
    m_xReliefLB->connect_changed( aLink );

    m_xUnderlineLB->set_active( 0 );
    m_xOverlineLB->set_active( 0 );
    m_xStrikeoutLB->set_active( 0 );
    m_xEmphasisLB->set_active( 0 );
    m_xPositionLB->set_active( 0 );
    SelectHdl_Impl(nullptr);
    SelectHdl_Impl(m_xEmphasisLB.get());

    m_xEffectsLB->set_active( 0 );

    m_xIndividualWordsBtn->connect_toggled(LINK(this, SvxCharEffectsPage, CbClickHdl_Impl));
    Link<weld::ToggleButton&,void> aLink2 = LINK(this, SvxCharEffectsPage, TristClickHdl_Impl);
    m_xOutlineBtn->connect_toggled(aLink2);
    m_xShadowBtn->connect_toggled(aLink2);

    if ( !SvtLanguageOptions().IsAsianTypographyEnabled() )
    {
        m_xEmphasisFT->hide();
        m_xEmphasisLB->hide();
        m_xPositionFT->hide();
        m_xPositionLB->hide();
    }

    m_xA11yWarningFT->set_visible(officecfg::Office::Common::Accessibility::IsAutomaticFontColor::get());
}

void SvxCharEffectsPage::UpdatePreview_Impl()
{
    SvxFont& rFont = GetPreviewFont();
    SvxFont& rCJKFont = GetPreviewCJKFont();
    SvxFont& rCTLFont = GetPreviewCTLFont();

    const Color& rSelectedColor = m_xFontColorLB->GetSelectEntryColor();
    rFont.SetColor(GetPreviewFontColor(rSelectedColor));
    rCJKFont.SetColor(GetPreviewFontColor(rSelectedColor));
    rCTLFont.SetColor(GetPreviewFontColor(rSelectedColor));

    FontLineStyle eUnderline = static_cast<FontLineStyle>(m_xUnderlineLB->get_active_id().toInt32());
    FontLineStyle eOverline = static_cast<FontLineStyle>(m_xOverlineLB->get_active_id().toInt32());
    FontStrikeout eStrikeout = static_cast<FontStrikeout>(m_xStrikeoutLB->get_active_id().toInt32());
    rFont.SetUnderline( eUnderline );
    rCJKFont.SetUnderline( eUnderline );
    rCTLFont.SetUnderline( eUnderline );
    m_aPreviewWin.SetTextLineColor( m_xUnderlineColorLB->GetSelectEntryColor() );
    rFont.SetOverline( eOverline );
    rCJKFont.SetOverline( eOverline );
    rCTLFont.SetOverline( eOverline );
    m_aPreviewWin.SetOverlineColor( m_xOverlineColorLB->GetSelectEntryColor() );
    rFont.SetStrikeout( eStrikeout );
    rCJKFont.SetStrikeout( eStrikeout );
    rCTLFont.SetStrikeout( eStrikeout );

    auto nEmphasis = m_xEmphasisLB->get_active();
    if (nEmphasis != -1)
    {
        bool bUnder = (CHRDLG_POSITION_UNDER == m_xPositionLB->get_active_id().toInt32());
        FontEmphasisMark eMark = static_cast<FontEmphasisMark>(nEmphasis);
        eMark |= bUnder ? FontEmphasisMark::PosBelow : FontEmphasisMark::PosAbove;
        rFont.SetEmphasisMark( eMark );
        rCJKFont.SetEmphasisMark( eMark );
        rCTLFont.SetEmphasisMark( eMark );
    }

    auto nRelief = m_xReliefLB->get_active();
    if (nRelief != -1)
    {
        rFont.SetRelief( static_cast<FontRelief>(nRelief) );
        rCJKFont.SetRelief( static_cast<FontRelief>(nRelief) );
        rCTLFont.SetRelief( static_cast<FontRelief>(nRelief) );
    }

    rFont.SetOutline( StateToAttr( m_xOutlineBtn->get_state() ) );
    rCJKFont.SetOutline( rFont.IsOutline() );
    rCTLFont.SetOutline( rFont.IsOutline() );

    rFont.SetShadow( StateToAttr( m_xShadowBtn->get_state() ) );
    rCJKFont.SetShadow( rFont.IsShadow() );
    rCTLFont.SetShadow( rFont.IsShadow() );

    auto nCapsPos = m_xEffectsLB->get_active();
    if (nCapsPos != -1)
    {
        SvxCaseMap eCaps = static_cast<SvxCaseMap>(nCapsPos);
        rFont.SetCaseMap( eCaps );
        rCJKFont.SetCaseMap( eCaps );
        // #i78474# small caps do not exist in CTL fonts
        rCTLFont.SetCaseMap( eCaps == SvxCaseMap::SmallCaps ? SvxCaseMap::NotMapped : eCaps );
    }

    bool bWordLine = m_xIndividualWordsBtn->get_active();
    rFont.SetWordLineMode( bWordLine );
    rCJKFont.SetWordLineMode( bWordLine );
    rCTLFont.SetWordLineMode( bWordLine );

    m_aPreviewWin.Invalidate();
}

void SvxCharEffectsPage::SetCaseMap_Impl( SvxCaseMap eCaseMap )
{
    if ( SvxCaseMap::End > eCaseMap )
        m_xEffectsLB->set_active(
            sal::static_int_cast< sal_Int32 >( eCaseMap ) );
    else
    {
        // not mapped
        m_xEffectsLB->set_active(-1);
    }

    UpdatePreview_Impl();
}

void SvxCharEffectsPage::ResetColor_Impl( const SfxItemSet& rSet )
{
    sal_uInt16 nWhich = GetWhich( SID_ATTR_CHAR_COLOR );
    SfxItemState eState = rSet.GetItemState( nWhich );

    m_bOrigFontColor = false;
    switch ( eState )
    {
        case SfxItemState::UNKNOWN:
            m_xFontColorFT->hide();
            m_xFontColorLB->hide();
            break;

        case SfxItemState::DISABLED:
        case SfxItemState::READONLY:
            m_xFontColorFT->set_sensitive(false);
            m_xFontColorLB->set_sensitive(false);
            break;

        case SfxItemState::DONTCARE:
            //Related: tdf#106080 if there is no font color, then allow "none"
            //as a color so the listbox can display that state.
            EnableNoneFontColor();
            m_xFontColorLB->SetNoSelection();
            break;

        case SfxItemState::DEFAULT:
        case SfxItemState::SET:
        {
            SvxFont& rFont = GetPreviewFont();
            SvxFont& rCJKFont = GetPreviewCJKFont();
            SvxFont& rCTLFont = GetPreviewCTLFont();

            const SvxColorItem& rItem = static_cast<const SvxColorItem&>(rSet.Get( nWhich ));
            Color aColor = rItem.GetValue();
            rFont.SetColor(GetPreviewFontColor(aColor));
            rCJKFont.SetColor(GetPreviewFontColor(aColor));
            rCTLFont.SetColor(GetPreviewFontColor(aColor));

            m_aPreviewWin.Invalidate();

            m_xFontColorLB->SelectEntry(aColor);

            m_aOrigFontColor = aColor;
            m_bOrigFontColor = true;
            break;
        }
    }
    m_bNewFontColor = false;
}

bool SvxCharEffectsPage::FillItemSetColor_Impl( SfxItemSet& rSet )
{
    sal_uInt16 nWhich = GetWhich( SID_ATTR_CHAR_COLOR );
    const SfxItemSet& rOldSet = GetItemSet();

    Color aSelectedColor;
    bool bChanged = m_bNewFontColor;

    if (bChanged)
    {
        aSelectedColor = m_xFontColorLB->GetSelectEntryColor();
        if (m_bOrigFontColor)
            bChanged = aSelectedColor != m_aOrigFontColor;
        if (m_bEnableNoneFontColor && bChanged && aSelectedColor == COL_NONE_COLOR)
            bChanged = false;
    }

    if (bChanged)
        rSet.Put( SvxColorItem( aSelectedColor, nWhich ) );
    else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich, false ) )
        rSet.InvalidateItem(nWhich);

    return bChanged;
}

IMPL_LINK( SvxCharEffectsPage, SelectListBoxHdl_Impl, weld::ComboBox&, rBox, void )
{
    SelectHdl_Impl(&rBox);
}

void SvxCharEffectsPage::SelectHdl_Impl(const weld::ComboBox* pBox)
{
    if (m_xEmphasisLB.get() == pBox)
    {
        auto nEPos = m_xEmphasisLB->get_active();
        bool bEnable = nEPos > 0;
        m_xPositionFT->set_sensitive( bEnable );
        m_xPositionLB->set_sensitive( bEnable );
    }
    else if (m_xReliefLB.get() == pBox)
    {
        bool bEnable = ( pBox->get_active() == 0 );
        m_xOutlineBtn->set_sensitive( bEnable );
        m_xShadowBtn->set_sensitive( bEnable );
    }
    else if (m_xPositionLB.get() != pBox)
    {
        bool bUEnable = false;
        if (!m_bUnderlineColorDisabled)
        {
            auto nUPos = m_xUnderlineLB->get_active();
            bUEnable = nUPos > 0;
            m_xUnderlineColorFT->set_sensitive(bUEnable);
            m_xUnderlineColorLB->set_sensitive(bUEnable);
        }

        auto nOPos = m_xOverlineLB->get_active();
        bool bOEnable = nOPos > 0;
        m_xOverlineColorFT->set_sensitive(bOEnable);
        m_xOverlineColorLB->set_sensitive(bOEnable);

        auto nSPos = m_xStrikeoutLB->get_active();
        m_xIndividualWordsBtn->set_sensitive( bUEnable || bOEnable || nSPos > 0);
    }
    UpdatePreview_Impl();
}

IMPL_LINK_NOARG(SvxCharEffectsPage, UpdatePreview_Impl, weld::ComboBox&, void)
{
    bool bEnable = ( ( m_xUnderlineLB->get_active() > 0 ) ||
                     ( m_xOverlineLB->get_active()  > 0 ) ||
                     ( m_xStrikeoutLB->get_active() > 0 ) );
    m_xIndividualWordsBtn->set_sensitive( bEnable );

    UpdatePreview_Impl();
}

IMPL_LINK_NOARG(SvxCharEffectsPage, CbClickHdl_Impl, weld::ToggleButton&, void)
{
    UpdatePreview_Impl();
}

IMPL_LINK_NOARG(SvxCharEffectsPage, TristClickHdl_Impl, weld::ToggleButton&, void)
{
    UpdatePreview_Impl();
}

IMPL_LINK(SvxCharEffectsPage, ColorBoxSelectHdl_Impl, ColorListBox&, rBox, void)
{
    if (m_xFontColorLB.get() == &rBox)
        m_bNewFontColor = true;
    UpdatePreview_Impl();
}

DeactivateRC SvxCharEffectsPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( _pSet );
    return DeactivateRC::LeavePage;
}

VclPtr<SfxTabPage> SvxCharEffectsPage::Create( TabPageParent pParent, const SfxItemSet* rSet )
{
    return VclPtr<SvxCharEffectsPage>::Create( pParent, *rSet );
}

void SvxCharEffectsPage::Reset( const SfxItemSet* rSet )
{
    SvxFont& rFont = GetPreviewFont();
    SvxFont& rCJKFont = GetPreviewCJKFont();
    SvxFont& rCTLFont = GetPreviewCTLFont();

    bool bEnable = false;

    // Underline
    sal_uInt16 nWhich = GetWhich( SID_ATTR_CHAR_UNDERLINE );
    rFont.SetUnderline( LINESTYLE_NONE );
    rCJKFont.SetUnderline( LINESTYLE_NONE );
    rCTLFont.SetUnderline( LINESTYLE_NONE );

    m_xUnderlineLB->set_active( 0 );
    SfxItemState eState = rSet->GetItemState( nWhich );

    if ( eState >= SfxItemState::DONTCARE )
    {
        if ( eState == SfxItemState::DONTCARE )
            m_xUnderlineLB->set_active(-1);
        else
        {
            const SvxUnderlineItem& rItem = static_cast<const SvxUnderlineItem&>(rSet->Get( nWhich ));
            FontLineStyle eUnderline = rItem.GetValue();
            rFont.SetUnderline( eUnderline );
            rCJKFont.SetUnderline( eUnderline );
            rCTLFont.SetUnderline( eUnderline );

            if ( eUnderline != LINESTYLE_NONE )
            {
                auto nPos = m_xUnderlineLB->find_id(OUString::number(eUnderline));
                if (nPos != -1)
                {
                    m_xUnderlineLB->set_active(nPos);
                    bEnable = true;
                }
                Color aColor = rItem.GetColor();
                m_xUnderlineColorLB->SelectEntry(aColor);
            }
            else
            {
                m_xUnderlineColorLB->SelectEntry(COL_AUTO);
                m_xUnderlineColorLB->set_sensitive(false);
            }
        }
    }

    // Overline
    nWhich = GetWhich( SID_ATTR_CHAR_OVERLINE );
    rFont.SetOverline( LINESTYLE_NONE );
    rCJKFont.SetOverline( LINESTYLE_NONE );
    rCTLFont.SetOverline( LINESTYLE_NONE );

    m_xOverlineLB->set_active( 0 );
    eState = rSet->GetItemState( nWhich );

    if ( eState >= SfxItemState::DONTCARE )
    {
        if ( eState == SfxItemState::DONTCARE )
            m_xOverlineLB->set_active(-1);
        else
        {
            const SvxOverlineItem& rItem = static_cast<const SvxOverlineItem&>(rSet->Get( nWhich ));
            FontLineStyle eOverline = rItem.GetValue();
            rFont.SetOverline( eOverline );
            rCJKFont.SetOverline( eOverline );
            rCTLFont.SetOverline( eOverline );

            if ( eOverline != LINESTYLE_NONE )
            {
                auto nPos = m_xOverlineLB->find_id(OUString::number(eOverline));
                if (nPos != -1)
                {
                    m_xOverlineLB->set_active(nPos);
                    bEnable = true;
                }
                Color aColor = rItem.GetColor();
                m_xOverlineColorLB->SelectEntry(aColor);
            }
            else
            {
                m_xOverlineColorLB->SelectEntry(COL_AUTO);
                m_xOverlineColorLB->set_sensitive(false);
            }
        }
    }

    //  Strikeout
    nWhich = GetWhich( SID_ATTR_CHAR_STRIKEOUT );
    rFont.SetStrikeout( STRIKEOUT_NONE );
    rCJKFont.SetStrikeout( STRIKEOUT_NONE );
    rCTLFont.SetStrikeout( STRIKEOUT_NONE );

    m_xStrikeoutLB->set_active( 0 );
    eState = rSet->GetItemState( nWhich );

    if ( eState >= SfxItemState::DONTCARE )
    {
        if ( eState == SfxItemState::DONTCARE )
            m_xStrikeoutLB->set_active(-1);
        else
        {
            const SvxCrossedOutItem& rItem = static_cast<const SvxCrossedOutItem&>(rSet->Get( nWhich ));
            FontStrikeout eStrikeout = rItem.GetValue();
            rFont.SetStrikeout( eStrikeout );
            rCJKFont.SetStrikeout( eStrikeout );
            rCTLFont.SetStrikeout( eStrikeout );

            if ( eStrikeout != STRIKEOUT_NONE )
            {
                auto nPos = m_xStrikeoutLB->find_id(OUString::number(eStrikeout));
                if (nPos != -1)
                {
                    m_xStrikeoutLB->set_active(nPos);
                    bEnable = true;
                }
            }
        }
    }

    // WordLineMode
    nWhich = GetWhich( SID_ATTR_CHAR_WORDLINEMODE );
    eState = rSet->GetItemState( nWhich );

    switch ( eState )
    {
        case SfxItemState::UNKNOWN:
            m_xIndividualWordsBtn->hide();
            break;

        case SfxItemState::DISABLED:
        case SfxItemState::READONLY:
            m_xIndividualWordsBtn->set_sensitive(false);
            break;

        case SfxItemState::DONTCARE:
            m_xIndividualWordsBtn->set_state( TRISTATE_INDET );
            break;

        case SfxItemState::DEFAULT:
        case SfxItemState::SET:
        {
            const SvxWordLineModeItem& rItem = static_cast<const SvxWordLineModeItem&>(rSet->Get( nWhich ));
            rFont.SetWordLineMode( rItem.GetValue() );
            rCJKFont.SetWordLineMode( rItem.GetValue() );
            rCTLFont.SetWordLineMode( rItem.GetValue() );

            m_xIndividualWordsBtn->set_active(rItem.GetValue());
            m_xIndividualWordsBtn->set_sensitive(bEnable);
            break;
        }
    }

    // Emphasis
    nWhich = GetWhich( SID_ATTR_CHAR_EMPHASISMARK );
    eState = rSet->GetItemState( nWhich );

    if ( eState >= SfxItemState::DEFAULT )
    {
        const SvxEmphasisMarkItem& rItem = static_cast<const SvxEmphasisMarkItem&>(rSet->Get( nWhich ));
        FontEmphasisMark eMark = rItem.GetEmphasisMark();
        rFont.SetEmphasisMark( eMark );
        rCJKFont.SetEmphasisMark( eMark );
        rCTLFont.SetEmphasisMark( eMark );

        m_xEmphasisLB->set_active( static_cast<sal_Int32>(FontEmphasisMark( eMark & FontEmphasisMark::Style )) );
        eMark &= ~FontEmphasisMark::Style;
        int nEntryData = ( eMark == FontEmphasisMark::PosAbove )
            ? CHRDLG_POSITION_OVER
            : ( eMark == FontEmphasisMark::PosBelow ) ? CHRDLG_POSITION_UNDER : 0;

        auto nPos = m_xPositionLB->find_id(OUString::number(nEntryData));
        if (nPos != -1)
            m_xPositionLB->set_active(nPos);
    }
    else if ( eState == SfxItemState::DONTCARE )
        m_xEmphasisLB->set_active(-1);
    else if ( eState == SfxItemState::UNKNOWN )
    {
        m_xEmphasisFT->hide();
        m_xEmphasisLB->hide();
    }
    else // SfxItemState::DISABLED or SfxItemState::READONLY
    {
        m_xEmphasisFT->set_sensitive(false);
        m_xEmphasisLB->set_sensitive(false);
    }

    // the select handler for the underline/overline/strikeout list boxes
    SelectHdl_Impl(m_xUnderlineLB.get());

    // the select handler for the emphasis listbox
    SelectHdl_Impl(m_xEmphasisLB.get());

    // Effects
    SvxCaseMap eCaseMap = SvxCaseMap::End;
    nWhich = GetWhich( SID_ATTR_CHAR_CASEMAP );
    eState = rSet->GetItemState( nWhich );
    switch ( eState )
    {
        case SfxItemState::UNKNOWN:
            m_xEffectsFT->hide();
            m_xEffectsLB->hide();
            break;

        case SfxItemState::DISABLED:
        case SfxItemState::READONLY:
            m_xEffectsFT->set_sensitive(false);
            m_xEffectsLB->set_sensitive(false);
            break;

        case SfxItemState::DONTCARE:
            m_xEffectsLB->set_active(-1);
            break;

        case SfxItemState::DEFAULT:
        case SfxItemState::SET:
        {
            const SvxCaseMapItem& rItem = static_cast<const SvxCaseMapItem&>(rSet->Get( nWhich ));
            eCaseMap = rItem.GetValue();
            break;
        }
    }
    SetCaseMap_Impl( eCaseMap );

    //Relief
    nWhich = GetWhich(SID_ATTR_CHAR_RELIEF);
    eState = rSet->GetItemState( nWhich );
    switch ( eState )
    {
        case SfxItemState::UNKNOWN:
            m_xReliefFT->hide();
            m_xReliefLB->hide();
            break;

        case SfxItemState::DISABLED:
        case SfxItemState::READONLY:
            m_xReliefFT->set_sensitive(false);
            m_xReliefLB->set_sensitive(false);
            break;

        case SfxItemState::DONTCARE:
            m_xReliefLB->set_active(-1);
            break;

        case SfxItemState::DEFAULT:
        case SfxItemState::SET:
        {
            const SvxCharReliefItem& rItem = static_cast<const SvxCharReliefItem&>(rSet->Get( nWhich ));
            m_xReliefLB->set_active(static_cast<sal_Int32>(rItem.GetValue()));
            SelectHdl_Impl(m_xReliefLB.get());
            break;
        }
    }

    // Outline
    nWhich = GetWhich( SID_ATTR_CHAR_CONTOUR );
    eState = rSet->GetItemState( nWhich );
    switch ( eState )
    {
        case SfxItemState::UNKNOWN:
            m_xOutlineBtn->hide();
            break;

        case SfxItemState::DISABLED:
        case SfxItemState::READONLY:
            m_xOutlineBtn->set_sensitive(false);
            break;

        case SfxItemState::DONTCARE:
            m_xOutlineBtn->set_state(TRISTATE_INDET);
            break;

        case SfxItemState::DEFAULT:
        case SfxItemState::SET:
        {
            const SvxContourItem& rItem = static_cast<const SvxContourItem&>(rSet->Get( nWhich ));
            m_xOutlineBtn->set_state(static_cast<TriState>(rItem.GetValue()));
            break;
        }
    }

    // Shadow
    nWhich = GetWhich( SID_ATTR_CHAR_SHADOWED );
    eState = rSet->GetItemState( nWhich );

    switch ( eState )
    {
        case SfxItemState::UNKNOWN:
            m_xShadowBtn->hide();
            break;

        case SfxItemState::DISABLED:
        case SfxItemState::READONLY:
            m_xShadowBtn->set_sensitive(false);
            break;

        case SfxItemState::DONTCARE:
            m_xShadowBtn->set_state( TRISTATE_INDET );
            break;

        case SfxItemState::DEFAULT:
        case SfxItemState::SET:
        {
            const SvxShadowedItem& rItem = static_cast<const SvxShadowedItem&>(rSet->Get( nWhich ));
            m_xShadowBtn->set_state( static_cast<TriState>(rItem.GetValue()) );
            break;
        }
    }

    // Blinking
    nWhich = GetWhich( SID_ATTR_FLASH );
    eState = rSet->GetItemState( nWhich );

    switch ( eState )
    {
        case SfxItemState::UNKNOWN:
            m_xBlinkingBtn->hide();
            break;

        case SfxItemState::DISABLED:
        case SfxItemState::READONLY:
            m_xBlinkingBtn->set_sensitive(false);
            break;

        case SfxItemState::DONTCARE:
            m_xBlinkingBtn->set_state( TRISTATE_INDET );
            break;

        case SfxItemState::DEFAULT:
        case SfxItemState::SET:
        {
            const SvxBlinkItem& rItem = static_cast<const SvxBlinkItem&>(rSet->Get( nWhich ));
            m_xBlinkingBtn->set_state( static_cast<TriState>(rItem.GetValue()) );
            break;
        }
    }
    // Hidden
    nWhich = GetWhich( SID_ATTR_CHAR_HIDDEN );
    eState = rSet->GetItemState( nWhich );

    switch ( eState )
    {
        case SfxItemState::UNKNOWN:
            m_xHiddenBtn->hide();
            break;

        case SfxItemState::DISABLED:
        case SfxItemState::READONLY:
            m_xHiddenBtn->set_sensitive(false);
            break;

        case SfxItemState::DONTCARE:
            m_xHiddenBtn->set_state(TRISTATE_INDET);
            break;

        case SfxItemState::DEFAULT:
        case SfxItemState::SET:
        {
            const SvxCharHiddenItem& rItem = static_cast<const SvxCharHiddenItem&>(rSet->Get( nWhich ));
            m_xHiddenBtn->set_state(static_cast<TriState>(rItem.GetValue()));
            break;
        }
    }

    SetPrevFontWidthScale( *rSet );
    ResetColor_Impl( *rSet );

    // preview update
    m_aPreviewWin.Invalidate();

    // save this settings
    ChangesApplied();
}

void SvxCharEffectsPage::ChangesApplied()
{
    m_xUnderlineLB->save_value();
    m_xOverlineLB->save_value();
    m_xStrikeoutLB->save_value();
    m_xIndividualWordsBtn->save_state();
    m_xEmphasisLB->save_value();
    m_xPositionLB->save_value();
    m_xEffectsLB->save_value();
    m_xReliefLB->save_value();
    m_xOutlineBtn->save_state();
    m_xShadowBtn->save_state();
    m_xBlinkingBtn->save_state();
    m_xHiddenBtn->save_state();
}

bool SvxCharEffectsPage::FillItemSet( SfxItemSet* rSet )
{
    const SfxPoolItem* pOld = nullptr;
    const SfxItemSet& rOldSet = GetItemSet();
    bool bModified = false;
    bool bChanged = true;

    // Underline
    sal_uInt16 nWhich = GetWhich( SID_ATTR_CHAR_UNDERLINE );
    pOld = GetOldItem( *rSet, SID_ATTR_CHAR_UNDERLINE );
    auto nPos = m_xUnderlineLB->get_active();
    FontLineStyle eUnder = static_cast<FontLineStyle>(m_xUnderlineLB->get_active_id().toInt32());

    if ( pOld )
    {
        //! if there are different underline styles in the selection the
        //! item-state in the 'rOldSet' will be invalid. In this case
        //! changing the underline style will be allowed if a style is
        //! selected in the listbox.
        bool bAllowChg = nPos != -1  &&
                         SfxItemState::DEFAULT > rOldSet.GetItemState( nWhich );

        const SvxUnderlineItem& rItem = *static_cast<const SvxUnderlineItem*>(pOld);
        if ( rItem.GetValue() == eUnder &&
             ( LINESTYLE_NONE == eUnder || rItem.GetColor() == m_xUnderlineColorLB->GetSelectEntryColor() ) &&
             ! bAllowChg )
            bChanged = false;
    }

    if ( bChanged )
    {
        SvxUnderlineItem aNewItem( eUnder, nWhich );
        aNewItem.SetColor( m_xUnderlineColorLB->GetSelectEntryColor() );
        rSet->Put( aNewItem );
        bModified = true;
    }
    else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich, false ) )
        rSet->InvalidateItem(nWhich);

    bChanged = true;

    // Overline
    nWhich = GetWhich( SID_ATTR_CHAR_OVERLINE );
    pOld = GetOldItem( *rSet, SID_ATTR_CHAR_OVERLINE );
    nPos = m_xOverlineLB->get_active();
    FontLineStyle eOver = static_cast<FontLineStyle>(m_xOverlineLB->get_active_id().toInt32());

    if ( pOld )
    {
        //! if there are different underline styles in the selection the
        //! item-state in the 'rOldSet' will be invalid. In this case
        //! changing the underline style will be allowed if a style is
        //! selected in the listbox.
        bool bAllowChg = nPos != -1 &&
                         SfxItemState::DEFAULT > rOldSet.GetItemState( nWhich );

        const SvxOverlineItem& rItem = *static_cast<const SvxOverlineItem*>(pOld);
        if ( rItem.GetValue() == eOver &&
             ( LINESTYLE_NONE == eOver || rItem.GetColor() == m_xOverlineColorLB->GetSelectEntryColor() ) &&
             ! bAllowChg )
            bChanged = false;
    }

    if ( bChanged )
    {
        SvxOverlineItem aNewItem( eOver, nWhich );
        aNewItem.SetColor( m_xOverlineColorLB->GetSelectEntryColor() );
        rSet->Put( aNewItem );
        bModified = true;
    }
    else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich, false ) )
        rSet->InvalidateItem(nWhich);

    bChanged = true;

    // Strikeout
    nWhich = GetWhich( SID_ATTR_CHAR_STRIKEOUT );
    pOld = GetOldItem( *rSet, SID_ATTR_CHAR_STRIKEOUT );
    nPos = m_xStrikeoutLB->get_active();
    FontStrikeout eStrike = static_cast<FontStrikeout>(m_xStrikeoutLB->get_active_id().toInt32());

    if ( pOld )
    {
        //! if there are different strikeout styles in the selection the
        //! item-state in the 'rOldSet' will be invalid. In this case
        //! changing the strikeout style will be allowed if a style is
        //! selected in the listbox.
        bool bAllowChg = nPos != -1 &&
                         SfxItemState::DEFAULT > rOldSet.GetItemState( nWhich );

        const SvxCrossedOutItem& rItem = *static_cast<const SvxCrossedOutItem*>(pOld);
        if ( !m_xStrikeoutLB->get_sensitive()
            || (rItem.GetValue() == eStrike  && !bAllowChg) )
            bChanged = false;
    }

    if ( bChanged )
    {
        rSet->Put( SvxCrossedOutItem( eStrike, nWhich ) );
        bModified = true;
    }
    else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich, false ) )
        rSet->InvalidateItem(nWhich);

    bChanged = true;

    // Individual words
    nWhich = GetWhich( SID_ATTR_CHAR_WORDLINEMODE );
    pOld = GetOldItem( *rSet, SID_ATTR_CHAR_WORDLINEMODE );

    if ( pOld )
    {
        const SvxWordLineModeItem& rItem = *static_cast<const SvxWordLineModeItem*>(pOld);
        if ( rItem.GetValue() == m_xIndividualWordsBtn->get_active() )
            bChanged = false;
    }

    if ( rOldSet.GetItemState( nWhich ) == SfxItemState::DONTCARE &&
         ! m_xIndividualWordsBtn->get_state_changed_from_saved() )
        bChanged = false;

    if ( bChanged )
    {
        rSet->Put( SvxWordLineModeItem( m_xIndividualWordsBtn->get_active(), nWhich ) );
        bModified = true;
    }
    else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich, false ) )
        rSet->InvalidateItem(nWhich);

    bChanged = true;

    // Emphasis
    nWhich = GetWhich( SID_ATTR_CHAR_EMPHASISMARK );
    pOld = GetOldItem( *rSet, SID_ATTR_CHAR_EMPHASISMARK );
    int nMarkPos = m_xEmphasisLB->get_active();
    OUString sMarkPos = m_xEmphasisLB->get_active_text();
    OUString sPosPos = m_xPositionLB->get_active_text();
    FontEmphasisMark eMark = static_cast<FontEmphasisMark>(nMarkPos);
    if (m_xPositionLB->get_sensitive())
    {
        eMark |= (CHRDLG_POSITION_UNDER == m_xPositionLB->get_active_id().toInt32())
            ? FontEmphasisMark::PosBelow : FontEmphasisMark::PosAbove;
    }

    if ( pOld )
    {
        if( rOldSet.GetItemState( nWhich ) != SfxItemState::DONTCARE )
        {
            const SvxEmphasisMarkItem& rItem = *static_cast<const SvxEmphasisMarkItem*>(pOld);
            if ( rItem.GetEmphasisMark() == eMark )
                bChanged = false;
        }
    }

    if (rOldSet.GetItemState( nWhich ) == SfxItemState::DONTCARE &&
         m_xEmphasisLB->get_saved_value() == sMarkPos && m_xPositionLB->get_saved_value() == sPosPos)
    {
        bChanged = false;
    }

    if (bChanged)
    {
        rSet->Put( SvxEmphasisMarkItem( eMark, nWhich ) );
        bModified = true;
    }
    else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich, false ) )
        rSet->InvalidateItem(nWhich);

    bChanged = true;

    // Effects
    nWhich = GetWhich( SID_ATTR_CHAR_CASEMAP );
    pOld = GetOldItem( *rSet, SID_ATTR_CHAR_CASEMAP );
    SvxCaseMap eCaseMap = SvxCaseMap::NotMapped;
    bool bChecked = false;
    auto nCapsPos = m_xEffectsLB->get_active();
    if (nCapsPos != -1)
    {
        eCaseMap = static_cast<SvxCaseMap>(nCapsPos);
        bChecked = true;
    }

    if ( pOld )
    {
        //! if there are different effect styles in the selection the
        //! item-state in the 'rOldSet' will be invalid. In this case
        //! changing the effect style will be allowed if a style is
        //! selected in the listbox.
        bool bAllowChg = nPos != -1 &&
                         SfxItemState::DEFAULT > rOldSet.GetItemState( nWhich );

        const SvxCaseMapItem& rItem = *static_cast<const SvxCaseMapItem*>(pOld);
        if ( rItem.GetValue() == eCaseMap  &&  !bAllowChg )
            bChanged = false;
    }

    if ( bChanged && bChecked )
    {
        rSet->Put( SvxCaseMapItem( eCaseMap, nWhich ) );
        bModified = true;
    }
    else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich, false ) )
        rSet->InvalidateItem(nWhich);

    bChanged = true;

    //Relief
    nWhich = GetWhich(SID_ATTR_CHAR_RELIEF);
    if (m_xReliefLB->get_value_changed_from_saved())
    {
        m_xReliefLB->save_value();
        SvxCharReliefItem aRelief(static_cast<FontRelief>(m_xReliefLB->get_active()), nWhich);
        rSet->Put(aRelief);
    }

    // Outline
    const SfxItemSet* pExampleSet = GetDialogExampleSet();
    nWhich = GetWhich( SID_ATTR_CHAR_CONTOUR );
    pOld = GetOldItem( *rSet, SID_ATTR_CHAR_CONTOUR );
    TriState eState = m_xOutlineBtn->get_state();
    const SfxPoolItem* pItem;

    if ( pOld )
    {
        const SvxContourItem& rItem = *static_cast<const SvxContourItem*>(pOld);
        if ( rItem.GetValue() == StateToAttr( eState ) && m_xOutlineBtn->get_saved_state() == eState )
            bChanged = false;
    }

    if ( !bChanged && pExampleSet && pExampleSet->GetItemState( nWhich, false, &pItem ) == SfxItemState::SET &&
         !StateToAttr( eState ) && static_cast<const SvxContourItem*>(pItem)->GetValue() )
        bChanged = true;

    if ( bChanged && eState != TRISTATE_INDET )
    {
        rSet->Put( SvxContourItem( StateToAttr( eState ), nWhich ) );
        bModified = true;
    }
    else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich, false ) )
        rSet->InvalidateItem(nWhich);

    bChanged = true;

    // Shadow
    nWhich = GetWhich( SID_ATTR_CHAR_SHADOWED );
    pOld = GetOldItem( *rSet, SID_ATTR_CHAR_SHADOWED );
    eState = m_xShadowBtn->get_state();

    if ( pOld )
    {
        const SvxShadowedItem& rItem = *static_cast<const SvxShadowedItem*>(pOld);
        if ( rItem.GetValue() == StateToAttr( eState ) && m_xShadowBtn->get_saved_state() == eState )
            bChanged = false;
    }

    if ( !bChanged && pExampleSet && pExampleSet->GetItemState( nWhich, false, &pItem ) == SfxItemState::SET &&
         !StateToAttr( eState ) && static_cast<const SvxShadowedItem*>(pItem)->GetValue() )
        bChanged = true;

    if ( bChanged && eState != TRISTATE_INDET )
    {
        rSet->Put( SvxShadowedItem( StateToAttr( eState ), nWhich ) );
        bModified = true;
    }
    else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich, false ) )
        rSet->InvalidateItem(nWhich);

    bChanged = true;

    // Blinking
    nWhich = GetWhich( SID_ATTR_FLASH );
    pOld = GetOldItem( *rSet, SID_ATTR_FLASH );
    eState = m_xBlinkingBtn->get_state();

    if ( pOld )
    {
        const SvxBlinkItem& rItem = *static_cast<const SvxBlinkItem*>(pOld);
        if ( rItem.GetValue() == StateToAttr( eState ) && m_xBlinkingBtn->get_saved_state() == eState )
            bChanged = false;
    }

    if ( !bChanged && pExampleSet && pExampleSet->GetItemState( nWhich, false, &pItem ) == SfxItemState::SET &&
         !StateToAttr( eState ) && static_cast<const SvxBlinkItem*>(pItem)->GetValue() )
        bChanged = true;

    if ( bChanged && eState != TRISTATE_INDET )
    {
        rSet->Put( SvxBlinkItem( StateToAttr( eState ), nWhich ) );
        bModified = true;
    }
    else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich, false ) )
        rSet->InvalidateItem(nWhich);

    // Hidden
    nWhich = GetWhich( SID_ATTR_CHAR_HIDDEN );
    pOld = GetOldItem( *rSet, SID_ATTR_CHAR_HIDDEN );
    eState = m_xHiddenBtn->get_state();
    bChanged = true;

    if ( pOld )
    {
        const SvxCharHiddenItem& rItem = *static_cast<const SvxCharHiddenItem*>(pOld);
        if ( rItem.GetValue() == StateToAttr( eState ) && m_xHiddenBtn->get_saved_state() == eState )
            bChanged = false;
    }

    if ( !bChanged && pExampleSet && pExampleSet->GetItemState( nWhich, false, &pItem ) == SfxItemState::SET &&
         !StateToAttr( eState ) && static_cast<const SvxCharHiddenItem*>(pItem)->GetValue() )
        bChanged = true;

    if ( bChanged && eState != TRISTATE_INDET )
    {
        rSet->Put( SvxCharHiddenItem( StateToAttr( eState ), nWhich ) );
        bModified = true;
    }
    else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich, false ) )
        rSet->InvalidateItem(nWhich);

    bModified |= FillItemSetColor_Impl( *rSet );

    return bModified;
}

void SvxCharEffectsPage::DisableControls( sal_uInt16 nDisable )
{
    if ( ( DISABLE_CASEMAP & nDisable ) == DISABLE_CASEMAP )
    {
        m_xEffectsFT->set_sensitive(false);
        m_xEffectsLB->set_sensitive(false);
    }

    if ( ( DISABLE_WORDLINE & nDisable ) == DISABLE_WORDLINE )
        m_xIndividualWordsBtn->set_sensitive(false);

    if ( ( DISABLE_BLINK & nDisable ) == DISABLE_BLINK )
        m_xBlinkingBtn->set_sensitive(false);

    if ( ( DISABLE_UNDERLINE_COLOR & nDisable ) == DISABLE_UNDERLINE_COLOR )
    {
        // disable the controls
        m_xUnderlineColorFT->set_sensitive(false);
        m_xUnderlineColorLB->set_sensitive(false);
        m_bUnderlineColorDisabled = true;
    }
}

void SvxCharEffectsPage::PageCreated(const SfxAllItemSet& aSet)
{
    const SfxUInt16Item* pDisableCtlItem = aSet.GetItem<SfxUInt16Item>(SID_DISABLE_CTL, false);
    const SfxUInt32Item* pFlagItem = aSet.GetItem<SfxUInt32Item>(SID_FLAG_TYPE, false);
    if (pDisableCtlItem)
        DisableControls(pDisableCtlItem->GetValue());

    if (pFlagItem)
    {
        sal_uInt32 nFlags=pFlagItem->GetValue();
        if ( ( nFlags & SVX_ENABLE_FLASH ) == SVX_ENABLE_FLASH )
            m_xBlinkingBtn->show();
        if ( ( nFlags & SVX_PREVIEW_CHARACTER ) == SVX_PREVIEW_CHARACTER )
            // the writer uses SID_ATTR_BRUSH as font background
            m_bPreviewBackgroundToCharacter = true;
    }
}

// class SvxCharPositionPage ---------------------------------------------

SvxCharPositionPage::SvxCharPositionPage(TabPageParent pParent, const SfxItemSet& rInSet)
    : SvxCharBasePage(pParent, "cui/ui/positionpage.ui", "PositionPage", rInSet)
    , m_nSuperEsc(short(DFLT_ESC_SUPER))
    , m_nSubEsc(short(DFLT_ESC_SUB))
    , m_nScaleWidthItemSetVal(100)
    , m_nScaleWidthInitialVal(100)
    , m_nSuperProp(sal_uInt8(DFLT_ESC_PROP))
    , m_nSubProp(sal_uInt8(DFLT_ESC_PROP))
    , m_xHighPosBtn(m_xBuilder->weld_radio_button("superscript"))
    , m_xNormalPosBtn(m_xBuilder->weld_radio_button("normal"))
    , m_xLowPosBtn(m_xBuilder->weld_radio_button("subscript"))
    , m_xHighLowFT(m_xBuilder->weld_label("raiselower"))
    , m_xHighLowMF(m_xBuilder->weld_metric_spin_button("raiselowersb", FieldUnit::PERCENT))
    , m_xHighLowRB(m_xBuilder->weld_check_button("automatic"))
    , m_xFontSizeFT(m_xBuilder->weld_label("relativefontsize"))
    , m_xFontSizeMF(m_xBuilder->weld_metric_spin_button("fontsizesb", FieldUnit::PERCENT))
    , m_xRotationContainer(m_xBuilder->weld_widget("rotationcontainer"))
    , m_xScalingFT(m_xBuilder->weld_label("scale"))
    , m_xScalingAndRotationFT(m_xBuilder->weld_label("rotateandscale"))
    , m_x0degRB(m_xBuilder->weld_radio_button("0deg"))
    , m_x90degRB(m_xBuilder->weld_radio_button("90deg"))
    , m_x270degRB(m_xBuilder->weld_radio_button("270deg"))
    , m_xFitToLineCB(m_xBuilder->weld_check_button("fittoline"))
    , m_xScaleWidthMF(m_xBuilder->weld_metric_spin_button("scalewidthsb", FieldUnit::PERCENT))
    , m_xKerningMF(m_xBuilder->weld_metric_spin_button("kerningsb", FieldUnit::POINT))
    , m_xPairKerningBtn(m_xBuilder->weld_check_button("pairkerning"))
{
    m_xPreviewWin.reset(new weld::CustomWeld(*m_xBuilder, "preview", m_aPreviewWin));
#ifdef IOS
    m_xPreviewWin->hide();
#endif
    Initialize();
}

SvxCharPositionPage::~SvxCharPositionPage()
{
}


void SvxCharPositionPage::Initialize()
{
    // to handle the changes of the other pages
    SetExchangeSupport();

    GetPreviewFont().SetFontSize( Size( 0, 240 ) );
    GetPreviewCJKFont().SetFontSize( Size( 0, 240 ) );
    GetPreviewCTLFont().SetFontSize( Size( 0, 240 ) );

    m_xNormalPosBtn->set_active(true);
    PositionHdl_Impl(*m_xNormalPosBtn);

    Link<weld::ToggleButton&,void> aLink2 = LINK(this, SvxCharPositionPage, PositionHdl_Impl);
    m_xHighPosBtn->connect_toggled(aLink2);
    m_xNormalPosBtn->connect_toggled(aLink2);
    m_xLowPosBtn->connect_toggled(aLink2);

    aLink2 = LINK( this, SvxCharPositionPage, RotationHdl_Impl );
    m_x0degRB->connect_toggled(aLink2);
    m_x90degRB->connect_toggled(aLink2);
    m_x270degRB->connect_toggled(aLink2);

    Link<weld::MetricSpinButton&,void> aLink3 = LINK(this, SvxCharPositionPage, ValueChangedHdl_Impl);
    m_xHighLowMF->connect_value_changed(aLink3);
    m_xFontSizeMF->connect_value_changed(aLink3);

    m_xHighLowRB->connect_toggled(LINK(this, SvxCharPositionPage, AutoPositionHdl_Impl));
    m_xFitToLineCB->connect_toggled(LINK(this, SvxCharPositionPage, FitToLineHdl_Impl));
    m_xKerningMF->connect_value_changed(LINK(this, SvxCharPositionPage, KerningModifyHdl_Impl));
    m_xScaleWidthMF->connect_value_changed(LINK(this, SvxCharPositionPage, ScaleWidthModifyHdl_Impl));
}

void SvxCharPositionPage::UpdatePreview_Impl( sal_uInt8 nProp, sal_uInt8 nEscProp, short nEsc )
{
    SetPrevFontEscapement( nProp, nEscProp, nEsc );
}


void SvxCharPositionPage::SetEscapement_Impl( SvxEscapement nEsc )
{
    SvxEscapementItem aEscItm( nEsc, SID_ATTR_CHAR_ESCAPEMENT );

    if ( SvxEscapement::Superscript == nEsc )
    {
        aEscItm.GetEsc() = m_nSuperEsc;
        aEscItm.GetProportionalHeight() = m_nSuperProp;
    }
    else if ( SvxEscapement::Subscript == nEsc )
    {
        aEscItm.GetEsc() = m_nSubEsc;
        aEscItm.GetProportionalHeight() = m_nSubProp;
    }

    short nFac = aEscItm.GetEsc() < 0 ? -1 : 1;

    m_xHighLowMF->set_value(aEscItm.GetEsc() * nFac, FieldUnit::PERCENT);
    m_xFontSizeMF->set_value(aEscItm.GetProportionalHeight(), FieldUnit::PERCENT);

    if ( SvxEscapement::Off == nEsc )
    {
        m_xHighLowFT->set_sensitive(false);
        m_xHighLowMF->set_sensitive(false);
        m_xFontSizeFT->set_sensitive(false);
        m_xFontSizeMF->set_sensitive(false);
        m_xHighLowRB->set_sensitive(false);
    }
    else
    {
        m_xFontSizeFT->set_sensitive(true);
        m_xFontSizeMF->set_sensitive(true);
        m_xHighLowRB->set_sensitive(true);

        if (!m_xHighLowRB->get_active())
        {
            m_xHighLowFT->set_sensitive(true);
            m_xHighLowMF->set_sensitive(true);
        }
        else
            AutoPositionHdl_Impl(*m_xHighLowRB);
    }

    UpdatePreview_Impl( 100, aEscItm.GetProportionalHeight(), aEscItm.GetEsc() );
}


IMPL_LINK_NOARG(SvxCharPositionPage, PositionHdl_Impl, weld::ToggleButton&, void)
{
    SvxEscapement nEsc = SvxEscapement::Off;   // also when pBtn == NULL

    if (m_xHighPosBtn->get_active())
        nEsc = SvxEscapement::Superscript;
    else if (m_xLowPosBtn->get_active())
        nEsc = SvxEscapement::Subscript;

    SetEscapement_Impl( nEsc );
}

IMPL_LINK_NOARG(SvxCharPositionPage, RotationHdl_Impl, weld::ToggleButton&, void)
{
    bool bEnable = false;
    if (m_x90degRB->get_active() || m_x270degRB->get_active())
        bEnable = true;
    else
        OSL_ENSURE(m_x0degRB->get_active(), "unexpected button");
    m_xFitToLineCB->set_sensitive(bEnable);
}

void SvxCharPositionPage::FontModifyHdl_Impl()
{
    sal_uInt8 nEscProp = static_cast<sal_uInt8>(m_xFontSizeMF->get_value(FieldUnit::PERCENT));
    short nEsc  = static_cast<short>(m_xHighLowMF->get_value(FieldUnit::PERCENT));
    nEsc *= m_xLowPosBtn->get_active() ? -1 : 1;
    UpdatePreview_Impl( 100, nEscProp, nEsc );
}

IMPL_LINK(SvxCharPositionPage, AutoPositionHdl_Impl, weld::ToggleButton&, rBox, void)
{
    if (rBox.get_active())
    {
        m_xHighLowFT->set_sensitive(false);
        m_xHighLowMF->set_sensitive(false);
    }
    else
        PositionHdl_Impl(m_xHighPosBtn->get_active() ? *m_xHighPosBtn
                                                     : m_xLowPosBtn->get_active() ? *m_xLowPosBtn
                                                                                  : *m_xNormalPosBtn);
}

IMPL_LINK_NOARG(SvxCharPositionPage, FitToLineHdl_Impl, weld::ToggleButton&, void)
{
    sal_uInt16 nVal = m_nScaleWidthInitialVal;
    if (m_xFitToLineCB->get_active())
        nVal = m_nScaleWidthItemSetVal;
    m_xScaleWidthMF->set_value(nVal, FieldUnit::PERCENT);
    m_aPreviewWin.SetFontWidthScale( nVal );
}

IMPL_LINK_NOARG(SvxCharPositionPage, KerningModifyHdl_Impl, weld::MetricSpinButton&, void)
{
    long nVal = static_cast<long>(m_xKerningMF->get_value(FieldUnit::POINT));
    nVal = LogicToLogic( nVal, MapUnit::MapPoint, MapUnit::MapTwip );
    long nKern = static_cast<short>(m_xKerningMF->denormalize(nVal));

    SvxFont& rFont = GetPreviewFont();
    SvxFont& rCJKFont = GetPreviewCJKFont();
    SvxFont& rCTLFont = GetPreviewCTLFont();

    rFont.SetFixKerning( static_cast<short>(nKern) );
    rCJKFont.SetFixKerning( static_cast<short>(nKern) );
    rCTLFont.SetFixKerning( static_cast<short>(nKern) );
    m_aPreviewWin.Invalidate();
}

IMPL_LINK(SvxCharPositionPage, ValueChangedHdl_Impl, weld::MetricSpinButton&, rField, void)
{
    bool bHigh = m_xHighPosBtn->get_active();
    bool bLow = m_xLowPosBtn->get_active();
    DBG_ASSERT( bHigh || bLow, "normal position is not valid" );

    if (m_xHighLowMF.get() == &rField)
    {
        if ( bLow )
            m_nSubEsc = static_cast<short>(m_xHighLowMF->get_value(FieldUnit::PERCENT)) * -1;
        else
            m_nSuperEsc = static_cast<short>(m_xHighLowMF->get_value(FieldUnit::PERCENT));
    }
    else if (m_xFontSizeMF.get() == &rField)
    {
        if ( bLow )
            m_nSubProp = static_cast<sal_uInt8>(m_xFontSizeMF->get_value(FieldUnit::PERCENT));
        else
            m_nSuperProp = static_cast<sal_uInt8>(m_xFontSizeMF->get_value(FieldUnit::PERCENT));
    }

    FontModifyHdl_Impl();
}

IMPL_LINK_NOARG(SvxCharPositionPage, ScaleWidthModifyHdl_Impl, weld::MetricSpinButton&, void)
{
    m_aPreviewWin.SetFontWidthScale(sal_uInt16(m_xScaleWidthMF->get_value(FieldUnit::PERCENT)));
}

DeactivateRC SvxCharPositionPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( _pSet );
    return DeactivateRC::LeavePage;
}

VclPtr<SfxTabPage> SvxCharPositionPage::Create(TabPageParent pParent, const SfxItemSet* rSet)
{
    return VclPtr<SvxCharPositionPage>::Create(pParent, *rSet);
}

void SvxCharPositionPage::Reset( const SfxItemSet* rSet )
{
    OUString sUser = GetUserData();

    if ( !sUser.isEmpty() )
    {
        sal_Int32 nIdx {0};
        m_nSuperEsc = static_cast<short>(sUser.getToken( 0, ';', nIdx ).toInt32());
        m_nSubEsc = static_cast<short>(sUser.getToken( 0, ';', nIdx ).toInt32());
        m_nSuperProp = static_cast<sal_uInt8>(sUser.getToken( 0, ';', nIdx ).toInt32());
        m_nSubProp = static_cast<sal_uInt8>(sUser.getToken( 0, ';', nIdx ).toInt32());

        //fdo#75307 validate all the entries and discard all of them if any are
        //out of range
        bool bValid = true;
        if (m_nSuperEsc < m_xHighLowMF->get_min(FieldUnit::PERCENT) || m_nSuperEsc > m_xHighLowMF->get_max(FieldUnit::PERCENT))
            bValid = false;
        if (m_nSubEsc*-1 < m_xHighLowMF->get_min(FieldUnit::PERCENT) || m_nSubEsc*-1 > m_xHighLowMF->get_max(FieldUnit::PERCENT))
            bValid = false;
        if (m_nSuperProp < m_xFontSizeMF->get_min(FieldUnit::PERCENT) || m_nSuperProp > m_xFontSizeMF->get_max(FieldUnit::PERCENT))
            bValid = false;
        if (m_nSubProp < m_xFontSizeMF->get_min(FieldUnit::PERCENT) || m_nSubProp > m_xFontSizeMF->get_max(FieldUnit::PERCENT))
            bValid = false;

        if (!bValid)
        {
            m_nSuperEsc = DFLT_ESC_SUPER;
            m_nSubEsc = DFLT_ESC_SUB;
            m_nSuperProp = DFLT_ESC_PROP;
            m_nSubProp = DFLT_ESC_PROP;
        }
    }

    short nEsc = 0;
    sal_uInt8 nEscProp = 100;

    m_xHighLowFT->set_sensitive(false);
    m_xHighLowMF->set_sensitive(false);
    m_xFontSizeFT->set_sensitive(false);
    m_xFontSizeMF->set_sensitive(false);

    SvxFont& rFont = GetPreviewFont();
    SvxFont& rCJKFont = GetPreviewCJKFont();
    SvxFont& rCTLFont = GetPreviewCTLFont();
    sal_uInt16 nWhich = GetWhich( SID_ATTR_CHAR_ESCAPEMENT );

    if ( rSet->GetItemState( nWhich ) >= SfxItemState::DEFAULT )
    {
        const SvxEscapementItem& rItem = static_cast<const SvxEscapementItem&>(rSet->Get( nWhich ));
        nEsc = rItem.GetEsc();
        nEscProp = rItem.GetProportionalHeight();

        if ( nEsc != 0 )
        {
            m_xHighLowFT->set_sensitive(true);
            m_xHighLowMF->set_sensitive(true);
            m_xFontSizeFT->set_sensitive(true);
            m_xFontSizeMF->set_sensitive(true);

            short nFac;
            bool bAutomatic(false);

            if ( nEsc > 0 )
            {
                nFac = 1;
                m_xHighPosBtn->set_active(true);
                if ( nEsc == DFLT_ESC_AUTO_SUPER )
                {
                    nEsc = DFLT_ESC_SUPER;
                    bAutomatic = true;
                }
            }
            else
            {
                nFac = -1;
                m_xLowPosBtn->set_active(true);
                if ( nEsc == DFLT_ESC_AUTO_SUB )
                {
                    nEsc = DFLT_ESC_SUB;
                    bAutomatic = true;
                }
            }
            if (!m_xHighLowRB->get_sensitive())
            {
                m_xHighLowRB->set_sensitive(true);
            }
            m_xHighLowRB->set_active(bAutomatic);

            if (m_xHighLowRB->get_active())
            {
                m_xHighLowFT->set_sensitive(false);
                m_xHighLowMF->set_sensitive(false);
            }
            m_xHighLowMF->set_value(m_xHighLowMF->normalize(nFac * nEsc), FieldUnit::PERCENT);
        }
        else
        {
            m_xNormalPosBtn->set_active(true);
            m_xHighLowRB->set_active(true);
            PositionHdl_Impl(*m_xNormalPosBtn);
        }
        //the height has to be set after the handler is called to keep the value also if the escapement is zero
        m_xFontSizeMF->set_value(m_xFontSizeMF->normalize(nEscProp), FieldUnit::PERCENT);
    }
    else
    {
        m_xHighPosBtn->set_active(false);
        m_xNormalPosBtn->set_active(false);
        m_xLowPosBtn->set_active(false);
    }

    // set BspFont
    SetPrevFontEscapement( 100, nEscProp, nEsc );

    // Kerning
    nWhich = GetWhich( SID_ATTR_CHAR_KERNING );

    if ( rSet->GetItemState( nWhich ) >= SfxItemState::DEFAULT )
    {
        const SvxKerningItem& rItem = static_cast<const SvxKerningItem&>(rSet->Get( nWhich ));
        MapUnit eUnit = rSet->GetPool()->GetMetric( nWhich );
        long nBig = static_cast<long>(m_xKerningMF->normalize( static_cast<long>(rItem.GetValue()) ));
        long nKerning = LogicToLogic( nBig, eUnit, MapUnit::MapPoint );

        // set Kerning at the Font, convert into Twips before
        long nKern = LogicToLogic( rItem.GetValue(), eUnit, MapUnit::MapTwip );
        rFont.SetFixKerning( static_cast<short>(nKern) );
        rCJKFont.SetFixKerning( static_cast<short>(nKern) );
        rCTLFont.SetFixKerning( static_cast<short>(nKern) );

        //the attribute value must be displayed also if it's above the maximum allowed value
        long nVal = static_cast<long>(m_xKerningMF->get_max(FieldUnit::POINT));
        if(nVal < nKerning)
            m_xKerningMF->set_max(nKerning, FieldUnit::POINT);
        m_xKerningMF->set_value(nKerning, FieldUnit::POINT);
    }
    else
        m_xKerningMF->set_text(OUString());

    // Pair kerning
    nWhich = GetWhich( SID_ATTR_CHAR_AUTOKERN );

    if ( rSet->GetItemState( nWhich ) >= SfxItemState::DEFAULT )
    {
        const SvxAutoKernItem& rItem = static_cast<const SvxAutoKernItem&>(rSet->Get( nWhich ));
        m_xPairKerningBtn->set_active(rItem.GetValue());
    }
    else
        m_xPairKerningBtn->set_active(false);

    // Scale Width
    nWhich = GetWhich( SID_ATTR_CHAR_SCALEWIDTH );
    if ( rSet->GetItemState( nWhich ) >= SfxItemState::DEFAULT )
    {
        const SvxCharScaleWidthItem& rItem = static_cast<const SvxCharScaleWidthItem&>( rSet->Get( nWhich ) );
        m_nScaleWidthInitialVal = rItem.GetValue();
        m_xScaleWidthMF->set_value(m_nScaleWidthInitialVal, FieldUnit::PERCENT);
    }
    else
        m_xScaleWidthMF->set_value(100, FieldUnit::PERCENT);

    nWhich = GetWhich( SID_ATTR_CHAR_WIDTH_FIT_TO_LINE );
    if ( rSet->GetItemState( nWhich ) >= SfxItemState::DEFAULT )
        m_nScaleWidthItemSetVal = static_cast<const SfxUInt16Item&>( rSet->Get( nWhich )).GetValue();

    // Rotation
    nWhich = GetWhich( SID_ATTR_CHAR_ROTATED );
    SfxItemState eState = rSet->GetItemState( nWhich );
    if( SfxItemState::UNKNOWN == eState )
    {
        m_xRotationContainer->hide();
        m_xScalingAndRotationFT->hide();
        m_xScalingFT->show();
    }
    else
    {
        m_xRotationContainer->show();
        m_xScalingAndRotationFT->show();
        m_xScalingFT->hide();

        if( eState >= SfxItemState::DEFAULT )
        {
            const SvxCharRotateItem& rItem =
                    static_cast<const SvxCharRotateItem&>( rSet->Get( nWhich ));
            if (rItem.IsBottomToTop())
                m_x90degRB->set_active(true);
            else if (rItem.IsTopToBottom())
                m_x270degRB->set_active(true);
            else
            {
                DBG_ASSERT( 0 == rItem.GetValue(), "incorrect value" );
                m_x0degRB->set_active(true);
            }
            m_xFitToLineCB->set_active(rItem.IsFitToLine());
        }
        else
        {
            if( eState == SfxItemState::DONTCARE )
            {
                m_x0degRB->set_active(false);
                m_x90degRB->set_active(false);
                m_x270degRB->set_active(false);
            }
            else
                m_x0degRB->set_active(true);

            m_xFitToLineCB->set_active(false);
        }
        m_xFitToLineCB->set_sensitive(!m_x0degRB->get_active());

        // is this value set?
        if( SfxItemState::UNKNOWN == rSet->GetItemState( GetWhich(
                                        SID_ATTR_CHAR_WIDTH_FIT_TO_LINE ) ))
            m_xFitToLineCB->hide();
    }
    ChangesApplied();
}

void SvxCharPositionPage::ChangesApplied()
{
    m_xHighPosBtn->save_state();
    m_xNormalPosBtn->save_state();
    m_xLowPosBtn->save_state();
    m_xHighLowRB->save_state();
    m_x0degRB->save_state();
    m_x90degRB->save_state();
    m_x270degRB->save_state();
    m_xFitToLineCB->save_state();
    m_xScaleWidthMF->save_value();
    m_xKerningMF->save_value();
    m_xPairKerningBtn->save_state();
}

bool SvxCharPositionPage::FillItemSet( SfxItemSet* rSet )
{
    //  Position (high, normal or low)
    const SfxItemSet& rOldSet = GetItemSet();
    bool bModified = false, bChanged = true;
    sal_uInt16 nWhich = GetWhich( SID_ATTR_CHAR_ESCAPEMENT );
    const SfxPoolItem* pOld = GetOldItem( *rSet, SID_ATTR_CHAR_ESCAPEMENT );
    const bool bHigh = m_xHighPosBtn->get_active();
    short nEsc;
    sal_uInt8  nEscProp;

    if (bHigh || m_xLowPosBtn->get_active())
    {
        if (m_xHighLowRB->get_active())
            nEsc = bHigh ? DFLT_ESC_AUTO_SUPER : DFLT_ESC_AUTO_SUB;
        else
        {
            nEsc = static_cast<short>(m_xHighLowMF->denormalize(m_xHighLowMF->get_value(FieldUnit::PERCENT)));
            nEsc *= (bHigh ? 1 : -1);
        }
        nEscProp = static_cast<sal_uInt8>(m_xFontSizeMF->denormalize(m_xFontSizeMF->get_value(FieldUnit::PERCENT)));
    }
    else
    {
        nEsc  = 0;
        nEscProp = 100;
    }

    if ( pOld )
    {
        const SvxEscapementItem& rItem = *static_cast<const SvxEscapementItem*>(pOld);
        if (rItem.GetEsc() == nEsc && rItem.GetProportionalHeight() == nEscProp)
            bChanged = false;
    }

    if ( !bChanged && !m_xHighPosBtn->get_saved_state() &&
         !m_xNormalPosBtn->get_saved_state() && !m_xLowPosBtn->get_saved_state() )
        bChanged = true;

    if ( bChanged &&
         ( m_xHighPosBtn->get_active() || m_xNormalPosBtn->get_active() || m_xLowPosBtn->get_active() ) )
    {
        rSet->Put( SvxEscapementItem( nEsc, nEscProp, nWhich ) );
        bModified = true;
    }
    else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich, false ) )
        rSet->InvalidateItem(nWhich);

    bChanged = true;

    // Kerning
    nWhich = GetWhich( SID_ATTR_CHAR_KERNING );
    pOld = GetOldItem( *rSet, SID_ATTR_CHAR_KERNING );
    short nKerning = 0;
    MapUnit eUnit = rSet->GetPool()->GetMetric( nWhich );

    long nTmp = static_cast<long>(m_xKerningMF->get_value(FieldUnit::POINT));
    long nVal = LogicToLogic( nTmp, MapUnit::MapPoint, eUnit );
    nKerning = static_cast<short>(m_xKerningMF->denormalize( nVal ));

    SfxItemState eOldKernState = rOldSet.GetItemState( nWhich, false );
    if ( pOld )
    {
        const SvxKerningItem& rItem = *static_cast<const SvxKerningItem*>(pOld);
        if ( (eOldKernState >= SfxItemState::DEFAULT || m_xKerningMF->get_text().isEmpty()) && rItem.GetValue() == nKerning )
            bChanged = false;
    }

    if ( bChanged )
    {
        rSet->Put( SvxKerningItem( nKerning, nWhich ) );
        bModified = true;
    }
    else if ( SfxItemState::DEFAULT == eOldKernState )
        rSet->InvalidateItem(nWhich);

    // Pair-Kerning
    nWhich = GetWhich( SID_ATTR_CHAR_AUTOKERN );

    if (m_xPairKerningBtn->get_state_changed_from_saved())
    {
        rSet->Put( SvxAutoKernItem( m_xPairKerningBtn->get_active(), nWhich ) );
        bModified = true;
    }
    else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich, false ) )
        rSet->InvalidateItem(nWhich);

    // Scale Width
    nWhich = GetWhich( SID_ATTR_CHAR_SCALEWIDTH );
    if (m_xScaleWidthMF->get_value_changed_from_saved())
    {
        rSet->Put(SvxCharScaleWidthItem(static_cast<sal_uInt16>(m_xScaleWidthMF->get_value(FieldUnit::PERCENT)), nWhich));
        bModified = true;
    }
    else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich, false ) )
        rSet->InvalidateItem(nWhich);

    // Rotation
    nWhich = GetWhich( SID_ATTR_CHAR_ROTATED );
    if ( m_x0degRB->get_state_changed_from_saved()  ||
         m_x90degRB->get_state_changed_from_saved()  ||
         m_x270degRB->get_state_changed_from_saved()  ||
         m_xFitToLineCB->get_state_changed_from_saved() )
    {
        SvxCharRotateItem aItem( 0, m_xFitToLineCB->get_active(), nWhich );
        if (m_x90degRB->get_active())
            aItem.SetBottomToTop();
        else if (m_x270degRB->get_active())
            aItem.SetTopToBottom();
        rSet->Put( aItem );
        bModified = true;
    }
    else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich, false ) )
        rSet->InvalidateItem(nWhich);

    return bModified;
}


void SvxCharPositionPage::FillUserData()
{
    const OUString cTok( ";" );

    OUString sUser = OUString::number( m_nSuperEsc )  + cTok +
                     OUString::number( m_nSubEsc )    + cTok +
                     OUString::number( m_nSuperProp ) + cTok +
                     OUString::number( m_nSubProp );
    SetUserData( sUser );
}


void SvxCharPositionPage::PageCreated(const SfxAllItemSet& aSet)
{
    const SfxUInt32Item* pFlagItem = aSet.GetItem<SfxUInt32Item>(SID_FLAG_TYPE, false);
    if (pFlagItem)
    {
        sal_uInt32 nFlags=pFlagItem->GetValue();
        if ( ( nFlags & SVX_PREVIEW_CHARACTER ) == SVX_PREVIEW_CHARACTER )
            // the writer uses SID_ATTR_BRUSH as font background
            m_bPreviewBackgroundToCharacter = true;
    }
}
// class SvxCharTwoLinesPage ------------------------------------------------

SvxCharTwoLinesPage::SvxCharTwoLinesPage(TabPageParent pParent, const SfxItemSet& rInSet)
    : SvxCharBasePage(pParent, "cui/ui/twolinespage.ui", "TwoLinesPage", rInSet)
    , m_nStartBracketPosition( 0 )
    , m_nEndBracketPosition( 0 )
    , m_xTwoLinesBtn(m_xBuilder->weld_check_button("twolines"))
    , m_xEnclosingFrame(m_xBuilder->weld_widget("enclosing"))
    , m_xStartBracketLB(m_xBuilder->weld_tree_view("startbracket"))
    , m_xEndBracketLB(m_xBuilder->weld_tree_view("endbracket"))
{
    for (size_t i = 0; i < SAL_N_ELEMENTS(TWOLINE_OPEN); ++i)
        m_xStartBracketLB->append(OUString::number(TWOLINE_OPEN[i].second), CuiResId(TWOLINE_OPEN[i].first));
    for (size_t i = 0; i < SAL_N_ELEMENTS(TWOLINE_CLOSE); ++i)
        m_xEndBracketLB->append(OUString::number(TWOLINE_CLOSE[i].second), CuiResId(TWOLINE_CLOSE[i].first));

    m_xPreviewWin.reset(new weld::CustomWeld(*m_xBuilder, "preview", m_aPreviewWin));
#ifdef IOS
    m_xPreviewWin->hide();
#endif
    Initialize();
}

SvxCharTwoLinesPage::~SvxCharTwoLinesPage()
{
}

void SvxCharTwoLinesPage::Initialize()
{
    m_xTwoLinesBtn->set_active(false);
    TwoLinesHdl_Impl(*m_xTwoLinesBtn);

    m_xTwoLinesBtn->connect_toggled(LINK(this, SvxCharTwoLinesPage, TwoLinesHdl_Impl));

    Link<weld::TreeView&,void> aLink = LINK(this, SvxCharTwoLinesPage, CharacterMapHdl_Impl);
    m_xStartBracketLB->connect_changed(aLink);
    m_xEndBracketLB->connect_changed(aLink);

    SvxFont& rFont = GetPreviewFont();
    SvxFont& rCJKFont = GetPreviewCJKFont();
    SvxFont& rCTLFont = GetPreviewCTLFont();
    rFont.SetFontSize( Size( 0, 220 ) );
    rCJKFont.SetFontSize( Size( 0, 220 ) );
    rCTLFont.SetFontSize( Size( 0, 220 ) );
}

void SvxCharTwoLinesPage::SelectCharacter(weld::TreeView* pBox)
{
    bool bStart = pBox == m_xStartBracketLB.get();
    SvxCharacterMap aDlg(GetFrameWeld(), nullptr, false);
    aDlg.DisableFontSelection();

    if (aDlg.run() == RET_OK)
    {
        sal_Unicode cChar = static_cast<sal_Unicode>(aDlg.GetChar());
        SetBracket( cChar, bStart );
    }
    else
    {
        pBox->select(bStart ? m_nStartBracketPosition : m_nEndBracketPosition);
    }
}


void SvxCharTwoLinesPage::SetBracket( sal_Unicode cBracket, bool bStart )
{
    int nEntryPos = 0;
    weld::TreeView* pBox = bStart ? m_xStartBracketLB.get() : m_xEndBracketLB.get();
    if (cBracket == 0)
        pBox->select(0);
    else
    {
        bool bFound = false;
        for (int i = 1; i < pBox->n_children(); ++i)
        {
            if (pBox->get_id(i).toInt32() != CHRDLG_ENCLOSE_SPECIAL_CHAR)
            {
                const sal_Unicode cChar = pBox->get_text(i)[0];
                if (cChar == cBracket)
                {
                    pBox->select(i);
                    nEntryPos = i;
                    bFound = true;
                    break;
                }
            }
        }

        if (!bFound)
        {
            pBox->append_text(OUString(cBracket));
            nEntryPos = pBox->n_children() - 1;
            pBox->select(nEntryPos);
        }
    }
    if (bStart)
        m_nStartBracketPosition = nEntryPos;
    else
        m_nEndBracketPosition = nEntryPos;
}

IMPL_LINK_NOARG(SvxCharTwoLinesPage, TwoLinesHdl_Impl, weld::ToggleButton&, void)
{
    bool bChecked = m_xTwoLinesBtn->get_active();
    m_xEnclosingFrame->set_sensitive(bChecked);
    UpdatePreview_Impl();
}

IMPL_LINK(SvxCharTwoLinesPage, CharacterMapHdl_Impl, weld::TreeView&, rBox, void)
{
    int nPos = rBox.get_selected_index();
    if (rBox.get_id(nPos).toInt32() == CHRDLG_ENCLOSE_SPECIAL_CHAR)
        SelectCharacter( &rBox );
    else
    {
        bool bStart = &rBox == m_xStartBracketLB.get();
        if (bStart)
            m_nStartBracketPosition = nPos;
        else
            m_nEndBracketPosition = nPos;
    }
    UpdatePreview_Impl();
}

void SvxCharTwoLinesPage::ActivatePage( const SfxItemSet& rSet )
{
    SvxCharBasePage::ActivatePage(rSet);
}

DeactivateRC SvxCharTwoLinesPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( _pSet );
    return DeactivateRC::LeavePage;
}

VclPtr<SfxTabPage> SvxCharTwoLinesPage::Create(TabPageParent pParent, const SfxItemSet* rSet)
{
    return VclPtr<SvxCharTwoLinesPage>::Create(pParent, *rSet);
}

void SvxCharTwoLinesPage::Reset( const SfxItemSet* rSet )
{
    m_xTwoLinesBtn->set_active(false);
    sal_uInt16 nWhich = GetWhich( SID_ATTR_CHAR_TWO_LINES );
    SfxItemState eState = rSet->GetItemState( nWhich );

    if ( eState >= SfxItemState::DONTCARE )
    {
        const SvxTwoLinesItem& rItem = static_cast<const SvxTwoLinesItem&>(rSet->Get( nWhich ));
        m_xTwoLinesBtn->set_active(rItem.GetValue());

        if ( rItem.GetValue() )
        {
            SetBracket( rItem.GetStartBracket(), true );
            SetBracket( rItem.GetEndBracket(), false );
        }
    }
    TwoLinesHdl_Impl(*m_xTwoLinesBtn);

    SetPrevFontWidthScale( *rSet );
}

bool SvxCharTwoLinesPage::FillItemSet( SfxItemSet* rSet )
{
    const SfxItemSet& rOldSet = GetItemSet();
    bool bModified = false, bChanged = true;
    sal_uInt16 nWhich = GetWhich( SID_ATTR_CHAR_TWO_LINES );
    const SfxPoolItem* pOld = GetOldItem( *rSet, SID_ATTR_CHAR_TWO_LINES );
    bool bOn = m_xTwoLinesBtn->get_active();
    sal_Unicode cStart = ( bOn && m_xStartBracketLB->get_selected_index() > 0 )
        ? m_xStartBracketLB->get_selected_text()[0] : 0;
    sal_Unicode cEnd = ( bOn && m_xEndBracketLB->get_selected_index() > 0 )
        ? m_xEndBracketLB->get_selected_text()[0] : 0;

    if ( pOld )
    {
        const SvxTwoLinesItem& rItem = *static_cast<const SvxTwoLinesItem*>(pOld);
        if ( rItem.GetValue() ==  bOn &&
             ( !bOn || ( rItem.GetStartBracket() == cStart && rItem.GetEndBracket() == cEnd ) ) )
            bChanged = false;
    }

    if ( bChanged )
    {
        rSet->Put( SvxTwoLinesItem( bOn, cStart, cEnd, nWhich ) );
        bModified = true;
    }
    else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich, false ) )
        rSet->InvalidateItem(nWhich);

    return bModified;
}

void    SvxCharTwoLinesPage::UpdatePreview_Impl()
{
    sal_Unicode cStart = m_xStartBracketLB->get_selected_index() > 0
        ? m_xStartBracketLB->get_selected_text()[0] : 0;
    sal_Unicode cEnd = m_xEndBracketLB->get_selected_index() > 0
        ? m_xEndBracketLB->get_selected_text()[0] : 0;
    m_aPreviewWin.SetBrackets(cStart, cEnd);
    m_aPreviewWin.SetTwoLines(m_xTwoLinesBtn->get_active());
    m_aPreviewWin.Invalidate();
}

void SvxCharTwoLinesPage::PageCreated(const SfxAllItemSet& aSet)
{
    const SfxUInt32Item* pFlagItem = aSet.GetItem<SfxUInt32Item>(SID_FLAG_TYPE, false);
    if (pFlagItem)
    {
        sal_uInt32 nFlags=pFlagItem->GetValue();
        if ( ( nFlags & SVX_PREVIEW_CHARACTER ) == SVX_PREVIEW_CHARACTER )
            // the writer uses SID_ATTR_BRUSH as font background
            m_bPreviewBackgroundToCharacter = true;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
