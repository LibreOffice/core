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

#ifndef _MYBASIC_HXX
#define _MYBASIC_HXX

#include <basic/sbstar.hxx>
#include <vector>

class BasicApp;
class AppBasEd;
class ErrorEntry;

#define SBXID_MYBASIC   0x594D      // MyBasic: MY
#define SBXCR_TEST      0x54534554  // TEST

//-----------------------------------------------------------------------------
class BasicError {
    AppBasEd* pWin;
    sal_uInt16  nLine, nCol1, nCol2;
    String aText;
public:
    BasicError( AppBasEd*, sal_uInt16, const String&, sal_uInt16, sal_uInt16, sal_uInt16 );
    void Show();
};

//-----------------------------------------------------------------------------
class MyBasic : public StarBASIC
{
    SbError nError;
    virtual sal_Bool ErrorHdl();
    virtual sal_uInt16 BreakHdl();

protected:
    ::std::vector< BasicError* > aErrors;
    size_t CurrentError;
    Link GenLogHdl();
    Link GenWinInfoHdl();
    Link GenModuleWinExistsHdl();
    Link GenWriteStringHdl();

    virtual void StartListeningTT( SfxBroadcaster &rBroadcaster );

    String GenRealString( const String &aResString );

public:
    SBX_DECL_PERSIST_NODATA(SBXCR_TEST,SBXID_MYBASIC,1);
    TYPEINFO();
    MyBasic();
    virtual ~MyBasic();
    virtual sal_Bool Compile( SbModule* );
    void Reset();
    SbError GetErrors() { return nError; }
    size_t GetCurrentError() { return CurrentError; }
    BasicError* FirstError();
    BasicError* NextError();
    BasicError* PrevError();

    // Do not use #ifdefs here because this header file is both used for testtool and basic
    SbxObject *pTestObject; // for Testool; otherwise NULL

    virtual void LoadIniFile();

    // Determines the extended symbol type for syntax highlighting
    virtual SbTextType GetSymbolType( const String &Symbol, sal_Bool bWasTTControl );
    virtual const String GetSpechialErrorText();
    virtual void ReportRuntimeError( AppBasEd *pEditWin );
    virtual void DebugFindNoErrors( sal_Bool bDebugFindNoErrors );

    static void SetCompileModule( SbModule *pMod );
    static SbModule *GetCompileModule();
};

SV_DECL_IMPL_REF(MyBasic)

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
