/*************************************************************************
 *
 *  $RCSfile: excrecds.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: dr $ $Date: 2001-03-19 13:23:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------

#include "scitems.hxx"
#include <svx/editdata.hxx>
#include <svx/editeng.hxx>
#include <svx/editobj.hxx>
#include <svx/editstat.hxx>

#define ITEMID_FIELD EE_FEATURE_FIELD
#include <svx/flditem.hxx>
#undef ITEMID_FIELD

#include <svx/algitem.hxx>
#include <svx/boxitem.hxx>
#include <svx/brshitem.hxx>
#include <svx/pageitem.hxx>
#include <svx/paperinf.hxx>
#include <svx/sizeitem.hxx>
#include <svtools/intitem.hxx>
#include <svtools/zforlist.hxx>
#include <svtools/zformat.hxx>

#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>

#include <string.h>

#include "global.hxx"
#include "docpool.hxx"
#include "patattr.hxx"
#include "cell.hxx"
#include "document.hxx"
#include "scextopt.hxx"
#include "patattr.hxx"
#include "attrib.hxx"
#include "progress.hxx"
#include "dociter.hxx"
#include "rangenam.hxx"
#include "dbcolect.hxx"
#include "stlsheet.hxx"
#include "stlpool.hxx"
#include "editutil.hxx"

#include "excrecds.hxx"
#include "root.hxx"
#include "fontbuff.hxx"
#include "excupn.hxx"

#ifndef _SC_XCLEXPSTREAM_HXX
#include "XclExpStream.hxx"
#endif

#include "xcl97rec.hxx"




//---------------------------------------------------- class ExcETabNumBuffer -

ExcETabNumBuffer::ExcETabNumBuffer( ScDocument& rDoc ) :
    bEnableLog( FALSE ),
    nCodeCnt( 0 )
{
    nScCnt = rDoc.GetTableCount();
    pBuffer = nScCnt ? new UINT32[ nScCnt ] : NULL;

    for( UINT16 nTab = 0; nTab < nScCnt; nTab++ )
    {
        pBuffer[ nTab ] = 0;

        // ignored tables (skipped by export, invalid Excel table number)
        if( rDoc.IsScenario( nTab ) )
            pBuffer[ nTab ] = EXC_TABBUF_FLAGIGNORE;

        // external tables (skipped, with valid Excel table number for ref's)
        else if( rDoc.GetLinkMode( nTab ) == SC_LINK_VALUE )
            pBuffer[ nTab ] = EXC_TABBUF_FLAGEXT;
    }
    ApplyBuffer();

    if( rDoc.GetExtDocOptions() )
    {
        CodenameList* pCList = rDoc.GetExtDocOptions()->GetCodenames();
        if( pCList )
            nCodeCnt = (UINT16) Min( pCList->Count(), (ULONG) 0xFFFF );
    }


}

ExcETabNumBuffer::~ExcETabNumBuffer()
{
    if( pBuffer )
        delete[] pBuffer;
}

void ExcETabNumBuffer::ApplyBuffer()
{
    UINT16 nIndex = 0;
    UINT16 nTab;
    nExcCnt = nExtCnt = 0;

    // regular tables
    for( nTab = 0; nTab < nScCnt; nTab++ )
    {
        if( IsExportTable( nTab ) )
        {
            pBuffer[ nTab ] |= nIndex;
             nIndex++;
            nExcCnt++;
        }
        else
            pBuffer[ nTab ] |= EXC_TABBUF_INVALID;
    }

    // external tables
    for( nTab = 0; nTab < nScCnt; nTab++ )
        if( IsExternal( nTab ) )
        {
            pBuffer[ nTab ] &= EXC_TABBUF_MASKFLAGS;
            pBuffer[ nTab ] |= nIndex;
             nIndex++;
            nExtCnt++;
        }
}

BOOL ExcETabNumBuffer::IsExternal( UINT16 nScTab ) const
{
    return (nScTab < nScCnt) ? TRUEBOOL( pBuffer[ nScTab ] & EXC_TABBUF_FLAGEXT ) : FALSE;
}

BOOL ExcETabNumBuffer::IsExportTable( UINT16 nScTab ) const
{
    DBG_ASSERT( nScTab < nScCnt, "ExcETabNumBuffer::IsExportTable() - out of range!" );
    return (pBuffer[ nScTab ] & EXC_TABBUF_MASKFLAGS) == 0;
}

UINT16 ExcETabNumBuffer::GetExcTable( UINT16 nScTab ) const
{
    return (nScTab < nScCnt) ? (UINT16)(pBuffer[ nScTab ] & EXC_TABBUF_MASKTAB) : EXC_TABBUF_INVALID;
}

void ExcETabNumBuffer::AppendTabRef( UINT16 nExcFirst, UINT16 nExcLast )
{
    if( bEnableLog )
    {
        Append( nExcFirst );
        Append( nExcLast );
    }
}



//------------------------------------------------------------- class ExcCell -

UINT32          ExcCell::nCellCount = 0UL;
ScProgress*     ExcCell::pPrgrsBar = NULL;
#ifdef DBG_UTIL
INT32           ExcCell::_nRefCount = 0L;
#endif


//--------------------------------------------------------- class ExcDummy_00 -
const BYTE      ExcDummy_00::pMyData[] = {
    0xe1, 0x00, 0x00, 0x00,                                 // INTERFACEHDR
    0xc1, 0x00, 0x02, 0x00, 0x00, 0x00,                     // MMS
    0xbf, 0x00, 0x00, 0x00,                                 // TOOLBARHDR
    0xc0, 0x00, 0x00, 0x00,                                 // TOOLBAREND
    0xe2, 0x00, 0x00, 0x00,                                 // INTERFACEEND
    0x5c, 0x00, 0x20, 0x00, 0x04, 0x4d, 0x72, 0x20, 0x58,   // WRITEACCESS
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x42, 0x00, 0x02, 0x00, 0xe4, 0x04,                     // CODEPAGE
    0x9c, 0x00, 0x02, 0x00, 0x0e, 0x00                      // FNGROUPCOUNT
};
const ULONG ExcDummy_00::nMyLen = sizeof( ExcDummy_00::pMyData );
//  ( UINT16 ) 0x00 + 0x02 + 0x00 + 0x00 + 0x00 + 0x20 +
//  0x02 + 0x02 + 8 * 4;

//-------------------------------------------------------- class ExcDummy_04x -
const BYTE      ExcDummy_040::pMyData[] = {
    0x19, 0x00, 0x02, 0x00, 0x00, 0x00,                     // WINDOWPROTECT
    0x12, 0x00, 0x02, 0x00, 0x00, 0x00,                     // PROTECT
    0x13, 0x00, 0x02, 0x00, 0x00, 0x00,                     // PASSWORD
    0x3d, 0x00, 0x12, 0x00, 0xe0, 0x01, 0x5a, 0x00, 0xcf,   // WINDOW1
    0x3f, 0x4e, 0x2a, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x58, 0x02,
    0x40, 0x00, 0x02, 0x00, 0x00, 0x00,                     // BACKUP
    0x8d, 0x00, 0x02, 0x00, 0x00, 0x00,                     // HIDEOBJ
};
const ULONG ExcDummy_040::nMyLen = sizeof( ExcDummy_040::pMyData );
//  ( UINT16 ) 0x02 + 0x02 + 0x02 + 0x12 + 0x02 + 0x02 + 6 * 4;

const BYTE      ExcDummy_041::pMyData[] = {
    0x0e, 0x00, 0x02, 0x00, 0x01, 0x00,                     // PRECISION
    0xda, 0x00, 0x02, 0x00, 0x00, 0x00                      // BOOKBOOL
};
const ULONG ExcDummy_041::nMyLen = sizeof( ExcDummy_041::pMyData );
//  ( UINT16 ) 0x02 + 0x02 + 2 * 4;

//--------------------------------------------------------- class ExcDummy_01-
// Default Font Records
const BYTE      ExcDummy_01::pMyData[] = {   //////////
    0x31, 0x00, 0x14, 0x00, 0xc8, 0x00, 0x00, 0x00, 0xff, 0x7f, 0x90, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xa5, 0x05, 0x41, 0x72, 0x69, 0x61, 0x6c,
    //                              'alter Wert 0xa5, hier Reserved
    0x31, 0x00, 0x14, 0x00, 0xc8, 0x00, 0x01, 0x00, 0xff, 0x7f, 0xbc, 0x02,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xa5, 0x05, 0x41, 0x72, 0x69, 0x61, 0x6c,

    0x31, 0x00, 0x14, 0x00, 0xc8, 0x00, 0x02, 0x00, 0xff, 0x7f, 0x90, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xa5, 0x05, 0x41, 0x72, 0x69, 0x61, 0x6c,

    0x31, 0x00, 0x14, 0x00, 0xc8, 0x00, 0x03, 0x00, 0xff, 0x7f, 0xbc, 0x02,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xa5, 0x05, 0x41, 0x72, 0x69, 0x61, 0x6c,

    0x31, 0x00, 0x14, 0x00, 0xc8, 0x00, 0x00, 0x00, 0xff, 0x7f, 0x90, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xa5, 0x05, 0x41, 0x72, 0x69, 0x61, 0x6c
};
const ULONG ExcDummy_01::nMyLen = sizeof( ExcDummy_01::pMyData );   //( ( UINT16 ) 0x14 + 4 ) * 5;


//--------------------------------------------------------- class ExcDummy_Fm -
const BYTE      ExcDummy_Fm::pMyData[] = {
    0x1e, 0x04, 0x1c, 0x00, 0x05, 0x00, 0x19, 0x23, 0x2c, 0x23, 0x23, 0x30,
    0x5c, 0x20, 0x22, 0x44, 0x4d, 0x22, 0x3b, 0x5c, 0x2d, 0x23, 0x2c, 0x23,
    0x23, 0x30, 0x5c, 0x20, 0x22, 0x44, 0x4d, 0x22,
    0x1e, 0x04, 0x21, 0x00, 0x06, 0x00, 0x1e, 0x23, 0x2c, 0x23, 0x23, 0x30,
    0x5c, 0x20, 0x22, 0x44, 0x4d, 0x22, 0x3b, 0x5b, 0x52, 0x65, 0x64, 0x5d,
    0x5c, 0x2d, 0x23, 0x2c, 0x23, 0x23, 0x30, 0x5c, 0x20, 0x22, 0x44, 0x4d,
    0x22,
    0x1e, 0x04, 0x22, 0x00, 0x07, 0x00, 0x1f, 0x23, 0x2c, 0x23, 0x23, 0x30,
    0x2e, 0x30, 0x30, 0x5c, 0x20, 0x22, 0x44, 0x4d, 0x22, 0x3b, 0x5c, 0x2d,
    0x23, 0x2c, 0x23, 0x23, 0x30, 0x2e, 0x30, 0x30, 0x5c, 0x20, 0x22, 0x44,
    0x4d, 0x22,
    0x1e, 0x04, 0x27, 0x00, 0x08, 0x00, 0x24, 0x23, 0x2c, 0x23, 0x23, 0x30,
    0x2e, 0x30, 0x30, 0x5c, 0x20, 0x22, 0x44, 0x4d, 0x22, 0x3b, 0x5b, 0x52,
    0x65, 0x64, 0x5d, 0x5c, 0x2d, 0x23, 0x2c, 0x23, 0x23, 0x30, 0x2e, 0x30,
    0x30, 0x5c, 0x20, 0x22, 0x44, 0x4d, 0x22,
    0x1e, 0x04, 0x3c, 0x00, 0x2a, 0x00, 0x39, 0x5f, 0x2d, 0x2a, 0x20, 0x23,
    0x2c, 0x23, 0x23, 0x30, 0x5c, 0x20, 0x22, 0x44, 0x4d, 0x22, 0x5f, 0x2d,
    0x3b, 0x5c, 0x2d, 0x2a, 0x20, 0x23, 0x2c, 0x23, 0x23, 0x30, 0x5c, 0x20,
    0x22, 0x44, 0x4d, 0x22, 0x5f, 0x2d, 0x3b, 0x5f, 0x2d, 0x2a, 0x20, 0x22,
    0x2d, 0x22, 0x5c, 0x20, 0x22, 0x44, 0x4d, 0x22, 0x5f, 0x2d, 0x3b, 0x5f,
    0x2d, 0x40, 0x5f, 0x2d,
    0x1e, 0x04, 0x3c, 0x00, 0x29, 0x00, 0x39, 0x5f, 0x2d, 0x2a, 0x20, 0x23,
    0x2c, 0x23, 0x23, 0x30, 0x5c, 0x20, 0x5f, 0x44, 0x5f, 0x4d, 0x5f, 0x2d,
    0x3b, 0x5c, 0x2d, 0x2a, 0x20, 0x23, 0x2c, 0x23, 0x23, 0x30, 0x5c, 0x20,
    0x5f, 0x44, 0x5f, 0x4d, 0x5f, 0x2d, 0x3b, 0x5f, 0x2d, 0x2a, 0x20, 0x22,
    0x2d, 0x22, 0x5c, 0x20, 0x5f, 0x44, 0x5f, 0x4d, 0x5f, 0x2d, 0x3b, 0x5f,
    0x2d, 0x40, 0x5f, 0x2d,
    0x1e, 0x04, 0x44, 0x00, 0x2c, 0x00, 0x41, 0x5f, 0x2d, 0x2a, 0x20, 0x23,
    0x2c, 0x23, 0x23, 0x30, 0x2e, 0x30, 0x30, 0x5c, 0x20, 0x22, 0x44, 0x4d,
    0x22, 0x5f, 0x2d, 0x3b, 0x5c, 0x2d, 0x2a, 0x20, 0x23, 0x2c, 0x23, 0x23,
    0x30, 0x2e, 0x30, 0x30, 0x5c, 0x20, 0x22, 0x44, 0x4d, 0x22, 0x5f, 0x2d,
    0x3b, 0x5f, 0x2d, 0x2a, 0x20, 0x22, 0x2d, 0x22, 0x3f, 0x3f, 0x5c, 0x20,
    0x22, 0x44, 0x4d, 0x22, 0x5f, 0x2d, 0x3b, 0x5f, 0x2d, 0x40, 0x5f, 0x2d,

    0x1e, 0x04, 0x44, 0x00, 0x2b, 0x00, 0x41, 0x5f, 0x2d, 0x2a, 0x20, 0x23,
    0x2c, 0x23, 0x23, 0x30, 0x2e, 0x30, 0x30, 0x5c, 0x20, 0x5f, 0x44, 0x5f,
    0x4d, 0x5f, 0x2d, 0x3b, 0x5c, 0x2d, 0x2a, 0x20, 0x23, 0x2c, 0x23, 0x23,
    0x30, 0x2e, 0x30, 0x30, 0x5c, 0x20, 0x5f, 0x44, 0x5f, 0x4d, 0x5f, 0x2d,
    0x3b, 0x5f, 0x2d, 0x2a, 0x20, 0x22, 0x2d, 0x22, 0x3f, 0x3f, 0x5c, 0x20,
    0x5f, 0x44, 0x5f, 0x4d, 0x5f, 0x2d, 0x3b, 0x5f, 0x2d, 0x40, 0x5f, 0x2d
};
const ULONG ExcDummy_Fm::nMyLen = sizeof( ExcDummy_Fm::pMyData );
//  ( UINT16 ) 0x1c + 0x21 + 0x22 + 0x27 + 0x3c + 0x3c + 0x44 + 0x44 + 4 * 8;


//--------------------------------------------------------- class ExcDummy_XF -
const BYTE      ExcDummy_XF::pMyData[] = {
    0xe0, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf5, 0xff, 0x20, 0x00,
    0xc0, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xe0, 0x00, 0x10, 0x00, 0x01, 0x00, 0x00, 0x00, 0xf5, 0xff, 0x20, 0xf4,
    0xc0, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xe0, 0x00, 0x10, 0x00, 0x01, 0x00, 0x00, 0x00, 0xf5, 0xff, 0x20, 0xf4,
    0xc0, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xe0, 0x00, 0x10, 0x00, 0x02, 0x00, 0x00, 0x00, 0xf5, 0xff, 0x20, 0xf4,
    0xc0, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xe0, 0x00, 0x10, 0x00, 0x02, 0x00, 0x00, 0x00, 0xf5, 0xff, 0x20, 0xf4,
    0xc0, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xe0, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf5, 0xff, 0x20, 0xf4,
    0xc0, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xe0, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf5, 0xff, 0x20, 0xf4,
    0xc0, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xe0, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf5, 0xff, 0x20, 0xf4,
    0xc0, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xe0, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf5, 0xff, 0x20, 0xf4,
    0xc0, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xe0, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf5, 0xff, 0x20, 0xf4,
    0xc0, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xe0, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf5, 0xff, 0x20, 0xf4,
    0xc0, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xe0, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf5, 0xff, 0x20, 0xf4,
    0xc0, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xe0, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf5, 0xff, 0x20, 0xf4,
    0xc0, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xe0, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf5, 0xff, 0x20, 0xf4,
    0xc0, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xe0, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf5, 0xff, 0x20, 0xf4,
    0xc0, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xe0, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x20, 0x00,
    0xc0, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xe0, 0x00, 0x10, 0x00, 0x05, 0x00, 0x2b, 0x00, 0xf5, 0xff, 0x20, 0xf8,
    0xc0, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xe0, 0x00, 0x10, 0x00, 0x05, 0x00, 0x29, 0x00, 0xf5, 0xff, 0x20, 0xf8,
    0xc0, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xe0, 0x00, 0x10, 0x00, 0x05, 0x00, 0x09, 0x00, 0xf5, 0xff, 0x20, 0xf8,
    0xc0, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xe0, 0x00, 0x10, 0x00, 0x05, 0x00, 0x2c, 0x00, 0xf5, 0xff, 0x20, 0xf8,
    0xc0, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xe0, 0x00, 0x10, 0x00, 0x05, 0x00, 0x2a, 0x00, 0xf5, 0xff, 0x20, 0xf8,
    0xc0, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
const ULONG ExcDummy_XF::nMyLen = sizeof( ExcDummy_XF::pMyData );
//      ( UINT16 ) ( 0x10 + 4 ) * 21;

//------------------------------------------------------ class ExcDummy_Style -
const BYTE      ExcDummy_Style::pMyData[] = {
    0x93, 0x02, 0x04, 0x00, 0x10, 0x80, 0x03, 0xff,
    0x93, 0x02, 0x04, 0x00, 0x11, 0x80, 0x06, 0xff,
    0x93, 0x02, 0x04, 0x00, 0x12, 0x80, 0x05, 0xff,
    0x93, 0x02, 0x04, 0x00, 0x00, 0x80, 0x00, 0xff,
    0x93, 0x02, 0x04, 0x00, 0x13, 0x80, 0x04, 0xff,
    0x93, 0x02, 0x04, 0x00, 0x14, 0x80, 0x07, 0xff
};
const ULONG ExcDummy_Style::nMyLen = sizeof( ExcDummy_Style::pMyData );
//      ( UINT16 ) ( 0x04 + 4 ) * 6;

//--------------------------------------------------------- class ExcDummy_02 -
const BYTE      ExcDummy_02::pMyData[] = {
    0x0d, 0x00, 0x02, 0x00, 0x01, 0x00,                     // CALCMODE
    0x0c, 0x00, 0x02, 0x00, 0x64, 0x00,                     // CALCCOUNT
    0x0f, 0x00, 0x02, 0x00, 0x01, 0x00,                     // REFMODE
    0x11, 0x00, 0x02, 0x00, 0x00, 0x00,                     // ITERATION
    0x10, 0x00, 0x08, 0x00, 0xfc, 0xa9, 0xf1, 0xd2, 0x4d,   // DELTA
    0x62, 0x50, 0x3f,
    0x5f, 0x00, 0x02, 0x00, 0x01, 0x00,                     // SAVERECALC
//  0x2a, 0x00, 0x02, 0x00, 0x00, 0x00,                     // PRINTHEADERS
//  0x2b, 0x00, 0x02, 0x00, 0x01, 0x00,                     // PRINTGRIDLINES
    0x82, 0x00, 0x02, 0x00, 0x01, 0x00,                     // GRIDSET
//  0x80, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // GUTS
//  0x00, 0x00, 0x00,
    0x25, 0x02, 0x04, 0x00, 0x00, 0x00, 0xff, 0x00,         // DEFAULTROWHEIGHT
    0x8c, 0x00, 0x04, 0x00, 0x31, 0x00, 0x31, 0x00,         // COUNTRY
    0x81, 0x00, 0x02, 0x00, 0xc1, 0x04,                     // WSBOOL
//  0x14, 0x00, 0x03, 0x00, 0x02, 0x26, 0x41,               // HEADER
//  0x15, 0x00, 0x09, 0x00, 0x08, 0x53, 0x65, 0x69, 0x74,   // FOOTER
//  0x65, 0x20, 0x26, 0x50,
//  0x83, 0x00, 0x02, 0x00, 0x00, 0x00,                     // HCENTER
//  0x84, 0x00, 0x02, 0x00, 0x00, 0x00,                     // VCENTER
//  0xa1, 0x00, 0x22, 0x00, 0x00, 0x00, 0x31, 0x00, 0x01,   // SETUP
//  0x00, 0x01, 0x00, 0x01, 0x00, 0x44, 0x00, 0x72, 0x75,
//  0x6e, 0x67, 0xc1, 0x8d, 0xb0, 0x82, 0xc1, 0x60, 0xe0,
//  0x3f, 0xc1, 0x8d, 0xb0, 0x82, 0xc1, 0x60, 0xe0, 0x3f,
//  0xf0, 0x00//,
//  0x55, 0x00, 0x02, 0x00, 0x0a, 0x00                      // DEFCOLWIDTH
};
const ULONG ExcDummy_02::nMyLen = sizeof( ExcDummy_02::pMyData );
//  ( UINT16 ) 0x02 + 0x02 + 0x02 + 0x02 +
//  0x08 + 0x02 + 0x02 + 0x02 + 0x02 + 0x08 + 0x04 + 0x04 + 0x02 + /*0x03 +*/
//  /*0x09 +*/ 0x02 + 0x02 + /*0x22 +*/ /*0x02 + */15 * 4;


