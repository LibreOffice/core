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

#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/sfxsids.hrc>
#include <filter/msfilter/msoleexp.hxx>
#include <svx/svxerr.hxx>
#include <unotools/streamwrap.hxx>
#include <sot/storage.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/processfactory.hxx>
#include <officecfg/Office/Impress.hxx>
#include <officecfg/Office/Common.hxx>

#include <com/sun/star/packages/XPackageEncryption.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <sdpptwrp.hxx>
#include <DrawDocShell.hxx>
#include <sfx2/frame.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::frame;

SdPPTFilter::SdPPTFilter( SfxMedium& rMedium, ::sd::DrawDocShell& rDocShell ) :
    SdFilter( rMedium, rDocShell ),
    pBas    ( nullptr )
{
}

SdPPTFilter::~SdPPTFilter()
{
    delete pBas;    // deleting the compressed basic storage
}

static void lcl_getListOfStreams(SotStorage * pStorage, comphelper::SequenceAsHashMap& aStreamsData, std::u16string_view sPrefix)
{
    SvStorageInfoList aElements;
    pStorage->FillInfoList(&aElements);
    for (const auto & aElement : aElements)
    {
        OUString sStreamFullName = sPrefix.size() ? OUString::Concat(sPrefix) + "/" + aElement.GetName() : aElement.GetName();
        if (aElement.IsStorage())
        {
            rtl::Reference<SotStorage> xSubStorage = pStorage->OpenSotStorage(aElement.GetName(), StreamMode::STD_READ | StreamMode::SHARE_DENYALL);
            lcl_getListOfStreams(xSubStorage.get(), aStreamsData, sStreamFullName);
        }
        else
        {
            // Read stream
            rtl::Reference<SotStorageStream> rStream = pStorage->OpenSotStream(aElement.GetName(), StreamMode::READ | StreamMode::SHARE_DENYALL);
            if (rStream.is())
            {
                sal_Int32 nStreamSize = rStream->GetSize();
                Sequence< sal_Int8 > oData;
                oData.realloc(nStreamSize);
                sal_Int32 nReadBytes = rStream->ReadBytes(oData.getArray(), nStreamSize);
                if (nStreamSize == nReadBytes)
                    aStreamsData[sStreamFullName] <<= oData;
            }
        }
    }
}

static rtl::Reference<SotStorage> lcl_DRMDecrypt(const SfxMedium& rMedium, const rtl::Reference<SotStorage>& rStorage, std::shared_ptr<SvStream>& rNewStorageStrm)
{
    rtl::Reference<SotStorage> aNewStorage;

    // We have DRM encrypted storage. We should try to decrypt it first, if we can
    Sequence< Any > aArguments;
    Reference<XComponentContext> xComponentContext(comphelper::getProcessComponentContext());
    Reference< css::packages::XPackageEncryption > xPackageEncryption(
        xComponentContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            u"com.sun.star.comp.oox.crypto.DRMDataSpace"_ustr, aArguments, xComponentContext), UNO_QUERY);

    if (!xPackageEncryption.is())
    {
        // We do not know how to decrypt this
        return aNewStorage;
    }

    comphelper::SequenceAsHashMap aStreamsData;
    lcl_getListOfStreams(rStorage.get(), aStreamsData, u"");

    try {
        Sequence<NamedValue> aStreams = aStreamsData.getAsConstNamedValueList();
        if (!xPackageEncryption->readEncryptionInfo(aStreams))
        {
            // We failed with decryption
            return aNewStorage;
        }

        rtl::Reference<SotStorageStream> rContentStream = rStorage->OpenSotStream(u"\011DRMContent"_ustr, StreamMode::READ | StreamMode::SHARE_DENYALL);
        if (!rContentStream.is())
        {
            return aNewStorage;
        }

        rNewStorageStrm = std::make_shared<SvMemoryStream>();

        Reference<css::io::XInputStream > xInputStream(new utl::OSeekableInputStreamWrapper(rContentStream.get(), false));
        Reference<css::io::XOutputStream > xDecryptedStream(new utl::OSeekableOutputStreamWrapper(*rNewStorageStrm));

        if (!xPackageEncryption->decrypt(xInputStream, xDecryptedStream))
        {
            // We failed with decryption
            return aNewStorage;
        }

        rNewStorageStrm->Seek(0);

        // Further reading is done from new document
        aNewStorage = new SotStorage(*rNewStorageStrm);

        // Set the media descriptor data
        Sequence<NamedValue> aEncryptionData = xPackageEncryption->createEncryptionData(u""_ustr);
        rMedium.GetItemSet().Put(SfxUnoAnyItem(SID_ENCRYPTIONDATA, Any(aEncryptionData)));
    }
    catch (const std::exception&)
    {
        return aNewStorage;
    }

    return aNewStorage;
}

