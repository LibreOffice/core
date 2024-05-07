/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/Bibliography.hpp>

using namespace ::com::sun::star;

namespace
{
/// Covers extensions/source/bibliography/ fixes.
class Test : public test::BootstrapFixture, public unotest::MacrosTest
{
};
}

CPPUNIT_TEST_FIXTURE(Test, testBibliographyLoader)
{
    // Given a bibliography provider:
    uno::Reference<container::XNameAccess> xBibAccess
        = frame::Bibliography::create(mxComponentContext);
    uno::Reference<beans::XPropertySet> xPropSet(xBibAccess, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aSeq;

    // When getting the column names:
    xPropSet->getPropertyValue(u"BibliographyDataFieldNames"_ustr) >>= aSeq;

    // Then make sure we have columns and all have non-empty names:
    CPPUNIT_ASSERT(aSeq.hasElements());

    // Without the accompanying fix in place, this test would have failed, as the last column
    // (LOCAL_URL) had an empty field name:
    for (const auto& rPair : aSeq)
    {
        CPPUNIT_ASSERT(!rPair.Name.isEmpty());
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
