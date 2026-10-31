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
#include <common/Unit.hpp>
#include <helpers.hpp>
#include <lokassert.hpp>
#include <net/HttpHelper.hpp>

#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/URI.h>
#include <Poco/Util/LayeredConfiguration.h>

#include <cstddef>
#include <iterator>
#include <string>
#include <utility>

/// When the host lists thumbnail sizes, a save uploads the document together with one PNG per
/// size in a multipart body, and so does a Save As. A save of an unmodified document uploads
/// nothing. After a failed upload, the retry carries the thumbnails too. Without sizes, with a
/// password-protected document or with a watermark, the upload is the plain document, as it
/// always was.
class UnitWOPIThumbnails : public WopiTestServer
{
public:
    using Shape = helpers::ThumbnailShape;

    /// What the case checks.
    enum class Mode
    {
        Thumbnails, ///< A save and a Save As upload thumbnails with the document.
        Template, ///< The upload right after loading a template carries thumbnails.
        Retry, ///< The first upload fails, and its retry after an unmodified save has thumbnails.
        NoSizes, ///< The host lists no sizes, so the upload is plain.
        Password, ///< The document is password protected, so the upload is plain.
        Watermark, ///< The host sets a watermark, so the upload is plain.
        NotesView ///< The view shows the notes pages of a presentation, so the upload is plain.
    };

private:
    STATE_ENUM(Phase, Load, WaitModified, WaitFirstUpload, WaitRetry, WaitUnmodifiedSave,
               WaitSaveAs, Done)
    _phase;

    const Mode _mode;
    const std::string _filename;
    /// The ODF mimetype the uploaded document has to carry.
    const std::string _mimetype;
    const Shape _shape;
    const bool _backgroundSave;

    /// The sizes the host asks for. The second is wider than high, so it tells a portrait page
    /// from a landscape one as well as the square box does.
    static constexpr std::pair<int, int> Sizes[] = { { 256, 256 }, { 100, 60 } };

    static constexpr const char* TemplateFile = "test.ott";

public:
    UnitWOPIThumbnails(const std::string& name, Mode mode, const std::string& filename,
                       const std::string& mimetype, Shape shape, bool backgroundSave)
        : WopiTestServer("UnitWOPIThumbnails_" + name, filename)
        , _phase(Phase::Load)
        , _mode(mode)
        , _filename(filename)
        , _mimetype(mimetype)
        , _shape(shape)
        , _backgroundSave(backgroundSave)
    {
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        WopiTestServer::configure(config);

        config.setBool("per_document.background_autosave", _backgroundSave);
        config.setBool("per_document.background_manualsave", _backgroundSave);
    }

    void configCheckFileInfo(const Poco::Net::HTTPRequest& /*request*/,
                             Poco::JSON::Object::Ptr& fileInfo) override
    {
        fileInfo->set("BaseFileName", _filename);

        if (_mode != Mode::NoSizes)
        {
            Poco::JSON::Array::Ptr sizes = new Poco::JSON::Array();
            for (const auto& [width, height] : Sizes)
                sizes->add(std::to_string(width) + 'x' + std::to_string(height));
            fileInfo->set("ThumbnailSizes", sizes);
        }

        if (_mode == Mode::Template)
        {
            fileInfo->set("TemplateSource", helpers::getTestServerURI() + '/' + TemplateFile +
                                                "?testname=" + getTestname());
        }

        if (_mode == Mode::Watermark)
            fileInfo->set("WatermarkText", "Thumbnails");
    }

    bool handleHttpGetRequest(const Poco::Net::HTTPRequest& request,
                              const std::shared_ptr<StreamSocket>& socket) override
    {
        const Poco::URI uriReq(request.getURI());
        if (uriReq.getPath() == std::string("/") + TemplateFile)
        {
            TST_LOG("FakeWOPIHost: Serving the template " << uriReq.getPath());
            http::Response response(http::StatusCode::OK);
            HttpHelper::sendFileAndShutdown(socket, TDOC "/" + std::string(TemplateFile), response);
            return true;
        }

        return WopiTestServer::handleHttpGetRequest(request, socket);
    }

    /// Asserts that the uploaded document is an ODF package of the expected kind.
    void checkDocument()
    {
        const std::string& document = getUploadedDocument();
        LOK_ASSERT_MESSAGE("The uploaded document is not a zip package",
                           document.starts_with("PK"));
        // The first entry of an ODF package is the mimetype, stored as it is.
        LOK_ASSERT_MESSAGE("The uploaded document is not of type " + _mimetype,
                           document.find("mimetype" + _mimetype) != std::string::npos);
    }

