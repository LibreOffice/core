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

#include <sal/config.h>

#include <cstddef>
#include <functional>
#include <map>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <cpo/uno/Reference.hxx>
#include <comphelper/base64.hxx>
#include <comphelper/json.hxx>
#include <comphelper/kit.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/scopeguard.hxx>
#include <config_srcdir.h>
#include <cool.hpp>
#include <cppu/unotype.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <jsuno/jsuno.hxx>
#include <o3tl/safeint.hxx>
#include <osl/file.hxx>
#include <rtl/character.hxx>
#include <rtl/textcvt.h>
#include <rtl/textenc.h>
#include <rtl/uri.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <scriptinterop/XDocument.hpp>
#include <test/unoapi_test.hxx>
#include <tools/stream.hxx>

namespace {

OUString read(OUString const & url) {
    SvFileStream stream(url, StreamMode::READ);
    CPPUNIT_ASSERT_MESSAGE((std::ostringstream() << "cannot open: " << url).str(), stream.good());
    sal_uInt64 const n = stream.remainingSize();
    auto const bytes = read_uInt8s_ToOString(stream, n);
    CPPUNIT_ASSERT_EQUAL(n, sal_uInt64(o3tl::make_unsigned(bytes.getLength())));
    OUString text;
    CPPUNIT_ASSERT_MESSAGE(
        (std::ostringstream() << "not valid UTF-8: " << url).str(),
        rtl_convertStringToUString(
            &text.pData, bytes.getStr(), bytes.getLength(), RTL_TEXTENCODING_UTF8,
            RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
                | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR));
    return text;
}

OUString jsLiteral(std::u16string_view source) {
    OUStringBuffer buf("`");
    for (std::size_t i = 0; i != source.size(); ++i) {
        auto const c = source[i];
        if (c == '\\' || c == '`') {
            buf.append("\\" + OUStringChar(c));
        } else if (c == '$' && i + 1 != source.size() && source[i + 1] == '{') {
            buf.append("\\$");
        } else {
            buf.append(c);
        }
    }
    buf.append('`');
    return buf.makeStringAndClear();
}

class Test: public UnoApiTest {
public:
    Test(): UnoApiTest(u"/scriptinterop/qa/unit/data/"_ustr) {}

protected:
    // A library for the runner's libraries argument, made of files in qa/unit/data, and with the
    // libraries that it declares in turn, as for runScript:
    OUString library(
        std::u16string_view userSymbol, std::vector<std::u16string_view> const & files,
        std::u16string_view libraries)
    {
        OUStringBuffer sources;
        for (auto const & file: files) {
            if (!sources.isEmpty()) {
                sources.append(", ");
            }
            sources.append(jsLiteral(read(createFileURL(file))));
        }
        return "{userSymbol: " + jsLiteral(userSymbol) + ", dir: '', scripts: [], sources: ["
            + sources + "], libraries: [" + libraries + "]}";
    }

    // Makes the frame of the loaded document the active one, which is what getActiveDocument and
    // getActivePresentation resolve against:
    void makeActive() {
        css::frame::Desktop::create(comphelper::getProcessComponentContext())->setActiveFrame(
            cpo::uno::Reference<css::frame::XModel>(mxComponent, cpo::uno::UNO_QUERY_THROW)->
            getCurrentController()->getFrame());
    }

    void loadActiveDocument(std::u16string_view filename) {
        loadFromURL(createFileURL(filename));
        makeActive();
    }

    void loadActivePresentation() {
        mxComponent = loadFromDesktop(u"private:factory/simpress"_ustr);
        makeActive();
    }

