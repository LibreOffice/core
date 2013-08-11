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
#include <svx/xtable.hxx>       // XColorList
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

#define ISITEMSET   rSet.GetItemState(nWhich)>=SFX_ITEM_DEFAULT

#define CLEARTITEM  rSet.InvalidateItem(nWhich)

#define LW_NORMAL   0
#define LW_EXPANDED 1
#define LW_CONDENSED   2

// static ----------------------------------------------------------------

static sal_uInt16 pNameRanges[] =
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

static sal_uInt16 pEffectsRanges[] =
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

static sal_uInt16 pPositionRanges[] =
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

static sal_uInt16 pTwoLinesRanges[] =
{
    SID_ATTR_CHAR_TWO_LINES,
    SID_ATTR_CHAR_TWO_LINES,
    0
};

// C-Funktion ------------------------------------------------------------

inline sal_Bool StateToAttr( TriState aState )
{
    return ( STATE_CHECK == aState );
}

// class SvxCharBasePage -------------------------------------------------

inline SvxFont& SvxCharBasePage::GetPreviewFont()
{
    return m_pPreviewWin->GetFont();
}

// -----------------------------------------------------------------------

inline SvxFont& SvxCharBasePage::GetPreviewCJKFont()
{
    return m_pPreviewWin->GetCJKFont();
}
// -----------------------------------------------------------------------

inline SvxFont& SvxCharBasePage::GetPreviewCTLFont()
{
    return m_pPreviewWin->GetCTLFont();
}

// -----------------------------------------------------------------------

SvxCharBasePage::SvxCharBasePage(Window* pParent, const OString& rID, const OUString& rUIXMLDescription, const SfxItemSet& rItemset)
    : SfxTabPage( pParent, rID, rUIXMLDescription, rItemset )
    , m_pPreviewWin(NULL)
    , m_bPreviewBackgroundToCharacter( sal_False )
{
}

// -----------------------------------------------------------------------

SvxCharBasePage::~SvxCharBasePage()
{
}

// -----------------------------------------------------------------------

void SvxCharBasePage::ActivatePage( const SfxItemSet& rSet )
{
    m_pPreviewWin->SetFromItemSet( rSet, m_bPreviewBackgroundToCharacter );
}


// -----------------------------------------------------------------------

void SvxCharBasePage::SetPrevFontWidthScale( const SfxItemSet& rSet )
{
    sal_uInt16 nWhich = GetWhich( SID_ATTR_CHAR_SCALEWIDTH );
    if( ISITEMSET )
    {
        const SvxCharScaleWidthItem &rItem = ( SvxCharScaleWidthItem& ) rSet.Get( nWhich );
        m_pPreviewWin->SetFontWidthScale( rItem.GetValue() );
    }
}

// -----------------------------------------------------------------------
namespace
{
    // -----------------------------------------------------------------------
    void setPrevFontEscapement(SvxFont& _rFont,sal_uInt8 nProp, sal_uInt8 nEscProp, short nEsc )
    {
        _rFont.SetPropr( nProp );
        _rFont.SetProprRel( nEscProp );
        _rFont.SetEscapement( nEsc );
    }
    // -----------------------------------------------------------------------
    // -----------------------------------------------------------------------
}
// -----------------------------------------------------------------------

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
    Timer           m_aUpdateTimer;
    String          m_aNoStyleText;
    const FontList* m_pFontList;
    sal_uInt16          m_nExtraEntryPos;
    sal_Bool            m_bMustDelete;
    sal_Bool            m_bInSearchMode;

    SvxCharNamePage_Impl() :

        m_pFontList     ( NULL ),
        m_nExtraEntryPos( LISTBOX_ENTRY_NOTFOUND ),
        m_bMustDelete   ( sal_False ),
        m_bInSearchMode ( sal_False )

    {
        m_aUpdateTimer.SetTimeout( 350 );
    }

    ~SvxCharNamePage_Impl()
    {
        if ( m_bMustDelete )
            delete m_pFontList;
    }
};

// class SvxCharNamePage -------------------------------------------------

SvxCharNamePage::SvxCharNamePage( Window* pParent, const SfxItemSet& rInSet )
    : SvxCharBasePage(pParent, "CharNamePage", "cui/ui/charnamepage.ui", rInSet)
    , m_pImpl(new SvxCharNamePage_Impl)
{
    m_pImpl->m_aNoStyleText = String( CUI_RES( RID_SVXSTR_CHARNAME_NOSTYLE ) );

    SvtLanguageOptions aLanguageOptions;
    sal_Bool bShowCJK = aLanguageOptions.IsCJKFontEnabled();
    sal_Bool bShowCTL = aLanguageOptions.IsCTLFontEnabled();
    sal_Bool bShowNonWestern = bShowCJK || bShowCTL;

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
    String sFontFamilyString(CUI_RES(RID_SVXSTR_CHARNAME_FONT));
#else
    String sFontFamilyString(CUI_RES(RID_SVXSTR_CHARNAME_FAMILY));
#endif
    m_pWestFontNameFT->SetText(sFontFamilyString);
    m_pEastFontNameFT->SetText(sFontFamilyString);
    m_pCTLFontNameFT->SetText(sFontFamilyString);

#ifdef MACOSX
    String sFontStyleString(CUI_RES(RID_SVXSTR_CHARNAME_TYPEFACE));
#else
    String sFontStyleString(CUI_RES(RID_SVXSTR_CHARNAME_STYLE));
#endif
    m_pWestFontStyleFT->SetText(sFontStyleString);
    m_pEastFontStyleFT->SetText(sFontStyleString);
    m_pCTLFontStyleFT->SetText(sFontStyleString);

    m_pWestFrame->Show(true);
    m_pEastFrame->Show(bShowCJK);
    m_pCTLFrame->Show(bShowCTL);

    get(m_pPreviewWin, "preview");

    m_pWestFontLanguageLB->SetLanguageList(LANG_LIST_WESTERN, sal_True, sal_False, sal_True);
    m_pEastFontLanguageLB->SetLanguageList(LANG_LIST_CJK, sal_True, sal_False, sal_True);
    m_pCTLFontLanguageLB->SetLanguageList(LANG_LIST_CTL, sal_True, sal_False, sal_True);

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

// -----------------------------------------------------------------------

SvxCharNamePage::~SvxCharNamePage()
{
    delete m_pImpl;
}

// -----------------------------------------------------------------------

void SvxCharNamePage::Initialize()
{
    // to handle the changes of the other pages
    SetExchangeSupport();

    Link aLink = LINK( this, SvxCharNamePage, FontModifyHdl_Impl );
    m_pWestFontNameLB->SetModifyHdl( aLink );
    m_pWestFontStyleLB->SetModifyHdl( aLink );
    m_pWestFontSizeLB->SetModifyHdl( aLink );
    m_pWestFontLanguageLB->SetSelectHdl( aLink );
    m_pEastFontNameLB->SetModifyHdl( aLink );
    m_pEastFontStyleLB->SetModifyHdl( aLink );
    m_pEastFontSizeLB->SetModifyHdl( aLink );
    m_pEastFontLanguageLB->SetSelectHdl( aLink );
    m_pCTLFontNameLB->SetModifyHdl( aLink );
    m_pCTLFontStyleLB->SetModifyHdl( aLink );
    m_pCTLFontSizeLB->SetModifyHdl( aLink );
    m_pCTLFontLanguageLB->SetSelectHdl( aLink );

    m_pImpl->m_aUpdateTimer.SetTimeoutHdl( LINK( this, SvxCharNamePage, UpdateHdl_Impl ) );
}

// -----------------------------------------------------------------------

const FontList* SvxCharNamePage::GetFontList() const
{
    if ( !m_pImpl->m_pFontList )
    {
        SfxObjectShell* pDocSh = SfxObjectShell::Current();
        const SfxPoolItem* pItem;

        /* #110771# SvxFontListItem::GetFontList can return NULL */
        if ( pDocSh )
        {
            pItem = pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST );
            if ( pItem != NULL )
            {
                DBG_ASSERT(NULL != ( (SvxFontListItem*)pItem )->GetFontList(),
                           "Where is the font list?");
                    m_pImpl->m_pFontList =  static_cast<const SvxFontListItem*>(pItem )->GetFontList()->Clone();
                m_pImpl->m_bMustDelete = sal_True;
            }
        }
        if(!m_pImpl->m_pFontList)
        {
            m_pImpl->m_pFontList =
                new FontList( Application::GetDefaultDevice() );
            m_pImpl->m_bMustDelete = sal_True;
        }
    }

    return m_pImpl->m_pFontList;
}

// -----------------------------------------------------------------------------
namespace
{
    FontInfo calcFontInfo(  SvxFont& _rFont,
                    SvxCharNamePage* _pPage,
                    const FontNameBox* _pFontNameLB,
                    const FontStyleBox* _pFontStyleLB,
                    const FontSizeBox* _pFontSizeLB,
                    const SvxLanguageBox* _pLanguageLB,
                    const FontList* _pFontList,
                    sal_uInt16 _nFontWhich,
                    sal_uInt16 _nFontHeightWhich)
    {
        Size aSize = _rFont.GetSize();
        aSize.Width() = 0;
        FontInfo aFontInfo;
        String sFontName(_pFontNameLB->GetText());
        sal_Bool bFontAvailable = _pFontList->IsAvailable( sFontName );
        if (bFontAvailable  || _pFontNameLB->GetSavedValue() != sFontName)
            aFontInfo = _pFontList->Get( sFontName, _pFontStyleLB->GetText() );
        else
        {
            //get the font from itemset
            SfxItemState eState = _pPage->GetItemSet().GetItemState( _nFontWhich );
            if ( eState >= SFX_ITEM_DEFAULT )
            {
                const SvxFontItem* pFontItem = (const SvxFontItem*)&( _pPage->GetItemSet().Get( _nFontWhich ) );
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
            const SvxFontHeightItem& rOldItem = (SvxFontHeightItem&)_pPage->GetItemSet().GetParent()->Get( _nFontHeightWhich );

            // old value, scaled
            long nHeight;
            if ( _pFontSizeLB->IsPtRelative() )
                nHeight = rOldItem.GetHeight() + PointToTwips( static_cast<long>(_pFontSizeLB->GetValue() / 10) );
            else
                nHeight = static_cast<long>(rOldItem.GetHeight() * _pFontSizeLB->GetValue() / 100);

            // conversion twips for the example-window
            aSize.Height() =
                ItemToControl( nHeight, _pPage->GetItemSet().GetPool()->GetMetric( _nFontHeightWhich ), SFX_FUNIT_TWIP );
        }
        else if ( !_pFontSizeLB->GetText().isEmpty() )
            aSize.Height() = PointToTwips( static_cast<long>(_pFontSizeLB->GetValue() / 10) );
        else
            aSize.Height() = 200;   // default 10pt
        aFontInfo.SetSize( aSize );

        _rFont.SetLanguage(_pLanguageLB->GetSelectLanguage());

        _rFont.SetFamily( aFontInfo.GetFamily() );
        _rFont.SetName( aFontInfo.GetName() );
        _rFont.SetStyleName( aFontInfo.GetStyleName() );
        _rFont.SetPitch( aFontInfo.GetPitch() );
        _rFont.SetCharSet( aFontInfo.GetCharSet() );
        _rFont.SetWeight( aFontInfo.GetWeight() );
        _rFont.SetItalic( aFontInfo.GetItalic() );
        _rFont.SetSize( aFontInfo.GetSize() );

        return aFontInfo;
    }
}

// -----------------------------------------------------------------------

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

