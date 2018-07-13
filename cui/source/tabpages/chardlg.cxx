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

inline bool StateToAttr( TriState aState )
{
    return ( TRISTATE_TRUE == aState );
}

// class SvxCharBasePage -------------------------------------------------

inline SvxFont& SvxCharBasePage::GetPreviewFont()
{
    return m_pPreviewWin->GetFont();
}


inline SvxFont& SvxCharBasePage::GetPreviewCJKFont()
{
    return m_pPreviewWin->GetCJKFont();
}


inline SvxFont& SvxCharBasePage::GetPreviewCTLFont()
{
    return m_pPreviewWin->GetCTLFont();
}


SvxCharBasePage::SvxCharBasePage(vcl::Window* pParent, const OString& rID, const OUString& rUIXMLDescription, const SfxItemSet& rItemset)
    : SfxTabPage( pParent, rID, rUIXMLDescription, &rItemset )
    , m_pPreviewWin(nullptr)
    , m_bPreviewBackgroundToCharacter( false )
{
}

SvxCharBasePage::~SvxCharBasePage()
{
    disposeOnce();
}

void SvxCharBasePage::dispose()
{
    m_pPreviewWin.clear();
    SfxTabPage::dispose();
}

void SvxCharBasePage::ActivatePage( const SfxItemSet& rSet )
{
    m_pPreviewWin->SetFromItemSet( rSet, m_bPreviewBackgroundToCharacter );
}

void SvxCharBasePage::SetPrevFontWidthScale( const SfxItemSet& rSet )
{
    sal_uInt16 nWhich = GetWhich( SID_ATTR_CHAR_SCALEWIDTH );
    if (rSet.GetItemState(nWhich)>=SfxItemState::DEFAULT)
    {
        const SvxCharScaleWidthItem &rItem = static_cast<const SvxCharScaleWidthItem&>( rSet.Get( nWhich ) );
        m_pPreviewWin->SetFontWidthScale( rItem.GetValue() );
    }
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


inline SvxFont& CharBasePage::GetPreviewFont()
{
    return m_aPreviewWin.GetFont();
}

inline SvxFont& CharBasePage::GetPreviewCJKFont()
{
    return m_aPreviewWin.GetCJKFont();
}

inline SvxFont& CharBasePage::GetPreviewCTLFont()
{
    return m_aPreviewWin.GetCTLFont();
}

CharBasePage::CharBasePage(TabPageParent pParent, const OUString& rUIXMLDescription, const OString& rID, const SfxItemSet& rItemset)
    : SfxTabPage(pParent, rUIXMLDescription, rID, &rItemset)
    , m_bPreviewBackgroundToCharacter( false )
{
}

CharBasePage::~CharBasePage()
{
}

void CharBasePage::ActivatePage(const SfxItemSet& rSet)
{
    m_aPreviewWin.SetFromItemSet(rSet, m_bPreviewBackgroundToCharacter);
}

void CharBasePage::SetPrevFontWidthScale( const SfxItemSet& rSet )
{
    sal_uInt16 nWhich = GetWhich( SID_ATTR_CHAR_SCALEWIDTH );
    if (rSet.GetItemState(nWhich)>=SfxItemState::DEFAULT)
    {
        const SvxCharScaleWidthItem &rItem = static_cast<const SvxCharScaleWidthItem&>( rSet.Get( nWhich ) );
        m_aPreviewWin.SetFontWidthScale(rItem.GetValue());
    }
}

void CharBasePage::SetPrevFontEscapement( sal_uInt8 nProp, sal_uInt8 nEscProp, short nEsc )
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
    sal_Int32           m_nExtraEntryPos;
    bool            m_bInSearchMode;

    SvxCharNamePage_Impl() :
        m_nExtraEntryPos( COMBOBOX_ENTRY_NOTFOUND ),
        m_bInSearchMode ( false )

    {
        m_aUpdateIdle.SetPriority( TaskPriority::LOWEST );
    }
};

// class SvxCharNamePage -------------------------------------------------

SvxCharNamePage::SvxCharNamePage( vcl::Window* pParent, const SfxItemSet& rInSet )
    : SvxCharBasePage(pParent, "CharNamePage", "cui/ui/charnamepage.ui", rInSet)
    , m_pImpl(new SvxCharNamePage_Impl)
{
    m_pImpl->m_aNoStyleText = CuiResId( RID_SVXSTR_CHARNAME_NOSTYLE );

    SvtLanguageOptions aLanguageOptions;
    bool bShowCJK = aLanguageOptions.IsCJKFontEnabled();
    bool bShowCTL = aLanguageOptions.IsCTLFontEnabled();
    bool bShowNonWestern = bShowCJK || bShowCTL;

    if (bShowNonWestern)
    {
        get(m_pWestFrame, "western");
        get(m_pWestFontNameFT, "westfontnameft-cjk");
        get(m_pWestFontNameLB, "westfontnamelb-cjk");
        get(m_pWestFontStyleFT, "weststyleft-cjk");
        get(m_pWestFontStyleLB, "weststylelb-cjk");
        get(m_pWestFontSizeFT, "westsizeft-cjk");
        get(m_pWestFontSizeLB, "westsizelb-cjk");

        get(m_pWestFontLanguageFT, "westlangft-cjk");
        get(m_pWestFontLanguageLB, "westlanglb-cjk");
        get(m_pWestFontTypeFT, "westfontinfo-cjk");

        get(m_pWestFontFeaturesButton, "west_features_button-cjk");
    }
    else
    {
        get(m_pWestFrame, "simple");
        get(m_pWestFontNameFT, "westfontnameft-nocjk");
        get(m_pWestFontNameLB, "westfontnamelb-nocjk");
        get(m_pWestFontStyleFT, "weststyleft-nocjk");
        get(m_pWestFontStyleLB, "weststylelb-nocjk");
        get(m_pWestFontSizeFT, "westsizeft-nocjk");
        get(m_pWestFontSizeLB, "westsizelb-nocjk");

        get(m_pWestFontLanguageFT, "westlangft-nocjk");
        get(m_pWestFontLanguageLB, "westlanglb-nocjk");
        get(m_pWestFontTypeFT, "westfontinfo-nocjk");

        get(m_pWestFontFeaturesButton, "west_features_button-nocjk");
    }

    get(m_pEastFrame, "asian");
    get(m_pEastFontNameFT, "eastfontnameft");
    get(m_pEastFontNameLB, "eastfontnamelb");
    get(m_pEastFontStyleFT, "eaststyleft");
    get(m_pEastFontStyleLB, "eaststylelb");
    get(m_pEastFontSizeFT, "eastsizeft");
    get(m_pEastFontSizeLB, "eastsizelb");
    get(m_pEastFontLanguageFT, "eastlangft");
    get(m_pEastFontLanguageLB, "eastlanglb");
    get(m_pEastFontTypeFT, "eastfontinfo");
    get(m_pEastFontFeaturesButton, "east_features_button");

    get(m_pCTLFrame, "ctl");
    get(m_pCTLFontNameFT, "ctlfontnameft");
    get(m_pCTLFontNameLB, "ctlfontnamelb");
    get(m_pCTLFontStyleFT, "ctlstyleft");
    get(m_pCTLFontStyleLB, "ctlstylelb");
    get(m_pCTLFontSizeFT, "ctlsizeft");
    get(m_pCTLFontSizeLB, "ctlsizelb");
    get(m_pCTLFontLanguageFT, "ctllangft");
    get(m_pCTLFontLanguageLB, "ctllanglb");
    get(m_pCTLFontTypeFT, "ctlfontinfo");
    get(m_pCTLFontFeaturesButton, "ctl_features_button");

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
    m_pWestFontNameFT->SetText(sFontFamilyString);
    m_pEastFontNameFT->SetText(sFontFamilyString);
    m_pCTLFontNameFT->SetText(sFontFamilyString);

#ifdef MACOSX
    OUString sFontStyleString(CuiResId(RID_SVXSTR_CHARNAME_TYPEFACE));
#else
    OUString sFontStyleString(CuiResId(RID_SVXSTR_CHARNAME_STYLE));
#endif
    m_pWestFontStyleFT->SetText(sFontStyleString);
    m_pEastFontStyleFT->SetText(sFontStyleString);
    m_pCTLFontStyleFT->SetText(sFontStyleString);

    m_pWestFrame->Show();
    m_pEastFrame->Show(bShowCJK);
    m_pCTLFrame->Show(bShowCTL);

    get(m_pPreviewWin, "preview");

    m_pWestFontLanguageLB->SetLanguageList(SvxLanguageListFlags::WESTERN, true, false, true);
    m_pEastFontLanguageLB->SetLanguageList(SvxLanguageListFlags::CJK, true, false, true);
    m_pCTLFontLanguageLB->SetLanguageList(SvxLanguageListFlags::CTL, true, false, true);

    if (!bShowNonWestern)
    {
        //10 lines
        sal_Int32 nHeight = m_pWestFontSizeLB->CalcWindowSizePixel(10);
        m_pWestFontNameLB->set_height_request(nHeight);
        m_pWestFontStyleLB->set_height_request(nHeight);
        m_pWestFontSizeLB->set_height_request(nHeight);
    }

    Initialize();
}


SvxCharNamePage::~SvxCharNamePage()
{
    disposeOnce();
}