    // The libraries are the elements of the runner's libraries argument, as made by library and
    // separated by commas:
    void runScript(
        OUString const & url, std::function<void(OUString const &)> proxyCallHook,
        std::u16string_view libraries)
    {
        OUString gasUrl;
        auto const rc = osl::FileBase::getFileURLFromSystemPath(
            u"" SRC_ROOT "/../browser/extensions/gas-kit-runner.js"_ustr, gasUrl);
        CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, rc);
        OUString const script(
            "var window = globalThis;\n" + read(gasUrl)
            + "\n__gasKitRunner('scriptinterop_document_test', [" + jsLiteral(read(url)) + "], ["
            + jsLiteral(url) + "], 'test', [], 'scriptinterop_test', [" + libraries + "]);");
        try {
            bool usedLegacyUnoApi;
            jsuno::execute(
                script, u"<input>"_ustr, 1,
                [](OUString const & level, OUString const & message) {
                    OUString const msg("console." + level + ": " + message);
                    if (level == u"assert") {
                        CPPUNIT_FAIL((std::ostringstream() << msg).str());
                    } else {
                        std::cout << msg << std::endl;
                    }
                },
                proxyCallHook, &usedLegacyUnoApi);
            CPPUNIT_ASSERT(!usedLegacyUnoApi);
        } catch (jsuno::Exception const & e) {
            std::ostringstream buf;
            buf << e.name << ": " << e.message;
            for (auto const & frame: e.stack) {
                buf << "\n  at " << frame.functionName << " (" << frame.source << ":" << frame.line
                    << ":" << frame.column << ")";
            }
            CPPUNIT_FAIL(buf.str());
        }
    }
};

// Pull the "callId" value out of a proxy-call JSON payload of the shape
//
//   {"proxyId":"...","callId":"...","method":"...","args":[...]}
//
// or return empty when the caller did not attach one (a void-returning method):
std::u16string_view extractCallId(std::u16string_view payload) {
    // The JsonWriter emits '"callId": "..."' with a space after the colon; step over the opening
    // quote of the value rather than fixing the space count in the search key:
    static std::u16string_view const key = u"\"callId\":";
    auto const start = payload.find(key);
    if (start == std::u16string_view::npos) {
        return {};
    }
    auto from = payload.find(u'"', start + key.size());
    if (from == std::u16string_view::npos) {
        return {};
    }
    ++from;
    auto const end = payload.find(u'"', from);
    if (end == std::u16string_view::npos) {
        return {};
    }
    return payload.substr(from, end - from);
}

void appendJsonString(OStringBuffer & buf, OUString const & text) {
    comphelper::appendUnoAsJson(buf, cppu::UnoType<OUString>::get(), &text);
}

bool isUtf8(std::string const & bytes) {
    OUString text;
    return rtl_convertStringToUString(
        &text.pData, bytes.data(), bytes.size(), RTL_TEXTENCODING_UTF8,
        RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR | RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR
            | RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR);
}

std::string formDecode(std::string const & text) {
    return std::string(OUStringToOString(
        rtl::Uri::decode(
            OUString::fromUtf8(text).replace('+', ' '), rtl_UriDecodeWithCharset,
            RTL_TEXTENCODING_UTF8),
        RTL_TEXTENCODING_UTF8));
}

// httpbin reports request header names with each dash-separated part capitalized:
std::string titleCase(std::string name) {
    bool start = true;
    for (auto & c: name) {
        auto const u = static_cast<unsigned char>(c);
        c = static_cast<char>(start ? rtl::toAsciiUpperCase(u) : rtl::toAsciiLowerCase(u));
        start = c == '-';
    }
    return name;
}

