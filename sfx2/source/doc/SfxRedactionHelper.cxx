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
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>

// For page margin related methods
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/sheet/XSpreadsheetView.hpp>

#include <sfx2/request.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/viewfrm.hxx>

#include <svl/eitem.hxx>
#include <svl/stritem.hxx>

#include <svtools/DocumentToGraphicRenderer.hxx>

#include <tools/gen.hxx>

#include <vcl/gdimtf.hxx>
#include <vcl/graph.hxx>
#include <sal/log.hxx>

#include <vcl/wmf.hxx>
#include <vcl/gdimetafiletools.hxx>
#include <vcl/metaact.hxx>
#include <vcl/outdev.hxx>
#include <vcl/vcllayout.hxx>

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
/*
 * Roundtrip the gdimetafile to and from WMF
 * to get rid of the position and size irregularities
 * We better check the conversion method to see what it
 * actually does to correct these issues, and do it ourselves.
 * */
void fixMetaFile(GDIMetaFile& tmpMtf)
{
    SvMemoryStream aDestStrm(65535, 65535);
    ConvertGDIMetaFileToWMF(tmpMtf, aDestStrm, nullptr, false);
    aDestStrm.Seek(0);

    tmpMtf.Clear();

    ReadWindowMetafile(aDestStrm, tmpMtf);
}

/*
 * Sets page margins for a Draw page. Negative values are considered erronous.
 * */
void setPageMargins(uno::Reference<beans::XPropertySet>& xPageProperySet,
                    const PageMargins& aPageMargins)
{
    if (aPageMargins.nTop < 0 || aPageMargins.nBottom < 0 || aPageMargins.nLeft < 0
        || aPageMargins.nRight < 0)
        return;

    xPageProperySet->setPropertyValue("BorderTop", css::uno::makeAny(aPageMargins.nTop));
    xPageProperySet->setPropertyValue("BorderBottom", css::uno::makeAny(aPageMargins.nBottom));
    xPageProperySet->setPropertyValue("BorderLeft", css::uno::makeAny(aPageMargins.nLeft));
    xPageProperySet->setPropertyValue("BorderRight", css::uno::makeAny(aPageMargins.nRight));
}