//--------------------------------------------------------- class ExcDummy_03 -
const BYTE      ExcDummy_03::pMyData[] = {
    0x1d, 0x00, 0x0f, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,   // SELECTION
    0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00
};
const ULONG ExcDummy_03::nMyLen = sizeof( ExcDummy_03::pMyData );


ExcPalette2*    ExcFont::pPalette2 = NULL;
#ifdef DBG_UTIL
UINT16          ExcFont::nObjCnt = 0;
#endif

ExcPalette2*    ExcXf::pPalette2 = NULL;
#ifdef DBG_UTIL
UINT16          ExcXf::nObjCnt = 0;
#endif

SvNumberFormatter*  ExcFormat::pFormatter = NULL;
UINT32          ExcFormat::nObjCnt = 0;




//----------------------------------------------------------- class ExcRecord -

ExcRecord::~ExcRecord()
{
}


void ExcRecord::SaveCont( XclExpStream& rStrm )
{
}


void ExcRecord::Save( XclExpStream& rStrm )
{
    rStrm.StartRecord( GetNum(), GetLen() );
    SaveCont( rStrm );
    rStrm.EndRecord();
}



//--------------------------------------------------------- class ExcEmptyRec -

void ExcEmptyRec::Save( XclExpStream& rStrm )
{
}


UINT16 ExcEmptyRec::GetNum() const
{
    return 0;
}


ULONG ExcEmptyRec::GetLen() const
{
    return 0;
}



//------------------------------------------------------- class ExcRecordList -

ExcRecordList::~ExcRecordList()
{
    for( ExcRecord* pRec = First(); pRec; pRec = Next() )
        delete pRec;
}


void ExcRecordList::Save( XclExpStream& rStrm )
{
    for( ExcRecord* pRec = First(); pRec; pRec = Next() )
        pRec->Save( rStrm );
}



//--------------------------------------------------------- class ExcDummyRec -

void ExcDummyRec::Save( XclExpStream& rStrm )
{
    rStrm.Write( GetData(), GetLen() );     // raw write mode
}


UINT16 ExcDummyRec::GetNum( void ) const
{
    return 0x0000;
}



//------------------------------------------------------- class ExcBoolRecord -

ExcBoolRecord::ExcBoolRecord( SfxItemSet* pItemSet, USHORT nWhich, BOOL bDefault )
{
    bVal = pItemSet? ( ( const SfxBoolItem& ) pItemSet->Get( nWhich ) ).GetValue() : bDefault;
}


void ExcBoolRecord::SaveCont( XclExpStream& rStrm )
{
    rStrm << (UINT16)(bVal ? 0x0001 : 0x0000);
}


ULONG ExcBoolRecord::GetLen( void ) const
{
    return 2;
}




//--------------------------------------------------------- class ExcBof_Base -

ExcBof_Base::ExcBof_Base() :
    nRupBuild( 0x096C ),    // copied from Excel
    nRupYear( 0x07C9 )      // copied from Excel
{
}



//-------------------------------------------------------------- class ExcBof -

ExcBof::ExcBof( void )
{
    nDocType = 0x0010;
    nVers = 0x0500;
}


void ExcBof::SaveCont( XclExpStream& rStrm )
{
    rStrm << nVers << nDocType << nRupBuild << nRupYear;
}


UINT16 ExcBof::GetNum( void ) const
{
    return 0x0809;
}


ULONG ExcBof::GetLen( void ) const
{
    return 8;
}



//------------------------------------------------------------- class ExcBofW -

ExcBofW::ExcBofW( void )
{
    nDocType = 0x0005;
    nVers = 0x0500;
}


void ExcBofW::SaveCont( XclExpStream& rStrm )
{
    rStrm << nVers << nDocType << nRupBuild << nRupYear;
}



UINT16 ExcBofW::GetNum( void ) const
{
    return 0x0809;
}



ULONG ExcBofW::GetLen( void ) const
{
    return 8;
}



//-------------------------------------------------------------- class ExcEof -

UINT16 ExcEof::GetNum( void ) const
{
    return 0x000A;
}


ULONG ExcEof::GetLen( void ) const
{
    return 0;
}



//----------------------------------------------------- class ExcFngroupcount -

void ExcFngroupcount::SaveCont( XclExpStream& rStrm )
{
    rStrm << ( UINT16 ) 0x000E;     // copied from Excel
}


UINT16 ExcFngroupcount::GetNum( void ) const
{
    return 0x009C;
}


ULONG ExcFngroupcount::GetLen( void ) const
{
    return 2;
}



//--------------------------------------------------------- class ExcDummy_00 -

ULONG ExcDummy_00::GetLen( void ) const
{
    return nMyLen;
}


const BYTE* ExcDummy_00::GetData( void ) const
{
    return pMyData;
}



//-------------------------------------------------------- class ExcDummy_04x -

ULONG ExcDummy_040::GetLen( void ) const
{
    return nMyLen;
}


const BYTE* ExcDummy_040::GetData( void ) const
{
    return pMyData;
}




ULONG ExcDummy_041::GetLen( void ) const
{
    return nMyLen;
}


const BYTE* ExcDummy_041::GetData( void ) const
{
    return pMyData;
}



//------------------------------------------------------------- class Exc1904 -

Exc1904::Exc1904( ScDocument& rDoc )
{
    Date* pDate = rDoc.GetFormatTable()->GetNullDate();
    bVal = pDate ? (*pDate == Date( 1, 1, 1904 )) : FALSE;
}


UINT16 Exc1904::GetNum( void ) const
{
    return 0x0022;
}



//------------------------------------------------------------- class ExcFont -

ExcFont::ExcFont( RootData& rRootData ) :
        eBiff( rRootData.eDateiTyp ),
        eCharSet( *rRootData.pCharset )
{
#ifdef DBG_UTIL
    nObjCnt++;
#endif
    DBG_ASSERT( pPalette2, "*ExcFont::ExcFont() - Missing palette!" );

    nNameLen = nHeight = nAttr = nWeight = 0;
    nUnder = nCharset = 0;
    nFamily = EXC_FONTFAM_SYSTEM;
    nSign = 0;
    bIgnoreCol = TRUE;
    nColorSer = pPalette2->InsertIndex( EXC_FONTCOL_IGNORE );
}


ExcFont::ExcFont( Font* pFont, RootData& rRootData ) :
        eBiff( rRootData.eDateiTyp ),
        eCharSet( *rRootData.pCharset )
{
#ifdef DBG_UTIL
    nObjCnt++;
#endif
    DBG_ASSERT( pPalette2, "*ExcFont::ExcFont() - Missing palette!" );
    DBG_ASSERT( pFont, "*ExcFont::ExcFont() - Missing font!" );

    SetName( pFont->GetName() );

    // attributes
    nAttr = EXC_FONTATTR_NONE;
    SetItalic( pFont->GetItalic() != ITALIC_NONE );
    SetStrikeout( pFont->GetStrikeout() != STRIKEOUT_NONE );
    SetOutline( pFont->IsOutline() );
    SetShadow( pFont->IsShadow() );

    SetHeight( (UINT16) pFont->GetSize().Height() );
    SetColor( pPalette2->InsertColor( pFont->GetColor(), EXC_COLOR_CELLTEXT ) );
    SetWeight( pFont->GetWeight() );
    SetUnderline( pFont->GetUnderline() );
    SetFamily( pFont->GetFamily() );
    SetCharSet( pFont->GetCharSet() );

    // checksum for quick comparison
    nSign = 0;
    for( const sal_Unicode* pAkt = sName.GetBuffer(); *pAkt; pAkt++ )
        nSign += (UINT16) *pAkt;
    nSign += nWeight * 3;
    nSign += nCharset * 5;
    nSign += nNameLen * 7;
    nSign += nHeight * 11;
    nSign += nUnder * 13;
    nSign += nAttr * 17;
    nSign += nFamily * 19;
}


ExcFont::~ExcFont()
{
#ifdef DBG_UTIL
    DBG_ASSERT( nObjCnt, "ExcFont::ExcFont() - nObjCnt underflow!" );
    nObjCnt--;
#endif
}


void ExcFont::SetName( const String& rName )
{
    sName.Assign( rName );
    nNameLen = (UINT16) Min( sName.Len(), (xub_StrLen) 0xFFFF );
}


void ExcFont::SetColor( UINT32 nSerial )
{
    nColorSer = nSerial;
    bIgnoreCol = FALSE;
}


void ExcFont::SaveCont( XclExpStream& rStrm )
{
    rStrm   << nHeight                                  // font height
            << nAttr                                    // attributes
            << pPalette2->GetColorIndex( nColorSer )    // color index
            << nWeight                                  // boldeness - Biff5 only
            << (UINT16)0                                // super-/subscript (none)
            << nUnder                                   // underline
            << nFamily                                  // family
            << nCharset                                 // character set
            << (BYTE) 0xA5;                             // original from Excel

    // Name
    DBG_ASSERT( nNameLen < 256, "ExcFont::SaveCont() - sName too long!" );
    if ( eBiff < Biff8 )
        rStrm.WriteByteString( ByteString( sName, eCharSet ) );     // 8 bit length, max 255 chars
    else
        XclExpUniString( sName, 255, TRUE ).Write( rStrm, FALSE );  //! 8 bit length, always 16-bit-chars
}


BOOL ExcFont::operator==( const ExcFont &rRef ) const
{
    if( nSign != rRef.nSign )
        return FALSE;
    if( (nColorSer != rRef.nColorSer) && !bIgnoreCol && !rRef.bIgnoreCol )
        return FALSE;
    return
    (
        (nNameLen == rRef.nNameLen) && (nHeight == rRef.nHeight)    &&
        (nAttr == rRef.nAttr)       && (nWeight == rRef.nWeight)    &&
        (nUnder == rRef.nUnder)     && (nFamily == rRef.nFamily)    &&
        (nCharset == rRef.nCharset) && (sName == rRef.sName) );
}


void ExcFont::SetPalette( ExcPalette2& rPalette2 )
{
    pPalette2 = &rPalette2;
}


UINT16 ExcFont::GetWeight( const FontWeight eWeight )
{
    switch( eWeight )
    {
        case WEIGHT_DONTKNOW:   return EXC_FONTWGHT_DONTKNOW;   // DONTCARE
        case WEIGHT_THIN:       return EXC_FONTWGHT_THIN;
        case WEIGHT_ULTRALIGHT: return EXC_FONTWGHT_ULTRALIGHT;
        case WEIGHT_LIGHT:      return EXC_FONTWGHT_LIGHT;
        case WEIGHT_SEMILIGHT:  return EXC_FONTWGHT_SEMILIGHT;
        case WEIGHT_NORMAL:     return EXC_FONTWGHT_NORMAL;
        case WEIGHT_MEDIUM:     return EXC_FONTWGHT_MEDIUM;
        case WEIGHT_SEMIBOLD:   return EXC_FONTWGHT_SEMIBOLD;
        case WEIGHT_BOLD:       return EXC_FONTWGHT_BOLD;
        case WEIGHT_ULTRABOLD:  return EXC_FONTWGHT_ULTRABOLD;
        case WEIGHT_BLACK:      return EXC_FONTWGHT_BLACK;
    }
    return EXC_FONTWGHT_NORMAL;
}


UINT8 ExcFont::GetUnderline( const FontUnderline eUnder )
{
    switch( eUnder )
    {
        case UNDERLINE_SINGLE:  return EXC_FONTUL_SINGLE;
        case UNDERLINE_DOUBLE:  return EXC_FONTUL_DOUBLE;
        case UNDERLINE_DOTTED:  return EXC_FONTUL_DOTTED;
    }
    return EXC_FONTUL_NONE;
}


UINT8 ExcFont::GetFamily( const FontFamily eFamily )
{
    switch( eFamily )
    {
        case FAMILY_DONTKNOW:   return EXC_FONTFAM_DONTKNOW;
        case FAMILY_DECORATIVE: return EXC_FONTFAM_DECORATIVE;
        case FAMILY_MODERN:     return EXC_FONTFAM_MODERN;
        case FAMILY_ROMAN:      return EXC_FONTFAM_ROMAN;
        case FAMILY_SCRIPT:     return EXC_FONTFAM_SCRIPT;
        case FAMILY_SWISS:      return EXC_FONTFAM_SWISS;
        case FAMILY_SYSTEM:     return EXC_FONTFAM_SYSTEM;
    }
    DBG_ERROR( "ExcFont::GetFamily() - unknown family!" );
    return EXC_FONTFAM_DONTKNOW;
}


UINT8 ExcFont::GetCharSet( const rtl_TextEncoding eCharset )
{
    switch( eCharset )
    {
        case RTL_TEXTENCODING_DONTKNOW:     return EXC_FONTCSET_DONTKNOW;
        case RTL_TEXTENCODING_MS_1252:      return EXC_FONTCSET_MS_1252;
        case RTL_TEXTENCODING_APPLE_ROMAN:  return EXC_FONTCSET_APPLE_ROMAN;
        case RTL_TEXTENCODING_IBM_437:      return EXC_FONTCSET_IBM_437;
        case RTL_TEXTENCODING_IBM_850:      return EXC_FONTCSET_IBM_850;
        case RTL_TEXTENCODING_IBM_860:      return EXC_FONTCSET_IBM_860;
        case RTL_TEXTENCODING_IBM_861:      return EXC_FONTCSET_IBM_861;
        case RTL_TEXTENCODING_IBM_863:      return EXC_FONTCSET_IBM_863;
        case RTL_TEXTENCODING_IBM_865:      return EXC_FONTCSET_IBM_865;
        case (rtl_TextEncoding) 9:          return EXC_FONTCSET_SYSTEM;
        case RTL_TEXTENCODING_SYMBOL:       return EXC_FONTCSET_SYMBOL;
    }

    return EXC_FONTCSET_DONTKNOW;
}


UINT16 ExcFont::GetNum( void ) const
{
    return 0x0031;
}


ULONG ExcFont::GetLen( void ) const
{
    return 15 + ( eBiff < Biff8 ? nNameLen : 1 + 2 * nNameLen );
}



//--------------------------------------------------------- class ExcDummy_01 -

ULONG ExcDummy_01::GetLen( void ) const
{
    return nMyLen;
}


const BYTE* ExcDummy_01::GetData( void ) const
{
    return pMyData;
}



//--------------------------------------------------------- class ExcDummy_Fm -

ULONG ExcDummy_Fm::GetLen( void ) const
{
    return nMyLen;
}


const BYTE* ExcDummy_Fm::GetData( void ) const
{
    return pMyData;
}



//--------------------------------------------------------- class ExcDummy_XF -

ULONG ExcDummy_XF::GetLen( void ) const
{
    return nMyLen;
}


const BYTE* ExcDummy_XF::GetData( void ) const
{
    return pMyData;
}



//------------------------------------------------------ class ExcDummy_Style -

ULONG ExcDummy_Style::GetLen( void ) const
{
    return nMyLen;
}


const BYTE* ExcDummy_Style::GetData( void ) const
{
    return pMyData;
}



//------------------------------------------------------ class ExcBundlesheet -

ExcBundlesheetBase::ExcBundlesheetBase( RootData& rRootData, UINT16 nTab ) :
    nGrbit( rRootData.pDoc->IsVisible( nTab ) ? 0x0000 : 0x0001 ),
    nOwnPos( STREAM_SEEK_TO_END ),
    nStrPos( STREAM_SEEK_TO_END )
{
}


ExcBundlesheetBase::ExcBundlesheetBase() :
    nGrbit( 0x0000 ),
    nOwnPos( STREAM_SEEK_TO_END ),
    nStrPos( STREAM_SEEK_TO_END )
{
}


void ExcBundlesheetBase::UpdateStreamPos( XclExpStream& rStrm )
{
    rStrm.SetStreamPos( nOwnPos );
    rStrm << nStrPos;
}


UINT16 ExcBundlesheetBase::GetNum( void ) const
{
    return 0x0085;
}




ExcBundlesheet::ExcBundlesheet( RootData& rRootData, UINT16 nTab ) :
    ExcBundlesheetBase( rRootData, nTab )
{
    String sTabName;
    rRootData.pDoc->GetName( nTab, sTabName );
    DBG_ASSERT( sTabName.Len() < 256, "ExcBundlesheet::ExcBundlesheet - table name too long" );
    aName = ByteString( sTabName, *rRootData.pCharset );
}


void ExcBundlesheet::SaveCont( XclExpStream& rStrm )
{
    nOwnPos = rStrm.GetStreamPos();
    rStrm   << (UINT32) 0x00000000              // dummy (stream position of the sheet)
            << nGrbit;
    rStrm.WriteByteString( aName );             // 8 bit length, max 255 chars
}


ULONG ExcBundlesheet::GetLen() const
{
    return 7 + Min( aName.Len(), (xub_StrLen) 255 );
}



//--------------------------------------------------------- class ExcDummy_02 -

ULONG ExcDummy_02::GetLen( void ) const
{
    return nMyLen;
}


const BYTE* ExcDummy_02::GetData( void ) const
{
    return pMyData;
}



//------------------------------------------------------------- class ExcNote -

ExcNote::ExcNote( const ScAddress aNewPos, const String& rText, RootData& rExcRoot ) :
    aPos( aNewPos )
{
    pText = new ByteString( rText, *rExcRoot.pCharset );
    nTextLen = (UINT16) Min( pText->Len(), (xub_StrLen) 0xFFFF );
}


ExcNote::~ExcNote()
{
    delete pText;
}


void ExcNote::Save( XclExpStream& rStrm )
{
    const sal_Char* pBuffer = pText->GetBuffer();
    UINT16          nLeft = nTextLen;
    BOOL            bFirstRun = TRUE;

    do
    {
        UINT16 nWriteLen = Min( nLeft, (UINT16) EXC_NOTE5_MAXTEXT );

        rStrm.StartRecord( 0x001C, 6 + nWriteLen );

        // first record: row, col, length of complete text
        // next records: -1, 0, length of current text segment
        if( bFirstRun )
            rStrm << (UINT16) aPos.Row() << (UINT16) aPos.Col() << nTextLen;
        else
            rStrm << (UINT16) 0xFFFF << (UINT16) 0 << nWriteLen;

        rStrm.Write( pBuffer, nWriteLen );
        pBuffer += nWriteLen;
        nLeft -= nWriteLen;
        bFirstRun = FALSE;

        rStrm.EndRecord();
    }
    while( nLeft );
}



//------------------------------------------------------------- class ExcCell -

ExcCell::ExcCell(
        const ScAddress rPos,
        const ScPatternAttr* pAttr,
        RootData& rRootData,
        const ULONG nAltNumForm,
        BOOL bForceAltNumForm ) :
    aPos( rPos )
{
    if( pPrgrsBar )
        pPrgrsBar->SetState( GetCellCount() );
    IncCellCount();

    DBG_ASSERT( rRootData.pXFRecs, "ExcCell::ExcCell - missing XF record list" );
    DBG_ASSERT( rRootData.pCellMerging, "ExcCell::ExcCell - missing cell merging list" );

    if( !rRootData.pCellMerging->FindMergeBaseXF( aPos, nXF ) )
        nXF = rRootData.pXFRecs->Find( pAttr, FALSE, nAltNumForm, bForceAltNumForm );

#ifdef DBG_UTIL
    _nRefCount++;
#endif
}


ExcCell::~ExcCell()
{
#ifdef DBG_UTIL
    _nRefCount--;
    DBG_ASSERT( _nRefCount >= 0, "*ExcCell::~ExcCell(): Das war mindestens einer zuviel!" );
#endif
}


UINT16 ExcCell::GetXF() const
{
    return nXF;
}