// The client side of XClientRuntime.urlFetch, answering the way https://httpbin.org does for the
// URLs that urlfetchapp-test.js uses:
void httpbin(OUString const & payload) {
    std::istringstream in(std::string(payload.toUtf8()));
    boost::property_tree::ptree call;
    boost::property_tree::read_json(in, call);
    CPPUNIT_ASSERT_EQUAL(std::string("urlFetch"), call.get<std::string>("method"));
    std::vector<boost::property_tree::ptree> args;
    for (auto const & arg: call.get_child("args")) {
        args.push_back(arg.second);
    }
    CPPUNIT_ASSERT_EQUAL(std::size_t(8), args.size());
    auto const url = args[0].data();
    auto const method = args[1].data();
    auto const contentType = args[2].data();
    std::string body = args[3].data();
    if (args[4].data() == "true") {
        cpo::uno::Sequence<sal_Int8> bytes;
        comphelper::Base64::decode(bytes, body);
        body.assign(reinterpret_cast<char const *>(bytes.getConstArray()), bytes.getLength());
    }
    sal_Int32 code;
    std::string responseType;
    std::string text;
    OUString error;
    if (url == "https://httpbin.org/anything") {
        boost::property_tree::ptree headers;
        std::vector<std::string> names;
        for (auto const & name: args[5]) {
            names.push_back(name.second.data());
        }
        std::size_t i = 0;
        for (auto const & value: args[6]) {
            headers.push_back({titleCase(names.at(i++)), boost::property_tree::ptree(
                value.second.data())});
        }
        if (!contentType.empty()) {
            headers.push_back({"Content-Type", boost::property_tree::ptree(contentType)});
        }
        boost::property_tree::ptree form;
        std::string data;
        // httpbin decodes a form body that is not UTF-8 into replacement characters, which the
        // test does not look at:
        if (contentType.starts_with("application/x-www-form-urlencoded")) {
            if (!isUtf8(body)) {
                body.clear();
            }
            std::istringstream fields(body);
            std::string field;
            while (std::getline(fields, field, '&')) {
                auto const eq = field.find('=');
                form.push_back({formDecode(field.substr(0, eq)), boost::property_tree::ptree(
                    eq == std::string::npos ? std::string() : formDecode(field.substr(eq + 1)))});
            }
        } else if (isUtf8(body)) {
            data = body;
        } else {
            OStringBuffer b64;
            comphelper::Base64::encode(
                b64, cpo::uno::Sequence<sal_Int8>(
                    reinterpret_cast<sal_Int8 const *>(body.data()), body.size()));
            data = "data:application/octet-stream;base64," + std::string(b64);
        }
        boost::property_tree::ptree echo;
        echo.put("data", data);
        echo.add_child("form", form);
        echo.add_child("headers", headers);
        echo.put("method", method);
        echo.put("url", url);
        std::ostringstream out;
        boost::property_tree::write_json(out, echo, false);
        code = 200;
        responseType = "application/json";
        text = out.str();
        // write_json writes an empty object as "", where httpbin writes {}:
        if (auto const pos = text.find("\"form\":\"\""); pos != std::string::npos) {
            text.replace(pos, 9, "\"form\":{}");
        }
    } else if (url == "https://httpbin.org/image/png") {
        code = 200;
        responseType = "image/png";
        text = "\x89PNG\r\n\x1a\n";
    } else if (url == "https://httpbin.org/status/404") {
        code = 404;
        responseType = "text/html; charset=utf-8";
    } else {
        code = 0;
        error = u"Failed to fetch"_ustr;
    }
    OStringBuffer buf("{\"code\":" + OString::number(code) + ",\"headerNames\":[");
    if (!responseType.empty()) {
        buf.append("\"content-type\"");
    }
    buf.append("],\"headerValues\":[");
    if (!responseType.empty()) {
        appendJsonString(buf, OUString::fromUtf8(responseType));
    }
    buf.append("],\"body\":\"");
    comphelper::Base64::encode(
        buf, cpo::uno::Sequence<sal_Int8>(
            reinterpret_cast<sal_Int8 const *>(text.data()), text.size()));
    buf.append("\",\"error\":");
    appendJsonString(buf, error);
    buf.append('}');
    jsuno::deliverProxyResult(
        OUString::fromUtf8(call.get<std::string>("callId")), OUString::fromUtf8(buf));
}

