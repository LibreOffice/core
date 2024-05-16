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

#include "PresenterTheme.hxx"
#include "PresenterBitmapContainer.hxx"
#include "PresenterCanvasHelper.hxx"
#include "PresenterConfigurationAccess.hxx"
#include <com/sun/star/drawing/XPresenterHelper.hpp>
#include <com/sun/star/rendering/PanoseWeight.hpp>
#include <osl/diagnose.h>
#include <map>
#include <numeric>
#include <utility>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sdext::presenter {

namespace {

class BorderSize
{
public:
    const static sal_Int32 mnInvalidValue = -10000;

    BorderSize() : mnLeft(mnInvalidValue),
                        mnTop(mnInvalidValue),
                        mnRight(mnInvalidValue),
                        mnBottom(mnInvalidValue) {}

    sal_Int32 mnLeft;
    sal_Int32 mnTop;
    sal_Int32 mnRight;
    sal_Int32 mnBottom;

    std::vector<sal_Int32> ToVector()
    {
        return
        {
            mnLeft == mnInvalidValue ? 0 : mnLeft,
            mnTop == mnInvalidValue ? 0 : mnTop,
            mnRight == mnInvalidValue ? 0 : mnRight,
            mnBottom == mnInvalidValue ? 0 : mnBottom
        };
    };

    void Merge (const BorderSize& rBorderSize)
    {
        if (mnLeft == mnInvalidValue)
            mnLeft = rBorderSize.mnLeft;
        if (mnTop == mnInvalidValue)
            mnTop = rBorderSize.mnTop;
        if (mnRight == mnInvalidValue)
            mnRight = rBorderSize.mnRight;
        if (mnBottom == mnInvalidValue)
            mnBottom = rBorderSize.mnBottom;
    }
};

/** Reading a theme from the configurations is done in various classes.  The
    ReadContext gives access to frequently used objects and functions to make
    the configuration handling easier.
*/
class ReadContext
{
public:
    Reference<XComponentContext> mxComponentContext;
    Reference<rendering::XCanvas> mxCanvas;
    Reference<drawing::XPresenterHelper> mxPresenterHelper;

    ReadContext (
        const Reference<XComponentContext>& rxContext,
        const Reference<rendering::XCanvas>& rxCanvas);

    /** Read data describing a font from the node that can be reached from
        the given root via the given path.
        @param rsFontPath
            May be empty.
    */
    static PresenterTheme::SharedFontDescriptor ReadFont (
        const css::uno::Reference<css::container::XHierarchicalNameAccess>& rxTheme,
        const PresenterTheme::SharedFontDescriptor& rpDefault);
    static PresenterTheme::SharedFontDescriptor ReadFont (
        const Reference<beans::XPropertySet>& rxFontProperties,
        const PresenterTheme::SharedFontDescriptor& rpDefault);

    std::shared_ptr<PresenterTheme::Theme> ReadTheme (
        PresenterConfigurationAccess& rConfiguration,
        const OUString& rsThemeName);

    static BorderSize ReadBorderSize (const Reference<container::XNameAccess>& rxNode);

private:
    static Any GetByName (
        const Reference<container::XNameAccess>& rxNode,
        const OUString& rsName);
};

/** A PaneStyle describes how a pane is rendered.
*/
class PaneStyle
{
public:
    PaneStyle();

    SharedBitmapDescriptor GetBitmap (const OUString& sBitmapName) const;

    OUString msStyleName;
    std::shared_ptr<PaneStyle> mpParentStyle;
    PresenterTheme::SharedFontDescriptor mpFont;
    BorderSize maInnerBorderSize;
    BorderSize maOuterBorderSize;
    std::shared_ptr<PresenterBitmapContainer> mpBitmaps;

    PresenterTheme::SharedFontDescriptor GetFont() const;
};

typedef std::shared_ptr<PaneStyle> SharedPaneStyle;

class PaneStyleContainer
{
private:
    ::std::vector<SharedPaneStyle> mStyles;

public:
    void Read (
        const ReadContext& rReadContext,
        const Reference<container::XHierarchicalNameAccess>& rThemeRoot);

    SharedPaneStyle GetPaneStyle (const OUString& rsStyleName) const;

private:
    void ProcessPaneStyle (
        ReadContext const & rReadContext,
        const ::std::vector<css::uno::Any>& rValues);
};

/** A ViewStyle describes how a view is displayed.
*/
class ViewStyle
{
public:
    ViewStyle();

    SharedBitmapDescriptor GetBitmap (std::u16string_view sBitmapName) const;

