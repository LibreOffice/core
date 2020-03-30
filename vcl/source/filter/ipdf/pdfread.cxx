/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/pdfread.hxx>

#include <config_features.h>

#if HAVE_FEATURE_PDFIUM
#include <fpdfview.h>
#include <fpdf_edit.h>
#include <fpdf_save.h>
#endif

#include <vcl/graph.hxx>
#include <bitmapwriteaccess.hxx>
#include <unotools/ucbstreamhelper.hxx>

using namespace com::sun::star;

namespace
{
#if HAVE_FEATURE_PDFIUM

/// Callback class to be used with FPDF_SaveWithVersion().
struct CompatibleWriter : public FPDF_FILEWRITE
{
    SvMemoryStream m_aStream;
};

int CompatibleWriterCallback(FPDF_FILEWRITE* pFileWrite, const void* pData, unsigned long nSize)
{
    auto pImpl = static_cast<CompatibleWriter*>(pFileWrite);
    pImpl->m_aStream.WriteBytes(pData, nSize);
    return 1;
}

/// Convert to inch, then assume 96 DPI.
inline double pointToPixel(const double fPoint, const double fResolutionDPI)
{
    return fPoint * fResolutionDPI / 72.;
}

/// Decide if PDF data is old enough to be compatible.
bool isCompatible(SvStream& rInStream, sal_uInt64 nPos, sal_uInt64 nSize)
{
    if (nSize < 8)
        return false;

    // %PDF-x.y
    sal_uInt8 aFirstBytes[8];
    rInStream.Seek(nPos);
    sal_uLong nRead = rInStream.ReadBytes(aFirstBytes, 8);
    if (nRead < 8)
        return false;

    if (aFirstBytes[0] != '%' || aFirstBytes[1] != 'P' || aFirstBytes[2] != 'D'
        || aFirstBytes[3] != 'F' || aFirstBytes[4] != '-')
        return false;

    sal_Int32 nMajor = OString(aFirstBytes[5]).toInt32();
    sal_Int32 nMinor = OString(aFirstBytes[7]).toInt32();
    return !(nMajor > 1 || (nMajor == 1 && nMinor > 5));
}

/// Takes care of transparently downgrading the version of the PDF stream in
/// case it's too new for our PDF export.
bool getCompatibleStream(SvStream& rInStream, SvStream& rOutStream, sal_uInt64 nPos,
                         sal_uInt64 nSize)
{
    bool bCompatible = isCompatible(rInStream, nPos, nSize);
    rInStream.Seek(nPos);
    if (bCompatible)
        // Not converting.
        rOutStream.WriteStream(rInStream, nSize);
    else
    {
        // Downconvert to PDF-1.5.
        FPDF_LIBRARY_CONFIG aConfig;
        aConfig.version = 2;
        aConfig.m_pUserFontPaths = nullptr;
        aConfig.m_pIsolate = nullptr;
        aConfig.m_v8EmbedderSlot = 0;
        FPDF_InitLibraryWithConfig(&aConfig);

        // Read input into a buffer.
        SvMemoryStream aInBuffer;
        aInBuffer.WriteStream(rInStream, nSize);

        // Load the buffer using pdfium.
        FPDF_DOCUMENT pPdfDocument
            = FPDF_LoadMemDocument(aInBuffer.GetData(), aInBuffer.GetSize(), /*password=*/nullptr);
        if (!pPdfDocument)
            return false;

        CompatibleWriter aWriter;
        aWriter.version = 1;
        aWriter.WriteBlock = &CompatibleWriterCallback;

        // 15 means PDF-1.5.
        if (!FPDF_SaveWithVersion(pPdfDocument, &aWriter, 0, 15))
            return false;

        FPDF_CloseDocument(pPdfDocument);
        FPDF_DestroyLibrary();

        aWriter.m_aStream.Seek(STREAM_SEEK_TO_BEGIN);
        rOutStream.WriteStream(aWriter.m_aStream);
    }

    return rOutStream.good();
}
#else
bool getCompatibleStream(SvStream& rInStream, SvStream& rOutStream, sal_uInt64 nPos,
                         sal_uInt64 nSize)
{
    rInStream.Seek(nPos);
    rOutStream.WriteStream(rInStream, nSize);
    return rOutStream.good();
}
#endif // HAVE_FEATURE_PDFIUM

VectorGraphicDataArray createVectorGraphicDataArray(SvStream& rStream)
{
    // Save the original PDF stream for later use.
    SvMemoryStream aMemoryStream;
    if (!getCompatibleStream(rStream, aMemoryStream, STREAM_SEEK_TO_BEGIN, STREAM_SEEK_TO_END))
        return VectorGraphicDataArray();

    const sal_uInt32 nStreamLength = aMemoryStream.TellEnd();

    VectorGraphicDataArray aPdfData(nStreamLength);

    aMemoryStream.Seek(STREAM_SEEK_TO_BEGIN);
    aMemoryStream.ReadBytes(aPdfData.begin(), nStreamLength);
    if (aMemoryStream.GetError())
        return VectorGraphicDataArray();

    return aPdfData;
}

} // end anonymous namespace

