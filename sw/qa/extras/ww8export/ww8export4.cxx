/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

#include <comphelper/sequenceashashmap.hxx>
#include <o3tl/string_view.hxx>

#include <docsh.hxx>
#include <IDocumentSettingAccess.hxx>
#include <unotxdoc.hxx>

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/ww8export/data/", "MS Word 97")
    {
    }

    bool mustTestImportOf(const char* filename) const override
    {
        // If the testcase is stored in some other format, it's pointless to test.
        return o3tl::ends_with(filename, ".doc");
    }
};

CPPUNIT_TEST_FIXTURE(Test, testTdf148360)
{
    loadAndReload("tdf148360.doc");
    const auto& pLayout = parseLayoutDump();

    // Ensure first element is a tab
    assertXPath(pLayout, "/root/page[1]/body/txt[1]/Text[1]", "nType", "PortionType::TabLeft");
    // and only then goes content
    assertXPath(pLayout, "/root/page[1]/body/txt[1]/Text[2]", "nType", "PortionType::Text");
}



CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
