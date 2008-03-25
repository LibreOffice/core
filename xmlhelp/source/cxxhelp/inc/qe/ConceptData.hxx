/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ConceptData.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 15:16:59 $
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
#ifndef _XMLSEARCH_QE_CONCEPTDATA_HXX_
#define _XMLSEARCH_QE_CONCEPTDATA_HXX_

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef INCLUDED_STL_VECTOR
#include <vector>
#define INCLUDED_STL_VECTOR
#endif


namespace xmlsearch {

    namespace qe {


        class RoleFiller;
        class Query;
        class ContextTables;

        class ConceptData
        {
        public:

            ConceptData( sal_Int32 id = 0,
                         sal_Int32 role = 0,
                         double score = 0.0,
                         sal_Int32 queryNo = 0,
                         sal_Int32 nColumns = 0,
                         ContextTables* contextTables = 0 );

            virtual ~ConceptData();

            void acquire() { ++m_nRefcount; }
            void release() { if( ! --m_nRefcount ) delete this; }

            bool cEquals( ConceptData* r ) { return concept_ == r->concept_; }
            bool crEquals( ConceptData* r ) { return cEquals( r ) && role_ == r->role_; }
            bool crqEquals( ConceptData* r ) { return crEquals( r ) && queryNo_ == r->queryNo_; }
            bool compareWith( ConceptData* r ) { return
                                                     concept_ < r->concept_                 ||
                                                     cEquals( r ) && role_ < r->role_       ||
                                                     crEquals( r ) && penalty_ < r->penalty_; }

            void addLast( ConceptData* r );

            sal_Int32 getConcept() const { return concept_; }

            sal_Int32 getQueryMask() const
            {
                return ( next_.is() ? next_->getQueryMask() : 0 ) | ( 1 << queryNo_ );
            }

            void runBy( std::vector< Query* >& queries );

            virtual void generateFillers( std::vector< RoleFiller* >& array, sal_Int32 last );

            sal_Int32 getConcept() { return concept_; }

            sal_uInt8 getRole() { return role_; }

            sal_uInt8 getQuery() { return queryNo_; }

            double getScore() { return penalty_; }

            double getPenalty() { return penalty_; }


        protected:

            static const sal_Int32 ProxPerTerm;

            sal_uInt8  queryNo_,nColumns_,role_;

            sal_Int32 concept_,proximity_;

            double penalty_;

            sal_uInt32 m_nRefcount;

            ContextTables* ctx_;
            rtl::Reference< ConceptData > next_;
            std::vector< rtl::Reference< RoleFiller > > roleFillers_;
        };



    }

}


#endif