    PresenterTheme::SharedFontDescriptor GetFont() const;

    OUString msStyleName;
    std::shared_ptr<ViewStyle> mpParentStyle;
    PresenterTheme::SharedFontDescriptor mpFont;
    SharedBitmapDescriptor mpBackground;
};

typedef std::shared_ptr<ViewStyle> SharedViewStyle;

class ViewStyleContainer
{
private:
    ::std::vector<SharedViewStyle> mStyles;

public:
    void Read (
        const ReadContext& rReadContext,
        const Reference<container::XHierarchicalNameAccess>& rThemeRoot);

    SharedViewStyle GetViewStyle (const OUString& rsStyleName) const;

private:
    void ProcessViewStyle(
        ReadContext const & rReadContext,
        const Reference<beans::XPropertySet>& rxProperties);
};

class StyleAssociationContainer
{
public:
    void Read (
        const Reference<container::XHierarchicalNameAccess>& rThemeRoot);

    OUString GetStyleName (const OUString& rsResourceName) const;

private:
    typedef std::map<OUString, OUString> StyleAssociations;
    StyleAssociations maStyleAssociations;

    void ProcessStyleAssociation(
        const ::std::vector<css::uno::Any>& rValues);
};

} // end of anonymous namespace

class PresenterTheme::Theme
{
public:
    Theme (
        const Reference<container::XHierarchicalNameAccess>& rThemeRoot,
        OUString sNodeName);

    void Read (
        PresenterConfigurationAccess& rConfiguration,
        ReadContext& rReadContext);

    OUString msConfigurationNodeName;
    std::shared_ptr<Theme> mpParentTheme;
    SharedBitmapDescriptor mpBackground;
    PaneStyleContainer maPaneStyles;
    ViewStyleContainer maViewStyles;
    StyleAssociationContainer maStyleAssociations;
    Reference<container::XHierarchicalNameAccess> mxThemeRoot;
    std::shared_ptr<PresenterBitmapContainer> mpIconContainer;
    typedef std::map<OUString,SharedFontDescriptor> FontContainer;
    FontContainer maFontContainer;

    SharedPaneStyle GetPaneStyle (const OUString& rsStyleName) const;
    SharedViewStyle GetViewStyle (const OUString& rsStyleName) const;

private:
    void ProcessFont(
        const OUString& rsKey,
        const Reference<beans::XPropertySet>& rxProperties);
};

//===== PresenterTheme ========================================================

PresenterTheme::PresenterTheme (
    css::uno::Reference<css::uno::XComponentContext> xContext,
    css::uno::Reference<css::rendering::XCanvas> xCanvas)
    : mxContext(std::move(xContext)),
      mxCanvas(std::move(xCanvas))
{
    mpTheme = ReadTheme();
}

PresenterTheme::~PresenterTheme()
{
}

std::shared_ptr<PresenterTheme::Theme> PresenterTheme::ReadTheme()
{
    ReadContext aReadContext(mxContext, mxCanvas);

    PresenterConfigurationAccess aConfiguration (
        mxContext,
        u"/org.openoffice.Office.PresenterScreen/"_ustr,
        PresenterConfigurationAccess::READ_ONLY);

    return aReadContext.ReadTheme(aConfiguration, OUString());
}

bool PresenterTheme::HasCanvas() const
{
    return mxCanvas.is();
}

void PresenterTheme::ProvideCanvas (const Reference<rendering::XCanvas>& rxCanvas)
{
    if ( ! mxCanvas.is() && rxCanvas.is())
    {
        mxCanvas = rxCanvas;
        ReadTheme();
    }
}

OUString PresenterTheme::GetStyleName (const OUString& rsResourceURL) const
{
    OUString sStyleName;
    std::shared_ptr<Theme> pTheme (mpTheme);
    while (sStyleName.isEmpty() && pTheme != nullptr)
    {
        sStyleName = pTheme->maStyleAssociations.GetStyleName(rsResourceURL);
        pTheme = pTheme->mpParentTheme;
    }
    return sStyleName;
}

::std::vector<sal_Int32> PresenterTheme::GetBorderSize (
    const OUString& rsStyleName,
    const bool bOuter) const
{
    OSL_ASSERT(mpTheme != nullptr);

    SharedPaneStyle pPaneStyle (mpTheme->GetPaneStyle(rsStyleName));
    if (pPaneStyle)
        if (bOuter)
            return pPaneStyle->maOuterBorderSize.ToVector();
        else
            return pPaneStyle->maInnerBorderSize.ToVector();
    else
    {
        return ::std::vector<sal_Int32>(4,0);
    }
}