void SvxCharNamePage::dispose()
{
    m_pImpl.reset();
    m_pWestFrame.clear();
    m_pWestFontNameFT.clear();
    m_pWestFontNameLB.clear();
    m_pWestFontStyleFT.clear();
    m_pWestFontStyleLB.clear();
    m_pWestFontSizeFT.clear();
    m_pWestFontSizeLB.clear();
    m_pWestFontLanguageFT.clear();
    m_pWestFontLanguageLB.clear();
    m_pWestFontFeaturesButton.clear();
    m_pWestFontTypeFT.clear();
    m_pEastFrame.clear();
    m_pEastFontNameFT.clear();
    m_pEastFontNameLB.clear();
    m_pEastFontStyleFT.clear();
    m_pEastFontStyleLB.clear();
    m_pEastFontSizeFT.clear();
    m_pEastFontSizeLB.clear();
    m_pEastFontLanguageFT.clear();
    m_pEastFontLanguageLB.clear();
    m_pEastFontTypeFT.clear();
    m_pEastFontFeaturesButton.clear();
    m_pCTLFrame.clear();
    m_pCTLFontNameFT.clear();
    m_pCTLFontNameLB.clear();
    m_pCTLFontStyleFT.clear();
    m_pCTLFontStyleLB.clear();
    m_pCTLFontSizeFT.clear();
    m_pCTLFontSizeLB.clear();
    m_pCTLFontLanguageFT.clear();
    m_pCTLFontLanguageLB.clear();
    m_pCTLFontTypeFT.clear();
    m_pCTLFontFeaturesButton.clear();

    SvxCharBasePage::dispose();
}

void SvxCharNamePage::Initialize()
{
    // to handle the changes of the other pages
    SetExchangeSupport();

    Link<Edit&,void> aLink = LINK( this, SvxCharNamePage, FontModifyEditHdl_Impl );
    m_pWestFontNameLB->SetModifyHdl( aLink );
    m_pWestFontStyleLB->SetModifyHdl( aLink );
    m_pWestFontSizeLB->SetModifyHdl( aLink );
    m_pWestFontLanguageLB->SetSelectHdl( LINK( this, SvxCharNamePage, FontModifyComboBoxHdl_Impl ) );

    m_pWestFontFeaturesButton->SetClickHdl(LINK(this, SvxCharNamePage, FontFeatureButtonClicked));

    m_pEastFontNameLB->SetModifyHdl( aLink );
    m_pEastFontStyleLB->SetModifyHdl( aLink );
    m_pEastFontSizeLB->SetModifyHdl( aLink );
    m_pEastFontLanguageLB->SetSelectHdl( LINK( this, SvxCharNamePage, FontModifyListBoxHdl_Impl ) );
    m_pEastFontFeaturesButton->SetClickHdl(LINK(this, SvxCharNamePage, FontFeatureButtonClicked));

    m_pCTLFontNameLB->SetModifyHdl( aLink );
    m_pCTLFontStyleLB->SetModifyHdl( aLink );
    m_pCTLFontSizeLB->SetModifyHdl( aLink );
    m_pCTLFontLanguageLB->SetSelectHdl( LINK( this, SvxCharNamePage, FontModifyListBoxHdl_Impl ) );
    m_pCTLFontFeaturesButton->SetClickHdl(LINK(this, SvxCharNamePage, FontFeatureButtonClicked));

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
                    const FontNameBox* _pFontNameLB,
                    const FontStyleBox* _pFontStyleLB,
                    const FontSizeBox* _pFontSizeLB,
                    const SvxLanguageBoxBase* _pLanguageLB,
                    const FontList* _pFontList,
                    sal_uInt16 _nFontWhich,
                    sal_uInt16 _nFontHeightWhich)
    {
        Size aSize = _rFont.GetFontSize();
        aSize.setWidth( 0 );
        FontMetric aFontMetrics;
        OUString sFontName(_pFontNameLB->GetText());
        bool bFontAvailable = _pFontList->IsAvailable( sFontName );
        if (bFontAvailable  || _pFontNameLB->IsValueChangedFromSaved())
            aFontMetrics = _pFontList->Get( sFontName, _pFontStyleLB->GetText() );
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
                nHeight = rOldItem.GetHeight() + PointToTwips( static_cast<long>(_pFontSizeLB->GetValue() / 10) );
            else
                nHeight = static_cast<long>(rOldItem.GetHeight() * _pFontSizeLB->GetValue() / 100);

            // conversion twips for the example-window
            aSize.setHeight(
                ItemToControl( nHeight, _pPage->GetItemSet().GetPool()->GetMetric( _nFontHeightWhich ), FUNIT_TWIP ) );
        }
        else if ( !_pFontSizeLB->GetText().isEmpty() )
            aSize.setHeight( PointToTwips( static_cast<long>(_pFontSizeLB->GetValue() / 10) ) );
        else
            aSize.setHeight( 200 );   // default 10pt
        aFontMetrics.SetFontSize( aSize );

        _rFont.SetLanguage(_pLanguageLB->GetSelectedLanguage());

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

    FontMetric aWestFontMetric = calcFontMetrics(rFont, this, m_pWestFontNameLB,
        m_pWestFontStyleLB, m_pWestFontSizeLB, m_pWestFontLanguageLB,
        pFontList, GetWhich(SID_ATTR_CHAR_FONT),
        GetWhich(SID_ATTR_CHAR_FONTHEIGHT));

    m_pWestFontTypeFT->SetText(pFontList->GetFontMapText(aWestFontMetric));

    FontMetric aEastFontMetric = calcFontMetrics(rCJKFont, this, m_pEastFontNameLB,
        m_pEastFontStyleLB, m_pEastFontSizeLB, m_pEastFontLanguageLB,
        pFontList, GetWhich(SID_ATTR_CHAR_CJK_FONT),
        GetWhich(SID_ATTR_CHAR_CJK_FONTHEIGHT));

    m_pEastFontTypeFT->SetText(pFontList->GetFontMapText(aEastFontMetric));

    FontMetric aCTLFontMetric = calcFontMetrics(rCTLFont,
        this, m_pCTLFontNameLB, m_pCTLFontStyleLB, m_pCTLFontSizeLB,
        m_pCTLFontLanguageLB, pFontList, GetWhich(SID_ATTR_CHAR_CTL_FONT),
        GetWhich(SID_ATTR_CHAR_CTL_FONTHEIGHT));

    m_pCTLFontTypeFT->SetText(pFontList->GetFontMapText(aCTLFontMetric));

    m_pPreviewWin->Invalidate();
}


void SvxCharNamePage::FillStyleBox_Impl( const FontNameBox* pNameBox )
{
    const FontList* pFontList = GetFontList();
    DBG_ASSERT( pFontList, "no fontlist" );

    FontStyleBox* pStyleBox = nullptr;

    if ( m_pWestFontNameLB == pNameBox )
        pStyleBox = m_pWestFontStyleLB;
    else if ( m_pEastFontNameLB == pNameBox )
        pStyleBox = m_pEastFontStyleLB;
    else if ( m_pCTLFontNameLB == pNameBox )
        pStyleBox = m_pCTLFontStyleLB;
    else
    {
        SAL_WARN( "cui.tabpages", "invalid font name box" );
        return;
    }

    pStyleBox->Fill( pNameBox->GetText(), pFontList );

    if ( m_pImpl->m_bInSearchMode )
    {
        // additional entries for the search:
        // "not bold" and "not italic"
        OUString aEntry = m_pImpl->m_aNoStyleText;
        const sal_Char sS[] = "%1";
        aEntry = aEntry.replaceFirst( sS, pFontList->GetBoldStr() );
        m_pImpl->m_nExtraEntryPos = pStyleBox->InsertEntry( aEntry );
        aEntry = m_pImpl->m_aNoStyleText;
        aEntry = aEntry.replaceFirst( sS, pFontList->GetItalicStr() );
        pStyleBox->InsertEntry( aEntry );
    }
}


void SvxCharNamePage::FillSizeBox_Impl( const FontNameBox* pNameBox )
{
    const FontList* pFontList = GetFontList();
    DBG_ASSERT( pFontList, "no fontlist" );

    FontStyleBox* pStyleBox = nullptr;
    FontSizeBox* pSizeBox = nullptr;

    if ( m_pWestFontNameLB == pNameBox )
    {
        pStyleBox = m_pWestFontStyleLB;
        pSizeBox = m_pWestFontSizeLB;
    }
    else if ( m_pEastFontNameLB == pNameBox )
    {
        pStyleBox = m_pEastFontStyleLB;
        pSizeBox = m_pEastFontSizeLB;
    }
    else if ( m_pCTLFontNameLB == pNameBox )
    {
        pStyleBox = m_pCTLFontStyleLB;
        pSizeBox = m_pCTLFontSizeLB;
    }
    else
    {
        SAL_WARN( "cui.tabpages", "invalid font name box" );
        return;
    }

    FontMetric _aFontMetric( pFontList->Get( pNameBox->GetText(), pStyleBox->GetText() ) );
    pSizeBox->Fill( &_aFontMetric, pFontList );
}


