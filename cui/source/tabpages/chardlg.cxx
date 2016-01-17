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
#include <vcl/msgbox.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include <svtools/unitconv.hxx>
#include <svl/languageoptions.hxx>
#include <svx/xtable.hxx>
#include "chardlg.hxx"
#include "editeng/fontitem.hxx"
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/prszitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/blinkitem.hxx>
#include "editeng/flstitem.hxx"
#include <editeng/autokernitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/colritem.hxx>
#include "svx/drawitem.hxx"
#include "svx/dlgutil.hxx"
#include <dialmgr.hxx>
#include <sfx2/htmlmode.hxx>
#include "cuicharmap.hxx"
#include "chardlg.h"
#include <editeng/emphasismarkitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/twolinesitem.hxx>
#include <editeng/charhiddenitem.hxx>
#include <svl/stritem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <svx/svxdlg.hxx>
#include <cuires.hrc>
#include <svl/intitem.hxx>
#include <sfx2/request.hxx>
#include "svx/flagsdef.hxx"

using namespace ::com::sun::star;

// define ----------------------------------------------------------------

#define ISITEMSET   rSet.GetItemState(nWhich)>=SfxItemState::DEFAULT

#define CLEARTITEM  rSet.InvalidateItem(nWhich)

#define LW_NORMAL   0
#define LW_EXPANDED 1
#define LW_CONDENSED   2

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

// C-Funktion ------------------------------------------------------------

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
    if( ISITEMSET )
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


void SvxCharBasePage::SetPrevFontEscapement( sal_uInt8 nProp, sal_uInt8 nEscProp, short nEsc )
{
    setPrevFontEscapement(GetPreviewFont(),nProp,nEscProp,nEsc);
    setPrevFontEscapement(GetPreviewCJKFont(),nProp,nEscProp,nEsc);
    setPrevFontEscapement(GetPreviewCTLFont(),nProp,nEscProp,nEsc);
    m_pPreviewWin->Invalidate();
}

// SvxCharNamePage_Impl --------------------------------------------------

struct SvxCharNamePage_Impl
{
    Idle            m_aUpdateIdle;
    OUString        m_aNoStyleText;
    const FontList* m_pFontList;
    sal_Int32           m_nExtraEntryPos;
    bool            m_bMustDelete;
    bool            m_bInSearchMode;

    SvxCharNamePage_Impl() :

        m_pFontList     ( nullptr ),
        m_nExtraEntryPos( COMBOBOX_ENTRY_NOTFOUND ),
        m_bMustDelete   ( false ),
        m_bInSearchMode ( false )

    {
        m_aUpdateIdle.SetPriority( SchedulerPriority::LOWEST );
    }

    ~SvxCharNamePage_Impl()
    {
        if ( m_bMustDelete )
            delete m_pFontList;
    }
};

// class SvxCharNamePage -------------------------------------------------

SvxCharNamePage::SvxCharNamePage( vcl::Window* pParent, const SfxItemSet& rInSet )
    : SvxCharBasePage(pParent, "CharNamePage", "cui/ui/charnamepage.ui", rInSet)
    , m_pImpl(new SvxCharNamePage_Impl)
{
    m_pImpl->m_aNoStyleText = CUI_RES( RID_SVXSTR_CHARNAME_NOSTYLE );

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

    //In MacOSX the standard dialogs name font-name, font-style as
    //Family, Typeface
    //In GNOME the standard dialogs name font-name, font-style as
    //Family, Style
    //In Windows the standard dialogs name font-name, font-style as
    //Font, Style
#ifdef WNT
    OUString sFontFamilyString(CUI_RES(RID_SVXSTR_CHARNAME_FONT));
#else
    OUString sFontFamilyString(CUI_RES(RID_SVXSTR_CHARNAME_FAMILY));
#endif
    m_pWestFontNameFT->SetText(sFontFamilyString);
    m_pEastFontNameFT->SetText(sFontFamilyString);
    m_pCTLFontNameFT->SetText(sFontFamilyString);

#ifdef MACOSX
    OUString sFontStyleString(CUI_RES(RID_SVXSTR_CHARNAME_TYPEFACE));
#else
    OUString sFontStyleString(CUI_RES(RID_SVXSTR_CHARNAME_STYLE));
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
    delete m_pImpl;
    m_pImpl = nullptr;
    m_pWestFrame.clear();
    m_pWestFontNameFT.clear();
    m_pWestFontNameLB.clear();
    m_pWestFontStyleFT.clear();
    m_pWestFontStyleLB.clear();
    m_pWestFontSizeFT.clear();
    m_pWestFontSizeLB.clear();
    m_pWestFontLanguageFT.clear();
    m_pWestFontLanguageLB.clear();
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
    m_pEastFontNameLB->SetModifyHdl( aLink );
    m_pEastFontStyleLB->SetModifyHdl( aLink );
    m_pEastFontSizeLB->SetModifyHdl( aLink );
    m_pEastFontLanguageLB->SetSelectHdl( LINK( this, SvxCharNamePage, FontModifyListBoxHdl_Impl ) );
    m_pCTLFontNameLB->SetModifyHdl( aLink );
    m_pCTLFontStyleLB->SetModifyHdl( aLink );
    m_pCTLFontSizeLB->SetModifyHdl( aLink );
    m_pCTLFontLanguageLB->SetSelectHdl( LINK( this, SvxCharNamePage, FontModifyListBoxHdl_Impl ) );

    m_pImpl->m_aUpdateIdle.SetIdleHdl( LINK( this, SvxCharNamePage, UpdateHdl_Impl ) );
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
                    m_pImpl->m_pFontList =  static_cast<const SvxFontListItem*>(pItem )->GetFontList()->Clone();
                m_pImpl->m_bMustDelete = true;
            }
        }
        if(!m_pImpl->m_pFontList)
        {
            m_pImpl->m_pFontList =
                new FontList( Application::GetDefaultDevice() );
            m_pImpl->m_bMustDelete = true;
        }
    }

    return m_pImpl->m_pFontList;
}


namespace
{
    vcl::FontInfo calcFontInfo(  SvxFont& _rFont,
                    SvxCharNamePage* _pPage,
                    const FontNameBox* _pFontNameLB,
                    const FontStyleBox* _pFontStyleLB,
                    const FontSizeBox* _pFontSizeLB,
                    const SvxLanguageBoxBase* _pLanguageLB,
                    const FontList* _pFontList,
                    sal_uInt16 _nFontWhich,
                    sal_uInt16 _nFontHeightWhich)
    {
        Size aSize = _rFont.GetSize();
        aSize.Width() = 0;
        vcl::FontInfo aFontInfo;
        OUString sFontName(_pFontNameLB->GetText());
        bool bFontAvailable = _pFontList->IsAvailable( sFontName );
        if (bFontAvailable  || _pFontNameLB->IsValueChangedFromSaved())
            aFontInfo = _pFontList->Get( sFontName, _pFontStyleLB->GetText() );
        else
        {
            //get the font from itemset
            SfxItemState eState = _pPage->GetItemSet().GetItemState( _nFontWhich );
            if ( eState >= SfxItemState::DEFAULT )
            {
                const SvxFontItem* pFontItem = static_cast<const SvxFontItem*>(&( _pPage->GetItemSet().Get( _nFontWhich ) ));
                aFontInfo.SetName(pFontItem->GetFamilyName());
                aFontInfo.SetStyleName(pFontItem->GetStyleName());
                aFontInfo.SetFamily(pFontItem->GetFamily());
                aFontInfo.SetPitch(pFontItem->GetPitch());
                aFontInfo.SetCharSet(pFontItem->GetCharSet());
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
            aSize.Height() =
                ItemToControl( nHeight, _pPage->GetItemSet().GetPool()->GetMetric( _nFontHeightWhich ), FUNIT_TWIP );
        }
        else if ( !_pFontSizeLB->GetText().isEmpty() )
            aSize.Height() = PointToTwips( static_cast<long>(_pFontSizeLB->GetValue() / 10) );
        else
            aSize.Height() = 200;   // default 10pt
        aFontInfo.SetSize( aSize );

        _rFont.SetLanguage(_pLanguageLB->GetSelectLanguage());

        _rFont.SetFamily( aFontInfo.GetFamily() );
        _rFont.SetName( aFontInfo.GetFamilyName() );
        _rFont.SetStyleName( aFontInfo.GetStyleName() );
        _rFont.SetPitch( aFontInfo.GetPitch() );
        _rFont.SetCharSet( aFontInfo.GetCharSet() );
        _rFont.SetWeight( aFontInfo.GetWeight() );
        _rFont.SetItalic( aFontInfo.GetItalic() );
        _rFont.SetSize( aFontInfo.GetSize() );

        return aFontInfo;
    }
}


void SvxCharNamePage::UpdatePreview_Impl()
{
    SvxFont& rFont = GetPreviewFont();
    SvxFont& rCJKFont = GetPreviewCJKFont();
    SvxFont& rCTLFont = GetPreviewCTLFont();
    // Size
    Size aSize = rFont.GetSize();
    aSize.Width() = 0;
    Size aCJKSize = rCJKFont.GetSize();
    aCJKSize.Width() = 0;
    Size aCTLSize = rCTLFont.GetSize();
    aCTLSize.Width() = 0;
    // Font
    const FontList* pFontList = GetFontList();

    vcl::FontInfo aWestFontInfo = calcFontInfo(rFont, this, m_pWestFontNameLB,
        m_pWestFontStyleLB, m_pWestFontSizeLB, m_pWestFontLanguageLB,
        pFontList, GetWhich(SID_ATTR_CHAR_FONT),
        GetWhich(SID_ATTR_CHAR_FONTHEIGHT));
    m_pWestFontTypeFT->SetText(pFontList->GetFontMapText(aWestFontInfo));

    vcl::FontInfo aEastFontInfo = calcFontInfo(rCJKFont, this, m_pEastFontNameLB,
        m_pEastFontStyleLB, m_pEastFontSizeLB, m_pEastFontLanguageLB,
        pFontList, GetWhich(SID_ATTR_CHAR_CJK_FONT),
        GetWhich(SID_ATTR_CHAR_CJK_FONTHEIGHT));
    m_pEastFontTypeFT->SetText(pFontList->GetFontMapText(aEastFontInfo));

    vcl::FontInfo aCTLFontInfo = calcFontInfo(rCTLFont,
        this, m_pCTLFontNameLB, m_pCTLFontStyleLB, m_pCTLFontSizeLB,
        m_pCTLFontLanguageLB, pFontList, GetWhich(SID_ATTR_CHAR_CTL_FONT),
        GetWhich(SID_ATTR_CHAR_CTL_FONTHEIGHT));
    m_pCTLFontTypeFT->SetText(pFontList->GetFontMapText(aCTLFontInfo));

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

    vcl::FontInfo _aFontInfo( pFontList->Get( pNameBox->GetText(), pStyleBox->GetText() ) );
    pSizeBox->Fill( &_aFontInfo, pFontList );
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
        eItalic = (FontItalic)rItem.GetValue();
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
        eWeight = (FontWeight)rItem.GetValue();
    }
    else
        bStyle = false;
    bStyleAvailable = bStyleAvailable && (eState >= SfxItemState::DONTCARE);

