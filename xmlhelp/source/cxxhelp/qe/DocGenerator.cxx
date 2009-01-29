/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DocGenerator.cxx,v $
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
#include <qe/DocGenerator.hxx>
#include <qe/Query.hxx>


using namespace xmlsearch;
using namespace xmlsearch::qe;


const sal_Int32 NonnegativeIntegerGenerator::END = -1;
const sal_Int32 ConceptGroupGenerator::NConceptsInGroup = 16;
const sal_Int32 ConceptGroupGenerator::BitsInLabel = 4;


RoleFiller RoleFiller::roleFiller_;


RoleFiller::RoleFiller()
    : m_nRefcount( 0 ),
      fixedRole_( 0 ),
      filled_( 0 ),
      begin_( 0 ),
      end_( 0 ),
      parentContext_( 0 ),
      limit_( 0 ),
      next_( 0 ),
      fillers_( 0 ),
      conceptData_( 0 )
{
}


RoleFiller::RoleFiller( sal_Int32 nColumns,
                        ConceptData* first,
                        sal_Int32 role,
                        sal_Int32 pos,
                        sal_Int32 parentContext,
                        sal_Int32 limit )
    : m_nRefcount( 0 ),
      fixedRole_( sal_uInt8( role & 0xF ) ),                    // primary/constitutive concept/role
      next_( 0 ),
      fillers_( nColumns ),
      conceptData_( first )
{
    filled_ = sal_Int16( 1 << fixedRole_ );
    begin_ = pos;       // offset in file
    //    _end = _begin + first.getConceptLength();
    end_ = begin_ + 1;
    limit_ = limit;
    parentContext_ = parentContext;
    next_ = 0;
    for( sal_uInt32 i = 0; i < fillers_.size(); ++i )
        fillers_[i] = 0;
    fillers_[ role ] = this;
}


RoleFiller::~RoleFiller()
{

}


void RoleFiller::scoreList( Query* query,sal_Int32 document )
{
    sal_Int32 nColumns = query->getNColumns();
    RoleFiller* candidateHit = this;         // function called for the head of list
    RoleFiller* next;                        // lookahead: if overlap, if so, is it better

    // 'candidateHit' always points at the current candidate to be converted to a QueryHit
    // 'penalty' is its penalty
    // 'next' is used to explore earlier overlapping fillers
    // the decision to emit a QueryHit is made when either there's no next
    // or next doesn't overlap the current candidate
    // the loop's logic makes sure that at emit time there's no better/earlier filler
    // to overlap with the candidate

    double penalty_ = candidateHit->penalty( query,nColumns );

    for( next = candidateHit->next_; next; next = next->next_ )
        if( next->end_ < candidateHit->begin_ )
        { // no overlap
            candidateHit->makeQueryHit( query,document,penalty_ );
            candidateHit = next;
            penalty_ = candidateHit->penalty( query,nColumns );
        }
        else
        { // !!! can be computed in two steps
            double penalty2 = next->penalty( query,nColumns );
            if( penalty2 <= penalty_ )
            { // prefer next, disregard candidateHit
                penalty_ = penalty2;
                candidateHit = next;
            }
        }
    candidateHit->makeQueryHit(query,document,penalty_);
}




void RoleFiller::makeQueryHit( Query* query,sal_Int32 doc,double penalty_ )
{
    QueryHit* hit = query->maybeCreateQueryHit( penalty_,doc,
                                                begin_,end_,parentContext_ );
    if( hit )
    {
        sal_Int32 N;
        sal_Int32* matches = hit->getMatches( N );
        N /= 2;

        for( sal_Int32 i = 0,j = 0; i < N; ++i )
            if( filled_ & 1 << i )
            {
                matches[ j++ ] = fillers_[ i ]->getConcept();
                matches[ j++ ] = fillers_[ i ]->begin_;
            }
            else
                j += 2;
    }
}



sal_Int32 RoleFiller::getConcept()
{
    return conceptData_->getConcept();
}



