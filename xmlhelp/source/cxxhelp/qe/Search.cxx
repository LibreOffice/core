/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Search.cxx,v $
 * $Revision: 1.11 $
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
#include <qe/Search.hxx>


using namespace std;
using namespace xmlsearch;
using namespace xmlsearch::qe;




/******************************************************************************/
/*                                                                            */
/*                  Emtpy  ConceptData/Query                                  */
/*                                                                            */
/******************************************************************************/


class EmptyConceptData
    : public ConceptData
{
public:

    virtual void generateFillers( std::vector< RoleFiller* >& , sal_Int32 ) { }

};  // end class EmptyQuery



class EmptyQuery
    : public Query
{
public:

    EmptyQuery()
        : Query( 0,0,0,0,0 ),
          conceptDataInstance_( )
    {
    }

    ConceptData* makeConceptData( sal_Int32 col,
                                  sal_Int32 concept,
                                  double penalty,
                                  sal_Int32 queryNo )
    {
        (void)col;
        (void)concept;
        (void)penalty;
        (void)queryNo;

        return &conceptDataInstance_;
    }

private:

    EmptyConceptData  conceptDataInstance_;

};  // end class EmptyQuery



/******************************************************************************/
/*                                                                            */
/*                    ConceptData1/Query1                                     */
/*                                                                            */
/******************************************************************************/


class ConceptData1
    : public ConceptData
{
public:

    ConceptData1( sal_Int32 id,
                  sal_Int32 role,
                  double score,
                  sal_Int32 queryNo,
                  sal_Int32 nColumns,
                  ContextTables* ctxInfo,
                  sal_Int32 code )
        : ConceptData( id,role,score,queryNo,nColumns,ctxInfo ),
          fieldCode_( code )
    {
    }

    virtual void generateFillers( std::vector< RoleFiller* >& array, sal_Int32 pos );


private:

    sal_Int32 fieldCode_;

};  // end class ConceptData1


class Query1
    : public Query
{
public:

    Query1( XmlIndex* env,
            sal_Int32 nColumns,
            sal_Int32 nHits,
            sal_Int32 missingPenaltiesL,
            double* missingPenalties,
            sal_Int32 fieldCode )
        : Query( env,nColumns,nHits,missingPenaltiesL,missingPenalties ),
          searchFieldCode_( fieldCode )
    {
    }

    virtual ConceptData* makeConceptData( sal_Int32 col,
                                          sal_Int32 concept,
                                          double score,
                                          sal_Int32 query );


private:

    sal_Int32 searchFieldCode_;
};



/********************************************************************************/
// Impl
/********************************************************************************/


void ConceptData1::generateFillers( std::vector< RoleFiller* >& array, sal_Int32 pos )
{
    if( array[ queryNo_ ] != RoleFiller::STOP() )
    {   // not 'prohibited'
        // !!! candidate for a single _ctx op
        sal_Int32 ancestor = ctx_->firstParentWithCode(pos,fieldCode_);
        if( ancestor != -1 )
        {


            RoleFiller* p = new RoleFiller( nColumns_,
                                            this,
                                            role_,
                                            pos,
                                            ancestor,
                                            pos + proximity_);
            p->use( array,queryNo_ );
        }
    }

    if( next_.is() )
        next_->generateFillers( array,pos );
}



ConceptData* Query1::makeConceptData( sal_Int32 col,
                                      sal_Int32 concept,
                                      double score,
                                      sal_Int32 query )
{
    return new ConceptData1( concept,col,score,query,nColumns_,ctx_,searchFieldCode_ );
}



/******************************************************************************/
/*                                                                            */
/*                       QueryFactoryImpl                                     */
/*                                                                            */
/******************************************************************************/


class QueryFactoryImpl
{
public:

    Query* makeQuery( XmlIndex* env,const rtl::OUString& context,sal_Int32 nColumns,sal_Int32 nHits);

    Query* empty() { return &emptyQueryInstance_; }

private:

    EmptyQuery   emptyQueryInstance_;

};  // end class QueryFactoryImpl




