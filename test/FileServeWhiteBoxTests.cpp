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

/*
 * White box unit test for file server functionality.
 */

#include <config.h>

#include <wsd/ContentSecurityPolicy.hpp>
#include <wsd/FileServer.hpp>
#include <common/ContainerUtil.hpp>
#include <common/FileUtil.hpp>
#include <test/lokassert.hpp>

#include <Poco/String.h>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

#include <cstddef>
#include <memory>

/// File-Serve White-Box unit-tests.
class FileServeTests : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(FileServeTests);
    CPPUNIT_TEST(testUIDefaults);
    CPPUNIT_TEST(testCSSVars);
    CPPUNIT_TEST(testPreProcessedFile);
    CPPUNIT_TEST(testPreProcessedFileRoundtrip);
    CPPUNIT_TEST(testPreProcessedFileSubstitution);
    CPPUNIT_TEST(testCSPMergeNewlines);
    CPPUNIT_TEST_SUITE_END();

    void testUIDefaults();
    void testCSSVars();
    void testPreProcessedFile();
    void testPreProcessedFileRoundtrip();
    void testPreProcessedFileSubstitution();
    void testCSPMergeNewlines();

    void preProcessedFileSubstitution(const std::string_view testname,
                                      const Util::UnorderedStringMap<std::string>& variables);
    // Helper replace each occurrence of from in str to variables[to_key] except if to_key is not in variables
    std::string& replaceIfExist(std::string& str, const std::string& from,
                                const std::string& to_key,
                                const Util::UnorderedStringMap<std::string>& variables);
};

void FileServeTests::testUIDefaults()
{
    constexpr std::string_view testname = __func__;

    std::string uiMode;
    std::string uiTheme;
    std::string savedUIState;

    LOK_ASSERT_EQUAL_STR("{\"uiMode\":\"classic\"}",
                         FileServerRequestHandler::uiDefaultsToJSON("UIMode=classic;huh=bleh;",
                                                                    uiMode, uiTheme, savedUIState));
    LOK_ASSERT_EQUAL_STR("classic", uiMode);

    LOK_ASSERT_EQUAL_STR(
        "{\"spreadsheet\":{\"ShowSidebar\":\"false\"},\"text\":{\"ShowRuler\":\"true\"}}",
        FileServerRequestHandler::uiDefaultsToJSON("TextRuler=true;SpreadsheetSidebar=false",
                                                   uiMode, uiTheme, savedUIState));
    LOK_ASSERT_EQUAL_STR("", uiMode);

    LOK_ASSERT_EQUAL_STR(
        "{\"presentation\":{\"ShowStatusbar\":\"false\"},\"spreadsheet\":{\"ShowSidebar\":"
        "\"false\"},\"text\":{\"ShowRuler\":\"true\"},\"uiMode\":\"notebookbar\"}",
        FileServerRequestHandler::uiDefaultsToJSON(
            ";;UIMode=notebookbar;;PresentationStatusbar=false;;TextRuler=true;;bah=ugh;;"
            "SpreadsheetSidebar=false",
            uiMode, uiTheme, savedUIState));

    LOK_ASSERT_EQUAL_STR(
        "{\"drawing\":{\"ShowStatusbar\":\"true\"},\"presentation\":{"
        "\"ShowStatusbar\":\"false\"},\"spreadsheet\":{\"ShowSidebar\":\"false\"},"
        "\"text\":{\"ShowRuler\":\"true\"},\"uiMode\":\"notebookbar\"}",
        FileServerRequestHandler::uiDefaultsToJSON(
            ";;UIMode=notebookbar;;PresentationStatusbar=false;;TextRuler=true;;bah="
            "ugh;;SpreadsheetSidebar=false;;DrawingStatusbar=true",
            uiMode, uiTheme, savedUIState));

    LOK_ASSERT_EQUAL_STR("notebookbar", uiMode);
}