void RoleFiller::use( std::vector< RoleFiller*>& place,sal_Int32 query )
{
    RoleFiller* rf = place[ query ];
    if( rf )
    {
        place[ query ] = this;  // put at the head of list
        next_ = rf;
        while( rf->limit_ >= begin_ )
        {
            // check if we can grow/improve a hit
            // we don't ever replace filler's fixed role
            if( fixedRole_ != rf->fixedRole_ &&
                // in same parent context eg. PARA
                rf->parentContext_ == parentContext_ )
            {
                if( ( rf->filled_ & ( 1 << fixedRole_ ) ) == 0 )
                {
                    // not filled yet
                    rf->filled_ |= 1 << fixedRole_;
                    rf->fillers_[ fixedRole_ ] = this;
                    rf->end_ = end_;
                }
                else
                    rf->considerReplacementWith( this );
            }

            if( rf->next_ )
                rf = rf->next_;
            else
                return;
        }
    }
    else
        place[query] = this;
}


void RoleFiller::considerReplacementWith( RoleFiller* replacement )
{
    // !!! simplistic for now
    // needs gap and out of order
    sal_Int32 role = replacement->fixedRole_;
    if( replacement->getScore() > fillers_[role]->getScore() )
        fillers_[ role ] = replacement;
}



double RoleFiller::penalty( Query* query,sal_Int32 nColumns )
{
    sal_Int32 length = end_ - begin_ + 1;
    double penalty_ = query->lookupPenalty( filled_ );
    // !!! here is a chance to check against query
    // if hit worth scoring further
    // might not be if query already has lots of good hits
    for( sal_Int32 i = 0; i < nColumns; ++i )
        if( filled_ & ( 1 << i ) )
        {
            penalty_ += fillers_[i]->conceptData_->getPenalty();
            //length -= _fillers[i]._conceptData.getConceptLength() + 1;
            length -= 2;        // !!! ??? c.length is not used ?
            if( filled_ >> (i + 1) )
                for( sal_Int32 j = i + 1; j < nColumns; ++j )
                    if( ( filled_ & 1 << j ) && fillers_[j]->begin_ < begin_ )
                        penalty_ += query->getOutOufOrderPenalty();
        }
    double result = penalty_ + length * query->getGapPenalty();
    return result < 0.0 ? 0.0 : result; // !!! quick fix
}



NextDocGenerator::NextDocGenerator( ConceptData* cd,XmlIndex* env )
    : document_( 0 ),
      concept_( cd ? cd->getConcept() : -1 ),
      queryMask_( cd ? cd->getQueryMask() : -1 ),
      terms_( cd ),
      iterator_( env->getDocumentIterator( concept_ ) )
{
}



void NextDocGeneratorHeap::reset()
{
    for( sal_Int32 i = 0; i < heapSize_; ++i )
    {
        delete heap_[i]; heap_[i] = 0;
    }
    free_ = 0;
    nonEmpty_ = false;
}



void NextDocGeneratorHeap::addGenerator( NextDocGenerator* gen )
{
    if( sal_uInt32( free_ ) == heap_.size() )
    {
        heap_.push_back( 0 );
    }

    heap_[free_++] = gen;
}



void NextDocGeneratorHeap::start()
{
    if( ( heapSize_ = free_ ) > 0 )
    {
        for( sal_Int32 i = heapSize_ / 2; i >= 0; --i )
            heapify(i);
        nonEmpty_ = true;
    }
    else
        nonEmpty_ = false;
}


void NextDocGeneratorHeap::step() throw( excep::XmlSearchException )
{
    if( heap_[0]->next() != NonnegativeIntegerGenerator::END )
        heapify(0);
    else if ( heapSize_ > 1 )
    {
        delete heap_[0];
        heap_[0] = heap_[--heapSize_];
        heap_[ heapSize_ ] = 0;
        heapify(0);
    }
    else
        nonEmpty_ = false;
}


void NextDocGeneratorHeap::heapify( sal_Int32 i )
{
    NextDocGenerator* temp;
    for( sal_Int32 r,l,smallest; ; )
    {
        r = ( i + 1 ) << 1;
        l = r - 1;
        smallest = ( l < heapSize_ && heap_[l]->smallerThan( heap_[i] ) ) ? l : i;
        if( r < heapSize_ && heap_[r]->smallerThan( heap_[ smallest ] ) )
            smallest = r;
        if( smallest != i )
        {
            temp = heap_[ smallest ];
            heap_[ smallest ] = heap_[ i ];
            heap_[i] = temp;
            i = smallest;
        }
        else
            break;
    }
}


bool NextDocGeneratorHeap::atDocument( sal_Int32 document )
{
    return nonEmpty_ && heap_[0]->getDocument() == document;
}




