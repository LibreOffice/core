/*************************************************************************
 *
 *  $RCSfile: xcl97dum.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 13:44:13 $
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


#include "xcl97dum.hxx"


// --- ExcDummy8_xx Data ---------------------------------------------

// ... (8+) := neu in Biff8, ... (8*) := anders in Biff8

const BYTE ExcDummy8_00a::pMyData[] = {
    0xe1, 0x00, 0x02, 0x00, 0xb0, 0x04,                     // INTERFACEHDR
    0xc1, 0x00, 0x02, 0x00, 0x00, 0x00,                     // MMS
    0xe2, 0x00, 0x00, 0x00,                                 // INTERFACEEND
    0x5c, 0x00, 0x70, 0x00,                                 // WRITEACCESS (8*)
    0x04, 0x00, 0x00,  'C',  'a',  'l',  'c', 0x20,         // "Calc"
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x42, 0x00, 0x02, 0x00, 0xb0, 0x04,                     // CODEPAGE
    0x61, 0x01, 0x02, 0x00, 0x00, 0x00                      // DSF (8+)
};
const ULONG ExcDummy8_00a::nMyLen = sizeof(ExcDummy8_00a::pMyData);

                                                            // TABID (8+): ExcTabid

const BYTE ExcDummy8_00b::pMyData[] = {
    0x9c, 0x00, 0x02, 0x00, 0x0e, 0x00                      // FNGROUPCOUNT
};
const ULONG ExcDummy8_00b::nMyLen = sizeof(ExcDummy8_00b::pMyData);


const BYTE ExcDummy8_040::pMyData[] = {
    0x13, 0x00, 0x02, 0x00, 0x00, 0x00,                     // PASSWORD
    0xaf, 0x01, 0x02, 0x00, 0x00, 0x00,                     // PROT4REV (8+)
    0xbc, 0x01, 0x02, 0x00, 0x00, 0x00,                     // PROT4REVPASS (8+)
//  0x3d, 0x00, 0x12, 0x00, 0xe0, 0x01, 0x5a, 0x00, 0xcf,   // WINDOW1
//  0x3f, 0x4e, 0x2a, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00,
//  0x01, 0x00, 0x58, 0x02,
    0x40, 0x00, 0x02, 0x00, 0x00, 0x00,                     // BACKUP
    0x8d, 0x00, 0x02, 0x00, 0x00, 0x00                      // HIDEOBJ
};
const ULONG ExcDummy8_040::nMyLen = sizeof(ExcDummy8_040::pMyData);


const BYTE ExcDummy8_041::pMyData[] = {
    0x0e, 0x00, 0x02, 0x00, 0x01, 0x00,                     // PRECISION
    0xb7, 0x01, 0x02, 0x00, 0x00, 0x00,                     // REFRESHALL (8+)
    0xda, 0x00, 0x02, 0x00, 0x00, 0x00                      // BOOKBOOL
};
const ULONG ExcDummy8_041::nMyLen = sizeof(ExcDummy8_041::pMyData);



// Natural Language Formulas Flag (8+)  0 := nein, 1:= ja
const BYTE ExcDummy8_UsesElfs::pMyData[] = {
    0x60, 0x01, 0x02, 0x00, 0x01, 0x00
};
const ULONG ExcDummy8_UsesElfs::nMyLen = sizeof(ExcDummy8_UsesElfs::pMyData);



const BYTE ExcDummy8_02::pMyData[] = {
    0x5f, 0x00, 0x02, 0x00, 0x01, 0x00,                     // SAVERECALC
    0x25, 0x02, 0x04, 0x00, 0x00, 0x00, 0xff, 0x00          // DEFAULTROWHEIGHT
    };
const ULONG ExcDummy8_02::nMyLen = sizeof(ExcDummy8_02::pMyData);


const BYTE  XclRefmode::pMyData[] = { 0x0f, 0x00, 0x02, 0x00, 0x01, 0x00 }; // REFMODE
const ULONG XclRefmode::nMyLen = sizeof( XclRefmode::pMyData );


// --- class ExcDummy8_xx --------------------------------------------

ULONG ExcDummy8_00a::GetLen() const
{
    return nMyLen;
}


const BYTE* ExcDummy8_00a::GetData() const
{
    return pMyData;
}



ULONG ExcDummy8_00b::GetLen() const
{
    return nMyLen;
}


const BYTE* ExcDummy8_00b::GetData() const
{
    return pMyData;
}



ULONG ExcDummy8_040::GetLen() const
{
    return nMyLen;
}


const BYTE* ExcDummy8_040::GetData() const
{
    return pMyData;
}



ULONG ExcDummy8_041::GetLen() const
{
    return nMyLen;
}


const BYTE* ExcDummy8_041::GetData() const
{
    return pMyData;
}



ULONG ExcDummy8_UsesElfs::GetLen() const
{
    return nMyLen;
}


const BYTE* ExcDummy8_UsesElfs::GetData() const
{
    return pMyData;
}



ULONG ExcDummy8_02::GetLen() const
{
    return nMyLen;
}


const BYTE* ExcDummy8_02::GetData() const
{
    return pMyData;
}



ULONG XclRefmode::GetLen( void ) const
{
    return nMyLen;
}


const BYTE* XclRefmode::GetData( void ) const
{
    return pMyData;
}



