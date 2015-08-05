/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/txtimport/data/", "Text") {}
};

#define DECLARE_TXTIMPORT_ENC_TEST(TestName, filename) \
    DECLARE_SW_IMPORT_TEST(TestName, filename, Test) \
    {\
        uno::Reference< text::XTextRange > xTextRange = getRun( getParagraph(1), 1, "Hello World" );\
    }

DECLARE_TXTIMPORT_ENC_TEST(testASCII, "ascii.txt")
DECLARE_TXTIMPORT_ENC_TEST(testUTF16BE, "UTF16BE.txt")
DECLARE_TXTIMPORT_ENC_TEST(testUTF16LE, "UTF16LE.txt")
DECLARE_TXTIMPORT_ENC_TEST(testUTF16BEbare, "UTF16BE_bare.txt")
DECLARE_TXTIMPORT_ENC_TEST(testUTF16LEbare, "UTF16LE_bare.txt")
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
