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

#include <stdio.h>

#include <vcl/svapp.hxx>
#include <svtools/colorcfg.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/itempool.hxx>

#include <rtl/textenc.h>
#include <svx/ucsubset.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/fontcharmap.hxx>
#include <svl/stritem.hxx>
#include <o3tl/make_unique.hxx>
#include <officecfg/Office/Common.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/dispatchcommand.hxx>

#include <dialmgr.hxx>
#include <cuicharmap.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/app.hxx>
#include <svx/svxids.hrc>
#include <editeng/editids.hrc>
#include <editeng/fontitem.hxx>
#include <strings.hrc>
#include <macroass.hxx>
#include <unicode/uchar.h>
#include <unicode/utypes.h>

using namespace css;

SvxCharacterMap::SvxCharacterMap(weld::Window* pParent, const SfxItemSet* pSet, bool bInsert)
    : GenericDialogController(pParent, "cui/ui/specialcharacters.ui", "SpecialCharactersDialog")
    , m_xVirDev(VclPtr<VirtualDevice>::Create())
    , pSubsetMap( nullptr )
    , isSearchMode(true)
    , m_bHasInsert(bInsert)
    , mxContext(comphelper::getProcessComponentContext())
    , m_aRecentCharView{SvxCharView(m_xVirDev),
                        SvxCharView(m_xVirDev),
                        SvxCharView(m_xVirDev),
                        SvxCharView(m_xVirDev),
                        SvxCharView(m_xVirDev),
                        SvxCharView(m_xVirDev),
                        SvxCharView(m_xVirDev),
                        SvxCharView(m_xVirDev),
                        SvxCharView(m_xVirDev),
                        SvxCharView(m_xVirDev),
                        SvxCharView(m_xVirDev),
                        SvxCharView(m_xVirDev),
                        SvxCharView(m_xVirDev),
                        SvxCharView(m_xVirDev),
                        SvxCharView(m_xVirDev),
                        SvxCharView(m_xVirDev)}
    , m_aFavCharView{SvxCharView(m_xVirDev),
                     SvxCharView(m_xVirDev),
                     SvxCharView(m_xVirDev),
                     SvxCharView(m_xVirDev),
                     SvxCharView(m_xVirDev),
                     SvxCharView(m_xVirDev),
                     SvxCharView(m_xVirDev),
                     SvxCharView(m_xVirDev),
                     SvxCharView(m_xVirDev),
                     SvxCharView(m_xVirDev),
                     SvxCharView(m_xVirDev),
                     SvxCharView(m_xVirDev),
                     SvxCharView(m_xVirDev),
                     SvxCharView(m_xVirDev),
                     SvxCharView(m_xVirDev),
                     SvxCharView(m_xVirDev)}
    , m_aShowChar(m_xVirDev)
    , m_xOKBtn(bInsert ? m_xBuilder->weld_button("insert") : m_xBuilder->weld_button("ok"))
    , m_xFontText(m_xBuilder->weld_label("fontft"))
    , m_xFontLB(m_xBuilder->weld_combo_box_text("fontlb"))
    , m_xSubsetText(m_xBuilder->weld_label("subsetft"))
    , m_xSubsetLB(m_xBuilder->weld_combo_box_text("subsetlb"))
    , m_xSearchText(m_xBuilder->weld_entry("search"))
    , m_xHexCodeText(m_xBuilder->weld_entry("hexvalue"))
    , m_xDecimalCodeText(m_xBuilder->weld_entry("decimalvalue"))
    , m_xFavouritesBtn(m_xBuilder->weld_button("favbtn"))
    , m_xCharName(m_xBuilder->weld_label("charname"))
    , m_xRecentGrid(m_xBuilder->weld_widget("viewgrid"))
    , m_xFavGrid(m_xBuilder->weld_widget("favgrid"))
    , m_xShowChar(new weld::CustomWeld(*m_xBuilder, "showchar", m_aShowChar))
    , m_xRecentCharView{o3tl::make_unique<weld::CustomWeld>(*m_xBuilder, "viewchar1", m_aRecentCharView[0]),
                        o3tl::make_unique<weld::CustomWeld>(*m_xBuilder, "viewchar2", m_aRecentCharView[1]),
                        o3tl::make_unique<weld::CustomWeld>(*m_xBuilder, "viewchar3", m_aRecentCharView[2]),
                        o3tl::make_unique<weld::CustomWeld>(*m_xBuilder, "viewchar4", m_aRecentCharView[3]),
                        o3tl::make_unique<weld::CustomWeld>(*m_xBuilder, "viewchar5", m_aRecentCharView[4]),
                        o3tl::make_unique<weld::CustomWeld>(*m_xBuilder, "viewchar6", m_aRecentCharView[5]),
                        o3tl::make_unique<weld::CustomWeld>(*m_xBuilder, "viewchar7", m_aRecentCharView[6]),
                        o3tl::make_unique<weld::CustomWeld>(*m_xBuilder, "viewchar8", m_aRecentCharView[7]),
                        o3tl::make_unique<weld::CustomWeld>(*m_xBuilder, "viewchar9", m_aRecentCharView[8]),
                        o3tl::make_unique<weld::CustomWeld>(*m_xBuilder, "viewchar10", m_aRecentCharView[9]),
                        o3tl::make_unique<weld::CustomWeld>(*m_xBuilder, "viewchar11", m_aRecentCharView[10]),
                        o3tl::make_unique<weld::CustomWeld>(*m_xBuilder, "viewchar12", m_aRecentCharView[11]),
                        o3tl::make_unique<weld::CustomWeld>(*m_xBuilder, "viewchar13", m_aRecentCharView[12]),
                        o3tl::make_unique<weld::CustomWeld>(*m_xBuilder, "viewchar14", m_aRecentCharView[13]),
                        o3tl::make_unique<weld::CustomWeld>(*m_xBuilder, "viewchar15", m_aRecentCharView[14]),
                        o3tl::make_unique<weld::CustomWeld>(*m_xBuilder, "viewchar16", m_aRecentCharView[15])}
    , m_xFavCharView{o3tl::make_unique<weld::CustomWeld>(*m_xBuilder, "favchar1", m_aFavCharView[0]),
                     o3tl::make_unique<weld::CustomWeld>(*m_xBuilder, "favchar2", m_aFavCharView[1]),
                     o3tl::make_unique<weld::CustomWeld>(*m_xBuilder, "favchar3", m_aFavCharView[2]),
                     o3tl::make_unique<weld::CustomWeld>(*m_xBuilder, "favchar4", m_aFavCharView[3]),
                     o3tl::make_unique<weld::CustomWeld>(*m_xBuilder, "favchar5", m_aFavCharView[4]),
                     o3tl::make_unique<weld::CustomWeld>(*m_xBuilder, "favchar6", m_aFavCharView[5]),
                     o3tl::make_unique<weld::CustomWeld>(*m_xBuilder, "favchar7", m_aFavCharView[6]),
                     o3tl::make_unique<weld::CustomWeld>(*m_xBuilder, "favchar8", m_aFavCharView[7]),
                     o3tl::make_unique<weld::CustomWeld>(*m_xBuilder, "favchar9", m_aFavCharView[8]),
                     o3tl::make_unique<weld::CustomWeld>(*m_xBuilder, "favchar10", m_aFavCharView[9]),
                     o3tl::make_unique<weld::CustomWeld>(*m_xBuilder, "favchar11", m_aFavCharView[10]),
                     o3tl::make_unique<weld::CustomWeld>(*m_xBuilder, "favchar12", m_aFavCharView[11]),
                     o3tl::make_unique<weld::CustomWeld>(*m_xBuilder, "favchar13", m_aFavCharView[12]),
                     o3tl::make_unique<weld::CustomWeld>(*m_xBuilder, "favchar14", m_aFavCharView[13]),
                     o3tl::make_unique<weld::CustomWeld>(*m_xBuilder, "favchar15", m_aFavCharView[14]),
                     o3tl::make_unique<weld::CustomWeld>(*m_xBuilder, "favchar16", m_aFavCharView[15])}
    , m_xShowSet(new SvxShowCharSet(m_xBuilder->weld_scrolled_window("showscroll"), m_xVirDev))
    , m_xShowSetArea(new weld::CustomWeld(*m_xBuilder, "showcharset", *m_xShowSet))
    , m_xSearchSet(new SvxSearchCharSet(m_xBuilder->weld_scrolled_window("searchscroll"), m_xVirDev))
    , m_xSearchSetArea(new weld::CustomWeld(*m_xBuilder, "searchcharset", *m_xSearchSet))
{
    m_aShowChar.SetCentered(true);
    m_xFontLB->make_sorted();
    //lock the size request of this widget to the width of all possible entries
    fillAllSubsets(*m_xSubsetLB);
    m_xSubsetLB->set_size_request(m_xSubsetLB->get_preferred_size().Width(), -1);
    m_xCharName->set_size_request(m_aShowChar.get_preferred_size().Width(), m_xCharName->get_text_height() * 4);
    //lock the size request of this widget to the width of the original .ui string
    m_xHexCodeText->set_size_request(m_xHexCodeText->get_preferred_size().Width(), -1);
    //so things don't jump around if all the children are hidden
    m_xRecentGrid->set_size_request(-1, m_aRecentCharView[0].get_preferred_size().Height());
    m_xFavGrid->set_size_request(-1, m_aFavCharView[0].get_preferred_size().Height());

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

short SvxCharacterMap::execute()
{
    if( SvxShowCharSet::getSelectedChar() == ' ')
    {
        m_xOKBtn->set_sensitive(false);
        setFavButtonState(OUString(), OUString());
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

    return run();
}

void SvxCharacterMap::SetChar( sal_UCS4 c )
{
    m_xShowSet->SelectCharacter( c );
    setFavButtonState(OUString(&c, 1), aFont.GetFamilyName());
}

sal_UCS4 SvxCharacterMap::GetChar() const
{
   return m_aShowChar.GetText().toChar();
}

void SvxCharacterMap::DisableFontSelection()
{
    m_xFontText->set_sensitive(false);
    m_xFontLB->set_sensitive(false);
}


void SvxCharacterMap::getRecentCharacterList()
{
    //retrieve recent character list
    css::uno::Sequence< OUString > rRecentCharList( officecfg::Office::Common::RecentCharacters::RecentCharacterList::get() );
    for (int i = 0; i < rRecentCharList.getLength(); ++i)
    {
        maRecentCharList.push_back(rRecentCharList[i]);
    }

    //retrieve recent character font list
    css::uno::Sequence< OUString > rRecentCharFontList( officecfg::Office::Common::RecentCharacters::RecentCharacterFontList::get() );
    for (int i = 0; i < rRecentCharFontList.getLength(); ++i)
    {
        maRecentCharFontList.push_back(rRecentCharFontList[i]);
    }
}


void SvxCharacterMap::getFavCharacterList()
{
    maFavCharList.clear();
    maFavCharFontList.clear();
    //retrieve recent character list
    css::uno::Sequence< OUString > rFavCharList( officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterList::get() );
    for (int i = 0; i < rFavCharList.getLength(); ++i)
    {
        maFavCharList.push_back(rFavCharList[i]);
    }

    //retrieve recent character font list
    css::uno::Sequence< OUString > rFavCharFontList( officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterFontList::get() );
    for (int i = 0; i < rFavCharFontList.getLength(); ++i)
    {
        maFavCharFontList.push_back(rFavCharFontList[i]);
    }
}


void SvxCharacterMap::updateRecentCharControl()
{
    int i = 0;
    for ( std::deque< OUString >::iterator it = maRecentCharList.begin(), it2 = maRecentCharFontList.begin();
        it != maRecentCharList.end() || it2 != maRecentCharFontList.end();
        ++it, ++it2, i++)
    {
        m_aRecentCharView[i].SetText(*it);
        vcl::Font rFont = m_aRecentCharView[i].GetFont();
        rFont.SetFamilyName( *it2 );
        m_aRecentCharView[i].SetFont(rFont);
        m_aRecentCharView[i].Show();
    }

    for(; i < 16 ; i++)
    {
        m_aRecentCharView[i].SetText(OUString());
        m_aRecentCharView[i].Hide();
    }
}

void SvxCharacterMap::updateRecentCharacterList(const OUString& sTitle, const OUString& rFont)
{
    auto itChar = std::find_if(maRecentCharList.begin(),
         maRecentCharList.end(),
         [sTitle] (const OUString & a) { return a == sTitle; });

    auto itChar2 = std::find_if(maRecentCharFontList.begin(),
         maRecentCharFontList.end(),
         [rFont] (const OUString & a) { return a == rFont; });

    // if recent char to be added is already in list, remove it
    if( itChar != maRecentCharList.end() &&  itChar2 != maRecentCharFontList.end() )
    {
        maRecentCharList.erase( itChar );
        maRecentCharFontList.erase( itChar2);
    }

    if (maRecentCharList.size() == 16)
    {
        maRecentCharList.pop_back();
        maRecentCharFontList.pop_back();
    }

    maRecentCharList.push_front(sTitle);
    maRecentCharFontList.push_front(rFont);

    css::uno::Sequence< OUString > aRecentCharList(maRecentCharList.size());
    css::uno::Sequence< OUString > aRecentCharFontList(maRecentCharFontList.size());

    for (size_t i = 0; i < maRecentCharList.size(); ++i)
    {
        aRecentCharList[i] = maRecentCharList[i];
        aRecentCharFontList[i] = maRecentCharFontList[i];
    }

    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create(mxContext));
    officecfg::Office::Common::RecentCharacters::RecentCharacterList::set(aRecentCharList, batch);
    officecfg::Office::Common::RecentCharacters::RecentCharacterFontList::set(aRecentCharFontList, batch);
    batch->commit();

    updateRecentCharControl();
}


void SvxCharacterMap::updateFavCharacterList(const OUString& sTitle, const OUString& rFont)
{
    auto itChar = std::find_if(maFavCharList.begin(),
         maFavCharList.end(),
         [sTitle] (const OUString & a) { return a == sTitle; });

    auto itChar2 = std::find_if(maFavCharFontList.begin(),
         maFavCharFontList.end(),
         [rFont] (const OUString & a) { return a == rFont; });

    // if Fav char to be added is already in list, remove it
    if( itChar != maFavCharList.end() &&  itChar2 != maFavCharFontList.end() )
    {
        maFavCharList.erase( itChar );
        maFavCharFontList.erase( itChar2);
    }

    if (maFavCharList.size() == 16)
    {
        maFavCharList.pop_back();
        maFavCharFontList.pop_back();
    }

    maFavCharList.push_back(sTitle);
    maFavCharFontList.push_back(rFont);

    css::uno::Sequence< OUString > aFavCharList(maFavCharList.size());
    css::uno::Sequence< OUString > aFavCharFontList(maFavCharFontList.size());

    for (size_t i = 0; i < maFavCharList.size(); ++i)
    {
        aFavCharList[i] = maFavCharList[i];
        aFavCharFontList[i] = maFavCharFontList[i];
    }

    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create(mxContext));
    officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterList::set(aFavCharList, batch);
    officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterFontList::set(aFavCharFontList, batch);
    batch->commit();
}


