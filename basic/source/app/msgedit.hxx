/*************************************************************************
 *
 *  $RCSfile: msgedit.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:09 $
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

#ifndef _MSGEDIT_HXX
#define _MSGEDIT_HXX

#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtreebx.hxx>
#endif

#include "dataedit.hxx"
#include "testtool.hxx"
class BasicFrame;
class AppWin;

#define SelectChildren SelectChilds     // Sonst wird mir schlecht

typedef USHORT TTFeatures;          // Bitfeld für Features der Entrys
#define HasNothing  TTFeatures(0x00)
#define HasError    TTFeatures(0x01)
#define HasWarning  TTFeatures(0x02)
#define HasAssertion TTFeatures(0x04)


class TTTreeListBox : public SvTreeListBox
{
protected:
//  virtual void    Command( const CommandEvent& rCEvt );
//  USHORT          nDeselectParent;
    BasicFrame      *pBasicFrame;
    void            InitEntry( SvLBoxEntry*, const String&, const Image&, const Image& );
public:
    TTTreeListBox( Window* pParent, BasicFrame* pBF, WinBits nWinStyle=0 );
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
    AppWin* pAppWin;
    String aLogFileName;    // Name der Logdatei
public:
    MsgEdit( AppWin*, BasicFrame *pBF, const WinBits& );
    ~MsgEdit();
    void AddAnyMsg( TTLogMsg *LogMsg );
    void AddRun( String aMsg, TTDebugData aDebugData );
    void AddTestCase( String aMsg, TTDebugData aDebugData );
    void AddError( String aMsg, TTDebugData aDebugData );
    void AddCallStack( String aMsg, TTDebugData aDebugData );
    void AddMessage( String aMsg, TTDebugData aDebugData );
    void AddWarning( String aMsg, TTDebugData aDebugData );
    void AddAssertion( String aMsg, TTDebugData aDebugData );
DATA_FUNC_DEF( aEditTree, TTTreeListBox )
};

#endif