// The client side of XClientRuntime.cacheGet, cachePut and cacheRemove, which keeps the entries in
// memory and checks the expiration that cacheservice-test.js expects for some of its keys:
void cache(OUString const & payload) {
    static std::map<std::string, std::pair<std::string, std::string>> entries;
    std::istringstream in(std::string(payload.toUtf8()));
    boost::property_tree::ptree call;
    boost::property_tree::read_json(in, call);
    std::vector<std::string> args;
    for (auto const & arg: call.get_child("args")) {
        args.push_back(arg.second.data());
    }
    auto const method = call.get<std::string>("method");
    auto const key = args.at(0) + '\n' + args.at(1);
    if (method == "cacheGet") {
        OStringBuffer buf;
        auto const i = entries.find(key);
        if (i == entries.end()) {
            buf.append("{\"IsPresent\":false,\"Value\":\"\"}");
        } else {
            buf.append("{\"IsPresent\":true,\"Value\":");
            appendJsonString(buf, OUString::fromUtf8(i->second.first));
            buf.append('}');
        }
        jsuno::deliverProxyResult(
            OUString::fromUtf8(call.get<std::string>("callId")), OUString::fromUtf8(buf));
    } else if (method == "cachePut") {
        for (auto const & [suffix, ttl]:
             {std::pair{"-ttl-default", "600"}, {"-ttl-zero", "600"}, {"-ttl-big", "21600"}})
        {
            if (args.at(1).ends_with(suffix)) {
                CPPUNIT_ASSERT_EQUAL(std::string(ttl), args.at(3));
            }
        }
        entries[key] = {args.at(2), args.at(3)};
    } else {
        CPPUNIT_ASSERT_EQUAL(std::string("cacheRemove"), method);
        entries.erase(key);
    }
}

CPPUNIT_TEST_FIXTURE(Test, testDocument) {
    loadActiveDocument(u"document-test.rtf");
    runScript(createFileURL(u"document-test.js"), {}, u"");
}

CPPUNIT_TEST_FIXTURE(Test, testUtilities) {
    runScript(createFileURL(u"utilities-test.js"), {}, u"");
}

CPPUNIT_TEST_FIXTURE(Test, testCacheService) {
    runScript(createFileURL(u"cacheservice-test.js"), cache, u"");
}

CPPUNIT_TEST_FIXTURE(Test, testLibrary) {
    OUString const inner(library(u"Inner", {u"library-test/Inner.gs"}, u""));
    OUString const libraries(
        library(u"TestLibrary", {u"library-test/Main.gs", u"library-test/Helpers.gs"}, inner));
    runScript(createFileURL(u"library-test.js"), {}, libraries);
}

CPPUNIT_TEST_FIXTURE(Test, testScriptApp) {
    runScript(createFileURL(u"scriptapp-test.js"), {}, u"");
}

CPPUNIT_TEST_FIXTURE(Test, testUrlFetchApp) {
    runScript(createFileURL(u"urlfetchapp-test.js"), httpbin, u"");
}

CPPUNIT_TEST_FIXTURE(Test, testPropertiesService) {
    // The gas-kit-runner's PropertiesService facade calls XClientRuntime.userPropGetProperty for
    // every property read; the proxy hook plays the client side of that call and returns an absent
    // Optional<string> so `getProperty` should hand back null:
    runScript(
        createFileURL(u"propertiesservice-test.js"),
        [](OUString const & payload) {
            auto const callId = extractCallId(payload);
            if (!callId.empty()) {
                jsuno::deliverProxyResult(
                    OUString(callId), u"{\"IsPresent\":false,\"Value\":\"\"}"_ustr);
            }
        },
        u"");
}

CPPUNIT_TEST_FIXTURE(Test, testSession) {
    // The kit's language is one with a script and a region while the script runs:
    LanguageTag const language(comphelper::COKit::getLanguageTag());
    comphelper::COKit::setLanguageTag(LanguageTag(u"sr-Latn-RS"_ustr));
    comphelper::ScopeGuard const restore(
        [&language] { comphelper::COKit::setLanguageTag(language); });
    runScript(createFileURL(u"session-test.js"), {}, u"");
}

CPPUNIT_TEST_FIXTURE(Test, testSlidesApp) {
    loadActivePresentation();
    runScript(createFileURL(u"slidesapp-test.js"), {}, u"");
}

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
