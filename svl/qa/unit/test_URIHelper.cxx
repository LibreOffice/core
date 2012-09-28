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

#include "com/sun/star/lang/Locale.hpp"
#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/ucb/Command.hpp"
#include "com/sun/star/ucb/CommandAbortedException.hpp"
#include "com/sun/star/ucb/IllegalIdentifierException.hpp"
#include "com/sun/star/ucb/UniversalContentBroker.hpp"
#include "com/sun/star/ucb/XCommandProcessor.hpp"
#include "com/sun/star/ucb/XContent.hpp"
#include "com/sun/star/ucb/XContentIdentifier.hpp"
#include "com/sun/star/ucb/XContentProvider.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uri/XUriReference.hpp"
#include "cppuhelper/bootstrap.hxx"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/implbase2.hxx"
#include "cppunit/TestCase.h"
#include "cppunit/TestFixture.h"
#include "cppunit/TestSuite.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"
#include "rtl/strbuf.hxx"
#include "rtl/string.h"
#include "rtl/string.hxx"
#include "rtl/textenc.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/macros.h"
#include "sal/types.h"
#include "svl/urihelper.hxx"
#include "tools/solar.h"
#include "unotools/charclass.hxx"

namespace com { namespace sun { namespace star { namespace ucb {
    class XCommandEnvironment;
    class XContentEventListener;
} } } }

namespace {

namespace css = com::sun::star;

// This class only implements that subset of functionality of a proper
// css::ucb::Content that is known to be needed here:
class Content:
    public cppu::WeakImplHelper2<
        css::ucb::XContent, css::ucb::XCommandProcessor >
{
public:
    explicit Content(
        css::uno::Reference< css::ucb::XContentIdentifier > const & identifier);

    virtual css::uno::Reference< css::ucb::XContentIdentifier > SAL_CALL
    getIdentifier() throw (css::uno::RuntimeException) {
        return m_identifier;
    }

    virtual rtl::OUString SAL_CALL getContentType()
        throw (css::uno::RuntimeException)
    {
        return rtl::OUString();
    }

    virtual void SAL_CALL addContentEventListener(
        css::uno::Reference< css::ucb::XContentEventListener > const &)
        throw (css::uno::RuntimeException)
    {}

    virtual void SAL_CALL removeContentEventListener(
        css::uno::Reference< css::ucb::XContentEventListener > const &)
        throw (css::uno::RuntimeException)
    {}

    virtual sal_Int32 SAL_CALL createCommandIdentifier()
        throw (css::uno::RuntimeException)
    {
        return 0;
    }

    virtual css::uno::Any SAL_CALL execute(
        css::ucb::Command const & command, sal_Int32 commandId,
        css::uno::Reference< css::ucb::XCommandEnvironment > const &)
        throw (
            css::uno::Exception, css::ucb::CommandAbortedException,
            css::uno::RuntimeException);

    virtual void SAL_CALL abort(sal_Int32) throw (css::uno::RuntimeException) {}

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
    rtl::OUString uri(m_identifier->getContentIdentifier());
    if (!uri.matchIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM(m_prefix))
        || uri.indexOf('#', RTL_CONSTASCII_LENGTH(m_prefix)) != -1)
    {
        throw css::ucb::IllegalIdentifierException();
    }
}

css::uno::Any Content::execute(
    css::ucb::Command const & command, sal_Int32,
    css::uno::Reference< css::ucb::XCommandEnvironment > const &)
    throw (
        css::uno::Exception, css::ucb::CommandAbortedException,
        css::uno::RuntimeException)
{
    if ( command.Name != "getCasePreservingURL" )
    {
        throw css::uno::RuntimeException();
    }
    // If any non-empty segment starts with anything but '0', '1', or '2', fail;
    // otherwise, if the last non-empty segment starts with '1', add a final
    // slash, and if the last non-empty segment starts with '2', remove a final
    // slash (if any); also, turn the given uri into all-lowercase:
    rtl::OUString uri(m_identifier->getContentIdentifier());
    sal_Unicode c = '0';
    for (sal_Int32 i = RTL_CONSTASCII_LENGTH(m_prefix); i != -1;) {
        rtl::OUString seg(uri.getToken(0, '/', i));
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
        if (uri.getLength() > 0 && uri[uri.getLength() - 1] == '/') {
            uri = uri.copy(0, uri.getLength() -1);
        }
        break;
    }
    return css::uno::makeAny(uri.toAsciiLowerCase());
}

class Provider: public cppu::WeakImplHelper1< css::ucb::XContentProvider > {
public:
    virtual css::uno::Reference< css::ucb::XContent > SAL_CALL queryContent(
        css::uno::Reference< css::ucb::XContentIdentifier > const & identifier)
        throw (css::ucb::IllegalIdentifierException, css::uno::RuntimeException)
    {
        return new Content(identifier);
    }

