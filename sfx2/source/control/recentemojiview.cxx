/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/emojiview.hxx>
#include <sfx2/recentemojiview.hxx>
#include <sfx2/emojiviewitem.hxx>
#include <sfx2/templatelocalview.hxx>
#include <sfx2/sfxresid.hxx>
#include <tools/urlobj.hxx>
#include <vcl/layout.hxx>
#include <rtl/bootstrap.hxx>
#include <config_folders.h>
#include <officecfg/Office/Common.hxx>
#include <comphelper/processfactory.hxx>

#include <doc.hrc>

#include <vcl/builderfactory.hxx>
using namespace ::com::sun::star;

RecentEmojiView::RecentEmojiView (vcl::Window *pParent)
    : ThumbnailView(pParent, WB_TABSTOP | WB_VSCROLL)
{
    uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );
    OUString sFontName(officecfg::Office::Common::Misc::EmojiFont::get(xContext));
    vcl::Font aFont = GetControlFont();
    aFont.SetFamilyName( sFontName );
    SetControlFont(aFont);
}

VCL_BUILDER_FACTORY(RecentEmojiView)

RecentEmojiView::~RecentEmojiView()
{
    disposeOnce();
}

void RecentEmojiView::Populate()
{
    maRecentEmoji.clear();

    for (ThumbnailViewItem* pItem : mItemList)
        delete pItem;

    //retrieve recent emoji list
    css::uno::Sequence< OUString > rRecentEmojiList( officecfg::Office::Common::RecentEmoji::RecentEmojiCodes::get() );
    for (int i = 0; i < rRecentEmojiList.getLength(); ++i)
    {
        maRecentEmoji.push_back(OUString(rRecentEmojiList[i]));
    }

    for( int i = 0;i < rRecentEmojiList.getLength();i++ )
    {
        EmojiViewItem *pItem = new EmojiViewItem(*this, getNextItemId());

        pItem->maTitle = OUString(rRecentEmojiList[i]);
        pItem->setHelpText(pItem->maTitle);

        ThumbnailView::AppendItem(pItem);

        CalculateItemPositions();
    }
}

std::deque<OUString> RecentEmojiView::getRecentEmojiList()
{
    return maRecentEmoji;
}

void RecentEmojiView::ApplySettings(vcl::RenderContext& rRenderContext)
{
    ThumbnailView::ApplySettings(rRenderContext);
    mpItemAttrs->aFontSize.setX(ITEM_MAX_WIDTH - 2*ITEM_PADDING);
    mpItemAttrs->aFontSize.setY(ITEM_MAX_HEIGHT - 2*ITEM_PADDING);
}

void RecentEmojiView::MouseButtonDown( const MouseEvent& rMEvt )
{
    GrabFocus();

    if (rMEvt.IsLeft())
    {
        size_t nPos = ImplGetItem(rMEvt.GetPosPixel());
        ThumbnailViewItem* pItem = ImplGetItem(nPos);

        if(pItem)
            maInsertEmojiHdl.Call(pItem);
    }
}

void RecentEmojiView::KeyInput( const KeyEvent& rKEvt )
{
    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();

    if(aKeyCode == ( KEY_MOD1 | KEY_A ) )
    {
        for (ThumbnailViewItem* pItem : mFilteredItemList)
        {
            if (!pItem->isSelected())
            {
                pItem->setSelection(true);
                maItemStateHdl.Call(pItem);
            }
        }

        if (IsReallyVisible() && IsUpdateMode())
            Invalidate();
        return;
    }

    ThumbnailView::KeyInput(rKEvt);
}

void RecentEmojiView::setInsertEmojiHdl(const Link<ThumbnailViewItem*, void> &rLink)
{
    maInsertEmojiHdl = rLink;
}

void RecentEmojiView::AppendItem(const OUString &rTitle, const OUString &rCategory, const OUString &rName)
{
    EmojiViewItem *pItem = new EmojiViewItem(*this, getNextItemId());

    pItem->maTitle = rTitle;
    pItem->setCategory(rCategory);
    pItem->setHelpText(rName);

    ThumbnailView::AppendItem(pItem);

    CalculateItemPositions();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