    FontInfo aWestFontInfo = calcFontInfo(rFont, this, m_pWestFontNameLB,
        m_pWestFontStyleLB, m_pWestFontSizeLB, m_pWestFontLanguageLB,
        pFontList, GetWhich(SID_ATTR_CHAR_FONT),
        GetWhich(SID_ATTR_CHAR_FONTHEIGHT));
    m_pWestFontTypeFT->SetText(pFontList->GetFontMapText(aWestFontInfo));

    FontInfo aEastFontInfo = calcFontInfo(rCJKFont, this, m_pEastFontNameLB,
        m_pEastFontStyleLB, m_pEastFontSizeLB, m_pEastFontLanguageLB,
        pFontList, GetWhich(SID_ATTR_CHAR_CJK_FONT),
        GetWhich(SID_ATTR_CHAR_CJK_FONTHEIGHT));
    m_pEastFontTypeFT->SetText(pFontList->GetFontMapText(aEastFontInfo));

    FontInfo aCTLFontInfo = calcFontInfo(rCTLFont,
        this, m_pCTLFontNameLB, m_pCTLFontStyleLB, m_pCTLFontSizeLB,
        m_pCTLFontLanguageLB, pFontList, GetWhich(SID_ATTR_CHAR_CTL_FONT),
        GetWhich(SID_ATTR_CHAR_CTL_FONTHEIGHT));
    m_pCTLFontTypeFT->SetText(pFontList->GetFontMapText(aCTLFontInfo));

    m_pPreviewWin->Invalidate();
}

// -----------------------------------------------------------------------

void SvxCharNamePage::FillStyleBox_Impl( const FontNameBox* pNameBox )
{
    const FontList* pFontList = GetFontList();
    DBG_ASSERT( pFontList, "no fontlist" );

    FontStyleBox* pStyleBox = NULL;

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
        String aEntry = m_pImpl->m_aNoStyleText;
        const sal_Char sS[] = "%1";
        aEntry.SearchAndReplaceAscii( sS, pFontList->GetBoldStr() );
        m_pImpl->m_nExtraEntryPos = pStyleBox->InsertEntry( aEntry );
        aEntry = m_pImpl->m_aNoStyleText;
        aEntry.SearchAndReplaceAscii( sS, pFontList->GetItalicStr() );
        pStyleBox->InsertEntry( aEntry );
    }
}

// -----------------------------------------------------------------------

void SvxCharNamePage::FillSizeBox_Impl( const FontNameBox* pNameBox )
{
    const FontList* pFontList = GetFontList();
    DBG_ASSERT( pFontList, "no fontlist" );

    FontStyleBox* pStyleBox = NULL;
    FontSizeBox* pSizeBox = NULL;

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

    FontInfo _aFontInfo( pFontList->Get( pNameBox->GetText(), pStyleBox->GetText() ) );
    pSizeBox->Fill( &_aFontInfo, pFontList );
}

// -----------------------------------------------------------------------

void SvxCharNamePage::Reset_Impl( const SfxItemSet& rSet, LanguageGroup eLangGrp )
{
    FontNameBox* pNameBox = NULL;
    FixedText* pStyleLabel = NULL;
    FontStyleBox* pStyleBox = NULL;
    FixedText* pSizeLabel = NULL;
    FontSizeBox* pSizeBox = NULL;
    FixedText* pLangFT = NULL;
    SvxLanguageBox* pLangBox = NULL;
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

    const SvxFontItem* pFontItem = NULL;
    SfxItemState eState = rSet.GetItemState( nWhich );

    if ( eState >= SFX_ITEM_DEFAULT )
    {
        pFontItem = (const SvxFontItem*)&( rSet.Get( nWhich ) );
        pNameBox->SetText( pFontItem->GetFamilyName() );
    }
    else
    {
        pNameBox->SetText( String() );
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

    if ( eState >= SFX_ITEM_DEFAULT )
    {
        const SvxPostureItem& rItem = (SvxPostureItem&)rSet.Get( nWhich );
        eItalic = (FontItalic)rItem.GetValue();
        bStyle = true;
    }
    bStyleAvailable = bStyleAvailable && (eState >= SFX_ITEM_DONTCARE);

    switch ( eLangGrp )
    {
        case Western : nWhich = GetWhich( SID_ATTR_CHAR_WEIGHT ); break;
        case Asian : nWhich = GetWhich( SID_ATTR_CHAR_CJK_WEIGHT ); break;
        case Ctl : nWhich = GetWhich( SID_ATTR_CHAR_CTL_WEIGHT ); break;
    }
    eState = rSet.GetItemState( nWhich );

    if ( eState >= SFX_ITEM_DEFAULT )
    {
        SvxWeightItem& rItem = (SvxWeightItem&)rSet.Get( nWhich );
        eWeight = (FontWeight)rItem.GetValue();
    }
    else
        bStyle = false;
    bStyleAvailable = bStyleAvailable && (eState >= SFX_ITEM_DONTCARE);

    // currently chosen font
    if ( bStyle && pFontItem )
    {
        FontInfo aInfo = pFontList->Get( pFontItem->GetFamilyName(), eWeight, eItalic );
        pStyleBox->SetText( pFontList->GetStyleName( aInfo ) );
    }
    else if ( !m_pImpl->m_bInSearchMode || !bStyle )
    {
        pStyleBox->SetText( String() );
    }
    else if ( bStyle )
    {
        FontInfo aInfo = pFontList->Get( String(), eWeight, eItalic );
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
        const SvxFontHeightItem& rItem = (SvxFontHeightItem&)rSet.Get( nWhich );

        if( rItem.GetProp() != 100 || SFX_MAPUNIT_RELATIVE != rItem.GetPropUnit() )
        {
            sal_Bool bPtRel = SFX_MAPUNIT_POINT == rItem.GetPropUnit();
            pSizeBox->SetPtRelative( bPtRel );
            pSizeBox->SetValue( bPtRel ? ((short)rItem.GetProp()) * 10 : rItem.GetProp() );
        }
        else
        {
            pSizeBox->SetRelative();
            pSizeBox->SetValue( (long)CalcToPoint( rItem.GetHeight(), eUnit, 10 ) );
        }
    }
    else if ( eState >= SFX_ITEM_DEFAULT )
    {
        SfxMapUnit eUnit = rSet.GetPool()->GetMetric( nWhich );
        const SvxFontHeightItem& rItem = (SvxFontHeightItem&)rSet.Get( nWhich );
        pSizeBox->SetValue( (long)CalcToPoint( rItem.GetHeight(), eUnit, 10 ) );
    }
    else
    {
        pSizeBox->SetText( String() );
        if ( eState <= SFX_ITEM_READONLY )
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
    pLangBox->SetNoSelection();
    eState = rSet.GetItemState( nWhich );

    switch ( eState )
    {
        case SFX_ITEM_UNKNOWN:
            pLangFT->Hide();
            pLangBox->Hide();
            break;

        case SFX_ITEM_DISABLED:
        case SFX_ITEM_READONLY:
            pLangFT->Disable();
            pLangBox->Disable();
            break;

        case SFX_ITEM_DEFAULT:
        case SFX_ITEM_SET:
        {
            const SvxLanguageItem& rItem = (SvxLanguageItem&)rSet.Get( nWhich );
            LanguageType eLangType = (LanguageType)rItem.GetValue();
            DBG_ASSERT( eLangType != LANGUAGE_SYSTEM, "LANGUAGE_SYSTEM not allowed" );
            if ( eLangType != LANGUAGE_DONTKNOW )
                pLangBox->SelectLanguage( eLangType );
            break;
        }
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
    pLangBox->SaveValue();
}

// -----------------------------------------------------------------------

sal_Bool SvxCharNamePage::FillItemSet_Impl( SfxItemSet& rSet, LanguageGroup eLangGrp )
{
    sal_Bool bModified = sal_False;

    FontNameBox* pNameBox = NULL;
    FontStyleBox* pStyleBox = NULL;
    FontSizeBox* pSizeBox = NULL;
    SvxLanguageBox* pLangBox = NULL;
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
    const SfxPoolItem* pItem = NULL;
    const SfxItemSet& rOldSet = GetItemSet();
    const SfxPoolItem* pOld = NULL;

    const SfxItemSet* pExampleSet = GetTabDialog() ? GetTabDialog()->GetExampleSet() : NULL;

    bool bChanged = true;
    const String& rFontName  = pNameBox->GetText();
    const FontList* pFontList = GetFontList();
    String aStyleBoxText =pStyleBox->GetText();
    sal_uInt16 nEntryPos = pStyleBox->GetEntryPos( aStyleBoxText );
    if ( nEntryPos >= m_pImpl->m_nExtraEntryPos )
        aStyleBoxText.Erase();
    FontInfo aInfo( pFontList->Get( rFontName, aStyleBoxText ) );
    SvxFontItem aFontItem( aInfo.GetFamily(), aInfo.GetName(), aInfo.GetStyleName(),
                           aInfo.GetPitch(), aInfo.GetCharSet(), nWhich );
    pOld = GetOldItem( rSet, nSlot );

    if ( pOld )
    {
        const SvxFontItem& rItem = *( (const SvxFontItem*)pOld );

        if ( rItem.GetFamilyName() == aFontItem.GetFamilyName() )
            bChanged = false;
    }

    if ( !bChanged )
        bChanged = pNameBox->GetSavedValue().isEmpty();

    if ( !bChanged && pExampleSet &&
         pExampleSet->GetItemState( nWhich, sal_False, &pItem ) == SFX_ITEM_SET &&
         ( (SvxFontItem*)pItem )->GetFamilyName() != aFontItem.GetFamilyName() )
        bChanged = true;

    if ( bChanged && rFontName.Len() )
    {
        rSet.Put( aFontItem );
        bModified = sal_True;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, sal_False ) )
        rSet.ClearItem( nWhich );


    bChanged = sal_True;
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
        const SvxWeightItem& rItem = *( (const SvxWeightItem*)pOld );

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
         pExampleSet->GetItemState( nWhich, sal_False, &pItem ) == SFX_ITEM_SET &&
         ( (SvxWeightItem*)pItem )->GetValue() != aWeightItem.GetValue() )
        bChanged = true;

    if ( nEntryPos >= m_pImpl->m_nExtraEntryPos )
        bChanged = ( nEntryPos == m_pImpl->m_nExtraEntryPos );

    String aText( pStyleBox->GetText() ); // Tristate, then text empty

    if ( bChanged && aText.Len() )
    {
        rSet.Put( aWeightItem );
        bModified = sal_True;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, sal_False ) )
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
        const SvxPostureItem& rItem = *( (const SvxPostureItem*)pOld );

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
         pExampleSet->GetItemState( nWhich, sal_False, &pItem ) == SFX_ITEM_SET &&
         ( (SvxPostureItem*)pItem )->GetValue() != aPostureItem.GetValue() )
        bChanged = true;

    if ( nEntryPos >= m_pImpl->m_nExtraEntryPos )
        bChanged = ( nEntryPos == ( m_pImpl->m_nExtraEntryPos + 1 ) );

    if ( bChanged && aText.Len() )
    {
        rSet.Put( aPostureItem );
        bModified = sal_True;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, sal_False ) )
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
    const SvxFontHeightItem* pOldHeight = (const SvxFontHeightItem*)GetOldItem( rSet, nSlot );
    bChanged = ( nSize != nSavedSize );

    if ( !bChanged && pExampleSet &&
         pExampleSet->GetItemState( nWhich, sal_False, &pItem ) == SFX_ITEM_SET )
    {
        float fSize = (float)nSize / 10;
        long nVal = CalcToUnit( fSize, rSet.GetPool()->GetMetric( nWhich ) );
        if ( ( (SvxFontHeightItem*)pItem )->GetHeight() != (sal_uInt32)nVal )
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
                (const SvxFontHeightItem&)GetItemSet().GetParent()->Get( nWhich );

            SvxFontHeightItem aHeight( 240, 100, nWhich );
            if ( pSizeBox->IsPtRelative() )
                aHeight.SetHeight( rOldItem.GetHeight(), (sal_uInt16)( nSize / 10 ), SFX_MAPUNIT_POINT, eUnit );
            else
                aHeight.SetHeight( rOldItem.GetHeight(), (sal_uInt16)nSize, SFX_MAPUNIT_RELATIVE );
            rSet.Put( aHeight );
        }
        else
        {
            float fSize = (float)nSize / 10;
            rSet.Put( SvxFontHeightItem( CalcToUnit( fSize, eUnit ), 100, nWhich ) );
        }
        bModified = sal_True;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, sal_False ) )
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
    sal_uInt16 nLangPos = pLangBox->GetSelectEntryPos();
    LanguageType eLangType = (LanguageType)(sal_uLong)pLangBox->GetEntryData( nLangPos );

    if ( pOld )
    {
        const SvxLanguageItem& rItem = *( (const SvxLanguageItem*)pOld );

        if ( nLangPos == LISTBOX_ENTRY_NOTFOUND || eLangType == (LanguageType)rItem.GetValue() )
            bChanged = false;
    }

    if ( !bChanged )
        bChanged = ( pLangBox->GetSavedValue() == LISTBOX_ENTRY_NOTFOUND );

    if ( bChanged && nLangPos != LISTBOX_ENTRY_NOTFOUND )
    {
        rSet.Put( SvxLanguageItem( eLangType, nWhich ) );
        bModified = sal_True;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, sal_False ) )
        CLEARTITEM;

    return bModified;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxCharNamePage, UpdateHdl_Impl)
{
    UpdatePreview_Impl();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharNamePage, FontModifyHdl_Impl, void*, pNameBox )
{
    m_pImpl->m_aUpdateTimer.Start();

    if ( m_pWestFontNameLB == pNameBox || m_pEastFontNameLB == pNameBox || m_pCTLFontNameLB == pNameBox )
    {
        FillStyleBox_Impl( (FontNameBox*)pNameBox );
        FillSizeBox_Impl( (FontNameBox*)pNameBox );
    }
    return 0;
}

