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
#include <vcl/msgbox.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/fontcharmap.hxx>
#include <svl/stritem.hxx>
#include <officecfg/Office/Common.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
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

// class SvxCharacterMap =================================================

SvxCharacterMap::SvxCharacterMap( vcl::Window* pParent, const SfxItemSet* pSet, bool bInsert )
    : SfxModalDialog(pParent, "SpecialCharactersDialog", "cui/ui/specialcharacters.ui")
    , pSubsetMap( nullptr )
    , isSearchMode(true)
    , m_bHasInsert(bInsert)
    , mxContext(comphelper::getProcessComponentContext())
{
    get(m_pShowSet, "showcharset");
    get(m_pSearchSet, "searchcharset");
    get(m_pShowChar, "showchar");
    m_pShowChar->SetCentered(true);
    if (m_bHasInsert) get(m_pOKBtn, "insert");
    else get(m_pOKBtn, "ok");
    get(m_pFontText, "fontft");
    get(m_pFontLB, "fontlb");
    m_pFontLB->SetStyle(m_pFontLB->GetStyle() | WB_SORT);
    get(m_pSubsetText, "subsetft");
    get(m_pSubsetLB, "subsetlb");
    //lock the size request of this widget to the width of all possible entries
    fillAllSubsets(*m_pSubsetLB);
    m_pSubsetLB->set_width_request(m_pSubsetLB->get_preferred_size().Width());
    get(m_pHexCodeText, "hexvalue");    get(m_pDecimalCodeText, "decimalvalue");
    get(m_pFavouritesBtn, "favbtn");
    get(m_pCharName, "charname");
    m_pCharName->set_height_request(m_pCharName->GetTextHeight()*3);
    m_pCharName->SetPaintTransparent(true);
    get(m_pSearchText, "search");
    //lock the size request of this widget to the width of the original .ui string
    m_pHexCodeText->set_width_request(m_pHexCodeText->get_preferred_size().Width());

    get( m_pRecentCharView[0], "viewchar1" );
    get( m_pRecentCharView[1], "viewchar2" );
    get( m_pRecentCharView[2], "viewchar3" );
    get( m_pRecentCharView[3], "viewchar4" );
    get( m_pRecentCharView[4], "viewchar5" );
    get( m_pRecentCharView[5], "viewchar6" );
    get( m_pRecentCharView[6], "viewchar7" );
    get( m_pRecentCharView[7], "viewchar8" );
    get( m_pRecentCharView[8], "viewchar9" );
    get( m_pRecentCharView[9], "viewchar10" );
    get( m_pRecentCharView[10], "viewchar11" );
    get( m_pRecentCharView[11], "viewchar12" );
    get( m_pRecentCharView[12], "viewchar13" );
    get( m_pRecentCharView[13], "viewchar14" );
    get( m_pRecentCharView[14], "viewchar15" );
    get( m_pRecentCharView[15], "viewchar16" );

    get( m_pFavCharView[0], "favchar1" );
    get( m_pFavCharView[1], "favchar2" );
    get( m_pFavCharView[2], "favchar3" );
    get( m_pFavCharView[3], "favchar4" );
    get( m_pFavCharView[4], "favchar5" );
    get( m_pFavCharView[5], "favchar6" );
    get( m_pFavCharView[6], "favchar7" );
    get( m_pFavCharView[7], "favchar8" );
    get( m_pFavCharView[8], "favchar9" );
    get( m_pFavCharView[9], "favchar10" );
    get( m_pFavCharView[10], "favchar11" );
    get( m_pFavCharView[11], "favchar12" );
    get( m_pFavCharView[12], "favchar13" );
    get( m_pFavCharView[13], "favchar14" );
    get( m_pFavCharView[14], "favchar15" );
    get( m_pFavCharView[15], "favchar16" );

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

    CreateOutputItemSet( pSet ? *pSet->GetPool() : SfxGetpApp()->GetPool() );
    m_pShowSet->Show();
    m_pSearchSet->Hide();
}

SvxCharacterMap::~SvxCharacterMap()
{
    disposeOnce();
}

