/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Search.hxx,v $
 * $Revision: 1.3 $
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
#ifndef _XMLSEARCH_QE_SEARCH_HXX_
#define _XMLSEARCH_QE_SEARCH_HXX_


#ifndef INCLUDED_STL_VECTOR
#include <vector>
#define INCLUDED_STL_VECTOR
#endif
#include <rtl/ref.hxx>
#include  <rtl/ustring.hxx>
#include <qe/Query.hxx>
#include <qe/XmlIndex.hxx>
#ifndef _XMLSEARCH_DB_CONTEXTTABLES_HXX_
#include <qe/ContextTables.hxx>
#endif
#include <qe/ConceptData.hxx>
#include <qe/DocGenerator.hxx>


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
