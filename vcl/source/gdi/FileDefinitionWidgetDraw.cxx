/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sal/config.h>

#include <string_view>

#include <FileDefinitionWidgetDraw.hxx>
#include <widgetdraw/WidgetDefinitionReader.hxx>

#include <svdata.hxx>
#include <rtl/bootstrap.hxx>
#include <config_folders.h>
#include <osl/file.hxx>

#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/tuple/b2dtuple.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#include <tools/stream.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/BitmapTools.hxx>
#include <vcl/gradient.hxx>

#include <comphelper/seqstream.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/string.hxx>

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

bool lcl_directoryExists(OUString const& sDirectory)
{
    osl::DirectoryItem aDirectoryItem;
    osl::FileBase::RC eRes = osl::DirectoryItem::get(sDirectory, aDirectoryItem);
    return eRes == osl::FileBase::E_None;
}

bool lcl_fileExists(OUString const& sFilename)
{
    osl::File aFile(sFilename);
    osl::FileBase::RC eRC = aFile.open(osl_File_OpenFlag_Read);
    return osl::FileBase::E_None == eRC;
}

std::shared_ptr<WidgetDefinition> getWidgetDefinition(OUString const& rDefinitionFile,
                                                      OUString const& rDefinitionResourcesPath)
{
    auto pWidgetDefinition = std::make_shared<WidgetDefinition>();
    WidgetDefinitionReader aReader(rDefinitionFile, rDefinitionResourcesPath);
    if (aReader.read(*pWidgetDefinition))
        return pWidgetDefinition;
    return std::shared_ptr<WidgetDefinition>();
}

std::shared_ptr<WidgetDefinition> const&
getWidgetDefinitionForTheme(std::u16string_view rThemenName)
{
    static std::shared_ptr<WidgetDefinition> spDefinition;
    if (!spDefinition)
    {
        OUString sSharedDefinitionBasePath = lcl_getThemeDefinitionPath();
        OUString sThemeFolder = sSharedDefinitionBasePath + rThemenName + "/";
        OUString sThemeDefinitionFile = sThemeFolder + "definition.xml";
        if (lcl_directoryExists(sThemeFolder) && lcl_fileExists(sThemeDefinitionFile))
            spDefinition = getWidgetDefinition(sThemeDefinitionFile, sThemeFolder);
    }
    return spDefinition;
}

int getSettingValueInteger(OString const& rValue, int nDefault)
{
    if (rValue.isEmpty())
        return nDefault;
    if (!comphelper::string::isdigitAsciiString(rValue))
        return nDefault;
    return rValue.toInt32();
}

bool getSettingValueBool(std::string_view rValue, bool bDefault)
{
    if (rValue.empty())
        return bDefault;
    if (rValue == "true" || rValue == "false")
        return rValue == "true";
    return bDefault;
}

} // end anonymous namespace

