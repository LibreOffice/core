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


#include "sal/config.h"

#include "multi.hxx"

#include "rtl/string.hxx"
#include "rtl/textenc.h"
#include "rtl/ustring.hxx"

#include <sstream>

namespace {

struct CheckFailed {
    explicit CheckFailed(OUString const & theMessage): message(theMessage)
    {}

    OUString message;
};

::std::ostream& operator<< (::std::ostream& os, const OUString& str)
{
    return os << OUStringToOString(str, RTL_TEXTENCODING_UTF8).getStr();
}

template< typename T > void checkEqual(T const & value, T const & argument) {
    if (argument != value) {
        std::ostringstream s;
        s << value << " != " << argument;
        throw CheckFailed(
            OStringToOUString(
                OString(s.str().c_str()), RTL_TEXTENCODING_UTF8));
    }
}

}

namespace testtools { namespace bridgetest {

OUString testMulti(  css::uno::Reference< test::testtools::bridgetest::XMulti >  const & multi )
{
    try {
        checkEqual(
            0.0,
            css::uno::Reference< test::testtools::bridgetest::XMultiBase1 >(
                multi, css::uno::UNO_QUERY_THROW)->getatt1());
        checkEqual(
            0.0,
            static_cast< test::testtools::bridgetest::XMultiBase2 * >(
                multi.get())->getatt1());
        checkEqual(
            0.0,
            css::uno::Reference< test::testtools::bridgetest::XMultiBase2 >(
                multi, css::uno::UNO_QUERY_THROW)->getatt1());
        checkEqual(
            0.0,
            static_cast< test::testtools::bridgetest::XMultiBase5 * >(
                multi.get())->getatt1());
        checkEqual(
            0.0,
            css::uno::Reference< test::testtools::bridgetest::XMultiBase5 >(
                multi, css::uno::UNO_QUERY_THROW)->getatt1());
        css::uno::Reference< test::testtools::bridgetest::XMultiBase5 >(
            multi, css::uno::UNO_QUERY_THROW)->setatt1(0.1);
        checkEqual(
            0.1,
            css::uno::Reference< test::testtools::bridgetest::XMultiBase1 >(
                multi, css::uno::UNO_QUERY_THROW)->getatt1());
        checkEqual(
            0.1,
            static_cast< test::testtools::bridgetest::XMultiBase2 * >(
                multi.get())->getatt1());
        checkEqual(
            0.1,
            css::uno::Reference< test::testtools::bridgetest::XMultiBase2 >(
                multi, css::uno::UNO_QUERY_THROW)->getatt1());
        checkEqual(
            0.1,
            static_cast< test::testtools::bridgetest::XMultiBase5 * >(
                multi.get())->getatt1());
        checkEqual(
            0.1,
            css::uno::Reference< test::testtools::bridgetest::XMultiBase5 >(
                multi, css::uno::UNO_QUERY_THROW)->getatt1());
        checkEqual< sal_Int32 >(
            11 * 1,
            css::uno::Reference< test::testtools::bridgetest::XMultiBase1 >(
                multi, css::uno::UNO_QUERY_THROW)->fn11(1));
        checkEqual< sal_Int32 >(
            11 * 1,
            static_cast< test::testtools::bridgetest::XMultiBase2 * >(
                multi.get())->fn11(1));
        checkEqual< sal_Int32 >(
            11 * 2,
            css::uno::Reference< test::testtools::bridgetest::XMultiBase2 >(
                multi, css::uno::UNO_QUERY_THROW)->fn11(2));
        checkEqual< sal_Int32 >(
            11 * 1,
            static_cast< test::testtools::bridgetest::XMultiBase5 * >(
                multi.get())->fn11(1));
        checkEqual< sal_Int32 >(
            11 * 5,
            css::uno::Reference< test::testtools::bridgetest::XMultiBase5 >(
                multi, css::uno::UNO_QUERY_THROW)->fn11(5));
        checkEqual(
            OUString("12" "abc"),
            css::uno::Reference< test::testtools::bridgetest::XMultiBase1 >(
                multi, css::uno::UNO_QUERY_THROW)->fn12(
                    "abc"));
        checkEqual(
            OUString("12" "abc-2"),
            static_cast< test::testtools::bridgetest::XMultiBase2 * >(
                multi.get())->fn12("abc-2"));
        checkEqual(
            OUString("12" "abc-2"),
            css::uno::Reference< test::testtools::bridgetest::XMultiBase2 >(
                multi, css::uno::UNO_QUERY_THROW)->fn12("abc-2"));
        checkEqual(
            OUString("12" "abc-5"),
            static_cast< test::testtools::bridgetest::XMultiBase5 * >(
                multi.get())->fn12("abc-5"));
        checkEqual(
            OUString("12" "abc-5"),
            css::uno::Reference< test::testtools::bridgetest::XMultiBase5 >(
                multi, css::uno::UNO_QUERY_THROW)->fn12("abc-5"));
        checkEqual< sal_Int32 >(21 * 2, multi->fn21(2));
        checkEqual(
            OUString("22" "de"),
            multi->fn22("de"));
        checkEqual< sal_Int32 >(
            31 * 3,
            css::uno::Reference< test::testtools::bridgetest::XMultiBase3 >(
                multi, css::uno::UNO_QUERY_THROW)->fn31(3));
        checkEqual< sal_Int32 >(
            31 * 5,
            static_cast< test::testtools::bridgetest::XMultiBase5 * >(
                multi.get())->fn31(5));
        checkEqual< sal_Int32 >(
            31 * 5,
            css::uno::Reference< test::testtools::bridgetest::XMultiBase5 >(
                multi, css::uno::UNO_QUERY_THROW)->fn31(5));
        checkEqual(
            0.0,
            css::uno::Reference< test::testtools::bridgetest::XMultiBase3 >(
                multi, css::uno::UNO_QUERY_THROW)->getatt3());
        checkEqual(
            0.0,
            static_cast< test::testtools::bridgetest::XMultiBase5 * >(
                multi.get())->getatt3());
        checkEqual(
            0.0,
            css::uno::Reference< test::testtools::bridgetest::XMultiBase5 >(
                multi, css::uno::UNO_QUERY_THROW)->getatt3());
        css::uno::Reference< test::testtools::bridgetest::XMultiBase3 >(
            multi, css::uno::UNO_QUERY_THROW)->setatt3(0.3);
        checkEqual(
            0.3,
            css::uno::Reference< test::testtools::bridgetest::XMultiBase3 >(
                multi, css::uno::UNO_QUERY_THROW)->getatt3());
        checkEqual(
            0.3,
            static_cast< test::testtools::bridgetest::XMultiBase5 * >(
                multi.get())->getatt3());
        checkEqual(
            0.3,
            css::uno::Reference< test::testtools::bridgetest::XMultiBase5 >(
                multi, css::uno::UNO_QUERY_THROW)->getatt3());
        checkEqual(
            OUString("32" "f"),
            css::uno::Reference< test::testtools::bridgetest::XMultiBase3 >(
                multi, css::uno::UNO_QUERY_THROW)->fn32("f"));
        checkEqual(
            OUString("32" "f-5"),
            static_cast< test::testtools::bridgetest::XMultiBase5 * >(
                multi.get())->fn32("f-5"));
        checkEqual(
            OUString("32" "f-5"),
            css::uno::Reference< test::testtools::bridgetest::XMultiBase5 >(
                multi, css::uno::UNO_QUERY_THROW)->fn32("f-5"));
        checkEqual< sal_Int32 >(
            33,
            css::uno::Reference< test::testtools::bridgetest::XMultiBase3 >(
                multi, css::uno::UNO_QUERY_THROW)->fn33());
        checkEqual< sal_Int32 >(
            33,
            static_cast< test::testtools::bridgetest::XMultiBase5 * >(
                multi.get())->fn33());
        checkEqual< sal_Int32 >(
            33,
            css::uno::Reference< test::testtools::bridgetest::XMultiBase5 >(
                multi, css::uno::UNO_QUERY_THROW)->fn33());
        checkEqual< sal_Int32 >(41 * 4, multi->fn41(4));
        checkEqual< sal_Int32 >(61 * 6, multi->fn61(6));
        checkEqual(
            OUString("62" ""),
            multi->fn62(OUString()));
        checkEqual< sal_Int32 >(71 * 7, multi->fn71(7));
        checkEqual(
            OUString("72" "g"),
            multi->fn72("g"));
        checkEqual< sal_Int32 >(73, multi->fn73());
    } catch (CheckFailed const & f) {
        return f.message;
    }
    return OUString();
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
