/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef INCLUDED_BINARYURP_SOURCE_OUTGOINGREQUEST_HXX
#define INCLUDED_BINARYURP_SOURCE_OUTGOINGREQUEST_HXX

#include "sal/config.h"

#include "typelib/typedescription.hxx"

namespace binaryurp {

struct OutgoingRequest {
    enum Kind { KIND_NORMAL, KIND_REQUEST_CHANGE, KIND_COMMIT_CHANGE };

    OutgoingRequest(
        Kind theKind, com::sun::star::uno::TypeDescription const & theMember,
        bool theSetter):
        kind(theKind), member(theMember), setter(theSetter)
    {}

    Kind kind;

    com::sun::star::uno::TypeDescription member;

    bool setter;
};

}

#endif
