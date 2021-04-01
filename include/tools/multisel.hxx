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
#ifndef INCLUDED_TOOLS_MULTISEL_HXX
#define INCLUDED_TOOLS_MULTISEL_HXX

#include <tools/toolsdllapi.h>
#include <tools/gen.hxx>
#include <rtl/ustring.hxx>

#include <vector>
#include <o3tl/sorted_vector.hxx>

#define SFX_ENDOFSELECTION      (-1)

class SAL_WARN_UNUSED TOOLS_DLLPUBLIC MultiSelection
{
private:
    std::vector< Range >
                    aSels;      // array of SV-selections
    Range           aTotRange;  // total range of indexes
    sal_Int32       nCurSubSel; // index in aSels of current selected index
    sal_Int32       nCurIndex;  // current selected entry
    sal_Int32       nSelCount;  // number of selected indexes
    bool            bCurValid;  // are nCurIndex and nCurSubSel valid

    TOOLS_DLLPRIVATE void           ImplClear();
    TOOLS_DLLPRIVATE sal_Int32      ImplFindSubSelection( sal_Int32 nIndex ) const;
    TOOLS_DLLPRIVATE void           ImplMergeSubSelections( sal_Int32 nPos1, sal_Int32 nPos2 );

public:
                    MultiSelection();
                    MultiSelection( const MultiSelection& rOrig );
                    MultiSelection( const Range& rRange );
                    ~MultiSelection();

    MultiSelection& operator= ( const MultiSelection& rOrig );

    void            SelectAll( bool bSelect = true );
    bool            Select( sal_Int32 nIndex, bool bSelect = true );
    void            Select( const Range& rIndexRange, bool bSelect = true );
    bool            IsSelected( sal_Int32 nIndex ) const;
    bool            IsAllSelected() const
                        { return nSelCount == aTotRange.Len(); }
    sal_Int32       GetSelectCount() const { return nSelCount; }

    void            SetTotalRange( const Range& rTotRange );
    void            Insert( sal_Int32 nIndex, sal_Int32 nCount = 1 );
    void            Remove( sal_Int32 nIndex );
    void            Reset();

    const Range&    GetTotalRange() const { return aTotRange; }
    sal_Int32       FirstSelected();
    sal_Int32       LastSelected();
    sal_Int32       NextSelected();

    sal_Int32       GetRangeCount() const { return aSels.size(); }
    const Range&    GetRange( sal_Int32 nRange ) const { return aSels[nRange]; }
};

class SAL_WARN_UNUSED TOOLS_DLLPUBLIC StringRangeEnumerator
{
    struct Range
    {
        sal_Int32   nFirst;
        sal_Int32   nLast;

        Range( sal_Int32 i_nFirst, sal_Int32 i_nLast ) : nFirst( i_nFirst ), nLast( i_nLast ) {}
    };
    std::vector< StringRangeEnumerator::Range >            maSequence;
    sal_Int32                                              mnCount;
    sal_Int32                                              mnMin;
    sal_Int32                                              mnMax;
    sal_Int32                                              mnOffset;
    bool                                                   mbValidInput;

    bool setRange( const OUString& i_rNewRange );
    bool insertRange( sal_Int32 nFirst, sal_Int32 nLast, bool bSequence );
    void insertJoinedRanges( const std::vector< sal_Int32 >& rNumbers );
    bool checkValue( sal_Int32, const o3tl::sorted_vector< sal_Int32 >* i_pPossibleValues = nullptr ) const;
public:
    class TOOLS_DLLPUBLIC Iterator
    {
        const StringRangeEnumerator*      pEnumerator;
        const o3tl::sorted_vector< sal_Int32 >* pPossibleValues;
        sal_Int32                         nRangeIndex;
        sal_Int32                         nCurrent;

        friend class StringRangeEnumerator;
        Iterator( const StringRangeEnumerator* i_pEnum,
                  const o3tl::sorted_vector< sal_Int32 >* i_pPossibleValues,
                  sal_Int32 i_nRange,
                  sal_Int32 i_nCurrent )
        : pEnumerator( i_pEnum ), pPossibleValues( i_pPossibleValues )
        , nRangeIndex( i_nRange ), nCurrent( i_nCurrent ) {}

    public:
        Iterator& operator++();
        sal_Int32 operator*() const { return nCurrent;}
        bool operator==(const Iterator&) const;
        bool operator!=(const Iterator& i_rComp) const
        { return ! (*this == i_rComp); }
    };

    friend class StringRangeEnumerator::Iterator;

    StringRangeEnumerator( const OUString& i_rInput,
                           sal_Int32 i_nMinNumber,
                           sal_Int32 i_nMaxNumber,
                           sal_Int32 i_nLogicalOffset = -1
                           );

    sal_Int32 size() const { return mnCount; }
    Iterator begin( const o3tl::sorted_vector< sal_Int32 >* i_pPossibleValues = nullptr ) const;
    Iterator end( const o3tl::sorted_vector< sal_Int32 >* i_pPossibleValues = nullptr ) const;

    bool hasValue( sal_Int32 nValue, const o3tl::sorted_vector< sal_Int32 >* i_pPossibleValues = nullptr ) const;

    /**
    i_rPageRange:     the string to be changed into a sequence of numbers
                      valid format example "5-3,9,9,7-8" ; instead of ',' ';' or ' ' are allowed as well
    o_rPageVector:    the output sequence of numbers
    i_nLogicalOffset: an offset to be applied to each number in the string before inserting it in the resulting sequence
                      example: a user enters page numbers from 1 to n (since that is logical)
                               of course usable page numbers in code would start from 0 and end at n-1
                               so the logical offset would be -1
    i_nMinNumber:     the minimum allowed number
    i_nMaxNumber:     the maximum allowed number

    @returns: true if the input string was valid, o_rPageVector will contain the resulting sequence
              false if the input string was invalid, o_rPageVector will contain
                    the sequence that parser is able to extract

    behavior:
    - only non-negative sequence numbers are allowed
    - only non-negative values in the input string are allowed
    - the string "-3" means the sequence i_nMinNumber to 3
    - the string "3-" means the sequence 3 to i_nMaxNumber
    - the string "-" means the sequence i_nMinNumber to i_nMaxNumber
    - single number that doesn't fit in [i_nMinNumber,i_nMaxNumber] will be ignored
    - range that doesn't fit in [i_nMinNumber,i_nMaxNumber] will be adjusted
    */
    static bool getRangesFromString( const OUString& i_rPageRange,
                                     std::vector< sal_Int32 >& o_rPageVector,
                                     sal_Int32 i_nMinNumber,
                                     sal_Int32 i_nMaxNumber,
                                     sal_Int32 i_nLogicalOffset = -1,
                                     o3tl::sorted_vector< sal_Int32 > const * i_pPossibleValues = nullptr
                                    );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
