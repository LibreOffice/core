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

#include <vcl/scrbar.hxx>
#include <svtools/txtattr.hxx>
#include <svtools/xtextedt.hxx>

#include "descedit.hxx"

#include "dp_gui.hrc"

using dp_gui::DescriptionEdit;

// DescriptionEdit -------------------------------------------------------

DescriptionEdit::DescriptionEdit( Window* pParent, const ResId& rResId ) :

    ExtMultiLineEdit( pParent, rResId ),

    m_bIsVerticalScrollBarHidden( true )

{
    Init();
}

// -----------------------------------------------------------------------

void DescriptionEdit::Init()
{
    Clear();
    // no tabstop
    SetStyle( ( GetStyle() & ~WB_TABSTOP ) | WB_NOTABSTOP );
    // read-only
    SetReadOnly();
    // no cursor
    EnableCursor( sal_False );
}

// -----------------------------------------------------------------------

void DescriptionEdit::UpdateScrollBar()
{
    if ( m_bIsVerticalScrollBarHidden )
    {
        ScrollBar*  pVScrBar = GetVScrollBar();
        if ( pVScrBar && pVScrBar->GetVisibleSize() < pVScrBar->GetRangeMax() )
        {
            pVScrBar->Show();
            m_bIsVerticalScrollBarHidden = false;
        }
    }
}

// -----------------------------------------------------------------------

void DescriptionEdit::Clear()
{
    SetText( String() );

    m_bIsVerticalScrollBarHidden = true;
    ScrollBar*  pVScrBar = GetVScrollBar();
    if ( pVScrBar )
        pVScrBar->Hide();
}

// -----------------------------------------------------------------------

void DescriptionEdit::SetDescription( const String& rDescription )
{
    SetText( rDescription );
    UpdateScrollBar();
}

