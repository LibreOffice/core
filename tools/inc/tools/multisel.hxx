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
#ifndef _SV_MULTISEL_HXX
#define _SV_MULTISEL_HXX

#include "tools/toolsdllapi.h"
#include <tools/gen.hxx>
#include <tools/list.hxx>
#include <tools/string.hxx>

#include <vector>
#include <set>

//------------------------------------------------------------------

#ifdef _SV_MULTISEL_CXX
DECLARE_LIST( ImpSelList, Range* )
#else
#define ImpSelList List
#endif

#define SFX_ENDOFSELECTION      CONTAINER_ENTRY_NOTFOUND

//------------------------------------------------------------------

// ------------------
// - MultiSelection -
// ------------------

class TOOLS_DLLPUBLIC MultiSelection
{
private:
    ImpSelList      aSels;      // array of SV-selections
    Range           aTotRange;  // total range of indexes
    sal_uIntPtr         nCurSubSel; // index in aSels of current selected index
    long            nCurIndex;  // current selected entry
    sal_uIntPtr         nSelCount;  // number of selected indexes
    sal_Bool            bInverseCur;// inverse cursor
    sal_Bool            bCurValid;  // are nCurIndex and nCurSubSel valid
    sal_Bool            bSelectNew; // auto-select newly inserted indexes

#ifdef _SV_MULTISEL_CXX
    TOOLS_DLLPRIVATE void           ImplClear();
    TOOLS_DLLPRIVATE sal_uIntPtr            ImplFindSubSelection( long nIndex ) const;
    TOOLS_DLLPRIVATE sal_Bool           ImplMergeSubSelections( sal_uIntPtr nPos1, sal_uIntPtr nPos2 );
    TOOLS_DLLPRIVATE long           ImplFwdUnselected();
    TOOLS_DLLPRIVATE long           ImplBwdUnselected();
#endif

public:
                    MultiSelection();
                    MultiSelection( const MultiSelection& rOrig );
                    MultiSelection( const Range& rRange );
                    MultiSelection( const UniString& rString,
                                    sal_Unicode cRange = '-',
                                    sal_Unicode cSep = ';' );
                    ~MultiSelection();

    MultiSelection& operator= ( const MultiSelection& rOrig );
    sal_Bool            operator== ( MultiSelection& rOrig );
    sal_Bool            operator!= ( MultiSelection& rOrig )
                        { return !operator==( rOrig ); }
    sal_Bool            operator !() const
                        { return nSelCount == 0; }

    void            SelectAll( sal_Bool bSelect = sal_True );
    sal_Bool            Select( long nIndex, sal_Bool bSelect = sal_True );
    void            Select( const Range& rIndexRange, sal_Bool bSelect = sal_True );
    sal_Bool            IsSelected( long nIndex ) const;
    sal_Bool            IsAllSelected() const
                        { return nSelCount == sal_uIntPtr(aTotRange.Len()); }
    long            GetSelectCount() const { return nSelCount; }

    void            SetTotalRange( const Range& rTotRange );
    void            Insert( long nIndex, long nCount = 1 );
    void            Remove( long nIndex );
    void            Append( long nCount = 1 );

    const Range&    GetTotalRange() const { return aTotRange; }
    sal_Bool            IsCurValid() const { return bCurValid; }
    long            GetCurSelected() const { return nCurIndex; }
    long            FirstSelected( sal_Bool bInverse = sal_False );
    long            LastSelected();
    long            NextSelected();
    long            PrevSelected();

    sal_uIntPtr         GetRangeCount() const { return aSels.Count(); }
    const Range&    GetRange( sal_uIntPtr nRange ) const { return *(const Range*)aSels.GetObject(nRange); }
};

class TOOLS_DLLPUBLIC StringRangeEnumerator
{
    struct Range
    {
        sal_Int32   nFirst;
        sal_Int32   nLast;

        Range() : nFirst( -1 ), nLast( -1 ) {}
        Range( sal_Int32 i_nFirst, sal_Int32 i_nLast ) : nFirst( i_nFirst ), nLast( i_nLast ) {}
    };
    std::vector< StringRangeEnumerator::Range >            maSequence;
    sal_Int32                                              mnCount;
    sal_Int32                                              mnMin;
    sal_Int32                                              mnMax;
    sal_Int32                                              mnOffset;

