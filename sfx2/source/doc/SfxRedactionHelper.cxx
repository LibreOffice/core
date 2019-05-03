/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SfxRedactionHelper.hxx>

#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>

#include <sfx2/request.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/viewfrm.hxx>

#include <svl/eitem.hxx>
#include <svl/stritem.hxx>

#include <svtools/DocumentToGraphicRenderer.hxx>

#include <vcl/gdimtf.hxx>
#include <vcl/graph.hxx>
#include <sal/log.hxx>

#include <vcl/wmf.hxx>
#include <vcl/gdimetafiletools.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

bool SfxRedactionHelper::isRedactMode(const SfxRequest& rReq)
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    if (pArgs)
    {
        const SfxBoolItem* pIsRedactMode = rReq.GetArg<SfxBoolItem>(SID_IS_REDACT_MODE);
        if (pIsRedactMode && pIsRedactMode->GetValue())
            return true;
    }

    return false;
}

OUString SfxRedactionHelper::getStringParam(const SfxRequest& rReq, const sal_uInt16& nParamId)
{
    OUString sStringParam;

    const SfxItemSet* pArgs = rReq.GetArgs();
    if (!pArgs)
        return sStringParam;

    const SfxStringItem* pStringArg = rReq.GetArg<SfxStringItem>(nParamId);
    if (!pStringArg)
        return sStringParam;

    sStringParam = pStringArg->GetValue();
    return sStringParam;
}

namespace
{
void fixMetaFile(GDIMetaFile& tmpMtf)
{
    SvMemoryStream aDestStrm(65535, 65535);
    ConvertGDIMetaFileToWMF(tmpMtf, aDestStrm, nullptr, false);
    aDestStrm.Seek(0);

    tmpMtf.Clear();

    ReadWindowMetafile(aDestStrm, tmpMtf);
}
}

void SfxRedactionHelper::getPageMetaFilesFromDoc(std::vector<GDIMetaFile>& aMetaFiles,
                                                 std::vector<::Size>& aPageSizes,
                                                 const sal_Int32& nPages,
                                                 DocumentToGraphicRenderer& aRenderer)
{
    for (sal_Int32 nPage = 1; nPage <= nPages; ++nPage)
    {
        ::Size aDocumentSizePixel = aRenderer.getDocumentSizeInPixels(nPage);
        ::Point aLogicPos;
        ::Point aCalcPageLogicPos;
        ::Size aCalcPageContentSize;
        ::Size aLogic = aRenderer.getDocumentSizeIn100mm(nPage, &aLogicPos, &aCalcPageLogicPos,
                                                         &aCalcPageContentSize);
        // FIXME: This is a temporary hack. Need to figure out a proper way to derive this scale factor.
        ::Size aTargetSize(aDocumentSizePixel.Width(), aDocumentSizePixel.Height());

        aPageSizes.push_back(aLogic);

        Graphic aGraphic = aRenderer.renderToGraphic(nPage, aDocumentSizePixel, aTargetSize,
                                                     COL_TRANSPARENT, true);
        auto& rGDIMetaFile = const_cast<GDIMetaFile&>(aGraphic.GetGDIMetaFile());

        // Set preferred map unit and size on the metafile, so the Shape size
        // will be correct in MM.
        MapMode aMapMode;
        aMapMode.SetMapUnit(MapUnit::Map100thMM);

        rGDIMetaFile.SetPrefMapMode(aMapMode);
        rGDIMetaFile.SetPrefSize(aLogic);

        fixMetaFile(rGDIMetaFile);

        aMetaFiles.push_back(rGDIMetaFile);
    }
}

void SfxRedactionHelper::addPagesToDraw(uno::Reference<XComponent>& xComponent,
                                        const sal_Int32& nPages,
                                        const std::vector<GDIMetaFile>& aMetaFiles,
                                        const std::vector<::Size>& aPageSizes)
{
    // Access the draw pages
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(xComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPages> xDrawPages = xDrawPagesSupplier->getDrawPages();

    uno::Reference<css::lang::XMultiServiceFactory> xFactory(xComponent, uno::UNO_QUERY);

    for (sal_Int32 nPage = 0; nPage < nPages; ++nPage)
    {
        GDIMetaFile rGDIMetaFile = aMetaFiles[nPage];
        Graphic aGraphic(rGDIMetaFile);

        sal_Int32 nPageHeight(aPageSizes[nPage].Height());
        sal_Int32 nPageWidth(aPageSizes[nPage].Width());

        uno::Reference<graphic::XGraphic> xGraph = aGraphic.GetXGraphic();
        uno::Reference<drawing::XDrawPage> xPage = xDrawPages->insertNewByIndex(nPage);

        // Set page size
        uno::Reference<beans::XPropertySet> xPageProperySet(xPage, uno::UNO_QUERY);
        xPageProperySet->setPropertyValue("Height", css::uno::makeAny(nPageHeight));
        xPageProperySet->setPropertyValue("Width", css::uno::makeAny(nPageWidth));

        // Create and insert the shape
        uno::Reference<drawing::XShape> xShape(
            xFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xShapeProperySet(xShape, uno::UNO_QUERY);
        xShapeProperySet->setPropertyValue("Graphic", uno::Any(xGraph));
        //xShapeProperySet->setPropertyValue("MoveProtect", uno::Any(true));
        //xShapeProperySet->setPropertyValue("SizeProtect", uno::Any(true));

        // Set size and position
        xShape->setSize(
            awt::Size(rGDIMetaFile.GetPrefSize().Width(), rGDIMetaFile.GetPrefSize().Height()));

        xPage->add(xShape);

        // Shapes from Calc have the size of the content instead of the whole standard page (like A4)
        // so it needs positioning on the draw page
        /*if (bIsCalc)
            xShape->setPosition(awt::Point(0, 0));*/
    }

    // Remove the extra page at the beginning
    uno::Reference<drawing::XDrawPage> xPage(xDrawPages->getByIndex(0), uno::UNO_QUERY_THROW);
    xDrawPages->remove(xPage);
}

void SfxRedactionHelper::showRedactionToolbar(SfxViewFrame* pViewFrame)
{
    if (!pViewFrame)
        return;

    Reference<frame::XFrame> xFrame = pViewFrame->GetFrame().GetFrameInterface();
    Reference<css::beans::XPropertySet> xPropSet(xFrame, UNO_QUERY);
    Reference<css::frame::XLayoutManager> xLayoutManager;

    if (xPropSet.is())
    {
        try
        {
            Any aValue = xPropSet->getPropertyValue("LayoutManager");
            aValue >>= xLayoutManager;
            xLayoutManager->createElement("private:resource/toolbar/redactionbar");
            xLayoutManager->showElement("private:resource/toolbar/redactionbar");
        }
        catch (const css::uno::RuntimeException&)
        {
            throw;
        }
        catch (css::uno::Exception&)
        {
            SAL_WARN("sfx.doc", "Exception while trying to show the Redaction Toolbar!");
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
