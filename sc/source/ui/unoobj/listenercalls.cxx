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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



#include <com/sun/star/util/XModifyListener.hpp>
#include <tools/debug.hxx>

#include "listenercalls.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------

ScUnoListenerCalls::ScUnoListenerCalls()
{
}

ScUnoListenerCalls::~ScUnoListenerCalls()
{
    DBG_ASSERT( aEntries.empty(), "unhandled listener calls remaining" );
}

void ScUnoListenerCalls::Add( const uno::Reference<util::XModifyListener>& rListener,
                                const lang::EventObject& rEvent )
{
    if ( rListener.is() )
        aEntries.push_back( ScUnoListenerEntry( rListener, rEvent ) );
}

void ScUnoListenerCalls::ExecuteAndClear()
{
    //  Execute all stored calls and remove them from the list.
    //  During each modified() call, Add may be called again.
    //  These new calls are executed here, too.

    if (!aEntries.empty())
    {
        std::list<ScUnoListenerEntry>::iterator aItr(aEntries.begin());
        std::list<ScUnoListenerEntry>::iterator aEndItr(aEntries.end());
        while ( aItr != aEndItr )
        {
            ScUnoListenerEntry aEntry = *aItr;
            try
            {
                aEntry.xListener->modified( aEntry.aEvent );
            }
            catch ( uno::RuntimeException )
            {
                // the listener is an external object and may throw a RuntimeException
                // for reasons we don't know
            }

            //  New calls that are added during the modified() call are appended to the end
            //  of aEntries, so the loop will catch them, too (as long as erase happens
            //  after modified).

            aItr = aEntries.erase(aItr);
        }
    }
}

