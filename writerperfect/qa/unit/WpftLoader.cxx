/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "WpftLoader.hxx"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XTypeDetection.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XDesktop2.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XCloseable.hpp>

#include <ucbhelper/content.hxx>

namespace beans = com::sun::star::beans;
namespace container = com::sun::star::container;
namespace document = com::sun::star::document;
namespace frame = com::sun::star::frame;
namespace lang = com::sun::star::lang;
namespace ucb = com::sun::star::ucb;
namespace uno = com::sun::star::uno;
namespace util = com::sun::star::util;

namespace writerperfect
{
namespace test
{

WpftLoader::WpftLoader(
    const rtl::OUString &rURL,
    const css::uno::Reference<css::document::XFilter> &rxFilter,
    const rtl::OUString &rFactoryURL,
    const css::uno::Reference<css::frame::XDesktop2> &rxDesktop,
    const css::uno::Reference<css::container::XNameAccess> &rxTypeMap,
    const css::uno::Reference<css::uno::XComponentContext> &rxContext
)
    : m_aURL(rURL)
    , m_aFactoryURL(rFactoryURL)
    , m_xFilter(rxFilter)
    , m_xDesktop(rxDesktop)
    , m_xTypeMap(rxTypeMap)
    , m_xContext(rxContext)
{
    if (!impl_load())
        impl_dispose();
}

WpftLoader::WpftLoader(
    const css::uno::Reference<css::io::XInputStream> &rxInputStream,
    const css::uno::Reference<css::document::XFilter> &rxFilter,
    const rtl::OUString &rFactoryURL,
    const css::uno::Reference<css::frame::XDesktop2> &rxDesktop,
    const css::uno::Reference<css::uno::XComponentContext> &rxContext
)
    : m_xInputStream(rxInputStream)
    , m_aFactoryURL(rFactoryURL)
    , m_xFilter(rxFilter)
    , m_xDesktop(rxDesktop)
    , m_xContext(rxContext)
{
    if (!impl_load())
        impl_dispose();
}

WpftLoader::~WpftLoader()
{
    try
    {
        impl_dispose();
    }
    catch (...)
    {
    }
}

const css::uno::Reference<css::lang::XComponent> &WpftLoader::getDocument() const
{
    return m_xDoc;
}

bool WpftLoader::impl_load()
{
    // create an empty frame
    m_xDoc.set(
        m_xDesktop->loadComponentFromURL(m_aFactoryURL, "_blank", 0, uno::Sequence<beans::PropertyValue>()),
        uno::UNO_QUERY_THROW);

    // Find the model and frame. We need them later.
    m_xFrame.set(m_xDoc, uno::UNO_QUERY);
    uno::Reference<frame::XModel> xModel(m_xDoc, uno::UNO_QUERY);
    uno::Reference<frame::XController> xController(m_xDoc, uno::UNO_QUERY);

    if (m_xFrame.is())
    {
        xController = m_xFrame->getController();
        xModel = xController->getModel();
    }
    else if (xModel.is())
    {
        xController = xModel->getCurrentController();
        m_xFrame = xController->getFrame();
    }
    else if (xController.is())
    {
        m_xFrame = xController->getFrame();
        xModel = xController->getModel();
    }

    if (!m_xFrame.is() || !xModel.is())
        throw uno::RuntimeException();

    // try to import the document (and load it into the prepared frame)
    try
    {
        const uno::Reference<document::XImporter> xImporter(m_xFilter, uno::UNO_QUERY_THROW);

        xImporter->setTargetDocument(m_xDoc);

        uno::Sequence<beans::PropertyValue> aDescriptor(3);
        aDescriptor[0].Name = "URL";
        aDescriptor[0].Value <<= m_aURL;
        if (m_xInputStream.is())
        {
            aDescriptor[1].Name = "InputStream";
            aDescriptor[1].Value <<= m_xInputStream;
        }
        else
        {
            ucbhelper::Content aContent(m_aURL, uno::Reference<ucb::XCommandEnvironment>(), m_xContext);
            aDescriptor[1].Name = "InputStream";
            aDescriptor[1].Value <<= aContent.openStream();
            aDescriptor[2].Name = "UCBContent";
            aDescriptor[2].Value <<= aContent.get();
        }

        const uno::Reference<document::XExtendedFilterDetection> xDetector(m_xFilter, uno::UNO_QUERY_THROW);

        const rtl::OUString aTypeName(xDetector->detect(aDescriptor));
        if (aTypeName.isEmpty())
            throw lang::IllegalArgumentException();

        if (m_xTypeMap.is())
            impl_detectFilterName(aDescriptor, aTypeName);

        xModel->lockControllers();
        const bool bLoaded = m_xFilter->filter(aDescriptor);
        xModel->unlockControllers();
        return bLoaded;
    }
    catch (const uno::Exception &)
    {
        // ignore
    }

    return false;
}

void WpftLoader::impl_dispose()
{
    // close the opened document
    uno::Reference<util::XCloseable> xCloseable(m_xFrame, uno::UNO_QUERY);
    if (xCloseable.is())
        xCloseable->close(true);
    else if (m_xDoc.is())
        m_xDoc->dispose();
    m_xDoc.clear();
    m_xFrame.clear();
}

void WpftLoader::impl_detectFilterName(uno::Sequence<beans::PropertyValue> &rDescriptor, const rtl::OUString &rTypeName)
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
