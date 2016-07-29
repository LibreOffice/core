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
#include <sfx2/templatelocalview.hxx>
#include <sfx2/sfxresid.hxx>
#include <tools/urlobj.hxx>
#include <vcl/layout.hxx>
#include <rtl/bootstrap.hxx>
#include <config_folders.h>
#include <officecfg/Office/Common.hxx>
#include <comphelper/processfactory.hxx>

#include <orcus/json_document_tree.hpp>
#include <orcus/config.hpp>
#include <orcus/pstring.hpp>
#include <string>
#include <fstream>
#include <streambuf>

#include "../doc/doc.hrc"

#include <vcl/builderfactory.hxx>
using namespace ::com::sun::star;

bool ViewFilter_Category::isFilteredCategory(FILTER_CATEGORY filter, const OUString &rCategory)
{
    bool bRet = true;

    if (filter == FILTER_CATEGORY::PEOPLE)
        bRet = rCategory.match("people");
    else if (filter == FILTER_CATEGORY::NATURE)
        bRet = rCategory.match("nature");
    else if (filter == FILTER_CATEGORY::FOOD)
        bRet = rCategory.match("food");
    else if (filter == FILTER_CATEGORY::ACTIVITY)
        bRet = rCategory.match("activity");
    else if (filter == FILTER_CATEGORY::TRAVEL)
        bRet = rCategory.match("travel");
    else if (filter == FILTER_CATEGORY::OBJECTS)
        bRet = rCategory.match("objects");
    else if (filter == FILTER_CATEGORY::SYMBOLS)
        bRet = rCategory.match("symbols");
    else if (filter == FILTER_CATEGORY::FLAGS)
        bRet = rCategory.match("flags");
    else if (filter == FILTER_CATEGORY::UNICODE9)
        bRet = rCategory.match("unicode9");

    return bRet;
}

bool ViewFilter_Category::operator () (const ThumbnailViewItem *pItem)
{
    const EmojiViewItem *pViewItem = dynamic_cast<const EmojiViewItem*>(pItem);
    if (pViewItem)
        return isFilteredCategory(mCategory, pViewItem->getCategory());

    return true;
}

EmojiView::EmojiView (vcl::Window *pParent, WinBits nWinStyle)
    : ThumbnailView(pParent,nWinStyle)
{
    //locate json data file
    OUString sPath("$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/emojiconfig/emoji.json");
    rtl::Bootstrap::expandMacros(sPath);
    std::string strPath = OUStringToOString(sPath.copy(strlen("file://")), RTL_TEXTENCODING_UTF8).getStr();

    std::ifstream file(strPath);
    if(!file.is_open())
        return;

    msJSONData = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if(msJSONData.empty())
        return;

    uno::Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );
    OUString sFontName(officecfg::Office::Common::Misc::EmojiFont::get(xContext));
    vcl::Font aFont = GetControlFont();
    aFont.SetFamilyName( sFontName );
    SetControlFont(aFont);
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
    aEmojiInfo.load(msJSONData, config);

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
                if(paramVal == "unicode")
                {
                    sTitle = rtl::OStringToOUString(OString( prop.string_value().get(), prop.string_value().size() ), RTL_TEXTENCODING_UTF8);
                }
                else if(paramVal == "category")
                {
                    sCategory = rtl::OStringToOUString(OString( prop.string_value().get(), prop.string_value().size() ), RTL_TEXTENCODING_UTF8);
                }
            }

            //TODO: Check whether the glyph is present in the font file
            //If the glyph is present, Call EmojiView::AppendItem() to populate each template as it is parsed
            AppendItem(sTitle, sCategory);
        }
    }
}

void EmojiView::MouseButtonDown( const MouseEvent& rMEvt )
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
