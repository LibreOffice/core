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

#ifndef INCLUDED_WW8_BIN_TABLE_IMPL_HXX
#define INCLUDED_WW8_BIN_TABLE_IMPL_HXX

#include <istream>
#include <WW8BinTable.hxx>
#include <PLCF.hxx>

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
    
    virtual void dump(OutputWithDepth<string> & out) const;
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
