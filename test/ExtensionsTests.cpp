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

#include <config.h>

#include <string>

#include <unistd.h>

#include <Poco/File.h>
#include <Poco/FileStream.h>
#include <Poco/Path.h>
#include <Poco/TemporaryFile.h>
#include <cppunit/extensions/HelperMacros.h>

#include <test/lokassert.hpp>
#include <wsd/Extensions.hpp>

namespace {

// The child root of the test data, with the extension presets of the configuration configId:
constexpr char childRoot[] = TDOC "/extensions-childroot";
constexpr char configId[] = "shared-https_example.com:9980/settings";

class ExtensionsTests: public CPPUNIT_NS::TestFixture {
public:
#if ENABLE_DEBUG
    // The built-in extensions are in a copy of the test data, as their directory is recorded before
    // any lookup:
    void setUp() override {
        Poco::File(TDOC "/extensions-builtin").copyTo(builtinDirectory_.path());
        Extensions::synthesizeBuiltinExtensionsIndex(builtinDirectory_.path());
    }
#endif

    CPPUNIT_TEST_SUITE(ExtensionsTests);

    CPPUNIT_TEST(testRemote);
    CPPUNIT_TEST(testGas);
    CPPUNIT_TEST(testGasSidecar);
    CPPUNIT_TEST(testServableFile);
    CPPUNIT_TEST(testDocumentEvent);
    CPPUNIT_TEST(testUnknown);
#if ENABLE_DEBUG
    CPPUNIT_TEST(testBuiltin);
#endif

    CPPUNIT_TEST_SUITE_END();

    void testRemote() {
        constexpr std::string_view testname = __func__;

        // A remote call runs the extension's remote files in their order, each under its own name,
        // and then calls the function that they added to the remote object:
        std::string error;
        auto const script = Extensions::makeCallScript(
            childRoot, configId, "com.example.remote", "remote", "add", error);
        LOK_ASSERT(script);
        LOK_ASSERT_EQUAL(std::string("<remote-call>"), script->source);
        LOK_ASSERT_EQUAL(1, script->line);
        LOK_ASSERT(script->text.starts_with(
            "const remote = {};\n$internal.evalWithSource(\"function twice(x) {\\n"));
        LOK_ASSERT(
            script->text.find(
                ", \"com.example.remote/helper.js\", 1);\n"
                "$internal.evalWithSource(\"remote.add = function add(a, b) {\\n")
            != std::string::npos);
        LOK_ASSERT(
            script->text.find(", \"com.example.remote/remote.js\", 1);\n(function (name) {")
            != std::string::npos);
        LOK_ASSERT(script->text.ends_with("})(\"add\")"));

        // Any name can be looked up in the remote object:
        auto const other = Extensions::makeCallScript(
            childRoot, configId, "com.example.remote", "remote", "foo-bar", error);
        LOK_ASSERT(other);
        LOK_ASSERT(other->text.ends_with("})(\"foo-bar\")"));

        // An extension that is neither a preset nor built-in is not installed:
        error.clear();
        LOK_ASSERT(!Extensions::makeCallScript(
            childRoot, configId, "com.example.missing", "remote", "add", error));
        LOK_ASSERT(!error.empty());

        // An extension ID names a directory directly within the extensions directory:
        error.clear();
        LOK_ASSERT(!Extensions::makeCallScript(
            childRoot, configId, "com.example.gas/../com.example.remote", "remote", "add", error));
        LOK_ASSERT(!error.empty());
    }