bool SdPPTFilter::Import()
{
    bool bRet = false;
    std::shared_ptr<SvStream> aDecryptedStorageStrm;
    rtl::Reference<SotStorage> pStorage = new SotStorage(mrMedium.GetInStream(), false);
    if( !pStorage->GetError() )
    {
        /* check if there is a dualstorage, then the
        document is probably a PPT95 containing PPT97 */
        rtl::Reference<SotStorage> xDualStorage;
        OUString sDualStorage( u"PP97_DUALSTORAGE"_ustr  );
        if ( pStorage->IsContained( sDualStorage ) )
        {
            xDualStorage = pStorage->OpenSotStorage( sDualStorage, StreamMode::STD_READ );
            pStorage = xDualStorage;
        }
        if (pStorage->IsContained(u"\011DRMContent"_ustr))
        {
            // Document is DRM encrypted
            pStorage = lcl_DRMDecrypt(mrMedium, pStorage, aDecryptedStorageStrm);
        }
        rtl::Reference<SotStorageStream> pDocStream(pStorage->OpenSotStream( u"PowerPoint Document"_ustr , StreamMode::STD_READ ));
        if( pDocStream )
        {
            pDocStream->SetVersion( pStorage->GetVersion() );
            pDocStream->SetCryptMaskKey(pStorage->GetKey());

            if ( pStorage->IsStream( u"EncryptedSummary"_ustr ) )
                mrMedium.SetError(ERRCODE_SVX_READ_FILTER_PPOINT);
            else
            {
                bRet = ImportPPT( &mrDocument, *pDocStream, *pStorage, mrMedium );

                if ( !bRet )
                    mrMedium.SetError(SVSTREAM_WRONGVERSION);
            }
        }
    }

    return bRet;
}