void ExcCell::SaveCont( XclExpStream& rStrm )
{
    if( pPrgrsBar )
        pPrgrsBar->SetState( GetCellCount() );
    IncCellCount();

    rStrm << ( UINT16 ) aPos.Row() << ( UINT16 ) aPos.Col() << nXF;
    SaveDiff( rStrm );
}


void ExcCell::SaveDiff( XclExpStream& rStrm )
{
}


ULONG ExcCell::GetLen() const
{
    return 6 + GetDiffLen();
}



//----------------------------------------------------------- class ExcNumber -

ExcNumber::ExcNumber(
        const ScAddress rPos,
        const ScPatternAttr* pAttr,
        RootData& rRootData,
        const double& rNewVal ) :
    ExcCell( rPos, pAttr, rRootData )
{
    fVal = rNewVal;
}


void ExcNumber::SaveDiff( XclExpStream& rStrm )
{
    rStrm << fVal;
}


UINT16 ExcNumber::GetNum( void ) const
{
    return 0x0203;
}


ULONG ExcNumber::GetDiffLen( void ) const
{
    return 8;
}



//---------------------------------------------------------- class ExcBoolerr -

ExcBoolerr::ExcBoolerr(
        const ScAddress rPos,
        const ScPatternAttr* pAttr,
        RootData& rRootData,
        UINT8 nValP,
        BOOL bIsError ) :
    // #73420# force to "no number format" if boolean value
    ExcCell( rPos, pAttr, rRootData, (bIsError ? NUMBERFORMAT_ENTRY_NOT_FOUND : 0), !bIsError ),
    nVal( bIsError ? nValP : (nValP != 0) ),
    bError( bIsError != 0 )
{
}


void ExcBoolerr::SaveDiff( XclExpStream &rStr )
{
    rStr << nVal << bError;
}


UINT16 ExcBoolerr::GetNum( void ) const
{
    return 0x0205;
}


ULONG ExcBoolerr::GetDiffLen( void ) const
{
    return 2;
}



//---------------------------------------------------------- class ExcRKMulRK -

ExcRKMulRK::ExcRKMulRK(
        const ScAddress rPos,
        const ScPatternAttr *pAttr,
        RootData& rRootData,
        const INT32 nVal ) :
    ExcCell( rPos, pAttr, rRootData )
{
    if( ExcCell::pPrgrsBar )
        ExcCell::pPrgrsBar->SetState( ExcCell::GetCellCount() );
    ExcCell::IncCellCount();

    ExcRKMulRKEntry* pNewCont = new ExcRKMulRKEntry;
    pNewCont->nXF = nXF;
    pNewCont->nVal = nVal;

    List::Insert( pNewCont, LIST_APPEND );
}


ExcRKMulRK::~ExcRKMulRK()
{
    for( ExcRKMulRKEntry* pDel = _First(); pDel; pDel = _Next() )
        delete pDel;
}


ExcRKMulRK* ExcRKMulRK::Extend(
        const ScAddress rPos,
        const ScPatternAttr* pAttr,
        RootData& rRootData,
        const INT32 nVal )
{
    if( aPos.Row() == rPos.Row() && aPos.Col() + List::Count() == rPos.Col() )
    {// extendable
        if( ExcCell::pPrgrsBar )
            ExcCell::pPrgrsBar->SetState( ExcCell::GetCellCount() );
        ExcCell::IncCellCount();

        ExcRKMulRKEntry* pNewCont = new ExcRKMulRKEntry;

        if( !rRootData.pCellMerging->FindMergeBaseXF( rPos, pNewCont->nXF ) )
            pNewCont->nXF = rRootData.pXFRecs->Find( pAttr );
        pNewCont->nVal = nVal;
        List::Insert( pNewCont, LIST_APPEND );
        return NULL;
    }
    else
    {// create new
        return new ExcRKMulRK( rPos, pAttr, rRootData, nVal );
    }
}


UINT16 ExcRKMulRK::GetXF() const
{
    ExcRKMulRKEntry* pLast = List::Count() ? _Get( List::Count() - 1 ) : NULL;
    return pLast ? pLast->nXF : 0;
}


void ExcRKMulRK::SaveCont( XclExpStream& rStrm )
{
    if( ExcCell::pPrgrsBar )
        ExcCell::pPrgrsBar->SetState( ExcCell::GetCellCount() );

    ExcRKMulRKEntry* pCurr = _First();
    DBG_ASSERT( pCurr, "ExcRKMulRK::SaveDiff - list empty" );
    if( !pCurr ) return;

    if( IsRK() )
    {
        rStrm << (UINT16) aPos.Row() << (UINT16) aPos.Col() << pCurr->nXF << pCurr->nVal;
        ExcCell::IncCellCount();
    }
    else
    {
        UINT16  nLastCol = aPos.Col();
        rStrm << (UINT16) aPos.Row()  << nLastCol;
        while( pCurr )
        {
            rStrm << pCurr->nXF << pCurr->nVal;
            pCurr = _Next();
            ExcCell::IncCellCount();
            nLastCol++;
        }

        nLastCol--;
        rStrm << nLastCol;
    }
}


UINT16 ExcRKMulRK::GetNum( void ) const
{
    return IsRK() ? 0x027E : 0x00BD;
}


ULONG ExcRKMulRK::GetDiffLen( void ) const
{
    return IsRK() ? 4 : List::Count() * 6;
}



//------------------------------------------------------------ class ExcLabel -

ExcLabel::ExcLabel(
        const ScAddress rPos,
        const ScPatternAttr* pAttr,
        RootData& rRootData,
        const String& rText ) :
    ExcCell( rPos, pAttr, rRootData ),
    aText( rText, *rRootData.pCharset )
{
    nTextLen = (UINT16) Min( aText.Len(), (xub_StrLen) 0xFFFF );

    if( nTextLen > 255 )
    {
        nTextLen = 255;
        DBG_WARNING( "ExcLabel::ExcLabel(): String truncated" );
    }
}


ExcLabel::~ExcLabel()
{
}


void ExcLabel::SaveDiff( XclExpStream& rStrm )
{
    rStrm.WriteByteString( aText, nTextLen, TRUE );     // 16 bit length
}


UINT16 ExcLabel::GetNum( void ) const
{
    return 0x0204;
}


ULONG ExcLabel::GetDiffLen( void ) const
{
    return 2 + nTextLen;
}



//---------------------------------------------------------- class ExcRichStr

ExcRichStr::ExcRichStr( ExcCell& rExcCell, String& rText, const ScPatternAttr* pAttr,
                        const ScEditCell& rEdCell, RootData& rRoot, xub_StrLen nMaxChars ) :
    eBiff( rRoot.eDateiTyp )
{
    const EditTextObject*   p = rEdCell.GetData();
    XclHlink*&              rpLastHlink = rRoot.pLastHlink;

    if( rpLastHlink )
    {
        delete rpLastHlink;
        rpLastHlink = NULL;
    }

    if( p )
    {
        ScEditEngineDefaulter& rEdEng = rRoot.GetEdEng();
        const BOOL          bOldMode = rEdEng.GetUpdateMode();
        rEdEng.SetUpdateMode( TRUE );
        {
        SfxItemSet*         pTmpItemSet = new SfxItemSet( rEdEng.GetEmptyItemSet() );

        if( !pAttr )
            pAttr = rRoot.pDoc->GetDefPattern();

        pAttr->FillEditItemSet( pTmpItemSet );
        rEdEng.SetDefaults( pTmpItemSet );
        }
        rEdEng.SetText( *p );

        const USHORT        nParCnt = rEdEng.GetParagraphCount();
        USHORT              nPar;
        USHORT              n;
        USHORT              nF = 0;
        USHORT              nListLen;
        xub_StrLen          nParPos = 0;
        xub_StrLen          nExcStartPos;
        String              aParText;
        const sal_Unicode   cParSep = 0x0A;
        ESelection          aSel;
        ScPatternAttr       aPatAttr( rRoot.pDoc->GetPool() );
        UsedFontList&       rFontList = *rRoot.pFontRecs;
        String              sURLList;
        BOOL                bMultipleHlink = FALSE;

        // first font is the cell font, following font changes are stored in richstring
        Font*               pFont = new Font;
        pAttr->GetFont( *pFont );
        USHORT              nLastFontIndex = rFontList.Add( pFont );

        for( nPar = 0 ; nPar < nParCnt ; )
        {
            aSel.nStartPara = aSel.nEndPara = nPar;
            nParPos = rText.Len();

            aParText = rEdEng.GetText( nPar );
            String aExcParText;

            if( aParText.Len() )
            {
//              if ( eBiff < Biff8 )
//                  aParText.Convert( CHARSET_SYSTEM, CHARSET_ANSI );

                SvUShorts aPosList;
                rEdEng.GetPortions( nPar, aPosList );
                nListLen = aPosList.Count();

                aSel.nStartPos = 0;
                for( n = 0 ; n < nListLen ; n++ )
                {
                    aSel.nEndPos = ( xub_StrLen ) aPosList.GetObject( n );
                    nExcStartPos = nParPos + aExcParText.Len();
                    aExcParText += aParText.Copy( aSel.nStartPos, aSel.nEndPos - aSel.nStartPos );

                    {
                        SfxItemSet  aItemSet( rEdEng.GetAttribs( aSel ) );
                        BOOL        bWasHLink = FALSE;

                        // detect hyperlinks, export single hyperlink, create note if multiple hyperlinks,
                        // export hyperlink text in every case
                        if( aSel.nEndPos == (aSel.nStartPos + 1) )
                        {
                            const SfxPoolItem*          pItem;

                            if( aItemSet.GetItemState( EE_FEATURE_FIELD, FALSE, &pItem ) == SFX_ITEM_ON )
                            {
                                const SvxFieldData*     pField = ((const SvxFieldItem*) pItem)->GetField();

                                if( pField && pField->ISA( SvxURLField ) )
                                {
                                    // create new excel hyperlink and add text to cell text
                                    const SvxURLField& rURLField = *((const SvxURLField*) pField);
                                    bWasHLink = TRUE;

                                    if( sURLList.Len() )
                                        sURLList += cParSep;
                                    sURLList += rURLField.GetURL();

                                    XclHlink* pNewHlink = new XclHlink( rRoot, rURLField );
                                    const String* pReprString = pNewHlink->GetRepr();
                                    if( pReprString )
                                    {
                                        aExcParText.Erase( aExcParText.Len() - 1 );
                                        aExcParText += *pReprString;
                                    }

                                    if( rpLastHlink )
                                    {
                                        bMultipleHlink = TRUE;
                                        delete rpLastHlink;
                                        rpLastHlink = NULL;
                                    }
                                    if( bMultipleHlink )
                                        delete pNewHlink;
                                    else
                                        rpLastHlink = pNewHlink;
                                }
                            }
                        }

                        aPatAttr.GetItemSet().ClearItem();
                        aPatAttr.GetFromEditItemSet( &aItemSet );

                        Font* pFont = new Font;
                        aPatAttr.GetFont( *pFont );
                        if( bWasHLink )
                        {
                            pFont->SetColor( Color( COL_LIGHTBLUE ) );
                            pFont->SetUnderline( UNDERLINE_SINGLE );
                        }

                        UINT16 nFontIndex = rFontList.Add( pFont );

                        if( nFontIndex > 255 && eBiff < Biff8 )
                            nFontIndex = 0;

                        if( nExcStartPos <= nMaxChars && (eBiff >= Biff8 || nF < 256) )
                        {
                            if( nLastFontIndex != nFontIndex )
                            {
                                DBG_ASSERT( nExcStartPos <= 0xFFFF, "*ExcRichStr::ExcRichStr(): Start pos to big!" );
                                aForms.Append( ( UINT16 ) nExcStartPos );
                                aForms.Append( nFontIndex );
                                nLastFontIndex = nFontIndex;
                                nF++;
                            }
                        }
                    }
                    aSel.nStartPos = aSel.nEndPos;
                }
                rText += aExcParText;
            }

            nPar++;
            if( nPar < nParCnt )
                rText += cParSep;
        }

        rEdEng.SetUpdateMode( bOldMode );

        if( bMultipleHlink && sURLList.Len() )
        {
            if( rRoot.sAddNoteText.Len() )
                (rRoot.sAddNoteText += cParSep) += cParSep;
            rRoot.sAddNoteText += sURLList;
        }

        // XF mit Umbruch auswaehlen?
        rExcCell.SetXF( nParCnt <= 1 ? rRoot.pXFRecs->Find( pAttr ) : rRoot.pXFRecs->FindWithLineBreak( pAttr ) );
    }
    else
    {
        rEdCell.GetString( rText );
//      if ( eBiff < Biff8 )
//          rText.Convert( CHARSET_SYSTEM, CHARSET_ANSI );
    }
}


ExcRichStr::~ExcRichStr()
{
}


void ExcRichStr::Write( SvStream& rStrm )
{
    UINT32 nEnd = (UINT32) GetFormCount() * 2;

    if( eBiff >= Biff8 )
        for( UINT32 nIndex = 0 ; nIndex < nEnd ; nIndex++ )
            rStrm << aForms.Get( nIndex );
    else
        for( UINT32 nIndex = 0 ; nIndex < nEnd ; nIndex++ )
            rStrm << (UINT8) aForms.Get( nIndex );
}


void ExcRichStr::Write( XclExpStream& rStrm )
{
    UINT32 nEnd = (UINT32) GetFormCount() * 2;

    if( eBiff >= Biff8 )
    {
        rStrm.SetSliceLen( 4 );
        for( UINT32 nIndex = 0 ; nIndex < nEnd ; nIndex++ )
            rStrm << aForms.Get( nIndex );
    }
    else
    {
        rStrm.SetSliceLen( 2 );
        for( UINT32 nIndex = 0 ; nIndex < nEnd ; nIndex++ )
            rStrm << (UINT8) aForms.Get( nIndex );
    }
    rStrm.SetSliceLen( 0 );
}




//---------------------------------------------------------- class ExcRString -

ExcRString::ExcRString(
        const ScAddress aNewPos,
        const ScPatternAttr* pAttr,
        RootData& rRootData,
        const ScEditCell& rEdCell ) :
    ExcCell( aNewPos, pAttr, rRootData )
{
    pRichStr = new ExcRichStr( *this, aText, pAttr, rEdCell, rRootData, 255 );
    DBG_ASSERT( aText.Len() <= 0xFFFF, "*ExcRString::ExcRString(): String to long!" );
    nTextLen = (UINT16) Min( aText.Len(), (xub_StrLen) 255 );
}


ExcRString::~ExcRString()
{
    delete pRichStr;
}


void ExcRString::SaveDiff( XclExpStream& rStrm )
{
    rStrm.WriteByteString(
        ByteString( aText, *pExcRoot->pCharset ), nTextLen, TRUE ); // 16 bit length
    rStrm << (UINT8) pRichStr->GetFormCount();
    pRichStr->Write( rStrm );
}


UINT16 ExcRString::GetNum( void ) const
{
    return 0x00D6;
}


ULONG ExcRString::GetDiffLen( void ) const
{
    return 2 + nTextLen + 1 + pRichStr->GetByteCount();
}



//---------------------------------------------------------- class ExcFormula -

ExcFormula::ExcFormula(
        const ScAddress rPos,
        const ScPatternAttr* pAttr,
        RootData& rRootData,
        const ULONG nAltNumForm,
        BOOL bForceAltNumForm,
        const ScTokenArray& rTokArray,
        ExcArray** ppArray,
        ScMatrixMode eMM,
        ExcShrdFmla** ppShrdFmla,
        ExcArrays* pShrdFmlas ) :
    ExcCell( rPos, pAttr, rRootData, nAltNumForm, bForceAltNumForm )
{
    bShrdFmla = FALSE;
    EC_Codetype     eCodeType;
    ExcUPN*         pExcUPN;

    switch( eMM )
    {
        case MM_FORMULA:
            pExcUPN = new ExcUPN( TRUE, &rRootData, rTokArray, eCodeType, rPos );
            if( eCodeType == EC_ArrayFmla )
                break;
            else
            {
                delete pExcUPN;
                pExcUPN = NULL;
            }
            goto newchance;
        case MM_REFERENCE:
            pExcUPN = new ExcUPN( rTokArray, eCodeType );
            if( eCodeType == EC_ArrayFmla )
                break;
            else
            {
                delete pExcUPN;
                pExcUPN = NULL;
            }

            // no break here!
        default:
            newchance:
            pExcUPN = new ExcUPN( &rRootData, rTokArray, eCodeType, &rPos, FALSE, pShrdFmlas );
    }

    switch( eCodeType )
    {
        case EC_StdFmla:
            nFormLen = pExcUPN->GetLen();

            if( nFormLen > 0 )
            {
                pData = new sal_Char[ nFormLen ];
                memcpy( pData, pExcUPN->GetData(), nFormLen );
            }
            else
                pData = NULL;
            break;
        case EC_ArrayFmla:
            nFormLen = pExcUPN->GetLenWithShrdFmla();
            pExcUPN->GetShrdFmla( pData, nFormLen );

            if( pExcUPN->GetCode() && pExcUPN->GetLen() )
                *ppArray = new ExcArray( *pExcUPN, ( UINT8 ) rPos.Col(), rPos.Row() );
            else
                *ppArray = new ExcArray( ( UINT8 ) rPos.Col(), rPos.Row(), pExcUPN->GetArrayFormId() );
            break;
        case EC_ShrdFmla:
            if( ppShrdFmla && pExcUPN->GetShrdFmla( pData, nFormLen ) )
            {   // shared formula ref only
                bShrdFmla = TRUE;
                if( pExcUPN->IsFirstShrdFmla() )
                    // ... and create shared formula record the first time only
                    *ppShrdFmla = new ExcShrdFmla( pExcUPN->GetData(), pExcUPN->GetLen(), rPos );
                else
                    *ppShrdFmla = NULL;
            }
            else
            {
                pData = NULL;
                nFormLen = 0;
            }
            break;
    }

    delete pExcUPN;

/*  if( ppShrdFmla && aExcUPN.GetShrdFmla( pData, nFormLen ) )
    {   // shared formula ref only
        if( aExcUPN.IsFirstShrdFmla() )
            // ... and create shared formula record the first time only
            *ppShrdFmla = new ExcShrdFmla( aExcUPN.GetData(), aExcUPN.GetLen(), rPos );
        else
            *ppShrdFmla = NULL;
    }
    else
    {
        nFormLen = aExcUPN.GetLen();

        if( nFormLen > 0 )
        {
//          if( pAltArrayFormula )
//              rpArray = new ExcArray( *pAltArrayFormula, ( UINT8 ) rPos.Col(), rPos.Row() );
            pData = new sal_Char[ nFormLen ];
            memcpy( pData, aExcUPN.GetData(), nFormLen );
        }
        else
            pData = NULL;

    }*/
}


ExcFormula::~ExcFormula()
{
    if( pData )
        delete[] pData;
}


void ExcFormula::SetTableOp( USHORT nCol, USHORT nRow )
{
    if( pData )
        delete[] pData;
    nFormLen = 5;
    pData = new sal_Char[ nFormLen ];
    pData[ 0 ] = 0x02;
    ShortToSVBT16( (UINT16) nRow, (BYTE*) &pData[ 1 ] );
    ShortToSVBT16( (UINT16) nCol, (BYTE*) &pData[ 3 ] );
}


void ExcFormula::SaveDiff( XclExpStream& rStrm )
{//                         grbit               chn
    UINT16      nGrBit = bShrdFmla? 0x000B : 0x0003;
    rStrm   << ( double ) 0.0 << nGrBit  << ( UINT32 ) 0x00000000
            << nFormLen;
    rStrm.Write( pData, nFormLen );
}


UINT16 ExcFormula::GetNum( void ) const
{
    return 0x0006;
}


ULONG ExcFormula::GetDiffLen( void ) const
{
    return 16 + nFormLen;
}



//---------------------------------------------------- class ExcBlankMulblank -

ExcBlankMulblank::ExcBlankMulblank(
        const ScAddress rPos,
        const ScPatternAttr* pAttr,
        RootData& rRootData,
        UINT16 nCount ) :
    ExcCell( rPos, NULL, rRootData )
{
    nRecLen = 2 * nCount - 2;
    nLastCol = aPos.Col() + nCount - 1;
    bMulBlank = (nCount > 1);

    AddEntries( rPos, pAttr, rRootData, nCount );
    nXF = GetXF( UINT32List::Get( 0 ) );        // store first XF in ExcCell::nXF
}


void ExcBlankMulblank::Add(
        const ScAddress rPos,
        const ScPatternAttr* pAttr,
        RootData& rRootData,
        UINT16 nCount )
{
    DBG_ASSERT( rPos.Col() == nLastCol + 1, "ExcBlankMulblank::Add - wrong address" );

    nRecLen += 2 * nCount;
    nLastCol += nCount;
    bMulBlank = TRUE;

    AddEntries( rPos, pAttr, rRootData, nCount );
}


