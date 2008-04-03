/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterTheme.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:06:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "PresenterTheme.hxx"
#include "PresenterComponent.hxx"
#include "PresenterConfigurationAccess.hxx"
#include "PresenterHelper.hxx"
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/deployment/XPackageInformationProvider.hpp>
#include <com/sun/star/drawing/XPresenterHelper.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/rendering/XBitmap.hpp>
#include <com/sun/star/util/Color.hpp>
#include <boost/bind.hpp>
#include <map>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::std;
using ::rtl::OUString;

namespace {
OUString A2S (const sal_Char* pString) { return OUString::createFromAscii(pString); }
}

namespace sdext { namespace presenter {

namespace {

class BorderSize
{
public:
    BorderSize (void) : mnLeft(0),mnTop(0),mnRight(0),mnBottom(0) {}

    sal_Int32 mnLeft;
    sal_Int32 mnTop;
    sal_Int32 mnRight;
    sal_Int32 mnBottom;
};




/** Reading a theme from the configurations is done in various classes.  The
    ReadContext gives access to frequently used objects and functions to make
    the configuration handling easier.
*/
class ReadContext
{
public:
    PresenterConfigurationAccess& mrConfiguration;
    Reference<rendering::XCanvas> mxCanvas;
    ::boost::shared_ptr<PresenterTheme::PropertyMap> mpPropertyMap;
    Reference<drawing::XPresenterHelper> mxPresenterHelper;
    OUString msBasePath;

    ReadContext (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        PresenterConfigurationAccess& rConfiguration,
        const Reference<rendering::XCanvas>& rxCanvas);
    ~ReadContext (void);

    /** Read one bitmap from a file and store this and the other given data
        into a new BitmapDescriptor object.
    */
    ::boost::shared_ptr<PresenterTheme::BitmapDescriptor> ReadBitmap (
        const ::rtl::OUString& rsKey,
        const ::rtl::OUString& rsFileName,
        const sal_Int32 nXOffset,
        const sal_Int32 nYOffset,
        const util::Color nColor);

    /** Read data describing a font from the node that can be reached from
        the given root via the given path.
        @param rsFontPath
            May be empty.
    */
    ::boost::shared_ptr<PresenterTheme::FontDescriptor> ReadFont (
        const css::uno::Reference<css::container::XHierarchicalNameAccess>& rxTheme,
        const ::rtl::OUString& rsFontPath);

    /** Read a single bitmap, that typically is not member of a set, and
        return a new BitmapDescriptor with the data describing the bitmap.
    */
    ::boost::shared_ptr<PresenterTheme::BitmapDescriptor> ReadSingleBitmap (
        const ::rtl::OUString& rsKey,
        const css::uno::Reference<css::container::XHierarchicalNameAccess>& rxNode,
        const ::rtl::OUString& rsRelativePath);

    /** Read a set of bitmaps from the given node and return a new
        BitmapContainer that contains them.
    */
    ::boost::shared_ptr<PresenterTheme::BitmapContainer> ReadBorderBitmaps (
        const css::uno::Reference<css::container::XHierarchicalNameAccess>& rxNode);

    ::boost::shared_ptr<PresenterTheme::Theme> ReadTheme (
        const OUString& rsThemeName);

    BorderSize ReadBorderSize (const Reference<container::XNameAccess>& rxNode);

private:
    /** Used by ReadBorderBitmaps to read a single bitmap.
        @param rpContainer
            The container to insert the new bitmap into.
        @param rsKey
            The key of the configuration node that contains the bitmap data.
        @param rValues
            A set of Anys that contain Strings, nodes, and other data that
            describe a bitmap.
    */
    void ProcessBitmap (
        const ::boost::shared_ptr<PresenterTheme::BitmapContainer>& rpContainer,
        const ::rtl::OUString& rsKey,
        const ::std::vector<css::uno::Any>& rValues);
};




/** A PaneStyle describes how a pane is rendered.
*/
class PaneStyle
{
public:
    PaneStyle (void);
    ~PaneStyle (void);

    void AddProperties (
        const OUString& rsPropertyPrefix,
        const ::boost::shared_ptr<PresenterTheme::PropertyMap>& rpPropertyMap);

