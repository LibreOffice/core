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
#include "precompiled_basic.hxx"

#include "runtime.hxx"
#include "stdobj.hxx"
#include "rtlproto.hxx"
#include "errobject.hxx"
#include "basegfx/numeric/ftools.hxx"


// Properties und Methoden legen beim Get (bWrite = FALSE) den Returnwert
// im Element 0 des Argv ab; beim Put (bWrite = TRUE) wird der Wert aus
// Element 0 gespeichert.

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
            INT32 nVal = rPar.Get( 0 )->GetLong();
            if( nVal <= 65535L )
                StarBASIC::Error( StarBASIC::GetSfxFromVBError( (USHORT) nVal ) );
        }
        else
            rPar.Get( 0 )->PutLong( StarBASIC::GetVBErrorCode( StarBASIC::GetErrBasic() ) );
    }
}

RTLFUNC(False)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutBool( FALSE );
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

    // liefert eine leere Objekt-Variable.
    rPar.Get( 0 )->PutObject( NULL );
}

RTLFUNC(Null)
{
    (void)pBasic;
    (void)bWrite;

    // liefert eine leere Objekt-Variable.
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

    rPar.Get( 0 )->PutBool( TRUE );
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

RTLFUNC(LINEPROP)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(0);
}
RTLFUNC(LINE_1)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(1);
}
RTLFUNC(LINE_15)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(2);
}
RTLFUNC(LINE_2)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(3);
}

RTLFUNC(TYP_JUMPEDITFLD)
{
    (void)pBasic;
    (void)bWrite;

    rPar.Get(0)->PutInteger(38);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
