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

#include <string.h>
#include <stdio.h>


#include <bf_so3/iface.hxx>
#include <sot/agg.hxx>
#include <bf_svtools/ownlist.hxx>

#include "bf_so3/soerr.hxx"

#include <tools/debug.hxx>

namespace binfilter {

/************** class SvObject ******************************************/
SV_IMPL_FACTORY(SvObjectFactory)
    {
    }
};
TYPEINIT1(SvObjectFactory,SvFactory);


SO2_IMPL_CLASS1_DLL(SvObject,SvObjectFactory,SotObject,
                   SvGlobalName( 0x7F7E0E60L, 0xC32D, 0x101B,
                            0x80, 0x4C, 0x04, 0x02, 0x1C, 0x00, 0x70, 0x02 ) )

/*************************************************************************
|*  SvObject::GetMemberInterface()
|*
|*	Beschreibung:
*************************************************************************/
::IUnknown * SvObject::GetMemberInterface( const SvGlobalName & )
{
    return NULL;
}

/*************************************************************************
|*  SvObject::TestMemberObjRef()
|*
|*	Beschreibung:
*************************************************************************/
void SvObject::TestMemberObjRef( BOOL /*bFree*/ )
{
}

/*************************************************************************
|*  SvObject::TestMemberObjRef()
|*
|*	Beschreibung:
*************************************************************************/

#ifdef TEST_INVARIANT
void SvObject::TestMemberInvariant( BOOL /*bPrint*/ )
{
#ifdef DBG_UTIL
    if( !Owner() && pClient )
    {
        ByteString aTest( "\t\tpClient == " );
        aTest += ByteString::CreateFromInt32( (ULONG)pClient );
        DBG_TRACE( aTest.GetBuffer() );
    }
    if( Owner() && pService )
    {
        ByteString aTest( "\t\tpService == " );
        aTest += ByteString::CreateFromInt32( (ULONG)pService );
        DBG_TRACE( aTest.GetBuffer() );
    }
#endif
}
#endif

/*************************************************************************
|*    SvObject::SvObject()
|*
|*    Beschreibung
*************************************************************************/
SvObject::SvObject()
    : pObj        ( NULL ) // union mit pClient, pService
    , nExtCount   ( 0 )
{
}

/*************************************************************************
|*
|*    SvObject::~SvObject()
|*
|*    Beschreibung
|*    Ersterstellung    MM 05.06.94
|*    Letzte Aenderung  MM 05.06.94
|*
*************************************************************************/
SvObject::~SvObject()
{
}

/*************************************************************************
|*
|*    SvObject::ReleaseRef()
|*
|*    Beschreibung
|*    Ersterstellung    MM 05.06.94
|*    Letzte Aenderung  MM 05.06.94
|*
*************************************************************************/
UINT32 SvObject::ReleaseRef()
{
#ifdef DBG_UTIL
    if( GetRefCount() == nExtCount )
    {
        ByteString aStr( "Internal RefCount underflow: Count == " );
        aStr += ByteString::CreateFromInt32( GetRefCount() );
        aStr += ", ExtCount == ";
        aStr += ByteString::CreateFromInt32( nExtCount );
        DBG_ERROR( aStr.GetBuffer() );
    }
#endif
    return SotObject::ReleaseRef();
}

//========================================================================
void SvObject::MakeUnknown()
/* [Beschreibung]

    Klassen, die immer extern sind, aber nicht sofort <SvObject::Init>
    rufen, m"ussen diese Methode "uberladen.
    Darin muss das externe Objekt erzeugt und mit Init eingesetzt werden.
    Im Konstruktor muss das Objekt als extern gekennzeichnet werden.

    [Beispiel]

    void MyClass::MyClass()
    {
        SetExtern();
    }

    void MyClass::MakeUnknown()
    {
        CreateBindCtx( 0, &pObjI );
        Init( pObjI );
    }
*/
{
}



#if defined (_INC_WINDOWS) || defined (_WINDOWS_)
RECT GetSysRect( const Rectangle & rRect )
{
    RECT aRect;
    aRect.top    = (int)rRect.Top();
    aRect.left   = (int)rRect.Left();
    if( rRect.Right() == RECT_EMPTY )
        aRect.right  = aRect.left;
    else
        aRect.right  = (int)(rRect.Right() +1);
    if( rRect.Bottom() == RECT_EMPTY )
        aRect.bottom = aRect.top;
    else
        aRect.bottom = (int)(rRect.Bottom() +1);
    return aRect;
}
Rectangle GetSvRect( const RECT & rRect )
{
    Rectangle aRect;
    aRect.Top()     = rRect.top;
    aRect.Left()    = rRect.left;
    if( rRect.right != rRect.left )
        aRect.Right()   = rRect.right -1;
    if( rRect.bottom != rRect.top )
        aRect.Bottom()  = rRect.bottom -1;
    return aRect;
}
#endif

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