void SvxCharacterMap::updateFavCharControl()
{
    int i = 0;
    for ( std::deque< OUString >::iterator it = maFavCharList.begin(), it2 = maFavCharFontList.begin();
        it != maFavCharList.end() || it2 != maFavCharFontList.end();
        ++it, ++it2, i++)
    {
        m_aFavCharView[i].SetText(*it);
        vcl::Font rFont = m_aFavCharView[i].GetFont();
        rFont.SetFamilyName( *it2 );
        m_aFavCharView[i].SetFont(rFont);
        m_aFavCharView[i].Show();
    }

    for(; i < 16 ; i++)
    {
        m_aFavCharView[i].SetText(OUString());
        m_aFavCharView[i].Hide();
    }
    m_xShowSet->getFavCharacterList();
    m_xSearchSet->getFavCharacterList();
}

void SvxCharacterMap::deleteFavCharacterFromList(const OUString& sTitle, const OUString& rFont)
{
    auto itChar = std::find_if(maFavCharList.begin(),
         maFavCharList.end(),
         [sTitle] (const OUString & a) { return a == sTitle; });

    auto itChar2 = std::find_if(maFavCharFontList.begin(),
         maFavCharFontList.end(),
         [rFont] (const OUString & a) { return a == rFont; });

    // if Fav char to be added is already in list, remove it
    if( itChar != maFavCharList.end() &&  itChar2 != maFavCharFontList.end() )
    {
        maFavCharList.erase( itChar );
        maFavCharFontList.erase( itChar2);
    }

    css::uno::Sequence< OUString > aFavCharList(maFavCharList.size());
    css::uno::Sequence< OUString > aFavCharFontList(maFavCharFontList.size());

    for (size_t i = 0; i < maFavCharList.size(); ++i)
    {
        aFavCharList[i] = maFavCharList[i];
        aFavCharFontList[i] = maFavCharFontList[i];
    }

    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create(mxContext));
    officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterList::set(aFavCharList, batch);
    officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterFontList::set(aFavCharFontList, batch);
    batch->commit();
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
    int nCount = m_xVirDev->GetDevFontCount();
    m_xFontLB->freeze();
    for (int i = 0; i < nCount; ++i)
    {
        OUString aFontName( m_xVirDev->GetDevFont( i ).GetFamilyName() );
        if (aFontName != aLastName)
        {
            aLastName = aFontName;
            m_xFontLB->append(OUString::number(i), aFontName);
        }
    }
    m_xFontLB->thaw();
    // the font may not be in the list =>
    // try to find a font name token in list and select found font,
    // else select topmost entry
    bool bFound = (m_xFontLB->find_text(aDefStr) == -1);
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

    m_xFontLB->connect_changed(LINK( this, SvxCharacterMap, FontSelectHdl));
    m_xSubsetLB->connect_changed(LINK( this, SvxCharacterMap, SubsetSelectHdl));
    m_xOKBtn->connect_clicked(LINK(this, SvxCharacterMap, InsertClickHdl));
    m_xOKBtn->show();

    m_xShowSet->SetDoubleClickHdl( LINK( this, SvxCharacterMap, CharDoubleClickHdl ) );
    m_xShowSet->SetSelectHdl( LINK( this, SvxCharacterMap, CharSelectHdl ) );
    m_xShowSet->SetHighlightHdl( LINK( this, SvxCharacterMap, CharHighlightHdl ) );
    m_xShowSet->SetPreSelectHdl( LINK( this, SvxCharacterMap, CharPreSelectHdl ) );
    m_xShowSet->SetFavClickHdl( LINK( this, SvxCharacterMap, FavClickHdl ) );

    m_xSearchSet->SetDoubleClickHdl( LINK( this, SvxCharacterMap, SearchCharDoubleClickHdl ) );
    m_xSearchSet->SetSelectHdl( LINK( this, SvxCharacterMap, SearchCharSelectHdl ) );
    m_xSearchSet->SetHighlightHdl( LINK( this, SvxCharacterMap, SearchCharHighlightHdl ) );
    m_xSearchSet->SetPreSelectHdl( LINK( this, SvxCharacterMap, SearchCharPreSelectHdl ) );
    m_xSearchSet->SetFavClickHdl( LINK( this, SvxCharacterMap, FavClickHdl ) );

    m_xDecimalCodeText->connect_changed( LINK( this, SvxCharacterMap, DecimalCodeChangeHdl ) );
    m_xHexCodeText->connect_changed( LINK( this, SvxCharacterMap, HexCodeChangeHdl ) );
    m_xFavouritesBtn->connect_clicked( LINK(this, SvxCharacterMap, FavSelectHdl));

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

    getRecentCharacterList();
    updateRecentCharControl();

    getFavCharacterList();
    updateFavCharControl();

    for(int i = 0; i < 16; i++)
    {
        m_aRecentCharView[i].SetHasInsert(m_bHasInsert);
        m_aRecentCharView[i].setMouseClickHdl(LINK(this,SvxCharacterMap, CharClickHdl));
        m_aRecentCharView[i].setClearClickHdl(LINK(this,SvxCharacterMap, RecentClearClickHdl));
        m_aRecentCharView[i].setClearAllClickHdl(LINK(this,SvxCharacterMap, RecentClearAllClickHdl));
        m_aFavCharView[i].SetHasInsert(m_bHasInsert);
        m_aFavCharView[i].setMouseClickHdl(LINK(this,SvxCharacterMap, CharClickHdl));
        m_aFavCharView[i].setClearClickHdl(LINK(this,SvxCharacterMap, FavClearClickHdl));
        m_aFavCharView[i].setClearAllClickHdl(LINK(this,SvxCharacterMap, FavClearAllClickHdl));
    }

    setCharName(90);

    m_xSearchText->connect_focus_in(LINK( this, SvxCharacterMap, SearchFieldGetFocusHdl ));
    m_xSearchText->connect_changed(LINK(this, SvxCharacterMap, SearchUpdateHdl));
}