    /// Asserts that the upload carries the document and one thumbnail per size.
    void checkMultipartUpload(const Poco::Net::HTTPRequest& request)
    {
        LOK_ASSERT_EQUAL_STR("true", request.get("X-COOL-WOPI-HasThumbnails", std::string()));
        LOK_ASSERT_MESSAGE("Content-Type: " + request.getContentType(),
                           request.getContentType().starts_with("multipart/form-data; boundary="));

        checkDocument();

        const auto& parts = getUploadedParts();
        LOK_ASSERT_EQUAL(std::size(Sizes), parts.size());
        for (const auto& [boxWidth, boxHeight] : Sizes)
        {
            const std::string name =
                "thumbnail-" + std::to_string(boxWidth) + 'x' + std::to_string(boxHeight);
            const auto it = parts.find(name);
            LOK_ASSERT_MESSAGE("Missing part " + name, it != parts.end());
            LOK_ASSERT_EQUAL_STR("image/png", it->second.contentType);
            helpers::assertThumbnailShape(it->second.content, boxWidth, boxHeight, _shape,
                                          testname);
        }
    }

    /// Asserts that the upload is the plain document and nothing else.
    void checkPlainUpload(const Poco::Net::HTTPRequest& request)
    {
        LOK_ASSERT_MESSAGE("Unexpected X-COOL-WOPI-HasThumbnails header",
                           !request.has("X-COOL-WOPI-HasThumbnails"));
        LOK_ASSERT_EQUAL_STR("application/octet-stream", request.getContentType());
        LOK_ASSERT_EQUAL(0UL, getUploadedParts().size());
        LOK_ASSERT_EQUAL(static_cast<std::size_t>(request.getContentLength()),
                         getUploadedDocument().size());
        checkDocument();
    }

    bool onDocumentLoaded(const std::string& message) override
    {
        TST_LOG("onDocumentLoaded: [" << message << ']');

        // A template is uploaded right after the load, with no save asked for.
        if (_mode != Mode::Template)
        {
            LOK_ASSERT_STATE(_phase, Phase::WaitModified);
            if (_mode == Mode::NotesView)
                WSD_CMD("uno .uno:NotesMode");

            WSD_CMD("uno .uno:Modified {\"Modified\":{\"type\":\"boolean\",\"value\":true}}");
        }

        return true;
    }

    std::unique_ptr<http::Response>
    assertPutFileRequest(const Poco::Net::HTTPRequest& request) override
    {
        if (_mode == Mode::Retry && _phase == Phase::WaitRetry)
        {
            checkMultipartUpload(request);
            TRANSITION_STATE(_phase, Phase::Done);
            return nullptr;
        }

        LOK_ASSERT_STATE(_phase, Phase::WaitFirstUpload);

        switch (_mode)
        {
            case Mode::Thumbnails:
                checkMultipartUpload(request);
                break;

            case Mode::Retry:
                checkMultipartUpload(request);
                return std::make_unique<http::Response>(http::StatusCode::InternalServerError);

            case Mode::Template:
                checkMultipartUpload(request);
                TRANSITION_STATE(_phase, Phase::Done);
                break;

            case Mode::NoSizes:
            case Mode::Password:
            case Mode::Watermark:
            case Mode::NotesView:
                checkPlainUpload(request);
                TRANSITION_STATE(_phase, Phase::Done);
                break;
        }

        return nullptr;
    }

    void onDocumentUploaded(bool success) override
    {
        if (_mode == Mode::Retry && _phase == Phase::WaitFirstUpload)
        {
            LOK_ASSERT(!success);
            TRANSITION_STATE(_phase, Phase::WaitRetry);

            // The document is unmodified since the failed upload, so this save writes no new
            // version, and the upload that follows retries the version that failed.
            WSD_CMD("save dontTerminateEdit=1 dontSaveIfUnmodified=1");
            return;
        }

        LOK_ASSERT(success);
        if (_phase == Phase::WaitFirstUpload)
        {
            TRANSITION_STATE(_phase, Phase::WaitUnmodifiedSave);

            // The document is unmodified since the save, so this saves and uploads nothing.
            WSD_CMD("save dontTerminateEdit=1 dontSaveIfUnmodified=1");
        }
        else if (_phase == Phase::Done && _mode == Mode::Template)
        {
            passTest("The upload after loading a template carries the thumbnails");
        }
        else if (_phase == Phase::Done && _mode == Mode::Retry)
        {
            passTest("The retry of a failed upload carries the thumbnails");
        }
        else if (_phase == Phase::Done && _mode != Mode::Thumbnails)
        {
            passTest("The upload is the plain document");
        }
    }

