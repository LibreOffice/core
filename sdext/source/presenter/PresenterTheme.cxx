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
#include "PresenterComponent.hxx"
#include "PresenterConfigurationAccess.hxx"
#include "PresenterHelper.hxx"
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/deployment/XPackageInformationProvider.hpp>
#include <com/sun/star/drawing/XPresenterHelper.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/rendering/PanoseWeight.hpp>
#include <com/sun/star/rendering/XBitmap.hpp>
#include <com/sun/star/util/Color.hpp>
#include <boost/bind.hpp>
#include <map>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::std;
using ::rtl::OUString;

#define A2S(s) (::rtl::OUString(s))

namespace sdext { namespace presenter {

namespace {

class BorderSize
{
public:
    const static sal_Int32 mnInvalidValue = -10000;

    BorderSize (void) : mnLeft(mnInvalidValue),
                        mnTop(mnInvalidValue),
                        mnRight(mnInvalidValue),
                        mnBottom(mnInvalidValue) {}

    sal_Int32 mnLeft;
    sal_Int32 mnTop;
    sal_Int32 mnRight;
    sal_Int32 mnBottom;

    vector<sal_Int32> ToVector (void)
    {
        vector<sal_Int32> aSequence (4);
        aSequence[0] = mnLeft == mnInvalidValue ? 0 : mnLeft;
        aSequence[1] = mnTop == mnInvalidValue ? 0 : mnTop;
        aSequence[2] = mnRight == mnInvalidValue ? 0 : mnRight;
        aSequence[3] = mnBottom == mnInvalidValue ? 0 : mnBottom;
        return aSequence;
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
    OUString msBasePath;

    ReadContext (
        const Reference<XComponentContext>& rxContext,
        const Reference<rendering::XCanvas>& rxCanvas);
    ~ReadContext (void);

    /** Read data describing a font from the node that can be reached from
        the given root via the given path.
        @param rsFontPath
            May be empty.
    */
    static PresenterTheme::SharedFontDescriptor ReadFont (
        const css::uno::Reference<css::container::XHierarchicalNameAccess>& rxTheme,
        const ::rtl::OUString& rsFontPath,
        const PresenterTheme::SharedFontDescriptor& rpDefault);
    static PresenterTheme::SharedFontDescriptor ReadFont (
        const Reference<beans::XPropertySet>& rxFontProperties,
        const PresenterTheme::SharedFontDescriptor& rpDefault);

    ::boost::shared_ptr<PresenterTheme::Theme> ReadTheme (
        PresenterConfigurationAccess& rConfiguration,
        const OUString& rsThemeName);

    BorderSize ReadBorderSize (const Reference<container::XNameAccess>& rxNode);

    void SetBitmapSourceExtension (const OUString& rsExtensionName);

private:
    Any GetByName (
        const Reference<container::XNameAccess>& rxNode,
        const OUString& rsName) const;
};

/** A PaneStyle describes how a pane is rendered.
*/
class PaneStyle
{
public:
    PaneStyle (void);
    ~PaneStyle (void);

    const SharedBitmapDescriptor GetBitmap (const OUString& sBitmapName) const;

    OUString msStyleName;
    ::boost::shared_ptr<PaneStyle> mpParentStyle;
    PresenterTheme::SharedFontDescriptor mpFont;
    BorderSize maInnerBorderSize;
    BorderSize maOuterBorderSize;
    ::boost::shared_ptr<PresenterBitmapContainer> mpBitmaps;

    PresenterTheme::SharedFontDescriptor GetFont (void) const;

private:

    void UpdateBorderSize (BorderSize& rBorderSize, bool bInner);
};

typedef ::boost::shared_ptr<PaneStyle> SharedPaneStyle;

class PaneStyleContainer : vector<SharedPaneStyle>
{
public:
    void Read (
        ReadContext& rReadContext,
        const Reference<container::XHierarchicalNameAccess>& rThemeRoot);

    SharedPaneStyle GetPaneStyle (const OUString& rsStyleName) const;

private:
    void ProcessPaneStyle (
        ReadContext& rReadContext,
        const ::rtl::OUString& rsKey,
        const ::std::vector<css::uno::Any>& rValues);
};

/** A ViewStyle describes how a view is displayed.
*/
class ViewStyle
{
public:
    ViewStyle (void);
    ~ViewStyle (void);

    const SharedBitmapDescriptor GetBitmap (const OUString& sBitmapName) const;

    PresenterTheme::SharedFontDescriptor GetFont (void) const;

