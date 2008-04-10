/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: anyperformance.cxx,v $
 * $Revision: 1.4 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include <cstdlib>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <iterator>

#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

using namespace ::std;
using namespace ::com::sun::star;

#define DBG_OUTPUT

typedef double testType;
// typedef sal_Int32 testType;

struct lcl_Data
{
    int m_nSeries;
    int m_nPoints;
    enum Type
    {
        TYPE_DOUBLE,
        TYPE_ANY
    } m_eType;

    lcl_Data() :
            m_nSeries( 10 ),
            m_nPoints( 10000 ),
            m_eType( TYPE_DOUBLE )
    {}

    lcl_Data( int nSeries, int nPoints, Type eType ) :
            m_nSeries( nSeries ),
            m_nPoints( nPoints ),
            m_eType( eType )
    {}
};

lcl_Data lcl_parseArgs( const vector< string > & aArgs )
{
    lcl_Data aResult;

    if( aArgs.size() > 1 )
    {
        aResult.m_eType = ( aArgs[ 1 ] == string( "any" ))
            ? lcl_Data::TYPE_ANY
            : lcl_Data::TYPE_DOUBLE;
    }
    else
    {
        cerr << "Usage: " << aArgs[ 0 ] << " double|any [series] [points-per-series]" << endl;
        exit( EXIT_FAILURE );
    }

    if( aArgs.size() > 2 )
    {
        stringstream aStrStream;
        aStrStream << aArgs[ 2 ];
        aStrStream >> aResult.m_nSeries;
    }
    if( aArgs.size() > 3 )
    {
        stringstream aStrStream;
        aStrStream << aArgs[ 3 ];
        aStrStream >> aResult.m_nPoints;
    }

    return aResult;
}

template< typename T >
    struct lcl_MakeAny : public unary_function< uno::Any, T >
    {
        uno::Any operator() ( const T & rVal )
        {
            return uno::makeAny( rVal );
        }
    };

// ========================================
//                   MAIN
// ========================================

int SAL_CALL main( int argc, char* argv[] )
{
    // parse arguments
    // arg0: number of series
    // arg1: number of points per series
    vector< string > aArgs( argc );
    copy( argv, argv + argc, aArgs.begin() );

    lcl_Data aParams( lcl_parseArgs( aArgs ));

    // create data
    vector< testType > aSeries( aParams.m_nPoints, 23 );
    vector< vector< testType > > aData( aParams.m_nSeries, aSeries );

    switch( aParams.m_eType )
    {
        case lcl_Data::TYPE_DOUBLE:
#ifdef DBG_OUTPUT
            cout << "Testing native type: " << endl;
#endif
            {
                uno::Sequence< uno::Sequence< testType > > aSeq;
                aSeq.realloc( aParams.m_nSeries );
                uno::Sequence< testType > * pArr = aSeq.getArray();

                for( vector< vector< testType > >::const_iterator aIter = aData.begin();
                     aIter != aData.end();
                     ++aIter, ++pArr )
                {
                    pArr->realloc( aParams.m_nPoints );
                    copy( (*aIter).begin(), (*aIter).end(),
                          pArr->getArray() );
                }
            }
            break;
        case lcl_Data::TYPE_ANY:
#ifdef DBG_OUTPUT
            cout << "Testing Anies" << endl;
#endif
            {
                uno::Sequence< uno::Sequence< uno::Any > > aSeq;
                aSeq.realloc( aParams.m_nSeries );
                uno::Sequence< uno::Any > * pArr = aSeq.getArray();

                for( vector< vector< testType > >::const_iterator aIter = aData.begin();
                     aIter != aData.end();
                     ++aIter, ++pArr )
                {
                    pArr->realloc( aParams.m_nPoints );
                    transform( (*aIter).begin(), (*aIter).end(),
                               pArr->getArray(), lcl_MakeAny< testType >() );
                }
            }
            break;
    }


#if 0
    // output data
    for( vector< vector< testType > >::const_iterator aIter = aData.begin();
         aIter != aData.end();
         ++aIter)
    {
        cout << "Series: * ";
        copy( (*aIter).begin(), (*aIter).end(),
              ostream_iterator< testType >( cout, " * " ));
        cout << endl;
    }
#endif

    return EXIT_SUCCESS;
}
