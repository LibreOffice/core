/*************************************************************************
*
*  OpenOffice.org - a multi-platform office productivity suite
*
*  $RCSfile: RangeEdit.cxx,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: vg $ $Date: 2007-09-18 14:53:26 $
*
*  The Contents of this file are made available subject to
*  the terms of GNU Lesser General Public License Version 2.1.
*
*
*    GNU Lesser General Public License Version 2.1
*    =============================================
*    Copyright 2005 by Sun Microsystems, Inc.
*    901 San Antonio Road, Palo Alto, CA 94303, USA
*
*    This library is free software; you can redistribute it and/or
*    modify it under the terms of the GNU Lesser General Public
*    License version 2.1, as published by the Free Software Foundation.
*
*    This library is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*    Lesser General Public License for more details.
*
*    You should have received a copy of the GNU Lesser General Public
*    License along with this library; if not, write to the Free Software
*    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
*    MA  02111-1307  USA
*
************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "RangeEdit.hxx"

namespace chart
{

//============================================================================
// class RangeEdit
//----------------------------------------------------------------------------
RangeEdit::RangeEdit( Window* pParent, const ResId& rResId):
    Edit( pParent , rResId )
{
}

RangeEdit::~RangeEdit()
{
}

void RangeEdit::SetKeyInputHdl( const Link& rKeyInputLink )
{
    m_aKeyInputLink = rKeyInputLink;
}

void RangeEdit::KeyInput( const KeyEvent& rKEvt )
{
    const KeyCode& rKeyCode = rKEvt.GetKeyCode();
    if( !rKeyCode.GetModifier() && rKeyCode.GetCode() == KEY_F2 )
    {
        if ( m_aKeyInputLink.IsSet() )
        {
            m_aKeyInputLink.Call( NULL );
        }
    }
    else
       Edit::KeyInput( rKEvt );
}

//.............................................................................
} //namespace chart
//.............................................................................