// -----------------------------------------------------------------------

void SvxCharNamePage::ActivatePage( const SfxItemSet& rSet )
{
    SvxCharBasePage::ActivatePage( rSet );

    UpdatePreview_Impl();       // instead of asynchronous calling in ctor
}

// -----------------------------------------------------------------------

int SvxCharNamePage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( *_pSet );
    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

SfxTabPage* SvxCharNamePage::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new SvxCharNamePage( pParent, rSet );
}

// -----------------------------------------------------------------------

sal_uInt16* SvxCharNamePage::GetRanges()
{
    return pNameRanges;
}

// -----------------------------------------------------------------------

void SvxCharNamePage::Reset( const SfxItemSet& rSet )
{
    Reset_Impl( rSet, Western );
    Reset_Impl( rSet, Asian );
    Reset_Impl( rSet, Ctl );

    SetPrevFontWidthScale( rSet );
    UpdatePreview_Impl();
}

// -----------------------------------------------------------------------

sal_Bool SvxCharNamePage::FillItemSet( SfxItemSet& rSet )
{
    sal_Bool bModified = FillItemSet_Impl( rSet, Western );
    bModified |= FillItemSet_Impl( rSet, Asian );
    bModified |= FillItemSet_Impl( rSet, Ctl );
    return bModified;
}

// -----------------------------------------------------------------------

void SvxCharNamePage::SetFontList( const SvxFontListItem& rItem )
{
    if ( m_pImpl->m_bMustDelete )
    {
        delete m_pImpl->m_pFontList;
    }
    m_pImpl->m_pFontList = rItem.GetFontList()->Clone();
    m_pImpl->m_bMustDelete = sal_True;
}

// -----------------------------------------------------------------------
namespace
{
    void enableRelativeMode( SvxCharNamePage* _pPage, FontSizeBox* _pFontSizeLB, sal_uInt16 _nHeightWhich )
    {
        _pFontSizeLB->EnableRelativeMode( 5, 995, 5 ); // min 5%, max 995%, step 5

        const SvxFontHeightItem& rHeightItem =
            (SvxFontHeightItem&)_pPage->GetItemSet().GetParent()->Get( _nHeightWhich );
        SfxMapUnit eUnit = _pPage->GetItemSet().GetPool()->GetMetric( _nHeightWhich );
        short nCurHeight =
            static_cast< short >( CalcToPoint( rHeightItem.GetHeight(), eUnit, 1 ) * 10 );

        // based on the current height:
        //      - negative until minimum of 2 pt
        //      - positive until maximum of 999 pt
        _pFontSizeLB->EnablePtRelativeMode( sal::static_int_cast< short >(-(nCurHeight - 20)), (9999 - nCurHeight), 10 );
    }
}
// -----------------------------------------------------------------------------

void SvxCharNamePage::EnableRelativeMode()
{
    DBG_ASSERT( GetItemSet().GetParent(), "RelativeMode, but no ParentSet!" );
    enableRelativeMode(this,m_pWestFontSizeLB,GetWhich( SID_ATTR_CHAR_FONTHEIGHT ));
    enableRelativeMode(this,m_pEastFontSizeLB,GetWhich( SID_ATTR_CHAR_CJK_FONTHEIGHT ));
    enableRelativeMode(this,m_pCTLFontSizeLB,GetWhich( SID_ATTR_CHAR_CTL_FONTHEIGHT ));
}

// -----------------------------------------------------------------------

void SvxCharNamePage::EnableSearchMode()
{
    m_pImpl->m_bInSearchMode = sal_True;
}
// -----------------------------------------------------------------------
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

// -----------------------------------------------------------------------
void SvxCharNamePage::SetPreviewBackgroundToCharacter()
{
    m_bPreviewBackgroundToCharacter = sal_True;
}

// -----------------------------------------------------------------------
void SvxCharNamePage::PageCreated (SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pFontListItem,SvxFontListItem,SID_ATTR_CHAR_FONTLIST,sal_False);
    SFX_ITEMSET_ARG (&aSet,pFlagItem,SfxUInt32Item,SID_FLAG_TYPE,sal_False);
    SFX_ITEMSET_ARG (&aSet,pDisalbeItem,SfxUInt16Item,SID_DISABLE_CTL,sal_False);
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

SvxCharEffectsPage::SvxCharEffectsPage( Window* pParent, const SfxItemSet& rInSet )
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

// -----------------------------------------------------------------------

void SvxCharEffectsPage::Initialize()
{
    // to handle the changes of the other pages
    SetExchangeSupport();

    // HTML-Mode
    const SfxPoolItem* pItem;
    SfxObjectShell* pShell;
    if ( SFX_ITEM_SET == GetItemSet().GetItemState( SID_HTML_MODE, sal_False, &pItem ) ||
         ( NULL != ( pShell = SfxObjectShell::Current() ) &&
           NULL != ( pItem = pShell->GetItem( SID_HTML_MODE ) ) ) )
    {
        m_nHtmlMode = ( (const SfxUInt16Item*)pItem )->GetValue();
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
        if ( pItem != NULL )
            pColorTable = ( (SvxColorListItem*)pItem )->GetColorList();
    }

    if ( !pColorTable.is() )
        pColorTable = XColorList::CreateStdColorList();

    m_pUnderlineColorLB->SetUpdateMode( sal_False );
    m_pOverlineColorLB->SetUpdateMode( sal_False );
    m_pFontColorLB->SetUpdateMode( sal_False );

    {
        SfxPoolItem* pDummy = NULL;
        SfxViewFrame* pFrame = SfxViewFrame::GetFirst( pDocSh );
        if ( !pFrame ||
             SFX_ITEM_DEFAULT > pFrame->GetBindings().QueryState( SID_ATTR_AUTO_COLOR_INVALID, pDummy ) )
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

    m_pUnderlineColorLB->SetUpdateMode( sal_True );
    m_pOverlineColorLB->SetUpdateMode( sal_True );
    m_pFontColorLB->SetUpdateMode( sal_True );
    m_pFontColorLB->SetSelectHdl( LINK( this, SvxCharEffectsPage, ColorBoxSelectHdl_Impl ) );

    // handler
    Link aLink = LINK( this, SvxCharEffectsPage, SelectHdl_Impl );
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
    SelectHdl_Impl( NULL );
    SelectHdl_Impl( m_pEmphasisLB );

    m_pEffectsLB->SelectEntryPos( 0 );

    m_pIndividualWordsBtn->SetClickHdl( LINK( this, SvxCharEffectsPage, CbClickHdl_Impl ) );
    aLink = LINK( this, SvxCharEffectsPage, TristClickHdl_Impl );
    m_pOutlineBtn->SetClickHdl( aLink );
    m_pShadowBtn->SetClickHdl( aLink );

    if ( !SvtLanguageOptions().IsAsianTypographyEnabled() )
    {
        m_pEmphasisFT->Hide();
        m_pEmphasisLB->Hide();
        m_pPositionFT->Hide();
        m_pPositionLB->Hide();
    }
}

SvxCharEffectsPage::~SvxCharEffectsPage()
{
}

// -----------------------------------------------------------------------

void SvxCharEffectsPage::UpdatePreview_Impl()
{
    SvxFont& rFont = GetPreviewFont();
    SvxFont& rCJKFont = GetPreviewCJKFont();
    SvxFont& rCTLFont = GetPreviewCTLFont();

    sal_uInt16 nPos = m_pUnderlineLB->GetSelectEntryPos();
    FontUnderline eUnderline = (FontUnderline)(sal_uLong)m_pUnderlineLB->GetEntryData( nPos );
    nPos = m_pOverlineLB->GetSelectEntryPos();
    FontUnderline eOverline = (FontUnderline)(sal_uLong)m_pOverlineLB->GetEntryData( nPos );
    nPos = m_pStrikeoutLB->GetSelectEntryPos();
    FontStrikeout eStrikeout = (FontStrikeout)(sal_uLong)m_pStrikeoutLB->GetEntryData( nPos );
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
    sal_Bool bUnder = ( CHRDLG_POSITION_UNDER == (sal_uLong)m_pPositionLB->GetEntryData( nPos ) );
    FontEmphasisMark eMark = (FontEmphasisMark)m_pEmphasisLB->GetSelectEntryPos();
    eMark |= bUnder ? EMPHASISMARK_POS_BELOW : EMPHASISMARK_POS_ABOVE;
    rFont.SetEmphasisMark( eMark );
    rCJKFont.SetEmphasisMark( eMark );
    rCTLFont.SetEmphasisMark( eMark );

    sal_uInt16 nRelief = m_pReliefLB->GetSelectEntryPos();
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

    sal_uInt16 nCapsPos = m_pEffectsLB->GetSelectEntryPos();
    if ( nCapsPos != LISTBOX_ENTRY_NOTFOUND )
    {
        rFont.SetCaseMap( (SvxCaseMap)nCapsPos );
        rCJKFont.SetCaseMap( (SvxCaseMap)nCapsPos );
        // #i78474# small caps do not exist in CTL fonts
        rCTLFont.SetCaseMap( static_cast<SvxCaseMap>( nCapsPos == SVX_CASEMAP_KAPITAELCHEN ? SVX_CASEMAP_NOT_MAPPED : (SvxCaseMap)nCapsPos) );
    }

    sal_Bool bWordLine = m_pIndividualWordsBtn->IsChecked();
    rFont.SetWordLineMode( bWordLine );
    rCJKFont.SetWordLineMode( bWordLine );
    rCTLFont.SetWordLineMode( bWordLine );

    m_pPreviewWin->Invalidate();
}

// -----------------------------------------------------------------------

void SvxCharEffectsPage::SetCaseMap_Impl( SvxCaseMap eCaseMap )
{
    if ( SVX_CASEMAP_END > eCaseMap )
        m_pEffectsLB->SelectEntryPos(
            sal::static_int_cast< sal_uInt16 >( eCaseMap ) );
    else
    {
        m_pEffectsLB->SetNoSelection();
        eCaseMap = SVX_CASEMAP_NOT_MAPPED;
    }

    UpdatePreview_Impl();
}

// -----------------------------------------------------------------------

void SvxCharEffectsPage::ResetColor_Impl( const SfxItemSet& rSet )
{
    sal_uInt16 nWhich = GetWhich( SID_ATTR_CHAR_COLOR );
    SfxItemState eState = rSet.GetItemState( nWhich );

    switch ( eState )
    {
        case SFX_ITEM_UNKNOWN:
            m_pFontColorFT->Hide();
            m_pFontColorLB->Hide();
            break;

        case SFX_ITEM_DISABLED:
        case SFX_ITEM_READONLY:
            m_pFontColorFT->Disable();
            m_pFontColorLB->Disable();
            break;

        case SFX_ITEM_DONTCARE:
            m_pFontColorLB->SetNoSelection();
            break;

        case SFX_ITEM_DEFAULT:
        case SFX_ITEM_SET:
        {
            SvxFont& rFont = GetPreviewFont();
            SvxFont& rCJKFont = GetPreviewCJKFont();
            SvxFont& rCTLFont = GetPreviewCTLFont();

            const SvxColorItem& rItem = (SvxColorItem&)rSet.Get( nWhich );
            Color aColor = rItem.GetValue();
            rFont.SetColor( aColor.GetColor() == COL_AUTO ? Color(COL_BLACK) : aColor );
            rCJKFont.SetColor( aColor.GetColor() == COL_AUTO ? Color(COL_BLACK) : aColor );
            rCTLFont.SetColor( aColor.GetColor() == COL_AUTO ? Color(COL_BLACK) : aColor );

            m_pPreviewWin->Invalidate();
            sal_uInt16 nSelPos = m_pFontColorLB->GetEntryPos( aColor );
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
                        m_pFontColorLB->InsertEntry( aColor, String( SVX_RES( RID_SVXSTR_COLOR_USER ) ) ) );
            }
            break;
        }
    }
}

