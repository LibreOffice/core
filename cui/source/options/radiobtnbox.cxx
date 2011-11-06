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
#include "precompiled_cui.hxx"

// include ---------------------------------------------------------------

#include "radiobtnbox.hxx"
#include <dialmgr.hxx>

namespace svx {

// class SvxRadioButtonListBox ----------------------------------------------------

SvxRadioButtonListBox::SvxRadioButtonListBox( Window* _pParent, const ResId& _rId ) :

    SvxSimpleTable( _pParent, _rId )

{
    EnableCheckButton( new SvLBoxButtonData( this, true ) );
}

SvxRadioButtonListBox::~SvxRadioButtonListBox()
{
}

void SvxRadioButtonListBox::SetTabs()
{
    SvxSimpleTable::SetTabs();
/*
    sal_uInt16 nAdjust = SV_LBOXTAB_ADJUST_RIGHT | SV_LBOXTAB_ADJUST_LEFT |
                     SV_LBOXTAB_ADJUST_CENTER | SV_LBOXTAB_ADJUST_NUMERIC | SV_LBOXTAB_FORCE;
    if ( aTabs.Count() > 0 )
    {
        SvLBoxTab* pTab = (SvLBoxTab*)aTabs.GetObject(0);
        pTab->nFlags &= ~nAdjust;
        pTab->nFlags |= SV_LBOXTAB_PUSHABLE | SV_LBOXTAB_ADJUST_CENTER | SV_LBOXTAB_FORCE;
    }
*/
}

void SvxRadioButtonListBox::MouseButtonUp( const MouseEvent& _rMEvt )
{
    m_aCurMousePoint = _rMEvt.GetPosPixel();
    SvxSimpleTable::MouseButtonUp( _rMEvt );
}

void SvxRadioButtonListBox::KeyInput( const KeyEvent& rKEvt )
{
    if ( !rKEvt.GetKeyCode().GetModifier() && KEY_SPACE == rKEvt.GetKeyCode().GetCode() )
    {
        SvLBoxEntry* pEntry = FirstSelected();
        if ( GetCheckButtonState( pEntry ) == SV_BUTTON_UNCHECKED )
        {
            SetCheckButtonState( pEntry, SV_BUTTON_CHECKED );
            GetCheckButtonHdl().Call( NULL );
            return ;
        }
    }

    SvxSimpleTable::KeyInput( rKEvt );
}

void SvxRadioButtonListBox::HandleEntryChecked( SvLBoxEntry* _pEntry )
{
    Select( _pEntry, sal_True );
    SvButtonState eState = GetCheckButtonState( _pEntry );

    if ( SV_BUTTON_CHECKED == eState )
    {
        // we have radio button behavior -> so uncheck the other entries
        SvLBoxEntry* pEntry = First();
        while ( pEntry )
        {
            if ( pEntry != _pEntry )
                SetCheckButtonState( pEntry, SV_BUTTON_UNCHECKED );
            pEntry = Next( pEntry );
        }
    }
    else
        SetCheckButtonState( _pEntry, SV_BUTTON_CHECKED );
}

const Point& SvxRadioButtonListBox::GetCurMousePoint() const
{
    return m_aCurMousePoint;
}

} // end of namespace ::svx

