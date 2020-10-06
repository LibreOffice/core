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
#include <sfx2/frame.hxx>
#include <sfx2/sfxsids.hrc>
#include <sot/storage.hxx>
#include <sot/exchange.hxx>
#include <filter/msfilter/classids.hxx>
#include <tools/globname.hxx>
#include <com/sun/star/packages/XPackageEncryption.hpp>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <unotools/streamwrap.hxx>
#include <unotools/defaultencoding.hxx>
#include <unotools/wincodepage.hxx>
#include <osl/diagnose.h>
#include <filter.hxx>
#include <document.hxx>
#include <xistream.hxx>
#include <xltools.hxx>
#include <docoptio.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/processfactory.hxx>

#include <docsh.hxx>
#include <scerrors.hxx>
#include <imp_op.hxx>
#include <excimp8.hxx>
#include <exp_op.hxx>
#include <scdll.hxx>

#include <memory>

using namespace css;

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
                uno::Sequence< sal_Int8 > oData;
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
    uno::Sequence< uno::Any > aArguments;
    uno::Reference<uno::XComponentContext> xComponentContext(comphelper::getProcessComponentContext());
    uno::Reference< packages::XPackageEncryption > xPackageEncryption(
        xComponentContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            "com.sun.star.comp.oox.crypto.DRMDataSpace", aArguments, xComponentContext), uno::UNO_QUERY);

    if (!xPackageEncryption.is())
    {
        // We do not know how to decrypt this
        return aNewStorage;
    }

    comphelper::SequenceAsHashMap aStreamsData;
    lcl_getListOfStreams(rStorage.get(), aStreamsData, "");

    try {
        uno::Sequence<beans::NamedValue> aStreams = aStreamsData.getAsConstNamedValueList();
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

        uno::Reference<io::XInputStream > xInputStream(new utl::OSeekableInputStreamWrapper(rContentStream.get(), false));
        uno::Reference<io::XOutputStream > xDecryptedStream(new utl::OSeekableOutputStreamWrapper(*rNewStorageStrm));

        if (!xPackageEncryption->decrypt(xInputStream, xDecryptedStream))
        {
            // We failed with decryption
            return aNewStorage;
        }

        rNewStorageStrm->Seek(0);

        // Further reading is done from new document
        aNewStorage = new SotStorage(*rNewStorageStrm);

        // Set the media descriptor data
        uno::Sequence<beans::NamedValue> aEncryptionData = xPackageEncryption->createEncryptionData("");
        rMedium.GetItemSet()->Put(SfxUnoAnyItem(SID_ENCRYPTIONDATA, uno::makeAny(aEncryptionData)));
    }
    catch (const std::exception&)
    {
        return aNewStorage;
    }

    return aNewStorage;
}

