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

    // read a string from the resource
    static sal_uInt32   GetStringRes( UniString& rStr, const sal_uInt8* pStr )
    { return ResMgr::GetString( rStr, pStr ); }

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
    UniString ReadStringRes()
    { return m_pResMgr->ReadString(); }
    // read a byte string from resource data and increment pointer
    rtl::OString ReadByteStringRes()
    { return m_pResMgr->ReadByteString(); }

    // Gibt die Resource frei (this-Zeiger fuer Fehlerueberpruefung)
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