Query* QueryFactoryImpl::makeQuery( XmlIndex* env,
                                    const rtl::OUString& context,
                                    sal_Int32 nColumns,
                                    sal_Int32 nHits )
{
    if( ! context.getLength() )
        return new Query( env,nColumns,nHits,0,0 );
    else if( context.indexOf( sal_Unicode( '|' ) ) != -1 )
    {
        return 0;      //t
    }
    else if( context.indexOf( rtl::OUString::createFromAscii( "//" ) ) != -1 )
    {
        return 0;      //t
    }
    else if( context.indexOf( sal_Unicode( '/' ) ) != -1 )
    {
        return 0;      //t
    }
    else if( context.indexOf( sal_Unicode( '@' ) ) != -1 )
    {
        return 0;     //t
    }
    else if( context.indexOf( sal_Unicode( '[' ) ) != -1 )
    {
        return 0;     //t
    }
    else
    {
        sal_Int32 code = env->getContextInfo()->linkCode( context );
        if( code != -1 )
            return new Query1( env,nColumns,nHits,0,0,code);
        else
            return &emptyQueryInstance_;
    }
}



Search::Search( XmlIndex* env )
    : env_( env ),
      queryFactory_( 0 ),
      nextDocGenHeap_(),
      firstGenerator_(),
      free2_( 0 ),
      limit_( 0 ),
      base_( 0 ),
      concepts_( new sal_Int32[ ConceptGroupGenerator::NConceptsInGroup ] ),
      dataL_( 0 ),
      data_( 0 )
{
}



Search::~Search()
{
    sal_uInt32 i;
    Query* stopq = queryFactory_ ? queryFactory_->empty() : 0;
    ConceptData* stopc = stopq ? stopq->makeConceptData( 0,0,0.0,0 ) : 0;
    (void)stopc;

    for( i = 0; i < queries_.size(); ++i )
        if( queries_[i] != stopq )
            delete queries_[i];

    delete[] concepts_;

    delete queryFactory_;
}




Query* Search::addQuery( const rtl::OUString& context,
                         sal_Int32 nValidTerms,sal_Int32 nMissingTerms,sal_Int32 nHits,
                         double variantPenalty,
                         const std::vector< sal_Int32 >& primary,
                         const std::vector< std::vector< sal_Int32 > >& columns )
{
    // by now, scope == context
    if( ! queryFactory_ )
        queryFactory_ = new QueryFactoryImpl();

    Query* query = queryFactory_->makeQuery( env_,context,nValidTerms,nHits );
    query->missingTerms( nMissingTerms );
    queries_.push_back( query );

    for( sal_Int32 i = 0; i < nValidTerms; ++i )
    {
        if( primary[i] > 0 )
            addTerm( i,primary[i],0.0 );

        for( sal_uInt32 j = 0; j < columns[i].size(); ++j )
            addTerm( i,columns[i][j],variantPenalty );
    }

    // start stop
    query->addControlConceptData( this,queries_.size()-1 );
    return query;
}



void Search::startSearch()
{
    sal_Int32 i,j;
    // set up ConceptData lists
    // order search terms
    quicksort( 0, free2_ - 1);

    // remove duplicates
    for (i = 0; i < free2_ - 1; i = j)
    {
        for (j = i + 1; j < free2_; j++)
        {
            if( conceptData_[i]->crqEquals( conceptData_[j].get() ) )
                conceptData_[j] = 0;
            else
                i = j;
        }
    }

    // create lists
    for( i = 0; i < free2_ - 1; i = j )
    {
        for (j = i + 1; j < free2_; j++ )
        {
            if( conceptData_[j].is() )
            {
                if( conceptData_[i]->cEquals( conceptData_[j].get() ) )
                {
                    conceptData_[i]->addLast( conceptData_[j].get() );
                    conceptData_[j] = 0;
                }
                else
                    i = j;
            }
        }
    }

    // densify
    for( i = 0; i < free2_ - 1; i++)
    {
        if( ! conceptData_[i].is() )
        {
            for( j = i + 1; j < free2_; j++)
            {
                if (conceptData_[j].is() )
                {
                    conceptData_[i] = conceptData_[j];
                    conceptData_[j] = 0;
                    break;
                }
            }
        }
    }

    // set up new document generators
    nextDocGenHeap_.reset();
    for( i = 0; i < free2_ && conceptData_[i].is(); i++)
    {
        NextDocGenerator* gen = new NextDocGenerator( conceptData_[i].get(),env_ );
        try
        {
            sal_Int32 doc;
            gen->first();
            if( ( doc = gen->getDocument() ) != NonnegativeIntegerGenerator::END )
            {
                /* !!! ??? is concept length used any more in any way
                   conceptData_[i].
                   setConceptLength(_env.
                   getConceptLength(conceptData_[i].getConcept()));
                */
                nextDocGenHeap_.addGenerator( gen );
            }
        }
        catch( ... )
        {
        }
    }

    nextDocGenHeap_.start();
    env_->reset();
    env_->resetContextSearch();
    searchDocument();
}