FileDefinitionWidgetDraw::FileDefinitionWidgetDraw(SalGraphics& rGraphics)
    : m_rGraphics(rGraphics)
    , m_bIsActive(false)
{
    m_pWidgetDefinition = getWidgetDefinitionForTheme(u"online");
#ifdef IOS
    if (!m_pWidgetDefinition)
        m_pWidgetDefinition = getWidgetDefinitionForTheme(u"ios");
#endif

    if (!m_pWidgetDefinition)
        return;

    auto& pSettings = m_pWidgetDefinition->mpSettings;

    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maNWFData.mbNoFocusRects = true;
    pSVData->maNWFData.mbNoFocusRectsForFlatButtons = true;
    pSVData->maNWFData.mbNoActiveTabTextRaise
        = getSettingValueBool(pSettings->msNoActiveTabTextRaise, true);
    pSVData->maNWFData.mbCenteredTabs = getSettingValueBool(pSettings->msCenteredTabs, true);
    pSVData->maNWFData.mbProgressNeedsErase = true;
    pSVData->maNWFData.mnStatusBarLowerRightOffset = 10;
    pSVData->maNWFData.mbCanDrawWidgetAnySize = true;

    int nDefaultListboxEntryMargin = pSVData->maNWFData.mnListBoxEntryMargin;
    pSVData->maNWFData.mnListBoxEntryMargin
        = getSettingValueInteger(pSettings->msListBoxEntryMargin, nDefaultListboxEntryMargin);

    m_bIsActive = true;
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

void FileDefinitionWidgetDraw::drawPolyPolygon(SalGraphics& rGraphics,
                                               const basegfx::B2DHomMatrix& rObjectToDevice,
                                               const basegfx::B2DPolyPolygon& i_rPolyPolygon,
                                               double i_fTransparency)
{
    rGraphics.drawPolyPolygon(rObjectToDevice, i_rPolyPolygon, i_fTransparency);
}

void FileDefinitionWidgetDraw::drawPolyLine(
    SalGraphics& rGraphics, const basegfx::B2DHomMatrix& rObjectToDevice,
    const basegfx::B2DPolygon& i_rPolygon, double i_fTransparency, double i_fLineWidth,
    const std::vector<double>* i_pStroke, basegfx::B2DLineJoin i_eLineJoin,
    css::drawing::LineCap i_eLineCap, double i_fMiterMinimumAngle, bool bPixelSnapHairline)
{
    rGraphics.drawPolyLine(rObjectToDevice, i_rPolygon, i_fTransparency, i_fLineWidth, i_pStroke,
                           i_eLineJoin, i_eLineCap, i_fMiterMinimumAngle, bPixelSnapHairline);
}

void FileDefinitionWidgetDraw::drawBitmap(SalGraphics& rGraphics, const SalTwoRect& rPosAry,
                                          const SalBitmap& rSalBitmap)
{
    rGraphics.drawBitmap(rPosAry, rSalBitmap);
}

void FileDefinitionWidgetDraw::drawBitmap(SalGraphics& rGraphics, const SalTwoRect& rPosAry,
                                          const SalBitmap& rSalBitmap,
                                          const SalBitmap& rTransparentBitmap)
{
    rGraphics.drawBitmap(rPosAry, rSalBitmap, rTransparentBitmap);
}

void FileDefinitionWidgetDraw::implDrawGradient(SalGraphics& rGraphics,
                                                const basegfx::B2DPolyPolygon& rPolyPolygon,
                                                const SalGradient& rGradient)
{
    rGraphics.implDrawGradient(rPolyPolygon, rGradient);
}

namespace
{
void drawFromDrawCommands(gfx::DrawRoot const& rDrawRoot, SalGraphics& rGraphics, tools::Long nX,
                          tools::Long nY, tools::Long nWidth, tools::Long nHeight)
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
                    FileDefinitionWidgetDraw::drawPolyPolygon(rGraphics, basegfx::B2DHomMatrix(),
                                                              basegfx::B2DPolyPolygon(aB2DPolygon),
                                                              1.0 - rRectangle.mnOpacity);
                }
                else if (rRectangle.mpFillGradient)
                {
                    rGraphics.SetLineColor();
                    rGraphics.SetFillColor();
                    if (rRectangle.mpFillGradient->meType == gfx::GradientType::Linear)
                    {
                        auto* pLinearGradient = static_cast<gfx::LinearGradientInfo*>(
                            rRectangle.mpFillGradient.get());
                        SalGradient aGradient;
                        double x, y;

                        x = pLinearGradient->x1;
                        y = pLinearGradient->y1;

                        if (x > aSVGRect.getCenterX())
                            x = x + fDeltaX;
                        if (y > aSVGRect.getCenterY())
                            y = y + fDeltaY;

                        aGradient.maPoint1 = basegfx::B2DPoint(x, y);
                        aGradient.maPoint1 *= basegfx::utils::createTranslateB2DHomMatrix(
                            aTargetSurface.getMinX() - 0.5, aTargetSurface.getMinY() - 0.5);

                        x = pLinearGradient->x2;
                        y = pLinearGradient->y2;

                        if (x > aSVGRect.getCenterX())
                            x = x + fDeltaX;
                        if (y > aSVGRect.getCenterY())
                            y = y + fDeltaY;

                        aGradient.maPoint2 = basegfx::B2DPoint(x, y);
                        aGradient.maPoint2 *= basegfx::utils::createTranslateB2DHomMatrix(
                            aTargetSurface.getMinX() - 0.5, aTargetSurface.getMinY() - 0.5);

                        for (gfx::GradientStop const& rStop : pLinearGradient->maGradientStops)
                        {
                            Color aColor(rStop.maColor);
                            aColor.SetAlpha(255
                                            - (rStop.mfOpacity * (1.0f - rRectangle.mnOpacity)));
                            aGradient.maStops.emplace_back(aColor, rStop.mfOffset);
                        }
                        FileDefinitionWidgetDraw::implDrawGradient(
                            rGraphics, basegfx::B2DPolyPolygon(aB2DPolygon), aGradient);
                    }
                }
                if (rRectangle.mpStrokeColor)
                {
                    rGraphics.SetLineColor(Color(*rRectangle.mpStrokeColor));
                    rGraphics.SetFillColor();
                    FileDefinitionWidgetDraw::drawPolyLine(
                        rGraphics, basegfx::B2DHomMatrix(), aB2DPolygon, 1.0 - rRectangle.mnOpacity,
                        rRectangle.mnStrokeWidth,
                        nullptr, // MM01
                        basegfx::B2DLineJoin::Round, css::drawing::LineCap_ROUND, 0.0f, false);
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
                    FileDefinitionWidgetDraw::drawPolyPolygon(rGraphics, basegfx::B2DHomMatrix(),
                                                              aPolyPolygon, 1.0 - rPath.mnOpacity);
                }
                if (rPath.mpStrokeColor)
                {
                    rGraphics.SetLineColor(Color(*rPath.mpStrokeColor));
                    rGraphics.SetFillColor();
                    for (auto const& rPolygon : aPolyPolygon)
                    {
                        FileDefinitionWidgetDraw::drawPolyLine(
                            rGraphics, basegfx::B2DHomMatrix(), rPolygon, 1.0 - rPath.mnOpacity,
                            rPath.mnStrokeWidth,
                            nullptr, // MM01
                            basegfx::B2DLineJoin::Round, css::drawing::LineCap_ROUND, 0.0f, false);
                    }
                }
            }
            break;

            default:
                break;
        }
    }
}

