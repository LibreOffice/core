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

#ifndef SFX_SIDEBAR_CONTEXT_LIST_HXX
#define SFX_SIDEBAR_CONTEXT_LIST_HXX

#include "Context.hxx"
#include <sal/types.h>
#include <rtl/ustring.hxx>

#include <vector>


namespace sfx2 { namespace sidebar {


class ContextList
{
public:
    ContextList (void);
    ~ContextList (void);

    class Entry
    {
    public:
        Context maContext;
        bool mbIsInitiallyVisible;
        ::rtl::OUString msMenuCommand;
    };

    /** Return <TRUE/> when the given context matches any of the stored contexts.
    */
    const Entry* GetMatch (
        const Context& rContext) const;

    void AddContextDescription (
        const Context& rContext,
        const bool bIsInitiallyVisible,
        const ::rtl::OUString& rsMenuCommand);

    /** Returns <TRUE/> when no call to AddContextDescription() was made before.
    */
    bool IsEmpty (void);


private:
    ::std::vector<Entry> maEntries;

    ::std::vector<Entry>::const_iterator FindBestMatch (const Context& rContext) const;
};


} } // end of namespace sfx2::sidebar

#endif
