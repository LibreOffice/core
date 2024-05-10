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

#include <sal/config.h>

#include <stdio.h>

#include <utility>
#include <vcl/svapp.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/itempool.hxx>

#include <rtl/textenc.h>
#include <svx/ucsubset.hxx>
#include <vcl/settings.hxx>
#include <vcl/fontcharmap.hxx>
#include <vcl/virdev.hxx>
#include <svl/stritem.hxx>
#include <o3tl/temporary.hxx>
#include <officecfg/Office/Common.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/dispatchcommand.hxx>

#include <svx/dialmgr.hxx>
#include <svx/cuicharmap.hxx>
#include <svx/strings.hrc>
#include <sfx2/app.hxx>
#include <svx/svxids.hrc>
#include <editeng/editids.hrc>
#include <editeng/fontitem.hxx>
#include <unicode/uchar.h>
#include <unicode/utypes.h>

using namespace css;

SvxCharacterMap::SvxCharacterMap(weld::Widget* pParent, const SfxItemSet* pSet,
                                 css::uno::Reference<css::frame::XFrame> xFrame)
    : SfxDialogController(pParent, u"cui/ui/specialcharacters.ui"_ustr, u"SpecialCharactersDialog"_ustr)
    , m_xVirDev(VclPtr<VirtualDevice>::Create())
    , isSearchMode(true)
    , m_xFrame(std::move(xFrame))
    , m_aCharmapContents(*m_xBuilder, m_xVirDev, true)
    , m_aShowChar(m_xVirDev)
    , m_xOKBtn(m_xFrame.is() ? m_xBuilder->weld_button(u"insert"_ustr) : m_xBuilder->weld_button(u"ok"_ustr))
    , m_xFontText(m_xBuilder->weld_label(u"fontft"_ustr))
    , m_xFontLB(m_xBuilder->weld_combo_box(u"fontlb"_ustr))
    , m_xSubsetText(m_xBuilder->weld_label(u"subsetft"_ustr))
    , m_xSubsetLB(m_xBuilder->weld_combo_box(u"subsetlb"_ustr))
    , m_xSearchText(m_xBuilder->weld_entry(u"search"_ustr))
    , m_xHexCodeText(m_xBuilder->weld_entry(u"hexvalue"_ustr))
    , m_xDecimalCodeText(m_xBuilder->weld_entry(u"decimalvalue"_ustr))
    , m_xFavouritesBtn(m_xBuilder->weld_button(u"favbtn"_ustr))
    , m_xCharName(m_xBuilder->weld_label(u"charname"_ustr))
    , m_xShowChar(new weld::CustomWeld(*m_xBuilder, u"showchar"_ustr, m_aShowChar))
    , m_xShowSet(new SvxShowCharSet(m_xBuilder->weld_scrolled_window(u"showscroll"_ustr, true), m_xVirDev))
    , m_xShowSetArea(new weld::CustomWeld(*m_xBuilder, u"showcharset"_ustr, *m_xShowSet))
    , m_xSearchSet(new SvxSearchCharSet(m_xBuilder->weld_scrolled_window(u"searchscroll"_ustr, true), m_xVirDev))
    , m_xSearchSetArea(new weld::CustomWeld(*m_xBuilder, u"searchcharset"_ustr, *m_xSearchSet))
{
    m_aShowChar.SetCentered(true);
    m_xFontLB->make_sorted();
    //lock the size request of this widget to the width of all possible entries
    fillAllSubsets(*m_xSubsetLB);
    m_xSubsetLB->set_size_request(m_xSubsetLB->get_preferred_size().Width(), -1);
    m_xCharName->set_size_request(m_aShowChar.get_preferred_size().Width(), m_xCharName->get_text_height() * 4);
    //lock the size request of this widget to the width of the original .ui string
    m_xHexCodeText->set_size_request(m_xHexCodeText->get_preferred_size().Width(), -1);

    init();

    const SfxInt32Item* pCharItem = SfxItemSet::GetItem<SfxInt32Item>(pSet, SID_ATTR_CHAR, false);
    if ( pCharItem )
        SetChar( pCharItem->GetValue() );

    const SfxBoolItem* pDisableItem = SfxItemSet::GetItem<SfxBoolItem>(pSet, FN_PARAM_2, false);
    if ( pDisableItem && pDisableItem->GetValue() )
        DisableFontSelection();

    const SvxFontItem* pFontItem = SfxItemSet::GetItem<SvxFontItem>(pSet, SID_ATTR_CHAR_FONT, false);
    const SfxStringItem* pFontNameItem = SfxItemSet::GetItem<SfxStringItem>(pSet, SID_FONT_NAME, false);
    if ( pFontItem )
    {
        vcl::Font aTmpFont( pFontItem->GetFamilyName(), pFontItem->GetStyleName(), GetCharFont().GetFontSize() );
        aTmpFont.SetCharSet( pFontItem->GetCharSet() );
        aTmpFont.SetPitch( pFontItem->GetPitch() );
        SetCharFont( aTmpFont );
    }
    else if ( pFontNameItem )
    {
        vcl::Font aTmpFont( GetCharFont() );
        aTmpFont.SetFamilyName( pFontNameItem->GetValue() );
        SetCharFont( aTmpFont );
    }

    m_xOutputSet.reset(new SfxAllItemSet(pSet ? *pSet->GetPool() : SfxGetpApp()->GetPool()));
    m_xShowSet->Show();
    m_xSearchSet->Hide();
}