short SvxCharacterMap::Execute()
{
    if( SvxShowCharSet::getSelectedChar() == ' ')
    {
        m_pOKBtn->Disable();
        setFavButtonState(OUString(), OUString());
    }
    else
    {
        sal_UCS4 cChar = m_pShowSet->GetSelectCharacter();
        // using the new UCS4 constructor
        OUString aOUStr( &cChar, 1 );
        m_pShowChar->SetText(aOUStr);

        setFavButtonState(aOUStr, m_pShowChar->GetFont().GetFamilyName());
        m_pOKBtn->Enable();
    }

    return ModalDialog::Execute();
}

void SvxCharacterMap::dispose()
{
    for(int i = 0; i < 16; i++)
        m_pRecentCharView[i].clear();

    m_pShowSet.clear();
    m_pSearchSet.clear();
    m_pOKBtn.clear();
    m_pFontText.clear();
    m_pFontLB.clear();
    m_pSubsetText.clear();
    m_pSubsetLB.clear();
    m_pShowChar.clear();
    m_pHexCodeText.clear();
    m_pDecimalCodeText.clear();
    m_pCharName.clear();

    maRecentCharList.clear();
    maRecentCharFontList.clear();
    maFavCharList.clear();
    maFavCharFontList.clear();

    m_pFavouritesBtn.clear();
    m_pSearchText.clear();

    SfxModalDialog::dispose();
}


void SvxCharacterMap::SetChar( sal_UCS4 c )
{
    m_pShowSet->SelectCharacter( c );

    setFavButtonState(OUString(&c, 1), aFont.GetFamilyName());
}


sal_UCS4 SvxCharacterMap::GetChar() const
{
   return (m_pShowChar->GetText()).toChar();
}