    OUString msStyleName;
    ::boost::shared_ptr<ViewStyle> mpParentStyle;
    PresenterTheme::SharedFontDescriptor mpFont;
    ::boost::shared_ptr<PresenterBitmapContainer> mpBitmaps;
    SharedBitmapDescriptor mpBackground;
};

typedef ::boost::shared_ptr<ViewStyle> SharedViewStyle;

class ViewStyleContainer : vector<SharedViewStyle>
{
public:
    void Read (
        ReadContext& rReadContext,
        const Reference<container::XHierarchicalNameAccess>& rThemeRoot);

    SharedViewStyle GetViewStyle (const OUString& rsStyleName) const;

private:
    void ProcessViewStyle(
        ReadContext& rReadContext,
        const Reference<beans::XPropertySet>& rxProperties);
};

class ViewDescriptor
{
};
typedef ::boost::shared_ptr<ViewDescriptor> SharedViewDescriptor;
typedef ::std::vector<SharedViewDescriptor> ViewDescriptorContainer;

class StyleAssociationContainer
{
public:
    void Read (
        ReadContext& rReadContext,
        const Reference<container::XHierarchicalNameAccess>& rThemeRoot);

    OUString GetStyleName (const OUString& rsResourceName) const;

private:
    typedef map<OUString, OUString> StyleAssociations;
    StyleAssociations maStyleAssociations;

    void ProcessStyleAssociation(
        ReadContext& rReadContext,
        const ::rtl::OUString& rsKey,
        const ::std::vector<css::uno::Any>& rValues);
};

} // end of anonymous namespace

class PresenterTheme::Theme
{
public:
    Theme (
        const OUString& rsName,
        const Reference<container::XHierarchicalNameAccess>& rThemeRoot,
        const OUString& rsNodeName);
    ~Theme (void);

    void Read (
        PresenterConfigurationAccess& rConfiguration,
        ReadContext& rReadContext);

    OUString msThemeName;
    OUString msConfigurationNodeName;
    ::boost::shared_ptr<Theme> mpParentTheme;
    SharedBitmapDescriptor mpBackground;
    PaneStyleContainer maPaneStyles;
    ViewStyleContainer maViewStyles;
    ViewDescriptorContainer maViewDescriptors;
    StyleAssociationContainer maStyleAssociations;
    Reference<container::XHierarchicalNameAccess> mxThemeRoot;
    ::boost::shared_ptr<PresenterBitmapContainer> mpIconContainer;
    typedef map<rtl::OUString,SharedFontDescriptor> FontContainer;
    FontContainer maFontContainer;