namespace vcl
{
size_t RenderPDFBitmaps(const void* pBuffer, int nSize, std::vector<Bitmap>& rBitmaps,
                        const size_t nFirstPage, int nPages, const double fResolutionDPI)
{
#if HAVE_FEATURE_PDFIUM
    FPDF_LIBRARY_CONFIG aConfig;
    aConfig.version = 2;
    aConfig.m_pUserFontPaths = nullptr;
    aConfig.m_pIsolate = nullptr;
    aConfig.m_v8EmbedderSlot = 0;
    FPDF_InitLibraryWithConfig(&aConfig);

    // Load the buffer using pdfium.
    FPDF_DOCUMENT pPdfDocument = FPDF_LoadMemDocument(pBuffer, nSize, /*password=*/nullptr);
    if (!pPdfDocument)
        return 0;

    const int nPageCount = FPDF_GetPageCount(pPdfDocument);
    if (nPages <= 0)
        nPages = nPageCount;
    const size_t nLastPage = std::min<int>(nPageCount, nFirstPage + nPages) - 1;
    for (size_t nPageIndex = nFirstPage; nPageIndex <= nLastPage; ++nPageIndex)
    {
        // Render next page.
        FPDF_PAGE pPdfPage = FPDF_LoadPage(pPdfDocument, nPageIndex);
        if (!pPdfPage)
            break;

        // Returned unit is points, convert that to pixel.
        const size_t nPageWidth = pointToPixel(FPDF_GetPageWidth(pPdfPage), fResolutionDPI);
        const size_t nPageHeight = pointToPixel(FPDF_GetPageHeight(pPdfPage), fResolutionDPI);
        FPDF_BITMAP pPdfBitmap = FPDFBitmap_Create(nPageWidth, nPageHeight, /*alpha=*/1);
        if (!pPdfBitmap)
            break;

        const FPDF_DWORD nColor = FPDFPage_HasTransparency(pPdfPage) ? 0x00000000 : 0xFFFFFFFF;
        FPDFBitmap_FillRect(pPdfBitmap, 0, 0, nPageWidth, nPageHeight, nColor);
        FPDF_RenderPageBitmap(pPdfBitmap, pPdfPage, /*start_x=*/0, /*start_y=*/0, nPageWidth,
                              nPageHeight, /*rotate=*/0, /*flags=*/0);

        // Save the buffer as a bitmap.
        Bitmap aBitmap(Size(nPageWidth, nPageHeight), 24);
        {
            BitmapScopedWriteAccess pWriteAccess(aBitmap);
            const auto pPdfBuffer = static_cast<ConstScanline>(FPDFBitmap_GetBuffer(pPdfBitmap));
            const int nStride = FPDFBitmap_GetStride(pPdfBitmap);
            for (size_t nRow = 0; nRow < nPageHeight; ++nRow)
            {
                ConstScanline pPdfLine = pPdfBuffer + (nStride * nRow);
                // pdfium byte order is BGRA.
                pWriteAccess->CopyScanline(nRow, pPdfLine, ScanlineFormat::N32BitTcBgra, nStride);
            }
        }

        rBitmaps.emplace_back(std::move(aBitmap));
        FPDFBitmap_Destroy(pPdfBitmap);
        FPDF_ClosePage(pPdfPage);
    }

    FPDF_CloseDocument(pPdfDocument);
    FPDF_DestroyLibrary();

    return rBitmaps.size();
#else
    (void)pBuffer;
    (void)nSize;
    (void)rBitmaps;
    (void)nFirstPage;
    (void)nPages;
    (void)fResolutionDPI;
    return 0;
#endif // HAVE_FEATURE_PDFIUM
}

bool ImportPDF(SvStream& rStream, Graphic& rGraphic)
{
    VectorGraphicDataArray aPdfDataArray = createVectorGraphicDataArray(rStream);
    if (!aPdfDataArray.hasElements())
        return false;

    auto aVectorGraphicDataPtr = std::make_shared<VectorGraphicData>(aPdfDataArray, OUString(),
                                                                     VectorGraphicDataType::Pdf);

    rGraphic = Graphic(aVectorGraphicDataPtr);
    return true;
}

size_t ImportPDFUnloaded(const OUString& rURL, std::vector<std::pair<Graphic, Size>>& rGraphics)
{
#if HAVE_FEATURE_PDFIUM
    std::unique_ptr<SvStream> xStream(
        ::utl::UcbStreamHelper::CreateStream(rURL, StreamMode::READ | StreamMode::SHARE_DENYNONE));

    // Save the original PDF stream for later use.
    VectorGraphicDataArray aPdfDataArray = createVectorGraphicDataArray(*xStream);
    if (!aPdfDataArray.hasElements())
        return 0;

    // Prepare the link with the PDF stream.
    const size_t nGraphicContentSize = aPdfDataArray.getLength();
    std::unique_ptr<sal_uInt8[]> pGraphicContent(new sal_uInt8[nGraphicContentSize]);

    std::copy(aPdfDataArray.begin(), aPdfDataArray.end(), pGraphicContent.get());

    auto pGfxLink = std::make_shared<GfxLink>(std::move(pGraphicContent), nGraphicContentSize,
                                              GfxLinkType::NativePdf);

    FPDF_LIBRARY_CONFIG aConfig;
    aConfig.version = 2;
    aConfig.m_pUserFontPaths = nullptr;
    aConfig.m_pIsolate = nullptr;
    aConfig.m_v8EmbedderSlot = 0;
    FPDF_InitLibraryWithConfig(&aConfig);

    // Load the buffer using pdfium.
    FPDF_DOCUMENT pPdfDocument
        = FPDF_LoadMemDocument(pGfxLink->GetData(), pGfxLink->GetDataSize(), /*password=*/nullptr);
    if (!pPdfDocument)
        return 0;

    const int nPageCount = FPDF_GetPageCount(pPdfDocument);
    if (nPageCount <= 0)
        return 0;

    for (int nPageIndex = 0; nPageIndex < nPageCount; ++nPageIndex)
    {
        double fPageWidth = 0;
        double fPageHeight = 0;
        if (FPDF_GetPageSizeByIndex(pPdfDocument, nPageIndex, &fPageWidth, &fPageHeight) == 0)
            continue;

        // Returned unit is points, convert that to 100th mm (hmm).
        // 1 pt = 20 twips, 1 twip = 1.7638888888888889 hmm
        // TODO: use some conversion class for that
        constexpr double pointToHMMconversionRatio = 20.0 * 1.7638888888888889;

        long nPageWidth = fPageWidth * pointToHMMconversionRatio;
        long nPageHeight = fPageHeight * pointToHMMconversionRatio;

        auto aVectorGraphicDataPtr = std::make_shared<VectorGraphicData>(
            aPdfDataArray, OUString(), VectorGraphicDataType::Pdf, nPageIndex);

        // Create the Graphic with the VectorGraphicDataPtr and link the original PDF stream.
        // We swap out this Graphic as soon as possible, and a later swap in
        // actually renders the correct Bitmap on demand.
        Graphic aGraphic(aVectorGraphicDataPtr);
        aGraphic.SetGfxLink(pGfxLink);

        rGraphics.emplace_back(std::move(aGraphic), Size(nPageWidth, nPageHeight));
    }

    FPDF_CloseDocument(pPdfDocument);
    FPDF_DestroyLibrary();

    return rGraphics.size();
#else
    (void)rURL;
    (void)rGraphics;
    return 0;
#endif // HAVE_FEATURE_PDFIUM
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