    virtual sal_Int32 SAL_CALL compareContentIds(
        css::uno::Reference< css::ucb::XContentIdentifier > const & id1,
        css::uno::Reference< css::ucb::XContentIdentifier > const & id2)
        throw (css::uno::RuntimeException)
    {
        assert(id1.is() && id2.is());
        return
            id1->getContentIdentifier().compareTo(id2->getContentIdentifier());
    }
};

class Test: public CppUnit::TestFixture {
public:
    virtual void setUp();

    void finish();

    void testNormalizedMakeRelative();

    void testFindFirstURLInText();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testNormalizedMakeRelative);
    CPPUNIT_TEST(testFindFirstURLInText);
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
    css::ucb::UniversalContentBroker::create(m_context)->
        registerContentProvider(
            new Provider, rtl::OUString("test"),
            true);
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
#if defined WNT
        { "file:///c:/nonex1/nonex2", "file:///C:/nonex1/nonex3/nonex4",
          "nonex3/nonex4" }
#endif
    };
    for (std::size_t i = 0; i < SAL_N_ELEMENTS(tests); ++i) {
        css::uno::Reference< css::uri::XUriReference > ref(
            URIHelper::normalizedMakeRelative(
                m_context, rtl::OUString::createFromAscii(tests[i].base),
                rtl::OUString::createFromAscii(tests[i].absolute)));
        bool ok = tests[i].relative == 0
            ? !ref.is()
            : ref.is() && ref->getUriReference().equalsAscii(tests[i].relative);
        rtl::OString msg;
        if (!ok) {
            rtl::OStringBuffer buf;
            buf.append('<');
            buf.append(tests[i].base);
            buf.append(RTL_CONSTASCII_STRINGPARAM(">, <"));
            buf.append(tests[i].absolute);
            buf.append(RTL_CONSTASCII_STRINGPARAM(">: "));
            if (ref.is()) {
                buf.append('<');
                buf.append(
                    rtl::OUStringToOString(
                        ref->getUriReference(), RTL_TEXTENCODING_UTF8));
                buf.append('>');
            } else {
                buf.append(RTL_CONSTASCII_STRINGPARAM("none"));
            }
            buf.append(RTL_CONSTASCII_STRINGPARAM(" instead of "));
            if (tests[i].relative == 0) {
                buf.append(RTL_CONSTASCII_STRINGPARAM("none"));
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
        xub_StrLen begin;
        xub_StrLen end;
    };
    static Data const tests[] = {
        { "...ftp://bla.bla.bla/blubber/...",
          "ftp://bla.bla.bla/blubber/", 3, 29 },
        { "..\\ftp://bla.bla.bla/blubber/...", 0, 0, 0 },
        { "..\\ftp:\\\\bla.bla.bla\\blubber/...",
//Sync with tools/source/fsys/urlobj.cxx and changeScheme
#ifdef LINUX
          "smb://bla.bla.bla/blubber%2F", 7, 29 },
#else
          "file://bla.bla.bla/blubber%2F", 7, 29 },
#endif
        { "http://sun.com", "http://sun.com/", 0, 14 },
        { "http://sun.com/", "http://sun.com/", 0, 15 },
        { "http://www.xerox.com@www.pcworld.com/go/3990332.htm", 0, 0, 0 },
        { "ftp://www.xerox.com@www.pcworld.com/go/3990332.htm",
          "ftp://www.xerox.com@www.pcworld.com/go/3990332.htm", 0, 50 },
        { "Version.1.2.3", 0, 0, 0 },
        { "Version:1.2.3", 0, 0, 0 },
        { "a.b.c", 0, 0, 0 },
        { "file:///a|...", "file:///a:", 0, 10 },
        { "file:///a||...", "file:///a%7C%7C", 0, 11 },
        { "file:///a|/bc#...", "file:///a:/bc", 0, 13 },
        { "file:///a|/bc#de...", "file:///a:/bc#de", 0, 16 },
        { "abc.def.ghi,ftp.xxx.yyy/zzz...", "ftp://ftp.xxx.yyy/zzz", 12, 27 },
        { "abc.def.ghi,Ftp.xxx.yyy/zzz...", "ftp://Ftp.xxx.yyy/zzz", 12, 27 },
        { "abc.def.ghi,www.xxx.yyy...", "http://www.xxx.yyy/", 12, 23 },
        { "abc.def.ghi,wwww.xxx.yyy...", 0, 0, 0 },
        { "abc.def.ghi,wWW.xxx.yyy...", "http://wWW.xxx.yyy/", 12, 23 },
        { "Bla {mailto.me@abc.def.g.h.i}...",
          "mailto:%7Bmailto.me@abc.def.g.h.i", 4, 28 },
        { "abc@def@ghi", 0, 0, 0 },
        { "lala@sun.com", "mailto:lala@sun.com", 0, 12 },
        { "1lala@sun.com", "mailto:1lala@sun.com", 0, 13 },
        { "aaa_bbb@xxx.yy", "mailto:aaa_bbb@xxx.yy", 0, 14 },
        { "{a:\\bla/bla/bla...}", "file:///a:/bla/bla/bla", 1, 15 },
        { "#b:/c/d#e#f#", "file:///b:/c/d", 1, 7 },
        { "a:/", "file:///a:/", 0, 3 },
        { ".component:", 0, 0, 0 },
        { ".uno:", 0, 0, 0 },
        { "cid:", 0, 0, 0 },
        { "data:", 0, 0, 0 },
        { "db:", 0, 0, 0 },
        { "file:", 0, 0, 0 },
        { "ftp:", 0, 0, 0 },
        { "http:", 0, 0, 0 },
        { "https:", 0, 0, 0 },
        { "imap:", 0, 0, 0 },
        { "javascript:", 0, 0, 0 },
        { "ldap:", 0, 0, 0 },
        { "macro:", 0, 0, 0 },
        { "mailto:", 0, 0, 0 },
        { "news:", 0, 0, 0 },
        { "out:", 0, 0, 0 },
        { "pop3:", 0, 0, 0 },
        { "private:", 0, 0, 0 },
        { "slot:", 0, 0, 0 },
        { "staroffice.component:", 0, 0, 0 },
        { "staroffice.db:", 0, 0, 0 },
        { "staroffice.factory:", 0, 0, 0 },
        { "staroffice.helpid:", 0, 0, 0 },
        { "staroffice.java:", 0, 0, 0 },
        { "staroffice.macro:", 0, 0, 0 },
        { "staroffice.out:", 0, 0, 0 },
        { "staroffice.pop3:", 0, 0, 0 },
        { "staroffice.private:", 0, 0, 0 },
        { "staroffice.searchfolder:", 0, 0, 0 },
        { "staroffice.slot:", 0, 0, 0 },
        { "staroffice.trashcan:", 0, 0, 0 },
        { "staroffice.uno:", 0, 0, 0 },
        { "staroffice.vim:", 0, 0, 0 },
        { "staroffice:", 0, 0, 0 },
        { "vim:", 0, 0, 0 },
        { "vnd.sun.star.cmd:", 0, 0, 0 },
        { "vnd.sun.star.help:", 0, 0, 0 },
        { "vnd.sun.star.hier:", 0, 0, 0 },
        { "vnd.sun.star.odma:", 0, 0, 0 },
        { "vnd.sun.star.pkg:", 0, 0, 0 },
        { "vnd.sun.star.script:", 0, 0, 0 },
        { "vnd.sun.star.webdav:", 0, 0, 0 },
        { "vnd.sun.star.wfs:", 0, 0, 0 },
        { "generic:path", 0, 0, 0 },
        { "wfs:", 0, 0, 0 }
    };
    CharClass charClass(
        css::uno::Reference< css::lang::XMultiServiceFactory >(
            m_context->getServiceManager(), css::uno::UNO_QUERY_THROW),
        com::sun::star::lang::Locale("en", "US", ""));
    for (std::size_t i = 0; i < SAL_N_ELEMENTS(tests); ++i) {
        rtl::OUString input(rtl::OUString::createFromAscii(tests[i].input));
        xub_StrLen begin = 0;
        xub_StrLen end = static_cast< xub_StrLen >(input.getLength());
        rtl::OUString result(
            URIHelper::FindFirstURLInText(input, begin, end, charClass));
        bool ok = tests[i].result == 0
            ? (result.getLength() == 0 && begin == input.getLength()
               && end == input.getLength())
            : (result.equalsAscii(tests[i].result) && begin == tests[i].begin
               && end == tests[i].end);
        rtl::OString msg;
        if (!ok) {
            rtl::OStringBuffer buf;
            buf.append('"');
            buf.append(tests[i].input);
            buf.append(RTL_CONSTASCII_STRINGPARAM("\" -> "));
            buf.append(tests[i].result == 0 ? "none" : tests[i].result);
            buf.append(RTL_CONSTASCII_STRINGPARAM(" ("));
            buf.append(static_cast< sal_Int32 >(tests[i].begin));
            buf.append(RTL_CONSTASCII_STRINGPARAM(", "));
            buf.append(static_cast< sal_Int32 >(tests[i].end));
            buf.append(')');
            buf.append(RTL_CONSTASCII_STRINGPARAM(" != "));
            buf.append(rtl::OUStringToOString(result, RTL_TEXTENCODING_UTF8));
            buf.append(RTL_CONSTASCII_STRINGPARAM(" ("));
            buf.append(static_cast< sal_Int32 >(begin));
            buf.append(RTL_CONSTASCII_STRINGPARAM(", "));
            buf.append(static_cast< sal_Int32 >(end));
            buf.append(')');
            msg = buf.makeStringAndClear();
        }
        CPPUNIT_ASSERT_MESSAGE(msg.getStr(), ok);
    }
}

css::uno::Reference< css::uno::XComponentContext > Test::m_context;

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
