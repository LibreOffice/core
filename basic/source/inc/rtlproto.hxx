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

#pragma once

#include <basic/sbstar.hxx>

typedef void( *RtlCall ) ( StarBASIC* p, SbxArray& rArgs, bool bWrite );

// Properties

extern void SbRtl_Date(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Err(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Erl(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_False(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Empty(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Nothing(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Null(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_True(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);

extern void SbRtl_ATTR_NORMAL(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_ATTR_READONLY(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_ATTR_HIDDEN(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_ATTR_SYSTEM(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_ATTR_VOLUME(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_ATTR_DIRECTORY(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_ATTR_ARCHIVE(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);

extern void SbRtl_V_EMPTY(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_V_NULL(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_V_INTEGER(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_V_LONG(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_V_SINGLE(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_V_DOUBLE(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_V_CURRENCY(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_V_DATE(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_V_STRING(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);

extern void SbRtl_MB_OK(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_MB_OKCANCEL(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_MB_ABORTRETRYIGNORE(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_MB_YESNOCANCEL(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_MB_YESNO(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_MB_RETRYCANCEL(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_MB_ICONSTOP(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_MB_ICONQUESTION(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_MB_ICONEXCLAMATION(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_MB_ICONINFORMATION(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_MB_DEFBUTTON1(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_MB_DEFBUTTON2(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_MB_DEFBUTTON3(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_MB_APPLMODAL(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_MB_SYSTEMMODAL(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);

extern void SbRtl_IDOK(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_IDCANCEL(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_IDABORT(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_IDRETRY(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_IDYES(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_IDNO(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);

extern void SbRtl_CF_TEXT(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_CF_BITMAP(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_CF_METAFILEPICT(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);

extern void SbRtl_PI(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);

extern void SbRtl_SET_OFF(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_SET_ON(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TOGGLE(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);

extern void SbRtl_TYP_AUTHORFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_CHAPTERFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_CONDTXTFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_DATEFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_DBFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_DBNAMEFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_DBNEXTSETFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_DBNUMSETFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_DBSETNUMBERFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_DDEFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_DOCINFOFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_DOCSTATFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_EXTUSERFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_FILENAMEFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_FIXDATEFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_FIXTIMEFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_FORMELFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_GETFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_GETREFFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_HIDDENPARAFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_HIDDENTXTFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_INPUTFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_MACROFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_NEXTPAGEFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_PAGENUMBERFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_POSTITFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_PREVPAGEFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_SEQFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_SETFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_SETINPFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_SETREFFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_TEMPLNAMEFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_TIMEFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_USERFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_USRINPFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_SETREFPAGEFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_GETREFPAGEFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_INTERNETFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TYP_JUMPEDITFLD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);

extern void SbRtl_FRAMEANCHORPAGE(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_FRAMEANCHORPARA(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_FRAMEANCHORCHAR(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);

extern void SbRtl_CLEAR_ALLTABS(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_CLEAR_TAB(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_SET_TAB(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);

// Methods

extern void SbRtl_CreateObject(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Error(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Sin(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Abs(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Asc(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Atn(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Chr(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_ChrW(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Cos(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_CurDir(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_ChDir(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);  // JSM
extern void SbRtl_ChDrive(StarBASIC * pBasic, SbxArray & rPar, bool bWrite); // JSM
extern void SbRtl_FileCopy(StarBASIC * pBasic, SbxArray & rPar, bool bWrite); // JSM
extern void SbRtl_Kill(StarBASIC * pBasic, SbxArray & rPar, bool bWrite); // JSM
extern void SbRtl_MkDir(StarBASIC * pBasic, SbxArray & rPar, bool bWrite); // JSM
extern void SbRtl_RmDir(StarBASIC * pBasic, SbxArray & rPar, bool bWrite); // JSM
extern void SbRtl_SendKeys(StarBASIC * pBasic, SbxArray & rPar, bool bWrite); // JSM
extern void SbRtl_DDB(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_DimArray(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Dir(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_DoEvents(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Exp(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_FileLen(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Fix(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_FV(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Hex(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Input(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_InStr(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_InStrRev(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Int(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_IPmt(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_IRR(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Join(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_LCase(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Left(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Log(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_LTrim(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Mid(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_MIRR(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_NPer(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_NPV(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Oct(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Pmt(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_PPmt(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_PV(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Rate(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Replace(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Right(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_RTrim(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_RTL(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Sgn(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_SLN(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Space(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Split(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Sqr(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Str(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_StrComp(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_String(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_StrReverse(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_SYD(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Tab(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Tan(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_UCase(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Val(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Len(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Spc(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_DateSerial(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TimeSerial(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_DateValue(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TimeValue(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Day(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Hour(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Minute(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Month(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_MonthName(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Now(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Second(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Time(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Timer(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Weekday(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_WeekdayName(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Year(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_InputBox(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Me(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_MsgBox(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_IsArray(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_IsDate(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_IsEmpty(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_IsError(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_IsNull(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_IsNumeric(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_IsObject(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_IsUnoStruct(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);

extern void SbRtl_FileDateTime(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Format(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_FormatNumber(StarBASIC* pBasic, SbxArray& rPar, bool bWrite);
extern void SbRtl_GetAttr(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Randomize(StarBASIC * pBasic, SbxArray & rPar, bool bWrite); // JSM
extern void SbRtl_Round(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Frac(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Rnd(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Shell(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_VarType(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TypeName(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TypeLen(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);

extern void SbRtl_EOF(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_FileAttr(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Loc(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Lof(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Seek(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_SetAttr(StarBASIC * pBasic, SbxArray & rPar, bool bWrite); // JSM
extern void SbRtl_Reset(StarBASIC * pBasic, SbxArray & rPar, bool bWrite); // JSM

extern void SbRtl_DDEInitiate(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_DDETerminate(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_DDETerminateAll(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_DDERequest(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_DDEExecute(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_DDEPoke(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);

extern void SbRtl_FreeFile(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_IsMissing(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_LBound(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_UBound(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_RGB(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_QBColor(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_StrConv(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);

extern void SbRtl_Beep(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);

extern void SbRtl_Load(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Unload(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_LoadPicture(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_SavePicture(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);

extern void SbRtl_CallByName(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_CBool(StarBASIC * pBasic, SbxArray & rPar, bool bWrite); // JSM
extern void SbRtl_CByte(StarBASIC * pBasic, SbxArray & rPar, bool bWrite); // JSM
extern void SbRtl_CCur(StarBASIC * pBasic, SbxArray & rPar, bool bWrite); // JSM
extern void SbRtl_CDate(StarBASIC * pBasic, SbxArray & rPar, bool bWrite); // JSM
extern void SbRtl_CDbl(StarBASIC * pBasic, SbxArray & rPar, bool bWrite); // JSM
extern void SbRtl_CInt(StarBASIC * pBasic, SbxArray & rPar, bool bWrite); // JSM
extern void SbRtl_CLng(StarBASIC * pBasic, SbxArray & rPar, bool bWrite); // JSM
extern void SbRtl_CSng(StarBASIC * pBasic, SbxArray & rPar, bool bWrite); // JSM
extern void SbRtl_CStr(StarBASIC * pBasic, SbxArray & rPar, bool bWrite); // JSM
extern void SbRtl_CVar(StarBASIC * pBasic, SbxArray & rPar, bool bWrite); // JSM
extern void SbRtl_CVErr(StarBASIC * pBasic, SbxArray & rPar, bool bWrite); // JSM

extern void SbRtl_Iif(StarBASIC * pBasic, SbxArray & rPar, bool bWrite); // JSM

extern void SbRtl_DumpAllObjects(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);

extern void SbRtl_GetSystemType(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_GetGUIType(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Red(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Green(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Blue(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);

extern void SbRtl_Switch(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Wait(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
//i#64882# add new WaitUntil
extern void SbRtl_WaitUntil(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_FuncCaller(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);

extern void SbRtl_GetGUIVersion(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Choose(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Trim(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);

extern void SbRtl_DateAdd(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_DateDiff(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_DatePart(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_FormatDateTime(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_GetSolarVersion(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TwipsPerPixelX(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_TwipsPerPixelY(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_FreeLibrary(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Array(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_FindObject(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_FindPropertyObject(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_EnableReschedule(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);

extern void SbRtl_Put(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Get(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_Environ(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_GetDialogZoomFactorX(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_GetDialogZoomFactorY(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_GetSystemTicks(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_GetPathSeparator(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_ResolvePath(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_CreateUnoStruct(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_CreateUnoService(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_CreateUnoServiceWithArguments(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_CreateUnoValue(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_GetProcessServiceManager(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_GetDefaultContext(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_CreatePropertySet(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_CreateUnoListener(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_HasUnoInterfaces(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_EqualUnoObjects(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_CreateUnoDialog(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_GlobalScope(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_FileExists(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_ConvertToUrl(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_ConvertFromUrl(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_CDateToUnoDate(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_CDateFromUnoDate(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_CDateToUnoTime(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_CDateFromUnoTime(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_CDateToUnoDateTime(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_CDateFromUnoDateTime(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_CDateToIso(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_CDateFromIso(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_CompatibilityMode(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);
extern void SbRtl_CDec(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);

extern void SbRtl_Partition(StarBASIC * pBasic, SbxArray & rPar, bool bWrite);

extern bool LibreOffice6FloatingPointMode();
extern double Now_Impl();
extern void Wait_Impl( bool bDurationBased, SbxArray& rPar );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
