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

#ifndef SFX_SIDEBAR_CONTEXT_HXX
#define SFX_SIDEBAR_CONTEXT_HXX

#include "rtl/ustring.hxx"

#include <vector>

namespace sfx2 { namespace sidebar {

class Context
{
public:
    ::rtl::OUString msApplication;
    ::rtl::OUString msContext;

    Context (void);
    Context (
        const ::rtl::OUString& rsApplication,
        const ::rtl::OUString& rsContext);

    /** When two contexts are matched against each other then
        application or context name may have the wildcard value 'any'.
        In order to prefer matches without wildcards over matches with
        wildcards we introduce a integer evaluation for matches.
    */
    const static sal_Int32 NoMatch;
    const static sal_Int32 OptimalMatch;
    const static sal_Int32 ApplicationWildcardMatch;
    const static sal_Int32 ContextWildcardMatch;

    /** Return the numeric value that describes how good the match
        between two contexts is.
        Smaller values represent better matches.
    */
    sal_Int32 EvaluateMatch (const Context& rOther) const;

    /** Return the best match against the given list of contexts.
    */
    sal_Int32 EvaluateMatch (const ::std::vector<Context>& rOthers) const;

    bool operator== (const Context& rOther) const;
    bool operator!= (const Context& rOther) const;
};


} } // end of namespace sfx2::sidebar

#endif