void munchDrawCommands(std::vector<std::shared_ptr<WidgetDrawAction>> const& rDrawActions,
                       SalGraphics& rGraphics, tools::Long nX, tools::Long nY, tools::Long nWidth,
                       tools::Long nHeight)
{
    for (std::shared_ptr<WidgetDrawAction> const& pDrawAction : rDrawActions)
    {
        switch (pDrawAction->maType)
        {
            case WidgetDrawActionType::RECTANGLE:
            {
                auto const& rWidgetDraw
                    = static_cast<WidgetDrawActionRectangle const&>(*pDrawAction);

                basegfx::B2DRectangle rRect(
                    nX + (nWidth * rWidgetDraw.mfX1), nY + (nHeight * rWidgetDraw.mfY1),
                    nX + (nWidth * rWidgetDraw.mfX2), nY + (nHeight * rWidgetDraw.mfY2));

                basegfx::B2DPolygon aB2DPolygon = basegfx::utils::createPolygonFromRect(
                    rRect, rWidgetDraw.mnRx / rRect.getWidth() * 2.0,
                    rWidgetDraw.mnRy / rRect.getHeight() * 2.0);

                rGraphics.SetLineColor();
                rGraphics.SetFillColor(rWidgetDraw.maFillColor);
                FileDefinitionWidgetDraw::drawPolyPolygon(
                    rGraphics, basegfx::B2DHomMatrix(), basegfx::B2DPolyPolygon(aB2DPolygon), 0.0f);
                rGraphics.SetLineColor(rWidgetDraw.maStrokeColor);
                rGraphics.SetFillColor();
                FileDefinitionWidgetDraw::drawPolyLine(
                    rGraphics, basegfx::B2DHomMatrix(), aB2DPolygon, 0.0f,
                    rWidgetDraw.mnStrokeWidth, nullptr, // MM01
                    basegfx::B2DLineJoin::Round, css::drawing::LineCap_ROUND, 0.0f, false);
            }
            break;
            case WidgetDrawActionType::LINE:
            {
                auto const& rWidgetDraw = static_cast<WidgetDrawActionLine const&>(*pDrawAction);
                Point aRectPoint(nX + 1, nY + 1);

                Size aRectSize(nWidth - 1, nHeight - 1);

                rGraphics.SetFillColor();
                rGraphics.SetLineColor(rWidgetDraw.maStrokeColor);

                basegfx::B2DPolygon aB2DPolygon{
                    { aRectPoint.X() + (aRectSize.Width() * rWidgetDraw.mfX1),
                      aRectPoint.Y() + (aRectSize.Height() * rWidgetDraw.mfY1) },
                    { aRectPoint.X() + (aRectSize.Width() * rWidgetDraw.mfX2),
                      aRectPoint.Y() + (aRectSize.Height() * rWidgetDraw.mfY2) },
                };

                FileDefinitionWidgetDraw::drawPolyLine(
                    rGraphics, basegfx::B2DHomMatrix(), aB2DPolygon, 0.0f,
                    rWidgetDraw.mnStrokeWidth, nullptr, // MM01
                    basegfx::B2DLineJoin::Round, css::drawing::LineCap_ROUND, 0.0f, false);
            }
            break;
            case WidgetDrawActionType::IMAGE:
            {
                double nScaleFactor = 1.0;
                if (comphelper::LibreOfficeKit::isActive())
                    nScaleFactor = comphelper::LibreOfficeKit::getDPIScale();

                auto const& rWidgetDraw = static_cast<WidgetDrawActionImage const&>(*pDrawAction);
                auto& rCacheImages = ImplGetSVData()->maGDIData.maThemeImageCache;
                OUString rCacheKey = rWidgetDraw.msSource + "@" + OUString::number(nScaleFactor);
                auto aIterator = rCacheImages.find(rCacheKey);

                BitmapEx aBitmap;
                if (aIterator == rCacheImages.end())
                {
                    SvFileStream aFileStream(rWidgetDraw.msSource, StreamMode::READ);

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

                tools::Long nImageWidth = aBitmap.GetSizePixel().Width();
                tools::Long nImageHeight = aBitmap.GetSizePixel().Height();
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
                        FileDefinitionWidgetDraw::drawBitmap(rGraphics, aTR, *pSalBitmap,
                                                             *pSalBitmapAlpha);
                    }
                    else
                    {
                        FileDefinitionWidgetDraw::drawBitmap(rGraphics, aTR, *pSalBitmap);
                    }
                }
            }
            break;
            case WidgetDrawActionType::EXTERNAL:
            {
                auto const& rWidgetDraw
                    = static_cast<WidgetDrawActionExternal const&>(*pDrawAction);

                auto& rCacheDrawCommands = ImplGetSVData()->maGDIData.maThemeDrawCommandsCache;

                auto aIterator = rCacheDrawCommands.find(rWidgetDraw.msSource);

                if (aIterator == rCacheDrawCommands.end())
                {
                    SvFileStream aFileStream(rWidgetDraw.msSource, StreamMode::READ);

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
                                std::make_pair(rWidgetDraw.msSource, *pDrawRoot));
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
                                                 const ImplControlValue& rValue, tools::Long nX,
                                                 tools::Long nY, tools::Long nWidth,
                                                 tools::Long nHeight)
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
                munchDrawCommands(pState->mpWidgetDrawActions, m_rGraphics, nX, nY, nWidth,
                                  nHeight);
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
                                                 const OUString& /*aCaptions*/,
                                                 const Color& /*rBackgroundColor*/)
{
    bool bOldAA = m_rGraphics.getAntiAlias();
    m_rGraphics.setAntiAlias(true);

    tools::Long nWidth = rControlRegion.GetWidth() - 1;
    tools::Long nHeight = rControlRegion.GetHeight() - 1;
    tools::Long nX = rControlRegion.Left();
    tools::Long nY = rControlRegion.Top();

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

                    tools::Long nUpperX = pSpinVal->maUpperRect.Left();
                    tools::Long nUpperY = pSpinVal->maUpperRect.Top();
                    tools::Long nUpperWidth = pSpinVal->maUpperRect.GetWidth() - 1;
                    tools::Long nUpperHeight = pSpinVal->maUpperRect.GetHeight() - 1;

                    bOK = resolveDefinition(eType, eUpButtonPart, eUpButtonState,
                                            ImplControlValue(), nUpperX, nUpperY, nUpperWidth,
                                            nUpperHeight);
                }

                if (bOK)
                {
                    ControlPart eDownButtonPart = pSpinVal->mnLowerPart;
                    ControlState eDownButtonState = pSpinVal->mnLowerState;

                    tools::Long nLowerX = pSpinVal->maLowerRect.Left();
                    tools::Long nLowerY = pSpinVal->maLowerRect.Top();
                    tools::Long nLowerWidth = pSpinVal->maLowerRect.GetWidth() - 1;
                    tools::Long nLowerHeight = pSpinVal->maLowerRect.GetHeight() - 1;

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
            tools::Long nThumbX = pSliderValue->maThumbRect.Left();
            tools::Long nThumbY = pSliderValue->maThumbRect.Top();
            tools::Long nThumbWidth = pSliderValue->maThumbRect.GetWidth() - 1;
            tools::Long nThumbHeight = pSliderValue->maThumbRect.GetHeight() - 1;

            if (ePart == ControlPart::TrackHorzArea)
            {
                tools::Long nCenterX = nThumbX + nThumbWidth / 2;

                bOK = resolveDefinition(eType, ControlPart::TrackHorzLeft, eState, rValue, nX, nY,
                                        nCenterX - nX, nHeight);
                if (bOK)
                    bOK = resolveDefinition(eType, ControlPart::TrackHorzRight, eState, rValue,
                                            nCenterX, nY, nX + nWidth - nCenterX, nHeight);
            }
            else if (ePart == ControlPart::TrackVertArea)
            {
                tools::Long nCenterY = nThumbY + nThumbHeight / 2;

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

    m_rGraphics.setAntiAlias(bOldAA);

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

            auto const& pEntirePart
                = m_pWidgetDefinition->getDefinition(eType, ControlPart::Entire);

            OString sOrientation = pEntirePart->msOrientation;

            if (sOrientation.isEmpty() || sOrientation == "stacked")
            {
                return false;
            }
            else if (sOrientation == "decrease-edit-increase")
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
            else if (sOrientation == "edit-decrease-increase")
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

            tools::Long nWidth = std::max(rBoundingControlRegion.GetWidth() + pPart->mnMarginWidth,
                                          tools::Long(pPart->mnWidth));
            tools::Long nHeight
                = std::max(rBoundingControlRegion.GetHeight() + pPart->mnMarginHeight,
                           tools::Long(pPart->mnHeight));

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
                                 - aComboButtonSize.Width() - 1,
                             aLocation.Y());
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
                rNativeBoundingRegion.expand(2);
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

    auto& pDefinitionStyle = m_pWidgetDefinition->mpStyle;

    aStyleSet.SetFaceColor(pDefinitionStyle->maFaceColor);
    aStyleSet.SetCheckedColor(pDefinitionStyle->maCheckedColor);
    aStyleSet.SetLightColor(pDefinitionStyle->maLightColor);
    aStyleSet.SetLightBorderColor(pDefinitionStyle->maLightBorderColor);
    aStyleSet.SetShadowColor(pDefinitionStyle->maShadowColor);
    aStyleSet.SetDarkShadowColor(pDefinitionStyle->maDarkShadowColor);
    aStyleSet.SetDefaultButtonTextColor(pDefinitionStyle->maDefaultButtonTextColor);
    aStyleSet.SetButtonTextColor(pDefinitionStyle->maButtonTextColor);
    aStyleSet.SetDefaultActionButtonTextColor(pDefinitionStyle->maDefaultActionButtonTextColor);
    aStyleSet.SetActionButtonTextColor(pDefinitionStyle->maActionButtonTextColor);
    aStyleSet.SetFlatButtonTextColor(pDefinitionStyle->maFlatButtonTextColor);
    aStyleSet.SetDefaultButtonRolloverTextColor(pDefinitionStyle->maDefaultButtonRolloverTextColor);
    aStyleSet.SetButtonRolloverTextColor(pDefinitionStyle->maButtonRolloverTextColor);
    aStyleSet.SetDefaultActionButtonRolloverTextColor(
        pDefinitionStyle->maDefaultActionButtonRolloverTextColor);
    aStyleSet.SetActionButtonRolloverTextColor(pDefinitionStyle->maActionButtonRolloverTextColor);
    aStyleSet.SetFlatButtonRolloverTextColor(pDefinitionStyle->maFlatButtonRolloverTextColor);
    aStyleSet.SetDefaultButtonPressedRolloverTextColor(
        pDefinitionStyle->maDefaultButtonPressedRolloverTextColor);
    aStyleSet.SetButtonPressedRolloverTextColor(pDefinitionStyle->maButtonPressedRolloverTextColor);
    aStyleSet.SetDefaultActionButtonPressedRolloverTextColor(
        pDefinitionStyle->maDefaultActionButtonPressedRolloverTextColor);
    aStyleSet.SetActionButtonPressedRolloverTextColor(
        pDefinitionStyle->maActionButtonPressedRolloverTextColor);
    aStyleSet.SetFlatButtonPressedRolloverTextColor(
        pDefinitionStyle->maFlatButtonPressedRolloverTextColor);
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

    auto& pSettings = m_pWidgetDefinition->mpSettings;

    int nFontSize = getSettingValueInteger(pSettings->msDefaultFontSize, 10);
    vcl::Font aFont(FAMILY_SWISS, Size(0, nFontSize));
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

    int nTitleHeight = getSettingValueInteger(pSettings->msTitleHeight, aStyleSet.GetTitleHeight());
    aStyleSet.SetTitleHeight(nTitleHeight);

    int nFloatTitleHeight
        = getSettingValueInteger(pSettings->msFloatTitleHeight, aStyleSet.GetFloatTitleHeight());
    aStyleSet.SetFloatTitleHeight(nFloatTitleHeight);

    int nLogicWidth = getSettingValueInteger(pSettings->msListBoxPreviewDefaultLogicWidth,
                                             15); // See vcl/source/app/settings.cxx
    int nLogicHeight = getSettingValueInteger(pSettings->msListBoxPreviewDefaultLogicHeight, 7);
    aStyleSet.SetListBoxPreviewDefaultLogicSize(Size(nLogicWidth, nLogicHeight));

    rSettings.SetStyleSettings(aStyleSet);

    return true;
}

} // end vcl namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
