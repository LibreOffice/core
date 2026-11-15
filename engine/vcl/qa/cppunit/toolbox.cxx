/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>

#include <boost/property_tree/json_parser.hpp>
#include <tools/json_writer.hxx>
#include <vcl/image.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/wrkwin.hxx>

#include <sstream>

namespace
{
class ToolBoxTest : public test::BootstrapFixture
{
public:
    ToolBoxTest()
        : BootstrapFixture(true, false)
    {
    }
};

/// A property the JSON of the toolbox gives for its only item.
std::string dumpedItemProperty(ToolBox& rToolBox, const char* pName)
{
    tools::JsonWriter aJsonWriter;
    rToolBox.DumpAsPropertyTree(aJsonWriter);
    std::stringstream aStream{ std::string(aJsonWriter.finishAndGetAsOString()) };
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);
    return aTree.get_child("children").front().second.get<std::string>(pName, "");
}

/// The icon name the JSON of the toolbox gives for its only item.
std::string dumpedIconName(ToolBox& rToolBox) { return dumpedItemProperty(rToolBox, "icon"); }

CPPUNIT_TEST_FIXTURE(ToolBoxTest, testIconOfImageSetByCode)
{
    // Given a toolbox item with the icon its .ui file names:
    ScopedVclPtrInstance<WorkWindow> xWindow(nullptr, WB_APP | WB_STDWORK);
    ScopedVclPtrInstance<ToolBox> xToolBox(xWindow.get());
    const ToolBoxItemId nId(1);
    xToolBox->InsertItem(nId, Image(StockImage::Yes, u"sc/res/dropurl.png"_ustr), u"Mode"_ustr);
    xToolBox->SetItemIconName(nId, u"sc/res/dropurl.png"_ustr);
    CPPUNIT_ASSERT_EQUAL(std::string("lc_dropurl.svg"), dumpedIconName(*xToolBox));

    // When code shows another icon, then that is the icon the client is told of. Without the fix
    // in place, the client was told of the icon of the .ui file, and showed it, or the bitmap.
    xToolBox->SetItemImage(nId, Image(StockImage::Yes, u"sc/res/dropcopy.png"_ustr));
    CPPUNIT_ASSERT_EQUAL(std::string("lc_dropcopy.svg"), dumpedIconName(*xToolBox));
}

CPPUNIT_TEST_FIXTURE(ToolBoxTest, testIconPathTellsModulesApart)
{
    // Given a toolbox item showing the paragraph styles icon of Writer:
    ScopedVclPtrInstance<WorkWindow> xWindow(nullptr, WB_APP | WB_STDWORK);
    ScopedVclPtrInstance<ToolBox> xToolBox(xWindow.get());
    const ToolBoxItemId nId(1);
    xToolBox->InsertItem(nId, Image(StockImage::Yes, u"sw/res/sf01.png"_ustr), u"Styles"_ustr);

    // Then the client gets its path, as the icon name alone is the one of the cell styles icon
    // of Calc as well.
    CPPUNIT_ASSERT_EQUAL(std::string("lc_sf01.svg"), dumpedIconName(*xToolBox));
    CPPUNIT_ASSERT_EQUAL(std::string("sw/res/sf01.png"), dumpedItemProperty(*xToolBox, "iconpath"));
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
