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

#include <basic/sbstar.hxx>
#include "sbtrace.hxx"

#define RTLFUNC( name ) void SbRtl_##name( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite )
#define RTLNAME( name ) &SbRtl_##name

typedef void( *RtlCall ) ( StarBASIC* p, SbxArray& rArgs, sal_Bool bWrite );

// Properties

extern RTLFUNC(Date);
extern RTLFUNC(Err);
extern RTLFUNC(Erl);
extern RTLFUNC(False);
extern RTLFUNC(Empty);
extern RTLFUNC(Nothing);
extern RTLFUNC(Null);
extern RTLFUNC(True);

extern RTLFUNC(ATTR_NORMAL);
extern RTLFUNC(ATTR_READONLY);
extern RTLFUNC(ATTR_HIDDEN);
extern RTLFUNC(ATTR_SYSTEM);
extern RTLFUNC(ATTR_VOLUME);
extern RTLFUNC(ATTR_DIRECTORY);
extern RTLFUNC(ATTR_ARCHIVE);

extern RTLFUNC(V_EMPTY);
extern RTLFUNC(V_NULL);
extern RTLFUNC(V_INTEGER);
extern RTLFUNC(V_LONG);
extern RTLFUNC(V_SINGLE);
extern RTLFUNC(V_DOUBLE);
extern RTLFUNC(V_CURRENCY);
extern RTLFUNC(V_DATE);
extern RTLFUNC(V_STRING);

extern RTLFUNC(MB_OK);
extern RTLFUNC(MB_OKCANCEL);
extern RTLFUNC(MB_ABORTRETRYIGNORE);
extern RTLFUNC(MB_YESNOCANCEL);
extern RTLFUNC(MB_YESNO);
extern RTLFUNC(MB_RETRYCANCEL);
extern RTLFUNC(MB_ICONSTOP);
extern RTLFUNC(MB_ICONQUESTION);
extern RTLFUNC(MB_ICONEXCLAMATION);
extern RTLFUNC(MB_ICONINFORMATION);
extern RTLFUNC(MB_DEFBUTTON1);
extern RTLFUNC(MB_DEFBUTTON2);
extern RTLFUNC(MB_DEFBUTTON3);
extern RTLFUNC(MB_APPLMODAL);
extern RTLFUNC(MB_SYSTEMMODAL);

extern RTLFUNC(IDOK);
extern RTLFUNC(IDCANCEL);
extern RTLFUNC(IDABORT);
extern RTLFUNC(IDRETRY);
extern RTLFUNC(IDYES);
extern RTLFUNC(IDNO);

extern RTLFUNC(CF_TEXT);
extern RTLFUNC(CF_BITMAP);
extern RTLFUNC(CF_METAFILEPICT);

extern RTLFUNC(PI);

extern RTLFUNC(SET_OFF);
extern RTLFUNC(SET_ON);
extern RTLFUNC(TOGGLE);

extern RTLFUNC(TYP_AUTHORFLD);
extern RTLFUNC(TYP_CHAPTERFLD);
extern RTLFUNC(TYP_CONDTXTFLD);
extern RTLFUNC(TYP_DATEFLD);
extern RTLFUNC(TYP_DBFLD);
extern RTLFUNC(TYP_DBNAMEFLD);
extern RTLFUNC(TYP_DBNEXTSETFLD);
extern RTLFUNC(TYP_DBNUMSETFLD);
extern RTLFUNC(TYP_DBSETNUMBERFLD);
extern RTLFUNC(TYP_DDEFLD);
extern RTLFUNC(TYP_DOCINFOFLD);
extern RTLFUNC(TYP_DOCSTATFLD);
extern RTLFUNC(TYP_EXTUSERFLD);
extern RTLFUNC(TYP_FILENAMEFLD);
extern RTLFUNC(TYP_FIXDATEFLD);
extern RTLFUNC(TYP_FIXTIMEFLD);
extern RTLFUNC(TYP_FORMELFLD);
extern RTLFUNC(TYP_GETFLD);
extern RTLFUNC(TYP_GETREFFLD);
extern RTLFUNC(TYP_HIDDENPARAFLD);
extern RTLFUNC(TYP_HIDDENTXTFLD);
extern RTLFUNC(TYP_INPUTFLD);
extern RTLFUNC(TYP_MACROFLD);
extern RTLFUNC(TYP_NEXTPAGEFLD);
extern RTLFUNC(TYP_PAGENUMBERFLD);
extern RTLFUNC(TYP_POSTITFLD);
extern RTLFUNC(TYP_PREVPAGEFLD);
extern RTLFUNC(TYP_SEQFLD);
extern RTLFUNC(TYP_SETFLD);
extern RTLFUNC(TYP_SETINPFLD);
extern RTLFUNC(TYP_SETREFFLD);
extern RTLFUNC(TYP_TEMPLNAMEFLD);
extern RTLFUNC(TYP_TIMEFLD);
extern RTLFUNC(TYP_USERFLD);
extern RTLFUNC(TYP_USRINPFLD);
extern RTLFUNC(TYP_SETREFPAGEFLD);
extern RTLFUNC(TYP_GETREFPAGEFLD);
extern RTLFUNC(TYP_INTERNETFLD);
extern RTLFUNC(TYP_JUMPEDITFLD);

