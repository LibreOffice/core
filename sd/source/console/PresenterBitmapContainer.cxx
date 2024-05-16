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

#include <com/sun/star/drawing/XPresenterHelper.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <utility>
#include <osl/diagnose.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sdext::presenter {

//===== PresenterBitmapContainer ==============================================

PresenterBitmapContainer::PresenterBitmapContainer (
    const OUString& rsConfigurationBase,
    std::shared_ptr<PresenterBitmapContainer> xParentContainer,
    const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext,
    css::uno::Reference<css::rendering::XCanvas> xCanvas,
    css::uno::Reference<css::drawing::XPresenterHelper> xPresenterHelper)
    : mpParentContainer(std::move(xParentContainer)),
      mxCanvas(std::move(xCanvas)),
      mxPresenterHelper(std::move(xPresenterHelper))
{
    Initialize(rxComponentContext);

    // Get access to the configuration.
    PresenterConfigurationAccess aConfiguration (
        rxComponentContext,
        u"org.openoffice.Office.PresenterScreen"_ustr,
        PresenterConfigurationAccess::READ_ONLY);
    Reference<container::XNameAccess> xBitmapList (
        aConfiguration.GetConfigurationNode(rsConfigurationBase),
        UNO_QUERY_THROW);

    LoadBitmaps(xBitmapList);
}

PresenterBitmapContainer::PresenterBitmapContainer (
    const css::uno::Reference<css::container::XNameAccess>& rxRootNode,
    std::shared_ptr<PresenterBitmapContainer> xParentContainer,
    const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext,
    css::uno::Reference<css::rendering::XCanvas> xCanvas,
    css::uno::Reference<css::drawing::XPresenterHelper> xPresenterHelper)
    : mpParentContainer(std::move(xParentContainer)),
      mxCanvas(std::move(xCanvas)),
      mxPresenterHelper(std::move(xPresenterHelper))
{
    Initialize(rxComponentContext);

    LoadBitmaps(rxRootNode);
}

void PresenterBitmapContainer::Initialize (
    const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext)
{
    if (  mxPresenterHelper.is())
        return;

    // Create an object that is able to load the bitmaps in a format that is
    // supported by the canvas.
    Reference<lang::XMultiComponentFactory> xFactory =
        rxComponentContext->getServiceManager();
    if ( ! xFactory.is())
        return;
    mxPresenterHelper.set(
        xFactory->createInstanceWithContext(
            u"com.sun.star.drawing.PresenterHelper"_ustr,
            rxComponentContext),
        UNO_QUERY_THROW);
}

PresenterBitmapContainer::~PresenterBitmapContainer()
{
    maIconContainer.clear();
}

std::shared_ptr<PresenterBitmapContainer::BitmapDescriptor> PresenterBitmapContainer::GetBitmap (
    const OUString& rsName) const
{
    BitmapContainer::const_iterator iSet (maIconContainer.find(rsName));
    if (iSet != maIconContainer.end())
        return iSet->second;
    else if (mpParentContainer != nullptr)
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

std::shared_ptr<PresenterBitmapContainer::BitmapDescriptor> PresenterBitmapContainer::LoadBitmap (
    const css::uno::Reference<css::container::XHierarchicalNameAccess>& rxNode,
    const OUString& rsPath,
    const css::uno::Reference<css::drawing::XPresenterHelper>& rxPresenterHelper,
    const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
    const std::shared_ptr<BitmapDescriptor>& rpDefault)
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
    if ( ! (PresenterConfigurationAccess::GetProperty(rxProperties, u"Name"_ustr) >>= sName))
        sName = rsKey;

    maIconContainer[sName] = LoadBitmap(
        rxProperties,
        mxPresenterHelper,
        mxCanvas,
        SharedBitmapDescriptor());
}

