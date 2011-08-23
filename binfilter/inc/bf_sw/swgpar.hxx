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
#ifndef _SWGPAR_HXX
#define _SWGPAR_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
class UniString;
class SvStream;
namespace binfilter {


class SwDoc;
class SwPaM;
class SwSwgReader;


class SfxDocumentInfo;

enum SwgReadOptions {
    SWGRD_CHARFMTS 		= 0x0001,		// Zeichenvorlagen
    SWGRD_FRAMEFMTS 	= 0x0002,		// Rahmenvorlagen
    SWGRD_PARAFMTS		= 0x0004,		// Absatzvorlagen
    SWGRD_PAGEFMTS  	= 0x0008,		// Seitenvorlagen
    SWGRD_MACROS    	= 0x0010,		// globale Makros
    SWGRD_NUMRULES  	= 0x0020,		// globale Numerierungsregeln

    SWGRD_FORCE			= 0x1000,		// Vorlagen immer einlesen

    SWGRD_NORMAL		= 0x00FF		// alles (Normalfall)
};

class SwSwgParser
{
private:
    SwSwgReader* pRdr;
public:
    ULONG  CallParser( USHORT = SWGRD_NORMAL );
    BOOL NeedsPasswd();
    BOOL CheckPasswd( const UniString& );

    SwSwgParser( SwDoc *pSwDoc, const SwPaM* pSwPaM,
                 SvStream *pIstream, const UniString& rFileName, //$ istream
                 BOOL bNewDoc=TRUE );
    SwSwgParser( SvStream *pIstream ); //$ istream
    ~SwSwgParser();
};

} //namespace binfilter
#endif // _SWGPAR_HXX
