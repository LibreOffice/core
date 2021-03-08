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


#include <runtime.hxx>
#include <rtlproto.hxx>
#include <errobject.hxx>
#include <basegfx/numeric/ftools.hxx>


// Properties and methods lay the return value down at Get (bWrite = sal_False)
// at the element 0 of the Argv; at Put (bWrite = sal_True) the value from
// element 0 is stored.

void SbRtl_Erl(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutLong(StarBASIC::GetErl()); }

void SbRtl_Err(StarBASIC *, SbxArray & rPar, bool bWrite)
{
    if( SbiRuntime::isVBAEnabled() )
    {
        rPar.Get(0)->PutObject(SbxErrObject::getErrObject().get());
    }
    else
    {
        if( bWrite )
        {
            sal_Int32 nVal = rPar.Get(0)->GetLong();
            if( nVal <= 65535 )
                StarBASIC::Error( StarBASIC::GetSfxFromVBError( static_cast<sal_uInt16>(nVal) ) );
        }
        else
            rPar.Get(0)->PutLong(StarBASIC::GetVBErrorCode(StarBASIC::GetErrBasic()));
    }
}

void SbRtl_False(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutBool(false); }

void SbRtl_Empty(StarBASIC *, SbxArray &, bool) {}

void SbRtl_Nothing(StarBASIC *, SbxArray & rPar, bool)
{
    // return an empty object
    rPar.Get(0)->PutObject(nullptr);
}

void SbRtl_Null(StarBASIC *, SbxArray & rPar, bool)
{
    // returns an empty object-variable
    rPar.Get(0)->PutNull();
}

void SbRtl_PI(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutDouble(F_PI); }

void SbRtl_True(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutBool(true); }

void SbRtl_ATTR_NORMAL(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(0); }
void SbRtl_ATTR_READONLY(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(1); }
void SbRtl_ATTR_HIDDEN(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(2); }
void SbRtl_ATTR_SYSTEM(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(4); }
void SbRtl_ATTR_VOLUME(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(8); }
void SbRtl_ATTR_DIRECTORY(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(16); }
void SbRtl_ATTR_ARCHIVE(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(32); }

void SbRtl_V_EMPTY(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(0); }
void SbRtl_V_NULL(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(1); }
void SbRtl_V_INTEGER(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(2); }
void SbRtl_V_LONG(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(3); }
void SbRtl_V_SINGLE(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(4); }
void SbRtl_V_DOUBLE(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(5); }
void SbRtl_V_CURRENCY(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(6); }
void SbRtl_V_DATE(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(7); }
void SbRtl_V_STRING(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(8); }

void SbRtl_MB_OK(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(0); }
void SbRtl_MB_OKCANCEL(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(1); }
void SbRtl_MB_ABORTRETRYIGNORE(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(2); }
void SbRtl_MB_YESNOCANCEL(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(3); }
void SbRtl_MB_YESNO(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(4); }
void SbRtl_MB_RETRYCANCEL(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(5); }
void SbRtl_MB_ICONSTOP(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(16); }
void SbRtl_MB_ICONQUESTION(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(32); }
void SbRtl_MB_ICONEXCLAMATION(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(48); }
void SbRtl_MB_ICONINFORMATION(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(64); }
void SbRtl_MB_DEFBUTTON1(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(0); }
void SbRtl_MB_DEFBUTTON2(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(256); }
void SbRtl_MB_DEFBUTTON3(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(512); }
void SbRtl_MB_APPLMODAL(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(0); }
void SbRtl_MB_SYSTEMMODAL(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(4096); }

void SbRtl_IDOK(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(1); }

void SbRtl_IDCANCEL(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(2); }
void SbRtl_IDABORT(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(3); }
void SbRtl_IDRETRY(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(4); }
void SbRtl_IDYES(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(6); }
void SbRtl_IDNO(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(7); }

void SbRtl_CF_TEXT(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(1); }
void SbRtl_CF_BITMAP(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(2); }
void SbRtl_CF_METAFILEPICT(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(3); }

void SbRtl_TYP_AUTHORFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(7); }
void SbRtl_TYP_CHAPTERFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(4); }
void SbRtl_TYP_CONDTXTFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(27); }
void SbRtl_TYP_DATEFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(0); }
void SbRtl_TYP_DBFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(19); }
void SbRtl_TYP_DBNAMEFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(3); }
void SbRtl_TYP_DBNEXTSETFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(24); }
void SbRtl_TYP_DBNUMSETFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(25); }
void SbRtl_TYP_DBSETNUMBERFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(26); }
void SbRtl_TYP_DDEFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(14); }
void SbRtl_TYP_DOCINFOFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(18); }
void SbRtl_TYP_DOCSTATFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(6); }
void SbRtl_TYP_EXTUSERFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(30); }
void SbRtl_TYP_FILENAMEFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(2); }
void SbRtl_TYP_FIXDATEFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(31); }
void SbRtl_TYP_FIXTIMEFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(32); }
void SbRtl_TYP_FORMELFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(10); }
void SbRtl_TYP_GETFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(9); }
void SbRtl_TYP_GETREFFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(13); }
void SbRtl_TYP_HIDDENPARAFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(17); }
void SbRtl_TYP_HIDDENTXTFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(11); }
void SbRtl_TYP_INPUTFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(16); }
void SbRtl_TYP_MACROFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(15); }
void SbRtl_TYP_NEXTPAGEFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(28); }
void SbRtl_TYP_PAGENUMBERFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(5); }
void SbRtl_TYP_POSTITFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(21); }
void SbRtl_TYP_PREVPAGEFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(29); }
void SbRtl_TYP_SEQFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(23); }
void SbRtl_TYP_SETFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(8); }
void SbRtl_TYP_SETINPFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(33); }
void SbRtl_TYP_SETREFFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(12); }
void SbRtl_TYP_TEMPLNAMEFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(22); }
void SbRtl_TYP_TIMEFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(1); }
void SbRtl_TYP_USERFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(20); }
void SbRtl_TYP_USRINPFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(34); }
void SbRtl_TYP_SETREFPAGEFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(35); }
void SbRtl_TYP_GETREFPAGEFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(36); }
void SbRtl_TYP_INTERNETFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(37); }

void SbRtl_SET_ON(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(1); }
void SbRtl_SET_OFF(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(0); }
void SbRtl_TOGGLE(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(2); }

void SbRtl_FRAMEANCHORPAGE(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(1); }
void SbRtl_FRAMEANCHORPARA(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(14); }
void SbRtl_FRAMEANCHORCHAR(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(15); }

void SbRtl_CLEAR_ALLTABS(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(2); }
void SbRtl_CLEAR_TAB(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(1); }
void SbRtl_SET_TAB(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(0); }

void SbRtl_TYP_JUMPEDITFLD(StarBASIC*, SbxArray& rPar, bool) { rPar.Get(0)->PutInteger(38); }


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