// #i10613# Extracted from ImplCheckRect::ImplCreate
tools::Rectangle ImplCalcActionBounds(const MetaAction& rAct, const OutputDevice& rOut,
                                      const OUString& sSubString, const sal_Int32& nStrPos)
{
    tools::Rectangle aActionBounds;

    switch (rAct.GetType())
    {
            /*case MetaActionType::PIXEL:
            aActionBounds = tools::Rectangle(static_cast<const MetaPixelAction&>(rAct).GetPoint(),
                                             Size(1, 1));
            break;

        case MetaActionType::POINT:
            aActionBounds = tools::Rectangle(static_cast<const MetaPointAction&>(rAct).GetPoint(),
                                             Size(1, 1));
            break;

        case MetaActionType::LINE:
        {
            const MetaLineAction& rMetaLineAction = static_cast<const MetaLineAction&>(rAct);
            aActionBounds
                = tools::Rectangle(rMetaLineAction.GetStartPoint(), rMetaLineAction.GetEndPoint());
            aActionBounds.Justify();
            const long nLineWidth(rMetaLineAction.GetLineInfo().GetWidth());
            if (nLineWidth)
            {
                const long nHalfLineWidth((nLineWidth + 1) / 2);
                aActionBounds.AdjustLeft(-nHalfLineWidth);
                aActionBounds.AdjustTop(-nHalfLineWidth);
                aActionBounds.AdjustRight(nHalfLineWidth);
                aActionBounds.AdjustBottom(nHalfLineWidth);
            }
            break;
        }

        case MetaActionType::RECT:
            aActionBounds = static_cast<const MetaRectAction&>(rAct).GetRect();
            break;

        case MetaActionType::ROUNDRECT:
            aActionBounds
                = tools::Polygon(static_cast<const MetaRoundRectAction&>(rAct).GetRect(),
                                 static_cast<const MetaRoundRectAction&>(rAct).GetHorzRound(),
                                 static_cast<const MetaRoundRectAction&>(rAct).GetVertRound())
                      .GetBoundRect();
            break;

        case MetaActionType::ELLIPSE:
        {
            const tools::Rectangle& rRect = static_cast<const MetaEllipseAction&>(rAct).GetRect();
            aActionBounds
                = tools::Polygon(rRect.Center(), rRect.GetWidth() >> 1, rRect.GetHeight() >> 1)
                      .GetBoundRect();
            break;
        }

        case MetaActionType::ARC:
            aActionBounds = tools::Polygon(static_cast<const MetaArcAction&>(rAct).GetRect(),
                                           static_cast<const MetaArcAction&>(rAct).GetStartPoint(),
                                           static_cast<const MetaArcAction&>(rAct).GetEndPoint(),
                                           PolyStyle::Arc)
                                .GetBoundRect();
            break;

        case MetaActionType::PIE:
            aActionBounds = tools::Polygon(static_cast<const MetaPieAction&>(rAct).GetRect(),
                                           static_cast<const MetaPieAction&>(rAct).GetStartPoint(),
                                           static_cast<const MetaPieAction&>(rAct).GetEndPoint(),
                                           PolyStyle::Pie)
                                .GetBoundRect();
            break;

        case MetaActionType::CHORD:
            aActionBounds
                = tools::Polygon(static_cast<const MetaChordAction&>(rAct).GetRect(),
                                 static_cast<const MetaChordAction&>(rAct).GetStartPoint(),
                                 static_cast<const MetaChordAction&>(rAct).GetEndPoint(),
                                 PolyStyle::Chord)
                      .GetBoundRect();
            break;

        case MetaActionType::POLYLINE:
        {
            const MetaPolyLineAction& rMetaPolyLineAction
                = static_cast<const MetaPolyLineAction&>(rAct);
            aActionBounds = rMetaPolyLineAction.GetPolygon().GetBoundRect();
            const long nLineWidth(rMetaPolyLineAction.GetLineInfo().GetWidth());
            if (nLineWidth)
            {
                const long nHalfLineWidth((nLineWidth + 1) / 2);
                aActionBounds.AdjustLeft(-nHalfLineWidth);
                aActionBounds.AdjustTop(-nHalfLineWidth);
                aActionBounds.AdjustRight(nHalfLineWidth);
                aActionBounds.AdjustBottom(nHalfLineWidth);
            }
            break;
        }

        case MetaActionType::POLYGON:
            aActionBounds = static_cast<const MetaPolygonAction&>(rAct).GetPolygon().GetBoundRect();
            break;

        case MetaActionType::POLYPOLYGON:
            aActionBounds
                = static_cast<const MetaPolyPolygonAction&>(rAct).GetPolyPolygon().GetBoundRect();
            break;

        case MetaActionType::BMP:
            aActionBounds = tools::Rectangle(
                static_cast<const MetaBmpAction&>(rAct).GetPoint(),
                rOut.PixelToLogic(
                    static_cast<const MetaBmpAction&>(rAct).GetBitmap().GetSizePixel()));
            break;

        case MetaActionType::BMPSCALE:
            aActionBounds
                = tools::Rectangle(static_cast<const MetaBmpScaleAction&>(rAct).GetPoint(),
                                   static_cast<const MetaBmpScaleAction&>(rAct).GetSize());
            break;

        case MetaActionType::BMPSCALEPART:
            aActionBounds
                = tools::Rectangle(static_cast<const MetaBmpScalePartAction&>(rAct).GetDestPoint(),
                                   static_cast<const MetaBmpScalePartAction&>(rAct).GetDestSize());
            break;

        case MetaActionType::BMPEX:
            aActionBounds = tools::Rectangle(
                static_cast<const MetaBmpExAction&>(rAct).GetPoint(),
                rOut.PixelToLogic(
                    static_cast<const MetaBmpExAction&>(rAct).GetBitmapEx().GetSizePixel()));
            break;

        case MetaActionType::BMPEXSCALE:
            aActionBounds
                = tools::Rectangle(static_cast<const MetaBmpExScaleAction&>(rAct).GetPoint(),
                                   static_cast<const MetaBmpExScaleAction&>(rAct).GetSize());
            break;

        case MetaActionType::BMPEXSCALEPART:
            aActionBounds = tools::Rectangle(
                static_cast<const MetaBmpExScalePartAction&>(rAct).GetDestPoint(),
                static_cast<const MetaBmpExScalePartAction&>(rAct).GetDestSize());
            break;

        case MetaActionType::MASK:
            aActionBounds = tools::Rectangle(
                static_cast<const MetaMaskAction&>(rAct).GetPoint(),
                rOut.PixelToLogic(
                    static_cast<const MetaMaskAction&>(rAct).GetBitmap().GetSizePixel()));
            break;

        case MetaActionType::MASKSCALE:
            aActionBounds
                = tools::Rectangle(static_cast<const MetaMaskScaleAction&>(rAct).GetPoint(),
                                   static_cast<const MetaMaskScaleAction&>(rAct).GetSize());
            break;

        case MetaActionType::MASKSCALEPART:
            aActionBounds
                = tools::Rectangle(static_cast<const MetaMaskScalePartAction&>(rAct).GetDestPoint(),
                                   static_cast<const MetaMaskScalePartAction&>(rAct).GetDestSize());
            break;

        case MetaActionType::GRADIENT:
            aActionBounds = static_cast<const MetaGradientAction&>(rAct).GetRect();
            break;

        case MetaActionType::GRADIENTEX:
            aActionBounds
                = static_cast<const MetaGradientExAction&>(rAct).GetPolyPolygon().GetBoundRect();
            break;

        case MetaActionType::HATCH:
            aActionBounds
                = static_cast<const MetaHatchAction&>(rAct).GetPolyPolygon().GetBoundRect();
            break;

        case MetaActionType::WALLPAPER:
            aActionBounds = static_cast<const MetaWallpaperAction&>(rAct).GetRect();
            break;

        case MetaActionType::Transparent:
            aActionBounds
                = static_cast<const MetaTransparentAction&>(rAct).GetPolyPolygon().GetBoundRect();
            break;

        case MetaActionType::FLOATTRANSPARENT:
            aActionBounds
                = tools::Rectangle(static_cast<const MetaFloatTransparentAction&>(rAct).GetPoint(),
                                   static_cast<const MetaFloatTransparentAction&>(rAct).GetSize());
            break;

        case MetaActionType::EPS:
            aActionBounds = tools::Rectangle(static_cast<const MetaEPSAction&>(rAct).GetPoint(),
                                             static_cast<const MetaEPSAction&>(rAct).GetSize());
            break;

        case MetaActionType::TEXT:
        {
            const MetaTextAction& rTextAct = static_cast<const MetaTextAction&>(rAct);
            const OUString aString(rTextAct.GetText().copy(rTextAct.GetIndex(), rTextAct.GetLen()));

            if (!aString.isEmpty())
            {
                const Point aPtLog(rTextAct.GetPoint());

                // #105987# Use API method instead of Impl* methods
                // #107490# Set base parameter equal to index parameter
                rOut.GetTextBoundRect(aActionBounds, rTextAct.GetText(), rTextAct.GetIndex(),
                                      rTextAct.GetIndex(), rTextAct.GetLen());
                aActionBounds.Move(aPtLog.X(), aPtLog.Y());
            }
        }
        break;*/

        case MetaActionType::TEXTARRAY:
        {
            const MetaTextArrayAction& rTextAct = static_cast<const MetaTextArrayAction&>(rAct);
            const OUString aString(rTextAct.GetText().copy(rTextAct.GetIndex(), rTextAct.GetLen()));

            if (!aString.isEmpty())
            {
                // #105987# ImplLayout takes everything in logical coordinates
                std::unique_ptr<SalLayout> pSalLayout1 = rOut.ImplLayout(
                    aString, 0, nStrPos, rTextAct.GetPoint(), 0, rTextAct.GetDXArray());
                std::unique_ptr<SalLayout> pSalLayout2
                    = rOut.ImplLayout(aString, 0, nStrPos + sSubString.getLength(),
                                      rTextAct.GetPoint(), 0, rTextAct.GetDXArray());
                if (pSalLayout2)
                {
                    tools::Rectangle aBoundRect2(
                        const_cast<OutputDevice&>(rOut).ImplGetTextBoundRect(*pSalLayout2));
                    aActionBounds = rOut.PixelToLogic(aBoundRect2);
                }
                if (pSalLayout1 && nStrPos > 0)
                {
                    tools::Rectangle aBoundRect1(
                        const_cast<OutputDevice&>(rOut).ImplGetTextBoundRect(*pSalLayout1));
                    aActionBounds.SetLeft(rOut.PixelToLogic(aBoundRect1).getX()
                                          + rOut.PixelToLogic(aBoundRect1).getWidth());
                }

                // FIXME: Is this really needed?
                aActionBounds.SetTop(aActionBounds.getY() + 100);
            }
        }
        break;

            /*case MetaActionType::TEXTRECT:
            aActionBounds = static_cast<const MetaTextRectAction&>(rAct).GetRect();
            break;

        case MetaActionType::STRETCHTEXT:
        {
            const MetaStretchTextAction& rTextAct = static_cast<const MetaStretchTextAction&>(rAct);
            const OUString aString(rTextAct.GetText().copy(rTextAct.GetIndex(), rTextAct.GetLen()));

            // #i16195# Literate copy from TextArray action, the
            // semantics for the ImplLayout call are copied from the
            // OutDev::DrawStretchText() code. Unfortunately, also in
            // this case, public outdev methods such as GetTextWidth()
            // don't provide enough info.
            if (!aString.isEmpty())
            {
                // #105987# ImplLayout takes everything in logical coordinates
                std::unique_ptr<SalLayout> pSalLayout
                    = rOut.ImplLayout(rTextAct.GetText(), rTextAct.GetIndex(), rTextAct.GetLen(),
                                      rTextAct.GetPoint(), rTextAct.GetWidth());
                if (pSalLayout)
                {
                    tools::Rectangle aBoundRect(
                        const_cast<OutputDevice&>(rOut).ImplGetTextBoundRect(*pSalLayout));
                    aActionBounds = rOut.PixelToLogic(aBoundRect);
                }
            }
        }
        break;

        case MetaActionType::TEXTLINE:
            SAL_WARN("sfx.doc", "MetaActionType::TEXTLINE not supported");
            break;*/

        default:
            break;
    }

    if (!aActionBounds.IsEmpty())
    {
        // fdo#40421 limit current action's output to clipped area
        if (rOut.IsClipRegion())
            return rOut.GetClipRegion().GetBoundRect().Intersection(aActionBounds);
        else
            return aActionBounds;
    }
    else
        return tools::Rectangle(0, 0, 0, 0);
}

} // End of anon namespace

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

        aPageSizes.push_back(aLogic);

        Graphic aGraphic = aRenderer.renderToGraphic(nPage, aDocumentSizePixel, aDocumentSizePixel,
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
                                        const std::vector<::Size>& aPageSizes,
                                        const PageMargins& aPageMargins)
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

        // Set page size & margins
        uno::Reference<beans::XPropertySet> xPageProperySet(xPage, uno::UNO_QUERY);
        xPageProperySet->setPropertyValue("Height", css::uno::makeAny(nPageHeight));
        xPageProperySet->setPropertyValue("Width", css::uno::makeAny(nPageWidth));

        setPageMargins(xPageProperySet, aPageMargins);

        // Create and insert the shape
        uno::Reference<drawing::XShape> xShape(
            xFactory->createInstance("com.sun.star.drawing.GraphicObjectShape"), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xShapeProperySet(xShape, uno::UNO_QUERY);
        xShapeProperySet->setPropertyValue("Graphic", uno::Any(xGraph));
        xShapeProperySet->setPropertyValue("MoveProtect", uno::Any(true));
        xShapeProperySet->setPropertyValue("SizeProtect", uno::Any(true));

        // Set size
        xShape->setSize(
            awt::Size(rGDIMetaFile.GetPrefSize().Width(), rGDIMetaFile.GetPrefSize().Height()));

        xPage->add(xShape);

        // Search for the redaction strings, and get the rectangle coordinates
        std::vector<::tools::Rectangle> aRedactionRectangles;
        searchInMetaFile("deployment", rGDIMetaFile, aRedactionRectangles, xComponent);

        // Add the redaction rectangles to the page
        addRedactionRectToPage(xComponent, xPage, aRedactionRectangles);
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

PageMargins
SfxRedactionHelper::getPageMarginsForWriter(css::uno::Reference<css::frame::XModel>& xModel)
{
    PageMargins aPageMargins = { -1, -1, -1, -1 };

    Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(xModel->getCurrentController(),
                                                                     UNO_QUERY);
    if (!xTextViewCursorSupplier.is())
    {
        SAL_WARN("sfx.doc", "Ref to xTextViewCursorSupplier is null in setPageMargins().");
        return aPageMargins;
    }

    Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(), UNO_QUERY);

    uno::Reference<beans::XPropertySet> xPageProperySet(xCursor, UNO_QUERY);
    OUString sPageStyleName;
    Any aValue = xPageProperySet->getPropertyValue("PageStyleName");
    aValue >>= sPageStyleName;

    Reference<css::style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(xModel, UNO_QUERY);
    if (!xStyleFamiliesSupplier.is())
    {
        SAL_WARN("sfx.doc", "Ref to xStyleFamiliesSupplier is null in setPageMargins().");
        return aPageMargins;
    }
    uno::Reference<container::XNameAccess> xStyleFamilies(
        xStyleFamiliesSupplier->getStyleFamilies(), UNO_QUERY);

    if (!xStyleFamilies.is())
        return aPageMargins;

    uno::Reference<container::XNameAccess> xPageStyles(xStyleFamilies->getByName("PageStyles"),
                                                       UNO_QUERY);

    if (!xPageStyles.is())
        return aPageMargins;

    uno::Reference<css::style::XStyle> xPageStyle(xPageStyles->getByName(sPageStyleName),
                                                  UNO_QUERY);

    if (!xPageStyle.is())
        return aPageMargins;

    uno::Reference<beans::XPropertySet> xPageProperties(xPageStyle, uno::UNO_QUERY);

    if (!xPageProperties.is())
        return aPageMargins;

    xPageProperties->getPropertyValue("LeftMargin") >>= aPageMargins.nLeft;
    xPageProperties->getPropertyValue("RightMargin") >>= aPageMargins.nRight;
    xPageProperties->getPropertyValue("TopMargin") >>= aPageMargins.nTop;
    xPageProperties->getPropertyValue("BottomMargin") >>= aPageMargins.nBottom;

    return aPageMargins;
}

