/*************************************************************************
 *
 *  $RCSfile: Search.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: abi $ $Date: 2001-05-08 12:02:45 $
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
#ifndef _XMLSEARCH_QE_SEARCH_HXX_
#include <qe/Search.hxx>
#endif


using namespace std;
using namespace xmlsearch;
using namespace xmlsearch::qe;


class QueryFactoryImpl
{
public:

    Query* makeQuery( XmlIndex* env,const rtl::OUString& context,sal_Int32 nColumns,sal_Int32 nHits);

};


Query* QueryFactoryImpl::makeQuery( XmlIndex* env,
                                    const rtl::OUString& context,
                                    sal_Int32 nColumns,
                                    sal_Int32 nHits )
{
    // ContextTables* contextTables = env->getContextInfo();

    if( ! context.getLength() )
    {
        // cout << "contextlength zero" << endl;
        return new Query( env,nColumns,nHits,0,0 );
    }
    else if( context.indexOf( sal_Unicode( '|' ) ) != -1 )
    {
        return 0;      // needs to be modified
    }
    else if( context.indexOf( rtl::OUString::createFromAscii( "//" ) ) != -1 )
    {
        return 0;      // needs to be modified
    }
    else if( context.indexOf( sal_Unicode( '/' ) ) != -1 )
    {
        return 0;      // needs to be modified
    }
    else if( context.indexOf( sal_Unicode( '@' ) ) != -1 )
    {
        return 0;     // needs to be modified
    }
    else if( context.indexOf( sal_Unicode( '[' ) ) != -1 )
    {
        return 0;     // needs to be modified
    }
    else
    {
        return 0;     // needs to be modified
    }
}



Search::Search( XmlIndex* env )
    : env_( env ),
      queryFactory_( 0 ),
      nextDocGenHeap_(),
      limit_( 0 ),
      firstGenerator_(),
      dataL_( 0 ),
      data_( 0 ),
      base_( 0 ),
      concepts_( new sal_Int32[ ConceptGroupGenerator::NConceptsInGroup ] )
{
}



Search::~Search()
{
    delete queryFactory_;

    sal_uInt32 i;

    for( i = 0; i < queries_.size(); ++i )
        delete queries_[i];

    for( i = 0; i < conceptData_.size(); ++i )
        delete conceptData_[i];

    delete[] concepts_;
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


#ifdef ABIDEBUG
extern ostream& operator<<( ostream& out,const rtl::OUString& bla );
#endif


void Search::startSearch()
{
#ifdef ABIDEBUG
//    for( int k = 0; k < conceptData_.size(); ++k )
//      {
//        if( ! conceptData_[k] )
//      continue;

//        cout << conceptData_[k]->getConcept() << endl;
//        cout << conceptData_[k]->getPenalty() << endl;
//        cout << sal_Int32( conceptData_[k]->getRole() ) << endl;
//        cout << sal_Int32( conceptData_[k]->getQuery() ) << endl;
//        cout << conceptData_[k]->getScore() << endl;
//      }
#endif

  sal_Int32 i,j;
  // set up ConceptData lists
  // order search terms
  sal_Int32 _free2 = conceptData_.size();
  quicksort( 0, _free2 - 1);

  // remove duplicates
  for (i = 0; i < _free2 - 1; i = j)
    for (j = i + 1; j < _free2; j++)
      if( conceptData_[i]->crqEquals( conceptData_[j] ) )
    conceptData_[j] = 0;
      else
    i = j;

  // create lists
  for( i = 0; i < _free2 - 1; i = j )
    for (j = i + 1; j < _free2; j++ )
      if( conceptData_[j] )
    if( conceptData_[i]->cEquals( conceptData_[j] ) )
      {
        conceptData_[i]->addLast( conceptData_[j] );
        conceptData_[j] = 0;
      }
    else
      i = j;

  // densify
  for (i = 0; i < _free2 - 1; i++)
    if( ! conceptData_[i] )
      for( j = i + 1; j < _free2; j++)
    if (conceptData_[j] )
      {
          conceptData_[i] = conceptData_[j];
          conceptData_[j] = 0;
        break;
      }

#ifdef ABIDEBUG
  for( i = 0; i < conceptData_.size(); ++i )
    {
      if( ! conceptData_[i] )
    continue;

//        cout << conceptData_[i]->getConcept() << endl;
//        cout << conceptData_[i]->getPenalty() << endl;
//        cout << sal_Int32( conceptData_[i]->getRole() ) << endl;
//        cout << sal_Int32( conceptData_[i]->getQuery() ) << endl;
//        cout << conceptData_[i]->getScore() << endl;
    }
#endif

  // set up new document generators
  nextDocGenHeap_.reset();
  for( i = 0; i < _free2 && conceptData_[i]; i++)
    {
#ifdef ABIDEBUG
//        if( conceptData_[i] )
//      cout << rtl::OUString::createFromAscii( "conceptData not zero" ) << endl;
#endif

      NextDocGenerator* gen = new NextDocGenerator( conceptData_[i],env_ );
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
//            cout << "doc = " << doc << endl;
          nextDocGenHeap_.addGenerator( gen );
        }
    }
      catch( ... )
    {
#ifdef ABIDEBUG
        // cout << "Search::startSearch -> catched exception" << endl;
#endif
    }
    }

  nextDocGenHeap_.start();
  env_->reset();
  env_->resetContextSearch();
  searchDocument();
}




void Search::addTerm( sal_Int32 col,sal_Int32 concept,double score )
{
#ifdef ABIDEBUG
//    cout << concept << endl;
#endif
    if( env_->occursInText( concept ) )
    {
#ifdef ABIDEBUG
//        cout << "occurs" << endl;
#endif
        conceptData_.push_back( queries_[queries_.size()-1]->makeConceptData( col,concept,score,queries_.size()-1 ) );
    }
#ifdef ABIDEBUG
//    else
//      cout << "does not occur" << endl;
#endif
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
            ( conceptData_[ index++ ] = nextDocGenHeap_.getTerms() )->runBy( queries_ );
            nextDocGenHeap_.step();
        }
        while( nextDocGenHeap_.atDocument( document_) );

        // if there is no saturation model, some query will always vote YES
        // and so every document will be opened
        // even if this case, however, savings can be achieved by not generating fillers
        // for some queries (not scoring, etc)
        // and, with more care, creation of some GroupGenerators can be avoided
        // saturating queries with lots of good hits will lead to best results

        rtl::OUString docName = env_->documentName( document_);


#ifdef ABIDEBUG
//          if( document_ == 148 )
//          {
//              cout << "_document = " << document_ << endl;
//              cout << "docName = " <<  env_->documentName( document_) << endl;
//          }
//          else
//          {
//
//              cout << "_document = " << document_ << endl;
//              cout << "docName = " << docName << endl;
//          }
#endif

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
            {// multi group
                // set up all needed generators
                sal_Int32 i = 0;
                while( ( queryMasks_[i] & voteMask ) == 0 )
                    ++i;
                //      assert(i < index);
                sal_Int32 c = docConcepts_[i];
                sal_Int32 group = 0;
                // find first group
                while( c > maxConcepts_[ group ] && ++group < limit_ )
                    ;
                gen = makeGenerator( group );
                gen->addTerms( indexOf(c), conceptData_[i] );

                for( ++i; i < index; i++ )
                    if( ( queryMasks_[i] & voteMask ) > 0 )
                    {
                        c = docConcepts_[i];
                        if( c > max_ )
                        {   // need to find another group
                            //      assert(group < _limit);
                            while( c > maxConcepts_[ group ] && ++group < limit_ ) ;
                            gen = makeGenerator( group );
                        }
                        gen->addTerms( indexOf(c), conceptData_[i] );
                    }
                return 0;
            }
            else
            {           // single group
                for( sal_Int32 i = 0; i < index; i++ )
                    if( queryMasks_[i] & voteMask )
//  #ifdef ABIDEBUG
//                      {
//                          sal_Int32 bla = indexOf( docConcepts_[i] );
//              cout << "indexOf = " << bla << endl;
//                          firstGenerator_.addTerms( bla,conceptData_[i] );
//                      }
//  #elif
                firstGenerator_.addTerms( indexOf( docConcepts_[i] ),conceptData_[i] );
//  #endif
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
//        cout << "false" << endl;
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
//        cout << "true" << endl;
      return true;

    case 1:         // single group, extents
    case 3:         // multi group, extents
      throw excep::XmlSearchException( rtl::OUString::createFromAscii( "extents not yet implemented\n" ) );
    }

//    cout << "false1" << endl;
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
    new ConceptGroupGenerator( dataL_,data_,index,kTable_[ 2*group + 1 ] );
  // decode concept table
  nConcepts_ = gen->decodeConcepts( kTable_[2*group],shift,concepts_ );

  if( group < limit_ )
    max_ = concepts_[ nConcepts_ ] = maxConcepts_[ group ];
  else
    max_ = concepts_[ nConcepts_ - 1 ];

  genHeap_.addGenerator( gen );
  startingIndex_ = 0;       // in _concepts; lower search index
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
  ConceptData* x = conceptData_[ ((p + r) >> 1) & 0x7FFFFFFF ];
  sal_Int32 i = p - 1, j = r + 1;
  while( true )
    {
      while( x->compareWith( conceptData_[--j] ) )
    ;
      while( conceptData_[++i]->compareWith( x ) )
    ;
      if( i < j )
    {
      ConceptData* t = conceptData_[i];
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
