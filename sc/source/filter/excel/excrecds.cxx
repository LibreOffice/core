/*************************************************************************
 *
 *  $RCSfile: excrecds.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: gt $ $Date: 2000-09-22 14:54:25 $
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
#include "patattr.hxx"
#include "attrib.hxx"
#include "dociter.hxx"
#include "progress.hxx"
#include "rangenam.hxx"
#include "dbcolect.hxx"
#include "stlsheet.hxx"
#include "stlpool.hxx"
#include "editutil.hxx"

#include "excrecds.hxx"
#include "root.hxx"
#include "fontbuff.hxx"
#include "excupn.hxx"

#include "xcl97exp.hxx"
#include "xcl97rec.hxx"




//---------------------------------------------------- class ExcETabNumBuffer -

ExcETabNumBuffer::ExcETabNumBuffer( ScDocument& rDoc )
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
    DBG_ASSERT( nScTab < nScCnt, "ExcETabNumBuffer::IsExternal() - out of range!" );
    return TRUEBOOL( pBuffer[ nScTab ] & EXC_TABBUF_FLAGEXT );
}

BOOL ExcETabNumBuffer::IsExportTable( UINT16 nScTab ) const
{
    DBG_ASSERT( nScTab < nScCnt, "ExcETabNumBuffer::IsExportTable() - out of range!" );
    return (pBuffer[ nScTab ] & EXC_TABBUF_MASKFLAGS) == 0;
}

UINT16 ExcETabNumBuffer::GetExcTable( UINT16 nScTab ) const
{
    if( nScTab < nScCnt )
        return (UINT16)(pBuffer[ nScTab ] & EXC_TABBUF_MASKTAB);
    return EXC_TABBUF_INVALID;
}




//----------------------------------------------------------- class ExcRecord -
const UINT16    ExcRecord::nIgnore = 0;

//------------------------------------------------------------- class ExcCell -
UsedAttrList*   ExcCell::pXFRecs = NULL;
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
const UINT16    ExcDummy_00::nMyLen = sizeof( ExcDummy_00::pMyData );
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
const UINT16    ExcDummy_040::nMyLen = sizeof( ExcDummy_040::pMyData );
//  ( UINT16 ) 0x02 + 0x02 + 0x02 + 0x12 + 0x02 + 0x02 + 6 * 4;

const BYTE      ExcDummy_041::pMyData[] = {
    0x0e, 0x00, 0x02, 0x00, 0x01, 0x00,                     // PRECISION
    0xda, 0x00, 0x02, 0x00, 0x00, 0x00                      // BOOKBOOL
};
const UINT16    ExcDummy_041::nMyLen = sizeof( ExcDummy_041::pMyData );
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
const UINT16    ExcDummy_01::nMyLen = sizeof( ExcDummy_01::pMyData );   //( ( UINT16 ) 0x14 + 4 ) * 5;


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
const UINT16    ExcDummy_Fm::nMyLen = sizeof( ExcDummy_Fm::pMyData );
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
const UINT16    ExcDummy_XF::nMyLen = sizeof( ExcDummy_XF::pMyData );
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
const UINT16    ExcDummy_Style::nMyLen = sizeof( ExcDummy_Style::pMyData );
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
    0x82, 0x00, 0x02, 0x00, 0x00, 0x00,                     // GRIDSET
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
const UINT16    ExcDummy_02::nMyLen = sizeof( ExcDummy_02::pMyData );
//  ( UINT16 ) 0x02 + 0x02 + 0x02 + 0x02 +
//  0x08 + 0x02 + 0x02 + 0x02 + 0x02 + 0x08 + 0x04 + 0x04 + 0x02 + /*0x03 +*/
//  /*0x09 +*/ 0x02 + 0x02 + /*0x22 +*/ /*0x02 + */15 * 4;


//--------------------------------------------------------- class ExcDummy_03 -
const BYTE      ExcDummy_03::pMyData[] = {
    0x1d, 0x00, 0x0f, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,   // SELECTION
    0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00
};
const UINT16    ExcDummy_03::nMyLen = sizeof( ExcDummy_03::pMyData );


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

void ExcRecord::SaveCont( SvStream& rStr )
{
}


void ExcRecord::_Save( SvStream& rStr )
{
    rStr << ( UINT16 ) GetNum() << ( UINT16 ) GetLen();
}


ExcRecord::~ExcRecord()
{
}


void ExcRecord::Save( SvStream& rStr )
{
    _Save( rStr );
    SaveCont( rStr );
}




ExcRecordList::~ExcRecordList()
{
    ExcRecord*          p = First();

    while( p )
    {
        delete p;
        p = Next();
    }
}


UINT16 ExcRecordList::GetNum( void ) const
{
    return 0;       // doesn't matter, dummy
}


UINT16 ExcRecordList::GetLen( void ) const
{
    const UINT32        nEnd = Count();
    UINT32              n;
    UINT16              nLen = 0;

    for( n = 0 ; n < nEnd ; n++ )
        nLen += Get( n )->GetLen();

    return nLen;
}


void ExcRecordList::Save( SvStream& r )
{
    ExcRecord*          p = First();

    while( p )
    {
        p->Save( r );
        p = Next();
    }
}




//--------------------------------------------------------- class ExcEmptyRec -

void ExcEmptyRec::_Save( SvStream& rStrm )
{   }

void ExcEmptyRec::Save( SvStream& rStrm )
{   }

UINT16 ExcEmptyRec::GetNum() const
{
    return 0;
}

UINT16 ExcEmptyRec::GetLen() const
{
    return 0;
}



//--------------------------------------------------------- class ExcDummyRec -

void ExcDummyRec::Save( SvStream& rStr )
{
    rStr.Write( GetData(), GetLen() );
}


UINT16 ExcDummyRec::GetNum( void ) const
{
    return 0x0000;  // weil Banane
}




ExcBof_Base::ExcBof_Base( void )
{
    nRupBuild = 0x096c;     // von Excel abgeschrieben
    nRupYear = 0x07c9;      // "
}




void ExcCell::SaveCont( SvStream& rStr )
{
    if( pPrgrsBar )
        pPrgrsBar->SetState( GetCellCount() );

    IncCellCount();

    rStr << ( UINT16 ) aPos.Row() << ( UINT16 ) aPos.Col() << nXF;
    SaveDiff( rStr );
}


void ExcCell::SaveDiff( SvStream& )
{
}


ExcCell::ExcCell( const ScAddress rPos, const ScPatternAttr* pAttr,
                    const ULONG nAltNumForm, BOOL bForceAltNumForm ) :
    aPos( rPos )
{
    if( pPrgrsBar )
        pPrgrsBar->SetState( GetCellCount() );

    IncCellCount();

    // Basis-Daten
    DBG_ASSERT( pXFRecs, "-ExcCell::ExcCell(): Halt Fremder!" );

    nXF = pXFRecs->Find( pAttr, FALSE, nAltNumForm, bForceAltNumForm );

#ifdef DBG_UTIL
    _nRefCount++;
#endif
}


ExcCell::~ExcCell()
{
    pXFRecs = NULL;
#ifdef DBG_UTIL
    _nRefCount--;
    DBG_ASSERT( _nRefCount >= 0, "*ExcCell::~ExcCell(): Das war mindestens einer zuviel!" );
#endif
}


