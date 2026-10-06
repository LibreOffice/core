/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_firebird.h>

#include <test/unoapi_test.hxx>

#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/form/XFormsSupplier.hpp>
#include <com/sun/star/form/runtime/XFilterController.hpp>
#include <com/sun/star/form/runtime/XFormController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdbc/XStatement.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/util/XModeSelector.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/view/XFormLayerAccess.hpp>

#include <vcl/scheduler.hxx>

using namespace ::com::sun::star;

namespace
{
// the test uses an embedded Firebird database
#if ENABLE_FIREBIRD_SDBC
/// Tests for svx/source/form/ code.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest(u"svx/qa/unit/data/"_ustr)
    {
    }

protected:
    /// Creates an embedded Firebird database with a "Books" table and returns its URL.
    OUString createBooksDatabase();
};

OUString Test::createBooksDatabase()
{
    uno::Reference<sdb::XOfficeDatabaseDocument> xDBDocument(
        m_xSFactory->createInstance(u"com.sun.star.sdb.OfficeDatabaseDocument"_ustr),
        uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xDataSourceProps(xDBDocument->getDataSource(),
                                                         uno::UNO_QUERY_THROW);
    xDataSourceProps->setPropertyValue(u"URL"_ustr, uno::Any(u"sdbc:embedded:firebird"_ustr));
    uno::Reference<frame::XStorable> xStorable(xDBDocument, uno::UNO_QUERY_THROW);
    xStorable->storeAsURL(maTempFile.GetURL(), {});

    // keep the database document open while the form uses it
    mxComponent2 = loadFromDesktop(maTempFile.GetURL());
    uno::Reference<sdb::XOfficeDatabaseDocument> xDocument(mxComponent2, uno::UNO_QUERY_THROW);
    uno::Reference<sdbc::XConnection> xConnection
        = xDocument->getDataSource()->getConnection(u""_ustr, u""_ustr);
    uno::Reference<sdbc::XStatement> xStatement = xConnection->createStatement();
    xStatement->execute(u"CREATE TABLE \"Books\" (\"ID\" INTEGER NOT NULL PRIMARY KEY,"
                        " \"Title\" VARCHAR(50))"_ustr);
    xStatement->execute(u"INSERT INTO \"Books\" VALUES (1, 'Apple')"_ustr);
    xStatement->execute(u"INSERT INTO \"Books\" VALUES (2, 'Banana')"_ustr);
    xStatement->execute(u"INSERT INTO \"Books\" VALUES (3, 'Apricot')"_ustr);
    xConnection->commit();
    uno::Reference<sdbcx::XTablesSupplier> xTablesSupplier(xConnection, uno::UNO_QUERY_THROW);
    uno::Reference<util::XRefreshable>(xTablesSupplier->getTables(), uno::UNO_QUERY_THROW)
        ->refresh();
    return maTempFile.GetURL();
}

CPPUNIT_TEST_FIXTURE(Test, testTdf170190)
{
    // Given a Writer form on a table, filtered by two conditions on the same column:
    const OUString aDatabaseURL = createBooksDatabase();
    loadFromURL(u"private:factory/swriter"_ustr);
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();

    uno::Reference<form::XForm> xForm(
        xFactory->createInstance(u"com.sun.star.form.component.DataForm"_ustr),
        uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> xFormProps(xForm, uno::UNO_QUERY_THROW);
    xFormProps->setPropertyValue(u"DataSourceName"_ustr, uno::Any(aDatabaseURL));
    xFormProps->setPropertyValue(u"Command"_ustr, uno::Any(u"Books"_ustr));
    xFormProps->setPropertyValue(u"CommandType"_ustr, uno::Any(sdb::CommandType::TABLE));
    xFormProps->setPropertyValue(u"Filter"_ustr,
                                 uno::Any(u"\"Title\" LIKE 'A%' AND \"Title\" LIKE '%e'"_ustr));
    xFormProps->setPropertyValue(u"ApplyFilter"_ustr, uno::Any(true));
    uno::Reference<form::XFormsSupplier> xFormsSupplier(xDrawPage, uno::UNO_QUERY_THROW);
    xFormsSupplier->getForms()->insertByName(u"Form"_ustr, uno::Any(xForm));

    uno::Reference<awt::XControlModel> xTextField(
        xFactory->createInstance(u"com.sun.star.form.component.TextField"_ustr),
        uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet>(xTextField, uno::UNO_QUERY_THROW)
        ->setPropertyValue(u"DataField"_ustr, uno::Any(u"Title"_ustr));
    uno::Reference<container::XNameContainer>(xForm, uno::UNO_QUERY_THROW)
        ->insertByName(u"Title"_ustr, uno::Any(xTextField));
    uno::Reference<drawing::XControlShape> xShape(
        xFactory->createInstance(u"com.sun.star.drawing.ControlShape"_ustr), uno::UNO_QUERY_THROW);
    xShape->setSize(awt::Size(5000, 1000));
    xShape->setControl(xTextField);
    xDrawPage->add(xShape);

    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<view::XFormLayerAccess> xFormLayer(xModel->getCurrentController(),
                                                      uno::UNO_QUERY_THROW);
    xFormLayer->setFormDesignMode(false);
    Scheduler::ProcessEventsToIdle();

    // When starting the form based filter, which shows the current filter in the controls:
    uno::Reference<form::runtime::XFormController> xFormController
        = xFormLayer->getFormController(xForm);
    uno::Reference<util::XModeSelector> xModeSelector(xFormController, uno::UNO_QUERY_THROW);
    xModeSelector->setMode(u"FilterMode"_ustr);
    uno::Reference<form::runtime::XFilterController> xFilterController(xFormController,
                                                                       uno::UNO_QUERY_THROW);
    const uno::Sequence<uno::Sequence<OUString>> aPredicates
        = xFilterController->getPredicateExpressions();
    xModeSelector->setMode(u"DataMode"_ustr);

    // Then make sure both conditions keep their operator:
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aPredicates.getLength());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1), aPredicates[0].getLength());
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: LIKE 'A%' AND LIKE '%e'
    // - Actual  : LIKE 'A%' AND '%e'
    CPPUNIT_ASSERT_EQUAL(u"LIKE 'A%' AND LIKE '%e'"_ustr,
                         aPredicates[0][0].replaceAll(u"  ", u" "));
}
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