ErrCode ScFormatFilterPluginImpl::ScImportExcel( SfxMedium& rMedium, ScDocument* pDocument, const EXCIMPFORMAT eFormat )
{
    // check the passed Calc document
    OSL_ENSURE( pDocument, "::ScImportExcel - no document" );
    if( !pDocument ) return SCERR_IMPORT_INTERNAL;        // should not happen

    /*  Import all BIFF versions regardless on eFormat, needed for import of
        external cells (file type detection returns Excel4.0). */
    if( (eFormat != EIF_AUTO) && (eFormat != EIF_BIFF_LE4) && (eFormat != EIF_BIFF5) && (eFormat != EIF_BIFF8) )
    {
        OSL_FAIL( "::ScImportExcel - wrong file format specification" );
        return SCERR_IMPORT_FORMAT;
    }

    // check the input stream from medium
    SvStream* pMedStrm = rMedium.GetInStream();
    OSL_ENSURE( pMedStrm, "::ScImportExcel - medium without input stream" );
    if( !pMedStrm ) return SCERR_IMPORT_OPEN;           // should not happen

    SvStream* pBookStrm = nullptr;            // The "Book"/"Workbook" stream containing main data.
    XclBiff eBiff = EXC_BIFF_UNKNOWN;   // The BIFF version of the main stream.

    // try to open an OLE storage
    tools::SvRef<SotStorage> xRootStrg;
    tools::SvRef<SotStorageStream> xStrgStrm;
    std::shared_ptr<SvStream> aNewStorageStrm;
    if( SotStorage::IsStorageFile( pMedStrm ) )
    {
        xRootStrg = new SotStorage( pMedStrm, false );
        if( xRootStrg->GetError() )
            xRootStrg = nullptr;
    }

    // try to open "Book" or "Workbook" stream in OLE storage
    if( xRootStrg.is() )
    {
        // Check if there is DRM encryption in storage
        tools::SvRef<SotStorageStream> xDRMStrm = ScfTools::OpenStorageStreamRead(xRootStrg, "\011DRMContent");
        if (xDRMStrm.is())
        {
            xRootStrg = lcl_DRMDecrypt(rMedium, xRootStrg, aNewStorageStrm);
        }

        // try to open the "Book" stream
        tools::SvRef<SotStorageStream> xBookStrm = ScfTools::OpenStorageStreamRead( xRootStrg, EXC_STREAM_BOOK );
        XclBiff eBookBiff = xBookStrm.is() ?  XclImpStream::DetectBiffVersion( *xBookStrm ) : EXC_BIFF_UNKNOWN;

        // try to open the "Workbook" stream
        tools::SvRef<SotStorageStream> xWorkbookStrm = ScfTools::OpenStorageStreamRead( xRootStrg, EXC_STREAM_WORKBOOK );
        XclBiff eWorkbookBiff = xWorkbookStrm.is() ?  XclImpStream::DetectBiffVersion( *xWorkbookStrm ) : EXC_BIFF_UNKNOWN;

        // decide which stream to use
        if( (eWorkbookBiff != EXC_BIFF_UNKNOWN) && ((eBookBiff == EXC_BIFF_UNKNOWN) || (eWorkbookBiff > eBookBiff)) )
        {
            /*  Only "Workbook" stream exists; or both streams exist,
                and "Workbook" has higher BIFF version than "Book" stream. */
            xStrgStrm = xWorkbookStrm;
            eBiff = eWorkbookBiff;
        }
        else if( eBookBiff != EXC_BIFF_UNKNOWN )
        {
            /*  Only "Book" stream exists; or both streams exist,
                and "Book" has higher BIFF version than "Workbook" stream. */
            xStrgStrm = xBookStrm;
            eBiff = eBookBiff;
        }

        pBookStrm = xStrgStrm.get();
    }

    // no "Book" or "Workbook" stream found, try plain input stream from medium (even for BIFF5+)
    if( !pBookStrm )
    {
        eBiff = XclImpStream::DetectBiffVersion( *pMedStrm );
        if( eBiff != EXC_BIFF_UNKNOWN )
            pBookStrm = pMedStrm;
    }

    // try to import the file
    ErrCode eRet = SCERR_IMPORT_UNKNOWN_BIFF;
    if( pBookStrm )
    {
        pBookStrm->SetBufferSize( 0x8000 );     // still needed?

        XclImpRootData aImpData(
            eBiff, rMedium, xRootStrg, *pDocument,
            utl_getWinTextEncodingFromLangStr(utl_getLocaleForGlobalDefaultEncoding()));
        std::unique_ptr< ImportExcel > xFilter;
        switch( eBiff )
        {
            case EXC_BIFF2:
            case EXC_BIFF3:
            case EXC_BIFF4:
            case EXC_BIFF5:
                xFilter.reset( new ImportExcel( aImpData, *pBookStrm ) );
            break;
            case EXC_BIFF8:
                xFilter.reset( new ImportExcel8( aImpData, *pBookStrm ) );
            break;
            default:    DBG_ERROR_BIFF();
        }

        eRet = xFilter ? xFilter->Read() : SCERR_IMPORT_INTERNAL;
    }

    return eRet;
}

