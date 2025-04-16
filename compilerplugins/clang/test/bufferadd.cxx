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
// replacing OUStringBuffer.append sequences to OUString+
namespace test1
{
void f1()
{
    // expected-error@+1 {{convert this append sequence into a *String + sequence [loplugin:bufferadd]}}
    OUStringBuffer v;
    v.append("xxx");
    v.append("xxx");
}
void f2()
{
    // expected-error@+1 {{convert this append sequence into a *String + sequence [loplugin:bufferadd]}}
    OUStringBuffer v;
    v.append("xxx").append("aaaa");
}
void f3(OString class_name)
{
    // expected-error@+1 {{convert this append sequence into a *String + sequence [loplugin:bufferadd]}}
    OStringBuffer sig_buf(5 + class_name.getLength());
    sig_buf.append("(I)L");
    //sig_buf.append( class_name.replace( '.', '/' ) );
    sig_buf.append(';');
    OString sig(sig_buf.makeStringAndClear());
    (void)sig;
}
void f4(sal_Unicode const* pPathBegin)
{
    // expected-error@+1 {{convert this append sequence into a *String + sequence [loplugin:bufferadd]}}
    OUStringBuffer v;
    v.append(pPathBegin, 12);
    v.append("aaaa");
}
void f5(OUStringBuffer& input)
{
    // expected-error@+1 {{convert this append sequence into a *String + sequence [loplugin:bufferadd]}}
    OUStringBuffer v(input);
    v.append("aaaa");
}
void f6(OString const& s)
{
    // expected-error@+1 {{convert this append sequence into a *String + sequence [loplugin:bufferadd]}}
    OUStringBuffer b("foo");
    b.append(OStringToOUString(s, RTL_TEXTENCODING_ASCII_US));
}
struct Footer
{
    OStringBuffer m_descriptorStart;
    OString m_descriptorEnd;
    OString f8() const
    {
        // expected-error@+1 {{convert this append sequence into a *String + sequence [loplugin:bufferadd]}}
        OStringBuffer buf(m_descriptorStart);
        buf.append(m_descriptorEnd);
        return buf.makeStringAndClear();
    }
};
}

namespace test2
{
void f2()
{
    // no warning expected
    OUStringBuffer v;
    v.append("xxx");
    if (true)
        v.append("yyyy");
}
void appendTo(OUStringBuffer&);
void f3()
{
    // no warning expected
    OUStringBuffer v;
    appendTo(v);
    v.append("xxx");
}
void f4()
{
    // no warning expected
    OUStringBuffer v;
    v.append("xxx");
    v.setLength(0);
}
void f5()
{
    // no warning expected
    OUStringBuffer v;
    v.append("xxx");
    v[1] = 'x';
}
void f6()
{
    // no warning expected
    OUStringBuffer noel1("xxx");
    while (true)
        noel1.append("ffff").append("aaa");
}
void f7()
{
    // expected-error@+1 {{convert this append sequence into a *String + sequence [loplugin:bufferadd]}}
    OStringBuffer noelf7("xxx");
    noelf7 = "xxx" + noelf7 + "xxx";
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
