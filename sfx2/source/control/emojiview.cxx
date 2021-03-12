/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <emojiview.hxx>
#include <emojiviewitem.hxx>
#include <rtl/bootstrap.hxx>
#include <sal/log.hxx>
#include <config_folders.h>
#include <officecfg/Office/Common.hxx>
#include <comphelper/processfactory.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weldutils.hxx>

#include <orcus/json_document_tree.hpp>
#include <orcus/config.hpp>
#include <orcus/pstring.hpp>
#include <string>
#include <string_view>
#include <fstream>

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

EmojiView::EmojiView(std::unique_ptr<weld::ScrolledWindow> xWindow)
    : ThumbnailView(std::move(xWindow), nullptr)
{
    // locate json data file
    OUString sPath("$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/emojiconfig/emoji.json");
    rtl::Bootstrap::expandMacros(sPath);
    std::string strPath = OUStringToOString(sPath.subView(strlen("file://")), RTL_TEXTENCODING_UTF8).getStr();

    std::ifstream file(strPath);
    if(!file.is_open())
        return;

    msJSONData = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if(msJSONData.empty())
        return;
}

void EmojiView::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    ThumbnailView::SetDrawingArea(pDrawingArea);

    uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());
    OUString sFontName(officecfg::Office::Common::Misc::EmojiFont::get(xContext));
    vcl::Font aFont = pDrawingArea->get_font();
    aFont.SetFamilyName(sFontName);
    OutputDevice& rDevice = pDrawingArea->get_ref_device();
    weld::SetPointFont(rDevice, aFont);

    mpItemAttrs->aFontSize.setX(ITEM_MAX_WIDTH - 2*ITEM_PADDING);
    mpItemAttrs->aFontSize.setY(ITEM_MAX_HEIGHT - 2*ITEM_PADDING);
}

EmojiView::~EmojiView()
{
}

void EmojiView::Populate()
{
    if (msJSONData.empty())
    {
        SAL_WARN("sfx", "Emoji config data is empty");
        return;
    }

    // Populate view using the orcus json parser
    using node = orcus::json::node;

    // default json config
    orcus::json_config config;

    orcus::json::document_tree aEmojiInfo;

    // Load JSON string into a document tree.
    aEmojiInfo.load(msJSONData, config);

    node root = aEmojiInfo.get_document_root();
    std::vector<orcus::pstring> keys = root.keys();

    for (auto const& key : keys)
    {
        node value = root.child(key);

        if(value.type() == orcus::json::node_t::object)
        {
            // iterate each element to get the keys
            std::vector<orcus::pstring> aEmojiParams = value.keys();
            OUString sTitle, sCategory, sName;
            bool bDuplicate = false;

            for (auto const& emojiParam : aEmojiParams)
            {
                node prop = value.child(emojiParam);

                // get values of parameters in AppendItem() function
                if(emojiParam == "unicode")
                {
                    sTitle = OStringToOUString(std::string_view( prop.string_value().get(), prop.string_value().size() ), RTL_TEXTENCODING_UTF8);
                }
                else if(emojiParam == "category")
                {
                    sCategory = OStringToOUString(std::string_view( prop.string_value().get(), prop.string_value().size() ), RTL_TEXTENCODING_UTF8);
                }
                else if(emojiParam == "name")
                {
                    sName = OStringToOUString(std::string_view( prop.string_value().get(), prop.string_value().size() ), RTL_TEXTENCODING_UTF8);
                }
                else if(emojiParam == "duplicate")
                {
                    bDuplicate = true;
                }
            }

            // Don't append if a duplicate emoji
            if(!bDuplicate)
            {
                AppendItem(sTitle, sCategory, sName);
            }
        }
    }
}

bool EmojiView::MouseButtonDown( const MouseEvent& rMEvt )
{
    GrabFocus();

    if (rMEvt.IsLeft())
    {
        size_t nPos = ImplGetItem(rMEvt.GetPosPixel());
        ThumbnailViewItem* pItem = ImplGetItem(nPos);

        if(pItem)
            maInsertEmojiHdl.Call(pItem);
    }

    return true;
}

bool EmojiView::KeyInput( const KeyEvent& rKEvt )
{
    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();

    if(aKeyCode == ( KEY_MOD1 | KEY_A ) )
    {
        for (ThumbnailViewItem* pItem : mFilteredItemList)
        {
            if (!pItem->isSelected())
            {
                pItem->setSelection(true);
            }
        }

        if (IsReallyVisible() && IsUpdateMode())
            Invalidate();
        return true;
    }

    return ThumbnailView::KeyInput(rKEvt);
}

void EmojiView::setInsertEmojiHdl(const Link<ThumbnailViewItem*, void> &rLink)
{
    maInsertEmojiHdl = rLink;
}

void EmojiView::AppendItem(const OUString &rTitle, const OUString &rCategory, const OUString &rName)
{
    std::unique_ptr<EmojiViewItem> pItem(new EmojiViewItem(*this, getNextItemId()));

    pItem->maTitle = rTitle;
    pItem->setCategory(rCategory);
    pItem->setHelpText(rName);

    ThumbnailView::AppendItem(std::move(pItem));

    CalculateItemPositions();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
