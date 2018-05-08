/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_QA_API_BASEINDEXTEST_HXX
#define INCLUDED_SW_QA_API_BASEINDEXTEST_HXX

#include "ApiTestBase.hxx"

#include <cppunit/TestAssert.h>
#include <test/unoapi_property_testers.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>

#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XTextSection.hpp>

namespace apitest
{
class BaseIndexTest : public ApiTestBase
{
public:
    void testBaseIndexProperties()
    {
        auto map = init();

        css::uno::Reference<css::beans::XPropertySet> xBaseIndex(map["text::BaseIndex"],
                                                                 css::uno::UNO_QUERY_THROW);
        testStringProperty(xBaseIndex, "Title", "Value");
        testBooleanProperty(xBaseIndex, "IsProtected");

        testStringProperty(xBaseIndex, "ParaStyleHeading", "Value");
        testStringProperty(xBaseIndex, "ParaStyleLevel1", "Value");
        testStringOptionalProperty(xBaseIndex, "ParaStyleLevel2");
        testStringOptionalProperty(xBaseIndex, "ParaStyleLevel3");
        testStringOptionalProperty(xBaseIndex, "ParaStyleLevel4");
        testStringOptionalProperty(xBaseIndex, "ParaStyleLevel5");
        testStringOptionalProperty(xBaseIndex, "ParaStyleLevel6");
        testStringOptionalProperty(xBaseIndex, "ParaStyleLevel7");
        testStringOptionalProperty(xBaseIndex, "ParaStyleLevel8");
        testStringOptionalProperty(xBaseIndex, "ParaStyleLevel9");
        testStringOptionalProperty(xBaseIndex, "ParaStyleLevel10");
        testStringOptionalProperty(xBaseIndex, "ParaStyleSeparator");

        //      [property] XTextColumns TextColumns;
        {
            OUString name = "TextColumns";

            css::uno::Reference<css::text::XTextColumns> xGetTextColumns;
            CPPUNIT_ASSERT(xBaseIndex->getPropertyValue(name) >>= xGetTextColumns);

            xGetTextColumns->setColumnCount(xGetTextColumns->getColumnCount() + 1);
            xBaseIndex->setPropertyValue(name, css::uno::makeAny(xGetTextColumns));

            css::uno::Reference<css::text::XTextColumns> xSetTextColumns;
            CPPUNIT_ASSERT(xBaseIndex->getPropertyValue(name) >>= xSetTextColumns);

            //CPPUNIT_ASSERT_EQUAL(xGetTextColumns->getColumnCount(), xSetTextColumns->getColumnCount());
        }

        //      [property] string BackGraphicURL;
        //      [property] com::sun::star::graphic::XGraphic BackGraphic;
        //      testStringProperty(xBaseIndex, "BackGraphicFilter", "Value");
        //      [property] com::sun::star::style::GraphicLocation BackGraphicLocation;
        testColorProperty(xBaseIndex, "BackColor");
        testBooleanProperty(xBaseIndex, "BackTransparent");
        //      [optional, property] com::sun::star::container::XIndexReplace LevelFormat;
        testBooleanOptionalProperty(xBaseIndex, "CreateFromChapter");

        //      [property] com::sun::star::text::XTextSection ContentSection;
        {
            OUString name = "ContentSection";

            css::uno::Reference<css::text::XTextSection> xGetTextSection;
            CPPUNIT_ASSERT_MESSAGE(name.toUtf8().getStr(),
                                   xBaseIndex->getPropertyValue(name) >>= xGetTextSection);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(name.toUtf8().getStr(), OUString(""),
                                         xGetTextSection->getAnchor()->getString());
        }
        //      [property] com::sun::star::text::XTextSection HeaderSection;
        {
            OUString name = "HeaderSection";

            css::uno::Reference<css::text::XTextSection> xGetTextSection;
            if (xBaseIndex->getPropertyValue(name).hasValue())
                CPPUNIT_ASSERT_MESSAGE(name.toUtf8().getStr(),
                                       xBaseIndex->getPropertyValue(name) >>= xGetTextSection);
        }
    }
};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
