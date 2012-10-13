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
    if ( ! (PresenterConfigurationAccess::GetProperty(rxProperties, A2S("Name")) >>= sName))
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
    if (PresenterConfigurationAccess::GetProperty(rxProperties, A2S("NormalFileName")) >>= sFileName)
        try
        {
            pBitmap->SetBitmap(
                BitmapDescriptor::Normal,
                rxPresenterHelper->loadBitmap(rsBasePath + sFileName, rxCanvas));
        }
        catch (Exception&)
        {}
    if (PresenterConfigurationAccess::GetProperty(rxProperties, A2S("MouseOverFileName")) >>= sFileName)
        try
        {
            pBitmap->SetBitmap(
                BitmapDescriptor::MouseOver,
                rxPresenterHelper->loadBitmap(rsBasePath + sFileName, rxCanvas));
        }
        catch (Exception&)
        {}
    if (PresenterConfigurationAccess::GetProperty(rxProperties, A2S("ButtonDownFileName")) >>= sFileName)
        try
        {
            pBitmap->SetBitmap(
                BitmapDescriptor::ButtonDown,
                rxPresenterHelper->loadBitmap(rsBasePath + sFileName, rxCanvas));
        }
        catch (Exception&)
        {}
    if (PresenterConfigurationAccess::GetProperty(rxProperties, A2S("DisabledFileName")) >>= sFileName)
        try
        {
            pBitmap->SetBitmap(
                BitmapDescriptor::Disabled,
                rxPresenterHelper->loadBitmap(rsBasePath + sFileName, rxCanvas));
        }
        catch (Exception&)
        {}
    if (PresenterConfigurationAccess::GetProperty(rxProperties, A2S("MaskFileName")) >>= sFileName)
        try
        {
            pBitmap->SetBitmap(
                BitmapDescriptor::Mask,
                rxPresenterHelper->loadBitmap(rsBasePath + sFileName, rxCanvas));
        }
        catch (Exception&)
        {}

    PresenterConfigurationAccess::GetProperty(rxProperties, A2S("XOffset")) >>= pBitmap->mnXOffset;
    PresenterConfigurationAccess::GetProperty(rxProperties, A2S("YOffset")) >>= pBitmap->mnYOffset;

    PresenterConfigurationAccess::GetProperty(rxProperties, A2S("XHotSpot")) >>= pBitmap->mnXHotSpot;
    PresenterConfigurationAccess::GetProperty(rxProperties, A2S("YHotSpot")) >>= pBitmap->mnYHotSpot;

    PresenterConfigurationAccess::GetProperty(rxProperties, A2S("ReplacementColor")) >>= pBitmap->maReplacementColor;

    OUString sTexturingMode;
    if (PresenterConfigurationAccess::GetProperty(rxProperties, A2S("HorizontalTexturingMode")) >>= sTexturingMode)
        pBitmap->meHorizontalTexturingMode = StringToTexturingMode(sTexturingMode);
    if (PresenterConfigurationAccess::GetProperty(rxProperties, A2S("VerticalTexturingMode")) >>= sTexturingMode)
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
