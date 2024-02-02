/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <ostream>
#include <sstream>
#include <string_view>
#include <string>

#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>

namespace test1
{
void f1(bool, const OString& s);
struct Foo
{
    void f1(bool, const OString& s);
};
void test1(Foo& foo)
{
    OString s;
    // expected-error@+1 {{unnecessary call to 'getStr' when passing to OString constructor [loplugin:unnecessarygetstr]}}
    f1(true, s.getStr());
    // expected-error@+1 {{unnecessary call to 'getStr' when passing to OString constructor [loplugin:unnecessarygetstr]}}
    foo.f1(true, s.getStr());
    // expected-error@+1 {{unnecessary call to 'getStr' when passing to OString constructor [loplugin:unnecessarygetstr]}}
    foo.f1(true, OString::boolean(true).getStr());
    // expected-error@+1 {{unnecessary call to 'getStr' when passing to OString constructor [loplugin:unnecessarygetstr]}}
    foo.f1(true, OString::number(12).getStr());

    // avoid false +
    OString aVal = "xx";
    OUString aCompText
        = "xx" + OUString(aVal.getStr(), aVal.getLength(), RTL_TEXTENCODING_ASCII_US);
    (void)aCompText;
}
}

namespace test2
{
// call to param that takes string_view
void f2(bool, std::string_view);
void f2(bool, std::u16string_view);
struct Foo2
{
    void f2(bool, std::string_view);
    void f2(bool, std::u16string_view);
};
void testOString(Foo2& foo)
{
    OString s;
    // expected-error@+1 {{unnecessary call to 'getStr' when passing to string_view constructor [loplugin:unnecessarygetstr]}}
    f2(true, s.getStr());
    // expected-error@+1 {{unnecessary call to 'getStr' when passing to string_view constructor [loplugin:unnecessarygetstr]}}
    foo.f2(true, s.getStr());
    // expected-error@+1 {{unnecessary call to 'getStr' when passing to string_view constructor [loplugin:unnecessarygetstr]}}
    foo.f2(true, OString::boolean(true).getStr());
    // expected-error@+1 {{unnecessary call to 'getStr' when passing to string_view constructor [loplugin:unnecessarygetstr]}}
    foo.f2(true, OString::number(12).getStr());
}
void testOUString(Foo2& foo)
{
    OUString s;
    // expected-error@+1 {{unnecessary call to 'getStr' when passing to string_view constructor [loplugin:unnecessarygetstr]}}
    f2(true, s.getStr());
    // expected-error@+1 {{unnecessary call to 'getStr' when passing to string_view constructor [loplugin:unnecessarygetstr]}}
    foo.f2(true, s.getStr());
    // expected-error@+1 {{unnecessary call to 'getStr' when passing to string_view constructor [loplugin:unnecessarygetstr]}}
    foo.f2(true, OUString::boolean(true).getStr());
    // expected-error@+1 {{unnecessary call to 'getStr' when passing to string_view constructor [loplugin:unnecessarygetstr]}}
    foo.f2(true, OUString::number(12).getStr());
}
}

namespace test3
{
// call to param that takes string_view
void f2(bool, std::string_view);
struct Foo2
{
    void f2(bool, std::string_view);
};
void test3(Foo2& foo)
{
    std::string s;
    // expected-error@+1 {{unnecessary call to 'c_str' when passing to string_view constructor [loplugin:unnecessarygetstr]}}
    f2(true, s.c_str());
    // expected-error@+1 {{unnecessary call to 'c_str' when passing to string_view constructor [loplugin:unnecessarygetstr]}}
    foo.f2(true, s.c_str());
}
}

namespace test4
{
void test()
{
    std::string s;
    // expected-error@+1 {{unnecessary call to 'c_str' when passing to OUString::createFromAscii [loplugin:unnecessarygetstr]}}
    OUString::createFromAscii(s.c_str());
}
}

namespace test5
{
void test(std::string v, OString o)
{
    // expected-error@+1 {{unnecessary call to 'c_str' when passing to string_view constructor [loplugin:unnecessarygetstr]}}
    std::string_view s1(v.c_str());
    // expected-error@+1 {{unnecessary call to 'getStr' when passing to string constructor [loplugin:unnecessarygetstr]}}
    std::string s2(o.getStr());
}
}

// no warning expected
namespace test6
{
void foo(const OString&);
void test(std::string v) { foo(v.c_str()); }
}

// no warning expected
namespace test7
{
void test(const OString& v) { std::stringstream aStream(v.getStr()); }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
