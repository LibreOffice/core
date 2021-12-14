/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <cassert>
#include <cstddef>

#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/ucb/Command.hpp>
#include <com/sun/star/ucb/IllegalIdentifierException.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uri/XUriReference.hpp>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <rtl/strbuf.hxx>
#include <rtl/string.h>
#include <rtl/string.hxx>
#include <rtl/textenc.h>
#include <rtl/ustring.hxx>
#include <sal/macros.h>
#include <sal/types.h>
#include <svl/urihelper.hxx>
#include <unotools/charclass.hxx>

namespace com::sun::star::ucb {
    class XCommandEnvironment;
    class XContentEventListener;
}

namespace {

// This class only implements that subset of functionality of a proper
// css::ucb::Content that is known to be needed here:
class Content:
    public cppu::WeakImplHelper<
        css::ucb::XContent, css::ucb::XCommandProcessor >
{
public:
    explicit Content(
        css::uno::Reference< css::ucb::XContentIdentifier > const & identifier);

    virtual css::uno::Reference< css::ucb::XContentIdentifier > SAL_CALL
    getIdentifier() override {
        return m_identifier;
    }

    virtual OUString SAL_CALL getContentType() override
    {
        return OUString();
    }

    virtual void SAL_CALL addContentEventListener(
        css::uno::Reference< css::ucb::XContentEventListener > const &) override
    {}

    virtual void SAL_CALL removeContentEventListener(
        css::uno::Reference< css::ucb::XContentEventListener > const &) override
    {}

    virtual sal_Int32 SAL_CALL createCommandIdentifier() override
    {
        return 0;
    }

    virtual css::uno::Any SAL_CALL execute(
        css::ucb::Command const & command, sal_Int32 commandId,
        css::uno::Reference< css::ucb::XCommandEnvironment > const &) override;

    virtual void SAL_CALL abort(sal_Int32) override {}

private:
    static char const m_prefix[];

    css::uno::Reference< css::ucb::XContentIdentifier > m_identifier;
};

char const Content::m_prefix[] = "test:";

Content::Content(
    css::uno::Reference< css::ucb::XContentIdentifier > const & identifier):
    m_identifier(identifier)
{
    assert(m_identifier.is());
    OUString uri(m_identifier->getContentIdentifier());
    if (!uri.matchIgnoreAsciiCase(m_prefix)
        || uri.indexOf('#', RTL_CONSTASCII_LENGTH(m_prefix)) != -1)
    {
        throw css::ucb::IllegalIdentifierException();
    }
}

css::uno::Any Content::execute(
    css::ucb::Command const & command, sal_Int32,
    css::uno::Reference< css::ucb::XCommandEnvironment > const &)
{
    if ( command.Name != "getCasePreservingURL" )
    {
        throw css::uno::RuntimeException();
    }
    // If any non-empty segment starts with anything but '0', '1', or '2', fail;
    // otherwise, if the last non-empty segment starts with '1', add a final
    // slash, and if the last non-empty segment starts with '2', remove a final
    // slash (if any); also, turn the given uri into all-lowercase:
    OUString uri(m_identifier->getContentIdentifier());
    sal_Unicode c = '0';
    for (sal_Int32 i = RTL_CONSTASCII_LENGTH(m_prefix); i != -1;) {
        OUString seg(uri.getToken(0, '/', i));
        if (seg.getLength() > 0) {
            c = seg[0];
            if (c < '0' || c > '2') {
                throw css::uno::Exception();
            }
        }
    }
    switch (c) {
    case '1':
        uri += "/";
        break;
    case '2':
        if (uri.endsWith("/")) {
            uri = uri.copy(0, uri.getLength() -1);
        }
        break;
    }
    return css::uno::makeAny(uri.toAsciiLowerCase());
}

class Provider: public cppu::WeakImplHelper< css::ucb::XContentProvider > {
public:
    virtual css::uno::Reference< css::ucb::XContent > SAL_CALL queryContent(
        css::uno::Reference< css::ucb::XContentIdentifier > const & identifier) override
    {
        return new Content(identifier);
    }