extern RTLFUNC(FRAMEANCHORPAGE);
extern RTLFUNC(FRAMEANCHORPARA);
extern RTLFUNC(FRAMEANCHORCHAR);

extern RTLFUNC(CLEAR_ALLTABS);
extern RTLFUNC(CLEAR_TAB);
extern RTLFUNC(SET_TAB);

extern RTLFUNC(LINEPROP);
extern RTLFUNC(LINE_1);
extern RTLFUNC(LINE_15);
extern RTLFUNC(LINE_2);

// Methoden

extern RTLFUNC(CreateObject);
extern RTLFUNC(Error);
extern RTLFUNC(Sin);
extern RTLFUNC(Abs);
extern RTLFUNC(Asc);
extern RTLFUNC(Atn);
extern RTLFUNC(Chr);
extern RTLFUNC(ChrW);
extern RTLFUNC(Cos);
extern RTLFUNC(CurDir);
extern RTLFUNC(ChDir);  // JSM
extern RTLFUNC(ChDrive); // JSM
extern RTLFUNC(FileCopy); // JSM
extern RTLFUNC(Kill); // JSM
extern RTLFUNC(MkDir); // JSM
extern RTLFUNC(RmDir); // JSM
extern RTLFUNC(SendKeys); // JSM
extern RTLFUNC(DDB);
extern RTLFUNC(DimArray);
extern RTLFUNC(Dir);
extern RTLFUNC(DoEvents);
extern RTLFUNC(Exp);
extern RTLFUNC(FileLen);
extern RTLFUNC(Fix);
extern RTLFUNC(FV);
extern RTLFUNC(Hex);
extern RTLFUNC(Input);
extern RTLFUNC(InStr);
extern RTLFUNC(InStrRev);
extern RTLFUNC(Int);
extern RTLFUNC(IPmt);
extern RTLFUNC(IRR);
extern RTLFUNC(Join);
extern RTLFUNC(LCase);
extern RTLFUNC(Left);
extern RTLFUNC(Log);
extern RTLFUNC(LTrim);
extern RTLFUNC(Mid);
extern RTLFUNC(MIRR);
extern RTLFUNC(NPer);
extern RTLFUNC(NPV);
extern RTLFUNC(Oct);
extern RTLFUNC(Pmt);
extern RTLFUNC(PPmt);
extern RTLFUNC(PV);
extern RTLFUNC(Rate);
extern RTLFUNC(Replace);
extern RTLFUNC(Right);
extern RTLFUNC(RTrim);
extern RTLFUNC(RTL);
extern RTLFUNC(Sgn);
extern RTLFUNC(SLN);
extern RTLFUNC(Space);
extern RTLFUNC(Split);
extern RTLFUNC(Sqr);
extern RTLFUNC(Str);
extern RTLFUNC(StrComp);
extern RTLFUNC(String);
extern RTLFUNC(StrReverse);
extern RTLFUNC(SYD);
extern RTLFUNC(Tan);
extern RTLFUNC(UCase);
extern RTLFUNC(Val);
extern RTLFUNC(Len);
extern RTLFUNC(Spc);
extern RTLFUNC(DateSerial);
extern RTLFUNC(TimeSerial);
extern RTLFUNC(DateValue);
extern RTLFUNC(TimeValue);
extern RTLFUNC(Day);
extern RTLFUNC(Hour);
extern RTLFUNC(Minute);
extern RTLFUNC(Month);
extern RTLFUNC(MonthName);
extern RTLFUNC(Now);
extern RTLFUNC(Second);
extern RTLFUNC(Time);
extern RTLFUNC(Timer);
extern RTLFUNC(Weekday);
extern RTLFUNC(WeekdayName);
extern RTLFUNC(Year);
extern RTLFUNC(Date);
extern RTLFUNC(InputBox);
extern RTLFUNC(Me);
extern RTLFUNC(MsgBox);
extern RTLFUNC(IsArray);
extern RTLFUNC(IsDate);
extern RTLFUNC(IsEmpty);
extern RTLFUNC(IsError);
extern RTLFUNC(IsNull);
extern RTLFUNC(IsNumeric);
extern RTLFUNC(IsObject);
extern RTLFUNC(IsUnoStruct);

extern RTLFUNC(FileDateTime);
extern RTLFUNC(Format);
extern RTLFUNC(GetAttr);
extern RTLFUNC(Randomize); // JSM
extern RTLFUNC(Round);
extern RTLFUNC(Frac);
extern RTLFUNC(Rnd);
extern RTLFUNC(Shell);
extern RTLFUNC(VarType);
extern RTLFUNC(TypeName);
extern RTLFUNC(TypeLen);

