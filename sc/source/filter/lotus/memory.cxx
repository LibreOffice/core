/*************************************************************************
 *
 *  $RCSfile: memory.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:14 $
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
//  Bemerkung:  Variablen nicht ueber Headerfile, Module muessen sich
//              selbst per extern ihre Sachen besorgen!

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop


#include "scitems.hxx"
#include <svx/algitem.hxx>

#include "attrib.hxx"

#include "decl.h"
#include "tool.h"

extern const long       nStackSize;
extern const int        nAnzNRange;

extern ScDocument*      pDoc;

const long              nStackSize = 8L * 1024;     // -> form_xxx.cpp
const int               nAnzNRange = 2048;          // -> tool_xxx.cpp, max. 2048 Named Ranges

sal_Char*               pPuffer;                    // -> flt_xxx.cxx
sal_Char*               pDummy1;                    // -> flt_xxx.cxx, ScanVersion()
sal_Char*               pDummy2;                    // -> tool.cxx, CreateTable()

extern BYTE*            pFormelBuffer;              // -> tool.cxx, fuer OP_Formula()
BYTE*                   pFormelBuffer;

extern FormCache*       pValueFormCache;            // -> tool.cxx

sal_Char*               pStack;                     // -> formel.cxx
sal_Char*               pPuffer0;                   // -> formel.cxx
sal_Char*               pPuffer1;                   // -> formel.cxx
extern const int        nMaxPar;
const int               nMaxPar = 128;              // max. 128 Parameter werden unterstuetzt
sal_Char**              pPar;                       // -> formel.cxx, Pn()

#ifndef _DOS                                        // -> op.cxx
sal_Char*               pAnsi;
#endif
sal_Char*               pErgebnis;                  // -> op.cxx

extern BOOL             bFormInit;                  // -> tool.cxx, fuer GetFormHandle()
BOOL                    bFormInit;

extern SvxHorJustifyItem    *pAttrRight, *pAttrLeft, *pAttrCenter,
                            *pAttrRepeat, *pAttrStandard;   // -> tool.cxx, fuer GetFormAttr()
extern ScProtectionAttr*    pAttrUnprot;   // -> tool.cxx, fuer PutFormString()



BOOL MemNew( void )
{
    pPuffer = new sal_Char [ 32L*1024L ];

    pDummy1 = new sal_Char [ 32 ];

    pDummy2 = new sal_Char [ 32 ];

    pStack = new sal_Char [ nStackSize * 3 ];   // alle drei auf einmal

    pPuffer0 = pStack + nStackSize;
    pPuffer1 = pPuffer0 + nStackSize;

    pAnsi = new sal_Char [ 2048 ];

    pErgebnis = new sal_Char [ 32L*1024L ];

    pPar = new sal_Char *[ nMaxPar ];

    pFormelBuffer = new BYTE[ 4096 ];

    pValueFormCache = new FormCache( pDoc );

    // fuer tool.cxx::PutFormString()
    pAttrUnprot = new ScProtectionAttr( TRUE );
    pAttrRight = new SvxHorJustifyItem( SVX_HOR_JUSTIFY_RIGHT );
    pAttrLeft = new SvxHorJustifyItem( SVX_HOR_JUSTIFY_LEFT );
    pAttrCenter = new SvxHorJustifyItem( SVX_HOR_JUSTIFY_CENTER );
    pAttrRepeat = new SvxHorJustifyItem( SVX_HOR_JUSTIFY_REPEAT );
    pAttrStandard = new SvxHorJustifyItem( SVX_HOR_JUSTIFY_STANDARD );
    bFormInit = TRUE;

    return TRUE;
}


void MemDelete( void )
{
    delete[] pPuffer;
    delete[] pDummy1;
    delete[] pDummy2;
    delete[] pStack;
    delete[] pAnsi;
    delete[] pErgebnis;
    delete[] pPar;
    delete[] pFormelBuffer;

    delete pValueFormCache;
    delete pAttrRight;
    delete pAttrLeft;
    delete pAttrCenter;
    delete pAttrRepeat;
    delete pAttrStandard;
    delete pAttrUnprot;
}