void FileServeTests::testCSSVars()
{
    constexpr std::string_view testname = __func__;

    LOK_ASSERT_EQUAL_STR(
        "OnJvb3Qgey0tY28tc29tZXN0eWxlLXRleHQ6IzEyMzQ1NjstLWNvLXNvbWVzdHlsZS1zaXplOjE1cHg7fQ==",
        FileServerRequestHandler::cssVarsToStyle(
            "--co-somestyle-text=#123456;--co-somestyle-size=15px;"));

    LOK_ASSERT_EQUAL_STR(
        "OnJvb3Qgey0tY28tc29tZXN0eWxlLXRleHQ6IzEyMzQ1NjstLWNvLXNvbWVzdHlsZS1zaXplOjE1cHg7fQ==",
        FileServerRequestHandler::cssVarsToStyle(
            ";;--co-somestyle-text=#123456;;--co-somestyle-size=15px;;;"));

    LOK_ASSERT_EQUAL_STR(
        "OnJvb3Qgey0tY28tc29tZXN0eWxlLXRleHQ6IzEyMzQ1NjstLWNvLXNvbWVzdHlsZS1zaXplOjE1cHg7fQ==",
        FileServerRequestHandler::cssVarsToStyle(
            "--co-somestyle-text=#123456;;--co-somestyle-size=15px;--co-sometext#324;;"));

    LOK_ASSERT_EQUAL_STR(
        "OnJvb3Qgey0tY28tc29tZXN0eWxlLXRleHQ6IzEyMzQ1Njt9",
        FileServerRequestHandler::cssVarsToStyle(
            "--co-somestyle-text=#123456;;--some-val=3453--some-other-val=4536;;"));
}