void ExcBlankMulblank::AddEntries(
        const ScAddress rPos,
        const ScPatternAttr* pAttr,
        RootData& rRootData,
        UINT16 nCount )
{
    DBG_ASSERT( nCount > 0, "ExcBlankMulblank::AddEntries - count==0!" );

    ScAddress   aCurrPos( rPos );
    UINT16      nCellXF = rRootData.pXFRecs->Find( pAttr );

    while( nCount )
    {
        UINT16 nMergeXF, nMergeCount;
        if( rRootData.pCellMerging->FindMergeBaseXF( aCurrPos, nMergeXF, nMergeCount ) )
        {
            nMergeCount = Min( nMergeCount, nCount );
            Append( nMergeXF, nMergeCount );
            nCount -= nMergeCount;
            aCurrPos.IncCol( nMergeCount );
        }
        else
        {
            UINT16 nMergeCol;
            UINT16 nColCount = nCount;

            if( rRootData.pCellMerging->FindNextMerge( aCurrPos, nMergeCol ) )
                nColCount = Min( (UINT16)(nMergeCol - aCurrPos.Col()), nCount );

            if( nColCount )
            {
                Append( nCellXF, nColCount );
                nCount -= nColCount;
                aCurrPos.IncCol( nColCount );
            }
        }
    }
}


UINT16 ExcBlankMulblank::GetXF() const
{
    DBG_ASSERT( UINT32List::Count(), "ExcBlankMulblank::GetXF - list empty" );
    return GetXF( UINT32List::Get( UINT32List::Count() - 1 ) );
}


void ExcBlankMulblank::SaveDiff( XclExpStream& rStrm )
{
    if( !bMulBlank ) return;

    UINT16 nLastCol = (UINT16) aPos.Col();
    for( ULONG nIndex = 0; nIndex < UINT32List::Count(); nIndex++ )
    {
        UINT32 nCurr = UINT32List::Get( nIndex );
        UINT16 nXF = GetXF( nCurr );
        UINT16 nTmpCount = GetCount( nCurr );

        if( !nIndex )
            nTmpCount--;        // very first XF is saved in ExcCell::SaveCont()

        nLastCol += nTmpCount;
        while( nTmpCount-- )
            rStrm << nXF;
    }
    rStrm << nLastCol;
}


UINT16 ExcBlankMulblank::GetNum() const
{
    return bMulBlank ? 0x00BE : 0x0201;
}


ULONG ExcBlankMulblank::GetDiffLen() const
{
    return bMulBlank ? (nRecLen + 2) : 0;
}



//---------------------------------------------------- class ExcNameListEntry -

ExcNameListEntry::~ExcNameListEntry()
{
}


UINT16 ExcNameListEntry::GetNum( void ) const
{
    return 0x0018;
}



//------------------------------------------------------------- class ExcName -

void ExcName::Init( BOOL bHid, BOOL bBIn )
{
    nFormLen = 0;
    pData = NULL;
    nTabNum = 0;
    bHidden = bHid;
    bBuiltIn = bBIn;
    bDummy = FALSE;
}


void ExcName::BuildFormula( const ScRange& rRange )
{   // build formula from range
    ScTokenArray        aArr;

    if( rRange.aStart == rRange.aEnd )
    {
        SingleRefData   aRef;
        aRef.InitAddress( rRange.aStart );
        aArr.AddSingleReference( aRef );
    }
    else
    {
        ComplRefData    aRef;
        aRef.InitRange( rRange );
        aArr.AddDoubleReference( aRef );
    }

    EC_Codetype         eDummy;
    ExcUPN              aExcUPN( pExcRoot, aArr, eDummy );
    nFormLen = aExcUPN.GetLen();

    DBG_ASSERT( nFormLen > 0, "+ExcName::BuildFormula(): No Data, no Name!" );

    if( nFormLen > 0 )
    {
        pData = new UINT8[ nFormLen ];
        memcpy( pData, aExcUPN.GetData(), nFormLen );
    }
    else
        pData = NULL;
}


ExcName::ExcName( RootData* pRD, ScRangeData* pRange ) : eBiff( pRD->eDateiTyp ), ExcRoot( pRD )
{
    Init();

    pRange->GetName( aName );

    bDummy = aName.CompareToAscii( ScFilterTools::GetBuiltInName( 0x06 ) ) == COMPARE_EQUAL;    // no PrintRanges

    if( !bDummy )
    {
        // Formel wandeln
        const ScTokenArray* pTokArray = pRange->GetCode();
        if( pTokArray )
        {
            EC_Codetype         eDummy;
            ExcUPN aExcUPN( pRD, *pTokArray, eDummy );
            nFormLen = aExcUPN.GetLen();

            DBG_ASSERT( nFormLen > 0, "+ExcName::ExcName(): No Data, no Name!" );

            if( nFormLen > 0 )
            {
                pData = new UINT8[ nFormLen ];
                memcpy( pData, aExcUPN.GetData(), nFormLen );
            }
        }
    }
}


ExcName::ExcName( RootData* pRD, ScDBData* pArea ) :
        ExcRoot( pRD ),
        eBiff( pRD->eDateiTyp )
{
    Init();

    pArea->GetName( aName );
    // generate unique name, DBData is merged into RangeName
    ScRangeName*                pRangeName = pRD->pDoc->GetRangeName();
    USHORT nPos;
    if ( pRangeName->SearchName( aName, nPos ) )
    {
        aName.AppendAscii( "_" );
        static const sal_Unicode    pExtend[] =
        {
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
            'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
            'U', 'V', 'W', 'X', 'Y', 'Z'
        };
        const USHORT nExtCnt = sizeof(pExtend) / sizeof(sal_Unicode);
        USHORT nExtend = nExtCnt;
        do
        {
            if ( nExtend < nExtCnt )
                aName.Erase( aName.Len() - 1, 1 );  // delete last char
            else
                nExtend = 0;
            aName.Append( pExtend[ nExtend++ ] );
        }
        while ( pRangeName->SearchName( aName, nPos ) );
    }

    ScRange aRange;
    pArea->GetArea( aRange );
    BuildFormula( aRange );
}


ExcName::ExcName( RootData* pRD, const ScRange& rRange, UINT8 nBuiltIn, BOOL bHid ) :
        ExcRoot( pRD ),
        eBiff( pRD->eDateiTyp )
{
    Init( bHid, TRUE );

    sal_Char aChar = (sal_Char) nBuiltIn;
    aName.AssignAscii( &aChar, 1 );
    nTabNum = rRange.aStart.Tab() + 1;

    BuildFormula( rRange );
}


ExcName::~ExcName()
{
    if( pData )
        delete[] pData;
}


void ExcName::Save( XclExpStream& rStrm )
{
    if( !bDummy )
        ExcNameListEntry::Save( rStrm );
}


void ExcName::SaveCont( XclExpStream& rStrm )
{
    UINT8       nNameLen = ( UINT8 ) Min( aName.Len(), ( xub_StrLen ) 255 );
    UINT16      nGrbit = (bHidden ? EXC_NAME_HIDDEN : 0) | (bBuiltIn ? EXC_NAME_BUILTIN : 0);

    rStrm   << nGrbit                   // grbit
            << (BYTE) 0x00              // chKey
            << nNameLen                 // cch
            << nFormLen                 // cce
            << (UINT16) 0x0000          // ixals
            << nTabNum                  // itab
            << (UINT32) 0x00000000;     // cch...

    if ( eBiff < Biff8 )
        rStrm.WriteByteStringBuffer( ByteString( aName, *pExcRoot->pCharset ), nNameLen );
    else
    {
        XclExpUniString aUni( aName, nNameLen );
        aUni.WriteFlags( rStrm );
        aUni.WriteBuffer( rStrm );
    }

    rStrm.Write( pData, nFormLen );
}


ULONG ExcName::GetLen( void ) const
{   // only a guess for Biff8 (8bit/16bit unknown)
    return bDummy ? 0 : (14 + nFormLen + (eBiff < Biff8 ? 0 : 1) + Min( aName.Len(), ( xub_StrLen ) 255 ));
}



//--------------------------------------------------------- class ExcNameList -

ExcNameList::~ExcNameList()
{
    for( ExcNameListEntry* pName = _First(); pName; pName = _Next() )
        delete pName;
}


void ExcNameList::Save( XclExpStream& rStrm )
{
    for( ExcNameListEntry* pName = _First(); pName; pName = _Next() )
        pName->Save( rStrm );
}



//--------------------------------------------------------- class ExcDummy_03 -

ULONG ExcDummy_03::GetLen( void ) const
{
    return nMyLen;
}


const BYTE* ExcDummy_03::GetData( void ) const
{
    return pMyData;
}



//------------------------------------------------------- class ExcDimensions -

ExcDimensions::ExcDimensions( BiffTyp eBiffP )
            : eBiff( eBiffP )
{
    nRwMic = nRwMac = nColMic = nColMac = 0;
}


ExcDimensions::ExcDimensions(  UINT16 nFirstCol, UINT16 nFirstRow,
    UINT16 nLastCol, UINT16 nLastRow, BiffTyp eBiffP )
            : eBiff( eBiffP )
{
    SetLimits( nFirstCol, nFirstRow, nLastCol, nLastRow );
}


void ExcDimensions::SetLimits( UINT16 nFirstCol, UINT16 nFirstRow,
    UINT16 nLastCol, UINT16 nLastRow )
{
    nRwMic = nFirstRow;
    nRwMac = nLastRow + 1;
    nColMic = nFirstCol;
    nColMac = nLastCol + 1;
}


void ExcDimensions::SaveCont( XclExpStream& rStrm )
{
    if ( eBiff < Biff8 )
        rStrm << nRwMic << nRwMac;
    else
        rStrm << (UINT32) nRwMic << (UINT32) nRwMac;
    rStrm << nColMic << nColMac << (UINT16) 0;
}


UINT16 ExcDimensions::GetNum( void ) const
{
    return 0x0200;
}


ULONG ExcDimensions::GetLen( void ) const
{
    return eBiff < Biff8 ? 10 : 14;
}



//--------------------------------------------------------- class ExcEOutline -

ExcEOutline::ExcEOutline( ScOutlineArray* pArray ) :
        pOLArray( pArray ),
        nCurrExcLevel( 0 ),
        bIsColl( FALSE )
{
    ScOutlineEntry* pEntry;

    for( UINT16 nLev = 0; nLev < SC_OL_MAXDEPTH; nLev++ )
    {
        pEntry = pArray ? pArray->GetEntryByPos( nLev, 0 ) : NULL;
        nEnd[ nLev ] = pEntry ? pEntry->GetEnd() : 0;
        bHidden[ nLev ] = FALSE;
    }

}


void ExcEOutline::Update( UINT16 nNum )
{
    if( !pOLArray )
        return;

    UINT16  nNewLevel;
    BOOL    bFound = pOLArray->FindTouchedLevel( nNum, nNum, nNewLevel );
    UINT16  nNewExcLevel = bFound ? nNewLevel + 1 : 0;
    UINT16  nLevel;

    if( nNewExcLevel >= nCurrExcLevel )
    {
        bIsColl = FALSE;
        for( nLevel = 0; nLevel < nNewExcLevel; nLevel++ )
            if( nEnd[ nLevel ] < nNum )
            {
                ScOutlineEntry* pEntry = pOLArray->GetEntryByPos( nLevel, nNum );
                if( pEntry )
                {
                    nEnd[ nLevel ] = pEntry->GetEnd();
                    bHidden[ nLevel ] = pEntry->IsHidden();
                }
            }
    }
    else
    {
        for( nLevel = nNewExcLevel; nLevel < nCurrExcLevel; nLevel++ )
            bIsColl |= bHidden[ nLevel ];
    }
    nCurrExcLevel = nNewExcLevel;
}



//------------------------------------------------------------ class ExcEGuts -

ExcEGuts::ExcEGuts( ScOutlineArray* pCol, ScOutlineArray* pRow )
{
    nRowLevel = nColLevel = 0;

    if( pCol )
        nColLevel = Min( pCol->GetDepth(), (UINT16) EXC_OUTLINE_MAX );
    if( pRow )
        nRowLevel = Min( pRow->GetDepth(), (UINT16) EXC_OUTLINE_MAX );
}


void ExcEGuts::SaveCont( XclExpStream& rStrm )
{
    rStrm   << (UINT16)(nRowLevel ? (12 * nRowLevel + 17) : 0)
            << (UINT16)(nColLevel ? (12 * nColLevel + 17) : 0)
            << (UINT16)(nRowLevel + (nRowLevel ? 1 : 0))
            << (UINT16)(nColLevel + (nRowLevel ? 1 : 0));
}


UINT16 ExcEGuts::GetNum() const
{
    return 0x0080;
}


ULONG ExcEGuts::GetLen() const
{
    return 8;
}



//-------------------------------------------------------------- class ExcRow -

ExcRow::ExcRow( UINT16 nRow, UINT16 nTab, UINT16 nFCol, UINT16 nLCol,
                UINT16 nNewXF, ScDocument& rDoc, ExcEOutline& rOutline ) :
        nNum( nRow ),
        nXF( nNewXF ),
        nOptions( 0x0000 )
{
    BYTE    nRowOptions = rDoc.GetRowFlags( nRow, nTab );
    BOOL    bUserHeight = TRUEBOOL( nRowOptions & CR_MANUALSIZE );

    SetRange( nFCol, nLCol );
    SetHeight( rDoc.GetRowHeight( nRow, nTab ), bUserHeight );

    if( TRUEBOOL( nRowOptions & CR_HIDDEN ) )
        nOptions |= EXC_ROW_ZEROHEIGHT;

    rOutline.Update( nRow );

    nOptions |= EXC_ROW_LEVELFLAGS( rOutline.GetLevel() );
    if( rOutline.IsCollapsed() )
        nOptions |= EXC_ROW_COLLAPSED;
}


void ExcRow::SetRange( UINT16 nFCol, UINT16 nLCol )
{
    DBG_ASSERT( nFCol <= nLCol, "+ExcRow::SetRange(): First Col > Last Col!" );

    nFirstCol = nFCol;
    nLastCol = nLCol;
}


void ExcRow::SetHeight( UINT16 nNewHeight, BOOL bUser )
{
    if( nNewHeight == 0 )
    {
        nOptions |= EXC_ROW_ZEROHEIGHT;
        nHeight = EXC_ROW_VALZEROHEIGHT;
    }
    else
        nHeight = nNewHeight;

    if( bUser )
        nOptions |= EXC_ROW_UNSYNCED;       // user height
//  else            not usable in Aplix as described in bug #76250#
//      nHeight |= EXC_ROW_FLAGDEFHEIGHT;   // default height
}


void ExcRow::SaveCont( XclExpStream& rStrm )
{
    nOptions |= EXC_ROW_FLAGCOMMON;
    rStrm   << nNum << nFirstCol << (UINT16)(nLastCol + 1)
            << nHeight << (UINT32)0 << nOptions << nXF;
}


UINT16 ExcRow::GetNum() const
{
    return 0x0208;
}


ULONG ExcRow::GetLen() const
{
    return 16;
}



//--------------------------------------------------------- class ExcRowBlock -

ExcRowBlock::ExcRowBlock()
{
    ppRows = new ExcRow* [ 32 ];
    nNext = 0;
}


ExcRowBlock::~ExcRowBlock()
{
    for( UINT16 nC = 0 ; nC < nNext ; nC++ )
        delete ppRows[ nC ];

    delete[] ppRows;
}


ExcRowBlock* ExcRowBlock::Append( ExcRow* pNewRow )
{
    if( nNext < 32 )
    {
        ppRows[ nNext ] = pNewRow;
        nNext++;
        return NULL;
    }
    else
    {
        ExcRowBlock* pRet = new ExcRowBlock;
        pRet->ppRows[ 0 ] = pNewRow;
        pRet->nNext = 1;
        return pRet;
    }
}


void ExcRowBlock::Save( XclExpStream& rStrm )
{
    for( UINT16 nC = 0 ; nC < nNext ; nC++ )
        ppRows[ nC ]->Save( rStrm );
}



//------------------------------------------------------ class ExcDefcolwidth -

ExcDefcolwidth::ExcDefcolwidth( UINT16 nNewWidth )
{
    nWidth = nNewWidth;
}


void ExcDefcolwidth::SaveCont( XclExpStream& rStrm )
{
    rStrm << nWidth;
}


UINT16 ExcDefcolwidth::GetNum( void ) const
{
    return 0x0055;
}


ULONG ExcDefcolwidth::GetLen( void ) const
{
    return 2;
}



//---------------------------------------------------------- class ExcColinfo -

ExcColinfo::ExcColinfo( UINT16 nCol, UINT16 nTab, UINT16 nNewXF, RootData& rRoot, ExcEOutline& rOutline )
{
    ScDocument&     rDoc = *rRoot.pDoc;

    nFirstCol = nLastCol = nCol;

    nXF = nNewXF;

    SetWidth( rDoc.GetColWidth( nCol, nTab ), rRoot.fColScale );

    BYTE nColFlags = rDoc.GetColFlags( nCol, nTab );
    nOptions = 0x0000;

    if( nColFlags & CR_HIDDEN )
        nOptions |= EXC_COL_HIDDEN;

    rOutline.Update( nCol );

    nOptions |= EXC_COL_LEVELFLAGS( rOutline.GetLevel() );
    if( rOutline.IsCollapsed() )
        nOptions |= EXC_COL_COLLAPSED;
}


void ExcColinfo::SetWidth( UINT16 nWidth, double fColScale )
{
    double  f = nWidth;
    f *= 1328.0 / 25.0;
    f += 90.0;
    f *= 1.0 / 23.0;
    f /= fColScale;

    nColWidth = (UINT16) f;
}


void ExcColinfo::Expand( ExcColinfo*& rpExp )
{
    if( !rpExp ) return;

    if( (nLastCol + 1 == rpExp->nFirstCol) && (nColWidth == rpExp->nColWidth) &&
        (nXF == rpExp->nXF) && (nOptions == rpExp->nOptions) )
    {// expand
        nLastCol = rpExp->nLastCol;
        delete rpExp;
        rpExp = NULL;
    }
}


void ExcColinfo::SaveCont( XclExpStream& rStrm )
{
    rStrm << nFirstCol << nLastCol << nColWidth << nXF << nOptions << (BYTE)0;
}


UINT16 ExcColinfo::GetNum( void ) const
{
    return 0x007D;
}


ULONG ExcColinfo::GetLen( void ) const
{
    return 11;
}



//--------------------------------------------------------------- class ExcXf -

