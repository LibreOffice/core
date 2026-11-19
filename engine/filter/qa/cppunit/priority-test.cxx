/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Unit test to check that we get the right filters for the right extensions.

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <sal/types.h>

#include <com/sun/star/document/XTypeDetection.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <comphelper/processfactory.hxx>

#include <unotest/bootstrapfixturebase.hxx>


using namespace css;
using namespace ::cpo;

namespace {

class PriorityFilterTest
    : public test::BootstrapFixtureBase
{
public:
    void testPriority();
    void testFilterCacheDoesNotDeadlock();

    CPPUNIT_TEST_SUITE(PriorityFilterTest);
    CPPUNIT_TEST(testPriority);
    CPPUNIT_TEST(testFilterCacheDoesNotDeadlock);
    CPPUNIT_TEST_SUITE_END();
};

void PriorityFilterTest::testPriority()
{
    uno::Reference<document::XTypeDetection> xDetection(
        comphelper::getProcessServiceFactory()->createInstance(u"com.sun.star.document.TypeDetection"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("No type detection component", xDetection.is());

    static struct {
        const char *pURL;
        const char *pFormat;
    } const aToCheck[] = {
        { "file:///tmp/foo.xls", "calc_MS_Excel_97" }
        // TODO: expand this to check more of these priorities
    };

    for (auto const[pURL, pFormat] : aToCheck)
    {
        OUString aURL = OUString::createFromAscii(pURL);
        try
        {
            OUString aTypeName = xDetection->queryTypeByURL(aURL);

            OUString aFormatCorrect = OUString::createFromAscii(pFormat);
            OUString aMsg = "Mis-matching formats "
                            "'" +
                            aTypeName + "' should be '" + aFormatCorrect + "'";
            CPPUNIT_ASSERT_EQUAL_MESSAGE(OUStringToOString(aMsg,
                                                          RTL_TEXTENCODING_UTF8).getStr(),
                                   aFormatCorrect, aTypeName);
        }
        catch (const cpo::uno::Exception &e)
        {
            OUString aMsg = "Exception querying for type: '" + e.Message + "'";
            CPPUNIT_FAIL(OUStringToOString(aMsg, RTL_TEXTENCODING_UTF8).getStr());
        }
    }
}

void PriorityFilterTest::testFilterCacheDoesNotDeadlock()
{
    uno::Reference<container::XNameContainer> xTD(
        comphelper::getProcessServiceFactory()->createInstance(
            u"com.sun.star.document.TypeDetection"_ustr), uno::UNO_QUERY_THROW);
    {
        uno::Sequence<beans::PropertyValue> aProps;
        xTD->getByName(u"writer8"_ustr) >>= aProps;
        for (auto& rProp : asNonConstRange(aProps))
            if (rProp.Name == "MediaType")
                rProp.Value <<= u"application/x-probe"_ustr;
        xTD->replaceByName(u"writer8"_ustr, uno::Any(aProps));
        uno::Reference<util::XFlushable>(xTD, uno::UNO_QUERY_THROW)->flush();
    }
    {
        uno::Reference<container::XNameContainer> xFF(
            comphelper::getProcessServiceFactory()->createInstance(
                u"com.sun.star.document.FilterFactory"_ustr), uno::UNO_QUERY_THROW);
        uno::Sequence<beans::PropertyValue> aProps;
        xFF->getByName(u"writer8"_ustr) >>= aProps;
        xFF->replaceByName(u"writer8"_ustr, uno::Any(aProps));
        uno::Reference<util::XFlushable>(xFF, uno::UNO_QUERY_THROW)->flush();
    }
    try
    {
        uno::Sequence<beans::PropertyValue> aProps;
        xTD->getByName(u"writer8"_ustr) >>= aProps;
        for (auto& rProp : asNonConstRange(aProps))
            if (rProp.Name == "Name")
                rProp.Value <<= u"probe_new_type"_ustr;
        xTD->insertByName(u"probe_new_type"_ustr, uno::Any(aProps));
        uno::Reference<util::XFlushable>(xTD, uno::UNO_QUERY_THROW)->flush();
    }
    catch (const css::lang::WrappedTargetRuntimeException&)
    {
        // exception is fine, we are testing for deadlock.
    }
    try
    {
        uno::Sequence<beans::PropertyValue> aProps;
        xTD->getByName(u"writer8"_ustr) >>= aProps;
        for (auto& rProp : asNonConstRange(aProps))
            if (rProp.Name == "MediaType")
                rProp.Value <<= u"application/x-probe"_ustr;
        xTD->replaceByName(u"writer8"_ustr, uno::Any(aProps));
        xTD->getByName(u"writer8"_ustr) ;
        uno::Reference<util::XFlushable>(xTD, uno::UNO_QUERY_THROW)->flush();
    }
    catch (const css::lang::WrappedTargetRuntimeException&)
    {
        // exception is fine, we are testing for deadlock.
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(PriorityFilterTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