void Search::addTerm( sal_Int32 col,sal_Int32 concept,double score )
{
    if( env_->occursInText( concept ) )
    {
        ConceptData* cd = queries_[queries_.size()-1]->makeConceptData( col,concept,score,queries_.size()-1 );
        if( sal_uInt32( free2_ ) == conceptData_.size() )
        {
            conceptData_.push_back( 0 );
//          conceptVisitor_ = &conceptData_[0];
        }
        conceptData_[ free2_++ ] = cd;
    }
}





void Search::searchDocument()
{
    std::vector< RoleFiller* > start( queries_.size() );
    do
    {
        try
        {
             switch( nextDocument( start ) )
            {
                case 0:     // multi group
                    genHeap_.start( start );
                    while( genHeap_.next( start ) )
                        ;
                    break;

                case 1:     // single group
                    while( firstGenerator_.next() )
                        firstGenerator_.generateFillers( start );
                    break;

                case 2:     // reached the end
                    return;
            }
        }
        catch( const excep::XmlSearchException& )
        {
            continue;
        }

        RoleFiller* next;
        for( sal_uInt32 i = 0; i < queries_.size(); ++i )
        {
            if( ( next = start[i] ) != 0 && next != RoleFiller::STOP() )
                next->scoreList( queries_[i],document_ );
            else if( queries_[i]->zoned() )
            {
                RoleFiller* rfs = queries_[i]->getRoleFillers();
                if( rfs && rfs != RoleFiller::STOP() )
                    rfs->scoreList( queries_[i],document_ );
            }
        }
        genHeap_.reset();
    }
    while( nextDocGenHeap_.isNonEmpty() );

    // #i80952
#if 0
    for( sal_uInt32 i = 0; i < start.size(); ++i )
        if( start[i] != RoleFiller::STOP() )
            delete start[i];
#endif
}



sal_Int32 Search::nextDocument( std::vector< RoleFiller* >& start ) throw( xmlsearch::excep::XmlSearchException )
{
    while( nextDocGenHeap_.isNonEmpty() )
    { // still something to do
        sal_uInt32 i;
        for( i = 0; i < queries_.size(); ++i )
            if( queries_[i] )
                queries_[i]->resetForNextDocument();

        // gather all concepts this document has
        // and store associated conceptData
        sal_Int32 index = 0;
        document_ = nextDocGenHeap_.getDocument();
        docConcepts_.clear();
        queryMasks_.clear();
        do
        {
            docConcepts_.push_back( nextDocGenHeap_.getConcept() );
            queryMasks_.push_back( nextDocGenHeap_.getQueryMask() );
            ConceptData *conceptData = ( conceptData_[ index++ ] = nextDocGenHeap_.getTerms() ).get();
            conceptData->runBy( queries_ );
            nextDocGenHeap_.step();
        }
        while( nextDocGenHeap_.atDocument( document_) );

        // if there is no saturation model, some query will always vote YES
        // and so every document will be opened
        // even if this case, however, savings can be achieved by not generating fillers
        // for some queries (not scoring, etc)
        // and, with more care, creation of some GroupGenerators can be avoided
        // saturating queries with lots of good hits will lead to best results

        sal_Int32 voteMask = 0;
        Query* query;
        for( i = 0; i < queries_.size(); ++i )
        {
            query = queries_[i];
            if( query )
            {
                query->saveRoleFillers( 0 );
                if( query->vote() )
                {
                    // normal reset
                    start[i] = query->zoned() ? RoleFiller::STOP() : 0;
                    voteMask |= 1 << i;
                }
                else
                    start[i] = RoleFiller::STOP();  // prohibit setting
            }
        }

        // we may eliminate some ConceptGroupGenerators
        // those which would be used only by Queries which voted NO
        if( voteMask != 0 )
        {       // need to open up document
            ConceptGroupGenerator* gen;
            // !!! don't gather Fillers for disinterested Queries
            if( openDocumentIndex( document_ ) )
            {   // multi group
                // set up all needed generators
                sal_Int32 j = 0;
                while( ( queryMasks_[j] & voteMask ) == 0 )
                    ++j;
                //      assert(j < index);
                sal_Int32 c = docConcepts_[j];
                sal_Int32 group = 0;
                // find first group
                while( /*group < maxConcepts_.size() &&*/
                    c > maxConcepts_[ group ] && ++group < limit_ )
                    ;
                gen = makeGenerator( group );
                gen->addTerms( indexOf(c),conceptData_[j].get() );

                for( ++j; j < index; j++ )
                    if( ( queryMasks_[j] & voteMask ) > 0 )
                    {
                        c = docConcepts_[j];
                        if( c > max_ )
                        {   // need to find another group
                            //      assert(group < _limit);
                            while( /*group < maxConcepts_.size() &&*/
                                c > maxConcepts_[ group ] && ++group < limit_ )
                                ;
                            gen = makeGenerator( group );
                        }
                        gen->addTerms( indexOf(c),conceptData_[j].get() );
                    }
                return 0;
            }
            else
            {           // single group
                for( sal_Int32 j = 0; j < index; j++ )
                    if( queryMasks_[j] & voteMask )
                        firstGenerator_.addTerms( indexOf( docConcepts_[j] ),conceptData_[j].get() );
                return 1;
            }
        }
    }
    return 2;
}




