/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WW8BinTableImpl.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:44:59 $
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

#ifndef INCLUDED_WW8_BIN_TABLE_IMPL_HXX
#define INCLUDED_WW8_BIN_TABLE_IMPL_HXX

#include <istream>
#ifndef INCLUDED_WW8_BIN_TABLE_HXX
#include <WW8BinTable.hxx>
#endif
#ifndef INCLUDED_PLCF_HXX
#include <PLCF.hxx>
#endif

#include <hash_map>

namespace writerfilter {
namespace doctok
{
using namespace ::std;

/**
   A number of a FKP.
 */
class PageNumber
{
    /// the page number
    sal_uInt32 mnPageNumber;

public:
    /// Pointer to a page number
    typedef boost::shared_ptr<PageNumber> Pointer_t;

    /// get size of a page number
    static size_t getSize() { return 4; }

    PageNumber(WW8StructBase::Sequence & rSeq, sal_uInt32 nOffset,
               sal_uInt32 /*nCount*/)
    : mnPageNumber(getU32(rSeq, nOffset))
    {
    }

    /// Return the page number
    sal_uInt32 get() const { return mnPageNumber; }

    virtual void dump(OutputWithDepth<string> & output) const;
};

/**
   Implementation class for a binary table
 */
class WW8BinTableImpl : public WW8BinTable
{
    /// PLCF containing the numbers of the FKPs of the binary table
    PLCF<PageNumber> mData;
    mutable hash_map<Fc, sal_uInt32, FcHash> mPageMap;

public:
    WW8BinTableImpl(WW8Stream & rStream, sal_uInt32 nOffset,
                    sal_uInt32 nCount)
    : mData(rStream, nOffset, nCount)

    {
    }

    virtual sal_uInt32 getEntryCount() const
    { return mData.getEntryCount(); }
    virtual Fc getFc(sal_uInt32 nIndex) const
    { return mData.getFc(nIndex); }
    virtual sal_uInt32 getPageNumber(sal_uInt32 nIndex) const
    { return mData.getEntry(nIndex)->get(); }
    virtual sal_uInt32 getPageNumber(const Fc & rFc) const;
    virtual string toString() const;
};

}}

#endif // INCLUDED_WW8_BIN_TABLE_IMPL_HXX
