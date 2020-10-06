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
#include <unotools/fltrcfg.hxx>
#include <unotools/streamwrap.hxx>
#include <sot/storage.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/packages/XPackageEncryption.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <sdpptwrp.hxx>
#include <DrawDocShell.hxx>
#include <sfx2/frame.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::frame;

typedef sal_Bool ( *ExportPPTPointer )( const std::vector< css::beans::PropertyValue >&, tools::SvRef<SotStorage> const&,
                                             Reference< XModel > const &,
                                             Reference< XStatusIndicator > const &,
                                             SvMemoryStream*, sal_uInt32 nCnvrtFlags );

typedef sal_Bool ( *ImportPPTPointer )( SdDrawDocument*, SvStream&, SotStorage&, SfxMedium& );

typedef sal_Bool ( *SaveVBAPointer )( SfxObjectShell&, SvMemoryStream*& );

#ifdef DISABLE_DYNLOADING

extern "C" sal_Bool ExportPPT( const std::vector< css::beans::PropertyValue >&, tools::SvRef<SotStorage> const&,
                               Reference< XModel > const &,
                               Reference< XStatusIndicator > const &,
                               SvMemoryStream*, sal_uInt32 nCnvrtFlags );

extern "C" sal_Bool ImportPPT( SdDrawDocument*, SvStream&, SotStorage&, SfxMedium& );

extern "C" sal_Bool SaveVBA( SfxObjectShell&, SvMemoryStream*& );

#endif


SdPPTFilter::SdPPTFilter( SfxMedium& rMedium, ::sd::DrawDocShell& rDocShell ) :
    SdFilter( rMedium, rDocShell ),
    pBas    ( nullptr )
{
}

SdPPTFilter::~SdPPTFilter()
{
    delete pBas;    // deleting the compressed basic storage
}

