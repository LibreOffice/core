/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <tools/resmgr.hxx>
#include <tools/resary.hxx>
#include <tools/rcid.h>

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

ResStringArray::~ResStringArray()
{
}

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