short SvxCharacterMap::run()
{
    if( SvxShowCharSet::getSelectedChar() == ' ')
    {
        m_xOKBtn->set_sensitive(false);
        setFavButtonState(u"", u"");
    }
    else
    {
        sal_UCS4 cChar = m_xShowSet->GetSelectCharacter();
        // using the new UCS4 constructor
        OUString aOUStr( &cChar, 1 );
        m_aShowChar.SetText(aOUStr);

        setFavButtonState(aOUStr, m_aShowChar.GetFont().GetFamilyName());
        m_xOKBtn->set_sensitive(true);
    }

    return SfxDialogController::run();
}

void SvxCharacterMap::SetChar( sal_UCS4 c )
{
    m_xShowSet->SelectCharacter( c );
    setFavButtonState(OUString(&c, 1), aFont.GetFamilyName());
}

sal_UCS4 SvxCharacterMap::GetChar() const
{
    return m_aShowChar.GetText().iterateCodePoints(&o3tl::temporary(sal_Int32(0)));
}

void SvxCharacterMap::DisableFontSelection()
{
    m_xFontText->set_sensitive(false);
    m_xFontLB->set_sensitive(false);
}

IMPL_LINK_NOARG(SvxCharacterMap, UpdateFavHdl, void*, void)
{
    m_xShowSet->getFavCharacterList();
    m_xSearchSet->getFavCharacterList();
    // tdf#109214 - redraw highlight of the favorite characters
    m_xShowSet->Invalidate();
}

