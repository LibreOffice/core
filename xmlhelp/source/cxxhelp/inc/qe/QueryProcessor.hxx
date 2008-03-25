/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: QueryProcessor.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 15:17:59 $
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
#ifndef _XMLSEARCH_QE_QUERYPROCESSOR_HXX_
#define _XMLSEARCH_QE_QUERYPROCESSOR_HXX_

#ifndef INCLUDED_STL_VECTOR
#include <vector>
#define INCLUDED_STL_VECTOR
#endif
#ifndef _RTL_USTRING_HXX_
#include  <rtl/ustring.hxx>
#endif
#ifndef _XMLSEARCH_EXCEP_XMLSEARCHEXCEPTIONS_HXX_
#include <excep/XmlSearchExceptions.hxx>
#endif
#ifndef _XMLSEARCH_QE_QUERY_HXX_
#include <qe/Query.hxx>
#endif
#ifndef _XMLSEARCH_QE_SEARCH_HXX_
#include <qe/Search.hxx>
#endif
#ifndef _XMLSEARCH_QE_XMLINDEX_HXX_
#include <xmlsearch/qe/XmlIndex.hxx>
#endif


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


            const sal_Int32 getHitCount() const { return nHits_; }

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