bool SvxCharacterMap::isFavChar(const OUString& sTitle, const OUString& rFont)
{
    auto itChar = std::find_if(maFavCharList.begin(),
         maFavCharList.end(),
         [sTitle] (const OUString & a) { return a == sTitle; });

    auto itChar2 = std::find_if(maFavCharFontList.begin(),
         maFavCharFontList.end(),
         [rFont] (const OUString & a) { return a == rFont; });

    // if Fav char to be added is already in list, remove it
    if( itChar != maFavCharList.end() &&  itChar2 != maFavCharFontList.end() )
        return true;
    else
        return false;
}


void SvxCharacterMap::setFavButtonState(const OUString& sTitle, const OUString& rFont)
{
    if(sTitle.isEmpty() || rFont.isEmpty())
    {
        m_xFavouritesBtn->set_sensitive(false);
        return;
    }
    else
        m_xFavouritesBtn->set_sensitive(true);

    if(isFavChar(sTitle, rFont))
    {
        m_xFavouritesBtn->set_label(CuiResId(RID_SVXSTR_REMOVE_FAVORITES));
    }
    else
    {
        if(maFavCharList.size() == 16)
        {
            m_xFavouritesBtn->set_sensitive(false);
        }

        m_xFavouritesBtn->set_label(CuiResId(RID_SVXSTR_ADD_FAVORITES));
    }
}