    bool insertRange( sal_Int32 nFirst, sal_Int32 nLast, bool bSequence, bool bMayAdjust );
    bool checkValue( sal_Int32, const std::set< sal_Int32 >* i_pPossibleValues = NULL ) const;
public:
    class TOOLS_DLLPUBLIC Iterator
    {
        const StringRangeEnumerator*      pEnumerator;
        const std::set< sal_Int32 >*      pPossibleValues;
        sal_Int32                         nRangeIndex;
        sal_Int32                         nCurrent;

        friend class StringRangeEnumerator;
        Iterator( const StringRangeEnumerator* i_pEnum,
                  const std::set< sal_Int32 >* i_pPossibleValues,
                  sal_Int32 i_nRange,
                  sal_Int32 i_nCurrent )
        : pEnumerator( i_pEnum ), pPossibleValues( i_pPossibleValues )
        , nRangeIndex( i_nRange ), nCurrent( i_nCurrent ) {}
    public:
        Iterator() : pEnumerator( NULL ), pPossibleValues( NULL ), nRangeIndex( -1 ), nCurrent( -1 ) {}
        Iterator& operator++();
        sal_Int32 operator*() const;
        bool operator==(const Iterator&) const;
        bool operator!=(const Iterator& i_rComp) const
        { return ! (*this == i_rComp); }
    };

    friend class StringRangeEnumerator::Iterator;

    StringRangeEnumerator() : mnCount( 0 ), mnMin( -1 ), mnMax( -1 ), mnOffset( -1 ) {}
    StringRangeEnumerator( const rtl::OUString& i_rInput,
                           sal_Int32 i_nMinNumber = -1,
                           sal_Int32 i_nMaxNumber = -1,
                           sal_Int32 i_nLogicalOffset = -1
                           );

    size_t size() const { return size_t(mnCount); }
    Iterator begin( const std::set< sal_Int32 >* i_pPossibleValues = NULL ) const;
    Iterator end( const std::set< sal_Int32 >* i_pPossibleValues = NULL ) const;

    sal_Int32 getMin() const { return mnMin; }
    void setMin( sal_Int32 i_nMinValue ) { mnMin = i_nMinValue; }
    sal_Int32 getMax() const { return mnMax; }
    void setMax( sal_Int32 i_nMaxValue ) { mnMax = i_nMaxValue; }
    sal_Int32 getLogicalOffset() const { return mnOffset; }
    void setLogicalOffset( sal_Int32 i_nOffset ) { mnOffset = i_nOffset; }

    bool setRange( const rtl::OUString& i_rNewRange, bool i_bStrict = false );
    bool hasValue( sal_Int32 nValue, const std::set< sal_Int32 >* i_pPossibleValues = NULL ) const;


    /**
    i_rPageRange:     the string to be changed into a sequence of numbers
                      valid format example "5-3,9,9,7-8" ; instead of ',' ';' or ' ' are allowed as well
    o_rPageVector:    the output sequence of numbers
    i_nLogicalOffset: an offset to be applied to each number in the string before inserting it in the resulting sequence
                      example: a user enters page numbers from 1 to n (since that is logical)
                               of course usable page numbers in code would start from 0 and end at n-1
                               so the logical offset would be -1
    i_nMinNumber:     the minimum allowed number, a negative number means no minimum check
    i_nMaxNumber:     the maximum allowed number, a negative number means no maximum check

    @returns: true if the input string was valid, o_rPageVector will contain the resulting sequence
              false if the input string was invalid, o_rPageVector will be unchanged

    behavior:
    - only non-negative sequence numbers are allowed
    - only non-negative values in the input string are allowed
    - the string "-3" will be either
      * an error if no minimum is given
      * or result in the sequence i_nMinNumber to 3
    - the string "3-" will be either
      * an error if no maximum is given
      * or result in the seqeuence 3 to i_nMaxNumber
    - an empty string as input is valid and will result in the range [min,max] if given
      or an empty vector, if not
    */
    static bool getRangesFromString( const rtl::OUString& i_rPageRange,
                                     std::vector< sal_Int32 >& o_rPageVector,
                                     sal_Int32 i_nMinNumber = -1,
                                     sal_Int32 i_nMaxNumber = -1,
                                     sal_Int32 i_nLogicalOffset = -1,
                                     std::set< sal_Int32 >* i_pPossibleValues = NULL
                                    );
};

#endif  // _SV_MULTISEL_HXX
