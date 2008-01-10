/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WW8FKPImpl.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:46:57 $
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

#ifndef INCLUDED_WW8_FKP_IMPL_HXX
#define INCLUDED_WW8_FKP_IMPL_HXX

#include <set>
#include <deque>
#ifndef INCLUDED_WW8_FKP_HXX
#include <WW8FKP.hxx>
#endif

#ifndef INCLUDED_OUTPUT_WITH_DEPTH_HXX
#include <resourcemodel/OutputWithDepth.hxx>
#endif

namespace writerfilter {
namespace doctok
{
/**
   Implementation class for formatted disk pages.
 */
class WW8FKPImpl : public WW8FKP
{
    sal_uInt32 mnPageNumber;
    bool mbComplex;

public:
    WW8FKPImpl(WW8Stream & rStream, sal_uInt32 nPageNumber, bool bComplex)
    : WW8FKP(rStream, nPageNumber * 512), mnPageNumber(nPageNumber),
      mbComplex(bComplex)
    {
    }

    virtual sal_uInt32 getPageNumber() const { return mnPageNumber; }

    virtual sal_uInt32 getEntryCount() const { return getU8(511); }
    virtual sal_uInt32 getRgb() const { return 4 * (getEntryCount() + 1); }
    virtual Fc getFc(sal_uInt32 nIndex) const
    { return Fc(getU32(nIndex * 4), mbComplex); }
    virtual Fc getFirstFc() const { return getFc(0); }
    virtual Fc getLastFc() const { return getFc(getEntryCount()); }

    virtual bool contains(const Fc & rFc) const
    { return getFirstFc() <= rFc && rFc < getLastFc(); }

    virtual sal_uInt32 getIndex(const Fc & rFc) const;

    friend bool operator < (const WW8FKPImpl & rA,
                            const WW8FKPImpl & rB);
};

/**
   Implementation class for formatted disk pages containing character
   properties.
 */
class WW8CHPFKPImpl : public WW8FKPImpl
{
public:
    WW8CHPFKPImpl(WW8Stream & rStream, sal_uInt32 nPageNumber,
                  bool bComplex)
    : WW8FKPImpl(rStream, nPageNumber, bComplex)
    {
    }

    virtual writerfilter::Reference<Properties>::Pointer_t
    getProperties(const Fc & rFc) const;

    virtual void dump(OutputWithDepth<string> & o) const;
};

/**
   Implementation class for formatted disk pages containing paragraph
   properties.
 */
class WW8PAPFKPImpl : public WW8FKPImpl
{
public:
    WW8PAPFKPImpl(WW8Stream & rStream, sal_uInt32 nPageNumber,
                  bool bComplex)
    : WW8FKPImpl(rStream, nPageNumber, bComplex)
    {
    }

    virtual writerfilter::Reference<Properties>::Pointer_t
    getProperties(const Fc & rFc) const;

    virtual void dump(OutputWithDepth<string> & o) const;
};

/**
   Tuple of page number and formattet disk page.
 */
class PageNumberAndFKP
{
    /// page number
    sal_uInt32 mnPageNumber;

    /// pointer to formatted disk page
    WW8FKP::Pointer_t mpFKP;

public:
    PageNumberAndFKP(sal_uInt32 nPageNumber, WW8FKP::Pointer_t pFKP)
    : mnPageNumber(nPageNumber), mpFKP(pFKP)
    {
    }

    /**
       Return page number.
     */
    sal_uInt32 getPageNumber() const { return mnPageNumber; }

    /**
       Return formatted disk page.
     */
    const WW8FKP::Pointer_t getFKP() const { return mpFKP; }

    friend bool operator < (const PageNumberAndFKP & rA,
                            const PageNumberAndFKP & rB);
};

/**
   Cache for formatted disk pages.
 */
class WW8FKPCacheImpl : public WW8FKPCache
{
    /// size of the cache
    sal_uInt32 mnCacheSize;

    /// set of tuples of page number and FKP
    typedef set<PageNumberAndFKP> PageNumbersAndFKPs;

    ///
    typedef deque<sal_uInt32> PageNumbers;

    PageNumbers mPageNumbers;
    PageNumbersAndFKPs mPageNumbersAndFKPs;

protected:
    WW8Stream::Pointer_t mpStream;
    virtual WW8FKP::Pointer_t createFKP(sal_uInt32 nPageNumber,
                                        bool bComplex) = 0;

public:
    WW8FKPCacheImpl(WW8Stream::Pointer_t rpStream, sal_uInt32 nCacheSize)
    : mnCacheSize(nCacheSize), mpStream(rpStream)
    {
    }

    virtual ~WW8FKPCacheImpl()
    {
    }

    WW8FKP::Pointer_t get(sal_uInt32 nPageNumber, bool bComplex);
};

class WW8CHPFKPCacheImpl : public WW8FKPCacheImpl
{
    virtual WW8FKP::Pointer_t createFKP(sal_uInt32 nPageNumber,
                                        bool bComplex);

public:
    WW8CHPFKPCacheImpl(WW8Stream::Pointer_t rpStream,
                       sal_uInt32 nCacheSize)
    : WW8FKPCacheImpl(rpStream, nCacheSize)
    {
    }

    virtual ~WW8CHPFKPCacheImpl()
    {
    }
};

class WW8PAPFKPCacheImpl : public WW8FKPCacheImpl
{
    virtual WW8FKP::Pointer_t createFKP(sal_uInt32 nPageNumber,
                                        bool bComplex);

public:
    WW8PAPFKPCacheImpl(WW8Stream::Pointer_t rpStream,
                       sal_uInt32 nCacheSize)
    : WW8FKPCacheImpl(rpStream, nCacheSize)
    {
    }

    virtual ~WW8PAPFKPCacheImpl()
    {
    }
};
}}

#endif // INCLUDED_WW8_FKP_IMPL_HXX
