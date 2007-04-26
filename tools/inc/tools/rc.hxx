/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rc.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-26 09:46:50 $
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
#ifndef _TOOLS_RC_HXX
#define _TOOLS_RC_HXX

#ifndef INCLUDED_TOOLSDLLAPI_H
#include "tools/toolsdllapi.h"
#endif

#ifndef INCLUDED_I18NPOOL_LANG_H
#include <i18npool/lang.h>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _TOOLS_RESMGR_HXX
#include <tools/resmgr.hxx>
#endif

// ------------
// - Resource -
// ------------

class TOOLS_DLLPUBLIC Resource
{
    protected:
    ResMgr* m_pResMgr;

    // check availability of Resource
    BOOL                IsAvailableRes( const ResId& rId ) const
    { return m_pResMgr->IsAvailable( rId, this ); }

    // Load a Resource
    void                GetRes( const ResId& rResId );

    // check Resource state
    void                TestRes();

    // Get a pointer to the Resource's data
    void* GetClassRes()
    { return m_pResMgr->GetClass(); }

    // read a string from the resource
    static sal_uInt32   GetStringRes( UniString& rStr, const BYTE* pStr )
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
