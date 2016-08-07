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

#ifndef _DBAUI_LISTVIEWITEMS_HXX_
#include "listviewitems.hxx"
#endif
//........................................................................
namespace dbaui
{
//........................................................................

    //========================================================================
    // class OBoldListboxString
    //========================================================================
    //------------------------------------------------------------------------
    void OBoldListboxString::InitViewData( SvLBox* pView,SvLBoxEntry* pEntry, SvViewDataItem* _pViewData)
    {
        SvLBoxString::InitViewData( pView, pEntry, _pViewData );
        if ( !m_bEmphasized )
            return;
        if (!_pViewData)
            _pViewData = pView->GetViewDataItem( pEntry, this );
        pView->Push(PUSH_ALL);
        Font aFont( pView->GetFont());
        aFont.SetWeight(WEIGHT_BOLD);
        pView->SetFont( aFont );
        _pViewData->aSize = Size(pView->GetTextWidth(GetText()), pView->GetTextHeight());
        pView->Pop();
    }

    //------------------------------------------------------------------------
    sal_uInt16 OBoldListboxString::IsA()
    {
        return SV_ITEM_ID_BOLDLBSTRING;
    }

    //------------------------------------------------------------------------
    void OBoldListboxString::Paint(const Point& rPos, SvLBox& rDev, sal_uInt16 nFlags, SvLBoxEntry* pEntry )
    {
        if (m_bEmphasized)
        {
            rDev.Push(PUSH_ALL);
            Font aFont( rDev.GetFont());
            aFont.SetWeight(WEIGHT_BOLD);
            rDev.SetFont( aFont );
            Point aPos(rPos);
            rDev.DrawText( aPos, GetText() );
            rDev.Pop();
        }
        else
            SvLBoxString::Paint(rPos, rDev, nFlags, pEntry);
    }

//........................................................................
}   // namespace dbaui
//........................................................................