static ErrCode lcl_ExportExcelBiff( SfxMedium& rMedium, ScDocument *pDocument,
        SvStream* pMedStrm, bool bBiff8, rtl_TextEncoding eNach )
{
    uno::Reference< packages::XPackageEncryption > xPackageEncryption;
    uno::Sequence< beans::NamedValue > aEncryptionData;
    const SfxUnoAnyItem* pEncryptionDataItem = SfxItemSet::GetItem<SfxUnoAnyItem>(rMedium.GetItemSet(), SID_ENCRYPTIONDATA, false);
    SvStream* pOriginalMediaStrm = pMedStrm;
    std::shared_ptr<SvStream> pMediaStrm;
    if (pEncryptionDataItem && (pEncryptionDataItem->GetValue() >>= aEncryptionData))
    {
        ::comphelper::SequenceAsHashMap aHashData(aEncryptionData);
        OUString sCryptoType = aHashData.getUnpackedValueOrDefault("CryptoType", OUString());

        if (sCryptoType.getLength())
        {
            uno::Reference<uno::XComponentContext> xComponentContext(comphelper::getProcessComponentContext());
            uno::Sequence<uno::Any> aArguments{
                uno::makeAny(beans::NamedValue("Binary", uno::makeAny(true))) };
            xPackageEncryption.set(
                xComponentContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                    "com.sun.star.comp.oox.crypto." + sCryptoType, aArguments, xComponentContext), uno::UNO_QUERY);

            if (xPackageEncryption.is())
            {
                // We have an encryptor. Export document into memory stream and encrypt it later
                pMediaStrm = std::make_shared<SvMemoryStream>();
                pMedStrm = pMediaStrm.get();

                // Temp removal of EncryptionData to avoid password protection triggering
                rMedium.GetItemSet()->ClearItem(SID_ENCRYPTIONDATA);
            }
        }
    }

    // try to open an OLE storage
    tools::SvRef<SotStorage> xRootStrg = new SotStorage( pMedStrm, false );
    if( xRootStrg->GetError() ) return SCERR_IMPORT_OPEN;

    // create BIFF dependent strings
    OUString aStrmName, aClipName, aClassName;
    if( bBiff8 )
    {
        aStrmName = EXC_STREAM_WORKBOOK;
        aClipName = "Biff8";
        aClassName = "Microsoft Excel 97-Tabelle";
    }
    else
    {
        aStrmName = EXC_STREAM_BOOK;
        aClipName = "Biff5";
        aClassName = "Microsoft Excel 5.0-Tabelle";
    }

    // open the "Book"/"Workbook" stream
    tools::SvRef<SotStorageStream> xStrgStrm = ScfTools::OpenStorageStreamWrite( xRootStrg, aStrmName );
    if( !xStrgStrm.is() || xStrgStrm->GetError() ) return SCERR_IMPORT_OPEN;

    xStrgStrm->SetBufferSize( 0x8000 );     // still needed?

    ErrCode eRet = SCERR_IMPORT_UNKNOWN_BIFF;
    XclExpRootData aExpData( bBiff8 ? EXC_BIFF8 : EXC_BIFF5, rMedium, xRootStrg, *pDocument, eNach );
    if ( bBiff8 )
    {
        ExportBiff8 aFilter( aExpData, *xStrgStrm );
        eRet = aFilter.Write();
    }
    else
    {
        ExportBiff5 aFilter( aExpData, *xStrgStrm );
        eRet = aFilter.Write();
    }

    if( eRet == SCWARN_IMPORT_RANGE_OVERFLOW )
        eRet = SCWARN_EXPORT_MAXROW;

    SvGlobalName aGlobName(MSO_EXCEL5_CLASSID);
    SotClipboardFormatId nClip = SotExchange::RegisterFormatName( aClipName );
    xRootStrg->SetClass( aGlobName, nClip, aClassName );

    xStrgStrm->Commit();
    xRootStrg->Commit();

    if (xPackageEncryption.is())
    {
        // Perform DRM encryption
        pMedStrm->Seek(0);

        xPackageEncryption->setupEncryption(aEncryptionData);

        uno::Reference<io::XInputStream > xInputStream(new utl::OSeekableInputStreamWrapper(pMedStrm, false));
        uno::Sequence<beans::NamedValue> aStreams = xPackageEncryption->encrypt(xInputStream);

        tools::SvRef<SotStorage> xEncryptedRootStrg = new SotStorage(pOriginalMediaStrm, false);
        for (const beans::NamedValue & aStreamData : std::as_const(aStreams))
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
                eRet = ERRCODE_IO_GENERAL;
                break;
            }

            tools::SvRef<SotStorageStream> pStream = pStorage->OpenSotStream(sFileName);
            if (!pStream)
            {
                eRet = ERRCODE_IO_GENERAL;
                break;
            }
            uno::Sequence<sal_Int8> aStreamContent;
            aStreamData.Value >>= aStreamContent;
            size_t nBytesWritten = pStream->WriteBytes(aStreamContent.getArray(), aStreamContent.getLength());
            if (nBytesWritten != static_cast<size_t>(aStreamContent.getLength()))
            {
                eRet = ERRCODE_IO_CANTWRITE;
                break;
            }
        }
        xEncryptedRootStrg->Commit();

        // Restore encryption data
        rMedium.GetItemSet()->Put(SfxUnoAnyItem(SID_ENCRYPTIONDATA, uno::makeAny(aEncryptionData)));
    }

    return eRet;
}

