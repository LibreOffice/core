/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <svtools/imageresourceaccess.hxx>

#include <com/sun/star/io/NotConnectedException.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/io/XStream.hpp>

#include <comphelper/propertyvalue.hxx>
#include <o3tl/string_view.hxx>
#include <osl/diagnose.h>
#include <tools/stream.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <unotools/streamwrap.hxx>
#include <cppuhelper/implbase.hxx>
#include <utility>

namespace svt::GraphicAccess
{

using namespace ::utl;
using namespace css;

typedef ::cppu::WeakImplHelper<io::XStream, io::XSeekable> StreamSupplier_Base;

namespace {

class StreamSupplier : public StreamSupplier_Base
{
private:
    uno::Reference<io::XInputStream> m_xInput;
    uno::Reference<io::XOutputStream> m_xOutput;
    uno::Reference<io::XSeekable> m_xSeekable;

public:
    StreamSupplier(uno::Reference<io::XInputStream> xInput, uno::Reference<io::XOutputStream> xOutput);

protected:
    // XStream
    virtual uno::Reference<io::XInputStream> SAL_CALL getInputStream() override;
    virtual uno::Reference<io::XOutputStream> SAL_CALL getOutputStream() override;

    // XSeekable
    virtual void SAL_CALL seek(sal_Int64 location) override;
    virtual sal_Int64 SAL_CALL getPosition() override;
    virtual sal_Int64 SAL_CALL getLength() override;
};

}

StreamSupplier::StreamSupplier(uno::Reference<io::XInputStream> xInput, uno::Reference<io::XOutputStream> xOutput)
    : m_xInput(std::move(xInput))
    , m_xOutput(std::move(xOutput))
{
    m_xSeekable.set(m_xInput, uno::UNO_QUERY);
    if (!m_xSeekable.is())
        m_xSeekable.set(m_xOutput, uno::UNO_QUERY);
    OSL_ENSURE(m_xSeekable.is(), "StreamSupplier::StreamSupplier: at least one of both must be seekable!");
}

uno::Reference<io::XInputStream> SAL_CALL StreamSupplier::getInputStream()
{
    return m_xInput;
}

uno::Reference<io::XOutputStream> SAL_CALL StreamSupplier::getOutputStream()
{
    return m_xOutput;
}

void SAL_CALL StreamSupplier::seek(sal_Int64 nLocation)
{
    if (!m_xSeekable.is())
        throw io::NotConnectedException();
    m_xSeekable->seek(nLocation);
}

sal_Int64 SAL_CALL StreamSupplier::getPosition()
{
    if (!m_xSeekable.is())
        throw io::NotConnectedException();
    return m_xSeekable->getPosition();
}

sal_Int64 SAL_CALL StreamSupplier::getLength()
{
    if (!m_xSeekable.is())
        throw io::NotConnectedException();

    return m_xSeekable->getLength();
}

bool isSupportedURL(std::u16string_view rURL)
{
    return o3tl::starts_with(rURL, u"private:resource/")
        || o3tl::starts_with(rURL, u"private:graphicrepository/")
        || o3tl::starts_with(rURL, u"private:standardimage/")
        || o3tl::starts_with(rURL, u"vnd.sun.star.extension://");
}

std::unique_ptr<SvStream> getImageStream(uno::Reference<uno::XComponentContext> const & rxContext, OUString const & rImageResourceURL)
{
    std::unique_ptr<SvMemoryStream> pMemBuffer;

    try
    {
        // get a GraphicProvider
        uno::Reference<graphic::XGraphicProvider> xProvider = css::graphic::GraphicProvider::create(rxContext);

        // let it create a graphic from the given URL
        uno::Sequence<beans::PropertyValue> aMediaProperties{ comphelper::makePropertyValue(
            u"URL"_ustr, rImageResourceURL) };
        uno::Reference<graphic::XGraphic> xGraphic(xProvider->queryGraphic(aMediaProperties));

        OSL_ENSURE(xGraphic.is(), "GraphicAccess::getImageStream: the provider did not give us a graphic object!");
        if (!xGraphic.is())
            return pMemBuffer;

        // copy the graphic to an in-memory buffer
        pMemBuffer.reset(new SvMemoryStream);
        uno::Reference<io::XStream> xBufferAccess = new StreamSupplier(
            new OSeekableInputStreamWrapper(*pMemBuffer),
            new OSeekableOutputStreamWrapper(*pMemBuffer));

        aMediaProperties = { comphelper::makePropertyValue(u"OutputStream"_ustr, xBufferAccess),
                             comphelper::makePropertyValue(u"MimeType"_ustr, u"image/png"_ustr) };
        xProvider->storeGraphic(xGraphic, aMediaProperties);

        pMemBuffer->Seek(0);
    }
    catch (const uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("svtools", "GraphicAccess::getImageStream");
        pMemBuffer.reset();
    }

    return pMemBuffer;
}

uno::Reference<io::XInputStream> getImageXStream(uno::Reference<uno::XComponentContext> const & rxContext, OUString const & rImageResourceURL)
{
    return new OSeekableInputStreamWrapper(getImageStream(rxContext, rImageResourceURL).release(), true);   // take ownership
}

} // namespace svt::GraphicAccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
