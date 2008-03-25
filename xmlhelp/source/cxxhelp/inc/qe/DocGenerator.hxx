/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DocGenerator.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 15:17:32 $
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
#ifndef _XMLSEARCH_QE_DOCGENERATOR_HXX_
#define _XMLSEARCH_QE_DOCGENERATOR_HXX_

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif
#ifndef _RTL_USTRING_HXX__
#include  <rtl/ustring.hxx>
#endif
#ifndef _XMLSEARCH_EXCEP_XMLSEARCHEXCEPTIONS_HXX_
#include <excep/XmlSearchExceptions.hxx>
#endif
#ifndef _XMLSEARCH_QE_XMLINDEX_HXX_
#include <qe/XmlIndex.hxx>
#endif
#ifndef _XMLSEARCH_QE_CONCEPTDATA_HXX_
#include <qe/ConceptData.hxx>
#endif
#ifndef _XMLSEARCH_UTIL_COMPRESSORITERATOR_HXX_
#include <util/CompressorIterator.hxx>
#endif
#ifndef _XMLSEARCH_UTIL_DECOMPRESSOR_HXX_
#include <util/Decompressor.hxx>
#endif


namespace xmlsearch {

    namespace qe {


        class Query;


        class NonnegativeIntegerGenerator
        {
        public:

            static const sal_Int32 END;

            virtual ~NonnegativeIntegerGenerator() { };
            virtual sal_Int32 first() throw( xmlsearch::excep::XmlSearchException ) = 0;
            virtual sal_Int32 next() throw( xmlsearch::excep::XmlSearchException ) = 0;
        };


        class NextDocGenerator
        {
        public:

            NextDocGenerator( ConceptData* cd,XmlIndex* env );

            ~NextDocGenerator() { delete iterator_; }

            sal_Int32 first() throw( xmlsearch::excep::XmlSearchException )
            {
                return ( document_ = ( iterator_ ? iterator_->first() : NonnegativeIntegerGenerator::END ) );
            }

            sal_Int32 next() throw( xmlsearch::excep::XmlSearchException )
            {
                if( iterator_ )
                    return document_ = iterator_->next();

                throw xmlsearch::excep::XmlSearchException( rtl::OUString::createFromAscii( "NextDocGenerator::next ->" ) );
            }

            sal_Int32 getDocument() { return document_; }

            sal_Int32 getConcept() { return concept_; }

            sal_Int32 getQueryMask() { return queryMask_; }

            rtl::Reference< ConceptData > getTerms() { return terms_; }

            bool smallerThan( NextDocGenerator* other )
            {
                return
                    document_ < other->document_
                    ||
                    document_ == other->document_ && concept_ < other->concept_;
            }


        private:

            sal_Int32                                document_,concept_,queryMask_;
            rtl::Reference< ConceptData >            terms_;
            NonnegativeIntegerGenerator*             iterator_;
        };


        class NextDocGeneratorHeap
        {
        public:

            NextDocGeneratorHeap()
                : nonEmpty_( false ),
                  heapSize_( 0 ),
                  free_( 0 )

            {
            }

            ~NextDocGeneratorHeap()
            {
                reset();
            }

            void reset();

            void addGenerator( NextDocGenerator* gen );

            void start();

            bool isNonEmpty() const { return nonEmpty_; }

            void step() throw( xmlsearch::excep::XmlSearchException );

            sal_Int32 getDocument()  { return heap_[0]->getDocument(); }

            sal_Int32 getConcept() { return heap_[0]->getConcept(); }

            sal_Int32 getQueryMask() { return heap_[0]->getQueryMask(); }

            rtl::Reference< ConceptData > getTerms() { return heap_[0]->getTerms(); }

            bool atDocument( sal_Int32 document );


        private:

            bool         nonEmpty_;

            sal_Int32    heapSize_,free_;

            std::vector< NextDocGenerator* >   heap_;

            void heapify( sal_Int32 i );
        };



        class RoleFiller
        {
        public:

            static RoleFiller* STOP() { return &roleFiller_; }

            RoleFiller();

            RoleFiller( sal_Int32 nColumns,
                        ConceptData* first,
                        sal_Int32 role,
                        sal_Int32 pos,
                        sal_Int32 parentContext,
                        sal_Int32 limit );

            ~RoleFiller();

            void acquire() { ++m_nRefcount; }
            void release() { if( ! --m_nRefcount ) delete this; }

            void scoreList( Query* ,sal_Int32 );

            sal_Int32 getConcept();

            void use( std::vector< RoleFiller*>& place,sal_Int32 query );

        private:

            static RoleFiller roleFiller_;

            sal_uInt32     m_nRefcount;
            sal_uInt8      fixedRole_;
            sal_Int16      filled_;
            sal_Int32      begin_,end_,parentContext_,limit_;

            RoleFiller*    next_;
            std::vector< RoleFiller* > fillers_;

            ConceptData*    conceptData_;

            double penalty( Query* query,sal_Int32 nColumns );

            void makeQueryHit( Query* query,sal_Int32 doc,double penalty_ );

            void considerReplacementWith( RoleFiller* replacement );

            double getScore() { return conceptData_->getScore(); }
        };



        class ConceptGroupGenerator
            : public xmlsearch::util::CompressorIterator
        {
        public:

            static const sal_Int32 NConceptsInGroup;

            ConceptGroupGenerator();

            ConceptGroupGenerator( sal_Int32 dataL,sal_Int8* data,sal_Int32 index,sal_Int32 k );

            virtual ~ConceptGroupGenerator();

            void init( sal_Int32 bytesL,sal_Int8* bytes,sal_Int32 index,sal_Int32 k );

            bool next() throw( xmlsearch::excep::XmlSearchException );

            void generateFillers( std::vector< RoleFiller* >& array );

            sal_Int32 position() { return last_; }

            void value( sal_Int32 value_ ) { last_ += value_; }

            sal_Int32 decodeConcepts( sal_Int32 k, sal_Int32 shift, sal_Int32 *concepts ) throw( xmlsearch::excep::XmlSearchException );

            void addTerms( sal_Int32 index, ConceptData* terms );

        private:

            static const sal_Int32 BitsInLabel;

            sal_Int32                                          last_,k1_,k2_;
            std::vector< rtl::Reference< ConceptData > >       table_;
            xmlsearch::util::Decompressor*                     bits_;
            rtl::Reference< ConceptData>                       cData_;
        };


        class GeneratorHeap
        {
        public:

            GeneratorHeap()
                : heapSize_( 0 ),
                  free_( 0 )
            { }

            ~GeneratorHeap() { reset(); }

            bool start( std::vector< RoleFiller* >& start ) throw( xmlsearch::excep::XmlSearchException );

            bool next( std::vector< RoleFiller* >& start ) throw( xmlsearch::excep::XmlSearchException );

            void reset();

            void addGenerator( ConceptGroupGenerator* cd );

        private:

            sal_Int32   heapSize_,free_;

            std::vector< ConceptGroupGenerator* >  heap_;

            void buildHeap();

            void heapify( sal_Int32 root );

        };



    }
}

#endif
