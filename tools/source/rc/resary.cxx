/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: resary.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-26 09:48:09 $
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
#include "precompiled_tools.hxx"

#define _TOOLS_RESARY_CXX

#ifndef _TOOLS_RESARY_HXX
#include <resary.hxx>
#endif
#ifndef _TOOLS_RCID_H
#include <rcid.h>
#endif

// =======================================================================

ResStringArray::ResStringArray( const ResId& rResId )
{
    rResId.SetRT( RSC_STRINGARRAY );
    ResMgr* pMgr = rResId.GetResMgr();
    if( pMgr && pMgr->GetResource( rResId ) )
    {
        pMgr->GetClass();
        pMgr->Increment( sizeof( RSHEADER_TYPE ) );
        const sal_uInt32 nItems = pMgr->ReadLong();
        if ( nItems )
        {
            m_aStrings.reserve( nItems );
            for ( sal_uInt32 i = 0; i < nItems; i++ )
            {
                // load string
                m_aStrings.push_back( ImplResStringItem( pMgr->ReadString() ) );

                // load value
                m_aStrings[i].m_nValue = pMgr->ReadLong();
            }
        }
    }
}

// -----------------------------------------------------------------------

ResStringArray::~ResStringArray()
{
}

// -----------------------------------------------------------------------

sal_uInt32 ResStringArray::FindIndex( long nValue ) const
{
    const sal_uInt32 nItems = m_aStrings.size();
    for ( sal_uInt32 i = 0; i < nItems; i++ )
    {
        if ( m_aStrings[i].m_nValue == nValue )
            return i;
    }
    return RESARRAY_INDEX_NOTFOUND;
}
