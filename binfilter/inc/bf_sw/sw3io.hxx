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
#ifndef _SW3IO_HXX
#define _SW3IO_HXX

#include <tools/solar.h>
class UniString; 

namespace binfilter {
class SvStorage; 
class SvStrings; 
class SvxMacroTableDtor; 

class Sw3IoImp;
class SwPaM;
class SwDoc;

class SwgReaderOption;

#define DRAWING_STREAM_NAME 	"DrawingLayer"

class Sw3Io
{
    friend class Sw3IoImp;
    Sw3IoImp* pImp;					// Implementationsdaten

    // nur ueber diese beiden Klassen darf ein Load/Save erfolgen!!
    friend class Sw3Reader;
    friend class Sw3Writer;

    ULONG Load( SvStorage*, SwPaM* = NULL );
    ULONG Save( SwPaM* = NULL, BOOL = TRUE );
    ULONG SaveAs( SvStorage*, SwPaM* = NULL, BOOL = TRUE );

public:
    Sw3Io( SwDoc& );
   ~Sw3Io();
    Sw3IoImp* GetImp() { return pImp; }
    void  SetReadOptions( const SwgReaderOption& rOpt, BOOL bOverwrite=TRUE );

     SvStorage* GetStorage();
    void  HandsOff();
    BOOL  SaveCompleted( SvStorage* );

     ULONG SaveStyles();
    // Erzeugen eines eindeutigen Streamnamens im gegebenen Storage
    static UniString UniqueName( SvStorage*, const sal_Char* pPrefix );

    // Ermitteln aller Sections eines Dokuments
};

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
