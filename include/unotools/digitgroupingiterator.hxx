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

#ifndef INCLUDED_UNOTOOLS_DIGITGROUPINGITERATOR_HXX
#define INCLUDED_UNOTOOLS_DIGITGROUPINGITERATOR_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <sal/log.hxx>

namespace utl {

/** Iterator to be used with a digit grouping as obtained through
    LocaleDataWrapper::getDigitGrouping().

    The iterator advances over the digit groupings, returning the number of
    digits per group. If the last group was encountered the iterator will
    always return the last grouping.

    Grouping values are sanitized to be >= 0, even if originally signed
    sal_Int32.

    Usage example with a string buffer containing a decimal representation of
    an integer number. Note that of course this loop could be optimized to not
    count single characters but hunks of groups instead using the get() method,
    this is just for illustrating usage. Anyway, for double values it is highly
    more efficient to use ::rtl::math::doubleToString() and pass the grouping
    sequence, instead of using this iterator and inserting characters into
    strings.

    DigitGroupingIterator aGrouping(...)
    sal_Int32 nCount = 0;
    sal_Int32 n = aBuffer.getLength();
    // >1 because we don't want to insert a separator if there is no leading digit.
    while (n-- > 1)
    {
        if (++nCount >= aGrouping.getPos())
        {
            aBuffer.insert( n, cSeparator);
            nGroupDigits = aGrouping.advance();
        }
    }

 */

class DigitGroupingIterator
{
    const ::com::sun::star::uno::Sequence< sal_Int32 > maGroupings;

    sal_Int32   mnGroup;        // current active grouping
    sal_Int32   mnDigits;       // current active digits per group
    sal_Int32   mnNextPos;      // position (in digits) of next grouping

    void setInfinite()
    {
        mnGroup = maGroupings.getLength();
    }

    bool isInfinite() const
    {
        return mnGroup >= maGroupings.getLength();
    }

    sal_Int32 getGrouping() const
    {
        if (mnGroup < maGroupings.getLength())
        {
            sal_Int32 n = maGroupings[mnGroup];
            SAL_WARN_IF( n < 0, "unotools.i18n", "DigitGroupingIterator::getGrouping: negative grouping");
            if (n < 0)
                n = 0;      // sanitize ...
            return n;
        }
        return 0;
    }

    void setPos()
    {
        // someone might be playing jokes on us, so check for overflow
        if (mnNextPos <= SAL_MAX_INT32 - mnDigits)
            mnNextPos += mnDigits;
    }

    void setDigits()
    {
        sal_Int32 nPrev = mnDigits;
        mnDigits = getGrouping();
        if (!mnDigits)
        {
            mnDigits = nPrev;
            setInfinite();
        }
        setPos();
    }

    void initGrouping()
    {
        mnDigits = 3;       // just in case of constructed with empty grouping
        mnGroup = 0;
        mnNextPos = 0;
        setDigits();
    }

    DigitGroupingIterator( const DigitGroupingIterator & ) = delete;
    DigitGroupingIterator & operator=( const DigitGroupingIterator & ) = delete;

public:

    explicit DigitGroupingIterator( const ::com::sun::star::uno::Sequence< sal_Int32 > & rGroupings )
        : maGroupings( rGroupings)
    {
        initGrouping();
    }

    /** Advance iterator to next grouping. */
    DigitGroupingIterator & advance()
    {
        if (isInfinite())
            setPos();
        else
        {
            ++mnGroup;
            setDigits();
        }
        return *this;
    }

    /** Obtain current grouping. Always > 0. */
    sal_Int32 get() const
    {
        return mnDigits;
    }

    /** The next position (in integer digits) from the right where to insert a
        group separator. */
    sal_Int32 getPos()
    {
        return mnNextPos;
    }

    /** Reset iterator to start again from the right beginning. */
    void reset()
    {
        initGrouping();
    }

    /** Create a sequence of bool values containing positions where to add a
        separator when iterating forward over a string and copying digit per
        digit. For example, for grouping in thousands and nIntegerDigits==7 the
        sequence returned would be {1,0,0,1,0,0,0} so the caller would add a
        separator after the 1st and the 4th digit. */
    static ::com::sun::star::uno::Sequence< sal_Bool > createForwardSequence(
            sal_Int32 nIntegerDigits,
            const ::com::sun::star::uno::Sequence< sal_Int32 > & rGroupings )
    {
        if (nIntegerDigits <= 0)
            return ::com::sun::star::uno::Sequence< sal_Bool >();
        DigitGroupingIterator aIterator( rGroupings);
        ::com::sun::star::uno::Sequence< sal_Bool > aSeq( nIntegerDigits);
        sal_Bool* pArr = aSeq.getArray();
        for (sal_Int32 j = 0; --nIntegerDigits >= 0; ++j)
        {
            if (j == aIterator.getPos())
            {
                pArr[nIntegerDigits] = sal_True;
                aIterator.advance();
            }
            else
                pArr[nIntegerDigits] = sal_False;
        }
        return aSeq;
    }
};

} // namespace utl

#endif // INCLUDED_UNOTOOLS_DIGITGROUPINGITERATOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