ExcXf::ExcXf( UINT16 nFont, UINT16 nForm, const ScPatternAttr* pPattAttr, BOOL& rbWrap, BOOL bSt ) :
    bStyle( bSt )
{
#ifdef DBG_UTIL
    nObjCnt++;
#endif

    bAtrNum = bAtrFnt = bAtrAlc = bAtrBdr = bAtrPat = bAtrProt = !bSt;  // always TRUE for cell xfs

    nIfnt = nFont;
    nIfmt = nForm;

    if( pPattAttr )
    {
        nOffs8 = 0x0000;
        const ScProtectionAttr& rProtAttr = (const ScProtectionAttr&) pPattAttr->GetItem( ATTR_PROTECTION );
        if( rProtAttr.GetProtection() )
            nOffs8 |= EXC_XF_LOCKED;
        if( rProtAttr.GetHideFormula() || rProtAttr.GetHideCell() )
            nOffs8 |= EXC_XF_HIDDEN;

        switch( ( SvxCellHorJustify )
            ((const SvxHorJustifyItem&)pPattAttr->GetItem( ATTR_HOR_JUSTIFY )).GetValue() )
        {
            case SVX_HOR_JUSTIFY_STANDARD:  eAlc = EHA_General; break;
            case SVX_HOR_JUSTIFY_LEFT:      eAlc = EHA_Left; break;
            case SVX_HOR_JUSTIFY_CENTER:    eAlc = EHA_Center; break;
            case SVX_HOR_JUSTIFY_RIGHT:     eAlc = EHA_Right; break;
            case SVX_HOR_JUSTIFY_BLOCK:     eAlc = EHA_Justify; break;
            case SVX_HOR_JUSTIFY_REPEAT:    eAlc = EHA_Fill; break;
            default:
                DBG_ERROR( "ExcXf::ExcXf(): ATTR_HOR_JUSTIFY unbekannt!" );
                eAlc = EHA_General;
        }

        switch( ( SvxCellVerJustify )
            ((const SvxVerJustifyItem&)pPattAttr->GetItem( ATTR_VER_JUSTIFY )).GetValue() )
        {
            case SVX_VER_JUSTIFY_STANDARD:  eAlcV = EVA_Bottom; break;
            case SVX_VER_JUSTIFY_TOP:       eAlcV = EVA_Top; break;
            case SVX_VER_JUSTIFY_CENTER:    eAlcV = EVA_Center; break;
            case SVX_VER_JUSTIFY_BOTTOM:    eAlcV = EVA_Bottom; break;
            default:
                DBG_ERROR( "ExcXf::ExcXf(): ATTR_VER_JUSTIFY unbekannt!" );
                eAlcV = EVA_Bottom;
        }

        switch( ( SvxCellOrientation )
            ((const SvxOrientationItem&)pPattAttr->GetItem( ATTR_ORIENTATION )).GetValue() )
        {
            case SVX_ORIENTATION_STANDARD:  eOri = ETO_NoRot; break;
            case SVX_ORIENTATION_TOPBOTTOM: eOri = ETO_90cw; break;
            case SVX_ORIENTATION_BOTTOMTOP: eOri = ETO_90ccw; break;
            case SVX_ORIENTATION_STACKED:   eOri = ETO_TopBottom; break;
            default:
                DBG_ERROR( "ExcXf::ExcXf(): ATTR_ORIENTATION unbekannt!" );
                eOri = ETO_NoRot;
        }

        bFWrap = rbWrap ||
            ( ( BOOL ) ((const SfxBoolItem&)pPattAttr->GetItem( ATTR_LINEBREAK )).GetValue() );

        // fore-/background
        const SvxBrushItem&     rBrushItem = ( const SvxBrushItem& ) pPattAttr->GetItem( ATTR_BACKGROUND );
        Color                   aCol = rBrushItem.GetColor();

        nFls = aCol.GetTransparency()? 0x0000 : 0x0001;

        nIcvForeSer = pPalette2->InsertColor( aCol, EXC_COLOR_CELLBGROUND );
        nIcvBackSer = pPalette2->InsertColor( Color( COL_BLACK ), EXC_COLOR_CELLBGROUND );

        // Umrandung
        const SvxBoxItem&   rBox = ((const SvxBoxItem&)pPattAttr->GetItem( ATTR_BORDER ));

        ScToExcBorderLine( rBox.GetTop(), nIcvTopSer, nDgTop );
        ScToExcBorderLine( rBox.GetLeft(), nIcvLftSer, nDgLeft );
        ScToExcBorderLine( rBox.GetBottom(), nIcvBotSer, nDgBottom );
        ScToExcBorderLine( rBox.GetRight(), nIcvRigSer, nDgRight );

        if( bSt )
        {
            static const USHORT pFntChkWichs[] =
            {
                ATTR_FONT_HEIGHT,
                ATTR_FONT_WEIGHT,
                ATTR_FONT_POSTURE,
                ATTR_FONT_UNDERLINE,
                ATTR_FONT_CROSSEDOUT,
                ATTR_FONT_CONTOUR,
                ATTR_FONT_SHADOWED,
                ATTR_FONT_COLOR,
                ATTR_FONT_LANGUAGE,
                0
            };

            const SfxItemSet& rSet = pPattAttr->GetItemSet();

            bAtrNum = rSet.GetItemState( ATTR_VALUE_FORMAT, FALSE ) == SFX_ITEM_SET;
            bAtrFnt = pPattAttr->HasItemsSet( pFntChkWichs );
            bAtrAlc =   ( rSet.GetItemState( ATTR_HOR_JUSTIFY, FALSE ) == SFX_ITEM_SET ) ||
                        ( rSet.GetItemState( ATTR_VER_JUSTIFY, FALSE ) == SFX_ITEM_SET ) ||
                        ( rSet.GetItemState( ATTR_LINEBREAK, FALSE ) == SFX_ITEM_SET );
            bAtrBdr = rSet.GetItemState( ATTR_BORDER, FALSE ) == SFX_ITEM_SET;
            bAtrPat = rSet.GetItemState( ATTR_BACKGROUND, FALSE ) == SFX_ITEM_SET;
            bAtrProt = rSet.GetItemState( ATTR_PROTECTION, FALSE ) == SFX_ITEM_SET;
        }
    }
    else
    {
        nOffs8 = EXC_XF_LOCKED;
        eAlc = EHA_General;
        eAlcV = EVA_Bottom;
        eOri = ETO_NoRot;
        bFWrap = FALSE;
        nFls = 0x0000;
        nIcvBackSer = pPalette2->InsertIndex( 65 );
        nIcvForeSer = pPalette2->InsertIndex( 64 );
        nIcvTopSer = nIcvBotSer = nIcvLftSer = nIcvRigSer = 0;
        nDgTop = nDgLeft = nDgBottom = nDgRight = 0;
    }

    rbWrap = bFWrap;

    bFSxButton = FALSE;     //  XF Record attached PivotTable Button
}


#ifdef DBG_UTIL
ExcXf::~ExcXf()
{
    nObjCnt--;

    if( !nObjCnt )
        pPalette2 = NULL;   // letzter macht das Licht aus...
}
#endif


void ExcXf::SetPalette( ExcPalette2& rPalette2 )
{
    pPalette2 = &rPalette2;
}


void ExcXf::ScToExcBorderLine( const SvxBorderLine* pLine, UINT32& rIcvSer, UINT16& rDg )
{
    if( !pLine )
    {
        rIcvSer = 0;
        rDg = 0;
        return;
    }

    rIcvSer = pPalette2->InsertColor( pLine->GetColor(), EXC_COLOR_CELLBORDER );

    if( pLine->GetDistance() )
        // doppelte Linien
        rDg = 6;
    else
    {// einfache Linien
        UINT16 nOut = pLine->GetOutWidth();

        if( nOut <= DEF_LINE_WIDTH_1 )
            rDg = 1;
        else if( nOut <= DEF_LINE_WIDTH_2 )
            rDg = 2;
        else
            rDg = 5;
    }
}


void ExcXf::SaveCont( XclExpStream& rStrm )
{
    UINT16  nTmp;

    rStrm << nIfnt << nIfmt                             // Offs 4 + 6
        << nOffs8;                                      // Offs 8

    nTmp = ( UINT16 ) eAlc;                             // Offs 10
    nTmp |= 0x0800;     // locked
    if( bFWrap )
        nTmp += 0x0008;
    nTmp += ( ( UINT16 ) eAlcV ) << 4;
    nTmp += ( ( UINT16 ) eOri ) << 8;
    rStrm << nTmp;

    UINT16 nForeInd, nBackInd;
    pPalette2->GetMixedColors( nIcvForeSer, nIcvBackSer, nForeInd, nBackInd, nFls );
    nTmp = nForeInd;                                    // Offs 12
    nTmp |= nBackInd << 7;

    if ( bFSxButton )
        nTmp |= 0x2000;
    rStrm << nTmp;

    nTmp = nFls;                                        // Offs 14
    nTmp += nDgBottom << 6;
    nTmp |= pPalette2->GetColorIndex( nIcvBotSer ) << 9;
    rStrm << nTmp;

    nTmp = nDgTop;                                      // Offs 16
    nTmp += nDgLeft << 3;
    nTmp += nDgRight << 6;
    nTmp |= pPalette2->GetColorIndex( nIcvTopSer ) << 9;
    rStrm << nTmp;

    nTmp = pPalette2->GetColorIndex( nIcvLftSer );      // Offs 18
    nTmp |= pPalette2->GetColorIndex( nIcvRigSer ) << 7;
    rStrm << nTmp;
}


UINT16 ExcXf::GetNum( void ) const
{
    return 0x00E0;
}


ULONG ExcXf::GetLen( void ) const
{
    return 16;
}



//----------------------------------------------------------- class ExcFormat -

ExcFormat::ExcFormat( RootData* pExcRoot, UINT32 nNewScIndex ) : ExcRoot( pExcRoot )
{
    // ACHTUNG: nIndex wird hier NICHT gesetzt -> bei Freunden machen!
    if( !nObjCnt )
        pFormatter = new SvNumberFormatter( pExcRoot->pDoc->GetServiceManager(), LANGUAGE_ENGLISH_US );

    nObjCnt++;  // Objektzaehler einen hoch

    eBiff = pExcRoot->eDateiTyp;

    nScIndex = nNewScIndex;

    DBG_ASSERT( pExcRoot->pDoc->GetFormatTable(), "-ExcFormat::ExcFormat(): Keine Format-Tabelle!" );

    const SvNumberformat*   pEntry = pExcRoot->pDoc->GetFormatTable()->GetEntry( nNewScIndex );

    if( pEntry )
    {
        if( pEntry->GetType() == NUMBERFORMAT_LOGICAL )
        {
            String  aTmpStr;
            Color   aDummyCol;
            Color*  pDummyCol = &aDummyCol;

            pForm = new String( '\"' );
            ( ( SvNumberformat * ) pEntry )->GetOutputString( 1.0, aTmpStr, &pDummyCol );
            *pForm += aTmpStr;
            pForm->AppendAscii( "\";\"" );
            *pForm += aTmpStr;
            pForm->AppendAscii( "\";\"" );
            ( ( SvNumberformat * ) pEntry )->GetOutputString( 0.0, aTmpStr, &pDummyCol );
            *pForm += aTmpStr;
            pForm->AppendAscii( "\"" );
        }
        else
        {
            LanguageType    eLang = pEntry->GetLanguage();
            if ( eLang == LANGUAGE_ENGLISH_US )
                pForm = new String( pEntry->GetFormatstring() );
            else
        {
                xub_StrLen  nDummy;
                UINT32      nConvertIndex;
                INT16       nTyp = NUMBERFORMAT_DEFINED;
                String      aTmpString( pEntry->GetFormatstring() );
                pFormatter->PutandConvertEntry( aTmpString, nDummy, nTyp,
                                nConvertIndex, eLang, LANGUAGE_ENGLISH_US );

                pForm = new String( pFormatter->GetEntry( nConvertIndex )->GetFormatstring() );
        }
            if( pForm->EqualsAscii( "Standard" ) )
                pForm->AssignAscii( "General" );
        }
    }
    else
        pForm = new String( RTL_CONSTASCII_STRINGPARAM( "\"Internal Error: Unknown Numberformat!\"" ) );

    nFormLen = ( BYTE ) pForm->Len();
}


ExcFormat::~ExcFormat()
{
    delete pForm;

    DBG_ASSERT( nObjCnt, "*ExcFormat::~ExcFormat(): zuviel des Guten" );
    nObjCnt--;
    if( !nObjCnt )
    {
        DBG_ASSERT( pFormatter, "*ExcFormat::~ExcFormat(): multiple delete!" );
        delete pFormatter;
        pFormatter = NULL;
    }
}


void ExcFormat::SaveCont( XclExpStream& rStrm )
{
    if ( eBiff < Biff8 )
    {
        rStrm << nIndex;
        rStrm.WriteByteString(
            ByteString( *pForm, *pExcRoot->pCharset ), nFormLen );  // 8 bit length
    }
    else
    {
        rStrm << nIndex;
        XclExpUniString( *pForm, nFormLen ).Write( rStrm );     // normal unicode string
    }
}


UINT16 ExcFormat::GetNum( void ) const
{
    return 0x041E;
}


ULONG ExcFormat::GetLen( void ) const
{   //! for Biff8 only a prediction, assumes 8-bit string
    return ( eBiff < Biff8 ? 2 + 1 + nFormLen : 2 + 3 + nFormLen );
}



//--------------------------------------------------------- class ExcPalette2 -
// ExcPal2Entry & ExcPalette2

INT32 lcl_GetColorDistance( const Color& rCol1, const Color& rCol2 )
{
    INT32 nDist = rCol1.GetRed() - rCol2.GetRed();
    nDist *= nDist * 77;
    INT32 nDummy = rCol1.GetGreen() - rCol2.GetGreen();
    nDist += nDummy * nDummy * 151;
    nDummy = rCol1.GetBlue() - rCol2.GetBlue();
    nDist += nDummy * nDummy * 28;
    return nDist;
}

void lcl_SetMixedColor( Color& rDest, const Color& rSrc1, const Color& rSrc2 )
{
    rDest.SetRed( (UINT8)(((UINT16) rSrc1.GetRed() + rSrc2.GetRed()) >> 1) );
    rDest.SetGreen( (UINT8)(((UINT16) rSrc1.GetGreen() + rSrc2.GetGreen()) >> 1) );
    rDest.SetBlue( (UINT8)(((UINT16) rSrc1.GetBlue() + rSrc2.GetBlue()) >> 1) );
}




void ExcPal2Entry::UpdateEntry( UINT16 nColorType )
{
    switch( nColorType )
    {
        case EXC_COLOR_CHARTLINE:
            nWeight++;
            break;
        case EXC_COLOR_CELLBORDER:
        case EXC_COLOR_CHARTAREA:
            nWeight += 2;
            break;
        case EXC_COLOR_CELLTEXT:
        case EXC_COLOR_CHARTTEXT:
            nWeight += 10;
            break;
        case EXC_COLOR_CELLBGROUND:
            nWeight += 20;
            break;
    }
}

void ExcPal2Entry::AddColor( const ExcPal2Entry& rEntry )
{
    UINT32  nWeight2 = rEntry.GetWeighting();
    UINT32  nWeightSum = nWeight + nWeight2;
    UINT32  nWeightSum2 = nWeightSum >> 1;
    UINT32  nDummy;

    nDummy = nWeight * GetRed() + nWeight2 * rEntry.GetRed() + nWeightSum2;
    SetRed( (UINT8)(nDummy / nWeightSum) );
    nDummy = nWeight * GetGreen() + nWeight2 * rEntry.GetGreen() + nWeightSum2;
    SetGreen( (UINT8)(nDummy / nWeightSum) );
    nDummy = nWeight * GetBlue() + nWeight2 * rEntry.GetBlue() + nWeightSum2;
    SetBlue( (UINT8)(nDummy / nWeightSum) );

    nWeight = nWeightSum;
}

void ExcPal2Entry::Save( XclExpStream& rStrm )
{
    rStrm << GetRed() << GetGreen() << GetBlue() << (UINT8)0x00;
}




ExcPalette2::ExcPalette2( ColorBuffer& rCB ) :
        rColBuff( rCB ),
        nLastInd( 0 ),
        nMaxSerial( 0 ),
        pColorIndex( NULL ),
        pColors( NULL )
{
    InsertColor( Color( COL_BLACK ), EXC_COLOR_CELLTEXT );
}

ExcPalette2::~ExcPalette2()
{
    for( ExcPal2Entry* pEntry = _First(); pEntry; pEntry = _Next() )
        delete pEntry;
    if( pColorIndex )
        delete[] pColorIndex;
    if( pColors )
        delete[] pColors;
}

void ExcPalette2::SearchEntry( const Color& rCol, UINT32& rIndex, BOOL& rbIsEqual ) const
{
    rbIsEqual = FALSE;
    rIndex = 0;
    if( !Count() )
        return;

    ExcPal2Entry* pEntry = _Get( nLastInd );
    if( pEntry )
        if( pEntry->IsEqual( rCol ) )
        {
            rIndex = nLastInd;
            rbIsEqual = TRUE;
            return;
        }

    UINT32  nStart  = 0;
    UINT32  nEnd    = List::Count() - 1;
    UINT32  nNew;

    while( !rbIsEqual && (nStart <= nEnd) )
    {
        nNew = (nStart + nEnd) >> 1;
        pEntry = _Get( nNew );
        rbIsEqual = pEntry->IsEqual( rCol );
        if( !rbIsEqual )
        {
            if( pEntry->IsGreater( rCol ) )
                nEnd = nNew - 1;
            else
            {
                nStart = nNew + 1;
                if( nStart > nEnd )
                    nNew++;
            }
        }
    }
    rIndex = nNew;
}

ExcPal2Entry* ExcPalette2::CreateEntry( const Color& rCol, UINT32 nIndex )
{
    ExcPal2Entry* pEntry = new ExcPal2Entry( rCol, List::Count() );
    List::Insert( pEntry, nIndex );
    return pEntry;
}

UINT32 ExcPalette2::InsertColor( const Color& rCol, UINT16 nColorType )
{
    UINT32  nIndex;
    BOOL    bIsEqual;
    SearchEntry( rCol, nIndex, bIsEqual );

    ExcPal2Entry*   pEntry = _Get( nIndex );
    if( !pEntry || !bIsEqual )
        pEntry = CreateEntry( rCol, nIndex );

    pEntry->UpdateEntry( nColorType );
    nLastInd = nIndex;
    return pEntry->GetSerial();
}

UINT32 ExcPalette2::InsertIndex( UINT16 nIndex )
{
    return EXC_PAL2_INDEXBASE | nIndex;
}

void ExcPalette2::RecalcColorIndex( UINT32 nKeep, UINT32 nRemove )
{
    UINT32  nInd;
    UINT32* pColInd;
    for( nInd = 0, pColInd = pColorIndex; nInd < nMaxSerial; nInd++, pColInd++ )
    {
        if( *pColInd == nRemove )
            *pColInd = nKeep;
        if( *pColInd > nRemove )
            (*pColInd)--;
    }
}

void ExcPalette2::MergeColors( UINT32 nKeep, UINT32 nRemove )
{
    if( nRemove == 0 )      // don't remove color 0
    {
        nRemove = nKeep;
        nKeep = 0;
    }

    ExcPal2Entry*   pKeepEntry      = _Get( nKeep );
    ExcPal2Entry*   pRemoveEntry    = _Get( nRemove );
    if( pKeepEntry && pRemoveEntry )
    {
        if( nKeep == 0 )
            pKeepEntry->AddWeighting( *pRemoveEntry );
        else
            pKeepEntry->AddColor( *pRemoveEntry );

        List::Remove( nRemove );
        delete pRemoveEntry;
        RecalcColorIndex( nKeep, nRemove );
    }
}

UINT32 ExcPalette2::GetRemoveColor() const
{
    UINT32 nFound   = 0;
    UINT32 nMinW    = 0xFFFFFFFF;

    for( UINT32 nInd = 0; nInd < List::Count(); nInd++ )
    {
        ExcPal2Entry* pEntry = _Get( nInd );
        if( pEntry )
            if( pEntry->GetWeighting() < nMinW )
            {
                nFound = nInd;
                nMinW = pEntry->GetWeighting();
            }
    }
    return nFound;
}

INT32 ExcPalette2::GetNearColors( const Color& rCol, UINT32& rFirst, UINT32& rSecond ) const
{
    rFirst = rSecond = 0;
    INT32 nMinD1 = 0x7FFFFFFF;
    INT32 nMinD2 = 0x7FFFFFFF;

    for( UINT32 nInd = 0; nInd < List::Count(); nInd++ )
    {
        ExcPal2Entry* pEntry = _Get( nInd );
        if( pEntry )
        {
            Color aEntryColor( pEntry->GetColor() );
            INT32 nDist = lcl_GetColorDistance( aEntryColor, rCol );
            if( nDist < nMinD1 )
            {
                rSecond = rFirst;
                nMinD2 = nMinD1;
                rFirst = nInd;
                nMinD1 = nDist;
            }
            else if( nDist < nMinD2 )
            {
                rSecond = nInd;
                nMinD2 = nDist;
            }
        }
    }
    return nMinD1;
}

UINT32 ExcPalette2::GetNearestColor( const Color& rCol, UINT32 nIgnore ) const
{
    UINT32  nFound  = 0;
    INT32   nMinD   = 0x7FFFFFFF;

    for( UINT32 nInd = 0; nInd < List::Count(); nInd++ )
        if( nInd != nIgnore )
        {
            ExcPal2Entry* pCheck = _Get( nInd );
            if( pCheck )
            {
                Color aCheckColor( pCheck->GetColor() );
                INT32 nDist = lcl_GetColorDistance( rCol, aCheckColor );
                if( nDist < nMinD )
                {
                    nFound = nInd;
                    nMinD = nDist;
                }
            }
        }
    return nFound;
}

UINT32 ExcPalette2::GetNearestColor( UINT32 nIndex ) const
{
    ExcPal2Entry* pEntry = _Get( nIndex );
    if( !pEntry )
        return 0;

    Color aCol( pEntry->GetColor() );
    return GetNearestColor( aCol, nIndex );
}

void ExcPalette2::ReduceColors()
{
    nMaxSerial = List::Count();
    if( !nMaxSerial )
        return;

    pColorIndex = new UINT32[ nMaxSerial ];
    pColors = new Color[ nMaxSerial ];

    for( UINT32 nInd = 0; nInd < nMaxSerial; nInd++ )
    {
        ExcPal2Entry* pEntry = _Get( nInd );
        if( pEntry )
        {
            pColorIndex[ pEntry->GetSerial() ] = nInd;
            pColors[ pEntry->GetSerial() ].SetColor( pEntry->GetColor() );
        }
    }

    while( List::Count() > rColBuff.GetAnz() )
    {
        UINT32 nRemove = GetRemoveColor();
        UINT32 nNearest = GetNearestColor( nRemove );
        MergeColors( nNearest, nRemove );
    }
}