    // currently chosen font
    if ( bStyle && pFontItem )
    {
        vcl::FontInfo aInfo = pFontList->Get( pFontItem->GetFamilyName(), eWeight, eItalic );
        pStyleBox->SetText( pFontList->GetStyleName( aInfo ) );
    }
    else if ( !m_pImpl->m_bInSearchMode || !bStyle )
    {
        pStyleBox->SetText( OUString() );
    }
    else if ( bStyle )
    {
        vcl::FontInfo aInfo = pFontList->Get( OUString(), eWeight, eItalic );
        pStyleBox->SetText( pFontList->GetStyleName( aInfo ) );
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
        SfxMapUnit eUnit = rSet.GetPool()->GetMetric( nWhich );
        const SvxFontHeightItem& rItem = static_cast<const SvxFontHeightItem&>(rSet.Get( nWhich ));

        if( rItem.GetProp() != 100 || SFX_MAPUNIT_RELATIVE != rItem.GetPropUnit() )
        {
            bool bPtRel = SFX_MAPUNIT_POINT == rItem.GetPropUnit();
            pSizeBox->SetPtRelative( bPtRel );
            pSizeBox->SetValue( bPtRel ? ((short)rItem.GetProp()) * 10 : rItem.GetProp() );
        }
        else
        {
            pSizeBox->SetRelative(false);
            pSizeBox->SetValue( (long)CalcToPoint( rItem.GetHeight(), eUnit, 10 ) );
        }
    }
    else if ( eState >= SfxItemState::DEFAULT )
    {
        SfxMapUnit eUnit = rSet.GetPool()->GetMetric( nWhich );
        const SvxFontHeightItem& rItem = static_cast<const SvxFontHeightItem&>(rSet.Get( nWhich ));
        pSizeBox->SetValue( (long)CalcToPoint( rItem.GetHeight(), eUnit, 10 ) );
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
            LanguageType eLangType = (LanguageType)rItem.GetValue();
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
    vcl::FontInfo aInfo( pFontList->Get( rFontName, aStyleBoxText ) );
    SvxFontItem aFontItem( aInfo.GetFamily(), aInfo.GetFamilyName(), aInfo.GetStyleName(),
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
        CLEARTITEM;

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
        CLEARTITEM;

    // FontSize
    long nSize = static_cast<long>(pSizeBox->GetValue());

    if ( pSizeBox->GetText().isEmpty() )   // GetValue() returns the min-value
        nSize = 0;
    long nSavedSize = pSizeBox->GetSavedValue().toInt32();
    bool bRel = true;

    if ( !pSizeBox->IsRelative() )
    {
        nSavedSize *= 10;
        bRel = false;
    }

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
        float fSize = (float)nSize / 10;
        long nVal = CalcToUnit( fSize, rSet.GetPool()->GetMetric( nWhich ) );
        if ( static_cast<const SvxFontHeightItem*>(pItem)->GetHeight() != (sal_uInt32)nVal )
            bChanged = true;
    }

    if ( bChanged || !pOldHeight ||
         bRel != ( SFX_MAPUNIT_RELATIVE != pOldHeight->GetPropUnit() || 100 != pOldHeight->GetProp() ) )
    {
        SfxMapUnit eUnit = rSet.GetPool()->GetMetric( nWhich );
        if ( pSizeBox->IsRelative() )
        {
            DBG_ASSERT( GetItemSet().GetParent(), "No parent set" );
            const SvxFontHeightItem& rOldItem =
                static_cast<const SvxFontHeightItem&>(GetItemSet().GetParent()->Get( nWhich ));

            SvxFontHeightItem aHeight( 240, 100, nWhich );
            if ( pSizeBox->IsPtRelative() )
                aHeight.SetHeight( rOldItem.GetHeight(), (sal_uInt16)( nSize / 10 ), SFX_MAPUNIT_POINT, eUnit );
            else
                aHeight.SetHeight( rOldItem.GetHeight(), (sal_uInt16)nSize );
            rSet.Put( aHeight );
        }
        else
        {
            float fSize = (float)nSize / 10;
            rSet.Put( SvxFontHeightItem( CalcToUnit( fSize, eUnit ), 100, nWhich ) );
        }
        bModified = true;
    }
    else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich, false ) )
        CLEARTITEM;

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
            case SvxLanguageComboBox::EDITED_NO:
                ;   // nothing to do
                break;
            case SvxLanguageComboBox::EDITED_VALID:
                {
                    const sal_Int32 nPos = pLangComboBox->SaveEditedAsEntry();
                    if (nPos != COMBOBOX_ENTRY_NOTFOUND)
                        pLangComboBox->SelectEntryPos( nPos);
                }
                break;
            case SvxLanguageComboBox::EDITED_INVALID:
                pLangComboBox->SelectEntryPos( pLangComboBox->GetSavedValueLBB());
                break;
        }
    }

    sal_Int32 nLangPos = pLangBox->GetSelectEntryPosLBB();
    LanguageType eLangType = (LanguageType)reinterpret_cast<sal_uLong>(pLangBox->GetEntryDataLBB( nLangPos ));

    if ( pOld )
    {
        const SvxLanguageItem& rItem = *static_cast<const SvxLanguageItem*>(pOld);

        if ( nLangPos == LISTBOX_ENTRY_NOTFOUND || eLangType == (LanguageType)rItem.GetValue() )
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
        CLEARTITEM;

    return bModified;
}


IMPL_LINK_NOARG_TYPED(SvxCharNamePage, UpdateHdl_Impl, Idle *, void)
{
    UpdatePreview_Impl();
}


IMPL_LINK_TYPED( SvxCharNamePage, FontModifyComboBoxHdl_Impl, ComboBox&, rBox, void )
{
    FontModifyHdl_Impl(&rBox);
}
IMPL_LINK_TYPED( SvxCharNamePage, FontModifyListBoxHdl_Impl, ListBox&, rBox, void )
{
    FontModifyHdl_Impl(&rBox);
}
IMPL_LINK_TYPED( SvxCharNamePage, FontModifyEditHdl_Impl, Edit&, rBox, void )
{
    FontModifyHdl_Impl(&rBox);
}
void SvxCharNamePage::FontModifyHdl_Impl(void* pNameBox)
{
    m_pImpl->m_aUpdateIdle.Start();

    if ( m_pWestFontNameLB == pNameBox || m_pEastFontNameLB == pNameBox || m_pCTLFontNameLB == pNameBox )
    {
        FillStyleBox_Impl( static_cast<FontNameBox*>(pNameBox) );
        FillSizeBox_Impl( static_cast<FontNameBox*>(pNameBox) );
    }
}


void SvxCharNamePage::ActivatePage( const SfxItemSet& rSet )
{
    SvxCharBasePage::ActivatePage( rSet );

    UpdatePreview_Impl();       // instead of asynchronous calling in ctor
}


SfxTabPage::sfxpg SvxCharNamePage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( _pSet );
    return LEAVE_PAGE;
}


VclPtr<SfxTabPage> SvxCharNamePage::Create( vcl::Window* pParent, const SfxItemSet* rSet )
{
    return VclPtr<SvxCharNamePage>::Create( pParent, *rSet );
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
    if ( m_pImpl->m_bMustDelete )
    {
        delete m_pImpl->m_pFontList;
    }
    m_pImpl->m_pFontList = rItem.GetFontList()->Clone();
    m_pImpl->m_bMustDelete = true;
}