    OUString msStyleName;
    ::boost::shared_ptr<PaneStyle> mpParentStyle;
    ::boost::shared_ptr<PresenterTheme::FontDescriptor> mpFont;
    BorderSize maInnerBorderSize;
    BorderSize maOuterBorderSize;
    ::boost::shared_ptr<PresenterTheme::BitmapContainer> mpBitmaps;
};




class PaneStyleContainer : vector<boost::shared_ptr<PaneStyle> >
{
public:
    void Read (
        ReadContext& rReadContext,
        const Reference<container::XHierarchicalNameAccess>& rThemeRoot);
    void AddProperties (
        const OUString& rsPropertyPrefix,
        const ::boost::shared_ptr<PresenterTheme::PropertyMap>& rpPropertyMap);


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

    void AddProperties (
        const OUString& rsPropertyPrefix,
        const ::boost::shared_ptr<PresenterTheme::PropertyMap>& rpPropertyMap);

    OUString msStyleName;
    ::boost::shared_ptr<ViewStyle> mpParentStyle;
    ::boost::shared_ptr<PresenterTheme::FontDescriptor> mpFont;
    ::boost::shared_ptr<PresenterTheme::BitmapDescriptor> mpBackground;
};




class ViewStyleContainer : vector<boost::shared_ptr<ViewStyle> >
{
public:
    void Read (
        ReadContext& rReadContext,
        const Reference<container::XHierarchicalNameAccess>& rThemeRoot);
    void AddProperties (
        const OUString& rsPropertyPrefix,
        const ::boost::shared_ptr<PresenterTheme::PropertyMap>& rpPropertyMap);

private:
    void ProcessViewStyle(
        ReadContext& rReadContext,
        const ::rtl::OUString& rsKey,
        const ::std::vector<css::uno::Any>& rValues);
};




class StyleAssociation
{
public:
    StyleAssociation (
        const OUString& rsResourceURL,
        const OUString& rsStyleName);
    ~StyleAssociation (void);
    OUString msResourceURL;
    OUString msStyleName;
};




class StyleAssociationContainer : vector<StyleAssociation>
{
public:
    void Read (
        ReadContext& rReadContext,
        const Reference<container::XHierarchicalNameAccess>& rThemeRoot);
    void AddProperties (
        const OUString& rsPropertyPrefix,
        const ::boost::shared_ptr<PresenterTheme::PropertyMap>& rpPropertyMap);

private:
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
        const Reference<container::XHierarchicalNameAccess>& rThemeRoot);
    ~Theme (void);

    void Read (
        ReadContext& rReadContext);
    void AddProperties (
        const OUString& rsPropertyPrefix,
        const ::boost::shared_ptr<PresenterTheme::PropertyMap>& rpPropertyMap);

private:
    OUString msThemeName;
    ::boost::shared_ptr<Theme> mpParentTheme;
    ::boost::shared_ptr<BitmapDescriptor> mpBackground;
    PaneStyleContainer maPaneStyles;
    ViewStyleContainer maViewStyles;
    StyleAssociationContainer maStyleAssociations;
    Reference<container::XHierarchicalNameAccess> mxThemeRoot;
};




class PresenterTheme::BitmapDescriptor
{
public:
    explicit BitmapDescriptor (void);
    void SetBitmap (const Reference<rendering::XBitmap>& rxBitmap);

    sal_Int32 mnWidth;
    sal_Int32 mnHeight;
    sal_Int32 mnXOffset;
    sal_Int32 mnYOffset;
    util::Color maReplacementColor;
    uno::Reference<rendering::XBitmap> mxBitmap;

    void AddProperties (
        const OUString& rsPropertyPrefix,
        const ::boost::shared_ptr<PresenterTheme::PropertyMap>& rpPropertyMap);
};




class PresenterTheme::BitmapContainer
    : public ::std::map<OUString,::boost::shared_ptr<BitmapDescriptor> >
{
public:
    BitmapContainer (void);

    void AddProperties (
        const OUString& rsPropertyPrefix,
        const ::boost::shared_ptr<PresenterTheme::PropertyMap>& rpPropertyMap);
};




typedef ::boost::function<Any(const OUString&)> PropertyProviderFunction;

class PresenterTheme::PropertyMap : public ::std::map<OUString,Any>
{
public:
    PropertyMap (void);

    void Set (const OUString& rsKey, const Any& rValue);
};




class PresenterTheme::FontDescriptor
{
public:
    FontDescriptor (void);
    ::rtl::OUString msName;
    sal_Int32 mnSize;
    sal_uInt32 mnColor;
    ::rtl::OUString msAnchor;

