/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/VectorGraphicSearch.hxx>

#include <sal/config.h>

#include <fpdf_doc.h>
#include <fpdf_text.h>

class VectorGraphicSearch::Implementation
{
public:
    FPDF_DOCUMENT mpPdfDocument;

    Implementation()
        : mpPdfDocument(nullptr)
    {
    }

    ~Implementation()
    {
        if (mpPdfDocument)
            FPDF_CloseDocument(mpPdfDocument);
    }
};

class SearchContext
{
public:
    bool bInitialized = false;

    FPDF_DOCUMENT mpPdfDocument;
    sal_Int32 mnPageIndex;
    FPDF_PAGE mpPage;
    FPDF_TEXTPAGE mpTextPage;
    OUString maSearchString;
    FPDF_SCHHANDLE mpSearchHandle;

    SearchContext(FPDF_DOCUMENT pPdfDocument, sal_Int32 nPageIndex, OUString const& rSearchString)
        : mpPdfDocument(pPdfDocument)
        , mnPageIndex(nPageIndex)
        , mpPage(nullptr)
        , mpTextPage(nullptr)
        , maSearchString(rSearchString)
        , mpSearchHandle(nullptr)
    {
    }

    ~SearchContext()
    {
        if (mpSearchHandle)
            FPDFText_FindClose(mpSearchHandle);
        if (mpTextPage)
            FPDFText_ClosePage(mpTextPage);
        if (mpPage)
            FPDF_ClosePage(mpPage);
    }

    basegfx::B2DSize getPageSize()
    {
        basegfx::B2DSize aSize;
        if (!mpPdfDocument)
            return aSize;

        FS_SIZEF aPDFSize;
        if (FPDF_GetPageSizeByIndexF(mpPdfDocument, mnPageIndex, &aPDFSize))
        {
            aSize = basegfx::B2DSize(convertPointToMm100(aPDFSize.width),
                                     convertPointToMm100(aPDFSize.height));
        }
        return aSize;
    }

    bool initialize()
    {
        if (!mpPdfDocument)
            return false;
        mpPage = FPDF_LoadPage(mpPdfDocument, mnPageIndex);
        if (!mpPage)
            return false;
        mpTextPage = FPDFText_LoadPage(mpPage);
        if (!mpTextPage)
            return false;

        FPDF_WIDESTRING pString = reinterpret_cast<FPDF_WIDESTRING>(maSearchString.getStr());
        mpSearchHandle = FPDFText_FindStart(mpTextPage, pString, 0, 0);

        return mpSearchHandle != nullptr;
    }

    bool next()
    {
        if (mpSearchHandle)
            return FPDFText_FindNext(mpSearchHandle);
        return false;
    }

    int index()
    {
        if (mpSearchHandle)
            return FPDFText_GetSchResultIndex(mpSearchHandle);
        return -1;
    }

    int size()
    {
        if (mpSearchHandle)
            return FPDFText_GetSchCount(mpSearchHandle);
        return -1;
    }

    std::vector<basegfx::B2DRectangle> getTextRectangles()
    {
        std::vector<basegfx::B2DRectangle> aRectangles;

        if (!mpTextPage || !mpSearchHandle)
            return aRectangles;

        int nIndex = index();
        if (nIndex < 0)
            return aRectangles;

        int nSize = size();
        if (nSize <= 0)
            return aRectangles;

        double fPageHeight = getPageSize().getY();

        for (int nCount = 0; nCount < nSize; nCount++)
        {
            double left = 0.0;
            double right = 0.0;
            double bottom = 0.0;
            double top = 0.0;

            if (FPDFText_GetCharBox(mpTextPage, nIndex + nCount, &left, &right, &bottom, &top))
            {
                left = convertPointToMm100(left);
                right = convertPointToMm100(right);
                top = fPageHeight - convertPointToMm100(top);
                bottom = fPageHeight - convertPointToMm100(bottom);

                aRectangles.emplace_back(left, bottom, right, top);
            }
        }

        return aRectangles;
    }
};

VectorGraphicSearch::VectorGraphicSearch(Graphic const& rGraphic)
    : mpImplementation(std::make_unique<VectorGraphicSearch::Implementation>())
    , maGraphic(rGraphic)
{
    FPDF_LIBRARY_CONFIG aConfig;
    aConfig.version = 2;
    aConfig.m_pUserFontPaths = nullptr;
    aConfig.m_pIsolate = nullptr;
    aConfig.m_v8EmbedderSlot = 0;
    FPDF_InitLibraryWithConfig(&aConfig);
}

VectorGraphicSearch::~VectorGraphicSearch()
{
    mpSearchContext.reset();
    mpImplementation.reset();
    FPDF_DestroyLibrary();
}

bool VectorGraphicSearch::search(OUString const& rSearchString)
{
    auto pData = maGraphic.getVectorGraphicData();

    if (pData && pData->getVectorGraphicDataType() == VectorGraphicDataType::Pdf)
    {
        return searchPDF(pData, rSearchString);
    }
    return false;
}

bool VectorGraphicSearch::searchPDF(std::shared_ptr<VectorGraphicData> const& rData,
                                    OUString const& rSearchString)
{
    if (rSearchString.isEmpty())
        return false;

    mpImplementation->mpPdfDocument
        = FPDF_LoadMemDocument(rData->getVectorGraphicDataArray().getConstArray(),
                               rData->getVectorGraphicDataArrayLength(), /*password=*/nullptr);

    if (!mpImplementation->mpPdfDocument)
    {
        //TODO: Handle failure to load.
        switch (FPDF_GetLastError())
        {
            case FPDF_ERR_SUCCESS:
                break;
            case FPDF_ERR_UNKNOWN:
                break;
            case FPDF_ERR_FILE:
                break;
            case FPDF_ERR_FORMAT:
                break;
            case FPDF_ERR_PASSWORD:
                break;
            case FPDF_ERR_SECURITY:
                break;
            case FPDF_ERR_PAGE:
                break;
            default:
                break;
        }
        return false;
    }

    sal_Int32 nPageIndex = std::max(rData->getPageIndex(), sal_Int32(0));

    mpSearchContext.reset(
        new SearchContext(mpImplementation->mpPdfDocument, nPageIndex, rSearchString));

    return mpSearchContext->initialize();
}

basegfx::B2DSize VectorGraphicSearch::pageSize()
{
    basegfx::B2DSize aSize;
    if (mpSearchContext)
        aSize = mpSearchContext->getPageSize();
    return aSize;
}

bool VectorGraphicSearch::next()
{
    if (mpSearchContext)
        return mpSearchContext->next();
    return false;
}

int VectorGraphicSearch::index()
{
    if (mpSearchContext)
        return mpSearchContext->index();
    return -1;
}

std::vector<basegfx::B2DRectangle> VectorGraphicSearch::getTextRectangles()
{
    if (mpSearchContext)
        return mpSearchContext->getTextRectangles();

    return std::vector<basegfx::B2DRectangle>();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
