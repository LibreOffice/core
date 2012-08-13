/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _XMLSEARCH_QE_QUERY_HXX_
#define _XMLSEARCH_QE_QUERY_HXX_

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <vector>


namespace xmlsearch {

    namespace qe {

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

            QueryHitData( double penalty,const rtl::OUString& document, rtl::OUString* terms )
                : penalty_( penalty ),
                  document_( document ),
                  terms_( terms )      { }

            ~QueryHitData() { delete[] terms_; }

            rtl::OUString getDocument() const { return document_; }

            double getPenalty() const { return penalty_; }


        private:

            double        penalty_;

            const rtl::OUString document_;

            rtl::OUString* terms_;

        };  // end class QueryHitData


        class PrefixTranslator
        {
        public:

            static PrefixTranslator* makePrefixTranslator( const rtl::OUString*,sal_Int32 )
            {
                return 0;
            }
        };
    }

}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
