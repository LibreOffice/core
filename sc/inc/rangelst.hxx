/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
