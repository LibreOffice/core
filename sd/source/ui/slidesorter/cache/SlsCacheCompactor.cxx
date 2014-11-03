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

#include "SlsCacheCompactor.hxx"

#include "SlsBitmapCompressor.hxx"
#include "SlsBitmapCache.hxx"
#include "SlsCacheConfiguration.hxx"

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <set>

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

    virtual void RequestCompaction (void) SAL_OVERRIDE { /* Ignored */ };

protected:
    virtual void Run (void) SAL_OVERRIDE { /* Do nothing */ };
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
        const ::boost::shared_ptr< ::sd::slidesorter::cache::BitmapCompressor>& rpCompressor);

protected:
    virtual void Run (void) SAL_OVERRIDE;

private:
    ::boost::shared_ptr< ::sd::slidesorter::cache::BitmapCompressor>  mpCompressor;
};

} // end of anonymous namespace

namespace sd { namespace slidesorter { namespace cache {

::std::unique_ptr<CacheCompactor> CacheCompactor::Create (
    BitmapCache& rCache,
    sal_Int32 nMaximalCacheSize)
{
    static const char sNone[] = "None";
    static const char sCompress[] = "Compress";
    static const char sErase[] = "Erase";
    static const char sResolution[] = "ResolutionReduction";
    static const char sPNGCompression[] = "PNGCompression";

    ::boost::shared_ptr<BitmapCompressor> pCompressor;
    OUString sCompressionPolicy(sPNGCompression);
    Any aCompressionPolicy (CacheConfiguration::Instance()->GetValue("CompressionPolicy"));
    if (aCompressionPolicy.has<OUString>())
        aCompressionPolicy >>= sCompressionPolicy;
    if (sCompressionPolicy == sNone)
        pCompressor.reset(new NoBitmapCompression());
    else if (sCompressionPolicy == sErase)
        pCompressor.reset(new CompressionByDeletion());
    else if (sCompressionPolicy == sResolution)
        pCompressor.reset(new ResolutionReduction());
    else
        pCompressor.reset(new PngCompression());

    ::std::unique_ptr<CacheCompactor> pCompactor;
    OUString sCompactionPolicy(sCompress);
    Any aCompactionPolicy (CacheConfiguration::Instance()->GetValue("CompactionPolicy"));
    if (aCompactionPolicy.has<OUString>())
        aCompactionPolicy >>= sCompactionPolicy;
    if (sCompactionPolicy == sNone)
        pCompactor.reset(new NoCacheCompaction(rCache,nMaximalCacheSize));
    else
        pCompactor.reset(new CacheCompactionByCompression(rCache,nMaximalCacheSize,pCompressor));

    return pCompactor;
}

void CacheCompactor::RequestCompaction (void)
{
    if ( ! mbIsCompactionRunning && ! maCompactionTimer.IsActive())
        maCompactionTimer.Start();
}

CacheCompactor::CacheCompactor(
    BitmapCache& rCache,
    sal_Int32 nMaximalCacheSize)
    : mrCache(rCache),
      mnMaximalCacheSize(nMaximalCacheSize),
      mbIsCompactionRunning(false)
{
    maCompactionTimer.SetTimeout(100 /*ms*/);
    maCompactionTimer.SetTimeoutHdl(LINK(this,CacheCompactor,CompactionCallback));

}

IMPL_LINK_NOARG(CacheCompactor, CompactionCallback)
{
    mbIsCompactionRunning = true;

    try
    {
        Run();
    }
    catch (const ::com::sun::star::uno::RuntimeException&)
    {
    }
    catch (const ::com::sun::star::uno::Exception&)
    {
    }

    mbIsCompactionRunning = false;
    return 1;
}

} } } // end of namespace ::sd::slidesorter::cache

namespace {

//===== CacheCompactionByCompression ==========================================

CacheCompactionByCompression::CacheCompactionByCompression (
    ::sd::slidesorter::cache::BitmapCache& rCache,
    sal_Int32 nMaximalCacheSize,
    const ::boost::shared_ptr< ::sd::slidesorter::cache::BitmapCompressor>& rpCompressor)
    : CacheCompactor(rCache,nMaximalCacheSize),
      mpCompressor(rpCompressor)
{
}

void CacheCompactionByCompression::Run (void)
{
    if (mrCache.GetSize() > mnMaximalCacheSize)
    {
        SAL_INFO("sd.sls", OSL_THIS_FUNC << ": bitmap cache uses to much space: " << mrCache.GetSize() << " > " << mnMaximalCacheSize);

        ::std::unique_ptr< ::sd::slidesorter::cache::BitmapCache::CacheIndex> pIndex (
            mrCache.GetCacheIndex(false,false));
        ::sd::slidesorter::cache::BitmapCache::CacheIndex::iterator iIndex;
        for (iIndex=pIndex->begin(); iIndex!=pIndex->end(); ++iIndex)
        {
            if (*iIndex == NULL)
                continue;

            mrCache.Compress(*iIndex, mpCompressor);
            if (mrCache.GetSize() < mnMaximalCacheSize)
                break;
        }
        mrCache.ReCalculateTotalCacheSize();
        SAL_INFO("sd.sls", OSL_THIS_FUNC << ":    there are now " << mrCache.GetSize() << " bytes occupied");
    }
}

} // end of anonymous namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
