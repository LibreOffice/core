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

#include <tools/resary.hxx>
#include <tools/resmgr.hxx>
#include <tools/rcid.h>

#include <vector>

namespace {

struct ImplResStringItem
{
    OUString m_aStr;
    sal_IntPtr m_nValue;

    ImplResStringItem( const OUString& rStr, long nValue = 0 ) :
        m_aStr(rStr),
        m_nValue(nValue)
    { }
};

}

struct ResStringArray::Impl
{
    std::vector<ImplResStringItem> m_aStrings;
};

ResStringArray::ResStringArray( const ResId& rResId ) :
    mpImpl(new Impl)
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
            mpImpl->m_aStrings.reserve( nItems );
            for ( sal_uInt32 i = 0; i < nItems; i++ )
            {
                // load string
                mpImpl->m_aStrings.push_back(ImplResStringItem(pMgr->ReadString()));

                // load value
                mpImpl->m_aStrings[i].m_nValue = pMgr->ReadLong();
            }
        }
    }
}

ResStringArray::~ResStringArray()
{
}

OUString ResStringArray::GetString( sal_uInt32 nIndex ) const
{
    return (nIndex < mpImpl->m_aStrings.size()) ? mpImpl->m_aStrings[nIndex].m_aStr : OUString();
}

sal_IntPtr ResStringArray::GetValue( sal_uInt32 nIndex ) const
{
    return (nIndex < mpImpl->m_aStrings.size()) ? mpImpl->m_aStrings[nIndex].m_nValue : -1;
}

sal_uInt32 ResStringArray::Count() const
{
    return sal_uInt32(mpImpl->m_aStrings.size());
}

sal_uInt32 ResStringArray::FindIndex( sal_IntPtr nValue ) const
{
    const sal_uInt32 nItems = mpImpl->m_aStrings.size();
    for ( sal_uInt32 i = 0; i < nItems; i++ )
    {
        if (mpImpl->m_aStrings[i].m_nValue == nValue)
            return i;
    }
    return RESARRAY_INDEX_NOTFOUND;
}

sal_uInt32 ResStringArray::AddItem( const OUString& rString, sal_IntPtr nValue )
{
    mpImpl->m_aStrings.push_back(ImplResStringItem(rString, nValue));
    return mpImpl->m_aStrings.size();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
