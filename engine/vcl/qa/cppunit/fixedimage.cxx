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
#include <vcl/toolkit/fixed.hxx>
#include <vcl/image.hxx>
#include <vcl/wrkwin.hxx>

#include <sstream>

namespace
{
class FixedImageTest : public test::BootstrapFixture
{
public:
    FixedImageTest()
        : BootstrapFixture(true, false)
    {
    }
};

CPPUNIT_TEST_FIXTURE(FixedImageTest, testIconPathOfThemeImage)
{
    // Given an image of the icon theme, as the paragraph sidebar shows next to its spacings:
    ScopedVclPtrInstance<WorkWindow> xWindow(nullptr, WB_APP | WB_STDWORK);
    ScopedVclPtrInstance<FixedImage> xImage(xWindow.get());
    xImage->SetImage(Image(StockImage::Yes, u"svx/res/symphony/spacing1.png"_ustr));

    // When it is sent to the client:
    tools::JsonWriter aJsonWriter;
    xImage->DumpAsPropertyTree(aJsonWriter);
    std::stringstream aStream{ std::string(aJsonWriter.finishAndGetAsOString()) };
    boost::property_tree::ptree aTree;
    boost::property_tree::read_json(aStream, aTree);

    // Then the client gets its path, to show an icon of its own instead of the bitmap. Without
    // the fix in place, only the bitmap was sent.
    CPPUNIT_ASSERT_EQUAL(std::string("svx/res/symphony/spacing1.png"),
                         aTree.get<std::string>("iconpath", ""));
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
