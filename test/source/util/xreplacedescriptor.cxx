/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/util/xreplacedescriptor.hxx>
#include <rtl/ustring.hxx>

#include <com/sun/star/util/XReplaceDescriptor.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace css;

namespace apitest
{
void XReplaceDescriptor::testGetSetReplaceString()
{
    uno::Reference<util::XReplaceDescriptor> xRD(init(), uno::UNO_QUERY_THROW);
    const OUString aReplaceStringOld = xRD->getReplaceString();
    CPPUNIT_ASSERT(aReplaceStringOld.isEmpty());

    xRD->setReplaceString(u"_XReplaceDescriptor"_ustr);
    const OUString aReplaceStringNew = xRD->getReplaceString();
    CPPUNIT_ASSERT_EQUAL(u"_XReplaceDescriptor"_ustr, aReplaceStringNew);
    CPPUNIT_ASSERT(aReplaceStringOld != aReplaceStringNew);
}

} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
