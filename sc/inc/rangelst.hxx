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

#include "global.hxx"
#include "address.hxx"
#include <ostream>
#include <vector>
#include <sal/types.h>

class ScDocument;


class SAL_WARN_UNUSED SC_DLLPUBLIC ScRangeList final : public SvRefBase
{
public:
    ScRangeList();
    ScRangeList( const ScRangeList& rList );
    ScRangeList(ScRangeList&& rList) noexcept;
    ScRangeList( const ScRange& rRange );
    virtual ~ScRangeList() override;

    ScRangeList& operator=(const ScRangeList& rList);
    ScRangeList& operator=(ScRangeList&& rList) noexcept;

    ScRefFlags      Parse( std::u16string_view, const ScDocument&,
                           formula::FormulaGrammar::AddressConvention eConv = formula::FormulaGrammar::CONV_OOO,
                           SCTAB nDefaultTab = 0, sal_Unicode cDelimiter = 0 );

    void            Format( OUString&, ScRefFlags nFlags, const ScDocument&,
                            formula::FormulaGrammar::AddressConvention eConv = formula::FormulaGrammar::CONV_OOO,
                            sal_Unicode cDelimiter = 0, bool bFullAddressNotation = false ) const;

    void            Join( const ScRange&, bool bIsInList = false );

    bool            UpdateReference( UpdateRefMode, const ScDocument*,
                                     const ScRange& rWhere,
                                     SCCOL nDx,
                                     SCROW nDy,
                                     SCTAB nDz
                                   );

    void            InsertRow( SCTAB nTab, SCCOL nColStart, SCCOL nColEnd, SCROW nRowPos, SCSIZE nSize );
    void            InsertCol( SCTAB nTab, SCROW nRowStart, SCROW nRowEnd, SCCOL nColPos, SCSIZE nSize );
    void            InsertCol( SCTAB nTab, SCCOL nColPos );

    /** For now this method assumes that nTab1 == nTab2
     * The algorithm will be much more complicated if nTab1 != nTab2
     */
    bool            DeleteArea( SCCOL nCol1, SCROW nRow1, SCTAB nTab1, SCCOL nCol2,
                                    SCROW nRow2, SCTAB nTab2 );

    const ScRange*  Find( const ScAddress& ) const;
    ScRange*        Find( const ScAddress& );
    bool            operator==( const ScRangeList& ) const;
    bool            operator!=( const ScRangeList& r ) const;
    bool            Intersects( const ScRange& ) const;
    bool            Contains( const ScRange& ) const;
    sal_uInt64      GetCellCount() const;
    ScAddress       GetTopLeftCorner() const;

    ScRangeList     GetIntersectedRange(const ScRange& rRange) const;

    void            Remove(size_t nPos);
    void            RemoveAll();

    ScRange         Combine() const;
    // Add new range, and do a partial combine up till one row back
    void            AddAndPartialCombine(const ScRange&);

    bool            empty() const { return maRanges.empty(); }
    size_t          size() const { return maRanges.size(); }
    ScRange&        operator[](size_t idx) { return maRanges[idx]; }
    const ScRange&  operator[](size_t idx) const { return maRanges[idx]; }
    ScRange&        front() { return maRanges.front(); }
    const ScRange&  front() const { return maRanges.front(); }
    ScRange&        back() { return maRanges.back(); }
    const ScRange&  back() const { return maRanges.back(); }
    void            push_back(const ScRange & rRange);
    ::std::vector<ScRange>::const_iterator begin() const { return maRanges.begin(); }
    ::std::vector<ScRange>::const_iterator end() const { return maRanges.end(); }
    ::std::vector<ScRange>::iterator begin() { return maRanges.begin(); }
    ::std::vector<ScRange>::iterator end() { return maRanges.end(); }
    void            insert(std::vector<ScRange>::iterator aPos, std::vector<ScRange>::const_iterator aSourceBegin, std::vector<ScRange>::const_iterator aSourceEnd)
                    { maRanges.insert(aPos, aSourceBegin, aSourceEnd); }

    void swap( ScRangeList& r );

private:
    ::std::vector<ScRange> maRanges;
    SCROW           mnMaxRowUsed;
};
typedef tools::SvRef<ScRangeList> ScRangeListRef;

// For use in SAL_DEBUG etc. Output format not guaranteed to be stable.
template<typename charT, typename traits>
inline std::basic_ostream<charT, traits> & operator <<(std::basic_ostream<charT, traits> & stream, const ScRangeList& rRangeList)
{
    stream << "(";
    for (size_t i = 0; i < rRangeList.size(); ++i)
    {
        if (i > 0)
            stream << ",";
        stream << rRangeList[i];
    }
    stream << ")";

    return stream;
}

// RangePairList:
//    aRange[0]: actual range,
//    aRange[1]: data for that range, e.g. Rows belonging to a ColName
class SC_DLLPUBLIC ScRangePairList final : public SvRefBase
{
public:
    virtual             ~ScRangePairList() override;
    ScRangePairList*    Clone() const;
    void                Append( const ScRangePair& rRangePair )
                        {
                            maPairs.push_back( rRangePair );
                        }
    void                Join( const ScRangePair&, bool bIsInList = false );
    void                UpdateReference( UpdateRefMode, const ScDocument*,
                                    const ScRange& rWhere,
                                    SCCOL nDx, SCROW nDy, SCTAB nDz );
    void                DeleteOnTab( SCTAB nTab );
    ScRangePair*        Find( const ScAddress& );
    ScRangePair*        Find( const ScRange& );
    std::vector<const ScRangePair*>
                        CreateNameSortedArray( ScDocument& ) const;

    void                Remove(size_t nPos);
    void                Remove(const ScRangePair & rAdr);

    size_t              size() const;
    ScRangePair&        operator[](size_t idx);
    const ScRangePair&  operator[](size_t idx) const;

private:
    ::std::vector< ScRangePair > maPairs;
};
typedef tools::SvRef<ScRangePairList> ScRangePairListRef;

extern "C"
int ScRangePairList_QsortNameCompare( const void*, const void* );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
