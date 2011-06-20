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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"




#include "xcl97dum.hxx"


// --- ExcDummy8_xx Data ---------------------------------------------

// ... (8+) := neu in Biff8, ... (8*) := anders in Biff8

const sal_uInt8 ExcDummy8_00a::pMyData[] = {
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
const sal_Size ExcDummy8_00a::nMyLen = sizeof(ExcDummy8_00a::pMyData);

                                                            // TABID (8+): ExcTabid

const sal_uInt8 ExcDummy8_00b::pMyData[] = {
    0x9c, 0x00, 0x02, 0x00, 0x0e, 0x00                      // FNGROUPCOUNT
};
const sal_Size ExcDummy8_00b::nMyLen = sizeof(ExcDummy8_00b::pMyData);


const sal_uInt8 ExcDummy8_040::pMyData[] = {
    0xaf, 0x01, 0x02, 0x00, 0x00, 0x00,                     // PROT4REV (8+)
    0xbc, 0x01, 0x02, 0x00, 0x00, 0x00,                     // PROT4REVPASS (8+)
//  0x3d, 0x00, 0x12, 0x00, 0xe0, 0x01, 0x5a, 0x00, 0xcf,   // WINDOW1
//  0x3f, 0x4e, 0x2a, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00,
//  0x01, 0x00, 0x58, 0x02,
    0x40, 0x00, 0x02, 0x00, 0x00, 0x00,                     // BACKUP
    0x8d, 0x00, 0x02, 0x00, 0x00, 0x00                      // HIDEOBJ
};
const sal_Size ExcDummy8_040::nMyLen = sizeof(ExcDummy8_040::pMyData);


const sal_uInt8 ExcDummy8_041::pMyData[] = {
    0xb7, 0x01, 0x02, 0x00, 0x00, 0x00,                     // REFRESHALL (8+)
    0xda, 0x00, 0x02, 0x00, 0x00, 0x00                      // BOOKBOOL
};
const sal_Size ExcDummy8_041::nMyLen = sizeof(ExcDummy8_041::pMyData);



const sal_uInt8 ExcDummy8_02::pMyData[] = {
    0x5f, 0x00, 0x02, 0x00, 0x01, 0x00                      // SAVERECALC
    };
const sal_Size ExcDummy8_02::nMyLen = sizeof(ExcDummy8_02::pMyData);


// --- class ExcDummy8_xx --------------------------------------------

sal_Size ExcDummy8_00a::GetLen() const
{
    return nMyLen;
}


const sal_uInt8* ExcDummy8_00a::GetData() const
{
    return pMyData;
}



sal_Size ExcDummy8_00b::GetLen() const
{
    return nMyLen;
}


const sal_uInt8* ExcDummy8_00b::GetData() const
{
    return pMyData;
}



sal_Size ExcDummy8_040::GetLen() const
{
    return nMyLen;
}


const sal_uInt8* ExcDummy8_040::GetData() const
{
    return pMyData;
}



sal_Size ExcDummy8_041::GetLen() const
{
    return nMyLen;
}


const sal_uInt8* ExcDummy8_041::GetData() const
{
    return pMyData;
}



sal_Size ExcDummy8_02::GetLen() const
{
    return nMyLen;
}


const sal_uInt8* ExcDummy8_02::GetData() const
{
    return pMyData;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