void SvxCharacterMap::SetCharFont( const vcl::Font& rFont )
{
    // first get the underlying info in order to get font names
    // like "Times New Roman;Times" resolved
    vcl::Font aTmp(m_xVirDev->GetFontMetric(rFont));

    if (aTmp.GetFamilyName() == "StarSymbol" && m_xFontLB->find_text(aTmp.GetFamilyName()) == -1)
    {
        //if for some reason, like font in an old document, StarSymbol is requested and its not available, then
        //try OpenSymbol instead
        aTmp.SetFamilyName("OpenSymbol");
    }

    if (m_xFontLB->find_text(aTmp.GetFamilyName()) == -1)
        return;

    m_xFontLB->set_active_text(aTmp.GetFamilyName());
    aFont = aTmp;
    FontSelectHdl(*m_xFontLB);
}

void SvxCharacterMap::fillAllSubsets(weld::ComboBoxText& rListBox)
{
    SubsetMap aAll(nullptr);
    rListBox.clear();
    rListBox.freeze();
    for (auto & subset : aAll.GetSubsetMap())
        rListBox.append_text(subset.GetName());
    rListBox.thaw();
}


void SvxCharacterMap::insertCharToDoc(const OUString& sGlyph)
{
    if(sGlyph.isEmpty())
        return;

    if (m_bHasInsert) {
        uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );

        uno::Sequence<beans::PropertyValue> aArgs(2);
        aArgs[0].Name = "Symbols";
        aArgs[0].Value <<= sGlyph;

        aArgs[1].Name = "FontName";
        aArgs[1].Value <<= aFont.GetFamilyName();
        comphelper::dispatchCommand(".uno:InsertSymbol", aArgs);

        updateRecentCharacterList(sGlyph, aFont.GetFamilyName());

    } else {
        sal_Int32 tmp = 0;
        sal_UCS4 cChar = sGlyph.iterateCodePoints(&tmp);
        const SfxItemPool* pPool = m_xOutputSet->GetPool();
        m_xOutputSet->Put( SfxStringItem( pPool->GetWhich(SID_CHARMAP), sGlyph ) );
        m_xOutputSet->Put( SvxFontItem( aFont.GetFamilyType(), aFont.GetFamilyName(),
            aFont.GetStyleName(), aFont.GetPitch(), aFont.GetCharSet(), pPool->GetWhich(SID_ATTR_CHAR_FONT) ) );
        m_xOutputSet->Put( SfxStringItem( pPool->GetWhich(SID_FONT_NAME), aFont.GetFamilyName() ) );
        m_xOutputSet->Put( SfxInt32Item( pPool->GetWhich(SID_ATTR_CHAR), cChar ) );
    }
}

