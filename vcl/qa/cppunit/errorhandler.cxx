/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <cppunit/TestAssert.h>

#include <vcl/errinf.hxx>

class ErrorHandlerTest;

namespace {

class MockErrorHandler : private ErrorHandler
{
    friend ErrorHandlerTest;

protected:
    virtual bool CreateString(const ErrCodeMsg&, OUString &rErrString) const override
    {
        rErrString = "Non-dynamic error";
        return true;
    }
};

}

class ErrorHandlerTest : public test::BootstrapFixture
{
public:
    ErrorHandlerTest() : BootstrapFixture(true, false) {}

    void testGetErrorString();

    CPPUNIT_TEST_SUITE(ErrorHandlerTest);
    CPPUNIT_TEST(testGetErrorString);
    CPPUNIT_TEST_SUITE_END();
};

void ErrorHandlerTest::testGetErrorString()
{
    MockErrorHandler aErrHdlr;
    OUString aErrStr;

    CPPUNIT_ASSERT_MESSAGE("GetErrorString(ERRCODE_ABORT, aErrStr) should return false",
                           !ErrorHandler::GetErrorString(ERRCODE_ABORT, aErrStr));
    // normally protected, but MockErrorHandler is a friend of this class
    aErrHdlr.CreateString(ERRCODE_ABORT, aErrStr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("error message should be non-dynamic", u"Non-dynamic error"_ustr, aErrStr);

    CPPUNIT_ASSERT_MESSAGE("GetErrorString(ERRCODE_NONE, aErrStr) should return false",
                           !ErrorHandler::GetErrorString(ERRCODE_NONE, aErrStr));
    aErrHdlr.CreateString(ERRCODE_NONE, aErrStr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("error message should be non-dynamic", u"Non-dynamic error"_ustr, aErrStr);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ErrorHandlerTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