UINT16 ExcPalette2::GetColorIndex( const Color& rCol ) const
{
    UINT32 nIndex = GetNearestColor( rCol, EXC_PAL2_IGNORE );
    return (UINT16)(nIndex + ColorBuffer::GetIndCorrect());
}

UINT16 ExcPalette2::GetColorIndex( UINT32 nSerial ) const
{
    if( nSerial >= EXC_PAL2_INDEXBASE )
        return (UINT16)(nSerial & ~EXC_PAL2_INDEXBASE);
    if( (nSerial >= nMaxSerial) || !pColorIndex )
        return 0;
    return (UINT16)(pColorIndex[ nSerial ] + ColorBuffer::GetIndCorrect());
}

void ExcPalette2::GetMixedColors( UINT32 nForeSer, UINT32 nBackSer,
                                UINT16& rForeInd, UINT16& rBackInd, UINT16& rPatt ) const
{
    if( (nForeSer >= EXC_PAL2_INDEXBASE) || (nBackSer >= EXC_PAL2_INDEXBASE) ||
        ( !rPatt ) )        // <> 0x0000 -> non-solid brush
    {
        rForeInd = GetColorIndex( nForeSer );
        rBackInd = GetColorIndex( nBackSer );
        return;
    }

    rForeInd = rBackInd = 0;
    if( (nForeSer >= nMaxSerial) || !pColors )
        return;

    UINT32  nIndex1, nIndex2;
    Color   aColorArr[ 5 ];
    INT32   nFirstDist = GetNearColors( pColors[ nForeSer ], nIndex1, nIndex2 );

    ExcPal2Entry*   pEntry1 = _Get( nIndex1 );
    ExcPal2Entry*   pEntry2 = _Get( nIndex2 );
    if( !pEntry1 || !pEntry2 )
        return;

    aColorArr[ 0 ].SetColor( pEntry1->GetColor() );
    aColorArr[ 4 ].SetColor( pEntry2->GetColor() );
    lcl_SetMixedColor( aColorArr[ 2 ], aColorArr[ 0 ], aColorArr[ 4 ] );
    lcl_SetMixedColor( aColorArr[ 1 ], aColorArr[ 0 ], aColorArr[ 2 ] );
    lcl_SetMixedColor( aColorArr[ 3 ], aColorArr[ 2 ], aColorArr[ 4 ] );

    INT32   nMinDist    = nFirstDist;
    UINT16  nMinIndex   = 0;
    for( UINT16 nCnt = 1; nCnt < 4; nCnt++ )
    {
        INT32 nDist = lcl_GetColorDistance( pColors[ nForeSer ], aColorArr[ nCnt ] );
        if( nDist < nMinDist )
        {
            nMinDist = nDist;
            nMinIndex = nCnt;
        }
    }
    rForeInd = (UINT16)(nIndex1 + ColorBuffer::GetIndCorrect());
    rBackInd = (UINT16)(nIndex2 + ColorBuffer::GetIndCorrect());
    if( nMinDist < (nFirstDist >> 1) )
        switch( nMinIndex )
        {
            case 1: rPatt = 3; break;
            case 2: rPatt = 2; break;
            case 3: rPatt = 4; break;
        }
}

ColorData ExcPalette2::GetRGBValue( UINT16 nIndex ) const
{
    if( nIndex >= ColorBuffer::GetIndCorrect() )
    {
        ExcPal2Entry* pEntry = _Get( nIndex - ColorBuffer::GetIndCorrect() );
        if( pEntry )
            return pEntry->GetColor();
    }
    return 0;
}

void ExcPalette2::SaveCont( XclExpStream& rStrm )
{
    rStrm << rColBuff.GetAnz();

    for( ExcPal2Entry* pEntry = _First(); pEntry; pEntry = _Next() )
        pEntry->Save( rStrm );

    for( UINT16 nInd = (UINT16) List::Count(); nInd < rColBuff.GetAnz(); nInd++ )
        rStrm   << rColBuff.GetRed( nInd )
                << rColBuff.GetGreen( nInd )
                << rColBuff.GetBlue( nInd )
                << (UINT8)0x00;
}

UINT16 ExcPalette2::GetNum() const
{
    return 0x0092;
}

ULONG ExcPalette2::GetLen() const
{
    return 2 + 4 * rColBuff.GetAnz();
}



//------------------------------------------------------ class ExcExterncount -

ExcExterncount::ExcExterncount( RootData* pRD, const BOOL bTableNew ) :
    ExcRoot( pRD ),
    bTable( bTableNew )
{
}


void ExcExterncount::SaveCont( XclExpStream& rStrm )
{
    UINT16  nNumTabs = pExcRoot->pDoc->GetTableCount();

    if( nNumTabs && bTable )
        nNumTabs--;

    rStrm << nNumTabs;
}


UINT16 ExcExterncount::GetNum( void ) const
{
    return 0x0016;
}


ULONG ExcExterncount::GetLen( void ) const
{
    return 2;
}



//------------------------------------------------------ class ExcExternsheet -

ExcExternsheet::ExcExternsheet( RootData* pExcRoot, const UINT16 nNewTabNum ) : ExcRoot( pExcRoot )
{
    DBG_ASSERT( pExcRoot->pDoc->HasTable( nNewTabNum ),
        "*ExcExternsheet::ExcExternsheet(): table not found" );

    pExcRoot->pDoc->GetName( nNewTabNum, aTabName );
    DBG_ASSERT( aTabName.Len() < 255,
        "*ExcExternsheet::ExcExternsheet(): table name too long" );
}


void ExcExternsheet::SaveCont( XclExpStream& rStrm )
{
    rStrm << ( UINT8 ) Min( (xub_StrLen)(aTabName.Len() + 1), (xub_StrLen) 255 ) << ( UINT8 ) 0x03;
    rStrm.WriteByteStringBuffer(
        ByteString( aTabName, *pExcRoot->pCharset ), 254 );     // max 254 chars (leading 0x03!)
}


UINT16 ExcExternsheet::GetNum( void ) const
{
    return 0x0017;
}


ULONG ExcExternsheet::GetLen( void ) const
{
    return 2 + Min( aTabName.Len(), (xub_StrLen) 254 );
}



//-------------------------------------------------- class ExcExternsheetList -

ExcExternsheetList::~ExcExternsheetList()
{
    for( ExcExternsheet* pSheet = _First(); pSheet; pSheet = _Next() )
        delete pSheet;
}


void ExcExternsheetList::Save( XclExpStream& rStrm )
{
    for( ExcExternsheet* pSheet = _First(); pSheet; pSheet = _Next() )
        pSheet->Save( rStrm );
}



//-------------------------------------------------------- class ExcExternDup -

ExcExternDup::ExcExternDup( ExcExterncount& rC, ExcExternsheetList& rL ) :
    rExtCnt( rC ), rExtSheetList( rL )
{
}


ExcExternDup::ExcExternDup( const ExcExternDup& r ) :
    rExtCnt( r.rExtCnt ), rExtSheetList( r.rExtSheetList )
{
}


void ExcExternDup::Save( XclExpStream& rStrm )
{
    rExtCnt.Save( rStrm );
    rExtSheetList.Save( rStrm );
}



//---------------------------------------------------------- class ExcWindow2 -

ExcWindow2::ExcWindow2( UINT16 nTab ) : nTable( nTab )
{
}