namespace
{
    void enableRelativeMode( SvxCharNamePage* _pPage, FontSizeBox* _pFontSizeLB, sal_uInt16 _nHeightWhich )
    {
        _pFontSizeLB->EnableRelativeMode( 5, 995 ); // min 5%, max 995%, step 5

        const SvxFontHeightItem& rHeightItem =
            static_cast<const SvxFontHeightItem&>(_pPage->GetItemSet().GetParent()->Get( _nHeightWhich ));
        SfxMapUnit eUnit = _pPage->GetItemSet().GetPool()->GetMetric( _nHeightWhich );
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


void SvxCharNamePage::SetPreviewBackgroundToCharacter()
{
    m_bPreviewBackgroundToCharacter = true;
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
            SetPreviewBackgroundToCharacter();
    }
    if (pDisalbeItem)
        DisableControls(pDisalbeItem->GetValue());
}
// class SvxCharEffectsPage ----------------------------------------------

SvxCharEffectsPage::SvxCharEffectsPage( vcl::Window* pParent, const SfxItemSet& rInSet )
    : SvxCharBasePage(pParent, "EffectsPage", "cui/ui/effectspage.ui", rInSet)
    , m_aTransparentColorName(CUI_RES(RID_SVXSTR_CHARNAME_TRANSPARENT))
{
    get(m_pFontColorFT, "fontcolorft");
    get(m_pFontColorLB, "fontcolorlb");
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
    get(m_pStrikeoutLB, "strikeoutlb");
    get(m_pUnderlineLB, "underlinelb");
    get(m_pUnderlineColorFT, "underlinecolorft");
    get(m_pUnderlineColorLB, "underlinecolorlb");
    get(m_pIndividualWordsBtn, "individualwordscb");
    get(m_pEmphasisFT, "emphasisft");
    get(m_pEmphasisLB, "emphasislb");
    get(m_pPositionFT, "positionft");
    get(m_pPositionLB, "positionlb");

    get(m_pPreviewWin, "preview");
    Initialize();
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

    // fill the color box
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    DBG_ASSERT( pDocSh, "DocShell not found!" );
    XColorListRef pColorTable;

    if ( pDocSh )
    {
        pItem = pDocSh->GetItem( SID_COLOR_TABLE );
        if ( pItem != nullptr )
            pColorTable = static_cast<const SvxColorListItem*>(pItem)->GetColorList();
    }

    if ( !pColorTable.is() )
        pColorTable = XColorList::CreateStdColorList();

    m_pUnderlineColorLB->SetUpdateMode( false );
    m_pOverlineColorLB->SetUpdateMode( false );
    m_pFontColorLB->SetUpdateMode( false );

    {
        SfxPoolItem* pDummy = nullptr;
        SfxViewFrame* pFrame = SfxViewFrame::GetFirst( pDocSh );
        if ( !pFrame ||
             SfxItemState::DEFAULT > pFrame->GetBindings().QueryState( SID_ATTR_AUTO_COLOR_INVALID, pDummy ) )
        {
            m_pUnderlineColorLB->InsertAutomaticEntryColor( Color( COL_AUTO ) );
            m_pOverlineColorLB->InsertAutomaticEntryColor( Color( COL_AUTO ) );
            m_pFontColorLB->InsertAutomaticEntryColor( Color( COL_AUTO ) );
        }
        delete pDummy;
    }
    for ( long i = 0; i < pColorTable->Count(); i++ )
    {
        XColorEntry* pEntry = pColorTable->GetColor(i);
        m_pUnderlineColorLB->InsertEntry( pEntry->GetColor(), pEntry->GetName() );
        m_pOverlineColorLB->InsertEntry( pEntry->GetColor(), pEntry->GetName() );
        m_pFontColorLB->InsertEntry( pEntry->GetColor(), pEntry->GetName() );
    }

    m_pUnderlineColorLB->SetUpdateMode( true );
    m_pOverlineColorLB->SetUpdateMode( true );
    m_pFontColorLB->SetUpdateMode( true );
    m_pFontColorLB->SetSelectHdl( LINK( this, SvxCharEffectsPage, ColorBoxSelectHdl_Impl ) );

    // handler
    Link<ListBox&,void> aLink = LINK( this, SvxCharEffectsPage, SelectListBoxHdl_Impl );
    m_pUnderlineLB->SetSelectHdl( aLink );
    m_pUnderlineColorLB->SetSelectHdl( aLink );
    m_pOverlineLB->SetSelectHdl( aLink );
    m_pOverlineColorLB->SetSelectHdl( aLink );
    m_pStrikeoutLB->SetSelectHdl( aLink );
    m_pEmphasisLB->SetSelectHdl( aLink );
    m_pPositionLB->SetSelectHdl( aLink );
    m_pEffectsLB->SetSelectHdl( aLink );
    m_pReliefLB->SetSelectHdl( aLink );

    m_pUnderlineLB->SelectEntryPos( 0 );
    m_pUnderlineColorLB->SelectEntryPos( 0 );
    m_pOverlineLB->SelectEntryPos( 0 );
    m_pOverlineColorLB->SelectEntryPos( 0 );
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
}

void SvxCharEffectsPage::UpdatePreview_Impl()
{
    SvxFont& rFont = GetPreviewFont();
    SvxFont& rCJKFont = GetPreviewCJKFont();
    SvxFont& rCTLFont = GetPreviewCTLFont();

    sal_Int32 nPos = m_pUnderlineLB->GetSelectEntryPos();
    FontUnderline eUnderline = (FontUnderline)reinterpret_cast<sal_uLong>(m_pUnderlineLB->GetEntryData( nPos ));
    nPos = m_pOverlineLB->GetSelectEntryPos();
    FontUnderline eOverline = (FontUnderline)reinterpret_cast<sal_uLong>(m_pOverlineLB->GetEntryData( nPos ));
    nPos = m_pStrikeoutLB->GetSelectEntryPos();
    FontStrikeout eStrikeout = (FontStrikeout)reinterpret_cast<sal_uLong>(m_pStrikeoutLB->GetEntryData( nPos ));
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

    nPos = m_pPositionLB->GetSelectEntryPos();
    bool bUnder = ( CHRDLG_POSITION_UNDER == reinterpret_cast<sal_uLong>(m_pPositionLB->GetEntryData( nPos )) );
    FontEmphasisMark eMark = (FontEmphasisMark)m_pEmphasisLB->GetSelectEntryPos();
    eMark |= bUnder ? EMPHASISMARK_POS_BELOW : EMPHASISMARK_POS_ABOVE;
    rFont.SetEmphasisMark( eMark );
    rCJKFont.SetEmphasisMark( eMark );
    rCTLFont.SetEmphasisMark( eMark );

    sal_Int32 nRelief = m_pReliefLB->GetSelectEntryPos();
    if(LISTBOX_ENTRY_NOTFOUND != nRelief)
    {
        rFont.SetRelief( (FontRelief)nRelief );
        rCJKFont.SetRelief( (FontRelief)nRelief );
        rCTLFont.SetRelief( (FontRelief)nRelief );
    }

    rFont.SetOutline( StateToAttr( m_pOutlineBtn->GetState() ) );
    rCJKFont.SetOutline( rFont.IsOutline() );
    rCTLFont.SetOutline( rFont.IsOutline() );

    rFont.SetShadow( StateToAttr( m_pShadowBtn->GetState() ) );
    rCJKFont.SetShadow( rFont.IsShadow() );
    rCTLFont.SetShadow( rFont.IsShadow() );

    sal_Int32 nCapsPos = m_pEffectsLB->GetSelectEntryPos();
    if ( nCapsPos != LISTBOX_ENTRY_NOTFOUND )
    {
        rFont.SetCaseMap( (SvxCaseMap)nCapsPos );
        rCJKFont.SetCaseMap( (SvxCaseMap)nCapsPos );
        // #i78474# small caps do not exist in CTL fonts
        rCTLFont.SetCaseMap( static_cast<SvxCaseMap>( nCapsPos == SVX_CASEMAP_KAPITAELCHEN ? SVX_CASEMAP_NOT_MAPPED : (SvxCaseMap)nCapsPos) );
    }

    bool bWordLine = m_pIndividualWordsBtn->IsChecked();
    rFont.SetWordLineMode( bWordLine );
    rCJKFont.SetWordLineMode( bWordLine );
    rCTLFont.SetWordLineMode( bWordLine );

    m_pPreviewWin->Invalidate();
}


void SvxCharEffectsPage::SetCaseMap_Impl( SvxCaseMap eCaseMap )
{
    if ( SVX_CASEMAP_END > eCaseMap )
        m_pEffectsLB->SelectEntryPos(
            sal::static_int_cast< sal_Int32 >( eCaseMap ) );
    else
    {
        m_pEffectsLB->SetNoSelection();
        eCaseMap = SVX_CASEMAP_NOT_MAPPED;
    }

    UpdatePreview_Impl();
}


void SvxCharEffectsPage::ResetColor_Impl( const SfxItemSet& rSet )
{
    sal_uInt16 nWhich = GetWhich( SID_ATTR_CHAR_COLOR );
    SfxItemState eState = rSet.GetItemState( nWhich );

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
            rFont.SetColor( aColor.GetColor() == COL_AUTO ? Color(COL_BLACK) : aColor );
            rCJKFont.SetColor( aColor.GetColor() == COL_AUTO ? Color(COL_BLACK) : aColor );
            rCTLFont.SetColor( aColor.GetColor() == COL_AUTO ? Color(COL_BLACK) : aColor );

            m_pPreviewWin->Invalidate();
            sal_Int32 nSelPos = m_pFontColorLB->GetEntryPos( aColor );
            if ( nSelPos == LISTBOX_ENTRY_NOTFOUND && aColor == Color( COL_TRANSPARENT ) )
                nSelPos = m_pFontColorLB->GetEntryPos( m_aTransparentColorName );

            if ( LISTBOX_ENTRY_NOTFOUND != nSelPos )
                m_pFontColorLB->SelectEntryPos( nSelPos );
            else
            {
                nSelPos = m_pFontColorLB->GetEntryPos( aColor );
                if ( LISTBOX_ENTRY_NOTFOUND != nSelPos )
                    m_pFontColorLB->SelectEntryPos( nSelPos );
                else
                    m_pFontColorLB->SelectEntryPos(
                        m_pFontColorLB->InsertEntry( aColor, OUString( SVX_RES( RID_SVXSTR_COLOR_USER ) ) ) );
            }
            break;
        }
    }
}


bool SvxCharEffectsPage::FillItemSetColor_Impl( SfxItemSet& rSet )
{
    sal_uInt16 nWhich = GetWhich( SID_ATTR_CHAR_COLOR );
    const SvxColorItem* pOld = static_cast<const SvxColorItem*>(GetOldItem( rSet, SID_ATTR_CHAR_COLOR ));
    const SvxColorItem* pItem = nullptr;
    bool bChanged = true;
    const SfxItemSet* pExampleSet = GetTabDialog() ? GetTabDialog()->GetExampleSet() : nullptr;
    const SfxItemSet& rOldSet = GetItemSet();

    Color aSelectedColor;
    if ( m_pFontColorLB->GetSelectEntry() == m_aTransparentColorName )
        aSelectedColor = Color( COL_TRANSPARENT );
    else
        aSelectedColor = m_pFontColorLB->GetSelectEntryColor();

    if ( pOld && pOld->GetValue() == aSelectedColor )
        bChanged = false;

    if ( !bChanged )
        bChanged = ( m_pFontColorLB->GetSavedValue() == LISTBOX_ENTRY_NOTFOUND );

    if ( !bChanged && pExampleSet &&
         pExampleSet->GetItemState( nWhich, false, reinterpret_cast<const SfxPoolItem**>(&pItem) ) == SfxItemState::SET &&
         pItem->GetValue() != aSelectedColor )
        bChanged = true;

    bool bModified = false;

    if ( bChanged && m_pFontColorLB->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND )
    {
        rSet.Put( SvxColorItem( aSelectedColor, nWhich ) );
        bModified = true;
    }
    else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich, false ) )
        CLEARTITEM;

    return bModified;
}


