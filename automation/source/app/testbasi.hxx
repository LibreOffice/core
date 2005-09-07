/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: testbasi.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:16:15 $
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

#ifndef _TTBASIC_HXX
#define _TTBASIC_HXX

#ifndef _SB_SBSTAR_HXX //autogen
#include <basic/sbstar.hxx>
#endif
#include <basic/mybasic.hxx>

class ErrorEntry;

#define SBXID_TTBASIC   0x5454      // TTBasic: TT

#define SBXCR_TEST2   0x54534554L   // TEST

class TTBasic : public MyBasic
{
public:
    SBX_DECL_PERSIST_NODATA(SBXCR_TEST2,SBXID_TTBASIC,1);
    TYPEINFO();
    TTBasic();
   ~TTBasic();
    BOOL Compile( SbModule* );
    static MyBasic* CreateMyBasic();

    // nicht mit #ifdefs klammern, da diese Headerdatei für testtool und basic
    // gleichermaßen verwendet wird.
    DECL_LINK( CErrorImpl, ErrorEntry* );
//  SbxObject *pTestObject;         // für das Testtool; ansonsten NULL

    void LoadIniFile();
    SbTextType GetSymbolType( const String &Symbol, BOOL bWasTTControl );   // Besimmt den erweiterten Symboltyp für das Syntaxhighlighting
    virtual const String GetSpechialErrorText();
    virtual void ReportRuntimeError( AppBasEd *pEditWin );
    virtual void DebugFindNoErrors( BOOL bDebugFindNoErrors );
};

SV_DECL_IMPL_REF(TTBasic)

#endif