void ExcCell::SetXFRecs( UsedAttrList *pNewXFRecs )
{
    pXFRecs = pNewXFRecs;
}




ExcBof::ExcBof( void )
{
    nDocType = 0x0010;
    nVers = 0x0500;
}


void ExcBof::SaveCont( SvStream &rStr )
{
    rStr << nVers << nDocType << nRupBuild << nRupYear;
}


UINT16 ExcBof::GetNum( void ) const
{
    return 0x0809;
}


UINT16 ExcBof::GetLen( void ) const
{
    return 8;
}




ExcBofW::ExcBofW( void )
{
    nDocType = 0x0005;
    nVers = 0x0500;
}


void ExcBofW::SaveCont( SvStream &rStr )
{
    rStr << nVers << nDocType << nRupBuild << nRupYear;
}


UINT16 ExcBofW::GetNum( void ) const
{
    return 0x0809;
}


UINT16 ExcBofW::GetLen( void ) const
{
    return 8;
}




void ExcFngroupcount::SaveCont( SvStream &rStr )
{
    rStr << ( UINT16 ) 0x000E;  // abgekupfert von einem Original
}


UINT16 ExcFngroupcount::GetNum( void ) const
{
    return 0x009C;
}


UINT16 ExcFngroupcount::GetLen( void ) const
{
    return 2;
}




UINT16 ExcEof::GetNum( void ) const
{
    return 0x000A;
}


UINT16 ExcEof::GetLen( void ) const
{
    return 0;
}




UINT16 ExcDummy_00::GetLen( void ) const
{
    return nMyLen;
}


const BYTE* ExcDummy_00::GetData( void ) const
{
    return pMyData;
}




UINT16 ExcDummy_040::GetLen( void ) const
{
    return nMyLen;
}


const BYTE* ExcDummy_040::GetData( void ) const
{
    return pMyData;
}




UINT16 ExcDummy_041::GetLen( void ) const
{
    return nMyLen;
}


const BYTE* ExcDummy_041::GetData( void ) const
{
    return pMyData;
}




void Exc1904::SaveCont( SvStream& r )
{
    UINT16  n = b1904? 0x0001 : 0x0000;
    r << n;
}


Exc1904::Exc1904( ScDocument& r )
{
    Date*   p = r.GetFormatTable()->GetNullDate();
    if( p )
        b1904 = ( *p == Date( 1, 1, 1904 ) );
    else
        b1904 = FALSE;
}


UINT16 Exc1904::GetNum( void ) const
{
    return 0x0022;
}


UINT16 Exc1904::GetLen( void ) const
{
    return 2;
}




ExcFont::ExcFont( BiffTyp eBiffP ) :
        eBiff( eBiffP )
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


ExcFont::ExcFont( Font* pFont, BiffTyp eBiffP ) :
        eBiff( eBiffP )
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

//  if( !nObjCnt )
//      pPalette2 = NULL;
#endif
}


void ExcFont::SetName( const String& rName )
{
    sName.Assign( rName );
    nNameLen = (UINT16) sName.Len();
}


void ExcFont::SetColor( UINT32 nSerial )
{
    nColorSer = nSerial;
    bIgnoreCol = FALSE;
}


