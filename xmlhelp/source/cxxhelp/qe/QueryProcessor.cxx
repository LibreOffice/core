/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: QueryProcessor.cxx,v $
 * $Revision: 1.10 $
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
#include "precompiled_xmlhelp.hxx"
#include <qe/QueryProcessor.hxx>
#include <db/DBEnv.hxx>



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

                for( sal_uInt32 j = 0; j < variants.size(); ++j )
                {
                    formID = env_.fetch( variants[j] );
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
    for( sal_uInt32 i = 0; i < queryHits_.size(); ++i )
        delete queryHits_[i];
}


QueryResults* QueryProcessor::makeQueryResults( Query* query,sal_Int32 nHits )
{
    return new QueryResults( query,nHits );
}

