/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_tools.hxx"

#define _TOOLS_RESARY_CXX
#include <tools/resary.hxx>
#include <tools/rcid.h>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
