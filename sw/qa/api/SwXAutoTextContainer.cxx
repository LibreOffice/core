/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/container/xnameaccess.hxx>
#include <test/container/xindexaccess.hxx>
#include <test/container/xelementaccess.hxx>
#include <test/text/xautotextcontainer.hxx>

#include <com/sun/star/frame/Desktop.hpp>

#include <com/sun/star/text/XAutoTextContainer.hpp>
#include <com/sun/star/text/AutoTextContainer.hpp>

#include <comphelper/processfactory.hxx>

using namespace css;
using namespace css::uno;

namespace
{
/**
 * Initial tests for SwXAutoTextContainer.
 */
class SwXAutoTextContainer final : public UnoApiTest,
                                   public apitest::XElementAccess,
                                   public apitest::XIndexAccess,
                                   public apitest::XNameAccess
{
public:
    SwXAutoTextContainer()
        : UnoApiTest(u""_ustr)
        , XElementAccess(cppu::UnoType<text::XAutoTextGroup>::get())
        , XIndexAccess(3)
        , XNameAccess(u"crdbus50"_ustr)
    {
    }

    virtual void setUp() override
    {
        UnoApiTest::setUp();
        mxDesktop.set(frame::Desktop::create(mxComponentContext));
    }

    Reference<XInterface> init() override
    {
        Reference<text::XAutoTextContainer> xAutoTextContainer
            = text::AutoTextContainer::create(comphelper::getProcessComponentContext());

        Reference<container::XNameAccess> xNA(xAutoTextContainer, UNO_QUERY_THROW);
        Sequence<rtl::OUString> aNames = xNA->getElementNames();
        std::cout << aNames[0] << std::endl;

        return Reference<XInterface>(xAutoTextContainer, UNO_QUERY_THROW);
    }

    CPPUNIT_TEST_SUITE(SwXAutoTextContainer);
    CPPUNIT_TEST(testGetByName);
    CPPUNIT_TEST(testGetElementNames);
    CPPUNIT_TEST(testHasByName);
    CPPUNIT_TEST(testGetCount);
    CPPUNIT_TEST(testGetByIndex);
    CPPUNIT_TEST(testGetElementType);
    CPPUNIT_TEST(testHasElements);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SwXAutoTextContainer);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
