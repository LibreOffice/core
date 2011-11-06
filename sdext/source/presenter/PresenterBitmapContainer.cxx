/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sdext.hxx"

#include "PresenterBitmapContainer.hxx"
#include "PresenterComponent.hxx"
#include "PresenterConfigurationAccess.hxx"

#include <com/sun/star/deployment/XPackageInformationProvider.hpp>
#include <com/sun/star/drawing/XPresenterHelper.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <boost/bind.hpp>
#include <map>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::std;
using ::rtl::OUString;

#define A2S(pString) (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(pString)))


namespace sdext { namespace presenter {

namespace {
static OUString gsNameProperty (A2S("Name"));
static OUString gsNormalFileNameProperty (A2S("NormalFileName"));
static OUString gsMouseOverFileNameProperty (A2S("MouseOverFileName"));
static OUString gsButtonDownFileNameProperty (A2S("ButtonDownFileName"));
static OUString gsDisabledFileNameProperty (A2S("DisabledFileName"));
static OUString gsMaskFileNameProperty (A2S("MaskFileName"));
static OUString gsXOffsetProperty (A2S("XOffset"));
static OUString gsYOffsetProperty (A2S("YOffset"));
static OUString gsXHotSpotProperty (A2S("XHotSpot"));
static OUString gsYHotSpotProperty (A2S("YHotSpot"));
static OUString gsReplacementColorProperty (A2S("ReplacementColor"));
static OUString gsHorizontalTexturingModeProperty (A2S("HorizontalTexturingMode"));
static OUString gsVerticalTexturingModeProperty (A2S("VerticalTexturingMode"));
}

//===== PresenterBitmapContainer ==============================================

PresenterBitmapContainer::PresenterBitmapContainer (
    const ::rtl::OUString& rsConfigurationBase,
    const ::boost::shared_ptr<PresenterBitmapContainer>& rpParentContainer,
    const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext,
    const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
    const OUString& rsBasePath,
    const css::uno::Reference<css::drawing::XPresenterHelper>& rxPresenterHelper)
    : mpParentContainer(rpParentContainer),
      maIconContainer(),
      msBasePath(rsBasePath),
      mxCanvas(rxCanvas),
      mxPresenterHelper(rxPresenterHelper)
{
    Initialize(rxComponentContext);

    // Get access to the configuration.
    PresenterConfigurationAccess aConfiguration (
        rxComponentContext,
        A2S("org.openoffice.Office.extension.PresenterScreen"),
        PresenterConfigurationAccess::READ_ONLY);
    Reference<container::XNameAccess> xBitmapList (
        aConfiguration.GetConfigurationNode(rsConfigurationBase),
        UNO_QUERY_THROW);

    LoadBitmaps(xBitmapList);
}





PresenterBitmapContainer::PresenterBitmapContainer (
    const css::uno::Reference<css::container::XNameAccess>& rxRootNode,
    const ::boost::shared_ptr<PresenterBitmapContainer>& rpParentContainer,
    const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext,
    const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
    const OUString& rsBasePath,
    const css::uno::Reference<css::drawing::XPresenterHelper>& rxPresenterHelper)
    : mpParentContainer(rpParentContainer),
      maIconContainer(),
      msBasePath(rsBasePath),
      mxCanvas(rxCanvas),
      mxPresenterHelper(rxPresenterHelper)
{
    Initialize(rxComponentContext);

    LoadBitmaps(rxRootNode);
}




void PresenterBitmapContainer::Initialize (
    const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext)
{
    if ( ! mxPresenterHelper.is())
    {
        // Create an object that is able to load the bitmaps in a format that is
        // supported by the canvas.
        Reference<lang::XMultiComponentFactory> xFactory (
            rxComponentContext->getServiceManager(), UNO_QUERY);
        if ( ! xFactory.is())
            return;
        mxPresenterHelper = Reference<drawing::XPresenterHelper>(
            xFactory->createInstanceWithContext(
                A2S("com.sun.star.drawing.PresenterHelper"),
                rxComponentContext),
            UNO_QUERY_THROW);
    }
}




PresenterBitmapContainer::~PresenterBitmapContainer (void)
{
    maIconContainer.clear();
}




SharedBitmapDescriptor PresenterBitmapContainer::GetBitmap (
    const OUString& rsName) const
{
    BitmapContainer::const_iterator iSet (maIconContainer.find(rsName));
    if (iSet != maIconContainer.end())
        return iSet->second;
    else if (mpParentContainer.get() != NULL)
        return mpParentContainer->GetBitmap(rsName);
    else
        return SharedBitmapDescriptor();
}




void PresenterBitmapContainer::LoadBitmaps (
    const css::uno::Reference<css::container::XNameAccess>& rxBitmapList)
{
    if ( ! mxCanvas.is())
        return;

    if ( ! rxBitmapList.is())
        return;

    try
    {
        // Load all button bitmaps.
        if (rxBitmapList.is())
        {
            PresenterConfigurationAccess::ForAll(
                rxBitmapList,
                ::boost::bind(&PresenterBitmapContainer::ProcessBitmap, this, _1, _2));
        }
    }
    catch (Exception&)
    {
        OSL_ASSERT(false);
    }
}




SharedBitmapDescriptor PresenterBitmapContainer::LoadBitmap (
    const css::uno::Reference<css::container::XHierarchicalNameAccess>& rxNode,
    const ::rtl::OUString& rsPath,
    const css::uno::Reference<css::drawing::XPresenterHelper>& rxPresenterHelper,
    const OUString& rsBasePath,
    const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
    const SharedBitmapDescriptor& rpDefault)
{
    SharedBitmapDescriptor pBitmap;

    if (rxNode.is())
    {
        try
        {
            Reference<beans::XPropertySet> xBitmapProperties (
                PresenterConfigurationAccess::GetConfigurationNode(rxNode, rsPath),
                UNO_QUERY);
            if (xBitmapProperties.is())
                pBitmap = LoadBitmap(
                    xBitmapProperties,
                    rxPresenterHelper,
                    rsBasePath,
                    rxCanvas,
                    rpDefault);
        }
        catch (Exception&)
        {
            OSL_ASSERT(false);
        }
    }

    return pBitmap;
}




void PresenterBitmapContainer::ProcessBitmap (
    const OUString& rsKey,
    const Reference<beans::XPropertySet>& rxProperties)
{
    OUString sName;
    if ( ! (PresenterConfigurationAccess::GetProperty(rxProperties, gsNameProperty) >>= sName))
        sName = rsKey;

    maIconContainer[sName] = LoadBitmap(
        rxProperties,
        mxPresenterHelper,
        msBasePath,
        mxCanvas,
        SharedBitmapDescriptor());
}




SharedBitmapDescriptor PresenterBitmapContainer::LoadBitmap (
    const Reference<beans::XPropertySet>& rxProperties,
    const css::uno::Reference<css::drawing::XPresenterHelper>& rxPresenterHelper,
    const OUString& rsBasePath,
    const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
    const SharedBitmapDescriptor& rpDefault)
{
    OSL_ASSERT(rxCanvas.is());
    OSL_ASSERT(rxPresenterHelper.is());

    SharedBitmapDescriptor pBitmap (new BitmapDescriptor(rpDefault));

    if ( ! rxProperties.is())
        return pBitmap;

    OUString sFileName;

    // Load bitmaps.
    if (PresenterConfigurationAccess::GetProperty(rxProperties, gsNormalFileNameProperty) >>= sFileName)
        try
        {
            pBitmap->SetBitmap(
                BitmapDescriptor::Normal,
                rxPresenterHelper->loadBitmap(rsBasePath + sFileName, rxCanvas));
        }
        catch (Exception&)
        {}
    if (PresenterConfigurationAccess::GetProperty(rxProperties, gsMouseOverFileNameProperty) >>= sFileName)
        try
        {
            pBitmap->SetBitmap(
                BitmapDescriptor::MouseOver,
                rxPresenterHelper->loadBitmap(rsBasePath + sFileName, rxCanvas));
        }
        catch (Exception&)
        {}
    if (PresenterConfigurationAccess::GetProperty(rxProperties, gsButtonDownFileNameProperty) >>= sFileName)
        try
        {
            pBitmap->SetBitmap(
                BitmapDescriptor::ButtonDown,
                rxPresenterHelper->loadBitmap(rsBasePath + sFileName, rxCanvas));
        }
        catch (Exception&)
        {}
    if (PresenterConfigurationAccess::GetProperty(rxProperties, gsDisabledFileNameProperty) >>= sFileName)
        try
        {
            pBitmap->SetBitmap(
                BitmapDescriptor::Disabled,
                rxPresenterHelper->loadBitmap(rsBasePath + sFileName, rxCanvas));
        }
        catch (Exception&)
        {}
    if (PresenterConfigurationAccess::GetProperty(rxProperties, gsMaskFileNameProperty) >>= sFileName)
        try
        {
            pBitmap->SetBitmap(
                BitmapDescriptor::Mask,
                rxPresenterHelper->loadBitmap(rsBasePath + sFileName, rxCanvas));
        }
        catch (Exception&)
        {}


    PresenterConfigurationAccess::GetProperty(rxProperties, gsXOffsetProperty) >>= pBitmap->mnXOffset;
    PresenterConfigurationAccess::GetProperty(rxProperties, gsYOffsetProperty) >>= pBitmap->mnYOffset;

    PresenterConfigurationAccess::GetProperty(rxProperties, gsXHotSpotProperty) >>= pBitmap->mnXHotSpot;
    PresenterConfigurationAccess::GetProperty(rxProperties, gsYHotSpotProperty) >>= pBitmap->mnYHotSpot;

    PresenterConfigurationAccess::GetProperty(rxProperties, gsReplacementColorProperty) >>= pBitmap->maReplacementColor;

    OUString sTexturingMode;
    if (PresenterConfigurationAccess::GetProperty(rxProperties, gsHorizontalTexturingModeProperty) >>= sTexturingMode)
        pBitmap->meHorizontalTexturingMode = StringToTexturingMode(sTexturingMode);
    if (PresenterConfigurationAccess::GetProperty(rxProperties, gsVerticalTexturingModeProperty) >>= sTexturingMode)
        pBitmap->meVerticalTexturingMode = StringToTexturingMode(sTexturingMode);

    return pBitmap;
}




PresenterBitmapContainer::BitmapDescriptor::TexturingMode
    PresenterBitmapContainer::StringToTexturingMode (const OUString& rsTexturingMode)
{
    if (rsTexturingMode == A2S("Once"))
        return PresenterBitmapContainer::BitmapDescriptor::Once;
    else if (rsTexturingMode == A2S("Repeat"))
        return PresenterBitmapContainer::BitmapDescriptor::Repeat;
    else if (rsTexturingMode == A2S("Stretch"))
        return PresenterBitmapContainer::BitmapDescriptor::Stretch;
    else
        return PresenterBitmapContainer::BitmapDescriptor::Once;
}




//===== PresenterBitmapContainer::BitmapSet ===================================

PresenterBitmapContainer::BitmapDescriptor::BitmapDescriptor (void)
    : mnWidth(0),
      mnHeight(0),
      mnXOffset(0),
      mnYOffset(0),
      mnXHotSpot(0),
      mnYHotSpot(0),
      maReplacementColor(0x00000000),
      meHorizontalTexturingMode(Once),
      meVerticalTexturingMode(Once),
      mxNormalBitmap(),
      mxMouseOverBitmap(),
      mxButtonDownBitmap(),
      mxDisabledBitmap(),
      mxMaskBitmap()
{
}




PresenterBitmapContainer::BitmapDescriptor::BitmapDescriptor (
    const ::boost::shared_ptr<PresenterBitmapContainer::BitmapDescriptor>& rpDefault)
    : mnWidth(0),
      mnHeight(0),
      mnXOffset(0),
      mnYOffset(0),
      mnXHotSpot(0),
      mnYHotSpot(0),
      maReplacementColor(0x00000000),
      meHorizontalTexturingMode(Once),
      meVerticalTexturingMode(Once),
      mxNormalBitmap(),
      mxMouseOverBitmap(),
      mxButtonDownBitmap(),
      mxDisabledBitmap(),
      mxMaskBitmap()
{
    if (rpDefault.get() != NULL)
    {
        mnWidth = rpDefault->mnWidth;
        mnHeight = rpDefault->mnHeight;
        mnXOffset = rpDefault->mnXOffset;
        mnYOffset = rpDefault->mnYOffset;
        mnXHotSpot = rpDefault->mnXHotSpot;
        mnYHotSpot = rpDefault->mnYHotSpot;
        maReplacementColor = rpDefault->maReplacementColor;
        meHorizontalTexturingMode = rpDefault->meHorizontalTexturingMode;
        meVerticalTexturingMode = rpDefault->meVerticalTexturingMode;
        mxNormalBitmap = rpDefault->mxNormalBitmap;
        mxMouseOverBitmap = rpDefault->mxMouseOverBitmap;
        mxButtonDownBitmap = rpDefault->mxButtonDownBitmap;
        mxDisabledBitmap = rpDefault->mxDisabledBitmap;
        mxMaskBitmap = rpDefault->mxMaskBitmap;
    }
}


css::uno::Reference<css::rendering::XBitmap>
    PresenterBitmapContainer::BitmapDescriptor::GetNormalBitmap (void) const
{
    return mxNormalBitmap;
}




css::uno::Reference<css::rendering::XBitmap>
    PresenterBitmapContainer::BitmapDescriptor::GetBitmap (
        const Mode eMode,
        const bool bMissingDefaultsToNormal) const
{
    switch (eMode)
    {
        case Normal:
        default:
            return mxNormalBitmap;

        case MouseOver:
            if (mxMouseOverBitmap.is())
                return mxMouseOverBitmap;
            else if (bMissingDefaultsToNormal)
                return mxNormalBitmap;

        case ButtonDown:
            if (mxButtonDownBitmap.is())
                return mxButtonDownBitmap;
            else if (bMissingDefaultsToNormal)
                return mxNormalBitmap;

        case Disabled:
            if (mxDisabledBitmap.is())
                return mxDisabledBitmap;
            else if (bMissingDefaultsToNormal)
                return mxNormalBitmap;

        case Mask:
            return mxMaskBitmap;
    }
    return NULL;
}




void PresenterBitmapContainer::BitmapDescriptor::SetBitmap (
    const Mode eMode,
    const css::uno::Reference<css::rendering::XBitmap>& rxBitmap)
{
    switch (eMode)
    {
        case Normal:
        default:
            mxNormalBitmap = rxBitmap;
            if (mxNormalBitmap.is())
            {
                const geometry::IntegerSize2D aSize (mxNormalBitmap->getSize());
                mnWidth = aSize.Width;
                mnHeight = aSize.Height;
            }
            break;

        case MouseOver:
            mxMouseOverBitmap = rxBitmap;
            break;

        case ButtonDown:
            mxButtonDownBitmap = rxBitmap;
            break;

        case Disabled:
            mxDisabledBitmap = rxBitmap;
            break;

        case Mask:
            mxMaskBitmap = rxBitmap;
            break;
    }
}




} } // end of namespace ::sdext::presenter
