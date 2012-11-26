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
#include "precompiled_svx.hxx"
#include "AccessibleTextEventQueue.hxx"
#include <svx/unoshape.hxx>
#include "editeng/unolingu.hxx"
#include <editeng/unotext.hxx>

#include "editeng/unoedhlp.hxx"
#include "editeng/unopracc.hxx"
#include <svx/svdmodel.hxx>
#include <svx/svdpntv.hxx>
#include <editeng/editdata.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>

namespace accessibility
{
    //------------------------------------------------------------------------
    //
    // EventQueue implementation
    //
    //------------------------------------------------------------------------

    AccessibleTextEventQueue::AccessibleTextEventQueue()
    {
    }

    AccessibleTextEventQueue::~AccessibleTextEventQueue()
    {
        Clear();
    }

    void AccessibleTextEventQueue::Append( const SfxHint& rHint )
    {
        maEventQueue.push_back( new SfxHint( rHint ) );
    }

    void AccessibleTextEventQueue::Append( const SdrBaseHint& rHint )
    {
        maEventQueue.push_back( new SdrBaseHint( rHint ) );
    }

    void AccessibleTextEventQueue::Append( const SfxSimpleHint& rHint )
    {
        maEventQueue.push_back( new SfxSimpleHint( rHint ) );
    }

    void AccessibleTextEventQueue::Append( const TextHint& rHint )
    {
        maEventQueue.push_back( new TextHint( rHint ) );
    }

    void AccessibleTextEventQueue::Append( const SvxViewHint& rHint )
    {
        maEventQueue.push_back( new SvxViewHint( rHint ) );
    }

    void AccessibleTextEventQueue::Append( const SvxEditSourceHint& rHint )
    {
        maEventQueue.push_back( new SvxEditSourceHint( rHint ) );
    }

    ::std::auto_ptr< SfxHint > AccessibleTextEventQueue::PopFront()
    {
        ::std::auto_ptr< SfxHint > aRes( *(maEventQueue.begin()) );
        maEventQueue.pop_front();
        return aRes;
    }

    bool AccessibleTextEventQueue::IsEmpty() const
    {
        return maEventQueue.empty();
    }

    void AccessibleTextEventQueue::Clear()
    {
        // clear queue
        while( !IsEmpty() )
            PopFront();
    }

} // end of namespace accessibility

//------------------------------------------------------------------------
