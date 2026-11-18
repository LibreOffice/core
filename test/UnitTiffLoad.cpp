/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Unit test for TIFF image loading functionality.
 */

#include <config.h>

#include <memory>
#include <string>

#include <Poco/URI.h>
#include <test/lokassert.hpp>

#include <Unit.hpp>
#include <common/Util.hpp>
#include <helpers.hpp>

/// TIFF load testcase.
class UnitTiffLoad : public UnitWSD
{
public:
    UnitTiffLoad();

    void invokeWSDTest() override;
};

UnitTiffLoad::UnitTiffLoad()
    : UnitWSD("UnitTiffLoad")
{
}

void UnitTiffLoad::invokeWSDTest()
{
    // Load a document which has a TIFF image in it.
    std::string documentPath;
    std::string documentURL;
    helpers::getDocumentPathAndURL("tiff.odt", documentPath, documentURL, testname);

    std::shared_ptr<SocketPoll> socketPoll = std::make_shared<SocketPoll>("TiffLoadPoll");
    socketPoll->startThread();

    std::shared_ptr<http::WebSocketSession> socket = helpers::loadDocAndGetSession(
        socketPoll, Poco::URI(helpers::getTestServerURI()), documentURL, testname);

    // Select the image.
    helpers::sendTextFrame(socket, "uno .uno:JumpToNextFrame", testname);
    helpers::sendTextFrame(socket, "rendershapeselection mimetype=image/svg+xml", testname);

    // Make sure we can get an SVG representation of the image; this failed as the TIFF import was
    // broken.
    const std::string content
        = helpers::assertResponseString(socket, "shapeselectioncontent:", testname);
    LOK_ASSERT(content.starts_with("shapeselectioncontent:\n"));

    exitTest(TestResult::Ok);
}

UnitBase* unit_create_wsd(void) { return new UnitTiffLoad(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