    void AddProperties (
        const OUString& rsPropertyPrefix,
        const ::boost::shared_ptr<PresenterTheme::PropertyMap>& rpPropertyMap);
};




//===== PresenterTheme ========================================================

PresenterTheme::PresenterTheme (
    const css::uno::Reference<css::uno::XComponentContext>& rxContext,
    const rtl::OUString& rsThemeName,
    const css::uno::Reference<css::rendering::XCanvas>& rxCanvas)
    : mxContext(rxContext),
      msThemeName(rsThemeName),
      mpBitmapContainer(new BitmapContainer()),
      mxCanvas(rxCanvas),
      mpPropertyMap(new PropertyMap())
{
    ReadTheme();
}




PresenterTheme::~PresenterTheme (void)
{
}




void SAL_CALL PresenterTheme::disposing (void)
{
}




void PresenterTheme::ReadTheme (void)
{
    PresenterConfigurationAccess aConfiguration (
        mxContext,
        OUString::createFromAscii("/org.openoffice.Office.extension.PresenterScreen/"),
        PresenterConfigurationAccess::READ_ONLY);

    // Read the theme and properties.
    ReadContext aReadContext(mxContext, aConfiguration, mxCanvas);
    ::boost::shared_ptr<Theme> pTheme (aReadContext.ReadTheme(msThemeName));
    if (pTheme.get() != NULL)
    {
        pTheme->Read(aReadContext);
        pTheme->AddProperties(msThemeName, mpPropertyMap);
    }
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




Any PresenterTheme::getPropertyValue (const OUString& rsPropertyName)
{
    PropertyMap::const_iterator iProperty (mpPropertyMap->find(rsPropertyName));
    if (iProperty != mpPropertyMap->end())
        return iProperty->second;
    else
        throw beans::UnknownPropertyException();
}




OUString PresenterTheme::GetPaneStyle (const ::rtl::OUString& rsResourceURL) const
{
    OUString sStyleName (PresenterHelper::msDefaultPaneStyle);
    PropertyMap::const_iterator iProperty (mpPropertyMap->find(rsResourceURL));
    if (iProperty != mpPropertyMap->end())
        iProperty->second >>= sStyleName;
    return sStyleName;
}




OUString PresenterTheme::GetViewStyle (const ::rtl::OUString& rsResourceURL) const
{
    OUString sStyleName (PresenterHelper::msDefaultViewStyle);
    PropertyMap::const_iterator iProperty (mpPropertyMap->find(rsResourceURL));
    if (iProperty != mpPropertyMap->end())
        iProperty->second >>= sStyleName;
    return sStyleName;
}




//===== BitmapDescriptor ======================================================

PresenterTheme::BitmapDescriptor::BitmapDescriptor (void)
    : mnWidth(0),
      mnHeight(0),
      mnXOffset(0),
      mnYOffset(0),
      maReplacementColor(0x00000000),
      mxBitmap()
{
}




void PresenterTheme::BitmapDescriptor::SetBitmap (const Reference<rendering::XBitmap>& rxBitmap)
{
    mxBitmap = rxBitmap;
    if (mxBitmap.is())
    {
        mnWidth = mxBitmap->getSize().Width;
        mnHeight = mxBitmap->getSize().Height;
    }
    else
    {
        mnWidth = 0;
        mnHeight = 0;
    }
}




void PresenterTheme::BitmapDescriptor::AddProperties (
    const OUString& rsPropertyPrefix,
    const ::boost::shared_ptr<PresenterTheme::PropertyMap>& rpPropertyMap)
{
    rpPropertyMap->Set(rsPropertyPrefix + A2S("Bitmap"), Any(mxBitmap));
    rpPropertyMap->Set(rsPropertyPrefix + A2S("Offset"), Any(awt::Point(mnXOffset,mnYOffset)));
    rpPropertyMap->Set(rsPropertyPrefix + A2S("Color"), Any(maReplacementColor));
}




//===== BitmapContainer =======================================================

PresenterTheme::BitmapContainer::BitmapContainer (void)
{
}




void PresenterTheme::BitmapContainer::AddProperties (
    const OUString& rsPropertyPrefix,
    const ::boost::shared_ptr<PresenterTheme::PropertyMap>& rpPropertyMap)
{
    const_iterator iDescriptor;
    for (iDescriptor=begin(); iDescriptor!=end(); ++iDescriptor)
    {
        if (iDescriptor->second.get() == NULL)
        {
            OSL_ASSERT(false);
            continue;
        }
        iDescriptor->second->AddProperties(
            rsPropertyPrefix+iDescriptor->first+A2S("_"),
            rpPropertyMap);
    }
}




//===== PropertyMap ===========================================================

PresenterTheme::PropertyMap::PropertyMap (void)
{
}




void PresenterTheme::PropertyMap::Set (const OUString& rsKey, const Any& rValue)
{
    iterator iItem (find(rsKey));
    if (iItem == end())
        insert(value_type(rsKey,rValue));
    else
        iItem->second = rValue;
}




//===== FontDescriptor ========================================================

PresenterTheme::FontDescriptor::FontDescriptor (void)
    : msName(OUString::createFromAscii("Albany")),
      mnSize(12),
      mnColor(0x00000000),
      msAnchor(OUString::createFromAscii("Left"))
{
}




void PresenterTheme::FontDescriptor::AddProperties (
    const OUString& rsPropertyPrefix,
    const ::boost::shared_ptr<PresenterTheme::PropertyMap>& rpPropertyMap)
{
    rpPropertyMap->Set(rsPropertyPrefix+A2S("Name"), Any(msName));
    rpPropertyMap->Set(rsPropertyPrefix+A2S("Size"), Any(mnSize));
    rpPropertyMap->Set(rsPropertyPrefix+A2S("Color"), Any(mnColor));
    rpPropertyMap->Set(rsPropertyPrefix+A2S("Anchor"), Any(msAnchor));
}




//===== Theme =================================================================

PresenterTheme::Theme::Theme (
    const OUString& rsName,
    const Reference<container::XHierarchicalNameAccess>& rxThemeRoot)
    : msThemeName(rsName),
      mpParentTheme(),
      maPaneStyles(),
      maViewStyles(),
      maStyleAssociations(),
      mxThemeRoot(rxThemeRoot)
{
}




PresenterTheme::Theme::~Theme (void)
{
}




void PresenterTheme::Theme::Read (ReadContext& rReadContext)
{
    PresenterConfigurationAccess::GetConfigurationNode(mxThemeRoot, A2S("ThemeName"))
        >>= msThemeName;

    // Parent theme name.
    OUString sParentThemeName;
    if ((PresenterConfigurationAccess::GetConfigurationNode(mxThemeRoot, A2S("ParentTheme"))
            >>= sParentThemeName)
        && sParentThemeName.getLength()>0)
    {
        mpParentTheme = rReadContext.ReadTheme(sParentThemeName);
    }

    // Background.
    mpBackground
        = rReadContext.ReadSingleBitmap(A2S("Background"), mxThemeRoot, A2S("Background"));

    // Style associations.
    maStyleAssociations.Read(rReadContext, mxThemeRoot);

    // Pane styles.
    maPaneStyles.Read(rReadContext, mxThemeRoot);

    // View styles.
    maViewStyles.Read(rReadContext, mxThemeRoot);

}




void PresenterTheme::Theme::AddProperties (
    const OUString& rsPropertyPrefix,
    const ::boost::shared_ptr<PresenterTheme::PropertyMap>& rpPropertyMap)
{
    if (mpParentTheme.get() != NULL)
        mpParentTheme->AddProperties(rsPropertyPrefix, rpPropertyMap);

    // Background.
    if (mpBackground.get() != NULL)
        mpBackground->AddProperties(rsPropertyPrefix+A2S("Background_"), rpPropertyMap);

    // Style associations.
    maStyleAssociations.AddProperties(rsPropertyPrefix, rpPropertyMap);

    // Pane styles.
    maPaneStyles.AddProperties(rsPropertyPrefix, rpPropertyMap);

    // View styles.
    maViewStyles.AddProperties(rsPropertyPrefix, rpPropertyMap);
}




namespace {

//===== ReadContext ===========================================================

ReadContext::ReadContext (
    const css::uno::Reference<css::uno::XComponentContext>& rxContext,
    PresenterConfigurationAccess& rConfiguration,
    const Reference<rendering::XCanvas>& rxCanvas)
    : mrConfiguration(rConfiguration),
      mxCanvas(rxCanvas),
      mxPresenterHelper(),
      msBasePath()
{
    Reference<lang::XMultiComponentFactory> xFactory (rxContext->getServiceManager());
    if (xFactory.is())
    {
        mxPresenterHelper = Reference<drawing::XPresenterHelper>(
            xFactory->createInstanceWithContext(
                OUString::createFromAscii("com.sun.star.comp.Draw.PresenterHelper"),
                rxContext),
            UNO_QUERY_THROW);
    }

    // Get base path to bitmaps.
    Reference<deployment::XPackageInformationProvider> xInformationProvider (
        rxContext->getValueByName(OUString::createFromAscii(
            "/singletons/com.sun.star.deployment.PackageInformationProvider")),
        UNO_QUERY_THROW);
    if (xInformationProvider.is())
        msBasePath = xInformationProvider->getPackageLocation(gsExtensionIdentifier);
    msBasePath += OUString::createFromAscii("/");
}




ReadContext::~ReadContext (void)
{
}




::boost::shared_ptr<PresenterTheme::BitmapDescriptor> ReadContext::ReadBitmap (
    const OUString& rsKey,
    const OUString& rsFileName,
    const sal_Int32 nXOffset,
    const sal_Int32 nYOffset,
    const util::Color nColor)
{
    (void)rsKey;

    ::boost::shared_ptr<PresenterTheme::BitmapDescriptor> pDescriptor(
        new PresenterTheme::BitmapDescriptor());

    if ( ! mxCanvas.is())
        return pDescriptor;

    if (mxPresenterHelper.is())
    {

        pDescriptor->mnXOffset = nXOffset;
        pDescriptor->mnYOffset = nYOffset;
        pDescriptor->maReplacementColor = nColor;
        pDescriptor->mxBitmap = mxPresenterHelper->loadBitmap(
            msBasePath + rsFileName,
            mxCanvas);
    }
    else
    {
        OSL_ASSERT(mxPresenterHelper.is());
    }
    return pDescriptor;
}




::boost::shared_ptr<PresenterTheme::FontDescriptor> ReadContext::ReadFont (
    const Reference<container::XHierarchicalNameAccess>& rxNode,
    const OUString& rsFontPath)
{
    ::boost::shared_ptr<PresenterTheme::FontDescriptor> pDescriptor (
        new PresenterTheme::FontDescriptor());

    if ( ! rxNode.is())
        return pDescriptor;

    try
    {
        Reference<container::XHierarchicalNameAccess> xFont (
            PresenterConfigurationAccess::GetConfigurationNode(
                rxNode,
                rsFontPath),
                UNO_QUERY_THROW);

        PresenterConfigurationAccess::GetConfigurationNode(
            xFont,
            OUString::createFromAscii("FamilyName")) >>= pDescriptor->msName;

        PresenterConfigurationAccess::GetConfigurationNode(
            xFont,
            OUString::createFromAscii("Size")) >>= pDescriptor->mnSize;

        PresenterConfigurationAccess::GetConfigurationNode(
            xFont,
            OUString::createFromAscii("Color")) >>= pDescriptor->mnColor;

        PresenterConfigurationAccess::GetConfigurationNode(
            xFont,
            OUString::createFromAscii("Anchor")) >>= pDescriptor->msAnchor;
    }
    catch (Exception&)
    {
        OSL_ASSERT(false);
    }

    return pDescriptor;
}




::boost::shared_ptr<PresenterTheme::BitmapContainer> ReadContext::ReadBorderBitmaps (
    const Reference<container::XHierarchicalNameAccess>& rxNode)
{
    ::boost::shared_ptr<PresenterTheme::BitmapContainer> pContainer (
        new PresenterTheme::BitmapContainer());
    Reference<container::XNameAccess> xBitmapList (rxNode, UNO_QUERY);
    ::std::vector<rtl::OUString> aProperties (4);
    aProperties[0] = OUString::createFromAscii("FileName");
    aProperties[1] = OUString::createFromAscii("XOffset");
    aProperties[2] = OUString::createFromAscii("YOffset");
    aProperties[3] = OUString::createFromAscii("ReplacementColor");
    PresenterConfigurationAccess::ForAll(
        xBitmapList,
        aProperties,
        ::boost::bind(&ReadContext::ProcessBitmap, this, pContainer, _1, _2));
    return pContainer;
}




void ReadContext::ProcessBitmap (
    const ::boost::shared_ptr<PresenterTheme::BitmapContainer>& rpContainer,
    const OUString& rsKey,
    const ::std::vector<Any>& rValues)
{
    if (rValues.size() != 4)
        return;

    OUString sFileName;
    sal_Int32 nXOffset (0);
    sal_Int32 nYOffset (0);
    sal_uInt32 nColor (0);
    rValues[0] >>= sFileName;
    rValues[1] >>= nXOffset;
    rValues[2] >>= nYOffset;
    rValues[3] >>= nColor;

    rpContainer->insert(
        PresenterTheme::BitmapContainer::value_type(
            rsKey,
                ReadBitmap(rsKey, sFileName, nXOffset, nYOffset, nColor)));
}




::boost::shared_ptr<PresenterTheme::BitmapDescriptor> ReadContext::ReadSingleBitmap (
    const OUString& rsKey,
    const Reference<container::XHierarchicalNameAccess>& rxTheme,
    const OUString& rsRelativeNodePath)
{
    if ( ! mxCanvas.is())
        return ::boost::shared_ptr<PresenterTheme::BitmapDescriptor>();

    Reference<container::XHierarchicalNameAccess> xBitmapNode(
        PresenterConfigurationAccess::GetConfigurationNode(
            rxTheme,
            rsRelativeNodePath),
        UNO_QUERY);
    if (xBitmapNode.is())
    {
        OUString sFileName;
        sal_Int32 nXOffset (0);
        sal_Int32 nYOffset (0);
        util::Color nColor (0);
        PresenterConfigurationAccess::GetConfigurationNode(xBitmapNode,A2S("FileName"))
            >>= sFileName;
        PresenterConfigurationAccess::GetConfigurationNode(xBitmapNode,A2S("XOffset"))
            >>= nXOffset;
        PresenterConfigurationAccess::GetConfigurationNode(xBitmapNode,A2S("YOffset"))
            >>= nYOffset;
        PresenterConfigurationAccess::GetConfigurationNode(xBitmapNode,A2S("ReplacementColor"))
            >>= nColor;
        return ReadBitmap(
            rsKey,
            sFileName,
            nXOffset,
            nYOffset,
            nColor);
    }
    return ::boost::shared_ptr<PresenterTheme::BitmapDescriptor>();
}




::boost::shared_ptr<PresenterTheme::Theme> ReadContext::ReadTheme (
    const OUString& rsThemeName)
{
    ::boost::shared_ptr<PresenterTheme::Theme> pTheme;

    OUString sCurrentThemeName (rsThemeName);
     if (sCurrentThemeName.getLength() == 0)
     {
         // No theme name given.  Look up the CurrentTheme property.
         mrConfiguration.GetConfigurationNode(A2S("Presenter/CurrentTheme")) >>= sCurrentThemeName;
         if (sCurrentThemeName.getLength() == 0)
         {
             // Still no name.  Use "DefaultTheme".
             sCurrentThemeName = A2S("DefaultTheme");
         }
     }

    Reference<container::XNameAccess> xThemes (
        mrConfiguration.GetConfigurationNode(OUString::createFromAscii("Presenter/Themes")),
        UNO_QUERY);
    if (xThemes.is())
    {
        // Iterate over all themes and search the one with the given name.
        Sequence<OUString> aKeys (xThemes->getElementNames());
        for (sal_Int32 nItemIndex=0; nItemIndex<aKeys.getLength(); ++nItemIndex)
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
                    pTheme.reset(new PresenterTheme::Theme(sThemeName,xTheme));
                    break;
                }
            }
        }
    }

