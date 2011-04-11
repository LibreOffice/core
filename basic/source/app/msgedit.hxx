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

#ifndef _MSGEDIT_HXX
#define _MSGEDIT_HXX

#include <svtools/svtreebx.hxx>
#include <basic/testtool.hxx>
#include "dataedit.hxx"

class BasicFrame;
class AppError;

#define SelectChildren SelectChilds

typedef sal_uInt16 TTFeatures;          // Bitfield for features of the entries
#define HasNothing  TTFeatures(0x00)
#define HasError    TTFeatures(0x01)
#define HasWarning  TTFeatures(0x02)
#define HasAssertion TTFeatures(0x04)
#define HasQAError  TTFeatures(0x08)


class TTTreeListBox : public SvTreeListBox
{
protected:
    BasicFrame      *pBasicFrame;
    void            InitEntry( SvLBoxEntry*, const String&, const Image&,
                               const Image&, SvLBoxButtonKind eButtonKind );
    AppError        *pAppError;

    sal_Bool JumpToSourcecode( SvLBoxEntry *pThisEntry );

public:
    TTTreeListBox( AppError* pParent, BasicFrame* pBF, WinBits nWinStyle=0 );
    ~TTTreeListBox(){}

    virtual sal_Bool    DoubleClickHdl();

    virtual void    KeyInput( const KeyEvent& rKEvt );

    TTFeatures      GetFeatures( SvLBoxEntry* );
};

class MsgEdit : public DataEdit
{
    BasicFrame *pBasicFrame;

    SvLBoxEntry *pCurrentRun;
    SvLBoxEntry *pCurrentTestCase;
    SvLBoxEntry *pCurrentAssertion;
    SvLBoxEntry *pCurrentError;
    sal_Bool bModified;
    Link lModify;
    sal_Bool bFileLoading;      // sal_True while loading a file
    String Impl_MakeText( SvLBoxEntry *pEntry ) const;
    String Impl_MakeSaveText( SvLBoxEntry *pEntry ) const;
    String Impl_MakeSaveText( TTDebugData aData ) const;
    sal_uInt16 nVersion;        // Stores file version
    AppError* pAppError;
    String aLogFileName;

    static sal_uInt16 nMaxLogLen;
    static sal_Bool bLimitLogLen;
    static sal_Bool bPrintLogToStdout;
    static sal_Bool bPrintLogToStdoutSet;   // has it been initialized yet
public:
    MsgEdit( AppError*, BasicFrame *pBF, const WinBits& );
    ~MsgEdit();
    void AddAnyMsg( TTLogMsg *LogMsg );
    void AddRun( String aMsg, TTDebugData aDebugData );
    void AddTestCase( String aMsg, TTDebugData aDebugData );
    void AddError( String aMsg, TTDebugData aDebugData );
    void AddCallStack( String aMsg, TTDebugData aDebugData );
    void AddMessage( String aMsg, TTDebugData aDebugData );
    void AddWarning( String aMsg, TTDebugData aDebugData );
    void AddAssertion( String aMsg, TTDebugData aDebugData );
    void AddAssertionStack( String aMsg, TTDebugData aDebugData );
    void AddQAError( String aMsg, TTDebugData aDebugData );

    static void SetMaxLogLen( sal_uInt16 nLen ) { nMaxLogLen = nLen; bLimitLogLen = sal_True; }
DATA_FUNC_DEF( aEditTree, TTTreeListBox )
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
