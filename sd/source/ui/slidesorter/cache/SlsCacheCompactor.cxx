/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsCacheCompactor.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-10-24 07:39:54 $
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




IMPL_LINK(CacheCompactor, CompactionCallback, Timer*, pTimer)
{
    mbIsCompactionRunning = true;

    try
    {
        Run();
    }
    catch(::com::sun::star::uno::RuntimeException e) { }
    catch(::com::sun::star::uno::Exception e) { }

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