    virtual sal_Int32 SAL_CALL compareContentIds(
        css::uno::Reference< css::ucb::XContentIdentifier > const & id1,
        css::uno::Reference< css::ucb::XContentIdentifier > const & id2) override
    {
        assert(id1.is() && id2.is());
        return
            id1->getContentIdentifier().compareTo(id2->getContentIdentifier());
    }
};

class Test: public CppUnit::TestFixture {
public:
    virtual void setUp() override;

    void finish();

    void testNormalizedMakeRelative();

    void testFindFirstURLInText();

    void testResolveIdnaHost();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testNormalizedMakeRelative);
    CPPUNIT_TEST(testFindFirstURLInText);
    CPPUNIT_TEST(testResolveIdnaHost);
    CPPUNIT_TEST(finish);
    CPPUNIT_TEST_SUITE_END();

private:
    static css::uno::Reference< css::uno::XComponentContext > m_context;
};

void Test::setUp() {
    // For whatever reason, on W32 it does not work to create/destroy a fresh
    // component context for each test in Test::setUp/tearDown; therefore, a
    // single component context is used for all tests and destroyed in the last
    // pseudo-test "finish":
    if (!m_context.is()) {
        m_context = cppu::defaultBootstrap_InitialComponentContext();
    }
}

void Test::finish() {
    css::uno::Reference< css::lang::XComponent >(
        m_context, css::uno::UNO_QUERY_THROW)->dispose();
}

void Test::testNormalizedMakeRelative() {
    auto ucb(css::ucb::UniversalContentBroker::create(m_context));
    ucb->registerContentProvider(new Provider, "test", true);
    ucb->registerContentProvider(
        css::uno::Reference<css::ucb::XContentProvider>(
            m_context->getServiceManager()->createInstanceWithContext(
                "com.sun.star.comp.ucb.FileProvider", m_context),
            css::uno::UNO_QUERY_THROW),
        "file", true);
    struct Data {
        char const * base;
        char const * absolute;
        char const * relative;
    };
    static Data const tests[] = {
        { "hierarchical:/", "mailto:def@a.b.c.", "mailto:def@a.b.c." },
        { "hierarchical:/", "a/b/c", "a/b/c" },
        { "hierarchical:/a", "hierarchical:/a/b/c?d#e", "/a/b/c?d#e" },
        { "hierarchical:/a/", "hierarchical:/a/b/c?d#e", "b/c?d#e" },
        { "test:/0/0/a", "test:/0/b", "../b" },
        { "test:/1/1/a", "test:/1/b", "../b" },
        { "test:/2/2//a", "test:/2/b", "../../b" },
        { "test:/0a/b", "test:/0A/c#f", "c#f" },
        { "file:///usr/bin/nonex1/nonex2",
          "file:///usr/bin/nonex1/nonex3/nonex4", "nonex3/nonex4" },
        { "file:///usr/bin/nonex1/nonex2#fragmentA",
          "file:///usr/bin/nonex1/nonex3/nonex4#fragmentB",
          "nonex3/nonex4#fragmentB" },
        { "file:///usr/nonex1/nonex2", "file:///usr/nonex3", "../nonex3" },
        { "file:///c:/windows/nonex1", "file:///c:/nonex2", "../nonex2" },
#if defined(_WIN32)
        { "file:///c:/nonex1/nonex2", "file:///C:/nonex1/nonex3/nonex4",
          "nonex3/nonex4" }
#endif
    };
    for (std::size_t i = 0; i < SAL_N_ELEMENTS(tests); ++i) {
        css::uno::Reference< css::uri::XUriReference > ref(
            URIHelper::normalizedMakeRelative(
                m_context, OUString::createFromAscii(tests[i].base),
                OUString::createFromAscii(tests[i].absolute)));
        bool ok = tests[i].relative == nullptr
            ? !ref.is()
            : ref.is() && ref->getUriReference().equalsAscii(tests[i].relative);
        OString msg;
        if (!ok) {
            OStringBuffer buf;
            buf.append('<');
            buf.append(tests[i].base);
            buf.append(">, <");
            buf.append(tests[i].absolute);
            buf.append(">: ");
            if (ref.is()) {
                buf.append('<');
                buf.append(
                    OUStringToOString(
                        ref->getUriReference(), RTL_TEXTENCODING_UTF8));
                buf.append('>');
            } else {
                buf.append("none");
            }
            buf.append(" instead of ");
            if (tests[i].relative == nullptr) {
                buf.append("none");
            } else {
                buf.append('<');
                buf.append(tests[i].relative);
                buf.append('>');
            }
            msg = buf.makeStringAndClear();
        }
        CPPUNIT_ASSERT_MESSAGE(msg.getStr(), ok);
    }
}

