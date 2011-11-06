/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
