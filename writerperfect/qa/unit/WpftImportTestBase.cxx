/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XTypeDetection.hpp>
#include <com/sun/star/frame/theDesktop.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/util/XCloseable.hpp>

#include "WpftImportTestBase.hxx"

namespace beans = com::sun::star::beans;
namespace container = com::sun::star::container;
namespace document = com::sun::star::document;
namespace frame = com::sun::star::frame;
namespace io = com::sun::star::io;
namespace lang = com::sun::star::lang;
namespace ucb = com::sun::star::ucb;
namespace uno = com::sun::star::uno;
namespace util = com::sun::star::util;

namespace writerperfect
{
namespace test
{

WpftImportTestBase::WpftImportTestBase(const rtl::OUString &rFactoryURL)
    : ::test::FiltersTest()
    , ::test::BootstrapFixture()
    , m_aFactoryURL(rFactoryURL)
    , m_xDesktop()
    , m_xFileAccess()
    , m_xFilter()
    , m_xTypeMap()
{
}

void WpftImportTestBase::setUp()
{
    ::test::BootstrapFixture::setUp();

    m_xDesktop = frame::theDesktop::get(m_xContext);
    m_xFileAccess = ucb::SimpleFileAccess::create(m_xContext);

    const uno::Reference<document::XTypeDetection> xTypeDetection(
            m_xFactory->createInstanceWithContext("com.sun.star.document.TypeDetection", m_xContext),
            uno::UNO_QUERY_THROW);
    m_xTypeMap.set(xTypeDetection, uno::UNO_QUERY_THROW);
}

void WpftImportTestBase::tearDown()
{
    m_xDesktop->terminate();

    ::test::BootstrapFixture::tearDown();
}

bool WpftImportTestBase::load(const OUString &, const OUString &rURL, const OUString &,
    unsigned int, unsigned int, unsigned int)
{
    // create an empty frame
    const uno::Reference<lang::XComponent> xDoc(
            m_xDesktop->loadComponentFromURL(m_aFactoryURL, "_blank", 0, uno::Sequence<beans::PropertyValue>()),
            uno::UNO_QUERY_THROW);

    bool result = false;

    // try to import the document (and load it into the prepared frame)
    try
    {
        const uno::Reference<document::XImporter> xImporter(m_xFilter, uno::UNO_QUERY_THROW);

        xImporter->setTargetDocument(xDoc);

        uno::Sequence<beans::PropertyValue> aDescriptor(2);
        aDescriptor[0].Name = "URL";
        aDescriptor[0].Value <<= rURL;

        const uno::Reference<io::XInputStream> xInputStream(m_xFileAccess->openFileRead(rURL), uno::UNO_QUERY_THROW);
        aDescriptor[1].Name = "InputStream";
        aDescriptor[1].Value <<= xInputStream;

        const uno::Reference<document::XExtendedFilterDetection> xDetector(m_xFilter, uno::UNO_QUERY_THROW);

        const rtl::OUString aTypeName(xDetector->detect(aDescriptor));
        if (aTypeName.isEmpty())
            throw lang::IllegalArgumentException();

        impl_detectFilterName(aDescriptor, aTypeName);

        result = m_xFilter->filter(aDescriptor);
    }
    catch (const uno::Exception &)
    {
        // ignore
    }

    // close the opened document
    uno::Reference<util::XCloseable> xCloseable(xDoc, uno::UNO_QUERY);

    if (!xCloseable.is())
    {
        uno::Reference<frame::XController> xController(xDoc, uno::UNO_QUERY);

        if (!xController.is())
        {
            const uno::Reference<frame::XModel> xModel(xDoc, uno::UNO_QUERY);
            if (xModel.is())
                xController = xModel->getCurrentController();
        }

        if (xController.is())
        {
            const uno::Reference<frame::XFrame> xFrame = xController->getFrame();
            if (xFrame.is())
                xCloseable.set(xFrame, uno::UNO_QUERY);
        }
    }

    try
    {
        if (xCloseable.is())
            xCloseable->close(true);
        else
            xDoc->dispose();
    }
    catch (const uno::Exception &)
    {
        // ignore
    }

    return result;
}

void WpftImportTestBase::doTest(const rtl::OUString &rFilter, const rtl::OUString &rPath)
{
    m_xFilter.set(m_xFactory->createInstanceWithContext(rFilter, m_xContext), uno::UNO_QUERY_THROW);
    testDir(OUString(), getURLFromSrc(rPath), OUString());
}

void WpftImportTestBase::impl_detectFilterName(uno::Sequence<beans::PropertyValue> &rDescriptor, const rtl::OUString &rTypeName)
{
    const sal_Int32 nDescriptorLen = rDescriptor.getLength();

    for (sal_Int32 n = 0; nDescriptorLen != n; ++n)
    {
        if ("FilterName" == rDescriptor[n].Name)
            return;
    }

    uno::Sequence<beans::PropertyValue> aTypes;
    if (m_xTypeMap->getByName(rTypeName) >>= aTypes)
    {
        for (sal_Int32 n = 0; aTypes.getLength() != n; ++n)
        {
            rtl::OUString aFilterName;
            if (("PreferredFilter" == aTypes[n].Name) && (aTypes[n].Value >>= aFilterName))
            {
                rDescriptor.realloc(nDescriptorLen + 1);
                rDescriptor[nDescriptorLen].Name = "FilterName";
                rDescriptor[nDescriptorLen].Value <<= aFilterName;
                return;
            }
        }
    }

    throw container::NoSuchElementException();
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
