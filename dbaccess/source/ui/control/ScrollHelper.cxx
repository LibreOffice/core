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
#include "precompiled_dbui.hxx"

#ifndef DBAUI_SCROLLHELPER_HXX
#include "ScrollHelper.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#define LISTBOX_SCROLLING_AREA  12
namespace dbaui
{
DBG_NAME(OScrollHelper)

// -----------------------------------------------------------------------------


    OScrollHelper::OScrollHelper()
    {
        DBG_CTOR(OScrollHelper,NULL);
    }
    // -----------------------------------------------------------------------------
    OScrollHelper::~OScrollHelper()
    {

        DBG_DTOR(OScrollHelper,NULL);
    }
    // -----------------------------------------------------------------------------
    void OScrollHelper::scroll(const Point& _rPoint, const Size& _rOutputSize)
    {
        // Scrolling Areas
        Rectangle aScrollArea( Point(0, _rOutputSize.Height() - LISTBOX_SCROLLING_AREA),
                                     Size(_rOutputSize.Width(), LISTBOX_SCROLLING_AREA) );

        Link aToCall;
        // if pointer in bottom area begin scroll
        if( aScrollArea.IsInside(_rPoint) )
            aToCall = m_aUpScroll;
        else
        {
            aScrollArea.SetPos(Point(0,0));
            // if pointer in top area begin scroll
            if( aScrollArea.IsInside(_rPoint) )
                aToCall = m_aDownScroll;
        }
        if ( aToCall.IsSet() )
            aToCall.Call( NULL );
    }
    // -----------------------------------------------------------------------------
}
// -----------------------------------------------------------------------------