IMPL_LINK_NOARG(SvxCharacterMap, FontSelectHdl, weld::ComboBoxText&, void)
{
    const sal_uInt32 nFont = m_xFontLB->get_active_id().toUInt32();
    aFont = m_xVirDev->GetDevFont(nFont);
    aFont.SetWeight( WEIGHT_DONTKNOW );
    aFont.SetItalic( ITALIC_NONE );
    aFont.SetWidthType( WIDTH_DONTKNOW );
    aFont.SetPitch( PITCH_DONTKNOW );
    aFont.SetFamily( FAMILY_DONTKNOW );

    // notify children using this font
    m_xShowSet->SetFont( aFont );
    m_xSearchSet->SetFont( aFont );
    m_aShowChar.SetFont( aFont );
    if (isSearchMode)
    {
        SearchUpdateHdl(*m_xSearchText);
        SearchCharHighlightHdl(m_xSearchSet.get());
    }

    // setup unicode subset listbar with font specific subsets,
    // hide unicode subset listbar for symbol fonts
    // TODO: get info from the Font once it provides it
    delete pSubsetMap;
    pSubsetMap = nullptr;
    m_xSubsetLB->clear();

    bool bNeedSubset = (aFont.GetCharSet() != RTL_TEXTENCODING_SYMBOL);
    if (bNeedSubset)
    {
        FontCharMapRef xFontCharMap = m_xShowSet->GetFontCharMap();
        pSubsetMap = new SubsetMap( xFontCharMap );

        // update subset listbox for new font's unicode subsets
        bool bFirst = true;
        for (auto const& subset : pSubsetMap->GetSubsetMap())
        {
            m_xSubsetLB->append(OUString::number(reinterpret_cast<sal_uInt64>(&subset)), subset.GetName());
            // NOTE: subset must live at least as long as the selected font
            if (bFirst)
                m_xSubsetLB->set_active(0);
            bFirst = false;
        }

        if (m_xSubsetLB->get_count() <= 1)
            bNeedSubset = false;
    }

    m_xSubsetText->set_sensitive(bNeedSubset);
    m_xSubsetLB->set_sensitive(bNeedSubset);
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

IMPL_LINK_NOARG(SvxCharacterMap, SubsetSelectHdl, weld::ComboBoxText&, void)
{
    const sal_Int32 nPos = m_xSubsetLB->get_active();
    const Subset* pSubset = reinterpret_cast<const Subset*>(m_xSubsetLB->get_active_id().toUInt64());

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

IMPL_LINK(SvxCharacterMap, RecentClearClickHdl, SvxCharView*, rView, void)
{
    OUString sTitle = rView->GetText();
    auto itChar = std::find_if(maRecentCharList.begin(),
         maRecentCharList.end(),
         [sTitle] (const OUString & a) { return a == sTitle; });

    OUString sFont = rView->GetFont().GetFamilyName();
    auto itChar2 = std::find_if(maRecentCharFontList.begin(),
         maRecentCharFontList.end(),
         [sFont] (const OUString & a)
         { return a == sFont; });

    // if recent char to be added is already in list, remove it
    if( itChar != maRecentCharList.end() &&  itChar2 != maRecentCharFontList.end() )
    {
        maRecentCharList.erase( itChar );
        maRecentCharFontList.erase( itChar2);
    }

    css::uno::Sequence< OUString > aRecentCharList(maRecentCharList.size());
    css::uno::Sequence< OUString > aRecentCharFontList(maRecentCharFontList.size());

    for (size_t i = 0; i < maRecentCharList.size(); ++i)
    {
        aRecentCharList[i] = maRecentCharList[i];
        aRecentCharFontList[i] = maRecentCharFontList[i];
    }

    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create(mxContext));
    officecfg::Office::Common::RecentCharacters::RecentCharacterList::set(aRecentCharList, batch);
    officecfg::Office::Common::RecentCharacters::RecentCharacterFontList::set(aRecentCharFontList, batch);
    batch->commit();

    updateRecentCharControl();
}

IMPL_LINK_NOARG(SvxCharacterMap, RecentClearAllClickHdl, SvxCharView*, void)
{
    css::uno::Sequence< OUString > aRecentCharList(0);
    css::uno::Sequence< OUString > aRecentCharFontList(0);

    maRecentCharList.clear();
    maRecentCharFontList.clear();

    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create(mxContext));
    officecfg::Office::Common::RecentCharacters::RecentCharacterList::set(aRecentCharList, batch);
    officecfg::Office::Common::RecentCharacters::RecentCharacterFontList::set(aRecentCharFontList, batch);
    batch->commit();

    updateRecentCharControl();
}

