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

#ifndef SFX_SIDEBAR_CONTEXT_MATCHER_HXX
#define SFX_SIDEBAR_CONTEXT_MATCHER_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>

#include <vector>


namespace sfx2 { namespace sidebar {

class Context;


/** Data read from the configuration for matching contexts.
*/
class ContextMatcher
{
public:
    ContextMatcher (void);
    ~ContextMatcher (void);

    sal_Int32 EvaluateMatch (
        const Context& rContext) const;

    void AddMatcher (
        const ::rtl::OUString& rsApplicationName,
        const ::std::vector<rtl::OUString>& rContextNames,
        const bool mbIsContextListNegated);
    void AddMatcher (
        const ::rtl::OUString& rsApplicationName,
        const ::rtl::OUString& rsContextName);

private:
    class Entry
    {
    public:
        ::rtl::OUString msApplicationName;
        ::std::vector<rtl::OUString> maContextNames;
        bool mbIsContextListNegated;
    };
    ::std::vector<Entry> maEntries;

    sal_Int32 EvaluateMatch (
        const Context& rContext,
        const Entry& rEntry) const;

};
static bool IsMatchBetterThan (const sal_Int32 nMatchA, const sal_Int32 nMatchB);


} } // end of namespace sfx2::sidebar

#endif
