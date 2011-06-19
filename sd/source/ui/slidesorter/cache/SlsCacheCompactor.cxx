/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "SlsCacheCompactor.hxx"

#include "SlsBitmapCompressor.hxx"
#include "SlsBitmapCache.hxx"
#include "SlsCacheCompactor.hxx"
#include "SlsCacheConfiguration.hxx"

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <set>

using namespace ::com::sun::star::uno;

// Uncomment the definition of VERBOSE to get some more OSL_TRACE messages.
#ifdef DEBUG
//#define VERBOSE
#endif

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

    virtual void RequestCompaction (void) { /* Ignored */ };

protected:
    virtual void Run (void) { /* Do nothing */ };
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
    virtual void Run (void);

private:
    ::boost::shared_ptr< ::sd::slidesorter::cache::BitmapCompressor>  mpCompressor;
};


} // end of anonymous namespace

namespace sd { namespace slidesorter { namespace cache {


::std::auto_ptr<CacheCompactor> CacheCompactor::Create (
    BitmapCache& rCache,
    sal_Int32 nMaximalCacheSize)
{
    static const ::rtl::OUString sNone (RTL_CONSTASCII_USTRINGPARAM("None"));
    static const ::rtl::OUString sCompress (RTL_CONSTASCII_USTRINGPARAM("Compress"));
    static const ::rtl::OUString sErase (RTL_CONSTASCII_USTRINGPARAM("Erase"));
    static const ::rtl::OUString sResolution (RTL_CONSTASCII_USTRINGPARAM("ResolutionReduction"));
    static const ::rtl::OUString sPNGCompression (RTL_CONSTASCII_USTRINGPARAM("PNGCompression"));

    ::boost::shared_ptr<BitmapCompressor> pCompressor;
    ::rtl::OUString sCompressionPolicy(sPNGCompression);
    Any aCompressionPolicy (CacheConfiguration::Instance()->GetValue(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CompressionPolicy"))));
    if (aCompressionPolicy.has<rtl::OUString>())
        aCompressionPolicy >>= sCompressionPolicy;
    if (sCompressionPolicy == sNone)
        pCompressor.reset(new NoBitmapCompression());
    else if (sCompressionPolicy == sErase)
        pCompressor.reset(new CompressionByDeletion());
    else if (sCompressionPolicy == sResolution)
        pCompressor.reset(new ResolutionReduction());
    else
        pCompressor.reset(new PngCompression());

    ::std::auto_ptr<CacheCompactor> pCompactor (NULL);
    ::rtl::OUString sCompactionPolicy(sCompress);
    Any aCompactionPolicy (CacheConfiguration::Instance()->GetValue(
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CompactionPolicy"))));
    if (aCompactionPolicy.has<rtl::OUString>())
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




IMPL_LINK(CacheCompactor, CompactionCallback, Timer*, EMPTYARG)
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
#ifdef VERBOSE
        OSL_TRACE ("bitmap cache uses to much space: %d > %d",
            mrCache.GetSize(), mnMaximalCacheSize);
#endif

        ::std::auto_ptr< ::sd::slidesorter::cache::BitmapCache::CacheIndex> pIndex (
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
#ifdef VERBOSE
        OSL_TRACE ("    there are now %d bytes occupied", mrCache.GetSize());
#endif
    }
}


} // end of anonymous namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
