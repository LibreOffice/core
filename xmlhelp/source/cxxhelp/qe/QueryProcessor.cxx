/*************************************************************************
 *
 *  $RCSfile: QueryProcessor.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: abi $ $Date: 2001-06-19 13:41:05 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _XMLSEARCH_QE_QUERYPROCESSOR_HXX_
#include <qe/QueryProcessor.hxx>
#endif
#ifndef _XMLSEARCH_DB_DBENV_HXX_
#include <db/DBEnv.hxx>
#endif



using namespace std;
using namespace xmlsearch;
using namespace xmlsearch::excep;
using namespace xmlsearch::qe;


const double QueryProcessor::INFLpenalty = 0.0;


QueryProcessor::QueryProcessor( const rtl::OUString& installDir )
    throw( IOException )
    : env_( installDir )
{
}


QueryProcessor::~QueryProcessor()
{
    // delete env_;
}



QueryResults* QueryProcessor::processQuery( const QueryStatement& ment )
{
    Search search( &env_ );
    Query* query = processQuery( search,ment );
    query->setIgnoredElements( 0,0 );
    search.startSearch();
    return makeQueryResults( query,ment.getHitCount() );
}


Query* QueryProcessor::processQuery( Search& search,const QueryStatement& ment )
{
    sal_Int32 nValidTerms = 0, nMissingTerms = 0, nContentTerms = 0;
    double variantPenalty = 0.0;

    const sal_Int32 nHits = ment.getHitCount();
    const rtl::OUString scope = ment.getScope();
    const vector< rtl::OUString >& terms = ment.getTerms();
    const sal_Int32 nTerms = terms.size();

    vector< sal_Int32 > primary( nTerms );
    vector< sal_Int32 > missingTerms( nTerms );
    vector< vector< sal_Int32 > > columns( nTerms );

    for( int i = 0; i < nTerms; ++i )
    {
        const sal_Int32 lgt = terms[i].getLength();
        const sal_Unicode* str = terms[i].getStr();

        if( str[0] == sal_Unicode('+') )
        {
            // poor man's escape for query control
            // not implemented yet
        }
        else
        {
            ++nContentTerms;
            rtl::OUString term = terms[i].toAsciiLowerCase();
            sal_Int32 id = 0;
            std::vector< sal_Int32 > ids;
            if( str[0] == sal_Unicode('\"') )
            {
                id = env_.fetch( term.copy( 1 ) );                // goes to BtreeDict::fetch
            }
            else if( str[lgt-1] == sal_Unicode( '*' ) )
            {
                ids = env_.withPrefix( term.copy( 0,lgt - 1 ) );     // goes to BtreeDict::withPrefix
                variantPenalty = 0.0;
            }
            else
            {
                sal_Int32 formID;
                id = env_.fetch( term );

                // std::vector< rtl::OUString > variants( morph_->getVariants( term ) );
                std::vector< rtl::OUString > variants;

                for( sal_uInt32 i = 0; i < variants.size(); ++i )
                {
                    formID = env_.fetch( variants[i] );
                    if( formID > 0 && formID != id )
                        ids.push_back( formID );
                }
                variantPenalty = INFLpenalty;
            }

            if( ids.size() > 0 || id > 0 )
            {
                columns[ nValidTerms ] = ids;
                primary[ nValidTerms++ ] = id;
            }
            else
            {
                ++nMissingTerms;
                // !!! not used now (intended to fill out appropriate missing terms in QueryHits
                missingTerms.push_back( nContentTerms - 1 );
            }

        }
    }

    return search.addQuery( scope,
                            nValidTerms,nMissingTerms,nHits,
                            variantPenalty,
                            primary,
                            columns );
}



QueryResults::QueryResults( Query* query, sal_Int32 nHits )
{
    if( query )
        query->getHits( queryHits_,nHits );
}


QueryResults::~QueryResults()
{
    for( sal_Int32 i = 0; i < queryHits_.size(); ++i )
        delete queryHits_[i];
}


QueryResults* QueryProcessor::makeQueryResults( Query* query,sal_Int32 nHits )
{
    return new QueryResults( query,nHits );
}

