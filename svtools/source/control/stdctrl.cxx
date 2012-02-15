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
#include "precompiled_svtools.hxx"

#include <svtools/stdctrl.hxx>

// =======================================================================

FixedInfo::FixedInfo( Window* pParent, WinBits nWinStyle ) :
    FixedText( pParent, nWinStyle | WB_INFO )
{
}

// -----------------------------------------------------------------------

FixedInfo::FixedInfo( Window* pParent, const ResId& rResId ) :
    FixedText( pParent, rResId )
{
    SetStyle( GetStyle() | WB_INFO );
}

namespace svt
{
    // class svt::SelectableFixedText ----------------------------------------

    SelectableFixedText::SelectableFixedText( Window* pParent, WinBits nWinStyle ) :
        Edit( pParent, nWinStyle )
    {
        Init();
    }

    // -----------------------------------------------------------------------

    SelectableFixedText::SelectableFixedText( Window* pParent, const ResId& rResId ) :
        Edit( pParent, rResId )
    {
        Init();
    }

    // -----------------------------------------------------------------------

    SelectableFixedText::~SelectableFixedText()
    {
    }

    // -----------------------------------------------------------------------

    void SelectableFixedText::Init()
    {
        // no border
        SetBorderStyle( WINDOW_BORDER_NOBORDER );
        // read-only
        SetReadOnly();
        // make it transparent
        SetControlBackground();
        SetBackground();
        SetPaintTransparent( sal_True );
    }

    // -----------------------------------------------------------------------

    void SelectableFixedText::LoseFocus()
    {
        Edit::LoseFocus();
        // clear cursor
        Invalidate();
    }

} // namespace svt

