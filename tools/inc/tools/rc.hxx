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
#ifndef _TOOLS_RC_HXX
#define _TOOLS_RC_HXX

#include "tools/toolsdllapi.h"
#include <i18npool/lang.h>
#include <tools/string.hxx>
#include <tools/resmgr.hxx>

// ------------
// - Resource -
// ------------

class TOOLS_DLLPUBLIC Resource
{
    protected:
    ResMgr* m_pResMgr;

    // check availability of Resource
    sal_Bool                IsAvailableRes( const ResId& rId ) const
    { return m_pResMgr->IsAvailable( rId, this ); }

    // Load a Resource
    void                GetRes( const ResId& rResId );

    // check Resource state
    void                TestRes();

    // Get a pointer to the Resource's data
    void* GetClassRes()
    { return m_pResMgr->GetClass(); }

    // increase the memory pointer gotten by GetClassRes()
    void* IncrementRes( sal_uInt32 nBytes )
    { return m_pResMgr->Increment( nBytes ); }

    // return the memory size of a Resource data block
    static sal_uInt32   GetObjSizeRes( RSHEADER_TYPE * pHT )
    { return ResMgr::GetObjSize( pHT ); }

    // return the remaining size of this Resource's data
    sal_uInt32 GetRemainSizeRes()
    { return m_pResMgr->GetRemainSize(); }

    // get a 32bit value from Resource data
    static sal_Int32    GetLongRes( void * pLong )
    { return ResMgr::GetLong( pLong ); }
    // get a 16bit value from Resource data
    static sal_Int16    GetShortRes( void * pShort )
    { return ResMgr::GetShort( pShort ); }

    // read a 32bit value from resource data and increment pointer
    sal_Int32 ReadLongRes()
    { return m_pResMgr->ReadLong(); }
    // read a 16bit value from resource data and increment pointer
    sal_Int16 ReadShortRes()
    { return m_pResMgr->ReadShort(); }
    // read a string from resource data and increment pointer
    rtl::OUString ReadStringRes()
    { return m_pResMgr->ReadString(); }
    // read a byte string from resource data and increment pointer
    rtl::OString ReadByteStringRes()
    { return m_pResMgr->ReadByteString(); }

    // free the resource from m_pResMgr's stack (pass this ptr for validation)
    void FreeResource()
    { m_pResMgr->PopContext( this ); }

    // constructors
    Resource() : m_pResMgr( NULL ) {}
    Resource( const ResId& rResId );

    public:
    #ifdef DBG_UTIL
    ~Resource() { TestRes(); }
    #else
    ~Resource() {}
    #endif
};

#endif // _SV_RC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
