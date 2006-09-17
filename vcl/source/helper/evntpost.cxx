/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: evntpost.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 12:14:02 $
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
#include "precompiled_vcl.hxx"



#include "evntpost.hxx"
#include "svapp.hxx"

namespace vcl
{

EventPoster::EventPoster( const Link& rLink )
    : m_aLink(rLink)
{
    m_nId = 0;
}

EventPoster::~EventPoster()
{
    if ( m_nId )
        GetpApp()->RemoveUserEvent( m_nId );
}

void EventPoster::Post( UserEvent* pEvent )

{
    m_nId = GetpApp()->PostUserEvent( ( LINK( this, EventPoster, DoEvent_Impl ) ), pEvent );
}

IMPL_LINK_INLINE_START( EventPoster, DoEvent_Impl, UserEvent*, pEvent )
{
    m_nId = 0;
    m_aLink.Call( pEvent );
    return 0;
}
IMPL_LINK_INLINE_END( EventPoster, DoEvent_Impl, UserEvent*, pEvent )

}
