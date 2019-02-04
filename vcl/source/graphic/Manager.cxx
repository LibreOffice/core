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

#include <graphic/Manager.hxx>
#include <impgraph.hxx>
#include <vcl/lazydelete.hxx>
#include <sal/log.hxx>

using namespace css;

namespace vcl
{
namespace graphic
{
namespace
{
void setupConfigurationValuesIfPossible(sal_Int64& rMemoryLimit,
                                        std::chrono::seconds& rAllowedIdleTime)
{
    if (utl::ConfigManager::IsFuzzing())
        return;

    try
    {
        using officecfg::Office::Common::Cache;

        rMemoryLimit = Cache::GraphicManager::GraphicMemoryLimit::get();
        rAllowedIdleTime
            = std::chrono::seconds(Cache::GraphicManager::GraphicAllowedIdleTime::get());
    }
    catch (...)
    {
    }
}
}

Manager& Manager::get()
{
    static std::unique_ptr<Manager> gStaticManager(new Manager);
    return *gStaticManager;
}

Manager::Manager()
    : mnAllowedIdleTime(10)
    , mnMemoryLimit(300000000)
    , mnUsedSize(0)
    , maSwapOutTimer("graphic::Manager maSwapOutTimer")
{
    setupConfigurationValuesIfPossible(mnMemoryLimit, mnAllowedIdleTime);

    maSwapOutTimer.SetInvokeHandler(LINK(this, Manager, SwapOutTimerHandler));
    maSwapOutTimer.SetTimeout(10000);
    maSwapOutTimer.SetDebugName("graphic::Manager maSwapOutTimer");
    maSwapOutTimer.Start();
}

void Manager::reduceGraphicMemory()
{
    for (ImpGraphic* pEachImpGraphic : m_pImpGraphicList)
    {
        if (mnUsedSize < mnMemoryLimit * 0.7)
            return;

        sal_Int64 nCurrentGraphicSize = getGraphicSizeBytes(pEachImpGraphic);
        if (!pEachImpGraphic->ImplIsSwapOut() && nCurrentGraphicSize > 1000000)
        {
            if (!pEachImpGraphic->mpContext)
            {
                auto aCurrent = std::chrono::high_resolution_clock::now();
                auto aDeltaTime = aCurrent - pEachImpGraphic->maLastUsed;
                auto aSeconds = std::chrono::duration_cast<std::chrono::seconds>(aDeltaTime);

                if (aSeconds > mnAllowedIdleTime)
                    pEachImpGraphic->ImplSwapOut();
            }
        }
    }
}

sal_Int64 Manager::getGraphicSizeBytes(const ImpGraphic* pImpGraphic)
{
    if (!pImpGraphic->isAvailable())
        return 0;
    return pImpGraphic->ImplGetSizeBytes();
}

IMPL_LINK(Manager, SwapOutTimerHandler, Timer*, pTimer, void)
{
    pTimer->Stop();
    reduceGraphicMemory();
    pTimer->Start();
}

void Manager::registerGraphic(const std::shared_ptr<ImpGraphic>& pImpGraphic,
                              OUString const& /*rsContext*/)
{
    // make some space first
    if (mnUsedSize > mnMemoryLimit)
        reduceGraphicMemory();

    // Insert and update the used size (bytes)
    mnUsedSize += getGraphicSizeBytes(pImpGraphic.get());
    m_pImpGraphicList.insert(pImpGraphic.get());

    // calculate size of the graphic set
    sal_Int64 calculatedSize = 0;
    for (ImpGraphic* pEachImpGraphic : m_pImpGraphicList)
    {
        if (!pEachImpGraphic->ImplIsSwapOut())
        {
            calculatedSize += getGraphicSizeBytes(pEachImpGraphic);
        }
    }

    if (calculatedSize != mnUsedSize)
    {
        SAL_WARN_IF(calculatedSize != mnUsedSize, "vcl.gdi",
                    "Calculated size mismatch. Variable size is '"
                        << mnUsedSize << "' but calculated size is '" << calculatedSize << "'");
        mnUsedSize = calculatedSize;
    }
}

void Manager::unregisterGraphic(ImpGraphic* pImpGraphic)
{
    mnUsedSize -= getGraphicSizeBytes(pImpGraphic);
    m_pImpGraphicList.erase(pImpGraphic);
}

std::shared_ptr<ImpGraphic> Manager::copy(std::shared_ptr<ImpGraphic> const& rImpGraphicPtr)
{
    auto pReturn = std::make_shared<ImpGraphic>(*rImpGraphicPtr);
    registerGraphic(pReturn, "Copy");
    return pReturn;
}

std::shared_ptr<ImpGraphic> Manager::newInstance()
{
    auto pReturn = std::make_shared<ImpGraphic>();
    registerGraphic(pReturn, "Empty");
    return pReturn;
}

std::shared_ptr<ImpGraphic> Manager::newInstance(const Bitmap& rBitmap)
{
    auto pReturn = std::make_shared<ImpGraphic>(rBitmap);
    registerGraphic(pReturn, "Bitmap");
    return pReturn;
}

std::shared_ptr<ImpGraphic> Manager::newInstance(const BitmapEx& rBitmapEx)
{
    auto pReturn = std::make_shared<ImpGraphic>(rBitmapEx);
    registerGraphic(pReturn, "BitmapEx");
    return pReturn;
}

std::shared_ptr<ImpGraphic> Manager::newInstance(const Animation& rAnimation)
{
    auto pReturn = std::make_shared<ImpGraphic>(rAnimation);
    registerGraphic(pReturn, "Animation");
    return pReturn;
}

std::shared_ptr<ImpGraphic> Manager::newInstance(const VectorGraphicDataPtr& rVectorGraphicDataPtr)
{
    auto pReturn = std::make_shared<ImpGraphic>(rVectorGraphicDataPtr);
    registerGraphic(pReturn, "VectorGraphic");
    return pReturn;
}

std::shared_ptr<ImpGraphic> Manager::newInstance(const GDIMetaFile& rMetaFile)
{
    auto pReturn = std::make_shared<ImpGraphic>(rMetaFile);
    registerGraphic(pReturn, "Metafile");
    return pReturn;
}

std::shared_ptr<ImpGraphic> Manager::newInstance(const GraphicExternalLink& rGraphicLink)
{
    auto pReturn = std::make_shared<ImpGraphic>(rGraphicLink);
    registerGraphic(pReturn, "GraphicExternalLink");
    return pReturn;
}

void Manager::swappedIn(const ImpGraphic* pImpGraphic)
{
    mnUsedSize += getGraphicSizeBytes(pImpGraphic);
}

void Manager::swappedOut(const ImpGraphic* pImpGraphic)
{
    mnUsedSize -= getGraphicSizeBytes(pImpGraphic);
}

void Manager::changeExisting(const ImpGraphic* pImpGraphic, sal_Int64 nOldSizeBytes)
{
    mnUsedSize -= nOldSizeBytes;
    mnUsedSize += getGraphicSizeBytes(pImpGraphic);
}
}
} // end vcl::graphic

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