IMPL_LINK(SvxCharacterMap, FavClearClickHdl, SvxCharView*, rView, void)
{
    deleteFavCharacterFromList(rView->GetText(), rView->GetFont().GetFamilyName());
    updateFavCharControl();
}

IMPL_LINK_NOARG(SvxCharacterMap, FavClearAllClickHdl, SvxCharView*, void)
{
    css::uno::Sequence< OUString > aFavCharList(0);
    css::uno::Sequence< OUString > aFavCharFontList(0);

    maFavCharList.clear();
    maFavCharFontList.clear();

    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create(mxContext));
    officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterList::set(aFavCharList, batch);
    officecfg::Office::Common::FavoriteCharacters::FavoriteCharacterFontList::set(aFavCharFontList, batch);
    batch->commit();

    updateFavCharControl();
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
    }
    else
    {
        toggleSearchView(false);
    }
}


IMPL_LINK(SvxCharacterMap, CharClickHdl, SvxCharView*, rView, void)
{
    m_aShowChar.SetText( rView->GetText() );
    m_aShowChar.SetFont(rView->GetFont());
    m_aShowChar.Invalidate();

    setFavButtonState(rView->GetText(), rView->GetFont().GetFamilyName());//check state
    rView->GrabFocus();

    // Get the hexadecimal code
    OUString charValue = rView->GetText();
    sal_Int32 tmp = 1;
    sal_UCS4 cChar = charValue.iterateCodePoints(&tmp, -1);
    OUString aHexText = OUString::number(cChar, 16).toAsciiUpperCase();

    // Get the decimal code
    OUString aDecimalText = OUString::number(cChar);

    m_xHexCodeText->set_text(aHexText);
    m_xDecimalCodeText->set_text(aDecimalText);
    setCharName(cChar);

    rView->Invalidate();
    m_xOKBtn->set_sensitive(true);
}

