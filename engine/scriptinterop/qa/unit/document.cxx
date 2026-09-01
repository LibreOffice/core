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
#include <iostream>
#include <sstream>
#include <string_view>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <comphelper/processfactory.hxx>
#include <config_srcdir.h>
#include <cool.hpp>
#include <jsuno/jsuno.hxx>
#include <o3tl/safeint.hxx>
#include <osl/file.hxx>
#include <rtl/textcvt.h>
#include <rtl/textenc.h>
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
    void loadActiveDocument(std::u16string_view filename) {
        loadFromURL(createFileURL(filename));
        css::frame::Desktop::create(comphelper::getProcessComponentContext())->setActiveFrame(
            css::uno::Reference<css::frame::XModel>(mxComponent, css::uno::UNO_QUERY_THROW)->
            getCurrentController()->getFrame());
    }

    void runScript(OUString const & url, std::u16string_view entry) {
        OUString gasUrl;
        auto const rc = osl::FileBase::getFileURLFromSystemPath(
            u"" SRC_ROOT "/../browser/extensions/gas-kit-runner.js"_ustr, gasUrl);
        CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, rc);
        OUString const script(
            "var window = globalThis;\n" + read(gasUrl)
            + "\n__gasKitRunner('scriptinterop_document_test', [" + jsLiteral(read(url)) + "], ["
            + jsLiteral(url) + "], " + jsLiteral(entry) + ", []);");
        try {
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
                {}, nullptr);
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

CPPUNIT_TEST_FIXTURE(Test, testDocument) {
    loadActiveDocument(u"document-test.rtf");
    runScript(createFileURL(u"document-test.js"), u"documentTest");
}

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
