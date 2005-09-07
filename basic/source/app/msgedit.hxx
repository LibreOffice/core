/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: msgedit.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:17:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _MSGEDIT_HXX
#define _MSGEDIT_HXX

#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtreebx.hxx>
#endif

#include "dataedit.hxx"
#include "testtool.hxx"
class BasicFrame;
class AppError;

#define SelectChildren SelectChilds     // Sonst wird mir schlecht

typedef USHORT TTFeatures;          // Bitfeld für Features der Entrys
#define HasNothing  TTFeatures(0x00)
#define HasError    TTFeatures(0x01)
#define HasWarning  TTFeatures(0x02)
#define HasAssertion TTFeatures(0x04)
#define HasQAError  TTFeatures(0x08)


class TTTreeListBox : public SvTreeListBox
{
protected:
//  virtual void    Command( const CommandEvent& rCEvt );
//  USHORT          nDeselectParent;
    BasicFrame      *pBasicFrame;
    void            InitEntry( SvLBoxEntry*, const String&, const Image&, const Image& );
    AppError        *pAppError;

    BOOL JumpToSourcecode( SvLBoxEntry *pThisEntry );

public:
    TTTreeListBox( AppError* pParent, BasicFrame* pBF, WinBits nWinStyle=0 );
    ~TTTreeListBox(){}

//  virtual void    SelectHdl();
//  virtual void    DeselectHdl();
    virtual BOOL    DoubleClickHdl();

    virtual void    KeyInput( const KeyEvent& rKEvt );

//  ULONG           SelectChildren( SvLBoxEntry* pParent, BOOL bSelect );
    TTFeatures      GetFeatures( SvLBoxEntry* );
};

class MsgEdit : public DataEdit
{
    SvLBoxEntry *pCurrentRun;
    SvLBoxEntry *pCurrentTestCase;
    SvLBoxEntry *pCurrentAssertion;
    SvLBoxEntry *pCurrentError;
protected:
    BOOL bModified;
    Link lModify;
    BasicFrame *pBasicFrame;
    BOOL bFileLoading;      // TRUE während eine Datei geladen wird.
    String Impl_MakeText( SvLBoxEntry *pEntry ) const;
    String Impl_MakeSaveText( SvLBoxEntry *pEntry ) const;
    String Impl_MakeSaveText( TTDebugData aData ) const;
    USHORT nVersion;        // Speichert die Dateiversion
    AppError* pAppError;
    String aLogFileName;    // Name der Logdatei

    static USHORT nMaxLogLen;
    static BOOL bLimitLogLen;
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

    static void SetMaxLogLen( USHORT nLen ) { nMaxLogLen = nLen; bLimitLogLen = TRUE; }
DATA_FUNC_DEF( aEditTree, TTTreeListBox )
};

#endif