/// Tests file pre-processing through PreProcessedFile class.
void FileServeTests::testPreProcessedFile()
{
    constexpr std::string_view testname = __func__;

    {
        const std::string data = "Data %VAR% Data";
        const PreProcessedFile ppf("filename", data);
        LOK_ASSERT_EQUAL(ppf.filename(), std::string("filename"));
        LOK_ASSERT_EQUAL(ppf.size(), data.size());
        LOK_ASSERT_EQUAL(3UL, ppf._segments.size());
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Data, ppf._segments[0].first);
        LOK_ASSERT_EQUAL_STR("Data ", ppf._segments[0].second);
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Variable, ppf._segments[1].first);
        LOK_ASSERT_EQUAL_STR("VAR", ppf._segments[1].second);
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Data, ppf._segments[2].first);
        LOK_ASSERT_EQUAL_STR(" Data", ppf._segments[2].second);
    }

    {
        const std::string data = "Data %VAR%";
        const PreProcessedFile ppf("filename", data);
        LOK_ASSERT_EQUAL(ppf.filename(), std::string("filename"));
        LOK_ASSERT_EQUAL(ppf.size(), data.size());
        LOK_ASSERT_EQUAL(2UL, ppf._segments.size());
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Data, ppf._segments[0].first);
        LOK_ASSERT_EQUAL_STR("Data ", ppf._segments[0].second);
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Variable, ppf._segments[1].first);
        LOK_ASSERT_EQUAL_STR("VAR", ppf._segments[1].second);
    }

    {
        const std::string data = "%VAR% Data";
        const PreProcessedFile ppf("filename", data);
        LOK_ASSERT_EQUAL(ppf.filename(), std::string("filename"));
        LOK_ASSERT_EQUAL(ppf.size(), data.size());
        LOK_ASSERT_EQUAL(2UL, ppf._segments.size());
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Variable, ppf._segments[0].first);
        LOK_ASSERT_EQUAL_STR("VAR", ppf._segments[0].second);
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Data, ppf._segments[1].first);
        LOK_ASSERT_EQUAL_STR(" Data", ppf._segments[1].second);
    }

    {
        const std::string data = "%VAR%";
        const PreProcessedFile ppf("filename", data);
        LOK_ASSERT_EQUAL(ppf.filename(), std::string("filename"));
        LOK_ASSERT_EQUAL(ppf.size(), data.size());
        LOK_ASSERT_EQUAL(1UL, ppf._segments.size());
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Variable, ppf._segments[0].first);
        LOK_ASSERT_EQUAL_STR("VAR", ppf._segments[0].second);
    }

    {
        const std::string data = "%VAR%Data1 %VAR% Data%VAR%";
        const PreProcessedFile ppf("filename", data);
        LOK_ASSERT_EQUAL(ppf.filename(), std::string("filename"));
        LOK_ASSERT_EQUAL(ppf.size(), data.size());
        LOK_ASSERT_EQUAL(5UL, ppf._segments.size());
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Variable, ppf._segments[0].first);
        LOK_ASSERT_EQUAL_STR("VAR", ppf._segments[0].second);
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Data, ppf._segments[1].first);
        LOK_ASSERT_EQUAL_STR("Data1 ", ppf._segments[1].second);
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Variable, ppf._segments[2].first);
        LOK_ASSERT_EQUAL_STR("VAR", ppf._segments[2].second);
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Data, ppf._segments[3].first);
        LOK_ASSERT_EQUAL_STR(" Data", ppf._segments[3].second);
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Variable, ppf._segments[4].first);
        LOK_ASSERT_EQUAL_STR("VAR", ppf._segments[4].second);
    }

    {
        const std::string data = "Data %VAR Data";
        const PreProcessedFile ppf("filename", data);
        LOK_ASSERT_EQUAL(ppf.filename(), std::string("filename"));
        LOK_ASSERT_EQUAL(ppf.size(), data.size());
        LOK_ASSERT_EQUAL(1UL, ppf._segments.size());
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Data, ppf._segments[0].first);
        LOK_ASSERT_EQUAL_STR("Data %VAR Data", ppf._segments[0].second);
    }

    {
        const std::string data = "Data 5% Data 7%";
        const PreProcessedFile ppf("filename", data);
        LOK_ASSERT_EQUAL(ppf.filename(), std::string("filename"));
        LOK_ASSERT_EQUAL(ppf.size(), data.size());
        LOK_ASSERT_EQUAL(1UL, ppf._segments.size());
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Data, ppf._segments[0].first);
        LOK_ASSERT_EQUAL_STR("Data 5% Data 7%", ppf._segments[0].second);
    }

    {
        const std::string data = "Data <!--%VAR%--> Data";
        const PreProcessedFile ppf("filename", data);
        LOK_ASSERT_EQUAL(ppf.filename(), std::string("filename"));
        LOK_ASSERT_EQUAL(ppf.size(), data.size());
        LOK_ASSERT_EQUAL(3UL, ppf._segments.size());
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Data, ppf._segments[0].first);
        LOK_ASSERT_EQUAL_STR("Data ", ppf._segments[0].second);
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::CommentedVariable, ppf._segments[1].first);
        LOK_ASSERT_EQUAL_STR("VAR", ppf._segments[1].second);
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Data, ppf._segments[2].first);
        LOK_ASSERT_EQUAL_STR(" Data", ppf._segments[2].second);
    }

    {
        const std::string data = "Data <!--%VAR%-->";
        const PreProcessedFile ppf("filename", data);
        LOK_ASSERT_EQUAL(ppf.filename(), std::string("filename"));
        LOK_ASSERT_EQUAL(ppf.size(), data.size());
        LOK_ASSERT_EQUAL(2UL, ppf._segments.size());
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Data, ppf._segments[0].first);
        LOK_ASSERT_EQUAL_STR("Data ", ppf._segments[0].second);
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::CommentedVariable, ppf._segments[1].first);
        LOK_ASSERT_EQUAL_STR("VAR", ppf._segments[1].second);
    }

    {
        const std::string data = "<!--%VAR%--> Data";
        const PreProcessedFile ppf("filename", data);
        LOK_ASSERT_EQUAL(ppf.filename(), std::string("filename"));
        LOK_ASSERT_EQUAL(ppf.size(), data.size());
        LOK_ASSERT_EQUAL(2UL, ppf._segments.size());
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::CommentedVariable, ppf._segments[0].first);
        LOK_ASSERT_EQUAL_STR("VAR", ppf._segments[0].second);
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Data, ppf._segments[1].first);
        LOK_ASSERT_EQUAL_STR(" Data", ppf._segments[1].second);
    }

    {
        const std::string data = "<!--%VAR%-->";
        const PreProcessedFile ppf("filename", data);
        LOK_ASSERT_EQUAL(ppf.filename(), std::string("filename"));
        LOK_ASSERT_EQUAL(ppf.size(), data.size());
        LOK_ASSERT_EQUAL(1UL, ppf._segments.size());
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::CommentedVariable, ppf._segments[0].first);
        LOK_ASSERT_EQUAL_STR("VAR", ppf._segments[0].second);
    }

    {
        const std::string data = "<!--%VAR%-->Data1 <!--%VAR%--> Data2<!--%VAR%-->";
        const PreProcessedFile ppf("filename", data);
        LOK_ASSERT_EQUAL(ppf.filename(), std::string("filename"));
        LOK_ASSERT_EQUAL(ppf.size(), data.size());
        LOK_ASSERT_EQUAL(5UL, ppf._segments.size());
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::CommentedVariable, ppf._segments[0].first);
        LOK_ASSERT_EQUAL_STR("VAR", ppf._segments[0].second);
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Data, ppf._segments[1].first);
        LOK_ASSERT_EQUAL_STR("Data1 ", ppf._segments[1].second);
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::CommentedVariable, ppf._segments[2].first);
        LOK_ASSERT_EQUAL_STR("VAR", ppf._segments[2].second);
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Data, ppf._segments[3].first);
        LOK_ASSERT_EQUAL_STR(" Data2", ppf._segments[3].second);
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::CommentedVariable, ppf._segments[4].first);
        LOK_ASSERT_EQUAL_STR("VAR", ppf._segments[4].second);
    }

    {
        const std::string data = "<!--%VARA% Data2 <!--%VARB%--> Data4<!--%VARC%-->";
        const PreProcessedFile ppf("filename", data);
        LOK_ASSERT_EQUAL(ppf.filename(), std::string("filename"));
        LOK_ASSERT_EQUAL(ppf.size(), data.size());
        LOK_ASSERT_EQUAL(4UL, ppf._segments.size());
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Data, ppf._segments[0].first);
        LOK_ASSERT_EQUAL_STR("<!--%VARA% Data2 ", ppf._segments[0].second);
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::CommentedVariable, ppf._segments[1].first);
        LOK_ASSERT_EQUAL_STR("VARB", ppf._segments[1].second);
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Data, ppf._segments[2].first);
        LOK_ASSERT_EQUAL_STR(" Data4", ppf._segments[2].second);
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::CommentedVariable, ppf._segments[3].first);
        LOK_ASSERT_EQUAL_STR("VARC", ppf._segments[3].second);
    }

    {
        const std::string data = "<!--%VAR%-->Data1 <!--%VAR%--> Data2<!--%VAR%";
        const PreProcessedFile ppf("filename", data);
        LOK_ASSERT_EQUAL(ppf.filename(), std::string("filename"));
        LOK_ASSERT_EQUAL(ppf.size(), data.size());
        LOK_ASSERT_EQUAL(4UL, ppf._segments.size());
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::CommentedVariable, ppf._segments[0].first);
        LOK_ASSERT_EQUAL_STR("VAR", ppf._segments[0].second);
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Data, ppf._segments[1].first);
        LOK_ASSERT_EQUAL_STR("Data1 ", ppf._segments[1].second);
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::CommentedVariable, ppf._segments[2].first);
        LOK_ASSERT_EQUAL_STR("VAR", ppf._segments[2].second);
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Data, ppf._segments[3].first);
        LOK_ASSERT_EQUAL_STR(" Data2<!--%VAR%", ppf._segments[3].second);
    }

    {
        const std::string data = R"xxx(<!DOCTYPE html>
<html %UI_RTL_SETTINGS% style="height:100%"><head><meta http-equiv="Content-Type" content="text/html;charset=utf-8">
<title>Online Editor</title>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1, minimum-scale=1, maximum-scale=1, user-scalable=no, interactive-widget=resizes-content">

<script>





// FIXME: This is temporary and not what we actually eventually want.

// What we really want is not a separate HTML file (produced with M4 conditionals on the below
// ) for a "WASM app". What we want is that the same cool.html page adapts on demand to
// instead run locally using WASM, if the connection to the COOL server breaks. (And then
// re-connects to the COOL server when possible.)



window.welcomeUrl = '%WELCOME_URL%';
  window.feedbackUrl = '%FEEDBACK_URL%';
  window.buyProductUrl = '%BUYPRODUCT_URL%';
)xxx";
        const PreProcessedFile ppf("filename", data);
        LOK_ASSERT_EQUAL(ppf.filename(), std::string("filename"));
        LOK_ASSERT_EQUAL(ppf.size(), data.size());
        LOK_ASSERT_EQUAL(9UL, ppf._segments.size());
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Data, ppf._segments[0].first);
        // LOK_ASSERT_EQUAL_STR("VAR", ppf._segments[0].second);
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Variable, ppf._segments[1].first);
        LOK_ASSERT_EQUAL_STR("UI_RTL_SETTINGS", ppf._segments[1].second);
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Data, ppf._segments[2].first);
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Variable, ppf._segments[3].first);
        LOK_ASSERT_EQUAL_STR("WELCOME_URL", ppf._segments[3].second);
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Data, ppf._segments[4].first);
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Variable, ppf._segments[5].first);
        LOK_ASSERT_EQUAL_STR("FEEDBACK_URL", ppf._segments[5].second);
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Data, ppf._segments[6].first);
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Variable, ppf._segments[7].first);
        LOK_ASSERT_EQUAL_STR("BUYPRODUCT_URL", ppf._segments[7].second);
        LOK_ASSERT_EQUAL(PreProcessedFile::SegmentType::Data, ppf._segments[8].first);
    }
}