bool SdPPTFilter::Export()
{
    bool bRet = false;

    if( mxModel.is() )
    {
        sal_uInt32          nCnvrtFlags = 0;
        if ( officecfg::Office::Common::Filter::Microsoft::Export::MathToMathType::get() )
            nCnvrtFlags |= OLE_STARMATH_2_MATHTYPE;
        if ( officecfg::Office::Common::Filter::Microsoft::Export::WriterToWinWord::get() )
            nCnvrtFlags |= OLE_STARWRITER_2_WINWORD;
        if ( officecfg::Office::Common::Filter::Microsoft::Export::CalcToExcel::get() )
            nCnvrtFlags |= OLE_STARCALC_2_EXCEL;
        if ( officecfg::Office::Common::Filter::Microsoft::Export::ImpressToPowerPoint::get() )
            nCnvrtFlags |= OLE_STARIMPRESS_2_POWERPOINT;
        if ( officecfg::Office::Common::Filter::Microsoft::Export::EnablePowerPointPreview::get() )
            nCnvrtFlags |= 0x8000;

        CreateStatusIndicator();

        //OUString sBaseURI( "BaseURI");
        std::vector< PropertyValue > aProperties;
        PropertyValue aProperty;
        aProperty.Name = "BaseURI";
        aProperty.Value <<= mrMedium.GetBaseURL( true );
        aProperties.push_back( aProperty );

        SvStream * pOutputStrm = mrMedium.GetOutStream();

        Sequence< NamedValue > aEncryptionData;
        Reference< css::packages::XPackageEncryption > xPackageEncryption;
        const SfxUnoAnyItem* pEncryptionDataItem = mrMedium.GetItemSet().GetItem(SID_ENCRYPTIONDATA, false);
        std::shared_ptr<SvStream> pMediaStrm;
        if (pEncryptionDataItem && (pEncryptionDataItem->GetValue() >>= aEncryptionData))
        {
            ::comphelper::SequenceAsHashMap aHashData(aEncryptionData);
            OUString sCryptoType = aHashData.getUnpackedValueOrDefault(u"CryptoType"_ustr, OUString());

            if (sCryptoType.getLength())
            {
                Reference<XComponentContext> xComponentContext(comphelper::getProcessComponentContext());
                Sequence<Any> aArguments{
                    Any(NamedValue(u"Binary"_ustr, Any(true))) };
                xPackageEncryption.set(
                    xComponentContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                        "com.sun.star.comp.oox.crypto." + sCryptoType, aArguments, xComponentContext), UNO_QUERY);

                if (xPackageEncryption.is())
                {
                    // We have an encryptor. Export document into memory stream and encrypt it later
                    pMediaStrm = std::make_shared<SvMemoryStream>();
                    pOutputStrm = pMediaStrm.get();

                    // Temp removal of EncryptionData to avoid password protection triggering
                    mrMedium.GetItemSet().ClearItem(SID_ENCRYPTIONDATA);
                }
            }
        }

        rtl::Reference<SotStorage> xStorRef = new SotStorage(pOutputStrm, false);

        if (xStorRef.is())
        {
            bRet = ExportPPT(aProperties, xStorRef, mxModel, mxStatusIndicator, pBas, nCnvrtFlags);
            xStorRef->Commit();

            if (xPackageEncryption.is())
            {
                // Perform DRM encryption
                pOutputStrm->Seek(0);

                xPackageEncryption->setupEncryption(aEncryptionData);

                Reference<css::io::XInputStream > xInputStream(new utl::OSeekableInputStreamWrapper(pOutputStrm, false));
                Sequence<NamedValue> aStreams = xPackageEncryption->encrypt(xInputStream);

                rtl::Reference<SotStorage> xEncryptedRootStrg = new SotStorage(mrMedium.GetOutStream(), false);
                for (const NamedValue& aStreamData : aStreams)
                {
                    // To avoid long paths split and open substorages recursively
                    // Splitting paths manually, since comphelper::string::split is trimming special characters like \0x01, \0x09
                    rtl::Reference<SotStorage> pStorage = xEncryptedRootStrg;
                    OUString sFileName;
                    sal_Int32 idx = 0;
                    do
                    {
                        OUString sPathElem = aStreamData.Name.getToken(0, L'/', idx);
                        if (!sPathElem.isEmpty())
                        {
                            if (idx < 0)
                            {
                                sFileName = sPathElem;
                            }
                            else
                            {
                                pStorage = pStorage->OpenSotStorage(sPathElem);
                            }
                        }
                    } while (pStorage && idx >= 0);

                    if (!pStorage)
                    {
                        bRet = false;
                        break;
                    }

                    rtl::Reference<SotStorageStream> pStream = pStorage->OpenSotStream(sFileName);
                    if (!pStream)
                    {
                        bRet = false;
                        break;
                    }
                    Sequence<sal_Int8> aStreamContent;
                    aStreamData.Value >>= aStreamContent;
                    size_t nBytesWritten = pStream->WriteBytes(aStreamContent.getConstArray(), aStreamContent.getLength());
                    if (nBytesWritten != static_cast<size_t>(aStreamContent.getLength()))
                    {
                        bRet = false;
                        break;
                    }
                }
                xEncryptedRootStrg->Commit();

                // Restore encryption data
                mrMedium.GetItemSet().Put(SfxUnoAnyItem(SID_ENCRYPTIONDATA, Any(aEncryptionData)));
            }
        }
    }

    return bRet;
}

void SdPPTFilter::PreSaveBasic()
{
    if( officecfg::Office::Impress::Filter::Import::VBA::Save::get() )
    {
        SaveVBA( static_cast<SfxObjectShell&>(mrDocShell), pBas );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