void SvxCharacterMap::DisableFontSelection()
{
    m_pFontText->Disable();
    m_pFontLB->Disable();
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
        m_pRecentCharView[i]->SetText(*it);
        vcl::Font rFont = m_pRecentCharView[i]->GetControlFont();
        rFont.SetFamilyName( *it2 );
        m_pRecentCharView[i]->SetFont(rFont);
        m_pRecentCharView[i]->Show();
    }

    for(; i < 16 ; i++)
    {
        m_pRecentCharView[i]->SetText(OUString());
        m_pRecentCharView[i]->Hide();
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
        m_pFavCharView[i]->SetText(*it);
        vcl::Font rFont = m_pFavCharView[i]->GetControlFont();
        rFont.SetFamilyName( *it2 );
        m_pFavCharView[i]->SetFont(rFont);
        m_pFavCharView[i]->Show();
    }

    for(; i < 16 ; i++)
    {
        m_pFavCharView[i]->SetText(OUString());
        m_pFavCharView[i]->Hide();
    }
    m_pShowSet->getFavCharacterList();
    m_pSearchSet->getFavCharacterList();
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
    aFont = GetFont();
    aFont.SetTransparent( true );
    aFont.SetFamily( FAMILY_DONTKNOW );
    aFont.SetPitch( PITCH_DONTKNOW );
    aFont.SetCharSet( RTL_TEXTENCODING_DONTKNOW );

    OUString aDefStr( aFont.GetFamilyName() );
    OUString aLastName;
    int nCount = GetDevFontCount();
    for ( int i = 0; i < nCount; i++ )
    {
        OUString aFontName( GetDevFont( i ).GetFamilyName() );
        if ( aFontName != aLastName )
        {
            aLastName = aFontName;
            const sal_Int32 nPos = m_pFontLB->InsertEntry( aFontName );
            m_pFontLB->SetEntryData( nPos, reinterpret_cast<void*>(i) );
        }
    }
    // the font may not be in the list =>
    // try to find a font name token in list and select found font,
    // else select topmost entry
    bool bFound = (m_pFontLB->GetEntryPos( aDefStr ) == LISTBOX_ENTRY_NOTFOUND );
    if( !bFound )
    {
        sal_Int32 nIndex = 0;
        do
        {
            OUString aToken = aDefStr.getToken(0, ';', nIndex);
            if ( m_pFontLB->GetEntryPos( aToken ) != LISTBOX_ENTRY_NOTFOUND )
            {
                aDefStr = aToken;
                bFound = true;
                break;
            }
        }
        while ( nIndex >= 0 );
    }

    if ( bFound )
        m_pFontLB->SelectEntry( aDefStr );
    else if ( m_pFontLB->GetEntryCount() )
        m_pFontLB->SelectEntryPos(0);
    FontSelectHdl(*m_pFontLB);

    m_pFontLB->SetSelectHdl( LINK( this, SvxCharacterMap, FontSelectHdl ) );
    m_pSubsetLB->SetSelectHdl( LINK( this, SvxCharacterMap, SubsetSelectHdl ) );
    m_pOKBtn->SetClickHdl( LINK( this, SvxCharacterMap, InsertClickHdl ) );
    m_pOKBtn->Show();


    m_pShowSet->SetDoubleClickHdl( LINK( this, SvxCharacterMap, CharDoubleClickHdl ) );
    m_pShowSet->SetSelectHdl( LINK( this, SvxCharacterMap, CharSelectHdl ) );
    m_pShowSet->SetHighlightHdl( LINK( this, SvxCharacterMap, CharHighlightHdl ) );
    m_pShowSet->SetPreSelectHdl( LINK( this, SvxCharacterMap, CharPreSelectHdl ) );
    m_pShowSet->SetFavClickHdl( LINK( this, SvxCharacterMap, FavClickHdl ) );

    m_pSearchSet->SetDoubleClickHdl( LINK( this, SvxCharacterMap, SearchCharDoubleClickHdl ) );
    m_pSearchSet->SetSelectHdl( LINK( this, SvxCharacterMap, SearchCharSelectHdl ) );
    m_pSearchSet->SetHighlightHdl( LINK( this, SvxCharacterMap, SearchCharHighlightHdl ) );
    m_pSearchSet->SetPreSelectHdl( LINK( this, SvxCharacterMap, SearchCharPreSelectHdl ) );
    m_pSearchSet->SetFavClickHdl( LINK( this, SvxCharacterMap, FavClickHdl ) );

    m_pDecimalCodeText->SetModifyHdl( LINK( this, SvxCharacterMap, DecimalCodeChangeHdl ) );
    m_pHexCodeText->SetModifyHdl( LINK( this, SvxCharacterMap, HexCodeChangeHdl ) );
    m_pFavouritesBtn->SetClickHdl( LINK(this, SvxCharacterMap, FavSelectHdl));

    if( SvxShowCharSet::getSelectedChar() == ' ')
    {
        m_pOKBtn->Disable();
    }
    else
    {
        sal_UCS4 cChar = m_pShowSet->GetSelectCharacter();
        // using the new UCS4 constructor
        OUString aOUStr( &cChar, 1 );
        m_pShowChar->SetText(aOUStr);

        setFavButtonState(aOUStr, aDefStr);
        m_pOKBtn->Enable();
    }

    getRecentCharacterList();
    updateRecentCharControl();

    getFavCharacterList();
    updateFavCharControl();

    for(int i = 0; i < 16; i++)
    {
        m_pRecentCharView[i]->setMouseClickHdl(LINK(this,SvxCharacterMap, CharClickHdl));
        m_pRecentCharView[i]->setClearClickHdl(LINK(this,SvxCharacterMap, RecentClearClickHdl));
        m_pRecentCharView[i]->setClearAllClickHdl(LINK(this,SvxCharacterMap, RecentClearAllClickHdl));
        m_pRecentCharView[i]->SetLoseFocusHdl(LINK(this,SvxCharacterMap, LoseFocusHdl));
        m_pFavCharView[i]->setMouseClickHdl(LINK(this,SvxCharacterMap, CharClickHdl));
        m_pFavCharView[i]->setClearClickHdl(LINK(this,SvxCharacterMap, FavClearClickHdl));
        m_pFavCharView[i]->setClearAllClickHdl(LINK(this,SvxCharacterMap, FavClearAllClickHdl));
        m_pFavCharView[i]->SetLoseFocusHdl(LINK(this,SvxCharacterMap, LoseFocusHdl));
    }

    setCharName(90);

    m_pSearchText->SetGetFocusHdl(LINK( this, SvxCharacterMap, SearchFieldGetFocusHdl ));
    m_pSearchText->SetUpdateDataHdl(LINK( this, SvxCharacterMap, SearchUpdateHdl ));
    m_pSearchText->EnableUpdateData();
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
        m_pFavouritesBtn->Disable();
        return;
    }
    else
        m_pFavouritesBtn->Enable();

    if(isFavChar(sTitle, rFont))
    {
        m_pFavouritesBtn->SetText(CuiResId(RID_SVXSTR_REMOVE_FAVORITES));
    }
    else
    {
        if(maFavCharList.size() == 16)
        {
            m_pFavouritesBtn->Disable();
        }

        m_pFavouritesBtn->SetText(CuiResId(RID_SVXSTR_ADD_FAVORITES));
    }
}


