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

#ifndef _BUFFER_HXX
#define _BUFFER_HXX

#include <tools/solar.h>
#include <tools/string.hxx>

namespace binfilter {

class SbiParser;

class SbiBuffer { 					// Code/Konstanten-Puffer:
    SbiParser* pParser;				// fuer Fehlermeldungen
    char*	pBuf;					// Puffer-Pointer
    char*	pCur;					// aktueller Puffer-Pointer
    UINT32	nOff;					// aktuelles Offset
    UINT32	nSize;					// aktuelle Groesse
    short	nInc;					// Inkrement
    BOOL    Check( USHORT );		// Buffergroesse testen
public:
    SbiBuffer( SbiParser*, short );	// Inkrement
   ~SbiBuffer();
    BOOL operator += (const String&);// Basic-String speichern
    BOOL operator += (INT8);		// Zeichen speichern
    BOOL operator += (INT16);		// Integer speichern
    BOOL operator += (UINT8);		// Zeichen speichern
    BOOL operator += (UINT16);		// Integer speichern
    BOOL operator += (UINT32);		// Integer speichern
    BOOL operator += (INT32);		// Integer speichern
    char*  GetBuffer();				// Puffer rausgeben (selbst loeschen!)
    char*  GetBufferPtr(){ return pBuf; }
    UINT32 GetSize()				{ return nOff; }
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