ErrCode ScFormatFilterPluginImpl::ScExportExcel5( SfxMedium& rMedium, ScDocument *pDocument,
    ExportFormatExcel eFormat, rtl_TextEncoding eNach )
{
    if( eFormat != ExpBiff5 && eFormat != ExpBiff8 )
        return SCERR_IMPORT_NI;

    // check the passed Calc document
    OSL_ENSURE( pDocument, "::ScExportExcel5 - no document" );
    if( !pDocument ) return SCERR_IMPORT_INTERNAL;        // should not happen

    // check the output stream from medium
    SvStream* pMedStrm = rMedium.GetOutStream();
    OSL_ENSURE( pMedStrm, "::ScExportExcel5 - medium without output stream" );
    if( !pMedStrm ) return SCERR_IMPORT_OPEN;           // should not happen

    ErrCode eRet = lcl_ExportExcelBiff(rMedium, pDocument, pMedStrm, eFormat == ExpBiff8, eNach);
    return eRet;
}

extern "C" SAL_DLLPUBLIC_EXPORT bool TestImportCalcRTF(SvStream &rStream)
{
    ScDLL::Init();
    ScDocument aDocument;
    ScDocOptions aDocOpt = aDocument.GetDocOptions();
    aDocOpt.SetLookUpColRowNames(false);
    aDocument.SetDocOptions(aDocOpt);
    aDocument.MakeTable(0);
    aDocument.EnableExecuteLink(false);
    aDocument.SetInsertingFromOtherDoc(true);
    ScRange aRange;
    return ScFormatFilter::Get().ScImportRTF(rStream, OUString(), &aDocument, aRange) == ERRCODE_NONE;
}

extern "C" SAL_DLLPUBLIC_EXPORT bool TestImportXLS(SvStream& rStream)
{
    ScDLL::Init();
    SfxMedium aMedium;
    css::uno::Reference<css::io::XInputStream> xStm(new utl::OInputStreamWrapper(rStream));
    aMedium.GetItemSet()->Put(SfxUnoAnyItem(SID_INPUTSTREAM, css::uno::makeAny(xStm)));

    ScDocShellRef xDocShell = new ScDocShell(SfxModelFlags::EMBEDDED_OBJECT |
                                             SfxModelFlags::DISABLE_EMBEDDED_SCRIPTS |
                                             SfxModelFlags::DISABLE_DOCUMENT_RECOVERY);

    xDocShell->DoInitNew();

    ScDocument& rDoc = xDocShell->GetDocument();

    ScDocOptions aDocOpt = rDoc.GetDocOptions();
    aDocOpt.SetLookUpColRowNames(false);
    rDoc.SetDocOptions(aDocOpt);
    rDoc.MakeTable(0);
    rDoc.EnableExecuteLink(false);
    rDoc.SetInsertingFromOtherDoc(true);
    rDoc.InitDrawLayer(xDocShell.get());
    bool bRet(false);
    try
    {
        bRet = ScFormatFilter::Get().ScImportExcel(aMedium, &rDoc, EIF_AUTO) == ERRCODE_NONE;
    }
    catch (const css::ucb::ContentCreationException&)
    {
    }
    catch (const std::out_of_range&)
    {
    }
    xDocShell->DoClose();
    xDocShell.clear();
    return bRet;
}

extern "C" SAL_DLLPUBLIC_EXPORT bool TestImportDIF(SvStream &rStream)
{
    ScDLL::Init();
    ScDocument aDocument;
    ScDocOptions aDocOpt = aDocument.GetDocOptions();
    aDocOpt.SetLookUpColRowNames(false);
    aDocument.SetDocOptions(aDocOpt);
    aDocument.MakeTable(0);
    aDocument.EnableExecuteLink(false);
    aDocument.SetInsertingFromOtherDoc(true);
    return ScFormatFilter::Get().ScImportDif(rStream, &aDocument, ScAddress(0, 0, 0), RTL_TEXTENCODING_IBM_850) == ERRCODE_NONE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
