/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ConceptData.hxx,v $
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
#ifndef _XMLSEARCH_QE_CONCEPTDATA_HXX_
#define _XMLSEARCH_QE_CONCEPTDATA_HXX_

#include <rtl/ref.hxx>
#include <sal/types.h>
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
                                                     (concept_ < r->concept_)                 ||
                                                     (cEquals( r ) && role_ < r->role_)       ||
                                                     (crEquals( r ) && penalty_ < r->penalty_); }

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