bool Search::openDocumentIndex( sal_Int32 docNo ) throw( excep::XmlSearchException )
{
    data_ = env_->getPositions( dataL_,docNo );
    base_ = env_->getDocumentIndex( docNo );

    startingIndex_ = 0;
    sal_Int32 kk = data_[ base_ ] & 0xFF, k2;

    switch( kk >> 6 )
    {       // get type
        case 0:         // single group, no extents
            k2 = data_[base_ + 1];
            firstGenerator_.init( dataL_,data_,base_ += 2,k2 );
            // decode concept table
            nConcepts_ = firstGenerator_.decodeConcepts( kk & 0x3F,0,concepts_ );
            return false;

        case 2:         // multi group, no extents
        {
            kTable_.clear();
            offsets_.clear();
            maxConcepts_.clear();
            util::ByteArrayDecompressor compr( dataL_,data_,base_ + 1 );
            compr.decode( kk & 0x3F,kTable_ );

            sal_Int32 last = kTable_.back();
            kTable_.pop_back();
            compr.ascDecode( last,offsets_ );
            last = kTable_.back();
            kTable_.pop_back();
            compr.ascDecode( last,maxConcepts_ );

            base_ += 1 + compr.bytesRead();
            limit_ = maxConcepts_.size();
        }
        return true;

        case 1:         // single group, extents
        case 3:         // multi group, extents
            throw excep::XmlSearchException( rtl::OUString::createFromAscii( "extents not yet implemented\n" ) );
    }
    return false;
}





ConceptGroupGenerator* Search::makeGenerator( sal_Int32 group )
    throw( excep::XmlSearchException )
{
    sal_Int32 shift,index;
    if( group > 0 )
    {
        index = base_ + offsets_[ group - 1 ];
        shift = maxConcepts_[ group - 1 ];
    }
    else
    {
        index = base_;
        shift = 0;
    }

    // initialize generator
    ConceptGroupGenerator* gen =
        new ConceptGroupGenerator( dataL_,data_,index,kTable_[ 1 + 2*group ] );
    // decode concept table
    nConcepts_ = gen->decodeConcepts( kTable_[2*group],shift,concepts_ );

    if( group < limit_ )
        max_ = concepts_[ nConcepts_ ] = maxConcepts_[ group ];
    else
        max_ = concepts_[ nConcepts_ - 1 ];

    genHeap_.addGenerator( gen );
    startingIndex_ = 0;     // in _concepts; lower search index
    return gen;
}



sal_Int32 Search::indexOf(sal_Int32 concept) throw( excep::XmlSearchException )
{
    sal_Int32 i = startingIndex_,j = nConcepts_,k;
    while( i <= j )
        if( concepts_[ k = (i + j)/2 ] < concept )
            i = k + 1;
        else if( concept < concepts_[k] )
            j = k - 1;
        else
        {
            startingIndex_ = k + 1;
            return k;
        }
    throw excep::XmlSearchException( rtl::OUString::createFromAscii( "indexOf not found" ) );
}




sal_Int32 Search::partition( sal_Int32 p,sal_Int32 r )
{
    rtl::Reference< ConceptData > x = conceptData_[ ((p + r) >> 1) & 0x7FFFFFFF ];
    sal_Int32 i = p - 1, j = r + 1;
    while( true )
    {
        while( x->compareWith( conceptData_[--j].get() ) )
            ;
        while( conceptData_[++i]->compareWith( x.get() ) )
            ;
        if( i < j )
        {
            rtl::Reference< ConceptData > t = conceptData_[i];
            conceptData_[i] = conceptData_[j];
            conceptData_[j] = t;
        }
        else
            return j;
    }
}



void Search::quicksort( sal_Int32 p,sal_Int32 r )
{
    while (p < r)
    {
        sal_Int32 q = partition( p,r );
        quicksort(p, q);
        p = q + 1;
    }
}
