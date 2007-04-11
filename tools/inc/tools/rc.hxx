/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rc.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:16:06 $
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
    // Ist eine Resource verfuegbar
    BOOL                IsAvailableRes( const ResId& rId ) const;

    // Laedt eine Resource
    void                GetRes( const ResId& rResId );

    // Testet die Resource-Struktur
    void                TestRes();

    // Gibt einen Zeiger auf die Resource zurueck
    static void*        GetClassRes()
                            { return GetResManager()->GetClass(); }

    // Liefert einen String aus der Resource
    static sal_uInt32   GetStringRes( UniString& rStr, const BYTE* pStr )
                            { return ResMgr::GetString( rStr, pStr ); }

    // Erhoeht den Zeiger, der ueber GetClassRes geholt wird
    static void*        IncrementRes( sal_uInt32 nBytes )
                            { return GetResManager()->Increment( nBytes ); }

    // Gibt die Groesse eines Objektes in der Resource zurueck
    static sal_uInt32   GetObjSizeRes( RSHEADER_TYPE * pHT )
                            { return ResMgr::GetObjSize( pHT ); }

    // Gibt die verbleibende Groesse zurueck
    static sal_uInt32   GetRemainSizeRes()
                            { return GetResManager()->GetRemainSize(); }

    // Gibt einen long zurueck
    static INT32        GetLongRes( void * pLong )
                            { return ResMgr::GetLong( pLong ); }
    // Gibt einen long zurueck
    static INT16        GetShortRes( void * pShort )
                            { return ResMgr::GetShort( pShort ); }

    static INT32        ReadLongRes()
                            { return GetResManager()->ReadLong(); }
    static INT16        ReadShortRes()
                            { return GetResManager()->ReadShort(); }
    static UniString    ReadStringRes()
                            { return GetResManager()->ReadString(); }

    RSHEADER_TYPE*      CreateBlockRes( const ResId& rId );

    // Gibt die Resource frei (this-Zeiger fuer Fehlerueberpruefung)
    void                FreeResource()
                            { GetResManager()->PopContext( this ); }

                        // Resource-Konstruktoren
                        Resource() {}
                        Resource( const ResId& rResId );

public:
#ifdef DBG_UTIL
                        ~Resource() { TestRes(); }
#else
                        ~Resource() {}
#endif

    // neuen Resourcemanager setzen bzw. Manager zurueckgeben
    static void         SetResManager( ResMgr* paResMgr );
    static ResMgr*      GetResManager();
};

inline BOOL Resource::IsAvailableRes( const ResId& rId ) const
{
    return GetResManager()->IsAvailable( rId, this );
}

inline RSHEADER_TYPE* Resource::CreateBlockRes( const ResId& rId )
{
    return GetResManager()->CreateBlock( rId );
}

#endif // _SV_RC_HXX
