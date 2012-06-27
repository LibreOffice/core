/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 *   Copyright (C) 2012 Kohei Yoshida <kohei.yoshida@suse.com>
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "test/bootstrapfixture.hxx"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "svl/itempool.hxx"
#include "editeng/eerdll.hxx"
#include "editeng/eerdll2.hxx"
#include "editeng/editeng.hxx"
#include "editeng/eeitem.hxx"
#include "editeng/editids.hrc"
#include "editeng/editdoc.hxx"
#include "editeng/unofield.hxx"

#include <com/sun/star/text/textfield/Type.hpp>

using namespace com::sun::star;

namespace {

class Test : public test::BootstrapFixture
{
public:
    Test();

    virtual void setUp();
    virtual void tearDown();

    void testConstruction();

    /**
     * Test UNO service class that implements text field items.
     */
    void testUnoTextFields();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testConstruction);
    CPPUNIT_TEST(testUnoTextFields);
    CPPUNIT_TEST_SUITE_END();

private:
    EditEngineItemPool* mpItemPool;
};

Test::Test() : mpItemPool(NULL) {}

void Test::setUp()
{
    test::BootstrapFixture::setUp();

    mpItemPool = new EditEngineItemPool(true);
}

void Test::tearDown()
{
    SfxItemPool::Free(mpItemPool);
    test::BootstrapFixture::tearDown();
}

void Test::testConstruction()
{
    EditEngine aEngine(mpItemPool);

    rtl::OUString aParaText = "I am Edit Engine.";
    aEngine.SetText(aParaText);
}

namespace {

bool includes(const uno::Sequence<rtl::OUString>& rSeq, const rtl::OUString& rVal)
{
    for (sal_Int32 i = 0, n = rSeq.getLength(); i < n; ++i)
        if (rSeq[i] == rVal)
            return true;

    return false;
}

}

void Test::testUnoTextFields()
{
    {
        // DATE
        SvxUnoTextField aField(text::textfield::Type::DATE);
        uno::Sequence<rtl::OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.DateTime");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // URL
        SvxUnoTextField aField(text::textfield::Type::URL);
        uno::Sequence<rtl::OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.URL");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // PAGE
        SvxUnoTextField aField(text::textfield::Type::PAGE);
        uno::Sequence<rtl::OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.PageNumber");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // PAGES
        SvxUnoTextField aField(text::textfield::Type::PAGES);
        uno::Sequence<rtl::OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.PageCount");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // TIME
        SvxUnoTextField aField(text::textfield::Type::TIME);
        uno::Sequence<rtl::OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.DateTime");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // FILE
        SvxUnoTextField aField(text::textfield::Type::DOCINFO_TITLE);
        uno::Sequence<rtl::OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.docinfo.Title");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // TABLE
        SvxUnoTextField aField(text::textfield::Type::TABLE);
        uno::Sequence<rtl::OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.SheetName");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // EXTENDED TIME
        SvxUnoTextField aField(text::textfield::Type::EXTENDED_TIME);
        uno::Sequence<rtl::OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.DateTime");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // EXTENDED FILE
        SvxUnoTextField aField(text::textfield::Type::EXTENDED_FILE);
        uno::Sequence<rtl::OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.FileName");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // AUTHOR
        SvxUnoTextField aField(text::textfield::Type::AUTHOR);
        uno::Sequence<rtl::OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.Author");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // MEASURE
        SvxUnoTextField aField(text::textfield::Type::MEASURE);
        uno::Sequence<rtl::OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.text.textfield.Measure");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // PRESENTATION HEADER
        SvxUnoTextField aField(text::textfield::Type::PRESENTATION_HEADER);
        uno::Sequence<rtl::OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.presentation.textfield.Header");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // PRESENTATION FOOTER
        SvxUnoTextField aField(text::textfield::Type::PRESENTATION_FOOTER);
        uno::Sequence<rtl::OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.presentation.textfield.Footer");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }

    {
        // PRESENTATION DATE TIME
        SvxUnoTextField aField(text::textfield::Type::PRESENTATION_DATE_TIME);
        uno::Sequence<rtl::OUString> aSvcs = aField.getSupportedServiceNames();
        bool bGood = includes(aSvcs, "com.sun.star.presentation.textfield.DateTime");
        CPPUNIT_ASSERT_MESSAGE("expected service is not present.", bGood);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
