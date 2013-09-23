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

#ifndef INCLUDED_WW8_STRUCT_BASE_HXX
#define INCLUDED_WW8_STRUCT_BASE_HXX

#include <boost/shared_ptr.hpp>
#include <doctok/WW8Document.hxx>
#include <resourcemodel/OutputWithDepth.hxx>

namespace writerfilter {
namespace doctok {

/**
   Part of a stream.

   A part can have a parent, meaning its sequence of data is a
   subsequence of its parent's sequence of data.
 */
class WW8StructBase
{
public:
    typedef SubSequence<sal_uInt8> Sequence;
    typedef boost::shared_ptr<WW8StructBase> Pointer_t;

protected:
    /**
       Stream this part was created from.
    */
    ::com::sun::star::uno::Reference<com::sun::star::io::
    XInputStream> mrStream;

    /**
       The data.
     */
    mutable Sequence mSequence;

    /**
       This part's parent.
     */
    WW8StructBase * mpParent;

    /**
       This part's offset in parent.
    */
    sal_uInt32 mnOffsetInParent;

public:
    WW8StructBase(sal_Int32 nLength)
    : mSequence(nLength), mpParent(NULL)
    {
    }

    /**
       Creates a part from a steam.

       @param rStream    the stream
       @param nOffset    offset in @a rStream to start at
       @param nCount     count of bytes in the new part
     */
    WW8StructBase(WW8Stream & rStream,
                  sal_Int32 nOffset, sal_Int32 nCount)
    : mSequence(rStream.get(nOffset, nCount)), mpParent(0)
    {
    }

    /**
       Creates a part from a sequence.

       @param rSequence    the sequence
       @param nOffset    offset in @a rSequence to start at
       @param nCount     count of bytes in the new part
     */
    WW8StructBase(const Sequence & rSequence, sal_uInt32 nOffset = 0,
                  sal_uInt32 nCount = 0)
    : mSequence(rSequence, nOffset, nCount), mpParent(0)
    {
    }

    /**
       Creates a part from a parent part.

       @param pParent    the parent
       @param nOffset    offset in @a pParent to start at
       @param nCount     count of bytes in the new part
     */
    WW8StructBase(const WW8StructBase & rParent,
                  sal_uInt32 nOffset, sal_uInt32 nCount);

    /**
       Creates a part from a parent part.

       @param pParent    the parent
       @param nOffset    offset in @a pParent to start at
       @param nCount     count of bytes in the new part
     */
    WW8StructBase(WW8StructBase * pParent,
                  sal_uInt32 nOffset, sal_uInt32 nCount)
    : mSequence(pParent->mSequence, nOffset, nCount), mpParent(pParent),
      mnOffsetInParent(nOffset)
    {
        if (nOffset + nCount > pParent->mSequence.getCount())
            throw ExceptionOutOfBounds("WW8StructBase");
    }


    virtual ~WW8StructBase()
    {
    }

    /**
       Assign a part to this part.

       After assignment this part has the same content as the assigned
       part.

       @param rSrc     part to assign

       @return this part after assignment
     */
    virtual WW8StructBase & Assign(const WW8StructBase & rSrc);

    /**
       Return count of bytes in this part.
     */
    sal_uInt32 getCount() const { return mSequence.getCount(); }

    /**
       Return unsigned byte value at an offset.

       @param offset     offset to get value from
     */
    sal_uInt8 getU8(sal_uInt32 nOffset) const;

    /**
       Return unsigned 16-bit value at an offset.

       @param offset     offset to get value from
     */
    sal_uInt16 getU16(sal_uInt32 nOffset) const;

    /**
       Return unsigned 32-bit value at an offset.

       @param offset     offset to get value from
     */
    sal_uInt32 getU32(sal_uInt32 nOffset) const;

    /**
       Return signed 8-bit value at an offset.

       @param offset     offset to get value from
     */
    sal_Int8 getS8(sal_uInt32 nOffset) const
    { return (sal_Int8) getU8(nOffset); }

    /**
       Return signed 16-bit value at an offset.

       @param offset     offset to get value from
     */
    sal_Int16 getS16(sal_uInt32 nOffset) const
    {return (sal_Int16) getU16(nOffset); }

    /**
       Return signed 32-bit value at an offset.

       @param offset     offset to get value from
     */
    sal_Int32 getS32(sal_uInt32 nOffset) const
    { return (sal_Int32) getU32(nOffset); }

    /**
       Returns byte at an index.

       @param nIndex  index in this part of the byte to return
     */
    const sal_uInt8 * get(sal_uInt32 nIndex) const
    { return &((mSequence.getSequence())[nIndex + mSequence.getOffset()]); }

    /**
       Returns two byte character string starting at an offset.

       The string has to be Pascal like, e.g. the first word contains
       the lengthof the string in characters and is followed by the
       string's characters.

       @param nOffset    offset the string starts at

       @return  the string
     */
    OUString getString(sal_uInt32 nOffset) const;

    /**
       Returns binary object for remainder of this WW8StructBase

       @param nOffset     offset where remainder starts
     */
    WW8StructBase * getRemainder(sal_uInt32 nOffset) const;

    /**
       Returns two byte character string starting at an offset with a
       given length.

       @param nOffset     offset the string starts at
       @param nLength     number of characters in the string
     */
    OUString getString(sal_uInt32 nOffset, sal_uInt32) const;

    /**
       Dump the part.

       @param o       stream to dump to
     */
    virtual void dump(OutputWithDepth<string> & o) const { mSequence.dump(o); }
};

class WW8StructBaseTmpOffset
{
    sal_uInt32 mnOffset;
    WW8StructBase * mpStructBase;

public:
    WW8StructBaseTmpOffset(WW8StructBase * pStructBase);

    sal_uInt32 set(sal_uInt32 nOffset);
    sal_uInt32 get() const;
    sal_uInt32 inc(sal_uInt32 nOffset);

    operator sal_uInt32 () const;
};

/**
   Return unsigned byte from a sequence.

   @param rSeq        sequence to get value from
   @param nOffset     offset in sequence to get value from
 */
sal_uInt8 getU8(const WW8StructBase::Sequence & rSeq,  sal_uInt32 nOffset);

/**
   Return unsigned 16-bit value from a sequence.

   @param rSeq        sequence to get value from
   @param nOffset     offset in sequence to get value from
 */
sal_uInt16 getU16(const WW8StructBase::Sequence & rSeq, sal_uInt32 nOffset);

/**
   Return unsigned 32-bit value from a sequence.

   @param rSeq        sequence to get value from
   @param nOffset     offset in sequence to get value from
 */
sal_uInt32 getU32(const WW8StructBase::Sequence & rSeq, sal_uInt32 nOffset);

}}

#endif // INCLUDED_WW8_STRUCT_BASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
