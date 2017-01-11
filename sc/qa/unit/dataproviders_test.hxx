/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_QA_UNIT_DATAPROVIDER_TEST_HXX
#define INCLUDED_SC_QA_UNIT_DATAPROVIDER_TEST_HXX

#include "helper/qahelper.hxx"
#include "document.hxx"
#include <stringutil.hxx>
#include <memory>

struct TestImpl;

class ScDataProvidersTest : public test::BootstrapFixture
{
public:

    ScDataProvidersTest();
    virtual ~ScDataProvidersTest() override;

    ScDocShell& getDocShell();

    virtual void setUp() override;
    virtual void tearDown() override;

    void testCSVImport();

    CPPUNIT_TEST_SUITE(ScDataProvidersTest);
    CPPUNIT_TEST(testCSVImport);
    CPPUNIT_TEST_SUITE_END();

private:
    std::unique_ptr<TestImpl> m_pImpl;
    ScDocument *m_pDoc;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