void ExcFont::SaveCont( SvStream& rStrm )
{
    XclContinue aCont( rStrm, 0, GetLen() );

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
    {
        rStrm << (BYTE) nNameLen;
        rStrm.Write( sName.GetBuffer(), (BYTE) nNameLen );
    }
    else
    {
        //! always 16-bit
        XclRawUnicodeString aUni( sName, 255, TRUE );
        rStrm << (UINT8) aUni.GetLen() << aUni.GetGrbit();
        aUni.Write( aCont );
    }
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


UINT16 ExcFont::GetLen( void ) const
{
    return 15 + ( eBiff < Biff8 ? nNameLen : 1 + 2 * nNameLen );
}




UINT16 ExcDummy_01::GetLen( void ) const
{
    return nMyLen;
}


const BYTE* ExcDummy_01::GetData( void ) const
{
    return pMyData;
}




UINT16 ExcDummy_Fm::GetLen( void ) const
{
    return nMyLen;
}


const BYTE* ExcDummy_Fm::GetData( void ) const
{
    return pMyData;
}




UINT16 ExcDummy_XF::GetLen( void ) const
{
    return nMyLen;
}


const BYTE* ExcDummy_XF::GetData( void ) const
{
    return pMyData;
}




UINT16 ExcDummy_Style::GetLen( void ) const
{
    return nMyLen;
}


const BYTE* ExcDummy_Style::GetData( void ) const
{
    return pMyData;
}




ExcBundlesheet::ExcBundlesheet( RootData* p, const String& rNewName ) : aName( rNewName ), ExcRoot( p )
{
    DBG_ASSERT( rNewName.Len() < 256, "*ExcBundlesheet::Ctor: Tabellenname laenger als 255 Zeichen!" );

    nOwnPos = nStrPos = 0xFFFFFFFF;
}


void ExcBundlesheet::SaveCont( SvStream& rStr )
{
    nOwnPos = rStr.Tell();
    ByteString  aStr( aName, *pExcRoot->pCharset );
    BYTE        nNameLen = ( BYTE ) aStr.Len();                 // max. 255 Zeichen langer Name
    rStr << ( UINT32 ) 0x00000000 << nIgnore << nNameLen;       // Position ist nur Dummy
                    //  '-> Worksheet visible

    rStr.Write( aStr.GetBuffer(), nNameLen );
}


void ExcBundlesheet::UpdateStreamPos( SvStream& rOut )
{
    rOut.Seek( nOwnPos );
    rOut << nStrPos;
}


UINT16 ExcBundlesheet::GetNum( void ) const
{
    return 0x0085;
}


UINT16 ExcBundlesheet::GetLen( void ) const
{
    return 7 + ( UINT16 ) aName.Len();
}




UINT16 ExcDummy_02::GetLen( void ) const
{
    return nMyLen;
}


const BYTE* ExcDummy_02::GetData( void ) const
{
    return pMyData;
}




ExcNote::ExcNote( const ScAddress aNewPos, const String& rText, RootData& rExcRoot ) :
    aPos( aNewPos )
{
    pText = new ByteString( rText, *rExcRoot.pCharset );
    nTextLen = ( UINT16 ) pText->Len();

    if( nTextLen > 4080 )
    {
        nTextLen = 4080;
        DBG_WARNING( "ExcNote::ExcNote(): String truncated" );
    }
}


ExcNote::~ExcNote()
{
    delete pText;
}


void ExcNote::SaveCont( SvStream& rStr )
{
    rStr << ( UINT16 ) aPos.Row() << ( UINT16 ) aPos.Col() << nTextLen;
    rStr.Write( pText->GetBuffer(), nTextLen );
}


UINT16 ExcNote::GetNum( void ) const
{
    return 0x001C;
}


UINT16 ExcNote::GetLen( void ) const
{
    return 6 + nTextLen;
}




ExcNumber::ExcNumber( const ScAddress rPos, const ScPatternAttr* pAttr,
    const double& rNewVal ) :
    ExcCell( rPos, pAttr )
{
    fVal = rNewVal;
}


void ExcNumber::SaveDiff( SvStream &rStr )
{
    rStr << fVal;
}


UINT16 ExcNumber::GetNum( void ) const
{
    return 0x0203;
}


UINT16 ExcNumber::GetLen( void ) const
{
    return 6 + 8;
}




ExcBoolerr::ExcBoolerr( const ScAddress rPos, const ScPatternAttr* pAttr,
    UINT8 nValP, BOOL bIsError ) :
    // #73420# force to "no number format" if boolean value
    ExcCell( rPos, pAttr, (bIsError ? NUMBERFORMAT_ENTRY_NOT_FOUND : 0), !bIsError ),
    nVal( bIsError ? nValP : (nValP != 0) ),
    bError( bIsError != 0 )
{
}


void ExcBoolerr::SaveDiff( SvStream &rStr )
{
    rStr << nVal << bError;
}


UINT16 ExcBoolerr::GetNum( void ) const
{
    return 0x0205;
}


UINT16 ExcBoolerr::GetLen( void ) const
{
    return 6 + 2;
}




void ExcRKMulRK::SaveCont( SvStream& rStr )
{
    if( ExcCell::pPrgrsBar )
        ExcCell::pPrgrsBar->SetState( ExcCell::GetCellCount() );

    CONT*   pAct = ( CONT* ) List::First();

    DBG_ASSERT( pAct, "-ExcRKMulRk::SaveCont(): einen brauch' ich schon! ")

    if( IsRK() )
    {
        rStr << ( UINT16 ) aPos.Row()  << ( UINT16 ) aPos.Col() << pAct->nXF
            << pAct->nVal;
        ExcCell::IncCellCount();
    }
    else
    {
        UINT16  nLastCol = aPos.Col();
        rStr << ( UINT16 ) aPos.Row()  << nLastCol;
        while( pAct )
        {
            rStr << pAct->nXF << pAct->nVal;
            pAct = ( CONT* ) List::Next();
            ExcCell::IncCellCount();
            nLastCol++;
        }

        nLastCol--;
        rStr << nLastCol;
    }
}


ExcRKMulRK::ExcRKMulRK( const ScAddress rPos, const ScPatternAttr *pAttr, const INT32 nVal ) :
    aPos( rPos )
{
    if( ExcCell::pPrgrsBar )
        ExcCell::pPrgrsBar->SetState( ExcCell::GetCellCount() );

    ExcCell::IncCellCount();

    // Basis-Daten
    DBG_ASSERT( ExcCell::pXFRecs, "-ExcRKMulRk::ExcRKMulRK(): Halt Fremder!" );

    CONT*   pNewCont = new CONT;

    if( pAttr )
        pNewCont->nXF = ExcCell::pXFRecs->Find( pAttr );
    else
        pNewCont->nXF = 15;
    pNewCont->nVal = nVal;
    List::Insert( pNewCont, LIST_APPEND );
}


ExcRKMulRK::~ExcRKMulRK()
{
    CONT*   pDel = ( CONT* ) List::First();
    while( pDel )
    {
        delete pDel;
        pDel = ( CONT* ) List::Next();
    }
}


ExcRKMulRK* ExcRKMulRK::Extend(
    const ScAddress rPos, const ScPatternAttr* pAttr, const INT32 nVal )
{
    if( aPos.Row() == rPos.Row() && aPos.Col() + List::Count() == rPos.Col() )
    {// erweiterbar
        if( ExcCell::pPrgrsBar )
            ExcCell::pPrgrsBar->SetState( ExcCell::GetCellCount() );

        ExcCell::IncCellCount();

        CONT*   pNewCont = new CONT;

        if( pAttr )
            pNewCont->nXF = ExcCell::pXFRecs->Find( pAttr );
        else
            pNewCont->nXF = 15;
        pNewCont->nVal = nVal;
        List::Insert( pNewCont, LIST_APPEND );
        return NULL;
    }
    else
    {// neuen erzeugen
        return new ExcRKMulRK( rPos, pAttr, nVal );
    }
}


UINT16 ExcRKMulRK::GetNum( void ) const
{
    if( IsRK() )
        return 0x027E;
    else
        return 0x00BD;
}


UINT16 ExcRKMulRK::GetLen( void ) const
{
    if( IsRK() )
        return 6 + 4;
    else
        return 6 + ( UINT16 ) List::Count() * 6;
}




ExcLabel::ExcLabel( const ScAddress rPos, const ScPatternAttr* pAttr, const String& rText, RootData& rExcRoot ) :
    ExcCell( rPos, pAttr )
{
    pText = new ByteString( rText, *rExcRoot.pCharset );
    nTextLen = ( UINT16 ) pText->Len();

    if( nTextLen > 255 )
    {
        nTextLen = 255;
        DBG_WARNING( "ExcLabel::ExcLabel(): String truncated" );
    }
}


ExcLabel::~ExcLabel()
{
    delete pText;
}


void ExcLabel::SaveDiff( SvStream &rStr )
{
    rStr << nTextLen;
    rStr.Write( pText->GetBuffer(), nTextLen );
}


UINT16 ExcLabel::GetNum( void ) const
{
    return 0x0204;
}


UINT16 ExcLabel::GetLen( void ) const
{
    return 6 + 2 + nTextLen;
}




ExcRichStr::ExcRichStr( ExcCell& rExcCell, String& rText, const ScPatternAttr* pAttr,
                        const ScEditCell& rEdCell, RootData& rRoot, xub_StrLen nMaxChars ) :
    eBiff( rRoot.eDateiTyp )
{
    const EditTextObject*   p = rEdCell.GetData();

//  const SvxURLField*&     rpLastHlink = rRoot.pLastHlink;
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
        xub_StrLen          nStartPos;
        String              aParText;
        const sal_Unicode   cParSep = 0x0A;
        SvUShorts           aPosList;
        ESelection          aSel;
        ScPatternAttr       aPatAttr( rRoot.pDoc->GetPool() );
        UsedFontList&       rFontList = *rRoot.pFontRecs;

        // Erster Font ist der Font der Zelle, erst nachfolgende Abweichungen
        // werden im RichStr gespeichert.
        Font*               pFont = new Font;
        pAttr->GetFont( *pFont );
        USHORT              nLastFontIndex = rFontList.Add( pFont );

        for( nPar = 0 ; nPar < nParCnt ; )
        {
            aSel.nStartPara = aSel.nEndPara = nPar;
            nParPos = rText.Len();

            aParText = rEdEng.GetText( nPar );

            if( aParText.Len() )
            {
//              if ( eBiff < Biff8 )
//                  aParText.Convert( CHARSET_SYSTEM, CHARSET_ANSI );

                rEdEng.GetPortions( nPar, aPosList );
                nListLen = aPosList.Count();

                aSel.nStartPos = 0;
                for( n = 0 ; n < nListLen ; n++ )
                {
                    aSel.nEndPos = ( xub_StrLen ) aPosList.GetObject( n );

                    {
                        SfxItemSet  aItemSet( rEdEng.GetAttribs( aSel ) );

                        // detect hyperlink but only take first one!
                        if( aSel.nEndPos == aSel.nStartPos + 1 && !rpLastHlink )
                        {
                            const SfxPoolItem*          pItem;

                            if( aItemSet.GetItemState( EE_FEATURE_FIELD, FALSE, &pItem ) == SFX_ITEM_ON )
                            {
                                const SvxFieldData*     pField = ((const SvxFieldItem*) pItem)->GetField();

                                if( pField && pField->ISA( SvxURLField ) )
                                {
                                    rpLastHlink = new XclHlink( rRoot, *( ( const SvxURLField* ) pField ) );
                                    const String*       p = rpLastHlink->GetRepr();
                                    if( p )
                                        aParText = *p;
                                }
                            }
                        }

                        aPatAttr.GetItemSet().ClearItem();
                        aPatAttr.GetFromEditItemSet( &aItemSet );

                        Font*       pFont = new Font;
                        aPatAttr.GetFont( *pFont );
                        UINT16      nFontIndex = rFontList.Add( pFont );

                        if( nFontIndex > 255 && eBiff < Biff8 )
                            nFontIndex = 0;

                        nStartPos = nParPos + aSel.nStartPos;
                        if( nStartPos <= nMaxChars && (eBiff >= Biff8 || nF < 256) )
                        {
                            if( nLastFontIndex != nFontIndex )
                            {
                                DBG_ASSERT( nStartPos <= 0xFFFF, "*ExcRichStr::ExcRichStr(): Start pos to big!" );
                                aForms.Append( ( UINT16 ) nStartPos );
                                aForms.Append( nFontIndex );
                                nLastFontIndex = nFontIndex;
                                nF++;
                            }
                        }
                    }
                    aSel.nStartPos = aSel.nEndPos;
                }

                rText += aParText;
            }

            nPar++;
            if( nPar < nParCnt )
                rText += cParSep;
        }

        rEdEng.SetUpdateMode( bOldMode );

        // XF mit Umbruch auswaehlen?
        rExcCell.SetXF( nParCnt <= 1 ? ExcCell::GetXFRecs()->Find( pAttr ) :
            ExcCell::GetXFRecs()->FindWithLineBreak( pAttr ) );
    }
    else
    {
        rEdCell.GetString( rText );
//      if ( eBiff < Biff8 )
//          rText.Convert( CHARSET_SYSTEM, CHARSET_ANSI );
    }
}


