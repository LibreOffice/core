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
#ifndef _SV_MULTISEL_HXX
#define _SV_MULTISEL_HXX

#include "tools/toolsdllapi.h"
#include <tools/gen.hxx>
#include <rtl/ustring.hxx>

#include <vector>
#include <set>

typedef ::std::vector< Range* > ImpSelList;

#define SFX_ENDOFSELECTION      ULONG_MAX

class TOOLS_DLLPUBLIC MultiSelection
{
private:
    ImpSelList      aSels;      // array of SV-selections
    Range           aTotRange;  // total range of indexes
    sal_uIntPtr     nCurSubSel; // index in aSels of current selected index
    long            nCurIndex;  // current selected entry
    sal_uIntPtr     nSelCount;  // number of selected indexes
    sal_Bool        bInverseCur;// inverse cursor
    sal_Bool        bCurValid;  // are nCurIndex and nCurSubSel valid
    sal_Bool        bSelectNew; // auto-select newly inserted indexes

#ifdef _SV_MULTISEL_CXX
    TOOLS_DLLPRIVATE void           ImplClear();
    TOOLS_DLLPRIVATE size_t         ImplFindSubSelection( long nIndex ) const;
    TOOLS_DLLPRIVATE sal_Bool       ImplMergeSubSelections( size_t nPos1, size_t nPos2 );
    TOOLS_DLLPRIVATE long           ImplFwdUnselected();
#endif

public:
                    MultiSelection();
                    MultiSelection( const MultiSelection& rOrig );
                    MultiSelection( const Range& rRange );
                    ~MultiSelection();

    MultiSelection& operator= ( const MultiSelection& rOrig );
    sal_Bool        operator== ( MultiSelection& rOrig );
    sal_Bool        operator!= ( MultiSelection& rOrig )
                        { return !operator==( rOrig ); }
    sal_Bool        operator !() const
                        { return nSelCount == 0; }

    void            SelectAll( sal_Bool bSelect = sal_True );
    sal_Bool        Select( long nIndex, sal_Bool bSelect = sal_True );
    void            Select( const Range& rIndexRange, sal_Bool bSelect = sal_True );
    sal_Bool        IsSelected( long nIndex ) const;
    sal_Bool        IsAllSelected() const
                        { return nSelCount == sal_uIntPtr(aTotRange.Len()); }
    long            GetSelectCount() const { return nSelCount; }

    void            SetTotalRange( const Range& rTotRange );
    void            Insert( long nIndex, long nCount = 1 );
    void            Remove( long nIndex );

    const Range&    GetTotalRange() const { return aTotRange; }
    sal_Bool        IsCurValid() const { return bCurValid; }
    long            GetCurSelected() const { return nCurIndex; }
    long            FirstSelected( sal_Bool bInverse = sal_False );
    long            LastSelected();
    long            NextSelected();

    size_t          GetRangeCount() const { return aSels.size(); }
    const Range&    GetRange( size_t nRange ) const {
                        return *(const Range*)aSels[nRange];
                    }
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
    bool                                                   mbValidInput;

    bool setRange( const rtl::OUString& i_rNewRange, bool i_bStrict = false );
    bool insertRange( sal_Int32 nFirst, sal_Int32 nLast, bool bSequence, bool bMayAdjust );
    bool insertJoinedRanges( const std::vector< sal_Int32 >& rNumbers, bool i_bStrict );
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

    StringRangeEnumerator( const rtl::OUString& i_rInput,
                           sal_Int32 i_nMinNumber,
                           sal_Int32 i_nMaxNumber,
                           sal_Int32 i_nLogicalOffset = -1
                           );

    sal_Int32 size() const { return mnCount; }
    Iterator begin( const std::set< sal_Int32 >* i_pPossibleValues = NULL ) const;
    Iterator end( const std::set< sal_Int32 >* i_pPossibleValues = NULL ) const;

    bool isValidInput() const { return mbValidInput; }
    bool hasValue( sal_Int32 nValue, const std::set< sal_Int32 >* i_pPossibleValues = NULL ) const;

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
    static bool getRangesFromString( const rtl::OUString& i_rPageRange,
                                     std::vector< sal_Int32 >& o_rPageVector,
                                     sal_Int32 i_nMinNumber,
                                     sal_Int32 i_nMaxNumber,
                                     sal_Int32 i_nLogicalOffset = -1,
                                     std::set< sal_Int32 >* i_pPossibleValues = NULL
                                    );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
