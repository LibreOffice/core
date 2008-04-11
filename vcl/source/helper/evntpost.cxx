/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: evntpost.cxx,v $
 * $Revision: 1.8 $
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
#include "precompiled_vcl.hxx"



#include <vcl/evntpost.hxx>
#include <vcl/svapp.hxx>

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
