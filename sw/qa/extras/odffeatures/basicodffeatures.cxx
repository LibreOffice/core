/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <config_features.h>

class Test : public SwModelTestBase
{
    public:
        Test() : SwModelTestBase("/sw/qa/extras/odffeatures/data/", "writer8") {}
};

CPPUNIT_TEST_FIXTURE(Test, loadPlainODT)
{
    load(mpTestDocumentPath, "text_plain.odt");
    CPPUNIT_ASSERT_EQUAL( 1, getPages() );
}

CPPUNIT_TEST_FIXTURE(Test, loadBoldTextODT)
{
    load(mpTestDocumentPath, "text_bold.odt");
    CPPUNIT_ASSERT_EQUAL( 1, getPages() );
}

CPPUNIT_PLUGIN_IMPLEMENT();
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
