/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include <WopiTestServer.hpp>
#include <common/FileUtil.hpp>
#include <common/JailUtil.hpp>
#include <common/Thumbnail.hpp>
#include <common/Unit.hpp>
#include <helpers.hpp>
#include <lokassert.hpp>

#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Util/LayeredConfiguration.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

/// After a save, a PNG thumbnail of the document lies next to the copy that is uploaded, one for
/// each size the host listed, and after a Save As next to the new file. Each image fits in its
/// box and keeps the shape of the page, or fills the box for a spreadsheet.
class UnitWOPIThumbnailRender : public WopiTestServer
{
public:
    using Shape = helpers::ThumbnailShape;

private:
    STATE_ENUM(Phase, Load, WaitSave, WaitSaveAs, Done) _phase;

    const std::string _filename;
    const Shape _shape;
    const bool _backgroundSave;

    /// The sizes the host asks for. The second is wider than high, so it tells a portrait page
    /// from a landscape one as well as the square box does.
    static constexpr std::pair<int, int> Sizes[] = { { 256, 256 }, { 100, 60 } };

public:
    UnitWOPIThumbnailRender(const std::string& name, const std::string& filename, Shape shape,
                            bool backgroundSave)
        : WopiTestServer("UnitWOPIThumbnailRender_" + name, filename)
        , _phase(Phase::Load)
        , _filename(filename)
        , _shape(shape)
        , _backgroundSave(backgroundSave)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);

        config.setBool("per_document.background_manualsave", _backgroundSave);
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& /*request*/,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        // The document in the jail is named after the file, so the thumbnails are too.
        fileInfo->set("BaseFileName", _filename);

        Poco::JSON::Array::Ptr sizes = new Poco::JSON::Array();
        for (const auto& [width, height] : Sizes)
            sizes->add(std::to_string(width) + 'x' + std::to_string(height));
        fileInfo->set("ThumbnailSizes", sizes);
    }

    /// Returns the path of the file with the given name below the documents directory of the
    /// jail, or an empty string when there is none.
    std::string findInJail(const std::string& name)
    {
        const std::string docsDir = FileUtil::buildLocalPathToJail(
            JailUtil::isMountNamespacesEnabled(), getJailRoot(), "/tmp/user/docs");
        for (const auto& entry : std::filesystem::recursive_directory_iterator(docsDir))
        {
            if (entry.is_regular_file() && entry.path().filename() == name)
                return entry.path().string();
        }

        return std::string();
    }

    /// Checks the thumbnails of the file with the given base name in the jail.
    void checkThumbnails(const std::string& baseName)
    {
        for (const auto& [boxWidth, boxHeight] : Sizes)
        {
            const std::string name = Thumbnail::filePath(baseName, boxWidth, boxHeight);
            const std::string path = findInJail(name);
            LOK_ASSERT_MESSAGE("Missing thumbnail " + name, !path.empty());

            std::ifstream file(path, std::ios::binary);
            const std::string png((std::istreambuf_iterator<char>(file)),
                                  std::istreambuf_iterator<char>());
            TST_LOG("Checking the thumbnail " << name);
            helpers::assertThumbnailShape(png, boxWidth, boxHeight, _shape, testname);
        }
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');
        LOK_ASSERT_STATE(_phase, Phase::WaitSave);

        WSD_CMD("save dontTerminateEdit=1 dontSaveIfUnmodified=0");
        return true;
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        LOK_ASSERT_STATE(_phase, Phase::WaitSave);

        // The upload is of the copy with the upload suffix, and the thumbnails are named after it.
        checkThumbnails(_filename + TO_UPLOAD_SUFFIX);

        TRANSITION_STATE(_phase, Phase::WaitSaveAs);
        WSD_CMD("saveas url=wopi:///path/renamed." + FileUtil::extractFileExtension(_filename));

        return nullptr;
    }

    void assertPutRelativeFileRequest(const Poco::Net::HTTPRequest& /*request*/) override
    {
        LOK_ASSERT_STATE(_phase, Phase::WaitSaveAs);

        // The new file is uploaded from its own directory, with its thumbnails next to it.
        checkThumbnails("renamed." + FileUtil::extractFileExtension(_filename));

        TRANSITION_STATE(_phase, Phase::Done);
        passTest("The thumbnails of the save and of the Save As are on disk");
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitSave);

                initWebsocket("/wopi/files/0?access_token=anything");
                WSD_CMD("load url=" + getWopiSrc());
                break;
            }
            case Phase::WaitSave:
            case Phase::WaitSaveAs:
            case Phase::Done:
                break;
        }
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    using Shape = UnitWOPIThumbnailRender::Shape;
    return new UnitBase*[6]{
        new UnitWOPIThumbnailRender("Writer", "hello.odt", Shape::Portrait,
                                    /*backgroundSave=*/true),
        new UnitWOPIThumbnailRender("Calc", "hello.ods", Shape::Filled, /*backgroundSave=*/true),
        new UnitWOPIThumbnailRender("Impress", "hello.odp", Shape::Landscape,
                                    /*backgroundSave=*/true),
        new UnitWOPIThumbnailRender("Draw", "hello.odg", Shape::Portrait, /*backgroundSave=*/true),
        new UnitWOPIThumbnailRender("WriterForeground", "hello.odt", Shape::Portrait,
                                    /*backgroundSave=*/false),
        nullptr
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