std::shared_ptr<PresenterBitmapContainer::BitmapDescriptor> PresenterBitmapContainer::LoadBitmap (
    const Reference<beans::XPropertySet>& rxProperties,
    const css::uno::Reference<css::drawing::XPresenterHelper>& rxPresenterHelper,
    const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
    const std::shared_ptr<BitmapDescriptor>& rpDefault)
{
    OSL_ASSERT(rxCanvas.is());
    OSL_ASSERT(rxPresenterHelper.is());

    SharedBitmapDescriptor pBitmap = std::make_shared<BitmapDescriptor>(rpDefault);

    if ( ! rxProperties.is())
        return pBitmap;

    OUString sFileName;

    // Load bitmaps.
    if (PresenterConfigurationAccess::GetProperty(rxProperties, u"NormalFileName"_ustr) >>= sFileName)
        try
        {
            pBitmap->SetBitmap(
                BitmapDescriptor::Normal,
                rxPresenterHelper->loadBitmap(sFileName, rxCanvas));
        }
        catch (Exception&)
        {}
    if (PresenterConfigurationAccess::GetProperty(rxProperties, u"MouseOverFileName"_ustr) >>= sFileName)
        try
        {
            pBitmap->SetBitmap(
                BitmapDescriptor::MouseOver,
                rxPresenterHelper->loadBitmap(sFileName, rxCanvas));
        }
        catch (Exception&)
        {}
    if (PresenterConfigurationAccess::GetProperty(rxProperties, u"ButtonDownFileName"_ustr) >>= sFileName)
        try
        {
            pBitmap->SetBitmap(
                BitmapDescriptor::ButtonDown,
                rxPresenterHelper->loadBitmap(sFileName, rxCanvas));
        }
        catch (Exception&)
        {}
    if (PresenterConfigurationAccess::GetProperty(rxProperties, u"DisabledFileName"_ustr) >>= sFileName)
        try
        {
            pBitmap->SetBitmap(
                BitmapDescriptor::Disabled,
                rxPresenterHelper->loadBitmap(sFileName, rxCanvas));
        }
        catch (Exception&)
        {}
    if (PresenterConfigurationAccess::GetProperty(rxProperties, u"MaskFileName"_ustr) >>= sFileName)
        try
        {
            pBitmap->SetBitmap(
                BitmapDescriptor::Mask,
                rxPresenterHelper->loadBitmap(sFileName, rxCanvas));
        }
        catch (Exception&)
        {}

    PresenterConfigurationAccess::GetProperty(rxProperties, u"XOffset"_ustr) >>= pBitmap->mnXOffset;
    PresenterConfigurationAccess::GetProperty(rxProperties, u"YOffset"_ustr) >>= pBitmap->mnYOffset;

    PresenterConfigurationAccess::GetProperty(rxProperties, u"XHotSpot"_ustr) >>= pBitmap->mnXHotSpot;
    PresenterConfigurationAccess::GetProperty(rxProperties, u"YHotSpot"_ustr) >>= pBitmap->mnYHotSpot;

    PresenterConfigurationAccess::GetProperty(rxProperties, u"ReplacementColor"_ustr) >>= pBitmap->maReplacementColor;

    OUString sTexturingMode;
    if (PresenterConfigurationAccess::GetProperty(rxProperties, u"HorizontalTexturingMode"_ustr) >>= sTexturingMode)
        pBitmap->meHorizontalTexturingMode = StringToTexturingMode(sTexturingMode);
    if (PresenterConfigurationAccess::GetProperty(rxProperties, u"VerticalTexturingMode"_ustr) >>= sTexturingMode)
        pBitmap->meVerticalTexturingMode = StringToTexturingMode(sTexturingMode);

    return pBitmap;
}

PresenterBitmapContainer::BitmapDescriptor::TexturingMode
    PresenterBitmapContainer::StringToTexturingMode (std::u16string_view rsTexturingMode)
{
    if (rsTexturingMode == u"Once")
        return PresenterBitmapContainer::BitmapDescriptor::Once;
    else if (rsTexturingMode == u"Repeat")
        return PresenterBitmapContainer::BitmapDescriptor::Repeat;
    else if (rsTexturingMode == u"Stretch")
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
      meVerticalTexturingMode(Once)
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
      meVerticalTexturingMode(Once)
{
    if (rpDefault == nullptr)
        return;

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

} // end of namespace ::sdext::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
