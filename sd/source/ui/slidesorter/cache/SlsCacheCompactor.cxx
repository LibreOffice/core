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

#include <memory>
#include "SlsCacheCompactor.hxx"

#include "SlsBitmapCompressor.hxx"
#include "SlsBitmapCache.hxx"

#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <officecfg/Office/Impress.hxx>
#include <utility>

using namespace ::com::sun::star::uno;

namespace {

/** This is a trivial implementation of the CacheCompactor interface class.
    It ignores calls to RequestCompaction() and thus will never decrease the
    total size of off-screen preview bitmaps.
*/
class NoCacheCompaction
    : public ::sd::slidesorter::cache::CacheCompactor
{
public:
    NoCacheCompaction (
        ::sd::slidesorter::cache::BitmapCache& rCache,
        sal_Int32 nMaximalCacheSize)
        : CacheCompactor(rCache, nMaximalCacheSize)
    {}

    virtual void RequestCompaction() override { /* Ignored */ };

protected:
    virtual void Run() override { /* Do nothing */ };
};

/** This implementation of the CacheCompactor interface class uses one of
    several bitmap compression algorithms to reduce the number of the bytes
    of the off-screen previews in the bitmap cache.  See the documentation
    of CacheCompactor::Create() for more details on configuration properties
    that control the choice of compression algorithm.
*/
class CacheCompactionByCompression
    : public ::sd::slidesorter::cache::CacheCompactor
{
public:
    CacheCompactionByCompression (
        ::sd::slidesorter::cache::BitmapCache& rCache,
        sal_Int32 nMaximalCacheSize,
        std::shared_ptr< ::sd::slidesorter::cache::BitmapCompressor> pCompressor);

protected:
    virtual void Run() override;

private:
    std::shared_ptr< ::sd::slidesorter::cache::BitmapCompressor>  mpCompressor;
};

} // end of anonymous namespace

namespace sd::slidesorter::cache {

::std::unique_ptr<CacheCompactor> CacheCompactor::Create (
    BitmapCache& rCache,
    sal_Int32 nMaximalCacheSize)
{
    static const char sNone[] = "None";

    std::shared_ptr<BitmapCompressor> pCompressor;
    OUString sCompressionPolicy = officecfg::Office::Impress::MultiPaneGUI::SlideSorterBar::PreviewCache::CompressionPolicy::get();
    if (sCompressionPolicy == sNone)
        pCompressor = std::make_shared<NoBitmapCompression>();
    else if (sCompressionPolicy == "Erase")
        pCompressor = std::make_shared<CompressionByDeletion>();
    else if (sCompressionPolicy == "ResolutionReduction")
        pCompressor = std::make_shared<ResolutionReduction>();
    else // default: "PNGCompression"
        pCompressor = std::make_shared<PngCompression>();

    ::std::unique_ptr<CacheCompactor> pCompactor;
    OUString sCompactionPolicy = officecfg::Office::Impress::MultiPaneGUI::SlideSorterBar::PreviewCache::CompactionPolicy::get();
    if (sCompactionPolicy == sNone)
        pCompactor.reset(new NoCacheCompaction(rCache,nMaximalCacheSize));
    else // default: "Compress"
        pCompactor.reset(new CacheCompactionByCompression(rCache,nMaximalCacheSize,pCompressor));

    return pCompactor;
}

void CacheCompactor::RequestCompaction()
{
    if ( ! mbIsCompactionRunning && ! maCompactionTimer.IsActive())
        maCompactionTimer.Start();
}

CacheCompactor::CacheCompactor(
    BitmapCache& rCache,
    sal_Int32 nMaximalCacheSize)
    : mrCache(rCache),
      mnMaximalCacheSize(nMaximalCacheSize),
      maCompactionTimer("sd CacheCompactor maCompactionTimer"),
      mbIsCompactionRunning(false)
{
    maCompactionTimer.SetTimeout(100);
    maCompactionTimer.SetInvokeHandler(LINK(this,CacheCompactor,CompactionCallback));
}

IMPL_LINK_NOARG(CacheCompactor, CompactionCallback, Timer *, void)
{
    mbIsCompactionRunning = true;

    try
    {
        Run();
    }
    catch (const css::uno::RuntimeException&)
    {
    }
    catch (const css::uno::Exception&)
    {
    }

    mbIsCompactionRunning = false;
}

} // end of namespace ::sd::slidesorter::cache

namespace {

//===== CacheCompactionByCompression ==========================================

CacheCompactionByCompression::CacheCompactionByCompression (
    ::sd::slidesorter::cache::BitmapCache& rCache,
    sal_Int32 nMaximalCacheSize,
    std::shared_ptr< ::sd::slidesorter::cache::BitmapCompressor> pCompressor)
    : CacheCompactor(rCache,nMaximalCacheSize),
      mpCompressor(std::move(pCompressor))
{
}

void CacheCompactionByCompression::Run()
{
    if (mrCache.GetSize() <= mnMaximalCacheSize)
        return;

    SAL_INFO("sd.sls", __func__ << ": bitmap cache uses too much space: " << mrCache.GetSize() << " > " << mnMaximalCacheSize);

    ::sd::slidesorter::cache::BitmapCache::CacheIndex aIndex (
        mrCache.GetCacheIndex());
    for (const auto& rpIndex : aIndex)
    {
        if (rpIndex == nullptr)
            continue;

        mrCache.Compress(rpIndex, mpCompressor);
        if (mrCache.GetSize() < mnMaximalCacheSize)
            break;
    }
    mrCache.ReCalculateTotalCacheSize();
    SAL_INFO("sd.sls", __func__ << ":    there are now " << mrCache.GetSize() << " bytes occupied");
}

} // end of anonymous namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
