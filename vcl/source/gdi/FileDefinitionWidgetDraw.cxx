/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <FileDefinitionWidgetDraw.hxx>

#include <sal/config.h>
#include <svdata.hxx>
#include <rtl/bootstrap.hxx>
#include <config_folders.h>

#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include <tools/stream.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/BitmapTools.hxx>

#include <vcl/pngwrite.hxx>

#include <comphelper/seqstream.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/lok.hxx>

#include <com/sun/star/graphic/SvgTools.hpp>
#include <basegfx/DrawCommands.hxx>

using namespace css;

namespace vcl
{
namespace
{
OUString lcl_getThemeDefinitionPath()
{
    OUString sPath("$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/theme_definitions/");
    rtl::Bootstrap::expandMacros(sPath);
    return sPath;
}

std::shared_ptr<WidgetDefinition> setWidgetDefinition(OUString const& rDefinitionFile,
                                                      OUString const& rDefinitionResourcesPath)
{
    static std::shared_ptr<WidgetDefinition> spDefinition;
    if (!spDefinition)
    {
        spDefinition = std::make_shared<WidgetDefinition>();
        WidgetDefinitionReader aReader(rDefinitionFile, rDefinitionResourcesPath);
        aReader.read(*spDefinition);
    }
    return spDefinition;
}

} // end anonymous namespace

FileDefinitionWidgetDraw::FileDefinitionWidgetDraw(SalGraphics& rGraphics)
    : m_rGraphics(rGraphics)
{
    OUString sDefinitionBasePath = lcl_getThemeDefinitionPath();
    OUString sThemeName = "ios";
    OUString sThemeFolder = sDefinitionBasePath + sThemeName + "/";

    m_pWidgetDefinition = setWidgetDefinition(sThemeFolder + "definition.xml", sThemeFolder);

    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maNWFData.mbNoFocusRects = true;
    pSVData->maNWFData.mbNoFocusRectsForFlatButtons = true;
    pSVData->maNWFData.mbNoActiveTabTextRaise = true;
    pSVData->maNWFData.mbCenteredTabs = true;
    pSVData->maNWFData.mbProgressNeedsErase = true;
    pSVData->maNWFData.mnStatusBarLowerRightOffset = 10;
    pSVData->maNWFData.mbCanDrawWidgetAnySize = true;
}

bool FileDefinitionWidgetDraw::isNativeControlSupported(ControlType eType, ControlPart ePart)
{
    switch (eType)
    {
        case ControlType::Generic:
        case ControlType::Pushbutton:
        case ControlType::Radiobutton:
        case ControlType::Checkbox:
            return true;
        case ControlType::Combobox:
            if (ePart == ControlPart::HasBackgroundTexture)
                return false;
            return true;
        case ControlType::Editbox:
        case ControlType::EditboxNoBorder:
        case ControlType::MultilineEditbox:
            return true;
        case ControlType::Listbox:
            if (ePart == ControlPart::HasBackgroundTexture)
                return false;
            return true;
        case ControlType::Spinbox:
            if (ePart == ControlPart::AllButtons)
                return false;
            return true;
        case ControlType::SpinButtons:
            return false;
        case ControlType::TabItem:
        case ControlType::TabPane:
        case ControlType::TabHeader:
        case ControlType::TabBody:
            return true;
        case ControlType::Scrollbar:
            if (ePart == ControlPart::DrawBackgroundHorz
                || ePart == ControlPart::DrawBackgroundVert)
                return false;
            return true;
        case ControlType::Slider:
        case ControlType::Fixedline:
        case ControlType::Toolbar:
            return true;
        case ControlType::Menubar:
        case ControlType::MenuPopup:
            return true;
        case ControlType::Progress:
            return true;
        case ControlType::IntroProgress:
            return false;
        case ControlType::Tooltip:
            return true;
        case ControlType::WindowBackground:
        case ControlType::Frame:
        case ControlType::ListNode:
        case ControlType::ListNet:
        case ControlType::ListHeader:
            return true;
    }

    return false;
}

bool FileDefinitionWidgetDraw::hitTestNativeControl(
    ControlType /*eType*/, ControlPart /*ePart*/,
    const tools::Rectangle& /*rBoundingControlRegion*/, const Point& /*aPos*/, bool& /*rIsInside*/)
{
    return false;
}

namespace
{
void drawFromDrawCommands(gfx::DrawRoot const& rDrawRoot, SalGraphics& rGraphics, long nX, long nY,
                          long nWidth, long nHeight)
{
    basegfx::B2DRectangle aSVGRect = rDrawRoot.maRectangle;

    basegfx::B2DRange aTargetSurface(nX, nY, nX + nWidth + 1, nY + nHeight + 1);

    for (std::shared_ptr<gfx::DrawBase> const& pDrawBase : rDrawRoot.maChildren)
    {
        switch (pDrawBase->getType())
        {
            case gfx::DrawCommandType::Rectangle:
            {
                auto const& rRectangle = static_cast<gfx::DrawRectangle const&>(*pDrawBase);

                basegfx::B2DRange aInputRectangle(rRectangle.maRectangle);

                double fDeltaX = aTargetSurface.getWidth() - aSVGRect.getWidth();
                double fDeltaY = aTargetSurface.getHeight() - aSVGRect.getHeight();

                basegfx::B2DRange aFinalRectangle(
                    aInputRectangle.getMinX(), aInputRectangle.getMinY(),
                    aInputRectangle.getMaxX() + fDeltaX, aInputRectangle.getMaxY() + fDeltaY);

                aFinalRectangle.transform(basegfx::utils::createTranslateB2DHomMatrix(
                    aTargetSurface.getMinX() - 0.5, aTargetSurface.getMinY() - 0.5));

                basegfx::B2DPolygon aB2DPolygon = basegfx::utils::createPolygonFromRect(
                    aFinalRectangle, rRectangle.mnRx / aFinalRectangle.getWidth() * 2.0,
                    rRectangle.mnRy / aFinalRectangle.getHeight() * 2.0);

                if (rRectangle.mpFillColor)
                {
                    rGraphics.SetLineColor();
                    rGraphics.SetFillColor(Color(*rRectangle.mpFillColor));
                    rGraphics.DrawPolyPolygon(basegfx::B2DHomMatrix(),
                                              basegfx::B2DPolyPolygon(aB2DPolygon), 0.0f, nullptr);
                }
                if (rRectangle.mpStrokeColor)
                {
                    rGraphics.SetLineColor(Color(*rRectangle.mpStrokeColor));
                    rGraphics.SetFillColor();
                    rGraphics.DrawPolyLine(
                        basegfx::B2DHomMatrix(), aB2DPolygon, 0.0f,
                        basegfx::B2DVector(rRectangle.mnStrokeWidth, rRectangle.mnStrokeWidth),
                        basegfx::B2DLineJoin::Round, css::drawing::LineCap_ROUND, 0.0f, false,
                        nullptr);
                }
            }
            break;
            case gfx::DrawCommandType::Path:
            {
                auto const& rPath = static_cast<gfx::DrawPath const&>(*pDrawBase);

                double fDeltaX = aTargetSurface.getWidth() - aSVGRect.getWidth();
                double fDeltaY = aTargetSurface.getHeight() - aSVGRect.getHeight();

                basegfx::B2DPolyPolygon aPolyPolygon(rPath.maPolyPolygon);
                for (auto& rPolygon : aPolyPolygon)
                {
                    for (size_t i = 0; i < rPolygon.count(); ++i)
                    {
                        auto& rPoint = rPolygon.getB2DPoint(i);
                        double x = rPoint.getX();
                        double y = rPoint.getY();

                        if (x > aSVGRect.getCenterX())
                            x = x + fDeltaX;
                        if (y > aSVGRect.getCenterY())
                            y = y + fDeltaY;
                        rPolygon.setB2DPoint(i, basegfx::B2DPoint(x, y));
                    }
                }
                aPolyPolygon.transform(basegfx::utils::createTranslateB2DHomMatrix(
                    aTargetSurface.getMinX() - 0.5, aTargetSurface.getMinY() - 0.5));

                if (rPath.mpFillColor)
                {
                    rGraphics.SetLineColor();
                    rGraphics.SetFillColor(Color(*rPath.mpFillColor));
                    rGraphics.DrawPolyPolygon(basegfx::B2DHomMatrix(), aPolyPolygon, 0.0f, nullptr);
                }
                if (rPath.mpStrokeColor)
                {
                    rGraphics.SetLineColor(Color(*rPath.mpStrokeColor));
                    rGraphics.SetFillColor();
                    for (auto const& rPolygon : aPolyPolygon)
                    {
                        rGraphics.DrawPolyLine(
                            basegfx::B2DHomMatrix(), rPolygon, 0.0f,
                            basegfx::B2DVector(rPath.mnStrokeWidth, rPath.mnStrokeWidth),
                            basegfx::B2DLineJoin::Round, css::drawing::LineCap_ROUND, 0.0f, false,
                            nullptr);
                    }
                }
            }
            break;

            default:
                break;
        }
    }
}

void munchDrawCommands(std::vector<std::shared_ptr<DrawCommand>> const& rDrawCommands,
                       SalGraphics& rGraphics, long nX, long nY, long nWidth, long nHeight)
{
    for (std::shared_ptr<DrawCommand> const& pDrawCommand : rDrawCommands)
    {
        switch (pDrawCommand->maType)
        {
            case DrawCommandType::RECTANGLE:
            {
                auto const& rRectDrawCommand
                    = static_cast<RectangleDrawCommand const&>(*pDrawCommand);

                basegfx::B2DRectangle rRect(
                    nX + (nWidth * rRectDrawCommand.mfX1), nY + (nHeight * rRectDrawCommand.mfY1),
                    nX + (nWidth * rRectDrawCommand.mfX2), nY + (nHeight * rRectDrawCommand.mfY2));

                basegfx::B2DPolygon aB2DPolygon = basegfx::utils::createPolygonFromRect(
                    rRect, rRectDrawCommand.mnRx / rRect.getWidth() * 2.0,
                    rRectDrawCommand.mnRy / rRect.getHeight() * 2.0);

                rGraphics.SetLineColor();
                rGraphics.SetFillColor(rRectDrawCommand.maFillColor);
                rGraphics.DrawPolyPolygon(basegfx::B2DHomMatrix(),
                                          basegfx::B2DPolyPolygon(aB2DPolygon), 0.0f, nullptr);
                rGraphics.SetLineColor(rRectDrawCommand.maStrokeColor);
                rGraphics.SetFillColor();
                rGraphics.DrawPolyLine(basegfx::B2DHomMatrix(), aB2DPolygon, 0.0f,
                                       basegfx::B2DVector(rRectDrawCommand.mnStrokeWidth,
                                                          rRectDrawCommand.mnStrokeWidth),
                                       basegfx::B2DLineJoin::Round, css::drawing::LineCap_ROUND,
                                       0.0f, false, nullptr);
            }
            break;
            case DrawCommandType::CIRCLE:
            {
                auto const& rCircleDrawCommand
                    = static_cast<CircleDrawCommand const&>(*pDrawCommand);

                basegfx::B2DRectangle rRect(nX + (nWidth * rCircleDrawCommand.mfX1),
                                            nY + (nHeight * rCircleDrawCommand.mfY1),
                                            nX + (nWidth * rCircleDrawCommand.mfX2),
                                            nY + (nHeight * rCircleDrawCommand.mfY2));

                basegfx::B2DPolygon aB2DPolygon = basegfx::utils::createPolygonFromEllipse(
                    rRect.getCenter(), rRect.getWidth() / 2.0, rRect.getHeight() / 2.0);

                rGraphics.SetLineColor(rCircleDrawCommand.maStrokeColor);
                rGraphics.SetFillColor(rCircleDrawCommand.maFillColor);
                rGraphics.DrawPolyPolygon(basegfx::B2DHomMatrix(),
                                          basegfx::B2DPolyPolygon(aB2DPolygon), 0.0f, nullptr);
            }
            break;
            case DrawCommandType::LINE:
            {
                auto const& rLineDrawCommand = static_cast<LineDrawCommand const&>(*pDrawCommand);
                Point aRectPoint(nX + 1, nY + 1);

                Size aRectSize(nWidth - 1, nHeight - 1);

                rGraphics.SetFillColor();
                rGraphics.SetLineColor(rLineDrawCommand.maStrokeColor);

                basegfx::B2DPolygon aB2DPolygon{
                    { aRectPoint.X() + (aRectSize.Width() * rLineDrawCommand.mfX1),
                      aRectPoint.Y() + (aRectSize.Height() * rLineDrawCommand.mfY1) },
                    { aRectPoint.X() + (aRectSize.Width() * rLineDrawCommand.mfX2),
                      aRectPoint.Y() + (aRectSize.Height() * rLineDrawCommand.mfY2) },
                };

                rGraphics.DrawPolyLine(basegfx::B2DHomMatrix(), aB2DPolygon, 0.0f,
                                       basegfx::B2DVector(rLineDrawCommand.mnStrokeWidth,
                                                          rLineDrawCommand.mnStrokeWidth),
                                       basegfx::B2DLineJoin::Round, css::drawing::LineCap_ROUND,
                                       0.0f, false, nullptr);
            }
            break;
            case DrawCommandType::IMAGE:
            {
                double nScaleFactor = 1.0;
                if (comphelper::LibreOfficeKit::isActive())
                    nScaleFactor = comphelper::LibreOfficeKit::getDPIScale();

                auto const& rDrawCommand = static_cast<ImageDrawCommand const&>(*pDrawCommand);
                auto& rCacheImages = ImplGetSVData()->maGDIData.maThemeImageCache;
                OUString rCacheKey = rDrawCommand.msSource + "@" + OUString::number(nScaleFactor);
                auto& aIterator = rCacheImages.find(rCacheKey);

                BitmapEx aBitmap;
                if (aIterator == rCacheImages.end())
                {
                    SvFileStream aFileStream(rDrawCommand.msSource, StreamMode::READ);

                    vcl::bitmap::loadFromSvg(aFileStream, "", aBitmap, nScaleFactor);
                    if (!!aBitmap)
                    {
                        rCacheImages.insert(std::make_pair(rCacheKey, aBitmap));
                    }
                }
                else
                {
                    aBitmap = aIterator->second;
                }

                long nImageWidth = aBitmap.GetSizePixel().Width();
                long nImageHeight = aBitmap.GetSizePixel().Height();
                SalTwoRect aTR(0, 0, nImageWidth, nImageHeight, nX, nY, nImageWidth / nScaleFactor,
                               nImageHeight / nScaleFactor);
                if (!!aBitmap)
                {
                    const std::shared_ptr<SalBitmap> pSalBitmap
                        = aBitmap.GetBitmap().ImplGetSalBitmap();
                    if (aBitmap.IsAlpha())
                    {
                        const std::shared_ptr<SalBitmap> pSalBitmapAlpha
                            = aBitmap.GetAlpha().ImplGetSalBitmap();
                        rGraphics.DrawBitmap(aTR, *pSalBitmap, *pSalBitmapAlpha, nullptr);
                    }
                    else
                    {
                        rGraphics.DrawBitmap(aTR, *pSalBitmap, nullptr);
                    }
                }
            }
            break;
            case DrawCommandType::EXTERNAL:
            {
                auto const& rDrawCommand = static_cast<ImageDrawCommand const&>(*pDrawCommand);

                auto& rCacheDrawCommands = ImplGetSVData()->maGDIData.maThemeDrawCommandsCache;

                auto& aIterator = rCacheDrawCommands.find(rDrawCommand.msSource);

                gfx::DrawRoot aDrawRoot;

                if (aIterator == rCacheDrawCommands.end())
                {
                    SvFileStream aFileStream(rDrawCommand.msSource, StreamMode::READ);

                    uno::Reference<uno::XComponentContext> xContext(
                        comphelper::getProcessComponentContext());
                    const uno::Reference<graphic::XSvgParser> xSvgParser
                        = graphic::SvgTools::create(xContext);

                    std::size_t nSize = aFileStream.remainingSize();
                    std::vector<sal_Int8> aBuffer(nSize + 1);
                    aFileStream.ReadBytes(aBuffer.data(), nSize);
                    aBuffer[nSize] = 0;

                    uno::Sequence<sal_Int8> aData(aBuffer.data(), nSize + 1);
                    uno::Reference<io::XInputStream> aInputStream(
                        new comphelper::SequenceInputStream(aData));

                    uno::Any aAny = xSvgParser->getDrawCommands(aInputStream, "");
                    if (aAny.has<sal_uInt64>())
                    {
                        auto* pDrawRoot = reinterpret_cast<gfx::DrawRoot*>(aAny.get<sal_uInt64>());
                        if (pDrawRoot)
                        {
                            rCacheDrawCommands.insert(
                                std::make_pair(rDrawCommand.msSource, *pDrawRoot));
                            drawFromDrawCommands(*pDrawRoot, rGraphics, nX, nY, nWidth, nHeight);
                        }
                    }
                }
                else
                {
                    drawFromDrawCommands(aIterator->second, rGraphics, nX, nY, nWidth, nHeight);
                }
            }
            break;
        }
    }
}

} // end anonymous namespace

bool FileDefinitionWidgetDraw::resolveDefinition(ControlType eType, ControlPart ePart,
                                                 ControlState eState,
                                                 const ImplControlValue& rValue, long nX, long nY,
                                                 long nWidth, long nHeight)
{
    bool bOK = false;
    auto const& pPart = m_pWidgetDefinition->getDefinition(eType, ePart);
    if (pPart)
    {
        auto const& aStates = pPart->getStates(eType, ePart, eState, rValue);
        if (!aStates.empty())
        {
            // use last defined state
            auto const& pState = aStates.back();
            {
                munchDrawCommands(pState->mpDrawCommands, m_rGraphics, nX, nY, nWidth, nHeight);
                bOK = true;
            }
        }
    }
    return bOK;
}

bool FileDefinitionWidgetDraw::drawNativeControl(ControlType eType, ControlPart ePart,
                                                 const tools::Rectangle& rControlRegion,
                                                 ControlState eState,
                                                 const ImplControlValue& rValue,
                                                 const OUString& /*aCaptions*/)
{
    bool bOldAA = m_rGraphics.getAntiAliasB2DDraw();
    m_rGraphics.setAntiAliasB2DDraw(true);

    long nWidth = rControlRegion.GetWidth() - 1;
    long nHeight = rControlRegion.GetHeight() - 1;
    long nX = rControlRegion.Left();
    long nY = rControlRegion.Top();

    bool bOK = false;

    switch (eType)
    {
        case ControlType::Pushbutton:
        {
            /*bool bIsAction = false;
            const PushButtonValue* pPushButtonValue = static_cast<const PushButtonValue*>(&rValue);
            if (pPushButtonValue)
                bIsAction = pPushButtonValue->mbIsAction;*/

            bOK = resolveDefinition(eType, ePart, eState, rValue, nX, nY, nWidth, nHeight);
        }
        break;
        case ControlType::Radiobutton:
        {
            bOK = resolveDefinition(eType, ePart, eState, rValue, nX, nY, nWidth, nHeight);
        }
        break;
        case ControlType::Checkbox:
        {
            bOK = resolveDefinition(eType, ePart, eState, rValue, nX, nY, nWidth, nHeight);
        }
        break;
        case ControlType::Combobox:
        {
            bOK = resolveDefinition(eType, ePart, eState, rValue, nX, nY, nWidth, nHeight);
        }
        break;
        case ControlType::Editbox:
        case ControlType::EditboxNoBorder:
        case ControlType::MultilineEditbox:
        {
            bOK = resolveDefinition(eType, ePart, eState, rValue, nX, nY, nWidth, nHeight);
        }
        break;
        case ControlType::Listbox:
        {
            bOK = resolveDefinition(eType, ePart, eState, rValue, nX, nY, nWidth, nHeight);
        }
        break;
        case ControlType::Spinbox:
        {
            if (rValue.getType() == ControlType::SpinButtons)
            {
                const SpinbuttonValue* pSpinVal = static_cast<const SpinbuttonValue*>(&rValue);

                {
                    ControlPart eUpButtonPart = pSpinVal->mnUpperPart;
                    ControlState eUpButtonState = pSpinVal->mnUpperState;

                    long nUpperX = pSpinVal->maUpperRect.Left();
                    long nUpperY = pSpinVal->maUpperRect.Top();
                    long nUpperWidth = pSpinVal->maUpperRect.GetWidth() - 1;
                    long nUpperHeight = pSpinVal->maUpperRect.GetHeight() - 1;

                    bOK = resolveDefinition(eType, eUpButtonPart, eUpButtonState,
                                            ImplControlValue(), nUpperX, nUpperY, nUpperWidth,
                                            nUpperHeight);
                }

                if (bOK)
                {
                    ControlPart eDownButtonPart = pSpinVal->mnLowerPart;
                    ControlState eDownButtonState = pSpinVal->mnLowerState;

                    long nLowerX = pSpinVal->maLowerRect.Left();
                    long nLowerY = pSpinVal->maLowerRect.Top();
                    long nLowerWidth = pSpinVal->maLowerRect.GetWidth() - 1;
                    long nLowerHeight = pSpinVal->maLowerRect.GetHeight() - 1;

                    bOK = resolveDefinition(eType, eDownButtonPart, eDownButtonState,
                                            ImplControlValue(), nLowerX, nLowerY, nLowerWidth,
                                            nLowerHeight);
                }
            }
            else
            {
                bOK = resolveDefinition(eType, ePart, eState, rValue, nX, nY, nWidth, nHeight);
            }
        }
        break;
        case ControlType::SpinButtons:
            break;
        case ControlType::TabItem:
        case ControlType::TabHeader:
        case ControlType::TabPane:
        case ControlType::TabBody:
        {
            bOK = resolveDefinition(eType, ePart, eState, rValue, nX, nY, nWidth, nHeight);
        }
        break;
        case ControlType::Scrollbar:
        {
            bOK = resolveDefinition(eType, ePart, eState, rValue, nX, nY, nWidth, nHeight);
        }
        break;
        case ControlType::Slider:
        {
            const SliderValue* pSliderValue = static_cast<const SliderValue*>(&rValue);
            long nThumbX = pSliderValue->maThumbRect.Left();
            long nThumbY = pSliderValue->maThumbRect.Top();
            long nThumbWidth = pSliderValue->maThumbRect.GetWidth() - 1;
            long nThumbHeight = pSliderValue->maThumbRect.GetHeight() - 1;

            if (ePart == ControlPart::TrackHorzArea)
            {
                long nCenterX = nThumbX + nThumbWidth / 2;

                bOK = resolveDefinition(eType, ControlPart::TrackHorzLeft, eState, rValue, nX, nY,
                                        nCenterX - nX, nHeight);
                if (bOK)
                    bOK = resolveDefinition(eType, ControlPart::TrackHorzRight, eState, rValue,
                                            nCenterX, nY, nX + nWidth - nCenterX, nHeight);
            }
            else if (ePart == ControlPart::TrackVertArea)
            {
                long nCenterY = nThumbY + nThumbHeight / 2;

                bOK = resolveDefinition(eType, ControlPart::TrackVertUpper, eState, rValue, nX, nY,
                                        nWidth, nCenterY - nY);
                if (bOK)
                    bOK = resolveDefinition(eType, ControlPart::TrackVertLower, eState, rValue, nY,
                                            nCenterY, nWidth, nY + nHeight - nCenterY);
            }

            if (bOK)
            {
                bOK = resolveDefinition(eType, ControlPart::Button,
                                        eState | pSliderValue->mnThumbState, rValue, nThumbX,
                                        nThumbY, nThumbWidth, nThumbHeight);
            }
        }
        break;
        case ControlType::Fixedline:
        {
            bOK = resolveDefinition(eType, ePart, eState, rValue, nX, nY, nWidth, nHeight);
        }
        break;
        case ControlType::Toolbar:
        {
            bOK = resolveDefinition(eType, ePart, eState, rValue, nX, nY, nWidth, nHeight);
        }
        break;
        case ControlType::Menubar:
        case ControlType::MenuPopup:
        {
            bOK = resolveDefinition(eType, ePart, eState, rValue, nX, nY, nWidth, nHeight);
        }
        break;
        case ControlType::Progress:
        {
            bOK = resolveDefinition(eType, ePart, eState, rValue, nX, nY, nWidth, nHeight);
        }
        break;
        case ControlType::IntroProgress:
            break;
        case ControlType::Tooltip:
        {
            bOK = resolveDefinition(eType, ePart, eState, rValue, nX, nY, nWidth, nHeight);
        }
        break;
        case ControlType::WindowBackground:
        case ControlType::Frame:
        {
            bOK = resolveDefinition(eType, ePart, eState, rValue, nX, nY, nWidth, nHeight);
        }
        break;
        case ControlType::ListNode:
        case ControlType::ListNet:
        case ControlType::ListHeader:
        {
            bOK = resolveDefinition(eType, ePart, eState, rValue, nX, nY, nWidth, nHeight);
        }
        break;
        default:
            break;
    }

    m_rGraphics.setAntiAliasB2DDraw(bOldAA);

    return bOK;
}

bool FileDefinitionWidgetDraw::getNativeControlRegion(
    ControlType eType, ControlPart ePart, const tools::Rectangle& rBoundingControlRegion,
    ControlState /*eState*/, const ImplControlValue& /*aValue*/, const OUString& /*aCaption*/,
    tools::Rectangle& rNativeBoundingRegion, tools::Rectangle& rNativeContentRegion)
{
    Point aLocation(rBoundingControlRegion.TopLeft());

    switch (eType)
    {
        case ControlType::Spinbox:
        {
            auto const& pButtonUpPart
                = m_pWidgetDefinition->getDefinition(eType, ControlPart::ButtonUp);
            if (!pButtonUpPart)
                return false;
            Size aButtonSizeUp(pButtonUpPart->mnWidth, pButtonUpPart->mnHeight);

            auto const& pButtonDownPart
                = m_pWidgetDefinition->getDefinition(eType, ControlPart::ButtonDown);
            if (!pButtonDownPart)
                return false;
            Size aButtonSizeDown(pButtonDownPart->mnWidth, pButtonDownPart->mnHeight);

            OString sOrientation = "decrease-edit-increase";

            if (sOrientation == "decrease-edit-increase")
            {
                if (ePart == ControlPart::ButtonUp)
                {
                    Point aPoint(aLocation.X() + rBoundingControlRegion.GetWidth()
                                     - aButtonSizeUp.Width(),
                                 aLocation.Y());
                    rNativeContentRegion = tools::Rectangle(aPoint, aButtonSizeUp);
                    rNativeBoundingRegion = rNativeContentRegion;
                    return true;
                }
                else if (ePart == ControlPart::ButtonDown)
                {
                    rNativeContentRegion = tools::Rectangle(aLocation, aButtonSizeDown);
                    rNativeBoundingRegion = rNativeContentRegion;
                    return true;
                }
                else if (ePart == ControlPart::SubEdit)
                {
                    Point aPoint(aLocation.X() + aButtonSizeDown.Width(), aLocation.Y());
                    Size aSize(rBoundingControlRegion.GetWidth()
                                   - (aButtonSizeDown.Width() + aButtonSizeUp.Width()),
                               std::max(aButtonSizeUp.Height(), aButtonSizeDown.Height()));
                    rNativeContentRegion = tools::Rectangle(aPoint, aSize);
                    rNativeBoundingRegion = rNativeContentRegion;
                    return true;
                }
                else if (ePart == ControlPart::Entire)
                {
                    Size aSize(rBoundingControlRegion.GetWidth(),
                               std::max(aButtonSizeUp.Height(), aButtonSizeDown.Height()));
                    rNativeContentRegion = tools::Rectangle(aLocation, aSize);
                    rNativeBoundingRegion = rNativeContentRegion;
                    return true;
                }
            }
            else
            {
                if (ePart == ControlPart::ButtonUp)
                {
                    Point aPoint(aLocation.X() + rBoundingControlRegion.GetWidth()
                                     - aButtonSizeUp.Width(),
                                 aLocation.Y());
                    rNativeContentRegion = tools::Rectangle(aPoint, aButtonSizeUp);
                    rNativeBoundingRegion = rNativeContentRegion;
                    return true;
                }
                else if (ePart == ControlPart::ButtonDown)
                {
                    Point aPoint(aLocation.X() + rBoundingControlRegion.GetWidth()
                                     - (aButtonSizeDown.Width() + aButtonSizeUp.Width()),
                                 aLocation.Y());
                    rNativeContentRegion = tools::Rectangle(aPoint, aButtonSizeDown);
                    rNativeBoundingRegion = rNativeContentRegion;
                    return true;
                }
                else if (ePart == ControlPart::SubEdit)
                {
                    Size aSize(rBoundingControlRegion.GetWidth()
                                   - (aButtonSizeDown.Width() + aButtonSizeUp.Width()),
                               std::max(aButtonSizeUp.Height(), aButtonSizeDown.Height()));
                    rNativeContentRegion = tools::Rectangle(aLocation, aSize);
                    rNativeBoundingRegion = rNativeContentRegion;
                    return true;
                }
                else if (ePart == ControlPart::Entire)
                {
                    Size aSize(rBoundingControlRegion.GetWidth(),
                               std::max(aButtonSizeUp.Height(), aButtonSizeDown.Height()));
                    rNativeContentRegion = tools::Rectangle(aLocation, aSize);
                    rNativeBoundingRegion = rNativeContentRegion;
                    return true;
                }
            }
        }
        break;
        case ControlType::Checkbox:
        {
            auto const& pPart = m_pWidgetDefinition->getDefinition(eType, ControlPart::Entire);
            if (!pPart)
                return false;

            Size aSize(pPart->mnWidth, pPart->mnHeight);
            rNativeContentRegion = tools::Rectangle(Point(), aSize);
            return true;
        }
        case ControlType::Radiobutton:
        {
            auto const& pPart = m_pWidgetDefinition->getDefinition(eType, ControlPart::Entire);
            if (!pPart)
                return false;

            Size aSize(pPart->mnWidth, pPart->mnHeight);
            rNativeContentRegion = tools::Rectangle(Point(), aSize);
            return true;
        }
        case ControlType::TabItem:
        {
            auto const& pPart = m_pWidgetDefinition->getDefinition(eType, ControlPart::Entire);
            if (!pPart)
                return false;

            long nWidth = std::max(rBoundingControlRegion.GetWidth() + pPart->mnMarginWidth,
                                   long(pPart->mnWidth));
            long nHeight = std::max(rBoundingControlRegion.GetHeight() + pPart->mnMarginHeight,
                                    long(pPart->mnHeight));

            rNativeBoundingRegion = tools::Rectangle(aLocation, Size(nWidth, nHeight));
            rNativeContentRegion = rNativeBoundingRegion;
            return true;
        }
        case ControlType::Editbox:
        case ControlType::EditboxNoBorder:
        case ControlType::MultilineEditbox:
        {
            sal_Int32 nHeight = rBoundingControlRegion.GetHeight();

            auto const& pPart = m_pWidgetDefinition->getDefinition(eType, ControlPart::Entire);
            if (pPart)
                nHeight = std::max(nHeight, pPart->mnHeight);

            Size aSize(rBoundingControlRegion.GetWidth(), nHeight);
            rNativeContentRegion = tools::Rectangle(aLocation, aSize);
            rNativeBoundingRegion = rNativeContentRegion;
            rNativeBoundingRegion.expand(2);
            return true;
        }
        break;
        case ControlType::Scrollbar:
        {
            if (ePart == ControlPart::ButtonUp || ePart == ControlPart::ButtonDown
                || ePart == ControlPart::ButtonLeft || ePart == ControlPart::ButtonRight)
            {
                rNativeContentRegion = tools::Rectangle(aLocation, Size(0, 0));
                rNativeBoundingRegion = rNativeContentRegion;
                return true;
            }
            else
            {
                rNativeBoundingRegion = rBoundingControlRegion;
                rNativeContentRegion = rNativeBoundingRegion;
                return true;
            }
        }
        break;
        case ControlType::Combobox:
        case ControlType::Listbox:
        {
            auto const& pPart = m_pWidgetDefinition->getDefinition(eType, ControlPart::ButtonDown);
            Size aComboButtonSize(pPart->mnWidth, pPart->mnHeight);

            if (ePart == ControlPart::ButtonDown)
            {
                Point aPoint(aLocation.X() + rBoundingControlRegion.GetWidth()
                                 - aComboButtonSize.Width(),
                             aLocation.Y() + 1);
                rNativeContentRegion = tools::Rectangle(aPoint, aComboButtonSize);
                rNativeBoundingRegion = rNativeContentRegion;
                return true;
            }
            else if (ePart == ControlPart::SubEdit)
            {
                Size aSize(rBoundingControlRegion.GetWidth() - aComboButtonSize.Width(),
                           aComboButtonSize.Height());
                rNativeContentRegion = tools::Rectangle(aLocation + Point(1, 1), aSize);
                rNativeBoundingRegion = rNativeContentRegion;
                return true;
            }
            else if (ePart == ControlPart::Entire)
            {
                Size aSize(rBoundingControlRegion.GetWidth(), aComboButtonSize.Height());
                rNativeContentRegion = tools::Rectangle(aLocation, aSize);
                rNativeBoundingRegion = rNativeContentRegion;
                rNativeBoundingRegion.expand(1);
                return true;
            }
        }
        break;
        case ControlType::Slider:
            if (ePart == ControlPart::ThumbHorz || ePart == ControlPart::ThumbVert)
            {
                rNativeContentRegion = tools::Rectangle(aLocation, Size(28, 28));
                rNativeBoundingRegion = rNativeContentRegion;
                return true;
            }
            break;
        default:
            break;
    }

    return false;
}

bool FileDefinitionWidgetDraw::updateSettings(AllSettings& rSettings)
{
    StyleSettings aStyleSet = rSettings.GetStyleSettings();

    auto pDefinitionStyle = m_pWidgetDefinition->mpStyle;

    aStyleSet.SetFaceColor(pDefinitionStyle->maFaceColor);
    aStyleSet.SetCheckedColor(pDefinitionStyle->maCheckedColor);
    aStyleSet.SetLightColor(pDefinitionStyle->maLightColor);
    aStyleSet.SetLightBorderColor(pDefinitionStyle->maLightBorderColor);
    aStyleSet.SetShadowColor(pDefinitionStyle->maShadowColor);
    aStyleSet.SetDarkShadowColor(pDefinitionStyle->maDarkShadowColor);
    aStyleSet.SetButtonTextColor(pDefinitionStyle->maButtonTextColor);
    aStyleSet.SetButtonRolloverTextColor(pDefinitionStyle->maButtonRolloverTextColor);
    aStyleSet.SetRadioCheckTextColor(pDefinitionStyle->maRadioCheckTextColor);
    aStyleSet.SetGroupTextColor(pDefinitionStyle->maGroupTextColor);
    aStyleSet.SetLabelTextColor(pDefinitionStyle->maLabelTextColor);
    aStyleSet.SetWindowColor(pDefinitionStyle->maWindowColor);
    aStyleSet.SetWindowTextColor(pDefinitionStyle->maWindowTextColor);
    aStyleSet.SetDialogColor(pDefinitionStyle->maDialogColor);
    aStyleSet.SetDialogTextColor(pDefinitionStyle->maDialogTextColor);
    aStyleSet.SetWorkspaceColor(pDefinitionStyle->maWorkspaceColor);
    aStyleSet.SetMonoColor(pDefinitionStyle->maMonoColor);
    aStyleSet.SetFieldColor(pDefinitionStyle->maFieldColor);
    aStyleSet.SetFieldTextColor(pDefinitionStyle->maFieldTextColor);
    aStyleSet.SetFieldRolloverTextColor(pDefinitionStyle->maFieldRolloverTextColor);
    aStyleSet.SetActiveColor(pDefinitionStyle->maActiveColor);
    aStyleSet.SetActiveTextColor(pDefinitionStyle->maActiveTextColor);
    aStyleSet.SetActiveBorderColor(pDefinitionStyle->maActiveBorderColor);
    aStyleSet.SetDeactiveColor(pDefinitionStyle->maDeactiveColor);
    aStyleSet.SetDeactiveTextColor(pDefinitionStyle->maDeactiveTextColor);
    aStyleSet.SetDeactiveBorderColor(pDefinitionStyle->maDeactiveBorderColor);
    aStyleSet.SetMenuColor(pDefinitionStyle->maMenuColor);
    aStyleSet.SetMenuBarColor(pDefinitionStyle->maMenuBarColor);
    aStyleSet.SetMenuBarRolloverColor(pDefinitionStyle->maMenuBarRolloverColor);
    aStyleSet.SetMenuBorderColor(pDefinitionStyle->maMenuBorderColor);
    aStyleSet.SetMenuTextColor(pDefinitionStyle->maMenuTextColor);
    aStyleSet.SetMenuBarTextColor(pDefinitionStyle->maMenuBarTextColor);
    aStyleSet.SetMenuBarRolloverTextColor(pDefinitionStyle->maMenuBarRolloverTextColor);
    aStyleSet.SetMenuBarHighlightTextColor(pDefinitionStyle->maMenuBarHighlightTextColor);
    aStyleSet.SetMenuHighlightColor(pDefinitionStyle->maMenuHighlightColor);
    aStyleSet.SetMenuHighlightTextColor(pDefinitionStyle->maMenuHighlightTextColor);
    aStyleSet.SetHighlightColor(pDefinitionStyle->maHighlightColor);
    aStyleSet.SetHighlightTextColor(pDefinitionStyle->maHighlightTextColor);
    aStyleSet.SetActiveTabColor(pDefinitionStyle->maActiveTabColor);
    aStyleSet.SetInactiveTabColor(pDefinitionStyle->maInactiveTabColor);
    aStyleSet.SetTabTextColor(pDefinitionStyle->maTabTextColor);
    aStyleSet.SetTabRolloverTextColor(pDefinitionStyle->maTabRolloverTextColor);
    aStyleSet.SetTabHighlightTextColor(pDefinitionStyle->maTabHighlightTextColor);
    aStyleSet.SetDisableColor(pDefinitionStyle->maDisableColor);
    aStyleSet.SetHelpColor(pDefinitionStyle->maHelpColor);
    aStyleSet.SetHelpTextColor(pDefinitionStyle->maHelpTextColor);
    aStyleSet.SetLinkColor(pDefinitionStyle->maLinkColor);
    aStyleSet.SetVisitedLinkColor(pDefinitionStyle->maVisitedLinkColor);
    aStyleSet.SetToolTextColor(pDefinitionStyle->maToolTextColor);
    aStyleSet.SetFontColor(pDefinitionStyle->maFontColor);

    vcl::Font aFont(FAMILY_SWISS, Size(0, 10));
    aFont.SetCharSet(osl_getThreadTextEncoding());
    aFont.SetWeight(WEIGHT_NORMAL);
    aFont.SetFamilyName("Liberation Sans");
    aStyleSet.SetAppFont(aFont);
    aStyleSet.SetHelpFont(aFont);
    aStyleSet.SetMenuFont(aFont);
    aStyleSet.SetToolFont(aFont);
    aStyleSet.SetGroupFont(aFont);
    aStyleSet.SetLabelFont(aFont);
    aStyleSet.SetRadioCheckFont(aFont);
    aStyleSet.SetPushButtonFont(aFont);
    aStyleSet.SetFieldFont(aFont);
    aStyleSet.SetIconFont(aFont);
    aStyleSet.SetTabFont(aFont);

    aFont.SetWeight(WEIGHT_BOLD);
    aStyleSet.SetFloatTitleFont(aFont);
    aStyleSet.SetTitleFont(aFont);

    aStyleSet.SetTitleHeight(16);
    aStyleSet.SetFloatTitleHeight(12);

    rSettings.SetStyleSettings(aStyleSet);

    return true;
}

} // end vcl namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