PageMargins
SfxRedactionHelper::getPageMarginsForCalc(css::uno::Reference<css::frame::XModel>& xModel)
{
    PageMargins aPageMargins = { -1, -1, -1, -1 };
    OUString sPageStyleName("Default");

    css::uno::Reference<css::sheet::XSpreadsheetView> xSpreadsheetView(
        xModel->getCurrentController(), UNO_QUERY);

    if (!xSpreadsheetView.is())
    {
        SAL_WARN("sfx.doc", "Ref to xSpreadsheetView is null in getPageMarginsForCalc().");
        return aPageMargins;
    }

    uno::Reference<beans::XPropertySet> xSheetProperties(xSpreadsheetView->getActiveSheet(),
                                                         UNO_QUERY);

    xSheetProperties->getPropertyValue("PageStyle") >>= sPageStyleName;

    Reference<css::style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(xModel, UNO_QUERY);
    if (!xStyleFamiliesSupplier.is())
    {
        SAL_WARN("sfx.doc", "Ref to xStyleFamiliesSupplier is null in getPageMarginsForCalc().");
        return aPageMargins;
    }
    uno::Reference<container::XNameAccess> xStyleFamilies(
        xStyleFamiliesSupplier->getStyleFamilies(), UNO_QUERY);

    if (!xStyleFamilies.is())
        return aPageMargins;

    uno::Reference<container::XNameAccess> xPageStyles(xStyleFamilies->getByName("PageStyles"),
                                                       UNO_QUERY);

    if (!xPageStyles.is())
        return aPageMargins;

    uno::Reference<css::style::XStyle> xPageStyle(xPageStyles->getByName(sPageStyleName),
                                                  UNO_QUERY);

    if (!xPageStyle.is())
        return aPageMargins;

    uno::Reference<beans::XPropertySet> xPageProperties(xPageStyle, uno::UNO_QUERY);

    if (!xPageProperties.is())
        return aPageMargins;

    xPageProperties->getPropertyValue("LeftMargin") >>= aPageMargins.nLeft;
    xPageProperties->getPropertyValue("RightMargin") >>= aPageMargins.nRight;
    xPageProperties->getPropertyValue("TopMargin") >>= aPageMargins.nTop;
    xPageProperties->getPropertyValue("BottomMargin") >>= aPageMargins.nBottom;

    return aPageMargins;
}