PresenterTheme::SharedFontDescriptor PresenterTheme::ReadFont (
    const Reference<container::XHierarchicalNameAccess>& rxNode,
    const PresenterTheme::SharedFontDescriptor& rpDefault)
{
    return ReadContext::ReadFont(rxNode, rpDefault);
}

bool PresenterTheme::ConvertToColor (
    const Any& rColorSequence,
    sal_uInt32& rColor)
{
    Sequence<sal_Int8> aByteSequence;
    if (rColorSequence >>= aByteSequence)
    {
        rColor = std::accumulate(std::cbegin(aByteSequence), std::cend(aByteSequence), sal_uInt32(0),
            [](const sal_uInt32 nRes, const sal_uInt8 nByte) { return (nRes << 8) | nByte; });
        return true;
    }
    else
        return false;
}

std::shared_ptr<PresenterConfigurationAccess> PresenterTheme::GetNodeForViewStyle (
    const OUString& rsStyleName) const
{
    if (mpTheme == nullptr)
        return std::shared_ptr<PresenterConfigurationAccess>();

    // Open configuration for writing.
    auto pConfiguration = std::make_shared<PresenterConfigurationAccess>(
            mxContext,
            "/org.openoffice.Office.PresenterScreen/",
            PresenterConfigurationAccess::READ_WRITE);

    // Get configuration node for the view style container of the current
    // theme.
    if (pConfiguration->GoToChild( OUString(
        "Presenter/Themes/" + mpTheme->msConfigurationNodeName + "/ViewStyles")))
    {
        pConfiguration->GoToChild(
            [&rsStyleName] (OUString const&, uno::Reference<beans::XPropertySet> const& xProps)
            {
                return PresenterConfigurationAccess::IsStringPropertyEqual(
                        rsStyleName, u"StyleName"_ustr, xProps);
            });
    }
    return pConfiguration;
}

SharedBitmapDescriptor PresenterTheme::GetBitmap (
    const OUString& rsStyleName,
    const OUString& rsBitmapName) const
{
    if (mpTheme != nullptr)
    {
        if (rsStyleName.isEmpty())
        {
            if (rsBitmapName == "Background")
            {
                std::shared_ptr<Theme> pTheme (mpTheme);
                while (pTheme != nullptr && !pTheme->mpBackground)
                    pTheme = pTheme->mpParentTheme;
                if (pTheme != nullptr)
                    return pTheme->mpBackground;
                else
                    return SharedBitmapDescriptor();
            }
        }
        else
        {
            SharedPaneStyle pPaneStyle (mpTheme->GetPaneStyle(rsStyleName));
            if (pPaneStyle)
            {
                SharedBitmapDescriptor pBitmap (pPaneStyle->GetBitmap(rsBitmapName));
                if (pBitmap)
                    return pBitmap;
            }

            SharedViewStyle pViewStyle (mpTheme->GetViewStyle(rsStyleName));
            if (pViewStyle)
            {
                SharedBitmapDescriptor pBitmap (pViewStyle->GetBitmap(rsBitmapName));
                if (pBitmap)
                    return pBitmap;
            }
        }
    }

    return SharedBitmapDescriptor();
}

SharedBitmapDescriptor PresenterTheme::GetBitmap (
    const OUString& rsBitmapName) const
{
    if (mpTheme != nullptr)
    {
        if (rsBitmapName == "Background")
        {
            std::shared_ptr<Theme> pTheme (mpTheme);
            while (pTheme != nullptr && !pTheme->mpBackground)
                pTheme = pTheme->mpParentTheme;
            if (pTheme != nullptr)
                return pTheme->mpBackground;
            else
                return SharedBitmapDescriptor();
        }
        else
        {
            if (mpTheme->mpIconContainer != nullptr)
                return mpTheme->mpIconContainer->GetBitmap(rsBitmapName);
        }
    }

    return SharedBitmapDescriptor();
}

std::shared_ptr<PresenterBitmapContainer> PresenterTheme::GetBitmapContainer() const
{
    if (mpTheme != nullptr)
        return mpTheme->mpIconContainer;
    else
        return std::shared_ptr<PresenterBitmapContainer>();
}

