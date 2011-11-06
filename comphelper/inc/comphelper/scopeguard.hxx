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



#if ! defined(INCLUDED_COMPHELPER_SCOPEGUARD_HXX)
#define INCLUDED_COMPHELPER_SCOPEGUARD_HXX

#if ! defined(INCLUDED_COMPHELPERDLLAPI_H)
#include "comphelper/comphelperdllapi.h"
#endif
#include "boost/function.hpp"
#include "boost/noncopyable.hpp"
#include "boost/bind.hpp"

namespace comphelper {

/** ScopeGuard to ease writing exception-safe code.
 */
class COMPHELPER_DLLPUBLIC ScopeGuard : private ::boost::noncopyable
                                        // noncopyable until we have
                                        // good reasons...
{
public:
    enum exc_handling { IGNORE_EXCEPTIONS, ALLOW_EXCEPTIONS };

    /** @param func function object to be executed in dtor
        @param excHandling switches whether thrown exceptions in dtor will be
                           silently ignored (but OSL_ asserted)
    */
    template <typename func_type>
    explicit ScopeGuard( func_type const & func,
                         exc_handling excHandling = IGNORE_EXCEPTIONS )
        : m_func( func ), m_excHandling( excHandling ) {}

    ~ScopeGuard();

    /** Dismisses the scope guard, i.e. the function won't
        be executed.
    */
    void dismiss();

private:
    ::boost::function0<void> m_func; // preferring portable syntax
    exc_handling const m_excHandling;
};

} // namespace comphelper

#endif // ! defined(INCLUDED_COMPHELPER_SCOPEGUARD_HXX)

