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

#ifndef SC_RANGELST_HXX
#define SC_RANGELST_HXX

#include "global.hxx"
#include "address.hxx"
#include <tools/solar.h>
#include <vector>

class ScDocument;

class SC_DLLPUBLIC ScRangeList : public SvRefBase
{
public:
    ScRangeList();
    ScRangeList( const ScRangeList& rList );
    ScRangeList( const ScRange& rRange );
    virtual ~ScRangeList();

    ScRangeList& operator=(const ScRangeList& rList);
    void Append( const ScRange& rRange );

    sal_uInt16 Parse( const String&, ScDocument* = NULL,
                      sal_uInt16 nMask = SCA_VALID,
                      formula::FormulaGrammar::AddressConvention eConv = formula::FormulaGrammar::CONV_OOO,
                      SCTAB nDefaultTab = 0, sal_Unicode cDelimiter = 0 );

    void            Format( String&, sal_uInt16 nFlags = 0, ScDocument* = NULL,
                            formula::FormulaGrammar::AddressConvention eConv = formula::FormulaGrammar::CONV_OOO,
                            sal_Unicode cDelimiter = 0 ) const;
    void            Format( OUString&, sal_uInt16 nFlags = 0, ScDocument* = NULL,
                            formula::FormulaGrammar::AddressConvention eConv = formula::FormulaGrammar::CONV_OOO,
                            sal_Unicode cDelimiter = 0 ) const;

    void            Join( const ScRange&, bool bIsInList = false );

    bool            UpdateReference( UpdateRefMode, ScDocument*,
                                     const ScRange& rWhere,
                                     SCsCOL nDx,
                                     SCsROW nDy,
                                     SCsTAB nDz
                                   );

    void InsertRow( SCTAB nTab, SCCOL nColStart, SCCOL nColEnd, SCROW nRowPos, SCSIZE nSize );
    void InsertCol( SCTAB nTab, SCROW nRowStart, SCROW nRowEnd, SCCOL nColPos, SCSIZE nSize );

    /** For now this method assumes that nTab1 == nTab2
     * The algorithm will be much more complicated if nTab1 != nTab2
     */
    void            DeleteArea( SCCOL nCol1, SCROW nRow1, SCTAB nTab1, SCCOL nCol2,
                                    SCROW nRow2, SCTAB nTab2 );

    const ScRange*  Find( const ScAddress& ) const;
    ScRange*        Find( const ScAddress& );
    bool            operator==( const ScRangeList& ) const;
    bool            operator!=( const ScRangeList& r ) const;
    bool            Intersects( const ScRange& ) const;
    bool            In( const ScRange& ) const;
    size_t          GetCellCount() const;
    ScAddress       GetTopLeftCorner() const;

    ScRangeList     GetIntersectedRange(const ScRange& rRange) const;

    ScRange*        Remove(size_t nPos);
    void            RemoveAll();

    ScRange         Combine() const;

    bool            empty() const;
    size_t          size() const;
    ScRange*        operator[](size_t idx);
    const ScRange*  operator[](size_t idx) const;
    ScRange*        front();
    const ScRange*  front() const;
    ScRange*        back();
    const ScRange*  back() const;
    void            push_back(ScRange* p);

private:
    ::std::vector<ScRange*> maRanges;
    typedef std::vector<ScRange*>::iterator iterator;
    typedef std::vector<ScRange*>::const_iterator const_iterator;
};
SV_DECL_IMPL_REF( ScRangeList );


// RangePairList:
//    aRange[0]: actual range,
//    aRange[1]: data for that range, e.g. Rows belonging to a ColName
class SC_DLLPUBLIC ScRangePairList : public SvRefBase
{
public:
    virtual             ~ScRangePairList();
    ScRangePairList*    Clone() const;
    void                Append( const ScRangePair& rRangePair )
                        {
                            ScRangePair* pR = new ScRangePair( rRangePair );
                            maPairs.push_back( pR );
                        }
    void                Join( const ScRangePair&, bool bIsInList = false );
    bool                UpdateReference( UpdateRefMode, ScDocument*,
                                    const ScRange& rWhere,
                                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
    void                DeleteOnTab( SCTAB nTab );
    ScRangePair*        Find( const ScAddress& ) const;
    ScRangePair*        Find( const ScRange& ) const;
    ScRangePair**       CreateNameSortedArray( size_t& nCount, ScDocument* ) const;
    bool                operator==( const ScRangePairList& ) const;

    ScRangePair*        Remove(size_t nPos);
    ScRangePair*        Remove(ScRangePair* pAdr);

    size_t              size() const;
    ScRangePair*        operator[](size_t idx);
    const ScRangePair*  operator[](size_t idx) const;

private:
    ::std::vector< ScRangePair* > maPairs;
};
SV_DECL_IMPL_REF( ScRangePairList );

extern "C"
int SAL_CALL ScRangePairList_QsortNameCompare( const void*, const void* );


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
