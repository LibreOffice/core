/*************************************************************************
 *
 *  $RCSfile: Query.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 18:07:34 $
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
#ifndef _XMLSEARCH_QE_QUERY_HXX_
#include <qe/Query.hxx>
#endif
#ifndef _XMLSEARCH_QE_XMLINDEX_HXX_
#include <qe/XmlIndex.hxx>
#endif
#ifndef _XMLSEARCH_QE_CONCEPTDATA_HXX_
#include <qe/ConceptData.hxx>
#endif
#ifndef _XMLSEARCH_QE_QUERYPROCESSOR_HXX_
#include <qe/QueryProcessor.hxx>
#endif
#ifndef _XMLSEARCH_QE_CONTEXTTABLES_HXX_
#include <qe/ContextTables.hxx>
#endif


using namespace xmlsearch::qe;


sal_Int32* QueryHit::getMatches( sal_Int32& matchesL )
{
    matchesL = matchesL_;
    return matches_;
}


/******************************************************************************/
/*                                                                            */
/*                     HitStore                                               */
/*                                                                            */
/******************************************************************************/


HitStore::HitStore( double initialStandard,sal_Int32 limit,sal_Int32 nColumns )
    : standard_( initialStandard ),
      limit_( limit ),
      heap_( limit ),
      nColumns_( nColumns ),
      free_( 0 ),
      index_( 0 )
{
    for( sal_uInt32 i = 0; i < heap_.size(); ++i )
        heap_[i] = 0;
}



HitStore::~HitStore()
{
    for( sal_uInt32 i = 0; i < heap_.size(); ++i )
        delete heap_[i];
}



bool HitStore::goodEnough( double penalty, sal_Int32 begin, sal_Int32 end )
{
    return free_ == limit_ ? heap_[0]->worseThan( penalty,begin,end ) : true;
}


QueryHit* HitStore::createQueryHit( double penalty,sal_Int32 doc,sal_Int32 begin,sal_Int32 end )
{
    QueryHit* hit = new QueryHit( nColumns_,penalty,doc,begin,end );
    if( free_ == limit_ )
    { // goodEnough'ness checked already
        delete heap_[0];
        heap_[0] = hit;
        heapify( 0 );
        standard_ = heap_[0]->getPenalty();
    }
    else if( free_ < limit_ )
    {
        heap_[ free_++ ] = hit;
        if( free_ == limit_ )
        { // we have the needed number
            for( sal_Int32 i = free_/2; i >= 0; --i ) // build heap
                heapify( i );
            standard_ = heap_[0]->getPenalty();
        }
    }
    return hit;
}


struct CompareQueryHit
{
    bool operator()( const QueryHit* l,const QueryHit* r )
    {
        return l->compareTo( r );
    }
};


#include <stl/algorithm>


QueryHit* HitStore::firstBestQueryHit()
{
    if( free_ > 0)
    {
        CompareQueryHit bla;
        heap_.resize( free_ );
          std::stable_sort( heap_.begin(),heap_.end(),bla );
        index_ = 0;
        return nextBestQueryHit();
    }
    else
        return 0;
}


QueryHit* HitStore::nextBestQueryHit()
{
    return index_ < free_ ? heap_[ index_++ ] : 0;
}


void HitStore::heapify( sal_Int32 i )
{
    for( sal_Int32 r,l,worst; ; )
    {
        r = (i + 1) << 1; l = r - 1;
        worst = l < free_ && heap_[i]->betterThan( heap_[l] ) ? l : i;
        if( r < free_ && heap_[ worst ]->betterThan( heap_[r] ) )
            worst = r;
        if (worst != i)
        {
            QueryHit* temp = heap_[ worst ];
            heap_[ worst ] = heap_[ i ];
            heap_[i] = temp;
            i = worst;      // continue
        }
        else
            break;
    }
}


//  sal_Int32 HitStore::partition( sal_Int32 p,sal_Int32 r )
//  {
//      QueryHit* x = heap_[ ((p + r) >> 1) & 0x7FFFFFFF ];
//      sal_Int32 i = p - 1, j = r + 1;
//      while( true )
//      {
//          while( x->compareTo( heap_[--j] ) )
//              ;
//          while( heap_[++i]->compareTo( x ) )
//              ;
//          if( i < j )
//          {
//              QueryHit* t = heap_[i];
//              heap_[i] = heap_[j];
//              heap_[j] = t;
//          }
//          else
//              return j;
//      }
//  }


//  void HitStore::quicksort( sal_Int32 p,sal_Int32 r )
//  {
//      while( p < r )
//      {
//          sal_Int32 q = partition( p,r );
//          quicksort(p, q);
//          p = q + 1;
//      }
//  }



/******************************************************************************/
/*                                                                            */
/*                                 Query                                      */
/*                                                                            */
/******************************************************************************/


#define MissingTermPenalty 10.0