void SvxCharNamePage::Reset_Impl( const SfxItemSet& rSet, LanguageGroup eLangGrp )
{
    FontNameBox* pNameBox = nullptr;
    FixedText* pStyleLabel = nullptr;
    FontStyleBox* pStyleBox = nullptr;
    FixedText* pSizeLabel = nullptr;
    FontSizeBox* pSizeBox = nullptr;
    FixedText* pLangFT = nullptr;
    SvxLanguageBoxBase* pLangBox = nullptr;
    sal_uInt16 nWhich = 0;

    switch ( eLangGrp )
    {
        case Western :
            pNameBox = m_pWestFontNameLB;
            pStyleLabel = m_pWestFontStyleFT;
            pStyleBox = m_pWestFontStyleLB;
            pSizeLabel = m_pWestFontSizeFT;
            pSizeBox = m_pWestFontSizeLB;
            pLangFT = m_pWestFontLanguageFT;
            pLangBox = m_pWestFontLanguageLB;
            nWhich = GetWhich( SID_ATTR_CHAR_FONT );
            break;

        case Asian :
            pNameBox = m_pEastFontNameLB;
            pStyleLabel = m_pEastFontStyleFT;
            pStyleBox = m_pEastFontStyleLB;
            pSizeLabel = m_pEastFontSizeFT;
            pSizeBox = m_pEastFontSizeLB;
            pLangFT = m_pEastFontLanguageFT;
            pLangBox = m_pEastFontLanguageLB;
            nWhich = GetWhich( SID_ATTR_CHAR_CJK_FONT );
            break;

        case Ctl :
            pNameBox = m_pCTLFontNameLB;
            pStyleLabel = m_pCTLFontStyleFT;
            pStyleBox = m_pCTLFontStyleLB;
            pSizeLabel = m_pCTLFontSizeFT;
            pSizeBox = m_pCTLFontSizeLB;
            pLangFT = m_pCTLFontLanguageFT;
            pLangBox = m_pCTLFontLanguageLB;
            nWhich = GetWhich( SID_ATTR_CHAR_CTL_FONT );
            break;
    }

    const FontList* pFontList = GetFontList();
    pNameBox->Fill( pFontList );

    const SvxFontItem* pFontItem = nullptr;
    SfxItemState eState = rSet.GetItemState( nWhich );

    if ( eState >= SfxItemState::DEFAULT )
    {
        pFontItem = static_cast<const SvxFontItem*>(&( rSet.Get( nWhich ) ));
        pNameBox->SetText( pFontItem->GetFamilyName() );
    }
    else
    {
        pNameBox->SetText( OUString() );
    }

    FillStyleBox_Impl( pNameBox );

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
        pStyleBox->SetText( pFontList->GetStyleName( aFontMetric ) );
    }
    else if ( !m_pImpl->m_bInSearchMode || !bStyle )
    {
        pStyleBox->SetText( OUString() );
    }
    else if ( bStyle )
    {
        FontMetric aFontMetric = pFontList->Get( OUString(), eWeight, eItalic );
        pStyleBox->SetText( pFontList->GetStyleName( aFontMetric ) );
    }
    if (!bStyleAvailable)
    {
        pStyleBox->Disable( );
        pStyleLabel->Disable( );
    }

    FillSizeBox_Impl( pNameBox );
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
            pSizeBox->SetValue( bPtRel ? static_cast<short>(rItem.GetProp()) * 10 : rItem.GetProp() );
        }
        else
        {
            pSizeBox->SetRelative(false);
            pSizeBox->SetValue( CalcToPoint( rItem.GetHeight(), eUnit, 10 ) );
        }
    }
    else if ( eState >= SfxItemState::DEFAULT )
    {
        MapUnit eUnit = rSet.GetPool()->GetMetric( nWhich );
        const SvxFontHeightItem& rItem = static_cast<const SvxFontHeightItem&>(rSet.Get( nWhich ));
        pSizeBox->SetValue( CalcToPoint( rItem.GetHeight(), eUnit, 10 ) );
    }
    else
    {
        pSizeBox->SetText( OUString() );
        if ( eState <= SfxItemState::READONLY )
        {
            pSizeBox->Disable( );
            pSizeLabel->Disable( );
        }
    }

    switch ( eLangGrp )
    {
        case Western : nWhich = GetWhich( SID_ATTR_CHAR_LANGUAGE ); break;
        case Asian : nWhich = GetWhich( SID_ATTR_CHAR_CJK_LANGUAGE ); break;
        case Ctl : nWhich = GetWhich( SID_ATTR_CHAR_CTL_LANGUAGE ); break;
    }
    pLangBox->SetNoSelectionLBB();
    eState = rSet.GetItemState( nWhich );

    switch ( eState )
    {
        case SfxItemState::UNKNOWN:
            pLangFT->Hide();
            pLangBox->HideLBB();
            break;

        case SfxItemState::DISABLED:
        case SfxItemState::READONLY:
            pLangFT->Disable();
            pLangBox->DisableLBB();
            break;

        case SfxItemState::DEFAULT:
        case SfxItemState::SET:
        {
            const SvxLanguageItem& rItem = static_cast<const SvxLanguageItem&>(rSet.Get( nWhich ));
            LanguageType eLangType = rItem.GetValue();
            DBG_ASSERT( eLangType != LANGUAGE_SYSTEM, "LANGUAGE_SYSTEM not allowed" );
            if ( eLangType != LANGUAGE_DONTKNOW )
                pLangBox->SelectLanguage( eLangType );
            break;
        }
        case SfxItemState::DONTCARE:
            break;
    }

    OUString sMapText(pFontList->GetFontMapText(
        pFontList->Get(pNameBox->GetText(), pStyleBox->GetText())));

    switch (eLangGrp)
    {
        case Western:
            m_pWestFontTypeFT->SetText(sMapText);
            break;
        case Asian:
            m_pEastFontTypeFT->SetText(sMapText);
            break;
        case Ctl:
            m_pCTLFontTypeFT->SetText(sMapText);
            break;
    }

    // save these settings
    pNameBox->SaveValue();
    pStyleBox->SaveValue();
    pSizeBox->SaveValue();
    pLangBox->SaveValueLBB();
}

