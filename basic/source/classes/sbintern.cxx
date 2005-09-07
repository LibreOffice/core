/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sbintern.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:26:31 $
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

#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif

#include "sbintern.hxx"
#include "sbunoobj.hxx"
#include "token.hxx"                // Tokenizer
#include "symtbl.hxx"               // Symbolverwaltung
#include "parser.hxx"               // Parser
#include "codegen.hxx"              // Code-Generator
#include "basmgr.hxx"
#pragma hdrstop

SV_IMPL_PTRARR(SbErrorStack, SbErrorStackEntry*)

SbiGlobals* GetSbData()
{
    SbiGlobals** pp = (SbiGlobals**) ::GetAppData( SHL_SBC );
    SbiGlobals* p = *pp;
    if( !p )
        p = *pp = new SbiGlobals;
    return p;
}

SbiGlobals::SbiGlobals()
{
    pInst = NULL;
    pMod  = NULL;
    pSbFac= NULL;
    pUnoFac = NULL;
    pTypeFac = NULL;
    pOLEFac = NULL;
    pCompMod = NULL; // JSM
    nInst = 0;
    nCode = 0;
    nLine = 0;
    nCol1 = nCol2 = 0;
    bCompiler = FALSE;
    bGlobalInitErr = FALSE;
    bRunInit = FALSE;
    eLanguageMode = SB_LANG_BASIC;
    pErrStack = NULL;
    pTransliterationWrapper = NULL;
    bBlockCompilerError = FALSE;
    pAppBasMgr = NULL;
}

SbiGlobals::~SbiGlobals()
{
    delete pErrStack;
    delete pSbFac;
    delete pUnoFac;
    delete pTransliterationWrapper;
}

