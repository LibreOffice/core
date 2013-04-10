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

#include "precompiled_sfx2.hxx"

#include "ContextList.hxx"
#include "Context.hxx"

using ::rtl::OUString;

namespace sfx2 { namespace sidebar {


ContextList::ContextList (void)
    : maEntries()
{
}




ContextList::~ContextList (void)
{
}




const ContextList::Entry* ContextList::GetMatch (const Context& rContext) const
{
    const ::std::vector<Entry>::const_iterator iEntry = FindBestMatch(rContext);
    if (iEntry != maEntries.end())
        return &*iEntry;
    else
        return NULL;
}




::std::vector<ContextList::Entry>::const_iterator ContextList::FindBestMatch (const Context& rContext) const
{
    sal_Int32 nBestMatch (Context::NoMatch);
    ::std::vector<Entry>::const_iterator iBestMatch (maEntries.end());

    for (::std::vector<Entry>::const_iterator
             iEntry(maEntries.begin()),
             iEnd(maEntries.end());
         iEntry!=iEnd;
         ++iEntry)
    {
        const sal_Int32 nMatch (rContext.EvaluateMatch(iEntry->maContext));
        if (nMatch < nBestMatch)
        {
            nBestMatch = nMatch;
            iBestMatch = iEntry;
        }
        if (nBestMatch == Context::OptimalMatch)
            return iEntry;
    }

    return iBestMatch;
}




void ContextList::AddContextDescription (
    const Context& rContext,
    const bool bIsInitiallyVisible,
    const OUString& rsMenuCommand)
{
    maEntries.push_back(Entry());
    maEntries.back().maContext = rContext;
    maEntries.back().mbIsInitiallyVisible = bIsInitiallyVisible;
    maEntries.back().msMenuCommand = rsMenuCommand;
}




bool ContextList::IsEmpty (void)
{
    return maEntries.empty();
}


} } // end of namespace sfx2::sidebar
