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
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/util/XCloseable.hpp>

#include <tools/urlobj.hxx>

#include <ucbhelper/content.hxx>

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
    , m_xFilter()
    , m_xTypeMap()
    , m_pOptionalMap(nullptr)
{
}

void WpftImportTestBase::setUp()
{
    ::test::BootstrapFixture::setUp();

    m_xDesktop = frame::theDesktop::get(m_xContext);

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
                              SfxFilterFlags, SotClipboardFormatId, unsigned int)
{
    if (m_pOptionalMap)
    {
        // first check if this test file is supported by the used version of the library
        const INetURLObject aUrl(rURL);
        const WpftOptionalMap_t::const_iterator it(m_pOptionalMap->find(aUrl.getName()));
        if ((it != m_pOptionalMap->end()) && !it->second)
            return true; // skip the file
    }

    // create an empty frame
    const uno::Reference<lang::XComponent> xDoc(
        m_xDesktop->loadComponentFromURL(m_aFactoryURL, "_blank", 0, uno::Sequence<beans::PropertyValue>()),
        uno::UNO_QUERY_THROW);

    // Find the model and frame. We need them later.
    uno::Reference<frame::XFrame> xFrame(xDoc, uno::UNO_QUERY);
    uno::Reference<frame::XModel> xModel(xDoc, uno::UNO_QUERY);
    uno::Reference<frame::XController> xController(xDoc, uno::UNO_QUERY);

    if (xFrame.is())
    {
        xController = xFrame->getController();
        xModel = xController->getModel();
    }
    else if (xModel.is())
    {
        xController = xModel->getCurrentController();
        xFrame = xController->getFrame();
    }
    else if (xController.is())
    {
        xFrame = xController->getFrame();
        xModel = xController->getModel();
    }

    if (!xFrame.is() || !xModel.is())
        throw uno::RuntimeException();

    bool result = false;

    // try to import the document (and load it into the prepared frame)
    try
    {
        const uno::Reference<document::XImporter> xImporter(m_xFilter, uno::UNO_QUERY_THROW);

        xImporter->setTargetDocument(xDoc);

        uno::Sequence<beans::PropertyValue> aDescriptor(3);
        ucbhelper::Content aContent(rURL, uno::Reference<ucb::XCommandEnvironment>(), m_xContext);

        aDescriptor[0].Name = "URL";
        aDescriptor[0].Value <<= rURL;
        aDescriptor[1].Name = "InputStream";
        aDescriptor[1].Value <<= aContent.openStream();
        aDescriptor[2].Name = "UCBContent";
        aDescriptor[2].Value <<= aContent.get();

        const uno::Reference<document::XExtendedFilterDetection> xDetector(m_xFilter, uno::UNO_QUERY_THROW);

        const rtl::OUString aTypeName(xDetector->detect(aDescriptor));
        if (aTypeName.isEmpty())
            throw lang::IllegalArgumentException();

        impl_detectFilterName(aDescriptor, aTypeName);

        xModel->lockControllers();
        result = m_xFilter->filter(aDescriptor);
        xModel->unlockControllers();
    }
    catch (const uno::Exception &)
    {
        // ignore
    }

    // close the opened document
    try
    {
        uno::Reference<util::XCloseable> xCloseable(xFrame, uno::UNO_QUERY);
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

void WpftImportTestBase::doTest(const rtl::OUString &rFilter, const rtl::OUString &rPath, const WpftOptionalMap_t &rOptionalMap)
{
    m_xFilter.set(m_xFactory->createInstanceWithContext(rFilter, m_xContext), uno::UNO_QUERY_THROW);
    m_pOptionalMap = &rOptionalMap;
    testDir(OUString(), getURLFromSrc(rPath), OUString());
    m_pOptionalMap = nullptr;
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