extern RTLFUNC(EOF);
extern RTLFUNC(FileAttr);
extern RTLFUNC(Loc);
extern RTLFUNC(Lof);
extern RTLFUNC(Seek);
extern RTLFUNC(SetAttr); // JSM
extern RTLFUNC(Reset); // JSM

extern RTLFUNC(DDEInitiate);
extern RTLFUNC(DDETerminate);
extern RTLFUNC(DDETerminateAll);
extern RTLFUNC(DDERequest);
extern RTLFUNC(DDEExecute);
extern RTLFUNC(DDEPoke);

extern RTLFUNC(FreeFile);
extern RTLFUNC(IsMissing);
extern RTLFUNC(LBound);
extern RTLFUNC(UBound);
extern RTLFUNC(RGB);
extern RTLFUNC(QBColor);
extern RTLFUNC(StrConv);

extern RTLFUNC(Beep);

extern RTLFUNC(Load);
extern RTLFUNC(Unload);
extern RTLFUNC(LoadPicture);
extern RTLFUNC(SavePicture);

extern RTLFUNC(CallByName);
extern RTLFUNC(CBool); // JSM
extern RTLFUNC(CByte); // JSM
extern RTLFUNC(CCur); // JSM
extern RTLFUNC(CDate); // JSM
extern RTLFUNC(CDbl); // JSM
extern RTLFUNC(CInt); // JSM
extern RTLFUNC(CLng); // JSM
extern RTLFUNC(CSng); // JSM
extern RTLFUNC(CStr); // JSM
extern RTLFUNC(CVar); // JSM
extern RTLFUNC(CVErr); // JSM

extern RTLFUNC(Iif); // JSM

extern RTLFUNC(DumpAllObjects);

extern RTLFUNC(GetSystemType);
extern RTLFUNC(GetGUIType);
extern RTLFUNC(Red);
extern RTLFUNC(Green);
extern RTLFUNC(Blue);

extern RTLFUNC(Switch);
extern RTLFUNC(Wait);
//i#64882# add new WaitUntil
extern RTLFUNC(WaitUntil);
extern RTLFUNC(FuncCaller);

extern RTLFUNC(GetGUIVersion);
extern RTLFUNC(Choose);
extern RTLFUNC(Trim);

extern RTLFUNC(DateAdd);
extern RTLFUNC(DateDiff);
extern RTLFUNC(DatePart);
extern RTLFUNC(FormatDateTime);
extern RTLFUNC(GetSolarVersion);
extern RTLFUNC(TwipsPerPixelX);
extern RTLFUNC(TwipsPerPixelY);
extern RTLFUNC(FreeLibrary);
extern RTLFUNC(Array);
extern RTLFUNC(FindObject);
extern RTLFUNC(FindPropertyObject);
extern RTLFUNC(EnableReschedule);

extern RTLFUNC(Put);
extern RTLFUNC(Get);
extern RTLFUNC(Environ);
extern RTLFUNC(GetDialogZoomFactorX);
extern RTLFUNC(GetDialogZoomFactorY);
extern RTLFUNC(GetSystemTicks);
extern RTLFUNC(GetPathSeparator);
extern RTLFUNC(ResolvePath);
extern RTLFUNC(CreateUnoStruct);
extern RTLFUNC(CreateUnoService);
extern RTLFUNC(CreateUnoServiceWithArguments);
extern RTLFUNC(CreateUnoValue);
extern RTLFUNC(GetProcessServiceManager);
extern RTLFUNC(GetDefaultContext);
extern RTLFUNC(CreatePropertySet);
extern RTLFUNC(CreateUnoListener);
extern RTLFUNC(HasUnoInterfaces);
extern RTLFUNC(EqualUnoObjects);
extern RTLFUNC(CreateUnoDialog);
extern RTLFUNC(GlobalScope);
extern RTLFUNC(FileExists);
extern RTLFUNC(ConvertToUrl);
extern RTLFUNC(ConvertFromUrl);
extern RTLFUNC(CDateToUnoDate);
extern RTLFUNC(CDateFromUnoDate);
extern RTLFUNC(CDateToUnoTime);
extern RTLFUNC(CDateFromUnoTime);
extern RTLFUNC(CDateToUnoDateTime);
extern RTLFUNC(CDateFromUnoDateTime);
extern RTLFUNC(CDateToIso);
extern RTLFUNC(CDateFromIso);
extern RTLFUNC(CompatibilityMode);
extern RTLFUNC(CDec);
extern RTLFUNC(CaptureAssertions);

extern RTLFUNC(Partition); // Fong

#ifdef DBG_TRACE_BASIC
extern RTLFUNC(TraceCommand);
#endif

extern double Now_Impl();
extern void Wait_Impl( bool bDurationBased, SbxArray& rPar );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