// -----------------------------------------------------------------------

sal_Bool SvxCharEffectsPage::FillItemSetColor_Impl( SfxItemSet& rSet )
{
    sal_uInt16 nWhich = GetWhich( SID_ATTR_CHAR_COLOR );
    const SvxColorItem* pOld = (const SvxColorItem*)GetOldItem( rSet, SID_ATTR_CHAR_COLOR );
    const SvxColorItem* pItem = NULL;
    sal_Bool bChanged = sal_True;
    const SfxItemSet* pExampleSet = GetTabDialog() ? GetTabDialog()->GetExampleSet() : NULL;
    const SfxItemSet& rOldSet = GetItemSet();

    Color aSelectedColor;
    if ( m_pFontColorLB->GetSelectEntry() == m_aTransparentColorName )
        aSelectedColor = Color( COL_TRANSPARENT );
    else
        aSelectedColor = m_pFontColorLB->GetSelectEntryColor();

    if ( pOld && pOld->GetValue() == aSelectedColor )
        bChanged = sal_False;

    if ( !bChanged )
        bChanged = ( m_pFontColorLB->GetSavedValue() == LISTBOX_ENTRY_NOTFOUND );

    if ( !bChanged && pExampleSet &&
         pExampleSet->GetItemState( nWhich, sal_False, (const SfxPoolItem**)&pItem ) == SFX_ITEM_SET &&
         ( (SvxColorItem*)pItem )->GetValue() != aSelectedColor )
        bChanged = sal_True;

    sal_Bool bModified = sal_False;

    if ( bChanged && m_pFontColorLB->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND )
    {
        rSet.Put( SvxColorItem( aSelectedColor, nWhich ) );
        bModified = sal_True;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, sal_False ) )
        CLEARTITEM;

    return bModified;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharEffectsPage, SelectHdl_Impl, ListBox*, pBox )
{
    if ( m_pEmphasisLB == pBox )
    {
        sal_uInt16 nEPos = m_pEmphasisLB->GetSelectEntryPos();
        sal_Bool bEnable = ( nEPos > 0 && nEPos != LISTBOX_ENTRY_NOTFOUND );
        m_pPositionFT->Enable( bEnable );
        m_pPositionLB->Enable( bEnable );
    }
    else if( m_pReliefLB == pBox)
    {
        sal_Bool bEnable = ( pBox->GetSelectEntryPos() == 0 );
        m_pOutlineBtn->Enable( bEnable );
        m_pShadowBtn->Enable( bEnable );
    }
    else if ( m_pPositionLB != pBox )
    {
        sal_uInt16 nUPos = m_pUnderlineLB->GetSelectEntryPos(),
               nOPos = m_pOverlineLB->GetSelectEntryPos(),
               nSPos = m_pStrikeoutLB->GetSelectEntryPos();
        sal_Bool bUEnable = ( nUPos > 0 && nUPos != LISTBOX_ENTRY_NOTFOUND );
        sal_Bool bOEnable = ( nOPos > 0 && nOPos != LISTBOX_ENTRY_NOTFOUND );
        m_pUnderlineColorFT->Enable( bUEnable );
        m_pUnderlineColorLB->Enable( bUEnable );
        m_pOverlineColorFT->Enable( bOEnable );
        m_pOverlineColorLB->Enable( bOEnable );
        m_pIndividualWordsBtn->Enable( bUEnable || bOEnable || ( nSPos > 0 && nSPos != LISTBOX_ENTRY_NOTFOUND ) );
    }
    UpdatePreview_Impl();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxCharEffectsPage, UpdatePreview_Impl)
{
    bool bEnable = ( ( m_pUnderlineLB->GetSelectEntryPos() > 0 ) ||
                     ( m_pOverlineLB->GetSelectEntryPos()  > 0 ) ||
                     ( m_pStrikeoutLB->GetSelectEntryPos() > 0 ) );
    m_pIndividualWordsBtn->Enable( bEnable );

    UpdatePreview_Impl();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxCharEffectsPage, CbClickHdl_Impl)
{
    UpdatePreview_Impl();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxCharEffectsPage, TristClickHdl_Impl)
{
    UpdatePreview_Impl();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharEffectsPage, ColorBoxSelectHdl_Impl, ColorListBox*, pBox )
{
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
    return 0;
}
// -----------------------------------------------------------------------

int SvxCharEffectsPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( *_pSet );
    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

SfxTabPage* SvxCharEffectsPage::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new SvxCharEffectsPage( pParent, rSet );
}

// -----------------------------------------------------------------------

sal_uInt16* SvxCharEffectsPage::GetRanges()
{
    return pEffectsRanges;
}

// -----------------------------------------------------------------------