    return pTheme;
}




BorderSize ReadContext::ReadBorderSize (const Reference<container::XNameAccess>& rxNode)
{
    BorderSize aBorderSize;

    if (rxNode.is())
    {
        Any aValue;
        rxNode->getByName(A2S("Left")) >>= aBorderSize.mnLeft;
        rxNode->getByName(A2S("Top")) >>= aBorderSize.mnTop;
        rxNode->getByName(A2S("Right")) >>= aBorderSize.mnRight;
        rxNode->getByName(A2S("Bottom")) >>= aBorderSize.mnBottom;
    }

    return aBorderSize;
}




//===== PaneStyleContainer ====================================================

void PaneStyleContainer::Read (
    ReadContext& rReadContext,
    const Reference<container::XHierarchicalNameAccess>& rxThemeRoot)
{
    Reference<container::XNameAccess> xPaneStyleList (
        PresenterConfigurationAccess::GetConfigurationNode(
            rxThemeRoot,
            OUString::createFromAscii("PaneStyles")),
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
                pStyle->mpParentStyle = *iStyle;
    }

    Reference<container::XHierarchicalNameAccess> xFontNode (rValues[2], UNO_QUERY);
    pStyle->mpFont = rReadContext.ReadFont(xFontNode, A2S(""));

    Reference<container::XNameAccess> xInnerBorderSizeNode (rValues[3], UNO_QUERY);
    pStyle->maInnerBorderSize = rReadContext.ReadBorderSize(xInnerBorderSizeNode);
    Reference<container::XNameAccess> xOuterBorderSizeNode (rValues[4], UNO_QUERY);
    pStyle->maOuterBorderSize = rReadContext.ReadBorderSize(xOuterBorderSizeNode);

    if (rReadContext.mxCanvas.is())
    {
        Reference<container::XHierarchicalNameAccess> xBitmapsNode (rValues[5], UNO_QUERY);
        ::boost::shared_ptr<PresenterTheme::BitmapContainer> pBitmaps;
        pBitmaps = rReadContext.ReadBorderBitmaps(xBitmapsNode);
        pStyle->mpBitmaps = pBitmaps;
    }

    push_back(pStyle);
}




void PaneStyleContainer::AddProperties (
    const OUString& rsPropertyPrefix,
    const ::boost::shared_ptr<PresenterTheme::PropertyMap>& rpPropertyMap)
{
    for (const_iterator iStyle=begin(); iStyle!=end(); ++iStyle)
        (*iStyle)->AddProperties(
            rsPropertyPrefix+(*iStyle)->msStyleName+A2S("_"),
            rpPropertyMap);
}




//===== PaneStyle =============================================================

PaneStyle::PaneStyle (void)
    : msStyleName(),
      mpFont(),
      maInnerBorderSize(),
      maOuterBorderSize(),
      mpBitmaps(new PresenterTheme::BitmapContainer())
{
}




PaneStyle::~PaneStyle (void)
{
}




void PaneStyle::AddProperties(
    const OUString& rsPropertyPrefix,
    const ::boost::shared_ptr<PresenterTheme::PropertyMap>& rpPropertyMap)
{
    if (mpParentStyle.get() != NULL)
        mpParentStyle->AddProperties(rsPropertyPrefix, rpPropertyMap);
    if (mpFont.get() != NULL)
        mpFont->AddProperties(rsPropertyPrefix+A2S("Font_"), rpPropertyMap);
    if (mpBitmaps.get() != NULL)
        mpBitmaps->AddProperties(rsPropertyPrefix+A2S("Border_"), rpPropertyMap);

    Sequence<sal_Int32> aBorderSize (4);
    aBorderSize[0] = maInnerBorderSize.mnLeft;
    aBorderSize[1] = maInnerBorderSize.mnTop;
    aBorderSize[2] = maInnerBorderSize.mnRight;
    aBorderSize[3] = maInnerBorderSize.mnBottom;
    rpPropertyMap->Set(
        rsPropertyPrefix+A2S("InnerBorderSize"),
        Any(aBorderSize));
    aBorderSize[0] = maOuterBorderSize.mnLeft;
    aBorderSize[1] = maOuterBorderSize.mnTop;
    aBorderSize[2] = maOuterBorderSize.mnRight;
    aBorderSize[3] = maOuterBorderSize.mnBottom;
    rpPropertyMap->Set(
        rsPropertyPrefix+A2S("OuterBorderSize"),
        Any(aBorderSize));
}




//===== ViewStyleContainer ====================================================

void ViewStyleContainer::Read (
    ReadContext& rReadContext,
    const Reference<container::XHierarchicalNameAccess>& rxThemeRoot)
{
    Reference<container::XNameAccess> xViewStyleList (
        PresenterConfigurationAccess::GetConfigurationNode(
            rxThemeRoot,
            OUString::createFromAscii("ViewStyles")),
        UNO_QUERY);
    if (xViewStyleList.is())
    {
        ::std::vector<rtl::OUString> aProperties (4);
        aProperties[0] = OUString::createFromAscii("StyleName");
        aProperties[1] = OUString::createFromAscii("ParentStyle");
        aProperties[2] = OUString::createFromAscii("Font");
        aProperties[3] = OUString::createFromAscii("Background");
        PresenterConfigurationAccess::ForAll(
            xViewStyleList,
            aProperties,
            ::boost::bind(&ViewStyleContainer::ProcessViewStyle,
                this, ::boost::ref(rReadContext), _1, _2));
    }
}




void ViewStyleContainer::ProcessViewStyle(
    ReadContext& rReadContext,
    const OUString& rsKey,
    const ::std::vector<Any>& rValues)
{
    (void)rsKey;

    if (rValues.size() != 4)
        return;

    ::boost::shared_ptr<ViewStyle> pStyle (new ViewStyle());

    rValues[0] >>= pStyle->msStyleName;

    const OUString sPathToFont; // empty string
    Reference<container::XHierarchicalNameAccess> xFontNode (rValues[2], UNO_QUERY);
    pStyle->mpFont = rReadContext.ReadFont(xFontNode, sPathToFont);

    Reference<container::XHierarchicalNameAccess> xBackgroundNode (rValues[3], UNO_QUERY);
    pStyle->mpBackground = rReadContext.ReadSingleBitmap(
        A2S("Background"), xBackgroundNode, A2S(""));

    push_back(pStyle);
}




void ViewStyleContainer::AddProperties (
    const OUString& rsPropertyPrefix,
    const ::boost::shared_ptr<PresenterTheme::PropertyMap>& rpPropertyMap)
{
    for (const_iterator iStyle=begin(); iStyle!=end(); ++iStyle)
        (*iStyle)->AddProperties(
            rsPropertyPrefix+(*iStyle)->msStyleName+A2S("_"),
            rpPropertyMap);
}




//===== PaneStyle =============================================================

ViewStyle::ViewStyle (void)
    : msStyleName(),
      mpFont(),
      mpBackground()
{
}




ViewStyle::~ViewStyle (void)
{
}




void ViewStyle::AddProperties(
    const OUString& rsPropertyPrefix,
    const ::boost::shared_ptr<PresenterTheme::PropertyMap>& rpPropertyMap)
{
    if (mpParentStyle.get() != NULL)
        mpParentStyle->AddProperties(rsPropertyPrefix, rpPropertyMap);
    if (mpFont.get() != NULL)
        mpFont->AddProperties(rsPropertyPrefix+A2S("Font_"), rpPropertyMap);
    if (mpBackground.get() != NULL)
        mpBackground->AddProperties(rsPropertyPrefix+A2S("Background_"), rpPropertyMap);
}




//===== StyleAssociationContainer =============================================

void StyleAssociationContainer::Read (
    ReadContext& rReadContext,
    const Reference<container::XHierarchicalNameAccess>& rxThemeRoot)
{
    Reference<container::XNameAccess> xStyleAssociationList (
        PresenterConfigurationAccess::GetConfigurationNode(
            rxThemeRoot,
            OUString::createFromAscii("StyleAssociations")),
        UNO_QUERY);
    if (xStyleAssociationList.is())
    {
        ::std::vector<rtl::OUString> aProperties (2);
        aProperties[0] = OUString::createFromAscii("ResourceURL");
        aProperties[1] = OUString::createFromAscii("StyleName");
        PresenterConfigurationAccess::ForAll(
            xStyleAssociationList,
            aProperties,
            ::boost::bind(&StyleAssociationContainer::ProcessStyleAssociation,
                this, ::boost::ref(rReadContext), _1, _2));
    }
}




void StyleAssociationContainer::AddProperties (
    const OUString& rsPropertyPrefix,
    const ::boost::shared_ptr<PresenterTheme::PropertyMap>& rpPropertyMap)
{
    (void)rsPropertyPrefix;
    for (const_iterator iAssociation=begin(); iAssociation!=end(); ++iAssociation)
        rpPropertyMap->Set(
            iAssociation->msResourceURL,
            Any(iAssociation->msStyleName));
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
        push_back(StyleAssociation(sResourceURL, sStyleName));
    }
}




//===== StyleAssociation ======================================================

StyleAssociation::StyleAssociation (
    const OUString& rsResourceURL,
    const OUString& rsStyleName)
    : msResourceURL(rsResourceURL),
      msStyleName(rsStyleName)
{
}




StyleAssociation::~StyleAssociation (void)
{
}




} // end of anonymous namespace

} } // end of namespace ::sdext::presenter
