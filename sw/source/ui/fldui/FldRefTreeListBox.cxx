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
#include "precompiled_sw.hxx"

#include <FldRefTreeListBox.hxx>

#include <helpid.h>
#include <vcl/help.hxx>

SwFldRefTreeListBox::SwFldRefTreeListBox( Window* pParent,
                                          const ResId& rResId)
    : SvTreeListBox( pParent, rResId )
{
    SetHelpId( HID_REFSELECTION_TOOLTIP );
}

SwFldRefTreeListBox::~SwFldRefTreeListBox()
{
}

void SwFldRefTreeListBox::RequestHelp( const HelpEvent& rHEvt )
{
    bool bCallBase( true );
    if ( rHEvt.GetMode() & HELPMODE_QUICK )
    {
        Point aPos( ScreenToOutputPixel( rHEvt.GetMousePosPixel() ));
        SvLBoxEntry* pEntry = GetEntry( aPos );
        if( pEntry )
        {
            String sEntry( GetEntryText( pEntry ) );
            SvLBoxTab* pTab;
            SvLBoxItem* pItem = GetItem( pEntry, aPos.X(), &pTab );
            if( pItem )
            {
                aPos = GetEntryPosition( pEntry );

                aPos.X() = GetTabPos( pEntry, pTab );
                Size aSize( pItem->GetSize( this, pEntry ) );

                if((aPos.X() + aSize.Width()) > GetSizePixel().Width())
                    aSize.Width() = GetSizePixel().Width() - aPos.X();

                aPos = OutputToScreenPixel(aPos);
                Rectangle aItemRect( aPos, aSize );
                Help::ShowQuickHelp( this, aItemRect, sEntry,
                    QUICKHELP_LEFT|QUICKHELP_VCENTER );
                bCallBase = false;
            }
        }
    }
    if ( bCallBase )
    {
        Window::RequestHelp( rHEvt );
    }
}