void SvxCharacterMap::SetCharFont( const vcl::Font& rFont )
{
    // first get the underlying info in order to get font names
    // like "Times New Roman;Times" resolved
    vcl::Font aTmp( GetFontMetric( rFont ) );

    if (aTmp.GetFamilyName() == "StarSymbol" && m_pFontLB->GetEntryPos(aTmp.GetFamilyName()) == LISTBOX_ENTRY_NOTFOUND)
    {
        //if for some reason, like font in an old document, StarSymbol is requested and its not available, then
        //try OpenSymbol instead
        aTmp.SetFamilyName("OpenSymbol");
    }

    if ( m_pFontLB->GetEntryPos( aTmp.GetFamilyName() ) == LISTBOX_ENTRY_NOTFOUND )
        return;

    m_pFontLB->SelectEntry( aTmp.GetFamilyName() );
    aFont = aTmp;
    FontSelectHdl(*m_pFontLB);

    // for compatibility reasons
    ModalDialog::SetFont( aFont );
}


void SvxCharacterMap::fillAllSubsets(ListBox &rListBox)
{
    SubsetMap aAll(nullptr);
    rListBox.Clear();
    for (auto & subset : aAll.GetSubsetMap())
        rListBox.InsertEntry( subset.GetName() );
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

    } else {
        SfxItemSet* pSet = GetOutputSetImpl();
        if ( pSet )
        {
            sal_Int32 tmp = 0;
            sal_UCS4 cChar = sGlyph.iterateCodePoints(&tmp);
            const SfxItemPool* pPool = pSet->GetPool();
            pSet->Put( SfxStringItem( pPool->GetWhich(SID_CHARMAP), sGlyph ) );
            pSet->Put( SvxFontItem( aFont.GetFamilyType(), aFont.GetFamilyName(),
                aFont.GetStyleName(), aFont.GetPitch(), aFont.GetCharSet(), pPool->GetWhich(SID_ATTR_CHAR_FONT) ) );
            pSet->Put( SfxStringItem( pPool->GetWhich(SID_FONT_NAME), aFont.GetFamilyName() ) );
            pSet->Put( SfxInt32Item( pPool->GetWhich(SID_ATTR_CHAR), cChar ) );
        }
    }

    updateRecentCharacterList(sGlyph, aFont.GetFamilyName());
}


