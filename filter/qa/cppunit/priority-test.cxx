/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Unit test to check that we get the right filters for the right extensions.

#include <limits>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <sal/types.h>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/document/XTypeDetection.hpp>
#include <comphelper/processfactory.hxx>

#include <unotest/bootstrapfixturebase.hxx>


using namespace std;
using namespace css;

namespace {

class PriorityFilterTest
    : public test::BootstrapFixtureBase
{
public:
    void testPriority();

    CPPUNIT_TEST_SUITE(PriorityFilterTest);
    CPPUNIT_TEST(testPriority);
    CPPUNIT_TEST_SUITE_END();
};

void PriorityFilterTest::testPriority()
{
    uno::Reference<document::XTypeDetection> xDetection(
        comphelper::getProcessServiceFactory()->createInstance("com.sun.star.document.TypeDetection"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_MESSAGE("No type detection component", xDetection.is());

    static struct {
        const char *pURL;
        const char *pFormat;
    } aToCheck[] = {
        { "file:///tmp/foo.xls", "calc_MS_Excel_97" }
        // TODO: expand this to check more of these priorities
    };

    for (size_t i = 0; i < SAL_N_ELEMENTS(aToCheck); i++)
    {
        OUString aURL = OUString::createFromAscii(aToCheck[i].pURL);
        try
        {
            OUString aTypeName = xDetection->queryTypeByURL(aURL);

            OUString aFormatCorrect = OUString::createFromAscii(aToCheck[i].pFormat);
            OUStringBuffer aMsg("Mis-matching formats ");
            aMsg.append("'");
            aMsg.append(aTypeName);
            aMsg.append("' should be '");
            aMsg.append(aFormatCorrect);
            aMsg.append("'");
            CPPUNIT_ASSERT_EQUAL_MESSAGE(rtl::OUStringToOString(aMsg.makeStringAndClear(),
                                                          RTL_TEXTENCODING_UTF8).getStr(),
                                   aFormatCorrect, aTypeName);
        }
        catch (const uno::Exception &e)
        {
            OUStringBuffer aMsg("Exception querying for type: ");
            aMsg.append("'");
            aMsg.append(e.Message);
            aMsg.append("'");
            CPPUNIT_FAIL(rtl::OUStringToOString(aMsg.makeStringAndClear(),
                                                RTL_TEXTENCODING_UTF8).getStr());
        }
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(PriorityFilterTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