void ExcWindow2::SaveCont( XclExpStream& rStrm )
{
    BYTE pData[] = { 0xb6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    DBG_ASSERT( sizeof(pData) == GetLen(), "ExcWindow2::SaveCont: length mismatch" );
    if ( GetTable() == 0 )
        pData[1] |= 0x06;       // displayed and selected
    rStrm.Write( pData, GetLen() );
}


UINT16 ExcWindow2::GetNum( void ) const
{
    return 0x023E;
}


ULONG ExcWindow2::GetLen( void ) const
{
    return 10;
}



//------------------------------------------------------------ class UsedList -

void UsedList::Save( XclExpStream& rStrm )
{
    for( ExcRecord* pExcRec = (ExcRecord*) First(); pExcRec; pExcRec = (ExcRecord*) Next() )
        pExcRec->Save( rStrm );
}



//-------------------------------------------------------- class UsedFontList -

UsedFontList::~UsedFontList()
{
    for( ExcFont* pFont = _First(); pFont; pFont = _Next() )
        delete pFont;
}


BOOL UsedFontList::Find( ExcFont* pExcFont, UINT16& rIndex )
{
    rIndex = nBaseIndex;
    for( ExcFont* pCurr = _First(); pCurr; pCurr = _Next() )
    {
        if( *pCurr == *pExcFont )
        {
            if( !pExcFont->HasIgnoreCol() )
                pCurr->SetColor( pExcFont->GetColor() );    // overwrite ignore flag
            return TRUE;
        }
        rIndex++;
    }
    return FALSE;
}


UINT16 UsedFontList::Add( ExcFont* pExcFont )
{
    UINT16 nIndex;
    if( Find( pExcFont, nIndex ) )
    {
        delete pExcFont;
        return nIndex;
    }

    Insert( pExcFont, LIST_APPEND );

    return (UINT16)(Count() - 1 + nBaseIndex);
}


UINT16 UsedFontList::Add( Font* pFont )
{
    return Add( new ExcFont( pFont, *pExcRoot ) );
}



//-------------------------------------------------------- class UsedFormList -

UsedFormList::~UsedFormList()
{
    for( ExcFormat* pFormat = _First(); pFormat; pFormat = _Next() )
        delete pFormat;
}


UINT16 UsedFormList::Add( ExcFormat* pFormat )
{
    // Doubletten suchen
    UINT32      nSearch = pFormat->nScIndex;

    UINT16      nC = nBaseIndex;
    for( ExcFormat* pCurr = _First(); pCurr; pCurr = _Next() )
    {
        if( pCurr->nScIndex == nSearch )
        {
            delete pFormat;
            return nC;
        }
        nC++;
    }

    pFormat->nIndex = nC;
    Insert( pFormat, LIST_APPEND );
    return nC;
}



//-------------------------------------------------------- class UsedAttrList -

UsedAttrList::UsedAttrList( RootData* pRD, ExcPalette2& rPal2,
    UsedFontList& rUFntLst, UsedFormList& rUFrmLst ) :
    ExcRoot( pRD ),
    rPalette2( rPal2 ),
    rFntLst( rUFntLst ),
    rFrmLst( rUFrmLst )
{
}


UsedAttrList::~UsedAttrList()
{
    for( ENTRY* pEntry = _First(); pEntry; pEntry = _Next() )
        delete pEntry;
}


void UsedAttrList::AddNewXF( const ScPatternAttr* pAttr, const BOOL bStyle, const BOOL bExplLineBreak,
                                const ULONG nAltNumForm, BOOL bForceAltNumForm )
{
    // Aufbauen der Excel-Daten
    ENTRY*          pData = new ENTRY;
    pData->pPattAttr = pAttr;
    pData->bLineBreak = bExplLineBreak;
    pData->nAltNumForm = nAltNumForm;
    List::Insert( pData, LIST_APPEND );

    // Font-Daten
    UINT16          nFontIndex;

    // Format-Daten
    ULONG           nScForm;

    if( pAttr )
    {
        Font*       pFont = new Font;
        pAttr->GetFont( *pFont );
        nFontIndex = rFntLst.Add( pFont );

        if ( bForceAltNumForm )
            nScForm = nAltNumForm;
        else
        {
            nScForm = (( SfxUInt32Item& ) pAttr->GetItem( ATTR_VALUE_FORMAT )).GetValue();

            if( nAltNumForm != NUMBERFORMAT_ENTRY_NOT_FOUND &&
                    ( nScForm % SV_COUNTRY_LANGUAGE_OFFSET ) == 0 )
                nScForm = nAltNumForm;
        }
    }
    else
    {
        nScForm = nAltNumForm;
        nFontIndex = 0;
    }

    ExcFormat*      pFormat = new ExcFormat( pExcRoot, nScForm );

    UINT16          nFormatIndex = rFrmLst.Add( pFormat );

    if ( pExcRoot->eDateiTyp < Biff8 )
        pData->pXfRec = new ExcXf( nFontIndex, nFormatIndex, pAttr, pData->bLineBreak, bStyle );
    else
        pData->pXfRec = new ExcXf8( nFontIndex, nFormatIndex, pAttr, pData->bLineBreak, bStyle );
}


UINT16 UsedAttrList::Find( const ScPatternAttr* pAttr, const BOOL bStyle,
        const UINT32 nAltNumForm, BOOL bForceAltNumForm )
{
    if( !pAttr && nAltNumForm == NUMBERFORMAT_ENTRY_NOT_FOUND )
        return 15;      // default XF

    DBG_ASSERT( nBaseIndex, "*UsedAttrList::Find(): leer == nichtleer?" );

    UINT16          nC = nBaseIndex;
    for( ENTRY* pCurr = _First(); pCurr; pCurr = _Next() )
    {
        if( pCurr->Equal( pAttr, nAltNumForm ) )
            return nC;
        nC++;
    }

    // neuen ausdenken

    AddNewXF( pAttr, bStyle, FALSE, nAltNumForm, bForceAltNumForm );

    return nC;
}


UINT16 UsedAttrList::FindWithLineBreak( const ScPatternAttr* pAttr )
{
    DBG_ASSERT( nBaseIndex,
        "*UsedAttrList::FindWithLineBreak(): leer == nichtleer?" );

    UINT16          nC = nBaseIndex;
    for( ENTRY* pCurr = _First(); pCurr; pCurr = _Next() )
    {
        if( pCurr->pPattAttr == pAttr && pCurr->bLineBreak )
            return nC;
        nC++;
    }

    // neuen ausdenken

    AddNewXF( pAttr, FALSE, TRUE );

    return nC;
}


void UsedAttrList::Save( XclExpStream& rStrm )
{
    for( ENTRY* pEntry = _First(); pEntry; pEntry = _Next() )
        if( pEntry->pXfRec )
            pEntry->pXfRec->Save( rStrm );
}



//------------------------------------------------------------ class ExcSetup -

ExcSetup::ExcSetup( RootData* pExcRoot )
{
    SfxStyleSheet*          pStSh = pExcRoot->pStyleSheet;

    if( pStSh )
    {
        SfxItemSet&         rSet = *pExcRoot->pStyleSheetItemSet;
        const SvxPageItem&  rItem = ( const SvxPageItem& ) rSet.Get( ATTR_PAGE );
        const BOOL          bLandscape = rItem.IsLandscape();

        nGrbit = bLandscape? 0 : 0x0002;    // !fLandscape / fLandscape

        // direct copy from ImportExcel::Setup()!
        struct PAPER_SIZE
        {
            INT32                   nH;
            INT32                   nW;
        };
        static const                nAnzSizes = 42;
        static const PAPER_SIZE     pPS[ nAnzSizes ] =
        {
            {(INT32)(0),(INT32)(0)},                                        // undefined
            {(INT32)(TWIPS_PER_INCH*8.5),(INT32)(TWIPS_PER_INCH*11)},       // Letter 8 1/2 x 11 in
            {(INT32)(TWIPS_PER_INCH*8.5),(INT32)(TWIPS_PER_INCH*11)},       // Letter Small 8 1/2 x 11 in
            {(INT32)(TWIPS_PER_INCH*11),(INT32)(TWIPS_PER_INCH*17)},        // Tabloid 11 x 17 in
            {(INT32)(TWIPS_PER_INCH*17),(INT32)(TWIPS_PER_INCH*11)},        // Ledger 17 x 11 in
            {(INT32)(TWIPS_PER_INCH*8.5),(INT32)(TWIPS_PER_INCH*14)},       // Legal 8 1/2 x 14 in
            {(INT32)(TWIPS_PER_INCH*5.5),(INT32)(TWIPS_PER_INCH*8.5)},      // Statement 5 1/2 x 8 1/2 in
            {(INT32)(TWIPS_PER_INCH*7.25),(INT32)(TWIPS_PER_INCH*10.5)},    // Executive 7 1/4 x 10 1/2 in
            {(INT32)(TWIPS_PER_CM*29.7),(INT32)(TWIPS_PER_CM*42.0)},        // A3 297 x 420 mm
            {(INT32)(TWIPS_PER_CM*21.0),(INT32)(TWIPS_PER_CM*29.7)},        // A4 210 x 297 mm
            {(INT32)(TWIPS_PER_CM*21.0),(INT32)(TWIPS_PER_CM*29.7)},        // A4 Small 210 x 297 mm
            {(INT32)(TWIPS_PER_CM*14.8),(INT32)(TWIPS_PER_CM*21.0)},        // A5 148 x 210 mm
            {(INT32)(TWIPS_PER_CM*25.0),(INT32)(TWIPS_PER_CM*35.4)},        // B4 250 x 354
            {(INT32)(TWIPS_PER_CM*18.2),(INT32)(TWIPS_PER_CM*25.7)},        // B5 182 x 257 mm
            {(INT32)(TWIPS_PER_INCH*8.5),(INT32)(TWIPS_PER_INCH*13)},       // Folio 8 1/2 x 13 in
            {(INT32)(TWIPS_PER_CM*21.5),(INT32)(TWIPS_PER_CM*27.5)},        // Quarto 215 x 275 mm
            {(INT32)(TWIPS_PER_INCH*10),(INT32)(TWIPS_PER_INCH*14)},        // 10x14 in
            {(INT32)(TWIPS_PER_INCH*11),(INT32)(TWIPS_PER_INCH*17)},        // 11x17 in
            {(INT32)(TWIPS_PER_INCH*8.5),(INT32)(TWIPS_PER_INCH*11)},       // Note 8 1/2 x 11 in
            {(INT32)(TWIPS_PER_INCH*3.875),(INT32)(TWIPS_PER_INCH*8.875)},  // Envelope #9 3 7/8 x 8 7/8
            {(INT32)(TWIPS_PER_INCH*4.125),(INT32)(TWIPS_PER_INCH*9.5)},    // Envelope #10 4 1/8 x 9 1/2
            {(INT32)(TWIPS_PER_INCH*4.5),(INT32)(TWIPS_PER_INCH*10.375)},   // Envelope #11 4 1/2 x 10 3/8
            {(INT32)(TWIPS_PER_INCH*4.03),(INT32)(TWIPS_PER_INCH*11)},      // Envelope #12 4 \276 x 11
            {(INT32)(TWIPS_PER_INCH*14.5),(INT32)(TWIPS_PER_INCH*11.5)},    // Envelope #14 5 x 11 1/2
            {(INT32)(0),(INT32)(0)},                                        // C size sheet
            {(INT32)(0),(INT32)(0)},                                        // D size sheet
            {(INT32)(0),(INT32)(0)},                                        // E size sheet
            {(INT32)(TWIPS_PER_CM*11.0),(INT32)(TWIPS_PER_CM*22.0)},        // Envelope DL 110 x 220mm
            {(INT32)(TWIPS_PER_CM*16.2),(INT32)(TWIPS_PER_CM*22.9)},        // Envelope C5 162 x 229 mm
            {(INT32)(TWIPS_PER_CM*32.4),(INT32)(TWIPS_PER_CM*45.8)},        // Envelope C3  324 x 458 mm
            {(INT32)(TWIPS_PER_CM*22.9),(INT32)(TWIPS_PER_CM*32.4)},        // Envelope C4  229 x 324 mm
            {(INT32)(TWIPS_PER_CM*11.4),(INT32)(TWIPS_PER_CM*16.2)},        // Envelope C6  114 x 162 mm
            {(INT32)(TWIPS_PER_CM*11.4),(INT32)(TWIPS_PER_CM*22.9)},        // Envelope C65 114 x 229 mm
            {(INT32)(TWIPS_PER_CM*25.0),(INT32)(TWIPS_PER_CM*35.3)},        // Envelope B4  250 x 353 mm
            {(INT32)(TWIPS_PER_CM*17.6),(INT32)(TWIPS_PER_CM*25.0)},        // Envelope B5  176 x 250 mm
            {(INT32)(TWIPS_PER_CM*17.6),(INT32)(TWIPS_PER_CM*12.5)},        // Envelope B6  176 x 125 mm
            {(INT32)(TWIPS_PER_CM*11.0),(INT32)(TWIPS_PER_CM*23.0)},        // Envelope 110 x 230 mm
            {(INT32)(TWIPS_PER_INCH*3.875),(INT32)(TWIPS_PER_INCH*7.5)},    // Envelope Monarch 3.875 x 7.5 in
            {(INT32)(TWIPS_PER_INCH*3.625),(INT32)(TWIPS_PER_INCH*6.5)},    // 6 3/4 Envelope 3 5/8 x 6 1/2 in
            {(INT32)(TWIPS_PER_INCH*14.875),(INT32)(TWIPS_PER_INCH*11)},    // US Std Fanfold 14 7/8 x 11 in
            {(INT32)(TWIPS_PER_INCH*8.5),(INT32)(TWIPS_PER_INCH*12)},       // German Std Fanfold 8 1/2 x 12 in
            {(INT32)(TWIPS_PER_INCH*8.5),(INT32)(TWIPS_PER_INCH*13)}        // German Legal Fanfold 8 1/2 x 13 in
        };

        UINT16              n = 0;
        const PAPER_SIZE*   pAct = pPS;
        Size                aSize( ( ( const SvxSizeItem& ) rSet.Get( ATTR_PAGE_SIZE ) ).GetSize() );
        const long          nH = bLandscape? aSize.nB : aSize.nA;
        const long          nW = bLandscape? aSize.nA : aSize.nB;

        long                nMaxDH = 50;
        long                nMinDH = -nMaxDH;
        long                nMaxDW = 70;
        long                nMinDW = -nMaxDW;
        long                nDH, nDW;

        nPaperSize = 0;

        while( nPaperSize < nAnzSizes )
        {
            nDH = pAct->nH - nH;
            nDW = pAct->nW - nW;
            if( nDH >= nMinDH && nDH <= nMaxDH && nDW >= nMinDW && nDW <= nMaxDW )
                break;
            nPaperSize++;
            pAct++;
        }

        if( nPaperSize >= nAnzSizes )
            nPaperSize = 0;     // default if size doesn't match Excel sizes

        nScale = ( UINT16 ) ( ( const SfxUInt16Item& ) rSet.Get( ATTR_PAGE_SCALE ) ).GetValue();

        nPageStart = ( UINT16 ) ( ( const SfxUInt16Item& ) rSet.Get( ATTR_PAGE_FIRSTPAGENO ) ).GetValue();

        const UINT16    nTab = *pExcRoot->pAktTab;

        if( nPageStart )
        {
            if( nTab == 0 || pExcRoot->pDoc->NeedPageResetAfterTab( nTab - 1 ) )
                nGrbit |= 0x0080;   // fUsePage
        }

        if( !( ( const SfxBoolItem& ) rSet.Get( ATTR_PAGE_TOPDOWN ) ).GetValue() )
            nGrbit |= 0x0001;   // ffLeftToRight

        if( !( ( const SfxBoolItem& ) rSet.Get( ATTR_PAGE_NOTES ) ).GetValue() )
            nGrbit |= 0x0020;   // fNotes
    }
    else
    {   // defaults
        nPaperSize = 0;
        nScale = 100;
        nPageStart = 1;
        nGrbit = 0x0001;
    }
}


void ExcSetup::SaveCont( XclExpStream& rStrm )
{
    rStrm   << nPaperSize << nScale << nPageStart
            << ( UINT16 ) 1 << ( UINT16 ) 1             // FitWidth / FitHeight
            << nGrbit
            << ( UINT16 ) 0x012C << ( UINT16 ) 0x012C   // Res / VRes
            << ( UINT32 ) 0 << ( UINT32 ) 0x3FE00000    // Header margin (double)
            << ( UINT32 ) 0 << ( UINT32 ) 0x3FE00000    // Footer margin (double)
            << ( UINT16 ) 1;                            // num of copies
}


UINT16 ExcSetup::GetNum( void ) const
{
    return 0x00A1;
}


ULONG ExcSetup::GetLen( void ) const
{
    return 0x0022;
}



//------------------- class ExcHeaderFooter, class ExcHeader, class ExcFooter -

ExcHeaderFooter::ExcHeaderFooter( RootData* p, const BOOL b ) :
    nLen( 0 ),
    bUnicode( b ),
    ExcRoot( p )
{
}


String ExcHeaderFooter::GetFormatString( USHORT nWhich )
{
    SfxItemSet*                 pItemSet = pExcRoot->pStyleSheetItemSet;

    if( pItemSet )
        return GetFormatString( ( const ScPageHFItem& ) pItemSet->Get( nWhich ) );
    else
        return EMPTY_STRING;
}


String ExcHeaderFooter::GetFormatString( const ScPageHFItem& r )
{
    String                  aRet;

    const EditTextObject*   pLeft = r.GetLeftArea();
    const EditTextObject*   pMid = r.GetCenterArea();
    const EditTextObject*   pRight = r.GetRightArea();

    if( pLeft )
    {
        String              aStr( GetFormatString( *pLeft ) );

        if( aStr.Len() )
        {
            aRet.AppendAscii( "&L" );
            aRet.Append( aStr );
        }
    }

    if( pMid )
    {
        String              aStr( GetFormatString( *pMid ) );

        if( aStr.Len() )
        {
            aRet.AppendAscii( "&C" );
            aRet.Append( aStr );
        }
    }

    if( pRight )
    {
        String              aStr( GetFormatString( *pRight ) );

        if( aStr.Len() )
        {
            aRet.AppendAscii( "&R" );
            aRet.Append( aStr );
        }
    }

    return aRet;
}


String ExcHeaderFooter::GetFormatString( const EditTextObject& rEdTxtObj )
{
    String                  aRet;

    const sal_Unicode       cParSep = 0x0A;
    BOOL                    bFields = FALSE;

    EditEngine&             rEdEng = pExcRoot->GetEdEngForHF();

    rEdEng.SetText( rEdTxtObj );

    UINT16                  nParas = rEdEng.GetParagraphCount();
    if( nParas )
    {
        ESelection          aSel( 0, 0, nParas - 1, rEdEng.GetTextLen( nParas - 1 ) );
        SfxItemSet          aSet( rEdEng.GetAttribs( aSel ) );
        SfxItemState        eFieldState = aSet.GetItemState( EE_FEATURE_FIELD, FALSE );
        if( eFieldState == SFX_ITEM_DONTCARE || eFieldState == SFX_ITEM_SET )
            bFields = TRUE;
    }

    BOOL                bOldUpdateMode = rEdEng.GetUpdateMode();

    rEdEng.SetUpdateMode( TRUE );

    for( UINT16 nPar = 0 ; nPar < nParas ; nPar++ )
    {
        if( nPar )
            aRet += cParSep;

        if( bFields )   // text and fields
        {
            SvUShorts       aPortions;

            rEdEng.GetPortions( nPar, aPortions );

            UINT16          nCnt = aPortions.Count();
            UINT16          nStart = 0;
            UINT16          nPos;

            for( nPos = 0 ; nPos < nCnt ; nPos++ )
            {
                UINT16      nEnd = aPortions.GetObject( nPos );
                ESelection  aSel( nPar, nStart, nPar, nEnd );

                // fields are single characters
                if( nEnd == nStart + 1 )
                {
                    const SfxPoolItem*      pItem;
                    SfxItemSet              aSet = rEdEng.GetAttribs( aSel );
                    if( aSet.GetItemState( EE_FEATURE_FIELD, FALSE, &pItem ) == SFX_ITEM_ON )
                    {
                        const SvxFieldData* pField = ( ( const SvxFieldItem* ) pItem )->GetField();

                        if( pField )
                        {
                            const char*     p;
                            if( pField->ISA( SvxPageField ) )
                                p = "&P";
                            else if( pField->ISA( SvxPagesField ) )
                                p = "&N";
                            else if( pField->ISA( SvxDateField ) )
                                p = "&D";
                            else if( pField->ISA( SvxTimeField ) || pField->ISA( SvxExtTimeField ) )
                                p = "&T";
                            else if( pField->ISA( SvxFileField ) || pField->ISA( SvxExtFileField ) )
                                p = "&F";
                            else if( pField->ISA( SvxTableField ) )
                                p = "&A";
                            else
                                p = "<unsupported field>";

                            aRet.AppendAscii( p );
                        }
                    }
                    else
                        aRet += rEdEng.GetText( aSel );
                }
                else
                    aRet += rEdEng.GetText( aSel );

                nStart = nEnd;
            }
        }
        else        // no fields, text only
        {
            aRet += rEdTxtObj.GetText( nPar );
        }
    }
    rEdEng.SetUpdateMode( bOldUpdateMode );

    return aRet;
}


void ExcHeaderFooter::CalcLen()
{   //! for Biff8 only a prediction, assumes 8-bit string
    if( bUnicode )
        nLen = 3 + Min( aFormatString.Len(), (xub_StrLen) 0xFFFF );
    else
        nLen = 1 + Min( aFormatString.Len(), (xub_StrLen) 255 );
}


void ExcHeaderFooter::SaveCont( XclExpStream& rStrm )
{
    if( bUnicode )          // >= Biff8
        // normal unicode string
        XclExpUniString( aFormatString ).Write( rStrm );
    else                    // < Biff8
        // 8 bit length, max 255 chars
        rStrm.WriteByteString( ByteString( aFormatString, *pExcRoot->pCharset ), 255 );
}


void ExcHeaderFooter::Save( XclExpStream& rStrm )
{
    if( aFormatString.Len() )
        ExcRecord::Save( rStrm );
}


ULONG ExcHeaderFooter::GetLen( void ) const
{
    return nLen;
}




ExcHeader::ExcHeader( RootData* p, const BOOL b ) : ExcHeaderFooter( p, b )
{
    aFormatString = GetFormatString( ATTR_PAGE_HEADERRIGHT );
    CalcLen();
}


UINT16 ExcHeader::GetNum( void ) const
{
    return 0x0014;
}




ExcFooter::ExcFooter( RootData* p, const BOOL b ) : ExcHeaderFooter( p, b )
{
    aFormatString = GetFormatString( ATTR_PAGE_FOOTERRIGHT );
    CalcLen();
}


UINT16 ExcFooter::GetNum( void ) const
{
    return 0x0015;
}



//----------------------------------------------------- class ExcPrintheaders -

ExcPrintheaders::ExcPrintheaders( SfxItemSet* p ) : ExcBoolRecord( p, ATTR_PAGE_HEADERS, TRUE )
{
}


UINT16 ExcPrintheaders::GetNum( void ) const
{
    return 0x002A;
}



//--------------------------------------------------- class ExcPrintGridlines -

ExcPrintGridlines::ExcPrintGridlines( SfxItemSet* p ) : ExcBoolRecord( p, ATTR_PAGE_GRID, TRUE )
{
}


UINT16 ExcPrintGridlines::GetNum( void ) const
{
    return 0x002B;
}



//---------------------------------------------------------- class ExcHcenter -

ExcHcenter::ExcHcenter( SfxItemSet* p ) : ExcBoolRecord( p, ATTR_PAGE_HORCENTER, FALSE )
{
}


UINT16 ExcHcenter::GetNum( void ) const
{
    return 0x0083;
}



//---------------------------------------------------------- class ExcVcenter -

ExcVcenter::ExcVcenter( SfxItemSet* p ) : ExcBoolRecord( p, ATTR_PAGE_VERCENTER, FALSE )
{
}


UINT16 ExcVcenter::GetNum( void ) const
{
    return 0x0084;
}



//---------------------------------------------------------------- AutoFilter -

UINT16 ExcFilterMode::GetNum() const
{
    return 0x009B;
}

ULONG ExcFilterMode::GetLen() const
{
    return 0;
}




ExcAutoFilterInfo::~ExcAutoFilterInfo()
{
}

void ExcAutoFilterInfo::SaveCont( XclExpStream& rStrm )
{
    rStrm << nCount;
}

UINT16 ExcAutoFilterInfo::GetNum() const
{
    return 0x009D;
}

ULONG ExcAutoFilterInfo::GetLen() const
{
    return 2;
}




ExcFilterCondition::ExcFilterCondition() :
        nType( EXC_AFTYPE_NOTUSED ),
        nOper( EXC_AFOPER_EQUAL ),
        fVal( 0.0 ),
        pText( NULL )
{
}

ExcFilterCondition::~ExcFilterCondition()
{
    if( pText )
        delete pText;
}

ULONG ExcFilterCondition::GetTextBytes() const
{
    return pText ? 1 + pText->GetBufferByteCount() : 0;
}

void ExcFilterCondition::SetCondition( UINT8 nTp, UINT8 nOp, double fV, String* pT )
{
    nType = nTp;
    nOper = nOp;
    fVal = fV;

    if( pText )
        delete pText;
    pText = pT ? new XclExpUniString( *pT, 255 ) : NULL;
}

void ExcFilterCondition::Save( XclExpStream& rStrm )
{
    rStrm << nType << nOper;
    switch( nType )
    {
        case EXC_AFTYPE_DOUBLE:
            rStrm << fVal;
        break;
        case EXC_AFTYPE_STRING:
            DBG_ASSERT( pText, "ExcFilterCondition::Save() -- pText is NULL!" );
            rStrm << (UINT32)0 << (UINT8) pText->GetLen() << (UINT16)0 << (UINT8)0;
        break;
        case EXC_AFTYPE_BOOLERR:
            rStrm << (UINT8)0 << (UINT8)((fVal != 0) ? 1 : 0) << (UINT32)0 << (UINT16)0;
        break;
        default:
            rStrm << (UINT32)0 << (UINT32)0;
    }
}

void ExcFilterCondition::SaveText( XclExpStream& rStrm )
{
    if( nType == EXC_AFTYPE_STRING )
    {
        DBG_ASSERT( pText, "ExcFilterCondition::SaveText() -- pText is NULL!" );
        pText->WriteFlags( rStrm );
        pText->WriteBuffer( rStrm );
    }
}




ExcAutoFilter::ExcAutoFilter( UINT16 nC ) :
        nCol( nC ),
        nFlags( 0 )
{
}

BOOL ExcAutoFilter::AddCondition( ScQueryConnect eConn, UINT8 nType, UINT8 nOp,
                                    double fVal, String* pText, BOOL bSimple )
{
    if( !aCond[ 1 ].IsEmpty() )
        return FALSE;

    UINT16 nInd = aCond[ 0 ].IsEmpty() ? 0 : 1;

    if( nInd == 1 )
        nFlags |= (eConn == SC_OR) ? EXC_AFFLAG_OR : EXC_AFFLAG_AND;
    if( bSimple )
        nFlags |= (nInd == 0) ? EXC_AFFLAG_SIMPLE1 : EXC_AFFLAG_SIMPLE2;

    aCond[ nInd ].SetCondition( nType, nOp, fVal, pText );
    return TRUE;
}

BOOL ExcAutoFilter::AddEntry( RootData& rRoot, const ScQueryEntry& rEntry )
{
    BOOL    bConflict = FALSE;
    String  sText;

    if( rEntry.pStr )
        sText.Assign( *rEntry.pStr );

    BOOL bLen = TRUEBOOL( sText.Len() );

    // empty/nonempty fields
    if( !bLen && (rEntry.nVal == SC_EMPTYFIELDS) )
        bConflict = !AddCondition( rEntry.eConnect, EXC_AFTYPE_EMPTY, EXC_AFOPER_NONE, 0.0, NULL, TRUE );
    else if( !bLen && (rEntry.nVal == SC_NONEMPTYFIELDS) )
        bConflict = !AddCondition( rEntry.eConnect, EXC_AFTYPE_NOTEMPTY, EXC_AFOPER_NONE, 0.0, NULL, TRUE );
    // other conditions
    else
    {
        double  fVal    = 0.0;
        ULONG   nIndex  = 0;
        BOOL    bIsNum  = bLen ? rRoot.pDoc->GetFormatTable()->IsNumberFormat( sText, nIndex, fVal ) : TRUE;
        String* pText   = bIsNum ? NULL : &sText;

        // top10 flags
        UINT16 nNewFlags = 0x0000;
        switch( rEntry.eOp )
        {
            case SC_TOPVAL:
                nNewFlags = (EXC_AFFLAG_TOP10 | EXC_AFFLAG_TOP10TOP);
            break;
            case SC_BOTVAL:
                nNewFlags = EXC_AFFLAG_TOP10;
            break;
            case SC_TOPPERC:
                nNewFlags = (EXC_AFFLAG_TOP10 | EXC_AFFLAG_TOP10TOP | EXC_AFFLAG_TOP10PERC);
            break;
            case SC_BOTPERC:
                nNewFlags = (EXC_AFFLAG_TOP10 | EXC_AFFLAG_TOP10PERC);
            break;
        }
        BOOL bNewTop10 = TRUEBOOL( nNewFlags & EXC_AFFLAG_TOP10 );

        bConflict = HasTop10() && bNewTop10;
        if( !bConflict )
        {
            if( bNewTop10 )
            {
                if( fVal < 0 )      fVal = 0;
                if( fVal >= 501 )   fVal = 500;
                nFlags |= (nNewFlags | (UINT16)(fVal) << 7);
            }
            // normal condition
            else
            {
                UINT8 nType = bIsNum ? EXC_AFTYPE_DOUBLE : EXC_AFTYPE_STRING;
                UINT8 nOper = EXC_AFOPER_NONE;

                switch( rEntry.eOp )
                {
                    case SC_EQUAL:          nOper = EXC_AFOPER_EQUAL;           break;
                    case SC_LESS:           nOper = EXC_AFOPER_LESS;            break;
                    case SC_GREATER:        nOper = EXC_AFOPER_GREATER;         break;
                    case SC_LESS_EQUAL:     nOper = EXC_AFOPER_LESSEQUAL;       break;
                    case SC_GREATER_EQUAL:  nOper = EXC_AFOPER_GREATEREQUAL;    break;
                    case SC_NOT_EQUAL:      nOper = EXC_AFOPER_NOTEQUAL;        break;
                }
                bConflict = !AddCondition( rEntry.eConnect, nType, nOper, fVal, pText );
            }
        }
    }
    return bConflict;
}

void ExcAutoFilter::SaveCont( XclExpStream& rStrm )
{
    rStrm << nCol << nFlags;
    aCond[ 0 ].Save( rStrm );
    aCond[ 1 ].Save( rStrm );
    aCond[ 0 ].SaveText( rStrm );
    aCond[ 1 ].SaveText( rStrm );
}

UINT16 ExcAutoFilter::GetNum() const
{
    return 0x009E;
}

ULONG ExcAutoFilter::GetLen() const
{
    return 24 + aCond[ 0 ].GetTextBytes() + aCond[ 1 ].GetTextBytes();
}




ExcAutoFilterRecs::ExcAutoFilterRecs( RootData& rRoot, UINT16 nTab ) :
        pFilterMode( NULL ),
        pFilterInfo( NULL )
{
    ScDBCollection& rDBColl = *rRoot.pDoc->GetDBCollection();

    // search for first DB-range with filter
    UINT16      nIndex  = 0;
    BOOL        bFound  = FALSE;
    BOOL        bAdvanced;
    ScDBData*   pData   = NULL;
    ScRange     aRange;
    ScRange     aAdvRange;
    while( (nIndex < rDBColl.GetCount()) && !bFound )
    {
        pData = rDBColl[ nIndex ];
        if( pData )
        {
            pData->GetArea( aRange );
            bAdvanced = pData->GetAdvancedQuerySource( aAdvRange );
            bFound = (aRange.aStart.Tab() == nTab) &&
                (pData->HasQueryParam() || pData->HasAutoFilter() || bAdvanced);
        }
        if( !bFound )
            nIndex++;
    }

    if( pData && bFound )
    {
        ScQueryParam    aParam;
        pData->GetQueryParam( aParam );

        ScRange aRange( aParam.nCol1, aParam.nRow1, aParam.nTab,
                        aParam.nCol2, aParam.nRow2, aParam.nTab );
        UINT16  nColCnt = aParam.nCol2 - aParam.nCol1 + 1;

        rRoot.pNameList->Append( new ExcName( &rRoot, aRange, EXC_BUILTIN_AUTOFILTER, TRUE ) );

        // advanced filter
        if( bAdvanced )
        {
            // filter criteria, excel allows only same table
            if( aAdvRange.aStart.Tab() == nTab )
                rRoot.pNameList->Append(
                    new ExcName( &rRoot, aAdvRange, EXC_BUILTIN_CRITERIA ) );

            // filter destination range, excel allows only same table
            if( !aParam.bInplace )
            {
                ScRange aDestRange( aParam.nDestCol, aParam.nDestRow, aParam.nDestTab );
                aDestRange.aEnd.IncCol( nColCnt - 1 );
                if( aDestRange.aStart.Tab() == nTab )
                    rRoot.pNameList->Append(
                        new ExcName( &rRoot, aDestRange, EXC_BUILTIN_EXTRACT ) );
            }

            pFilterMode = new ExcFilterMode;
        }
        // AutoFilter
        else
        {
            BOOL    bConflict   = FALSE;
            BOOL    bContLoop   = TRUE;
            BOOL    bHasOr      = FALSE;
            UINT16  nFirstField = aParam.GetEntry( 0 ).nField;

            // create AUTOFILTER records for filtered columns
            for( UINT16 nEntry = 0; !bConflict && bContLoop && (nEntry < aParam.GetEntryCount()); nEntry++ )
            {
                const ScQueryEntry& rEntry  = aParam.GetEntry( nEntry );

                bContLoop = rEntry.bDoQuery;
                if( bContLoop )
                {
                    ExcAutoFilter* pFilter = GetByCol( rEntry.nField - aRange.aStart.Col() );

                    if( nEntry > 0 )
                        bHasOr |= (rEntry.eConnect == SC_OR);

                    bConflict = (nEntry > 1) && bHasOr;
                    if( !bConflict )
                        bConflict = (nEntry == 1) && (rEntry.eConnect == SC_OR) &&
                                    (nFirstField != rEntry.nField);
                    if( !bConflict )
                        bConflict = pFilter->AddEntry( rRoot, rEntry );
                }
            }

            // additional tests for conflicts
            for( ExcAutoFilter* pFilter = _First(); !bConflict && pFilter; pFilter = _Next() )
                bConflict = pFilter->HasCondition() && pFilter->HasTop10();

            if( bConflict )
                DeleteList();

            if( List::Count() )
                pFilterMode = new ExcFilterMode;
            pFilterInfo = new ExcAutoFilterInfo( nColCnt );
            AddObjRecs( rRoot, aRange.aStart, nColCnt );
        }
    }
}

ExcAutoFilterRecs::~ExcAutoFilterRecs()
{
    if( pFilterMode )
        delete pFilterMode;
    if( pFilterInfo )
        delete pFilterInfo;
    DeleteList();
}

void ExcAutoFilterRecs::DeleteList()
{
    for( ExcAutoFilter* pFilter = _First(); pFilter; pFilter = _Next() )
        delete pFilter;
    List::Clear();
}

ExcAutoFilter* ExcAutoFilterRecs::GetByCol( UINT16 nCol )
{
    ExcAutoFilter* pFilter;
    for( pFilter = _First(); pFilter; pFilter = _Next() )
        if( pFilter->GetCol() == nCol )
            return pFilter;
    pFilter = new ExcAutoFilter( nCol );
    Append( pFilter );
    return pFilter;
}

BOOL ExcAutoFilterRecs::IsFiltered( UINT16 nCol )
{
    ExcAutoFilter* pFilter;
    for( pFilter = _First(); pFilter; pFilter = _Next() )
        if( pFilter->GetCol() == nCol )
            return TRUE;
    return FALSE;
}

void ExcAutoFilterRecs::AddObjRecs( RootData& rRoot, const ScAddress& rPos, UINT16 nCols )
{
    ScAddress aAddr( rPos );
    for( UINT16 nObj = 0; nObj < nCols; nObj++ )
    {
        XclObjDropDown* pObj = new XclObjDropDown( rRoot, aAddr, IsFiltered( nObj ) );
        rRoot.pObjRecs->Add( pObj );
        aAddr.IncCol( 1 );
    }
}

void ExcAutoFilterRecs::Save( XclExpStream& rStrm )
{
    if( pFilterMode )
        pFilterMode->Save( rStrm );
    if( pFilterInfo )
        pFilterInfo->Save( rStrm );
    for( ExcAutoFilter* pFilter = _First(); pFilter; pFilter = _Next() )
        pFilter->Save( rStrm );
}



//----------------------------------------------------------- class ExcMargin -

ExcMargin::ExcMargin( long n, IMPEXC_MARGINSIDE e ) : nVal( ( n < 0 )? 0 : ( UINT16 ) n )
{
    switch( e )
    {
        case IMPEXC_MARGINSIDE_LEFT:    nId = 0x26;     break;
        case IMPEXC_MARGINSIDE_RIGHT:   nId = 0x27;     break;
        case IMPEXC_MARGINSIDE_TOP:     nId = 0x28;     break;
        case IMPEXC_MARGINSIDE_BOTTOM:  nId = 0x29;     break;
        default:
            nId = 0x26;
            DBG_WARNING( "ExcMargin::ExcMargin(): impossible enum state!" );
    }
}


void ExcMargin::SaveCont( XclExpStream& rStrm )
{
    double  f = nVal;

    f /= 1440.0;

    rStrm << f;
}


UINT16 ExcMargin::GetNum() const
{
    return nId;
}


ULONG ExcMargin::GetLen() const
{
    return 8;
}



//---------------------------------------------------- class XclExpPageBreaks -

XclExpPageBreaks::XclExpPageBreaks( RootData& rRootData, UINT16 nScTab, ExcPBOrientation eOrient ) :
    nRecNum( (eOrient == pbHorizontal) ? 0x001B : 0x001A )
{
    BYTE nFlags;
    if( eOrient == pbHorizontal )
    {
        for( USHORT nIndex = 0; nIndex <= MAXROW; nIndex++ )
        {
            nFlags = rRootData.pDoc->GetRowFlags( nIndex, nScTab );
            if( nFlags & CR_MANUALBREAK )
                aPageBreaks.Append( nIndex );
        }
    }
    else
    {
        for( USHORT nIndex = 0; nIndex <= MAXCOL; nIndex++ )
        {
            nFlags = rRootData.pDoc->GetColFlags( nIndex, nScTab );
            if( nFlags & CR_MANUALBREAK )
                aPageBreaks.Append( nIndex );
        }
    }
}

XclExpPageBreaks::~XclExpPageBreaks()
{
}

void XclExpPageBreaks::Save( XclExpStream& rStrm )
{
    if( aPageBreaks.Count() )
        ExcRecord::Save( rStrm );
}

void XclExpPageBreaks::SaveCont( XclExpStream& rStrm )
{
    rStrm << (UINT16) aPageBreaks.Count();
    for( UINT32 nIndex = 0; nIndex < aPageBreaks.Count(); nIndex++ )
        rStrm << aPageBreaks.Get( nIndex );
}

UINT16 XclExpPageBreaks::GetNum() const
{
    return nRecNum;
}

ULONG XclExpPageBreaks::GetLen() const
{
    return 2 + 2 * aPageBreaks.Count();
}



//------------------------ class ExcArray, class ExcArrays, class ExcShrdFmla -

void ExcArray::SetColRow( UINT8 nCol, UINT16 nRow, UINT32 nId )
{
    if( nId == 0xFFFFFFFF )
    {
        nID = nCol;
        nID <<= 16;
        nID += nRow;
        nID <<= 8;
    }
    else
        nID = nId;

    nFirstRow = nLastRow = nRow;
    nFirstCol = nLastCol = nCol;
}


void ExcArray::SaveCont( XclExpStream& rStrm )
{
    rStrm   << nFirstRow << nLastRow << nFirstCol << nLastCol
            << ( UINT8 ) 0 << nID << ( UINT8 ) 0xFE << nFormLen;

    if( pData )
        rStrm.Write( pData, nFormLen );
}


ExcArray::ExcArray( const sal_Char* p, UINT16 n, UINT8 nCol, UINT16 nRow )
{
    SetColRow( nCol, nRow );

    if( p && n )
    {
        pData = new sal_Char[ n ];
        nFormLen = n;
        memcpy( pData, p, n );
    }
    else
    {
        pData = NULL;
        nFormLen = 0;
    }
}


ExcArray::ExcArray( const ExcUPN& rUPN, UINT8 nCol, UINT16 nRow )
{
    SetColRow( nCol, nRow );

    nFormLen = rUPN.GetLen();

    if( nFormLen )
    {
        pData = new sal_Char[ nFormLen ];
        memcpy( pData, rUPN.GetCode(), nFormLen );
    }
    else
        pData = NULL;
}


ExcArray::ExcArray( UINT8 nCol, UINT16 nRow, UINT32 nId )
{
    SetColRow( nCol, nRow, nId );

    nFormLen = 0;
    pData = NULL;
}


ExcArray::~ExcArray()
{
    if( pData )
        delete pData;
}


UINT16 ExcArray::GetNum() const
{
    return 0x0221;
}


ULONG ExcArray::GetLen() const
{
    return nFormLen + 14;
}


BOOL ExcArray::AppendBy( const ExcArray& r )
{
    if( nID != r.nID )
        return FALSE;

    BOOL            bRet;

    const UINT16    nNewRow = nLastRow + 1;
    const UINT16    nNewCol = nLastCol + 1;

    if( nNewRow == r.nFirstRow && nFirstCol <= r.nFirstCol && nNewCol >= r.nFirstCol )
    {
        nLastRow = nNewRow;

        if( nNewCol == r.nFirstCol )
            nLastCol = ( UINT8 ) nNewCol;

        bRet = TRUE;
    }
    else if( nNewCol == r.nFirstCol && nFirstRow <= r.nFirstRow && nNewRow >= r.nLastRow )
    {
        nLastCol = ( UINT8 ) nNewCol;

        if( nNewRow == r.nFirstRow )
            nLastRow = nNewRow;

        bRet = TRUE;
    }
    else
        bRet = FALSE;

    return bRet;
}


BOOL ExcArray::AppendBy( UINT8 nFCol, UINT16 nFRow, UINT8 nLCol, UINT16 nLRow )
{
    if( nFCol != nFirstCol || nFRow != nFirstRow )
        return FALSE;

    BOOL    bRet;

    const UINT16    nNewRow = nLastRow + 1;
    const UINT16    nNewCol = nLastCol + 1;

    if( nNewRow == nLRow && nFirstCol <= nFCol && nNewCol >= nLCol )
    {
        nLastRow = nNewRow;

        if( nNewCol == nFCol )
            nLastCol = ( UINT8 ) nNewCol;

        bRet = TRUE;
    }
    else if( nNewCol == nLCol && nFirstRow <= nFRow && nNewRow >= nLRow )
    {
        nLastRow = ( UINT8 ) nNewRow;

        if( nNewRow == nFRow )
            nLastRow = nNewRow;

        bRet = TRUE;
    }
    else
        bRet = FALSE;

    return bRet;
}




ExcArrays::ExcArrays( void )
{
}


ExcArrays::~ExcArrays()
{
}


BOOL ExcArrays::Insert( ExcArray* p )
{
    ExcArray*   pAct = ( ExcArray* ) List::First();

    while( pAct )
    {
        if( pAct->AppendBy( *p ) )
            return FALSE;

        pAct = ( ExcArray* ) List::Next();
    }

    Append( p );

    return TRUE;
}


BOOL ExcArrays::Extend( UINT8 nStartCol, UINT16 nStartRow, UINT8 nEndCol, UINT16 nEndRow )
{
    ExcArray*   pAct = ( ExcArray* ) List::First();

    while( pAct )
    {
        if( pAct->AppendBy( nStartCol, nStartRow, nEndCol, nEndRow ) )
            return TRUE;

        pAct = ( ExcArray* ) List::Next();
    }

    return FALSE;
}



void ExcShrdFmla::SaveCont( XclExpStream& rStrm )
{
    rStrm << nFirstRow << nLastRow << nFirstCol << nLastCol << ( UINT16 ) 0x0000 << nFormLen;

    if( pData )
        rStrm.Write( pData, nFormLen );
}


ExcShrdFmla::ExcShrdFmla( const sal_Char* p, UINT16 n, const ScRange& r ) :
    ExcArray( p, n, ( UINT8 ) r.aStart.Col(), r.aStart.Row() )
{
}


ExcShrdFmla::~ExcShrdFmla()
{
}


UINT16 ExcShrdFmla::GetNum() const
{
    return 0x04BC;
}


ULONG ExcShrdFmla::GetLen() const
{
    return 10 + nFormLen;
}



//--------------------------- class XclExpTableOp, class XclExpTableOpManager -

XclExpTableOp::XclExpTableOp(
        ExcFormula& rFormula,
        const ScAddress& rColFirstPos,
        const ScAddress& rRowFirstPos,
        USHORT nNewMode ) :
    nMode( nNewMode ),
    nColInpCol( rColFirstPos.Col() ),
    nColInpRow( rColFirstPos.Row() ),
    nRowInpCol( rRowFirstPos.Col() ),
    nRowInpRow( rRowFirstPos.Row() ),
    bIsValid( FALSE )
{
    nFirstCol = nLastCol = nNextCol = rFormula.GetPosition().Col();
    nFirstRow = nLastRow = rFormula.GetPosition().Row();
    Append( &rFormula );
}

XclExpTableOp::~XclExpTableOp()
{
}

BOOL XclExpTableOp::IsAppendable( const ScAddress& rPos )
{
    return  ((rPos.Col() == nLastCol + 1) && (rPos.Row() == nFirstRow)) ||
            ((rPos.Col() == nNextCol) && (rPos.Row() == nLastRow + 1));
}

BOOL XclExpTableOp::CheckPosition(
    const ScAddress& rPos,
    const ScAddress& rFmlaPos,
    const ScAddress& rColFirstPos, const ScAddress& rColRelPos,
    const ScAddress& rRowFirstPos, const ScAddress& rRowRelPos,
    BOOL bMode2 )
{
    BOOL bRet = FALSE;

    if( ((nMode == 2) == bMode2) &&
        (rPos.Tab() == rFmlaPos.Tab()) &&
        (nColInpCol == rColFirstPos.Col()) &&
        (nColInpRow == rColFirstPos.Row()) &&
        (rPos.Tab() == rColFirstPos.Tab()) &&
        (rPos.Tab() == rColRelPos.Tab()) )
    {
        if( nMode == 0 )
        {
            bRet =  (rPos.Col() == rFmlaPos.Col()) &&
                    (nFirstRow == rFmlaPos.Row() + 1) &&
                    (nFirstCol == rColRelPos.Col() + 1) &&
                    (rPos.Row() == rColRelPos.Row());
        }
        else if( nMode == 1 )
        {
            bRet =  (nFirstCol == rFmlaPos.Col() + 1) &&
                    (rPos.Row() == rFmlaPos.Row()) &&
                    (rPos.Col() == rColRelPos.Col()) &&
                    (nFirstRow == rColRelPos.Row() + 1);
        }
        else if( nMode == 2 )
        {
            bRet =  (nFirstCol == rFmlaPos.Col() + 1) &&
                    (nFirstRow == rFmlaPos.Row() + 1) &&
                    (nFirstCol == rColRelPos.Col() + 1) &&
                    (rPos.Row() == rColRelPos.Row()) &&
                    (nRowInpCol == rRowFirstPos.Col()) &&
                    (nRowInpRow == rRowFirstPos.Row()) &&
                    (rPos.Tab() == rRowFirstPos.Tab()) &&
                    (rPos.Col() == rRowRelPos.Col()) &&
                    (nFirstRow == rRowRelPos.Row() + 1) &&
                    (rPos.Tab() == rRowRelPos.Tab());
        }
    }

    return bRet;
}

BOOL XclExpTableOp::CheckFirstPosition(
    const ScAddress& rPos,
    const ScAddress& rFmlaPos,
    const ScAddress& rColFirstPos, const ScAddress& rColRelPos,
    const ScAddress& rRowFirstPos, const ScAddress& rRowRelPos,
    BOOL bMode2, USHORT& rnMode )
{
    BOOL bRet = FALSE;

    if( (rPos.Tab() == rFmlaPos.Tab()) &&
        (rPos.Tab() == rColFirstPos.Tab()) &&
        (rPos.Tab() == rColRelPos.Tab()) )
    {
        if( bMode2 )
        {
            rnMode = 2;
            bRet =  (rPos.Col() == rFmlaPos.Col() + 1) &&
                    (rPos.Row() == rFmlaPos.Row() + 1) &&
                    (rPos.Col() == rColRelPos.Col() + 1) &&
                    (rPos.Row() == rColRelPos.Row()) &&
                    (rPos.Tab() == rRowFirstPos.Tab()) &&
                    (rPos.Col() == rRowRelPos.Col()) &&
                    (rPos.Row() == rRowRelPos.Row() + 1) &&
                    (rPos.Tab() == rRowRelPos.Tab());
        }
        else if( (rPos.Col() == rFmlaPos.Col()) &&
                (rPos.Row() == rFmlaPos.Row() + 1) &&
                (rPos.Col() == rColRelPos.Col() + 1) &&
                (rPos.Row() == rColRelPos.Row()) )
        {
            rnMode = 0;
            bRet = TRUE;
        }
        else if( (rPos.Col() == rFmlaPos.Col() + 1) &&
                (rPos.Row() == rFmlaPos.Row()) &&
                (rPos.Col() == rColRelPos.Col()) &&
                (rPos.Row() == rColRelPos.Row() + 1) )
        {
            rnMode = 1;
            bRet = TRUE;
        }
    }

    return bRet;
}

void XclExpTableOp::InsertCell( ExcFormula& rFormula )
{
    const ScAddress& rPos = rFormula.GetPosition();
    if( (rPos.Col() == nLastCol + 1) && (rPos.Row() == nFirstRow) )     // next cell of first row
    {
        nLastCol++;
        Append( &rFormula );
    }
    else if( (rPos.Col() == nNextCol) && (rPos.Row() == nLastRow + 1) ) // next cell of next row
    {
        nNextCol++;
        Append( &rFormula );

        if( nNextCol > nLastCol )   // next row is valid -> add to range
        {
            nLastRow++;
            nNextCol = nFirstCol;
        }
    }
}

void XclExpTableOp::UpdateCells()
{
    if( nMode == 0 )
        bIsValid =  (nColInpCol + 1 < nFirstCol) || (nColInpCol > nLastCol) ||
                    (nColInpRow < nFirstRow) || (nColInpRow > nLastRow);
    else if( nMode == 1 )
        bIsValid =  (nColInpCol < nFirstCol) || (nColInpCol > nLastCol) ||
                    (nColInpRow + 1 < nFirstRow) || (nColInpRow > nLastRow);
    else if( nMode == 2 )
        bIsValid =  ((nColInpCol + 1 < nFirstCol) || (nColInpCol > nLastCol) ||
                    (nColInpRow + 1 < nFirstRow) || (nColInpRow > nLastRow)) &&
                    ((nRowInpCol + 1 < nFirstCol) || (nRowInpCol > nLastCol) ||
                    (nRowInpRow + 1 < nFirstRow) || (nRowInpRow > nLastRow));

    if( !bIsValid ) return;

    for( ExcFormula* pFmla = _First(); pFmla; pFmla = _Next() )
    {
        const ScAddress& rPos = pFmla->GetPosition();
        if( (nFirstCol <= rPos.Col()) && (rPos.Col() <= nLastCol) &&
            (nFirstRow <= rPos.Row()) && (rPos.Row() <= nLastRow) )
            pFmla->SetTableOp( nFirstCol, nFirstRow );
    }
}

void XclExpTableOp::SaveCont( XclExpStream& rStrm )
{
    rStrm   << (UINT16) nFirstRow << (UINT16) nLastRow
            << (UINT8) nFirstCol << (UINT8) nLastCol;
    if( nMode == 2 )
        rStrm   << (UINT16) EXC_TABOP_BOTH
                << (UINT16) nRowInpRow << (UINT16) nRowInpCol
                << (UINT16) nColInpRow << (UINT16) nColInpCol;
    else
        rStrm   << (UINT16)((nMode == 1) ? EXC_TABOP_ROW : 0x0000)
                << (UINT16) nColInpRow << (UINT16) nColInpCol       // ref to col AND row stored in nColInp***
                << (UINT16) 0 << (UINT16) 0;
}

void XclExpTableOp::Save( XclExpStream& rStrm )
{
    if( bIsValid )
        ExcRecord::Save( rStrm );
}

UINT16 XclExpTableOp::GetNum() const
{
    return 0x0236;
}

ULONG XclExpTableOp::GetLen() const
{
    return 16;
}



// do not delete the records -> stored in and deleted by table record list
XclExpTableOpManager::~XclExpTableOpManager()
{
}

XclExpTableOp* XclExpTableOpManager::InsertCell( const ScTokenArray* pTokenArray, ExcFormula& rFormula )
{
    XclExpTableOp*  pTabOp = NULL;
    ScAddress       aFmlaPos;
    ScAddress       aColFirst;
    ScAddress       aColRel;
    ScAddress       aRowFirst;
    ScAddress       aRowRel;
    BOOL            bMode2;

    if( pTokenArray && pTokenArray->GetTableOpRefs( aFmlaPos, aColFirst, aColRel, aRowFirst, aRowRel, bMode2 ) )
    {
        const ScAddress& rPos = rFormula.GetPosition();

        pTabOp = _First();
        BOOL bFound = FALSE;
        while( pTabOp && !bFound )
        {
            bFound = pTabOp->IsAppendable( rPos );
            if( !bFound )
                pTabOp = _Next();
        }

        if( bFound )
        {
            if( pTabOp->CheckPosition( rPos, aFmlaPos, aColFirst, aColRel, aRowFirst, aRowRel, bMode2 ) )
                pTabOp->InsertCell( rFormula );
            pTabOp = NULL;
        }
        else
        {
            USHORT nMode;
            if( XclExpTableOp::CheckFirstPosition( rPos, aFmlaPos, aColFirst, aColRel, aRowFirst, aRowRel, bMode2, nMode ) )
            {
                pTabOp = new XclExpTableOp( rFormula, aColFirst, aRowFirst, nMode );
                List::Insert( pTabOp, (ULONG) 0 );
            }
        }
    }
    return pTabOp;
}

void XclExpTableOpManager::UpdateCells()
{
    for( XclExpTableOp* pTabOp = _First(); pTabOp; pTabOp = _Next() )
        pTabOp->UpdateCells();
}

