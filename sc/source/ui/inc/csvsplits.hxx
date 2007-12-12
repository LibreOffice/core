/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: csvsplits.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2007-12-12 13:21:02 $
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

// ============================================================================

#ifndef _SC_CSVSPLITS_HXX
#define _SC_CSVSPLITS_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#include <vector>


// ============================================================================

/** Constant for an invalid vector index. */
const sal_uInt32 CSV_VEC_NOTFOUND   = SAL_MAX_UINT32;
/** Constant for an invalid ruler position. */
const sal_Int32 CSV_POS_INVALID     = -1;


// ----------------------------------------------------------------------------

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
    inline sal_uInt32           Count() const
                                    { return maVec.size(); }
    /** Returns the position of the specified split. */
    sal_Int32                   GetPos( sal_uInt32 nIndex ) const;
    /** Returns the position of the specified split. */
    inline sal_Int32            operator[]( sal_uInt32 nIndex ) const
                                    { return GetPos( nIndex ); }

private:
    /** Returns the vector index of an iterator. */
    sal_uInt32                  GetIterIndex( const_iterator aIter ) const;
};


// ============================================================================

#endif

