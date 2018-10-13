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
public:
    CompatibleWriter();
    static int WriteBlockCallback(FPDF_FILEWRITE* pFileWrite, const void* pData,
                                  unsigned long nSize);

    SvMemoryStream m_aStream;
};

CompatibleWriter::CompatibleWriter()
{
    FPDF_FILEWRITE::version = 1;
    FPDF_FILEWRITE::WriteBlock = CompatibleWriter::WriteBlockCallback;
}

int CompatibleWriter::WriteBlockCallback(FPDF_FILEWRITE* pFileWrite, const void* pData,
                                         unsigned long nSize)
{
    auto pImpl = static_cast<CompatibleWriter*>(pFileWrite);
    pImpl->m_aStream.WriteBytes(pData, nSize);
    return 1;
}

/// Convert to inch, then assume 96 DPI.
double pointToPixel(const double fPoint, const double fResolutionDPI)
{
    return fPoint * fResolutionDPI / 72.;
}

/// Does PDF to bitmap conversion using pdfium.
size_t generatePreview(SvStream& rStream, std::vector<Bitmap>& rBitmaps, sal_uInt64 nPos,
                       sal_uInt64 nSize, const size_t nFirstPage, int nPages,
                       const double fResolutionDPI = 96.)
{
    FPDF_LIBRARY_CONFIG aConfig;
    aConfig.version = 2;
    aConfig.m_pUserFontPaths = nullptr;
    aConfig.m_pIsolate = nullptr;
    aConfig.m_v8EmbedderSlot = 0;
    FPDF_InitLibraryWithConfig(&aConfig);

    // Read input into a buffer.
    SvMemoryStream aInBuffer;
    rStream.Seek(nPos);
    aInBuffer.WriteStream(rStream, nSize);

    // Load the buffer using pdfium.
    FPDF_DOCUMENT pPdfDocument
        = FPDF_LoadMemDocument(aInBuffer.GetData(), aInBuffer.GetSize(), /*password=*/nullptr);
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
size_t generatePreview(SvStream&, std::vector<Bitmap>&, sal_uInt64, sal_uInt64, size_t, int,
                       const double)
{
    return 0;
}

bool getCompatibleStream(SvStream& rInStream, SvStream& rOutStream, sal_uInt64 nPos,
                         sal_uInt64 nSize)
{
    rInStream.Seek(nPos);
    rOutStream.WriteStream(rInStream, nSize);
    return rOutStream.good();
}
#endif // HAVE_FEATURE_PDFIUM
}

namespace vcl
{
bool ImportPDF(SvStream& rStream, Bitmap& rBitmap, size_t nPageIndex,
               css::uno::Sequence<sal_Int8>& rPdfData, sal_uInt64 nPos, sal_uInt64 nSize,
               const double fResolutionDPI)
{
    // Get the preview of the first page.
    std::vector<Bitmap> aBitmaps;
    if (generatePreview(rStream, aBitmaps, nPos, nSize, nPageIndex, 1, fResolutionDPI) != 1
        || aBitmaps.empty())
        return false;

    rBitmap = aBitmaps[0];

    // Save the original PDF stream for later use.
    SvMemoryStream aMemoryStream;
    if (!getCompatibleStream(rStream, aMemoryStream, nPos, nSize))
        return false;

    rPdfData = css::uno::Sequence<sal_Int8>(aMemoryStream.TellEnd());
    aMemoryStream.Seek(STREAM_SEEK_TO_BEGIN);
    aMemoryStream.ReadBytes(rPdfData.getArray(), rPdfData.getLength());

    return true;
}

bool ImportPDF(SvStream& rStream, Graphic& rGraphic, const double fResolutionDPI)
{
    uno::Sequence<sal_Int8> aPdfData;
    Bitmap aBitmap;
    const bool bRet = ImportPDF(rStream, aBitmap, 0, aPdfData, STREAM_SEEK_TO_BEGIN,
                                STREAM_SEEK_TO_END, fResolutionDPI);
    rGraphic = aBitmap;
    rGraphic.setPdfData(std::make_shared<css::uno::Sequence<sal_Int8>>(aPdfData));
    rGraphic.setPageNumber(0); // We currently import only the first page.
    return bRet;
}

size_t ImportPDF(const OUString& rURL, std::vector<Bitmap>& rBitmaps,
                 css::uno::Sequence<sal_Int8>& rPdfData, const double fResolutionDPI)
{
    std::unique_ptr<SvStream> xStream(
        ::utl::UcbStreamHelper::CreateStream(rURL, StreamMode::READ | StreamMode::SHARE_DENYNONE));

    if (generatePreview(*xStream, rBitmaps, STREAM_SEEK_TO_BEGIN, STREAM_SEEK_TO_END, 0, -1,
                        fResolutionDPI)
        == 0)
        return 0;

    // Save the original PDF stream for later use.
    SvMemoryStream aMemoryStream;
    if (!getCompatibleStream(*xStream, aMemoryStream, STREAM_SEEK_TO_BEGIN, STREAM_SEEK_TO_END))
        return 0;

    rPdfData = css::uno::Sequence<sal_Int8>(aMemoryStream.TellEnd());
    aMemoryStream.Seek(STREAM_SEEK_TO_BEGIN);
    aMemoryStream.ReadBytes(rPdfData.getArray(), rPdfData.getLength());

    return rBitmaps.size();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
