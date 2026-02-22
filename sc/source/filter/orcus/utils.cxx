/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus_utils.hxx>
#include <osl/thread.hxx>
#include <tools/stream.hxx>

orcus::file_content toFileContent(const OUString& rPath)
{
#ifdef _WIN32
    return orcus::file_content(rPath);
#else
    return orcus::file_content(OUStringToOString(rPath, osl_getThreadTextEncoding()));
#endif
}

CopiedTempStream::CopiedTempStream(SvStream& rSrc)
{
    maTemp.EnableKillingFile();
    SvStream* pDest = maTemp.GetStream(StreamMode::WRITE);

    rSrc.Seek(0);

    const std::size_t nReadBuffer = 1024 * 32;
    std::size_t nRead = 0;

    do
    {
        char pData[nReadBuffer];
        nRead = rSrc.ReadBytes(pData, nReadBuffer);
        pDest->WriteBytes(pData, nRead);
    } while (nRead == nReadBuffer);

    maTemp.CloseStream();
}

CopiedTempStream::CopiedTempStream(const css::uno::Reference<css::io::XInputStream>& xSrc)
{
    maTemp.EnableKillingFile();
    SvStream* pDest = maTemp.GetStream(StreamMode::WRITE);

    const sal_Int32 nBytes = 1024 * 32;
    css::uno::Sequence<sal_Int8> aSeq(nBytes);

    for (bool bEnd = false; !bEnd;)
    {
        sal_Int32 nReadBytes = xSrc->readBytes(aSeq, nBytes);
        bEnd = (nReadBytes != nBytes);
        pDest->WriteBytes(aSeq.getConstArray(), nReadBytes);
    }

    maTemp.CloseStream();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