    void assertPutRelativeFileRequest(const Poco::Net::HTTPRequest& request) override
    {
        LOK_ASSERT_STATE(_phase, Phase::WaitSaveAs);

        // The save of the unmodified document uploaded nothing.
        LOK_ASSERT_EQUAL(1UL, getCountPutFile());

        checkMultipartUpload(request);

        // X-WOPI-Size is the size of the document, which is less than that of the whole body.
        LOK_ASSERT_EQUAL(std::to_string(getUploadedDocument().size()), request.get("X-WOPI-Size"));
        LOK_ASSERT(static_cast<std::size_t>(request.getContentLength()) >
                   getUploadedDocument().size());

        TRANSITION_STATE(_phase, Phase::Done);
    }

    bool onFilterSendWebSocketMessage(const std::string_view message, const WSOpCode /*code*/,
                                      const bool /*flush*/, int& /*unitReturn*/) override
    {
        if (_phase == Phase::WaitModified && message == "statechanged: .uno:ModifiedStatus=true")
        {
            // The kit tracks the modified state from this notification, so the save goes out once
            // the kit has marked the document modified.
            TRANSITION_STATE(_phase, Phase::WaitFirstUpload);
            WSD_CMD("save dontTerminateEdit=1 dontSaveIfUnmodified=0");
        }
        else if (_phase == Phase::WaitUnmodifiedSave && message.starts_with("unocommandresult:") &&
                 message.find(".uno:Save") != std::string_view::npos)
        {
            TST_LOG("The save of the unmodified document is done: " << message);
            TRANSITION_STATE(_phase, Phase::WaitSaveAs);
            WSD_CMD("saveas url=wopi:///path/renamed." + FileUtil::extractFileExtension(_filename));
        }
        else if (_phase == Phase::Done && message.starts_with("saveas: url="))
        {
            passTest("A save and a Save As upload the thumbnails with the document");
        }

        return false;
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                // A template is uploaded right after the load, the others once modified.
                if (_mode == Mode::Template)
                    TRANSITION_STATE(_phase, Phase::WaitFirstUpload);
                else
                    TRANSITION_STATE(_phase, Phase::WaitModified);

                initWebsocket("/wopi/files/0?access_token=anything");
                WSD_CMD("load url=" + getWopiSrc() +
                        (_mode == Mode::Password ? " password=1" : ""));
                break;
            }
            case Phase::WaitModified:
            case Phase::WaitFirstUpload:
            case Phase::WaitRetry:
            case Phase::WaitUnmodifiedSave:
            case Phase::WaitSaveAs:
            case Phase::Done:
                break;
        }
    }
};

UnitBase** unit_create_wsd_multi(void)
{
    using Mode = UnitWOPIThumbnails::Mode;
    using Shape = UnitWOPIThumbnails::Shape;
    constexpr const char* Text = "application/vnd.oasis.opendocument.text";
    constexpr const char* Spreadsheet = "application/vnd.oasis.opendocument.spreadsheet";
    constexpr const char* Presentation = "application/vnd.oasis.opendocument.presentation";
    return new UnitBase*[11]{
        new UnitWOPIThumbnails("Writer", Mode::Thumbnails, "hello.odt", Text, Shape::Portrait,
                               /*backgroundSave=*/false),
        new UnitWOPIThumbnails("Calc", Mode::Thumbnails, "hello.ods", Spreadsheet, Shape::Filled,
                               /*backgroundSave=*/false),
        new UnitWOPIThumbnails("Impress", Mode::Thumbnails, "hello.odp", Presentation,
                               Shape::Landscape, /*backgroundSave=*/false),
        new UnitWOPIThumbnails("Template", Mode::Template, "hello.odt", Text, Shape::Portrait,
                               /*backgroundSave=*/false),
        new UnitWOPIThumbnails("Retry", Mode::Retry, "hello.odt", Text, Shape::Portrait,
                               /*backgroundSave=*/false),
        new UnitWOPIThumbnails("NoSizes", Mode::NoSizes, "hello.odt", Text, Shape::Portrait,
                               /*backgroundSave=*/false),
        new UnitWOPIThumbnails("Password", Mode::Password, "password-protected.ods", Spreadsheet,
                               Shape::Filled, /*backgroundSave=*/false),
        new UnitWOPIThumbnails("Watermark", Mode::Watermark, "hello.odt", Text, Shape::Portrait,
                               /*backgroundSave=*/false),
        new UnitWOPIThumbnails("Background", Mode::Thumbnails, "hello.odt", Text, Shape::Portrait,
                               /*backgroundSave=*/true),
        new UnitWOPIThumbnails("NotesView", Mode::NotesView, "hello.odp", Presentation,
                               Shape::Landscape, /*backgroundSave=*/false),
        nullptr
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
