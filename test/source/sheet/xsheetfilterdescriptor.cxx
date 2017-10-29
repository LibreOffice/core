/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xsheetfilterdescriptor.hxx>

#include <com/sun/star/sheet/TableFilterField.hpp>
#include <com/sun/star/sheet/XSheetFilterDescriptor.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <com/sun/star/uno/Sequence.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace css;
using namespace css::uno;

namespace apitest {

void XSheetFilterDescriptor::testGetSetFilterFields()
{
    uno::Reference< sheet::XSheetFilterDescriptor > xSFD(init(), UNO_QUERY_THROW);
    uno::Sequence< sheet::TableFilterField > xDefaultTFF = xSFD->getFilterFields();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unable to get TableFilterField", sal_Int32(0), xDefaultTFF.getLength());

    uno::Sequence< sheet::TableFilterField > xTFF;
    xTFF.realloc(1);

    xSFD->setFilterFields(xTFF);
    uno::Sequence< sheet::TableFilterField > xNewTFF = xSFD->getFilterFields();
    CPPUNIT_ASSERT_MESSAGE("Unable to set TableFilterField", xNewTFF != xDefaultTFF);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
