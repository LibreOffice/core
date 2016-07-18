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
#include "PresenterConfigurationAccess.hxx"

#include <com/sun/star/deployment/XPackageInformationProvider.hpp>
#include <com/sun/star/drawing/XPresenterHelper.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <osl/diagnose.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::std;

namespace sdext { namespace presenter {

//===== PresenterBitmapContainer ==============================================

PresenterBitmapContainer::PresenterBitmapContainer (
    const OUString& rsConfigurationBase,
    const std::shared_ptr<PresenterBitmapContainer>& rpParentContainer,
    const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext,
    const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
    const css::uno::Reference<css::drawing::XPresenterHelper>& rxPresenterHelper)
    : mpParentContainer(rpParentContainer),
      maIconContainer(),
      mxCanvas(rxCanvas),
      mxPresenterHelper(rxPresenterHelper)
{
    Initialize(rxComponentContext);

    // Get access to the configuration.
    PresenterConfigurationAccess aConfiguration (
        rxComponentContext,
        "org.openoffice.Office.PresenterScreen",
        PresenterConfigurationAccess::READ_ONLY);
    Reference<container::XNameAccess> xBitmapList (
        aConfiguration.GetConfigurationNode(rsConfigurationBase),
        UNO_QUERY_THROW);

    LoadBitmaps(xBitmapList);
}

PresenterBitmapContainer::PresenterBitmapContainer (
    const css::uno::Reference<css::container::XNameAccess>& rxRootNode,
    const std::shared_ptr<PresenterBitmapContainer>& rpParentContainer,
    const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext,
    const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
    const css::uno::Reference<css::drawing::XPresenterHelper>& rxPresenterHelper)
    : mpParentContainer(rpParentContainer),
      maIconContainer(),
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
        mxPresenterHelper.set(
            xFactory->createInstanceWithContext(
                "com.sun.star.drawing.PresenterHelper",
                rxComponentContext),
            UNO_QUERY_THROW);
    }
}

PresenterBitmapContainer::~PresenterBitmapContainer()
{
    maIconContainer.clear();
}

SharedBitmapDescriptor PresenterBitmapContainer::GetBitmap (
    const OUString& rsName) const
{
    BitmapContainer::const_iterator iSet (maIconContainer.find(rsName));
    if (iSet != maIconContainer.end())
        return iSet->second;
    else if (mpParentContainer.get() != nullptr)
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
                [this](OUString const& rKey, Reference<beans::XPropertySet> const& xProps)
                {
                    this->ProcessBitmap(rKey, xProps);
                });
        }
    }
    catch (Exception&)
    {
        OSL_ASSERT(false);
    }
}

SharedBitmapDescriptor PresenterBitmapContainer::LoadBitmap (
    const css::uno::Reference<css::container::XHierarchicalNameAccess>& rxNode,
    const OUString& rsPath,
    const css::uno::Reference<css::drawing::XPresenterHelper>& rxPresenterHelper,
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
    if ( ! (PresenterConfigurationAccess::GetProperty(rxProperties, "Name") >>= sName))
        sName = rsKey;

    maIconContainer[sName] = LoadBitmap(
        rxProperties,
        mxPresenterHelper,
        mxCanvas,
        SharedBitmapDescriptor());
}

