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

class MockErrorHandler : private ErrorHandler
{
    friend class ErrorHandlerTest;

protected:
    virtual bool CreateString(const ErrorInfo *pErrInfo, OUString &rErrString) const override
    {
        if (pErrInfo->GetErrorCode().IsDynamic())
            rErrString = "Dynamic error";
        else
            rErrString = "Non-dynamic error";

        return true;
    }
};


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
    std::unique_ptr<ErrorInfo> xErrorInfo;
    OUString aErrStr;

    CPPUNIT_ASSERT_MESSAGE("GetErrorString(ERRCODE_ABORT, aErrStr) should return false",
                           !ErrorHandler::GetErrorString(ERRCODE_ABORT, aErrStr));
    // normally protected, but MockErrorHandler is a friend of this class
    xErrorInfo = ErrorInfo::GetErrorInfo(ERRCODE_ABORT);
    aErrHdlr.CreateString(xErrorInfo.get(), aErrStr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("error message should be non-dynamic", OUString("Non-dynamic error"), aErrStr);

    CPPUNIT_ASSERT_MESSAGE("GetErrorString(ERRCODE_NONE, aErrStr) should return false",
                           !ErrorHandler::GetErrorString(ERRCODE_NONE, aErrStr));
    xErrorInfo = ErrorInfo::GetErrorInfo(ERRCODE_NONE);
    aErrHdlr.CreateString(xErrorInfo.get(), aErrStr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("error message should be non-dynamic", OUString("Non-dynamic error"), aErrStr);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ErrorHandlerTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