void SvxCharacterMap::init()
{
    aFont = m_xVirDev->GetFont();
    aFont.SetTransparent( true );
    aFont.SetFamily( FAMILY_DONTKNOW );
    aFont.SetPitch( PITCH_DONTKNOW );
    aFont.SetCharSet( RTL_TEXTENCODING_DONTKNOW );

    OUString aDefStr( aFont.GetFamilyName() );
    OUString aLastName;
    int nCount = m_xVirDev->GetFontFaceCollectionCount();
    std::vector<weld::ComboBoxEntry> aEntries;
    aEntries.reserve(nCount);
    for (int i = 0; i < nCount; ++i)
    {
        OUString aFontName( m_xVirDev->GetFontMetricFromCollection( i ).GetFamilyName() );
        if (aFontName != aLastName)
        {
            aLastName = aFontName;
            aEntries.emplace_back(aFontName, OUString::number(i));
        }
    }
    m_xFontLB->insert_vector(aEntries, true);
    // the font may not be in the list =>
    // try to find a font name token in list and select found font,
    // else select topmost entry
    bool bFound = (m_xFontLB->find_text(aDefStr) != -1);
    if (!bFound)
    {
        sal_Int32 nIndex = 0;
        do
        {
            OUString aToken = aDefStr.getToken(0, ';', nIndex);
            if (m_xFontLB->find_text(aToken) != -1)
            {
                aDefStr = aToken;
                bFound = true;
                break;
            }
        }
        while ( nIndex >= 0 );
    }

    if (bFound)
        m_xFontLB->set_active_text(aDefStr);
    else if (m_xFontLB->get_count() )
        m_xFontLB->set_active(0);
    FontSelectHdl(*m_xFontLB);
    if (m_xSubsetLB->get_count())
        m_xSubsetLB->set_active(0);

    m_xFontLB->connect_changed(LINK( this, SvxCharacterMap, FontSelectHdl));
    m_xSubsetLB->connect_changed(LINK( this, SvxCharacterMap, SubsetSelectHdl));
    m_xOKBtn->connect_clicked(LINK(this, SvxCharacterMap, InsertClickHdl));
    m_xOKBtn->show();

    m_xShowSet->SetDoubleClickHdl( LINK( this, SvxCharacterMap, CharDoubleClickHdl ) );
    m_xShowSet->SetReturnKeyPressHdl(LINK(this, SvxCharacterMap, ReturnKeypressOnCharHdl));
    m_xShowSet->SetSelectHdl( LINK( this, SvxCharacterMap, CharSelectHdl ) );
    m_xShowSet->SetHighlightHdl( LINK( this, SvxCharacterMap, CharHighlightHdl ) );
    m_xShowSet->SetPreSelectHdl( LINK( this, SvxCharacterMap, CharPreSelectHdl ) );
    m_xShowSet->SetFavClickHdl( LINK( this, SvxCharacterMap, FavClickHdl ) );

    m_xSearchSet->SetDoubleClickHdl( LINK( this, SvxCharacterMap, CharDoubleClickHdl ) );
    m_xSearchSet->SetReturnKeyPressHdl(LINK(this, SvxCharacterMap, ReturnKeypressOnCharHdl));
    m_xSearchSet->SetSelectHdl( LINK( this, SvxCharacterMap, CharSelectHdl ) );
    m_xSearchSet->SetHighlightHdl( LINK( this, SvxCharacterMap, SearchCharHighlightHdl ) );
    m_xSearchSet->SetPreSelectHdl( LINK( this, SvxCharacterMap, CharPreSelectHdl ) );
    m_xSearchSet->SetFavClickHdl( LINK( this, SvxCharacterMap, FavClickHdl ) );

    m_xDecimalCodeText->connect_changed( LINK( this, SvxCharacterMap, DecimalCodeChangeHdl ) );
    m_xHexCodeText->connect_changed( LINK( this, SvxCharacterMap, HexCodeChangeHdl ) );
    m_xFavouritesBtn->connect_clicked( LINK(this, SvxCharacterMap, FavSelectHdl));

    // tdf#117038 set the buttons width to its max possible width so it doesn't
    // make layout change when the label changes
    m_xFavouritesBtn->set_label(SvxResId(RID_SVXSTR_REMOVE_FAVORITES));
    auto nMaxWidth = m_xFavouritesBtn->get_preferred_size().Width();
    m_xFavouritesBtn->set_label(SvxResId(RID_SVXSTR_ADD_FAVORITES));
    nMaxWidth = std::max(nMaxWidth, m_xFavouritesBtn->get_preferred_size().Width());
    m_xFavouritesBtn->set_size_request(nMaxWidth, -1);

    if( SvxShowCharSet::getSelectedChar() == ' ')
    {
        m_xOKBtn->set_sensitive(false);
    }
    else
    {
        sal_UCS4 cChar = m_xShowSet->GetSelectCharacter();
        // using the new UCS4 constructor
        OUString aOUStr( &cChar, 1 );
        m_aShowChar.SetText(aOUStr);

        setFavButtonState(aOUStr, aDefStr);
        m_xOKBtn->set_sensitive(true);
    }

    m_aCharmapContents.init(m_xFrame.is(),
                            LINK(this, SvxCharacterMap, CharClickHdl),
                            LINK(this, SvxCharacterMap, UpdateFavHdl),
                            Link<void*, void>());

    setCharName(90);

    m_xSearchText->connect_focus_in(LINK( this, SvxCharacterMap, SearchFieldGetFocusHdl ));
    m_xSearchText->connect_changed(LINK(this, SvxCharacterMap, SearchUpdateHdl));
}

void SvxCharacterMap::setFavButtonState(std::u16string_view sTitle, std::u16string_view rFont)
{
    if(sTitle.empty() || rFont.empty())
    {
        m_xFavouritesBtn->set_sensitive(false);
        return;
    }
    else
        m_xFavouritesBtn->set_sensitive(true);

    if (m_aCharmapContents.isFavChar(sTitle, rFont))
    {
        m_xFavouritesBtn->set_label(SvxResId(RID_SVXSTR_REMOVE_FAVORITES));
    }
    else
    {
        if (m_aCharmapContents.FavCharListIsFull())
            m_xFavouritesBtn->set_sensitive(false);

        m_xFavouritesBtn->set_label(SvxResId(RID_SVXSTR_ADD_FAVORITES));
    }
}


