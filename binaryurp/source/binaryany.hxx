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



#ifndef INCLUDED_BINARYURP_SOURCE_BINARYANY_HXX
#define INCLUDED_BINARYURP_SOURCE_BINARYANY_HXX

#include "sal/config.h"

#include "uno/any2.h"

namespace com { namespace sun { namespace star { namespace uno {
    class TypeDescription;
} } } }

namespace binaryurp {

class BinaryAny {
public:
    BinaryAny() throw ();

    BinaryAny(com::sun::star::uno::TypeDescription const & type, void * value)
        throw ();

    explicit BinaryAny(uno_Any const & raw) throw ();
        // takes over raw.pData (but copies raw.pType); raw must not be passed
        // to uno_any_destruct

    BinaryAny(BinaryAny const & other) throw ();

    ~BinaryAny() throw ();

    BinaryAny & operator =(BinaryAny const & other) throw ();

    uno_Any * get() throw ();

    com::sun::star::uno::TypeDescription getType() const throw ();

    void * getValue(com::sun::star::uno::TypeDescription const & type) const
        throw ();

private:
    mutable uno_Any data_;
        // mutable so that getValue() can return a non-const void *, as in turn
        // required at various places in binary UNO
};

}

#endif
