/*************************************************************************
 *
 *  $RCSfile: props.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:11 $
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

#include <svtools/sbx.hxx>
#include "runtime.hxx"
#pragma hdrstop
#include "stdobj.hxx"
#include "rtlproto.hxx"

#include "segmentc.hxx"
#pragma SW_SEGMENT_CLASS( SBRUNTIME, SBRUNTIME_CODE )

#if defined (OS2) && defined (__BORLANDC__)
#pragma option -w-par
#endif


// Properties und Methoden legen beim Get (bWrite = FALSE) den Returnwert
// im Element 0 des Argv ab; beim Put (bWrite = TRUE) wird der Wert aus
// Element 0 gespeichert.

RTLFUNC(Erl)
{
    rPar.Get( 0 )->PutLong( StarBASIC::GetErl() );
}

RTLFUNC(Err)
{
    if( bWrite )
    {
        INT32 nVal = rPar.Get( 0 )->GetLong();
        if( nVal <= 65535L )
            StarBASIC::Error( StarBASIC::GetSfxFromVBError( (USHORT) nVal ) );
    }
    else
        rPar.Get( 0 )->PutLong( StarBASIC::GetVBErrorCode( StarBASIC::GetErr() ) );
}

RTLFUNC(False)
{
    rPar.Get(0)->PutBool( FALSE );
}

RTLFUNC(Nothing)
{
    // liefert eine leere Objekt-Variable.
    rPar.Get( 0 )->PutObject( NULL );
}

RTLFUNC(Null)
{
    // liefert eine leere Objekt-Variable.
    rPar.Get( 0 )->PutNull();
}

RTLFUNC(PI)
{
    rPar.Get( 0 )->PutDouble( F_PI );
}

RTLFUNC(True)
{
    rPar.Get( 0 )->PutBool( TRUE );
}

RTLFUNC(ATTR_NORMAL)
{
    rPar.Get(0)->PutInteger(0);
}
RTLFUNC(ATTR_READONLY)
{
    rPar.Get(0)->PutInteger(1);
}
RTLFUNC(ATTR_HIDDEN)
{
    rPar.Get(0)->PutInteger(2);
}
RTLFUNC(ATTR_SYSTEM)
{
    rPar.Get(0)->PutInteger(4);
}
RTLFUNC(ATTR_VOLUME)
{
    rPar.Get(0)->PutInteger(8);
}
RTLFUNC(ATTR_DIRECTORY)
{
    rPar.Get(0)->PutInteger(16);
}
RTLFUNC(ATTR_ARCHIVE)
{
    rPar.Get(0)->PutInteger(32);
}

RTLFUNC(V_EMPTY)
{
    rPar.Get(0)->PutInteger(0);
}
RTLFUNC(V_NULL)
{
    rPar.Get(0)->PutInteger(1);
}
RTLFUNC(V_INTEGER)
{
    rPar.Get(0)->PutInteger(2);
}
RTLFUNC(V_LONG)
{
    rPar.Get(0)->PutInteger(3);
}
RTLFUNC(V_SINGLE)
{
    rPar.Get(0)->PutInteger(4);
}
RTLFUNC(V_DOUBLE)
{
    rPar.Get(0)->PutInteger(5);
}
RTLFUNC(V_CURRENCY)
{
    rPar.Get(0)->PutInteger(6);
}
RTLFUNC(V_DATE)
{
    rPar.Get(0)->PutInteger(7);
}
RTLFUNC(V_STRING)
{
    rPar.Get(0)->PutInteger(8);
}

RTLFUNC(MB_OK)
{
    rPar.Get(0)->PutInteger(0);
}
RTLFUNC(MB_OKCANCEL)
{
    rPar.Get(0)->PutInteger(1);
}
RTLFUNC(MB_ABORTRETRYIGNORE)
{
    rPar.Get(0)->PutInteger(2);
}
RTLFUNC(MB_YESNOCANCEL)
{
    rPar.Get(0)->PutInteger(3);
}
RTLFUNC(MB_YESNO)
{
    rPar.Get(0)->PutInteger(4);
}
RTLFUNC(MB_RETRYCANCEL)
{
    rPar.Get(0)->PutInteger(5);
}
RTLFUNC(MB_ICONSTOP)
{
    rPar.Get(0)->PutInteger(16);
}
RTLFUNC(MB_ICONQUESTION)
{
    rPar.Get(0)->PutInteger(32);
}
RTLFUNC(MB_ICONEXCLAMATION)
{
    rPar.Get(0)->PutInteger(48);
}
RTLFUNC(MB_ICONINFORMATION)
{
    rPar.Get(0)->PutInteger(64);
}
RTLFUNC(MB_DEFBUTTON1)
{
    rPar.Get(0)->PutInteger(0);
}
RTLFUNC(MB_DEFBUTTON2)
{
    rPar.Get(0)->PutInteger(256);
}
RTLFUNC(MB_DEFBUTTON3)
{
    rPar.Get(0)->PutInteger(512);
}
RTLFUNC(MB_APPLMODAL)
{
    rPar.Get(0)->PutInteger(0);
}
RTLFUNC(MB_SYSTEMMODAL)
{
    rPar.Get(0)->PutInteger(4096);
}

RTLFUNC(IDOK)
{
    rPar.Get(0)->PutInteger(1);
}

RTLFUNC(IDCANCEL)
{
    rPar.Get(0)->PutInteger(2);
}
RTLFUNC(IDABORT)
{
    rPar.Get(0)->PutInteger(3);
}
RTLFUNC(IDRETRY)
{
    rPar.Get(0)->PutInteger(4);
}
RTLFUNC(IDYES)
{
    rPar.Get(0)->PutInteger(6);
}
RTLFUNC(IDNO)
{
    rPar.Get(0)->PutInteger(7);
}

RTLFUNC(CF_TEXT)
{
    rPar.Get(0)->PutInteger(1);
}
RTLFUNC(CF_BITMAP)
{
    rPar.Get(0)->PutInteger(2);
}
RTLFUNC(CF_METAFILEPICT)
{
    rPar.Get(0)->PutInteger(3);
}

RTLFUNC(TYP_AUTHORFLD)
{
    rPar.Get(0)->PutInteger(7);
}
RTLFUNC(TYP_CHAPTERFLD)
{
    rPar.Get(0)->PutInteger(4);
}
RTLFUNC(TYP_CONDTXTFLD)
{
    rPar.Get(0)->PutInteger(27);
}
RTLFUNC(TYP_DATEFLD)
{
    rPar.Get(0)->PutInteger(0);
}
RTLFUNC(TYP_DBFLD)
{
    rPar.Get(0)->PutInteger(19);
}
RTLFUNC(TYP_DBNAMEFLD)
{
    rPar.Get(0)->PutInteger(3);
}
RTLFUNC(TYP_DBNEXTSETFLD)
{
    rPar.Get(0)->PutInteger(24);
}
RTLFUNC(TYP_DBNUMSETFLD)
{
    rPar.Get(0)->PutInteger(25);
}
RTLFUNC(TYP_DBSETNUMBERFLD)
{
    rPar.Get(0)->PutInteger(26);
}
RTLFUNC(TYP_DDEFLD)
{
    rPar.Get(0)->PutInteger(14);
}
RTLFUNC(TYP_DOCINFOFLD)
{
    rPar.Get(0)->PutInteger(18);
}
RTLFUNC(TYP_DOCSTATFLD)
{
    rPar.Get(0)->PutInteger(6);
}
RTLFUNC(TYP_EXTUSERFLD)
{
    rPar.Get(0)->PutInteger(30);
}
RTLFUNC(TYP_FILENAMEFLD)
{
    rPar.Get(0)->PutInteger(2);
}
RTLFUNC(TYP_FIXDATEFLD)
{
    rPar.Get(0)->PutInteger(31);
}
RTLFUNC(TYP_FIXTIMEFLD)
{
    rPar.Get(0)->PutInteger(32);
}
RTLFUNC(TYP_FORMELFLD)
{
    rPar.Get(0)->PutInteger(10);
}
RTLFUNC(TYP_GETFLD)
{
    rPar.Get(0)->PutInteger(9);
}
RTLFUNC(TYP_GETREFFLD)
{
    rPar.Get(0)->PutInteger(13);
}
RTLFUNC(TYP_HIDDENPARAFLD)
{
    rPar.Get(0)->PutInteger(17);
}
RTLFUNC(TYP_HIDDENTXTFLD)
{
    rPar.Get(0)->PutInteger(11);
}
RTLFUNC(TYP_INPUTFLD)
{
    rPar.Get(0)->PutInteger(16);
}
RTLFUNC(TYP_MACROFLD)
{
    rPar.Get(0)->PutInteger(15);
}
RTLFUNC(TYP_NEXTPAGEFLD)
{
    rPar.Get(0)->PutInteger(28);
}
RTLFUNC(TYP_PAGENUMBERFLD)
{
    rPar.Get(0)->PutInteger(5);
}
RTLFUNC(TYP_POSTITFLD)
{
    rPar.Get(0)->PutInteger(21);
}
RTLFUNC(TYP_PREVPAGEFLD)
{
    rPar.Get(0)->PutInteger(29);
}
RTLFUNC(TYP_SEQFLD)
{
    rPar.Get(0)->PutInteger(23);
}
RTLFUNC(TYP_SETFLD)
{
    rPar.Get(0)->PutInteger(8);
}
RTLFUNC(TYP_SETINPFLD)
{
    rPar.Get(0)->PutInteger(33);
}
RTLFUNC(TYP_SETREFFLD)
{
    rPar.Get(0)->PutInteger(12);
}
RTLFUNC(TYP_TEMPLNAMEFLD)
{
    rPar.Get(0)->PutInteger(22);
}
RTLFUNC(TYP_TIMEFLD)
{
    rPar.Get(0)->PutInteger(1);
}
RTLFUNC(TYP_USERFLD)
{
    rPar.Get(0)->PutInteger(20);
}
RTLFUNC(TYP_USRINPFLD)
{
    rPar.Get(0)->PutInteger(34);
}
RTLFUNC(TYP_SETREFPAGEFLD)
{
    rPar.Get(0)->PutInteger(35);
}
RTLFUNC(TYP_GETREFPAGEFLD)
{
    rPar.Get(0)->PutInteger(36);
}
RTLFUNC(TYP_INTERNETFLD)
{
    rPar.Get(0)->PutInteger(37);
}

RTLFUNC(SET_ON)
{
    rPar.Get(0)->PutInteger(1);
}
RTLFUNC(SET_OFF)
{
    rPar.Get(0)->PutInteger(0);
}
RTLFUNC(TOGGLE)
{
    rPar.Get(0)->PutInteger(2);
}

RTLFUNC(FRAMEANCHORPAGE)
{
    rPar.Get(0)->PutInteger(1);
}
RTLFUNC(FRAMEANCHORPARA)
{
    rPar.Get(0)->PutInteger(14);
}
RTLFUNC(FRAMEANCHORCHAR)
{
    rPar.Get(0)->PutInteger(15);
}

RTLFUNC(CLEAR_ALLTABS)
{
    rPar.Get(0)->PutInteger(2);
}
RTLFUNC(CLEAR_TAB)
{
    rPar.Get(0)->PutInteger(1);
}
RTLFUNC(SET_TAB)
{
    rPar.Get(0)->PutInteger(0);
}

RTLFUNC(LINEPROP)
{
    rPar.Get(0)->PutInteger(0);
}
RTLFUNC(LINE_1)
{
    rPar.Get(0)->PutInteger(1);
}
RTLFUNC(LINE_15)
{
    rPar.Get(0)->PutInteger(2);
}
RTLFUNC(LINE_2)
{
    rPar.Get(0)->PutInteger(3);
}

RTLFUNC(TYP_JUMPEDITFLD)
{
    rPar.Get(0)->PutInteger(38);
}