    void testGas() {
        constexpr std::string_view testname = __func__;

        // An Apps Script call has the add-on's code files in the order of its _cool-gas.json, and
        // its libraries:
        std::string error;
        auto const script = Extensions::makeCallScript(
            childRoot, configId, "com.example.gas", "gas", "run", error);
        LOK_ASSERT(script);
        LOK_ASSERT(script->text.find("[\"b.gs\",\"a.gs\"]") != std::string::npos);
        LOK_ASSERT(script->text.starts_with("(function (proxyId, callArgs) {"));
        LOK_ASSERT(script->text.find("\"run\", callArgs") != std::string::npos);
        LOK_ASSERT(
            script->text.find("{\"userSymbol\":\"Lib\",\"dir\":\"_cool-gas-libraries/lib1/3/\","
                              "\"scripts\":[\"lib.gs\"],")
            != std::string::npos);
        LOK_ASSERT(script->text.find("return 3;") != std::string::npos);

        // An extension without appsscript.json is no Apps Script add-on:
        error.clear();
        LOK_ASSERT(!Extensions::makeCallScript(
            childRoot, configId, "com.example.remote", "gas", "run", error));
        LOK_ASSERT(!error.empty());

        // Neither is an extension with both an appsscript.json and a manifest.json:
        Poco::TemporaryFile root;
        Poco::Path const both = Poco::Path::forDirectory(
            root.path()
            + "/tmp/sharedpresets/shared-https_example.com%3A9980%2Fsettings/extensions/"
              "com.example.both/");
        Poco::File(both).createDirectories();
        Poco::FileOutputStream(Poco::Path(both, "appsscript.json").toString()) << "{}";
        Poco::FileOutputStream(Poco::Path(both, "manifest.json").toString())
            << "{\"manifestVersion\": \"0.2\"}";
        Poco::FileOutputStream(Poco::Path(both, "a.gs").toString()) << "function run() {}";
        error.clear();
        LOK_ASSERT(!Extensions::makeCallScript(
            root.path(), configId, "com.example.both", "gas", "run", error));
        LOK_ASSERT(!error.empty());
    }

    void testGasSidecar() {
        constexpr std::string_view testname = __func__;

        // The sidecar lists the scripts, and the names of the functions that they declare, each
        // once, in the order of first occurrence:
        auto const sidecar = Extensions::synthesizeGasSidecar(
            {{"b.gs", "function onOpen() {}\nfunction  run$1 (x) { function inner(){} }"},
             {"a.gs", "var f = function () {};\nfunction onOpen() {}\nfunction _helper\n()"}});
        LOK_ASSERT(
            sidecar.starts_with(
                "{\"scripts\":[\"b.gs\",\"a.gs\"],"
                "\"functions\":[\"onOpen\",\"run$1\",\"inner\",\"_helper\"]"));

        // Only the keyword followed by a name counts:
        auto const other = Extensions::synthesizeGasSidecar(
            {{"c.gs",
              "myfunction notMe() {}\nfunctionX() {}\nfunction 1st() {}\nfunction ok() {}"}});
        LOK_ASSERT(other.starts_with("{\"scripts\":[\"c.gs\"],\"functions\":[\"ok\"]"));
    }

    void testServableFile() {
        constexpr std::string_view testname = __func__;

        Poco::Path const presets(
            std::string(childRoot)
            + "/tmp/sharedpresets/shared-https_example.com%3A9980%2Fsettings/extensions/");

        // Pages of an Apps Script add-on can load everything but its scripts, also those of its
        // libraries:
        Poco::Path const gas(presets, "com.example.gas/");
        LOK_ASSERT(Extensions::servableFile(gas, "appsscript.json").has_value());
        LOK_ASSERT(!Extensions::servableFile(gas, "a.gs"));
        LOK_ASSERT(!Extensions::servableFile(gas, "_cool-gas-libraries/lib1/3/lib.gs"));
        LOK_ASSERT(
            Extensions::servableFile(gas, "_cool-gas-libraries/lib1/3/appsscript.json")
                .has_value());

        // Pages of a native extension can load everything but its remote files:
        Poco::Path const remote(presets, "com.example.remote/");
        LOK_ASSERT(Extensions::servableFile(remote, "manifest.json").has_value());
        LOK_ASSERT(!Extensions::servableFile(remote, "remote.js"));
        LOK_ASSERT(!Extensions::servableFile(remote, "helper.js"));

        // A file has a single spelling:
        LOK_ASSERT(!Extensions::servableFile(remote, "./remote.js"));
        LOK_ASSERT(
            !Extensions::servableFile(gas, "_cool-gas-libraries//lib1/3/appsscript.json"));
        LOK_ASSERT(!Extensions::servableFile(remote, std::string("remote.js\0", 10)));
        LOK_ASSERT_EQUAL(
            Poco::Path(
                std::string(childRoot)
                + "/tmp/sharedpresets/shared-https_example.com%3A9980%2Fsettings/extensions/"
                  "com.example.remote/manifest.json")
                .toString(),
            Extensions::servableFile(remote, "manifest.json")->toString());

        // A directory is no file:
        LOK_ASSERT(!Extensions::servableFile(gas, "_cool-gas-libraries/lib1"));

        // A file that the _cool-gas.json of an Apps Script add-on lists as a script is code,
        // whatever its name, and a symbolic link is never served:
        Poco::TemporaryFile directory;
        directory.createDirectories();
        Poco::Path const addOn = Poco::Path::forDirectory(directory.path());
        Poco::FileOutputStream(Poco::Path(addOn, "appsscript.json").toString()) << "{}";
        Poco::FileOutputStream(Poco::Path(addOn, "_cool-gas.json").toString())
            << "{\"scripts\": [\"code.txt\"]}";
        Poco::FileOutputStream(Poco::Path(addOn, "code.txt").toString()) << "function f() {}";
        Poco::FileOutputStream(Poco::Path(addOn, "page.html").toString()) << "<p></p>";
        LOK_ASSERT(!Extensions::servableFile(addOn, "code.txt"));
        LOK_ASSERT(Extensions::servableFile(addOn, "page.html").has_value());
        LOK_ASSERT_EQUAL(
            0,
            symlink(
                Poco::Path(addOn, "page.html").toString().c_str(),
                Poco::Path(addOn, "link.html").toString().c_str()));
        LOK_ASSERT(!Extensions::servableFile(addOn, "link.html"));

        // A directory with both an appsscript.json and a manifest.json is a native extension,
        // whose pages can load its .js files:
        Poco::FileOutputStream(Poco::Path(addOn, "manifest.json").toString()) << "{}";
        Poco::FileOutputStream(Poco::Path(addOn, "panel.js").toString()) << "";
        LOK_ASSERT(Extensions::servableFile(addOn, "panel.js").has_value());
    }

