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
    msJSONQueryOutput =(const char*) "{\"count\": 1, \"pagesize\": 25, \"items\": [{\"positive_ratings\": 0, \"total_comments\": 0,"
"        \"getCategories\": null, \"author_name\": null, \"path\":\"/liboexttempsite/templates/personal-budget-template\", \"commentators\": [],"
"       \"exclude_from_nav\": false, \"id\": \"personal-budget-template\", \"uid\": \"7cb547807f0e4d5c8bff56e89c8dd97f\", \"parent_id\": \"templates\","
"        \"end\": null, \"api_url\": \"http://vm141.documentfoundation.org:9201/liboexttempsite/@@API/plone/api/1.0/get/7cb547807f0e4d5c8bff56e89c8dd97f\","
"        \"title\": \"Personal Budget Template\", \"is_folderish\": true, \"sync_uid\": null, \"start\": null, \"location\": null, \"parent_url\":"
"        \"http://vm141.documentfoundation.org:9201/liboexttempsite/@@API/plone/api/1.0/get/a3a627d548c3432dba18376986e47a5c\", \"review_state\": \"published\","
"        \"description\":\"The Personal Budget Template is a spreadsheet template to help track your budget for the year and is available in three currencies, USD,"
"        EUR and GBP. The template can be modified to accommodate other currencies as needed.\", \"Language\": null, \"tags\": [], \"portal_type\":"
"        \"tdf.templateuploadcenter.tupproject\", \"expires\": \"2499-12-31T00:00:00+00:00\", \"TranslationGroup\": null, \"last_comment_date\": null,"
"        \"parent_uid\": \"a3a627d548c3432dba18376986e47a5c\", \"effective\": \"2016-04-05T17:22:00+00:00\", \"created\": \"2016-04-05T17:14:37+00:00\","
"        \"url\": \"http://vm141.documentfoundation.org:9201/liboexttempsite/templates/personal-budget-template\", \"author\": \"andreasma\", \"modified\":"
"        \"2016-04-05T17:38:22+00:00\", \"in_response_to\": null},{\"positive_ratings\": 0, \"total_comments\": 0,"
"        \"getCategories\": null, \"author_name\": null, \"path\":\"/liboexttempsite/templates/personal-budget-template\", \"commentators\": [],"
"       \"exclude_from_nav\": false, \"id\": \"gsoc-template\", \"uid\": \"7cb547807f0e4d5c8bff56e89c8dd97f\", \"parent_id\": \"templates\","
"        \"end\": null, \"api_url\": \"http://vm141.documentfoundation.org:9201/liboexttempsite/@@API/plone/api/1.0/get/7cb547807f0e4d5c8bff56e89c8dd97f\","
"        \"title\": \"Akshay's GSoC Template\", \"is_folderish\": true, \"sync_uid\": null, \"start\": null, \"location\": null, \"parent_url\":"
"        \"http://vm141.documentfoundation.org:9201/liboexttempsite/@@API/plone/api/1.0/get/a3a627d548c3432dba18376986e47a5c\", \"review_state\": \"published\","
"        \"description\":\"The Personal Budget Template is a spreadsheet template to help track your budget for the year and is available in three currencies, USD,"
"        EUR and GBP. The template can be modified to accommodate other currencies as needed.\", \"Language\": null, \"tags\": [], \"portal_type\":"
"        \"tdf.templateuploadcenter.tupproject\", \"expires\": \"2499-12-31T00:00:00+00:00\", \"TranslationGroup\": null, \"last_comment_date\": null,"
"        \"parent_uid\": \"a3a627d548c3432dba18376986e47a5c\", \"effective\": \"2016-04-05T17:22:00+00:00\", \"created\": \"2016-04-05T17:14:37+00:00\","
"        \"url\": \"http://vm141.documentfoundation.org:9201/liboexttempsite/templates/personal-budget-template\", \"author\": \"Akshay\", \"modified\":"
"        \"2016-04-05T17:38:22+00:00\", \"in_response_to\": null}], \"page\": 1, \"_runtime\": 0.6190857887268066, \"next\": null, \"pages\": 1,"
"        \"previous\": null}";
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

    orcus::json_document_tree aTemplatesInfo;

    // Load JSON string into a document tree.
    aTemplatesInfo.load(msJSONQueryOutput, config);

    node root = aTemplatesInfo.get_document_root();
    std::vector<orcus::pstring> keys = root.keys();

    for (auto it = keys.begin(), ite = keys.end(); it != ite; ++it)
    {
        orcus::pstring key = *it;
        node value = root.child(key);

        //get the array which contains information of templates
        if(value.type() == orcus::json_node_t::array)
        {
            for (size_t i = 0; i < value.child_count(); ++i)
            {
                //iterate each element to get the keys
                node aTemplateModel = value.child(i);
                std::vector<orcus::pstring> aTemplateParams = aTemplateModel.keys();
                OUString sTitle, sCategory;

                for (auto templateIter = aTemplateParams.begin(); templateIter != aTemplateParams.end(); ++templateIter)
                {
                    orcus::pstring templateVal = *templateIter;
                    node prop = aTemplateModel.child(templateVal);

                    //get values of parameters in AppendItem() function
                    if(templateVal == (const char*) "unicode")
                    {
                        sTitle = OUString::createFromAscii(prop.string_value().get());
                    }
                    else if(templateVal == (const char*)"category")
                    {
                        sCategory = OUString("_default");
                    }
                }

                //Call EmojiView::AppendItem() to populate each template as it is parsed
                AppendItem(sTitle, sCategory);
            }
        }
    }
}

void EmojiView::MouseButtonDown( const MouseEvent& /*rMEvt*/ )
{
    GrabFocus();
    return;
    //ThumbnailView::MouseButtonDown(rMEvt);
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