void FileServeTests::testPreProcessedFileRoundtrip()
{
    constexpr std::string_view testname = __func__;

    const Poco::Path path(TDIST);

    std::vector<std::string> files;
    Poco::File(path).list(files);
    for (const std::string& file : files)
    {
        std::unique_ptr<std::vector<char>> data =
            FileUtil::readFile(Poco::Path(path, file).toString());

        if (data)
        {
            const std::string orig(data->data(), data->size());
            PreProcessedFile ppf(file, orig);
            LOK_ASSERT_EQUAL(file, ppf.filename());
            LOK_ASSERT_EQUAL(data->size(), ppf.size());

            std::string recon = ppf.substitute({});

            LOK_ASSERT_EQUAL(orig, recon);
        }
    }
}

void FileServeTests::preProcessedFileSubstitution(
    const std::string_view testname, const Util::UnorderedStringMap<std::string>& variables)
{
    const Poco::Path path(TDIST);

    std::vector<std::string> files;
    Poco::File(path).list(files);
    for (const std::string& file : files)
    {
        std::unique_ptr<std::vector<char>> data =
            FileUtil::readFile(Poco::Path(path, file).toString());

        if (data)
        {
            std::string orig(data->data(), data->size());
            PreProcessedFile ppf(file, orig);
            LOK_ASSERT_EQUAL(file, ppf.filename());
            LOK_ASSERT_EQUAL(data->size(), ppf.size());

            const std::string recon = ppf.substitute(variables);

            replaceIfExist(orig, std::string("%ACCESS_TOKEN%"), "ACCESS_TOKEN", variables);
            replaceIfExist(orig, std::string("%ACCESS_TOKEN_TTL%"), "ACCESS_TOKEN_TTL", variables);
            replaceIfExist(orig, std::string("%ACCESS_HEADER%"), "ACCESS_HEADER", variables);
            replaceIfExist(orig, std::string("%UI_DEFAULTS%"), "UI_DEFAULTS", variables);
            replaceIfExist(orig, std::string("<!--%CSS_VARIABLES%-->"), "CSS_VARIABLES", variables);
            replaceIfExist(orig, std::string("%POSTMESSAGE_ORIGIN%"), "POSTMESSAGE_ORIGIN",
                           variables);
            replaceIfExist(orig, std::string("%BRANDING_THEME%"), "BRANDING_THEME", variables);
            replaceIfExist(orig, std::string("<!--%BRANDING_JS%-->"), "BRANDING_JS", variables);
            replaceIfExist(orig, std::string("%FOOTER%"), "FOOTER", variables);
            replaceIfExist(orig, std::string("%CHECK_FILE_INFO_OVERRIDE%"),
                           "CHECK_FILE_INFO_OVERRIDE", variables);
            replaceIfExist(orig, std::string("%BUYPRODUCT_URL%"), "BUYPRODUCT_URL", variables);

            LOK_ASSERT_EQUAL(orig, recon);
        }
    }
}