Query::Query( XmlIndex* env,
              sal_Int32 nColumns,
              sal_Int32 nHits,
              sal_Int32 missingPenaltiesL,
              double* missingPenalties )
    : env_( env ),
      ctx_( env ? env->getContextInfo() : 0 ),
      nColumns_( nColumns ),
      nHitsRequested_( nHits ),
      missingPenaltyL_( nColumns ),
      missingPenalty_( new double[ nColumns ] ),
      upperboundTemplateL_( nColumns ),
      upperboundTemplate_( new double[ nColumns ] ),
      penaltiesL_( missingPenaltiesL ),
      penalties_( missingPenalties ),
      currentStandard_( nColumns * MissingTermPenalty - 0.0001 ),
      missingTermsPenalty_( 0.0 ),
      store_( nColumns * MissingTermPenalty - 0.0001,nHits,nColumns ),
      ignoredElementsL_( 0 ),
      ignoredElements_( 0 )
{
    // for the EmptyQuery case (awaits arch improvement pass)

    if( missingPenalties )
        for( sal_Int32 i = 0;i < nColumns_; ++i )
            missingPenalty_[i] = missingPenalties[i];
    else
        for( sal_Int32 i = 0;i < nColumns_; ++i )
            missingPenalty_[i] = MissingTermPenalty;

    makePenaltiesTable();
    //  _roleFillerList = RoleFiller.STOP;
}


Query::~Query()
{
    delete[] missingPenalty_;
    delete[] upperboundTemplate_;
    delete[] penalties_;
    delete[] ignoredElements_;
}


void Query::setIgnoredElements( const sal_Int32 ignoredElementsL,const rtl::OUString* ignoredElements )
{
    if( ctx_ )
        ignoredElements_ = ctx_->getIgnoredElementsSet( ignoredElementsL_,
                                                        ignoredElementsL,ignoredElements );

    if( ! ctx_ )
    {
        ignoredElementsL_ = 0;
        ignoredElements_   = 0;
    }
}



void Query::missingTerms( sal_Int32 nMissingTerms )
{
    missingTermsPenalty_ = MissingTermPenalty * nMissingTerms;
}



ConceptData* Query::makeConceptData( sal_Int32 col,sal_Int32 concept,double penalty,sal_Int32 queryNo )
{
    return new ConceptData( concept,col,penalty,queryNo,nColumns_,env_->getContextInfo() );;
}


void Query::getHits( std::vector< QueryHitData* >& data,sal_Int32 n )
{
    if( n <= 0 )
        return;

    QueryHit* qh = store_.firstBestQueryHit();

    while( qh )
    {
        data.push_back( env_->hitToData( qh ) );
        qh = data.size() < sal_uInt32( n ) ? store_.nextBestQueryHit() : 0;
    }
}


QueryHit* Query::maybeCreateQueryHit( double penalty,
                                      sal_Int32 doc, sal_Int32 begin, sal_Int32 end, sal_Int32 parentContext )
{
    // hits are located using only terms actually present in text
    // if B is not present, the query A B C reduces to A C and penalties
    // are computed as if B did not occur in query
    // to meaningfully merge results from different servers, some of which
    // may have B, penalty has to be normalized to the common computing scheme

    QueryHit* res =
        ( store_.goodEnough( penalty += missingTermsPenalty_,begin,end )
          && ( ! ignoredElements_ || ctx_->notIgnored( parentContext,ignoredElementsL_,ignoredElements_ ) ) )
        ?
        store_.createQueryHit( penalty,doc,begin,end )
        :
        0;
    return res;
}


void Query::makePenaltiesTable()
{
    sal_Int32 nPatterns = 1 << nColumns_;
    delete[] penalties_;
    penalties_ = new double[ penaltiesL_ = nPatterns ];
    for (sal_Int32 i = 0; i < nPatterns; ++i )
        penalties_[i] = computePenalty(i);
}


double Query::computePenalty( sal_Int32 n )
{
    double penalty = 0.0;
    for( sal_Int32 i = 0; i < nColumns_; ++i )
        if( ( n & 1 << i ) == 0 )
            penalty += missingPenalty_[i];
    return penalty;
}


void Query::resetForNextDocument()
{
    currentStandard_ = store_.getCurrentStandard();
    // "everything's missing"
    for( sal_Int32 i = 0; i < nColumns_; i++ )
        upperboundTemplate_[i] = missingPenalty_[i];
    vote_ = false;
}


bool Query::vote()
{
    double sum = 0.0;
    for( sal_Int32 i = 0; i < nColumns_; i++ )
        sum += upperboundTemplate_[i];
    return vote_ = (sum <= currentStandard_ );
}


void Query::updateEstimate( sal_Int32 role,double penalty )
{
    if( penalty < upperboundTemplate_[ role ] )
        upperboundTemplate_[ role ] = penalty;
}


/******************************************************************************/
/*                                                                            */
/*                         QueryHitIterator                                   */
/*                                                                            */
/******************************************************************************/



QueryHitIterator::QueryHitIterator( const QueryResults* result )
    : result_( result ),
      index_( -1 )
{
}


QueryHitIterator::~QueryHitIterator()
{
    delete result_;
}


bool QueryHitIterator::next()
{
    return accessible_ = ( ++index_ < sal_Int32( result_->queryHits_.size() ) );
}


QueryHitData* QueryHitIterator::getHit( const PrefixTranslator* ) const
{
    if( accessible_ )
        return result_->queryHits_[index_];
    else
        return 0;
}


double QueryHitIterator::getPenalty()
{
    if( accessible_ )
        return result_->queryHits_[index_]->getPenalty();
    else
        return 1.0E30;
}