void Test::testFindFirstURLInText() {
    struct Data {
        char const * input;
        char const * result;
        sal_Int32 begin;
        sal_Int32 end;
    };
    static Data const tests[] = {
        { "...ftp://bla.bla.bla/blubber/...",
          "ftp://bla.bla.bla/blubber/", 3, 29 },
        { "..\\ftp://bla.bla.bla/blubber/...", nullptr, 0, 0 },
        { "..\\ftp:\\\\bla.bla.bla\\blubber/...",
          "file://bla.bla.bla/blubber%2F", 7, 29 },
        { "http://sun.com", "http://sun.com/", 0, 14 },
        { "http://sun.com/", "http://sun.com/", 0, 15 },
        { "http://www.xerox.com@www.pcworld.com/go/3990332.htm", nullptr, 0, 0 },
        { "ftp://www.xerox.com@www.pcworld.com/go/3990332.htm",
          "ftp://www.xerox.com@www.pcworld.com/go/3990332.htm", 0, 50 },
        { "Version.1.2.3", nullptr, 0, 0 },
        { "Version:1.2.3", nullptr, 0, 0 },
        { "a.b.c", nullptr, 0, 0 },
        { "file:///a|...", "file:///a:", 0, 10 },
        { "file:///a||...", "file:///a%7C%7C", 0, 11 },
        { "file:///a|/bc#...", "file:///a:/bc", 0, 13 },
        { "file:///a|/bc#de...", "file:///a:/bc#de", 0, 16 },
        { "abc.def.ghi,ftp.xxx.yyy/zzz...", "ftp://ftp.xxx.yyy/zzz", 12, 27 },
        { "abc.def.ghi,Ftp.xxx.yyy/zzz...", "ftp://Ftp.xxx.yyy/zzz", 12, 27 },
        { "abc.def.ghi,www.xxx.yyy...", "http://www.xxx.yyy/", 12, 23 },
        { "abc.def.ghi,wwww.xxx.yyy...", nullptr, 0, 0 },
        { "abc.def.ghi,wWW.xxx.yyy...", "http://wWW.xxx.yyy/", 12, 23 },
        { "Bla {mailto.me@abc.def.g.h.i}...",
          "mailto:%7Bmailto.me@abc.def.g.h.i", 4, 28 },
        { "abc@def@ghi", nullptr, 0, 0 },
        { "lala@sun.com", "mailto:lala@sun.com", 0, 12 },
        { "1lala@sun.com", "mailto:1lala@sun.com", 0, 13 },
        { "aaa_bbb@xxx.yy", "mailto:aaa_bbb@xxx.yy", 0, 14 },
        { "{a:\\bla/bla/bla...}", "file:///a:/bla/bla/bla", 1, 15 },
        { "#b:/c/d#e#f#", "file:///b:/c/d", 1, 7 },
        { "a:/", "file:///a:/", 0, 3 },
        { "http://sun.com/R_(l_a)", "http://sun.com/R_(l_a)", 0, 22 },
        { ".component:", nullptr, 0, 0 },
        { ".uno:", nullptr, 0, 0 },
        { "cid:", nullptr, 0, 0 },
        { "data:", nullptr, 0, 0 },
        { "db:", nullptr, 0, 0 },
        { "file:", nullptr, 0, 0 },
        { "ftp:", nullptr, 0, 0 },
        { "http:", nullptr, 0, 0 },
        { "https:", nullptr, 0, 0 },
        { "imap:", nullptr, 0, 0 },
        { "javascript:", nullptr, 0, 0 },
        { "ldap:", nullptr, 0, 0 },
        { "macro:", nullptr, 0, 0 },
        { "mailto:", nullptr, 0, 0 },
        { "news:", nullptr, 0, 0 },
        { "out:", nullptr, 0, 0 },
        { "pop3:", nullptr, 0, 0 },
        { "private:", nullptr, 0, 0 },
        { "slot:", nullptr, 0, 0 },
        { "staroffice.component:", nullptr, 0, 0 },
        { "staroffice.db:", nullptr, 0, 0 },
        { "staroffice.factory:", nullptr, 0, 0 },
        { "staroffice.helpid:", nullptr, 0, 0 },
        { "staroffice.java:", nullptr, 0, 0 },
        { "staroffice.macro:", nullptr, 0, 0 },
        { "staroffice.out:", nullptr, 0, 0 },
        { "staroffice.pop3:", nullptr, 0, 0 },
        { "staroffice.private:", nullptr, 0, 0 },
        { "staroffice.searchfolder:", nullptr, 0, 0 },
        { "staroffice.slot:", nullptr, 0, 0 },
        { "staroffice.trashcan:", nullptr, 0, 0 },
        { "staroffice.uno:", nullptr, 0, 0 },
        { "staroffice.vim:", nullptr, 0, 0 },
        { "staroffice:", nullptr, 0, 0 },
        { "vim:", nullptr, 0, 0 },
        { "vnd.sun.star.cmd:", nullptr, 0, 0 },
        { "vnd.sun.star.help:", nullptr, 0, 0 },
        { "vnd.sun.star.hier:", nullptr, 0, 0 },
        { "vnd.sun.star.pkg:", nullptr, 0, 0 },
        { "vnd.sun.star.script:", nullptr, 0, 0 },
        { "vnd.sun.star.webdav:", nullptr, 0, 0 },
        { "vnd.sun.star.wfs:", nullptr, 0, 0 },
        { "generic:path", nullptr, 0, 0 },
        { "wfs:", nullptr, 0, 0 }
    };
    CharClass charClass( m_context, LanguageTag( css::lang::Locale("en", "US", "")));
    for (std::size_t i = 0; i < SAL_N_ELEMENTS(tests); ++i) {
        OUString input(OUString::createFromAscii(tests[i].input));
        sal_Int32 begin = 0;
        sal_Int32 end = input.getLength();
        OUString result(
            URIHelper::FindFirstURLInText(input, begin, end, charClass));
        bool ok = tests[i].result == nullptr
            ? (result.getLength() == 0 && begin == input.getLength()
               && end == input.getLength())
            : (result.equalsAscii(tests[i].result) && begin == tests[i].begin
               && end == tests[i].end);
        OString msg;
        if (!ok) {
            OStringBuffer buf;
            buf.append('"');
            buf.append(tests[i].input);
            buf.append("\" -> ");
            buf.append(tests[i].result == nullptr ? "none" : tests[i].result);
            buf.append(" (");
            buf.append(tests[i].begin);
            buf.append(", ");
            buf.append(tests[i].end);
            buf.append(')');
            buf.append(" != ");
            buf.append(OUStringToOString(result, RTL_TEXTENCODING_UTF8));
            buf.append(" (");
            buf.append(begin);
            buf.append(", ");
            buf.append(end);
            buf.append(')');
            msg = buf.makeStringAndClear();
        }
        CPPUNIT_ASSERT_MESSAGE(msg.getStr(), ok);
    }
}