std::string&
FileServeTests::replaceIfExist(std::string& str, const std::string& from, const std::string& to_key,
                               const Util::UnorderedStringMap<std::string>& variables)
{
    auto search = variables.find(to_key);
    if (search == variables.end())
    {
        // key not found, do nothing
        return str;
    }
    return Poco::replaceInPlace(str, from, search->second);
}

void FileServeTests::testPreProcessedFileSubstitution()
{
    constexpr std::string_view testname = __func__;

    Util::UnorderedStringMap<std::string> variables = {
        { "ACCESS_TOKEN", "alksjdfiwjksnsdkafnsdl" },
        { "ACCESS_TOKEN_TTL", "123" },
        { "ACCESS_HEADER", "8923rweyhjsnjfnwoejl" },
        { "UI_DEFAULTS",
          "{\"presentation\":{\"ShowStatusbar\":false},\"spreadsheet\":{\"ShowSidebar\":"
          "false},\"text\":{\"ShowRuler\":true},\"uiMode\":\"notebookbar\"}" },
        { "CSS_VARIABLES",
          "<style>:root {--co-somestyle-text:#123456;--co-somestyle-size:15px;}</style>" },
        { "POSTMESSAGE_ORIGIN", "https://www.example.com:8080" },
        { "BRANDING_THEME", "cool_brand" },
        { "BRANDING_JS", "branding.js" },
        { "FOOTER", "<div><b>blah blah footer</b></div>" },
        { "CHECK_FILE_INFO_OVERRIDE", "DownloadAsPostMessage=true;blah=bleh" },
        { "BUYPRODUCT_URL", "https://buy.ourproduct.com/'" }
    };

    preProcessedFileSubstitution(testname, variables);
    preProcessedFileSubstitution(std::string(testname) + "_empty",
                                 Util::UnorderedStringMap<std::string>());
}

void FileServeTests::testCSPMergeNewlines()
{
    constexpr std::string_view testname = __func__;

    // Same line: the simple case.
    {
        ContentSecurityPolicy csp("frame-ancestors https://example.com; img-src https://example.com");
        LOK_ASSERT_EQUAL_STR(" https://example.com", csp.getDirective("frame-ancestors"));
        LOK_ASSERT_EQUAL_STR(" https://example.com", csp.getDirective("img-src"));
    }

    // Value on new line, as Poco XMLConfiguration returns when the XML
    // config value is on a separate line from the opening tag.
    {
        ContentSecurityPolicy csp("\n        frame-ancestors https://example.com; img-src https://example.com\n    ");
        LOK_ASSERT_EQUAL_STR(" https://example.com", csp.getDirective("frame-ancestors"));
        LOK_ASSERT_EQUAL_STR(" https://example.com", csp.getDirective("img-src"));
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(FileServeTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
