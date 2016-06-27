/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/emojiview.hxx>
#include <sfx2/emojiviewitem.hxx>
#include <sfx2/templateabstractview.hxx>
#include <sfx2/sfxresid.hxx>
#include <tools/urlobj.hxx>
#include <vcl/layout.hxx>

#include <orcus/json_document_tree.hpp>
#include <orcus/config.hpp>
#include <orcus/pstring.hpp>

#include "../doc/doc.hrc"

#include <vcl/builderfactory.hxx>

EmojiView::EmojiView (vcl::Window *pParent, WinBits nWinStyle)
    : ThumbnailView(pParent,nWinStyle)
{
    //Declare sample query
    msJSONInput =(const char*) "{\"grinning\":{\"unicode\":\"1f600\",\"unicode_alternates\":\"\",\"name\":\"grinning face\",\"shortname\":\":grinning:\",\"category\":\"people\",\"emoji_order\":\"1\",\"aliases\":[],\"aliases_ascii\":[],\"keywords\":[\"happy\",\"smiley\",\"emotion\",\"emotion\"]},"
"   \"grimacing\":{\"unicode\":\"1f62c\",\"unicode_alternates\":\"\",\"name\":\"grimacing face\",\"shortname\":\":grimacing:\",\"category\":\"people\",\"emoji_order\":\"2\",\"aliases\":[],\"aliases_ascii\":[],\"keywords\":[\"silly\",\"smiley\",\"emotion\",\"emotion\",\"selfie\",\"selfie\"]},"
"   \"grin\":{\"unicode\":\"1f601\",\"unicode_alternates\":\"\",\"name\":\"grinning face with smiling eyes\",\"shortname\":\":grin:\",\"category\":\"people\",\"emoji_order\":\"3\",\"aliases\":[],\"aliases_ascii\":[],\"keywords\":[\"happy\",\"silly\",\"smiley\",\"emotion\",\"emotion\",\"good\",\"good\",\"selfie\",\"selfie\"]},"
"   \"joy\":{\"unicode\":\"1f602\",\"unicode_alternates\":\"\",\"name\":\"face with tears of joy\",\"shortname\":\":joy:\",\"category\":\"people\",\"emoji_order\":\"4\",\"aliases\":[],\"aliases_ascii\":[\":')\",\":'-)\"],\"keywords\":[\"happy\",\"silly\",\"smiley\",\"cry\",\"laugh\",\"laugh\",\"emotion\",\"emotion\",\"sarcastic\",\"sarcastic\"]}}";
}

VCL_BUILDER_FACTORY(EmojiView)

EmojiView::~EmojiView()
{
    disposeOnce();
}

void EmojiView::Populate ()
{
    //TODO::populate view using the orcus json parser
    using node = orcus::json_document_tree::node;

    //default json config
    orcus::json_config config;

    orcus::json_document_tree aEmojiInfo;

    // Load JSON string into a document tree.
    aEmojiInfo.load(msJSONInput, config);

    node root = aEmojiInfo.get_document_root();
    std::vector<orcus::pstring> keys = root.keys();

    for (auto it = keys.begin(), ite = keys.end(); it != ite; ++it)
    {
        orcus::pstring key = *it;
        node value = root.child(key);

        if(value.type() == orcus::json_node_t::object)
        {
            //iterate each element to get the keys
            std::vector<orcus::pstring> aEmojiParams = value.keys();
            OUString sTitle, sCategory;

            for (auto paramIter = aEmojiParams.begin(); paramIter != aEmojiParams.end(); ++paramIter)
            {
                orcus::pstring paramVal = *paramIter;
                node prop = value.child(paramVal);

                //get values of parameters in AppendItem() function
                if(paramVal == (const char*) "unicode")
                {
                    sTitle = OUString::createFromAscii(prop.string_value().get());
                }
                else if(paramVal == (const char*)"category")
                {
                    sCategory = OUString::createFromAscii(prop.string_value().get());
                }
            }

            //Call EmojiView::AppendItem() to populate each template as it is parsed
            AppendItem(sTitle, sCategory);
        }
    }
}

void EmojiView::MouseButtonDown( const MouseEvent& rMEvt )
{
    GrabFocus();
    ThumbnailView::MouseButtonDown(rMEvt);
}

void EmojiView::KeyInput( const KeyEvent& rKEvt )
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

void EmojiView::setInsertEmojiHdl(const Link<ThumbnailViewItem*, void> &rLink)
{
    maInsertEmojiHdl = rLink;
}

void EmojiView::AppendItem(const OUString &rTitle, const OUString &rCategory)
{
    EmojiViewItem *pItem = new EmojiViewItem(*this, getNextItemId());

    pItem->maTitle = rTitle;
    pItem->setCategory(rCategory);

    ThumbnailView::AppendItem(pItem);

    CalculateItemPositions();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
