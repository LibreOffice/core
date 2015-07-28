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
#ifndef INCLUDED_XMLHELP_SOURCE_CXXHELP_INC_QE_QUERY_HXX
#define INCLUDED_XMLHELP_SOURCE_CXXHELP_INC_QE_QUERY_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <vector>
#include <string.h>

namespace xmlsearch {

    namespace qe {

        class QueryHit
        {
        public:

            QueryHit( sal_Int32 nColumns )
                : matchesL_( 2*nColumns ),
                  matches_( new sal_Int32[ 2*nColumns ] )
            {
                memset( matches_, 0, sizeof( sal_Int32 ) * matchesL_ );
            }

            ~QueryHit() { delete[] matches_; }

        private:
              sal_Int32    matchesL_;
              sal_Int32    *matches_;    // ...concept, word number, ...

        }; // end class QueryHit



        class QueryHitData
        {
        public:
            QueryHitData( OUString* terms )
                : terms_( terms )      { }

            ~QueryHitData() { delete[] terms_; }

        private:
            OUString* terms_;

        };  // end class QueryHitData
    }

}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
