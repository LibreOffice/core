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
    std::unique_ptr<vcl::pdf::PDFiumPage> mpPage;
    std::unique_ptr<vcl::pdf::PDFiumTextPage> mpTextPage;
    std::unique_ptr<vcl::pdf::PDFiumSearchHandle> mpSearchHandle;

public:
    sal_Int32 mnPageIndex;
    int mnCurrentIndex;
    OUString maSearchString;
    VectorGraphicSearchOptions maOptions;

    SearchContext(std::unique_ptr<vcl::pdf::PDFiumDocument>& pPdfDocument, sal_Int32 nPageIndex)
        : mpPdfDocument(pPdfDocument)
        , mnPageIndex(nPageIndex)
        , mnCurrentIndex(-1)
    {
    }

    ~SearchContext()
    {
        if (mpSearchHandle)
            mpSearchHandle.reset();
        if (mpTextPage)
            mpTextPage.reset();
        if (mpPage)
            mpPage.reset();
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
            mpSearchHandle.reset();

        if (mpTextPage)
            mpTextPage.reset();

        if (mpPage)
            mpPage.reset();

        maSearchString = rSearchString;
        maOptions = rOptions;

        mpPage = mpPdfDocument->openPage(mnPageIndex);
        if (!mpPage)
            return false;

        mpTextPage = mpPage->getTextPage();
        if (!mpTextPage)
            return false;

        // Index where to start to search. -1 => at the end
        int nStartIndex = maOptions.meStartPosition == SearchStartPosition::End ? -1 : 0;

        if (mnCurrentIndex >= 0)
            nStartIndex = mnCurrentIndex;

        // vcl::pdf::PDFFindFlags::MatchCase, vcl::pdf::PDFFindFlags::MatchWholeWord, vcl::pdf::PDFFindFlags::Consecutive
        // vcl::pdf::PDFFindFlags::MatchCase - If not set, it will not match case by default.
        // vcl::pdf::PDFFindFlags::MatchWholeWord - If not set, it will not match the whole word by default.
        // vcl::pdf::PDFFindFlags::Consecutive - If not set, it will skip past the current match to look for the next match.
        vcl::pdf::PDFFindFlags nSearchFlags{};
        if (maOptions.mbMatchCase)
            nSearchFlags |= vcl::pdf::PDFFindFlags::MatchCase;
        if (maOptions.mbMatchWholeWord)
            nSearchFlags |= vcl::pdf::PDFFindFlags::MatchWholeWord;

        mpSearchHandle = mpTextPage->findStart(maSearchString, nSearchFlags, nStartIndex);

        return mpSearchHandle != nullptr;
    }

    bool next()
    {
        if (mpSearchHandle && mpSearchHandle->findNext())
        {
            mnCurrentIndex = index();
            return true;
        }
        return false;
    }

    bool previous()
    {
        if (mpSearchHandle && mpSearchHandle->findPrev())
        {
            mnCurrentIndex = index();
            return true;
        }
        return false;
    }

    int index()
    {
        if (mpSearchHandle)
            return mpSearchHandle->getSearchResultIndex();
        return -1;
    }

    int size()
    {
        if (mpSearchHandle)
            return mpSearchHandle->getSearchCount();
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
            basegfx::B2DRectangle aRectangle = mpTextPage->getCharBox(nIndex + nCount, fPageHeight);
            if (!aRectangle.isEmpty())
            {
                aRectangles.push_back(aRectangle);
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
        switch (vcl::pdf::PDFium::getLastErrorCode())
        {
            case vcl::pdf::PDFErrorType::Success:
                break;
            case vcl::pdf::PDFErrorType::Unknown:
                break;
            case vcl::pdf::PDFErrorType::File:
                break;
            case vcl::pdf::PDFErrorType::Format:
                break;
            case vcl::pdf::PDFErrorType::Password:
                break;
            case vcl::pdf::PDFErrorType::Security:
                break;
            case vcl::pdf::PDFErrorType::Page:
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