PresenterTheme::SharedFontDescriptor PresenterTheme::GetFont (
    const OUString& rsStyleName) const
{
    if (mpTheme != nullptr)
    {
        SharedPaneStyle pPaneStyle (mpTheme->GetPaneStyle(rsStyleName));
        if (pPaneStyle)
            return pPaneStyle->GetFont();

        SharedViewStyle pViewStyle (mpTheme->GetViewStyle(rsStyleName));
        if (pViewStyle)
            return pViewStyle->GetFont();

        std::shared_ptr<Theme> pTheme (mpTheme);
        while (pTheme != nullptr)
        {
            Theme::FontContainer::const_iterator iFont (pTheme->maFontContainer.find(rsStyleName));
            if (iFont != pTheme->maFontContainer.end())
                return iFont->second;

            pTheme = pTheme->mpParentTheme;
        }
    }

    return SharedFontDescriptor();
}

//===== FontDescriptor ========================================================

PresenterTheme::FontDescriptor::FontDescriptor (
    const std::shared_ptr<FontDescriptor>& rpDescriptor)
    : mnSize(12),
      mnColor(0x00000000),
      msAnchor(u"Left"_ustr),
      mnXOffset(0),
      mnYOffset(0)
{
    if (rpDescriptor != nullptr)
    {
        msFamilyName = rpDescriptor->msFamilyName;
        msStyleName = rpDescriptor->msStyleName;
        mnSize = rpDescriptor->mnSize;
        mnColor = rpDescriptor->mnColor;
        msAnchor = rpDescriptor->msAnchor;
        mnXOffset = rpDescriptor->mnXOffset;
        mnYOffset = rpDescriptor->mnYOffset;
    }
}

bool PresenterTheme::FontDescriptor::PrepareFont (
    const Reference<rendering::XCanvas>& rxCanvas)
{
    if (mxFont.is())
        return true;

    if ( ! rxCanvas.is())
        return false;

    const double nCellSize (GetCellSizeForDesignSize(rxCanvas, mnSize));
    mxFont = CreateFont(rxCanvas, nCellSize);

    return mxFont.is();
}

Reference<rendering::XCanvasFont> PresenterTheme::FontDescriptor::CreateFont (
    const Reference<rendering::XCanvas>& rxCanvas,
    const double nCellSize) const
{
    rendering::FontRequest aFontRequest;
    aFontRequest.FontDescription.FamilyName = msFamilyName;
    if (msFamilyName.isEmpty())
        aFontRequest.FontDescription.FamilyName = "Tahoma";
    aFontRequest.FontDescription.StyleName = msStyleName;
    aFontRequest.CellSize = nCellSize;

    // Make an attempt at translating the style name(s)into a corresponding
    // font description.
    if (msStyleName == "Bold")
        aFontRequest.FontDescription.FontDescription.Weight = rendering::PanoseWeight::HEAVY;

    return rxCanvas->createFont(
        aFontRequest,
        Sequence<beans::PropertyValue>(),
        geometry::Matrix2D(1,0,0,1));
}

double PresenterTheme::FontDescriptor::GetCellSizeForDesignSize (
    const Reference<rendering::XCanvas>& rxCanvas,
    const double nDesignSize) const
{
    // Use the given design size as initial value in calculating the cell
    // size.
    double nCellSize (nDesignSize);

    if ( ! rxCanvas.is())
    {
        // We need the canvas to do the conversion.  Return the design size,
        // it is the our best guess in this circumstance.
        return nDesignSize;
    }

    Reference<rendering::XCanvasFont> xFont (CreateFont(rxCanvas, nCellSize));
    if ( ! xFont.is())
        return nDesignSize;

    geometry::RealRectangle2D aBox (PresenterCanvasHelper::GetTextBoundingBox (xFont, u"X"_ustr));

    const double nAscent (-aBox.Y1);
    //tdf#112408
    if (nAscent == 0)
        return nDesignSize;
    const double nDescent (aBox.Y2);
    const double nScale = (nAscent+nDescent) / nAscent;
    return nDesignSize * nScale;
}

//===== Theme =================================================================

PresenterTheme::Theme::Theme (
    const Reference<container::XHierarchicalNameAccess>& rxThemeRoot,
    OUString sNodeName)
    : msConfigurationNodeName(std::move(sNodeName)),
      maPaneStyles(),
      maViewStyles(),
      maStyleAssociations(),
      mxThemeRoot(rxThemeRoot)
{
}

