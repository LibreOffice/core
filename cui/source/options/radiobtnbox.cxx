/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

