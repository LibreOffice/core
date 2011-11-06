/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _MSGEDIT_HXX
#define _MSGEDIT_HXX

#include <svtools/svtreebx.hxx>
#include <basic/testtool.hxx>
#include "dataedit.hxx"

class BasicFrame;
class AppError;

#define SelectChildren SelectChilds     // Sonst wird mir schlecht

typedef sal_uInt16 TTFeatures;          // Bitfield for features of the entries
#define HasNothing  TTFeatures(0x00)
#define HasError    TTFeatures(0x01)
#define HasWarning  TTFeatures(0x02)
#define HasAssertion TTFeatures(0x04)
#define HasQAError  TTFeatures(0x08)


class TTTreeListBox : public SvTreeListBox
{
protected:
//  virtual void    Command( const CommandEvent& rCEvt );
//  sal_uInt16          nDeselectParent;
    BasicFrame      *pBasicFrame;
    void            InitEntry( SvLBoxEntry*, const String&, const Image&,
                               const Image&, SvLBoxButtonKind eButtonKind );
    AppError        *pAppError;

    sal_Bool JumpToSourcecode( SvLBoxEntry *pThisEntry );

public:
    TTTreeListBox( AppError* pParent, BasicFrame* pBF, WinBits nWinStyle=0 );
    ~TTTreeListBox(){}

//  virtual void    SelectHdl();
//  virtual void    DeselectHdl();
    virtual sal_Bool    DoubleClickHdl();

    virtual void    KeyInput( const KeyEvent& rKEvt );

//  sal_uIntPtr         SelectChildren( SvLBoxEntry* pParent, sal_Bool bSelect );
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
