/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/document/XExtendedFilterDetection.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>
#include <test/bootstrapfixture.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/streamwrap.hxx>
#include <tools/stream.hxx>

namespace com::sun::star::io
{
class XInputStream;
}

using namespace com::sun::star;

namespace
{
/// Test class for PlainTextFilterDetect.
class TextFilterDetectTest : public test::BootstrapFixture
{
};

char const DATA_DIRECTORY[] = "/filter/qa/unit/data/";

CPPUNIT_TEST_FIXTURE(TextFilterDetectTest, testTdf114428)
{
    uno::Reference<uno::XComponentContext> xComponentContext
        = comphelper::getComponentContext(getMultiServiceFactory());
    uno::Reference<document::XExtendedFilterDetection> xDetect(
        getMultiServiceFactory()->createInstance("com.sun.star.comp.filters.PlainTextFilterDetect"),
        uno::UNO_QUERY);
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "tdf114428.xhtml";
    SvFileStream aStream(aURL, StreamMode::READ);
    uno::Reference<io::XInputStream> xStream(new utl::OStreamWrapper(aStream));
    uno::Sequence<beans::PropertyValue> aDescriptor
        = { comphelper::makePropertyValue("DocumentService",
                                          OUString("com.sun.star.text.TextDocument")),
            comphelper::makePropertyValue("InputStream", xStream),
            comphelper::makePropertyValue("TypeName", OUString("generic_HTML")) };
    xDetect->detect(aDescriptor);
    utl::MediaDescriptor aMediaDesc(aDescriptor);
    OUString aFilterName = aMediaDesc.getUnpackedValueOrDefault("FilterName", OUString());
    // This was empty, XML declaration caused HTML detect to not handle XHTML.
    CPPUNIT_ASSERT_EQUAL(OUString("HTML (StarWriter)"), aFilterName);
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
