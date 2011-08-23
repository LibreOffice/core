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

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
namespace binfilter {

class ScDocument;

typedef ScRange* ScRangePtr;
DECLARE_LIST( ScRangeListBase, ScRangePtr )
class ScRangeList : public ScRangeListBase, public SvRefBase
{
public:
                    ScRangeList() {}
                    ScRangeList( const ScRangeList& rList );
    virtual 		~ScRangeList();
    ScRangeList&	operator=(const ScRangeList& rList);
    void			RemoveAll();
    void			Append( const ScRange& rRange )
                    {
                        ScRangePtr pR = new ScRange( rRange );
                        Insert( pR, LIST_APPEND );
                    }
    USHORT			Parse( const String&, ScDocument* = NULL,
                            USHORT nMask = SCA_VALID );
    void 			Format( String&, USHORT nFlags = 0, ScDocument* = NULL ) const;
    void			Join( const ScRange&, BOOL bIsInList = FALSE );
    BOOL 			UpdateReference( UpdateRefMode, ScDocument*,
                                    const ScRange& rWhere,
                                    short nDx, short nDy, short nDz );
/*N*/ 	BOOL			Load( SvStream&, USHORT nVer );
/*N*/ 	BOOL			Store( SvStream& ) const;
};
SV_DECL_IMPL_REF( ScRangeList );


// RangePairList: erster Range (aRange[0]) eigentlicher Range, zweiter
// Range (aRange[1]) Daten zu diesem Range, z.B. Rows eines ColName
DECLARE_LIST( ScRangePairListBase, ScRangePair* )
class ScRangePairList : public ScRangePairListBase, public SvRefBase
{
#if defined( ICC ) && defined( OS2 )
#endif
private:

public:
    virtual 		~ScRangePairList();
    ScRangePairList*	Clone() const;
    void			Append( const ScRangePair& rRangePair )
                    {
                        ScRangePair* pR = new ScRangePair( rRangePair );
                        Insert( pR, LIST_APPEND );
                    }
    void			Join( const ScRangePair&, BOOL bIsInList = FALSE );
    BOOL 			UpdateReference( UpdateRefMode, ScDocument*,
                                    const ScRange& rWhere,
                                    short nDx, short nDy, short nDz );
    ScRangePair*	Find( const ScRange& ) const;
/*N*/ 	BOOL			Load( SvStream&, USHORT nVer );
/*N*/ 	BOOL			Store( SvStream& ) const;
};
SV_DECL_IMPL_REF( ScRangePairList );

#if defined( ICC ) && defined( SC_RANGELST_CXX ) && defined( OS2 )
#endif


} //namespace binfilter
#endif