void SvxCharEffectsPage::Reset( const SfxItemSet& rSet )
{
    SvxFont& rFont = GetPreviewFont();
    SvxFont& rCJKFont = GetPreviewCJKFont();
    SvxFont& rCTLFont = GetPreviewCTLFont();

    sal_Bool bEnable = sal_False;

    // Underline
    sal_uInt16 nWhich = GetWhich( SID_ATTR_CHAR_UNDERLINE );
    rFont.SetUnderline( UNDERLINE_NONE );
    rCJKFont.SetUnderline( UNDERLINE_NONE );
    rCTLFont.SetUnderline( UNDERLINE_NONE );

    m_pUnderlineLB->SelectEntryPos( 0 );
    SfxItemState eState = rSet.GetItemState( nWhich );

    if ( eState >= SFX_ITEM_DONTCARE )
    {
        if ( eState == SFX_ITEM_DONTCARE )
            m_pUnderlineLB->SetNoSelection();
        else
        {
            const SvxUnderlineItem& rItem = (SvxUnderlineItem&)rSet.Get( nWhich );
            FontUnderline eUnderline = (FontUnderline)rItem.GetValue();
            rFont.SetUnderline( eUnderline );
            rCJKFont.SetUnderline( eUnderline );
            rCTLFont.SetUnderline( eUnderline );

            if ( eUnderline != UNDERLINE_NONE )
            {
                for ( sal_uInt16 i = 0; i < m_pUnderlineLB->GetEntryCount(); ++i )
                {
                    if ( (FontUnderline)(sal_uLong)m_pUnderlineLB->GetEntryData(i) == eUnderline )
                    {
                        m_pUnderlineLB->SelectEntryPos(i);
                        bEnable = sal_True;
                        break;
                    }
                }

                Color aColor = rItem.GetColor();
                sal_uInt16 nPos = m_pUnderlineColorLB->GetEntryPos( aColor );

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
                                String( SVX_RES( RID_SVXSTR_COLOR_USER ) ) ) );
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
    eState = rSet.GetItemState( nWhich );

    if ( eState >= SFX_ITEM_DONTCARE )
    {
        if ( eState == SFX_ITEM_DONTCARE )
            m_pOverlineLB->SetNoSelection();
        else
        {
            const SvxOverlineItem& rItem = (SvxOverlineItem&)rSet.Get( nWhich );
            FontUnderline eOverline = (FontUnderline)rItem.GetValue();
            rFont.SetOverline( eOverline );
            rCJKFont.SetOverline( eOverline );
            rCTLFont.SetOverline( eOverline );

            if ( eOverline != UNDERLINE_NONE )
            {
                for ( sal_uInt16 i = 0; i < m_pOverlineLB->GetEntryCount(); ++i )
                {
                    if ( (FontUnderline)(sal_uLong)m_pOverlineLB->GetEntryData(i) == eOverline )
                    {
                        m_pOverlineLB->SelectEntryPos(i);
                        bEnable = sal_True;
                        break;
                    }
                }

                Color aColor = rItem.GetColor();
                sal_uInt16 nPos = m_pOverlineColorLB->GetEntryPos( aColor );

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
                                String( SVX_RES( RID_SVXSTR_COLOR_USER ) ) ) );
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
    eState = rSet.GetItemState( nWhich );

    if ( eState >= SFX_ITEM_DONTCARE )
    {
        if ( eState == SFX_ITEM_DONTCARE )
            m_pStrikeoutLB->SetNoSelection();
        else
        {
            const SvxCrossedOutItem& rItem = (SvxCrossedOutItem&)rSet.Get( nWhich );
            FontStrikeout eStrikeout = (FontStrikeout)rItem.GetValue();
            rFont.SetStrikeout( eStrikeout );
            rCJKFont.SetStrikeout( eStrikeout );
            rCTLFont.SetStrikeout( eStrikeout );

            if ( eStrikeout != STRIKEOUT_NONE )
            {
                for ( sal_uInt16 i = 0; i < m_pStrikeoutLB->GetEntryCount(); ++i )
                {
                    if ( (FontStrikeout)(sal_uLong)m_pStrikeoutLB->GetEntryData(i) == eStrikeout )
                    {
                        m_pStrikeoutLB->SelectEntryPos(i);
                        bEnable = sal_True;
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
        case SFX_ITEM_UNKNOWN:
            m_pIndividualWordsBtn->Hide();
            break;

        case SFX_ITEM_DISABLED:
        case SFX_ITEM_READONLY:
            m_pIndividualWordsBtn->Disable();
            break;

        case SFX_ITEM_DONTCARE:
            m_pIndividualWordsBtn->SetState( STATE_DONTKNOW );
            break;

        case SFX_ITEM_DEFAULT:
        case SFX_ITEM_SET:
        {
            const SvxWordLineModeItem& rItem = (SvxWordLineModeItem&)rSet.Get( nWhich );
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
    eState = rSet.GetItemState( nWhich );

    if ( eState >= SFX_ITEM_DEFAULT )
    {
        const SvxEmphasisMarkItem& rItem = (SvxEmphasisMarkItem&)rSet.Get( nWhich );
        FontEmphasisMark eMark = rItem.GetEmphasisMark();
        rFont.SetEmphasisMark( eMark );
        rCJKFont.SetEmphasisMark( eMark );
        rCTLFont.SetEmphasisMark( eMark );

        m_pEmphasisLB->SelectEntryPos( (sal_uInt16)( eMark & EMPHASISMARK_STYLE ) );
        eMark &= ~EMPHASISMARK_STYLE;
        sal_uLong nEntryData = ( eMark == EMPHASISMARK_POS_ABOVE )
            ? CHRDLG_POSITION_OVER
            : ( eMark == EMPHASISMARK_POS_BELOW ) ? CHRDLG_POSITION_UNDER : 0;

        for ( sal_uInt16 i = 0; i < m_pPositionLB->GetEntryCount(); i++ )
        {
            if ( nEntryData == (sal_uLong)m_pPositionLB->GetEntryData(i) )
            {
                m_pPositionLB->SelectEntryPos(i);
                break;
            }
        }
    }
    else if ( eState == SFX_ITEM_DONTCARE )
        m_pEmphasisLB->SetNoSelection( );
    else if ( eState == SFX_ITEM_UNKNOWN )
    {
        m_pEmphasisFT->Hide();
        m_pEmphasisLB->Hide();
    }
    else // SFX_ITEM_DISABLED or SFX_ITEM_READONLY
    {
        m_pEmphasisFT->Disable();
        m_pEmphasisLB->Disable();
    }

    // the select handler for the underline/overline/strikeout list boxes
//  SelectHdl_Impl( NULL );
    DBG_ASSERT(m_pUnderlineLB->GetSelectHdl() == m_pOverlineLB->GetSelectHdl(),
        "SvxCharEffectsPage::Reset: inconsistence (1)!");
    DBG_ASSERT(m_pUnderlineLB->GetSelectHdl() == m_pStrikeoutLB->GetSelectHdl(),
        "SvxCharEffectsPage::Reset: inconsistence (1)!");
    m_pUnderlineLB->GetSelectHdl().Call(NULL);
        // don't call SelectHdl_Impl directly!
        // in DisableControls, we may have re-reouted the select handler

    // the select handler for the emphasis listbox
//  SelectHdl_Impl( m_pEmphasisLB );
    DBG_ASSERT(m_pEmphasisLB->GetSelectHdl() == LINK(this, SvxCharEffectsPage, SelectHdl_Impl),
        "SvxCharEffectsPage::Reset: inconsistence (2)!");
    m_pEmphasisLB->GetSelectHdl().Call( m_pEmphasisLB );
        // this is for consistency only. Here it would be allowed to call SelectHdl_Impl directly ...

    // Effects
    SvxCaseMap eCaseMap = SVX_CASEMAP_END;
    nWhich = GetWhich( SID_ATTR_CHAR_CASEMAP );
    eState = rSet.GetItemState( nWhich );
    switch ( eState )
    {
        case SFX_ITEM_UNKNOWN:
            m_pEffectsFT->Hide();
            m_pEffectsLB->Hide();
            break;

        case SFX_ITEM_DISABLED:
        case SFX_ITEM_READONLY:
            m_pEffectsFT->Disable();
            m_pEffectsLB->Disable();
            break;

        case SFX_ITEM_DONTCARE:
            m_pEffectsLB->SetNoSelection();
            break;

        case SFX_ITEM_DEFAULT:
        case SFX_ITEM_SET:
        {
            const SvxCaseMapItem& rItem = (const SvxCaseMapItem&)rSet.Get( nWhich );
            eCaseMap = (SvxCaseMap)rItem.GetValue();
            break;
        }
    }
    SetCaseMap_Impl( eCaseMap );

    //Relief
    nWhich = GetWhich(SID_ATTR_CHAR_RELIEF);
    eState = rSet.GetItemState( nWhich );
    switch ( eState )
    {
        case SFX_ITEM_UNKNOWN:
            m_pReliefFT->Hide();
            m_pReliefLB->Hide();
            break;

        case SFX_ITEM_DISABLED:
        case SFX_ITEM_READONLY:
            m_pReliefFT->Disable();
            m_pReliefLB->Disable();
            break;

        case SFX_ITEM_DONTCARE:
            m_pReliefLB->SetNoSelection();
            break;

        case SFX_ITEM_DEFAULT:
        case SFX_ITEM_SET:
        {
            const SvxCharReliefItem& rItem = (const SvxCharReliefItem&)rSet.Get( nWhich );
            m_pReliefLB->SelectEntryPos(rItem.GetValue());
            SelectHdl_Impl(m_pReliefLB);
            break;
        }
    }

    // Outline
    nWhich = GetWhich( SID_ATTR_CHAR_CONTOUR );
    eState = rSet.GetItemState( nWhich );
    switch ( eState )
    {
        case SFX_ITEM_UNKNOWN:
            m_pOutlineBtn->Hide();
            break;

        case SFX_ITEM_DISABLED:
        case SFX_ITEM_READONLY:
            m_pOutlineBtn->Disable();
            break;

        case SFX_ITEM_DONTCARE:
            m_pOutlineBtn->SetState( STATE_DONTKNOW );
            break;

        case SFX_ITEM_DEFAULT:
        case SFX_ITEM_SET:
        {
            const SvxContourItem& rItem = (SvxContourItem&)rSet.Get( nWhich );
            m_pOutlineBtn->SetState( (TriState)rItem.GetValue() );
            m_pOutlineBtn->EnableTriState( sal_False );
            break;
        }
    }

    // Shadow
    nWhich = GetWhich( SID_ATTR_CHAR_SHADOWED );
    eState = rSet.GetItemState( nWhich );

    switch ( eState )
    {
        case SFX_ITEM_UNKNOWN:
            m_pShadowBtn->Hide();
            break;

        case SFX_ITEM_DISABLED:
        case SFX_ITEM_READONLY:
            m_pShadowBtn->Disable();
            break;

        case SFX_ITEM_DONTCARE:
            m_pShadowBtn->SetState( STATE_DONTKNOW );
            break;

        case SFX_ITEM_DEFAULT:
        case SFX_ITEM_SET:
        {
            const SvxShadowedItem& rItem = (SvxShadowedItem&)rSet.Get( nWhich );
            m_pShadowBtn->SetState( (TriState)rItem.GetValue() );
            m_pShadowBtn->EnableTriState( sal_False );
            break;
        }
    }

    // Blinking
    nWhich = GetWhich( SID_ATTR_FLASH );
    eState = rSet.GetItemState( nWhich );

    switch ( eState )
    {
        case SFX_ITEM_UNKNOWN:
            m_pBlinkingBtn->Hide();
            break;

        case SFX_ITEM_DISABLED:
        case SFX_ITEM_READONLY:
            m_pBlinkingBtn->Disable();
            break;

        case SFX_ITEM_DONTCARE:
            m_pBlinkingBtn->SetState( STATE_DONTKNOW );
            break;

        case SFX_ITEM_DEFAULT:
        case SFX_ITEM_SET:
        {
            const SvxBlinkItem& rItem = (SvxBlinkItem&)rSet.Get( nWhich );
            m_pBlinkingBtn->SetState( (TriState)rItem.GetValue() );
            m_pBlinkingBtn->EnableTriState( sal_False );
            break;
        }
    }
    // Hidden
    nWhich = GetWhich( SID_ATTR_CHAR_HIDDEN );
    eState = rSet.GetItemState( nWhich );

    switch ( eState )
    {
        case SFX_ITEM_UNKNOWN:
            m_pHiddenBtn->Hide();
            break;

        case SFX_ITEM_DISABLED:
        case SFX_ITEM_READONLY:
            m_pHiddenBtn->Disable();
            break;

        case SFX_ITEM_DONTCARE:
            m_pHiddenBtn->SetState( STATE_DONTKNOW );
            break;

        case SFX_ITEM_DEFAULT:
        case SFX_ITEM_SET:
        {
            const SvxCharHiddenItem& rItem = (SvxCharHiddenItem&)rSet.Get( nWhich );
            m_pHiddenBtn->SetState( (TriState)rItem.GetValue() );
            m_pHiddenBtn->EnableTriState( sal_False );
            break;
        }
    }

    SetPrevFontWidthScale( rSet );
    ResetColor_Impl( rSet );

    // preview update
    m_pPreviewWin->Invalidate();

    // save this settings
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

// -----------------------------------------------------------------------

sal_Bool SvxCharEffectsPage::FillItemSet( SfxItemSet& rSet )
{
    const SfxPoolItem* pOld = 0;
    const SfxItemSet& rOldSet = GetItemSet();
    sal_Bool bModified = sal_False;
    bool bChanged = true;

    // Underline
    sal_uInt16 nWhich = GetWhich( SID_ATTR_CHAR_UNDERLINE );
    pOld = GetOldItem( rSet, SID_ATTR_CHAR_UNDERLINE );
    sal_uInt16 nPos = m_pUnderlineLB->GetSelectEntryPos();
    FontUnderline eUnder = (FontUnderline)(sal_uLong)m_pUnderlineLB->GetEntryData( nPos );

    if ( pOld )
    {
        //! if there are different underline styles in the selection the
        //! item-state in the 'rOldSet' will be invalid. In this case
        //! changing the underline style will be allowed if a style is
        //! selected in the listbox.
        sal_Bool bAllowChg = LISTBOX_ENTRY_NOTFOUND != nPos  &&
                         SFX_ITEM_DEFAULT > rOldSet.GetItemState( nWhich, sal_True );

        const SvxUnderlineItem& rItem = *( (const SvxUnderlineItem*)pOld );
        if ( (FontUnderline)rItem.GetValue() == eUnder &&
             ( UNDERLINE_NONE == eUnder || rItem.GetColor() == m_pUnderlineColorLB->GetSelectEntryColor() ) &&
             ! bAllowChg )
            bChanged = false;
    }

    if ( bChanged )
    {
        SvxUnderlineItem aNewItem( eUnder, nWhich );
        aNewItem.SetColor( m_pUnderlineColorLB->GetSelectEntryColor() );
        rSet.Put( aNewItem );
        bModified = sal_True;
    }
     else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, sal_False ) )
        CLEARTITEM;

    bChanged = true;

    // Overline
    nWhich = GetWhich( SID_ATTR_CHAR_OVERLINE );
    pOld = GetOldItem( rSet, SID_ATTR_CHAR_OVERLINE );
    nPos = m_pOverlineLB->GetSelectEntryPos();
    FontUnderline eOver = (FontUnderline)(sal_uLong)m_pOverlineLB->GetEntryData( nPos );

    if ( pOld )
    {
        //! if there are different underline styles in the selection the
        //! item-state in the 'rOldSet' will be invalid. In this case
        //! changing the underline style will be allowed if a style is
        //! selected in the listbox.
        sal_Bool bAllowChg = LISTBOX_ENTRY_NOTFOUND != nPos  &&
                         SFX_ITEM_DEFAULT > rOldSet.GetItemState( nWhich, sal_True );

        const SvxOverlineItem& rItem = *( (const SvxOverlineItem*)pOld );
        if ( (FontUnderline)rItem.GetValue() == eOver &&
             ( UNDERLINE_NONE == eOver || rItem.GetColor() == m_pOverlineColorLB->GetSelectEntryColor() ) &&
             ! bAllowChg )
            bChanged = false;
    }

    if ( bChanged )
    {
        SvxOverlineItem aNewItem( eOver, nWhich );
        aNewItem.SetColor( m_pOverlineColorLB->GetSelectEntryColor() );
        rSet.Put( aNewItem );
        bModified = sal_True;
    }
     else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, sal_False ) )
        CLEARTITEM;

    bChanged = true;

    // Strikeout
    nWhich = GetWhich( SID_ATTR_CHAR_STRIKEOUT );
    pOld = GetOldItem( rSet, SID_ATTR_CHAR_STRIKEOUT );
    nPos = m_pStrikeoutLB->GetSelectEntryPos();
    FontStrikeout eStrike = (FontStrikeout)(sal_uLong)m_pStrikeoutLB->GetEntryData( nPos );

    if ( pOld )
    {
        //! if there are different strikeout styles in the selection the
        //! item-state in the 'rOldSet' will be invalid. In this case
        //! changing the strikeout style will be allowed if a style is
        //! selected in the listbox.
        sal_Bool bAllowChg = LISTBOX_ENTRY_NOTFOUND != nPos  &&
                         SFX_ITEM_DEFAULT > rOldSet.GetItemState( nWhich, sal_True );

        const SvxCrossedOutItem& rItem = *( (const SvxCrossedOutItem*)pOld );
        if ( !m_pStrikeoutLB->IsEnabled()
            || ((FontStrikeout)rItem.GetValue() == eStrike  && !bAllowChg) )
            bChanged = false;
    }

    if ( bChanged )
    {
        rSet.Put( SvxCrossedOutItem( eStrike, nWhich ) );
        bModified = sal_True;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, sal_False ) )
        CLEARTITEM;

    bChanged = true;

    // Individual words
    nWhich = GetWhich( SID_ATTR_CHAR_WORDLINEMODE );
    pOld = GetOldItem( rSet, SID_ATTR_CHAR_WORDLINEMODE );

    if ( pOld )
    {
        const SvxWordLineModeItem& rItem = *( (const SvxWordLineModeItem*)pOld );
        if ( rItem.GetValue() == m_pIndividualWordsBtn->IsChecked() )
            bChanged = false;
    }

    if ( rOldSet.GetItemState( nWhich ) == SFX_ITEM_DONTCARE &&
         m_pIndividualWordsBtn->IsChecked() == m_pIndividualWordsBtn->GetSavedValue() )
        bChanged = false;

    if ( bChanged )
    {
        rSet.Put( SvxWordLineModeItem( m_pIndividualWordsBtn->IsChecked(), nWhich ) );
        bModified = sal_True;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, sal_False ) )
        CLEARTITEM;

    bChanged = true;

    // Emphasis
    nWhich = GetWhich( SID_ATTR_CHAR_EMPHASISMARK );
    pOld = GetOldItem( rSet, SID_ATTR_CHAR_EMPHASISMARK );
    sal_uInt16 nMarkPos = m_pEmphasisLB->GetSelectEntryPos();
    sal_uInt16 nPosPos = m_pPositionLB->GetSelectEntryPos();
    FontEmphasisMark eMark = (FontEmphasisMark)nMarkPos;
    if ( m_pPositionLB->IsEnabled() )
    {
        eMark |= ( CHRDLG_POSITION_UNDER == (sal_uLong)m_pPositionLB->GetEntryData( nPosPos ) )
            ? EMPHASISMARK_POS_BELOW : EMPHASISMARK_POS_ABOVE;
    }

    if ( pOld )
    {
        if( rOldSet.GetItemState( nWhich ) != SFX_ITEM_DONTCARE )
        {
            const SvxEmphasisMarkItem& rItem = *( (const SvxEmphasisMarkItem*)pOld );
            if ( rItem.GetEmphasisMark() == eMark )
                bChanged = false;
        }
    }

    if ( rOldSet.GetItemState( nWhich ) == SFX_ITEM_DONTCARE &&
         m_pEmphasisLB->GetSavedValue() == nMarkPos && m_pPositionLB->GetSavedValue() == nPosPos )
        bChanged = false;

    if ( bChanged )
    {
        rSet.Put( SvxEmphasisMarkItem( eMark, nWhich ) );
        bModified = sal_True;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, sal_False ) )
        CLEARTITEM;

    bChanged = true;

    // Effects
    nWhich = GetWhich( SID_ATTR_CHAR_CASEMAP );
    pOld = GetOldItem( rSet, SID_ATTR_CHAR_CASEMAP );
    SvxCaseMap eCaseMap = SVX_CASEMAP_NOT_MAPPED;
    bool bChecked = false;
    sal_uInt16 nCapsPos = m_pEffectsLB->GetSelectEntryPos();
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
        sal_Bool bAllowChg = LISTBOX_ENTRY_NOTFOUND != nPos  &&
                         SFX_ITEM_DEFAULT > rOldSet.GetItemState( nWhich, sal_True );

        const SvxCaseMapItem& rItem = *( (const SvxCaseMapItem*)pOld );
        if ( (SvxCaseMap)rItem.GetValue() == eCaseMap  &&  !bAllowChg )
            bChanged = false;
    }

    if ( bChanged && bChecked )
    {
        rSet.Put( SvxCaseMapItem( eCaseMap, nWhich ) );
        bModified = sal_True;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, sal_False ) )
        CLEARTITEM;

    bChanged = true;

    //Relief
    nWhich = GetWhich(SID_ATTR_CHAR_RELIEF);
    if(m_pReliefLB->GetSelectEntryPos() != m_pReliefLB->GetSavedValue())
    {
        m_pReliefLB->SaveValue();
        SvxCharReliefItem aRelief((FontRelief)m_pReliefLB->GetSelectEntryPos(), nWhich);
        rSet.Put(aRelief);
    }

    // Outline
    const SfxItemSet* pExampleSet = GetTabDialog() ? GetTabDialog()->GetExampleSet() : NULL;
    nWhich = GetWhich( SID_ATTR_CHAR_CONTOUR );
    pOld = GetOldItem( rSet, SID_ATTR_CHAR_CONTOUR );
    TriState eState = m_pOutlineBtn->GetState();
    const SfxPoolItem* pItem;

    if ( pOld )
    {
        const SvxContourItem& rItem = *( (const SvxContourItem*)pOld );
        if ( rItem.GetValue() == StateToAttr( eState ) && m_pOutlineBtn->GetSavedValue() == eState )
            bChanged = false;
    }

    if ( !bChanged && pExampleSet && pExampleSet->GetItemState( nWhich, sal_False, &pItem ) == SFX_ITEM_SET &&
         !StateToAttr( eState ) && ( (SvxContourItem*)pItem )->GetValue() )
        bChanged = true;

    if ( bChanged && eState != STATE_DONTKNOW )
    {
        rSet.Put( SvxContourItem( StateToAttr( eState ), nWhich ) );
        bModified = sal_True;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, sal_False ) )
        CLEARTITEM;

    bChanged = true;

    // Shadow
    nWhich = GetWhich( SID_ATTR_CHAR_SHADOWED );
    pOld = GetOldItem( rSet, SID_ATTR_CHAR_SHADOWED );
    eState = m_pShadowBtn->GetState();

    if ( pOld )
    {
        const SvxShadowedItem& rItem = *( (const SvxShadowedItem*)pOld );
        if ( rItem.GetValue() == StateToAttr( eState ) && m_pShadowBtn->GetSavedValue() == eState )
            bChanged = false;
    }

    if ( !bChanged && pExampleSet && pExampleSet->GetItemState( nWhich, sal_False, &pItem ) == SFX_ITEM_SET &&
         !StateToAttr( eState ) && ( (SvxShadowedItem*)pItem )->GetValue() )
        bChanged = true;

    if ( bChanged && eState != STATE_DONTKNOW )
    {
        rSet.Put( SvxShadowedItem( StateToAttr( eState ), nWhich ) );
        bModified = sal_True;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, sal_False ) )
        CLEARTITEM;

    bChanged = true;

    // Blinking
    nWhich = GetWhich( SID_ATTR_FLASH );
    pOld = GetOldItem( rSet, SID_ATTR_FLASH );
    eState = m_pBlinkingBtn->GetState();

    if ( pOld )
    {
        const SvxBlinkItem& rItem = *( (const SvxBlinkItem*)pOld );
        if ( rItem.GetValue() == StateToAttr( eState ) && m_pBlinkingBtn->GetSavedValue() == eState )
            bChanged = false;
    }

    if ( !bChanged && pExampleSet && pExampleSet->GetItemState( nWhich, sal_False, &pItem ) == SFX_ITEM_SET &&
         !StateToAttr( eState ) && ( (SvxBlinkItem*)pItem )->GetValue() )
        bChanged = true;

    if ( bChanged && eState != STATE_DONTKNOW )
    {
        rSet.Put( SvxBlinkItem( StateToAttr( eState ), nWhich ) );
        bModified = true;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, sal_False ) )
        CLEARTITEM;

    // Hidden
    nWhich = GetWhich( SID_ATTR_CHAR_HIDDEN );
    pOld = GetOldItem( rSet, SID_ATTR_CHAR_HIDDEN );
    eState = m_pHiddenBtn->GetState();
    bChanged = true;

    if ( pOld )
    {
        const SvxCharHiddenItem& rItem = *( (const SvxCharHiddenItem*)pOld );
        if ( rItem.GetValue() == StateToAttr( eState ) && m_pHiddenBtn->GetSavedValue() == eState )
            bChanged = false;
    }

    if ( !bChanged && pExampleSet && pExampleSet->GetItemState( nWhich, sal_False, &pItem ) == SFX_ITEM_SET &&
         !StateToAttr( eState ) && ( (SvxCharHiddenItem*)pItem )->GetValue() )
        bChanged = true;

    if ( bChanged && eState != STATE_DONTKNOW )
    {
        rSet.Put( SvxCharHiddenItem( StateToAttr( eState ), nWhich ) );
        bModified = sal_True;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, sal_False ) )
        CLEARTITEM;

    bModified |= FillItemSetColor_Impl( rSet );

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

