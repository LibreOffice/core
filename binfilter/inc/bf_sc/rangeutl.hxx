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

#ifndef SC_RANGEUTL_HXX
#define SC_RANGEUTL_HXX

#include <tools/string.hxx>
class SvStream;
namespace binfilter {

//------------------------------------------------------------------------


class ScArea;
class ScDocument;
class ScRefTripel;
class ScTripel;
class ScRange;
class ScRangeName;
class ScDBCollection;

enum RutlNameScope { RUTL_NONE=0, RUTL_NAMES, RUTL_DBASE };

//------------------------------------------------------------------------

class ScRangeUtil
{
public:
                ScRangeUtil()  {}
                ~ScRangeUtil() {}



    void	CutPosString		( const String&		theAreaStr,
                                  String&			thePosStr ) const;


    BOOL	IsAbsArea			( const String& rAreaStr,
                                  ScDocument*	pDoc,
                                  USHORT		nTab,
                                  String*		pCompleteStr = 0,
                                  ScRefTripel*	pStartPos	 = 0,
                                  ScRefTripel*	pEndPos		 = 0 ) const;


    BOOL	MakeRangeFromName	( const String&	rName,
                                    ScDocument*		pDoc,
                                    USHORT			nCurTab,
                                    ScRange&		rRange,
                                    RutlNameScope eScope=RUTL_NAMES
                                  ) const;
};

//------------------------------------------------------------------------

class ScArea
{
public:
            ScArea( USHORT tab      = 0,
                    USHORT colStart = 0,
                    USHORT rowStart = 0,
                    USHORT colEnd   = 0,
                    USHORT rowEnd   = 0 );

            ScArea( const ScArea& r );

    ScArea&	operator=	( const ScArea& r );

public:
    USHORT nTab;
    USHORT nColStart;
    USHORT nRowStart;
    USHORT nColEnd;
    USHORT nRowEnd;
};

/*N#116571#*/ SvStream& operator<< ( SvStream& rStream, const ScArea& rArea );
/*N#116571#*/ SvStream& operator>> ( SvStream& rStream, ScArea& rArea );

//------------------------------------------------------------------------

//
//	gibt Bereiche mit Referenz und alle DB-Bereiche zurueck
//



} //namespace binfilter
#endif // SC_RANGEUTL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
