/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sfx2/DocumentSigner.hxx>

#include <tools/stream.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>

#include <comphelper/storagehelper.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/security/DocumentDigitalSignatures.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/XStream.hpp>

using namespace css;

namespace sfx2
{
bool DocumentSigner::signDocument(uno::Reference<security::XCertificate> const& rxCertificate)
{
    std::unique_ptr<SvStream> pStream(
        utl::UcbStreamHelper::CreateStream(m_aUrl, StreamMode::READ | StreamMode::WRITE));
    uno::Reference<io::XStream> xInputStream(new utl::OStreamWrapper(std::move(pStream)));

    bool bResult = false;
    uno::Reference<embed::XStorage> xWriteableZipStore;
    try
    {
        xWriteableZipStore = comphelper::OStorageHelper::GetStorageOfFormatFromStream(
            ZIP_STORAGE_FORMAT_STRING, xInputStream);
    }
    catch (const io::IOException&)
    {
    }

    OUString aODFVersion(comphelper::OStorageHelper::GetODFVersionFromStorage(xWriteableZipStore));

    uno::Reference<security::XDocumentDigitalSignatures> xSigner(
        security::DocumentDigitalSignatures::createWithVersionAndValidSignature(
            comphelper::getProcessComponentContext(), aODFVersion,
            /*bHasValidDocumentSignature*/ true));

    try
    {
        uno::Reference<embed::XStorage> xMetaInf;
        if (xWriteableZipStore.is() && xWriteableZipStore->hasByName(u"META-INF"_ustr))
        {
            xMetaInf = xWriteableZipStore->openStorageElement(u"META-INF"_ustr,
                                                              embed::ElementModes::READWRITE);
            if (!xMetaInf.is())
                throw uno::RuntimeException();
        }
        if (xMetaInf.is())
        {
            uno::Reference<embed::XStorage> xStorage
                = comphelper::OStorageHelper::GetStorageOfFormatFromStream(
                    ZIP_STORAGE_FORMAT_STRING, xInputStream);

            // ODF.
            uno::Reference<io::XStream> xStream;
            xStream.set(
                xMetaInf->openStreamElement(xSigner->getDocumentContentSignatureDefaultStreamName(),
                                            embed::ElementModes::READWRITE),
                uno::UNO_SET_THROW);
            bool bSuccess = xSigner->signDocumentWithCertificate(rxCertificate, xStorage, xStream);
            if (bSuccess)
            {
                uno::Reference<embed::XTransactedObject> xTransact(xMetaInf, uno::UNO_QUERY_THROW);
                xTransact->commit();
                xTransact.set(xWriteableZipStore, uno::UNO_QUERY_THROW);
                xTransact->commit();
                bResult = true;
            }
        }
        else if (xWriteableZipStore.is())
        {
            uno::Reference<embed::XStorage> xStorage
                = comphelper::OStorageHelper::GetStorageOfFormatFromStream(
                    ZIP_STORAGE_FORMAT_STRING, xInputStream);

            // OOXML.
            uno::Reference<io::XStream> xStream;

            // We need read-write to be able to add the signature relation.
            bool bSuccess = xSigner->signDocumentWithCertificate(rxCertificate, xStorage, xStream);

            if (bSuccess)
            {
                uno::Reference<embed::XTransactedObject> xTransact(xWriteableZipStore,
                                                                   uno::UNO_QUERY_THROW);
                xTransact->commit();
                bResult = true;
            }
        }
        else
        {
            // Something not ZIP based: e.g. PDF.
            bResult = xSigner->signDocumentWithCertificate(
                rxCertificate, uno::Reference<embed::XStorage>(), xInputStream);
        }
    }
    catch (const uno::Exception&)
    {
    }
    return bResult;
}

} // namespace sfx2

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
