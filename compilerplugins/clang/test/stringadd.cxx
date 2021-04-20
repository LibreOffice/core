/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>

// ---------------------------------------------------------------
// += tests

namespace test1
{
static const char XXX1[] = "xxx";
static constexpr char16_t XXX1u[] = u"xxx";
static const char XXX2[] = "xxx";
void f1(OUString s1, int i, OString o)
{
    OUString s2 = s1;
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    s2 += "xxx";
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    s2 += "xxx";
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    s2 += s1;
    s2 = s1 + "xxx";
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    s2 += s1;
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    s2 += OUString::number(i);
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    s2 += XXX1;
    // expected-error@+2 {{rather use O[U]String::Concat than constructing 'rtl::OUStringLiteral<4>' from 'const char16_t [4]' on LHS of + (where RHS is of type 'const char [4]') [loplugin:stringadd]}}
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    s2 += OUStringLiteral(XXX1u) + XXX2;

    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    s2 += OStringToOUString(o, RTL_TEXTENCODING_UTF8);
}
void f2(OString s1, int i, OUString u)
{
    OString s2 = s1;
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    s2 += "xxx";
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    s2 += "xxx";
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    s2 += s1;
    s2 = s1 + "xxx";
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    s2 += s1;
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    s2 += OString::number(i);

    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    s2 += OUStringToOString(u, RTL_TEXTENCODING_ASCII_US);
}
void f3(OUString aStr, int nFirstContent)
{
    OUString aFirstStr = aStr.copy(0, nFirstContent);
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    aFirstStr += "...";
}
OUString side_effect();
void f4(int i)
{
    OUString s1;
    OUString s2("xxx");
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    s2 += "xxx";
    ++i;
    // any other kind of statement breaks the chain (at least for now)
    s2 += "xxx";
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    s2 += side_effect();
    s1 += "yyy";
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    s1 += "yyy";
}
}

namespace test2
{
void f(OUString s3)
{
    s3 += "xxx";
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    s3 += "xxx";
}
void g(OString s3)
{
    s3 += "xxx";
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    s3 += "xxx";
}
}

namespace test3
{
struct Bar
{
    OUString m_field;
};
void f(Bar b1, Bar& b2, Bar* b3)
{
    OUString s3 = "xxx";
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    s3 += b1.m_field;
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    s3 += b2.m_field;
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    s3 += b3->m_field;
}
OUString side_effect();
void f2(OUString s)
{
    OUString sRet = "xxx";
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    sRet += side_effect();
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    sRet += "xxx";
    sRet += side_effect();
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    sRet += "xxx";
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    sRet += "xxx";
    sRet += s;
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    sRet += "xxx";
}
}

// no warning expected
namespace test4
{
OUString side_effect();
void f()
{
    OUString sRet = "xxx";
#if OSL_DEBUG_LEVEL > 0
    sRet += ";";
#endif
    sRet += " ";
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    sRet += side_effect();
}
}

// no warning expected
namespace test5
{
OUString side_effect();
void f()
{
    OUString sRet = side_effect();
    sRet += side_effect();
}
}

namespace test6
{
void f(OUString sComma, OUString maExtension, int mnDocumentIconID)
{
    OUString sValue;
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    sValue += sComma + sComma + maExtension + sComma;
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    sValue += OUString::number(mnDocumentIconID) + sComma;
}
struct Foo
{
    OUString sFormula1;
};
void g(int x, const Foo& aValidation)
{
    OUString sCondition;
    switch (x)
    {
        case 1:
            sCondition += "cell-content-is-in-list(";
            // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
            sCondition += aValidation.sFormula1 + ")";
    }
}
}

// ---------------------------------------------------------------
// detecting OUString temporary construction in +

namespace test9
{
OUString getByValue();
const OUString& getByRef();
void f1(OUString s, OUString t, int i, const char* pChar)
{
    // no warning expected
    t = t + "xxx";
    // expected-error@+1 {{rather use O[U]String::Concat than constructing 'rtl::OUString' from 'const char [4]' on RHS of + (where LHS is of type 'rtl::OUString') [loplugin:stringadd]}}
    s = s + OUString("xxx");
    // expected-error@+1 {{rather use O[U]String::Concat than constructing 'rtl::OUString' from 'const rtl::OUString' on RHS of + (where LHS is of type 'rtl::OUString') [loplugin:stringadd]}}
    s = s + OUString(getByRef());

    // no warning expected
    OUString a;
    a = a + getByValue();

    // no warning expected
    OUString b;
    b = b + (i == 1 ? "aaa" : "bbb");

    // no warning expected
    OUString c;
    c = c + OUString(pChar, strlen(pChar), RTL_TEXTENCODING_UTF8);
}
void f2(char ch)
{
    OString s;
    // expected-error@+1 {{rather use O[U]String::Concat than constructing 'rtl::OString' from 'const char [4]' on RHS of + (where LHS is of type 'rtl::OString') [loplugin:stringadd]}}
    s = s + OString("xxx");
    // expected-error@+1 {{rather use O[U]String::Concat than constructing 'rtl::OString' from 'char' on RHS of + (where LHS is of type 'rtl::OString') [loplugin:stringadd]}}
    s = s + OString(ch);
}
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
