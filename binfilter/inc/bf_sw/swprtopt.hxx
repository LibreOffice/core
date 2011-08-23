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

#ifndef _SWPRTOPT_HXX
#define _SWPRTOPT_HXX

#include <tools/multisel.hxx>
#include <printdata.hxx>
namespace binfilter {

#define POSTITS_NONE	0
#define POSTITS_ONLY	1
#define POSTITS_ENDDOC	2
#define POSTITS_ENDPAGE 3

class SwPrtOptions : public SwPrintData
{
    USHORT nJobNo;
    String sJobName;

public:
    SwPrtOptions( const String& rJobName ) : aOffset( Point(0,0) ),
        nMergeCnt( 0 ), nMergeAct( 0 ), sJobName( rJobName ),
        nJobNo( 1 ),  bCollate(FALSE), bPrintSelection (FALSE),
        bJobStartet(FALSE)
    {}

    const String& GetJobName() const 	{ return sJobName; }

#if defined(TCPP)
    // seit neuestem (SV 223) kann der keinen mehr generieren
    inline	SwPrtOptions(const SwPrtOptions& rNew) {*this = rNew;}
#endif

    MultiSelection	aMulti;
    Point  aOffset;
    ULONG  nMergeCnt;			// Anzahl der Serienbriefe
    ULONG  nMergeAct;			// Aktueller Serienbriefnr.
    USHORT nCopyCount;

    BOOL   bCollate,
           bPrintSelection,     // Markierung drucken
           bJobStartet;

        SwPrtOptions& operator=(const SwPrintData& rData)
            {
                SwPrintData::operator=(rData);
                return *this;
            }
};


} //namespace binfilter
#endif //_SWPRTOPT_HXX















/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
