/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xsheetfilterdescriptor3.hxx>

#include <com/sun/star/sheet/TableFilterField3.hpp>
#include <com/sun/star/sheet/XSheetFilterDescriptor3.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace css;
using namespace css::uno;

namespace apitest
{
void XSheetFilterDescriptor3::testGetSetFilterFields3()
{
    uno::Reference<sheet::XSheetFilterDescriptor3> xSFD(init(), uno::UNO_QUERY_THROW);
    uno::Sequence<sheet::TableFilterField3> aDefaultTFF = xSFD->getFilterFields3();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get TableFilterField", sal_Int32(0),
                                 aDefaultTFF.getLength());

    uno::Sequence<sheet::TableFilterField3> aTFF;
    aTFF.realloc(1);

    xSFD->setFilterFields3(aTFF);
    uno::Sequence<sheet::TableFilterField3> aNewTFF = xSFD->getFilterFields3();
    CPPUNIT_ASSERT_MESSAGE("Unable to set TableFilterField", aNewTFF != aDefaultTFF);
}

} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
