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

#include <config_features.h>

#if HAVE_FEATURE_PDFIUM

#include <vcl/filter/PDFiumLibrary.hxx>
#include <tools/UnitConversion.hxx>

#include <sal/config.h>

#include <fpdf_doc.h>
#include <fpdf_text.h>

namespace
{
class SearchContext
{
private:
    std::unique_ptr<vcl::pdf::PDFiumDocument>& mpPdfDocument;
    FPDF_PAGE mpPage;
    FPDF_TEXTPAGE mpTextPage;
    FPDF_SCHHANDLE mpSearchHandle;

public:
    sal_Int32 mnPageIndex;
    int mnCurrentIndex;
    OUString maSearchString;
    VectorGraphicSearchOptions maOptions;

    SearchContext(std::unique_ptr<vcl::pdf::PDFiumDocument>& pPdfDocument, sal_Int32 nPageIndex)
        : mpPdfDocument(pPdfDocument)
        , mpPage(nullptr)
        , mpTextPage(nullptr)
        , mpSearchHandle(nullptr)
        , mnPageIndex(nPageIndex)
        , mnCurrentIndex(-1)
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

        basegfx::B2DSize aPDFSize = mpPdfDocument->getPageSize(mnPageIndex);
        aSize = basegfx::B2DSize(convertPointToMm100(aPDFSize.getX()),
                                 convertPointToMm100(aPDFSize.getY()));
        return aSize;
    }

    bool initialize(OUString const& rSearchString, VectorGraphicSearchOptions const& rOptions)
    {
        if (!mpPdfDocument)
            return false;

        if (rSearchString == maSearchString)
            return true;

        if (mpSearchHandle)
            FPDFText_FindClose(mpSearchHandle);

        if (mpTextPage)
            FPDFText_ClosePage(mpTextPage);

        if (mpPage)
            FPDF_ClosePage(mpPage);

        maSearchString = rSearchString;
        maOptions = rOptions;

        mpPage = FPDF_LoadPage(mpPdfDocument->getPointer(), mnPageIndex);
        if (!mpPage)
            return false;

        mpTextPage = FPDFText_LoadPage(mpPage);
        if (!mpTextPage)
            return false;

        FPDF_WIDESTRING pString = reinterpret_cast<FPDF_WIDESTRING>(maSearchString.getStr());

        // Index where to start to search. -1 => at the end
        int nStartIndex = maOptions.meStartPosition == SearchStartPosition::End ? -1 : 0;

        if (mnCurrentIndex >= 0)
            nStartIndex = mnCurrentIndex;

        // FPDF_MATCHCASE, FPDF_MATCHWHOLEWORD, FPDF_CONSECUTIVE
        // FPDF_MATCHCASE - If not set, it will not match case by default.
        // FPDF_MATCHWHOLEWORD - If not set, it will not match the whole word by default.
        // FPDF_CONSECUTIVE - If not set, it will skip past the current match to look for the next match.
        int nSearchFlags = 0;
        if (maOptions.mbMatchCase)
            nSearchFlags |= FPDF_MATCHCASE;
        if (maOptions.mbMatchWholeWord)
            nSearchFlags |= FPDF_MATCHWHOLEWORD;

        mpSearchHandle = FPDFText_FindStart(mpTextPage, pString, nSearchFlags, nStartIndex);

        return mpSearchHandle != nullptr;
    }

    bool next()
    {
        if (mpSearchHandle && FPDFText_FindNext(mpSearchHandle))
        {
            mnCurrentIndex = index();
            return true;
        }
        return false;
    }

    bool previous()
    {
        if (mpSearchHandle && FPDFText_FindPrev(mpSearchHandle))
        {
            mnCurrentIndex = index();
            return true;
        }
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

} // end anonymous namespace

class VectorGraphicSearch::Implementation
{
public:
    std::shared_ptr<vcl::pdf::PDFium> mpPDFium;
    std::unique_ptr<vcl::pdf::PDFiumDocument> mpPdfDocument;

    std::unique_ptr<SearchContext> mpSearchContext;

    Implementation()
        : mpPDFium(vcl::pdf::PDFiumLibrary::get())
    {
    }

    ~Implementation() { mpSearchContext.reset(); }
};

VectorGraphicSearch::VectorGraphicSearch(Graphic const& rGraphic)
    : mpImplementation(std::make_unique<VectorGraphicSearch::Implementation>())
    , maGraphic(rGraphic)
{
}

VectorGraphicSearch::~VectorGraphicSearch() { mpImplementation.reset(); }

bool VectorGraphicSearch::search(OUString const& rSearchString,
                                 VectorGraphicSearchOptions const& rOptions)
{
    if (!mpImplementation->mpSearchContext)
    {
        auto pData = maGraphic.getVectorGraphicData();

        if (pData && pData->getType() == VectorGraphicDataType::Pdf)
        {
            if (searchPDF(pData))
            {
                return mpImplementation->mpSearchContext->initialize(rSearchString, rOptions);
            }
        }
        return false;
    }
    return mpImplementation->mpSearchContext->initialize(rSearchString, rOptions);
}

bool VectorGraphicSearch::searchPDF(std::shared_ptr<VectorGraphicData> const& rData)
{
    mpImplementation->mpPdfDocument = mpImplementation->mpPDFium->openDocument(
        rData->getBinaryDataContainer().getData(), rData->getBinaryDataContainer().getSize());

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

    mpImplementation->mpSearchContext.reset(
        new SearchContext(mpImplementation->mpPdfDocument, nPageIndex));
    return true;
}

basegfx::B2DSize VectorGraphicSearch::pageSize()
{
    basegfx::B2DSize aSize;
    if (mpImplementation->mpSearchContext)
        aSize = mpImplementation->mpSearchContext->getPageSize();
    return aSize;
}

bool VectorGraphicSearch::next()
{
    if (mpImplementation->mpSearchContext)
        return mpImplementation->mpSearchContext->next();
    return false;
}

bool VectorGraphicSearch::previous()
{
    if (mpImplementation->mpSearchContext)
        return mpImplementation->mpSearchContext->previous();
    return false;
}

int VectorGraphicSearch::index()
{
    if (mpImplementation->mpSearchContext)
        return mpImplementation->mpSearchContext->index();
    return -1;
}

std::vector<basegfx::B2DRectangle> VectorGraphicSearch::getTextRectangles()
{
    if (mpImplementation->mpSearchContext)
        return mpImplementation->mpSearchContext->getTextRectangles();

    return std::vector<basegfx::B2DRectangle>();
}

#else // !HAVE_FEATURE_PDFIUM

class VectorGraphicSearch::Implementation
{
};

VectorGraphicSearch::VectorGraphicSearch(Graphic const& rGraphic)
    : maGraphic(rGraphic)
{
}

VectorGraphicSearch::~VectorGraphicSearch() {}

bool VectorGraphicSearch::search(OUString const& /*rSearchString*/,
                                 VectorGraphicSearchOptions const& /*rOptions*/)
{
    return false;
}

bool VectorGraphicSearch::searchPDF(std::shared_ptr<VectorGraphicData> const& /*rData*/)
{
    return false;
}

basegfx::B2DSize VectorGraphicSearch::pageSize() { return basegfx::B2DSize(); }

bool VectorGraphicSearch::next() { return false; }

bool VectorGraphicSearch::previous() { return false; }

int VectorGraphicSearch::index() { return -1; }

std::vector<basegfx::B2DRectangle> VectorGraphicSearch::getTextRectangles()
{
    return std::vector<basegfx::B2DRectangle>();
}

#endif // HAVE_FEATURE_PDFIUM

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
