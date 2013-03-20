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

#pragma once
#if 1

#include <boost/shared_ptr.hpp>
#include "WW8StructBase.hxx"

namespace writerfilter {
namespace doctok
{

class Empty
{
public:
    typedef boost::shared_ptr<Empty> Pointer_t;

    Empty() {}
    virtual ~Empty() {}

    sal_uInt32 getSize() { return 0; }
};

template <class T>
/**
   Plex in File

   A PLCF is a concatenation of two arrays. The first array contains
   file character positions. The second array contains elements of
   type T. If the first array contains N elements, the second contains
   N - 1 elements. The N-th element in the first array corresponds to
   the N-th element of the second array.

   The second array is referred to as the payload.
 */
class PLCF : public WW8StructBase
{
    /// number of entries
    sal_uInt32 nEntryCount;

    /// offset to payload
    sal_uInt32 nPayloadOffset;

    /// internal method to calculate the number of entries
    sal_uInt32 getEntryCount_() const;

public:
    typedef boost::shared_ptr< PLCF< T > > Pointer_t;

    PLCF(sal_uInt32 nLength)
    : WW8StructBase(nLength), nEntryCount(getEntryCount_()),
      nPayloadOffset((nEntryCount + 1) * 4)
    {
    }

    PLCF(WW8Stream & rStream,
         sal_Int32 nOffset, sal_Int32 nCount)
    : WW8StructBase(rStream, nOffset, nCount),
      nEntryCount(getEntryCount_()),
      nPayloadOffset((nEntryCount + 1) * 4)
    {
    }

    PLCF(const Sequence & rSequence)
    : WW8StructBase(rSequence), nEntryCount(getEntryCount_()),
      nPayloadOffset((nEntryCount + 1) * 4)
    {
    }

    /**
       Return the number of elements in the PLCF-
     */
    sal_uInt32 getEntryCount() const { return nEntryCount; }

    /**
       Return the file character position of a certain element.

       @param nIndex      the index of the element
     */
    sal_uInt32 getFc(sal_uInt32 nIndex) const;

    /**
       Return a C++ pointer to a certain payload entry.

       @param nIndex      the index of the element
     */
    T * getEntryPointer(sal_uInt32 nIndex) const;

    /**
       Return a shared pointer to a certain payload element.

       @param nIndex      the index of the element
     */
    typename T::Pointer_t getEntry(sal_uInt32 nIndex) const;

    /**
       Return a C++ pointer a certain payload element.

       @param nFc         the file character position of the element
     */
    T * getEntryByFc(sal_uInt32 nFc) const;

    virtual void dump(OutputWithDepth<string> & out) const;
};

template <class T>
sal_uInt32 PLCF<T>::getEntryCount_() const
{
    return (getCount() - 4) / (T::getSize() + 4);
}

template <class T>
sal_uInt32 PLCF<T>::getFc(sal_uInt32 nIndex) const
{
    return getU32(nIndex * 4);
}

template <class T>
T * PLCF<T>::getEntryPointer(sal_uInt32 nIndex) const
{
    return new T(mSequence, nPayloadOffset + nIndex * T::getSize(),
                 T::getSize());
}

template <class T>
typename T::Pointer_t PLCF<T>::getEntry(sal_uInt32 nIndex) const
{
    typename T::Pointer_t pResult(getEntryPointer(nIndex));

    return pResult;
}


template <class T>
T * PLCF<T>::getEntryByFc(sal_uInt32 nFc) const
{
    T * pResult = NULL;

    sal_uInt32 n = getEntryCount();

    while (getFc(n) > nFc)
        n--;

    pResult = getEntryPointer(n);

    return pResult;
}

template <class T>
void PLCF<T>::dump(OutputWithDepth<string> & output_) const
{
    output_.addItem("<plcf>");
    WW8StructBase::dump(output_);

    sal_uInt32 nCount = getEntryCount();
    for (sal_uInt32 n = 0; n < nCount; ++n)
    {
        Fc aFc = getFc(n);
        typename T::Pointer_t pT = getEntry(n);

        output_.addItem("<plcfentry cpandfc=\"" + aFc.toString() + "\">");
        pT->dump(output_);
        output_.addItem("</plcfentry>");
    }
    output_.addItem("</plcf>>");
}

}}

#endif // INCLUDED_PLCF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
