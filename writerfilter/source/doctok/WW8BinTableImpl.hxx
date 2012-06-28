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

#ifndef INCLUDED_WW8_BIN_TABLE_IMPL_HXX
#define INCLUDED_WW8_BIN_TABLE_IMPL_HXX

#include <istream>
#include <WW8BinTable.hxx>
#include <PLCF.hxx>

#include <boost/unordered_map.hpp>

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

    virtual ~PageNumber()
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
    mutable boost::unordered_map<Fc, sal_uInt32, FcHash> mPageMap;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