ConceptGroupGenerator::ConceptGroupGenerator( sal_Int32 dataL,sal_Int8* data,sal_Int32 index,sal_Int32 k )
    : last_( 0 ),
      k1_( k ),
      k2_( BitsInLabel ),
      table_( NConceptsInGroup ),
      bits_( new util::ByteArrayDecompressor( dataL,data,index ) )
{
}



ConceptGroupGenerator::ConceptGroupGenerator()
    : last_( 0 ),
      k1_( 0 ),
      k2_( BitsInLabel ),
      table_( NConceptsInGroup ),
      bits_( 0 )
{
}


ConceptGroupGenerator::~ConceptGroupGenerator()
{
    delete bits_;
}


void ConceptGroupGenerator::generateFillers( std::vector< RoleFiller* >& array )
{
    cData_->generateFillers( array,last_ );
}


bool ConceptGroupGenerator::next() throw( excep::XmlSearchException )
{
    while( bits_->readNext( k1_,this ) )
    {
        sal_Int32 bla = bits_->read( k2_ );
        if( ( cData_ = table_[ bla ] ).is() )
            return true;
    }
    return false;
}


sal_Int32 ConceptGroupGenerator::decodeConcepts( sal_Int32 k,
                                                 sal_Int32 shift,
                                                 sal_Int32 *concepts )
    throw( excep::XmlSearchException )
{
    return bits_->ascendingDecode( k,shift,concepts );
}



void ConceptGroupGenerator::init( sal_Int32 bytesL,sal_Int8* bytes,sal_Int32 index,sal_Int32 k )
{
    k1_ = k;
    delete bits_;
    bits_ = new util::ByteArrayDecompressor( bytesL,bytes,index );
    last_ = 0;
    for( sal_Int32 i = 0;i < NConceptsInGroup; i++ )
        table_[i] = 0;
}



void ConceptGroupGenerator::addTerms( sal_Int32 index,ConceptData* terms )
{
    table_[ index ] = terms;
}



void GeneratorHeap::reset()
{
    for( sal_Int32 i = 0; i < heapSize_; ++i )
    {
        delete heap_[i];
        heap_[i] = 0;
    }
    free_ = 0;
}


void GeneratorHeap::addGenerator( ConceptGroupGenerator* cd )
{
    if( sal_uInt32( free_ ) == heap_.size() )
    {
        heap_.push_back( 0 );
    }

    heap_[free_++] = cd;
}


void GeneratorHeap::buildHeap()
{
    for( sal_Int32 i = heapSize_/2; i >= 0; i-- )
        heapify(i);
}


void GeneratorHeap::heapify( sal_Int32 root )
{
    for( sal_Int32 smallest = 0; ; )
    {
        const sal_Int32 right = ( root + 1 ) << 1;
        const sal_Int32 left = right - 1;
        smallest = ( left < heapSize_ && heap_[left]->position() < heap_[ root ]->position() ) ? left : root;
        if( right< heapSize_ && heap_[right]->position() < heap_[smallest]->position() )
            smallest = right;
        if( smallest != root )
        {
            ConceptGroupGenerator* temp = heap_[smallest];
            heap_[smallest] = heap_[root];
            heap_[root] = temp;
            root = smallest;
        }
        else
            break;
    }
}


bool GeneratorHeap::start( std::vector< RoleFiller* >& array ) throw( xmlsearch::excep::XmlSearchException )
{
    if( ( heapSize_ = free_ ) > 0 )
    {
        for( sal_Int32 i = 0; i < free_; ++i )
            heap_[i]->next();

        buildHeap();
        heap_[0]->generateFillers( array );
        return true;
    }
    else
        return false;
}


bool GeneratorHeap::next( std::vector< RoleFiller* >& array ) throw( xmlsearch::excep::XmlSearchException )
{
    if( heapSize_ > 0 )
    {
        if( ! heap_[0]->next() ) // no more
        {
            if( heapSize_ > 1)
            {
                delete heap_[0];
                heap_[0] = heap_[--heapSize_];
                heap_[heapSize_] = 0;
            }
            else
            {
                heapSize_ = 0;
                return false;
            }
        }
        heapify(0);
        heap_[0]->generateFillers( array );
        return true;
    }
    else
        return false;
}
