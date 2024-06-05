/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/frame/XDispatchInformationProvider.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>

#include <comphelper/propertyvalue.hxx>
#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star;

namespace
{
class DispatchAPITest : public UnoApiTest
{
public:
    DispatchAPITest()
        : UnoApiTest(u"/framework/qa/cppunit/data/"_ustr)
    {
    }
    void checkDispatchInfo(uno::Reference<frame::XFrame> xFrame);
    uno::Reference<frame::XFrame> loadComponent(OUString url);
    uno::Reference<frame::XFrame> loadWithDBComponent(OUString url);
};

uno::Reference<frame::XFrame> DispatchAPITest::loadComponent(OUString url)
{
    uno::Reference<css::frame::XDesktop2> xDesktop
        = css::frame::Desktop::create(comphelper::getProcessComponentContext());
    uno::Reference<css::frame::XFrame> xFrame = xDesktop->findFrame(u"_blank"_ustr, 0);
    uno::Reference<frame::XComponentLoader> xComponentLoader(xFrame, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aLoadArgs{ comphelper::makePropertyValue(u"Hidden"_ustr,
                                                                                 false) };
    uno::Reference<lang::XComponent> xComponent
        = xComponentLoader->loadComponentFromURL(url, u"_default"_ustr, 0, aLoadArgs);
    CPPUNIT_ASSERT(xComponent.is());

    return xFrame;
}

uno::Reference<frame::XFrame> DispatchAPITest::loadWithDBComponent(OUString url)
{
    uno::Reference<css::frame::XDesktop2> xDesktop
        = css::frame::Desktop::create(comphelper::getProcessComponentContext());
    uno::Reference<css::frame::XFrame> xFrame = xDesktop->findFrame(u"_blank"_ustr, 0);

    // Get the database connection
    createTempCopy(u"checkDispatchAPIDB.odb");
    uno::Reference<lang::XComponent> xDBComponent = loadFromDesktop(maTempFile.GetURL());
    uno::Reference<sdb::XOfficeDatabaseDocument> xDBDocument(xDBComponent, uno::UNO_QUERY_THROW);

    uno::Reference<sdbc::XDataSource> xDataSource = xDBDocument->getDataSource();
    uno::Reference<sdbc::XConnection> xConnection = xDataSource->getConnection(u""_ustr, u""_ustr);
    CPPUNIT_ASSERT(xConnection.is());

    // Get the frame reference
    uno::Reference<lang::XMultiServiceFactory> xFactory(comphelper::getProcessServiceFactory());
    uno::Reference<uno::XInterface> xInterface
        = xFactory->createInstance(u"com.sun.star.frame.Desktop"_ustr);
    uno::Reference<frame::XComponentLoader> xComponentLoader(xFrame, uno::UNO_QUERY);

    uno::Sequence<beans::PropertyValue> aLoadArgs{ comphelper::makePropertyValue(
        u"ActiveConnection"_ustr, uno::Any(xConnection)) };
    uno::Reference<lang::XComponent> xComponent
        = xComponentLoader->loadComponentFromURL(url, u"_self"_ustr, 0, aLoadArgs);
    CPPUNIT_ASSERT(xComponent.is());

    return xFrame;
}

void DispatchAPITest::checkDispatchInfo(uno::Reference<frame::XFrame> xFrame)
{
    uno::Reference<frame::XDispatchInformationProvider> xProvider(xFrame, uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("Can't load XDispatchInformationProvider.", xProvider.is());

    uno::Sequence<sal_Int16> Groups = xProvider->getSupportedCommandGroups();
    CPPUNIT_ASSERT_MESSAGE("Couldn't get Supported Command Groups", Groups.getLength() > 0);

    for (sal_Int32 i = 0; i < Groups.getLength(); i++)
    {
        uno::Sequence<frame::DispatchInformation> DispatchInfos
            = xProvider->getConfigurableDispatchInformation(Groups[i]);
        CPPUNIT_ASSERT_MESSAGE("Couldn't get Dispatch Information for Supported Command Groups",
                               DispatchInfos.getLength() > 0);

        std::unordered_map<OUString, OUString> rCheckMap;

        for (sal_Int32 j = 0; j < DispatchInfos.getLength(); j++)
        {
            const frame::DispatchInformation& xDispatchInfo = DispatchInfos[j];
            CPPUNIT_ASSERT_EQUAL(xDispatchInfo.GroupId, Groups[i]);

            // Check the Dispatch Information
            // There should be no duplicates in rCheckMap
            // i.e. rCheckMap[xDispatchInfo.Command] should be empty
            CPPUNIT_ASSERT_EQUAL(rCheckMap[xDispatchInfo.Command], OUString());

            rCheckMap[xDispatchInfo.Command] = xDispatchInfo.Command;
        }
    }
}

CPPUNIT_TEST_FIXTURE(DispatchAPITest, testCheckDispatchInfoOfWriter)
{
    uno::Reference<frame::XFrame> xFrame = loadComponent(u"private:factory/swriter"_ustr);
    checkDispatchInfo(xFrame);
}

CPPUNIT_TEST_FIXTURE(DispatchAPITest, testCheckDispatchInfoOfCalc)
{
    uno::Reference<frame::XFrame> xFrame = loadComponent(u"private:factory/scalc"_ustr);
    checkDispatchInfo(xFrame);
}

CPPUNIT_TEST_FIXTURE(DispatchAPITest, testCheckDispatchInfoOfDraw)
{
    uno::Reference<frame::XFrame> xFrame = loadComponent(u"private:factory/sdraw"_ustr);
    checkDispatchInfo(xFrame);
}

CPPUNIT_TEST_FIXTURE(DispatchAPITest, testCheckDispatchInfoOfImpress)
{
    uno::Reference<frame::XFrame> xFrame = loadComponent(u"private:factory/simpress"_ustr);
    checkDispatchInfo(xFrame);
}

CPPUNIT_TEST_FIXTURE(DispatchAPITest, testCheckDispatchInfoOfChart)
{
    uno::Reference<frame::XFrame> xFrame = loadComponent(u"private:factory/schart"_ustr);
    checkDispatchInfo(xFrame);
}

CPPUNIT_TEST_FIXTURE(DispatchAPITest, testCheckDispatchInfoOfMath)
{
    uno::Reference<frame::XFrame> xFrame = loadComponent(u"private:factory/smath"_ustr);
    checkDispatchInfo(xFrame);
}

CPPUNIT_TEST_FIXTURE(DispatchAPITest, testCheckDispatchInfoOfDatabase)
{
    uno::Reference<frame::XFrame> xFrame = loadComponent(u"private:factory/sdatabase"_ustr);
    checkDispatchInfo(xFrame);
}

CPPUNIT_TEST_FIXTURE(DispatchAPITest, testCheckDispatchInfoOfBibliography)
{
    uno::Reference<frame::XFrame> xFrame = loadComponent(u".component:Bibliography/View1"_ustr);
    checkDispatchInfo(xFrame);
}

CPPUNIT_TEST_FIXTURE(DispatchAPITest, testCheckDispatchInfoOfFormGridView)
{
    uno::Reference<frame::XFrame> xFrame = loadComponent(u".component:DB/FormGridView"_ustr);
    checkDispatchInfo(xFrame);
}

CPPUNIT_TEST_FIXTURE(DispatchAPITest, testCheckDispatchInfoOfDataSourceBrowser)
{
    uno::Reference<frame::XFrame> xFrame = loadComponent(u".component:DB/DataSourceBrowser"_ustr);
    checkDispatchInfo(xFrame);
}

CPPUNIT_TEST_FIXTURE(DispatchAPITest, testCheckDispatchInfoOfRelationDesign)
{
    uno::Reference<frame::XFrame> xFrame
        = loadWithDBComponent(u".component:DB/RelationDesign"_ustr);
    checkDispatchInfo(xFrame);
}

CPPUNIT_TEST_FIXTURE(DispatchAPITest, testCheckDispatchInfoOfQueryDesign)
{
    uno::Reference<frame::XFrame> xFrame = loadWithDBComponent(u".component:DB/QueryDesign"_ustr);
    checkDispatchInfo(xFrame);
}

CPPUNIT_TEST_FIXTURE(DispatchAPITest, testCheckDispatchInfoOfTableDesign)
{
    uno::Reference<frame::XFrame> xFrame = loadWithDBComponent(u".component:DB/TableDesign"_ustr);
    checkDispatchInfo(xFrame);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