void ExcRichStr::Write( XclContinue& rCont )
{
    const UINT32        nEnd = ( UINT32 ) GetFormCount() * 2;
    if( nEnd )
    {
        if ( eBiff >= Biff8 )
            rCont.Write( &aForms, nEnd );
        else
        {   // keine CONTINUE Records, max 255 chars + 2 * 255 Forms
            SvStream&   rStr = rCont.GetStream();
            rStr << ( UINT8 ) aForms.First();
            for( UINT32 n = 1 ; n < nEnd ; n++ )
                rStr << ( UINT8 ) aForms.Next();
        }
    }
}


ExcRichStr::~ExcRichStr()
{
}



ExcRString::ExcRString( RootData* pRootData, const ScAddress aNewPos, const ScPatternAttr* pAttr,
    const ScEditCell& rEdCell ) :
    ExcCell( aNewPos, pAttr )
{
    pRichStr = new ExcRichStr( *this, aText, pAttr, rEdCell, *pRootData, 255 );
    DBG_ASSERT( aText.Len() <= 0xFFFF, "*ExcRString::ExcRString(): String to long!" );
    nTextLen = ( UINT16 ) aText.Len();

    if( nTextLen > 255 )
        nTextLen = 255;
}


ExcRString::~ExcRString()
{
    delete pRichStr;
}


void ExcRString::SaveDiff( SvStream& rStr )
{
    XclContinue     aCont( rStr, 6, GetLen() );
    rStr << nTextLen;
    rStr.Write( ByteString( aText, *pExcRoot->pCharset ).GetBuffer(), nTextLen );
    rStr << ( UINT8 ) pRichStr->GetFormCount();
    pRichStr->Write( aCont );
}


UINT16 ExcRString::GetNum( void ) const
{
    return 0x00D6;
}


UINT16 ExcRString::GetLen( void ) const
{
    return 6 + 2 + nTextLen + 1 + ( UINT16 ) pRichStr->GetByteCount();
}




ExcFormula::ExcFormula( RootData* pRD, const ScAddress rPos, const ScPatternAttr* pAttr,
                    const ULONG nAltNumForm, BOOL bForceAltNumForm, const ScTokenArray& rTokArray ) :
    ExcCell( rPos, pAttr, nAltNumForm, bForceAltNumForm )
{
    // Formel wandeln
    ExcUPN aExcUPN( pRD, rTokArray, &rPos );
    nFormLen = aExcUPN.GetLen();

    if( nFormLen > 0 )
        pData = new sal_Char[ nFormLen ];
    else
        pData = NULL;

    memcpy( pData, aExcUPN.GetData(), nFormLen );

    DBG_ASSERT( pData,
        "*ExcFormula::ExcFormula(): 'n bisschen 'was sollte es schon sein!" );
}


ExcFormula::~ExcFormula()
{
    if( pData )
        delete[] pData;
}


void ExcFormula::SaveDiff( SvStream &rStr )
{//                         grbit               chn
    rStr << ( double ) 0.0 << ( UINT16 ) 0x0003 << ( UINT32 ) 0x00000000
        << nFormLen;
    if( pData )
        rStr.Write( pData, nFormLen );
}


UINT16 ExcFormula::GetNum( void ) const
{
    return 0x0006;
}


UINT16 ExcFormula::GetLen( void ) const
{
    return 6 + 16 + nFormLen;
}




void ExcBlankMulblank::SaveDiff( SvStream &rStr )
{
    if( bMulBlank )
    {
        void    *pAkt = List::First();
        if( pAkt )
        {
            UINT16  nCol = ( UINT16 ) aPos.Col();
            UINT16  nXF, nTmpAnz;

            do{
                nXF = GetXF( pAkt );
                nTmpAnz = GetAnz( pAkt );

                nCol += nTmpAnz;

                while( nTmpAnz )
                {
                    rStr << nXF;
                    nTmpAnz--;
                }

                pAkt = List::Next();
            } while( pAkt );

            rStr << nCol;   // nur 'nCol' und nicht 'nCol-1', weil initial schon einen zu
                            //  wenig wegen 'Single-Schreiben' eines XFs in ExcCell!
        }
    }
}


ExcBlankMulblank::ExcBlankMulblank( const ScAddress rPos,
    const ScPatternAttr *pFirstAttr, UINT16 nFirstAnz ) :
    ExcCell( rPos, pFirstAttr )
{// nFirstAnz > 1 -> sofort Mulblank, ansonsten erstmal Blank!
    DBG_ASSERT( nFirstAnz > 0, "-ExcBlankMulblank::ExcBlankMulblank(): Anzahl <> 0!" );
    nLastCol = aPos.Col();

    nLen = 6;

    nFirstAnz--;    // einer ist schon in ExcCell!
    if( nFirstAnz )
    {
        List::Insert( MakeEntry( nXF, nFirstAnz ), LIST_APPEND );

        bMulBlank = TRUE;
        nLen += 2 * nFirstAnz + 2;
        nLastCol += nFirstAnz;
    }
    else
        bMulBlank = FALSE;
}


void ExcBlankMulblank::Add( const ScPatternAttr *pAttr, const UINT16 nAddAnz )
{
    DBG_ASSERT( pXFRecs, "-ExcMulblank::Add(): Wohin des Weges Fremder?" );

    if( !bMulBlank )
    {
        bMulBlank = TRUE;
        nLen += 2;
    }

    List::Insert( MakeEntry( pXFRecs->Find( pAttr ), nAddAnz ), LIST_APPEND );
    nLen += 2 * nAddAnz;
    nLastCol += nAddAnz;
}


