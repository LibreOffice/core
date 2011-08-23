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
#ifndef __LOTPAR_HXX__
#define __LOTPAR_HXX__

#include "fltbase.hxx"
class SvStream; 
class String; 
namespace binfilter {

// einige Forward-Deklaratioen

class SwPaM;
class SwDoc;


enum WKTyp {
    WKUnknown = 0,	// = unbekanntes Format
    WKS,			// = 1-2-3/1, 1-2-3/1A
    WK1,			// = 1-2-3/2
    WK3,			// = 1-2-3/3
    WK4				// = 1-2-3/4
    };


class SwLotusParser : public SwFilterBase
{
    int 			bNewDoc : 1;

    WKTyp			eDateiTyp;

    // ---------------------------------------------------------------
    void Bof();					// 0x00
    void Dimensions();			// 0x06
    void Blank1();				// 0x0C
    void Integer1();			// 0x0D
    void Number1();				// 0x0E
    void Label1();				// 0x0F
    void Formula1();			// 0x10
    // ---------------------------------------------------------------
    void			PutCell( USHORT nCol, USHORT nRow, short nVal );
    void			PutCell( USHORT nCol, USHORT nRow, double fVal );
    void			PutCell( USHORT nCol, USHORT nRow, const String &rText,
                            char cJusty );
    // ---------------------------------------------------------------
    void			Parse( void );
    void			Init( void );
public:
    // Parameter bReadNewDoc gibt an, ob in ein bestehendes Dokument ein-
    // gelesen wird. TRUE: in ein "neues" Dokument einfuegen.
    SwLotusParser( SwDoc& rDoc, const SwPaM & rCrsr, SvStream& pIn,
        int bReadNewDoc /*= TRUE*/, CharSet eQ  );

    ~SwLotusParser();

    ULONG CallParser();
};



} //namespace binfilter
#endif
    // _EXLPAR_HXX

