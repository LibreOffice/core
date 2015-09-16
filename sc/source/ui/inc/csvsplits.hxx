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

#ifndef INCLUDED_SC_SOURCE_UI_INC_CSVSPLITS_HXX
#define INCLUDED_SC_SOURCE_UI_INC_CSVSPLITS_HXX

#include <sal/types.h>

#include <vector>

/** Constant for an invalid vector index. */
const sal_uInt32 CSV_VEC_NOTFOUND   = SAL_MAX_UINT32;
/** Constant for an invalid ruler position. */
const sal_Int32 CSV_POS_INVALID     = -1;

/** A vector of column splits that supports inserting, removing and moving splits. */
class ScCsvSplits
{
private:
    typedef ::std::vector< sal_Int32 >      ScSplitVector;
    typedef ScSplitVector::iterator         iterator;
    typedef ScSplitVector::const_iterator   const_iterator;

    ScSplitVector               maVec;          /// The split containter.

public:
    // *** access by position *** ---------------------------------------------

    /** Inserts a new split at position nPos into the vector.
        @return  true = split inserted (nPos was valid and empty). */
    bool                        Insert( sal_Int32 nPos );
    /** Removes a split by position.
        @return  true = split found and removed. */
    bool                        Remove( sal_Int32 nPos );
    /** Removes a range of splits in the given position range. */
    void                        RemoveRange( sal_Int32 nPosStart, sal_Int32 nPosEnd );
    /** Removes all elements from the vector. */
    void                        Clear();

    /** Returns true if at position nPos is a split. */
    bool                        HasSplit( sal_Int32 nPos ) const;

    // *** access by index *** ------------------------------------------------

    /** Searches for a split at position nPos.
        @return  the vector index of the split. */
    sal_uInt32                  GetIndex( sal_Int32 nPos ) const;
    /** Returns index of the first split greater than or equal to nPos. */
    sal_uInt32                  LowerBound( sal_Int32 nPos ) const;
    /** Returns index of the last split less than or equal to nPos. */
    sal_uInt32                  UpperBound( sal_Int32 nPos ) const;

    /** Returns the number of splits. */
    sal_uInt32                  Count() const
                                    { return maVec.size(); }
    /** Returns the position of the specified split. */
    sal_Int32                   GetPos( sal_uInt32 nIndex ) const;
    /** Returns the position of the specified split. */
    sal_Int32                   operator[]( sal_uInt32 nIndex ) const
                                    { return GetPos( nIndex ); }

private:
    /** Returns the vector index of an iterator. */
    sal_uInt32                  GetIterIndex( const_iterator aIter ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
