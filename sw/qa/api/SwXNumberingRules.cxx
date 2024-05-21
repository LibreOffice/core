/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <test/container/xelementaccess.hxx>
#include <test/beans/xpropertyset.hxx>
#include <test/container/xindexaccess.hxx>
#include <test/text/numberingrules.hxx>

#include <com/sun/star/frame/Desktop.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

using namespace css;
using namespace css::uno;

namespace
{
/**
 * Initial tests for SwXNumberingRules.
 */
class SwXNumberingRules final : public UnoApiTest,
                                public apitest::XElementAccess,
                                public apitest::XIndexAccess,
                                public apitest::XPropertySet,
                                public apitest::NumberingRules
{
public:
    SwXNumberingRules()
        : UnoApiTest(u""_ustr)
        , XElementAccess(cppu::UnoType<Sequence<beans::PropertyValue>>::get())
        , XIndexAccess(10)
    {
    }

    virtual void setUp() override
    {
        UnoApiTest::setUp();
        mxDesktop.set(frame::Desktop::create(mxComponentContext));
        mxComponent = loadFromDesktop(u"private:factory/swriter"_ustr);
        CPPUNIT_ASSERT(mxComponent.is());
    }

    Reference<XInterface> init() override
    {
        Reference<text::XTextDocument> xTextDocument(mxComponent, UNO_QUERY_THROW);

        Reference<container::XIndexAccess> xIndexAccess;
        Reference<container::XIndexAccess> xNumRules;

        Reference<text::XText> xText = xTextDocument->getText();
        Reference<text::XTextCursor> xCursor = xText->createTextCursor();

        try
        {
            xText->insertString(xCursor, u"The quick brown fox jumps over the lazy dog"_ustr,
                                false);
            xText->insertControlCharacter(xCursor, text::ControlCharacter::PARAGRAPH_BREAK, false);
        }
        catch (lang::IllegalArgumentException&)
        {
        }

        Reference<style::XStyleFamiliesSupplier> xStyleFam(xTextDocument, UNO_QUERY_THROW);

        try
        {
            Reference<container::XNameAccess> xStyleFamNames = xStyleFam->getStyleFamilies();
            Reference<container::XNameContainer> xNumStyles(
                xStyleFamNames->getByName(u"NumberingStyles"_ustr), UNO_QUERY_THROW);
            xIndexAccess = Reference<container::XIndexAccess>(xNumStyles, UNO_QUERY_THROW);
        }
        catch (lang::WrappedTargetException&)
        {
        }
        catch (container::NoSuchElementException&)
        {
        }
        catch (lang::IllegalArgumentException&)
        {
        }

        try
        {
            Reference<beans::XPropertySet> xPropSet(xIndexAccess->getByIndex(0), UNO_QUERY_THROW);
            xNumRules = Reference<container::XIndexAccess>(
                xPropSet->getPropertyValue(u"NumberingRules"_ustr), UNO_QUERY_THROW);
        }
        catch (lang::WrappedTargetException&)
        {
        }
        catch (lang::IndexOutOfBoundsException&)
        {
        }
        catch (beans::UnknownPropertyException&)
        {
        }
        catch (lang::IllegalArgumentException&)
        {
        }

        return Reference<XInterface>(xNumRules, UNO_QUERY_THROW);
    }

    CPPUNIT_TEST_SUITE(SwXNumberingRules);
    CPPUNIT_TEST(testGetElementType);
    CPPUNIT_TEST(testHasElements);
    CPPUNIT_TEST(testGetCount);
    CPPUNIT_TEST(testGetByIndex);
    CPPUNIT_TEST(testGetPropertySetInfo);
    CPPUNIT_TEST(testSetPropertyValue);
    CPPUNIT_TEST(testGetPropertyValue);
    CPPUNIT_TEST(testPropertyChangeListener);
    CPPUNIT_TEST(testVetoableChangeListener);
    CPPUNIT_TEST(testNumberingRulesProperties);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SwXNumberingRules);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
