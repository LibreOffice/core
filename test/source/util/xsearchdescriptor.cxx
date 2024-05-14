/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/util/xsearchdescriptor.hxx>

#include <com/sun/star/util/XSearchDescriptor.hpp>

#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/TestAssert.h>

using namespace css;

namespace apitest
{
void XSearchDescriptor::testGetSetSearchString()
{
    uno::Reference<util::XSearchDescriptor> xDS(init(), uno::UNO_QUERY_THROW);
    const OUString aSearchStringOld = xDS->getSearchString();
    CPPUNIT_ASSERT(aSearchStringOld.isEmpty());

    xDS->setSearchString(u"_XSearchDescriptor"_ustr);
    const OUString aSearchStringNew = xDS->getSearchString();
    CPPUNIT_ASSERT_EQUAL(u"_XSearchDescriptor"_ustr, aSearchStringNew);
    CPPUNIT_ASSERT(aSearchStringOld != aSearchStringNew);
}

} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
