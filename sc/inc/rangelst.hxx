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

#ifndef SC_RANGELST_HXX
#define SC_RANGELST_HXX

#include "global.hxx"
#include "address.hxx"
#include <tools/solar.h>

class ScDocument;

typedef ScRange* ScRangePtr;
DECLARE_LIST( ScRangeListBase, ScRangePtr )
class SC_DLLPUBLIC ScRangeList : public ScRangeListBase, public SvRefBase
{
private:
    using ScRangeListBase::operator==;
    using ScRangeListBase::operator!=;

public:
                    ScRangeList() {}
                    ScRangeList( const ScRangeList& rList );
    virtual         ~ScRangeList();
    ScRangeList&    operator=(const ScRangeList& rList);
    void            RemoveAll();
    void            Append( const ScRange& rRange )
                    {
                        ScRangePtr pR = new ScRange( rRange );
                        Insert( pR, LIST_APPEND );
                    }
    sal_uInt16          Parse( const String&, ScDocument* = NULL,
                           sal_uInt16 nMask = SCA_VALID,
                           formula::FormulaGrammar::AddressConvention eConv = formula::FormulaGrammar::CONV_OOO,
                           sal_Unicode cDelimiter = 0 );
    void            Format( String&, sal_uInt16 nFlags = 0, ScDocument* = NULL,
                            formula::FormulaGrammar::AddressConvention eConv = formula::FormulaGrammar::CONV_OOO,
                            sal_Unicode cDelimiter = 0 ) const;
    void            Join( const ScRange&, sal_Bool bIsInList = sal_False );
    sal_Bool            UpdateReference( UpdateRefMode, ScDocument*,
                                    const ScRange& rWhere,
                                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
    ScRange*        Find( const ScAddress& ) const;
    sal_Bool            operator==( const ScRangeList& ) const;
    sal_Bool            operator!=( const ScRangeList& r ) const;
    sal_Bool            Intersects( const ScRange& ) const;
    sal_Bool            In( const ScRange& ) const;
    sal_uLong           GetCellCount() const;
};
SV_DECL_IMPL_REF( ScRangeList );


// RangePairList: erster Range (aRange[0]) eigentlicher Range, zweiter
// Range (aRange[1]) Daten zu diesem Range, z.B. Rows eines ColName
DECLARE_LIST( ScRangePairListBase, ScRangePair* )
class ScRangePairList : public ScRangePairListBase, public SvRefBase
{
private:
    using ScRangePairListBase::operator==;

public:
    virtual         ~ScRangePairList();
    ScRangePairList*    Clone() const;
    void            Append( const ScRangePair& rRangePair )
                    {
                        ScRangePair* pR = new ScRangePair( rRangePair );
                        Insert( pR, LIST_APPEND );
                    }
    void            Join( const ScRangePair&, sal_Bool bIsInList = sal_False );
    sal_Bool            UpdateReference( UpdateRefMode, ScDocument*,
                                    const ScRange& rWhere,
                                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
    void            DeleteOnTab( SCTAB nTab );
    ScRangePair*    Find( const ScAddress& ) const;
    ScRangePair*    Find( const ScRange& ) const;
    ScRangePair**   CreateNameSortedArray( sal_uLong& nCount, ScDocument* ) const;
    sal_Bool            operator==( const ScRangePairList& ) const;
};
SV_DECL_IMPL_REF( ScRangePairList );

extern "C" int
#ifdef WNT
__cdecl
#endif
ScRangePairList_QsortNameCompare( const void*, const void* );

#if defined( ICC ) && defined( SC_RANGELST_CXX ) && defined( OS2 )
    static int _Optlink ICCQsortRPairCompare( const void* a, const void* b)
                    { return ScRangePairList_QsortNameCompare(a,b); }
#endif


#endif
