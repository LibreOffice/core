/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swprtopt.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:11:30 $
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

#ifndef _SWPRTOPT_HXX
#define _SWPRTOPT_HXX

#ifndef _SV_MULTISEL_HXX //autogen
#include <tools/multisel.hxx>
#endif
#ifndef _SW_PRINTDATA_HXX
#include <printdata.hxx>
#endif

#define POSTITS_NONE    0
#define POSTITS_ONLY    1
#define POSTITS_ENDDOC  2
#define POSTITS_ENDPAGE 3

class SwPrtOptions : public SwPrintData
{
    USHORT nJobNo;
    String sJobName;

public:
    MultiSelection  aMulti;
    Point  aOffset;
    ULONG  nMergeCnt;           // Anzahl der Serienbriefe
    ULONG  nMergeAct;           // Aktueller Serienbriefnr.
    USHORT nCopyCount;

    BOOL   bCollate,
           bPrintSelection,     // Markierung drucken
           bJobStartet;

    SwPrtOptions( const String& rJobName ) :
        nJobNo( 1 ),
        sJobName( rJobName ),
        aOffset( Point(0,0) ),
        nMergeCnt( 0 ),
        nMergeAct( 0 ),
        bCollate(FALSE),
        bPrintSelection (FALSE),
        bJobStartet(FALSE)
    {}

    const String& MakeNextJobName();        // steht in vprint.cxx
    const String& GetJobName() const    { return sJobName; }

#if defined(TCPP)
    // seit neuestem (SV 223) kann der keinen mehr generieren
    inline  SwPrtOptions(const SwPrtOptions& rNew) {*this = rNew;}
#endif


        SwPrtOptions& operator=(const SwPrintData& rData)
            {
                SwPrintData::operator=(rData);
                return *this;
            }
};


#endif //_SWPRTOPT_HXX















