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


#include "runtime.hxx"
#include "stdobj.hxx"
#include "rtlproto.hxx"
#include "errobject.hxx"
#include <basegfx/numeric/ftools.hxx>


// Properties and methods lay the return value down at Get (bWrite = sal_False)
// at the element 0 of the Argv; at Put (bWrite = sal_True) the value from
// element 0 is stored.

RTLFUNC(Erl)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get( 0 )->PutLong( StarBASIC::GetErl() );
}

RTLFUNC(Err)
{
    (void)pBasic;
    (void)bWrite;

    if( SbiRuntime::isVBAEnabled() )
    {
        rPar.Get( 0 )->PutObject( SbxErrObject::getErrObject() );
    }
    else
    {
        if( bWrite )
        {
            sal_Int32 nVal = rPar.Get( 0 )->GetLong();
            if( nVal <= 65535L )
                StarBASIC::Error( StarBASIC::GetSfxFromVBError( (sal_uInt16) nVal ) );
        }
        else
            rPar.Get( 0 )->PutLong( StarBASIC::GetVBErrorCode( StarBASIC::GetErrBasic() ) );
    }
}

RTLFUNC(False)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutBool( false );
}

RTLFUNC(Empty)
{
    (void)pBasic;
    (void)bWrite;
    (void)rPar;
}

RTLFUNC(Nothing)
{
    (void)pBasic;
    (void)bWrite;

    // return an empty object
    rPar.Get( 0 )->PutObject( nullptr );
}

RTLFUNC(Null)
{
    (void)pBasic;
    (void)bWrite;

    // returns an empty object-variable
    rPar.Get( 0 )->PutNull();
}

RTLFUNC(PI)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get( 0 )->PutDouble( F_PI );
}

RTLFUNC(True)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get( 0 )->PutBool( true );
}

RTLFUNC(ATTR_NORMAL)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(0);
}
RTLFUNC(ATTR_READONLY)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(1);
}
RTLFUNC(ATTR_HIDDEN)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(2);
}
RTLFUNC(ATTR_SYSTEM)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(4);
}
RTLFUNC(ATTR_VOLUME)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(8);
}
RTLFUNC(ATTR_DIRECTORY)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(16);
}
RTLFUNC(ATTR_ARCHIVE)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(32);
}

RTLFUNC(V_EMPTY)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(0);
}
RTLFUNC(V_NULL)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(1);
}
RTLFUNC(V_INTEGER)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(2);
}
RTLFUNC(V_LONG)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(3);
}
RTLFUNC(V_SINGLE)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(4);
}
RTLFUNC(V_DOUBLE)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(5);
}
RTLFUNC(V_CURRENCY)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(6);
}
RTLFUNC(V_DATE)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(7);
}
RTLFUNC(V_STRING)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(8);
}

RTLFUNC(MB_OK)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(0);
}
RTLFUNC(MB_OKCANCEL)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(1);
}
RTLFUNC(MB_ABORTRETRYIGNORE)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(2);
}
RTLFUNC(MB_YESNOCANCEL)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(3);
}
RTLFUNC(MB_YESNO)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(4);
}
RTLFUNC(MB_RETRYCANCEL)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(5);
}
RTLFUNC(MB_ICONSTOP)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(16);
}
RTLFUNC(MB_ICONQUESTION)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(32);
}
RTLFUNC(MB_ICONEXCLAMATION)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(48);
}
RTLFUNC(MB_ICONINFORMATION)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(64);
}
RTLFUNC(MB_DEFBUTTON1)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(0);
}
RTLFUNC(MB_DEFBUTTON2)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(256);
}
RTLFUNC(MB_DEFBUTTON3)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(512);
}
RTLFUNC(MB_APPLMODAL)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(0);
}
RTLFUNC(MB_SYSTEMMODAL)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(4096);
}

RTLFUNC(IDOK)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(1);
}

RTLFUNC(IDCANCEL)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(2);
}
RTLFUNC(IDABORT)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(3);
}
RTLFUNC(IDRETRY)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(4);
}
RTLFUNC(IDYES)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(6);
}
RTLFUNC(IDNO)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(7);
}

RTLFUNC(CF_TEXT)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(1);
}
RTLFUNC(CF_BITMAP)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(2);
}
RTLFUNC(CF_METAFILEPICT)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(3);
}

RTLFUNC(TYP_AUTHORFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(7);
}
RTLFUNC(TYP_CHAPTERFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(4);
}
RTLFUNC(TYP_CONDTXTFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(27);
}
RTLFUNC(TYP_DATEFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(0);
}
RTLFUNC(TYP_DBFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(19);
}
RTLFUNC(TYP_DBNAMEFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(3);
}
RTLFUNC(TYP_DBNEXTSETFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(24);
}
RTLFUNC(TYP_DBNUMSETFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(25);
}
RTLFUNC(TYP_DBSETNUMBERFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(26);
}
RTLFUNC(TYP_DDEFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(14);
}
RTLFUNC(TYP_DOCINFOFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(18);
}
RTLFUNC(TYP_DOCSTATFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(6);
}
RTLFUNC(TYP_EXTUSERFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(30);
}
RTLFUNC(TYP_FILENAMEFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(2);
}
RTLFUNC(TYP_FIXDATEFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(31);
}
RTLFUNC(TYP_FIXTIMEFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(32);
}
RTLFUNC(TYP_FORMELFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(10);
}
RTLFUNC(TYP_GETFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(9);
}
RTLFUNC(TYP_GETREFFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(13);
}
RTLFUNC(TYP_HIDDENPARAFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(17);
}
RTLFUNC(TYP_HIDDENTXTFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(11);
}
RTLFUNC(TYP_INPUTFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(16);
}
RTLFUNC(TYP_MACROFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(15);
}
RTLFUNC(TYP_NEXTPAGEFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(28);
}
RTLFUNC(TYP_PAGENUMBERFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(5);
}
RTLFUNC(TYP_POSTITFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(21);
}
RTLFUNC(TYP_PREVPAGEFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(29);
}
RTLFUNC(TYP_SEQFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(23);
}
RTLFUNC(TYP_SETFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(8);
}
RTLFUNC(TYP_SETINPFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(33);
}
RTLFUNC(TYP_SETREFFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(12);
}
RTLFUNC(TYP_TEMPLNAMEFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(22);
}
RTLFUNC(TYP_TIMEFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(1);
}
RTLFUNC(TYP_USERFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(20);
}
RTLFUNC(TYP_USRINPFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(34);
}
RTLFUNC(TYP_SETREFPAGEFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(35);
}
RTLFUNC(TYP_GETREFPAGEFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(36);
}
RTLFUNC(TYP_INTERNETFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(37);
}

RTLFUNC(SET_ON)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(1);
}
RTLFUNC(SET_OFF)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(0);
}
RTLFUNC(TOGGLE)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(2);
}

RTLFUNC(FRAMEANCHORPAGE)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(1);
}
RTLFUNC(FRAMEANCHORPARA)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(14);
}
RTLFUNC(FRAMEANCHORCHAR)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(15);
}

RTLFUNC(CLEAR_ALLTABS)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(2);
}
RTLFUNC(CLEAR_TAB)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(1);
}
RTLFUNC(SET_TAB)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(0);
}

RTLFUNC(TYP_JUMPEDITFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(38);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