IMPL_LINK_NOARG(SvxCharacterMap, FontSelectHdl, ListBox&, void)
{
    const sal_Int32 nPos = m_pFontLB->GetSelectedEntryPos();
    const sal_uInt16 nFont = (sal_uInt16)reinterpret_cast<sal_uLong>(m_pFontLB->GetEntryData( nPos ));
    aFont = GetDevFont( nFont );
    aFont.SetWeight( WEIGHT_DONTKNOW );
    aFont.SetItalic( ITALIC_NONE );
    aFont.SetWidthType( WIDTH_DONTKNOW );
    aFont.SetPitch( PITCH_DONTKNOW );
    aFont.SetFamily( FAMILY_DONTKNOW );

    // notify children using this font
    m_pShowSet->SetFont( aFont );
    m_pSearchSet->SetFont( aFont );
    m_pShowChar->SetFont( aFont );
    if(isSearchMode)
    {
        SearchUpdateHdl(*m_pSearchText);
        SearchCharHighlightHdl(m_pSearchSet);
    }

    // setup unicode subset listbar with font specific subsets,
    // hide unicode subset listbar for symbol fonts
    // TODO: get info from the Font once it provides it
    delete pSubsetMap;
    pSubsetMap = nullptr;
    m_pSubsetLB->Clear();

    bool bNeedSubset = (aFont.GetCharSet() != RTL_TEXTENCODING_SYMBOL);
    if( bNeedSubset )
    {
        FontCharMapRef xFontCharMap( new FontCharMap() );
        m_pShowSet->GetFontCharMap( xFontCharMap );
        pSubsetMap = new SubsetMap( xFontCharMap );

        // update subset listbox for new font's unicode subsets
        bool bFirst = true;
        for (auto const& subset : pSubsetMap->GetSubsetMap())
        {
            const sal_Int32 nPos_ = m_pSubsetLB->InsertEntry( subset.GetName() );
            m_pSubsetLB->SetEntryData( nPos_, const_cast<Subset *>(&subset) );
            // NOTE: subset must live at least as long as the selected font
            if( bFirst )
                m_pSubsetLB->SelectEntryPos( nPos_ );
            bFirst = false;
        }
        if( m_pSubsetLB->GetEntryCount() <= 1 )
            bNeedSubset = false;
    }

    m_pSubsetText->Enable(bNeedSubset);
    m_pSubsetLB->Enable(bNeedSubset);
}