void Test::testResolveIdnaHost() {
    OUString input;

    input.clear();
    CPPUNIT_ASSERT_EQUAL(input, URIHelper::resolveIdnaHost(input));

    input = u"Foo.M\u00FCnchen.de";
    CPPUNIT_ASSERT_EQUAL(input, URIHelper::resolveIdnaHost(input));

    input = "foo://Muenchen.de";
    CPPUNIT_ASSERT_EQUAL(input, URIHelper::resolveIdnaHost(input));

    input = u"foo://-M\u00FCnchen.de";
    CPPUNIT_ASSERT_EQUAL(input, URIHelper::resolveIdnaHost(input));

    input = u"foo://M\u00FCnchen-.de";
    CPPUNIT_ASSERT_EQUAL(input, URIHelper::resolveIdnaHost(input));

    input = u"foo://xn--M\u00FCnchen.de";
    CPPUNIT_ASSERT_EQUAL(input, URIHelper::resolveIdnaHost(input));

    input = u"foo://xy--M\u00FCnchen.de";
    CPPUNIT_ASSERT_EQUAL(input, URIHelper::resolveIdnaHost(input));

    input = u"foo://.M\u00FCnchen.de";
    CPPUNIT_ASSERT_EQUAL(input, URIHelper::resolveIdnaHost(input));

    input = u"foo://-bar.M\u00FCnchen.de";
    CPPUNIT_ASSERT_EQUAL(input, URIHelper::resolveIdnaHost(input));

    input = u"foo://bar-.M\u00FCnchen.de";
    CPPUNIT_ASSERT_EQUAL(input, URIHelper::resolveIdnaHost(input));

    input = u"foo://xn--bar.M\u00FCnchen.de";
    CPPUNIT_ASSERT_EQUAL(input, URIHelper::resolveIdnaHost(input));

    input = u"foo://xy--bar.M\u00FCnchen.de";
    CPPUNIT_ASSERT_EQUAL(input, URIHelper::resolveIdnaHost(input));

    CPPUNIT_ASSERT_EQUAL(
        OUString(u"foo://M\u00FCnchen@xn--mnchen-3ya.de"),
        URIHelper::resolveIdnaHost(u"foo://M\u00FCnchen@M\u00FCnchen.de"));

    CPPUNIT_ASSERT_EQUAL(
        OUString("foo://xn--mnchen-3ya.de."),
        URIHelper::resolveIdnaHost(u"foo://M\u00FCnchen.de."));

    CPPUNIT_ASSERT_EQUAL(
        OUString("Foo://bar@xn--mnchen-3ya.de:123/?bar#baz"),
        URIHelper::resolveIdnaHost(u"Foo://bar@M\u00FCnchen.de:123/?bar#baz"));

    CPPUNIT_ASSERT_EQUAL(
        OUString("foo://xn--mnchen-3ya.de"),
        URIHelper::resolveIdnaHost(u"foo://Mu\u0308nchen.de"));

    CPPUNIT_ASSERT_EQUAL(
        OUString("foo://example.xn--m-eha"), URIHelper::resolveIdnaHost(u"foo://example.mü"));

    CPPUNIT_ASSERT_EQUAL(
        OUString("foo://example.xn--m-eha:0"), URIHelper::resolveIdnaHost(u"foo://example.mü:0"));

    CPPUNIT_ASSERT_EQUAL(
        OUString("foo://xn--e1afmkfd.xn--p1ai"), URIHelper::resolveIdnaHost(u"foo://пример.рф"));

    CPPUNIT_ASSERT_EQUAL(
        OUString("foo://xn--e1afmkfd.xn--p1ai:0"),
        URIHelper::resolveIdnaHost(u"foo://пример.рф:0"));
}

css::uno::Reference< css::uno::XComponentContext > Test::m_context;

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