void SfxRedactionHelper::searchInMetaFile(const rtl::OUString& sSearchTerm, const GDIMetaFile& rMtf,
                                          std::vector<::tools::Rectangle>& aRedactionRectangles,
                                          uno::Reference<XComponent>& xComponent)
{
    MetaAction* pCurrAct;

    // Watch for TEXTARRAY actions.
    // They contain the text of paragraphes.
    for (pCurrAct = const_cast<GDIMetaFile&>(rMtf).FirstAction(); pCurrAct;
         pCurrAct = const_cast<GDIMetaFile&>(rMtf).NextAction())
    {
        if (pCurrAct->GetType() == MetaActionType::TEXTARRAY)
        {
            MetaTextArrayAction* pMetaTextArrayAction = static_cast<MetaTextArrayAction*>(pCurrAct);

            //sal_Int32 aIndex = pMetaTextArrayAction->GetIndex();
            //sal_Int32 aLength = pMetaTextArrayAction->GetLen();
            //Point aPoint = pMetaTextArrayAction->GetPoint();
            OUString sText = pMetaTextArrayAction->GetText();
            sal_Int32 nFoundIndex = sText.indexOf(sSearchTerm);

            // If found the string, add the corresponding rectangle to the collection
            if (nFoundIndex >= 0)
            {
                OutputDevice* pOutputDevice
                    = SfxObjectShell::GetShellFromComponent(xComponent)->GetDocumentRefDev();
                tools::Rectangle aNewRect(ImplCalcActionBounds(
                    *pMetaTextArrayAction, *pOutputDevice, sSearchTerm, nFoundIndex));

                aRedactionRectangles.push_back(aNewRect);
            }
        }
    }
}