// -----------------------------------------------------------------------
void SvxCharEffectsPage::SetPreviewBackgroundToCharacter()
{
    m_bPreviewBackgroundToCharacter = sal_True;
}

// -----------------------------------------------------------------------
void SvxCharEffectsPage::PageCreated (SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pDisableCtlItem,SfxUInt16Item,SID_DISABLE_CTL,sal_False);
    SFX_ITEMSET_ARG (&aSet,pFlagItem,SfxUInt32Item,SID_FLAG_TYPE,sal_False);
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

SvxCharPositionPage::SvxCharPositionPage( Window* pParent, const SfxItemSet& rInSet )
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

// -----------------------------------------------------------------------

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
    KerningSelectHdl_Impl( NULL );

    Link aLink = LINK( this, SvxCharPositionPage, PositionHdl_Impl );
    m_pHighPosBtn->SetClickHdl( aLink );
    m_pNormalPosBtn->SetClickHdl( aLink );
    m_pLowPosBtn->SetClickHdl( aLink );

    aLink = LINK( this, SvxCharPositionPage, RotationHdl_Impl );
    m_p0degRB->SetClickHdl( aLink );
    m_p90degRB->SetClickHdl( aLink );
    m_p270degRB->SetClickHdl( aLink );

    aLink = LINK( this, SvxCharPositionPage, FontModifyHdl_Impl );
    m_pHighLowMF->SetModifyHdl( aLink );
    m_pFontSizeMF->SetModifyHdl( aLink );

    aLink = LINK( this, SvxCharPositionPage, LoseFocusHdl_Impl );
    m_pHighLowMF->SetLoseFocusHdl( aLink );
    m_pFontSizeMF->SetLoseFocusHdl( aLink );

    m_pHighLowRB->SetClickHdl( LINK( this, SvxCharPositionPage, AutoPositionHdl_Impl ) );
    m_pFitToLineCB->SetClickHdl( LINK( this, SvxCharPositionPage, FitToLineHdl_Impl ) );
    m_pKerningLB->SetSelectHdl( LINK( this, SvxCharPositionPage, KerningSelectHdl_Impl ) );
    m_pKerningMF->SetModifyHdl( LINK( this, SvxCharPositionPage, KerningModifyHdl_Impl ) );
    m_pPairKerningBtn->SetClickHdl( LINK( this, SvxCharPositionPage, PairKerningHdl_Impl ) );
    m_pScaleWidthMF->SetModifyHdl( LINK( this, SvxCharPositionPage, ScaleWidthModifyHdl_Impl ) );
}

