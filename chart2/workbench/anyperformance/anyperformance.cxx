/*************************************************************************
 *
 *  $RCSfile: anyperformance.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
