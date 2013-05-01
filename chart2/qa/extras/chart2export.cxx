/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "charttest.hxx"

class Chart2ExportTest : public ChartTest
{
public:
    void test();

    CPPUNIT_TEST_SUITE(Chart2ExportTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();

private:
};

void Chart2ExportTest::test()
{
    load("/chart2/qa/extras/data/ods/", "simple_export_chart.ods");
    reload("Calc Office Open XML");
}

CPPUNIT_TEST_SUITE_REGISTRATION(Chart2ExportTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