void SvxCharacterMap::toggleSearchView(bool state)
{
    isSearchMode = state;
    m_pHexCodeText->SetReadOnly(state);
    m_pDecimalCodeText->SetReadOnly(state);
    m_pSubsetLB->Enable(!state);

    if(state)
    {
        m_pSearchSet->Show();
        m_pShowSet->Hide();
    }
    else
    {
        m_pSearchSet->Hide();
        m_pShowSet->Show();
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
        m_pCharName->SetText(OUString::createFromAscii(buffer));
}

IMPL_LINK_NOARG(SvxCharacterMap, SubsetSelectHdl, ListBox&, void)
{
    const sal_Int32 nPos = m_pSubsetLB->GetSelectedEntryPos();
    const Subset* pSubset = static_cast<const Subset*> (m_pSubsetLB->GetEntryData(nPos));
    if( pSubset && !isSearchMode)
    {
        sal_UCS4 cFirst = pSubset->GetRangeMin();
        m_pShowSet->SelectCharacter( cFirst );

        setFavButtonState(OUString(&cFirst, 1), aFont.GetFamilyName());
        m_pSubsetLB->SelectEntryPos( nPos );
    }
    else if( pSubset && isSearchMode)
    {
        m_pSearchSet->SelectCharacter( pSubset );

        const Subset* curSubset = nullptr;
        if( pSubsetMap )
            curSubset = pSubsetMap->GetSubsetByUnicode( m_pSearchSet->GetSelectCharacter() );
        if( curSubset )
            m_pSubsetLB->SelectEntry( curSubset->GetName() );
        else
            m_pSubsetLB->SetNoSelection();

        sal_UCS4 sChar = m_pSearchSet->GetSelectCharacter();
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

IMPL_LINK_NOARG(SvxCharacterMap, SearchFieldGetFocusHdl, Control&, void)
{
    m_pOKBtn->Disable();
}


IMPL_LINK_NOARG(SvxCharacterMap, SearchUpdateHdl, Edit&, void)
{
    if(!m_pSearchText->GetText().isEmpty())
    {
        m_pSearchSet->ClearPreviousData();
        OUString aKeyword = m_pSearchText->GetText();

        toggleSearchView(true);

        FontCharMapRef xFontCharMap(new FontCharMap());
        m_pSearchSet->GetFontCharMap(xFontCharMap);

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
                    m_pSearchSet->AppendCharToList(sChar);
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
                m_pSearchSet->AppendCharToList(sChar);
        }

        m_pSearchSet->Resize();

    }
    else
    {
        toggleSearchView(false);
    }
}


IMPL_LINK(SvxCharacterMap, CharClickHdl, SvxCharView*, rView, void)
{
    m_pShowChar->SetText( rView->GetText() );
    m_pShowChar->SetFont(rView->GetFont());
    m_pShowChar->Update();

    setFavButtonState(rView->GetText(), rView->GetFont().GetFamilyName());//check state
    rView->GrabFocus();

    // Get the hexadecimal code
    OUString charValue = rView->GetText();
    sal_Int32 tmp = 1;
    sal_UCS4 cChar = charValue.iterateCodePoints(&tmp, -1);
    OUString aHexText = OUString::number(cChar, 16).toAsciiUpperCase();

    // Get the decimal code
    OUString aDecimalText = OUString::number(cChar);

    m_pHexCodeText->SetText( aHexText );
    m_pDecimalCodeText->SetText( aDecimalText );
    setCharName(cChar);

    rView->Invalidate();
    m_pOKBtn->Enable();
}

IMPL_LINK_NOARG(SvxCharacterMap, CharDoubleClickHdl, SvxShowCharSet*, void)
{
    sal_UCS4 cChar = m_pShowSet->GetSelectCharacter();
    // using the new UCS4 constructor
    OUString aOUStr( &cChar, 1 );
    setFavButtonState(aOUStr, aFont.GetFamilyName());
    insertCharToDoc(aOUStr);
}

IMPL_LINK_NOARG(SvxCharacterMap, SearchCharDoubleClickHdl, SvxShowCharSet*, void)
{
    sal_UCS4 cChar = m_pSearchSet->GetSelectCharacter();
    // using the new UCS4 constructor
    OUString aOUStr( &cChar, 1 );
    setFavButtonState(aOUStr, aFont.GetFamilyName());
    insertCharToDoc(aOUStr);
}

IMPL_LINK_NOARG(SvxCharacterMap, CharSelectHdl, SvxShowCharSet*, void)
{
    m_pOKBtn->Enable();
}

IMPL_LINK_NOARG(SvxCharacterMap, SearchCharSelectHdl, SvxShowCharSet*, void)
{
    m_pOKBtn->Enable();
}

IMPL_LINK_NOARG(SvxCharacterMap, InsertClickHdl, Button*, void)
{
   insertCharToDoc(m_pShowChar->GetText());
   EndDialog(RET_OK);
}


IMPL_STATIC_LINK(SvxCharacterMap, LoseFocusHdl, Control&, pItem, void)
{
    pItem.Invalidate();
}

IMPL_LINK_NOARG(SvxCharacterMap, FavSelectHdl, Button*, void)
{
    if(m_pFavouritesBtn->GetText().match(CuiResId(RID_SVXSTR_ADD_FAVORITES)))
    {
        updateFavCharacterList(m_pShowChar->GetText(), m_pShowChar->GetFont().GetFamilyName());
        setFavButtonState(m_pShowChar->GetText(), m_pShowChar->GetFont().GetFamilyName());
    }
    else
    {
        deleteFavCharacterFromList(m_pShowChar->GetText(), m_pShowChar->GetFont().GetFamilyName());
        m_pFavouritesBtn->SetText(CuiResId(RID_SVXSTR_ADD_FAVORITES));
        m_pFavouritesBtn->Disable();
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
    sal_UCS4 cChar = m_pShowSet->GetSelectCharacter();
    bool bSelect = (cChar > 0);

    // show char sample
    if ( bSelect )
    {
        // using the new UCS4 constructor
        aText = OUString( &cChar, 1 );

        const Subset* pSubset = nullptr;
        if( pSubsetMap )
            pSubset = pSubsetMap->GetSubsetByUnicode( cChar );
        if( pSubset )
            m_pSubsetLB->SelectEntry( pSubset->GetName() );
        else
            m_pSubsetLB->SetNoSelection();
    }

    if(m_pShowSet->HasFocus())
    {
        m_pShowChar->SetText( aText );
        m_pShowChar->SetFont( aFont );
        m_pShowChar->Update();

        setFavButtonState(aText, aFont.GetFamilyName());
    }

    // show char codes
    if ( bSelect )
    {
        // Get the hexadecimal code
        aHexText = OUString::number(cChar, 16).toAsciiUpperCase();
        // Get the decimal code
        aDecimalText = OUString::number(cChar);
        setCharName(cChar);
    }

    // Update the hex and decimal codes only if necessary
    if (m_pHexCodeText->GetText() != aHexText)
        m_pHexCodeText->SetText( aHexText );
    if (m_pDecimalCodeText->GetText() != aDecimalText)
        m_pDecimalCodeText->SetText( aDecimalText );
}

IMPL_LINK_NOARG(SvxCharacterMap, SearchCharHighlightHdl, SvxShowCharSet*, void)
{
    OUString aText;
    OUString aHexText;
    OUString aDecimalText;
    sal_UCS4 cChar = m_pSearchSet->GetSelectCharacter();
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
        if (m_pHexCodeText->GetText() != aHexText)
            m_pHexCodeText->SetText( aHexText );
        if (m_pDecimalCodeText->GetText() != aDecimalText)
            m_pDecimalCodeText->SetText( aDecimalText );

        const Subset* pSubset = nullptr;
        if( pSubsetMap )
            pSubset = pSubsetMap->GetSubsetByUnicode( cChar );
        if( pSubset )
            m_pSubsetLB->SelectEntry( pSubset->GetName() );
        else
            m_pSubsetLB->SetNoSelection();
    }

    if(m_pSearchSet->HasFocus())
    {
        m_pShowChar->SetText( aText );
        m_pShowChar->SetFont( aFont );
        m_pShowChar->Update();

        setFavButtonState(aText, aFont.GetFamilyName());
    }
}

void SvxCharacterMap::selectCharByCode(Radix radix)
{
    OUString aCodeString;
    switch(radix)
    {
        case Radix::decimal:
            aCodeString = m_pDecimalCodeText->GetText();
            break;
        case Radix::hexadecimal:
            aCodeString = m_pHexCodeText->GetText();
            break;
    }
    // Convert the code back to a character using the appropriate radix
    sal_UCS4 cChar = aCodeString.toUInt32(static_cast<sal_Int16> (radix));
    // Use FontCharMap::HasChar(sal_UCS4 cChar) to see if the desired character is in the font
    FontCharMapRef xFontCharMap(new FontCharMap());
    m_pShowSet->GetFontCharMap(xFontCharMap);
    if (xFontCharMap->HasChar(cChar))
        // Select the corresponding character
        SetChar(cChar);
}

IMPL_LINK_NOARG(SvxCharacterMap, DecimalCodeChangeHdl, Edit&, void)
{
    selectCharByCode(Radix::decimal);
}

IMPL_LINK_NOARG(SvxCharacterMap, HexCodeChangeHdl, Edit&, void)
{
    selectCharByCode(Radix::hexadecimal);
}

IMPL_LINK_NOARG(SvxCharacterMap, CharPreSelectHdl, SvxShowCharSet*, void)
{
    // adjust subset selection
    if( pSubsetMap )
    {
        sal_UCS4 cChar = m_pShowSet->GetSelectCharacter();

        setFavButtonState(OUString(&cChar, 1), aFont.GetFamilyName());
        const Subset* pSubset = pSubsetMap->GetSubsetByUnicode( cChar );
        if( pSubset )
            m_pSubsetLB->SelectEntry( pSubset->GetName() );
    }

    m_pOKBtn->Enable();
}

IMPL_LINK_NOARG(SvxCharacterMap, SearchCharPreSelectHdl, SvxShowCharSet*, void)
{
    // adjust subset selection
    if( pSubsetMap )
    {
        sal_UCS4 cChar = m_pSearchSet->GetSelectCharacter();

        setFavButtonState(OUString(&cChar, 1), aFont.GetFamilyName());
        const Subset* pSubset = pSubsetMap->GetSubsetByUnicode( cChar );
        if( pSubset )
            m_pSubsetLB->SelectEntry( pSubset->GetName() );
    }

    m_pOKBtn->Enable();
}



// class SvxShowText =====================================================

SvxShowText::SvxShowText(vcl::Window* pParent)
    : Control(pParent, WB_BORDER)
    , mnY(0)
    , mbCenter(false)
{}

VCL_BUILDER_FACTORY(SvxShowText)

void SvxShowText::Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle&)
{
    rRenderContext.SetFont(maFont);

    Color aTextCol = rRenderContext.GetTextColor();
    Color aFillCol = rRenderContext.GetFillColor();

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    const Color aWindowTextColor(rStyleSettings.GetDialogTextColor());
    const Color aWindowColor(rStyleSettings.GetWindowColor());
    rRenderContext.SetTextColor(aWindowTextColor);
    rRenderContext.SetFillColor(aWindowColor);

    const OUString aText = GetText();
    const Size aSize(GetOutputSizePixel());

    long nAvailWidth = aSize.Width();
    long nWinHeight = GetOutputSizePixel().Height();

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
        aFontSize.Height() = nFontHeight;
        aFont.SetFontSize(aFontSize);
        rRenderContext.SetFont(aFont);
        mnY = (nWinHeight - GetTextHeight()) / 2;
        bShrankFont = true;
    }

    Point aPoint(2, mnY);
    // adjust position using ink boundary if possible
    if (!bGotBoundary)
        aPoint.X() = (aSize.Width() - rRenderContext.GetTextWidth(aText)) / 2;
    else
    {
        // adjust position before it gets out of bounds
        aBoundRect += aPoint;

        // shift back vertically if needed
        int nYLDelta = aBoundRect.Top();
        int nYHDelta = aSize.Height() - aBoundRect.Bottom();
        if( nYLDelta <= 0 )
            aPoint.Y() -= nYLDelta - 1;
        else if( nYHDelta <= 0 )
            aPoint.Y() += nYHDelta - 1;

        if (mbCenter)
        {
            // move glyph to middle of cell
            aPoint.X() = -aBoundRect.Left() + (aSize.Width() - aBoundRect.GetWidth()) / 2;
        }
        else
        {
            // shift back horizontally if needed
            int nXLDelta = aBoundRect.Left();
            int nXHDelta = aSize.Width() - aBoundRect.Right();
            if( nXLDelta <= 0 )
                aPoint.X() -= nXLDelta - 1;
            else if( nXHDelta <= 0 )
                aPoint.X() += nXHDelta - 1;
        }
    }

    rRenderContext.DrawRect(tools::Rectangle(Point(0, 0), Size(GetOutputSizePixel().Width(), GetOutputSizePixel().Height())));
    rRenderContext.DrawText(aPoint, aText);
    rRenderContext.SetTextColor(aTextCol);
    rRenderContext.SetFillColor(aFillCol);
    if (bShrankFont)
        rRenderContext.SetFont(aOrigFont);
}


void SvxShowText::SetFont( const vcl::Font& rFont )
{
    long nWinHeight = GetOutputSizePixel().Height();
    maFont = vcl::Font(rFont);
    maFont.SetWeight(WEIGHT_NORMAL);
    maFont.SetAlignment(ALIGN_TOP);
    maFont.SetFontSize(PixelToLogic(Size(0, nWinHeight / 2)));
    maFont.SetTransparent(true);
    Control::SetFont(maFont);

    mnY = (nWinHeight - GetTextHeight()) / 2;

    Invalidate();
}

Size SvxShowText::GetOptimalSize() const
{
    const vcl::Font &rFont = GetFont();
    const Size rFontSize = rFont.GetFontSize();
    long nWinHeight = LogicToPixel(rFontSize).Height() * 2;
    return Size( GetTextWidth( GetText() ) + 2 * 12, nWinHeight );
}

void SvxShowText::Resize()
{
    Control::Resize();
    SetFont(GetFont()); //force recalculation of size
}


void SvxShowText::SetText( const OUString& rText )
{
    Control::SetText( rText );
    Invalidate();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