void PresenterTheme::Theme::Read (
    PresenterConfigurationAccess& rConfiguration,
    ReadContext& rReadContext)
{
    // Parent theme name.
    OUString sParentThemeName;
    if ((PresenterConfigurationAccess::GetConfigurationNode(mxThemeRoot, u"ParentTheme"_ustr)
            >>= sParentThemeName)
        && !sParentThemeName.isEmpty())
    {
        mpParentTheme = rReadContext.ReadTheme(rConfiguration, sParentThemeName);
    }

    // Background.
    mpBackground = PresenterBitmapContainer::LoadBitmap(
        mxThemeRoot,
        u"Background"_ustr,
        rReadContext.mxPresenterHelper,
        rReadContext.mxCanvas,
        SharedBitmapDescriptor());

    // Style associations.
    maStyleAssociations.Read(mxThemeRoot);

    // Pane styles.
    maPaneStyles.Read(rReadContext, mxThemeRoot);

    // View styles.
    maViewStyles.Read(rReadContext, mxThemeRoot);

    // Read bitmaps.
    mpIconContainer = std::make_shared<PresenterBitmapContainer>(
        Reference<container::XNameAccess>(
            PresenterConfigurationAccess::GetConfigurationNode(mxThemeRoot, u"Bitmaps"_ustr), UNO_QUERY),
        mpParentTheme != nullptr ? mpParentTheme->mpIconContainer
                                 : std::shared_ptr<PresenterBitmapContainer>(),
        rReadContext.mxComponentContext, rReadContext.mxCanvas);

    // Read fonts.
    Reference<container::XNameAccess> xFontNode(
        PresenterConfigurationAccess::GetConfigurationNode(mxThemeRoot, u"Fonts"_ustr),
        UNO_QUERY);
    PresenterConfigurationAccess::ForAll(
        xFontNode,
        [this] (OUString const& rKey, uno::Reference<beans::XPropertySet> const& xProps)
        {
            return this->ProcessFont(rKey, xProps);
        });
}

SharedPaneStyle PresenterTheme::Theme::GetPaneStyle (const OUString& rsStyleName) const
{
    SharedPaneStyle pPaneStyle (maPaneStyles.GetPaneStyle(rsStyleName));
    if (pPaneStyle)
        return pPaneStyle;
    else if (mpParentTheme != nullptr)
        return mpParentTheme->GetPaneStyle(rsStyleName);
    else
        return SharedPaneStyle();
}

SharedViewStyle PresenterTheme::Theme::GetViewStyle (const OUString& rsStyleName) const
{
    SharedViewStyle pViewStyle (maViewStyles.GetViewStyle(rsStyleName));
    if (pViewStyle)
        return pViewStyle;
    else if (mpParentTheme != nullptr)
        return mpParentTheme->GetViewStyle(rsStyleName);
    else
        return SharedViewStyle();
}

void PresenterTheme::Theme::ProcessFont(
    const OUString& rsKey,
    const Reference<beans::XPropertySet>& rxProperties)
{
    maFontContainer[rsKey] = ReadContext::ReadFont(rxProperties, SharedFontDescriptor());
}