    void testDocumentEvent() {
        constexpr std::string_view testname = __func__;

        std::string error;
        auto const listen = Extensions::makeCallScript(
            childRoot, configId, "com.example.remote", "listen", "selectionChanged", error);
        LOK_ASSERT(listen);
        LOK_ASSERT_EQUAL(
            std::string(
                "(function (proxyId) { $internal.listenToDocument(\"selectionChanged\","
                " proxyId); })"),
            listen->text);

        auto const unlisten = Extensions::makeCallScript(
            childRoot, configId, "com.example.remote", "unlisten", "modified", error);
        LOK_ASSERT(unlisten);
        LOK_ASSERT_EQUAL(
            std::string(
                "(function (proxyId) { $internal.stopListeningToDocument(\"modified\","
                " proxyId); })"),
            unlisten->text);

        // Only the built-in document events can be listened to:
        error.clear();
        LOK_ASSERT(!Extensions::makeCallScript(
            childRoot, configId, "com.example.remote", "listen", "disposing", error));
        LOK_ASSERT(!error.empty());
    }

    void testUnknown() {
        constexpr std::string_view testname = __func__;

        std::string error;
        LOK_ASSERT(!Extensions::makeCallScript(
            childRoot, configId, "com.example.remote", "eval", "add", error));
        LOK_ASSERT(!error.empty());
    }

#if ENABLE_DEBUG
    void testBuiltin() {
        constexpr std::string_view testname = __func__;

        // An extension in the directory of the built-in extensions is installed:
        std::string error;
        auto const builtin
            = Extensions::makeCallScript("", "", "com.example.builtin", "remote", "add", error);
        LOK_ASSERT(builtin);
        LOK_ASSERT(builtin->text.find("\"com.example.builtin/remote.js\"") != std::string::npos);

        // An extension preset of the document's configuration (in a directory with the URI-encoded
        // configuration ID as its name) takes the place of the built-in extension with the same ID:
        auto const preset = Extensions::makeCallScript(
            childRoot, configId, "com.example.remote", "remote", "add", error);
        LOK_ASSERT(preset);
        LOK_ASSERT(preset->text.find("\"com.example.remote/remote.js\"") != std::string::npos);

        // Another configuration has no such preset:
        auto const other = Extensions::makeCallScript(
            childRoot, "shared-https_example.com:9980/other", "com.example.remote", "remote", "add",
            error);
        LOK_ASSERT(other);
        LOK_ASSERT(other->text.find("\"com.example.remote/builtin.js\"") != std::string::npos);
    }
#endif

private:
#if ENABLE_DEBUG
    Poco::TemporaryFile builtinDirectory_;
#endif
};

}

CPPUNIT_TEST_SUITE_REGISTRATION(ExtensionsTests);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