SvxCharPositionPage::~SvxCharPositionPage()
{
}

// -----------------------------------------------------------------------

void SvxCharPositionPage::UpdatePreview_Impl( sal_uInt8 nProp, sal_uInt8 nEscProp, short nEsc )
{
    SetPrevFontEscapement( nProp, nEscProp, nEsc );
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharPositionPage, PositionHdl_Impl, RadioButton*, pBtn )
{
    sal_uInt16 nEsc = SVX_ESCAPEMENT_OFF;   // also when pBtn == NULL

    if ( m_pHighPosBtn == pBtn )
        nEsc = SVX_ESCAPEMENT_SUPERSCRIPT;
    else if ( m_pLowPosBtn == pBtn )
        nEsc = SVX_ESCAPEMENT_SUBSCRIPT;

    SetEscapement_Impl( nEsc );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharPositionPage, RotationHdl_Impl, RadioButton*, pBtn )
{
    sal_Bool bEnable = sal_False;
    if (m_p90degRB == pBtn  || m_p270degRB == pBtn)
        bEnable = sal_True;
    else
        OSL_ENSURE( m_p0degRB == pBtn, "unexpected button" );
    m_pFitToLineCB->Enable( bEnable );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxCharPositionPage, FontModifyHdl_Impl)
{
    sal_uInt8 nEscProp = (sal_uInt8)m_pFontSizeMF->GetValue();
    short nEsc  = (short)m_pHighLowMF->GetValue();
    nEsc *= m_pLowPosBtn->IsChecked() ? -1 : 1;
    UpdatePreview_Impl( 100, nEscProp, nEsc );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharPositionPage, AutoPositionHdl_Impl, CheckBox*, pBox )
{
    if ( pBox->IsChecked() )
    {
        m_pHighLowFT->Disable();
        m_pHighLowMF->Disable();
    }
    else
        PositionHdl_Impl( m_pHighPosBtn->IsChecked() ? m_pHighPosBtn
                                                      : m_pLowPosBtn->IsChecked() ? m_pLowPosBtn
                                                                                   : m_pNormalPosBtn );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharPositionPage, FitToLineHdl_Impl, CheckBox*, pBox )
{
    if (m_pFitToLineCB == pBox)
    {
        sal_uInt16 nVal = m_nScaleWidthInitialVal;
        if (m_pFitToLineCB->IsChecked())
            nVal = m_nScaleWidthItemSetVal;
        m_pScaleWidthMF->SetValue( nVal );

        m_pPreviewWin->SetFontWidthScale( nVal );
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxCharPositionPage, KerningSelectHdl_Impl)
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

    KerningModifyHdl_Impl( NULL );

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxCharPositionPage, KerningModifyHdl_Impl)
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
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxCharPositionPage, PairKerningHdl_Impl)
{
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharPositionPage, LoseFocusHdl_Impl, MetricField*, pField )
{
#ifdef DBG_UTIL
    sal_Bool bHigh = m_pHighPosBtn->IsChecked();
#endif
    sal_Bool bLow = m_pLowPosBtn->IsChecked();
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
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxCharPositionPage, ScaleWidthModifyHdl_Impl)
{
    m_pPreviewWin->SetFontWidthScale( sal_uInt16( m_pScaleWidthMF->GetValue() ) );

    return 0;
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

// -----------------------------------------------------------------------

int SvxCharPositionPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( *_pSet );
    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

SfxTabPage* SvxCharPositionPage::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new SvxCharPositionPage( pParent, rSet );
}

// -----------------------------------------------------------------------

sal_uInt16* SvxCharPositionPage::GetRanges()
{
    return pPositionRanges;
}

// -----------------------------------------------------------------------
void SvxCharPositionPage::Reset( const SfxItemSet& rSet )
{
    String sUser = GetUserData();

    if ( sUser.Len() )
    {
        m_nSuperEsc = (short)sUser.GetToken( 0 ).ToInt32();
        m_nSubEsc = (short)sUser.GetToken( 1 ).ToInt32();
        m_nSuperProp = (sal_uInt8)sUser.GetToken( 2 ).ToInt32();
        m_nSubProp = (sal_uInt8)sUser.GetToken( 3 ).ToInt32();
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

    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxEscapementItem& rItem = (SvxEscapementItem&)rSet.Get( nWhich );
        nEsc = rItem.GetEsc();
        nEscProp = rItem.GetProp();

        if ( nEsc != 0 )
        {
            m_pHighLowFT->Enable();
            m_pHighLowMF->Enable();
            m_pFontSizeFT->Enable();
            m_pFontSizeMF->Enable();

            short nFac;
            sal_Bool bAutomatic(sal_False);

            if ( nEsc > 0 )
            {
                nFac = 1;
                m_pHighPosBtn->Check( sal_True );
                if ( nEsc == DFLT_ESC_AUTO_SUPER )
                {
                    nEsc = DFLT_ESC_SUPER;
                    bAutomatic = sal_True;
                }
            }
            else
            {
                nFac = -1;
                m_pLowPosBtn->Check( sal_True );
                if ( nEsc == DFLT_ESC_AUTO_SUB )
                {
                    nEsc = DFLT_ESC_SUB;
                    bAutomatic = sal_True;
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
            m_pNormalPosBtn->Check( sal_True );
            m_pHighLowRB->Check( sal_True );
            PositionHdl_Impl( NULL );
        }
        //the height has to be set after the handler is called to keep the value also if the escapement is zero
        m_pFontSizeMF->SetValue( m_pFontSizeMF->Normalize( nEscProp ) );
    }
    else
    {
        m_pHighPosBtn->Check( sal_False );
        m_pNormalPosBtn->Check( sal_False );
        m_pLowPosBtn->Check( sal_False );
    }

    // set BspFont
    SetPrevFontEscapement( nProp, nEscProp, nEsc );

    // Kerning
    nWhich = GetWhich( SID_ATTR_CHAR_KERNING );

    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxKerningItem& rItem = (SvxKerningItem&)rSet.Get( nWhich );
        SfxMapUnit eUnit = rSet.GetPool()->GetMetric( nWhich );
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
        KerningSelectHdl_Impl(m_pKerningLB);
        //the attribute value must be displayed also if it's above the maximum allowed value
        long nVal = static_cast<long>(m_pKerningMF->GetMax());
        if(nVal < nKerning)
            m_pKerningMF->SetMax( nKerning );
        m_pKerningMF->SetValue( nKerning );
    }
    else
        m_pKerningMF->SetText( String() );

    // Pair kerning
    nWhich = GetWhich( SID_ATTR_CHAR_AUTOKERN );

    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxAutoKernItem& rItem = (SvxAutoKernItem&)rSet.Get( nWhich );
        m_pPairKerningBtn->Check( rItem.GetValue() );
    }
    else
        m_pPairKerningBtn->Check( sal_False );

    // Scale Width
    nWhich = GetWhich( SID_ATTR_CHAR_SCALEWIDTH );
    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxCharScaleWidthItem& rItem = ( SvxCharScaleWidthItem& ) rSet.Get( nWhich );
        m_nScaleWidthInitialVal = rItem.GetValue();
        m_pScaleWidthMF->SetValue( m_nScaleWidthInitialVal );
    }
    else
        m_pScaleWidthMF->SetValue( 100 );

    nWhich = GetWhich( SID_ATTR_CHAR_WIDTH_FIT_TO_LINE );
    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
        m_nScaleWidthItemSetVal = ((SfxUInt16Item&) rSet.Get( nWhich )).GetValue();

    // Rotation
    nWhich = GetWhich( SID_ATTR_CHAR_ROTATED );
    SfxItemState eState = rSet.GetItemState( nWhich );
    if( SFX_ITEM_UNKNOWN == eState )
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

        Link aOldLink( m_pFitToLineCB->GetClickHdl() );
        m_pFitToLineCB->SetClickHdl( Link() );
        if( eState >= SFX_ITEM_DEFAULT )
        {
            const SvxCharRotateItem& rItem =
                    (SvxCharRotateItem&) rSet.Get( nWhich );
            if (rItem.IsBottomToTop())
                m_p90degRB->Check( sal_True );
            else if (rItem.IsTopToBotton())
                m_p270degRB->Check( sal_True );
            else
            {
                DBG_ASSERT( 0 == rItem.GetValue(), "incorrect value" );
                m_p0degRB->Check( sal_True );
            }
            m_pFitToLineCB->Check( rItem.IsFitToLine() );
        }
        else
        {
            if( eState == SFX_ITEM_DONTCARE )
            {
                m_p0degRB->Check( sal_False );
                m_p90degRB->Check( sal_False );
                m_p270degRB->Check( sal_False );
            }
            else
                m_p0degRB->Check( sal_True );

            m_pFitToLineCB->Check( sal_False );
        }
        m_pFitToLineCB->SetClickHdl( aOldLink );
        m_pFitToLineCB->Enable( !m_p0degRB->IsChecked() );

        // is this value set?
        if( SFX_ITEM_UNKNOWN == rSet.GetItemState( GetWhich(
                                        SID_ATTR_CHAR_WIDTH_FIT_TO_LINE ) ))
            m_pFitToLineCB->Hide();
    }

    m_pHighPosBtn->SaveValue();
    m_pNormalPosBtn->SaveValue();
    m_pLowPosBtn->SaveValue();
    m_p0degRB->SaveValue();
    m_p90degRB->SaveValue();
    m_p270degRB->SaveValue();
    m_pFitToLineCB->SaveValue();
    m_pScaleWidthMF->SaveValue();
    m_pKerningLB->SaveValue();
    m_pKerningMF->SaveValue();
    m_pPairKerningBtn->SaveValue();
}

// -----------------------------------------------------------------------

