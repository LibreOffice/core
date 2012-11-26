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
#include "precompiled_desktop.hxx"


#include "svtools/svmedit2.hxx"
#include "svl/lstner.hxx"
#include "svtools/xtextedt.hxx"
#include "vcl/scrbar.hxx"

#include "dp_gui_autoscrolledit.hxx"


namespace dp_gui {


AutoScrollEdit::AutoScrollEdit( Window* pParent, const ResId& rResId )
    : ExtMultiLineEdit( pParent, rResId )
{
    ScrollBar*  pScroll = GetVScrollBar();
    if (pScroll)
        pScroll->Hide();
//    SetLeftMargin( 0 );
    StartListening( *GetTextEngine() );
}

AutoScrollEdit::~AutoScrollEdit()
{
    EndListeningAll();
}

void AutoScrollEdit::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    const TextHint* pTextHint = dynamic_cast< const TextHint* >(&rHint);

    if ( pTextHint )
    {
        sal_uLong   nId = pTextHint->GetId();
        if ( nId == TEXT_HINT_VIEWSCROLLED )
        {
            ScrollBar*  pScroll = GetVScrollBar();
            if ( pScroll )
                pScroll->Show();
        }
    }
}


} // namespace dp_gui