    SharedPaneStyle GetPaneStyle (const OUString& rsStyleName) const;
    SharedViewStyle GetViewStyle (const OUString& rsStyleName) const;

private:
    void ProcessFont(
        ReadContext& rReadContext,
        const OUString& rsKey,
        const Reference<beans::XPropertySet>& rxProperties);
};

//===== PresenterTheme ========================================================

PresenterTheme::PresenterTheme (
    const css::uno::Reference<css::uno::XComponentContext>& rxContext,
    const rtl::OUString& rsThemeName,
    const css::uno::Reference<css::rendering::XCanvas>& rxCanvas)
    : mxContext(rxContext),
      msThemeName(rsThemeName),
      mpTheme(),
      mpBitmapContainer(),
      mxCanvas(rxCanvas)
{
    mpTheme = ReadTheme();
}

PresenterTheme::~PresenterTheme (void)
{
}

::boost::shared_ptr<PresenterTheme::Theme> PresenterTheme::ReadTheme (void)
{
    ReadContext aReadContext(mxContext, mxCanvas);

    PresenterConfigurationAccess aConfiguration (
        mxContext,
        OUString("/org.openoffice.Office.extension.PresenterScreen/"),
        PresenterConfigurationAccess::READ_ONLY);

    return aReadContext.ReadTheme(aConfiguration, msThemeName);
}

bool PresenterTheme::HasCanvas (void) const
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

OUString PresenterTheme::GetStyleName (const ::rtl::OUString& rsResourceURL) const
{
    OUString sStyleName;
    ::boost::shared_ptr<Theme> pTheme (mpTheme);
    while (sStyleName.isEmpty() && pTheme.get()!=NULL)
    {
        sStyleName = pTheme->maStyleAssociations.GetStyleName(rsResourceURL);
        pTheme = pTheme->mpParentTheme;
    }
    return sStyleName;
}

::std::vector<sal_Int32> PresenterTheme::GetBorderSize (
    const ::rtl::OUString& rsStyleName,
    const bool bOuter) const
{
    OSL_ASSERT(mpTheme.get() != NULL);

    SharedPaneStyle pPaneStyle (mpTheme->GetPaneStyle(rsStyleName));
    if (pPaneStyle.get() != NULL)
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
    const OUString& rsFontPath,
    const PresenterTheme::SharedFontDescriptor& rpDefault)
{
    return ReadContext::ReadFont(rxNode, rsFontPath, rpDefault);
}

bool PresenterTheme::ConvertToColor (
    const Any& rColorSequence,
    sal_uInt32& rColor)
{
    Sequence<sal_Int8> aByteSequence;
    if (rColorSequence >>= aByteSequence)
    {
        const sal_Int32 nByteCount (aByteSequence.getLength());
        const sal_uInt8* pArray = reinterpret_cast<const sal_uInt8*>(aByteSequence.getConstArray());
        rColor = 0;
        for (sal_Int32 nIndex=0; nIndex<nByteCount; ++nIndex)
        {
            rColor = (rColor << 8) | *pArray++;
        }
        return true;
    }
    else
        return false;
}

::boost::shared_ptr<PresenterConfigurationAccess> PresenterTheme::GetNodeForViewStyle (
    const ::rtl::OUString& rsStyleName) const
{
    if (mpTheme.get() == NULL)
        return ::boost::shared_ptr<PresenterConfigurationAccess>();

    // Open configuration for writing.
    ::boost::shared_ptr<PresenterConfigurationAccess> pConfiguration (
        new PresenterConfigurationAccess(
            mxContext,
            OUString("/org.openoffice.Office.extension.PresenterScreen/"),
            PresenterConfigurationAccess::READ_WRITE));

    // Get configuration node for the view style container of the current
    // theme.
    if (pConfiguration->GoToChild(
        A2S("Presenter/Themes/") + mpTheme->msConfigurationNodeName + A2S("/ViewStyles")))
    {
        pConfiguration->GoToChild(
            ::boost::bind(&PresenterConfigurationAccess::IsStringPropertyEqual,
                rsStyleName,
                A2S("StyleName"),
                _2));
    }
    return pConfiguration;
}

SharedBitmapDescriptor PresenterTheme::GetBitmap (
    const OUString& rsStyleName,
    const OUString& rsBitmapName) const
{
    if (mpTheme.get() != NULL)
    {
        if (rsStyleName.isEmpty())
        {
            if (rsBitmapName == A2S("Background"))
            {
                ::boost::shared_ptr<Theme> pTheme (mpTheme);
                while (pTheme.get()!=NULL && pTheme->mpBackground.get()==NULL)
                    pTheme = pTheme->mpParentTheme;
                if (pTheme.get() != NULL)
                    return pTheme->mpBackground;
                else
                    return SharedBitmapDescriptor();
            }
        }
        else
        {
            SharedPaneStyle pPaneStyle (mpTheme->GetPaneStyle(rsStyleName));
            if (pPaneStyle.get() != NULL)
            {
                SharedBitmapDescriptor pBitmap (pPaneStyle->GetBitmap(rsBitmapName));
                if (pBitmap.get() != NULL)
                    return pBitmap;
            }

            SharedViewStyle pViewStyle (mpTheme->GetViewStyle(rsStyleName));
            if (pViewStyle.get() != NULL)
            {
                SharedBitmapDescriptor pBitmap (pViewStyle->GetBitmap(rsBitmapName));
                if (pBitmap.get() != NULL)
                    return pBitmap;
            }
        }
    }

    return SharedBitmapDescriptor();
}

SharedBitmapDescriptor PresenterTheme::GetBitmap (
    const OUString& rsBitmapName) const
{
    if (mpTheme.get() != NULL)
    {
        if (rsBitmapName == A2S("Background"))
        {
            ::boost::shared_ptr<Theme> pTheme (mpTheme);
            while (pTheme.get()!=NULL && pTheme->mpBackground.get()==NULL)
                pTheme = pTheme->mpParentTheme;
            if (pTheme.get() != NULL)
                return pTheme->mpBackground;
            else
                return SharedBitmapDescriptor();
        }
        else
        {
            if (mpTheme->mpIconContainer.get() != NULL)
                return mpTheme->mpIconContainer->GetBitmap(rsBitmapName);
        }
    }

    return SharedBitmapDescriptor();
}

::boost::shared_ptr<PresenterBitmapContainer> PresenterTheme::GetBitmapContainer (void) const
{
    if (mpTheme.get() != NULL)
        return mpTheme->mpIconContainer;
    else
        return ::boost::shared_ptr<PresenterBitmapContainer>();
}

PresenterTheme::SharedFontDescriptor PresenterTheme::GetFont (
    const OUString& rsStyleName) const
{
    if (mpTheme.get() != NULL)
    {
        SharedPaneStyle pPaneStyle (mpTheme->GetPaneStyle(rsStyleName));
        if (pPaneStyle.get() != NULL)
            return pPaneStyle->GetFont();

        SharedViewStyle pViewStyle (mpTheme->GetViewStyle(rsStyleName));
        if (pViewStyle.get() != NULL)
            return pViewStyle->GetFont();

        ::boost::shared_ptr<Theme> pTheme (mpTheme);
        while (pTheme.get() != NULL)
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
    const ::boost::shared_ptr<FontDescriptor>& rpDescriptor)
    : msFamilyName(),
      msStyleName(),
      mnSize(12),
      mnColor(0x00000000),
      msAnchor(OUString("Left")),
      mnXOffset(0),
      mnYOffset(0)
{
    if (rpDescriptor.get() != NULL)
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
        aFontRequest.FontDescription.FamilyName = A2S("Tahoma");
    aFontRequest.FontDescription.StyleName = msStyleName;
    aFontRequest.CellSize = nCellSize;

    // Make an attempt at translating the style name(s)into a corresponding
    // font description.
    if (msStyleName == A2S("Bold"))
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

    geometry::RealRectangle2D aBox (PresenterCanvasHelper::GetTextBoundingBox (xFont, A2S("X")));

    const double nAscent (-aBox.Y1);
    const double nDescent (aBox.Y2);
    const double nScale = (nAscent+nDescent) / nAscent;
    return nDesignSize * nScale;
}

//===== Theme =================================================================

PresenterTheme::Theme::Theme (
    const OUString& rsName,
    const Reference<container::XHierarchicalNameAccess>& rxThemeRoot,
    const OUString& rsNodeName)
    : msThemeName(rsName),
      msConfigurationNodeName(rsNodeName),
      mpParentTheme(),
      maPaneStyles(),
      maViewStyles(),
      maStyleAssociations(),
      mxThemeRoot(rxThemeRoot),
      mpIconContainer()
{
}

PresenterTheme::Theme::~Theme (void)
{
}

void PresenterTheme::Theme::Read (
    PresenterConfigurationAccess& rConfiguration,
    ReadContext& rReadContext)
{
    PresenterConfigurationAccess::GetConfigurationNode(mxThemeRoot, A2S("ThemeName"))
        >>= msThemeName;

    // Parent theme name.
    OUString sParentThemeName;
    if ((PresenterConfigurationAccess::GetConfigurationNode(mxThemeRoot, A2S("ParentTheme"))
            >>= sParentThemeName)
        && !sParentThemeName.isEmpty())
    {
        mpParentTheme = rReadContext.ReadTheme(rConfiguration, sParentThemeName);
    }

    // Read the extension that contains the bitmaps referenced in this
    // theme.
    OUString sBitmapSourceExtension;
    if ((PresenterConfigurationAccess::GetConfigurationNode(
        mxThemeRoot, A2S("BitmapSourceExtension")) >>= sBitmapSourceExtension)
        && !sBitmapSourceExtension.isEmpty())
    {
        rReadContext.SetBitmapSourceExtension(sBitmapSourceExtension);
    }
    else
    {
        rReadContext.SetBitmapSourceExtension(PresenterComponent::gsExtensionIdentifier);
    }

    // Background.
    mpBackground = PresenterBitmapContainer::LoadBitmap(
        mxThemeRoot,
        A2S("Background"),
        rReadContext.mxPresenterHelper,
        rReadContext.msBasePath,
        rReadContext.mxCanvas,
        SharedBitmapDescriptor());

    // Style associations.
    maStyleAssociations.Read(rReadContext, mxThemeRoot);

    // Pane styles.
    maPaneStyles.Read(rReadContext, mxThemeRoot);

    // View styles.
    maViewStyles.Read(rReadContext, mxThemeRoot);

    // Read bitmaps.
    mpIconContainer.reset(
        new PresenterBitmapContainer(
            Reference<container::XNameAccess>(
                PresenterConfigurationAccess::GetConfigurationNode(mxThemeRoot, A2S("Bitmaps")),
                UNO_QUERY),
            mpParentTheme.get()!=NULL
                ? mpParentTheme->mpIconContainer
                : ::boost::shared_ptr<PresenterBitmapContainer>(),
            rReadContext.mxComponentContext,
            rReadContext.mxCanvas,
            rReadContext.msBasePath));

    // Read fonts.
    Reference<container::XNameAccess> xFontNode(
        PresenterConfigurationAccess::GetConfigurationNode(mxThemeRoot, A2S("Fonts")),
        UNO_QUERY);
    PresenterConfigurationAccess::ForAll(
        xFontNode,
        ::boost::bind(&PresenterTheme::Theme::ProcessFont,
            this, ::boost::ref(rReadContext), _1, _2));
}

SharedPaneStyle PresenterTheme::Theme::GetPaneStyle (const OUString& rsStyleName) const
{
    SharedPaneStyle pPaneStyle (maPaneStyles.GetPaneStyle(rsStyleName));
    if (pPaneStyle.get() != NULL)
        return pPaneStyle;
    else if (mpParentTheme.get() != NULL)
        return mpParentTheme->GetPaneStyle(rsStyleName);
    else
        return SharedPaneStyle();
}

SharedViewStyle PresenterTheme::Theme::GetViewStyle (const OUString& rsStyleName) const
{
    SharedViewStyle pViewStyle (maViewStyles.GetViewStyle(rsStyleName));
    if (pViewStyle.get() != NULL)
        return pViewStyle;
    else if (mpParentTheme.get() != NULL)
        return mpParentTheme->GetViewStyle(rsStyleName);
    else
        return SharedViewStyle();
}

void PresenterTheme::Theme::ProcessFont(
    ReadContext& rReadContext,
    const OUString& rsKey,
    const Reference<beans::XPropertySet>& rxProperties)
{
    (void)rReadContext;
    maFontContainer[rsKey] = ReadContext::ReadFont(rxProperties, SharedFontDescriptor());
}

namespace {

//===== ReadContext ===========================================================

ReadContext::ReadContext (
    const css::uno::Reference<css::uno::XComponentContext>& rxContext,
    const Reference<rendering::XCanvas>& rxCanvas)
    : mxComponentContext(rxContext),
      mxCanvas(rxCanvas),
      mxPresenterHelper(),
      msBasePath()
{
    Reference<lang::XMultiComponentFactory> xFactory (rxContext->getServiceManager());
    if (xFactory.is())
    {
        mxPresenterHelper = Reference<drawing::XPresenterHelper>(
            xFactory->createInstanceWithContext(
                OUString("com.sun.star.comp.Draw.PresenterHelper"),
                rxContext),
            UNO_QUERY_THROW);
    }

    // Get base path to bitmaps.
    SetBitmapSourceExtension(PresenterComponent::gsExtensionIdentifier);
}

ReadContext::~ReadContext (void)
{
}

PresenterTheme::SharedFontDescriptor ReadContext::ReadFont (
    const Reference<container::XHierarchicalNameAccess>& rxNode,
    const OUString& rsFontPath,
    const PresenterTheme::SharedFontDescriptor& rpDefault)
{
    if ( ! rxNode.is())
        return PresenterTheme::SharedFontDescriptor();

    try
    {
        Reference<container::XHierarchicalNameAccess> xFont (
            PresenterConfigurationAccess::GetConfigurationNode(
                rxNode,
                rsFontPath),
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
    ::boost::shared_ptr<PresenterTheme::FontDescriptor> pDescriptor (
        new PresenterTheme::FontDescriptor(rpDefault));

    PresenterConfigurationAccess::GetProperty(rxProperties, A2S("FamilyName")) >>= pDescriptor->msFamilyName;
    PresenterConfigurationAccess::GetProperty(rxProperties, A2S("Style")) >>= pDescriptor->msStyleName;
    PresenterConfigurationAccess::GetProperty(rxProperties, A2S("Size")) >>= pDescriptor->mnSize;
    PresenterTheme::ConvertToColor(
        PresenterConfigurationAccess::GetProperty(rxProperties, A2S("Color")),
        pDescriptor->mnColor);
    PresenterConfigurationAccess::GetProperty(rxProperties, A2S("Anchor")) >>= pDescriptor->msAnchor;
    PresenterConfigurationAccess::GetProperty(rxProperties, A2S("XOffset")) >>= pDescriptor->mnXOffset;
    PresenterConfigurationAccess::GetProperty(rxProperties, A2S("YOffset")) >>= pDescriptor->mnYOffset;

    return pDescriptor;
}

Any ReadContext::GetByName (
    const Reference<container::XNameAccess>& rxNode,
    const OUString& rsName) const
{
    OSL_ASSERT(rxNode.is());
    if (rxNode->hasByName(rsName))
        return rxNode->getByName(rsName);
    else
        return Any();
}

::boost::shared_ptr<PresenterTheme::Theme> ReadContext::ReadTheme (
    PresenterConfigurationAccess& rConfiguration,
    const OUString& rsThemeName)
{
    ::boost::shared_ptr<PresenterTheme::Theme> pTheme;

    OUString sCurrentThemeName (rsThemeName);
     if (sCurrentThemeName.isEmpty())
     {
         // No theme name given.  Look up the CurrentTheme property.
         rConfiguration.GetConfigurationNode(A2S("Presenter/CurrentTheme")) >>= sCurrentThemeName;
         if (sCurrentThemeName.isEmpty())
         {
             // Still no name.  Use "DefaultTheme".
             sCurrentThemeName = A2S("DefaultTheme");
         }
     }

    Reference<container::XNameAccess> xThemes (
        rConfiguration.GetConfigurationNode(A2S("Presenter/Themes")),
        UNO_QUERY);
    if (xThemes.is())
    {
        // Iterate over all themes and search the one with the given name.
        Sequence<OUString> aKeys (xThemes->getElementNames());
        for (sal_Int32 nItemIndex=0; nItemIndex < aKeys.getLength(); ++nItemIndex)
        {
             const OUString& rsKey (aKeys[nItemIndex]);
            Reference<container::XHierarchicalNameAccess> xTheme (
                xThemes->getByName(rsKey), UNO_QUERY);
            if (xTheme.is())
            {
                OUString sThemeName;
                PresenterConfigurationAccess::GetConfigurationNode(xTheme, A2S("ThemeName"))
                    >>= sThemeName;
                if (sThemeName == sCurrentThemeName)
                {
                    pTheme.reset(new PresenterTheme::Theme(sThemeName,xTheme,rsKey));
                    break;
                }
            }
        }
    }

    if (pTheme.get() != NULL)
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
        GetByName(rxNode, A2S("Left")) >>= aBorderSize.mnLeft;
        GetByName(rxNode, A2S("Top")) >>= aBorderSize.mnTop;
        GetByName(rxNode, A2S("Right")) >>= aBorderSize.mnRight;
        GetByName(rxNode, A2S("Bottom")) >>= aBorderSize.mnBottom;
    }

    return aBorderSize;
}

void ReadContext::SetBitmapSourceExtension (const OUString& rsExtensionIdentifier)
{
    // Get base path to bitmaps.
    msBasePath = PresenterComponent::GetBasePath(mxComponentContext, rsExtensionIdentifier);
}

//===== PaneStyleContainer ====================================================

void PaneStyleContainer::Read (
    ReadContext& rReadContext,
    const Reference<container::XHierarchicalNameAccess>& rxThemeRoot)
{
    Reference<container::XNameAccess> xPaneStyleList (
        PresenterConfigurationAccess::GetConfigurationNode(
            rxThemeRoot,
            A2S("PaneStyles")),
        UNO_QUERY);
    if (xPaneStyleList.is())
    {
        ::std::vector<rtl::OUString> aProperties;
        aProperties.reserve(6);
        aProperties.push_back(A2S("StyleName"));
        aProperties.push_back(A2S("ParentStyle"));
        aProperties.push_back(A2S("TitleFont"));
        aProperties.push_back(A2S("InnerBorderSize"));
        aProperties.push_back(A2S("OuterBorderSize"));
        aProperties.push_back(A2S("BorderBitmapList"));
        PresenterConfigurationAccess::ForAll(
            xPaneStyleList,
            aProperties,
            ::boost::bind(&PaneStyleContainer::ProcessPaneStyle,
                this, ::boost::ref(rReadContext), _1, _2));
    }
}

void PaneStyleContainer::ProcessPaneStyle(
    ReadContext& rReadContext,
    const OUString& rsKey,
    const ::std::vector<Any>& rValues)
{
    (void)rsKey;

    if (rValues.size() != 6)
        return;

    ::boost::shared_ptr<PaneStyle> pStyle (new PaneStyle());

    rValues[0] >>= pStyle->msStyleName;

    OUString sParentStyleName;
    if (rValues[1] >>= sParentStyleName)
    {
        // Find parent style.
        PaneStyleContainer::const_iterator iStyle;
        for (iStyle=begin(); iStyle!=end(); ++iStyle)
            if ((*iStyle)->msStyleName.equals(sParentStyleName))
            {
                pStyle->mpParentStyle = *iStyle;
                break;
            }
    }

    Reference<container::XHierarchicalNameAccess> xFontNode (rValues[2], UNO_QUERY);
    pStyle->mpFont = rReadContext.ReadFont(
        xFontNode, A2S(""), PresenterTheme::SharedFontDescriptor());

    Reference<container::XNameAccess> xInnerBorderSizeNode (rValues[3], UNO_QUERY);
    pStyle->maInnerBorderSize = rReadContext.ReadBorderSize(xInnerBorderSizeNode);
    Reference<container::XNameAccess> xOuterBorderSizeNode (rValues[4], UNO_QUERY);
    pStyle->maOuterBorderSize = rReadContext.ReadBorderSize(xOuterBorderSizeNode);

    if (pStyle->mpParentStyle.get() != NULL)
    {
        pStyle->maInnerBorderSize.Merge(pStyle->mpParentStyle->maInnerBorderSize);
        pStyle->maOuterBorderSize.Merge(pStyle->mpParentStyle->maOuterBorderSize);
    }

    if (rReadContext.mxCanvas.is())
    {
        Reference<container::XNameAccess> xBitmapsNode (rValues[5], UNO_QUERY);
        pStyle->mpBitmaps.reset(new PresenterBitmapContainer(
            xBitmapsNode,
            pStyle->mpParentStyle.get()!=NULL
                ? pStyle->mpParentStyle->mpBitmaps
                : ::boost::shared_ptr<PresenterBitmapContainer>(),
            rReadContext.mxComponentContext,
            rReadContext.mxCanvas,
            rReadContext.msBasePath,
            rReadContext.mxPresenterHelper));
    }

    push_back(pStyle);
}

SharedPaneStyle PaneStyleContainer::GetPaneStyle (const OUString& rsStyleName) const
{
    const_iterator iEnd (end());
    for (const_iterator iStyle=begin(); iStyle!=iEnd; ++iStyle)
        if ((*iStyle)->msStyleName == rsStyleName)
            return *iStyle;
    return SharedPaneStyle();
}

//===== PaneStyle =============================================================

PaneStyle::PaneStyle (void)
    : msStyleName(),
      mpParentStyle(),
      mpFont(),
      maInnerBorderSize(),
      maOuterBorderSize(),
      mpBitmaps()
{
}

PaneStyle::~PaneStyle (void)
{
}

void PaneStyle::UpdateBorderSize (BorderSize& rBorderSize, bool bInner)
{
    if (mpParentStyle.get() != NULL)
        mpParentStyle->UpdateBorderSize(rBorderSize, bInner);

    BorderSize& rThisBorderSize (bInner ? maInnerBorderSize : maOuterBorderSize);
    if (rThisBorderSize.mnLeft >= 0)
        rBorderSize.mnLeft = rThisBorderSize.mnLeft;
    if (rThisBorderSize.mnTop >= 0)
        rBorderSize.mnTop = rThisBorderSize.mnTop;
    if (rThisBorderSize.mnRight >= 0)
        rBorderSize.mnRight = rThisBorderSize.mnRight;
    if (rThisBorderSize.mnBottom >= 0)
        rBorderSize.mnBottom = rThisBorderSize.mnBottom;
}

const SharedBitmapDescriptor PaneStyle::GetBitmap (const OUString& rsBitmapName) const
{
    if (mpBitmaps.get() != NULL)
    {
        const SharedBitmapDescriptor pBitmap = mpBitmaps->GetBitmap(rsBitmapName);
        if (pBitmap.get() != NULL)
            return pBitmap;
    }

    if (mpParentStyle.get() != NULL)
        return mpParentStyle->GetBitmap(rsBitmapName);
    else
        return SharedBitmapDescriptor();
}

PresenterTheme::SharedFontDescriptor PaneStyle::GetFont (void) const
{
    if (mpFont.get() != NULL)
        return mpFont;
    else if (mpParentStyle.get() != NULL)
        return mpParentStyle->GetFont();
    else
        return PresenterTheme::SharedFontDescriptor();
}

//===== ViewStyleContainer ====================================================

void ViewStyleContainer::Read (
    ReadContext& rReadContext,
    const Reference<container::XHierarchicalNameAccess>& rxThemeRoot)
{
    (void)rReadContext;

    Reference<container::XNameAccess> xViewStyleList (
        PresenterConfigurationAccess::GetConfigurationNode(
            rxThemeRoot,
            A2S("ViewStyles")),
        UNO_QUERY);
    if (xViewStyleList.is())
    {
        PresenterConfigurationAccess::ForAll(
            xViewStyleList,
            ::boost::bind(&ViewStyleContainer::ProcessViewStyle,
                this, ::boost::ref(rReadContext), _2));
    }
}

void ViewStyleContainer::ProcessViewStyle(
    ReadContext& rReadContext,
    const Reference<beans::XPropertySet>& rxProperties)
{
    ::boost::shared_ptr<ViewStyle> pStyle (new ViewStyle());

    PresenterConfigurationAccess::GetProperty(rxProperties, A2S("StyleName"))
        >>= pStyle->msStyleName;

    OUString sParentStyleName;
    if (PresenterConfigurationAccess::GetProperty(rxProperties, A2S("ParentStyle"))
        >>= sParentStyleName)
    {
        // Find parent style.
        ViewStyleContainer::const_iterator iStyle;
        for (iStyle=begin(); iStyle!=end(); ++iStyle)
            if ((*iStyle)->msStyleName.equals(sParentStyleName))
            {
                pStyle->mpParentStyle = *iStyle;
                pStyle->mpFont = (*iStyle)->mpFont;
                pStyle->mpBackground = (*iStyle)->mpBackground;
                break;
            }
    }

    const OUString sPathToFont; // empty string
    Reference<container::XHierarchicalNameAccess> xFontNode (
        PresenterConfigurationAccess::GetProperty(rxProperties, A2S("Font")), UNO_QUERY);
    PresenterTheme::SharedFontDescriptor pFont (
        rReadContext.ReadFont(xFontNode, sPathToFont, PresenterTheme::SharedFontDescriptor()));
    if (pFont.get() != NULL)
        pStyle->mpFont = pFont;

    Reference<container::XHierarchicalNameAccess> xBackgroundNode (
        PresenterConfigurationAccess::GetProperty(rxProperties, A2S("Background")),
        UNO_QUERY);
    SharedBitmapDescriptor pBackground (PresenterBitmapContainer::LoadBitmap(
        xBackgroundNode,
        OUString(),
        rReadContext.mxPresenterHelper,
        rReadContext.msBasePath,
        rReadContext.mxCanvas,
        SharedBitmapDescriptor()));
    if (pBackground.get() != NULL && pBackground->GetNormalBitmap().is())
        pStyle->mpBackground = pBackground;

    push_back(pStyle);
}

SharedViewStyle ViewStyleContainer::GetViewStyle (const OUString& rsStyleName) const
{
    const_iterator iEnd (end());
    for (const_iterator iStyle=begin(); iStyle!=iEnd; ++iStyle)
        if ((*iStyle)->msStyleName == rsStyleName)
            return *iStyle;
    return SharedViewStyle();
}

//===== ViewStyle =============================================================

ViewStyle::ViewStyle (void)
    : msStyleName(),
      mpParentStyle(),
      mpFont(),
      mpBackground()
{
}

ViewStyle::~ViewStyle (void)
{
}

const SharedBitmapDescriptor ViewStyle::GetBitmap (const OUString& rsBitmapName) const
{
    if (rsBitmapName == A2S("Background"))
        return mpBackground;
    else
        return SharedBitmapDescriptor();
}

PresenterTheme::SharedFontDescriptor ViewStyle::GetFont (void) const
{
    if (mpFont.get() != NULL)
        return mpFont;
    else if (mpParentStyle.get() != NULL)
        return mpParentStyle->GetFont();
    else
        return PresenterTheme::SharedFontDescriptor();
}

//===== StyleAssociationContainer =============================================

void StyleAssociationContainer::Read (
    ReadContext& rReadContext,
    const Reference<container::XHierarchicalNameAccess>& rxThemeRoot)
{
    Reference<container::XNameAccess> xStyleAssociationList (
        PresenterConfigurationAccess::GetConfigurationNode(
            rxThemeRoot,
            A2S("StyleAssociations")),
        UNO_QUERY);
    if (xStyleAssociationList.is())
    {
        ::std::vector<rtl::OUString> aProperties (2);
        aProperties[0] = A2S("ResourceURL");
        aProperties[1] = A2S("StyleName");
        PresenterConfigurationAccess::ForAll(
            xStyleAssociationList,
            aProperties,
            ::boost::bind(&StyleAssociationContainer::ProcessStyleAssociation,
                this, ::boost::ref(rReadContext), _1, _2));
    }
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
    ReadContext& rReadContext,
    const OUString& rsKey,
    const ::std::vector<Any>& rValues)
{
    (void)rReadContext;
    (void)rsKey;

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

} } // end of namespace ::sdext::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
