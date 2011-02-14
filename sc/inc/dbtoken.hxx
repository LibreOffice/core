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

#ifndef _DBTOKEN_HXX
#define _DBTOKEN_HXX

#include "compiler.hxx"


// SET DBG_TOKEN=string
// +  - make new data
// -  - do not make old data
// aA - dump old/new token array
// uU - dump old/new UPN array

sal_Bool DbgToken( char );
ScTokenArray* DbgMakeTokenArray
    ( ScDocument* pDoc, SCTAB nTab, const String& r, sal_uInt16& rErr );
void DbgDumpTokenArray( const sal_uInt8* pArr, sal_uInt16 nLen, const char* pMsg );
void DbgDelTokenArray( ScTokenArray* );
formula::StackVar DbgInterpret( ScDocument* pDok, const ScTokenArray* pToken,
                       SCCOL nCOL, SCROW nROW, SCTAB nTAB,
                                  formula::StackVar eformula::StackVar,
                                  char* &rStringErgPtr,
                                  double& rDoubleErg,
                                  sal_uInt16& rError,
                                  ScMatrix** ppMat);

#endif

