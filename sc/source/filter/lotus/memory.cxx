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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"
//  Bemerkung:  Variablen nicht ueber Headerfile, Module muessen sich
//              selbst per extern ihre Sachen besorgen!




#include "scitems.hxx"
#include <svx/algitem.hxx>
#include <editeng/justifyitem.hxx>

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

extern sal_uInt8*           pFormelBuffer;              // -> tool.cxx, fuer OP_Formula()
sal_uInt8*                  pFormelBuffer;

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

extern sal_Bool             bFormInit;                  // -> tool.cxx, fuer GetFormHandle()
sal_Bool                    bFormInit;

extern SvxHorJustifyItem    *pAttrRight, *pAttrLeft, *pAttrCenter,
                            *pAttrRepeat, *pAttrStandard;   // -> tool.cxx, fuer GetFormAttr()
extern ScProtectionAttr*    pAttrUnprot;   // -> tool.cxx, fuer PutFormString()



sal_Bool MemNew( void )
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

    pFormelBuffer = new sal_uInt8[ 4096 ];

    pValueFormCache = new FormCache( pDoc );

    // fuer tool.cxx::PutFormString()
    pAttrUnprot = new ScProtectionAttr( sal_True );
    pAttrRight = new SvxHorJustifyItem( SVX_HOR_JUSTIFY_RIGHT, ATTR_HOR_JUSTIFY );
    pAttrLeft = new SvxHorJustifyItem( SVX_HOR_JUSTIFY_LEFT, ATTR_HOR_JUSTIFY );
    pAttrCenter = new SvxHorJustifyItem( SVX_HOR_JUSTIFY_CENTER, ATTR_HOR_JUSTIFY );
    pAttrRepeat = new SvxHorJustifyItem( SVX_HOR_JUSTIFY_REPEAT, ATTR_HOR_JUSTIFY );
    pAttrStandard = new SvxHorJustifyItem( SVX_HOR_JUSTIFY_STANDARD, ATTR_HOR_JUSTIFY );
    bFormInit = sal_True;

    return sal_True;
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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
