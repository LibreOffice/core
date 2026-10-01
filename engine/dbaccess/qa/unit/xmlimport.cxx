/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star;
using namespace ::cpo;

namespace
{
class DBXMLImportTest : public UnoApiTest
{
public:
    DBXMLImportTest()
        : UnoApiTest(u"dbaccess/qa/unit/data"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(DBXMLImportTest, testDocumentJavaClassPathIgnored)
{
    // The database names a Java class path both as a db:java-classpath attribute and as a data
    // source setting. Neither reaches the data source, opened by URL as a database range does.
    uno::Reference<sdb::XDatabaseContext> xContext
        = sdb::DatabaseContext::create(comphelper::getProcessComponentContext());
    uno::Reference<beans::XPropertySet> xDataSource(
        xContext->getByName(createFileURL(u"document_java_classpath.odb")), uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aInfo;
    xDataSource->getPropertyValue(u"Info"_ustr) >>= aInfo;
    comphelper::NamedValueCollection aSettings(aInfo);
    CPPUNIT_ASSERT_EQUAL(OUString(),
                         aSettings.getOrDefault(u"JavaDriverClassPath"_ustr, OUString()));
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