void SvxCharacterMap::SetCharFont( const vcl::Font& rFont )
{
    // first get the underlying info in order to get font names
    // like "Times New Roman;Times" resolved
    vcl::Font aTmp(m_xVirDev->GetFontMetric(rFont));

    // tdf#56363 - search font family without the font feature after the colon
    OUString sFontFamilyName = aTmp.GetFamilyName();
    if (const sal_Int32 nIndex = sFontFamilyName.indexOf(":"); nIndex != -1)
        sFontFamilyName = sFontFamilyName.copy(0, nIndex);
    if (sFontFamilyName == "StarSymbol" && m_xFontLB->find_text(sFontFamilyName) == -1)
    {
        //if for some reason, like font in an old document, StarSymbol is requested and it's not available, then
        //try OpenSymbol instead
        aTmp.SetFamilyName(u"OpenSymbol"_ustr);
    }

    if (m_xFontLB->find_text(sFontFamilyName) == -1)
        return;

    m_xFontLB->set_active_text(sFontFamilyName);
    aFont = aTmp;
    FontSelectHdl(*m_xFontLB);
    if (m_xSubsetLB->get_count())
        m_xSubsetLB->set_active(0);
}

void SvxCharacterMap::fillAllSubsets(weld::ComboBox& rListBox)
{
    SubsetMap aAll(nullptr);
    std::vector<weld::ComboBoxEntry> aEntries;
    for (auto & subset : aAll.GetSubsetMap())
        aEntries.emplace_back(subset.GetName());
    rListBox.insert_vector(aEntries, true);
}

void SvxCharacterMap::insertCharToDoc(const OUString& sGlyph)
{
    if(sGlyph.isEmpty())
        return;

    if (m_xFrame.is()) {
        uno::Sequence<beans::PropertyValue> aArgs{
            comphelper::makePropertyValue(u"Symbols"_ustr, sGlyph),
            comphelper::makePropertyValue(u"FontName"_ustr, aFont.GetFamilyName())
        };
        comphelper::dispatchCommand(u".uno:InsertSymbol"_ustr, m_xFrame, aArgs);

        m_aCharmapContents.updateRecentCharacterList(sGlyph, aFont.GetFamilyName());

    } else {
        sal_UCS4 cChar = sGlyph.iterateCodePoints(&o3tl::temporary(sal_Int32(0)));
        const SfxItemPool* pPool = m_xOutputSet->GetPool();
        m_xOutputSet->Put( SfxStringItem( SID_CHARMAP, sGlyph ) );
        m_xOutputSet->Put( SvxFontItem( aFont.GetFamilyType(), aFont.GetFamilyName(),
            aFont.GetStyleName(), aFont.GetPitch(), aFont.GetCharSet(), pPool->GetWhichIDFromSlotID(SID_ATTR_CHAR_FONT) ) );
        m_xOutputSet->Put( SfxStringItem( SID_FONT_NAME, aFont.GetFamilyName() ) );
        m_xOutputSet->Put( SfxInt32Item( SID_ATTR_CHAR, cChar ) );
    }
}

IMPL_LINK_NOARG(SvxCharacterMap, FontSelectHdl, weld::ComboBox&, void)
{
    const sal_uInt32 nFont = m_xFontLB->get_active_id().toUInt32();
    aFont = m_xVirDev->GetFontMetricFromCollection(nFont);
    aFont.SetWeight( WEIGHT_DONTKNOW );
    aFont.SetItalic( ITALIC_NONE );
    aFont.SetWidthType( WIDTH_DONTKNOW );
    aFont.SetPitch( PITCH_DONTKNOW );
    aFont.SetFamily( FAMILY_DONTKNOW );

    // notify children using this font
    m_xShowSet->SetFont( aFont );
    m_xSearchSet->SetFont( aFont );
    m_aShowChar.SetFont( aFont );

    // setup unicode subset listbar with font specific subsets,
    // hide unicode subset listbar for symbol fonts
    // TODO: get info from the Font once it provides it
    pSubsetMap.reset();
    m_xSubsetLB->clear();

    bool bNeedSubset = (aFont.GetCharSet() != RTL_TEXTENCODING_SYMBOL);
    if (bNeedSubset)
    {
        FontCharMapRef xFontCharMap = m_xShowSet->GetFontCharMap();
        pSubsetMap.reset(new SubsetMap( xFontCharMap ));

        // update subset listbox for new font's unicode subsets
        for (auto const& subset : pSubsetMap->GetSubsetMap())
        {
            m_xSubsetLB->append(weld::toId(&subset), subset.GetName());
            // NOTE: subset must live at least as long as the selected font
        }

        if (m_xSubsetLB->get_count() <= 1)
            bNeedSubset = false;
    }

    m_xSubsetText->set_sensitive(bNeedSubset);
    m_xSubsetLB->set_sensitive(bNeedSubset);

    if (isSearchMode)
    {
        // tdf#137294 do this after modifying m_xSubsetLB sensitivity to
        // restore insensitive for the search case
        SearchUpdateHdl(*m_xSearchText);
        SearchCharHighlightHdl(m_xSearchSet.get());
    }

    // tdf#118304 reselect current glyph to see if it's still there in new font
    selectCharByCode(Radix::hexadecimal);
}