UINT16 ExcBlankMulblank::GetNum( void ) const
{
    if( bMulBlank )
        return 0x00BE;
    else
        return 0x0201;
}


UINT16 ExcBlankMulblank::GetLen( void ) const
{
    return nLen;
}




ExcNameListEntry::~ExcNameListEntry()
{
}


UINT16 ExcNameListEntry::GetNum( void ) const
{
    return 0x0018;
}




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
    ScTokenArray aArr;

    if ( rRange.aStart == rRange.aEnd )
    {
        SingleRefData aRef;
        aRef.InitAddress( rRange.aStart );
        aArr.AddSingleReference( aRef );
    }
    else
    {
        ComplRefData aRef;
        aRef.InitRange( rRange );
        aArr.AddDoubleReference( aRef );
    }
    ExcUPN aExcUPN( pExcRoot, aArr );
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

    bDummy = aName.CompareToAscii( GetExcBuiltInName( 0x06 ) ) == COMPARE_EQUAL;    // no PrintRanges

    if( !bDummy )
    {
        // Formel wandeln
        const ScTokenArray* pTokArray = pRange->GetCode();
        if( pTokArray )
        {
            ExcUPN aExcUPN( pRD, *pTokArray );
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


void ExcName::Save( SvStream& r )
{
    if( !bDummy )
    {
        _Save( r );
        SaveCont( r );
    }
}


void ExcName::SaveCont( SvStream& rStr )
{
    XclContinue aCont( rStr, 0, GetLen() );
    UINT8       nNameLen = ( UINT8 ) Min( aName.Len(), ( xub_StrLen ) 255 );
    UINT16      nGrbit = (bHidden ? EXC_NAME_HIDDEN : 0) | (bBuiltIn ? EXC_NAME_BUILTIN : 0);

    //      grbit           chKey       cch
    rStr << nGrbit << ( BYTE ) 0x00 << nNameLen
    //      cce             ixals           itab
        << nFormLen << ( UINT16 ) 0x0000 << nTabNum
    //      cch...
        << ( UINT32 ) 0x00000000;
    if ( eBiff < Biff8 )
    {
        rStr.Write( ByteString( aName, *pExcRoot->pCharset ).GetBuffer(), nNameLen );
        if( pData )
            rStr.Write( pData, nFormLen );
    }
    else
    {
        XclRawUnicodeString aUni( aName, nNameLen );
        rStr << aUni.GetGrbit();
        aUni.Write( aCont );
        aCont.Write( pData, nFormLen );
    }
}


UINT16 ExcName::GetLen( void ) const
{   // only a guess for Biff8 (8bit/16bit unknown)
    if( bDummy )
        return 0;
    else
        return 14 + nFormLen + (eBiff < Biff8 ? 0 : 1) + Min( aName.Len(), ( xub_StrLen ) 255 );
}




ExcNameList::~ExcNameList()
{
    for( ExcNameListEntry* pName = _First(); pName; pName = _Next() )
        delete pName;
}


void ExcNameList::_Save( SvStream& rStrm )
{
}


void ExcNameList::SaveCont( SvStream& rStrm )
{
    for( ExcNameListEntry* pName = _First(); pName; pName = _Next() )
        pName->Save( rStrm );
}


UINT16 ExcNameList::GetNum( void ) const
{
    return 0;
}


UINT16 ExcNameList::GetLen( void ) const
{
    return 0;
}




UINT16 ExcDummy_03::GetLen( void ) const
{
    return nMyLen;
}


const BYTE* ExcDummy_03::GetData( void ) const
{
    return pMyData;
}




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


void ExcDimensions::SaveCont( SvStream &rStr )
{
    if ( eBiff < Biff8 )
        rStr << nRwMic << nRwMac;
    else
        rStr << (UINT32) nRwMic << (UINT32) nRwMac;
    rStr << nColMic << nColMac << nIgnore;
}


UINT16 ExcDimensions::GetNum( void ) const
{
    return 0x0200;
}


UINT16 ExcDimensions::GetLen( void ) const
{
    return eBiff < Biff8 ? 10 : 14;
}


void ExcDimensions::SetLimits( UINT16 nFirstCol, UINT16 nFirstRow,
    UINT16 nLastCol, UINT16 nLastRow )
{
    nRwMic = nFirstRow;
    nRwMac = nLastRow + 1;
    nColMic = nFirstCol;
    nColMac = nLastCol + 1;
}




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




ExcEGuts::ExcEGuts( ScOutlineArray* pCol, ScOutlineArray* pRow )
{
    nRowLevel = nColLevel = 0;

    if( pCol )
        nColLevel = Min( pCol->GetDepth(), (UINT16) EXC_OUTLINE_MAX );
    if( pRow )
        nRowLevel = Min( pRow->GetDepth(), (UINT16) EXC_OUTLINE_MAX );
}

void ExcEGuts::SaveCont( SvStream& rStrm )
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

UINT16 ExcEGuts::GetLen() const
{
    return 8;
}




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
    else
        nHeight |= EXC_ROW_FLAGDEFHEIGHT;   // default height
}

void ExcRow::SaveCont( SvStream& rStrm )
{
    nOptions |= EXC_ROW_FLAGCOMMON;
    rStrm   << nNum << nFirstCol << (UINT16)(nLastCol + 1)
            << nHeight << (UINT32)0 << nOptions << nXF;
}

UINT16 ExcRow::GetNum() const
{
    return 0x0208;
}


UINT16 ExcRow::GetLen() const
{
    return 16;
}




ExcRowBlock::ExcRowBlock()
{
    ppRows = new ExcRow* [ 32 ];
    nNext = 0;
}

ExcRowBlock::~ExcRowBlock()
{
    UINT16  nC;
    for( nC = 0 ; nC < nNext ; nC++ )
        delete ppRows[ nC ];

    delete[] ppRows;
}

void ExcRowBlock::Save( SvStream& rOut )
{
    UINT16  nC;
    for( nC = 0 ; nC < nNext ; nC++ )
        ppRows[ nC ]->Save( rOut );
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
        ExcRowBlock*    pRet = new ExcRowBlock;
        pRet->ppRows[ 0 ] = pNewRow;
        pRet->nNext = 1;
        return pRet;
    }
}

UINT16 ExcRowBlock::GetNum() const
{
    return 0x0000;  // Dummy
}


UINT16 ExcRowBlock::GetLen() const
{
    UINT32 nLenSum = 0;
    if( nNext )
    {
        for( UINT16 nC = 0; nC < nNext; nC++ )
            nLenSum += ppRows[ nC ]->GetLen();

        nLenSum += (nNext - 1) * 4;
    }

    DBG_ASSERT( nLenSum <= 0xFFFF, "ExcRowBlock::GetLen() - size > 0xFFFF!" );
    return (UINT16) nLenSum;
}




ExcDefcolwidth::ExcDefcolwidth( UINT16 nNewWidth )
{
    nWidth = nNewWidth;
}


void ExcDefcolwidth::SaveCont( SvStream &rStr )
{
    rStr << nWidth;
}


UINT16 ExcDefcolwidth::GetNum( void ) const
{
    return 0x0055;
}


