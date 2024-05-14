/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XTextSection.hpp>

#include <vcl/BitmapTools.hxx>
#include <vcl/filter/PngImageWriter.hxx>
#include <unotools/tempfile.hxx>
#include <tools/stream.hxx>

#include <test/unoapi_property_testers.hxx>
#include <test/text/baseindex.hxx>

namespace
{
BitmapEx createExampleBitmap()
{
    vcl::bitmap::RawBitmap aRawBitmap(Size(4, 4), 24);
    aRawBitmap.SetPixel(0, 0, COL_LIGHTBLUE);
    aRawBitmap.SetPixel(0, 1, COL_LIGHTGREEN);
    aRawBitmap.SetPixel(1, 0, COL_LIGHTRED);
    aRawBitmap.SetPixel(1, 1, COL_LIGHTMAGENTA);
    return vcl::bitmap::CreateFromData(std::move(aRawBitmap));
}

void writerFileWithBitmap(OUString const& rURL)
{
    BitmapEx aBitmapEx = createExampleBitmap();
    SvFileStream aFileStream(rURL, StreamMode::READ | StreamMode::WRITE);
    vcl::PngImageWriter aWriter(aFileStream);
    aWriter.write(aBitmapEx);
    aFileStream.Seek(STREAM_SEEK_TO_BEGIN);
    aFileStream.Close();
}
}

namespace apitest
{
BaseIndex::~BaseIndex() {}

void BaseIndex::testBaseIndexProperties()
{
    css::uno::Reference<css::beans::XPropertySet> xBaseIndex(init(), css::uno::UNO_QUERY_THROW);
    testStringProperty(xBaseIndex, u"Title"_ustr, u"Value"_ustr);
    testBooleanProperty(xBaseIndex, u"IsProtected"_ustr);

    testStringProperty(xBaseIndex, u"ParaStyleHeading"_ustr, u"Value"_ustr);
    testStringProperty(xBaseIndex, u"ParaStyleLevel1"_ustr, u"Value"_ustr);
    testStringOptionalProperty(xBaseIndex, u"ParaStyleLevel2"_ustr);
    testStringOptionalProperty(xBaseIndex, u"ParaStyleLevel3"_ustr);
    testStringOptionalProperty(xBaseIndex, u"ParaStyleLevel4"_ustr);
    testStringOptionalProperty(xBaseIndex, u"ParaStyleLevel5"_ustr);
    testStringOptionalProperty(xBaseIndex, u"ParaStyleLevel6"_ustr);
    testStringOptionalProperty(xBaseIndex, u"ParaStyleLevel7"_ustr);
    testStringOptionalProperty(xBaseIndex, u"ParaStyleLevel8"_ustr);
    testStringOptionalProperty(xBaseIndex, u"ParaStyleLevel9"_ustr);
    testStringOptionalProperty(xBaseIndex, u"ParaStyleLevel10"_ustr);
    testStringOptionalProperty(xBaseIndex, u"ParaStyleSeparator"_ustr);

    //      [property] XTextColumns TextColumns;
    {
        OUString name = u"TextColumns"_ustr;

        css::uno::Reference<css::text::XTextColumns> xGetTextColumns;
        CPPUNIT_ASSERT(xBaseIndex->getPropertyValue(name) >>= xGetTextColumns);

        xGetTextColumns->setColumnCount(xGetTextColumns->getColumnCount() + 1);
        xBaseIndex->setPropertyValue(name, css::uno::Any(xGetTextColumns));

        css::uno::Reference<css::text::XTextColumns> xSetTextColumns;
        CPPUNIT_ASSERT(xBaseIndex->getPropertyValue(name) >>= xSetTextColumns);

        //CPPUNIT_ASSERT_EQUAL(xGetTextColumns->getColumnCount(), xSetTextColumns->getColumnCount());
    }

    //      [property] com::sun::star::graphic::XGraphic BackGraphic;
    //      [property] string BackGraphicURL;
    {
        OUString name = u"BackGraphicURL"_ustr;
        bool bOK = false;
        try
        {
            xBaseIndex->getPropertyValue(name);
        }
        catch (css::uno::RuntimeException const& /*ex*/)
        {
            bOK = true;
        }
        // BackGraphicURL is "set-only" attribute
        CPPUNIT_ASSERT_MESSAGE("Expected RuntimeException wasn't thrown", bOK);

        utl::TempFileNamed aTempFile;
        aTempFile.EnableKillingFile();
        writerFileWithBitmap(aTempFile.GetURL());

        css::uno::Reference<css::graphic::XGraphic> xGraphic;
        CPPUNIT_ASSERT(xBaseIndex->getPropertyValue(u"BackGraphic"_ustr) >>= xGraphic);
        CPPUNIT_ASSERT(!xGraphic.is());

        xBaseIndex->setPropertyValue(name, css::uno::Any(aTempFile.GetURL()));

        CPPUNIT_ASSERT(xBaseIndex->getPropertyValue(u"BackGraphic"_ustr) >>= xGraphic);
        CPPUNIT_ASSERT(xGraphic.is());
    }

    testStringProperty(xBaseIndex, u"BackGraphicFilter"_ustr, u"Value"_ustr);

    //      [property] com::sun::star::style::GraphicLocation BackGraphicLocation;

    testColorProperty(xBaseIndex, u"BackColor"_ustr);
    testBooleanProperty(xBaseIndex, u"BackTransparent"_ustr);

    //      [optional, property] com::sun::star::container::XIndexReplace LevelFormat;

    testBooleanOptionalProperty(xBaseIndex, u"CreateFromChapter"_ustr);

    //      [property] com::sun::star::text::XTextSection ContentSection;
    {
        OUString name = u"ContentSection"_ustr;

        css::uno::Reference<css::text::XTextSection> xGetTextSection;
        CPPUNIT_ASSERT_MESSAGE(name.toUtf8().getStr(),
                               xBaseIndex->getPropertyValue(name) >>= xGetTextSection);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(name.toUtf8().getStr(), u""_ustr,
                                     xGetTextSection->getAnchor()->getString());
    }

    //      [property] com::sun::star::text::XTextSection HeaderSection;
    {
        OUString name = u"HeaderSection"_ustr;

        css::uno::Reference<css::text::XTextSection> xGetTextSection;
        if (xBaseIndex->getPropertyValue(name).hasValue())
            CPPUNIT_ASSERT_MESSAGE(name.toUtf8().getStr(),
                                   xBaseIndex->getPropertyValue(name) >>= xGetTextSection);
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
