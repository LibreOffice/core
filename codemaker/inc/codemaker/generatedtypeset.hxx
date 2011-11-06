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



#ifndef INCLUDED_CODEMAKER_GENERATEDTYPESET_HXX
#define INCLUDED_CODEMAKER_GENERATEDTYPESET_HXX

#include "rtl/string.hxx"

#include <hash_set>

/// @HTML

namespace codemaker {

/**
   A simple class to track which types have already been processed by a code
   maker.

   <p>This class is not multi-thread&ndash;safe.</p>
 */
class GeneratedTypeSet {
public:
    GeneratedTypeSet() {}

    ~GeneratedTypeSet() {}

    /**
       Add a type to the set of generated types.

       <p>If the type was already present, nothing happens.</p>

       @param type a UNO type registry name
     */
    void add(rtl::OString const & type) { m_set.insert(type); }

    /**
       Checks whether a given type has already been generated.

       @param type a UNO type registry name

       @return true iff the given type has already been generated
     */
    bool contains(rtl::OString const & type) const
    { return m_set.find(type) != m_set.end(); }

private:
    GeneratedTypeSet(GeneratedTypeSet &); // not implemented
    void operator =(GeneratedTypeSet); // not implemented

    std::hash_set< rtl::OString, rtl::OStringHash > m_set;
};

}

#endif // INCLUDED_CODEMAKER_GENERATEDTYPESET_HXX
