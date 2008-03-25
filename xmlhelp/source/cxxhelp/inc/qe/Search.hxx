/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Search.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 15:18:10 $
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
#ifndef _XMLSEARCH_QE_SEARCH_HXX_
#define _XMLSEARCH_QE_SEARCH_HXX_


#ifndef INCLUDED_STL_VECTOR
#include <vector>
#define INCLUDED_STL_VECTOR
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include  <rtl/ustring.hxx>
#endif
#ifndef _XMLSEARCH_QE_QUERY_HXX_
#include <qe/Query.hxx>
#endif
#ifndef _XMLSEARCH_QE_XMLINDEX_HXX_
#include <qe/XmlIndex.hxx>
#endif
#ifndef _XMLSEARCH_DB_CONTEXTTABLES_HXX_
#include <qe/ContextTables.hxx>
#endif
#ifndef _XMLSEARCH_QE_CONCEPTDATA_HXX_
#include <qe/ConceptData.hxx>
#endif
#ifndef _XMLSEARCH_QE_DOCGENERATOR_HXX_
#include <qe/DocGenerator.hxx>
#endif


class QueryFactoryImpl;


namespace xmlsearch {

    namespace qe {


        class RoleFiller;


        class Search
        {
        public:

            Search( XmlIndex* env );
            ~Search();

            Query* addQuery( const rtl::OUString& context,
                             sal_Int32 nValidTerms,sal_Int32 nMissingTerms,sal_Int32 nHits,
                             double variantPenalty,
                             const std::vector< sal_Int32 >& primary,
                             const std::vector< std::vector< sal_Int32 > >& columns );

            void startSearch();


        private:

            XmlIndex*                                      env_;
            QueryFactoryImpl*                              queryFactory_;
            NextDocGeneratorHeap                           nextDocGenHeap_;
            GeneratorHeap                                  genHeap_;
            ConceptGroupGenerator                          firstGenerator_;
            std::vector< Query* >                          queries_;
            ConceptData**                                  conceptVisitor_;
            std::vector< rtl::Reference< ConceptData > >   conceptData_;
            sal_Int32                     free2_,limit_,document_,max_,base_,startingIndex_,nConcepts_;
            sal_Int32                     *concepts_;
            std::vector<sal_Int32>        docConcepts_,queryMasks_,maxConcepts_,kTable_,offsets_;

            sal_Int32                     dataL_;
            sal_Int8                      *data_;

            void addTerm( sal_Int32 col,sal_Int32 concept,double score );

            sal_Int32 partition( sal_Int32 p,sal_Int32 r );

            void quicksort( sal_Int32 p,sal_Int32 r );

            void searchDocument();

            sal_Int32 nextDocument( std::vector< RoleFiller* >& start ) throw( xmlsearch::excep::XmlSearchException );

            bool openDocumentIndex( sal_Int32 docNo ) throw( xmlsearch::excep::XmlSearchException );

            ConceptGroupGenerator* makeGenerator( sal_Int32 group ) throw( xmlsearch::excep::XmlSearchException ); //t

            sal_Int32 indexOf( sal_Int32 concept ) throw( xmlsearch::excep::XmlSearchException );
        };


    }

}


#endif
