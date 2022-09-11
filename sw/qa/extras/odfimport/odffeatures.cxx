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

constexpr OUStringLiteral DATA_DIRECTORY = u"/sw/qa/extras/odfimport/data/";

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(DATA_DIRECTORY, "writer8")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testFeatureText) { load(mpTestDocumentPath, "feature_text.odt"); }

CPPUNIT_TEST_FIXTURE(Test, testFeatureTextBold)
{
    load(mpTestDocumentPath, "feature_text_bold.odt");
}

CPPUNIT_TEST_FIXTURE(Test, testFeatureTextItalic)
{
    load(mpTestDocumentPath, "feature_text_italic.odt");
}

CPPUNIT_PLUGIN_IMPLEMENT();
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