bool SvxCharNamePage::FillItemSet_Impl( SfxItemSet& rSet, LanguageGroup eLangGrp )
{
    bool bModified = false;

    FontNameBox* pNameBox = nullptr;
    FontStyleBox* pStyleBox = nullptr;
    FontSizeBox* pSizeBox = nullptr;
    SvxLanguageBoxBase* pLangBox = nullptr;
    sal_uInt16 nWhich = 0;
    sal_uInt16 nSlot = 0;

    switch ( eLangGrp )
    {
        case Western :
            pNameBox = m_pWestFontNameLB;
            pStyleBox = m_pWestFontStyleLB;
            pSizeBox = m_pWestFontSizeLB;
            pLangBox = m_pWestFontLanguageLB;
            nSlot = SID_ATTR_CHAR_FONT;
            break;

        case Asian :
            pNameBox = m_pEastFontNameLB;
            pStyleBox = m_pEastFontStyleLB;
            pSizeBox = m_pEastFontSizeLB;
            pLangBox = m_pEastFontLanguageLB;
            nSlot = SID_ATTR_CHAR_CJK_FONT;
            break;

        case Ctl :
            pNameBox = m_pCTLFontNameLB;
            pStyleBox = m_pCTLFontStyleLB;
            pSizeBox = m_pCTLFontSizeLB;
            pLangBox = m_pCTLFontLanguageLB;
            nSlot = SID_ATTR_CHAR_CTL_FONT;
            break;
    }

    nWhich = GetWhich( nSlot );
    const SfxPoolItem* pItem = nullptr;
    const SfxItemSet& rOldSet = GetItemSet();
    const SfxPoolItem* pOld = nullptr;

    const SfxItemSet* pExampleSet = GetTabDialog() ? GetTabDialog()->GetExampleSet() : nullptr;

    bool bChanged = true;
    const OUString& rFontName  = pNameBox->GetText();
    const FontList* pFontList = GetFontList();
    OUString aStyleBoxText =pStyleBox->GetText();
    sal_Int32 nEntryPos = pStyleBox->GetEntryPos( aStyleBoxText );
    if ( nEntryPos >= m_pImpl->m_nExtraEntryPos )
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
        bChanged = pNameBox->GetSavedValue().isEmpty();

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
        bChanged = pStyleBox->GetSavedValue().isEmpty();

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

    OUString aText( pStyleBox->GetText() ); // Tristate, then text empty

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
        bChanged = pStyleBox->GetSavedValue().isEmpty();

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
    long nSize = static_cast<long>(pSizeBox->GetValue());

    if ( pSizeBox->GetText().isEmpty() )   // GetValue() returns the min-value
        nSize = 0;
    long nSavedSize = static_cast<long>(pSizeBox->GetSavedIntValue());
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
    SvxLanguageComboBox* pLangComboBox = dynamic_cast<SvxLanguageComboBox*>(pLangBox);
    if (pLangComboBox)
    {
        switch (pLangComboBox->GetEditedAndValid())
        {
            case SvxLanguageComboBox::EditedAndValid::No:
                ;   // nothing to do
                break;
            case SvxLanguageComboBox::EditedAndValid::Valid:
                {
                    const sal_Int32 nPos = pLangComboBox->SaveEditedAsEntry();
                    if (nPos != COMBOBOX_ENTRY_NOTFOUND)
                        pLangComboBox->SelectEntryPos( nPos);
                }
                break;
            case SvxLanguageComboBox::EditedAndValid::Invalid:
                pLangComboBox->SelectEntryPos( pLangComboBox->GetSavedValueLBB());
                break;
        }
    }

    sal_Int32 nLangPos = pLangBox->GetSelectedEntryPosLBB();
    LanguageType eLangType = LanguageType(reinterpret_cast<sal_uLong>(pLangBox->GetEntryDataLBB( nLangPos )));

    if ( pOld )
    {
        const SvxLanguageItem& rItem = *static_cast<const SvxLanguageItem*>(pOld);

        if ( nLangPos == LISTBOX_ENTRY_NOTFOUND || eLangType == rItem.GetValue() )
            bChanged = false;
    }

    if ( !bChanged )
        bChanged = ( pLangBox->GetSavedValueLBB() == LISTBOX_ENTRY_NOTFOUND );

    if ( bChanged && nLangPos != LISTBOX_ENTRY_NOTFOUND )
    {
        rSet.Put( SvxLanguageItem( eLangType, nWhich ) );
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


IMPL_LINK( SvxCharNamePage, FontModifyComboBoxHdl_Impl, ComboBox&, rBox, void )
{
    FontModifyHdl_Impl(&rBox);
}
IMPL_LINK( SvxCharNamePage, FontModifyListBoxHdl_Impl, ListBox&, rBox, void )
{
    FontModifyHdl_Impl(&rBox);
}
IMPL_LINK( SvxCharNamePage, FontModifyEditHdl_Impl, Edit&, rBox, void )
{
    FontModifyHdl_Impl(&rBox);
}
IMPL_LINK(SvxCharNamePage, FontFeatureButtonClicked, Button*, pButton, void )
{
    OUString sFontName;
    FontNameBox * pNameBox = nullptr;

    if (pButton == m_pWestFontFeaturesButton.get())
    {
        pNameBox = m_pWestFontNameLB;
        sFontName = GetPreviewFont().GetFamilyName();
    }
    else if (pButton == m_pEastFontFeaturesButton.get())
    {
        pNameBox = m_pEastFontNameLB;
        sFontName = GetPreviewCJKFont().GetFamilyName();
    }
    else if (pButton == m_pCTLFontFeaturesButton.get())
    {
        pNameBox = m_pCTLFontNameLB;
        sFontName = GetPreviewCTLFont().GetFamilyName();
    }

    if (!sFontName.isEmpty() && pNameBox)
    {
        ScopedVclPtrInstance<cui::FontFeaturesDialog> pDialog(this, sFontName);
        if (pDialog->Execute() == RET_OK)
        {
            pNameBox->SetText(pDialog->getResultFontName());
            UpdatePreview_Impl();
        }
    }
}

void SvxCharNamePage::FontModifyHdl_Impl(void const * pNameBox)
{
    m_pImpl->m_aUpdateIdle.Start();

    if ( m_pWestFontNameLB == pNameBox || m_pEastFontNameLB == pNameBox || m_pCTLFontNameLB == pNameBox )
    {
        FillStyleBox_Impl( static_cast<FontNameBox const *>(pNameBox) );
        FillSizeBox_Impl( static_cast<FontNameBox const *>(pNameBox) );
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


VclPtr<SfxTabPage> SvxCharNamePage::Create( TabPageParent pParent, const SfxItemSet* rSet )
{
    return VclPtr<SvxCharNamePage>::Create( pParent.pParent, *rSet );
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
    m_pWestFontNameLB->SaveValue();
    m_pWestFontStyleLB->SaveValue();
    m_pWestFontSizeLB->SaveValue();
    m_pWestFontLanguageLB->SaveValueLBB();
    m_pEastFontNameLB->SaveValue();
    m_pEastFontStyleLB->SaveValue();
    m_pEastFontSizeLB->SaveValue();
    m_pEastFontLanguageLB->SaveValueLBB();
    m_pCTLFontNameLB->SaveValue();
    m_pCTLFontStyleLB->SaveValue();
    m_pCTLFontSizeLB->SaveValue();
    m_pCTLFontLanguageLB->SaveValueLBB();
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
    void enableRelativeMode( SvxCharNamePage const * _pPage, FontSizeBox* _pFontSizeLB, sal_uInt16 _nHeightWhich )
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
    enableRelativeMode(this,m_pWestFontSizeLB,GetWhich( SID_ATTR_CHAR_FONTHEIGHT ));
    enableRelativeMode(this,m_pEastFontSizeLB,GetWhich( SID_ATTR_CHAR_CJK_FONTHEIGHT ));
    enableRelativeMode(this,m_pCTLFontSizeLB,GetWhich( SID_ATTR_CHAR_CTL_FONTHEIGHT ));
}


void SvxCharNamePage::EnableSearchMode()
{
    m_pImpl->m_bInSearchMode = true;
}

void SvxCharNamePage::DisableControls( sal_uInt16 nDisable )
{
    if ( DISABLE_LANGUAGE & nDisable )
    {
        if ( m_pWestFontLanguageFT ) m_pWestFontLanguageFT->Disable();
        if ( m_pWestFontLanguageLB ) m_pWestFontLanguageLB->Disable();
        if ( m_pEastFontLanguageFT ) m_pEastFontLanguageFT->Disable();
        if ( m_pEastFontLanguageLB ) m_pEastFontLanguageLB->Disable();
        if ( m_pCTLFontLanguageFT ) m_pCTLFontLanguageFT->Disable();
        if ( m_pCTLFontLanguageLB ) m_pCTLFontLanguageLB->Disable();
    }

    if ( DISABLE_HIDE_LANGUAGE & nDisable )
    {
        if ( m_pWestFontLanguageFT ) m_pWestFontLanguageFT->Hide();
        if ( m_pWestFontLanguageLB ) m_pWestFontLanguageLB->Hide();
        if ( m_pEastFontLanguageFT ) m_pEastFontLanguageFT->Hide();
        if ( m_pEastFontLanguageLB ) m_pEastFontLanguageLB->Hide();
        if ( m_pCTLFontLanguageFT ) m_pCTLFontLanguageFT->Hide();
        if ( m_pCTLFontLanguageLB ) m_pCTLFontLanguageLB->Hide();
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

SvxCharEffectsPage::SvxCharEffectsPage( vcl::Window* pParent, const SfxItemSet& rInSet )
    : SvxCharBasePage(pParent, "EffectsPage", "cui/ui/effectspage.ui", rInSet)
    , m_bOrigFontColor(false)
    , m_bNewFontColor(false)
    , m_bEnableNoneFontColor(false)
{
    get(m_pFontColorFT, "fontcolorft");
    get(m_pFontColorLB, "fontcolorlb");
    m_pFontColorLB->SetSlotId(SID_ATTR_CHAR_COLOR);
    get(m_pEffectsFT, "effectsft");
    get(m_pEffectsLB, "effectslb");
    get(m_pReliefFT, "reliefft");
    get(m_pReliefLB, "relieflb");
    get(m_pOutlineBtn, "outlinecb");
    get(m_pShadowBtn, "shadowcb");
    get(m_pBlinkingBtn, "blinkingcb");
    get(m_pHiddenBtn, "hiddencb");
    get(m_pOverlineLB, "overlinelb");
    get(m_pOverlineColorFT, "overlinecolorft");
    get(m_pOverlineColorLB, "overlinecolorlb");
    m_pOverlineColorLB->SetSlotId(SID_ATTR_CHAR_COLOR);
    get(m_pStrikeoutLB, "strikeoutlb");
    get(m_pUnderlineLB, "underlinelb");
    get(m_pUnderlineColorFT, "underlinecolorft");
    get(m_pUnderlineColorLB, "underlinecolorlb");
    m_pUnderlineColorLB->SetSlotId(SID_ATTR_CHAR_COLOR);
    get(m_pIndividualWordsBtn, "individualwordscb");
    get(m_pEmphasisFT, "emphasisft");
    get(m_pEmphasisLB, "emphasislb");
    get(m_pPositionFT, "positionft");
    get(m_pPositionLB, "positionlb");
    get(m_pA11yWarningFT, "a11ywarning");

    get(m_pPreviewWin, "preview");
    Initialize();
}

void SvxCharEffectsPage::EnableNoneFontColor()
{
    m_pFontColorLB->SetSlotId(SID_ATTR_CHAR_COLOR, true);
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
    m_pFontColorFT.clear();
    m_pFontColorLB.clear();
    m_pEffectsFT.clear();
    m_pEffectsLB.clear();
    m_pReliefFT.clear();
    m_pReliefLB.clear();
    m_pOutlineBtn.clear();
    m_pShadowBtn.clear();
    m_pBlinkingBtn.clear();
    m_pHiddenBtn.clear();
    m_pOverlineLB.clear();
    m_pOverlineColorFT.clear();
    m_pOverlineColorLB.clear();
    m_pStrikeoutLB.clear();
    m_pUnderlineLB.clear();
    m_pUnderlineColorFT.clear();
    m_pUnderlineColorLB.clear();
    m_pIndividualWordsBtn.clear();
    m_pEmphasisFT.clear();
    m_pEmphasisLB.clear();
    m_pPositionFT.clear();
    m_pPositionLB.clear();
    m_pA11yWarningFT.clear();
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

    m_pFontColorLB->SetSelectHdl(LINK(this, SvxCharEffectsPage, ColorBoxSelectHdl_Impl));

    // handler
    Link<ListBox&,void> aLink = LINK( this, SvxCharEffectsPage, SelectListBoxHdl_Impl );
    m_pUnderlineLB->SetSelectHdl( aLink );
    m_pUnderlineColorLB->SetSelectHdl(LINK(this, SvxCharEffectsPage, ColorBoxSelectHdl_Impl));
    m_pOverlineLB->SetSelectHdl( aLink );
    m_pOverlineColorLB->SetSelectHdl(LINK(this, SvxCharEffectsPage, ColorBoxSelectHdl_Impl));
    m_pStrikeoutLB->SetSelectHdl( aLink );
    m_pEmphasisLB->SetSelectHdl( aLink );
    m_pPositionLB->SetSelectHdl( aLink );
    m_pEffectsLB->SetSelectHdl( aLink );
    m_pReliefLB->SetSelectHdl( aLink );

    m_pUnderlineLB->SelectEntryPos( 0 );
    m_pOverlineLB->SelectEntryPos( 0 );
    m_pStrikeoutLB->SelectEntryPos( 0 );
    m_pEmphasisLB->SelectEntryPos( 0 );
    m_pPositionLB->SelectEntryPos( 0 );
    SelectHdl_Impl( nullptr );
    SelectHdl_Impl( m_pEmphasisLB );

    m_pEffectsLB->SelectEntryPos( 0 );

    m_pIndividualWordsBtn->SetClickHdl( LINK( this, SvxCharEffectsPage, CbClickHdl_Impl ) );
    Link<Button*,void> aLink2 = LINK( this, SvxCharEffectsPage, TristClickHdl_Impl );
    m_pOutlineBtn->SetClickHdl( aLink2 );
    m_pShadowBtn->SetClickHdl( aLink2 );

    if ( !SvtLanguageOptions().IsAsianTypographyEnabled() )
    {
        m_pEmphasisFT->Hide();
        m_pEmphasisLB->Hide();
        m_pPositionFT->Hide();
        m_pPositionLB->Hide();
    }

    m_pA11yWarningFT->Show(officecfg::Office::Common::Accessibility::IsAutomaticFontColor::get());
}

void SvxCharEffectsPage::UpdatePreview_Impl()
{
    SvxFont& rFont = GetPreviewFont();
    SvxFont& rCJKFont = GetPreviewCJKFont();
    SvxFont& rCTLFont = GetPreviewCTLFont();

    const Color& rSelectedColor = m_pFontColorLB->GetSelectEntryColor();
    rFont.SetColor(GetPreviewFontColor(rSelectedColor));
    rCJKFont.SetColor(GetPreviewFontColor(rSelectedColor));
    rCTLFont.SetColor(GetPreviewFontColor(rSelectedColor));

    sal_Int32 nPos = m_pUnderlineLB->GetSelectedEntryPos();
    FontLineStyle eUnderline = static_cast<FontLineStyle>(reinterpret_cast<sal_uLong>(m_pUnderlineLB->GetEntryData( nPos )));
    nPos = m_pOverlineLB->GetSelectedEntryPos();
    FontLineStyle eOverline = static_cast<FontLineStyle>(reinterpret_cast<sal_uLong>(m_pOverlineLB->GetEntryData( nPos )));
    nPos = m_pStrikeoutLB->GetSelectedEntryPos();
    FontStrikeout eStrikeout = static_cast<FontStrikeout>(reinterpret_cast<sal_uLong>(m_pStrikeoutLB->GetEntryData( nPos )));
    rFont.SetUnderline( eUnderline );
    rCJKFont.SetUnderline( eUnderline );
    rCTLFont.SetUnderline( eUnderline );
    m_pPreviewWin->SetTextLineColor( m_pUnderlineColorLB->GetSelectEntryColor() );
    rFont.SetOverline( eOverline );
    rCJKFont.SetOverline( eOverline );
    rCTLFont.SetOverline( eOverline );
    m_pPreviewWin->SetOverlineColor( m_pOverlineColorLB->GetSelectEntryColor() );
    rFont.SetStrikeout( eStrikeout );
    rCJKFont.SetStrikeout( eStrikeout );
    rCTLFont.SetStrikeout( eStrikeout );

    nPos = m_pPositionLB->GetSelectedEntryPos();
    sal_Int32 nEmphasis = m_pEmphasisLB->GetSelectedEntryPos();
    if (nEmphasis != LISTBOX_ENTRY_NOTFOUND)
    {
        bool bUnder = ( CHRDLG_POSITION_UNDER == reinterpret_cast<sal_uLong>(m_pPositionLB->GetEntryData( nPos )) );
        FontEmphasisMark eMark = static_cast<FontEmphasisMark>(nEmphasis);
        eMark |= bUnder ? FontEmphasisMark::PosBelow : FontEmphasisMark::PosAbove;
        rFont.SetEmphasisMark( eMark );
        rCJKFont.SetEmphasisMark( eMark );
        rCTLFont.SetEmphasisMark( eMark );
    }

    sal_Int32 nRelief = m_pReliefLB->GetSelectedEntryPos();
    if (LISTBOX_ENTRY_NOTFOUND != nRelief)
    {
        rFont.SetRelief( static_cast<FontRelief>(nRelief) );
        rCJKFont.SetRelief( static_cast<FontRelief>(nRelief) );
        rCTLFont.SetRelief( static_cast<FontRelief>(nRelief) );
    }

    rFont.SetOutline( StateToAttr( m_pOutlineBtn->GetState() ) );
    rCJKFont.SetOutline( rFont.IsOutline() );
    rCTLFont.SetOutline( rFont.IsOutline() );

    rFont.SetShadow( StateToAttr( m_pShadowBtn->GetState() ) );
    rCJKFont.SetShadow( rFont.IsShadow() );
    rCTLFont.SetShadow( rFont.IsShadow() );

    sal_Int32 nCapsPos = m_pEffectsLB->GetSelectedEntryPos();
    if ( nCapsPos != LISTBOX_ENTRY_NOTFOUND )
    {
        SvxCaseMap eCaps = static_cast<SvxCaseMap>(nCapsPos);
        rFont.SetCaseMap( eCaps );
        rCJKFont.SetCaseMap( eCaps );
        // #i78474# small caps do not exist in CTL fonts
        rCTLFont.SetCaseMap( eCaps == SvxCaseMap::SmallCaps ? SvxCaseMap::NotMapped : eCaps );
    }

    bool bWordLine = m_pIndividualWordsBtn->IsChecked();
    rFont.SetWordLineMode( bWordLine );
    rCJKFont.SetWordLineMode( bWordLine );
    rCTLFont.SetWordLineMode( bWordLine );

    m_pPreviewWin->Invalidate();
}


void SvxCharEffectsPage::SetCaseMap_Impl( SvxCaseMap eCaseMap )
{
    if ( SvxCaseMap::End > eCaseMap )
        m_pEffectsLB->SelectEntryPos(
            sal::static_int_cast< sal_Int32 >( eCaseMap ) );
    else
    {
        // not mapped
        m_pEffectsLB->SetNoSelection();
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
            m_pFontColorFT->Hide();
            m_pFontColorLB->Hide();
            break;

        case SfxItemState::DISABLED:
        case SfxItemState::READONLY:
            m_pFontColorFT->Disable();
            m_pFontColorLB->Disable();
            break;

        case SfxItemState::DONTCARE:
            //Related: tdf#106080 if there is no font color, then allow "none"
            //as a color so the listbox can display that state.
            EnableNoneFontColor();
            m_pFontColorLB->SetNoSelection();
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

            m_pPreviewWin->Invalidate();

            m_pFontColorLB->SelectEntry(aColor);

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
        aSelectedColor = m_pFontColorLB->GetSelectEntryColor();
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

IMPL_LINK( SvxCharEffectsPage, SelectListBoxHdl_Impl, ListBox&, rBox, void )
{
    SelectHdl_Impl(&rBox);
}

void SvxCharEffectsPage::SelectHdl_Impl( ListBox* pBox )
{
    if ( m_pEmphasisLB == pBox )
    {
        sal_Int32 nEPos = m_pEmphasisLB->GetSelectedEntryPos();
        bool bEnable = ( nEPos > 0 && nEPos != LISTBOX_ENTRY_NOTFOUND );
        m_pPositionFT->Enable( bEnable );
        m_pPositionLB->Enable( bEnable );
    }
    else if( m_pReliefLB == pBox)
    {
        bool bEnable = ( pBox->GetSelectedEntryPos() == 0 );
        m_pOutlineBtn->Enable( bEnable );
        m_pShadowBtn->Enable( bEnable );
    }
    else if ( m_pPositionLB.get() != pBox )
    {
        sal_Int32 nUPos = m_pUnderlineLB->GetSelectedEntryPos(),
               nOPos = m_pOverlineLB->GetSelectedEntryPos(),
               nSPos = m_pStrikeoutLB->GetSelectedEntryPos();
        bool bUEnable = ( nUPos > 0 && nUPos != LISTBOX_ENTRY_NOTFOUND );
        bool bOEnable = ( nOPos > 0 && nOPos != LISTBOX_ENTRY_NOTFOUND );
        m_pUnderlineColorFT->Enable( bUEnable );
        m_pUnderlineColorLB->Enable( bUEnable );
        m_pOverlineColorFT->Enable( bOEnable );
        m_pOverlineColorLB->Enable( bOEnable );
        m_pIndividualWordsBtn->Enable( bUEnable || bOEnable || ( nSPos > 0 && nSPos != LISTBOX_ENTRY_NOTFOUND ) );
    }
    UpdatePreview_Impl();
}


IMPL_LINK_NOARG(SvxCharEffectsPage, UpdatePreview_Impl, ListBox&, void)
{
    bool bEnable = ( ( m_pUnderlineLB->GetSelectedEntryPos() > 0 ) ||
                     ( m_pOverlineLB->GetSelectedEntryPos()  > 0 ) ||
                     ( m_pStrikeoutLB->GetSelectedEntryPos() > 0 ) );
    m_pIndividualWordsBtn->Enable( bEnable );

    UpdatePreview_Impl();
}


IMPL_LINK_NOARG(SvxCharEffectsPage, CbClickHdl_Impl, Button*, void)
{
    UpdatePreview_Impl();
}


IMPL_LINK_NOARG(SvxCharEffectsPage, TristClickHdl_Impl, Button*, void)
{
    UpdatePreview_Impl();
}


IMPL_LINK(SvxCharEffectsPage, ColorBoxSelectHdl_Impl, SvxColorListBox&, rBox, void)
{
    if (m_pFontColorLB == &rBox)
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
    return VclPtr<SvxCharEffectsPage>::Create( pParent.pParent, *rSet );
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

    m_pUnderlineLB->SelectEntryPos( 0 );
    SfxItemState eState = rSet->GetItemState( nWhich );

    if ( eState >= SfxItemState::DONTCARE )
    {
        if ( eState == SfxItemState::DONTCARE )
            m_pUnderlineLB->SetNoSelection();
        else
        {
            const SvxUnderlineItem& rItem = static_cast<const SvxUnderlineItem&>(rSet->Get( nWhich ));
            FontLineStyle eUnderline = rItem.GetValue();
            rFont.SetUnderline( eUnderline );
            rCJKFont.SetUnderline( eUnderline );
            rCTLFont.SetUnderline( eUnderline );

            if ( eUnderline != LINESTYLE_NONE )
            {
                for ( sal_Int32 i = 0; i < m_pUnderlineLB->GetEntryCount(); ++i )
                {
                    if ( static_cast<FontLineStyle>(reinterpret_cast<sal_uLong>(m_pUnderlineLB->GetEntryData(i))) == eUnderline )
                    {
                        m_pUnderlineLB->SelectEntryPos(i);
                        bEnable = true;
                        break;
                    }
                }

                Color aColor = rItem.GetColor();
                m_pUnderlineColorLB->SelectEntry(aColor);
            }
            else
            {
                m_pUnderlineColorLB->SelectEntry(COL_AUTO);
                m_pUnderlineColorLB->Disable();
            }
        }
    }

    // Overline
    nWhich = GetWhich( SID_ATTR_CHAR_OVERLINE );
    rFont.SetOverline( LINESTYLE_NONE );
    rCJKFont.SetOverline( LINESTYLE_NONE );
    rCTLFont.SetOverline( LINESTYLE_NONE );

    m_pOverlineLB->SelectEntryPos( 0 );
    eState = rSet->GetItemState( nWhich );

    if ( eState >= SfxItemState::DONTCARE )
    {
        if ( eState == SfxItemState::DONTCARE )
            m_pOverlineLB->SetNoSelection();
        else
        {
            const SvxOverlineItem& rItem = static_cast<const SvxOverlineItem&>(rSet->Get( nWhich ));
            FontLineStyle eOverline = rItem.GetValue();
            rFont.SetOverline( eOverline );
            rCJKFont.SetOverline( eOverline );
            rCTLFont.SetOverline( eOverline );

            if ( eOverline != LINESTYLE_NONE )
            {
                for ( sal_Int32 i = 0; i < m_pOverlineLB->GetEntryCount(); ++i )
                {
                    if ( static_cast<FontLineStyle>(reinterpret_cast<sal_uLong>(m_pOverlineLB->GetEntryData(i))) == eOverline )
                    {
                        m_pOverlineLB->SelectEntryPos(i);
                        bEnable = true;
                        break;
                    }
                }

                Color aColor = rItem.GetColor();
                m_pOverlineColorLB->SelectEntry(aColor);
            }
            else
            {
                m_pOverlineColorLB->SelectEntry(COL_AUTO);
                m_pOverlineColorLB->Disable();
            }
        }
    }

    //  Strikeout
    nWhich = GetWhich( SID_ATTR_CHAR_STRIKEOUT );
    rFont.SetStrikeout( STRIKEOUT_NONE );
    rCJKFont.SetStrikeout( STRIKEOUT_NONE );
    rCTLFont.SetStrikeout( STRIKEOUT_NONE );

    m_pStrikeoutLB->SelectEntryPos( 0 );
    eState = rSet->GetItemState( nWhich );

    if ( eState >= SfxItemState::DONTCARE )
    {
        if ( eState == SfxItemState::DONTCARE )
            m_pStrikeoutLB->SetNoSelection();
        else
        {
            const SvxCrossedOutItem& rItem = static_cast<const SvxCrossedOutItem&>(rSet->Get( nWhich ));
            FontStrikeout eStrikeout = rItem.GetValue();
            rFont.SetStrikeout( eStrikeout );
            rCJKFont.SetStrikeout( eStrikeout );
            rCTLFont.SetStrikeout( eStrikeout );

            if ( eStrikeout != STRIKEOUT_NONE )
            {
                for ( sal_Int32 i = 0; i < m_pStrikeoutLB->GetEntryCount(); ++i )
                {
                    if ( static_cast<FontStrikeout>(reinterpret_cast<sal_uLong>(m_pStrikeoutLB->GetEntryData(i))) == eStrikeout )
                    {
                        m_pStrikeoutLB->SelectEntryPos(i);
                        bEnable = true;
                        break;
                    }
                }
            }
        }
    }

    // WordLineMode
    nWhich = GetWhich( SID_ATTR_CHAR_WORDLINEMODE );
    switch ( eState )
    {
        case SfxItemState::UNKNOWN:
            m_pIndividualWordsBtn->Hide();
            break;

        case SfxItemState::DISABLED:
        case SfxItemState::READONLY:
            m_pIndividualWordsBtn->Disable();
            break;

        case SfxItemState::DONTCARE:
            m_pIndividualWordsBtn->SetState( TRISTATE_INDET );
            break;

        case SfxItemState::DEFAULT:
        case SfxItemState::SET:
        {
            const SvxWordLineModeItem& rItem = static_cast<const SvxWordLineModeItem&>(rSet->Get( nWhich ));
            rFont.SetWordLineMode( rItem.GetValue() );
            rCJKFont.SetWordLineMode( rItem.GetValue() );
            rCTLFont.SetWordLineMode( rItem.GetValue() );

            m_pIndividualWordsBtn->Check( rItem.GetValue() );
            m_pIndividualWordsBtn->Enable( bEnable );
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

        m_pEmphasisLB->SelectEntryPos( static_cast<sal_Int32>(FontEmphasisMark( eMark & FontEmphasisMark::Style )) );
        eMark &= ~FontEmphasisMark::Style;
        sal_uLong nEntryData = ( eMark == FontEmphasisMark::PosAbove )
            ? CHRDLG_POSITION_OVER
            : ( eMark == FontEmphasisMark::PosBelow ) ? CHRDLG_POSITION_UNDER : 0;

        for ( sal_Int32 i = 0; i < m_pPositionLB->GetEntryCount(); i++ )
        {
            if ( nEntryData == reinterpret_cast<sal_uLong>(m_pPositionLB->GetEntryData(i)) )
            {
                m_pPositionLB->SelectEntryPos(i);
                break;
            }
        }
    }
    else if ( eState == SfxItemState::DONTCARE )
        m_pEmphasisLB->SetNoSelection( );
    else if ( eState == SfxItemState::UNKNOWN )
    {
        m_pEmphasisFT->Hide();
        m_pEmphasisLB->Hide();
    }
    else // SfxItemState::DISABLED or SfxItemState::READONLY
    {
        m_pEmphasisFT->Disable();
        m_pEmphasisLB->Disable();
    }

    // the select handler for the underline/overline/strikeout list boxes
    DBG_ASSERT(m_pUnderlineLB->GetSelectHdl() == m_pOverlineLB->GetSelectHdl(),
        "SvxCharEffectsPage::Reset: inconsistence (1)!");
    DBG_ASSERT(m_pUnderlineLB->GetSelectHdl() == m_pStrikeoutLB->GetSelectHdl(),
        "SvxCharEffectsPage::Reset: inconsistence (1)!");
    m_pUnderlineLB->GetSelectHdl().Call(*m_pUnderlineLB);
        // don't call SelectHdl_Impl directly!
        // in DisableControls, we may have re-routed the select handler

    // the select handler for the emphasis listbox
    DBG_ASSERT(m_pEmphasisLB->GetSelectHdl() == LINK(this, SvxCharEffectsPage, SelectListBoxHdl_Impl),
        "SvxCharEffectsPage::Reset: inconsistence (2)!");
    m_pEmphasisLB->GetSelectHdl().Call( *m_pEmphasisLB );
        // this is for consistency only. Here it would be allowed to call SelectHdl_Impl directly ...

    // Effects
    SvxCaseMap eCaseMap = SvxCaseMap::End;
    nWhich = GetWhich( SID_ATTR_CHAR_CASEMAP );
    eState = rSet->GetItemState( nWhich );
    switch ( eState )
    {
        case SfxItemState::UNKNOWN:
            m_pEffectsFT->Hide();
            m_pEffectsLB->Hide();
            break;

        case SfxItemState::DISABLED:
        case SfxItemState::READONLY:
            m_pEffectsFT->Disable();
            m_pEffectsLB->Disable();
            break;

        case SfxItemState::DONTCARE:
            m_pEffectsLB->SetNoSelection();
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
            m_pReliefFT->Hide();
            m_pReliefLB->Hide();
            break;

        case SfxItemState::DISABLED:
        case SfxItemState::READONLY:
            m_pReliefFT->Disable();
            m_pReliefLB->Disable();
            break;

        case SfxItemState::DONTCARE:
            m_pReliefLB->SetNoSelection();
            break;

        case SfxItemState::DEFAULT:
        case SfxItemState::SET:
        {
            const SvxCharReliefItem& rItem = static_cast<const SvxCharReliefItem&>(rSet->Get( nWhich ));
            m_pReliefLB->SelectEntryPos(static_cast<sal_Int32>(rItem.GetValue()));
            SelectHdl_Impl(m_pReliefLB);
            break;
        }
    }

    // Outline
    nWhich = GetWhich( SID_ATTR_CHAR_CONTOUR );
    eState = rSet->GetItemState( nWhich );
    switch ( eState )
    {
        case SfxItemState::UNKNOWN:
            m_pOutlineBtn->Hide();
            break;

        case SfxItemState::DISABLED:
        case SfxItemState::READONLY:
            m_pOutlineBtn->Disable();
            break;

        case SfxItemState::DONTCARE:
            m_pOutlineBtn->SetState( TRISTATE_INDET );
            break;

        case SfxItemState::DEFAULT:
        case SfxItemState::SET:
        {
            const SvxContourItem& rItem = static_cast<const SvxContourItem&>(rSet->Get( nWhich ));
            m_pOutlineBtn->SetState( static_cast<TriState>(rItem.GetValue()) );
            m_pOutlineBtn->EnableTriState( false );
            break;
        }
    }

    // Shadow
    nWhich = GetWhich( SID_ATTR_CHAR_SHADOWED );
    eState = rSet->GetItemState( nWhich );

    switch ( eState )
    {
        case SfxItemState::UNKNOWN:
            m_pShadowBtn->Hide();
            break;

        case SfxItemState::DISABLED:
        case SfxItemState::READONLY:
            m_pShadowBtn->Disable();
            break;

        case SfxItemState::DONTCARE:
            m_pShadowBtn->SetState( TRISTATE_INDET );
            break;

        case SfxItemState::DEFAULT:
        case SfxItemState::SET:
        {
            const SvxShadowedItem& rItem = static_cast<const SvxShadowedItem&>(rSet->Get( nWhich ));
            m_pShadowBtn->SetState( static_cast<TriState>(rItem.GetValue()) );
            m_pShadowBtn->EnableTriState( false );
            break;
        }
    }

    // Blinking
    nWhich = GetWhich( SID_ATTR_FLASH );
    eState = rSet->GetItemState( nWhich );

    switch ( eState )
    {
        case SfxItemState::UNKNOWN:
            m_pBlinkingBtn->Hide();
            break;

        case SfxItemState::DISABLED:
        case SfxItemState::READONLY:
            m_pBlinkingBtn->Disable();
            break;

        case SfxItemState::DONTCARE:
            m_pBlinkingBtn->SetState( TRISTATE_INDET );
            break;

        case SfxItemState::DEFAULT:
        case SfxItemState::SET:
        {
            const SvxBlinkItem& rItem = static_cast<const SvxBlinkItem&>(rSet->Get( nWhich ));
            m_pBlinkingBtn->SetState( static_cast<TriState>(rItem.GetValue()) );
            m_pBlinkingBtn->EnableTriState( false );
            break;
        }
    }
    // Hidden
    nWhich = GetWhich( SID_ATTR_CHAR_HIDDEN );
    eState = rSet->GetItemState( nWhich );

    switch ( eState )
    {
        case SfxItemState::UNKNOWN:
            m_pHiddenBtn->Hide();
            break;

        case SfxItemState::DISABLED:
        case SfxItemState::READONLY:
            m_pHiddenBtn->Disable();
            break;

        case SfxItemState::DONTCARE:
            m_pHiddenBtn->SetState( TRISTATE_INDET );
            break;

        case SfxItemState::DEFAULT:
        case SfxItemState::SET:
        {
            const SvxCharHiddenItem& rItem = static_cast<const SvxCharHiddenItem&>(rSet->Get( nWhich ));
            m_pHiddenBtn->SetState( static_cast<TriState>(rItem.GetValue()) );
            m_pHiddenBtn->EnableTriState( false );
            break;
        }
    }

    SetPrevFontWidthScale( *rSet );
    ResetColor_Impl( *rSet );

    // preview update
    m_pPreviewWin->Invalidate();

    // save this settings
    ChangesApplied();
}

void SvxCharEffectsPage::ChangesApplied()
{
    m_pUnderlineLB->SaveValue();
    m_pOverlineLB->SaveValue();
    m_pStrikeoutLB->SaveValue();
    m_pIndividualWordsBtn->SaveValue();
    m_pEmphasisLB->SaveValue();
    m_pPositionLB->SaveValue();
    m_pEffectsLB->SaveValue();
    m_pReliefLB->SaveValue();
    m_pOutlineBtn->SaveValue();
    m_pShadowBtn->SaveValue();
    m_pBlinkingBtn->SaveValue();
    m_pHiddenBtn->SaveValue();
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
    sal_Int32 nPos = m_pUnderlineLB->GetSelectedEntryPos();
    FontLineStyle eUnder = static_cast<FontLineStyle>(reinterpret_cast<sal_uLong>(m_pUnderlineLB->GetEntryData( nPos )));

    if ( pOld )
    {
        //! if there are different underline styles in the selection the
        //! item-state in the 'rOldSet' will be invalid. In this case
        //! changing the underline style will be allowed if a style is
        //! selected in the listbox.
        bool bAllowChg = LISTBOX_ENTRY_NOTFOUND != nPos  &&
                         SfxItemState::DEFAULT > rOldSet.GetItemState( nWhich );

        const SvxUnderlineItem& rItem = *static_cast<const SvxUnderlineItem*>(pOld);
        if ( rItem.GetValue() == eUnder &&
             ( LINESTYLE_NONE == eUnder || rItem.GetColor() == m_pUnderlineColorLB->GetSelectEntryColor() ) &&
             ! bAllowChg )
            bChanged = false;
    }

    if ( bChanged )
    {
        SvxUnderlineItem aNewItem( eUnder, nWhich );
        aNewItem.SetColor( m_pUnderlineColorLB->GetSelectEntryColor() );
        rSet->Put( aNewItem );
        bModified = true;
    }
     else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich, false ) )
        rSet->InvalidateItem(nWhich);

    bChanged = true;

    // Overline
    nWhich = GetWhich( SID_ATTR_CHAR_OVERLINE );
    pOld = GetOldItem( *rSet, SID_ATTR_CHAR_OVERLINE );
    nPos = m_pOverlineLB->GetSelectedEntryPos();
    FontLineStyle eOver = static_cast<FontLineStyle>(reinterpret_cast<sal_uLong>(m_pOverlineLB->GetEntryData( nPos )));

    if ( pOld )
    {
        //! if there are different underline styles in the selection the
        //! item-state in the 'rOldSet' will be invalid. In this case
        //! changing the underline style will be allowed if a style is
        //! selected in the listbox.
        bool bAllowChg = LISTBOX_ENTRY_NOTFOUND != nPos  &&
                         SfxItemState::DEFAULT > rOldSet.GetItemState( nWhich );

        const SvxOverlineItem& rItem = *static_cast<const SvxOverlineItem*>(pOld);
        if ( rItem.GetValue() == eOver &&
             ( LINESTYLE_NONE == eOver || rItem.GetColor() == m_pOverlineColorLB->GetSelectEntryColor() ) &&
             ! bAllowChg )
            bChanged = false;
    }

    if ( bChanged )
    {
        SvxOverlineItem aNewItem( eOver, nWhich );
        aNewItem.SetColor( m_pOverlineColorLB->GetSelectEntryColor() );
        rSet->Put( aNewItem );
        bModified = true;
    }
    else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich, false ) )
        rSet->InvalidateItem(nWhich);

    bChanged = true;

    // Strikeout
    nWhich = GetWhich( SID_ATTR_CHAR_STRIKEOUT );
    pOld = GetOldItem( *rSet, SID_ATTR_CHAR_STRIKEOUT );
    nPos = m_pStrikeoutLB->GetSelectedEntryPos();
    FontStrikeout eStrike = static_cast<FontStrikeout>(reinterpret_cast<sal_uLong>(m_pStrikeoutLB->GetEntryData( nPos )));

    if ( pOld )
    {
        //! if there are different strikeout styles in the selection the
        //! item-state in the 'rOldSet' will be invalid. In this case
        //! changing the strikeout style will be allowed if a style is
        //! selected in the listbox.
        bool bAllowChg = LISTBOX_ENTRY_NOTFOUND != nPos  &&
                         SfxItemState::DEFAULT > rOldSet.GetItemState( nWhich );

        const SvxCrossedOutItem& rItem = *static_cast<const SvxCrossedOutItem*>(pOld);
        if ( !m_pStrikeoutLB->IsEnabled()
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
        if ( rItem.GetValue() == m_pIndividualWordsBtn->IsChecked() )
            bChanged = false;
    }

    if ( rOldSet.GetItemState( nWhich ) == SfxItemState::DONTCARE &&
         ! m_pIndividualWordsBtn->IsValueChangedFromSaved() )
        bChanged = false;

    if ( bChanged )
    {
        rSet->Put( SvxWordLineModeItem( m_pIndividualWordsBtn->IsChecked(), nWhich ) );
        bModified = true;
    }
    else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich, false ) )
        rSet->InvalidateItem(nWhich);

    bChanged = true;

    // Emphasis
    nWhich = GetWhich( SID_ATTR_CHAR_EMPHASISMARK );
    pOld = GetOldItem( *rSet, SID_ATTR_CHAR_EMPHASISMARK );
    sal_Int32 nMarkPos = m_pEmphasisLB->GetSelectedEntryPos();
    sal_Int32 nPosPos = m_pPositionLB->GetSelectedEntryPos();
    FontEmphasisMark eMark = static_cast<FontEmphasisMark>(nMarkPos);
    if ( m_pPositionLB->IsEnabled() )
    {
        eMark |= ( CHRDLG_POSITION_UNDER == reinterpret_cast<sal_uLong>(m_pPositionLB->GetEntryData( nPosPos )) )
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

    if ( rOldSet.GetItemState( nWhich ) == SfxItemState::DONTCARE &&
         m_pEmphasisLB->GetSavedValue() == nMarkPos && m_pPositionLB->GetSavedValue() == nPosPos )
        bChanged = false;

    if ( bChanged )
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
    sal_Int32 nCapsPos = m_pEffectsLB->GetSelectedEntryPos();
    if ( nCapsPos != LISTBOX_ENTRY_NOTFOUND )
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
        bool bAllowChg = LISTBOX_ENTRY_NOTFOUND != nPos  &&
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
    if(m_pReliefLB->IsValueChangedFromSaved())
    {
        m_pReliefLB->SaveValue();
        SvxCharReliefItem aRelief(static_cast<FontRelief>(m_pReliefLB->GetSelectedEntryPos()), nWhich);
        rSet->Put(aRelief);
    }

    // Outline
    const SfxItemSet* pExampleSet = GetTabDialog() ? GetTabDialog()->GetExampleSet() : nullptr;
    nWhich = GetWhich( SID_ATTR_CHAR_CONTOUR );
    pOld = GetOldItem( *rSet, SID_ATTR_CHAR_CONTOUR );
    TriState eState = m_pOutlineBtn->GetState();
    const SfxPoolItem* pItem;

    if ( pOld )
    {
        const SvxContourItem& rItem = *static_cast<const SvxContourItem*>(pOld);
        if ( rItem.GetValue() == StateToAttr( eState ) && m_pOutlineBtn->GetSavedValue() == eState )
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
    eState = m_pShadowBtn->GetState();

    if ( pOld )
    {
        const SvxShadowedItem& rItem = *static_cast<const SvxShadowedItem*>(pOld);
        if ( rItem.GetValue() == StateToAttr( eState ) && m_pShadowBtn->GetSavedValue() == eState )
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
    eState = m_pBlinkingBtn->GetState();

    if ( pOld )
    {
        const SvxBlinkItem& rItem = *static_cast<const SvxBlinkItem*>(pOld);
        if ( rItem.GetValue() == StateToAttr( eState ) && m_pBlinkingBtn->GetSavedValue() == eState )
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
    eState = m_pHiddenBtn->GetState();
    bChanged = true;

    if ( pOld )
    {
        const SvxCharHiddenItem& rItem = *static_cast<const SvxCharHiddenItem*>(pOld);
        if ( rItem.GetValue() == StateToAttr( eState ) && m_pHiddenBtn->GetSavedValue() == eState )
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
        m_pEffectsFT->Disable();
        m_pEffectsLB->Disable();
    }

    if ( ( DISABLE_WORDLINE & nDisable ) == DISABLE_WORDLINE )
        m_pIndividualWordsBtn->Disable();

    if ( ( DISABLE_BLINK & nDisable ) == DISABLE_BLINK )
        m_pBlinkingBtn->Disable();

    if ( ( DISABLE_UNDERLINE_COLOR & nDisable ) == DISABLE_UNDERLINE_COLOR )
    {
        // disable the controls
        m_pUnderlineColorFT->Disable( );
        m_pUnderlineColorLB->Disable( );
        // and reroute the selection handler of the controls which normally would affect the color box dis-/enabling
        m_pUnderlineLB->SetSelectHdl(LINK(this, SvxCharEffectsPage, UpdatePreview_Impl));
        m_pStrikeoutLB->SetSelectHdl(LINK(this, SvxCharEffectsPage, UpdatePreview_Impl));
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
            m_pBlinkingBtn->Show();
        if ( ( nFlags & SVX_PREVIEW_CHARACTER ) == SVX_PREVIEW_CHARACTER )
            // the writer uses SID_ATTR_BRUSH as font background
            m_bPreviewBackgroundToCharacter = true;
    }
}

// class SvxCharPositionPage ---------------------------------------------

SvxCharPositionPage::SvxCharPositionPage(TabPageParent pParent, const SfxItemSet& rInSet)
    : CharBasePage(pParent, "cui/ui/positionpage.ui", "PositionPage", rInSet)
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
    , m_xHighLowMF(m_xBuilder->weld_metric_spin_button("raiselowersb", FUNIT_PERCENT))
    , m_xHighLowRB(m_xBuilder->weld_check_button("automatic"))
    , m_xFontSizeFT(m_xBuilder->weld_label("relativefontsize"))
    , m_xFontSizeMF(m_xBuilder->weld_metric_spin_button("fontsizesb", FUNIT_PERCENT))
    , m_xRotationContainer(m_xBuilder->weld_widget("rotationcontainer"))
    , m_xScalingFT(m_xBuilder->weld_label("scale"))
    , m_xScalingAndRotationFT(m_xBuilder->weld_label("rotateandscale"))
    , m_x0degRB(m_xBuilder->weld_radio_button("0deg"))
    , m_x90degRB(m_xBuilder->weld_radio_button("90deg"))
    , m_x270degRB(m_xBuilder->weld_radio_button("270deg"))
    , m_xFitToLineCB(m_xBuilder->weld_check_button("fittoline"))
    , m_xScaleWidthMF(m_xBuilder->weld_metric_spin_button("scalewidthsb", FUNIT_PERCENT))
    , m_xKerningMF(m_xBuilder->weld_metric_spin_button("kerningsb", FUNIT_POINT))
    , m_xPairKerningBtn(m_xBuilder->weld_check_button("pairkerning"))
{
    m_xPreviewWin.reset(new weld::CustomWeld(*m_xBuilder, "preview", m_aPreviewWin));
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

    m_xHighLowMF->set_value(aEscItm.GetEsc() * nFac, FUNIT_PERCENT);
    m_xFontSizeMF->set_value(aEscItm.GetProportionalHeight(), FUNIT_PERCENT);

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
    sal_uInt8 nEscProp = static_cast<sal_uInt8>(m_xFontSizeMF->get_value(FUNIT_PERCENT));
    short nEsc  = static_cast<short>(m_xHighLowMF->get_value(FUNIT_PERCENT));
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
    m_xScaleWidthMF->set_value(nVal, FUNIT_PERCENT);
    m_aPreviewWin.SetFontWidthScale( nVal );
}

IMPL_LINK_NOARG(SvxCharPositionPage, KerningModifyHdl_Impl, weld::MetricSpinButton&, void)
{
    long nVal = static_cast<long>(m_xKerningMF->get_value(FUNIT_POINT));
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
            m_nSubEsc = static_cast<short>(m_xHighLowMF->get_value(FUNIT_PERCENT)) * -1;
        else
            m_nSuperEsc = static_cast<short>(m_xHighLowMF->get_value(FUNIT_PERCENT));
    }
    else if (m_xFontSizeMF.get() == &rField)
    {
        if ( bLow )
            m_nSubProp = static_cast<sal_uInt8>(m_xFontSizeMF->get_value(FUNIT_PERCENT));
        else
            m_nSuperProp = static_cast<sal_uInt8>(m_xFontSizeMF->get_value(FUNIT_PERCENT));
    }

    FontModifyHdl_Impl();
}

IMPL_LINK_NOARG(SvxCharPositionPage, ScaleWidthModifyHdl_Impl, weld::MetricSpinButton&, void)
{
    m_aPreviewWin.SetFontWidthScale(sal_uInt16(m_xScaleWidthMF->get_value(FUNIT_PERCENT)));
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
        m_nSuperEsc = static_cast<short>(sUser.getToken( 0, ';' ).toInt32());
        m_nSubEsc = static_cast<short>(sUser.getToken( 1, ';' ).toInt32());
        m_nSuperProp = static_cast<sal_uInt8>(sUser.getToken( 2, ';' ).toInt32());
        m_nSubProp = static_cast<sal_uInt8>(sUser.getToken( 3, ';' ).toInt32());

        //fdo#75307 validate all the entries and discard all of them if any are
        //out of range
        bool bValid = true;
        if (m_nSuperEsc < m_xHighLowMF->get_min(FUNIT_PERCENT) || m_nSuperEsc > m_xHighLowMF->get_max(FUNIT_PERCENT))
            bValid = false;
        if (m_nSubEsc*-1 < m_xHighLowMF->get_min(FUNIT_PERCENT) || m_nSubEsc*-1 > m_xHighLowMF->get_max(FUNIT_PERCENT))
            bValid = false;
        if (m_nSuperProp < m_xFontSizeMF->get_min(FUNIT_PERCENT) || m_nSuperProp > m_xFontSizeMF->get_max(FUNIT_PERCENT))
            bValid = false;
        if (m_nSubProp < m_xFontSizeMF->get_min(FUNIT_PERCENT) || m_nSubProp > m_xFontSizeMF->get_max(FUNIT_PERCENT))
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
            m_xHighLowMF->set_value(m_xHighLowMF->normalize(nFac * nEsc), FUNIT_PERCENT);
        }
        else
        {
            m_xNormalPosBtn->set_active(true);
            m_xHighLowRB->set_active(true);
            PositionHdl_Impl(*m_xNormalPosBtn);
        }
        //the height has to be set after the handler is called to keep the value also if the escapement is zero
        m_xFontSizeMF->set_value(m_xFontSizeMF->normalize(nEscProp), FUNIT_PERCENT);
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
        long nVal = static_cast<long>(m_xKerningMF->get_max(FUNIT_POINT));
        if(nVal < nKerning)
            m_xKerningMF->set_max(nKerning, FUNIT_POINT);
        m_xKerningMF->set_value(nKerning, FUNIT_POINT);
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
        m_xScaleWidthMF->set_value(m_nScaleWidthInitialVal, FUNIT_PERCENT);
    }
    else
        m_xScaleWidthMF->set_value(100, FUNIT_PERCENT);

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
            nEsc = static_cast<short>(m_xHighLowMF->denormalize(m_xHighLowMF->get_value(FUNIT_PERCENT)));
            nEsc *= (bHigh ? 1 : -1);
        }
        nEscProp = static_cast<sal_uInt8>(m_xFontSizeMF->denormalize(m_xFontSizeMF->get_value(FUNIT_PERCENT)));
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

    long nTmp = static_cast<long>(m_xKerningMF->get_value(FUNIT_POINT));
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
        rSet->Put(SvxCharScaleWidthItem(static_cast<sal_uInt16>(m_xScaleWidthMF->get_value(FUNIT_PERCENT)), nWhich));
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
    : CharBasePage(pParent, "cui/ui/twolinespage.ui", "TwoLinesPage", rInSet)
    , m_nStartBracketPosition( 0 )
    , m_nEndBracketPosition( 0 )
    , m_xTwoLinesBtn(m_xBuilder->weld_check_button("twolines"))
    , m_xEnclosingFrame(m_xBuilder->weld_widget("enclosing"))
    , m_xStartBracketLB(m_xBuilder->weld_tree_view("startbracket"))
    , m_xEndBracketLB(m_xBuilder->weld_tree_view("endbracket"))
{
    m_xPreviewWin.reset(new weld::CustomWeld(*m_xBuilder, "preview", m_aPreviewWin));
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

    if (aDlg.execute() == RET_OK)
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
    CharBasePage::ActivatePage(rSet);
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