namespace {

//===== ReadContext ===========================================================

ReadContext::ReadContext (
    const css::uno::Reference<css::uno::XComponentContext>& rxContext,
    const Reference<rendering::XCanvas>& rxCanvas)
    : mxComponentContext(rxContext),
      mxCanvas(rxCanvas)
{
    Reference<lang::XMultiComponentFactory> xFactory (rxContext->getServiceManager());
    if (xFactory.is())
    {
        mxPresenterHelper.set(
            xFactory->createInstanceWithContext(
                u"com.sun.star.comp.Draw.PresenterHelper"_ustr,
                rxContext),
            UNO_QUERY_THROW);
    }
}

PresenterTheme::SharedFontDescriptor ReadContext::ReadFont (
    const Reference<container::XHierarchicalNameAccess>& rxNode,
    const PresenterTheme::SharedFontDescriptor& rpDefault)
{
    if ( ! rxNode.is())
        return PresenterTheme::SharedFontDescriptor();

    try
    {
        Reference<container::XHierarchicalNameAccess> xFont (
            PresenterConfigurationAccess::GetConfigurationNode(
                rxNode,
                /*rsFontPath*/u""_ustr),
                UNO_QUERY_THROW);

        Reference<beans::XPropertySet> xProperties (xFont, UNO_QUERY_THROW);
        return ReadFont(xProperties, rpDefault);
    }
    catch (Exception&)
    {
        OSL_ASSERT(false);
    }

    return PresenterTheme::SharedFontDescriptor();
}

PresenterTheme::SharedFontDescriptor ReadContext::ReadFont (
    const Reference<beans::XPropertySet>& rxProperties,
    const PresenterTheme::SharedFontDescriptor& rpDefault)
{
    auto pDescriptor = std::make_shared<PresenterTheme::FontDescriptor>(rpDefault);

    PresenterConfigurationAccess::GetProperty(rxProperties, u"FamilyName"_ustr) >>= pDescriptor->msFamilyName;
    PresenterConfigurationAccess::GetProperty(rxProperties, u"Style"_ustr) >>= pDescriptor->msStyleName;
    PresenterConfigurationAccess::GetProperty(rxProperties, u"Size"_ustr) >>= pDescriptor->mnSize;
    PresenterTheme::ConvertToColor(
        PresenterConfigurationAccess::GetProperty(rxProperties, u"Color"_ustr),
        pDescriptor->mnColor);
    PresenterConfigurationAccess::GetProperty(rxProperties, u"Anchor"_ustr) >>= pDescriptor->msAnchor;
    PresenterConfigurationAccess::GetProperty(rxProperties, u"XOffset"_ustr) >>= pDescriptor->mnXOffset;
    PresenterConfigurationAccess::GetProperty(rxProperties, u"YOffset"_ustr) >>= pDescriptor->mnYOffset;

    return pDescriptor;
}

Any ReadContext::GetByName (
    const Reference<container::XNameAccess>& rxNode,
    const OUString& rsName)
{
    OSL_ASSERT(rxNode.is());
    if (rxNode->hasByName(rsName))
        return rxNode->getByName(rsName);
    else
        return Any();
}

std::shared_ptr<PresenterTheme::Theme> ReadContext::ReadTheme (
    PresenterConfigurationAccess& rConfiguration,
    const OUString& rsThemeName)
{
    std::shared_ptr<PresenterTheme::Theme> pTheme;

    OUString sCurrentThemeName (rsThemeName);
    if (sCurrentThemeName.isEmpty())
    {
         // No theme name given.  Look up the CurrentTheme property.
         rConfiguration.GetConfigurationNode(u"Presenter/CurrentTheme"_ustr) >>= sCurrentThemeName;
         if (sCurrentThemeName.isEmpty())
         {
             // Still no name.  Use "DefaultTheme".
             sCurrentThemeName = "DefaultTheme";
         }
    }

    Reference<container::XNameAccess> xThemes (
        rConfiguration.GetConfigurationNode(u"Presenter/Themes"_ustr),
        UNO_QUERY);
    if (xThemes.is())
    {
        // Iterate over all themes and search the one with the given name.
        const Sequence<OUString> aKeys (xThemes->getElementNames());
        for (const OUString& rsKey : aKeys)
        {
            Reference<container::XHierarchicalNameAccess> xTheme (
                xThemes->getByName(rsKey), UNO_QUERY);
            if (xTheme.is())
            {
                OUString sThemeName;
                PresenterConfigurationAccess::GetConfigurationNode(xTheme, u"ThemeName"_ustr)
                    >>= sThemeName;
                if (sThemeName == sCurrentThemeName)
                {
                    pTheme = std::make_shared<PresenterTheme::Theme>(xTheme,rsKey);
                    break;
                }
            }
        }
    }

    if (pTheme != nullptr)
    {
        pTheme->Read(rConfiguration, *this);
    }

    return pTheme;
}

BorderSize ReadContext::ReadBorderSize (const Reference<container::XNameAccess>& rxNode)
{
    BorderSize aBorderSize;

    if (rxNode.is())
    {
        GetByName(rxNode, u"Left"_ustr) >>= aBorderSize.mnLeft;
        GetByName(rxNode, u"Top"_ustr) >>= aBorderSize.mnTop;
        GetByName(rxNode, u"Right"_ustr) >>= aBorderSize.mnRight;
        GetByName(rxNode, u"Bottom"_ustr) >>= aBorderSize.mnBottom;
    }

    return aBorderSize;
}

//===== PaneStyleContainer ====================================================

void PaneStyleContainer::Read (
    const ReadContext& rReadContext,
    const Reference<container::XHierarchicalNameAccess>& rxThemeRoot)
{
    Reference<container::XNameAccess> xPaneStyleList (
        PresenterConfigurationAccess::GetConfigurationNode(
            rxThemeRoot,
            u"PaneStyles"_ustr),
        UNO_QUERY);
    if (!xPaneStyleList.is())
        return;

    ::std::vector<OUString> aProperties;
    aProperties.reserve(6);
    aProperties.emplace_back("StyleName");
    aProperties.emplace_back("ParentStyle");
    aProperties.emplace_back("TitleFont");
    aProperties.emplace_back("InnerBorderSize");
    aProperties.emplace_back("OuterBorderSize");
    aProperties.emplace_back("BorderBitmapList");
    PresenterConfigurationAccess::ForAll(
        xPaneStyleList,
        aProperties,
        [this, &rReadContext] (std::vector<uno::Any> const& rValues)
        {
            return this->ProcessPaneStyle(rReadContext, rValues);
        });
}

void PaneStyleContainer::ProcessPaneStyle(
    ReadContext const & rReadContext,
    const ::std::vector<Any>& rValues)
{
    if (rValues.size() != 6)
        return;

    auto pStyle = std::make_shared<PaneStyle>();

    rValues[0] >>= pStyle->msStyleName;

    OUString sParentStyleName;
    if (rValues[1] >>= sParentStyleName)
    {
        // Find parent style.
        auto iStyle = std::find_if(mStyles.begin(), mStyles.end(),
            [&sParentStyleName](const SharedPaneStyle& rxStyle) { return rxStyle->msStyleName == sParentStyleName; });
        if (iStyle != mStyles.end())
            pStyle->mpParentStyle = *iStyle;
    }

    Reference<container::XHierarchicalNameAccess> xFontNode (rValues[2], UNO_QUERY);
    pStyle->mpFont = ReadContext::ReadFont(
        xFontNode, PresenterTheme::SharedFontDescriptor());

    Reference<container::XNameAccess> xInnerBorderSizeNode (rValues[3], UNO_QUERY);
    pStyle->maInnerBorderSize = ReadContext::ReadBorderSize(xInnerBorderSizeNode);
    Reference<container::XNameAccess> xOuterBorderSizeNode (rValues[4], UNO_QUERY);
    pStyle->maOuterBorderSize = ReadContext::ReadBorderSize(xOuterBorderSizeNode);

    if (pStyle->mpParentStyle != nullptr)
    {
        pStyle->maInnerBorderSize.Merge(pStyle->mpParentStyle->maInnerBorderSize);
        pStyle->maOuterBorderSize.Merge(pStyle->mpParentStyle->maOuterBorderSize);
    }

    if (rReadContext.mxCanvas.is())
    {
        Reference<container::XNameAccess> xBitmapsNode (rValues[5], UNO_QUERY);
        pStyle->mpBitmaps = std::make_shared<PresenterBitmapContainer>(
            xBitmapsNode,
            pStyle->mpParentStyle != nullptr ? pStyle->mpParentStyle->mpBitmaps
                                             : std::shared_ptr<PresenterBitmapContainer>(),
            rReadContext.mxComponentContext, rReadContext.mxCanvas,
            rReadContext.mxPresenterHelper);
    }

    mStyles.push_back(pStyle);
}

SharedPaneStyle PaneStyleContainer::GetPaneStyle (const OUString& rsStyleName) const
{
    auto iStyle = std::find_if(mStyles.begin(), mStyles.end(),
        [&rsStyleName](const SharedPaneStyle& rxStyle) { return rxStyle->msStyleName == rsStyleName; });
    if (iStyle != mStyles.end())
        return *iStyle;
    return SharedPaneStyle();
}

//===== PaneStyle =============================================================

PaneStyle::PaneStyle()
{
}

SharedBitmapDescriptor PaneStyle::GetBitmap (const OUString& rsBitmapName) const
{
    if (mpBitmaps != nullptr)
    {
        SharedBitmapDescriptor pBitmap = mpBitmaps->GetBitmap(rsBitmapName);
        if (pBitmap)
            return pBitmap;
    }

    if (mpParentStyle != nullptr)
        return mpParentStyle->GetBitmap(rsBitmapName);
    else
        return SharedBitmapDescriptor();
}

PresenterTheme::SharedFontDescriptor PaneStyle::GetFont() const
{
    if (mpFont)
        return mpFont;
    else if (mpParentStyle != nullptr)
        return mpParentStyle->GetFont();
    else
        return PresenterTheme::SharedFontDescriptor();
}

//===== ViewStyleContainer ====================================================

void ViewStyleContainer::Read (
    const ReadContext& rReadContext,
    const Reference<container::XHierarchicalNameAccess>& rxThemeRoot)
{
    Reference<container::XNameAccess> xViewStyleList (
        PresenterConfigurationAccess::GetConfigurationNode(
            rxThemeRoot,
            u"ViewStyles"_ustr),
        UNO_QUERY);
    if (xViewStyleList.is())
    {
        PresenterConfigurationAccess::ForAll(
            xViewStyleList,
            [this, &rReadContext] (OUString const&, uno::Reference<beans::XPropertySet> const& xProps)
            {
                return this->ProcessViewStyle(rReadContext, xProps);
            });
    }
}

void ViewStyleContainer::ProcessViewStyle(
    ReadContext const & rReadContext,
    const Reference<beans::XPropertySet>& rxProperties)
{
    auto pStyle = std::make_shared<ViewStyle>();

    PresenterConfigurationAccess::GetProperty(rxProperties, u"StyleName"_ustr)
        >>= pStyle->msStyleName;

    OUString sParentStyleName;
    if (PresenterConfigurationAccess::GetProperty(rxProperties, u"ParentStyle"_ustr)
        >>= sParentStyleName)
    {
        // Find parent style.
        auto iStyle = std::find_if(mStyles.begin(), mStyles.end(),
            [&sParentStyleName](const SharedViewStyle& rxStyle) { return rxStyle->msStyleName == sParentStyleName; });
        if (iStyle != mStyles.end())
        {
            pStyle->mpParentStyle = *iStyle;
            pStyle->mpFont = (*iStyle)->mpFont;
            pStyle->mpBackground = (*iStyle)->mpBackground;
        }
    }

    Reference<container::XHierarchicalNameAccess> xFontNode (
        PresenterConfigurationAccess::GetProperty(rxProperties, u"Font"_ustr), UNO_QUERY);
    PresenterTheme::SharedFontDescriptor pFont (
        ReadContext::ReadFont(xFontNode, PresenterTheme::SharedFontDescriptor()));
    if (pFont)
        pStyle->mpFont = pFont;

    Reference<container::XHierarchicalNameAccess> xBackgroundNode (
        PresenterConfigurationAccess::GetProperty(rxProperties, u"Background"_ustr),
        UNO_QUERY);
    SharedBitmapDescriptor pBackground (PresenterBitmapContainer::LoadBitmap(
        xBackgroundNode,
        OUString(),
        rReadContext.mxPresenterHelper,
        rReadContext.mxCanvas,
        SharedBitmapDescriptor()));
    if (pBackground && pBackground->GetNormalBitmap().is())
        pStyle->mpBackground = pBackground;

    mStyles.push_back(pStyle);
}

SharedViewStyle ViewStyleContainer::GetViewStyle (const OUString& rsStyleName) const
{
    auto iStyle = std::find_if(mStyles.begin(), mStyles.end(),
        [&rsStyleName](const SharedViewStyle& rxStyle) { return rxStyle->msStyleName == rsStyleName; });
    if (iStyle != mStyles.end())
        return *iStyle;
    return SharedViewStyle();
}

//===== ViewStyle =============================================================

ViewStyle::ViewStyle()
{
}

SharedBitmapDescriptor ViewStyle::GetBitmap (std::u16string_view rsBitmapName) const
{
    if (rsBitmapName == u"Background")
        return mpBackground;
    else
        return SharedBitmapDescriptor();
}

PresenterTheme::SharedFontDescriptor ViewStyle::GetFont() const
{
    if (mpFont)
        return mpFont;
    else if (mpParentStyle != nullptr)
        return mpParentStyle->GetFont();
    else
        return PresenterTheme::SharedFontDescriptor();
}

//===== StyleAssociationContainer =============================================

void StyleAssociationContainer::Read (
    const Reference<container::XHierarchicalNameAccess>& rxThemeRoot)
{
    Reference<container::XNameAccess> xStyleAssociationList (
        PresenterConfigurationAccess::GetConfigurationNode(
            rxThemeRoot,
            u"StyleAssociations"_ustr),
        UNO_QUERY);
    if (!xStyleAssociationList.is())
        return;

    ::std::vector<OUString> aProperties { u"ResourceURL"_ustr, u"StyleName"_ustr };
    PresenterConfigurationAccess::ForAll(
        xStyleAssociationList,
        aProperties,
        [this] (std::vector<uno::Any> const& rValues)
        {
            return this->ProcessStyleAssociation(rValues);
        });
}

OUString StyleAssociationContainer::GetStyleName (const OUString& rsResourceName) const
{
    StyleAssociations::const_iterator iAssociation (maStyleAssociations.find(rsResourceName));
    if (iAssociation != maStyleAssociations.end())
        return iAssociation->second;
    else
        return OUString();
}

void StyleAssociationContainer::ProcessStyleAssociation(
    const ::std::vector<Any>& rValues)
{
    if (rValues.size() != 2)
        return;

    OUString sResourceURL;
    OUString sStyleName;
    if ((rValues[0] >>= sResourceURL)
        && (rValues[1] >>= sStyleName))
    {
        maStyleAssociations[sResourceURL] = sStyleName;
    }
}

} // end of anonymous namespace

} // end of namespace ::sdext::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
