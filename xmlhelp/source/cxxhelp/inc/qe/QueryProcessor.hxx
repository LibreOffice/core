/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: QueryProcessor.hxx,v $
 * $Revision: 1.3.26.1 $
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
#ifndef _XMLSEARCH_QE_QUERYPROCESSOR_HXX_
#define _XMLSEARCH_QE_QUERYPROCESSOR_HXX_

#ifndef INCLUDED_STL_VECTOR
#include <vector>
#define INCLUDED_STL_VECTOR
#endif
#include  <rtl/ustring.hxx>
#include <excep/XmlSearchExceptions.hxx>
#include <qe/Query.hxx>
#include <qe/Search.hxx>


namespace xmlsearch {

    namespace qe {


        class QueryStatement
        {
        public:

            QueryStatement( sal_Int32 nHits,
                            const std::vector< rtl::OUString >& terms,
                            const rtl::OUString& scope )
                : nHits_( nHits ),
                  terms_( terms ),
                  scope_( scope )
            {
            }

            ~QueryStatement()
            {
            }


            sal_Int32 getHitCount() const { return nHits_; }

            const std::vector< rtl::OUString >& getTerms() const { return terms_; }

            const rtl::OUString getScope() const { return scope_; }


        private:

            sal_Int32                    nHits_;
            std::vector< rtl::OUString > terms_;
            rtl::OUString                scope_;

        };



        class QueryResults
        {
            friend class QueryHitIterator;

        public:

            QueryResults( Query*, sal_Int32 );

            ~QueryResults();

            QueryHitIterator* makeQueryHitIterator() const
            {
                return new QueryHitIterator( this );
            }


        private:

            std::vector< QueryHitData* >   queryHits_;
        };



        class QueryProcessor
        {
        public:

            QueryProcessor( const rtl::OUString& installDir )
                throw( xmlsearch::excep::IOException );

            ~QueryProcessor();

            QueryResults* processQuery( const QueryStatement& ment );

        private:

            static const double INFLpenalty;

            Query* processQuery( Search&,const QueryStatement& );

            QueryResults* makeQueryResults( Query*,sal_Int32 nHits );

            XmlIndex   env_;
        };

    }

}


#endif
