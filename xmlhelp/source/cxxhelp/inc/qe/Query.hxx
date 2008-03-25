/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Query.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 15:17:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _XMLSEARCH_QE_QUERY_HXX_
#define _XMLSEARCH_QE_QUERY_HXX_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include  <rtl/ustring.hxx>
#endif
#ifndef _XMLSEARCH_QE_XMLINDEX_HXX_
#include <qe/XmlIndex.hxx>
#endif


namespace xmlsearch {

    namespace qe {

        class QueryResults;
        class Search;
        class ConceptData;
        class RoleFiller;
        class ContextTables;


        class QueryHit
        {
        public:

            QueryHit( sal_Int32 nColumns,double penalty,sal_Int32 doc,sal_Int32 begin,sal_Int32 end )
                : doc_( doc ),
                  begin_( begin ),
                  end_( end ),
                    matchesL_( 2*nColumns ),
                    matches_( new sal_Int32[ 2*nColumns ] ),
                  penalty_( penalty )
            {
                rtl_zeroMemory( matches_,sizeof( sal_Int32 ) * matchesL_ );
            }

            ~QueryHit() { delete[] matches_; }

            sal_Int32 getDocument() const { return doc_; }

            sal_Int32* getMatches( sal_Int32& matchesL );

            sal_Int32 countOfMatches() const { return matchesL_; }

            sal_Int32 getBegin() const { return begin_; }

            sal_Int32 getEnd() const { return end_; }

            double getPenalty() const { return penalty_; }

            bool betterThan( const QueryHit* o )
            {
                if( penalty_ != o->penalty_ )
                    return penalty_ < o->penalty_;
                else if( begin_ != o->begin_ )
                    return begin_ < o->begin_;
                else if( end_ != o->end_ )
                    return end_ < o->end_;
                else
                    return false;
            }

            bool worseThan( const QueryHit* o )
            {
                if( penalty_ != o->penalty_ )
                    return penalty_ > o->penalty_;
                else if( begin_ != o->begin_ )
                    return begin_ > o->begin_;
                else if( end_ != o->end_ )
                    return end_ > o->end_;
                else
                    return false;
            }

            bool worseThan( double penalty,sal_Int32 begin,sal_Int32 end )
            {
                if( penalty_ != penalty )
                    return penalty_ > penalty;
                else if( begin_ != begin )
                    return begin_ > begin;
                else if( end_ != end )
                    return end_ > end;
                else
                    return false;
            }

            bool compareTo( const QueryHit* o ) const
            {
                if( penalty_ != o->penalty_ )
                    return penalty_ < o->penalty_;
                else if( begin_ != o->begin_ )
                    return begin_ < o->begin_;
                else if( end_ != o->end_ )
                    return end_ < o->end_;
                else
                    return false;
            }


        private:

            sal_Int32    doc_,begin_,end_;

              sal_Int32    matchesL_;
              sal_Int32    *matches_;    // ...concept, word number, ...

            double penalty_;

        }; // end class QueryHit



        class QueryHitData
        {
        public:

            QueryHitData( double penalty,const rtl::OUString& document,sal_Int32 termsL, rtl::OUString* terms )
                : penalty_( penalty ),
                  document_( document ),
                  termsL_( termsL ),
                  terms_( terms )      { }

            ~QueryHitData() { delete[] terms_; }

            rtl::OUString getDocument() const { return document_; }

            double getPenalty() const { return penalty_; }


        private:

            double        penalty_;

            const rtl::OUString document_;

            sal_Int32      termsL_;
            rtl::OUString* terms_;

        };  // end class QueryHitData



        class HitStore
        {
        public:

            HitStore( double, sal_Int32,sal_Int32 );

            ~HitStore();

            QueryHit* firstBestQueryHit();

            QueryHit* nextBestQueryHit();

            bool goodEnough( double penalty,sal_Int32 begin,sal_Int32 end );

            QueryHit* createQueryHit( double penalty,sal_Int32 doc,sal_Int32 begin,sal_Int32 end );

            double getCurrentStandard() { return standard_; }


        private:

            const sal_Int32 limit_,nColumns_;

            sal_Int32 index_,free_;

            double standard_;

            std::vector< QueryHit* >  heap_;

            void heapify( sal_Int32 );

            void quicksort( sal_Int32 p,sal_Int32 r );

            sal_Int32 partition( sal_Int32 p,sal_Int32 r );
        };



        class Query
        {
        public:

            Query( XmlIndex* env,
                   sal_Int32 nColumns,
                   sal_Int32 nHits,
                   sal_Int32 missingPenaltiesL,
                   double*   missingPenalties );

            virtual ~Query();

            void missingTerms( sal_Int32 nMissingTerms );

            virtual void addControlConceptData( Search*,sal_Int32 ) { }

            virtual bool zoned() const { return false; }

            virtual ConceptData* makeConceptData( sal_Int32 col,sal_Int32 concept,double penalty,sal_Int32 queryNo );

            void getHits( std::vector< QueryHitData* >& data,sal_Int32 nHits );

            double lookupPenalty( sal_Int32 pattern ) { return penalties_[ pattern ]; }

            RoleFiller* getRoleFillers() { return roleFillerList_; }

            void resetForNextDocument();

            void saveRoleFillers( RoleFiller* roleFillerList ) { roleFillerList_ = roleFillerList; }

            bool vote();

            sal_Int32 getNColumns() { return nColumns_; }

            QueryHit* maybeCreateQueryHit( double penalty,
                                           sal_Int32 doc, sal_Int32 begin, sal_Int32 end, sal_Int32 parentContext);

            void setIgnoredElements( const sal_Int32 ignoredElementsL,const rtl::OUString* ignoredElements );

            double getOutOufOrderPenalty() { return 0.25; }

            double getGapPenalty() { return 0.005; }

            void updateEstimate( sal_Int32 role,double penalty );


        protected:

            XmlIndex*         env_;
            ContextTables*    ctx_;
            HitStore          store_;

            sal_Int32 nHitsRequested_,nColumns_;
            double    currentStandard_;

            sal_Int32  missingPenaltyL_,upperboundTemplateL_,penaltiesL_;
            double     *missingPenalty_,*upperboundTemplate_,*penalties_;

            sal_Int32 ignoredElementsL_;
            bool      *ignoredElements_;


        private:

            bool                vote_;
            double              missingTermsPenalty_;

            //  for use with Start/Stop

            RoleFiller  *roleFillerList_;

            void makePenaltiesTable();

            double computePenalty( sal_Int32 );

        };



        class PrefixTranslator
        {
        public:

            static PrefixTranslator* makePrefixTranslator( const rtl::OUString*,sal_Int32 )
            {
                return 0;
            }
        };




        class QueryResults;



        class QueryHitIterator
        {
        public:

            QueryHitIterator( const QueryResults* result );

            ~QueryHitIterator();

            bool next();

            QueryHitData* getHit( const PrefixTranslator* ) const;

            double getPenalty();


        private:

            bool           accessible_;
            sal_Int32      index_;
            const QueryResults*  result_;
        };


    }

}


#endif