void SvxCharacterMap::toggleSearchView(bool state)
{
    isSearchMode = state;
    m_xHexCodeText->set_editable(!state);
    m_xDecimalCodeText->set_editable(!state);
    m_xSubsetLB->set_sensitive(!state);

    if(state)
    {
        m_xSearchSet->Show();
        m_xShowSet->Hide();
    }
    else
    {
        m_xSearchSet->Hide();
        m_xShowSet->Show();
    }
}

void SvxCharacterMap::setCharName(sal_UCS4 nDecimalValue)
{
    /* get the character name */
    UErrorCode errorCode = U_ZERO_ERROR;
    // icu has a private uprv_getMaxCharNameLength function which returns the max possible
    // length of this property. Unicode 3.2 max char name length was 83
    char buffer[100];
    u_charName(nDecimalValue, U_UNICODE_CHAR_NAME, buffer, sizeof(buffer), &errorCode);
    if (U_SUCCESS(errorCode))
        m_xCharName->set_label(OUString::createFromAscii(buffer));
}

IMPL_LINK_NOARG(SvxCharacterMap, SubsetSelectHdl, weld::ComboBox&, void)
{
    const sal_Int32 nPos = m_xSubsetLB->get_active();
    const Subset* pSubset = weld::fromId<const Subset*>(m_xSubsetLB->get_active_id());

    if( pSubset && !isSearchMode)
    {
        sal_UCS4 cFirst = pSubset->GetRangeMin();
        m_xShowSet->SelectCharacter( cFirst );

        setFavButtonState(OUString(&cFirst, 1), aFont.GetFamilyName());
        m_xSubsetLB->set_active(nPos);
    }
    else if( pSubset && isSearchMode)
    {
        m_xSearchSet->SelectCharacter( pSubset );

        const Subset* curSubset = nullptr;
        if( pSubsetMap )
            curSubset = pSubsetMap->GetSubsetByUnicode( m_xSearchSet->GetSelectCharacter() );
        if( curSubset )
            m_xSubsetLB->set_active_text(curSubset->GetName());
        else
            m_xSubsetLB->set_active(-1);

        sal_UCS4 sChar = m_xSearchSet->GetSelectCharacter();
        setFavButtonState(OUString(&sChar, 1), aFont.GetFamilyName());
    }
}

IMPL_LINK_NOARG(SvxCharacterMap, SearchFieldGetFocusHdl, weld::Widget&, void)
{
    m_xOKBtn->set_sensitive(false);
}

IMPL_LINK_NOARG(SvxCharacterMap, SearchUpdateHdl, weld::Entry&, void)
{
    if (!m_xSearchText->get_text().isEmpty())
    {
        m_xSearchSet->ClearPreviousData();
        OUString aKeyword = m_xSearchText->get_text();

        toggleSearchView(true);

        FontCharMapRef xFontCharMap = m_xSearchSet->GetFontCharMap();

        sal_UCS4 sChar = xFontCharMap->GetFirstChar();
        while(sChar != xFontCharMap->GetLastChar())
        {
            UErrorCode errorCode = U_ZERO_ERROR;
            char buffer[100];
            u_charName(sChar, U_UNICODE_CHAR_NAME, buffer, sizeof(buffer), &errorCode);
            if (U_SUCCESS(errorCode))
            {
                OUString sName = OUString::createFromAscii(buffer);
                if(!sName.isEmpty() && sName.toAsciiLowerCase().indexOf(aKeyword.toAsciiLowerCase()) >= 0)
                    m_xSearchSet->AppendCharToList(sChar);
            }
            sChar = xFontCharMap->GetNextChar(sChar);
        }
        //for last char
        UErrorCode errorCode = U_ZERO_ERROR;
        char buffer[100];
        u_charName(sChar, U_UNICODE_CHAR_NAME, buffer, sizeof(buffer), &errorCode);
        if (U_SUCCESS(errorCode))
        {
            OUString sName = OUString::createFromAscii(buffer);
            if(!sName.isEmpty() && sName.toAsciiLowerCase().indexOf(aKeyword.toAsciiLowerCase()) >= 0)
                m_xSearchSet->AppendCharToList(sChar);
        }

        m_xSearchSet->UpdateScrollRange();
    }
    else
    {
        toggleSearchView(false);
    }
}