void SfxRedactionHelper::addRedactionRectToPage(
    uno::Reference<XComponent>& xComponent, uno::Reference<drawing::XDrawPage>& xPage,
    const std::vector<::tools::Rectangle>& aNewRectangles)
{
    if (!xComponent.is() || !xPage.is())
        return;

    if (aNewRectangles.empty())
        return;

    uno::Reference<css::lang::XMultiServiceFactory> xFactory(xComponent, uno::UNO_QUERY);

    for (auto const& aNewRectangle : aNewRectangles)
    {
        uno::Reference<drawing::XShape> xRectShape(
            xFactory->createInstance("com.sun.star.drawing.RectangleShape"), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xRectShapeProperySet(xRectShape, uno::UNO_QUERY);

        xRectShapeProperySet->setPropertyValue("Name",
                                               uno::Any(OUString("RectangleRedactionShape")));
        xRectShapeProperySet->setPropertyValue("FillTransparence",
                                               css::uno::makeAny(static_cast<sal_Int16>(50)));
        xRectShapeProperySet->setPropertyValue("FillColor", css::uno::makeAny(COL_GRAY7));
        xRectShapeProperySet->setPropertyValue(
            "LineStyle", css::uno::makeAny(css::drawing::LineStyle::LineStyle_NONE));

        xRectShape->setSize(awt::Size(aNewRectangle.GetWidth(), aNewRectangle.GetHeight()));
        xRectShape->setPosition(awt::Point(aNewRectangle.getX(), aNewRectangle.getY()));

        xPage->add(xRectShape);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
