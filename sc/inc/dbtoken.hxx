/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbtoken.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:31:25 $
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

#ifndef _DBTOKEN_HXX
#define _DBTOKEN_HXX

#ifndef SC_COMPILER_HXX
#include "compiler.hxx"
#endif


// SET DBG_TOKEN=string
// +  - make new data
// -  - do not make old data
// aA - dump old/new token array
// uU - dump old/new UPN array

BOOL DbgToken( char );
ScTokenArray* DbgMakeTokenArray
    ( ScDocument* pDoc, SCTAB nTab, const String& r, USHORT& rErr );
void DbgDumpTokenArray( const BYTE* pArr, USHORT nLen, const char* pMsg );
void DbgDelTokenArray( ScTokenArray* );
StackVar DbgInterpret( ScDocument* pDok, const ScTokenArray* pToken,
                       SCCOL nCOL, SCROW nROW, SCTAB nTAB,
                                  StackVar eStackVar,
                                  char* &rStringErgPtr,
                                  double& rDoubleErg,
                                  USHORT& rError,
                                  ScMatrix** ppMat);

#endif

