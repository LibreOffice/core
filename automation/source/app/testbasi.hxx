/*************************************************************************
 *
 *  $RCSfile: testbasi.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-18 16:03:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2002
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2002 by Sun Microsystems, Inc.
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
#define SBXCR_TEST2     0x545345542 // TEST

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