static void lcl_getListOfStreams(SotStorage * pStorage, comphelper::SequenceAsHashMap& aStreamsData, const OUString& sPrefix)
{
    SvStorageInfoList aElements;
    pStorage->FillInfoList(&aElements);
    for (const auto & aElement : aElements)
    {
        OUString sStreamFullName = sPrefix.getLength() ? sPrefix + "/" + aElement.GetName() : aElement.GetName();
        if (aElement.IsStorage())
        {
            tools::SvRef<SotStorage> xSubStorage = pStorage->OpenSotStorage(aElement.GetName(), StreamMode::STD_READ | StreamMode::SHARE_DENYALL);
            lcl_getListOfStreams(xSubStorage.get(), aStreamsData, sStreamFullName);
        }
        else
        {
            // Read stream
            tools::SvRef<SotStorageStream> rStream = pStorage->OpenSotStream(aElement.GetName(), StreamMode::READ | StreamMode::SHARE_DENYALL);
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

static tools::SvRef<SotStorage> lcl_DRMDecrypt(const SfxMedium& rMedium, const tools::SvRef<SotStorage>& rStorage, std::shared_ptr<SvStream>& rNewStorageStrm)
{
    tools::SvRef<SotStorage> aNewStorage;

    // We have DRM encrypted storage. We should try to decrypt it first, if we can
    Sequence< Any > aArguments;
    Reference<XComponentContext> xComponentContext(comphelper::getProcessComponentContext());
    Reference< css::packages::XPackageEncryption > xPackageEncryption(
        xComponentContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            "com.sun.star.comp.oox.crypto.DRMDataSpace", aArguments, xComponentContext), UNO_QUERY);

    if (!xPackageEncryption.is())
    {
        // We do not know how to decrypt this
        return aNewStorage;
    }

    comphelper::SequenceAsHashMap aStreamsData;
    lcl_getListOfStreams(rStorage.get(), aStreamsData, "");

    try {
        Sequence<NamedValue> aStreams = aStreamsData.getAsConstNamedValueList();
        if (!xPackageEncryption->readEncryptionInfo(aStreams))
        {
            // We failed with decryption
            return aNewStorage;
        }

        tools::SvRef<SotStorageStream> rContentStream = rStorage->OpenSotStream("\011DRMContent", StreamMode::READ | StreamMode::SHARE_DENYALL);
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
        Sequence<NamedValue> aEncryptionData = xPackageEncryption->createEncryptionData("");
        rMedium.GetItemSet()->Put(SfxUnoAnyItem(SID_ENCRYPTIONDATA, makeAny(aEncryptionData)));
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
    tools::SvRef<SotStorage> pStorage = new SotStorage( mrMedium.GetInStream(), false );
    if( !pStorage->GetError() )
    {
        /* check if there is a dualstorage, then the
        document is probably a PPT95 containing PPT97 */
        tools::SvRef<SotStorage> xDualStorage;
        OUString sDualStorage( "PP97_DUALSTORAGE"  );
        if ( pStorage->IsContained( sDualStorage ) )
        {
            xDualStorage = pStorage->OpenSotStorage( sDualStorage, StreamMode::STD_READ );
            pStorage = xDualStorage;
        }
        if (pStorage->IsContained("\011DRMContent"))
        {
            // Document is DRM encrypted
            pStorage = lcl_DRMDecrypt(mrMedium, pStorage, aDecryptedStorageStrm);
        }
        std::unique_ptr<SvStream> pDocStream(pStorage->OpenSotStream( "PowerPoint Document" , StreamMode::STD_READ ));
        if( pDocStream )
        {
            pDocStream->SetVersion( pStorage->GetVersion() );
            pDocStream->SetCryptMaskKey(pStorage->GetKey());

            if ( pStorage->IsStream( "EncryptedSummary" ) )
                mrMedium.SetError(ERRCODE_SVX_READ_FILTER_PPOINT);
            else
            {
#ifdef DISABLE_DYNLOADING
                ImportPPTPointer pPPTImport = ImportPPT;
#else
                ImportPPTPointer pPPTImport = reinterpret_cast< ImportPPTPointer >(
                    SdFilter::GetLibrarySymbol(mrMedium.GetFilter()->GetUserData(), "ImportPPT"));
#endif

                if ( pPPTImport )
                    bRet = pPPTImport( &mrDocument, *pDocStream, *pStorage, mrMedium );

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
#ifdef DISABLE_DYNLOADING
        ExportPPTPointer PPTExport = ExportPPT;
#else
        ExportPPTPointer PPTExport = reinterpret_cast< ExportPPTPointer >(
            SdFilter::GetLibrarySymbol(mrMedium.GetFilter()->GetUserData(), "ExportPPT"));
#endif

        if( PPTExport)
        {
            sal_uInt32          nCnvrtFlags = 0;
            const SvtFilterOptions& rFilterOptions = SvtFilterOptions::Get();
            if ( rFilterOptions.IsMath2MathType() )
                nCnvrtFlags |= OLE_STARMATH_2_MATHTYPE;
            if ( rFilterOptions.IsWriter2WinWord() )
                nCnvrtFlags |= OLE_STARWRITER_2_WINWORD;
            if ( rFilterOptions.IsCalc2Excel() )
                nCnvrtFlags |= OLE_STARCALC_2_EXCEL;
            if ( rFilterOptions.IsImpress2PowerPoint() )
                nCnvrtFlags |= OLE_STARIMPRESS_2_POWERPOINT;
            if ( rFilterOptions.IsEnablePPTPreview() )
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
            const SfxUnoAnyItem* pEncryptionDataItem = SfxItemSet::GetItem<SfxUnoAnyItem>(mrMedium.GetItemSet(), SID_ENCRYPTIONDATA, false);
            std::shared_ptr<SvStream> pMediaStrm;
            if (pEncryptionDataItem && (pEncryptionDataItem->GetValue() >>= aEncryptionData))
            {
                ::comphelper::SequenceAsHashMap aHashData(aEncryptionData);
                OUString sCryptoType = aHashData.getUnpackedValueOrDefault("CryptoType", OUString());

                if (sCryptoType.getLength())
                {
                    Reference<XComponentContext> xComponentContext(comphelper::getProcessComponentContext());
                    Sequence<Any> aArguments{
                        makeAny(NamedValue("Binary", makeAny(true))) };
                    xPackageEncryption.set(
                        xComponentContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                            "com.sun.star.comp.oox.crypto." + sCryptoType, aArguments, xComponentContext), UNO_QUERY);

                    if (xPackageEncryption.is())
                    {
                        // We have an encryptor. Export document into memory stream and encrypt it later
                        pMediaStrm = std::make_shared<SvMemoryStream>();
                        pOutputStrm = pMediaStrm.get();

                        // Temp removal of EncryptionData to avoid password protection triggering
                        mrMedium.GetItemSet()->ClearItem(SID_ENCRYPTIONDATA);
                    }
                }
            }

            tools::SvRef<SotStorage> xStorRef = new SotStorage(pOutputStrm, false);

            if (xStorRef.is())
            {
                bRet = PPTExport(aProperties, xStorRef, mxModel, mxStatusIndicator, pBas, nCnvrtFlags);
                xStorRef->Commit();

                if (xPackageEncryption.is())
                {
                    // Perform DRM encryption
                    pOutputStrm->Seek(0);

                    xPackageEncryption->setupEncryption(aEncryptionData);

                    Reference<css::io::XInputStream > xInputStream(new utl::OSeekableInputStreamWrapper(pOutputStrm, false));
                    Sequence<NamedValue> aStreams = xPackageEncryption->encrypt(xInputStream);

                    tools::SvRef<SotStorage> xEncryptedRootStrg = new SotStorage(mrMedium.GetOutStream(), false);
                    for (const NamedValue & aStreamData : std::as_const(aStreams))
                    {
                        // To avoid long paths split and open substorages recursively
                        // Splitting paths manually, since comphelper::string::split is trimming special characters like \0x01, \0x09
                        tools::SvRef<SotStorage> pStorage = xEncryptedRootStrg.get();
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

                        tools::SvRef<SotStorageStream> pStream = pStorage->OpenSotStream(sFileName);
                        if (!pStream)
                        {
                            bRet = false;
                            break;
                        }
                        Sequence<sal_Int8> aStreamContent;
                        aStreamData.Value >>= aStreamContent;
                        size_t nBytesWritten = pStream->WriteBytes(aStreamContent.getArray(), aStreamContent.getLength());
                        if (nBytesWritten != static_cast<size_t>(aStreamContent.getLength()))
                        {
                            bRet = false;
                            break;
                        }
                    }
                    xEncryptedRootStrg->Commit();

                    // Restore encryption data
                    mrMedium.GetItemSet()->Put(SfxUnoAnyItem(SID_ENCRYPTIONDATA, makeAny(aEncryptionData)));
                }
            }
        }
    }

    return bRet;
}

void SdPPTFilter::PreSaveBasic()
{
    const SvtFilterOptions& rFilterOptions = SvtFilterOptions::Get();
    if( rFilterOptions.IsLoadPPointBasicStorage() )
    {
#ifdef DISABLE_DYNLOADING
        SaveVBAPointer pSaveVBA= SaveVBA;
#else
        SaveVBAPointer pSaveVBA = reinterpret_cast< SaveVBAPointer >(
            SdFilter::GetLibrarySymbol(mrMedium.GetFilter()->GetUserData(), "SaveVBA"));
#endif
        if( pSaveVBA )
            pSaveVBA( static_cast<SfxObjectShell&>(mrDocShell), pBas );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