IMPL_LINK_NOARG(SvxCharacterMap, CharDoubleClickHdl, SvxShowCharSet*, void)
{
    sal_UCS4 cChar = m_xShowSet->GetSelectCharacter();
    // using the new UCS4 constructor
    OUString aOUStr( &cChar, 1 );
    setFavButtonState(aOUStr, aFont.GetFamilyName());
    insertCharToDoc(aOUStr);
}

IMPL_LINK_NOARG(SvxCharacterMap, SearchCharDoubleClickHdl, SvxShowCharSet*, void)
{
    sal_UCS4 cChar = m_xSearchSet->GetSelectCharacter();
    // using the new UCS4 constructor
    OUString aOUStr( &cChar, 1 );
    setFavButtonState(aOUStr, aFont.GetFamilyName());
    insertCharToDoc(aOUStr);
}

IMPL_LINK_NOARG(SvxCharacterMap, CharSelectHdl, SvxShowCharSet*, void)
{
    m_xOKBtn->set_sensitive(true);
}

IMPL_LINK_NOARG(SvxCharacterMap, SearchCharSelectHdl, SvxShowCharSet*, void)
{
    m_xOKBtn->set_sensitive(true);
}

IMPL_LINK_NOARG(SvxCharacterMap, InsertClickHdl, weld::Button&, void)
{
   insertCharToDoc(m_aShowChar.GetText());
   m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(SvxCharacterMap, FavSelectHdl, weld::Button&, void)
{
    if (m_xFavouritesBtn->get_label().match(CuiResId(RID_SVXSTR_ADD_FAVORITES)))
    {
        updateFavCharacterList(m_aShowChar.GetText(), m_aShowChar.GetFont().GetFamilyName());
        setFavButtonState(m_aShowChar.GetText(), m_aShowChar.GetFont().GetFamilyName());
    }
    else
    {
        deleteFavCharacterFromList(m_aShowChar.GetText(), m_aShowChar.GetFont().GetFamilyName());
        m_xFavouritesBtn->set_label(CuiResId(RID_SVXSTR_ADD_FAVORITES));
        m_xFavouritesBtn->set_sensitive(false);
    }

    updateFavCharControl();
}

IMPL_LINK_NOARG(SvxCharacterMap, FavClickHdl, SvxShowCharSet*, void)
{
    getFavCharacterList();
    updateFavCharControl();
}

IMPL_LINK_NOARG(SvxCharacterMap, CharHighlightHdl, SvxShowCharSet*, void)
{
    OUString aText;
    OUString aHexText;
    OUString aDecimalText;
    sal_UCS4 cChar = m_xShowSet->GetSelectCharacter();
    bool bSelect = (cChar > 0);

    // show char sample
    if ( bSelect )
    {
        // using the new UCS4 constructor
        aText = OUString( &cChar, 1 );
        // Get the hexadecimal code
        aHexText = OUString::number(cChar, 16).toAsciiUpperCase();
        // Get the decimal code
        aDecimalText = OUString::number(cChar);
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

    if(m_xShowSet->HasFocus() || m_xHexCodeText->has_focus() || m_xDecimalCodeText->has_focus() )
    {
        m_aShowChar.SetText( aText );
        m_aShowChar.SetFont( aFont );
        m_aShowChar.Invalidate();

        setFavButtonState(aText, aFont.GetFamilyName());
    }
}

IMPL_LINK_NOARG(SvxCharacterMap, SearchCharHighlightHdl, SvxShowCharSet*, void)
{
    OUString aText;
    OUString aHexText;
    OUString aDecimalText;
    sal_UCS4 cChar = m_xSearchSet->GetSelectCharacter();
    bool bSelect = (cChar > 0);

    // show char sample
    if ( bSelect )
    {
        aText = OUString( &cChar, 1 );
        // Get the hexadecimal code
        aHexText = OUString::number(cChar, 16).toAsciiUpperCase();
        // Get the decimal code
        aDecimalText = OUString::number(cChar);
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
        m_xCharName->set_label(CuiResId(RID_SVXSTR_MISSING_GLYPH));
        m_aShowChar.SetText(" ");
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

IMPL_LINK_NOARG(SvxCharacterMap, CharPreSelectHdl, SvxShowCharSet*, void)
{
    // adjust subset selection
    if( pSubsetMap )
    {
        sal_UCS4 cChar = m_xShowSet->GetSelectCharacter();

        setFavButtonState(OUString(&cChar, 1), aFont.GetFamilyName());
        const Subset* pSubset = pSubsetMap->GetSubsetByUnicode( cChar );
        if( pSubset )
            m_xSubsetLB->set_active_text(pSubset->GetName());
    }

    m_xOKBtn->set_sensitive(true);
}

IMPL_LINK_NOARG(SvxCharacterMap, SearchCharPreSelectHdl, SvxShowCharSet*, void)
{
    // adjust subset selection
    if( pSubsetMap )
    {
        sal_UCS4 cChar = m_xSearchSet->GetSelectCharacter();

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
    vcl::Font aFont = m_xVirDev->GetFont();
    Size aFontSize(aFont.GetFontSize().Width() * 5, aFont.GetFontSize().Height() * 5);
    aFont.SetFontSize(aFontSize);
    m_xVirDev->Push(PUSH_ALLFONT);
    m_xVirDev->SetFont(aFont);
    pDrawingArea->set_size_request(m_xVirDev->approximate_digit_width() + 2 * 12,
                                   m_xVirDev->LogicToPixel(aFontSize).Height() * 2);
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    m_xVirDev->Pop();
}

void SvxShowText::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    rRenderContext.SetFont(m_aFont);

    Color aTextCol = rRenderContext.GetTextColor();
    Color aFillCol = rRenderContext.GetFillColor();

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    const Color aWindowTextColor(rStyleSettings.GetDialogTextColor());
    const Color aWindowColor(rStyleSettings.GetWindowColor());
    rRenderContext.SetTextColor(aWindowTextColor);
    rRenderContext.SetFillColor(aWindowColor);

    const OUString aText = GetText();

    Size aSize(GetOutputSizePixel());
    long nAvailWidth = aSize.Width();
    long nWinHeight = aSize.Height();

    bool bGotBoundary = true;
    bool bShrankFont = false;
    vcl::Font aOrigFont(rRenderContext.GetFont());
    Size aFontSize(aOrigFont.GetFontSize());
    ::tools::Rectangle aBoundRect;

    for (long nFontHeight = aFontSize.Height(); nFontHeight > 0; nFontHeight -= 5)
    {
        if (!rRenderContext.GetTextBoundRect( aBoundRect, aText ) || aBoundRect.IsEmpty())
        {
            bGotBoundary = false;
            break;
        }
        if (!mbCenter)
            break;
        //only shrink in the single glyph large view mode
        long nTextWidth = aBoundRect.GetWidth();
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

    rRenderContext.DrawRect(tools::Rectangle(Point(0, 0), aSize));
    rRenderContext.DrawText(aPoint, aText);
    rRenderContext.SetTextColor(aTextCol);
    rRenderContext.SetFillColor(aFillCol);
    if (bShrankFont)
        rRenderContext.SetFont(aOrigFont);
}

void SvxShowText::SetFont( const vcl::Font& rFont )
{
    long nWinHeight = GetOutputSizePixel().Height();

    m_aFont = vcl::Font(rFont);
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