IMPL_LINK_TYPED( SvxCharEffectsPage, SelectListBoxHdl_Impl, ListBox&, rBox, void )
{
    SelectHdl_Impl(&rBox);
}
void SvxCharEffectsPage::SelectHdl_Impl( ListBox* pBox )
{
    if ( m_pEmphasisLB == pBox )
    {
        sal_Int32 nEPos = m_pEmphasisLB->GetSelectEntryPos();
        bool bEnable = ( nEPos > 0 && nEPos != LISTBOX_ENTRY_NOTFOUND );
        m_pPositionFT->Enable( bEnable );
        m_pPositionLB->Enable( bEnable );
    }
    else if( m_pReliefLB == pBox)
    {
        bool bEnable = ( pBox->GetSelectEntryPos() == 0 );
        m_pOutlineBtn->Enable( bEnable );
        m_pShadowBtn->Enable( bEnable );
    }
    else if ( m_pPositionLB.get() != pBox )
    {
        sal_Int32 nUPos = m_pUnderlineLB->GetSelectEntryPos(),
               nOPos = m_pOverlineLB->GetSelectEntryPos(),
               nSPos = m_pStrikeoutLB->GetSelectEntryPos();
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


IMPL_LINK_NOARG_TYPED(SvxCharEffectsPage, UpdatePreview_Impl, ListBox&, void)
{
    bool bEnable = ( ( m_pUnderlineLB->GetSelectEntryPos() > 0 ) ||
                     ( m_pOverlineLB->GetSelectEntryPos()  > 0 ) ||
                     ( m_pStrikeoutLB->GetSelectEntryPos() > 0 ) );
    m_pIndividualWordsBtn->Enable( bEnable );

    UpdatePreview_Impl();
}


IMPL_LINK_NOARG_TYPED(SvxCharEffectsPage, CbClickHdl_Impl, Button*, void)
{
    UpdatePreview_Impl();
}


IMPL_LINK_NOARG_TYPED(SvxCharEffectsPage, TristClickHdl_Impl, Button*, void)
{
    UpdatePreview_Impl();
}


IMPL_LINK_TYPED( SvxCharEffectsPage, ColorBoxSelectHdl_Impl, ListBox&, rListBox, void )
{
    ColorListBox* pBox = static_cast<ColorListBox*>(&rListBox);
    SvxFont& rFont = GetPreviewFont();
    SvxFont& rCJKFont = GetPreviewCJKFont();
    SvxFont& rCTLFont = GetPreviewCTLFont();

    Color aSelectedColor;
    if ( pBox->GetSelectEntry() == m_aTransparentColorName )
        aSelectedColor = Color( COL_TRANSPARENT );
    else
        aSelectedColor = pBox->GetSelectEntryColor();
    rFont.SetColor( aSelectedColor.GetColor() == COL_AUTO ? Color(COL_BLACK) : aSelectedColor );
    rCJKFont.SetColor( aSelectedColor.GetColor() == COL_AUTO ? Color(COL_BLACK) : aSelectedColor );
    rCTLFont.SetColor( aSelectedColor.GetColor() == COL_AUTO ? Color(COL_BLACK) : aSelectedColor );

    m_pPreviewWin->Invalidate();
}


SfxTabPage::sfxpg SvxCharEffectsPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( _pSet );
    return LEAVE_PAGE;
}


VclPtr<SfxTabPage> SvxCharEffectsPage::Create( vcl::Window* pParent, const SfxItemSet* rSet )
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
    rFont.SetUnderline( UNDERLINE_NONE );
    rCJKFont.SetUnderline( UNDERLINE_NONE );
    rCTLFont.SetUnderline( UNDERLINE_NONE );

    m_pUnderlineLB->SelectEntryPos( 0 );
    SfxItemState eState = rSet->GetItemState( nWhich );

    if ( eState >= SfxItemState::DONTCARE )
    {
        if ( eState == SfxItemState::DONTCARE )
            m_pUnderlineLB->SetNoSelection();
        else
        {
            const SvxUnderlineItem& rItem = static_cast<const SvxUnderlineItem&>(rSet->Get( nWhich ));
            FontUnderline eUnderline = (FontUnderline)rItem.GetValue();
            rFont.SetUnderline( eUnderline );
            rCJKFont.SetUnderline( eUnderline );
            rCTLFont.SetUnderline( eUnderline );

            if ( eUnderline != UNDERLINE_NONE )
            {
                for ( sal_Int32 i = 0; i < m_pUnderlineLB->GetEntryCount(); ++i )
                {
                    if ( (FontUnderline)reinterpret_cast<sal_uLong>(m_pUnderlineLB->GetEntryData(i)) == eUnderline )
                    {
                        m_pUnderlineLB->SelectEntryPos(i);
                        bEnable = true;
                        break;
                    }
                }

                Color aColor = rItem.GetColor();
                sal_Int32 nPos = m_pUnderlineColorLB->GetEntryPos( aColor );

                if ( LISTBOX_ENTRY_NOTFOUND != nPos )
                    m_pUnderlineColorLB->SelectEntryPos( nPos );
                else
                {
                    nPos = m_pUnderlineColorLB->GetEntryPos( aColor );
                    if ( LISTBOX_ENTRY_NOTFOUND != nPos )
                        m_pUnderlineColorLB->SelectEntryPos( nPos );
                    else
                        m_pUnderlineColorLB->SelectEntryPos(
                            m_pUnderlineColorLB->InsertEntry( aColor,
                                OUString( SVX_RES( RID_SVXSTR_COLOR_USER ) ) ) );
                }
            }
            else
            {
                m_pUnderlineColorLB->SelectEntry( Color( COL_AUTO ));
                m_pUnderlineColorLB->Disable();
            }
        }
    }

    // Overline
    nWhich = GetWhich( SID_ATTR_CHAR_OVERLINE );
    rFont.SetOverline( UNDERLINE_NONE );
    rCJKFont.SetOverline( UNDERLINE_NONE );
    rCTLFont.SetOverline( UNDERLINE_NONE );

    m_pOverlineLB->SelectEntryPos( 0 );
    eState = rSet->GetItemState( nWhich );

    if ( eState >= SfxItemState::DONTCARE )
    {
        if ( eState == SfxItemState::DONTCARE )
            m_pOverlineLB->SetNoSelection();
        else
        {
            const SvxOverlineItem& rItem = static_cast<const SvxOverlineItem&>(rSet->Get( nWhich ));
            FontUnderline eOverline = (FontUnderline)rItem.GetValue();
            rFont.SetOverline( eOverline );
            rCJKFont.SetOverline( eOverline );
            rCTLFont.SetOverline( eOverline );

            if ( eOverline != UNDERLINE_NONE )
            {
                for ( sal_Int32 i = 0; i < m_pOverlineLB->GetEntryCount(); ++i )
                {
                    if ( (FontUnderline)reinterpret_cast<sal_uLong>(m_pOverlineLB->GetEntryData(i)) == eOverline )
                    {
                        m_pOverlineLB->SelectEntryPos(i);
                        bEnable = true;
                        break;
                    }
                }

                Color aColor = rItem.GetColor();
                sal_Int32 nPos = m_pOverlineColorLB->GetEntryPos( aColor );

                if ( LISTBOX_ENTRY_NOTFOUND != nPos )
                    m_pOverlineColorLB->SelectEntryPos( nPos );
                else
                {
                    nPos = m_pOverlineColorLB->GetEntryPos( aColor );
                    if ( LISTBOX_ENTRY_NOTFOUND != nPos )
                        m_pOverlineColorLB->SelectEntryPos( nPos );
                    else
                        m_pOverlineColorLB->SelectEntryPos(
                            m_pOverlineColorLB->InsertEntry( aColor,
                                OUString( SVX_RES( RID_SVXSTR_COLOR_USER ) ) ) );
                }
            }
            else
            {
                m_pOverlineColorLB->SelectEntry( Color( COL_AUTO ));
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
            FontStrikeout eStrikeout = (FontStrikeout)rItem.GetValue();
            rFont.SetStrikeout( eStrikeout );
            rCJKFont.SetStrikeout( eStrikeout );
            rCTLFont.SetStrikeout( eStrikeout );

            if ( eStrikeout != STRIKEOUT_NONE )
            {
                for ( sal_Int32 i = 0; i < m_pStrikeoutLB->GetEntryCount(); ++i )
                {
                    if ( (FontStrikeout)reinterpret_cast<sal_uLong>(m_pStrikeoutLB->GetEntryData(i)) == eStrikeout )
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

        m_pEmphasisLB->SelectEntryPos( (sal_Int32)( eMark & EMPHASISMARK_STYLE ) );
        eMark &= ~EMPHASISMARK_STYLE;
        sal_uLong nEntryData = ( eMark == EMPHASISMARK_POS_ABOVE )
            ? CHRDLG_POSITION_OVER
            : ( eMark == EMPHASISMARK_POS_BELOW ) ? CHRDLG_POSITION_UNDER : 0;

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
    SvxCaseMap eCaseMap = SVX_CASEMAP_END;
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
            eCaseMap = (SvxCaseMap)rItem.GetValue();
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
            m_pReliefLB->SelectEntryPos(rItem.GetValue());
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
            m_pOutlineBtn->SetState( (TriState)rItem.GetValue() );
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
            m_pShadowBtn->SetState( (TriState)rItem.GetValue() );
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
            m_pBlinkingBtn->SetState( (TriState)rItem.GetValue() );
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
            m_pHiddenBtn->SetState( (TriState)rItem.GetValue() );
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

void  SvxCharEffectsPage::ChangesApplied()
{
    m_pUnderlineLB->SaveValue();
    m_pUnderlineColorLB->SaveValue();
    m_pOverlineLB->SaveValue();
    m_pOverlineColorLB->SaveValue();
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
    m_pFontColorLB->SaveValue();

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
    sal_Int32 nPos = m_pUnderlineLB->GetSelectEntryPos();
    FontUnderline eUnder = (FontUnderline)reinterpret_cast<sal_uLong>(m_pUnderlineLB->GetEntryData( nPos ));

    if ( pOld )
    {
        //! if there are different underline styles in the selection the
        //! item-state in the 'rOldSet' will be invalid. In this case
        //! changing the underline style will be allowed if a style is
        //! selected in the listbox.
        bool bAllowChg = LISTBOX_ENTRY_NOTFOUND != nPos  &&
                         SfxItemState::DEFAULT > rOldSet.GetItemState( nWhich );

        const SvxUnderlineItem& rItem = *static_cast<const SvxUnderlineItem*>(pOld);
        if ( (FontUnderline)rItem.GetValue() == eUnder &&
             ( UNDERLINE_NONE == eUnder || rItem.GetColor() == m_pUnderlineColorLB->GetSelectEntryColor() ) &&
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
    nPos = m_pOverlineLB->GetSelectEntryPos();
    FontUnderline eOver = (FontUnderline)reinterpret_cast<sal_uLong>(m_pOverlineLB->GetEntryData( nPos ));

    if ( pOld )
    {
        //! if there are different underline styles in the selection the
        //! item-state in the 'rOldSet' will be invalid. In this case
        //! changing the underline style will be allowed if a style is
        //! selected in the listbox.
        bool bAllowChg = LISTBOX_ENTRY_NOTFOUND != nPos  &&
                         SfxItemState::DEFAULT > rOldSet.GetItemState( nWhich );

        const SvxOverlineItem& rItem = *static_cast<const SvxOverlineItem*>(pOld);
        if ( (FontUnderline)rItem.GetValue() == eOver &&
             ( UNDERLINE_NONE == eOver || rItem.GetColor() == m_pOverlineColorLB->GetSelectEntryColor() ) &&
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
    nPos = m_pStrikeoutLB->GetSelectEntryPos();
    FontStrikeout eStrike = (FontStrikeout)reinterpret_cast<sal_uLong>(m_pStrikeoutLB->GetEntryData( nPos ));

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
            || ((FontStrikeout)rItem.GetValue() == eStrike  && !bAllowChg) )
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
        if ( rItem.GetValue() == (bool) m_pIndividualWordsBtn->IsChecked() )
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
    sal_Int32 nMarkPos = m_pEmphasisLB->GetSelectEntryPos();
    sal_Int32 nPosPos = m_pPositionLB->GetSelectEntryPos();
    FontEmphasisMark eMark = (FontEmphasisMark)nMarkPos;
    if ( m_pPositionLB->IsEnabled() )
    {
        eMark |= ( CHRDLG_POSITION_UNDER == reinterpret_cast<sal_uLong>(m_pPositionLB->GetEntryData( nPosPos )) )
            ? EMPHASISMARK_POS_BELOW : EMPHASISMARK_POS_ABOVE;
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
    SvxCaseMap eCaseMap = SVX_CASEMAP_NOT_MAPPED;
    bool bChecked = false;
    sal_Int32 nCapsPos = m_pEffectsLB->GetSelectEntryPos();
    if ( nCapsPos != LISTBOX_ENTRY_NOTFOUND )
    {
        eCaseMap = (SvxCaseMap)nCapsPos;
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
        if ( (SvxCaseMap)rItem.GetValue() == eCaseMap  &&  !bAllowChg )
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
        SvxCharReliefItem aRelief((FontRelief)m_pReliefLB->GetSelectEntryPos(), nWhich);
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

void SvxCharEffectsPage::EnableFlash()
{
    m_pBlinkingBtn->Show();
}


void SvxCharEffectsPage::SetPreviewBackgroundToCharacter()
{
    m_bPreviewBackgroundToCharacter = true;
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
            EnableFlash();
        if ( ( nFlags & SVX_PREVIEW_CHARACTER ) == SVX_PREVIEW_CHARACTER )
            SetPreviewBackgroundToCharacter();
    }
}

// class SvxCharPositionPage ---------------------------------------------

SvxCharPositionPage::SvxCharPositionPage( vcl::Window* pParent, const SfxItemSet& rInSet )
    : SvxCharBasePage(pParent, "PositionPage", "cui/ui/positionpage.ui", rInSet)
    , m_nSuperEsc((short)DFLT_ESC_SUPER)
    , m_nSubEsc((short)DFLT_ESC_SUB)
    , m_nScaleWidthItemSetVal(100)
    , m_nScaleWidthInitialVal(100)
    , m_nSuperProp((sal_uInt8)DFLT_ESC_PROP)
    , m_nSubProp((sal_uInt8)DFLT_ESC_PROP)
{
    get(m_pHighPosBtn, "superscript");
    get(m_pNormalPosBtn, "normal");
    get(m_pLowPosBtn, "subscript");
    get(m_pHighLowFT, "raiselower");
    get(m_pHighLowMF, "raiselowersb");
    get(m_pHighLowRB, "automatic");
    get(m_pFontSizeFT, "relativefontsize");
    get(m_pFontSizeMF, "fontsizesb");
    get(m_pRotationContainer, "rotationcontainer");
    get(m_pScalingFT, "rotate");
    get(m_pScalingAndRotationFT, "rotateandscale");
    get(m_p0degRB, "0deg");
    get(m_p90degRB, "90deg");
    get(m_p270degRB, "270deg");
    get(m_pFitToLineCB, "fittoline");
    get(m_pScaleWidthMF, "scalewidthsb");
    get(m_pKerningLB, "kerninglb");
    get(m_pKerningFT, "kerningft");
    get(m_pKerningMF, "kerningsb");
    get(m_pPairKerningBtn, "pairkerning");

    get(m_pPreviewWin, "preview");

    Initialize();
}

SvxCharPositionPage::~SvxCharPositionPage()
{
    disposeOnce();
}

void SvxCharPositionPage::dispose()
{
    m_pHighPosBtn.clear();
    m_pNormalPosBtn.clear();
    m_pLowPosBtn.clear();
    m_pHighLowFT.clear();
    m_pHighLowMF.clear();
    m_pHighLowRB.clear();
    m_pFontSizeFT.clear();
    m_pFontSizeMF.clear();
    m_pRotationContainer.clear();
    m_pScalingFT.clear();
    m_pScalingAndRotationFT.clear();
    m_p0degRB.clear();
    m_p90degRB.clear();
    m_p270degRB.clear();
    m_pFitToLineCB.clear();
    m_pScaleWidthMF.clear();
    m_pKerningLB.clear();
    m_pKerningFT.clear();
    m_pKerningMF.clear();
    m_pPairKerningBtn.clear();
    SvxCharBasePage::dispose();
}


void SvxCharPositionPage::Initialize()
{
    // to handle the changes of the other pages
    SetExchangeSupport();

    GetPreviewFont().SetSize( Size( 0, 240 ) );
    GetPreviewCJKFont().SetSize( Size( 0, 240 ) );
    GetPreviewCTLFont().SetSize( Size( 0, 240 ) );

    m_pNormalPosBtn->Check();
    PositionHdl_Impl( m_pNormalPosBtn );
    m_pKerningLB->SelectEntryPos( 0 );
    KerningSelectHdl_Impl( *m_pKerningLB );

    Link<Button*,void> aLink2 = LINK( this, SvxCharPositionPage, PositionHdl_Impl );
    m_pHighPosBtn->SetClickHdl( aLink2 );
    m_pNormalPosBtn->SetClickHdl( aLink2 );
    m_pLowPosBtn->SetClickHdl( aLink2 );

    aLink2 = LINK( this, SvxCharPositionPage, RotationHdl_Impl );
    m_p0degRB->SetClickHdl( aLink2 );
    m_p90degRB->SetClickHdl( aLink2 );
    m_p270degRB->SetClickHdl( aLink2 );

    Link<Edit&,void> aLink = LINK( this, SvxCharPositionPage, FontModifyHdl_Impl );
    m_pHighLowMF->SetModifyHdl( aLink );
    m_pFontSizeMF->SetModifyHdl( aLink );

    Link<Control&,void> aLink3 = LINK( this, SvxCharPositionPage, LoseFocusHdl_Impl );
    m_pHighLowMF->SetLoseFocusHdl( aLink3 );
    m_pFontSizeMF->SetLoseFocusHdl( aLink3 );

    m_pHighLowRB->SetClickHdl( LINK( this, SvxCharPositionPage, AutoPositionHdl_Impl ) );
    m_pFitToLineCB->SetClickHdl( LINK( this, SvxCharPositionPage, FitToLineHdl_Impl ) );
    m_pKerningLB->SetSelectHdl( LINK( this, SvxCharPositionPage, KerningSelectHdl_Impl ) );
    m_pKerningMF->SetModifyHdl( LINK( this, SvxCharPositionPage, KerningModifyHdl_Impl ) );
    m_pScaleWidthMF->SetModifyHdl( LINK( this, SvxCharPositionPage, ScaleWidthModifyHdl_Impl ) );
}

void SvxCharPositionPage::UpdatePreview_Impl( sal_uInt8 nProp, sal_uInt8 nEscProp, short nEsc )
{
    SetPrevFontEscapement( nProp, nEscProp, nEsc );
}


void SvxCharPositionPage::SetEscapement_Impl( sal_uInt16 nEsc )
{
    SvxEscapementItem aEscItm( (SvxEscapement)nEsc, SID_ATTR_CHAR_ESCAPEMENT );

    if ( SVX_ESCAPEMENT_SUPERSCRIPT == nEsc )
    {
        aEscItm.GetEsc() = m_nSuperEsc;
        aEscItm.GetProp() = m_nSuperProp;
    }
    else if ( SVX_ESCAPEMENT_SUBSCRIPT == nEsc )
    {
        aEscItm.GetEsc() = m_nSubEsc;
        aEscItm.GetProp() = m_nSubProp;
    }

    short nFac = aEscItm.GetEsc() < 0 ? -1 : 1;

    m_pHighLowMF->SetValue( aEscItm.GetEsc() * nFac );
    m_pFontSizeMF->SetValue( aEscItm.GetProp() );

    if ( SVX_ESCAPEMENT_OFF == nEsc )
    {
        m_pHighLowFT->Disable();
        m_pHighLowMF->Disable();
        m_pFontSizeFT->Disable();
        m_pFontSizeMF->Disable();
        m_pHighLowRB->Disable();
    }
    else
    {
        m_pFontSizeFT->Enable();
        m_pFontSizeMF->Enable();
        m_pHighLowRB->Enable();

        if ( !m_pHighLowRB->IsChecked() )
        {
            m_pHighLowFT->Enable();
            m_pHighLowMF->Enable();
        }
        else
            AutoPositionHdl_Impl( m_pHighLowRB );
    }

    UpdatePreview_Impl( 100, aEscItm.GetProp(), aEscItm.GetEsc() );
}


IMPL_LINK_TYPED( SvxCharPositionPage, PositionHdl_Impl, Button*, pBtn, void )
{
    sal_uInt16 nEsc = SVX_ESCAPEMENT_OFF;   // also when pBtn == NULL

    if ( m_pHighPosBtn == pBtn )
        nEsc = SVX_ESCAPEMENT_SUPERSCRIPT;
    else if ( m_pLowPosBtn == pBtn )
        nEsc = SVX_ESCAPEMENT_SUBSCRIPT;

    SetEscapement_Impl( nEsc );
}


IMPL_LINK_TYPED( SvxCharPositionPage, RotationHdl_Impl, Button*, pBtn, void )
{
    bool bEnable = false;
    if (m_p90degRB == pBtn  || m_p270degRB == pBtn)
        bEnable = true;
    else
        OSL_ENSURE( m_p0degRB == pBtn, "unexpected button" );
    m_pFitToLineCB->Enable( bEnable );
}


IMPL_LINK_NOARG_TYPED(SvxCharPositionPage, FontModifyHdl_Impl, Edit&, void)
{
    sal_uInt8 nEscProp = (sal_uInt8)m_pFontSizeMF->GetValue();
    short nEsc  = (short)m_pHighLowMF->GetValue();
    nEsc *= m_pLowPosBtn->IsChecked() ? -1 : 1;
    UpdatePreview_Impl( 100, nEscProp, nEsc );
}


IMPL_LINK_TYPED( SvxCharPositionPage, AutoPositionHdl_Impl, Button*, pBox, void )
{
    if ( static_cast<CheckBox*>(pBox)->IsChecked() )
    {
        m_pHighLowFT->Disable();
        m_pHighLowMF->Disable();
    }
    else
        PositionHdl_Impl( m_pHighPosBtn->IsChecked() ? m_pHighPosBtn
                                                      : m_pLowPosBtn->IsChecked() ? m_pLowPosBtn
                                                                                   : m_pNormalPosBtn );
}


IMPL_LINK_TYPED( SvxCharPositionPage, FitToLineHdl_Impl, Button*, pBox, void )
{
    if (m_pFitToLineCB == pBox)
    {
        sal_uInt16 nVal = m_nScaleWidthInitialVal;
        if (m_pFitToLineCB->IsChecked())
            nVal = m_nScaleWidthItemSetVal;
        m_pScaleWidthMF->SetValue( nVal );

        m_pPreviewWin->SetFontWidthScale( nVal );
    }
}


IMPL_LINK_NOARG_TYPED(SvxCharPositionPage, KerningSelectHdl_Impl, ListBox&, void)
{
    if ( m_pKerningLB->GetSelectEntryPos() > LW_NORMAL )
    {
        m_pKerningFT->Enable();
        m_pKerningMF->Enable();

        if ( m_pKerningLB->GetSelectEntryPos() == LW_CONDENSED )
        {
            // Condensed -> max value == 1/6 of the current font height
            SvxFont& rFont = GetPreviewFont();
            long nMax = rFont.GetSize().Height() / 6;
            m_pKerningMF->SetMax( m_pKerningMF->Normalize( nMax ), FUNIT_TWIP );
            m_pKerningMF->SetLast( m_pKerningMF->GetMax( m_pKerningMF->GetUnit() ) );
        }
        else
        {
            m_pKerningMF->SetMax( 9999 );
            m_pKerningMF->SetLast( 9999 );
        }
    }
    else
    {
        m_pKerningMF->SetValue( 0 );
        m_pKerningFT->Disable();
        m_pKerningMF->Disable();
    }

    KerningModifyHdl_Impl( *m_pKerningMF );
}


IMPL_LINK_NOARG_TYPED(SvxCharPositionPage, KerningModifyHdl_Impl, Edit&, void)
{
    long nVal = static_cast<long>(m_pKerningMF->GetValue());
    nVal = LogicToLogic( nVal, MAP_POINT, MAP_TWIP );
    long nKern = (short)m_pKerningMF->Denormalize( nVal );

    // Condensed? -> then negative
    if ( m_pKerningLB->GetSelectEntryPos() == LW_CONDENSED )
        nKern *= -1;

    SvxFont& rFont = GetPreviewFont();
    SvxFont& rCJKFont = GetPreviewCJKFont();
    SvxFont& rCTLFont = GetPreviewCTLFont();

    rFont.SetFixKerning( (short)nKern );
    rCJKFont.SetFixKerning( (short)nKern );
    rCTLFont.SetFixKerning( (short)nKern );
    m_pPreviewWin->Invalidate();
}


IMPL_LINK_TYPED( SvxCharPositionPage, LoseFocusHdl_Impl, Control&, rControl, void )
{
    MetricField* pField = static_cast<MetricField*>(&rControl);
    bool bHigh = m_pHighPosBtn->IsChecked();
    bool bLow = m_pLowPosBtn->IsChecked();
    DBG_ASSERT( bHigh || bLow, "normal position is not valid" );

    if ( m_pHighLowMF == pField )
    {
        if ( bLow )
            m_nSubEsc = (short)m_pHighLowMF->GetValue() * -1;
        else
            m_nSuperEsc = (short)m_pHighLowMF->GetValue();
    }
    else if ( m_pFontSizeMF == pField )
    {
        if ( bLow )
            m_nSubProp = (sal_uInt8)m_pFontSizeMF->GetValue();
        else
            m_nSuperProp = (sal_uInt8)m_pFontSizeMF->GetValue();
    }
}


IMPL_LINK_NOARG_TYPED(SvxCharPositionPage, ScaleWidthModifyHdl_Impl, Edit&, void)
{
    m_pPreviewWin->SetFontWidthScale( sal_uInt16( m_pScaleWidthMF->GetValue() ) );
}

void  SvxCharPositionPage::ActivatePage( const SfxItemSet& rSet )
{
    //update the preview
    SvxCharBasePage::ActivatePage( rSet );

    //the only thing that has to be checked is the max. allowed value for the
    //condense edit field
    if ( m_pKerningLB->GetSelectEntryPos() == LW_CONDENSED )
    {
        // Condensed -> max value == 1/6 of the current font height
        SvxFont& rFont = GetPreviewFont();
        long nMax = rFont.GetSize().Height() / 6;
        long nKern = (short)m_pKerningMF->Denormalize( LogicToLogic( static_cast<long>(m_pKerningMF->GetValue()), MAP_POINT, MAP_TWIP ) );
        m_pKerningMF->SetMax( m_pKerningMF->Normalize( nKern > nMax ? nKern : nMax ), FUNIT_TWIP );
        m_pKerningMF->SetLast( m_pKerningMF->GetMax( m_pKerningMF->GetUnit() ) );
    }
}


SfxTabPage::sfxpg SvxCharPositionPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( _pSet );
    return LEAVE_PAGE;
}


VclPtr<SfxTabPage> SvxCharPositionPage::Create( vcl::Window* pParent, const SfxItemSet* rSet )
{
    return VclPtr<SvxCharPositionPage>::Create( pParent, *rSet );
}


void SvxCharPositionPage::Reset( const SfxItemSet* rSet )
{
    OUString sUser = GetUserData();

    if ( !sUser.isEmpty() )
    {
        m_nSuperEsc = (short)sUser.getToken( 0, ';' ).toInt32();
        m_nSubEsc = (short)sUser.getToken( 1, ';' ).toInt32();
        m_nSuperProp = (sal_uInt8)sUser.getToken( 2, ';' ).toInt32();
        m_nSubProp = (sal_uInt8)sUser.getToken( 3, ';' ).toInt32();

        //fdo#75307 validate all the entries and discard all of them if any are
        //out of range
        bool bValid = true;
        if (m_nSuperEsc < m_pHighLowMF->GetMin() || m_nSuperEsc > m_pHighLowMF->GetMax())
            bValid = false;
        if (m_nSubEsc*-1 < m_pHighLowMF->GetMin() || m_nSubEsc*-1 > m_pHighLowMF->GetMax())
            bValid = false;
        if (m_nSuperProp < m_pFontSizeMF->GetMin() || m_nSuperProp > m_pFontSizeMF->GetMax())
            bValid = false;
        if (m_nSubProp < m_pFontSizeMF->GetMin() || m_nSubProp > m_pFontSizeMF->GetMax())
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
    sal_uInt8 nProp = 100;

    m_pHighLowFT->Disable();
    m_pHighLowMF->Disable();
    m_pFontSizeFT->Disable();
    m_pFontSizeMF->Disable();

    SvxFont& rFont = GetPreviewFont();
    SvxFont& rCJKFont = GetPreviewCJKFont();
    SvxFont& rCTLFont = GetPreviewCTLFont();
    sal_uInt16 nWhich = GetWhich( SID_ATTR_CHAR_ESCAPEMENT );

    if ( rSet->GetItemState( nWhich ) >= SfxItemState::DEFAULT )
    {
        const SvxEscapementItem& rItem = static_cast<const SvxEscapementItem&>(rSet->Get( nWhich ));
        nEsc = rItem.GetEsc();
        nEscProp = rItem.GetProp();

        if ( nEsc != 0 )
        {
            m_pHighLowFT->Enable();
            m_pHighLowMF->Enable();
            m_pFontSizeFT->Enable();
            m_pFontSizeMF->Enable();

            short nFac;
            bool bAutomatic(false);

            if ( nEsc > 0 )
            {
                nFac = 1;
                m_pHighPosBtn->Check();
                if ( nEsc == DFLT_ESC_AUTO_SUPER )
                {
                    nEsc = DFLT_ESC_SUPER;
                    bAutomatic = true;
                }
            }
            else
            {
                nFac = -1;
                m_pLowPosBtn->Check();
                if ( nEsc == DFLT_ESC_AUTO_SUB )
                {
                    nEsc = DFLT_ESC_SUB;
                    bAutomatic = true;
                }
            }
            if (!m_pHighLowRB->IsEnabled())
            {
                m_pHighLowRB->Enable();
            }
            m_pHighLowRB->Check(bAutomatic);

            if ( m_pHighLowRB->IsChecked() )
            {
                m_pHighLowFT->Disable();
                m_pHighLowMF->Disable();
            }
            m_pHighLowMF->SetValue( m_pHighLowMF->Normalize( nFac * nEsc ) );
        }
        else
        {
            m_pNormalPosBtn->Check();
            m_pHighLowRB->Check();
            PositionHdl_Impl( nullptr );
        }
        //the height has to be set after the handler is called to keep the value also if the escapement is zero
        m_pFontSizeMF->SetValue( m_pFontSizeMF->Normalize( nEscProp ) );
    }
    else
    {
        m_pHighPosBtn->Check( false );
        m_pNormalPosBtn->Check( false );
        m_pLowPosBtn->Check( false );
    }

    // set BspFont
    SetPrevFontEscapement( nProp, nEscProp, nEsc );

    // Kerning
    nWhich = GetWhich( SID_ATTR_CHAR_KERNING );

    if ( rSet->GetItemState( nWhich ) >= SfxItemState::DEFAULT )
    {
        const SvxKerningItem& rItem = static_cast<const SvxKerningItem&>(rSet->Get( nWhich ));
        SfxMapUnit eUnit = rSet->GetPool()->GetMetric( nWhich );
        MapUnit eOrgUnit = (MapUnit)eUnit;
        MapUnit ePntUnit( MAP_POINT );
        long nBig = static_cast<long>(m_pKerningMF->Normalize( static_cast<long>(rItem.GetValue()) ));
        long nKerning = LogicToLogic( nBig, eOrgUnit, ePntUnit );

        // set Kerning at the Font, convert into Twips before
        long nKern = LogicToLogic( rItem.GetValue(), (MapUnit)eUnit, MAP_TWIP );
        rFont.SetFixKerning( (short)nKern );
        rCJKFont.SetFixKerning( (short)nKern );
        rCTLFont.SetFixKerning( (short)nKern );

        if ( nKerning > 0 )
        {
            m_pKerningLB->SelectEntryPos( LW_EXPANDED );
        }
        else if ( nKerning < 0 )
        {
            m_pKerningLB->SelectEntryPos( LW_CONDENSED );
            nKerning = -nKerning;
        }
        else
        {
            nKerning = 0;
            m_pKerningLB->SelectEntryPos( LW_NORMAL );
        }
        //enable/disable and set min/max of the Edit
        KerningSelectHdl_Impl(*m_pKerningLB);
        //the attribute value must be displayed also if it's above the maximum allowed value
        long nVal = static_cast<long>(m_pKerningMF->GetMax());
        if(nVal < nKerning)
            m_pKerningMF->SetMax( nKerning );
        m_pKerningMF->SetValue( nKerning );
    }
    else
        m_pKerningMF->SetText( OUString() );

    // Pair kerning
    nWhich = GetWhich( SID_ATTR_CHAR_AUTOKERN );

    if ( rSet->GetItemState( nWhich ) >= SfxItemState::DEFAULT )
    {
        const SvxAutoKernItem& rItem = static_cast<const SvxAutoKernItem&>(rSet->Get( nWhich ));
        m_pPairKerningBtn->Check( rItem.GetValue() );
    }
    else
        m_pPairKerningBtn->Check( false );

    // Scale Width
    nWhich = GetWhich( SID_ATTR_CHAR_SCALEWIDTH );
    if ( rSet->GetItemState( nWhich ) >= SfxItemState::DEFAULT )
    {
        const SvxCharScaleWidthItem& rItem = static_cast<const SvxCharScaleWidthItem&>( rSet->Get( nWhich ) );
        m_nScaleWidthInitialVal = rItem.GetValue();
        m_pScaleWidthMF->SetValue( m_nScaleWidthInitialVal );
    }
    else
        m_pScaleWidthMF->SetValue( 100 );

    nWhich = GetWhich( SID_ATTR_CHAR_WIDTH_FIT_TO_LINE );
    if ( rSet->GetItemState( nWhich ) >= SfxItemState::DEFAULT )
        m_nScaleWidthItemSetVal = static_cast<const SfxUInt16Item&>( rSet->Get( nWhich )).GetValue();

    // Rotation
    nWhich = GetWhich( SID_ATTR_CHAR_ROTATED );
    SfxItemState eState = rSet->GetItemState( nWhich );
    if( SfxItemState::UNKNOWN == eState )
    {
        m_pRotationContainer->Hide();
        m_pScalingAndRotationFT->Hide();
        m_pScalingFT->Show();
    }
    else
    {
        m_pRotationContainer->Show();
        m_pScalingAndRotationFT->Show();
        m_pScalingFT->Hide();

        Link<Button*,void> aOldLink( m_pFitToLineCB->GetClickHdl() );
        m_pFitToLineCB->SetClickHdl( Link<Button*,void>() );
        if( eState >= SfxItemState::DEFAULT )
        {
            const SvxCharRotateItem& rItem =
                    static_cast<const SvxCharRotateItem&>( rSet->Get( nWhich ));
            if (rItem.IsBottomToTop())
                m_p90degRB->Check();
            else if (rItem.IsTopToBotton())
                m_p270degRB->Check();
            else
            {
                DBG_ASSERT( 0 == rItem.GetValue(), "incorrect value" );
                m_p0degRB->Check();
            }
            m_pFitToLineCB->Check( rItem.IsFitToLine() );
        }
        else
        {
            if( eState == SfxItemState::DONTCARE )
            {
                m_p0degRB->Check( false );
                m_p90degRB->Check( false );
                m_p270degRB->Check( false );
            }
            else
                m_p0degRB->Check();

            m_pFitToLineCB->Check( false );
        }
        m_pFitToLineCB->SetClickHdl( aOldLink );
        m_pFitToLineCB->Enable( !m_p0degRB->IsChecked() );

        // is this value set?
        if( SfxItemState::UNKNOWN == rSet->GetItemState( GetWhich(
                                        SID_ATTR_CHAR_WIDTH_FIT_TO_LINE ) ))
            m_pFitToLineCB->Hide();
    }
    ChangesApplied();
}

void SvxCharPositionPage::ChangesApplied()
{
    m_pHighPosBtn->SaveValue();
    m_pNormalPosBtn->SaveValue();
    m_pLowPosBtn->SaveValue();
    m_pHighLowRB->SaveValue();
    m_p0degRB->SaveValue();
    m_p90degRB->SaveValue();
    m_p270degRB->SaveValue();
    m_pFitToLineCB->SaveValue();
    m_pScaleWidthMF->SaveValue();
    m_pKerningLB->SaveValue();
    m_pKerningMF->SaveValue();
    m_pPairKerningBtn->SaveValue();
}


bool SvxCharPositionPage::FillItemSet( SfxItemSet* rSet )
{
    //  Position (high, normal or low)
    const SfxItemSet& rOldSet = GetItemSet();
    bool bModified = false, bChanged = true;
    sal_uInt16 nWhich = GetWhich( SID_ATTR_CHAR_ESCAPEMENT );
    const SfxPoolItem* pOld = GetOldItem( *rSet, SID_ATTR_CHAR_ESCAPEMENT );
    const bool bHigh = m_pHighPosBtn->IsChecked();
    short nEsc;
    sal_uInt8  nEscProp;

    if ( bHigh || m_pLowPosBtn->IsChecked() )
    {
        if ( m_pHighLowRB->IsChecked() )
            nEsc = bHigh ? DFLT_ESC_AUTO_SUPER : DFLT_ESC_AUTO_SUB;
        else
        {
            nEsc = (short)m_pHighLowMF->Denormalize( m_pHighLowMF->GetValue() );
            nEsc *= (bHigh ? 1 : -1);
        }
        nEscProp = (sal_uInt8)m_pFontSizeMF->Denormalize( m_pFontSizeMF->GetValue() );
    }
    else
    {
        nEsc  = 0;
        nEscProp = 100;
    }

    if ( pOld )
    {
        const SvxEscapementItem& rItem = *static_cast<const SvxEscapementItem*>(pOld);
        if ( rItem.GetEsc() == nEsc && rItem.GetProp() == nEscProp  )
            bChanged = false;
    }

    if ( !bChanged && !m_pHighPosBtn->GetSavedValue() &&
         !m_pNormalPosBtn->GetSavedValue() && !m_pLowPosBtn->GetSavedValue() )
        bChanged = true;

    if ( bChanged &&
         ( m_pHighPosBtn->IsChecked() || m_pNormalPosBtn->IsChecked() || m_pLowPosBtn->IsChecked() ) )
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
    sal_Int32 nPos = m_pKerningLB->GetSelectEntryPos();
    short nKerning = 0;
    SfxMapUnit eUnit = rSet->GetPool()->GetMetric( nWhich );

    if ( nPos == LW_EXPANDED || nPos == LW_CONDENSED )
    {
        long nTmp = static_cast<long>(m_pKerningMF->GetValue());
        long nVal = LogicToLogic( nTmp, MAP_POINT, (MapUnit)eUnit );
        nKerning = (short)m_pKerningMF->Denormalize( nVal );

        if ( nPos == LW_CONDENSED )
            nKerning *= - 1;
    }

    if ( pOld )
    {
        const SvxKerningItem& rItem = *static_cast<const SvxKerningItem*>(pOld);
        if ( rItem.GetValue() == nKerning )
            bChanged = false;
    }

    if ( !bChanged &&
         ( m_pKerningLB->GetSavedValue() == LISTBOX_ENTRY_NOTFOUND ||
           ( m_pKerningMF->GetSavedValue().isEmpty() && m_pKerningMF->IsEnabled() ) ) )
        bChanged = true;

    if ( bChanged && nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        rSet->Put( SvxKerningItem( nKerning, nWhich ) );
        bModified = true;
    }
    else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich, false ) )
        rSet->InvalidateItem(nWhich);

    // Pair-Kerning
    nWhich = GetWhich( SID_ATTR_CHAR_AUTOKERN );

    if ( m_pPairKerningBtn->IsValueChangedFromSaved() )
    {
        rSet->Put( SvxAutoKernItem( m_pPairKerningBtn->IsChecked(), nWhich ) );
        bModified = true;
    }
    else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich, false ) )
        rSet->InvalidateItem(nWhich);

    // Scale Width
    nWhich = GetWhich( SID_ATTR_CHAR_SCALEWIDTH );
    if ( m_pScaleWidthMF->IsValueChangedFromSaved() )
    {
        rSet->Put( SvxCharScaleWidthItem( (sal_uInt16)m_pScaleWidthMF->GetValue(), nWhich ) );
        bModified = true;
    }
    else if ( SfxItemState::DEFAULT == rOldSet.GetItemState( nWhich, false ) )
        rSet->InvalidateItem(nWhich);

    // Rotation
    nWhich = GetWhich( SID_ATTR_CHAR_ROTATED );
    if ( m_p0degRB->IsValueChangedFromSaved()  ||
         m_p90degRB->IsValueChangedFromSaved()  ||
         m_p270degRB->IsValueChangedFromSaved()  ||
         m_pFitToLineCB->IsValueChangedFromSaved() )
    {
        SvxCharRotateItem aItem( 0, m_pFitToLineCB->IsChecked(), nWhich );
        if (m_p90degRB->IsChecked())
            aItem.SetBottomToTop();
        else if (m_p270degRB->IsChecked())
            aItem.SetTopToBotton();
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


void SvxCharPositionPage::SetPreviewBackgroundToCharacter()
{
    m_bPreviewBackgroundToCharacter = true;
}

void SvxCharPositionPage::PageCreated(const SfxAllItemSet& aSet)
{
    const SfxUInt32Item* pFlagItem = aSet.GetItem<SfxUInt32Item>(SID_FLAG_TYPE, false);
    if (pFlagItem)
    {
        sal_uInt32 nFlags=pFlagItem->GetValue();
        if ( ( nFlags & SVX_PREVIEW_CHARACTER ) == SVX_PREVIEW_CHARACTER )
            SetPreviewBackgroundToCharacter();
    }
}
// class SvxCharTwoLinesPage ------------------------------------------------

SvxCharTwoLinesPage::SvxCharTwoLinesPage(vcl::Window* pParent, const SfxItemSet& rInSet)
    : SvxCharBasePage(pParent, "TwoLinesPage", "cui/ui/twolinespage.ui", rInSet)
    , m_nStartBracketPosition( 0 )
    , m_nEndBracketPosition( 0 )
{
    get(m_pTwoLinesBtn, "twolines");
    get(m_pEnclosingFrame, "enclosing");
    get(m_pStartBracketLB, "startbracket");
    get(m_pEndBracketLB, "endbracket");

    get(m_pPreviewWin, "preview");

    Initialize();
}

SvxCharTwoLinesPage::~SvxCharTwoLinesPage()
{
    disposeOnce();
}

void SvxCharTwoLinesPage::dispose()
{
    m_pTwoLinesBtn.clear();
    m_pEnclosingFrame.clear();
    m_pStartBracketLB.clear();
    m_pEndBracketLB.clear();
    SvxCharBasePage::dispose();
}

void SvxCharTwoLinesPage::Initialize()
{
    m_pTwoLinesBtn->Check( false );
    TwoLinesHdl_Impl( nullptr );

    m_pTwoLinesBtn->SetClickHdl( LINK( this, SvxCharTwoLinesPage, TwoLinesHdl_Impl ) );

    Link<ListBox&,void> aLink = LINK( this, SvxCharTwoLinesPage, CharacterMapHdl_Impl );
    m_pStartBracketLB->SetSelectHdl( aLink );
    m_pEndBracketLB->SetSelectHdl( aLink );

    SvxFont& rFont = GetPreviewFont();
    SvxFont& rCJKFont = GetPreviewCJKFont();
    SvxFont& rCTLFont = GetPreviewCTLFont();
    rFont.SetSize( Size( 0, 220 ) );
    rCJKFont.SetSize( Size( 0, 220 ) );
    rCTLFont.SetSize( Size( 0, 220 ) );
}


void SvxCharTwoLinesPage::SelectCharacter( ListBox* pBox )
{
    bool bStart = pBox == m_pStartBracketLB;
    VclPtrInstance< SvxCharacterMap > aDlg( this );
    aDlg->DisableFontSelection();

    if ( aDlg->Execute() == RET_OK )
    {
        sal_Unicode cChar = (sal_Unicode) aDlg->GetChar();
        SetBracket( cChar, bStart );
    }
    else
    {
        pBox->SelectEntryPos( bStart ? m_nStartBracketPosition : m_nEndBracketPosition );
    }
}


void SvxCharTwoLinesPage::SetBracket( sal_Unicode cBracket, bool bStart )
{
    sal_Int32 nEntryPos = 0;
    ListBox* pBox = bStart ? m_pStartBracketLB : m_pEndBracketLB;
    if ( 0 == cBracket )
        pBox->SelectEntryPos(0);
    else
    {
        bool bFound = false;
        for ( sal_Int32 i = 1; i < pBox->GetEntryCount(); ++i )
        {
            if ( reinterpret_cast<sal_uLong>(pBox->GetEntryData(i)) != CHRDLG_ENCLOSE_SPECIAL_CHAR )
            {
                const sal_Unicode cChar = pBox->GetEntry(i)[0];
                if ( cChar == cBracket )
                {
                    pBox->SelectEntryPos(i);
                    nEntryPos = i;
                    bFound = true;
                    break;
                }
            }
        }

        if ( !bFound )
        {
            nEntryPos = pBox->InsertEntry( OUString(cBracket) );
            pBox->SelectEntryPos( nEntryPos );
        }
    }
    if( bStart )
        m_nStartBracketPosition = nEntryPos;
    else
        m_nEndBracketPosition = nEntryPos;
}


IMPL_LINK_NOARG_TYPED(SvxCharTwoLinesPage, TwoLinesHdl_Impl, Button*, void)
{
    bool bChecked = m_pTwoLinesBtn->IsChecked();
    m_pEnclosingFrame->Enable( bChecked );

    UpdatePreview_Impl();
}


IMPL_LINK_TYPED( SvxCharTwoLinesPage, CharacterMapHdl_Impl, ListBox&, rBox, void )
{
    sal_Int32 nPos = rBox.GetSelectEntryPos();
    if ( CHRDLG_ENCLOSE_SPECIAL_CHAR == reinterpret_cast<sal_uLong>(rBox.GetEntryData( nPos )) )
        SelectCharacter( &rBox );
    else
    {
        bool bStart = &rBox == m_pStartBracketLB;
        if( bStart )
            m_nStartBracketPosition = nPos;
        else
            m_nEndBracketPosition = nPos;
    }
    UpdatePreview_Impl();
}


void SvxCharTwoLinesPage::ActivatePage( const SfxItemSet& rSet )
{
    SvxCharBasePage::ActivatePage( rSet );
}


SfxTabPage::sfxpg SvxCharTwoLinesPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( _pSet );
    return LEAVE_PAGE;
}


VclPtr<SfxTabPage> SvxCharTwoLinesPage::Create( vcl::Window* pParent, const SfxItemSet* rSet )
{
    return VclPtr<SvxCharTwoLinesPage>::Create( pParent, *rSet );
}

void SvxCharTwoLinesPage::Reset( const SfxItemSet* rSet )
{
    m_pTwoLinesBtn->Check( false );
    sal_uInt16 nWhich = GetWhich( SID_ATTR_CHAR_TWO_LINES );
    SfxItemState eState = rSet->GetItemState( nWhich );

    if ( eState >= SfxItemState::DONTCARE )
    {
        const SvxTwoLinesItem& rItem = static_cast<const SvxTwoLinesItem&>(rSet->Get( nWhich ));
        m_pTwoLinesBtn->Check( rItem.GetValue() );

        if ( rItem.GetValue() )
        {
            SetBracket( rItem.GetStartBracket(), true );
            SetBracket( rItem.GetEndBracket(), false );
        }
    }
    TwoLinesHdl_Impl( nullptr );

    SetPrevFontWidthScale( *rSet );
}

bool SvxCharTwoLinesPage::FillItemSet( SfxItemSet* rSet )
{
    const SfxItemSet& rOldSet = GetItemSet();
    bool bModified = false, bChanged = true;
    sal_uInt16 nWhich = GetWhich( SID_ATTR_CHAR_TWO_LINES );
    const SfxPoolItem* pOld = GetOldItem( *rSet, SID_ATTR_CHAR_TWO_LINES );
    bool bOn = m_pTwoLinesBtn->IsChecked();
    sal_Unicode cStart = ( bOn && m_pStartBracketLB->GetSelectEntryPos() > 0 )
        ? m_pStartBracketLB->GetSelectEntry()[0] : 0;
    sal_Unicode cEnd = ( bOn && m_pEndBracketLB->GetSelectEntryPos() > 0 )
        ? m_pEndBracketLB->GetSelectEntry()[0] : 0;

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
    sal_Unicode cStart = m_pStartBracketLB->GetSelectEntryPos() > 0
        ? m_pStartBracketLB->GetSelectEntry()[0] : 0;
    sal_Unicode cEnd = m_pEndBracketLB->GetSelectEntryPos() > 0
        ? m_pEndBracketLB->GetSelectEntry()[0] : 0;
    m_pPreviewWin->SetBrackets(cStart, cEnd);
    m_pPreviewWin->SetTwoLines(m_pTwoLinesBtn->IsChecked());
    m_pPreviewWin->Invalidate();
}

void SvxCharTwoLinesPage::SetPreviewBackgroundToCharacter()
{
    m_bPreviewBackgroundToCharacter = true;
}


void SvxCharTwoLinesPage::PageCreated(const SfxAllItemSet& aSet)
{
    const SfxUInt32Item* pFlagItem = aSet.GetItem<SfxUInt32Item>(SID_FLAG_TYPE, false);
    if (pFlagItem)
    {
        sal_uInt32 nFlags=pFlagItem->GetValue();
        if ( ( nFlags & SVX_PREVIEW_CHARACTER ) == SVX_PREVIEW_CHARACTER )
            SetPreviewBackgroundToCharacter();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