SharedBitmapDescriptor PresenterBitmapContainer::LoadBitmap (
    const Reference<beans::XPropertySet>& rxProperties,
    const css::uno::Reference<css::drawing::XPresenterHelper>& rxPresenterHelper,
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
    if (PresenterConfigurationAccess::GetProperty(rxProperties, "NormalFileName") >>= sFileName)
        try
        {
            pBitmap->SetBitmap(
                BitmapDescriptor::Normal,
                rxPresenterHelper->loadBitmap(sFileName, rxCanvas));
        }
        catch (Exception&)
        {}
    if (PresenterConfigurationAccess::GetProperty(rxProperties, "MouseOverFileName") >>= sFileName)
        try
        {
            pBitmap->SetBitmap(
                BitmapDescriptor::MouseOver,
                rxPresenterHelper->loadBitmap(sFileName, rxCanvas));
        }
        catch (Exception&)
        {}
    if (PresenterConfigurationAccess::GetProperty(rxProperties, "ButtonDownFileName") >>= sFileName)
        try
        {
            pBitmap->SetBitmap(
                BitmapDescriptor::ButtonDown,
                rxPresenterHelper->loadBitmap(sFileName, rxCanvas));
        }
        catch (Exception&)
        {}
    if (PresenterConfigurationAccess::GetProperty(rxProperties, "DisabledFileName") >>= sFileName)
        try
        {
            pBitmap->SetBitmap(
                BitmapDescriptor::Disabled,
                rxPresenterHelper->loadBitmap(sFileName, rxCanvas));
        }
        catch (Exception&)
        {}
    if (PresenterConfigurationAccess::GetProperty(rxProperties, "MaskFileName") >>= sFileName)
        try
        {
            pBitmap->SetBitmap(
                BitmapDescriptor::Mask,
                rxPresenterHelper->loadBitmap(sFileName, rxCanvas));
        }
        catch (Exception&)
        {}

    PresenterConfigurationAccess::GetProperty(rxProperties, "XOffset") >>= pBitmap->mnXOffset;
    PresenterConfigurationAccess::GetProperty(rxProperties, "YOffset") >>= pBitmap->mnYOffset;

    PresenterConfigurationAccess::GetProperty(rxProperties, "XHotSpot") >>= pBitmap->mnXHotSpot;
    PresenterConfigurationAccess::GetProperty(rxProperties, "YHotSpot") >>= pBitmap->mnYHotSpot;

    PresenterConfigurationAccess::GetProperty(rxProperties, "ReplacementColor") >>= pBitmap->maReplacementColor;

    OUString sTexturingMode;
    if (PresenterConfigurationAccess::GetProperty(rxProperties, "HorizontalTexturingMode") >>= sTexturingMode)
        pBitmap->meHorizontalTexturingMode = StringToTexturingMode(sTexturingMode);
    if (PresenterConfigurationAccess::GetProperty(rxProperties, "VerticalTexturingMode") >>= sTexturingMode)
        pBitmap->meVerticalTexturingMode = StringToTexturingMode(sTexturingMode);

    return pBitmap;
}

PresenterBitmapContainer::BitmapDescriptor::TexturingMode
    PresenterBitmapContainer::StringToTexturingMode (const OUString& rsTexturingMode)
{
    if (rsTexturingMode == "Once")
        return PresenterBitmapContainer::BitmapDescriptor::Once;
    else if (rsTexturingMode == "Repeat")
        return PresenterBitmapContainer::BitmapDescriptor::Repeat;
    else if (rsTexturingMode == "Stretch")
        return PresenterBitmapContainer::BitmapDescriptor::Stretch;
    else
        return PresenterBitmapContainer::BitmapDescriptor::Once;
}

//===== PresenterBitmapContainer::BitmapSet ===================================

PresenterBitmapContainer::BitmapDescriptor::BitmapDescriptor()
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
    const std::shared_ptr<PresenterBitmapContainer::BitmapDescriptor>& rpDefault)
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
    if (rpDefault.get() != nullptr)
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

const css::uno::Reference<css::rendering::XBitmap>&
    PresenterBitmapContainer::BitmapDescriptor::GetNormalBitmap() const
{
    return mxNormalBitmap;
}

css::uno::Reference<css::rendering::XBitmap> const &
    PresenterBitmapContainer::BitmapDescriptor::GetBitmap(const Mode eMode) const
{
    switch (eMode)
    {
        case Normal:
        default:
            return mxNormalBitmap;

        case MouseOver:
            if (mxMouseOverBitmap.is())
                return mxMouseOverBitmap;
            else
                return mxNormalBitmap;

        case ButtonDown:
            if (mxButtonDownBitmap.is())
                return mxButtonDownBitmap;
            else
                return mxNormalBitmap;

        case Disabled:
            if (mxDisabledBitmap.is())
                return mxDisabledBitmap;
            else
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