UINT16 ExcDefcolwidth::GetLen( void ) const
{
    return 2;
}




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

BOOL ExcColinfo::Expand( ExcColinfo* pExp )
{
    if( pExp && (nLastCol + 1 == pExp->nFirstCol) && (nColWidth == pExp->nColWidth) &&
        (nXF == pExp->nXF) && (nOptions == pExp->nOptions) )
    {// erweitern
        nLastCol = pExp->nLastCol;
        delete pExp;
        return TRUE;
    }
    else
        return FALSE;
}

void ExcColinfo::SaveCont( SvStream &rStr )
{
    rStr << nFirstCol << nLastCol << nColWidth << nXF << nOptions << (BYTE)0;
}

UINT16 ExcColinfo::GetNum( void ) const
{
    return 0x007D;
}

UINT16 ExcColinfo::GetLen( void ) const
{
    return 11;
}




ExcXf::ExcXf( UINT16 nFont, UINT16 nForm, const ScPatternAttr* pPattAttr, BOOL& rbWrap, BOOL bSt ) :
    bStyle( bSt )
{
#ifdef DBG_UTIL
    nObjCnt++;
#endif

    bAtrNum = bAtrFnt = bAtrAlc = bAtrBdr = bAtrPat = bAtrProt = !bSt;  // always TRUE for cell xfs

    nIfnt = nFont;
    nIfmt = nForm;

    nOffs8 = 0x0001;

    if( pPattAttr )
    {
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
    {
        pPalette2 = NULL;   // letzter macht das Licht aus...
    }
}
#endif


void ExcXf::SaveCont( SvStream& rStr )
{
    UINT16  nTmp;

    rStr << nIfnt << nIfmt                              // Offs 4 + 6
        << nOffs8;                                      // Offs 8

    nTmp = ( UINT16 ) eAlc;                             // Offs 10
    nTmp |= 0x0800;     // locked
    if( bFWrap )
        nTmp += 0x0008;
    nTmp += ( ( UINT16 ) eAlcV ) << 4;
    nTmp += ( ( UINT16 ) eOri ) << 8;
    rStr << nTmp;

    UINT16 nForeInd, nBackInd;
    pPalette2->GetMixedColors( nIcvForeSer, nIcvBackSer, nForeInd, nBackInd, nFls );
    nTmp = nForeInd;                                    // Offs 12
    nTmp |= nBackInd << 7;

    if ( bFSxButton )
        nTmp |= 0x2000;
    rStr << nTmp;

    nTmp = nFls;                                        // Offs 14
    nTmp += nDgBottom << 6;
    nTmp |= pPalette2->GetColorIndex( nIcvBotSer ) << 9;
    rStr << nTmp;

    nTmp = nDgTop;                                      // Offs 16
    nTmp += nDgLeft << 3;
    nTmp += nDgRight << 6;
    nTmp |= pPalette2->GetColorIndex( nIcvTopSer ) << 9;
    rStr << nTmp;

    nTmp = pPalette2->GetColorIndex( nIcvLftSer );      // Offs 18
    nTmp |= pPalette2->GetColorIndex( nIcvRigSer ) << 7;
    rStr << nTmp;
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


void ExcXf::SetPalette( ExcPalette2& rPalette2 )
{
    pPalette2 = &rPalette2;
}


UINT16 ExcXf::GetNum( void ) const
{
    return 0x00E0;
}


UINT16 ExcXf::GetLen( void ) const
{
    return 16;
}




ExcFormat::ExcFormat( RootData* pExcRoot, UINT32 nNewScIndex ) : ExcRoot( pExcRoot )
{
    // ACHTUNG: nIndex wird hier NICHT gesetzt -> bei Freunden machen!
    if( !nObjCnt )
        pFormatter = new SvNumberFormatter( LANGUAGE_ENGLISH_US );

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


void ExcFormat::SaveCont( SvStream &rOut )
{
    if ( eBiff < Biff8 )
    {
        rOut << nIndex << nFormLen;
        rOut.Write( ByteString( *pForm, *pExcRoot->pCharset ).GetBuffer(), nFormLen );
    }
    else
    {
        XclContinue aCont( rOut, 0, GetLen() );
        rOut << nIndex;
        XclUnicodeString( *pForm ).Write( aCont );
    }
}


UINT16 ExcFormat::GetNum( void ) const
{
    return 0x041E;
}


UINT16 ExcFormat::GetLen( void ) const
{   //! for Biff8 only a prediction, assumes 8-bit string
    return ( eBiff < Biff8 ? 2 + 1 + nFormLen : 2 + 3 + nFormLen );
}



//_____________________________________________________________________________
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

void ExcPal2Entry::Save( SvStream& rStrm )
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
    Color aCol( 0x00000000 );
    InsertColor( aCol, EXC_COLOR_CELLTEXT );
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
    if( nRemove == 0 )
    {
        UINT32 nSwap = nKeep;
        nKeep = nRemove;
        nRemove = nSwap;
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
        return (UINT16)(nSerial & !EXC_PAL2_INDEXBASE);
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

void ExcPalette2::SaveCont( SvStream& rStrm )
{
    rStrm << rColBuff.GetAnz();

    for( ExcPal2Entry* pEntry = _First(); pEntry; pEntry = _Next() )
        pEntry->Save( rStrm );

    for( UINT16 nInd = List::Count(); nInd < rColBuff.GetAnz(); nInd++ )
        rStrm   << rColBuff.GetRed( nInd )
                << rColBuff.GetGreen( nInd )
                << rColBuff.GetBlue( nInd )
                << (UINT8)0x00;
}

UINT16 ExcPalette2::GetNum() const
{
    return 0x0092;
}

UINT16 ExcPalette2::GetLen() const
{
    return 2 + 4 * rColBuff.GetAnz();
}
//_____________________________________________________________________________



ExcExterncount::ExcExterncount( RootData* pRD, const BOOL bTableNew ) : ExcRoot( pRD )
{
    bTable = bTableNew;
}


void ExcExterncount::SaveCont( SvStream &rOut )
{
    UINT16  nNumTabs = pExcRoot->pDoc->GetTableCount();

    if( nNumTabs && bTable )
        nNumTabs--;

    rOut << nNumTabs;
}


UINT16 ExcExterncount::GetNum( void ) const
{
    return 0x0016;
}


UINT16 ExcExterncount::GetLen( void ) const
{
    return 2;
}




ExcExternsheet::ExcExternsheet( RootData* pExcRoot, const UINT16 nNewTabNum ) : ExcRoot( pExcRoot )
{
    DBG_ASSERT( pExcRoot->pDoc->HasTable( nNewTabNum ),
        "*ExcExternsheet::ExcExternsheet(): Tabelle existiert nicht!" );

    pExcRoot->pDoc->GetName( nNewTabNum, aTabName );
    DBG_ASSERT( aTabName.Len() < 256,
        "*ExcExternsheet::ExcExternsheet(): Tabellenname zu lang fuer Excel!" );
}


void ExcExternsheet::SaveCont( SvStream& rOut )
{
    rOut << ( UINT8 ) ( aTabName.Len() + 1 ) << ( UINT8 ) 0x03;
    rOut.Write( ByteString( aTabName, *pExcRoot->pCharset ).GetBuffer(), aTabName.Len() );
}


UINT16 ExcExternsheet::GetNum( void ) const
{
    return 0x0017;
}


UINT16 ExcExternsheet::GetLen( void ) const
{
    DBG_ASSERT( aTabName.Len() <= 0xFFFF, "*ExcExternsheet::GetLen(): Tab name to long!" );
    return 2 + ( UINT16 ) aTabName.Len();
}




void ExcExternsheetList::_Save( SvStream& r )
{
    ExcExternsheet*     p = ( ExcExternsheet* ) First();

    while( p )
    {
        p->Save( r );
        p = ( ExcExternsheet* ) Next();
    }
}


ExcExternsheetList::~ExcExternsheetList()
{
    ExcExternsheet*     p = ( ExcExternsheet* ) First();

    while( p )
    {
        delete p;
        p = ( ExcExternsheet* ) Next();
    }
}


UINT16 ExcExternsheetList::GetNum( void ) const
{   // Dummy
    return 0;
}


UINT16 ExcExternsheetList::GetLen( void ) const
{   // Dummy
    return 0;
}




void ExcExternDup::_Save( SvStream& r )
{
    rExtCnt.Save( r );
    rExtSheetList.Save( r );
}


ExcExternDup::ExcExternDup( ExcExterncount& rC, ExcExternsheetList& rL ) :
    rExtCnt( rC ), rExtSheetList( rL )
{
}


ExcExternDup::ExcExternDup( const ExcExternDup& r ) :
    rExtCnt( r.rExtCnt ), rExtSheetList( r.rExtSheetList )
{
}


UINT16 ExcExternDup::GetNum( void ) const
{   // Dummy
    return 0;
}


UINT16 ExcExternDup::GetLen( void ) const
{   // Dummy
    return 0;
}




ExcWindow2::ExcWindow2( UINT16 nTab ) : nTable( nTab )
{
}


void ExcWindow2::SaveCont( SvStream& rOut )
{
    BYTE pData[] = { 0xb6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    DBG_ASSERT( sizeof(pData) == GetLen(), "ExcWindow2::SaveCont: length mismatch" );
    if ( GetTable() == 0 )
        pData[1] |= 0x06;       // displayed and selected
    rOut.Write( pData, GetLen() );
}


UINT16 ExcWindow2::GetNum( void ) const
{
    return 0x023e;
}


UINT16 ExcWindow2::GetLen( void ) const
{
    return 10;
}




UsedList::UsedList( void )
{
    nLen = 0;
}


void UsedList::SaveCont( SvStream& rOut )
{
    ExcRecord*  pExcRec = ( ExcRecord* ) First();

    while( pExcRec )
    {
        pExcRec->Save( rOut );
        pExcRec = ( ExcRecord * ) Next();
    }
}


void UsedList::_Save( SvStream &rOut )
{
}


void UsedList::SetBaseIndex( UINT16 nNewVal )
{
    nBaseIndex = nNewVal;
}


UINT16 UsedList::GetLen( void ) const
{
    return nLen;
}




UsedFontList::UsedFontList( BiffTyp eBiffP ) : eBiff( eBiffP )
{   }


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

    nLen += pExcFont->GetLen();
    return (UINT16)(Count() - 1 + nBaseIndex);
}


UINT16 UsedFontList::Add( Font* pFont )
{
    return Add( new ExcFont( pFont, eBiff ) );
}


UINT16 UsedFontList::GetNum( void ) const
{
    return 0xFF31;
}




UsedFormList::~UsedFormList()
{
    void*       pAkt = First();

    while( pAkt )
    {
        delete ( ExcFormat* ) pAkt;
        pAkt = Next();
    }
}


UINT16 UsedFormList::Add( ExcFormat* pFormat )
{
    // Doubletten suchen
    UINT32      nSearch = pFormat->nScIndex;

    ExcFormat*  pAkt = ( ExcFormat* ) First();
    UINT16      nC = nBaseIndex;
    while( pAkt )
    {
        if( pAkt->nScIndex == nSearch )
        {
            delete pFormat;
            return nC;
        }
        nC++;
        pAkt = ( ExcFormat* ) Next();
    }
    pFormat->nIndex = nC;
    Insert( pFormat, LIST_APPEND );
    nLen += pFormat->GetLen();
    return nC;
}


UINT16 UsedFormList::GetNum( void ) const
{
    return 0xFF1E;
}




void UsedAttrList::AddNewXF( const ScPatternAttr* pAttr, const BOOL bStyle, const BOOL bExplLineBreak,
                                const ULONG nAltNumForm, BOOL bForceAltNumForm )
{
    // Aufbauen der Excel-Daten
    ENTRY*          pData = new ENTRY;
    pData->pPattAttr = pAttr;
    pData->bLineBreak = bExplLineBreak;
    pData->nAltNumForm = nAltNumForm;
    Insert( ( void* ) pData, LIST_APPEND );

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

    // XF-Daten + Laengen-Update
    if ( pExcRoot->eDateiTyp < Biff8 )
        pData->pXfRec = new ExcXf( nFontIndex, nFormatIndex, pAttr, pData->bLineBreak, bStyle );
    else
        pData->pXfRec = new ExcXf8( nFontIndex, nFormatIndex, pAttr, pData->bLineBreak, bStyle );
    nLen += pData->pXfRec->GetLen();
}


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
    void*       pAkt = First();

    while( pAkt )
    {
        delete ( ( ENTRY* ) pAkt )->pXfRec; // erst ExcXf loeschen
        delete ( ENTRY* ) pAkt;             // ... dann Entry selbst
        pAkt = Next();
    }
}


void UsedAttrList::SaveCont( SvStream& rOut )
{
    ENTRY*      pEntry = ( ENTRY* ) First();

    while( pEntry )
    {
        pEntry->pXfRec->Save( rOut );
        pEntry = ( ENTRY* ) Next();
    }
}


UINT16 UsedAttrList::Find( const ScPatternAttr* pAttr, const BOOL bStyle,
        const UINT32 nAltNumForm, BOOL bForceAltNumForm )
{
    if( !pAttr && nAltNumForm == NUMBERFORMAT_ENTRY_NOT_FOUND )
        return 15;      // default XF

    DBG_ASSERT( nBaseIndex, "*UsedAttrList::Find(): leer == nichtleer?" );

    const ENTRY*    pAkt = ( const ENTRY* ) First();
    UINT16          nC = nBaseIndex;

    while( pAkt )
    {
        if( pAkt->Equal( pAttr, nAltNumForm ) )
            return nC;
        pAkt = ( const ENTRY* ) Next();
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

    ENTRY*          pAkt = ( ENTRY* ) First();
    UINT16          nC = nBaseIndex;

    while( pAkt )
    {
        if( pAkt->pPattAttr == pAttr && pAkt->bLineBreak )
            return nC;
        pAkt = ( ENTRY* ) Next();
        nC++;
    }

    // neuen ausdenken

    AddNewXF( pAttr, FALSE, TRUE );

    return nC;
}


UINT16 UsedAttrList::GetNum( void ) const
{
    return 0xFFE0;
}




void ExcSetup::SaveCont( SvStream& rStr )
{
    rStr << nPaperSize << nScale << nPageStart
            << ( UINT16 ) 1 << ( UINT16 ) 1             // FitWidth / FitHeight
            << nGrbit
            << ( UINT16 ) 0x012C << ( UINT16 ) 0x012C   // Res / VRes
            << ( UINT32 ) 0 << ( UINT32 ) 0x3FE00000    // Header margin (double)
            << ( UINT32 ) 0 << ( UINT32 ) 0x3FE00000    // Footer margin (double)
            << ( UINT16 ) 1;                            // num of copies
}


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

        if( nPageStart != 1 )
            nGrbit |= 0x0080;   // fUsePage

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


UINT16 ExcSetup::GetNum( void ) const
{
    return 0x00A1;
}


UINT16 ExcSetup::GetLen( void ) const
{
    return 0x0022;
}




void ExcHeaderFooter::_Save( SvStream& rStream )
{
    if( !aFormatString.Len() )
        return;

    if( bUnicode )
    {   // >= Biff8
        XclRawUnicodeString     aStr( aFormatString );
        DBG_ASSERT( aStr.GetByteCount() <= 0xFFFC, "*ExcHeaderFooter::SaveCont(): string is to long!" );

        nLen = ( UINT16 ) ( aStr.GetByteCount() + 3 );

        rStream << GetNum() << nLen << aStr.GetLen() << aStr.GetGrbit();
        aStr.WriteToStream( rStream );
    }
    else
    {   // < Biff8
        ByteString              aStr( aFormatString, *pExcRoot->pCharset );
        DBG_ASSERT( aStr.Len() <= 0xFE, "*ExcHeaderFooter::SaveCont(): string is to long!" );

        nLen = ( UINT16 ) ( aStr.Len() + 1 );

        rStream << GetNum() << nLen << ( UINT8 ) aStr.Len();
        rStream.Write( aStr.GetBuffer(), aStr.Len() );
    }
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
    if( bFields )
    {
        BOOL                bOldUpdateMode = rEdEng.GetUpdateMode();
        UINT16              nPar;

        rEdEng.SetUpdateMode( TRUE );

        for( nPar=0 ; nPar < nParas ; nPar++ )
        {
            if( nPar > 0 )
                aRet += cParSep;

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
        rEdEng.SetUpdateMode( bOldUpdateMode );
    }

    return aRet;
}


ExcHeaderFooter::ExcHeaderFooter( RootData* p, const BOOL b ) :
    nLen( 0 ),
    bUnicode( b ),
    ExcRoot( p )
{
}


UINT16 ExcHeaderFooter::GetLen( void ) const
{
    return nLen;
}




ExcHeader::ExcHeader( RootData* p, const BOOL b ) : ExcHeaderFooter( p, b )
{
    aFormatString = GetFormatString( ATTR_PAGE_HEADERRIGHT );
}


UINT16 ExcHeader::GetNum( void ) const
{
    return 0x0014;
}




ExcFooter::ExcFooter( RootData* p, const BOOL b ) : ExcHeaderFooter( p, b )
{
    aFormatString = GetFormatString( ATTR_PAGE_FOOTERRIGHT );
}


UINT16 ExcFooter::GetNum( void ) const
{
    return 0x0015;
}




void ExcBoolRecord::SaveCont( SvStream& r )
{
    r << ( UINT16 ) ( bVal? 0x0001 : 0x0000 );
}


ExcBoolRecord::ExcBoolRecord( SfxItemSet* pItemSet, USHORT nWhich, BOOL bDefault )
{
    bVal = pItemSet? ( ( const SfxBoolItem& ) pItemSet->Get( nWhich ) ).GetValue() : bDefault;
}


UINT16 ExcBoolRecord::GetLen( void ) const
{
    return 2;
}




ExcPrintheaders::ExcPrintheaders( SfxItemSet* p ) : ExcBoolRecord( p, ATTR_PAGE_HEADERS, TRUE )
{
}


UINT16 ExcPrintheaders::GetNum( void ) const
{
    return 0x002A;
}




ExcPrintGridlines::ExcPrintGridlines( SfxItemSet* p ) : ExcBoolRecord( p, ATTR_PAGE_GRID, TRUE )
{
}


UINT16 ExcPrintGridlines::GetNum( void ) const
{
    return 0x002B;
}




ExcHcenter::ExcHcenter( SfxItemSet* p ) : ExcBoolRecord( p, ATTR_PAGE_HORCENTER, FALSE )
{
}


UINT16 ExcHcenter::GetNum( void ) const
{
    return 0x0083;
}




ExcVcenter::ExcVcenter( SfxItemSet* p ) : ExcBoolRecord( p, ATTR_PAGE_VERCENTER, FALSE )
{
}


UINT16 ExcVcenter::GetNum( void ) const
{
    return 0x0084;
}



//___________________________________________________________________
// AutoFilter



void ExcFilterMode::SaveCont( SvStream& rStrm )
{   }

UINT16 ExcFilterMode::GetNum() const
{
    return 0x009B;
}

UINT16 ExcFilterMode::GetLen() const
{
    return 0;
}




ExcAutoFilterInfo::~ExcAutoFilterInfo()
{   }

void ExcAutoFilterInfo::SaveCont( SvStream& rStrm )
{
    rStrm << nCount;
}

UINT16 ExcAutoFilterInfo::GetNum() const
{
    return 0x009D;
}

UINT16 ExcAutoFilterInfo::GetLen() const
{
    return 2;
}




ExcFilterCondition::ExcFilterCondition() :
        nType( EXC_AFTYPE_NOTUSED ),
        nOper( EXC_AFOPER_EQUAL ),
        fVal( 0.0 ),
        pText( NULL )
{   }

ExcFilterCondition::~ExcFilterCondition()
{
    if( pText )
        delete pText;
}

UINT16 ExcFilterCondition::GetTextBytes() const
{
    if( !pText )
        return 0;
    return 1 + pText->GetByteCount();
}

void ExcFilterCondition::SetCondition( UINT8 nTp, UINT8 nOp, double fV, String* pT )
{
    nType = nTp;
    nOper = nOp;
    fVal = fV;

    if( pText )
        delete pText;
    pText = pT ? new XclRawUnicodeString( *pT ) : NULL;
}

void ExcFilterCondition::Save( SvStream& rStrm )
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

void ExcFilterCondition::SaveText( SvStream& rStrm )
{
    if( nType == EXC_AFTYPE_STRING )
    {
        DBG_ASSERT( pText, "ExcFilterCondition::SaveText() -- pText is NULL!" );
        rStrm << pText->GetGrbit();
        pText->WriteToStream( rStrm );
    }
}




ExcAutoFilter::ExcAutoFilter( UINT16 nC ) :
        nCol( nC ),
        nFlags( 0 )
{   }

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

void ExcAutoFilter::SaveCont( SvStream& rStrm )
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

UINT16 ExcAutoFilter::GetLen() const
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

void ExcAutoFilterRecs::Save( SvStream& rStrm )
{
    if( pFilterMode )
        pFilterMode->Save( rStrm );
    if( pFilterInfo )
        pFilterInfo->Save( rStrm );
    for( ExcAutoFilter* pFilter = _First(); pFilter; pFilter = _Next() )
        pFilter->Save( rStrm );
}




void ExcMargin::SaveCont( SvStream& r )
{
    double  f = nVal;

    f /= 1440.0;

    r << f;
}


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


UINT16 ExcMargin::GetNum() const
{
    return nId;
}


UINT16 ExcMargin::GetLen() const
{
    return 8;
}


