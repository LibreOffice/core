/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mybasic.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 12:50:47 $
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

#ifndef _MYBASIC_HXX
#define _MYBASIC_HXX

#ifndef _SB_SBSTAR_HXX //autogen
#include <basic/sbstar.hxx>
#endif

class BasicApp;
class AppBasEd;
class ErrorEntry;

class BasicError {
    AppBasEd* pWin;
    USHORT  nLine, nCol1, nCol2;
    String aText;
public:
    BasicError( AppBasEd*, USHORT, const String&, USHORT, USHORT, USHORT );
    void Show();
};

DECLARE_LIST( ErrorList, BasicError* )

#define SBXID_MYBASIC   0x594D      // MyBasic: MY
#define SBXCR_TEST      0x54534554  // TEST

class MyBasic : public StarBASIC
{
    SbError nError;
    virtual BOOL ErrorHdl();
    virtual USHORT BreakHdl();

protected:
    Link GenLogHdl();
    Link GenWinInfoHdl();
    Link GenModuleWinExistsHdl();
    Link GenWriteStringHdl();

    virtual void StartListeningTT( SfxBroadcaster &rBroadcaster );

    String GenRealString( const String &aResString );

public:
    SBX_DECL_PERSIST_NODATA(SBXCR_TEST,SBXID_MYBASIC,1);
    TYPEINFO();
    ErrorList aErrors;
    MyBasic();
    virtual ~MyBasic();
    virtual BOOL Compile( SbModule* );
    void Reset();
    SbError GetErrors() { return nError; }

    // nicht mit #ifdefs klammern, da diese Headerdatei für testtool und basic
    // gleichermaßen verwendet wird.
    SbxObject *pTestObject;         // für das Testtool; ansonsten NULL

    virtual void LoadIniFile();
    virtual SbTextType GetSymbolType( const String &Symbol, BOOL bWasTTControl );   // Besimmt den erweiterten Symboltyp für das Syntaxhighlighting
    virtual const String GetSpechialErrorText();
    virtual void ReportRuntimeError( AppBasEd *pEditWin );
    virtual void DebugFindNoErrors( BOOL bDebugFindNoErrors );

    static void SetCompileModule( SbModule *pMod );
    static SbModule *GetCompileModule();
};

SV_DECL_IMPL_REF(MyBasic)

#endif