sal_Bool SvxCharPositionPage::FillItemSet( SfxItemSet& rSet )
{
    //  Position (high, normal or low)
    const SfxItemSet& rOldSet = GetItemSet();
    sal_Bool bModified = sal_False, bChanged = sal_True;
    sal_uInt16 nWhich = GetWhich( SID_ATTR_CHAR_ESCAPEMENT );
    const SfxPoolItem* pOld = GetOldItem( rSet, SID_ATTR_CHAR_ESCAPEMENT );
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
        const SvxEscapementItem& rItem = *( (const SvxEscapementItem*)pOld );
        if ( rItem.GetEsc() == nEsc && rItem.GetProp() == nEscProp  )
            bChanged = sal_False;
    }

    if ( !bChanged && !m_pHighPosBtn->GetSavedValue() &&
         !m_pNormalPosBtn->GetSavedValue() && !m_pLowPosBtn->GetSavedValue() )
        bChanged = sal_True;

    if ( bChanged &&
         ( m_pHighPosBtn->IsChecked() || m_pNormalPosBtn->IsChecked() || m_pLowPosBtn->IsChecked() ) )
    {
        rSet.Put( SvxEscapementItem( nEsc, nEscProp, nWhich ) );
        bModified = sal_True;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, sal_False ) )
        CLEARTITEM;

    bChanged = sal_True;

    // Kerning
    nWhich = GetWhich( SID_ATTR_CHAR_KERNING );
    pOld = GetOldItem( rSet, SID_ATTR_CHAR_KERNING );
    sal_uInt16 nPos = m_pKerningLB->GetSelectEntryPos();
    short nKerning = 0;
    SfxMapUnit eUnit = rSet.GetPool()->GetMetric( nWhich );

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
        const SvxKerningItem& rItem = *( (const SvxKerningItem*)pOld );
        if ( rItem.GetValue() == nKerning )
            bChanged = sal_False;
    }

    if ( !bChanged &&
         ( m_pKerningLB->GetSavedValue() == LISTBOX_ENTRY_NOTFOUND ||
           ( m_pKerningMF->GetSavedValue().isEmpty() && m_pKerningMF->IsEnabled() ) ) )
        bChanged = sal_True;

    if ( bChanged && nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        rSet.Put( SvxKerningItem( nKerning, nWhich ) );
        bModified = sal_True;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, sal_False ) )
        CLEARTITEM;

    bChanged = sal_True;

    // Pair-Kerning
    nWhich = GetWhich( SID_ATTR_CHAR_AUTOKERN );

    if ( m_pPairKerningBtn->IsChecked() != m_pPairKerningBtn->GetSavedValue() )
    {
        rSet.Put( SvxAutoKernItem( m_pPairKerningBtn->IsChecked(), nWhich ) );
        bModified = sal_True;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, sal_False ) )
        CLEARTITEM;

    // Scale Width
    nWhich = GetWhich( SID_ATTR_CHAR_SCALEWIDTH );
    if ( m_pScaleWidthMF->GetText() != m_pScaleWidthMF->GetSavedValue() )
    {
        rSet.Put( SvxCharScaleWidthItem( (sal_uInt16)m_pScaleWidthMF->GetValue(), nWhich ) );
        bModified = sal_True;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, sal_False ) )
        CLEARTITEM;

    // Rotation
    nWhich = GetWhich( SID_ATTR_CHAR_ROTATED );
    if ( m_p0degRB->IsChecked() != m_p0degRB->GetSavedValue()  ||
         m_p90degRB->IsChecked() != m_p90degRB->GetSavedValue()  ||
         m_p270degRB->IsChecked() != m_p270degRB->GetSavedValue()  ||
         m_pFitToLineCB->IsChecked() != m_pFitToLineCB->GetSavedValue() )
    {
        SvxCharRotateItem aItem( 0, m_pFitToLineCB->IsChecked(), nWhich );
        if (m_p90degRB->IsChecked())
            aItem.SetBottomToTop();
        else if (m_p270degRB->IsChecked())
            aItem.SetTopToBotton();
        rSet.Put( aItem );
        bModified = sal_True;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, sal_False ) )
        CLEARTITEM;

    return bModified;
}

// -----------------------------------------------------------------------

void SvxCharPositionPage::FillUserData()
{
    const OUString cTok( ";" );

    OUString sUser = OUString::number( m_nSuperEsc )  + cTok +
                     OUString::number( m_nSubEsc )    + cTok +
                     OUString::number( m_nSuperProp ) + cTok +
                     OUString::number( m_nSubProp );
    SetUserData( sUser );
}

// -----------------------------------------------------------------------
void SvxCharPositionPage::SetPreviewBackgroundToCharacter()
{
    m_bPreviewBackgroundToCharacter = sal_True;
}
// -----------------------------------------------------------------------
void SvxCharPositionPage::PageCreated (SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pFlagItem,SfxUInt32Item,SID_FLAG_TYPE,sal_False);
    if (pFlagItem)
    {
        sal_uInt32 nFlags=pFlagItem->GetValue();
        if ( ( nFlags & SVX_PREVIEW_CHARACTER ) == SVX_PREVIEW_CHARACTER )
            SetPreviewBackgroundToCharacter();
    }
}
// class SvxCharTwoLinesPage ------------------------------------------------

SvxCharTwoLinesPage::SvxCharTwoLinesPage(Window* pParent, const SfxItemSet& rInSet)
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
}

// -----------------------------------------------------------------------

void SvxCharTwoLinesPage::Initialize()
{
    m_pTwoLinesBtn->Check( sal_False );
    TwoLinesHdl_Impl( NULL );

    m_pTwoLinesBtn->SetClickHdl( LINK( this, SvxCharTwoLinesPage, TwoLinesHdl_Impl ) );

    Link aLink = LINK( this, SvxCharTwoLinesPage, CharacterMapHdl_Impl );
    m_pStartBracketLB->SetSelectHdl( aLink );
    m_pEndBracketLB->SetSelectHdl( aLink );

    SvxFont& rFont = GetPreviewFont();
    SvxFont& rCJKFont = GetPreviewCJKFont();
    SvxFont& rCTLFont = GetPreviewCTLFont();
    rFont.SetSize( Size( 0, 220 ) );
    rCJKFont.SetSize( Size( 0, 220 ) );
    rCTLFont.SetSize( Size( 0, 220 ) );
}

// -----------------------------------------------------------------------

void SvxCharTwoLinesPage::SelectCharacter( ListBox* pBox )
{
    bool bStart = pBox == m_pStartBracketLB;
    SvxCharacterMap* aDlg = new SvxCharacterMap( this );
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
    delete aDlg;
}

// -----------------------------------------------------------------------

void SvxCharTwoLinesPage::SetBracket( sal_Unicode cBracket, sal_Bool bStart )
{
    sal_uInt16 nEntryPos = 0;
    ListBox* pBox = bStart ? m_pStartBracketLB : m_pEndBracketLB;
    if ( 0 == cBracket )
        pBox->SelectEntryPos(0);
    else
    {
        bool bFound = false;
        for ( sal_uInt16 i = 1; i < pBox->GetEntryCount(); ++i )
        {
            if ( (sal_uLong)pBox->GetEntryData(i) != CHRDLG_ENCLOSE_SPECIAL_CHAR )
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

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxCharTwoLinesPage, TwoLinesHdl_Impl)
{
    sal_Bool bChecked = m_pTwoLinesBtn->IsChecked();
    m_pEnclosingFrame->Enable( bChecked );

    UpdatePreview_Impl();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharTwoLinesPage, CharacterMapHdl_Impl, ListBox*, pBox )
{
    sal_uInt16 nPos = pBox->GetSelectEntryPos();
    if ( CHRDLG_ENCLOSE_SPECIAL_CHAR == (sal_uLong)pBox->GetEntryData( nPos ) )
        SelectCharacter( pBox );
    else
    {
        bool bStart = pBox == m_pStartBracketLB;
        if( bStart )
            m_nStartBracketPosition = nPos;
        else
            m_nEndBracketPosition = nPos;
    }
    UpdatePreview_Impl();
    return 0;
}

// -----------------------------------------------------------------------

void SvxCharTwoLinesPage::ActivatePage( const SfxItemSet& rSet )
{
    SvxCharBasePage::ActivatePage( rSet );
}

// -----------------------------------------------------------------------

int SvxCharTwoLinesPage::DeactivatePage( SfxItemSet* _pSet )
{
    if ( _pSet )
        FillItemSet( *_pSet );
    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

SfxTabPage* SvxCharTwoLinesPage::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new SvxCharTwoLinesPage( pParent, rSet );
}

// -----------------------------------------------------------------------

sal_uInt16* SvxCharTwoLinesPage::GetRanges()
{
    return pTwoLinesRanges;
}

// -----------------------------------------------------------------------

void SvxCharTwoLinesPage::Reset( const SfxItemSet& rSet )
{
    m_pTwoLinesBtn->Check( sal_False );
    sal_uInt16 nWhich = GetWhich( SID_ATTR_CHAR_TWO_LINES );
    SfxItemState eState = rSet.GetItemState( nWhich );

    if ( eState >= SFX_ITEM_DONTCARE )
    {
        const SvxTwoLinesItem& rItem = (SvxTwoLinesItem&)rSet.Get( nWhich );
        m_pTwoLinesBtn->Check( rItem.GetValue() );

        if ( rItem.GetValue() )
        {
            SetBracket( rItem.GetStartBracket(), sal_True );
            SetBracket( rItem.GetEndBracket(), sal_False );
        }
    }
    TwoLinesHdl_Impl( NULL );

    SetPrevFontWidthScale( rSet );
}

// -----------------------------------------------------------------------

sal_Bool SvxCharTwoLinesPage::FillItemSet( SfxItemSet& rSet )
{
    const SfxItemSet& rOldSet = GetItemSet();
    sal_Bool bModified = sal_False, bChanged = sal_True;
    sal_uInt16 nWhich = GetWhich( SID_ATTR_CHAR_TWO_LINES );
    const SfxPoolItem* pOld = GetOldItem( rSet, SID_ATTR_CHAR_TWO_LINES );
    sal_Bool bOn = m_pTwoLinesBtn->IsChecked();
    sal_Unicode cStart = ( bOn && m_pStartBracketLB->GetSelectEntryPos() > 0 )
        ? m_pStartBracketLB->GetSelectEntry()[0] : 0;
    sal_Unicode cEnd = ( bOn && m_pEndBracketLB->GetSelectEntryPos() > 0 )
        ? m_pEndBracketLB->GetSelectEntry()[0] : 0;

    if ( pOld )
    {
        const SvxTwoLinesItem& rItem = *( (const SvxTwoLinesItem*)pOld );
        if ( rItem.GetValue() ==  bOn &&
             ( !bOn || ( rItem.GetStartBracket() == cStart && rItem.GetEndBracket() == cEnd ) ) )
            bChanged = sal_False;
    }

    if ( bChanged )
    {
        rSet.Put( SvxTwoLinesItem( bOn, cStart, cEnd, nWhich ) );
        bModified = sal_True;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, sal_False ) )
        CLEARTITEM;

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
// -----------------------------------------------------------------------
void SvxCharTwoLinesPage::SetPreviewBackgroundToCharacter()
{
    m_bPreviewBackgroundToCharacter = sal_True;
}

// -----------------------------------------------------------------------
void SvxCharTwoLinesPage::PageCreated (SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pFlagItem,SfxUInt32Item,SID_FLAG_TYPE,sal_False);
    if (pFlagItem)
    {
        sal_uInt32 nFlags=pFlagItem->GetValue();
        if ( ( nFlags & SVX_PREVIEW_CHARACTER ) == SVX_PREVIEW_CHARACTER )
            SetPreviewBackgroundToCharacter();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