IMPL_LINK(SvxCharacterMap, CharClickHdl, SvxCharView*, rView, void)
{
    rView->GrabFocus();

    SetCharFont(rView->GetFont());
    m_aShowChar.SetText( rView->GetText() );
    m_aShowChar.SetFont(rView->GetFont());
    m_aShowChar.Invalidate();

    setFavButtonState(rView->GetText(), rView->GetFont().GetFamilyName());//check state

    // Get the hexadecimal code
    OUString charValue = rView->GetText();
    sal_UCS4 cChar = charValue.iterateCodePoints(&o3tl::temporary(sal_Int32(1)), -1);
    OUString aHexText = OUString::number(cChar, 16).toAsciiUpperCase();

    // Get the decimal code
    OUString aDecimalText = OUString::number(cChar);

    m_xHexCodeText->set_text(aHexText);
    m_xDecimalCodeText->set_text(aDecimalText);
    setCharName(cChar);

    rView->Invalidate();
    m_xOKBtn->set_sensitive(true);
}

void SvxCharacterMap::insertSelectedCharacter(const SvxShowCharSet* pCharSet)
{
    assert(pCharSet);
    sal_UCS4 cChar = pCharSet->GetSelectCharacter();
    // using the new UCS4 constructor
    OUString aOUStr( &cChar, 1 );
    setFavButtonState(aOUStr, aFont.GetFamilyName());
    insertCharToDoc(aOUStr);
}

IMPL_LINK(SvxCharacterMap, CharDoubleClickHdl, SvxShowCharSet*, pCharSet, void)
{
    insertSelectedCharacter(pCharSet);
}

IMPL_LINK_NOARG(SvxCharacterMap, CharSelectHdl, SvxShowCharSet*, void)
{
    m_xOKBtn->set_sensitive(true);
}

IMPL_LINK(SvxCharacterMap, ReturnKeypressOnCharHdl, SvxShowCharSet*, pCharSet, void)
{
    insertSelectedCharacter(pCharSet);
    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(SvxCharacterMap, InsertClickHdl, weld::Button&, void)
{
    OUString sChar = m_aShowChar.GetText();
    insertCharToDoc(sChar);
    // Need to update recent character list, when OK button does not insert
    if(!m_xFrame.is())
        m_aCharmapContents.updateRecentCharacterList(sChar, aFont.GetFamilyName());
    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(SvxCharacterMap, FavSelectHdl, weld::Button&, void)
{
    if (m_xFavouritesBtn->get_label().match(SvxResId(RID_SVXSTR_ADD_FAVORITES)))
    {
        m_aCharmapContents.updateFavCharacterList(m_aShowChar.GetText(), m_aShowChar.GetFont().GetFamilyName());
        setFavButtonState(m_aShowChar.GetText(), m_aShowChar.GetFont().GetFamilyName());
    }
    else
    {
        m_aCharmapContents.deleteFavCharacterFromList(m_aShowChar.GetText(), m_aShowChar.GetFont().GetFamilyName());
        m_xFavouritesBtn->set_label(SvxResId(RID_SVXSTR_ADD_FAVORITES));
        m_xFavouritesBtn->set_sensitive(false);
    }

    m_aCharmapContents.updateFavCharControl();
}

IMPL_LINK_NOARG(SvxCharacterMap, FavClickHdl, SvxShowCharSet*, void)
{
    m_aCharmapContents.getFavCharacterList();
    m_aCharmapContents.updateFavCharControl();
}

IMPL_LINK_NOARG(SvxCharacterMap, CharHighlightHdl, SvxShowCharSet*, void)
{
    OUString aText;
    sal_UCS4 cChar = m_xShowSet->GetSelectCharacter();
    bool bSelect = (cChar > 0);

    // show char sample
    if ( bSelect )
    {
        // using the new UCS4 constructor
        aText = OUString( &cChar, 1 );
        // Get the hexadecimal code
        OUString aHexText = OUString::number(cChar, 16).toAsciiUpperCase();
        // Get the decimal code
        OUString aDecimalText = OUString::number(cChar);
        setCharName(cChar);

        // Update the hex and decimal codes only if necessary
        if (!m_xHexCodeText->get_text().equalsIgnoreAsciiCase(aHexText))
            m_xHexCodeText->set_text(aHexText);
        if (m_xDecimalCodeText->get_text() != aDecimalText)
            m_xDecimalCodeText->set_text( aDecimalText );

        const Subset* pSubset = nullptr;
        if( pSubsetMap )
            pSubset = pSubsetMap->GetSubsetByUnicode( cChar );
        if( pSubset )
            m_xSubsetLB->set_active_text(pSubset->GetName());
        else
            m_xSubsetLB->set_active(-1);
    }

    m_aShowChar.SetText( aText );
    m_aShowChar.SetFont( aFont );
    m_aShowChar.Invalidate();

    setFavButtonState(aText, aFont.GetFamilyName());
}

IMPL_LINK_NOARG(SvxCharacterMap, SearchCharHighlightHdl, SvxShowCharSet*, void)
{
    OUString aText;
    sal_UCS4 cChar = m_xSearchSet->GetSelectCharacter();
    bool bSelect = (cChar > 0);

    // show char sample
    if ( bSelect )
    {
        aText = OUString( &cChar, 1 );
        // Get the hexadecimal code
        OUString aHexText = OUString::number(cChar, 16).toAsciiUpperCase();
        // Get the decimal code
        OUString aDecimalText = OUString::number(cChar);
        setCharName(cChar);

        // Update the hex and decimal codes only if necessary
        if (!m_xHexCodeText->get_text().equalsIgnoreAsciiCase(aHexText))
            m_xHexCodeText->set_text(aHexText);
        if (m_xDecimalCodeText->get_text() != aDecimalText)
            m_xDecimalCodeText->set_text( aDecimalText );

        const Subset* pSubset = nullptr;
        if( pSubsetMap )
            pSubset = pSubsetMap->GetSubsetByUnicode( cChar );
        if( pSubset )
            m_xSubsetLB->set_active_text(pSubset->GetName());
        else
            m_xSubsetLB->set_active(-1);
    }

    if(m_xSearchSet->HasFocus())
    {
        m_aShowChar.SetText( aText );
        m_aShowChar.SetFont( aFont );
        m_aShowChar.Invalidate();

        setFavButtonState(aText, aFont.GetFamilyName());
    }
}

void SvxCharacterMap::selectCharByCode(Radix radix)
{
    OUString aCodeString;
    switch(radix)
    {
        case Radix::decimal:
            aCodeString = m_xDecimalCodeText->get_text();
            break;
        case Radix::hexadecimal:
            aCodeString = m_xHexCodeText->get_text();
            break;
    }
    // Convert the code back to a character using the appropriate radix
    sal_UCS4 cChar = aCodeString.toUInt32(static_cast<sal_Int16> (radix));
    // Use FontCharMap::HasChar(sal_UCS4 cChar) to see if the desired character is in the font
    FontCharMapRef xFontCharMap = m_xShowSet->GetFontCharMap();
    if (xFontCharMap->HasChar(cChar))
        // Select the corresponding character
        SetChar(cChar);
    else {
        m_xCharName->set_label(SvxResId(RID_SVXSTR_MISSING_CHAR));
        m_aShowChar.SetText(u" "_ustr);
        switch(radix)
        {
            case Radix::decimal:
                m_xHexCodeText->set_text(OUString::number(cChar, 16));
                break;
            case Radix::hexadecimal:
                m_xDecimalCodeText->set_text(OUString::number(cChar));
                break;
        }
    }
}

IMPL_LINK_NOARG(SvxCharacterMap, DecimalCodeChangeHdl, weld::Entry&, void)
{
    selectCharByCode(Radix::decimal);
}

IMPL_LINK_NOARG(SvxCharacterMap, HexCodeChangeHdl, weld::Entry&, void)
{
    selectCharByCode(Radix::hexadecimal);
}

IMPL_LINK(SvxCharacterMap, CharPreSelectHdl, SvxShowCharSet*, pCharSet, void)
{
    assert(pCharSet);
    // adjust subset selection
    if( pSubsetMap )
    {
        sal_UCS4 cChar = pCharSet->GetSelectCharacter();

        setFavButtonState(OUString(&cChar, 1), aFont.GetFamilyName());
        const Subset* pSubset = pSubsetMap->GetSubsetByUnicode( cChar );
        if( pSubset )
            m_xSubsetLB->set_active_text(pSubset->GetName());
    }

    m_xOKBtn->set_sensitive(true);
}

// class SvxShowText =====================================================
SvxShowText::SvxShowText(const VclPtr<VirtualDevice>& rVirDev)
    : m_xVirDev(rVirDev)
    , mnY(0)
    , mbCenter(false)
{
}

void SvxShowText::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    vcl::Font aFont = m_xVirDev->GetFont();
    Size aFontSize(aFont.GetFontSize().Width() * 5, aFont.GetFontSize().Height() * 5);
    aFont.SetFontSize(aFontSize);
    m_xVirDev->Push(PUSH_ALLFONT);
    m_xVirDev->SetFont(aFont);
    pDrawingArea->set_size_request(m_xVirDev->approximate_digit_width() + 2 * 12,
                                   m_xVirDev->LogicToPixel(aFontSize).Height() * 2);
    m_xVirDev->Pop();
}

void SvxShowText::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    rRenderContext.SetFont(m_aFont);

    Color aTextCol = rRenderContext.GetTextColor();
    Color aFillCol = rRenderContext.GetFillColor();
    Color aLineCol = rRenderContext.GetLineColor();

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    const Color aWindowTextColor(rStyleSettings.GetDialogTextColor());
    const Color aWindowColor(rStyleSettings.GetWindowColor());
    const Color aShadowColor(rStyleSettings.GetShadowColor());
    rRenderContext.SetTextColor(aWindowTextColor);
    rRenderContext.SetFillColor(aWindowColor);

    const OUString aText = GetText();

    Size aSize(GetOutputSizePixel());
    tools::Long nAvailWidth = aSize.Width();
    tools::Long nWinHeight = aSize.Height();

    bool bGotBoundary = true;
    bool bShrankFont = false;
    vcl::Font aOrigFont(rRenderContext.GetFont());
    Size aFontSize(aOrigFont.GetFontSize());
    ::tools::Rectangle aBoundRect;

    for (tools::Long nFontHeight = aFontSize.Height(); nFontHeight > 0; nFontHeight -= 5)
    {
        if (!rRenderContext.GetTextBoundRect( aBoundRect, aText ) || aBoundRect.IsEmpty())
        {
            bGotBoundary = false;
            break;
        }
        if (!mbCenter)
            break;
        //only shrink in the single glyph large view mode
        tools::Long nTextWidth = aBoundRect.GetWidth();
        if (nAvailWidth > nTextWidth)
            break;
        vcl::Font aFont(aOrigFont);
        aFontSize.setHeight( nFontHeight );
        aFont.SetFontSize(aFontSize);
        rRenderContext.SetFont(aFont);
        mnY = (nWinHeight - rRenderContext.GetTextHeight()) / 2;
        bShrankFont = true;
    }

    Point aPoint(2, mnY);
    // adjust position using ink boundary if possible
    if (!bGotBoundary)
        aPoint.setX( (aSize.Width() - rRenderContext.GetTextWidth(aText)) / 2 );
    else
    {
        // adjust position before it gets out of bounds
        aBoundRect += aPoint;

        // shift back vertically if needed
        int nYLDelta = aBoundRect.Top();
        int nYHDelta = aSize.Height() - aBoundRect.Bottom();
        if( nYLDelta <= 0 )
            aPoint.AdjustY( -(nYLDelta - 1) );
        else if( nYHDelta <= 0 )
            aPoint.AdjustY(nYHDelta - 1 );

        if (mbCenter)
        {
            // move glyph to middle of cell
            aPoint.setX( -aBoundRect.Left() + (aSize.Width() - aBoundRect.GetWidth()) / 2 );
        }
        else
        {
            // shift back horizontally if needed
            int nXLDelta = aBoundRect.Left();
            int nXHDelta = aSize.Width() - aBoundRect.Right();
            if( nXLDelta <= 0 )
                aPoint.AdjustX( -(nXLDelta - 1) );
            else if( nXHDelta <= 0 )
                aPoint.AdjustX(nXHDelta - 1 );
        }
    }

    rRenderContext.SetLineColor(aShadowColor);
    rRenderContext.DrawRect(tools::Rectangle(Point(0, 0), aSize));
    rRenderContext.DrawText(aPoint, aText);
    rRenderContext.SetTextColor(aTextCol);
    rRenderContext.SetFillColor(aFillCol);
    rRenderContext.SetLineColor(aLineCol);
    if (bShrankFont)
        rRenderContext.SetFont(aOrigFont);
}

void SvxShowText::SetFont( const vcl::Font& rFont )
{
    tools::Long nWinHeight = GetOutputSizePixel().Height();

    m_aFont = rFont;
    m_aFont.SetWeight(WEIGHT_NORMAL);
    m_aFont.SetAlignment(ALIGN_TOP);
    m_aFont.SetFontSize(m_xVirDev->PixelToLogic(Size(0, nWinHeight / 2)));
    m_aFont.SetTransparent(true);

    m_xVirDev->Push(PUSH_ALLFONT);
    m_xVirDev->SetFont(m_aFont);
    mnY = (nWinHeight - m_xVirDev->GetTextHeight()) / 2;
    m_xVirDev->Pop();

    Invalidate();
}

void SvxShowText::Resize()
{
    SetFont(GetFont()); //force recalculation of size
}

void SvxShowText::SetText(const OUString& rText)
{
    m_sText = rText;
    Invalidate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
